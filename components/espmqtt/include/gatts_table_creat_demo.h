
/** @file gatts_table_creat_demo.h
 *  @brief This file is for gatt server. It can send adv data, be connected by client.
 *  @author Duraisamy Pandurangan (V-Guard Industrial Ltd,R&D)
 *  @modified by Binu Antony (V-Guard Industrial Ltd,R&D)
 *  @modifed on:17-11-20
 */

#ifndef _GATT_SERVER_SERVICE_TABLE_H_
#define _GATT_SERVER_SERVICE_TABLE_H_
/*Header Include*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"
#include "uart_esp.h"
#include "driver/gpio.h"
#include "esp_sleep.h"
/*Pre-Processor Define*/
#define BLE_LED 22
#define GATTS_TABLE_TAG "GATTS_TABLE_DEMO" /*!<Hold the Debug Console Tag Message*/
#define PROFILE_NUM 			        1
#define PROFILE_APP_IDX 			    0
#define ESP_APP_ID			            0x55
#define SAMPLE_DEVICE_NAME              "VG_SMART_BT_WF_1" /*!<Ble Advertisement Name*/
#define SVC_INST_ID	    	            0
#define GATTS_DEMO_CHAR_VAL_LEN_MAX		30
#define PREPARE_BUF_MAX_SIZE 30
#define CHAR_DECLARATION_SIZE   (sizeof(uint8_t))
#define ADV_CONFIG_FLAG      (1 << 0)
#define SCAN_RSP_CONFIG_FLAG (1 << 1)
static uint8_t adv_config_done = 0;


/*Variable Declaration*/
bool reboot=false/*!<Holds the system reboot call*/,WriteRequest=false/*!<Hold the File write status*/;

uint16_t spp_conn_id/*!<*Hold the Ble Connection ID*/,transid/*!<Hold the BLe transmits id*/;
uint8_t volatile ble_connect=0;/*!<Hold Ble connection status*/
char tx_buff[8];/*!<Holds Gatt Transmit buffer data*/
uint8_t SSID_count=0,PWD_count=0;
char Reconfig_Flag=0;
/* Service */
static const uint16_t GATTS_SERVICE_UUID_TEST    = 0x00FF;
static const uint16_t GATTS_CHAR_UUID_TEST_A     = 0xFF01;
static const uint16_t GATTS_CHAR_UUID_TEST_B     = 0xFF02;
static const uint16_t GATTS_CHAR_UUID_TEST_C     = 0xFF03;

static const uint16_t primary_service_uuid         = ESP_GATT_UUID_PRI_SERVICE;
static const uint16_t character_declaration_uuid   = ESP_GATT_UUID_CHAR_DECLARE;
static const uint16_t character_client_config_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
static const uint8_t char_prop_read                =  ESP_GATT_CHAR_PROP_BIT_READ;
static const uint8_t char_prop_write               = ESP_GATT_CHAR_PROP_BIT_WRITE;
static const uint8_t char_prop_read_notify			= ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY;
static const uint8_t char_prop_read_write_notify   = ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY;
static const uint8_t heart_measurement_ccc[2] = {0x00, 0x00};
static const uint8_t char_value[4] = {0x11, 0x22, 0x33, 0x44};

/*Function Declaration*/
extern uint8_t Write_systemConf();
extern uint8_t Write_Inverter();

esp_gatt_rsp_t rsp;/*!<Hold the ble gatts response struct*/
esp_gatt_if_t spp_gatts_if = 0xff;

/**
 * Enumeration of  GATTS Table
 */
enum GATTS_TABLE{
	IDX_SVC,
	IDX_CHAR_A,
	IDX_CHAR_VAL_A,
	IDX_CHAR_CFG_A,

	IDX_CHAR_B,
	IDX_CHAR_VAL_B,
	IDX_CHAR_CFG_B,

	HRS_IDX_NB,
};

uint16_t heart_rate_handle_table[HRS_IDX_NB];

/**
 * Structure Represents the Information store inside Flash with Filename "system.ini"
 */

struct SecuredInformation{

	unsigned char BrokerAddress[30];/*!<Hold the Server Name or Internet Address*/
	unsigned char BrokerPort[30];/*!<Hold the Server Port Address*/
	unsigned char serverUname[20];/*!<Hold the Server username*/
	unsigned char serverpass[20];/*!<Hold the server Password*/
	unsigned char Wifi_ssid[50];/*!<Hold the WiFi ssid*/
	unsigned char wifi_username[20];/*!<Hold the WiFi Name --Not in use*/
	unsigned char wifi_password[50];/*!<Hold the WiFi password*/
	unsigned char Serial_number[20];/*!<Hold the Serial No*/
	unsigned char GPS_latitude[20];/*!<Hold the GPS latitude*/
	unsigned char GPS_Longitude[20];/*!<Hold the GPS longitude*/
	int PowerSave;/*!<Hold the power save status*/
	int ResetTimer;/*!<Hold the Power save timer*/
	unsigned char interval[4];/*!<Hold the Ble connection Delay(in ms)-Not in use*/

}Userdata;

