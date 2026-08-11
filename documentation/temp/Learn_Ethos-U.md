# Ethos-U Learning Path

Table of Content

- Ethos-U55 Architecture
- How Ethos-U Works
- Command stream - what is it?
- Ethos-U Configuration (and Registers)
- Vela Configuration
- How to determine memory requirements
- How to configure Ethos-U driver

## Ethos-U55 Architecture

- Ethos-U has two AXI manager/master interfaces used by its internal DMA to access memory
  - AXI0 / M0: read-write AXI manager
  - AXI1 / M1: read-only AXI manager

- Ethos-U AXI managers connect into the SoC memory system through interconnect

- Various options are possible:
  - AXI0 to SRAM, AXI1 to Flash        (SRAM for activations/scratch/output, Flash for read-only weights/constants)
  - AXI0 to SRAM, AXI1 unused/tied off (useful for SRAM-only system)
  - General: chip integrator decides where they go, they are just two NPU memory access paths.

- Typical U55-style system:

  ```text
  Ethos-U DMA
    |
    +-- AXI0 / M0, read-write --> SRAM / system RAM
    |
    +-- AXI1 / M1, read-only  --> Flash / MRAM / ROM / SRAM
  ```

### Three configuration layers

1. SoC hardware integration defines where the Ethos-U AXI ports are physically connected.
2. Vela configuration tells the Vela compiler what memory exists behind AXI manager.

    Relevant vela.ini fields:

    ```ini
    [System_Config.<name>]
    axi0_port=Sram
    axi1_port=OffChipFlash

    [Memory_Mode.<name>]
    const_mem_area=Axi1
    arena_mem_area=Axi0
    cache_mem_area=Axi0
    ```

3. Driver configuration tells NPU which AXI path to use for each command stream/base pointer.

    Relevant U55 options:

    ```txt
    NPU_QCONFIG       -> command stream AXI path
    NPU_REGIONCFG_0   -> BASEP0, often constants
    NPU_REGIONCFG_1   -> BASEP1, often arena/scratch
    NPU_REGIONCFG_2   -> BASEP2, often cache/scratch-fast if used
    NPU_REGIONCFG_3..7
    ```

To maximize bus performance, configure these consistently:

```txt
SoC integration:
- AXI0/AXI1 connected to memories with enough bandwidth
- AXI clocks high enough
- interconnect arbitration/QoS not starving the NPU
- Flash/MRAM/DRAM controllers tuned for burst reads

Vela:
- accurate bandwidth/latency values for axi0_port and axi1_port
- constants placed on read-only/high-latency path if appropriate
- arena/scratch/output placed on writable low-latency path
- memory mode matches real hardware

Driver:
- NPU_QCONFIG and NPU_REGIONCFG_x match Vela placement
- AXI_LIMIT read/write outstanding values tuned
- burst split alignment set for best target behavior
- AxCACHE/memory type matches cache/interconnect policy

Application:
- linker places sections in the intended memories
- MPU/cache attributes match AXI attributes
- cache clean/invalidate is correct when CPU and NPU share cached memory
```

Good placement is usually:

```txt
command stream -> AXI1 or AXI0, depending where stored
weights/constants -> AXI1
activations/scratch/output -> AXI0
```

## How Ethos-U Works

CPU gives Ethos-U prepared command stream, memory pointers and then Ethos-U
performs many small tensor operations by reading and writing memory.

Typical flow:

```txt
Input tensor(s)
   |
   v
Application / ML runtime
   |
   v
Ethos-U command stream + base addresses
   |
   v
Ethos-U NPU
   |
   v
Output tensor(s)
```

More concretely:

1. The model is compiled first
   - A TensorFlow Lite Micro model is passed through Arm Vela.
   - Vela decides which operators can run on Ethos-U.
   - Vela generates an optimized command stream for the NPU.
   - The model still contains metadata used by the runtime.

2. The application prepares memory
   - Input tensor data is placed in memory.
   - Output tensor memory is reserved.
   - Intermediate activation/scratch memory is reserved.
   - Weights, biases, scales, and command stream are available in memory.

3. The CPU starts the NPU
   - The CPU writes Ethos-U control registers.
   - It provides:
     - command stream address
     - command stream size
     - base pointer addresses for model memory regions
   - Then it tells the NPU to run.

