#!/usr/bin/env python3
# Copyright (c) 2026 Arm Limited (or its affiliates). All rights reserved.
# SPDX-License-Identifier: Apache-2.0
"""Train, quantize, Vela-compile and emit the C sources for the ML-Model layer.

Produces, for each model, under Model/<name>/:

    <name>_int8.tflite       fully int8-quantized model, kept for recompilation
    <name>_int8_vela.tflite  Vela output for Ethos-U55-128, what the NPU runs
    <name>_model.c           the Vela build as a C array in section "ethos_model"
    VELA_SUMMARY.md          the Vela report, committed for review

It also prints one golden input/output pair per model as C, ready to paste into
Source/test_main.cpp, which embeds the vectors directly rather than linking a
generated table. The expected outputs come from the host TFLite reference
interpreter, so they are independent of the NPU under test.

Usage:
    python3 generate.py            retrain, re-quantize, re-run Vela
    python3 generate.py --compile  re-run Vela on the committed int8 models
    python3 generate.py --check    re-run Vela on the committed models and diff
"""

import argparse
import math
import os
import re
import shutil
import subprocess
import sys
import tempfile

os.environ.setdefault("TF_CPP_MIN_LOG_LEVEL", "3")

import numpy as np
import tensorflow as tf

HERE = os.path.dirname(os.path.abspath(__file__))
MODEL_DIR = os.path.dirname(os.path.dirname(HERE))
VELA_INI = os.path.join(MODEL_DIR, "vela.ini")

# Must match ethosu.num_macs in Board/Corstone-300/fvp_config_u55.txt.
ACCELERATOR = "ethos-u55-128"
SYSTEM_CONFIG = "Ethos_U55_High_End_Embedded"
MEMORY_MODE = "Shared_Sram"

SEED = 1
NUM_VECTORS = 8
# Which of the generated vectors test_main.cpp embeds. Index 2 is a strong,
# unambiguous case for both models: near the sine peak, and a wide-margin class.
SELECTED_VECTOR = 2


# --------------------------------------------------------------------------
# Model definitions
# --------------------------------------------------------------------------

def build_hello_world():
    """The classic TFLM 'Hello World': approximate y = sin(x).

    Dense-only, so the whole graph is FULLY_CONNECTED. Tiny and easy to read --
    this is the minimal end-to-end proof that the NPU path works.
    """
    samples = 1000
    x = np.random.uniform(0.0, 2.0 * math.pi, samples).astype(np.float32)
    y = np.sin(x).astype(np.float32) + 0.1 * np.random.randn(samples).astype(np.float32)

    model = tf.keras.Sequential([
        tf.keras.Input(shape=(1,), batch_size=1),
        tf.keras.layers.Dense(16, activation="relu"),
        tf.keras.layers.Dense(16, activation="relu"),
        tf.keras.layers.Dense(1),
    ])
    model.compile(optimizer="adam", loss="mse", metrics=["mae"])
    model.fit(x, y, epochs=500, batch_size=64, verbose=0)

    rep = x[:500].reshape(-1, 1, 1)

    # Test inputs sweep one period of the sine, so the printed device output is
    # directly comparable against sin(x).
    test_x = np.linspace(0.0, 2.0 * math.pi, NUM_VECTORS, dtype=np.float32)
    return model, rep, [np.array([[v]], dtype=np.float32) for v in test_x], None


def _patterns(count):
    """Synthetic 16x16 greyscale patterns in four classes.

    Deterministic and generated in-process, so the example needs no dataset
    download to be retrained.
    """
    xs = np.zeros((count, 16, 16, 1), dtype=np.float32)
    ys = np.zeros((count,), dtype=np.int32)
    rows, cols = np.mgrid[0:16, 0:16]

    for i in range(count):
        cls = i % 4
        phase = np.random.randint(0, 4)
        period = np.random.choice([3, 4, 5])
        if cls == 0:      # horizontal stripes
            img = ((rows + phase) % period == 0).astype(np.float32)
        elif cls == 1:    # vertical stripes
            img = ((cols + phase) % period == 0).astype(np.float32)
        elif cls == 2:    # diagonal stripes
            img = ((rows + cols + phase) % period == 0).astype(np.float32)
        else:             # blank
            img = np.zeros((16, 16), dtype=np.float32)
        img = img + 0.10 * np.random.randn(16, 16).astype(np.float32)
        xs[i, :, :, 0] = np.clip(img, 0.0, 1.0)
        ys[i] = cls

    return xs, ys


