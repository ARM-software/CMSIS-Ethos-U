# Hello-Ethos-U

This CMSIS-Toolbox example provides target solutions for Arm Ethos-U55,
Ethos-U65, and Ethos-U85 NPUs. The shared project runs on the matching
Corstone FVP simulation or hardware when used as a
[reference application](https://open-cmsis-pack.github.io/cmsis-toolbox/ReferenceApplications/).*

The example demonstrates an end-to-end TensorFlow Lite Micro (TFLM) integration
for an Ethos-U85 system. It builds and runs two Vela-compiled models, supplies a
golden input to each model, and compares the NPU output bit-for-bit with output
captured from the host TensorFlow Lite reference interpreter.

Key features include:

- **Full NPU offload:** both models contain zero CPU operators after Vela
  compilation.
- **Self-checking execution:** embedded host-reference vectors turn the example
  into an integration test with a clear `PASS` or `FAIL` result.
- **Layered CMSIS solution:** target-specific code is in a `Board` layer and
  TFLM, model, and tensor-arena content is in an `ML-Model` layer.
- **Model artifacts provided:** the original quantized `.tflite` files and the
  Vela-generated models can be recompiled.
- **Configurable integration:** the supplied `vela.ini` provides alternative
  memory modes, while the original models can be recompiled for other NPU
  configurations by updating the related generator and platform settings.

## Usage with Keil Studio

The example is located in the `examples/Hello-Ethos-U` directory.

- [vcpkg-configuration.json](vcpkg-configuration.json) lists the tool
  dependencies that can be installed with
  [Arm Tools Environment Manager](https://marketplace.visualstudio.com/items?itemName=Arm.environment-manager).
- [Keil Studio for VS Code](https://marketplace.visualstudio.com/items?itemName=Arm.keil-studio-pack)
  can open the example. In the CMSIS view, use the
  [Action buttons](https://github.com/Open-CMSIS-Pack/vscode-cmsis-solution?tab=readme#action-buttons)
  to configure and build the example. Run the FVP from the command line as
  described below.

## Build and run from command line

By default, the committed Vela models target Ethos-U55 with 128 MACs. Build and
run this configuration from the
`examples/Hello-Ethos-U` directory:

```console
cbuild Hello-Ethos-U55.csolution.yml --active SSE-300-U55 --update-rte --packs
FVP_Corstone_SSE-300_Ethos-U55 -f Board/Corstone-300/fvp_config_u55.txt -a out/Hello-Ethos-U/SSE-300-U55/Debug/Hello-Ethos-U.hex
```

The test reports the detected NPU configuration followed by one result for each
model. A successful run ends with:

```text
[PASS] hello_world (max delta 0 LSB)
[PASS] tiny_cnn (max delta 0 LSB)

2 of 2 checks passed
TEST RESULT: PASS
```

For instructions on using the example with the Keil Studio IDE, see
[Tutorial: Create an Ethos-U application](https://arm-software.github.io/CMSIS_Ethos_U/latest/integration).

## Project structure

The `Hello-Ethos-U55.csolution.yml`, `Hello-Ethos-U65.csolution.yml`, and
`Hello-Ethos-U85.csolution.yml` files each contain one target and share one
project assembled from these parts:

- `Hello-Ethos-U.cproject.yml`: connects the layers and test application.
- `Board/Corstone-320/`: device startup, UART standard I/O, memory layout,
  Ethos-U85 driver, interrupt wiring, and FVP configuration.
- `Model/`: TFLM components, tensor arena, original and Vela-compiled models,
  model generator, and Vela configuration.
- `Source/test_main.cpp`: invokes both models and checks their output against
  the golden vectors.

## ML models

| ML model | Purpose | Operators exercised | Vela result |
| --- | --- | --- | --- |
| `hello_world` | Approximates `sin(x)` with the classic TinyML dense network | `FULLY_CONNECTED` | 3 NPU, 0 CPU |
| `tiny_cnn` | Classifies four types of synthetic 16 x 16 stripe patterns | `CONV_2D`, `DEPTHWISE_CONV_2D`, `MAX_POOL_2D`, `RESHAPE`, `FULLY_CONNECTED` | 6 NPU, 0 CPU |

`hello_world` is the classic TinyML "hello world" — the sine-approximation
model from the TensorFlow Lite Micro examples, popularized by Pete Warden and
Daniel Situnayake's book *TinyML*. It is the smallest thing that proves the
NPU path works end to end.

`tiny_cnn` was written for this example: a dense-only graph never touches
the convolution, depthwise convolution, and pooling paths that real
workloads depend on. It classifies synthetic 16x16 stripe patterns generated
in-process, so retraining needs no dataset download.

Both models are trained, quantized, and Vela-compiled by
`Model/hello_world/gen/generate.py`, and both compile to **zero CPU operators** — the whole
graph runs on the NPU. Only the Vela builds are linked into the firmware; the
original `.tflite` files is provided so the models can be
recompiled for a different NPU configuration or memory mode.

## Regenerate or check the models

The generator requires Python packages pinned in `Model/hello_world/gen/requirements.txt`.
Install them and check that the committed Vela output is current:

```console
python3 -m pip install -r Model/hello_world/gen/requirements.txt
python3 Model/hello_world/gen/generate.py --check
```

To retrain, quantize, and Vela-compile both models:

```console
python3 Model/hello_world/gen/generate.py
```

To Vela-compile the committed quantized models without retraining:

```console
python3 Model/hello_world/gen/generate.py --compile
```

The full generation command updates each model's `.tflite`, generated C array,
and Vela summary. It also prints new golden vectors ready to paste into
`Source/test_main.cpp`. Training is not bit-reproducible; use `--check` when the
goal is only to detect Vela output drift without retraining.

## Exploring Ethos-U configurations

The Vela configuration, generated command stream, linker placement, and driver
region configuration describe the same memory system and must remain
consistent. When changing the configuration, review these together:

- the `System_Config`, `Memory_Mode`, and `arena_cache_size` values in
  `Model/vela.ini`;
- `NPU_QCONFIG` and `NPU_REGIONCFG_*` in
  `Hello-Ethos-U85.csolution.yml`;
- the `ethos_model`, `ethos_arena`, and `ethos_cache` linker sections in
  the Board layer; and
- the cache, security, and MPU/SAU attributes for those physical memories.

The shipped `arena_cache_size` is 384 KiB and must match
`ETHOS_CACHE_BUF_SIZE` in `Board/Corstone-320/ethos_setup.c`. A cache-size or
memory-region mismatch can select the wrong NPU access path or cause the NPU to
read outside the configured buffer.

The committed Vela models use `ethos-u55-128` and match the
`ethosu.num_macs=128` setting in `Board/Corstone-300/fvp_config_u55.txt`. The CI
workflow recompiles the models for every tested target; its accelerator setting
must match the MAC configuration in the corresponding FVP configuration file.

## Details

For the general device-integration workflow, memory mapping rules, cache hooks,
address remapping, and driver configuration, see 
[Ethos-U Integration for Cortex-M](https://arm-software.github.io/CMSIS_Ethos_U/latest/integration).
