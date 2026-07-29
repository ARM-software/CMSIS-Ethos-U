# Drivers {#mainpage}

This chapter is the technical reference for the low-level Ethos-U driver. It
explains the NPU execution contract and links to the API generated directly from
`ethosu_driver.h`.

## Driver responsibilities

The driver:

- Initializes one driver instance for each NPU;
- Programs the command-stream and model-region base addresses;
- Starts the NPU and handles completion or fault interrupts;
- Supports synchronous and asynchronous invocation;
- Provides access to the Ethos-U Performance Monitoring Unit (PMU);
- Exposes weak platform hooks for power, cache, address, and RTOS integration.

It does not compile models, allocate the ML framework's tensor arena, choose a
memory mode in `vela.ini`, or place sections in physical memory.

## Components

Ethos-U CMSIS pack provides components that should be added to the CMSIS project
according to the target NPU.

The main component is `ARM::Machine Learning:NPU Support:Ethos-U Driver` and is
available in three variants:

- Generic U55
- Generic U65
- Generic U85

Project adds main component and one of the listed variants. For example, add:

```yaml
components:
  - component: "ARM::Machine Learning:NPU Support:Ethos-U Driver&Generic U55" # For Ethos-U55
```

```yaml
components:
  - component: "ARM::Machine Learning:NPU Support:Ethos-U Driver&Generic U65" # For Ethos-U65
```

```yaml
components:
  - component: "ARM::Machine Learning:NPU Support:Ethos-U Driver&Generic U85" # For Ethos-U85
```

Add exactly one driver variant. CMSIS-Toolbox resolves the component from
the `ARM::CMSIS-Ethos-U` pack.

## Configuration {#driver_configuration}

The selected driver component supplies one target-specific configuration header:

| Driver variant | Configuration header  |
| -------------- | --------------------- |
| Generic U55    | `ethosu_config_u55.h` |
| Generic U65    | `ethosu_config_u65.h` |
| Generic U85    | `ethosu_config_u85.h` |

Configuration header gets copied from the pack locally to the project where one may
modify the configuration defines. Each configuration define provides annotations
for CMSIS Configuration Wizard and is guarded by `#ifndef`, so it may be changed
on solution level as well.

The headers configure how the driver programs the NPU.

### Configuration Options

#### Command-stream and base-pointer routing

All three targets use `NPU_QCONFIG` and `NPU_REGIONCFG_0` to `NPU_REGIONCFG_7`,
but the selected value has target-specific meaning:

| Option               | Configures Access     |
| -------------------- | --------------------- |
| `NPU_QCONFIG`        | command-stream        |
| `NPU_REGIONCFG_0..7` | constants/arena/cache |

On Ethos-U55 and Ethos-U65, the value selects an AXI port, outstanding
transaction counter, and AXI limit entry:

| Value | AXI path        | Limit entry  |
| ----- | --------------- | ------------ |
| `0`   | AXI0, counter 0 | `AXI_LIMIT0` |
| `1`   | AXI0, counter 1 | `AXI_LIMIT1` |
| `2`   | AXI1, counter 2 | `AXI_LIMIT2` |
| `3`   | AXI1, counter 3 | `AXI_LIMIT3` |

On Ethos-U85, the value selects `MEM_ATTR0..3` which then specifies the AXI port,
memory domain, and memory type.

### Ethos-U55 and Ethos-U65 AXI limits

Ethos-U55 and Ethos-U65 provide four independently configured limit entries:

- `AXI_LIMIT0` and `AXI_LIMIT1` apply to the two AXI0 counters;
- `AXI_LIMIT2` and `AXI_LIMIT3` apply to the two AXI1 counters.

Each entry has the following options:

| Option                              | Purpose                                       |
| ----------------------------------- | --------------------------------------------- |
| `AXI_LIMITx_MAX_BEATS_BYTES`        | boundary at which the NPU splits an AXI burst |
| `AXI_LIMITx_MEM_TYPE`               | AXI read and write cache attributes           |
| `AXI_LIMITx_MAX_OUTSTANDING_READS`  | maximum outstanding read transactions         |
| `AXI_LIMITx_MAX_OUTSTANDING_WRITES` | maximum outstanding write transactions        |

`AXI_LIMITx_MAX_BEATS_BYTES` limits the span of each burst to meet the boundary
requirements of the AXI interconnect and memory system.

It can have the following values:

| Value  | Burst Limit |
| ------ | ----------- |
| 0      | 64 bytes    |
| 1 or 2 | 128 bytes   |

`AXI_LIMITx_MEM_TYPE` encoding is common to U55, U65, and the U85 `MEM_ATTR` entries:

