/**
 * @file    tasks.c
 * @author  Jose
 * @brief   FreeRTOS tasks creation
 * @date    15/9/26
 * @project firmware-leaf
 */

#ifndef TASKS_H
#define TASKS_H

#include <stdint.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "event_groups.h"
#include "queue.h"
#include "semphr.h"
#include "sht40.h"

#define FLAG_SHT_DONE   (1 << 1)
#define ALL_SYNC_FLAGS  (FLAG_SHT_DONE)

typedef enum {
 SENSOR_TYPE_SHT40 = 0
} SensorType_t;

typedef struct {
 SensorType_t type;
 uint32_t timestamp_ms;
 union {
  struct {
   float temperature;
   float humidity;
  } sht40;
 } data;
} SensorDataMsg_t;

extern QueueHandle_t xShtMailbox;
extern SemaphoreHandle_t xI2C2Mutex;
extern EventGroupHandle_t xSyncEventGroup;
extern TaskHandle_t sht40_task_handle;
extern TaskHandle_t spi_test_handle;

void SHT40_Measure_Task(void *pvParameters);
void SPI_Test_Task(void *pvParameters);

bool Tasks_Init(SHT40_Handle_t *sht_h);

#endif //TASKS_H