def build_tiny_cnn():
    """A small CNN that exercises the operators a real NPU workload uses.

    Covers CONV_2D, DEPTHWISE_CONV_2D, MAX_POOL_2D and FULLY_CONNECTED. Still
    tiny, but ~26k MACs rather than 280, so memory-mode changes are measurable.
    """
    x_train, y_train = _patterns(2048)
    x_val, y_val = _patterns(256)

    model = tf.keras.Sequential([
        # batch_size=1 keeps every shape static, so the converter emits no
        # SHAPE/STRIDED_SLICE/PACK operators for the reshape.
        tf.keras.Input(shape=(16, 16, 1), batch_size=1),
        tf.keras.layers.Conv2D(8, 3, padding="same", activation="relu"),
        tf.keras.layers.MaxPooling2D(2),
        tf.keras.layers.DepthwiseConv2D(3, padding="same", activation="relu"),
        tf.keras.layers.Conv2D(16, 1, activation="relu"),
        tf.keras.layers.MaxPooling2D(2),
        # Reshape with an explicit target shape, not Flatten: Flatten converts
        # to a dynamic SHAPE + RESHAPE pair, which forces the CPU interpreter to
        # carry shape operators it should not need on a fixed-size input.
        tf.keras.layers.Reshape((4 * 4 * 16,)),
        tf.keras.layers.Dense(4),
    ])
    model.compile(optimizer="adam",
                  loss=tf.keras.losses.SparseCategoricalCrossentropy(from_logits=True),
                  metrics=["accuracy"])
    model.fit(x_train, y_train, epochs=30, batch_size=64,
              validation_data=(x_val, y_val), verbose=0)

    acc = model.evaluate(x_val, y_val, verbose=0)[1]
    print(f"  tiny_cnn validation accuracy: {acc:.3f}")
    if acc < 0.90:
        raise SystemExit(f"tiny_cnn accuracy {acc:.3f} too low; refusing to ship it")

    rep = x_train[:256].reshape(-1, 1, 16, 16, 1)

    # Two held-out patterns per class, so the test can also assert that the NPU
    # predicts the right class and not merely a number matching the reference.
    test_x, test_y = _patterns(NUM_VECTORS)
    return model, rep, [test_x[i:i + 1] for i in range(NUM_VECTORS)], test_y


MODELS = {
    "hello_world": build_hello_world,
    "tiny_cnn": build_tiny_cnn,
}


# --------------------------------------------------------------------------
# Quantization, Vela, C emission
# --------------------------------------------------------------------------

def quantize(model, representative):
    """Full-integer int8 quantization with int8 input and output.

    Vela only offloads a fully quantized graph, and int8 I/O keeps the test
    vectors exact integers with no host-side conversion in the comparison.
    """
    converter = tf.lite.TFLiteConverter.from_keras_model(model)
    converter.optimizations = [tf.lite.Optimize.DEFAULT]
    converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
    converter.inference_input_type = tf.int8
    converter.inference_output_type = tf.int8
    converter.representative_dataset = lambda: ([s] for s in representative)
    return converter.convert()


def run_vela(tflite_path, out_dir):
    cmd = [
        "vela", tflite_path,
        "--accelerator-config", ACCELERATOR,
        "--config", VELA_INI,
        "--system-config", SYSTEM_CONFIG,
        "--memory-mode", MEMORY_MODE,
        "--output-dir", out_dir,
    ]
    proc = subprocess.run(cmd, capture_output=True, text=True)
    if proc.returncode != 0:
        sys.stderr.write(proc.stdout + proc.stderr)
        raise SystemExit(f"vela failed for {tflite_path}")

    report = proc.stdout
    cpu_ops = re.search(r"CPU operators\s*=\s*(\d+)", report)
    npu_ops = re.search(r"NPU operators\s*=\s*(\d+)", report)
    if not cpu_ops or int(cpu_ops.group(1)) != 0:
        sys.stderr.write(report)
        raise SystemExit(
            f"{os.path.basename(tflite_path)}: Vela left operators on the CPU. "
            "The NPU test asserts a single fused 'ethos-u' operator, so the "
            "model must offload completely."
        )
    print(f"  vela: {npu_ops.group(1)} NPU operators, 0 on the CPU")
    return report


def operator_names(tflite_bytes):
    """Builtin operators in a .tflite, so the C op resolver can be kept in step."""
    interp = tf.lite.Interpreter(model_content=tflite_bytes)
    ops = sorted({d["op_name"] for d in interp._get_ops_details()})
    return ops


