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

#include "reference_data.h"

/*
  tiny_cnn: one 16x16 diagonal-stripe image, quantized to int8 the same way.
  The four outputs are per-class logits; the largest is at index 2, which is the
  diagonal-stripe class.
*/
const int8_t tiny_cnn_input[] = {
   -128, -128,  110,  -96,  -83,  127, -108, -125,   83, -128, -128,  127, -128, -128,  127, -128,
   -109,   94, -128,  -89,  127, -119, -124,  122, -128, -107,  101, -128, -128,   98, -128, -114,
    127, -128, -126,   88, -128, -128,   93, -107,  -93,  119, -128, -128,  127, -128, -121,  117,
   -115, -128,  127, -118, -128,  127, -128, -128,   78, -117,  -88,  114, -128, -110,  109, -126,
    -91,  104,  -88, -128,  115, -128, -124,  127, -125, -128,  127, -109, -106,  127, -128, -128,
    127,  -90, -128,  127, -128, -128,  127, -124, -116,   98, -126, -100,  127, -118, -125,  112,
   -117, -118,  119, -128, -128,  127, -128, -128,  121, -128, -128,  107, -128, -111,   82, -102,
   -122,  121, -128, -128,  127, -128, -115,  117, -128, -122,  102, -113, -128,  123, -128, -127,
    127,  -85, -106,   89, -128, -128,   73, -103, -117,  127, -128, -128,   93, -128, -128,  127,
   -122, -102,  127,  -74, -128,  120, -128, -128,  125, -125, -128,  127, -128, -128,  127, -128,
   -128,  127, -108, -128,  127, -128, -127,  127, -128, -111,  127, -123,  -89,  123, -126, -125,
    119,  -41, -128,  105, -128, -128,  127, -107, -128,  127,  -97, -112,  127,  -98, -128,  120,
   -128,  -90,  127, -128, -125,  100, -128, -114,  112, -128, -103,   63,  -68, -116,  127,  -84,
   -111,  127, -128, -128,  127, -128, -126,  111, -111, -124,  119, -128, -103,  119,  -75, -128,
    127, -128, -102,   71, -112,  -99,  126, -123, -128,  127, -128,  -69,  113, -128, -125,  127,
   -128,  -93,  116, -106, -128,  112, -128, -128,  109, -128, -128,  127,  -98,  -75,  127, -128,
};
const int8_t tiny_cnn_expected[] = {
    -23,  -32,  123,  -92,
};

