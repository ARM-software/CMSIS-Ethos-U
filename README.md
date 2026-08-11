[![License](https://img.shields.io/github/license/ARM-software/CMSIS-Ethos-U?label=License)](./LICENSE)
[![Pack](https://img.shields.io/github/actions/workflow/status/ARM-software/CMSIS-Ethos-U/pack.yml?logo=arm&logoColor=0091bd&label=Build%20pack)](./.github/workflows/pack.yml)
[![GH Pages](https://img.shields.io/github/actions/workflow/status/ARM-software/CMSIS-Ethos-U/gh-pages.yml?logo=arm&logoColor=0091bd&label=Deploy%20content)](./.github/workflows/gh-pages.yml)

# Arm CMSIS Ethos-U

This repository contains the source of the CMSIS software pack [`Arm::CMSIS-Ethos-U`](https://www.open-cmsis-pack.org/) containing the Arm Ethos-U NPU core driver and supporting documentation. The driver supplies the low-level interface between embedded software and Arm Ethos-U55, Ethos-U65, and Ethos-U85 NPUs.

The pack exposes a generic driver component for each supported NPU family. Applications can use the driver to initialize an NPU, invoke optimized neural-network command streams synchronously or asynchronously, handle interrupts, and collect performance-monitoring data.

Refer to the [CMSIS-Ethos-U documentation](https://https://arm-software.github.io/CMSIS-Ethos-U/main/index.html) for architecture concepts, Vela configuration, driver usage, and system-integration guidance.

## Supported NPUs

| NPU | Component variant | Device implementation |
|---|---|---|
| [Arm Ethos-U55](https://www.arm.com/products/silicon-ip-cpu/ethos/ethos-u55) | `Generic U55` | `source/src/ethosu_device_u55_u65.c` |
| [Arm Ethos-U65](https://www.arm.com/products/silicon-ip-cpu/ethos/ethos-u65) | `Generic U65` | `source/src/ethosu_device_u55_u65.c` |
| [Arm Ethos-U85](https://www.arm.com/products/silicon-ip-cpu/ethos/ethos-u85) | `Generic U85` | `source/src/ethosu_device_u85.c` |

The selected pack component supplies the corresponding configuration header and preprocessor definitions.

## Documentation

The documentation is organized into five sections:

- [General](https://arm-software.github.io/CMSIS-Ethos-U/main/general/index.html) introduces the architecture, terminology, memory modes, and deployment lifecycle.
- [Vela](https://arm-software.github.io/CMSIS-Ethos-U/main/vela/index.html) covers compiler installation, model compilation, configuration, memory placement, and diagnostics.
- [Driver](https://arm-software.github.io/CMSIS-Ethos-U/main/driver/index.html) describes the core-driver API, execution contract, platform hooks, PMU, and bring-up checks.
- [Integration](https://arm-software.github.io/CMSIS-Ethos-U/main/integration/index.html) connects Vela output to linker placement, memory attributes, cache policy, driver configuration, and system memory budgeting.
- [Zephyr](https://arm-software.github.io/CMSIS-Ethos-U/main/zephyr/index.html) explains how to configure, build, and run Ethos-U-accelerated Zephyr applications.

## Files and Directories

This is a list of the relevant files and directories.

| File or directory | Description |
|---|---|
| [`ARM.CMSIS-Ethos-U.pdsc`](./ARM.CMSIS-Ethos-U.pdsc) | Open-CMSIS-Pack description and Ethos-U driver component definitions. |
| [`source`](./source/) | Core-driver source, public headers, NPU configuration headers, CMake build files, and Zephyr module metadata. |
| [`source/README.md`](./source/README.md) | Driver build instructions, API usage, cache-coherency requirements, and platform hooks. |
| [`documentation`](./documentation/) | Doxygen source, generated HTML documentation, styles, and authoring guidance. |
| [`overview`](./overview/) | Overview content included in the generated software pack. |
| [`gen_pack.sh`](./gen_pack.sh) | Script that generates and validates the CMSIS software pack. |
| [`.github/workflows`](./.github/workflows/) | GitHub Actions for building the pack and publishing its documentation. |

## Building the Driver using CMake

Projects that build the driver directly with CMake select the target through `ETHOSU_TARGET_NPU_CONFIG`, for example `ethos-u55-128`.

The driver is intended to be cross-compiled for the Arm Cortex-M processor used by the target system. Configure the toolchain, processor, and Ethos-U configuration before building:

```bash
cmake -S source -B build \
  -DCMAKE_TOOLCHAIN_FILE=<toolchain> \
  -DCMAKE_SYSTEM_PROCESSOR=cortex-m<nr><features> \
  -DETHOSU_TARGET_NPU_CONFIG=ethos-u<nr>-<macs>
cmake --build build
```

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