4. Ethos-U reads the command stream
   - The NPU fetches instructions/commands from memory.
   - These commands describe operations such as convolution, depthwise convolution, pooling, elementwise ops, DMA moves, etc.

5. Ethos-U moves data through its internal pipeline
   - The DMA engine reads input feature maps, weights, biases, and scales.
   - Data is staged into internal buffers.
   - The MAC engine performs multiply-accumulate work.
   - The output unit applies things like quantization, activation clamping, and writes results.

6. Ethos-U writes results back to memory
   - Final output tensors are written into the output tensor buffer.
   - Intermediate tensors are written/read from scratch or arena memory as needed.
   - When the command stream ends, Ethos-U raises an interrupt or status flag.

## Command stream - what is it?

The command stream tells the NPU what to do and is produced by Vela, from the ML model:

```txt
Original model
   |
   v
Vela compiler
   |
   v
Optimized model containing:
  - model metadata
  - tensors and buffers
  - weights/constants
  - Ethos-U custom operator(s)
  - Ethos-U command stream
```

The command stream contains encoded NPU commands such as:

```txt
read this tensor region
read these weights
configure convolution parameters
run this operation
write result here
move/stage data
```

But it does not stand alone usefully without the rest of the model memory:

```txt
command stream -> instructions for the NPU
weights        -> constants used by the operations
arena/scratch  -> intermediate tensors
input tensor   -> user/application data
output tensor  -> result buffer
metadata       -> used by TFLite Micro/runtime
```

So:

```txt
Vela output = optimized model
Command stream = NPU instruction payload inside that optimized model
```

At runtime, the CPU/ML framework runtime finds the Ethos-U custom operator, extracts the command stream address and base addresses, then starts the NPU.

## Ethos-U Configuration (and Registers)

For normal inference execution, the most important registers are:

```txt
BASE.QBASE0
BASE.QBASE1
BASE.QSIZE
BASE.QCONFIG
BASEP0..BASEP15
BASE.REGIONCFG
BASE.CMD
BASE.STATUS
```

| Register      | Meaning                                 |
| ------------- | --------------------------------------- |
| `BASE.QBASE0` | Command stream base address (low bits)  |
| `BASE.QBASE1` | Command stream base address (high bits) |
| `BASE.QSIZE`  | Command stream size (bytes)             |

Ethos-U needs to know `how to access that memory`. That is what `BASE.QCONFIG` does:

| Register       | Meaning                                                 |
| -------------- | ------------------------------------------------------- |
| `BASE.QCONFIG` | Which AXI path/attributes to use to read command stream |

`BASE.QCONFIG` affects only command stream fetches. It does not configure where weights, activations, scratch buffers, or outputs are accessed. Those use `BASE.REGIONCFG` and the `BASEP0..BASEP15` region pointers:

| Register       | Meaning                                                 |
| -------------- | ------------------------------------------------------- |
| `BASEP0`       | Base pointer for region 0 (low bits)                    |
| `BASEP1`       | Base pointer for region 0 (high bits)                   |
| `BASEP2`       | Base pointer for region 1 (low bits)                    |
| `BASEP3`       | Base pointer for region 1 (high bits)                   |
|  ...           | ...                                                     |
| `BASEP14`      | Base pointer for region 7 (low bits)                    |
| `BASEP15`      | Base pointer for region 7 (high bits)                   |

There are 16 registers, but they represent 8 base pointer regions.

The command stream can then say things like:

```txt
read from region 0 + offset
write to region 1 + offset
read weights from region 0 + offset
use scratch at region 2 + offset
```

Common mapping in a model:

```txt
region 0 -> constants / weights / scales
region 1 -> arena / activations / scratch / output
region 2 -> cache or scratch-fast area, if used
```

Register `BASE.REGIONCFG` is related to `BASEn` registers. It tells Ethos-U which AXI path to use for each region and what AXI limit settings are used when accessing each region:

```txt
region 0 uses AXI path selected by REGIONCFG bits [1:0]
region 1 uses AXI path selected by REGIONCFG bits [3:2]
region 2 uses AXI path selected by REGIONCFG bits [5:4]
...
```

