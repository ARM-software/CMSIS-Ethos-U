# Test-Ethos-U

This CMSIS-Toolbox integration test provides target solutions for
Arm Ethos-U55, Ethos-U65, and Ethos-U85 NPUs. The project runs on
the matching Corstone FVP simulation or hardware when used as a
[reference application](https://open-cmsis-pack.github.io/cmsis-toolbox/ReferenceApplications/).

The test demonstrates end-to-end TensorFlow Lite Micro (TFLM) integration for
Ethos-U systems. It builds and runs two Vela-compiled models, supplies a golden
input to each model, and compares the NPU output bit-for-bit with output captured
from the host TensorFlow Lite reference interpreter.

Key features include:

- **Full NPU offload:** both models contain zero CPU operators after Vela compilation.
- **Self-checking execution:** embedded host-reference vectors turn the example
  into an integration test with a clear `PASS` or `FAIL` result.
- **Layered CMSIS solution:** target-specific code is in a `Board` layer and
  ML specific content is in an `ML-Model` layer.
- **Model artifacts provided:** the original quantized `.tflite` files and the
  Vela-generated models can be recompiled.
- **Configurable integration:** the supplied `vela.ini` provides alternative
  memory modes, while the original models can be recompiled for other NPU
  configurations with the supplied model converter.

## Usage with Keil Studio

- [vcpkg-configuration.json](vcpkg-configuration.json) lists the tool dependencies that can be installed with
  [Arm Tools Environment Manager](https://marketplace.visualstudio.com/items?itemName=Arm.environment-manager).
- [Keil Studio for VS Code](https://marketplace.visualstudio.com/items?itemName=Arm.keil-studio-pack)
  can open the example. In the CMSIS view, use the
  [Action buttons](https://github.com/Open-CMSIS-Pack/vscode-cmsis-solution?tab=readme#action-buttons)
  to configure and build the example. Run the FVP from the command line as described below.

## Build and run from command line

By default, the committed Vela models target Ethos-U85 with 256 MACs.

To build this configuration execute:

```sh
cbuild Test-Ethos-U85.csolution.yml --active SSE-320-U85 --update-rte --packs
```

Once the application image is available, run it using the following command:

```sh
FVP_Corstone_SSE-320 -f Board/Corstone-320/fvp_config_u85.txt -a out/Test-Ethos-U/SSE-320-U85/Debug/Test-Ethos-U.hex
```

The test reports the detected NPU configuration followed by one result for each model.
A successful run ends with:

```text
[PASS] hello_world (max delta 0 LSB)
[PASS] tiny_cnn (max delta 0 LSB)

2 of 2 checks passed
TEST RESULT: PASS
```

For instructions on using the example with the Keil Studio IDE, see
[Tutorial: Create an Ethos-U application](https://arm-software.github.io/CMSIS_Ethos_U/latest/integration).

## Project structure

Project consists of three solutions:

- `Test-Ethos-U55.csolution.yml`: uses Cortex-M55 and Ethos-U55
- `Test-Ethos-U65.csolution.yml`: uses Cortex-M55 and Ethos-U65
- `Test-Ethos-U85.csolution.yml`: uses Cortex-M85 and Ethos-U85

Each solution contains one target and share project assembled from:

- `Test-Ethos-U.cproject.yml`: connects the layers and test application.
- `Board/Corstone-300/` and `Board/Corstone-320/`: board layers for SSE-300 and SSE-320 FVPs.
- `Model/`: contains TFLM components, tensor arena, original and Vela-compiled models, Vela configuration.
- `script/`: model converter and its documentation.
- `Source/test_main.cpp`: invokes both models and checks their output against the golden vectors.

## ML models

| ML model      | Purpose                                                     |
| ------------- | ----------------------------------------------------------- |
| `hello_world` | Approximates `sin(x)` with the classic TinyML dense network |
| `tiny_cnn`    | Classifies four types of synthetic 16 x 16 stripe patterns  |

`hello_world` is the classic TinyML "hello world" — the sine-approximation
model from the TensorFlow Lite Micro examples, popularized by Pete Warden and
Daniel Situnayake's book *TinyML*. It is the smallest thing that proves the
NPU path works end to end.

`tiny_cnn` was written for this example: a dense-only graph never touches
the convolution, depthwise convolution, and pooling paths that real
workloads depend on. It classifies synthetic 16x16 stripe patterns generated
in-process, so the test needs no external dataset.

Both supplied quantized models compile to **zero CPU operators** — the whole
graph runs on the NPU:

| ML model      | Operators exercised                                                         | Vela result  |
| ------------- | --------------------------------------------------------------------------- | ------------ |
| `hello_world` | `FULLY_CONNECTED`                                                           | 3 NPU, 0 CPU |
| `tiny_cnn`    | `CONV_2D`, `DEPTHWISE_CONV_2D`, `MAX_POOL_2D`, `RESHAPE`, `FULLY_CONNECTED` | 6 NPU, 0 CPU |

The original `.tflite` files are provided so the models can be recompiled for a
different NPU configuration or memory mode.

## Recompile the models

The model converter reads the generated `*.cbuild-mlops.yml` file and recompiles
the selected quantized models with the Vela settings for that solution. For
prerequisites, command-line and VS Code usage, configuration overrides, and
generated files, see the [LiteRT model converter documentation](script/README.md).

```console
python script/model-converter.py Test-Ethos-U85.cbuild-mlops.yml
```

## Exploring Ethos-U configurations

The Vela configuration, generated command stream, linker placement, and driver
region configuration describe the same memory system and must remain
consistent.

The
[Configure Ethos-U for FVP Simulation Models](https://arm-software.github.io/CMSIS_Ethos_U/latest/integration/fvp-ethos-setup.html)
reference lists the supported configurations and their corresponding values.

After selecting a configuration, apply the values from its table column and
review these project settings together:

- the `System_Config`, `Memory_Mode`, and `arena_cache_size` values in `Model/vela.ini`;
- `NPU_QCONFIG` and `NPU_REGIONCFG_*` in `Test-Ethos-U*.csolution.yml`;
- the `ethos_model`, `ethos_arena`, and `ethos_cache` linker sections in the Board layer; and
- the cache, security, and MPU/SAU attributes for those physical memories.

The default `arena_cache_size` is 384 KiB and must match `ETHOS_CACHE_SIZE`
in boards layer `ethos_setup.c`.

## Details

For the general device-integration workflow, memory mapping rules, cache hooks,
address remapping, and driver configuration, see:

- [Ethos-U Integration for Cortex-M](https://arm-software.github.io/CMSIS_Ethos_U/latest/integration).