def golden_vectors(tflite_bytes, samples, labels):
    """Reference input/output pairs from the host interpreter.

    'samples' are float inputs, quantized here with the model's own input
    scale/zero-point so the stored vectors are exactly the int8 the device will
    be handed. Outputs come from the reference interpreter, which makes them
    independent of both the CMSIS-NN kernels and the NPU being tested.
    """
    interp = tf.lite.Interpreter(model_content=tflite_bytes)
    interp.allocate_tensors()
    inp = interp.get_input_details()[0]
    out = interp.get_output_details()[0]

    scale, zero = inp["quantization"]
    vectors = []
    for i, sample in enumerate(samples):
        q = np.round(sample / scale + zero)
        q = np.clip(q, -128, 127).astype(np.int8).reshape(inp["shape"])

        interp.set_tensor(inp["index"], q)
        interp.invoke()
        result = interp.get_tensor(out["index"]).astype(np.int8)

        vectors.append((q.flatten(), result.flatten(),
                        -1 if labels is None else int(labels[i])))

    return vectors


def _rows(data, fmt, per_line):
    body = []
    for i in range(0, len(data), per_line):
        row = "".join(fmt(int(b)) for b in data[i:i + per_line])
        body.append("  " + row.rstrip())
    return "\n".join(body)


def c_bytes(name, data, per_line=12, attrs=""):
    """uint8 payload as hex."""
    joined = _rows(data, lambda b: f"0x{b & 0xff:02x}, ", per_line)
    return f"const {name}[]{attrs} = {{\n{joined}\n}};\n"


def c_int8(name, data, per_line=16):
    """int8 payload as signed decimal, so the values fit int8_t without casts."""
    joined = _rows(data, lambda b: f"{b:4d}, ", per_line)
    return f"static const int8_t {name}[] = {{\n{joined}\n}};\n"


HEADER = ("/* Generated by Model/hello_world/gen/generate.py -- do not edit by hand. */\n"
          "/* SPDX-License-Identifier: Apache-2.0 */\n\n")


def emit_model_c(path, name, npu_bytes):
    """Emit only the Vela build: that is what the firmware executes.

    The original int8 .tflite stays on disk beside it so the model can be
    recompiled for another NPU configuration or memory mode, but there is no
    reason to spend flash on a copy the device never runs.
    """
    # Placed in "ethos_model" (ROM2 = DDR4, reached over Axi1) and 16-byte aligned,
    # which both the flatbuffer accessors and the NPU command stream require.
    attrs = '\n    __attribute__((aligned(16), section("ethos_model")))'
    with open(path, "w", newline="\n") as f:
        f.write(HEADER)
        f.write("#include <stdint.h>\n\n")
        f.write(c_bytes(f"uint8_t {name}_int8_vela_tflite", npu_bytes, attrs=attrs))
        f.write(f"const uint32_t {name}_int8_vela_tflite_len = {len(npu_bytes)};\n")


def print_vector_snippet(name, vectors):
    """Print the chosen golden vector as C, ready to paste into test_main.cpp.

    The test embeds one input/output pair per model directly rather than
    linking a generated table, so this is the hand-off point between the
    generator and the firmware.
    """
    vin, vout, _ = vectors[SELECTED_VECTOR]
    print(f"\n  ---- paste into Source/test_main.cpp ----")
    print(c_int8(f"{name}_input", vin), end="")
    print(c_int8(f"{name}_expected", vout), end="")


def generate(out_root):
    np.random.seed(SEED)
    tf.random.set_seed(SEED)

    for name, builder in MODELS.items():
        print(f"[{name}]")
        out_dir = os.path.join(out_root, name)
        os.makedirs(out_dir, exist_ok=True)

        model, representative, test_inputs, test_labels = builder()
        cpu_bytes = quantize(model, representative)

        cpu_path = os.path.join(out_dir, f"{name}_int8.tflite")
        with open(cpu_path, "wb") as f:
            f.write(cpu_bytes)

        ops = operator_names(cpu_bytes)
        print(f"  cpu operators: {', '.join(ops)}")

        with tempfile.TemporaryDirectory() as tmp:
            report = run_vela(cpu_path, tmp)
            vela_src = os.path.join(tmp, f"{name}_int8_vela.tflite")
            npu_path = os.path.join(out_dir, f"{name}_int8_vela.tflite")
            shutil.copy(vela_src, npu_path)

        with open(npu_path, "rb") as f:
            npu_bytes = f.read()

        emit_model_c(os.path.join(out_dir, f"{name}_model.c"), name, npu_bytes)
        vectors = golden_vectors(cpu_bytes, test_inputs, test_labels)

        if test_labels is not None:
            correct = sum(1 for v in vectors if int(np.argmax(v[1])) == v[2])
            print(f"  golden vectors classify {correct}/{len(vectors)} correctly")
            if correct != len(vectors):
                raise SystemExit("reference interpreter misclassifies its own "
                                 "test vectors; regenerate or retrain")

        with open(os.path.join(out_dir, "VELA_SUMMARY.md"), "w", newline="\n") as f:
            f.write(f"# Vela report: {name}\n\n")
            f.write(f"- accelerator: `{ACCELERATOR}`\n")
            f.write(f"- system config: `{SYSTEM_CONFIG}`\n")
            f.write(f"- memory mode: `{MEMORY_MODE}`\n\n")
            f.write("```\n" + report.strip() + "\n```\n")

        print(f"  {len(cpu_bytes)} B int8 -> {len(npu_bytes)} B vela")
        print_vector_snippet(name, vectors)