/**
 * Structure Represents the Control data store inside Flash with Filename "Inverter.ini"
 */
struct Control{
	int Data_log;/*!<Hold the VG041 KPI status*/
	int Totalunit;
	unsigned char Key[17];/*!<Hold the AES Key */
	unsigned char IV[17];/*!<Holds the AES IV*/
	int Today_unit;
}Inverter;


typedef struct {
	uint8_t                 *prepare_buf;
	int                     prepare_len;
} prepare_type_env_t;

static prepare_type_env_t prepare_write_env;

//#define CONFIG_SET_RAW_ADV_DATA
#ifdef CONFIG_SET_RAW_ADV_DATA
static uint8_t raw_adv_data[] = {
		/* flags */
		0x02, 0x01, 0x06,
		/* tx power*/
		0x02, 0x0a, 0xeb,
		/* service uuid */
		0x03, 0x03, 0xFF, 0x00,
		/* device name */
		0x0f, 0x09, 'E', 'S', 'P', '_', 'G', 'A', 'T', 'T', 'S', '_', 'D','E', 'M', 'O'
};
static uint8_t raw_scan_rsp_data[] = {
		/* flags */
		0x02, 0x01, 0x06,
		/* tx power */
		0x02, 0x0a, 0xeb,
		/* service uuid */
		0x03, 0x03, 0xFF,0x00
};

#else
static uint8_t service_uuid[16] = {/*!<Primary Service:0003CDD0-0000-1000-8000-00805F9B0131*/
		/* LSB <--------------------------------------------------------------------------------> MSB */
		//first uuid, 16bit, [12],[13] is the value
		0x31, 0x01, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xd0, 0xcd, 0x03, 0x00,
};

static uint8_t service_uuid1[16] = {/*!<Primary Service-1:0003CDD1-0000-1000-8000-00805F9B0131*/
		/* LSB <--------------------------------------------------------------------------------> MSB */
		//first uuid, 16bit, [12],[13] is the value
		0x31, 0x01, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xd1, 0xcd, 0x03, 0x00,
};

