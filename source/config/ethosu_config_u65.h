/*
 * Copyright (c) 2019-2020,2022,2026 Arm Limited.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ETHOSU_CONFIG_U65_H
#define ETHOSU_CONFIG_U65_H

// <<< Use Configuration Wizard in Context Menu >>>

// <h>Ethos-U65 AXI Configuration
// <i>Configure command stream routing, base pointer routing, and AXI access limits.

// <o> Command stream AXI target
//   <0=> AXI0 counter 0, uses AXI_LIMIT0
//   <1=> AXI0 counter 1, uses AXI_LIMIT1
//   <2=> AXI1 counter 2, uses AXI_LIMIT2
//   <3=> AXI1 counter 3, uses AXI_LIMIT3
// <i>Selects the AXI port and outstanding counter for command stream fetches.
// <i>Default: 2
#ifndef NPU_QCONFIG
#define NPU_QCONFIG                             2
#endif

// <o> Base pointer region 0 AXI target
//   <0=> AXI0 counter 0, uses AXI_LIMIT0
//   <1=> AXI0 counter 1, uses AXI_LIMIT1
//   <2=> AXI1 counter 2, uses AXI_LIMIT2
//   <3=> AXI1 counter 3, uses AXI_LIMIT3
// <i>Selects the AXI port and outstanding counter for BASEP0 accesses.
// <i>Default: 3
#ifndef NPU_REGIONCFG_0
#define NPU_REGIONCFG_0                         3
#endif

// <o> Base pointer region 1 AXI target
//   <0=> AXI0 counter 0, uses AXI_LIMIT0
//   <1=> AXI0 counter 1, uses AXI_LIMIT1
//   <2=> AXI1 counter 2, uses AXI_LIMIT2
//   <3=> AXI1 counter 3, uses AXI_LIMIT3
// <i>Selects the AXI port and outstanding counter for BASEP1 accesses.
// <i>Default: 2
#ifndef NPU_REGIONCFG_1
#define NPU_REGIONCFG_1                         2
#endif

// <o> Base pointer region 2 AXI target
//   <0=> AXI0 counter 0, uses AXI_LIMIT0
//   <1=> AXI0 counter 1, uses AXI_LIMIT1
//   <2=> AXI1 counter 2, uses AXI_LIMIT2
//   <3=> AXI1 counter 3, uses AXI_LIMIT3
// <i>Selects the AXI port and outstanding counter for BASEP2 accesses.
// <i>Default: 1
#ifndef NPU_REGIONCFG_2
#define NPU_REGIONCFG_2                         1
#endif

// <o> Base pointer region 3 AXI target
//   <0=> AXI0 counter 0, uses AXI_LIMIT0
//   <1=> AXI0 counter 1, uses AXI_LIMIT1
//   <2=> AXI1 counter 2, uses AXI_LIMIT2
//   <3=> AXI1 counter 3, uses AXI_LIMIT3
// <i>Selects the AXI port and outstanding counter for BASEP3 accesses.
// <i>Default: 1
#ifndef NPU_REGIONCFG_3
#define NPU_REGIONCFG_3                         1
#endif

// <o> Base pointer region 4 AXI target
//   <0=> AXI0 counter 0, uses AXI_LIMIT0
//   <1=> AXI0 counter 1, uses AXI_LIMIT1
//   <2=> AXI1 counter 2, uses AXI_LIMIT2
//   <3=> AXI1 counter 3, uses AXI_LIMIT3
// <i>Selects the AXI port and outstanding counter for BASEP4 accesses.
// <i>Default: 1
#ifndef NPU_REGIONCFG_4
#define NPU_REGIONCFG_4                         1
#endif

// <o> Base pointer region 5 AXI target
//   <0=> AXI0 counter 0, uses AXI_LIMIT0
//   <1=> AXI0 counter 1, uses AXI_LIMIT1
//   <2=> AXI1 counter 2, uses AXI_LIMIT2
//   <3=> AXI1 counter 3, uses AXI_LIMIT3
// <i>Selects the AXI port and outstanding counter for BASEP5 accesses.
// <i>Default: 1
#ifndef NPU_REGIONCFG_5
#define NPU_REGIONCFG_5                         1
#endif

// <o> Base pointer region 6 AXI target
//   <0=> AXI0 counter 0, uses AXI_LIMIT0
//   <1=> AXI0 counter 1, uses AXI_LIMIT1
//   <2=> AXI1 counter 2, uses AXI_LIMIT2
//   <3=> AXI1 counter 3, uses AXI_LIMIT3
// <i>Selects the AXI port and outstanding counter for BASEP6 accesses.
// <i>Default: 1
#ifndef NPU_REGIONCFG_6
#define NPU_REGIONCFG_6                         1
#endif

// <o> Base pointer region 7 AXI target
//   <0=> AXI0 counter 0, uses AXI_LIMIT0
//   <1=> AXI0 counter 1, uses AXI_LIMIT1
//   <2=> AXI1 counter 2, uses AXI_LIMIT2
//   <3=> AXI1 counter 3, uses AXI_LIMIT3
// <i>Selects the AXI port and outstanding counter for BASEP7 accesses.
// <i>Default: 1
#ifndef NPU_REGIONCFG_7
#define NPU_REGIONCFG_7                         1
#endif

// <h>AXI_LIMIT0 - AXI0 Counter 0
// <i>Configures AXI transaction attributes and outstanding limits for traffic assigned to AXI0 counter 0.

// <o> Burst split alignment
//   <0=> 64 bytes
//   <1=> 128 bytes
//   <2=> 256 bytes
// <i>An AXI burst that would cross the selected aligned boundary is split into multiple bursts.
// <i>This limits the span of each burst to meet the boundary requirements of the AXI interconnect and memory system.
// <i>Default: 0
#ifndef AXI_LIMIT0_MAX_BEATS_BYTES
#define AXI_LIMIT0_MAX_BEATS_BYTES              0x0
#endif

// <o> Memory type
//   <0x0=> Device non-bufferable
//   <0x1=> Device bufferable
//   <0x2=> Normal non-cacheable non-bufferable
//   <0x3=> Normal non-cacheable bufferable
//   <0x4=> Write-through no allocate
//   <0x5=> Write-through read allocate
//   <0x6=> Write-through write allocate
//   <0x7=> Write-through read and write allocate
//   <0x8=> Write-back no allocate
//   <0x9=> Write-back read allocate
//   <0xA=> Write-back write allocate
//   <0xB=> Write-back read and write allocate
// <i>Sets the buffering and caching attributes requested for traffic assigned to AXI0 counter 0.
// <i>Default: 0x0
#ifndef AXI_LIMIT0_MEM_TYPE
#define AXI_LIMIT0_MEM_TYPE                     0x0
#endif

// <o> Maximum outstanding reads <1..64>
// <i>Sets the AXI_LIMIT0 read transaction limit. The driver writes value minus one.
// <i>Default: 64
#ifndef AXI_LIMIT0_MAX_OUTSTANDING_READS
#define AXI_LIMIT0_MAX_OUTSTANDING_READS        64
#endif

// <o> Maximum outstanding writes <1..32>
// <i>Sets the AXI_LIMIT0 write transaction limit. The driver writes value minus one.
// <i>Default: 32
#ifndef AXI_LIMIT0_MAX_OUTSTANDING_WRITES
#define AXI_LIMIT0_MAX_OUTSTANDING_WRITES       32
#endif

// </h>
// <h>AXI_LIMIT1 - AXI0 Counter 1
// <i>Configures AXI transaction attributes and outstanding limits for traffic assigned to AXI0 counter 1.

// <o> Burst split alignment
//   <0=> 64 bytes
//   <1=> 128 bytes
//   <2=> 256 bytes
// <i>An AXI burst that would cross the selected aligned boundary is split into multiple bursts.
// <i>This limits the span of each burst to meet the boundary requirements of the AXI interconnect and memory system.
// <i>Default: 0
#ifndef AXI_LIMIT1_MAX_BEATS_BYTES
#define AXI_LIMIT1_MAX_BEATS_BYTES              0x0
#endif

// <o> Memory type
//   <0x0=> Device non-bufferable
//   <0x1=> Device bufferable
//   <0x2=> Normal non-cacheable non-bufferable
//   <0x3=> Normal non-cacheable bufferable
//   <0x4=> Write-through no allocate
//   <0x5=> Write-through read allocate
//   <0x6=> Write-through write allocate
//   <0x7=> Write-through read and write allocate
//   <0x8=> Write-back no allocate
//   <0x9=> Write-back read allocate
//   <0xA=> Write-back write allocate
//   <0xB=> Write-back read and write allocate
// <i>Sets the buffering and caching attributes requested for traffic assigned to AXI0 counter 1.
// <i>Default: 0x0
#ifndef AXI_LIMIT1_MEM_TYPE
#define AXI_LIMIT1_MEM_TYPE                     0x0
#endif

// <o> Maximum outstanding reads <1..64>
// <i>Sets the AXI_LIMIT1 read transaction limit. The driver writes value minus one.
// <i>Default: 64
#ifndef AXI_LIMIT1_MAX_OUTSTANDING_READS
#define AXI_LIMIT1_MAX_OUTSTANDING_READS        64
#endif

// <o> Maximum outstanding writes <1..32>
// <i>Sets the AXI_LIMIT1 write transaction limit. The driver writes value minus one.
// <i>Default: 32
#ifndef AXI_LIMIT1_MAX_OUTSTANDING_WRITES
#define AXI_LIMIT1_MAX_OUTSTANDING_WRITES       32
#endif

// </h>
// <h>AXI_LIMIT2 - AXI1 Counter 2
// <i>Configures AXI transaction attributes and outstanding limits for traffic assigned to AXI1 counter 2.

// <o> Burst split alignment
//   <0=> 64 bytes
//   <1=> 128 bytes
//   <2=> 256 bytes
// <i>An AXI burst that would cross the selected aligned boundary is split into multiple bursts.
// <i>This limits the span of each burst to meet the boundary requirements of the AXI interconnect and memory system.
// <i>Default: 0
#ifndef AXI_LIMIT2_MAX_BEATS_BYTES
#define AXI_LIMIT2_MAX_BEATS_BYTES              0x0
#endif

// <o> Memory type
//   <0x0=> Device non-bufferable
//   <0x1=> Device bufferable
//   <0x2=> Normal non-cacheable non-bufferable
//   <0x3=> Normal non-cacheable bufferable
//   <0x4=> Write-through no allocate
//   <0x5=> Write-through read allocate
//   <0x6=> Write-through write allocate
//   <0x7=> Write-through read and write allocate
//   <0x8=> Write-back no allocate
//   <0x9=> Write-back read allocate
//   <0xA=> Write-back write allocate
//   <0xB=> Write-back read and write allocate
// <i>Sets the buffering and caching attributes requested for traffic assigned to AXI1 counter 2.
// <i>Default: 0x0
#ifndef AXI_LIMIT2_MEM_TYPE
#define AXI_LIMIT2_MEM_TYPE                     0x0
#endif

// <o> Maximum outstanding reads <1..64>
// <i>Sets the AXI_LIMIT2 read transaction limit. The driver writes value minus one.
// <i>Default: 64
#ifndef AXI_LIMIT2_MAX_OUTSTANDING_READS
#define AXI_LIMIT2_MAX_OUTSTANDING_READS        64
#endif

// <o> Maximum outstanding writes <1..32>
// <i>Sets the AXI_LIMIT2 write transaction limit. The driver writes value minus one.
// <i>Default: 32
#ifndef AXI_LIMIT2_MAX_OUTSTANDING_WRITES
#define AXI_LIMIT2_MAX_OUTSTANDING_WRITES       32
#endif

// </h>
// <h>AXI_LIMIT3 - AXI1 Counter 3
// <i>Configures AXI transaction attributes and outstanding limits for traffic assigned to AXI1 counter 3.

// <o> Burst split alignment
//   <0=> 64 bytes
//   <1=> 128 bytes
//   <2=> 256 bytes
// <i>An AXI burst that would cross the selected aligned boundary is split into multiple bursts.
// <i>This limits the span of each burst to meet the boundary requirements of the AXI interconnect and memory system.
// <i>Default: 0
#ifndef AXI_LIMIT3_MAX_BEATS_BYTES
#define AXI_LIMIT3_MAX_BEATS_BYTES              0x0
#endif

// <o> Memory type
//   <0x0=> Device non-bufferable
//   <0x1=> Device bufferable
//   <0x2=> Normal non-cacheable non-bufferable
//   <0x3=> Normal non-cacheable bufferable
//   <0x4=> Write-through no allocate
//   <0x5=> Write-through read allocate
//   <0x6=> Write-through write allocate
//   <0x7=> Write-through read and write allocate
//   <0x8=> Write-back no allocate
//   <0x9=> Write-back read allocate
//   <0xA=> Write-back write allocate
//   <0xB=> Write-back read and write allocate
// <i>Sets the buffering and caching attributes requested for traffic assigned to AXI1 counter 3.
// <i>Default: 0x0
#ifndef AXI_LIMIT3_MEM_TYPE
#define AXI_LIMIT3_MEM_TYPE                     0x0
#endif

// <o> Maximum outstanding reads <1..64>
// <i>Sets the AXI_LIMIT3 read transaction limit. The driver writes value minus one.
// <i>Default: 64
#ifndef AXI_LIMIT3_MAX_OUTSTANDING_READS
#define AXI_LIMIT3_MAX_OUTSTANDING_READS        64
#endif

// <o> Maximum outstanding writes <1..32>
// <i>Sets the AXI_LIMIT3 write transaction limit. The driver writes value minus one.
// <i>Default: 32
#ifndef AXI_LIMIT3_MAX_OUTSTANDING_WRITES
#define AXI_LIMIT3_MAX_OUTSTANDING_WRITES       32
#endif

// </h>
// </h>

// <<< end of configuration section >>>

#endif /* ETHOSU_CONFIG_U65_H */
