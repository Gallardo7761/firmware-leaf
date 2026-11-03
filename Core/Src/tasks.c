/**
 * @file    tasks.c
 * @author  Jose
 * @brief   FreeRTOS tasks creation
 * @date    15/9/26
 * @project firmware-leaf
 */

#include "tasks.h"
#include "task.h"
#include "sht40.h"
#include "main.h"

extern SPI_HandleTypeDef hspi1;

QueueHandle_t xShtMailbox = NULL;
SemaphoreHandle_t xI2C2Mutex;
EventGroupHandle_t xSyncEventGroup = NULL;
TaskHandle_t sht40_task_handle;
TaskHandle_t spi_test_handle;

bool Tasks_Init(SHT40_Handle_t *sht_h)
{
    xShtMailbox = xQueueCreate(1, sizeof(SensorDataMsg_t));
    if (xShtMailbox == NULL) return false;

    xSyncEventGroup = xEventGroupCreate();
    if (xSyncEventGroup == NULL) return false;

    xI2C2Mutex = xSemaphoreCreateMutex();
    if (xI2C2Mutex == NULL) return false;

    BaseType_t ret;

    ret = xTaskCreate(
        SHT40_Measure_Task,
        "SHT40 Task",
        256,
        sht_h,
        2,
        &sht40_task_handle
    );
    if (ret != pdPASS) return false;

    ret = xTaskCreate(
        SPI_Test_Task,
        "SPI Test",
        256,
        NULL,
        1,
        &spi_test_handle
    );
    if (ret != pdPASS) return false;

    return true;
}

void SHT40_Measure_Task(void *pvParameters)
{
    SHT40_Handle_t *sht40_h = (SHT40_Handle_t *)pvParameters;
    if (sht40_h == NULL) vTaskDelete(NULL);

    float temp = 0.0f;
    float hum = 0.0f;
    SensorDataMsg_t msg;
    msg.type = SENSOR_TYPE_SHT40;

    for (;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        xSemaphoreTake(xI2C2Mutex, portMAX_DELAY);
        SHT40_Status_t st = SHT40_Read(sht40_h, &temp, &hum);
        xSemaphoreGive(xI2C2Mutex);

        if ( st == SHT40_OK) {
            msg.timestamp_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
            msg.data.sht40.temperature = temp;
            msg.data.sht40.humidity = hum;

            xQueueOverwrite(xShtMailbox, &msg);
        }

        xEventGroupSetBits(xSyncEventGroup, FLAG_SHT_DONE);
    }
}

#define RFM95_REG_VERSION   0x42
#define RFM95_VERSION_ID    0x12

volatile uint8_t spi_test_version = 0;
volatile uint32_t spi_test_ok = 0;
volatile uint32_t spi_test_fail = 0;

void SPI_Test_Task(void *pvParameters)
{
    (void)pvParameters;

    /* SX1276: bit 7 del address a 0 = lectura */
    uint8_t tx[2] = {RFM95_REG_VERSION & 0x7F, 0x00};
    uint8_t rx[2];

    for (;;) {
        rx[1] = 0;
        HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_RESET);
        HAL_StatusTypeDef st = HAL_SPI_TransmitReceive(&hspi1, tx, rx, sizeof(tx), 100);
        HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET);

        spi_test_version = rx[1];
        if (st == HAL_OK && rx[1] == RFM95_VERSION_ID) spi_test_ok++;
        else spi_test_fail++;

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}