static uint8_t service_uuid2[16] = {/*!<Primary Service-2:0003CDD2-0000-1000-8000-00805F9B0131*/
		/* LSB <--------------------------------------------------------------------------------> MSB */
		//first uuid, 16bit, [12],[13] is the value
		0x31, 0x01, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xd2, 0xcd, 0x03, 0x00,
};
/* The length of adv data must be less than 31 bytes */
static esp_ble_adv_data_t adv_data = {
		.set_scan_rsp        = false,
		.include_name        = true,
		.include_txpower     = true,
		.min_interval        = 0x20, //0x20
		.max_interval        = 0x40, //0x40
		.appearance          = 0x00,
		.manufacturer_len    = 0,    //TEST_MANUFACTURER_DATA_LEN,
		.p_manufacturer_data = NULL, //test_manufacturer,
		.service_data_len    = 0,
		.p_service_data      = NULL,
		.service_uuid_len    = sizeof(service_uuid),
		.p_service_uuid      = service_uuid,
		.flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

// scan response data
static esp_ble_adv_data_t scan_rsp_data = {
		.set_scan_rsp = true,
		.include_name = true,
		.include_txpower = true,
		.min_interval = 0x20,
		.max_interval = 0x40,
		.appearance = 0x00,
		.manufacturer_len = 0, //TEST_MANUFACTURER_DATA_LEN,
		.p_manufacturer_data =  NULL, //&test_manufacturer[0],
		.service_data_len = 0,
		.p_service_data = NULL,
		.service_uuid_len = 16,
		.p_service_uuid = service_uuid,
		.flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};
#endif /* CONFIG_SET_RAW_ADV_DATA */

static esp_ble_adv_params_t adv_params = {
		.adv_int_min        = 0x20,
		.adv_int_max        = 0x40,
		.adv_type           = ADV_TYPE_IND,
		.own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
		.channel_map        = ADV_CHNL_ALL,
		.adv_filter_policy  = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

struct gatts_profile_inst {
	esp_gatts_cb_t gatts_cb;
	uint16_t gatts_if;
	uint16_t app_id;
	uint16_t conn_id;
	uint16_t service_handle;
	esp_gatt_srvc_id_t service_id;
	uint16_t char_handle;
	esp_bt_uuid_t char_uuid;
	esp_gatt_perm_t perm;
	esp_gatt_char_prop_t property;
	uint16_t descr_handle;
	esp_bt_uuid_t descr_uuid;
};

static void gatts_profile_event_handler(esp_gatts_cb_event_t event,
		esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

/* One gatt-based profile one app_id and one gatts_if, this array will store the gatts_if returned by ESP_GATTS_REG_EVT */
static struct gatts_profile_inst heart_rate_profile_tab[PROFILE_NUM] = {
		[PROFILE_APP_IDX] = {
				.gatts_cb = gatts_profile_event_handler,
				.gatts_if = ESP_GATT_IF_NONE,       /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
		},
};


esp_gatts_cb_event_t global_event = 0;

/* Full Database Description - Used to add attributes into the database */
static const esp_gatts_attr_db_t gatt_db[HRS_IDX_NB] =
{
		// Service Declaration
		[IDX_SVC]        =
		{{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid, ESP_GATT_PERM_READ,
				sizeof(uint16_t), sizeof(service_uuid), (uint8_t *)&service_uuid}},

				/* Characteristic Declaration */
				[IDX_CHAR_A]     =
				{{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
						CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_notify}},

						/* Characteristic Value */
						[IDX_CHAR_VAL_A] =
						{{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_128, (uint8_t *)&service_uuid1, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
								GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(char_value), (uint8_t *)char_value}},

								/* Client Characteristic Configuration Descriptor */
								[IDX_CHAR_CFG_A]  =
								{{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
										sizeof(uint16_t), sizeof(heart_measurement_ccc), (uint8_t *)heart_measurement_ccc}},

										/* Characteristic Declaration */
										[IDX_CHAR_B]      =
										{{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
												CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write_notify}},

												/* Characteristic Value */
												[IDX_CHAR_VAL_B]  =
												{{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_128, (uint8_t *)&service_uuid2, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
														GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(char_value), (uint8_t *)char_value}},

														[IDX_CHAR_CFG_B]  =
														{{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
																sizeof(uint16_t), sizeof(heart_measurement_ccc), (uint8_t *)heart_measurement_ccc}},

																//
																//    /* Characteristic Declaration */
																//    [IDX_CHAR_C]      =
																//    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
																//      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_write}},
																//
																//    /* Characteristic Value */
																//    [IDX_CHAR_VAL_C]  =
																//    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_TEST_C, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
																//      GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(char_value), (uint8_t *)char_value}},

};

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
	switch (event) {
#ifdef CONFIG_SET_RAW_ADV_DATA
	case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
		adv_config_done &= (~ADV_CONFIG_FLAG);
		if (adv_config_done == 0){
			esp_ble_gap_start_advertising(&adv_params);
		}
		break;
	case ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT:
		adv_config_done &= (~SCAN_RSP_CONFIG_FLAG);
		if (adv_config_done == 0){
			esp_ble_gap_start_advertising(&adv_params);
		}
		break;
#else
	case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
		adv_config_done &= (~ADV_CONFIG_FLAG);
		if (adv_config_done == 0){
			esp_ble_gap_start_advertising(&adv_params);
		}
		break;
	case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
		adv_config_done &= (~SCAN_RSP_CONFIG_FLAG);
		if (adv_config_done == 0){
			esp_ble_gap_start_advertising(&adv_params);
		}
		break;
#endif
	case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
		/* advertising start complete event to indicate advertising start successfully or failed */
		if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
			ESP_LOGE(GATTS_TABLE_TAG, "advertising start failed\n");
		}else{
#ifdef Debug

			ESP_LOGI(GATTS_TABLE_TAG, "advertising start successfully\n");
#endif
		}
		break;
	case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
		if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) {
			ESP_LOGE(GATTS_TABLE_TAG, "Advertising stop failed\n");
		}
		else {
#ifdef Debug

			ESP_LOGI(GATTS_TABLE_TAG, "Stop adv successfully\n");
#endif
		}
		break;
	case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
		//ble_connect = 1;

		ESP_LOGI(GATTS_TABLE_TAG, "update connetion params status = %d, min_int = %d, max_int = %d,conn_int = %d,latency = %d, timeout = %d",
				param->update_conn_params.status,
				param->update_conn_params.min_int,
				param->update_conn_params.max_int,
				param->update_conn_params.conn_int,
				param->update_conn_params.latency,
				param->update_conn_params.timeout);
		break;
	default:
		break;
	}
}

