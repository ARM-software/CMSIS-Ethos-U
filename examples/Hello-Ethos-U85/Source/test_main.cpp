/*---------------------------------------------------------------------------
 * Copyright (c) 2026 Arm Limited (or its affiliates). All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 *      Name:    test_main.cpp
 *      Purpose: Run each model on the Ethos-U85 and check its output
 *
 * Both models are Vela-compiled, so each is a single "ethos-u" custom operator
 * that the NPU executes. One golden input/output pair per model is embedded
 * below; the expected values come from the host TensorFlow reference
 * interpreter, so they are independent of the NPU being tested.
 *
 * Regenerate the models and these vectors with Model/gen/generate.py, which
 * prints a ready-to-paste copy of the arrays below.
 *---------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>

#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/cortex_m_generic/debug_log_callback.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "model_data.h"
#include "cmsis_os2.h"
#include "main.h"

/*
  hello_world. These are int8 *quantized* values, not the sine itself. Each
  tensor carries a scale and a zero point, and the real value is
  (quantized - zero_point) * scale:

    input   x = (-55 - (-128)) * 0.024574 = 1.794 rad
    output  y = (124 -     3)  * 0.008061 = 0.975     sin(1.794) = 0.975

  Model/gen/generate.py reads those scales out of the model and applies them
  when it captures the vector, so nothing here has to convert at runtime.
*/
static const int8_t hello_world_input[] = {
    -55,
};
static const int8_t hello_world_expected[] = {
    124,
};

/*
  tiny_cnn: one 16x16 diagonal-stripe image, quantized to int8 the same way.
  The four outputs are per-class logits; the largest is at index 2, which is the
  diagonal-stripe class.
*/
static const int8_t tiny_cnn_input[] = {
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
static const int8_t tiny_cnn_expected[] = {
    -23,  -32,  123,  -92,
};

static void TflmDebugLog(const char *s) {
  printf("%s", s);
}

static int checks_run = 0;
static int checks_failed = 0;

static void check(const char *name, bool ok, const char *detail) {
  checks_run++;
  if (!ok) {
    checks_failed++;
  }
  printf("[%s] %s (%s)\n", ok ? "PASS" : "FAIL", name, detail);
}

static void RunModel(const char *name,
                     const uint8_t *model_data,
                     const int8_t *input,  size_t input_len,
                     const int8_t *expected, size_t output_len) {

  /* A fully offloaded Vela model is one operator: the "ethos-u" custom op. If
     Vela had left anything on the CPU, AllocateTensors() would fail here. */
  tflite::MicroMutableOpResolver<1> resolver;
  resolver.AddEthosU();

  tflite::MicroInterpreter interpreter(tflite::GetModel(model_data), resolver,
                                       g_tensor_arena, g_tensor_arena_size);

  if (interpreter.AllocateTensors() != kTfLiteOk) {
    check(name, false, "AllocateTensors failed");
    return;
  }

  memcpy(interpreter.input(0)->data.int8, input, input_len);

  if (interpreter.Invoke() != kTfLiteOk) {
    check(name, false, "Invoke failed");
    return;
  }

  const int8_t *output = interpreter.output(0)->data.int8;
  int worst = 0;
  for (size_t i = 0; i < output_len; i++) {
    int delta = output[i] - expected[i];
    if (delta < 0) {
      delta = -delta;
    }
    if (delta > worst) {
      worst = delta;
    }
  }

  char detail[48];
  snprintf(detail, sizeof(detail), "max delta %d LSB", worst);
  check(name, worst == 0, detail);
}

void app_main_thread(void *arg) {

  RegisterDebugLogCallback(TflmDebugLog);

  printf("\nEthos-U85 / Corstone-320 TFLM integration test\n");

  RunModel("hello_world", hello_world_int8_vela_tflite,
           hello_world_input, sizeof(hello_world_input),
           hello_world_expected, sizeof(hello_world_expected));

  RunModel("tiny_cnn", tiny_cnn_int8_vela_tflite,
           tiny_cnn_input, sizeof(tiny_cnn_input),
           tiny_cnn_expected, sizeof(tiny_cnn_expected));

  printf("\n%d of %d checks passed\n", checks_run - checks_failed, checks_run);
  printf("TEST RESULT: %s\n", (checks_failed == 0) ? "PASS" : "FAIL");

  /* EOT stops the simulation */
  printf("\x04\n");

  while(1);
}

/* Application initialization */
int app_main (void) {
  const osThreadAttr_t attr = {
    .stack_size = 4096,
  };

  /* Initialize CMSIS-RTOS2, create application thread and start the kernel */
  osKernelInitialize();
  osThreadNew(app_main_thread, NULL, &attr);
  osKernelStart();
  return 0;
}