So:

```txt
BASEP registers = where each memory region starts
REGIONCFG       = how Ethos-U accesses each region
```

Example:

```txt
BASEP0/1 = address of weights/constants region
BASEP2/3 = address of arena/scratch region
REGIONCFG says region 0 uses AXI1, region 1 uses AXI0
```

| Register         | Meaning                                                   |
| ---------------- | --------------------------------------------------------- |
| `BASE.REGIONCFG` | Configures which AXI is used for each base pointer region |

`BASE.REGIONCFG` uses 2 bits per region:

```txt
region 0 -> bits [1:0]
region 1 -> bits [3:2]
region 2 -> bits [5:4]
region 3 -> bits [7:6]
region 4 -> bits [9:8]
region 5 -> bits [11:10]
region 6 -> bits [13:12]
region 7 -> bits [15:14]
```

Each 2-bit value means:

```txt
0 = AXI0, outstanding counter 0, use AXI_LIMIT0
1 = AXI0, outstanding counter 1, use AXI_LIMIT1
2 = AXI1, outstanding counter 2, use AXI_LIMIT2
3 = AXI1, outstanding counter 3, use AXI_LIMIT3
```

Example:

```txt
BASE.REGIONCFG region 0 = 3
BASE.REGIONCFG region 1 = 0
BASE.REGIONCFG region 2 = 1
```

Means:

```txt
region 0 uses AXI1 counter 3 -> AXI_LIMIT3
region 1 uses AXI0 counter 0 -> AXI_LIMIT0
region 2 uses AXI0 counter 1 -> AXI_LIMIT1
```

For a typical memory layout:

```txt
region 0 = constants / weights / scales
region 1 = arena / activations / scratch / output
region 2 = cache or scratch-fast area
```

So if constants are in Flash behind AXI1, and activations are in SRAM behind AXI0:

```txt
region 0 -> AXI1
region 1 -> AXI0
```

Important difference from `BASE.QCONFIG`:

```txt
BASE.QCONFIG   -> AXI path for command stream fetches
BASE.REGIONCFG -> AXI path for BASEP data regions
```

`BASE.AXI_LIMIT0..BASE.AXI_LIMIT3` registers define the bus behavior limits and memory attributes for Ethos-U AXI transactions. They define how aggressively Ethos-U may use the bus once a region or command stream selects one of the AXI paths.

Each AXI_LIMITx register contains fields like:

```txt
max_outstanding_read_m1
max_outstanding_write_m1
memtype
max_beats
```

Meaning:

| Field                      | Meaning                                                             |
| -------------------------- | ------------------------------------------------------------------- |
| `max_outstanding_read_m1`  | Maximum number of outstanding AXI read transactions minus 1         |
| `max_outstanding_write_m1` | Maximum number of outstanding AXI write transactions minus 1        |
| `memtype`                  | AXI cache/memory attribute encoding                                 |
| `max_beats`                | Burst split alignment setting                                       |

**Outstanding transactions**

An “outstanding” transaction is a bus request that Ethos-U has issued but that has not fully completed yet.
`AXI_LIMIT` registers controls how many bus transactions Ethos-U can have in flight at once.

For example `max_outstanding_read_m1 = 31` means maximum outstanding reads = 32.
Higher values can improve performance when memory latency is high,
because the NPU can keep more requests in flight. But too high can
overload the interconnect or hurt other bus masters.

Example:
```txt
Ethos-U sends AXI read request #1
Ethos-U sends AXI read request #2
Ethos-U sends AXI read request #3

memory has not returned all data yet

outstanding read count = 3
```

The `AXI_LIMIT` registers set the maximum number of such in-flight transactions allowed.

Why have multiple counters? So different traffic classes can have different limits even on the same AXI port. For example:

```txt
AXI0 counter 0 -> arena/scratch traffic
AXI0 counter 1 -> another region or command/data class
AXI1 counter 2 -> constants/weights
AXI1 counter 3 -> command stream or another read-only region
```

**Memory type**

`memtype` controls the AXI cache attributes:

```txt
Device
Normal non-cacheable
Write-through
Write-back
...
(see Ethos-U Reference Manual for all possible values)
```

