# Vela report: hello_world

- accelerator: `ethos-u55-128`
- system config: `Ethos_U55_High_End_Embedded`
- memory mode: `Shared_Sram`

```
Network summary for hello_world_int8
Accelerator configuration               Ethos_U55_128
System configuration             Ethos_U55_High_End_Embedded
Memory mode                               Shared_Sram
Accelerator clock                                 500 MHz
Design peak SRAM bandwidth                       3.73 GB/s
Design peak Off-chip Flash bandwidth             0.47 GB/s

Total SRAM used                                  0.03 KiB
Total Off-chip Flash used                        0.75 KiB

CPU operators = 0 (0.0%)
NPU operators = 3 (100.0%)

Average SRAM bandwidth                           0.04 GB/s
Input   SRAM bandwidth                           0.00 MB/batch
Weight  SRAM bandwidth                           0.00 MB/batch
Output  SRAM bandwidth                           0.00 MB/batch
Total   SRAM bandwidth                           0.00 MB/batch
Total   SRAM bandwidth            per input      0.00 MB/inference (batch size 1)

Average Off-chip Flash bandwidth                 0.46 GB/s
Input   Off-chip Flash bandwidth                 0.00 MB/batch
Weight  Off-chip Flash bandwidth                 0.00 MB/batch
Output  Off-chip Flash bandwidth                 0.00 MB/batch
Total   Off-chip Flash bandwidth                 0.00 MB/batch
Total   Off-chip Flash bandwidth  per input      0.00 MB/inference (batch size 1)

Neural network macs                               288 MACs/batch
```
