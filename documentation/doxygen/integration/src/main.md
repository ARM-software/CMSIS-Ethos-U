# Integration {#mainpage}

This chapter explains how to integrate a pretrained, quantized ML model into an
Edge AI MCU based on Cortex-M and Ethos-U. Model training, quantization, and
functional accuracy validation are outside the scope of this integration flow.

## Starting point

The starting point is a pretrained, quantized ML model that meets the
application's functional requirements. Before selecting a specific Edge AI MCU,
compile the model with Vela for one or more Ethos-U reference systems as
described in
[Compile for an Ethos-U reference system](../vela/index.html#vela_compile_reference_system).

The Vela outputs estimates of NPU cycles, memory bandwidth, and
model memory requirements. Vela also identifies which operations are assigned to
the NPU and which remain on the CPU. Use these results to compare Ethos-U
configurations and memory modes and to identify Edge AI MCUs with suitable NPU
performance and memory capacity. The Vela estimates support device selection; they
do not replace a device-specific compile or measurements on the final target.

Some model zoos already provide corresponding performance and memory data for
Ethos-U-based systems. Confirm that the published configuration is relevant to
the candidate device before using those results.

### Determine the memory budget

Most embedded applications are resource constraint and therefore the memory budget is an important aspect. Use this three-step approach to estimate the total memory requirements of the application:

- **Establish the ML model floor.** Compile the ML model and with
  `--optimise Size` and record the reported memory
  areas. See [Vela memory mode parameters](../vela/index.html#vela_memory_mode).
- **Build the system budget.** Add runtime and application data, stacks, heaps,
  alignment, padding, and a safety margin.
- **Tune and optimize.** Use the remaining memory budget for performance gains. Use different Vela system configurations and memory modes combined with `--arena-cache-size` as described in [Understand arena cache and spilling](../vela/index.html#vela_arena_cache_size).

## Integration workflow

Complete the following steps in order because later steps depend on earlier
decisions and measurements.

1. **Select the Edge AI MCU and DFP.** Compare the reference results with the
   device's NPU configuration and memory capacity and with the application's
   performance requirements. Obtain the matching Device Family Pack (DFP) from
   [www.keil.arm.com/packs](https://www.keil.arm.com/packs).
2. **Check the DFP resources.** Determine whether the DFP provides a
   device-specific `vela.ini` file, matching linker scripts, and other required
   resources. If it does not, contact the device or SoC vendor or
   [create a device-specific `vela.ini` file](../vela/index.html#vela_create_configuration).
3. **[Create the CMSIS-Toolbox project](#integration_create_csolution).** Select the device and build context, and specify the Vela system configuration and memory mode.
   Use the generated [MLOps information](https://open-cmsis-pack.github.io/cmsis-toolbox/build-overview/#mlops-information)
   to obtain the Vela parameters and resources supplied by the DFP.
4. **[Compile the ML model for the device](#integration_compile_model).** Run Vela with the device-specific
   parameters and confirm that its performance and memory estimates meet the
   application requirements. Treat the performance figures as model-based
   estimates and retain sufficient margin.
5. **[Configure memory placement and the linker script](#integration_configure_memory).** Keep the Vela memory
   mode, linker placement, and driver region configuration consistent. Account
   for the ML inference runtime, stacks, heaps, application data, alignment, and
   a safety margin. Build the system and review the compiler and linker reports.
6. **[Complete application integration](#integration_complete_application).** Add any application-specific RTOS,
   power, timeout, cache, and fault handling.
7. **[Validate and tune](#integration_validate_tune).** Verify correctness, memory allocation, ML model performance,
   bandwidth, latency, and concurrency on the actual target system.

A change to a memory mode, linker section, cache attribute, or driver region
value requires a review of the other descriptions of that memory region.

### General integration guidance

- Keep the `vela.ini`, `System_Config`, selected `Memory_Mode`, linker sections,
  MPU/SAU attributes, and driver build definitions consistent.
- Apply `arena_cache_size` according to the selected memory mode as described in
  [Understand arena cache and spilling](../vela/index.html#vela_arena_cache_size).
- Place model constants, activations, and any separate scratch-fast storage as
  described in
  [Create the linker script](../vela/index.html#vela_create_linker_script).
- Override the driver weak hooks when the default integration assumptions do not
  match the platform, especially for data cache maintenance, address remapping,
  and RTOS synchronization.
- Ensure the NPU completion interrupt is eventually serviced. Very low jitter is
  usually not required for inference workloads, but completion handling must not
  be postponed indefinitely.
- During bring-up, use timeouts, fault reporting, and a minimal known-good model
  before moving to full application graphs.

### Add ML model and configuration to version control

It is good practice to document the handoff boundary between model, platform, and application
engineers. This also gives automated tools enough context to check consistency.
CMSIS-Toolbox already records the selected packs and `vela.ini` configuration
file in the *csolution project* files and in the metafiles `*.cbuild-pack.yml` and `*.cbuild-mlops.yml`.
Keep these files along with the input ML model under version control.

## Create the *csolution project* {#integration_create_csolution}

CMSIS-Toolbox simplifies MLOps by combining device and DFP data with project
settings into machine-readable
[MLOps information](https://open-cmsis-pack.github.io/cmsis-toolbox/build-overview/#mlops-information)
that tools can use to generate the ML model and test it on hardware or a
simulator.

### Use a project example and add device

This pack includes [examples](https://mdk-packs.github.io/vscode-cmsis-solution-docs/create_app.html) for FVP simulation models that show the Ethos-U NPU integration in a Cortex-M target. These projects are [reference applications](https://open-cmsis-pack.github.io/cmsis-toolbox/ReferenceApplications) that can be deployed to other boards that provide a board layer with an [STDOUT connection](https://open-cmsis-pack.github.io/cmsis-toolbox/ReferenceApplications/#connections).

Target Board (FVP Simulator)   | NPU       | Example project
:------------------------------|:----------|:---------------------------
V2M-MPS3-SSE-300-FVP           | Ethos-U55 | `Hello-Ethos-U55.csolution.yml`
V2M-MPS3-SSE-300-FVP           | Ethos-U65 | `Hello-Ethos-U65.csolution.yml`
SSE-320                        | Ethos-U85 | `Hello-Ethos-U85.csolution.yml`

A hardware target can be added in the `*.csolution.yml` file as shown below:

```yaml
  packs:
    - pack: AlifSemiconductor::Ensemble      # Add DFP and optional BSP

  target-types:
    - type: MyHardware                       # Add hardware target
      device: AE722F80F55D5LS
      board: AppKit-E7-AIML
// todo
    - type: SSE-300-U55
```

### Add MLOps information

The `mlops:` node in the `*.csolution.yml` file configures [MLOps Management](https://open-cmsis-pack.github.io/cmsis-toolbox/YML-Input-Format/#mlops-management).
CMSIS-Toolbox combines this information with the selected device and DFP and generates the [`*.cbuild-mlops.yml`](https://open-cmsis-pack.github.io/cmsis-toolbox/YML-CBuild-Format/#cbuild-mlopsyml) file for the MLOps workflow.

The following example selects an Ethos-U55 configuration and identifies the software layer that contains the ML model:

```yaml
solution:
  mlops:
    description: Person detection model
    npu:
      type: Ethos-U55
      macs: 128
    vela:
      system: Ethos_U55_High_End_Embedded
      memory: Shared_Sram
    model:
      clayer: $ML-Layer$      # this can also be an absolute path
```

### Add ML model layer

Replace the NPU and Vela selectors with values supported by the selected device
and set `model.clayer` to the layer that contains the model. The example omits
`vela.ini`, so CMSIS-Toolbox uses the configuration supplied by the device or
DFP.

## Compile the ML model for the device {#integration_compile_model}

CMSIS-Toolbox combines DFP information with the csolution project configuration
and generates the MLOps information file `*.cbuild-mlops.yml`. The `vela:` node provides the `ini:` configuration file and `options:` that can be used to invoke Vela.

```console
vela --config <vela.ini> <vela.options> ml-model.tflite
```

## Configure memory placement and the linker script {#integration_configure_memory}

The common relationship between compiler memory areas and driver regions is
described in
[Match the driver configuration](../vela/index.html#vela_match_driver_configuration).
The generated command stream uses numeric NPU regions, also
called base pointer indices; it does not use the `Axi0` or `Axi1` aliases.

Generated commands such as `NPU_SET_IFM_REGION`, `NPU_SET_OFM_REGION`,
`NPU_SET_WEIGHT_REGION`, `NPU_SET_SCALE_REGION`, `NPU_SET_DMA0_SRC_REGION`, and
`NPU_SET_DMA0_DST_REGION` carry region numbers and offsets. The runtime and
driver supply the base addresses and memory attributes for those regions.

The following driver sections explain the platform hooks and region attributes
that must agree with this placement.

Create the physical sections for model constants, the tensor arena, and any
separate scratch-fast storage as described in
[Create the linker script](../vela/index.html#vela_create_linker_script). Use the
linker map to verify that their addresses and sizes match the selected memory
mode and that every generated NPU region is accessible through the driver
configuration.

## Complete application integration {#integration_complete_application}

Validate interrupt wiring alongside Vela, linker, MPU/SAU, cache, and driver settings.


## Validate and tune {#integration_validate_tune}

----

## Ethos-U configuration

The Vela guide explains how to select an existing configuration and how to
create one for a device:

- See [use the Ethos-U configuration](../vela/index.html#vela_use_configuration)
  to inspect the resolved `System_Config` and `Memory_Mode`; and
- See [create device-specific `vela.ini` file](../vela/index.html#vela_create_configuration)
  for `vela.ini` syntax, memory modes, performance parameters, arena-cache
  behavior, and spilling. The constraints for parsing `vela.ini` can differ
  between Ethos-U cores, so follow the requirements for the selected core.

This integration guide assumes that the accelerator configuration,
`System_Config`, and `Memory_Mode` have already been selected. The remaining
task is to keep that configuration consistent with the generated command-stream
regions, physical memory placement, driver settings, and application memory
budget described below.

## Linker configuration

The following driver sections explain the platform hooks and region attributes
that must agree with this placement.

## Driver weak hooks

The driver provides weak functions that a platform can override when the default
implementation is not sufficient. These hooks are part of the integration layer:
they connect the generic driver to the cache, address map, and synchronization
policy of the target system.

For data cache maintenance, the driver exposes \ref ethosu_flush_dcache
"ethosu_flush_dcache()" and \ref ethosu_invalidate_dcache
"ethosu_invalidate_dcache()". The default implementations are no-ops. Override
them when the Cortex-M data cache is enabled and the NPU accesses memory that may
also be cached by the CPU. Clean and invalidate the cache as appropriate.

- \ref ethosu_flush_dcache "ethosu_flush_dcache()" prepares CPU-written data for
  NPU access. It is used before inference so the NPU sees the command stream,
  constants, and input or activation data that may still be dirty in the CPU
  cache.
- \ref ethosu_invalidate_dcache "ethosu_invalidate_dcache()" prepares
  NPU-written data for CPU access. It is used after inference so the CPU does not
  read stale cache lines for output or activation buffers updated by the NPU.
  Clean and invalidate the cache as appropriate.

Cache management policy is platform-specific, but these rules are useful during
integration:

- Invalidating a cache range without first cleaning dirty lines can lose data
  that has only been written to the CPU cache. Use invalidate-only operations
  only when the platform can guarantee that the range has no dirty CPU-owned
  data.
- Cleaning by address can be slower for large model buffers, but cleaning the
  full data cache can disturb unrelated software by evicting or writing back data
  outside the inference buffers.
- If a cache hook is asked to operate on a non-cacheable region, such as TCM or a
  memory region configured as device/non-cacheable, it should do nothing for that
  range except apply any required ordering barrier.

The driver also exposes \ref ethosu_address_remap "ethosu_address_remap()" and
\ref ethosu_config_select "ethosu_config_select()" for address and
region-configuration handling.

- \ref ethosu_address_remap "ethosu_address_remap()" converts the address used
  by Cortex-M software into the address that must be programmed into the NPU when
  both agents see the same memory through different address windows. This is
  common when linker sections are placed in TCM for the CPU while the NPU reaches
  that storage through an AXI alias.
- \ref ethosu_config_select "ethosu_config_select()" selects the NPU memory
  region attributes for a command stream or base pointer. The default build-time
  definitions are enough for many systems; override this hook when the region
  selection depends on the address or memory placement.

For bare-metal systems, the default synchronization hooks may be sufficient if
only one execution context submits work to the driver. For RTOS-based systems or
for integrations where multiple contexts may use the driver, override the mutex
and semaphore hooks to use the operating system primitives:
\ref ethosu_mutex_create "ethosu_mutex_create()",
\ref ethosu_mutex_destroy "ethosu_mutex_destroy()",
\ref ethosu_mutex_lock "ethosu_mutex_lock()",
\ref ethosu_mutex_unlock "ethosu_mutex_unlock()",
\ref ethosu_semaphore_create "ethosu_semaphore_create()",
\ref ethosu_semaphore_destroy "ethosu_semaphore_destroy()",
\ref ethosu_semaphore_take "ethosu_semaphore_take()", and
\ref ethosu_semaphore_give "ethosu_semaphore_give()".

These hooks are separate from the memory placement model, but they are commonly
implemented in the same platform integration layer as cache maintenance and
address remapping. The Driver API documentation provides the detailed function
prototypes and calling contract.

## Driver build configuration

The current Ethos-U driver build selects one NPU memory configuration at build
time. A single binary cannot switch between different memory-area mappings unless
the project builds separate variants or adds a platform-specific selection
mechanism.

With CMSIS-Toolbox, select the required mode by providing the driver compile
definitions in the relevant build context, layer, or target configuration. The
values used for `NPU_QCONFIG` and `NPU_REGIONCFG_x` tell the driver which NPU
access path to use for the command stream and for each base pointer region:

| Driver define | Used for | Compiler memory area in the examples below |
| --- | --- | --- |
| `NPU_QCONFIG` | command stream | usually placed with model constants |
| `NPU_REGIONCFG_0` | base pointer region 0 | `const_mem_area` |
| `NPU_REGIONCFG_1` | base pointer region 1 | `arena_mem_area` |
| `NPU_REGIONCFG_2` | base pointer region 2 | `cache_mem_area`, when the generated model uses scratch-fast storage |

Additional `NPU_REGIONCFG_3` to `NPU_REGIONCFG_7` definitions exist for command
streams used by other platform configurations. For example, the experimental
Ethos-U85 Direct Drive flow on Linux uses region 3 for input tensors and region
4 for output tensors. Cortex-M integrations use regions 0 to 2 and do not need
to configure these additional regions.

For the default driver configuration, the practical mapping is:

| Value | Ethos-U55/U65 meaning | Ethos-U85 meaning with the default `NPU_MEM_ATTR_0` to `NPU_MEM_ATTR_3` |
| --- | --- | --- |
| `0` or `1` | use AXI0 | use AXI_SRAM |
| `2` or `3` | use AXI1 | use AXI_EXT |

This lets the same region values give the same intended behavior across
Ethos-U55, Ethos-U65, and Ethos-U85: use `0` or `1` for base pointer regions
whose compiler storage role is resolved through the `Axi0` alias, and use `2` or
`3` for base pointer regions whose role is resolved through the `Axi1` alias.

The value does not name a physical memory by itself. It selects the NPU access
path and the attributes programmed for that path. The platform integration must
still ensure that the linker placement, MPU/SAU attributes, cache policy, and
driver AXI limit settings match the real SRAM, Flash, MRAM, DRAM, or TCM behind
the selected region configuration.

For example, a shared-SRAM memory mode may place the command stream and
constants in the memory selected by `Axi1`, and activations in the memory
selected by `Axi0`.

If the driver configuration maps `2` or `3` to the memory selected by `Axi1` and
`0` or `1` to the memory selected by `Axi0`, the corresponding build definitions
can be:

```yaml
defines:
  - NPU_QCONFIG: 2
  - NPU_REGIONCFG_0: 3
  - NPU_REGIONCFG_1: 0
  - NPU_REGIONCFG_2: 1
```

For a dedicated-SRAM memory mode, where supported, activations may move to the
memory selected by `Axi1` while the staging or cache region remains in the
memory selected by `Axi0`.

The build definitions then follow that placement:

```yaml
defines:
  - NPU_QCONFIG: 2
  - NPU_REGIONCFG_0: 3
  - NPU_REGIONCFG_1: 2
  - NPU_REGIONCFG_2: 1
```

For an SRAM-only memory mode, all generated regions can use the memory selected
by `Axi0`:

```yaml
defines:
  - NPU_QCONFIG: 0
  - NPU_REGIONCFG_0: 1
  - NPU_REGIONCFG_1: 0
  - NPU_REGIONCFG_2: 1
```

The distinction between `0` and `1`, or between `2` and `3`, is target- and
system-specific. On Ethos-U55 and Ethos-U65, the value is the REGIONCFG encoding
and selects one of the AXI limit entries: `0` and `1` use the AXI0-side limit
settings, while `2` and `3` use the AXI1-side limit settings. On Ethos-U85, the
value is a MEM_ATTR index; the driver defaults make MEM_ATTR0 and MEM_ATTR1 use
AXI_SRAM, and MEM_ATTR2 and MEM_ATTR3 use AXI_EXT. Change `NPU_MEM_ATTR_0` through
`NPU_MEM_ATTR_3` only when the platform needs different U85 memory attributes.

The AXI limit values are separate platform-tuning settings. Simplified,
`AXI_LIMIT0` and `AXI_LIMIT1` on Ethos-U55 and Ethos-U65 correspond to the
AXI0-side configuration, while `AXI_LIMIT2` and `AXI_LIMIT3` correspond to the
AXI1-side configuration. On Ethos-U85, the equivalent limit settings are grouped
under AXI_SRAM and AXI_EXT. The exact outstanding transaction and burst settings
depend on the SoC interconnect and memory system.

Keep these definitions synchronized with the `Memory_Mode` used to compile the
model with the Vela compiler and with the linker sections used by the
application. The driver
must ultimately program region attributes that match the memory used for the
command stream and each model base pointer. If the meaning of a numeric value is
not clear for the selected Ethos-U target, use the target integration guide,
hardware register descriptions, or a platform-provided configuration as the source
of truth before benchmarking or releasing the build.
