# Overview

The **Arm CMSIS Ethos-U pack** provides the low-level driver for Arm [Ethos-U55](https://www.arm.com/products/silicon-ip-cpu/ethos/ethos-u55),
  [Ethos-U65](https://www.arm.com/products/silicon-ip-cpu/ethos/ethos-u65), and
  [Ethos-U85](https://www.arm.com/products/silicon-ip-cpu/ethos/ethos-u85) neural processing units (NPUs). The driver connects an
ML inference runtime to the NPU and supports initialization, neural-network
command-stream execution, interrupt handling, and performance monitoring.

Before deployment, the [Vela compiler](https://arm-software.github.io/CMSIS-Ethos-U/main/vela/index.html)
optimizes a quantized ML model for the selected Ethos-U target. At runtime, the
Cortex-M application and ML inference runtime use this pack's driver to submit
the compiled command stream and model memory regions to the NPU.

## Memory modes

Ethos-U systems can place model constants, the tensor arena, and temporary data
in different memories. Vela uses the selected memory mode and the target's
memory-performance configuration to optimize data movement and execution.

![Comparison of Ethos-U memory modes](./memory-modes.png)

- **SRAM-only mode** keeps both constants and the tensor arena in SRAM for low
  access latency, but the complete model and arena must fit there.
- **Shared-SRAM mode** keeps the tensor arena in SRAM while read-only constants
  reside in Flash, MRAM, or DRAM, reducing SRAM usage.
- **Dedicated-SRAM mode** places constants and the tensor arena in larger memory
  and uses SRAM as a fast staging area for selected data.

See the [General documentation](https://arm-software.github.io/CMSIS-Ethos-U/main/general/index.html)
for memory-mode concepts and the [Integration documentation](https://arm-software.github.io/CMSIS-Ethos-U/main/integration/index.html)
for linker placement, memory attributes, cache policy, and driver configuration.

## Integration and tutorial

The [Integration guide](https://arm-software.github.io/CMSIS-Ethos-U/main/integration/index.html)
describes the end-to-end workflow for creating an ML application for your
target hardware. It covers selecting the device configuration, creating a CMSIS
solution project, compiling the model with Vela, placing the generated model
regions in physical memory, configuring the driver, and validating the complete
system.

The [tutorial](https://arm-software.github.io/CMSIS-Ethos-U/main/integration/index.html#tutorial)
uses Keil Studio for VS Code to start from an example that matches the target's
Ethos-U NPU. It shows how to obtain the resolved MLOps and Vela settings,
generate an NPU-optimized model, build the application, and extend the solution
with a board layer and configuration for the physical target hardware.

The pack includes three `Hello-Ethos-U` CMSIS solution examples. Each includes
an ML model and FVP simulator configuration for initial validation. You can extend an
example with other ML models and a configuration for the target hardware to
validate the system integration. Select the example that matches the target NPU:

| Example | NPU | FVP Simulation Model |
|---|---|---|
| `Hello-Ethos-U55` | Ethos-U55 | V2M-MPS3-SSE-300 FVP |
| `Hello-Ethos-U65` | Ethos-U65 | V2M-MPS3-SSE-300 FVP |
| `Hello-Ethos-U85` | Ethos-U85 | SSE-320 FVP |

## Zephyr

Zephyr applications use the Ethos-U driver to execute Vela-compiled ML models.
The Vela target and memory configuration must match the driver and Zephyr board
integration. See the
[Zephyr chapter](https://arm-software.github.io/CMSIS-Ethos-U/main/zephyr/index.html)
for details.

## Features

- Single-variant core-driver components for Arm Ethos-U55, Ethos-U65, and
  Ethos-U85, plus a multi-variant component for run-time NPU selection.
- Synchronous and asynchronous command-stream execution.
- Configurable command-stream and model-region memory access.
- Interrupt handling and Performance Monitoring Unit (PMU) support.
- Per-family configuration and per-instance configuration for multi-variant
  systems.
- Platform hooks for cache maintenance, address translation, locking, and
  logging.
- Guidance for Vela configuration, driver use, system integration, and Zephyr
  deployment.

## Links

- [Documentation](https://arm-software.github.io/CMSIS-Ethos-U/main/index.html)
- [Zephyr](https://arm-software.github.io/CMSIS-Ethos-U/main/zephyr/index.html)
- [Repository](https://github.com/Arm-Software/CMSIS-Ethos-U)
- [Issues](https://github.com/Arm-Software/CMSIS-Ethos-U/issues)