void example_prepare_write_event_env(esp_gatt_if_t gatts_if, prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param)
{
#ifdef Debug

	ESP_LOGI(GATTS_TABLE_TAG, "prepare write, handle = %d, value len = %d", param->write.handle, param->write.len);
#endif
	esp_gatt_status_t status = ESP_GATT_OK;
	if (prepare_write_env->prepare_buf == NULL) {
		prepare_write_env->prepare_buf = (uint8_t *)malloc(PREPARE_BUF_MAX_SIZE * sizeof(uint8_t));
		prepare_write_env->prepare_len = 0;
		if (prepare_write_env->prepare_buf == NULL) {
#ifdef Debug
			ESP_LOGE(GATTS_TABLE_TAG, "%s, Gatt_server prep no mem\n", __func__);
#endif
			status = ESP_GATT_NO_RESOURCES;
		}
	} else {
		if(param->write.offset > PREPARE_BUF_MAX_SIZE) {
			status = ESP_GATT_INVALID_OFFSET;
		} else if ((param->write.offset + param->write.len) > PREPARE_BUF_MAX_SIZE) {
			status = ESP_GATT_INVALID_ATTR_LEN;
		}
	}
	/*send response when param->write.need_rsp is true */
	if (param->write.need_rsp){
		esp_gatt_rsp_t *gatt_rsp = (esp_gatt_rsp_t *)malloc(sizeof(esp_gatt_rsp_t));
		if (gatt_rsp != NULL){
			gatt_rsp->attr_value.len = param->write.len;
			gatt_rsp->attr_value.handle = param->write.handle;
			gatt_rsp->attr_value.offset = param->write.offset;
			gatt_rsp->attr_value.auth_req = ESP_GATT_AUTH_REQ_NONE;
			memcpy(gatt_rsp->attr_value.value, param->write.value, param->write.len);
			esp_err_t response_err = esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, status, gatt_rsp);
			if (response_err != ESP_OK){
				ESP_LOGE(GATTS_TABLE_TAG, "Send response error\n");
			}
			free(gatt_rsp);
		}else{
			ESP_LOGE(GATTS_TABLE_TAG, "%s, malloc failed", __func__);
		}
	}
	if (status != ESP_GATT_OK){
		return;
	}
	memcpy(prepare_write_env->prepare_buf + param->write.offset,
			param->write.value,
			param->write.len);
	prepare_write_env->prepare_len += param->write.len;

}

void example_exec_write_event_env(prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param){
	if (param->exec_write.exec_write_flag == ESP_GATT_PREP_WRITE_EXEC && prepare_write_env->prepare_buf){
		esp_log_buffer_hex(GATTS_TABLE_TAG, prepare_write_env->prepare_buf, prepare_write_env->prepare_len);
	}else{
#ifdef Debug
		ESP_LOGI(GATTS_TABLE_TAG,"ESP_GATT_PREP_WRITE_CANCEL");
#endif
	}
	if (prepare_write_env->prepare_buf) {
		free(prepare_write_env->prepare_buf);
		prepare_write_env->prepare_buf = NULL;
	}
	prepare_write_env->prepare_len = 0;
}

