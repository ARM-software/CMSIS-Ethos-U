/*---------------------------------------------------------------------------
 * Copyright (c) 2026 Arm Limited (or its affiliates). All rights reserved.
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
 *
 *      Name:    arena.c
 *      Purpose: TensorFlow Lite Micro tensor arena
 *
 *---------------------------------------------------------------------------*/

#include "model_data.h"

/*
  Tensor arena.

  Placed in section "ethos_arena", which the board linker scripts map to RAM1
  (SRAM_VM0_S @ 0x31000000). That region is reachable by the NPU, which is a
  requirement: the Ethos-U reads and writes activations here directly.

  16-byte aligned for the NPU's access requirements.

  Sized well above what the shipped models need -- Vela reports 5.00 KiB of SRAM
  for tiny_cnn and 0.03 KiB for hello_world (see the VELA_SUMMARY.md files) --
  because TFLM additionally places tensor metadata and per-op scratch here. One
  arena is reused by every interpreter in turn; they are never live together.
*/
#ifndef TENSOR_ARENA_SIZE
#define TENSOR_ARENA_SIZE   (64U * 1024U)
#endif

uint8_t g_tensor_arena[TENSOR_ARENA_SIZE]
  __attribute__((aligned(16), section("ethos_arena")));

const uint32_t g_tensor_arena_size = TENSOR_ARENA_SIZE;