This must match the system memory/cache policy. If the CPU cache, MPU, interconnect,
and Ethos-U AXI attributes disagree, you can get coherency bugs or poor performance.

**Max beats**

`max_beats` controls burst split alignment, for U55:

```txt
0 = 64 bytes
1 = 128 bytes
2 = 128 bytes
3 = reserved
```

So in practice it limits how transactions are split at burst boundaries.

A simple example:

```txt
REGIONCFG region 0 = 3
```

means region 0 uses:

```txt
AXI1 counter 3
BASE.AXI_LIMIT3 settings
```

If AXI_LIMIT3 has high outstanding reads and a cacheable/read-friendly memory type, then reads for region 0 can be more aggressive.

## Vela Configuration File

Vela configuration file consists of two sections:

- System_Config
- Memory_Mode

At least one entry is required is each section.

`Memory_Mode` tells Vela where different classes of model data should live. It only says:

```txt
constants go through which Vela AXI area?
arena/scratch goes through which Vela AXI area?
cache/staging goes through which Vela AXI area?
how much arena/cache memory may Vela use?
```

In Vela the section looks like this:

```ini
[Memory_Mode.My_Mode]
const_mem_area=Axi1
arena_mem_area=Axi0
cache_mem_area=Axi0
arena_cache_size=262144
```

The four main options are:

| Option             | Meaning |
| ------------------ | --- |
| `const_mem_area`   | Where Vela places read-only data: weights, scales, biases, constant tensors. |
| `arena_mem_area`   | Where Vela places read-write tensor arena data: input, output, intermediate activations, scratch tensors. |
| `cache_mem_area`   | Where Vela places dedicated fast SRAM/cache/staging memory if that mode uses it. |
| `arena_cache_size` | Size in bytes available for the arena or cache, depending on the selected memory mode. |

The values Axi0 and Axi1 are Vela aliases, not physical memories by themselves.
System_Config later says what memory type each alias represents:

```ini
[System_Config.My_System]
axi0_port=Sram
axi1_port=OffChipFlash
```

So this:
```ini
[Memory_Mode.Shared_Sram]
const_mem_area=Axi1
arena_mem_area=Axi0
cache_mem_area=Axi0
```

means:
```txt
constants -> Axi1 -> OffChipFlash
arena     -> Axi0 -> Sram
cache     -> Axi0 -> Sram
```

Built-in modes in Vela's Arm config are:

```ini
[Memory_Mode.Sram_Only]
const_mem_area=Axi0
arena_mem_area=Axi0
cache_mem_area=Axi0

[Memory_Mode.Shared_Sram]
const_mem_area=Axi1
arena_mem_area=Axi0
cache_mem_area=Axi0

[Memory_Mode.Dedicated_Sram]
const_mem_area=Axi1
arena_mem_area=Axi1
cache_mem_area=Axi0
```

- Sram_Only
  - All model data is in SRAM. Constants and arena are separate logical regions,
    but both use the same memory path. cache_mem_area is not used as a separate cache.
- Shared_Sram
  - SRAM is shared between Ethos-U and Cortex-M software. Activations/scratch/output
    go to SRAM, while constants usually go to another memory such as Flash, MRAM,
    or DRAM.
- Dedicated_Sram
  - Fast SRAM is reserved for Ethos-U cache/staging. Constants and arena go to another
    memory, commonly DRAM or external memory. arena_cache_size is the size of the
    dedicated SRAM cache/staging area.

**Important**: arena_cache_size changes meaning by mode:

- Sram_Only / Shared_Sram:
  - arena_cache_size limits arena_mem_area

- Dedicated_Sram:
  - arena_cache_size limits cache_mem_area

> NOTE
>
> Settings must match the driver and application memory placement. If Vela thinks
> constants are on Axi1, but the application/linker/driver place them somewhere
> else, the generated command stream will not match the target system.

`System_Config` describes the hardware performance model that Vela should assume for
the target system and use for scheduling, placement, and performance decisions.

Where `Memory_Mode` says which logical data goes to Axi0 or Axi1, `System_Config`
says what Axi0 and Axi1 actually represent, and how fast those memories are.

A typical section from Vela's Arm config looks like this:

```ini
[System_Config.Ethos_U55_High_End_Embedded]
core_clock=500e6
axi0_port=Sram
axi1_port=OffChipFlash

Sram_clock_scale=1.0
Sram_burst_length=32
Sram_read_latency=32
Sram_write_latency=32
Sram_max_reads=4
Sram_max_writes=4

OffChipFlash_clock_scale=0.125
OffChipFlash_burst_length=128
OffChipFlash_read_latency=64
OffChipFlash_write_latency=0
OffChipFlash_max_reads=2
OffChipFlash_max_writes=0
```

The first part defines the NPU clock and AXI mapping:

| Option       | Meaning                                                         |
| ------------ | --------------------------------------------------------------- |
| `core_clock` | Ethos-U clock frequency in Hz. Used for performance estimation. |
| `axi0_port`  | Memory type behind Vela’s `Axi0` alias.                         |
| `axi1_port`  | Memory type behind Vela’s `Axi1` alias.                         |

Supported memory type names are:

```txt
- Sram
- Dram
- OnChipFlash
- OffChipFlash
```

Each selected memory type has performance parameters:

| Option pattern           | Meaning |
| ------------------------ | --------------------------------------------- |
| `<Memory>_clock_scale`   | Memory bandwidth scale relative to `core_clock`. |
| `<Memory>_ports_used`    | Number of memory ports used. Optional in older configs; Vela clamps to at least 1. |
| `<Memory>_burst_length`  | Minimum efficient burst length, in bytes.     |
| `<Memory>_read_latency`  | Read latency, in cycles.                      |
| `<Memory>_write_latency` | Write latency, in cycles.                     |
| `<Memory>_max_reads`     | Maximum outstanding reads Vela should model.  |
| `<Memory>_max_writes`    | Maximum outstanding writes Vela should model. |

### Create hardware performance model

Creating hardware performance model for Vela means determining value for
`Memory_Mode` and `System_Config`. Keep in mind, that this is simplified
performance model and derived values give a first model.

Lets derive Vela configuration for Alif Ensemble E7 NPU-HP: Ethos-U55,
256MAC/cycle, 400 MHz. We will place constants in MRAM and arena/cache in SRAM.

The Vela memory-mode side is:

```ini
[Memory_Mode.Shared_Sram]
const_mem_area=Axi1
arena_mem_area=Axi0
cache_mem_area=Axi0
```

So the matching System_Config should model:

```txt
Axi0 -> SRAM
Axi1 -> MRAM
```

Vela does not have an MRAM memory type name, so the closest Vela category is
OnChipFlash: on-chip, non-volatile, read-only storage.

Lets go step by step:

1. NPU clock
   - from the E7 datasheet, maximum performance table: 400MHz
     So
     ```ini
     core_clock=400e6
     ```

2. AXI memory mapping
   - from the memory placement
     ```ini
     axi0_port=Sram
     axi1_port=OnChipFlash
     ```

     OnChipFlash here means “Vela model for on-chip non-volatile constant memory”;
     the physical memory is MRAM.

3. SRAM bandwidth
   - E7& interconnect states:

     ```txt
     64-bit wide read/write data paths
     400 MHz bus clock
     ```

     So theoretical peak bandwidth is:

     ```txt
     64 bits = 8 bytes
     8 bytes/cycle * 400 MHz = 3200 MB/s
     ```

     For Ethos-U55 at 400 MHz, that corresponds to:

     ```txt
     Sram_clock_scale=1.0
     ```

4. MRAM bandwidth
   - E7 MRAM read table gives:

     ```txt
     Read operation: 16 bytes
     Read time: 69 ns to 276 ns
     Effective read rate: 232 MB/s to 58 MB/s
     ```

     Relative to the 3200 MB/s 64-bit/400 MHz model:

     ```txt
     MRAM peak scale        = 232 / 3200 = 0.0725
     MRAM conservative scale = 58 / 3200 = 0.018125
     ```

     MRAM read latency in NPU cycles:
     ```txt
     400 MHz -> 2.5 ns/cycle

     69 ns  / 2.5 = 27.6  -> 28 cycles
     276 ns / 2.5 = 110.4 -> 111 cycles
     ```