static void gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{

	//	printf("event:%d\n",event);
	switch (event) {
	case ESP_GATTS_REG_EVT:{
		esp_err_t set_dev_name_ret = esp_ble_gap_set_device_name(SAMPLE_DEVICE_NAME);
		if (set_dev_name_ret){
			ESP_LOGE(GATTS_TABLE_TAG, "set device name failed, error code = %x", set_dev_name_ret);
		}
#ifdef CONFIG_SET_RAW_ADV_DATA
		esp_err_t raw_adv_ret = esp_ble_gap_config_adv_data_raw(raw_adv_data, sizeof(raw_adv_data));
		if (raw_adv_ret){
			ESP_LOGE(GATTS_TABLE_TAG, "config raw adv data failed, error code = %x ", raw_adv_ret);
		}
		adv_config_done |= ADV_CONFIG_FLAG;
		esp_err_t raw_scan_ret = esp_ble_gap_config_scan_rsp_data_raw(raw_scan_rsp_data, sizeof(raw_scan_rsp_data));
		if (raw_scan_ret){
			ESP_LOGE(GATTS_TABLE_TAG, "config raw scan rsp data failed, error code = %x", raw_scan_ret);
		}
		adv_config_done |= SCAN_RSP_CONFIG_FLAG;
#else
		//config adv data
		esp_err_t ret = esp_ble_gap_config_adv_data(&adv_data);
		if (ret){
			ESP_LOGE(GATTS_TABLE_TAG, "config adv data failed, error code = %x", ret);
		}
		adv_config_done |= ADV_CONFIG_FLAG;
		//config scan response data
		ret = esp_ble_gap_config_adv_data(&scan_rsp_data);
		if (ret){
			ESP_LOGE(GATTS_TABLE_TAG, "config scan response data failed, error code = %x", ret);
		}
		adv_config_done |= SCAN_RSP_CONFIG_FLAG;
#endif
		esp_err_t create_attr_ret = esp_ble_gatts_create_attr_tab(gatt_db, gatts_if, HRS_IDX_NB, SVC_INST_ID);
		if (create_attr_ret){
			ESP_LOGE(GATTS_TABLE_TAG, "create attr table failed, error code = %x", create_attr_ret);
		}
	}
	break;
	case ESP_GATTS_READ_EVT:
#ifdef Debug
		ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_READ_EVT");
#endif
		break;
	case ESP_GATTS_WRITE_EVT:

		if (!param->write.is_prep){
#ifdef Debug
			ESP_LOGI(GATTS_TABLE_TAG, "GATT_WRITE_EVT, handle = %d, value len = %d, value :", param->write.handle, param->write.len);
#endif
			//

			//Ble_Watchdog_Timer = 0;


			bool  writingflag=false;

			char *Result = malloc(param->write.len+1);
			Result[param->write.len] = 0;
			strncpy(Result,(const char*)param->write.value,param->write.len);

			Result = strtok(Result, ":");
			if(strcmp(Result,"SN") == 0){
				SSID_count=0;
				PWD_count=0;
				//	printf("Serial Number:%s",strtok(NULL, ":"));
				rsp.attr_value.value[6] = 0x01;
				rsp.attr_value.value[7] = 0x23;
				char *data = strtok(NULL,":");
					printf("SN:%s\n",(const char*)data);
				if(data != NULL){
					if(Inverter.Totalunit!= 0 && strcmp((char *)Userdata.Serial_number,(const char*)data)!=0){
						Inverter.Totalunit = 0;
						Write_Inverter();

					}
					if(Reconfig_Flag>0){
					Reconfig_Flag=0;
					strcpy((char *)Userdata.Serial_number,(const char*)data);
					}
				}else{
					strcpy((char *)Userdata.Serial_number,"");
				}
				printf("Serial Number:%s\n",Userdata.Serial_number);
				writingflag = true;
				reboot = true;
			}else if(strcmp(Result,"SSID") == 0){
				PWD_count=0;
				writingflag = true;
				reboot = true;
				rsp.attr_value.value[6] = 0x02;
				rsp.attr_value.value[7] = 0x23;
				char *data = strtok(NULL,":");
				//	printf("Wifi:%s\n",Result);
				if(data != NULL)
				{
					if(SSID_count==0)
					{
						SSID_count=1;
						strcpy((char *)Userdata.Wifi_ssid,(const char*)data);
					}
					else
					{
						strcat((char *)Userdata.Wifi_ssid,(const char *)data);
					}
				}
				else
				{
					strcpy((char *)Userdata.Wifi_ssid,"");
				}

			}else if(strcmp(Result,"PWD") == 0){
				SSID_count=0;
				writingflag = true;
				reboot = true;
				rsp.attr_value.value[6] = 0x03;
				rsp.attr_value.value[7] = 0x23;
				//printf("Password:%s\n",Userdata.wifi_password);
				char *data = strtok(NULL,":");
				if(data != NULL)
				{
					if(PWD_count==0)
					{
						PWD_count=1;
						strcpy((char *)Userdata.wifi_password,(const char*)data);
					}
					else
					{
						strcat((char *)Userdata.wifi_password,(const char *)data);
					}
				}
				else
					strcpy((char *)Userdata.wifi_password,"");

			}else if(strcmp(Result,"Lat") == 0){
				SSID_count=0;
				PWD_count=0;
				writingflag = true;
				//reboot = true;
				rsp.attr_value.value[6] = 0x04;
				rsp.attr_value.value[7] = 0x23;
				//printf("Password:%s\n",Userdata.wifi_password);
				char *data = strtok(NULL,":");
				if(data != NULL){
					strcpy((char *)Userdata.GPS_latitude,(const char*)data);
				}
				else
					strcpy((char *)Userdata.GPS_latitude,"");


			}else if(strcmp(Result,"Lon") == 0){
				SSID_count=0;
				PWD_count=0;
				writingflag = true;
				//	reboot = true;
				rsp.attr_value.value[6] = 0x05;
				rsp.attr_value.value[7] = 0x23;
				//printf("Password:%s\n",Userdata.wifi_password);
				char *data = strtok(NULL,":");
				if(data != NULL){
					strcpy((char *)Userdata.GPS_Longitude,(const char*)data);
				}
				else
					strcpy((char *)Userdata.GPS_Longitude,"");

				Write_systemConf();
			}else if(strcmp(Result,"K") == 0){
				SSID_count=0;
				PWD_count=0;
				rsp.attr_value.value[6] = 0x07;
				rsp.attr_value.value[7] = 0x23;
				writingflag = false;
				char *data = strtok(NULL,":");
				if(data != NULL && (strlen(data) == 16)/* && (strcmp((char *)Userdata.Serial_number,data) == 0)*/){
					strcpy((char *)Inverter.Key,(const char*)data);
				}else{
					esp_ble_gatts_send_indicate(spp_gatts_if, spp_conn_id, heart_rate_handle_table[IDX_CHAR_VAL_A],
							5,(const char*)"ERROR", false);
				}

			}else if(strcmp(Result,"IV") == 0){
				SSID_count=0;
				PWD_count=0;
				reboot = true;
				writingflag = false;
				rsp.attr_value.value[6] = 0x08;
				rsp.attr_value.value[7] = 0x23;
				char *data = strtok(NULL,":");
				if(data != NULL && (strlen(data) == 16) /*&& (strcmp((char *)Inverter.Key,data) == 0)*/){
					strcpy((char *)Inverter.IV,(const char*)data);
				}else{
					esp_ble_gatts_send_indicate(spp_gatts_if, spp_conn_id, heart_rate_handle_table[IDX_CHAR_VAL_A],
							5,(const char*)"ERROR", false);
				}

				Write_Inverter();
			}else if(strcmp(Result,"INT") == 0){
				SSID_count=0;
				PWD_count=0;
				writingflag = true;
				reboot = true;
				rsp.attr_value.value[6] = 0x09;
				rsp.attr_value.value[7] = 0x23;
				char *data = strtok(NULL,":");
				printf("Interval:%s\n",Result);
				if(data != NULL){
					strcpy((char *)Userdata.interval,(const char*)data);
				}else
					strcpy((char *)Userdata.interval,"");

			}else if(param->write.len==8 ){
				SSID_count=0;
				PWD_count=0;
				uart_write_bytes(UART_NUM_1, (const char*) param->write.value,8);
				spp_gatts_if = gatts_if;
				spp_conn_id = param->read.conn_id;
				transid = param->write.trans_id;
				global_event = event;

			}


			if(writingflag == true){


				esp_err_t response_err =  esp_ble_gatts_send_indicate(spp_gatts_if, spp_conn_id, heart_rate_handle_table[IDX_CHAR_VAL_A],
						8, rsp.attr_value.value, false);
				rsp.attr_value.value[6] = 0;
				rsp.attr_value.value[7] = 0;
				//return;
			}

			free(Result);


		}else{
			/* handle prepare write */
			example_prepare_write_event_env(gatts_if, &prepare_write_env, param);
		}
		break;
	case ESP_GATTS_EXEC_WRITE_EVT:
#ifdef Debug
		ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_EXEC_WRITE_EVT");
#endif

		example_exec_write_event_env(&prepare_write_env, param);
		break;
	case ESP_GATTS_MTU_EVT:
#ifdef Debug
		ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_MTU_EVT, MTU %d", param->mtu.mtu);
#endif
		break;
	case ESP_GATTS_CONF_EVT:
#ifdef Debug
		ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_CONF_EVT, status = %d", param->conf.status);
#endif
		break;
	case ESP_GATTS_START_EVT:
#ifdef Debug
		ESP_LOGI(GATTS_TABLE_TAG, "SERVICE_START_EVT, status %d, service_handle %d\n", param->start.status, param->start.service_handle);
#endif
		break;
	case ESP_GATTS_CONNECT_EVT:
		//	LED_DISPLAY(0x0B);
		ble_connect = 1;
		WriteRequest=false;

		//		gpio_set_level(BLE_LED, 1);


#ifdef Debug
		ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_CONNECT_EVT, conn_id = %d", param->connect.conn_id);
#endif
		//		//	esp_log_buffer_hex(GATTS_TABLE_TAG, param->connect.remote_bda, 6);
		//		esp_ble_conn_update_params_t conn_params = {0};
		//		memcpy(conn_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));
		//		/* For the IOS system, please reference the apple official documents about the ble connection parameters restrictions. */
		//		conn_params.latency = 0;
		////		conn_params.max_int = 0x20;//20    // max_int = 0x20*1.25ms = 40ms
		////		conn_params.min_int = 0x0A;//10    // min_int = 0x10*1.25ms = 20ms
		//		//conn_params.timeout = 2000; //1000   // timeout = 400*10ms = 4000ms
		//		//start sent the update connection parameters to the peer device.
		//
		//	 	// gl_profile_tab[PROFILE_APP_IDX].conn_id = param->connect.conn_id;
		//
		//		esp_ble_gap_update_conn_params(&conn_params);
		//		//		esp_ble_gap_start_advertising(&adv_params);
		////		vTaskDelay(500 / portTICK_RATE_MS); //5000 //Delay added for product reconfig issues

		//		esp_ble_conn_update_params_t conn_params;
		//		memcpy(conn_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));
		//		conn_params.min_int = 0x06; // x 1.25ms
		//		conn_params.max_int = 0x40; // x 1.25ms
		//		conn_params.latency = 0x00; //number of skippable connection events
		//		conn_params.timeout = 48; // x 6.25ms, time before peripherial will assume connection is dropped.
		//
		//		esp_ble_gap_update_conn_params(&conn_params);
		break;
	case ESP_GATTS_DISCONNECT_EVT:
		ble_connect = 0;
//		gpio_set_level(BLE_LED, 0);
		printf("Ble disconnected\n");
#ifdef Debug
		ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_DISCONNECT_EVT, reason = %d", param->disconnect.reason);
#endif
		esp_ble_gap_start_advertising(&adv_params);

		//LED_DISPLAY(0);


		if(reboot == true ){
			Write_systemConf();

			vTaskDelay(100 / portTICK_RATE_MS);
			//			esp_restart();

			esp_sleep_enable_timer_wakeup(1000000); //Set sleep mode to wake up controller
			esp_deep_sleep_start();

		}

		break;
	case ESP_GATTS_CREAT_ATTR_TAB_EVT:{
		if (param->add_attr_tab.status != ESP_GATT_OK){

			ESP_LOGE(GATTS_TABLE_TAG, "create attribute table failed, error code=0x%x", param->add_attr_tab.status);
		}
		else if (param->add_attr_tab.num_handle != HRS_IDX_NB){
			ESP_LOGE(GATTS_TABLE_TAG, "create attribute table abnormally, num_handle (%d) \
                        doesn't equal to HRS_IDX_NB(%d)", param->add_attr_tab.num_handle, HRS_IDX_NB);
		}
		else {
#ifdef Debug
			ESP_LOGI(GATTS_TABLE_TAG, "create attribute table successfully, the number handle = %d\n",param->add_attr_tab.num_handle);
#endif
			memcpy(heart_rate_handle_table, param->add_attr_tab.handles, sizeof(heart_rate_handle_table));
			esp_ble_gatts_start_service(heart_rate_handle_table[IDX_SVC]);
		}
		break;
	}
	case ESP_GATTS_LISTEN_EVT:

		break;
	case ESP_GATTS_STOP_EVT:
	case ESP_GATTS_OPEN_EVT:
	case ESP_GATTS_CANCEL_OPEN_EVT:
	case ESP_GATTS_CLOSE_EVT:
	case ESP_GATTS_CONGEST_EVT:
	case ESP_GATTS_UNREG_EVT:
	case ESP_GATTS_DELETE_EVT:
	default:
		break;
	}
}