| Value | Memory type                            |
| ----- | -------------------------------------- |
| `0x0` | Device non-bufferable                  |
| `0x1` | Device bufferable                      |
| `0x2` | Normal non-cacheable, non-bufferable   |
| `0x3` | Normal non-cacheable, bufferable       |
| `0x4` | Write-through, no allocate             |
| `0x5` | Write-through, read allocate           |
| `0x6` | Write-through, write allocate          |
| `0x7` | Write-through, read and write allocate |
| `0x8` | Write-back, no allocate                |
| `0x9` | Write-back, read allocate              |
| `0xA` | Write-back, write allocate             |
| `0xB` | Write-back, read and write allocate    |

The outstanding-transaction options configure the maximum number of outstanding
AXI transactions,

| Option                              | U55 range | U65 range |
| ----------------------------------- | --------- | --------- |
| `AXI_LIMITx_MAX_OUTSTANDING_READS`  | `1..32`   | `1..64`   |
| `AXI_LIMITx_MAX_OUTSTANDING_WRITES` | `1..16`   | `1..32`   |

### Ethos-U85 power ramping

`NPU_MAC_PWR_RAMP_CYCLES` sets the interval between MAC-unit steps during power
ramp-up and ramp-down:

| Value   | Interval                                       |
| ------- | ---------------------------------------------- |
| `0..63` | `4 * NPU_MAC_PWR_RAMP_CYCLES` NPU clock cycles |

Value `0` disabled ramping.

### Ethos-U85 memory attributes

Ethos-U85 provides four memory-attribute entries configured by `NPU_MEM_ATTR_0..3`.
Each option is an encoded byte:

| Bits    | Field         | Values                                                                      |
| ------- | ------------- | --------------------------------------------------------------------------- |
| `[1:0]` | memory domain | `0`: non-shareable; `1`: inner shareable; `2`: outer shareable; `3`: system |
| `[2]`   | AXI port      | `0`: SRAM; `1`: EXT                                                         |
| `[3]`   | reserved      | keep clear                                                                  |
| `[7:4]` | memory type   | `0x0` to `0xB` as listed in the memory-type table above                     |

`NPU_QCONFIG` and each `NPU_REGIONCFG_x` select one of these entries. Therefore,
changing a `NPU_MEM_ATTR_x` option changes every command-stream or base-pointer
access routed to that entry.

### Ethos-U85 AXI limits

Ethos-U85 applies separate settings to the SRAM and EXT AXI ports:

| Option                                    | Values  | Purpose                                  |
| ----------------------------------------- | ------: | ---------------------------------------- |
| `AXI_LIMIT_SRAM_MAX_OUTSTANDING_READ_M1`  | `1..12` | maximum outstanding reads per SRAM port  |
| `AXI_LIMIT_SRAM_MAX_OUTSTANDING_WRITE_M1` | `1..16` | maximum outstanding writes per SRAM port |
| `AXI_LIMIT_SRAM_MAX_BEATS`                | `0..2`  | SRAM burst-split alignment               |
| `AXI_LIMIT_EXT_MAX_OUTSTANDING_READ_M1`   | `1..64` | maximum outstanding reads per EXT port   |
| `AXI_LIMIT_EXT_MAX_OUTSTANDING_WRITE_M1`  | `1..32` | maximum outstanding writes per EXT port  |
| `AXI_LIMIT_EXT_MAX_BEATS`                 | `0..2`  | EXT burst-split alignment                |

Either of `*_MAX_BEATS` defines can have the following values:

| Value  | Burst Limit |
| ------ | ----------- |
| 0      | 64 bytes    |
| 1      | 128 bytes   |
| 2      | 256 bytes   |

meaning that an AXI burst that crosses the selected aligned boundary is split into multiple bursts.

## API entry points

| Task | API |
| --- | --- |
| initialize or remove an NPU instance | \ref ethosu_init "ethosu_init()", \ref ethosu_deinit "ethosu_deinit()" |
| invoke and wait synchronously | \ref ethosu_invoke_v3 "ethosu_invoke_v3()" |
| submit asynchronously and poll or block | \ref ethosu_invoke_async "ethosu_invoke_async()", \ref ethosu_wait "ethosu_wait()" |
| handle the target interrupt | \ref ethosu_irq_handler "ethosu_irq_handler()" |
| inspect driver and hardware versions | \ref ethosu_get_driver_version "ethosu_get_driver_version()", \ref ethosu_get_hw_info "ethosu_get_hw_info()" |
| recover the NPU from an error | \ref ethosu_soft_reset "ethosu_soft_reset()" |
| manage power lifetime | \ref ethosu_request_power "ethosu_request_power()", \ref ethosu_release_power "ethosu_release_power()" |
| reserve an instance in a multi-NPU system | \ref ethosu_reserve_driver "ethosu_reserve_driver()", \ref ethosu_release_driver "ethosu_release_driver()" |