5. TODO

## How to determine memory requirements

### Select the network and target

Determine:

- quantized TFLite/TOSA network (i.e. select ML model)
- target device:
  - Ethos-U type and MAC count
  - Memory topology
    - Constant-storage location
    - Arena-storage location

Consider memory modes from Arm's reference configuration for Vela:

```ini
[Memory_Mode.Sram_Only]
const_mem_area=Axi0
arena_mem_area=Axi0
cache_mem_area=Axi0

[Memory_Mode.Shared_Sram]
const_mem_area=Axi1
arena_mem_area=Axi0
cache_mem_area=Axi0

[Memory_Mode.Dedicated_Sram]
const_mem_area=Axi1
arena_mem_area=Axi1
cache_mem_area=Axi0
```

- Sram_Only
  - All model data is in SRAM. Constants and arena are separate logical regions, but both use
    the same memory path. cache_mem_area is not used as a separate cache.
- Shared_Sram
  - SRAM is shared between Ethos-U and Cortex-M software. Activations/scratch/output go to SRAM,
    while constants usually go to another memory such as Flash, MRAM, or DRAM.
- Dedicated_Sram
  - Fast SRAM is reserved for Ethos-U cache/staging. Constants and arena go to another memory,
    commonly DRAM or external memory. arena-cache-size is the size of the dedicated SRAM cache/staging area.

Example:

- device with Ethos-U55, 256 MACs
- constants will reside in constant-memory (Flash/MRAM)
- arena will reside in SRAM

### Compile the network for minimum model memory

Use Vela compiler and invoke it for selected model and with the `--optimise Size` option.
Such invocation will optimize for minimal SRAM usage and Vela will output network summary
which will give an estimate about the minimum required arena size when using selected model.

```shell
vela network.tflite
  --config <device-vela.ini>
  --accelerator-config ethos-u55-256
  --system-config <System_Config>
  --memory-mode <Memory_Mode>
  --optimise Size
  --verbose-allocation
  --output-dir out\vela-size
```

Example summary:

```shell
Network summary for <selected model>
...
Total SRAM used                                102.00 KiB
Total Off-chip Flash used                      346.92 KiB
...
```

Keep in mind that this is minimum memory consumed by the model-controlled tensor allocation,
not the complete ML framework. ML framework requires memory for its own operation and also adds
overhead to the activation buffer size and there should be some safety margin as well to account
for buffer alignment needs.

> NOTE: `--optimise Size` produces Vela’s practical memory-minimized schedule for the exact model,
> Vela version, accelerator, and system configuration. It is not a mathematically guaranteed
> global minimum.

### Determine memory budget

Once model-controlled minimum memory allocation is known one can determine total memory
requirements for the target memory region:

The amount of Off-chip Flash used is constant. One should only pay attention to allocate this
memory into correct region.

For SRAM, there are other objects in memory that reside in the same region as ML model:

```txt
Total Required SRAM = model SRAM used +
                      ML framework +
                      safety +
                      application objects + ...
```

If "Total Required SRAM" is less than the available memory in selected SRAM region, memory
available to the ML model can be increased and model can be re-compiled for performance.

### Compile the network for performance

When re-compiling ML model network for performance, invoke Vela using `--optimise Performance`
and using additional argument `--arena-cache-size <budget_bytes>`.

This argument has the same meaning as `arena_cache_size` in configuration file:

```ini
arena_cache_size=<budget-bytes>
```

To keep the configuration file generic `arena_cache_size` is usually not specified.

```shell
vela network.tflite
  --config <device-vela.ini>
  --accelerator-config ethos-u55-256
  --system-config <System_Config>
  --memory-mode <Memory_Mode>
  --optimise Performance
  --arena-cache-size <budget_bytes>
  --verbose-allocation
  --output-dir out\vela-performance
```

To evaluate ML model performance, compile several candidates, using different setting for `--arena-cache-size`.

Note that `--arena-cache-size` for reference memory mode `Shared_Sram` is an optimization target,
not an absolute allocation guarantee. Vela may therefore emit:

```txt
Warning: SRAM target for arena memory area exceeded.
Target = ... Bytes, Actual = ... Bytes
```

In such case, use the reported actual memory area size.

