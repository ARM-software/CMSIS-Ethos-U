/*
 * SPDX-FileCopyrightText: Copyright 2020, 2024, 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
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

#ifndef ETHOSU_CONFIG_U85_H
#define ETHOSU_CONFIG_U85_H

// <<< Use Configuration Wizard in Context Menu >>>

// <h>Ethos-U85 Configuration
// <i>Configure power ramping, memory attributes, memory routing, and AXI access limits.

// <h>MAC Power Ramping

// <o> MAC power ramp step interval <0..63>
// <i>Controls the interval between MAC unit steps during ramp-up and ramp-down.
// <i>The interval is 4 * NPU_MAC_PWR_RAMP_CYCLES NPU clock cycles.
// <i>Set to 0 to disable power ramping.
// <i>Default: 0
#ifndef NPU_MAC_PWR_RAMP_CYCLES
#define NPU_MAC_PWR_RAMP_CYCLES                 0
#endif

// </h>
// <h>Memory Attributes
// <i>Each MEM_ATTR entry selects a memory domain, AXI port, and memory type.

// <h>MEM_ATTR0
// <o.0..1> Memory domain
//   <0=> Non-shareable
//   <1=> Inner shareable
//   <2=> Outer shareable
//   <3=> System
// <o.2> AXI port
//   <0=> SRAM
//   <1=> EXT
// <o.4..7> Memory type
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
// <i>Configure memory attribute entry 0.
// <i>Default: 0x00
#ifndef NPU_MEM_ATTR_0
#define NPU_MEM_ATTR_0                          0x00
#endif
// </h>

// <h>MEM_ATTR1
// <o.0..1> Memory domain
//   <0=> Non-shareable
//   <1=> Inner shareable
//   <2=> Outer shareable
//   <3=> System
// <o.2> AXI port
//   <0=> SRAM
//   <1=> EXT
// <o.4..7> Memory type
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
// <i>Configure memory attribute entry 1.
// <i>Default: 0x00
#ifndef NPU_MEM_ATTR_1
#define NPU_MEM_ATTR_1                          0x00
#endif
// </h>

// <h>MEM_ATTR2
// <o.0..1> Memory domain
//   <0=> Non-shareable
//   <1=> Inner shareable
//   <2=> Outer shareable
//   <3=> System
// <o.2> AXI port
//   <0=> SRAM
//   <1=> EXT
// <o.4..7> Memory type
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
// <i>Configure memory attribute entry 2.
// <i>Default: 0x04
#ifndef NPU_MEM_ATTR_2
#define NPU_MEM_ATTR_2                          0x04
#endif
// </h>

// <h>MEM_ATTR3
// <o.0..1> Memory domain
//   <0=> Non-shareable
//   <1=> Inner shareable
//   <2=> Outer shareable
//   <3=> System
// <o.2> AXI port
//   <0=> SRAM
//   <1=> EXT
// <o.4..7> Memory type
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
// <i>Configure memory attribute entry 3.
// <i>Default: 0x04
#ifndef NPU_MEM_ATTR_3
#define NPU_MEM_ATTR_3                          0x04
#endif
// </h>

// </h>
// <h>Memory Routing
// <i>Select which MEM_ATTR entry applies to the command stream and each base pointer region.

// <o> Command stream memory attribute
//   <0=> MEM_ATTR0
//   <1=> MEM_ATTR1
//   <2=> MEM_ATTR2
//   <3=> MEM_ATTR3
// <i>Selects the MEM_ATTR entry for command stream fetches.
// <i>Default: 2
#ifndef NPU_QCONFIG
#define NPU_QCONFIG                             2
#endif

// <o> Base pointer region 0 memory attribute
//   <0=> MEM_ATTR0
//   <1=> MEM_ATTR1
//   <2=> MEM_ATTR2
//   <3=> MEM_ATTR3
// <i>Selects the MEM_ATTR entry for BASEP0 accesses.
// <i>Default: 3
#ifndef NPU_REGIONCFG_0
#define NPU_REGIONCFG_0                         3
#endif

// <o> Base pointer region 1 memory attribute
//   <0=> MEM_ATTR0
//   <1=> MEM_ATTR1
//   <2=> MEM_ATTR2
//   <3=> MEM_ATTR3
// <i>Selects the MEM_ATTR entry for BASEP1 accesses.
// <i>Default: 0
#ifndef NPU_REGIONCFG_1
#define NPU_REGIONCFG_1                         0
#endif

// <o> Base pointer region 2 memory attribute
//   <0=> MEM_ATTR0
//   <1=> MEM_ATTR1
//   <2=> MEM_ATTR2
//   <3=> MEM_ATTR3
// <i>Selects the MEM_ATTR entry for BASEP2 accesses.
// <i>Default: 1
#ifndef NPU_REGIONCFG_2
#define NPU_REGIONCFG_2                         1
#endif

// <o> Base pointer region 3 memory attribute
//   <0=> MEM_ATTR0
//   <1=> MEM_ATTR1
//   <2=> MEM_ATTR2
//   <3=> MEM_ATTR3
// <i>Selects the MEM_ATTR entry for BASEP3 accesses.
// <i>Default: 1
#ifndef NPU_REGIONCFG_3
#define NPU_REGIONCFG_3                         1
#endif

// <o> Base pointer region 4 memory attribute
//   <0=> MEM_ATTR0
//   <1=> MEM_ATTR1
//   <2=> MEM_ATTR2
//   <3=> MEM_ATTR3
// <i>Selects the MEM_ATTR entry for BASEP4 accesses.
// <i>Default: 1
#ifndef NPU_REGIONCFG_4
#define NPU_REGIONCFG_4                         1
#endif

// <o> Base pointer region 5 memory attribute
//   <0=> MEM_ATTR0
//   <1=> MEM_ATTR1
//   <2=> MEM_ATTR2
//   <3=> MEM_ATTR3
// <i>Selects the MEM_ATTR entry for BASEP5 accesses.
// <i>Default: 1
#ifndef NPU_REGIONCFG_5
#define NPU_REGIONCFG_5                         1
#endif

// <o> Base pointer region 6 memory attribute
//   <0=> MEM_ATTR0
//   <1=> MEM_ATTR1
//   <2=> MEM_ATTR2
//   <3=> MEM_ATTR3
// <i>Selects the MEM_ATTR entry for BASEP6 accesses.
// <i>Default: 1
#ifndef NPU_REGIONCFG_6
#define NPU_REGIONCFG_6                         1
#endif

// <o> Base pointer region 7 memory attribute
//   <0=> MEM_ATTR0
//   <1=> MEM_ATTR1
//   <2=> MEM_ATTR2
//   <3=> MEM_ATTR3
// <i>Selects the MEM_ATTR entry for BASEP7 accesses.
// <i>Default: 1
#ifndef NPU_REGIONCFG_7
#define NPU_REGIONCFG_7                         1
#endif

// </h>

// <h>SRAM AXI Limits
// <i>Configure the limits applied separately to each SRAM AXI port.

// <o> Maximum outstanding reads per SRAM port <1..12>
// <i>The driver writes value minus one to the register field.
// <i>Default: 12
#ifndef AXI_LIMIT_SRAM_MAX_OUTSTANDING_READ_M1
#define AXI_LIMIT_SRAM_MAX_OUTSTANDING_READ_M1  12
#endif

// <o> Maximum outstanding writes per SRAM port <1..16>
// <i>The driver writes value minus one to the register field.
// <i>Default: 16
#ifndef AXI_LIMIT_SRAM_MAX_OUTSTANDING_WRITE_M1
#define AXI_LIMIT_SRAM_MAX_OUTSTANDING_WRITE_M1 16
#endif

// <o> Burst split alignment
//   <0=> 64 bytes
//   <1=> 128 bytes
//   <2=> 256 bytes
// <i>An AXI burst that would cross the selected aligned boundary is split into multiple bursts.
// <i>The configured hardware cap can impose a smaller boundary.
// <i>Default: 2
#ifndef AXI_LIMIT_SRAM_MAX_BEATS
#define AXI_LIMIT_SRAM_MAX_BEATS                2
#endif

// </h>
// <h>EXT AXI Limits
// <i>Configure the limits applied separately to each EXT AXI port.

// <o> Maximum outstanding reads per EXT port <1..64>
// <i>The driver writes value minus one to the register field.
// <i>U85-128 and U85-256 hardware configurations cap the effective limit at 32.
// <i>Default: 64
#ifndef AXI_LIMIT_EXT_MAX_OUTSTANDING_READ_M1
#define AXI_LIMIT_EXT_MAX_OUTSTANDING_READ_M1   64
#endif

// <o> Maximum outstanding writes per EXT port <1..32>
// <i>The driver writes value minus one to the register field.
// <i>Default: 32
#ifndef AXI_LIMIT_EXT_MAX_OUTSTANDING_WRITE_M1
#define AXI_LIMIT_EXT_MAX_OUTSTANDING_WRITE_M1  32
#endif

// <o> Burst split alignment
//   <0=> 64 bytes
//   <1=> 128 bytes
//   <2=> 256 bytes
// <i>An AXI burst that would cross the selected aligned boundary is split into multiple bursts.
// <i>The configured hardware cap can impose a smaller boundary.
// <i>Default: 2
#ifndef AXI_LIMIT_EXT_MAX_BEATS
#define AXI_LIMIT_EXT_MAX_BEATS                 2
#endif

// </h>
// </h>

// <<< end of configuration section >>>

#endif /* ETHOSU_CONFIG_U85_H */
