/*
 * Copyright 2026 Arm Limited and/or its affiliates.
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

#ifndef TINY_CNN_REFERENCE_DATA_H
#define TINY_CNN_REFERENCE_DATA_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const int8_t tiny_cnn_input[256];
extern const int8_t tiny_cnn_expected[4];

#ifdef __cplusplus
}
#endif

#endif