See \ref ethosu_public_api "Driver functions" for the complete generated API
and \ref ethosu_driver_structs "Driver structures" for public data types.

## Platform hooks

The default weak implementations are suitable only when their assumptions match
the target. Review every category during a port:

| Concern | Hooks | When an override is normally needed |
| --- | --- | --- |
| CPU data cache | \ref ethosu_flush_dcache "ethosu_flush_dcache()", \ref ethosu_invalidate_dcache "ethosu_invalidate_dcache()" | CPU-cached memory is shared with the NPU |
| address windows | \ref ethosu_address_remap "ethosu_address_remap()" | CPU and NPU use different addresses for the same storage |
| region attributes | \ref ethosu_config_select "ethosu_config_select()" | attributes depend on address or run-time placement |
| RTOS locking | mutex and semaphore hooks in \ref ethosu_callback_api "Callbacks" | multiple tasks or NPUs can use the driver |
| inference instrumentation | \ref ethosu_inference_begin "ethosu_inference_begin()", \ref ethosu_inference_end "ethosu_inference_end()" | tracing, power, or application callbacks are required |

Cache policy, linker placement, and region configuration are system-level
decisions. Detailed guidance is in
[Driver weak hooks](../integration/index.html) and
[Driver build configuration](../integration/index.html).

## Performance Monitoring Unit

The driver exposes the Ethos-U PMU through `pmu_ethosu.h`. The API supports a
64-bit cycle counter and programmable event counters. Ethos-U55 and Ethos-U65
builds provide four event counters; Ethos-U85 builds provide eight. Use
`ETHOSU_PMU_Get_NumEventCounters()` when code must work with more than one
Ethos-U target.

The target-specific `enum ethosu_pmu_event_type` lists the supported events.
They include NPU and MAC activity or stalls, weight-decoder and activation-output
activity, memory transactions and stalls, latency ranges, and ECC events. The
available event names differ between Ethos-U55/U65 and Ethos-U85. Use the
symbolic enum values with `ETHOSU_PMU_Set_EVTYPER()`; do not program hardware
event numbers directly.

The main API groups are:

| Task | PMU API |
|---|---|
| Enable or disable the PMU | `ETHOSU_PMU_Enable()`, `ETHOSU_PMU_Disable()` |
| Select an event | `ETHOSU_PMU_Set_EVTYPER()`, `ETHOSU_PMU_Get_EVTYPER()` |
| Reset counters | `ETHOSU_PMU_CYCCNT_Reset()`, `ETHOSU_PMU_EVCNTR_ALL_Reset()` |
| Enable or disable counters | `ETHOSU_PMU_CNTR_Enable()`, `ETHOSU_PMU_CNTR_Disable()` |
| Read counters | `ETHOSU_PMU_Get_CCNTR()`, `ETHOSU_PMU_Get_EVCNTR()` |
| Handle overflow | `ETHOSU_PMU_Get_CNTR_OVS()`, `ETHOSU_PMU_Set_CNTR_OVS()`, `ETHOSU_PMU_Set_CNTR_IRQ_Enable()`, `ETHOSU_PMU_Set_CNTR_IRQ_Disable()` |

Counter masks use `ETHOSU_PMU_CNT1_Msk` and the other event-counter masks, plus
`ETHOSU_PMU_CCNT_Msk` for the cycle counter. Enabling the PMU requests NPU power;
disabling it releases that request, so always pair the two operations.

Override `ethosu_inference_begin()` to select and reset events immediately
before an inference, and override `ethosu_inference_end()` to read the counters
and disable the PMU afterward. These callbacks receive the same `user_arg` that
was passed to `ethosu_invoke_v3()` or `ethosu_invoke_async()`, which can identify
where the results should be stored. PMU measurements are hardware observations;
compare them with Vela compiler estimates, but do not treat the estimates as
cycle-accurate measurements.

## Bring-up checklist

- Confirm the NPU identity and MAC configuration match the Vela compiler target.
- Confirm the command stream and every used base region are NPU-accessible.
- Wire the interrupt to `ethosu_irq_handler()` and exercise a timeout path.
- Verify cache clean/invalidate behavior with caches enabled, not only disabled.
- Check address remapping for TCM or aliased memory windows.
- Start with one known-good, fully supported model before testing a large graph.
- Use PMU cycle, activity, stall, and memory events when validating performance
  or investigating a difference from compiler estimates.
- Capture driver fault information before resetting after an error.

Continue with the end-to-end [Integration](../integration/index.html) checklist
before treating driver bring-up as complete.
