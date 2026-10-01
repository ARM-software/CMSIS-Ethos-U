# Configure Ethos-U for FVP Simulation Models {#fvp-ethos-setup}

Each column describes one supported combination of Corstone system, Ethos-U
accelerator, and Vela memory mode for the `Test-Ethos-U` examples. For the
complete process of adapting these settings to physical hardware, see the
<a href="index.html">Ethos-U integration workflow</a>.

## Ethos-U55

| Setting            | SRAM only                     | Shared SRAM                   |
|--------------------|-------------------------------|-------------------------------|
| Target type        | `SSE-300-U55`                 | `SSE-300-U55`                 |
| Solution           | `Test-Ethos-U55`              | `Test-Ethos-U55`              |
| NPU                | U55, 128 MACs                 | U55, 128 MACs                 |
| Vela INI           | `Model/vela.ini`              | `Model/vela.ini`              |
| Vela `system`      | `Ethos_U55_High_End_Embedded` | `Ethos_U55_High_End_Embedded` |
| Vela `memory`      | `Sram_Only`                   | `Shared_Sram`                 |
| `NPU_QCONFIG`      | `1`                           | `2` (default)                 |
| `NPU_REGIONCFG_0`  | `1`                           | `3` (default)                 |
| `NPU_REGIONCFG_1`  | `0`                           | `0` (default)                 |
| `NPU_REGIONCFG_2`  | Unused                        | Unused                        |
| `ETHOS_CACHE_SIZE` | Not used                      | Not used                      |
| `ethos_model`      | SRAM_VM0 / RAM1 via `AXI0`    | DDR4_3 / ROM2 via `AXI1`      |
| `ethos_arena`      | SRAM_VM0 / RAM1 via `AXI0`    | SRAM_VM0 / RAM1 via `AXI0`    |
| `ethos_cache`      | Unused                        | Unused                        |

## Ethos-U65

| Setting            | SRAM only                     | Shared SRAM                   | Dedicated SRAM                |
|--------------------|-------------------------------|-------------------------------|-------------------------------|
| Target type        | `SSE-300-U65`                 | `SSE-300-U65`                 | `SSE-300-U65`                 |
| Solution           | `Test-Ethos-U65`              | `Test-Ethos-U65`              | `Test-Ethos-U65`              |
| NPU                | U65, 256 MACs                 | U65, 256 MACs                 | U65, 256 MACs                 |
| Vela INI           | `Model/vela.ini`              | `Model/vela.ini`              | `Model/vela.ini`              |
| Vela `system`      | `Ethos_U65_Embedded`          | `Ethos_U65_Embedded`          | `Ethos_U65_Mid_End`           |
| Vela `memory`      | `Sram_Only`                   | `Shared_Sram`                 | `Dedicated_Sram_384KB`        |
| `NPU_QCONFIG`      | `1`                           | `2` (default)                 | `2`                           |
| `NPU_REGIONCFG_0`  | `1`                           | `3` (default)                 | `3`                           |
| `NPU_REGIONCFG_1`  | `0`                           | `0` (default)                 | `2`                           |
| `NPU_REGIONCFG_2`  | Unused                        | Unused                        | `1`                           |
| `ETHOS_CACHE_SIZE` | Not used                      | Not used                      | `393216` bytes                |
| `ethos_model`      | SRAM_VM0 / RAM1 via `AXI0`    | DDR4_3 / ROM2 via `AXI1`      | DDR4_3 / ROM2 via `AXI1`      |
| `ethos_arena`      | SRAM_VM0 / RAM1 via `AXI0`    | SRAM_VM0 / RAM1 via `AXI0`    | DDR4_1 / RAM0 via `AXI1`      |
| `ethos_cache`      | Unused                        | Unused                        | SRAM_VM0 / RAM1 via `AXI0`    |

## Ethos-U85

| Setting            | SRAM only                      | Shared SRAM                    | Dedicated SRAM                 |
|--------------------|--------------------------------|--------------------------------|--------------------------------|
| Target type        | `SSE-320-U85`                  | `SSE-320-U85`                  | `SSE-320-U85`                  |
| Solution           | `Test-Ethos-U85`               | `Test-Ethos-U85`               | `Test-Ethos-U85`               |
| NPU                | U85, 256 MACs                  | U85, 256 MACs                  | U85, 256 MACs                  |
| Vela INI           | `Model/vela.ini`               | `Model/vela.ini`               | `Model/vela.ini`               |
| Vela `system`      | `Ethos_U85_SYS_DRAM_Mid`       | `Ethos_U85_SYS_DRAM_Mid`       | `Ethos_U85_SYS_DRAM_Mid`       |
| Vela `memory`      | `Sram_Only`                    | `Shared_Sram`                  | `Dedicated_Sram_384KB`         |
| `NPU_QCONFIG`      | `1`                            | `2` (default)                  | `2`                            |
| `NPU_REGIONCFG_0`  | `1`                            | `3` (default)                  | `3`                            |
| `NPU_REGIONCFG_1`  | `0`                            | `0` (default)                  | `2`                            |
| `NPU_REGIONCFG_2`  | Unused                         | Unused                         | `1`                            |
| `ETHOS_CACHE_SIZE` | Not used                       | Not used                       | `393216` bytes                 |
| `ethos_model`      | SRAM_VM0 / RAM1 via `AXI_SRAM` | DDR4_3 / ROM2 via `AXI_EXT`    | DDR4_3 / ROM2 via `AXI_EXT`    |
| `ethos_arena`      | SRAM_VM0 / RAM1 via `AXI_SRAM` | SRAM_VM0 / RAM1 via `AXI_SRAM` | DDR4_1 / RAM0 via `AXI_EXT`    |
| `ethos_cache`      | Unused                         | Unused                         | SRAM_VM0 / RAM1 via `AXI_SRAM` |

The values marked **default** are supplied by the selected Generic Ethos-U
driver configuration when the board layer does not override them. The
SRAM-only U55 configuration must override the shared-SRAM defaults.

After selecting a column, regenerate the solution's `*.cbuild-mlops.yml` file
and convert both models before building:

| NPU | Conversion command |
|-----|--------------------|
| U55 | `python script/model-converter.py Test-Ethos-U55.cbuild-mlops.yml` |
| U65 | `python script/model-converter.py Test-Ethos-U65.cbuild-mlops.yml` |
| U85 | `python script/model-converter.py Test-Ethos-U85.cbuild-mlops.yml` |

> NOTES
>
> - The generated files under `Model/` are shared by all solutions. Always
>   reconvert them after changing the target or Vela configuration.
> - `ROM2` is a read-only linker execution region backed by DDR4_3; it is not
>   physical ROM.
> - `Shared_Sram` places the command stream and constants in external memory
>   and the writable tensor arena in SRAM. It does not use a dedicated Vela
>   arena cache.
> - `Dedicated_Sram_384KB` places the model and writable arena in external
>   DDR4 and reserves 384 KiB of SRAM as the Vela arena cache.
>   `ETHOS_CACHE_SIZE` must equal Vela's `arena_cache_size` of 393216 bytes.
> - Ethos-U55 does not use `Dedicated_Sram` because its AXI1 interface is
>   read-only, while that mode requires a writable arena on Axi1.
