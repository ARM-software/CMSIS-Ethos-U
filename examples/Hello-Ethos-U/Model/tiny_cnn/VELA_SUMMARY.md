# Vela report: tiny_cnn

- accelerator: `ethos-u55-128`
- system config: `Ethos_U55_High_End_Embedded`
- memory mode: `Shared_Sram`

```
Network summary for tiny_cnn_int8
Accelerator configuration               Ethos_U55_128
System configuration             Ethos_U55_High_End_Embedded
Memory mode                               Shared_Sram
Accelerator clock                                 500 MHz
Design peak SRAM bandwidth                       3.73 GB/s
Design peak Off-chip Flash bandwidth             0.47 GB/s

Total SRAM used                                  5.20 KiB
Total Off-chip Flash used                        2.12 KiB

CPU operators = 0 (0.0%)
NPU operators = 6 (100.0%)

Average SRAM bandwidth                           1.17 GB/s
Input   SRAM bandwidth                           0.01 MB/batch
Weight  SRAM bandwidth                           0.00 MB/batch
Output  SRAM bandwidth                           0.01 MB/batch
Total   SRAM bandwidth                           0.02 MB/batch
Total   SRAM bandwidth            per input      0.02 MB/inference (batch size 1)

Average Off-chip Flash bandwidth                 0.13 GB/s
Input   Off-chip Flash bandwidth                 0.00 MB/batch
Weight  Off-chip Flash bandwidth                 0.00 MB/batch
Output  Off-chip Flash bandwidth                 0.00 MB/batch
Total   Off-chip Flash bandwidth                 0.00 MB/batch
Total   Off-chip Flash bandwidth  per input      0.00 MB/inference (batch size 1)

Neural network macs                             35328 MACs/batch
```
