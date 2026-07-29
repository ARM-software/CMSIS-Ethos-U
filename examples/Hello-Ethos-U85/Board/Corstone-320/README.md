# Board layer: Arm Corstone-320 (SSE-320) FVP with Ethos-U85

```yml
  board:  ARM::SSE-320
  device: ARM::SSE-320-FVP
```

Derived from the Corstone-320 board layer in the
[SDS-Framework template](https://github.com/ARM-software/SDS-Framework/tree/main/template),
reduced to what this example needs: the VSI/VIO virtual peripherals and the
CPU-only layer variant have been removed.

## Provided interfaces

| Interface | Notes |
|---|---|
| `STDIN` / `STDOUT` / `STDERR` | via UART0; `mps4_board.uart0.out_file=-` sends it to the host stdout |
| `Heap` | 96 kB |

## System configuration

| Setting | Value | Where |
|---|---|---|
| Stack (MSP) | 32 kB | `regions_SSE-320.h` |
| Heap | 96 kB | `regions_SSE-320.h` |
| Ethos-U cache | 384 kB | `ETHOS_CACHE_BUF_SIZE` in `ethos_setup.c` |
| NPU MACs/cycle | 256 | `fvp_config.txt` |

The stack is raised from the template's 4 kB: TensorFlow Lite Micro builds its
interpreter, allocator and op resolver on the stack, and the CMSIS-NN kernels
need working space on top of that. 4 kB is not enough for the CPU inference
path. (The template's own README already documented 32 kB; its header disagreed.)

## Memory map

| Region | Symbol | Address | Size | Contents |
|---|---|---|---|---|
| ROM0 | `BOOT_ROM_S` | `0x11000000` | 128 kB | vectors, startup |
| ROM1 | `FPGA_SRAM_S` | `0x12000000` | 2 MB | `.text`, `.rodata` |
| ROM2 | `DDR4_3_S` | `0x90000000` | 256 MB | section `ethos_model` |
| RAM0 | `DDR4_1_S` | `0x70000000` | 256 MB | `.data`, `.bss`, heap, stack |
| RAM1 | `SRAM_VM0_S` | `0x31000000` | 2 MB | `ethos_arena`, `ethos_cache` |
| RAM3 | `DTCM_S` | `0x30000000` | 32 kB | spillover |

Three section names are the contract between this layer and the ML-Model layer:

- `ethos_model` — model data, in DDR, reached by the NPU over Axi1.
- `ethos_arena` — the tensor arena, in on-chip SRAM, reached over Axi0.
- `ethos_cache` — the NPU's cache/staging buffer, also in SRAM.

`ETHOS_CACHE_BUF_SIZE` (384 kB) must equal `arena_cache_size` in
`Model/vela.ini`. If Vela plans for a larger cache than the driver provides, the
NPU reads past the end of the buffer.

## NPU wiring

`ethos_setup.c` initializes the driver at `NPU0_APB_BASE_S`, secure and
privileged, enables `NPU0_IRQn`, and prints the hardware identity.
`NPU0_Handler` forwards to `ethosu_irq_handler`.

Caches are disabled by `SystemInit()` in `system_SSE320.c`, so no
`ethosu_flush_dcache` / `ethosu_invalidate_dcache` overrides are needed. If you
enable the data cache, those hooks must be implemented.

## FVP configuration

`fvp_config.txt` sets:

- `mps4_board.subsystem.ethosu.num_macs=256` — **must** match the
  `--accelerator-config ethos-u85-256` used to compile the models. The
  integration test asserts this at runtime.
- `mps4_board.uart0.out_file=-` — UART0 to host stdout.
- `mps4_board.uart0.shutdown_on_eot=1` — an EOT byte (ASCII 4) ends the
  simulation, which is how the test terminates itself.
- `mps4_board.subsystem.ethosu.extra_args='--fast'` — fast functional NPU mode.
  NPU performance counters are not representative of real hardware in this mode.

Note the instance prefix on Corstone-320 is `mps4_board.`, not the `mps3_board.`
used by Corstone-300 and Corstone-310.

## UART0 base address workaround

`ARM::SSE_320_BSP@1.1.0` carries a UART0 base address that does not match the
FVP. `main.c` defines a corrected device structure at `0x58203000 + 0x01100000`,
enabled by the `UART0_BASE_S_TMP` define in `Board.clayer.yml` and paired with an
edit in `RTE/Device/SSE-320-FVP/device_cfg.h`. Without it stdio produces no
output at all. Remove all three once the BSP is fixed.
