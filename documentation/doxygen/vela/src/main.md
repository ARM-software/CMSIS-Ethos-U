# Vela

Arm Vela is an ahead-of-time (AOT) neural network model compiler for the
[Ethos-U55](https://www.arm.com/products/silicon-ip-cpu/ethos/ethos-u55),
[Ethos-U65](https://www.arm.com/products/silicon-ip-cpu/ethos/ethos-u65), and
[Ethos-U85](https://www.arm.com/products/silicon-ip-cpu/ethos/ethos-u85) NPUs.
It converts a quantized TensorFlow Lite
(`.tflite`) or TOSA (`.tosa`) ML model into command streams and encoded constant
data for a selected Ethos-U configuration.

This chapter covers Vela 5.1 and later. Use `vela --version` to identify the
installed version and consult its release notes for version-specific changes.

## Feature overview

The Vela compiler performs the offline, target-specific part of an Ethos-U
deployment:

- Reads quantized TFLite/LiteRT and TOSA ML models. Activations and weights must
  be quantized for Ethos-U acceleration.
- Detects operations supported by the selected NPU. In TFLite ML models,
  unsupported regions remain CPU operations; TOSA compilation has no CPU
  fallback.
- Rewrites, decomposes, packs, and fuses graph operations into Ethos-U work.
- Selects execution schedules and block configurations for the target MAC
  configuration.
- Compresses weights and uses cascading, striping, buffering, and tensor reuse
  to reduce storage and runtime tensor-memory demand.
- Allocates tensors among the constant, arena, and cache memory areas described
  by the selected system configuration and memory mode.
- Optimizes either for performance or for minimum peak SRAM use.
- Generates an optimized TFLite model or raw command-stream data.
- Reports operator placement, estimated cycles, bandwidth, and memory use.

Striping divides feature maps into smaller regions to reduce the active working
set. Cascading feeds each region directly through a sequence of compatible
operations, allowing Vela to use rolling buffers instead of storing complete
intermediate feature maps.

For the normal deployment flow, see the
<a href="../general/index.html#system-overview">system overview</a>.

For TFLite output, supported regions become Ethos-U custom operators containing
the NPU command stream and related data. Unsupported TFLite operators remain in
the model for CPU execution, commonly using TensorFlow Lite Micro reference or
[CMSIS-NN](https://www.keil.arm.com/packs/cmsis-nn-arm) kernels.
Always review compiler warnings and `--show-cpu-operations`;
a successful compilation does not imply that every operation runs on the NPU.

The Vela compiler's cycle and bandwidth figures are cost-model estimates. Use
them to guide compiler decisions and compare like-for-like builds. Validate
final performance on an FPGA or target silicon; an
[Arm Fixed Virtual Platform (FVP)](https://www.arm.com/products/development-tools/simulation/fixed-virtual-platforms)
can provide approximate behavior.

## Installation

The Vela compiler runs on Linux, macOS, and Windows. The released package
requires Python 3.10 or newer:

```console
python -m pip install ethos-u-vela
vela --version
```

### Update an existing installation

Plain `pip install` considers an already installed version satisfactory and
does not necessarily install a newer release. Request an upgrade explicitly:

```console
python -m pip install --upgrade ethos-u-vela
python -m pip show ethos-u-vela
vela --version
```

#### Troubleshooting installation

If PyPI reports a newer package but `vela --version` still shows the old
version, the `pip` command and the `vela` executable usually belong to different
Python environments. Locate both and compare them:

```console
python -c "import sys; print(sys.executable)"
python -m pip --version
python -m pip show ethos-u-vela
```

On Windows use `where vela`; on Linux or macOS use `command -v vela`. Activate
the intended virtual environment, remove the stale executable from `PATH`, or
upgrade through the Python interpreter associated with that executable. A clean
reinstallation in the selected environment is the final fallback:

```console
python -m pip install --upgrade --force-reinstall ethos-u-vela
vela --version
```

Use `py -m pip ...` instead of `python -m pip ...` on Windows when the Python
launcher is how the intended interpreter is selected. Avoid `pip` without
`python -m`: it is easier for that command to address a different environment.

PyPI supplies wheels for supported host combinations. If no matching wheel is
available, `pip` builds from source and requires Python development headers,
CMake, a C99 compiler, and a C++17 compiler. The package depends on FlatBuffers
and NumPy 1.23 or newer.

## Obtain an Ethos-U configuration for a device

The Vela compiler requires the selected Ethos-U variant and MAC configuration,
plus a `vela.ini` configuration file. The `vela.ini` file contains `System_Config`
sections that model device memory performance and `Memory_Mode` sections that define 
where model data is placed.

For an Edge AI MCU, a Device Family Pack (DFP), available from
[www.keil.arm.com/packs](https://www.keil.arm.com/packs), can provide the Ethos-U
configuration information, including the device-specific `vela.ini` file.
CMSIS-Toolbox exports these resources for the selected device and build context
through its
[MLOps information](https://open-cmsis-pack.github.io/cmsis-toolbox/build-overview/#mlops-information).

When the DFP does not provide this information, create the equivalent configuration
manually as described in <a href="#create-device-specific-velaini-file">Create device-specific <code>vela.ini</code> file</a>.

## Invocation

```console
vela [Options] NETWORK
```

- `Options` are described in the tables below and can precede or follow `NETWORK`. Use `vela --help` for the exact `Options` provided by the installed version.
- `NETWORK` is the path to the `.tflite` or `.tosa` ML model input file.

Only the input ML model file and accelerator configuration are needed for a basic
build:

```console
vela --accelerator-config ethos-u55-128 my_network.tflite
```

For meaningful scheduling and performance estimates, also select a platform
configuration, memory mode, and optimization strategy:

```console
vela my_network.tflite \
  --accelerator-config ethos-u55-128 \
  --config path/to/device-vela.ini \
  --system-config DEVICE_SYSTEM_CONFIG \
  --memory-mode DEVICE_MEMORY_MODE \
  --optimise Performance
```

### Options for input, output, and discovery

| Option | Description |
|---|---|
| `-h`, `--help` | Show command help and exit. |
| `--version` | Show the installed Vela version and exit. |
| `--api-version` | Show the deprecated external-API version. Planned for removal. |
| `--supported-ops-report` | Write `SUPPORTED_OPS.md` for TFLite operator constraints and exit. |
| `--list-config-files` | List packaged `vela.ini` configuration files and exit. |
| `--list-configs FILE` | List system configurations and memory modes in a `vela.ini` file and exit. |
| `--output-dir DIR` | Output directory; defaults to `output`. |
| `--output-format {tflite,raw}` | Select the output format; defaults to `tflite`. Raw output is an `.npz` archive and requires complete NPU placement. |
| `--enable-debug-db` | Write an ML model debug database into the output directory. |
| `--config FILE` | Read a ConfigParser-format `vela.ini` file. May be supplied more than once; later files can extend or override earlier definitions. |
| `--timing` | Report time spent in compiler stages. |
| `--force-symmetric-int-weights` | Force signed integer weight zero-points to zero. |

Raw output holds command streams, weight data, and tensor quantization metadata
for Ethos-U regions. It omits CPU regions and therefore is not a general
TFLite-to-TOSA conversion format.

### Options for Ethos-U target and scheduling

| Option | Description and values |
|---|---|
| `--accelerator-config TARGET` | Select Ethos-U hardware target: `ethos-u55-{32,64,128,256}`, `ethos-u65-{256,512}`, or `ethos-u85-{128,256,512,1024,2048}`. The suffix is the MACs-per-cycle configuration. |
| `--system-config NAME` | Select `[System_Config.NAME]` from the configuration files. The internal default provides functional defaults, but a platform-specific definition gives useful scheduling estimates. |
| `--memory-mode NAME` | Select `[Memory_Mode.NAME]`, which maps constants, arena, and cache to the system's memory areas. |
| `--tensor-allocator {LinearAlloc,Greedy,HillClimb}` | Choose the tensor allocator; defaults to `HillClimb`. |
| `--max-block-dependency {0,1,2,3}` | Deprecated. Set the maximum block-dependency delay between NPU kernel operations; defaults to `3`. A lower value can increase execution time. |
| `--optimise {Performance,Size}` | `Performance` is the default and minimizes inference time; `Size` minimizes peak SRAM and ignores the arena-cache size. |
| `--arena-cache-size BYTES` | Override the selected memory mode's cache capacity for `Performance` optimization. This is a byte count, not KiB. |
| `--cpu-tensor-alignment BYTES` | Alignment for CPU tensors, including custom-operator inputs and outputs; defaults to `16`. Keep it consistent with the ML inference runtime allocation. |
| `--recursion-limit COUNT` | Python recursion limit used during compilation; defaults to `1000`. |
| `--hillclimb-max-iterations COUNT` | Maximum HillClimb allocator iterations; defaults to `99999`. |
| `--cop-format {COP1,COP2}` | Select the custom-operator payload metadata format; defaults to `COP1`. |
| `--separate-io-regions` | Place custom-operator inputs and outputs into separate logical regions. Requires `--cop-format COP2`. |
| `--ignore-ops OP[,OP...]` | Force named TFLite builtin operator types, such as `ADD,ARGMAX`, onto the CPU. Repeatable and ignored for TOSA. |

### Options for reporting and diagnostics

| Option | Description |
|---|---|
| `--show-cpu-operations` | List TFLite operations that were not placed on the NPU. |
| `--show-subgraph-io-summary` | Summarize every subgraph and its inputs and outputs. |
| `--verbose-all` | Enable all verbose reports. The resulting output can be very large. |
| `--verbose-config` | Show resolved system and memory configuration. |
| `--verbose-graph` | Trace graph rewrites. |
| `--verbose-quantization` | Show quantization processing. |
| `--verbose-packing` | Show operation/pass packing. |
| `--verbose-performance` | Show detailed performance estimates. |
| `--verbose-tensor-purpose` | Show tensor-purpose assignment. |
| `--verbose-tensor-format` | Show tensor-format assignment. |
| `--verbose-schedule` | Show the selected schedule. |
| `--verbose-allocation` | Show tensor allocation. |
| `--verbose-high-level-command-stream` | Print the high-level command stream. |
| `--verbose-register-command-stream` | Print the register command stream. |
| `--verbose-operators` | List operators. |
| `--verbose-weights` | Show weight information. |
| `--verbose-cycle-estimate` | Show cycle-estimation details. |
| `--verbose-progress` | Show compilation progress. |

## Use the Ethos-U configuration

Compiling an ML model requires three selections from the Ethos-U configuration:

| Option | Describes |
|---|---|
| `--accelerator-config` | NPU architecture and MAC configuration. The generated command stream is Ethos-U target-specific. |
| `--system-config` | Core clock, AXI port mapping, memory clock ratios, burst lengths, latencies, and outstanding transactions used by the cost model. |
| `--memory-mode` | Placement of constant, arena, and cache memory areas on the AXI-connected memories. |

When a DFP supplies device-specific Ethos-U configuration information and the
CMSIS-Toolbox project includes
[MLOps information](https://open-cmsis-pack.github.io/cmsis-toolbox/build-overview/#mlops-information),
CMSIS-Toolbox places the resolved configuration file in the `.cmsis` directory.
List its available system configurations and memory modes with:

```console
vela --list-configs .cmsis/device-vela.ini  # Use device-specific vela.ini
```

If the project does not provide a device configuration, the Vela installation
includes the generic reference file `Arm/vela.ini` for evaluation,
silicon-vendor development, and manual integration. Its parameters do not
describe a specific production device. List the installed configuration files
and their definitions with:

```console
vela --list-config-files
vela --list-configs Arm/vela.ini
```

The generic `Arm/vela.ini` reference file includes these system configurations:

- Ethos-U55: `Ethos_U55_Deep_Embedded`,
  `Ethos_U55_High_End_Embedded`.
- Ethos-U65: `Ethos_U65_Embedded`, `Ethos_U65_Mid_End`,
  `Ethos_U65_High_End`, `Ethos_U65_Client_Server`.
- Ethos-U85: `Ethos_U85_SYS_Flash_Low`, `Ethos_U85_SYS_Flash_High`,
  `Ethos_U85_SYS_DRAM_Low`, `Ethos_U85_SYS_DRAM_Mid`,
  `Ethos_U85_SYS_DRAM_High`.

### Select a memory mode

The reference configuration defines typically memory modes as shown below.

| Memory Mode       | const<br/>_mem_area     | arena<br/>_mem_area     | cache<br/>_mem_area     | Content |
|-------------------|:-----------------------:|:-----------------------:|:-----------------------:|---------|
| `Sram_Only`       | `Axi0`                  | `Axi0`                  | `Axi0`                  | All model storage uses the memory type selected for `Axi0`. |
| `Shared_Sram`     | `Axi1`                  | `Axi0`                  | `Axi0`                  | Constants remain in the memory selected for `Axi1`; arena and cache share the `Axi0` memory. |
| `Dedicated_Sram`  | `Axi1`                  | `Axi1`                  | `Axi0`                  | The `Axi0` memory is a fast staging cache for an arena in the writable memory selected for `Axi1`. |

Note that there can be memory mode variants such as `Dedicated_Sram_256KB`, `_384KB`, `_512KB`, and
`_1024KB` that inherit `Dedicated_Sram` and set `arena_cache_size`.

The names `Axi0` and `Axi1` are logical aliases in `vela.ini`. Their physical
memory types come from the selected system configuration.

Ethos-U85 can have up to six AXI port interfaces, which Vela maps to `Axi0` and `Axi1`.
The `vela.ini` parameter `<Memory>_ports_used` contains the number of physical AXI
ports that map to a logical alias.

The `vela.ini` file, device interconnect, linker placement, MPU/SAU attributes,
cache policy, driver region indices, and ML inference runtime tensor arena must
agree. The Vela compiler cannot validate the complete firmware memory map.

### Understand arena cache and spilling

`cache_mem_area` does not always create a separate cache allocation. When it
resolves to the same memory type as `arena_mem_area`, the fast-scratch memory region is
folded into `arena_mem_area`. `cache_mem_area` is a distinct area only when
`arena_mem_area` is in a different memory type.

Ethos-U55 provides one read/write and one read-only AXI port and supports
`Sram_Only` and `Shared_Sram` modes. `Dedicated_Sram` places the writable arena
in the memory selected for `Axi1` while reserving the `Axi0` memory for fast
staging. Both paths must be writable, so the `Dedicated_Sram` mode is available
on Ethos-U65 and Ethos-U85.

## Create device-specific vela.ini file

To support a device manually, create a device-specific `vela.ini` file.
Application developers normally obtain this file from the silicon vendor,
either directly or from a DFP through CMSIS-Toolbox.

Creating an Ethos-U configuration requires these steps:

1. Select the Ethos-U variant and MAC configuration integrated into the device.
2. Identify the physical memories accessible to the NPU and obtain their clock,
   bandwidth, latency, burst, and outstanding-transaction characteristics.
3. Define a `System_Config` that models those memories through the logical
   `Axi0` and `Axi1` aliases.
4. Define one or more `Memory_Mode` sections that place constants, the writable
   arena, and optional fast staging storage on those aliases.
5. Create matching linker and driver configurations for the physical memory
   placement.
6. Validate the complete configuration with compiler reports, the linker map,
   and measurements on the device.

The `vela.ini`, linker, MPU/SAU, cache, and driver settings must describe the
same finished device integration.

`vela.ini` uses Python ConfigParser syntax and is case-sensitive for section
names, keys, and values. It contains two section types:

```ini
[System_Config.ConfigurationName]
; NPU clock, AXI mapping, and memory performance

[Memory_Mode.ModeName]
; compiler memory-area placement
```

`ConfigurationName` and `ModeName` must not contain spaces. A file can define
multiple sections of either type; select them with `--system-config` and
`--memory-mode`. Every property is optional, when omitted it defaults to `1`.
It is recommended to define every property that affects the target system.

Any section can contain `inherit=Part.Name` to inherit the values of another section.

- Inherited sections can appear in any order. Do not create recursive inheritance.
- `axi0_port` and `axi1_port` accept only the supported Vela memory types
  `Sram`, `Dram`, `OnChipFlash`, and `OffChipFlash`; custom memory type names are
  not supported.
- Start the name of a custom Ethos-U55 system configuration with
  `Ethos_U55`. The Vela compiler uses this prefix when selecting the U55 AXI
  bandwidth width while translating memory-performance values.

### System configuration parameters

The system configuration maps the two logical aliases in `vela.ini` to memory
types and supplies the performance model. `axi0_port` and `axi1_port` connect
the aliases used by a memory mode to those types; they do not by themselves
identify a physical memory or hardware port. Parameters for a memory type are
needed only when `axi0_port` or `axi1_port` selects that type.

Derive the values from the device data sheet, interconnect description, and
measured memory behavior:

- use the integrated Ethos-U clock for `core_clock`;
- map each logical alias to the memory type that most closely represents the
  physical memory connected through that access path;
- express memory clock or bandwidth scaling relative to the Ethos-U core clock;
- convert read and write latency to Ethos-U core cycles; and
- use supported burst lengths and outstanding-transaction limits from the
  device interconnect.

These inputs form a scheduling cost model, not a cycle-accurate hardware model.
Start with conservative values when the hardware documentation gives a range,
then compare compiler estimates and hardware measurements for representative ML
models.

| Parameter | Type or values | Description |
|---|---|---|
| `core_clock` | Float, Hz | Ethos-U core frequency. Scientific notation such as `500e6` is accepted. |
| `axi0_port` | `Sram`, `Dram`, `OnChipFlash`, `OffChipFlash` | Memory type assigned to the logical `Axi0` alias. |
| `axi1_port` | `Sram`, `Dram`, `OnChipFlash`, `OffChipFlash` | Memory type assigned to the logical `Axi1` alias. |
| `<Memory>_clock_scale` | Float, `0.0` to `1.0` | Memory clock/bandwidth scale relative to `core_clock`. `<Memory>` is `Sram`, `Dram`, `OnChipFlash`, or `OffChipFlash`. |
| `<Memory>_ports_used` | Integer | Number of ports used for memory (used for `Sram`, `Dram`, `OnChipFlash`) |
| `<Memory>_burst_length` | Integer, bytes | Minimum efficient transfer burst. |
| `<Memory>_read_latency` | Integer, cycles | Read latency used by the cost model. |
| `<Memory>_write_latency` | Integer, cycles | Write latency used by the cost model. Use `0` for a read-only memory when appropriate. |
| `<Memory>_max_reads` | Integer | Maximum outstanding reads. |
| `<Memory>_max_writes` | Integer | Maximum outstanding writes. Use `0` for a read-only memory when appropriate. |

The option reference lists the full metric set for `Sram`, `Dram`, and
`OffChipFlash`, and lists `OnChipFlash_clock_scale` for on-chip Flash.
Use `--verbose-config` with the installed Vela version to inspect the resolved
properties supported by that version.

The values in `vela.ini` are used by the Vela compiler to optimize the ML model for the target and the output report.
They can alter scheduling, buffering, DMA insertion, allocation sizes, and the generated
command stream. `core_clock` is primarily used to convert cycle estimates to time.

### Memory mode parameters

| Parameter | Type or values | Description |
|---|---|---|
| `const_mem_area` | `Axi0` or `Axi1` | Location for read-only constants, including weights, scales, biases, and constant tensors. |
| `arena_mem_area` | `Axi0` or `Axi1` | Location for read/write feature maps, intermediate tensors, and Vela-managed working storage. |
| `cache_mem_area` | `Axi0` or `Axi1` | Staging or fast-scratch location. It is separate from the arena only when it resolves to a different memory type from `arena_mem_area`. |
| `arena_cache_size` | Integer, bytes | Scheduler's fast-memory budget: the arena target when arena and cache resolve to the same memory type, or the separate staging-cache size when they differ. The CLI `--arena-cache-size` overrides it for `Performance` optimization. |
| `inherit` | `Part.Name` | Parent section whose parameters are inherited. Child values take precedence. |

The memory-area mapping must be consistent with the Ethos-U driver, the ML
inference runtime tensor arena, the linker script, and the physical memory
system. These options guide compilation; they do not configure the hardware.

With `--optimise Size`, the Vela compiler minimizes SRAM use and does not use
the arena-cache size. With `--optimise Performance`, it uses the configured or
command-line arena-cache size. If neither is supplied, the compiler uses the
maximum addressable size for the selected Ethos-U target.

### Create the linker script

The linker script places the generated model artifacts and runtime buffers in
the physical memories represented by the selected `System_Config` and
`Memory_Mode`. It does not use the logical names `Axi0` and `Axi1` directly.
Instead, it provides sections or memory regions for the Ethos-U I/O resources.

The linker section names are device- and runtime-specific. The examples provided with this pack use the following section names.

| Vela area | Linker section<br/>in examples | Content | Access |
|---|---|---|---|
| `const_mem_area` | `ethos_model` | Compiled model, command stream, encoded weights, scales, and other constants | Read-only |
| `arena_mem_area` | `ethos_arena` | Input, output, intermediate activations, and scratch storage used by the ML inference runtime | Read-write |
| `cache_mem_area` | `ethos_cache` | Optional scratch-fast or staging buffer when it resolves to a different physical memory from `arena_mem_area` | Read-write |

Different section names can be used, the important
property is their physical placement. If `arena_mem_area` and `cache_mem_area`
resolve to the same memory type, a separate cache section is not required. If
they resolve to different memory types, reserve the configured
`arena_cache_size` in the fast memory used for `cache_mem_area`.

The linker script, MPU/SAU attributes, and CPU cache policy must make every
generated region accessible to both the ML inference runtime and the NPU as
required. When Cortex-M software and the NPU use different addresses for the
same memory, the driver must provide the corresponding address translation.
The driver's command-stream and region attributes must also select access paths
that match the linker placement.

Validate a new linker configuration by checking:

- the link map contains the compiled model and all runtime buffers in the
  intended physical memories;
- the allocated arena and optional scratch-fast buffer are at least as large as
  the compiler reports;
- all used command-stream base regions are accessible to the NPU;
- MPU/SAU and cache attributes match the driver cache-maintenance policy; and
- the optimized model runs correctly and meets its memory and performance goals
  on the device.

### Match the driver configuration

The driver configuration must select NPU access paths that match the physical
placement established by `vela.ini` and the linker script. For the common
embedded output flow, verify these relationships:

| Generated content | Typical compiler memory area | Driver setting |
|---|---|---|
| Command stream | Placed with compiled model constants | `NPU_QCONFIG` |
| Base region 0: constants and encoded weights | `const_mem_area` | `NPU_REGIONCFG_0` |
| Base region 1: activations and scratch | `arena_mem_area` | `NPU_REGIONCFG_1` |
| Base region 2: optional scratch-fast storage | `cache_mem_area` | `NPU_REGIONCFG_2` |

The numeric values of these driver settings are target-specific. Additional
base regions can be present in specialized output flows, so use the generated
command stream and the device integration information as the authority. Also
configure address translation and cache maintenance when Cortex-M software and
the NPU access the same physical memory through different addresses or cache
policies.

See <a href="../integration/index.html">Integration</a> for system validation, platform
hooks, target-specific driver build configuration, cache maintenance, and
address-remapping guidance.

### Complete `My_vela.ini` example

```ini
; Ethos-U55 at 500 MHz with SRAM on AXI0 and read-only Flash on AXI1
[System_Config.My_Ethos_U55_System]
core_clock=500e6
axi0_port=Sram
axi1_port=OffChipFlash

Sram_clock_scale=1.0
Sram_ports_used=1
Sram_burst_length=32
Sram_read_latency=32
Sram_write_latency=32
Sram_max_reads=4
Sram_max_writes=4

OffChipFlash_clock_scale=0.125
OffChipFlash_ports_used=1
OffChipFlash_burst_length=128
OffChipFlash_read_latency=64
OffChipFlash_write_latency=0
OffChipFlash_max_reads=2
OffChipFlash_max_writes=0

; Constants remain in Flash; the tensor arena occupies shared SRAM
[Memory_Mode.My_Shared_Sram]
const_mem_area=Axi1
arena_mem_area=Axi0
cache_mem_area=Axi0
arena_cache_size=262144

; Derived mode with a smaller 128 KiB arena budget
[Memory_Mode.My_Shared_Sram_128KB]
inherit=Memory_Mode.My_Shared_Sram
arena_cache_size=131072
```

Invoke it with:

```console
vela model.tflite --config My_vela.ini \
  --accelerator-config ethos-u55-128 \
  --system-config My_Ethos_U55_System \
  --memory-mode My_Shared_Sram
```

Print the effective values while validating a new configuration:

```console
vela model.tflite --config My_vela.ini \
  --system-config My_Ethos_U55_System \
  --memory-mode My_Shared_Sram \
  --verbose-config
```

### Publish Ethos-U configuration in a DFP

A CMSIS Device Family Pack (DFP) can publish the NPU capabilities, `vela.ini` file, and matching linker scripts in its DFP description. The CMSIS-Toolbox can then select these resources for the device, processor, and toolchain used by a project.

Declare each integrated NPU with a device `feature`. The `n` attribute identifies
the Ethos-U variant, `m` identifies its MAC configuration, and `Pname` associates
it with a processor when the device contains more than one:

```xml
<feature type="NPU" n="Ethos-U55" m="256MACs" Pname="M55_P0"/>
<feature type="NPU" n="Ethos-U55" m="128MACs" Pname="M55_P1"/>
```

Publish the device-specific Vela configuration through the `VELA` environment:

```xml
<environment name="VELA">
  <file name="Device/scripts/vela/device_vela.ini" type="ini"/>
</environment>
```

Add linker scripts as component files. Conditions can select the script that
matches a processor and toolchain:

```xml
<file category="linkerScript" name="Device/linker/linker_ac6.sct"    attr="config" condition="AC6"/>
<file category="linkerScript" name="Device/linker/linker_gnu.ld.src" attr="config" condition="GCC"/>
```

Keep the NPU feature, Vela environment, `My_vela.ini`, linker scripts, and their
conditions consistent. A project that selects a different processor or toolchain
must resolve to the corresponding NPU configuration and linker script.

For the complete pack structure and element rules, see:

- [DFP Pack Hands-On](https://github.com/Open-CMSIS-Pack/DFP-Pack-HandsOn);
- [`feature` element](https://open-cmsis-pack.github.io/Open-CMSIS-Pack-Spec/main/html/pdsc_family_pg.html#element_feature);
- [`environment` element](https://open-cmsis-pack.github.io/Open-CMSIS-Pack-Spec/main/html/pdsc_family_pg.html#element_environment); and
- [component `file` element](https://open-cmsis-pack.github.io/Open-CMSIS-Pack-Spec/main/html/pdsc_components_pg.html#element_file).

## Read the Vela reports

After compilation, Vela prints a summary report and writes a CSV file in the
output directory. The report reflects the selected accelerator configuration,
system configuration, memory mode, and optimization strategy.

| Report area | Meaning |
|---|---|
| Configuration | Selected accelerator, system and memory configurations, NPU clock, and design peak memory bandwidth. |
| Memory use | Required storage in each Vela memory type: `Sram`, `Dram`, `OnChipFlash`, or `OffChipFlash`. The selected system configuration and memory mode determine these memory types. |
| Operator placement | Number and percentage of operators assigned to the CPU and NPU. |
| Memory traffic | Estimated average bandwidth and bytes transferred for feature maps and weights. |
| MACs | Number of multiply-accumulate operations per batch. |

The CSV file uses `weights_storage_area` for the Vela memory type selected by
`const_mem_area` and `feature_map_storage_area` for the type selected by
`arena_mem_area`. It reports memory use as a total for each Vela memory type. In
`Dedicated_Sram_256KB`, both data roles resolve to `Dram`, so
`dram_memory_used` contains their combined requirement.
`total_npu_encoded_weights` covers only encoded weights, not the complete
constant area, so it cannot be used to derive the size of `arena_mem_area`.

Use `--verbose-cycle-estimate` to add estimated NPU and memory-access cycles,
total cycles, inference time, and inferences per second. Use
`--verbose-performance` to generate a per-layer CSV with memory use, cycle
estimates, MAC utilization, and the contribution of each layer to the network.
The summary report is normally sufficient for configuration comparison; use the
per-layer report to investigate the reason for a difference.

> [!CAUTION]
> Vela cycle counts and inference times are estimates, not measured
> performance. They are useful for estimating expected performance and comparing
> candidates, but the selected configuration must be validated on the target
> hardware.

## Compare Ethos-U configurations

Before selecting a device, use the Ethos-U reference systems in the generic
`Arm/vela.ini` file for an initial comparison. They provide example
system and memory configurations for estimating performance and memory use, but
do not describe a specific production device. After selecting a device, repeat
the comparison with its device-specific `vela.ini` file.

Use the same ML model to test each accelerator, system configuration, and memory
mode.

> [!Tip]
> Keep the Vela version and other compiler options unchanged. Check that the same
> operations run on the NPU, and use a separate output directory for every build.

The following example comparison uses the
[`ad_large_int8.tflite` MicroNet Large INT8 anomaly-detection model](https://github.com/ARM-software/ML-zoo/blob/master/models/anomaly_detection/micronet_large/tflite_int8/ad_large_int8.tflite)
from the [Arm ML Zoo](https://github.com/ARM-software/ML-zoo) and Vela 5.1.0.
**Other memory** is `OffChipFlash` for the Ethos-U55 system and `Dram` for the
Ethos-U85 system.

To optimize for memory size, compile with `--optimise Size`:

| Accelerator | System configuration | Memory mode | SRAM | Other memory | Estimated time |
|---|---|---|---:|---:|---:|
| `ethos-u55-128` | `Ethos_U55_Deep_Embedded` | `Shared_Sram` | 136.00 KiB | 425.36 KiB | 27.816 ms |
| `ethos-u85-256` | `Ethos_U85_SYS_DRAM_High` | `Shared_Sram` | 136.00 KiB | 419.02 KiB | 1.158 ms |

To optimize for performance, compile with `--optimise Performance`:

| Accelerator | System configuration | Memory mode | SRAM | Other memory | Estimated time |
|---|---|---|---:|---:|---:|
| `ethos-u55-128` | `Ethos_U55_Deep_Embedded` | `Shared_Sram` | 390.61 KiB | 425.52 KiB | 5.806 ms |
| `ethos-u85-256` | `Ethos_U85_SYS_DRAM_High` | `Shared_Sram` | 436.77 KiB | 419.11 KiB | 0.458 ms |

## Examples

### Compile for an Ethos-U reference system

```console
vela person_detect.tflite \
  --accelerator-config ethos-u55-128 \
  --config Arm/vela.ini \
  --system-config Ethos_U55_High_End_Embedded \
  --memory-mode Shared_Sram \
  --optimise Performance \
  --verbose-cycle-estimate
```

The `output` directory contains the optimized ML model `person_detect_vela.tflite` and a report.
The optimized model is for deployment to the Edge AI MCU.

The console output shows several details including configuration parameters, memory usage, and with `--verbose-cycle-estimate` a rough estimation of the inference time.

### Minimize peak SRAM

```console
vela keyword_spotting.tflite \
  --accelerator-config ethos-u55-64 \
  --optimise Size
```

`Size` scheduling favors tensor reuse and cascading. It can reread more weights
and reduce throughput, so measure both memory and performance.

### Set a performance-scheduler SRAM budget

```console
vela person_detect.tflite \
  --accelerator-config ethos-u55-128 \
  --optimise Performance \
  --arena-cache-size 61440
```

This supplies a 60 KiB cache budget. Reducing it may increase AXI1 traffic as
weights or feature-map data are reread. The reported `Total SRAM used` covers
the model's NPU-visible tensors and working buffers, not the RTOS, stacks,
application allocations, CPU kernels, or other firmware state.

### Compile TOSA for Ethos-U85 raw output

```console
vela my_network.tosa \
  --config Arm/vela.ini \
  --accelerator-config ethos-u85-256 \
  --system-config Ethos_U85_SYS_DRAM_High \
  --memory-mode Dedicated_Sram_384KB \
  --output-format raw
```

The resulting `.npz` contains the NPU command streams, constants, and
quantization metadata required by an integration that consumes raw compiler
output.

### Inspect placement and compiler decisions

```console
vela model.tflite \
  --accelerator-config ethos-u85-512 \
  --show-cpu-operations \
  --show-subgraph-io-summary \
  --verbose-config \
  --verbose-performance \
  --enable-debug-db
```

Start with focused reports. `--verbose-all` can produce impractically large
logs for representative ML models.

### Embed an optimized TFLite model in firmware

On hosts that provide `xxd`, convert the Vela compiler output to a C array:

```console
xxd -i output/my_network_vela.tflite my_network_model.h
```

Place the generated data in the linker region that matches the memory
configuration selected from `vela.ini`. Ensure that the ML inference runtime
registers the Ethos-U custom operator and uses an Ethos-U driver compatible with
the compiled command stream.

## ExecuTorch Arm example flow

For the ExecuTorch Arm integration see the official PyTorch
[Arm Ethos-U backend documentation](https://docs.pytorch.org/executorch/stable/backends-arm-ethos-u.html)
for the workflow and examples.

## Troubleshooting

| Symptom | Check |
|---|---|
| Most operations run on the CPU | Confirm integer quantization, inspect `--show-cpu-operations`, and generate `--supported-ops-report` to check every operator constraint. |
| Compilation succeeds but estimates look unrealistic | Use a platform-specific `System_Config`; verify clocks, AXI mappings, latency, bandwidth, and memory mode. Compare only builds with the same configuration. |
| Runtime allocation fails | Check the Vela compiler's peak-memory report and add firmware overhead. Align linker regions and the tensor arena with `cpu_tensor_alignment` and the chosen memory mode. |
| Performance degrades after reducing SRAM | A smaller cache can cause more AXI1 reads. Inspect the performance CSV/report and hardware PMU counters. |
| A wheel is unavailable | Install Python development headers, CMake, and C99/C++17 build tools, or use a supported host/Python combination. |
| Output does not run on the target | Recompile for the exact Ethos-U architecture/MAC configuration and keep `vela.ini`, driver, linker, and memory-region configuration consistent. |
