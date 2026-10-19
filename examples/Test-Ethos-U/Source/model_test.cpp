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

#include <stdio.h>
#include <string.h>

#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "tensorflow/lite/micro/cortex_m_generic/debug_log_callback.h"

#include "model_test.h"

/*
  Tensor arena.

  Placed in section "ethos_arena". The selected board linker script must map
  this section to memory that is directly reachable by both the CPU and NPU.

  16-byte aligned for the NPU's access requirements.

  TFLM additionally places tensor metadata and per-op scratch here. One arena
  is reused by every interpreter in turn; they are never live together.
*/
#ifndef TENSOR_ARENA_SIZE
#define TENSOR_ARENA_SIZE   (64U * 1024U)
#endif

static uint8_t g_tensor_arena[TENSOR_ARENA_SIZE] __attribute__((aligned(16), section("ethos_arena")));

static int checks_run;
static int checks_failed;

/**
  \brief Validate quantized model output against its reference values.

  Each absolute difference must be at most ModelData::allowed_delta, measured
  in quantized steps. If ModelData::check_class is true, the indices of the
  largest actual and expected values must also match. Ties use the first index.

  \param[in] model Model descriptor with expected output values and comparison
                   settings. Its expected buffer must contain output_size values.
  \param[in] output Actual int8 output buffer containing output_size values.
  \param[in] output_size Number of output values (bytes); must be nonzero and
                         equal to ModelData::output_size.
  \param[out] max_delta Optional destination for the largest absolute difference,
                        including when comparison fails. May be NULL. Receives
                        -1 if arguments are invalid.
  \retval true All values satisfy the tolerance and any required class check.
  \retval false Comparison failed, model or output is NULL, the expected buffer
                is NULL, output_size is zero or mismatched, or allowed_delta
                is negative.
*/
static bool model_data_check_output(const ModelData_Int8 *model, const int8_t *output, size_t output_size, int *max_delta) {

  if (max_delta != NULL) {
    *max_delta = -1;
  }

  if (model == NULL || output == NULL || model->expected == NULL ||
      output_size == 0 || output_size != model->output_size ||
      model->allowed_delta < 0) {
    return false;
  }

  int worst = 0;
  size_t predicted_class = 0;
  size_t expected_class  = 0;

  for (size_t i = 0; i < output_size; ++i) {
    int delta = (int)output[i] - (int)model->expected[i];
    if (delta < 0) delta = -delta;
    if (delta > worst) worst = delta;
    if (output[i] > output[predicted_class]) predicted_class = i;
    if (model->expected[i] > model->expected[expected_class]) expected_class = i;
  }

  if (max_delta != NULL) {
    *max_delta = worst;
  }

  return worst <= model->allowed_delta && (!model->check_class || predicted_class == expected_class);
}

/* Print the test result */
static bool result(const char *model_name, bool ok, const char *detail) {

  checks_run++;
  if (!ok) {
    checks_failed++;
  }

  printf("[%s] %s (%s)\n", ok ? "PASS" : "FAIL", model_name, detail);
  return ok;
}

/* Debug log callback for TensorFlow Lite Micro */
static void TflmDebugLog(const char *s) {
  printf("%s", s);
}

/* Initialize the test variables */
void TestInitialize(void) {
  checks_run = 0;
  checks_failed = 0;

  RegisterDebugLogCallback(TflmDebugLog);
}

/* Print a summary of the test results */
void TestSummary(void) {
  printf("\n%d of %d checks passed\n", checks_run - checks_failed, checks_run);
  printf("TEST RESULT: %s\n", (checks_run > 0 && checks_failed == 0) ? "PASS" : "FAIL");

    /* EOT stops the simulation or pyOCD */
  printf("\x04\n");
}

