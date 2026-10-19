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

#include "main.h"
#include "cmsis_os2.h"

#include "model_test.h"

#if defined(ETHOSU55)
#define ETHOS_U_VARIANT "Ethos-U55"
#elif defined(ETHOSU65)
#define ETHOS_U_VARIANT "Ethos-U65"
#elif defined(ETHOSU85)
#define ETHOS_U_VARIANT "Ethos-U85"
#else
#error "Unsupported Ethos-U variant"
#endif

/* Selection flags are supplied by ML-Model layer. */
#if defined(MODEL_HELLO_WORLD)
#include "../Model/hello_world/reference_data.h"
extern const uint8_t hello_world_int8_vela_tflite[];

const ModelData_Int8 Model_Hello_World = {
  "hello_world",
  hello_world_int8_vela_tflite,
  hello_world_input,
  sizeof(hello_world_input),
  hello_world_expected,
  sizeof(hello_world_expected),
  0,
  false
};
#endif

#if defined(MODEL_TINY_CNN)
#include "../Model/tiny_cnn/reference_data.h"
extern const uint8_t tiny_cnn_int8_vela_tflite[];

const ModelData_Int8 Model_Tiny_Cnn = {
  "tiny_cnn",
  tiny_cnn_int8_vela_tflite,
  tiny_cnn_input,
  sizeof(tiny_cnn_input),
  tiny_cnn_expected,
  sizeof(tiny_cnn_expected),
  0,
  true
};
#endif

/* Application main thread */
void app_main_thread(void *arg) {

  printf("\n%s LiteRT (TFLu) integration test\n\n", ETHOS_U_VARIANT);

  TestInitialize();

  /* Test INT8 models using the selected Ethos-U variant */
#if defined(MODEL_HELLO_WORLD)
  TestModel_Int8(Model_Hello_World);
#endif

#if defined(MODEL_TINY_CNN)
  TestModel_Int8(Model_Tiny_Cnn);
#endif

  TestSummary();

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
