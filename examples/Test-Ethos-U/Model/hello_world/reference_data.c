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
  hello_world. These are int8 *quantized* values, not the sine itself. Each
  tensor carries a scale and a zero point, and the real value is
  (quantized - zero_point) * scale:

    input   x = (-55 - (-128)) * 0.024574 = 1.794 rad
    output  y = (124 -     3)  * 0.008061 = 0.975     sin(1.794) = 0.975

  Model/hello_world/gen/generate.py reads those scales out of the model and
  applies them when it captures the vector, so nothing here has to convert at
  runtime.
*/
const int8_t hello_world_input[] = {
    -55,
};
const int8_t hello_world_expected[] = {
    124,
};