bool TestModel_Int8(const ModelData_Int8 &model) {
  int max_delta;
  char detail[64];

  if (model.model    == NULL ||
      model.input    == NULL ||
      model.expected == NULL ||
      model.input_size == 0  ||
      model.output_size == 0) {
    return result(model.name, false, "missing model or reference data");
  }

  /* Read the embedded FlatBuffer using the schema supported by this runtime */
  const tflite::Model *graph = tflite::GetModel(model.model);

  /* Verify that the model schema version is supported */
  if (graph->version() != TFLITE_SCHEMA_VERSION) {
    return result(model.name, false, "unsupported model schema");
  }

  /* The generic runner supports a single fully offloaded inference graph */
  if (graph->subgraphs() == NULL || graph->subgraphs()->size() != 1) {
    return result(model.name, false, "expected one fully offloaded subgraph");
  }

  /* Extract the operators and their corresponding codes from the subgraph */
  const auto *ops   = graph->subgraphs()->Get(0)->operators();
  const auto *codes = graph->operator_codes();

  if (ops == NULL || ops->size() == 0 || codes == NULL) {
    /* No operators found in the subgraph */
    return result(model.name, false, "no Ethos-U operators");
  }

  /* Every graph operator must dispatch a Vela command stream to Ethos-U */
  for (const auto *op : *ops) {
    if (op->opcode_index() >= codes->size()) {
      /* The operator index is out of bounds for the available operator codes */
      return result(model.name, false, "invalid operator index");
    }
    const auto *code = codes->Get(op->opcode_index());
    if ((code->builtin_code() != tflite::BuiltinOperator_CUSTOM) ||
        (code->custom_code() == NULL)                            ||
        (strcmp(code->custom_code()->c_str(), "ethos-u") != 0)) {
      return result(model.name, false, "model is not fully offloaded to Ethos-U");
    }
  }

  /* Register only the NPU operator: no CPU fallback can satisfy this test */
  tflite::MicroMutableOpResolver<1> resolver;
  if (resolver.AddEthosU() != kTfLiteOk) {
    /* Failed to register the Ethos-U operator with the resolver */
    return result(model.name, false, "Ethos-U registration failed");
  }

  /* Allocate tensors (reused by sequential model tests) */
  tflite::MicroInterpreter interpreter(graph, resolver, g_tensor_arena, sizeof(g_tensor_arena));

  if (interpreter.AllocateTensors() != kTfLiteOk) {
    /* Failed to allocate tensors for the interpreter */
    return result(model.name, false, "AllocateTensors failed");
  }

  /* Match the model's tensors to the single-input/single-output int8 test data */
  if (interpreter.inputs_size() != 1 || interpreter.outputs_size() != 1) {
    return result(model.name, false, "expected one input and one output");
  }

  TfLiteTensor *input  = interpreter.input(0);
  TfLiteTensor *output = interpreter.output(0);

  if ((input == NULL) || (output == NULL)) {
    /* One or both of the input/output tensors are NULL */
    return result(model.name, false, "input or output tensor is NULL");
  }

  if ((input->type != kTfLiteInt8) || (input->bytes != model.input_size)) {
    /* The input tensor does not match the expected type or size */
    return result(model.name, false, "unexpected input tensor type or size");
  }

  if ((output->type != kTfLiteInt8) || (output->bytes != model.output_size)) {
    /* The output tensor does not match the expected type or size */
    return result(model.name, false, "unexpected output tensor type or size");
  }

  /* Load the reference input and execute the NPU inference */
  memcpy(input->data.int8, model.input, model.input_size);

  if (interpreter.Invoke() != kTfLiteOk) {
    /* The inference failed */
    return result(model.name, false, "Invoke failed");
  }

  /* Apply the model's output tolerance and optional winning-class check */
  bool ok = model_data_check_output(&model, output->data.int8, output->bytes, &max_delta);

  /* Report the maximum delta observed during Ethos-U inference */
  snprintf(detail, sizeof(detail), "Ethos-U inference, max delta %d LSB", max_delta);

  return result(model.name, ok, detail);
}

