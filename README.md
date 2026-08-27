[![License](https://img.shields.io/github/license/ARM-software/CMSIS-Ethos-U?label=License)](./LICENSE)
[![Pack](https://img.shields.io/github/actions/workflow/status/ARM-software/CMSIS-Ethos-U/pack.yml?logo=arm&logoColor=0091bd&label=Build%20pack)](./.github/workflows/pack.yml)
[![GH Pages](https://img.shields.io/github/actions/workflow/status/ARM-software/CMSIS-Ethos-U/gh-pages.yml?logo=arm&logoColor=0091bd&label=Deploy%20content)](./.github/workflows/gh-pages.yml)

# Arm CMSIS Ethos-U

This repository contains the source of the CMSIS software pack [`ARM::CMSIS-Ethos-U`](https://www.open-cmsis-pack.org/) containing the Arm Ethos-U NPU core driver and supporting documentation. The driver supplies the low-level interface between embedded software and Arm Ethos-U55, Ethos-U65, and Ethos-U85 NPUs.

The pack provides a single-variant driver component for each supported NPU
family and a multi-variant component containing support for all three families.
A single-variant build selects the NPU at compile time. A multi-variant build
selects the NPU product and MAC configuration for each driver instance at run
time.

Applications can use the driver to initialize an NPU, configure memory access,
invoke Vela-optimized ML models synchronously or asynchronously, handle
interrupts, and collect performance-monitoring data.

Refer to the [CMSIS-Ethos-U documentation](https://arm-software.github.io/CMSIS-Ethos-U/main/general/index.html) for architecture concepts, Vela configuration, driver usage, and system-integration guidance.

## Supported NPUs

| NPU support | Component variant | Backend implementation |
|---|---|---|
| [Arm Ethos-U55](https://www.arm.com/products/silicon-ip-cpu/ethos/ethos-u55) | `Generic U55` | `source/src/ethosu_backend_u55.c` |
| [Arm Ethos-U65](https://www.arm.com/products/silicon-ip-cpu/ethos/ethos-u65) | `Generic U65` | `source/src/ethosu_backend_u65.c` |
| [Arm Ethos-U85](https://www.arm.com/products/silicon-ip-cpu/ethos/ethos-u85) | `Generic U85` | `source/src/ethosu_backend_u85.c` |
| Ethos-U55, Ethos-U65, and Ethos-U85 | `Multi-Variant` | All three backends |

The selected single-variant component supplies the corresponding configuration
header and NPU-family definition. The `Multi-Variant` component supplies all
three backends and defines `ETHOSU_MULTI_VARIANT`.

## Documentation

The documentation is organized into five sections:

- [General](https://arm-software.github.io/CMSIS-Ethos-U/main/general/index.html) introduces the architecture, terminology, memory modes, and deployment lifecycle.
- [Vela](https://arm-software.github.io/CMSIS-Ethos-U/main/vela/index.html) covers compiler installation, model compilation, configuration, memory placement, and diagnostics.
- [Driver](https://arm-software.github.io/CMSIS-Ethos-U/main/driver/index.html) describes single-variant and multi-variant configuration, memory access, the core-driver API, execution contract, platform hooks, PMU, and bring-up checks.
- [Integration](https://arm-software.github.io/CMSIS-Ethos-U/main/integration/index.html) connects Vela output to linker placement, memory attributes, cache policy, driver configuration, and system memory budgeting.
- [Zephyr](https://arm-software.github.io/CMSIS-Ethos-U/main/zephyr/index.html) explains how to configure, build, and run Ethos-U-accelerated Zephyr applications.

## Files and Directories

This is a list of the relevant files and directories.

| File or directory | Description |
|---|---|
| [`source`](./source/) | Core-driver source, public headers, single- and multi-variant NPU support, CMake build files, and Zephyr module metadata. |
| [`interface`](./interface/) | CMSIS-RTOS2 and data-cache interface templates supplied by the driver components. |
| [`examples`](./examples/Hello-Ethos-U/) | CMSIS-Toolbox example solutions for Ethos-U55, Ethos-U65, and Ethos-U85, including Corstone targets, TFLM models, and self-checking tests. |
| [`documentation`](./documentation/) | Doxygen sources for the general, Vela, driver, integration, and Zephyr guides, plus generated web content, styles, and authoring guidance. |
| [`overview`](./overview/) | Pack overview content and images displayed by CMSIS tooling. |
| [`.github`](./.github/) | Workflows that test the example, build the pack, and publish documentation. |

## Building the Driver using CMake

Projects that build a single-variant driver directly with CMake select the target
through `ETHOSU_TARGET_NPU_CONFIG`, for example `ethos-u55-128`.

The driver is intended to be cross-compiled for the Arm Cortex-M processor used by the target system. Configure the toolchain, processor, and Ethos-U configuration before building:

```bash
cmake -S source -B build \
  -DCMAKE_TOOLCHAIN_FILE=<toolchain> \
  -DCMAKE_SYSTEM_PROCESSOR=cortex-m<nr><features> \
  -DETHOSU_TARGET_NPU_CONFIG=ethos-u<nr>-<macs>
cmake --build build
```

For a multi-variant build, set `ETHOSU_MULTI_VARIANT=ON` instead of selecting
`ETHOSU_TARGET_NPU_CONFIG`. The build includes support for all three NPU
families. Applications use `ethosu_init_ex()` and `ethosu_reserve_driver_ex()`
to select the product and MAC configuration at run time.

When using a toolchain from the Ethos-U core platform, set `TARGET_CPU` instead of `CMAKE_SYSTEM_PROCESSOR`. See the [driver README](./source/README.md) for API examples and integration requirements.

## Pack and Documentation Generation

Generate the documentation from a Bash shell with:

```bash
./documentation/doxygen/gen_doc.sh
```

Generate the CMSIS-Ethos-U software pack with:

```bash
./gen_pack.sh
```

The pack script builds the documentation as a preprocessing step and writes the resulting archive to `output` by default. See the [documentation authoring guide](./documentation/README.md) for prerequisites and document structure.

## Continuous Integration (CI)

| CI workflow | Description |
|---|---|
| [`pack`](./.github/workflows/pack.yml) | Generates the documentation and software pack for pull requests, pushes to `main`, and published releases. |
| [`gh-pages`](./.github/workflows/gh-pages.yml) | Deploys the generated content from the `gh-pages` branch to GitHub Pages. |

## License

The CMSIS-Ethos-U pack and core driver are licensed under the [Apache License 2.0](./LICENSE).

## Contributions and Issues

Contributions are accepted under the Apache License 2.0 and must include a Developer Certificate of Origin sign-off. See the [driver contribution guidance](./source/README.md#contributions) for details.

Use [GitHub Issues](https://github.com/Arm-Software/CMSIS-Ethos-U/issues) to report defects, request enhancements, or discuss documentation improvements. For security-related matters, follow the [security policy](./source/SECURITY.md).