static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{


	spp_gatts_if = gatts_if;
	spp_conn_id = param->read.conn_id;

	/* If event is register event, store the gatts_if for each profile */
	if (event == ESP_GATTS_REG_EVT) {
		if (param->reg.status == ESP_GATT_OK) {
			heart_rate_profile_tab[PROFILE_APP_IDX].gatts_if = gatts_if;
		} else {
			ESP_LOGI(GATTS_TABLE_TAG, "reg app failed, app_id %04x, status %d\n",
					param->reg.app_id,
					param->reg.status);
			return;
		}
	}
	do {
		int idx;
		for (idx = 0; idx < PROFILE_NUM; idx++) {
			/* ESP_GATT_IF_NONE, not specify a certain gatt_if, need to call every profile cb function */
			if (gatts_if == ESP_GATT_IF_NONE || gatts_if == heart_rate_profile_tab[idx].gatts_if) {
				if (heart_rate_profile_tab[idx].gatts_cb) {
					heart_rate_profile_tab[idx].gatts_cb(event, gatts_if, param);
				}
			}
		}
	} while (0);
}



void RX_hexbytes(uint8_t *bytes){



#ifdef Debug
	ESP_LOGE(GATTS_TABLE_TAG, "Notify the data");
#endif

	//	if(nbytes < 8){
	//		gpio_set_level(BLE_LED, 0);
	//
	vTaskDelay( 30/ portTICK_RATE_MS);
	//
	//		gpio_set_level(BLE_LED, 1);
	//
	//		uart_write_bytes(UART_NUM_1, (const char*) tx_buff, sizeof(tx_buff));
	//		return;
	//	}
	//if(nbytes == 8){
	//	nbytes = 8;
	esp_err_t response_err;

	response_err =	esp_ble_gatts_send_indicate(spp_gatts_if, spp_conn_id, heart_rate_handle_table[IDX_CHAR_VAL_A],
			8, bytes, false);
	//}
	//}
}


