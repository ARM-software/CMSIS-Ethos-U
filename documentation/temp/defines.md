# Driver compile-time definitions

The configurable compile-time definitions are:

| CMake input | Resulting C definition |
|---|---|
| `ETHOSU_TARGET_NPU_CONFIG` | `ETHOSU_ARCH`, `ETHOSU_MACS`, and one of `ETHOSU55`, `ETHOSU65`, `ETHOSU85` |
| `ETHOSU_INFERENCE_TIMEOUT` | `ETHOSU_SEMAPHORE_WAIT_INFERENCE` |
| `ETHOSU_LOG_ENABLE` | `ETHOSU_LOG_ENABLE` |
| `ETHOSU_LOG_SEVERITY` | `ETHOSU_LOG_SEVERITY` |

These mappings are in `source/CMakeLists.txt`.

Direct configuration defines shared by all NPUs:

- `NPU_QCONFIG`
- `NPU_REGIONCFG_0` through `NPU_REGIONCFG_7`

Ethos-U55/U65 defines, for each `x = 0..3`:

- `AXI_LIMITx_MAX_BEATS_BYTES`
- `AXI_LIMITx_MEM_TYPE`
- `AXI_LIMITx_MAX_OUTSTANDING_READS`
- `AXI_LIMITx_MAX_OUTSTANDING_WRITES`

See `source/config/ethosu_config_u55.h` and
`source/config/ethosu_config_u65.h`.

Ethos-U85 defines:

- `NPU_MAC_PWR_RAMP_CYCLES`
- `NPU_MEM_ATTR_0` through `NPU_MEM_ATTR_3`
- `AXI_LIMIT_SRAM_MAX_OUTSTANDING_READ_M1`
- `AXI_LIMIT_SRAM_MAX_OUTSTANDING_WRITE_M1`
- `AXI_LIMIT_SRAM_MAX_BEATS`
- `AXI_LIMIT_EXT_MAX_OUTSTANDING_READ_M1`
- `AXI_LIMIT_EXT_MAX_OUTSTANDING_WRITE_M1`
- `AXI_LIMIT_EXT_MAX_BEATS`

See `source/config/ethosu_config_u85.h`.

Fixed supporting macros—not configuration options—include:

- `ETHOSU_SEMAPHORE_WAIT_FOREVER`
- `ETHOSU_LOG_ERR`, `ETHOSU_LOG_WARN`, `ETHOSU_LOG_INFO`, `ETHOSU_LOG_DEBUG`
- `ETHOSU` target marker

The main inconsistency is that logging uses identical names for CMake variables
and C definitions, while timeout and target selection use different CMake and C
names. Hardware configuration options are only direct C definitions.