def compile_committed():
    """Vela-compile the committed int8 models without retraining them."""
    for name in MODELS:
        print(f"[{name}]")
        out_dir = os.path.join(MODEL_DIR, name)
        cpu_path = os.path.join(out_dir, f"{name}_int8.tflite")
        if not os.path.exists(cpu_path):
            raise SystemExit(f"committed model is missing: {cpu_path}")

        with tempfile.TemporaryDirectory() as tmp:
            report = run_vela(cpu_path, tmp)
            vela_src = os.path.join(tmp, f"{name}_int8_vela.tflite")
            npu_path = os.path.join(out_dir, f"{name}_int8_vela.tflite")
            shutil.copy(vela_src, npu_path)

        with open(npu_path, "rb") as f:
            npu_bytes = f.read()
        emit_model_c(os.path.join(out_dir, f"{name}_model.c"), name, npu_bytes)

        with open(os.path.join(out_dir, "VELA_SUMMARY.md"), "w", newline="\n") as f:
            f.write(f"# Vela report: {name}\n\n")
            f.write(f"- accelerator: `{ACCELERATOR}`\n")
            f.write(f"- system config: `{SYSTEM_CONFIG}`\n")
            f.write(f"- memory mode: `{MEMORY_MODE}`\n\n")
            f.write("```\n" + report.strip() + "\n```\n")

        print(f"  generated {len(npu_bytes)} B Vela model and C array")

    return 0


def check():
    """Re-run Vela on the committed .tflite files and diff the results.

    Deliberately does not retrain: Keras training is not bit-reproducible across
    machines, so comparing freshly trained models would always report a
    difference and tell us nothing. What this does catch is the drift that
    actually invalidates the committed golden vectors -- a Vela version or
    configuration that no longer produces the command stream the device was
    tested against.
    """
    problems = []

    with tempfile.TemporaryDirectory() as tmp:
        for name in MODELS:
            out_dir = os.path.join(MODEL_DIR, name)
            cpu_path = os.path.join(out_dir, f"{name}_int8.tflite")
            npu_path = os.path.join(out_dir, f"{name}_int8_vela.tflite")

            if not (os.path.exists(cpu_path) and os.path.exists(npu_path)):
                problems.append(f"{name}: committed .tflite files are missing")
                continue

            print(f"[{name}]")
            with open(cpu_path, "rb") as f:
                cpu_bytes = f.read()

            run_vela(cpu_path, tmp)
            with open(os.path.join(tmp, f"{name}_int8_vela.tflite"), "rb") as f:
                fresh_npu = f.read()
            with open(npu_path, "rb") as f:
                committed_npu = f.read()

            if fresh_npu != committed_npu:
                problems.append(
                    f"{name}: this Vela build produces a different command stream "
                    f"({len(fresh_npu)} B) than the committed one ({len(committed_npu)} B)")
                continue

            generated = os.path.join(tmp, f"{name}_model.c")
            emit_model_c(generated, name, committed_npu)
            with open(generated) as a, open(os.path.join(out_dir, f"{name}_model.c")) as b:
                if a.read() != b.read():
                    problems.append(f"{name}: {name}_model.c is stale")

    if problems:
        print("\nProblems found:")
        for p in problems:
            print(f"  {p}")
        print("\nRe-run 'python3 generate.py' to regenerate.")
        return 1

    print("\nCommitted Vela output and model sources are current.")
    return 0


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--compile", action="store_true",
                        help="Vela-compile committed int8 models without retraining")
    parser.add_argument("--check", action="store_true",
                        help="re-run Vela on the committed .tflite files and diff")
    args = parser.parse_args()

    if args.compile and args.check:
        parser.error("--compile and --check are mutually exclusive")
    if args.compile:
        return compile_committed()
    if args.check:
        return check()

    generate(MODEL_DIR)
    return 0


if __name__ == "__main__":
    sys.exit(main())
