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

#include "ethosu_driver.h"
#include "FreeRTOS.h"
#include "semphr.h"

/* Maximum number of Ethos-U instances.                        */
/* Used as maximum semaphore count in ethosu_semaphore_create. */
#ifndef ETHOSU_MAX_INSTANCES
#define ETHOSU_MAX_INSTANCES 2
#endif

/**
  \brief Create a mutex object.
  \return Pointer to a mutex object.
*/
void *ethosu_mutex_create(void) {
  SemaphoreHandle_t hMutex = xSemaphoreCreateMutex();
  return (void *)hMutex;
}

/**
  \brief Destroy a mutex object.
  \param[in]  mutex  mutex object.
*/
void ethosu_mutex_destroy(void *mutex) {
  SemaphoreHandle_t hMutex = (SemaphoreHandle_t)mutex;
  vSemaphoreDelete (hMutex);
}

/**
  \brief Lock a mutex.
  \param[in] mutex  mutex object.
  \return 0 on success, else negative error codes
*/
int ethosu_mutex_lock(void *mutex) {
  SemaphoreHandle_t hMutex = (SemaphoreHandle_t)mutex;
  BaseType_t status;
  int rval;

  status = xSemaphoreTake(hMutex, portMAX_DELAY);
  if (status == pdTRUE) {
    rval = 0;
  } else {
    rval = -1;
  }
  return rval;
}

/**
  \brief Unlock a mutex.
  \param[in] mutex  mutex object.
  \return 0 on success, else negative error codes
*/
int ethosu_mutex_unlock(void *mutex) {
  SemaphoreHandle_t hMutex = (SemaphoreHandle_t)mutex;
  BaseType_t status;
  int rval;

  status = xSemaphoreGive(hMutex);
  if (status == pdTRUE) {
    rval = 0;
  } else {
    rval = -1;
  }
  return rval;
}

/**
  \brief Create a semaphore object.
  \return Pointer to a semaphore object.
*/
void *ethosu_semaphore_create(void) {
  SemaphoreHandle_t hSemaphore;

  hSemaphore = xSemaphoreCreateCounting(ETHOSU_MAX_INSTANCES, 0);
  return (void *)hSemaphore;
}


/**
  \brief Destroy a semaphore object.
  \param[in]  sem  semaphore object.
*/
void ethosu_semaphore_destroy(void *sem) {
  SemaphoreHandle_t hSemaphore = (SemaphoreHandle_t)sem;
  vSemaphoreDelete(hSemaphore);
}

/**
  \brief Take a semaphore object with timeout.
  \param[in]  sem      semaphore object.
  \param[in]  timeout  timeout value in ticks.
  \return 0 on success, else negative error codes
*/
int ethosu_semaphore_take(void *sem, uint64_t timeout) {
  SemaphoreHandle_t hSemaphore = (SemaphoreHandle_t)sem;
  BaseType_t status;
  int rval;

  status = xSemaphoreTake(hSemaphore, timeout);
  if (status == pdTRUE) {
    rval = 0;
  } else {
    rval = -1;
  }
  return rval;
}

/**
  \brief Give a semaphore object.
  \param[in]  sem  semaphore object.
  \return 0 on success, else negative error codes
*/
int ethosu_semaphore_give(void *sem) {
  SemaphoreHandle_t hSemaphore = (SemaphoreHandle_t)sem;
  BaseType_t status;
  BaseType_t yield;
  int rval;

  if (xPortIsInsideInterrupt()) {
    yield = pdFALSE;

    if (xSemaphoreGiveFromISR (hSemaphore, &yield) != pdTRUE) {
      rval = -1;
    } else {
      portYIELD_FROM_ISR (yield);
      rval = 0;
    }
  }
  else {
    status = xSemaphoreGive(hSemaphore);
    if (status == pdTRUE) {
      rval = 0;
    } else {
      rval = -1;
    }
  }
  return rval;
}
