# Vela report: hello_world

- accelerator: `ethos-u85-256`
- system config: `Corstone_320`
- memory mode: `Dedicated_Sram_384KB`

```
Network summary for hello_world_int8
Accelerator configuration               Ethos_U85_256
System configuration                     Corstone_320
Memory mode                      Dedicated_Sram_384KB
Accelerator clock                                1000 MHz
Design peak SRAM bandwidth                      29.80 GB/s
Design peak DRAM bandwidth                      11.18 GB/s

Total SRAM used                                  0.03 KiB
Total DRAM used                                  1.03 KiB

CPU operators = 0 (0.0%)
NPU operators = 3 (100.0%)

Average SRAM bandwidth                           0.05 GB/s
Input   SRAM bandwidth                           0.00 MB/batch
Weight  SRAM bandwidth                           0.00 MB/batch
Output  SRAM bandwidth                           0.00 MB/batch
Total   SRAM bandwidth                           0.00 MB/batch
Total   SRAM bandwidth            per input      0.00 MB/inference (batch size 1)

Average DRAM bandwidth                           0.74 GB/s
Input   DRAM bandwidth                           0.00 MB/batch
Weight  DRAM bandwidth                           0.00 MB/batch
Output  DRAM bandwidth                           0.00 MB/batch
Total   DRAM bandwidth                           0.00 MB/batch
Total   DRAM bandwidth            per input      0.00 MB/inference (batch size 1)

Neural network macs                               280 MACs/batch
```
