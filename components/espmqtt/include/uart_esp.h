/*
 * Uart_Esp.h
 *
 *  Created on: Dec 29, 2017
 *      Author: Duraisamy P
 */

#ifndef UART_ESP_H_
#define UART_ESP_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "soc/uart_struct.h"
#include "string.h"
#include "DUPS_Communication.h"
//#include "gatts_demo.h"
#include "gatts_table_creat_demo.h"

static const int RX_BUF_SIZE = 128;

//#define TXD_PIN (GPIO_NUM_10)
//#define RXD_PIN (GPIO_NUM_9)

#define TXD_PIN (GPIO_NUM_18)
#define RXD_PIN (GPIO_NUM_19)



#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_log.h"


static const char *RX_TASK_TAG = "uart_events";

/**
 * This  shows how to use the UART driver to handle special UART events.
 *
 * It also reads data from UART0 directly, and echoes it to console.
 *
 * - Port: UART0
 * - Receive (Rx) buffer: on
 * - Transmit (Tx) buffer: off
 * - Flow control: off
 * - Event queue: on
 * - Pin assignment: TxD (default), RxD (default)
 */

#define EX_UART_NUM UART_NUM_1

#define BUF_SIZE (129)
static QueueHandle_t uart0_queue;

static void uart_event_task(void *pvParameters)
{
	uart_event_t event;
	size_t buffered_size;
	uint8_t *dtmp = (uint8_t *) malloc(BUF_SIZE);
	while (1) {
		/* Waiting for UART event.
           If it happens then print out information what is it */
		if (xQueueReceive(uart0_queue, (void * )&event, (portTickType)portMAX_DELAY)) {
			   //ESP_LOGI(RX_TASK_TAG, "uart[%d] event:", EX_UART_NUM);
			switch (event.type) {
			case UART_DATA:
				/* Event of UART receiving data
				 * We'd better handler data event fast, there would be much more data events
				 * than other types of events.
				 * If we take too much time on data event, the queue might be full.
				 * In this example, we don't process data in event, but read data outside.
				 */
				uart_get_buffered_data_len(UART_NUM_1, &buffered_size);
				// ESP_LOGI(RX_TASK_TAG, "data, len:: %d; buffered len: %d",event.size, buffered_size);
				//uart_read_bytes(UART_NUM_1, dtmp, BUF_SIZE, 100*portTICK_RATE_MS); // 200 * for normal supply
				//uart_read_bytes(UART_NUM_1, dtmp, BUF_SIZE, 20/portTICK_RATE_MS); //20/portTICK_RATE_MS

				uart_read_bytes(UART_NUM_1, dtmp, event.size, portMAX_DELAY);
				//ESP_LOGI(RX_TASK_TAG, "data:%s\n",dtmp);
				if(event.size == 8 )//Check data byte length
				pr_hexbytes(dtmp);
				uart_flush_input(EX_UART_NUM);
				break;
				//Event of HW FIFO overflow detected
			case UART_FIFO_OVF:
				ESP_LOGI(RX_TASK_TAG, "hw fifo overflow");
				// If fifo overflow happened, you should consider adding flow control for your application.
				// The ISR has already reset the rx FIFO,
				// As an example, we directly flush the rx buffer here in order to read more data.
				uart_flush(UART_NUM_1);
				xQueueReset(uart0_queue);
				break;
				//Event of UART ring buffer full
			case UART_BUFFER_FULL:
				ESP_LOGI(RX_TASK_TAG, "ring buffer full");
				// If buffer full happened, you should consider encreasing your buffer size
				// As an example, we directly flush the rx buffer here in order to read more data.
				uart_flush(UART_NUM_1);
				xQueueReset(uart0_queue);
				break;
				//Event of UART RX break detected
			case UART_BREAK:
				ESP_LOGI(RX_TASK_TAG, "uart rx break");
				break;
				//Event of UART parity check error
			case UART_PARITY_ERR:
				ESP_LOGI(RX_TASK_TAG, "uart parity error");
				break;
				//Event of UART frame error
			case UART_FRAME_ERR:
				ESP_LOGI(RX_TASK_TAG, "uart frame error");
				break;
			default:
				ESP_LOGI(RX_TASK_TAG, "uart event type: %d", event.type);
				break;

			}
		}
		//vTaskDelay(1 / portTICK_PERIOD_MS);
	}
	free(dtmp);
	//Task delete and create new task for uart handler
	vTaskDelete(uart_event_task);
	xTaskCreate(uart_event_task, "uart_event_task", 2048, NULL, 12, NULL);

}

void init()
{
	// esp_log_level_set(TAG, ESP_LOG_INFO);

	/* Configure parameters of an UART driver,
	 * communication pins and install the driver */
	uart_config_t uart_config = {
			.baud_rate = 9600,
			.data_bits = UART_DATA_8_BITS,
			.parity = UART_PARITY_DISABLE,
			.stop_bits = UART_STOP_BITS_1,
			.flow_ctrl = UART_HW_FLOWCTRL_DISABLE
	};
	uart_param_config(EX_UART_NUM, &uart_config);
//
//	   uint32_t Bm;
//
//
//	uart_get_baudrate(EX_UART_NUM, &Bm);
//	printf("\n***********%d*********\n",Bm);
	// Set UART pins using UART0 default pins i.e. no changes
	uart_set_pin(EX_UART_NUM, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
	uart_driver_install(EX_UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 10, &uart0_queue, 0);
		// Set uart pattern detection function
//	uart_enable_pattern_det_intr(EX_UART_NUM, '+', 3, 10000, 10, 10);

	// Create a task to handle uart event from ISR
	//xTaskCreate(uart_event_task, "uart_event_task", 2048, NULL, 5, NULL);
	xTaskCreate(uart_event_task, "uart_event_task", 3048, NULL, 5, NULL);


}

void Init_Uart()
{
	init();
	//  xTaskCreate(rx_task, "uart_rx_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);

}

#endif /* UART_ESP_H */
