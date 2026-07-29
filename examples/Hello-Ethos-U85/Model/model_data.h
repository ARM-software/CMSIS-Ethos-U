/*---------------------------------------------------------------------------
 * Copyright (c) 2026 Arm Limited (or its affiliates). All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 *      Name:    model_data.h
 *      Purpose: The Vela-compiled models and the shared tensor arena
 *
 * Only the Vela builds are linked into the firmware. The original int8
 * .tflite files stay in the tree next to them, so the models can be recompiled
 * for a different NPU configuration or memory mode.
 *---------------------------------------------------------------------------*/

#ifndef MODEL_DATA_H__
#define MODEL_DATA_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Vela command streams, placed in section "ethos_model". */
extern const uint8_t  hello_world_int8_vela_tflite[];
extern const uint32_t hello_world_int8_vela_tflite_len;

extern const uint8_t  tiny_cnn_int8_vela_tflite[];
extern const uint32_t tiny_cnn_int8_vela_tflite_len;

/* Tensor arena, placed in section "ethos_arena" (NPU-visible SRAM). */
extern uint8_t        g_tensor_arena[];
extern const uint32_t g_tensor_arena_size;

#ifdef __cplusplus
}
#endif

#endif /* MODEL_DATA_H__ */
