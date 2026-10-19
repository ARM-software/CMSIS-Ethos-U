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

#ifndef MODEL_TEST_H
#define MODEL_TEST_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
  \brief Model and reference data for one single-input/single-output INT8 test.
  \details
   Tensor buffers contain flattened, already quantized int8 values. Tensor
   shapes and quantization parameters are read from the embedded model.
   Buffer sizes are in bytes, equal to the number of int8 elements.
   All pointers must be non-NULL and remain valid for the duration of the test;
   the descriptor does not own or modify the referenced data.
 */
typedef struct {
  const char *name;         /**< Null-terminated model name used in test results. */
  const uint8_t *model;     /**< Valid Vela-compiled TFLite model for the target NPU. */
  const int8_t *input;      /**< Reference input buffer containing input_size bytes. */
  size_t input_size;        /**< Nonzero input size; must match the input tensor's byte size. */
  const int8_t *expected;   /**< Expected output buffer containing output_size bytes. */
  size_t output_size;       /**< Nonzero expected-output size; must match the output tensor's byte size. */
  int allowed_delta;        /**< Nonnegative maximum absolute error per output value in quantized steps; zero requires an exact match. */
  bool check_class;         /**< Also require matching actual/expected argmax indices; ties select the first index. */
} ModelData_Int8;

#ifdef __cplusplus
}

/* Initialize the test framework */
void TestInitialize(void);

/**
  \brief Run and validate one fully offloaded Ethos-U INT8 model test.
  \details
  Requires one nonempty subgraph containing only ethos-u operators, with one
  int8 input tensor and one int8 output tensor matching the reference sizes.
  Allocates tensors in the shared arena, copies the input, invokes inference,
  and compares every output value using ModelData_Int8::allowed_delta.
  If ModelData_Int8::check_class is true, the argmax indices must also match.

  Prints one PASS/FAIL result and updates the test counters. Successful
  inference reports the maximum absolute output difference in quantized steps.

  \param[in] model Test descriptor containing a valid Vela-compiled model,
                   name, input and expected-output buffers, and comparison settings.
  \retval true Inference succeeded and all output checks passed.
  \retval false A descriptor, model, tensor, allocation, inference, or output
               check failed; the printed result identifies the failure.
 */
bool TestModel_Int8(const ModelData_Int8 &model);

/* Print a summary of all tests run so far */
void TestSummary(void);
#endif

#endif /* MODEL_TEST_H */