### Quick Summary

To determine memory requirements for a project, follow high-level flow below:

```txt
Network + target architecture
        ↓
Vela --optimise Size
        ↓
Minimum model-controlled arena
        ↓
Add ML framework overhead + application requirements
        ↓
Determine feasible ML model memory budget
        ↓
Vela Performance budget sweep
        ↓
Choose performance based on project requirements
        ↓
Determine actual final allocation
        ↓
Set activation buffer size and verify
```

## How to configure Ethos-U driver

Assumptions:

- ML model was successfully compiled using Vela
- linker script for the target device is properly constructed

Before we can continue one must understand how Vela constructs the Ethos-U custom
operator inputs. There are slight differences depending on the Vela's output format.

### TFLite-style custom operators

The order is below:

```txt
Operator input 0: command-stream/COP data
Operator input 1: read-only model data
Operator input 2: main scratch/arena
Operator input 3: fast scratch
Operator input 4+: network inputs
```

A "network input" (operator inputs 4+) is data entering the neural-network computation, such as:

- an input image
- an audio feature tensor
- sensor data
- an output from a preceding CPU operator
- an output from another NPU subgraph

So, the first four operator inputs are Vela runtime infrastructure. Inputs 4 onward carry actual inference data.

How does this apply to Ethos-U?

Ethos-U uses separate register for command stream fetches (QBASE) and multiple registers (BASEP) to
access weights, activations, scratch buffers or outputs.

TFLM will convert operator input ordering into:

```txt
base_addrs[0] = operator input 1 = model/constants
base_addrs[1] = operator input 2 = main arena
base_addrs[2] = operator input 3 = fast scratch
```

Considering this ordering, there are several combinations depending on how the ML model was compiled.

Assuming configuration where:

```txt
command stream  -> AXI1 -> OffChipFlash
model/constants -> AXI1 -> OffChipFlash
main arena      -> AXI0 -> SRAM
fast scratch    -> AXI0 -> SRAM
network inputs  -> AXI0 -> SRAM
```

One would configure QBASE and BASEP0 for access to flash, while the rest of BASEPn registers
would be configured for access to SRAM.

It makes sense to split command stream and model constants accesses to separate counters, hence
in configuration header select:

```txt
NPU_QCONFIG     -> AXI1 counter 2, uses AXI_LIMIT2
NPU_REGIONCFG_0 -> AXI1 counter 3, uses AXI_LIMIT3
```

This covers accesses to Flash. For accesses to SRAM it makes sense to split accesses to main arena
and everything other to separate counters, hence:

```txt
NPU_REGIONCFG_1    -> AXI0 counter 0, uses AXI_LIMIT0
NPU_REGIONCFG_2..7 -> AXI0 counter 1, uses AXI_LIMIT1
```

### ExecuTorch custom operators (Vela's raw output format)

Ethos-U backend in ExecuTorch uses Vela's raw output format and constructs a fixed three-entry
base-address array.

The raw Vela output already separates:

```txt
- command data
- weight data
- scratch-buffer size
- input offsets within scratch
- output offsets within scratch
```

At runtime, Ethos-U backend in ExecuTorch iterates over input arguments and constructs something like:

```txt
ExecuTorch args[0] -> copied to scratch + Vela input[0].offset
ExecuTorch args[1] -> copied to scratch + Vela input[1].offset
...
```

The NPU accesses those input through BASEP1 + Vela-generated offset.
ExecuTorch does not pass each network input as a separate BASEP address.
Instead it defines three base addresses:

| Array entry | NPU register | Assignment                                   |
|------------:|--------------|----------------------------------------------|
| `bases[0]`  | `BASEP0`     | weights and constants                        |
| `bases[1]`  | `BASEP1`     | scratch buffer, including inputs and outputs |
| `bases[2]`  | `BASEP2`     | Dedicated fast-scratch buffer, if used       |

BASEP3..7 are not used.

Ethos-U driver configuration is therefore the same as in the case for TFLite
except that BASEP3..7 are not used, hence their configuration is irrelevant.


----

https://github.com/ArmDeveloperEcosystem/arm-learning-paths/compare/main...matt-cossins:arm-learning-paths:alif_conformer