void Config_GATT()
{

	esp_err_t ret;
	//
	//    /* Initialize NVS. */
	//    ret = nvs_flash_init();
	//    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
	//        ESP_ERROR_CHECK(nvs_flash_erase());
	//        ret = nvs_flash_init();
	//    }
	//    ESP_ERROR_CHECK( ret );

	esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
	ret = esp_bt_controller_init(&bt_cfg);
	if (ret) {
		ESP_LOGE(GATTS_TABLE_TAG, "%s enable controller failed\n", __func__);
		return;
	}


	ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
	if (ret) {
		ESP_LOGE(GATTS_TABLE_TAG, "%s enable controller failed\n", __func__);
		return;

	}

	esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT,ESP_PWR_LVL_P9);

	ESP_LOGI(GATTS_TABLE_TAG, "%s init bluetooth\n", __func__);
	ret = esp_bluedroid_init();
	if (ret) {

		ESP_LOGE(GATTS_TABLE_TAG, "%s init bluetooth failed\n", __func__);
		esp_sleep_enable_timer_wakeup(1000000); //Set sleep mode to wake up controller
		esp_deep_sleep_start();
		return;
	}
	ret = esp_bluedroid_enable();
	if (ret) {
		ESP_LOGE(GATTS_TABLE_TAG, "%s enable bluetooth failed\n", __func__);
		esp_sleep_enable_timer_wakeup(1000000); //Set sleep mode to wake up controller
		esp_deep_sleep_start();
		return;
	}

	ret = esp_ble_gatts_register_callback(gatts_event_handler);
	if (ret){
		ESP_LOGE(GATTS_TABLE_TAG, "gatts register error, error code = %x", ret);
		esp_sleep_enable_timer_wakeup(1000000); //Set sleep mode to wake up controller
		esp_deep_sleep_start();

		return;
	}
	ret = esp_ble_gap_register_callback(gap_event_handler);
	if (ret){
		ESP_LOGE(GATTS_TABLE_TAG, "gap register error, error code = %x", ret);
		esp_sleep_enable_timer_wakeup(1000000); //Set sleep mode to wake up controller
		esp_deep_sleep_start();
		return;
	}
	ret = esp_ble_gatts_app_register(ESP_APP_ID);
	if (ret){
		ESP_LOGE(GATTS_TABLE_TAG, "gatts app register error, error code = %x", ret);
		esp_sleep_enable_timer_wakeup(1000000); //Set sleep mode to wake up controller
		esp_deep_sleep_start();
		return;
	}
	esp_err_t local_mtu_ret = esp_ble_gatt_set_local_mtu(500);
	if (local_mtu_ret){
		ESP_LOGE(GATTS_TABLE_TAG, "set local  MTU failed, error code = %x", local_mtu_ret);
		esp_sleep_enable_timer_wakeup(1000000); //Set sleep mode to wake up controller
		esp_deep_sleep_start();
	}
}


int ble_if_close(void){

	esp_err_t local_mtu_ret = esp_bluedroid_disable();
	if (local_mtu_ret){
		ESP_LOGE(GATTS_TABLE_TAG, "esp_bluedroid_disable, error code = %x", local_mtu_ret);
		esp_sleep_enable_timer_wakeup(1000000); //Set sleep mode to wake up controller
		esp_deep_sleep_start();
	}
	local_mtu_ret = esp_bluedroid_deinit();
	if (local_mtu_ret){
		ESP_LOGE(GATTS_TABLE_TAG, "esp_bluedroid_deinit, error code = %x", local_mtu_ret);
		esp_sleep_enable_timer_wakeup(1000000); //Set sleep mode to wake up controller
		esp_deep_sleep_start();
	}
	local_mtu_ret = esp_bt_controller_disable();
	if (local_mtu_ret){
		ESP_LOGE(GATTS_TABLE_TAG, "esp_bt_controller_disable, error code = %x", local_mtu_ret);
		esp_sleep_enable_timer_wakeup(1000000); //Set sleep mode to wake up controller
		esp_deep_sleep_start();
	}
	local_mtu_ret = esp_bt_controller_deinit();
	if (local_mtu_ret){
		ESP_LOGE(GATTS_TABLE_TAG, "esp_bt_controller_deinit, error code = %x", local_mtu_ret);
		esp_sleep_enable_timer_wakeup(1000000); //Set sleep mode to wake up controller
		esp_deep_sleep_start();
	}
	return 0;
}
#endif
