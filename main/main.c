#define Version "2.0"

/*Head Includes*/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <rom/rtc.h>
#include "esp_wifi.h"
#include "rtc.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event_loop.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "uart_esp.h"
#include "DUPS_Communication.h"
#include "esp_bt_device.h"
#include "esp_sleep.h"


#include "gatts_table_creat_demo.h"
#include "File.h"
#include "ota_checker.h"
#include "diagnostic.h"
#include "AES_CBC.h"
#ifdef SSL
extern const uint8_t iot_eclipse_org_pem_start[] asm("_binary_iot_eclipse_org_pem_start");
extern const uint8_t iot_eclipse_org_pem_end[]   asm("_binary_iot_eclipse_org_pem_end");
#endif
/* Pre process includes*/
#define ProjectName "dups"/*!<Hold the Product name*/
const char *MQTT_TAG = "Server";/*!<Display Debug console log TAG*/

static EventGroupHandle_t wifi_event_group; /*!<Create the handler for WiFi*/
const static int CONNECTED_BIT = BIT0;/*!<Hold the WiFi connection Bit*/
bool Server_Started = 0;
char SubTopic[50]/*<Hold the Subscribe Topic for Server*/, Data_Collect[50];/*<Hold the data collection Subscribe Topic for Server*/
bool Time_sync = false/*!<Hold the WiFi time syn status*/,
		AES_KEY = false/*!<Hold the AES key process status*/;

/**
 *@brief SendMessage to sever
 *@param Publish message
 *@return void
 */
void SendMessage(char *PublishMessage) {

	//Enable data collection
	bootcalled = 1;
	/**Publish message length should be more than length of five*/
	if (strlen(PublishMessage) > 5) {
		/*AES -CBC mode*/
		if (Secured == true) {
			int EncryptLength = Encrypt_CBC128(
					(const unsigned char*) PublishMessage, Inverter.Key,
					Inverter.IV);
			if (esp_mqtt_client_publish(DUPS_client, Pub_Topic,
					(char*) PublishMessage, EncryptLength, 0, 0) == -1) {
				bootcalled = 0; /*Disable the data collection*/
			}
			/*Plain Text mode*/
		} else if (esp_mqtt_client_publish(DUPS_client, Pub_Topic,
				(char*) PublishMessage, strlen((char*) PublishMessage), 0, 0)
				== -1) {
			//Retry
			vTaskDelay(250 / portTICK_RATE_MS);
			if (esp_mqtt_client_publish(DUPS_client, Pub_Topic,
					(char*) PublishMessage, strlen((char*) PublishMessage), 0,
					0) == -1) {

				bootcalled = 0;
			}
		}
		Flag15_Counter = 0;
		if (Time_out_Flag) {
			bootcalled = 0;
			App_Timeout = 0;
		}
	}
}
/**
 * @brief Handled the Mqtt Event Handler
 * @param Mqtt events
 */
static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event) {
	DUPS_client = event->client;
	int msg_id = 0;
	switch (event->event_id) {
	case MQTT_EVENT_ANY:
		break;
	case MQTT_EVENT_CONNECTED:
		connection_error = 0;
		sprintf(SubTopic, "device/webapi/nous/timestamp/%s",
				(const char*) Userdata.Serial_number);
		esp_mqtt_client_subscribe(DUPS_client, SubTopic, 0);
		sprintf(Data_Collect, "apps/dups/%s", ModelCode);
		esp_mqtt_client_subscribe(DUPS_client, Data_Collect, 0);
		//Subscribe Topic
		sprintf(SubTopic, "apps/dups/%s/%s", ModelCode,
				(const char*) Userdata.Serial_number);
		msg_id = esp_mqtt_client_subscribe(DUPS_client, SubTopic, 0);
		sprintf(Pub_Topic, "device/%s/%s/lwt/%s", ProjectName,
				(const char*) ModelCode, (const char*) Userdata.Serial_number);/*LWT publish topic*/
		esp_mqtt_client_publish(DUPS_client, Pub_Topic, "online", 6, 0, 1);/*Publish LWT data*/
		sprintf(Pub_Topic, "device/dups/%s/%s", ModelCode,
				(const char*) Userdata.Serial_number);
		obtain_time();
		Get_IP_From_Url();
		char strftime_buf[64];
		// update 'now' variable with current time
		time(&now);
		setenv("TZ", "IST-5:30", 1); // only Indian Standard time
		tzset();
		localtime_r(&now, &timeinfo);

		if (timeinfo.tm_year < (2016 - 1900)) {
			printf("date and time sync From Nouse\n");
			char Timestamp[50];
			sprintf(Timestamp, "gmtindia*%s*yyyyMMddHHmmss",
					(const char*) Userdata.Serial_number);
			esp_mqtt_client_publish(DUPS_client, "device/webapi/nous",
					Timestamp, strlen((char*) Timestamp), 0, 0);
			Time_sync = true;
			timeFlag = false;
		} else {
			printf("Time Fetched from SNTP\n");
			strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
			printf("Time: %s\n", strftime_buf);
			timeFlag = true;
			if (timeCheckMode == TIME_CHECK_REQUESTED) {
				timeCheckMode = TIME_CHECK_SUCCESS;
			}

		}
		break;
	case MQTT_EVENT_DISCONNECTED:
		connection_error = 1;

		break;

	case MQTT_EVENT_SUBSCRIBED:
		ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);

		break;
	case MQTT_EVENT_UNSUBSCRIBED:
		ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
		break;
	case MQTT_EVENT_PUBLISHED:
		break;
	case MQTT_EVENT_DATA:
		App_Timeout = 0;
		Time_out_Flag = 0;
		/*IF AES-CBC mode*/
		if (Secured == true
				&& (strncmp(SubTopic, event->topic, (char*) event->topic_len)
						== 0)) {
			event->data = Decrypt_CBC128((char*) event->data, event->data_len,
					Inverter.Key, Inverter.IV);
			event->data_len = trimTrailing((char*) event->data);
		}
		/*If data collection interval*/
		if ((event->data_len < 20 && ble_connect == 0)
				&& (strncmp(Data_Collect, event->topic,
						(char*) event->topic_len) == 0)) {

			if (Lock_system == true && Inverter.Data_log == 1)
				break;
			if (strncmp(event->data, "info", event->data_len) == 0) {
//				Data_log_Count=1;Data_log_flag=true;
//				SendMessage(Info_Json);
//				printf("\nInfo From Nouse\n");
//				Statusflag = false;
			} else if (strncmp(event->data, "stop", event->data_len) == 0) {
//				 Data_log_Count=0;Data_log_flag=false;
//				printf("\nStop From Nouse\n");
//				bootcalled = 0;
			}

			/*If mobile application connected*/
		} else if ((event->data_len < 80 && ble_connect == 0)
				&& (strncmp(SubTopic, event->topic, (char*) event->topic_len)
						== 0)) {

			/*if Info message*/
			if (strncmp(event->data, "info", event->data_len) == 0) {
				Data_log_Count = 0;
				Data_log_flag = false;
				SendMessage(Info_Json);
				/*If stop message*/
			} else if (strncmp(event->data, "stop", event->data_len) == 0) {
				bootcalled = 0;
				Data_log_Count = 0;
				Data_log_flag = false;
				/*if Start Message*/
			} else if (strncmp(event->data, "start", event->data_len) == 0) {
				Data_log_Count = 0;
				Data_log_flag = false;
				printf("\nStart From App\n");
				SendMessage(Actual_Json);
			} else {

				bootcalled = 1; //Enable the data collection
				Statusflag = false;
				/*if system locked,Return the function*/
				if (Lock_system == true)
					break;

				char *Result = malloc(event->data_len + 1);
				Result[event->data_len] = 0;

				strncpy(Result, event->data, event->data_len);
				strcpy(event->data, "");
				Result = strtok(Result, ":");

				memset(event->data, 0, sizeof(0));
				strncpy(event->data, Result + 2, 3);

				switch (atoi(event->data)) {

				case 94: //TURN OFF -ON
					Write_Data(SWITCH_STAT, atoi(strtok(NULL, ":")), true);
					break;
				case 21: //Mode Setting
					Write_Data(Mode, atoi(strtok(NULL, ":")), true);
					break;

				case 41: //Abbreviation flag in DUPS
				{
					bootcalled = 0;

					int Push = atoi(strtok(NULL, ":"));
					if (Push <= 1) {
						if (Push != Inverter.Data_log) {
							Inverter.Data_log = Push;
							Data_log = Inverter.Data_log;
							Write_Inverter();
						}
					}
				}
					break;
				case 74: //Set Present Time
					//Don't Change the Time if inverter having the Value 43100 - 43200 /..
					break;
				case 75: //Configure Battery
					break;
				case 105: //clear the Battery
					//Clear_BatteryCharge();
					Write_Data(Charging, 0, true);
					break;

				case 109: //Set Time //YYYYMMDDhhmmss

					event->data = strtok(NULL, ":");
					if (strlen(event->data) == 14) {
						setDate((const char*) event->data, 1);
					}
					break;
				case 34: //Buzzer Setting
					Write_Data(Buzzer_settings, atoi(strtok(NULL, ":")), true);

					break;
				case 35:  //Voltage Regulator Status
					Write_Data(Output_voltage, atoi(strtok(NULL, ":")), true);
					break;
				case 36: //Appliance mode status
					Write_Data(Appliance_Mode, atoi(strtok(NULL, ":")), true);
					break;
				case 37: //main Force cut time
					Write_Data(Mains_forced_cut_time, atoi(strtok(NULL, ":")),
					true);
					break;
				case 38: //Main force cut status
					Write_Data(Mains_force_cut_status, atoi(strtok(NULL, ":")),
					true);
					break;
				case 43: //Set 140V mode
					Write_Data(Slr_Clr_Day, atoi(strtok(NULL, ":")), true);
					break;
				case Solar_Savings_Total:
					Inverter.Totalunit = atoi(strtok(NULL, ":"));
					Write_Data(Solar_Savings_Total, Inverter.Totalunit, true);
					Write_Inverter();
					break;
				case 99: //Turbo charging
					Write_Data(Turbo_Charging_status, atoi(strtok(NULL, ":")),
					true);
					break;
				case 100: //Holiday Mode
					Write_Data(Holiday_Mode_status, atoi(strtok(NULL, ":")),
					true);
					break;
				case 50: //Load Alarm set
					Write_Data(Load_alarm_status, atoi(strtok(NULL, ":")),
					true);
					break;
				case 71: //Battery Remaining Alarm set
					Write_Data(Battery_remaining_alarm_status,
							atoi(strtok(NULL, ":")), true);
					break;
				case 72: //Extra Battery Backup
					Write_Data(Extra_Backup_status, 0, true);
					break;
				case 185: //Set daily load usage limit
					Write_Data(Daily_Day_time_loadUsage,
							atoi(strtok(NULL, ":")), true);
					break;
				case 186: //Solar Panel Clearing reminder
					Write_Data(Cleaning_Reminder, atoi(strtok(NULL, ":")),
					true);
					break;
				case 202:
					Write_Data(Mains_Charger_ON_OFF, atoi(strtok(NULL, ":")),
					true);
					break;
				case 203: //Authentication KPI
					event->data = strtok(NULL, ":");
					if (event->data != NULL && strlen(event->data) == 16
							&& strcmp((char*) Userdata.Serial_number,
									event->data) == 0) {
						AES_KEY = true;
						strcpy(event->data, "VG203:8966");
					} else {
						strcpy(event->data, "VG203:ERROR");
					}
					SendMessage(event->data);
					break;
				case 204: // Key updation KPI
					event->data = strtok(NULL, ":");
					if (event->data != NULL
							&& strlen(event->data) == 16 && AES_KEY == true) {
						strcpy((char*) Inverter.Key, (const char*) event->data);
						strcpy(event->data, "VG204:8967");

					} else {
						AES_KEY = false;
						strcpy(event->data, "VG204:ERROR");
					}
					SendMessage(event->data);
					break;
				case 205: //IV updation KPI
					event->data = strtok(NULL, ":");

					if (event->data != NULL && (strlen(event->data) == 16)
							&& (strcmp((char*) Inverter.Key, event->data) == 0)
							&& AES_KEY == true) {
						strcpy((char*) Inverter.IV, (const char*) event->data);
						Write_Inverter();
						strcpy(event->data, "VG205:8968");
						Secured = true;
						AES_KEY = false;
					} else {
						AES_KEY = false;
						strcpy(event->data, "VG205:ERROR");
					}
					SendMessage(event->data);
					break;
				case 990: {
					bootcalled = 0;
					ota_check(strtok(NULL, ":"));

				}
					break;

				case 219: //Daily_Load_Start
					Write_Data(Daily_Load_Start, atoi(strtok(NULL, ":")), true);
					break;

				default:
					break;
				}
				free(Result);
			}
		} else {
			/*Time updation block*/
			printf("Time Stamb Topic:%s\n", event->data);
			char *Result = malloc(event->data_len + 1);
			Result[event->data_len] = 0;
			strncpy(Result, event->data, event->data_len);
			strcpy(event->data, Result);
			if (strlen(event->data) == 14) {
				setDate((const char*) event->data, 0);
				Result = (char*) realloc(Result, 50);
				sprintf(Result, "device/webapi/nous/timestamp/%s",
						(const char*) Userdata.Serial_number);
				esp_mqtt_client_unsubscribe(DUPS_client, Result);
				timeFlag = true;
				if (timeCheckMode == TIME_CHECK_REQUESTED) {
					timeCheckMode = TIME_CHECK_SUCCESS;
				}
			}
			free(Result);
		}
		break;
	case MQTT_EVENT_ERROR:
		ESP_LOGE(TAG, "MQTT_EVENT_ERROR, msg_id=%d", event->msg_id);
		break;

	case MQTT_EVENT_BEFORE_CONNECT:
		break;
	case MQTT_EVENT_DELETED:
		break;
	}
	return ESP_OK;
}
/*IF MQTT-SSL with certificate*/
#ifdef SSL
static void mqtt_app_start(void)
{
	char lwtMessage[30];
	char data[10];
	memset(lwtMessage,0,sizeof(lwtMessage));
	memset(data,0,sizeof(data));
	sprintf((char *)data,"/%s/lwt/",(const char*)ModelCode);
	strcpy((char *)lwtMessage,(const char*)"device/");
	strcat((char *)lwtMessage,(const char*)ProjectName);
	strcat((char *)lwtMessage,(const char*)data);
	strcat((char *)lwtMessage,(const char*)Userdata.Serial_number);
	const esp_mqtt_client_config_t mqtt_cfg = {
			.uri = "mqtts://vguardbox.com:8883",
			.event_handle = mqtt_event_handler,
			.cert_pem = (const char *)iot_eclipse_org_pem_start,
			.client_id =(const char*)Userdata.Serial_number,
			.lwt_topic =(const char*)lwtMessage,
			.lwt_msg = "shutdown",
			.lwt_msg_len = 8,
			.task_stack = 4096,
			.lwt_qos = 0,
			.lwt_retain = 1
	};
	esp_mqtt_client_handle_t client1 = esp_mqtt_client_init(&mqtt_cfg);
	esp_mqtt_client_start(client1);
}

#else
/**
 * @brief Read the WiFi mac address
 */
void GetSystem_Mac() {

	uint8_t l_Mac[6];
	esp_wifi_get_mac(ESP_IF_WIFI_STA, l_Mac);
	char tt[3];
	memset(tt, 0, sizeof(tt));
	memset(Wifi_address, 0, sizeof(Wifi_address));
	for (uint8_t i = 0; i < 6; i++) {
		sprintf(tt, "%02x", l_Mac[i]);
		strcat(Wifi_address, tt);
	}
	return;
}
/**
 *@brief configure and init the MQTT service.
 */
static void mqtt_app_start(void) {

	char lwtMessage[30];
	char data[30];
	char SN[30];
	memset(lwtMessage, 0, sizeof(lwtMessage));
	memset(data, 0, sizeof(data));
	/*subscribe the LWT topic*/
	sprintf((char*) data, "/%s/lwt/", (const char*) ModelCode);
	strcpy((char*) lwtMessage, (const char*) "device/");
	strcat((char*) lwtMessage, (const char*) ProjectName);
	strcat((char*) lwtMessage, (const char*) data);
	strcat((char*) lwtMessage, (const char*) Userdata.Serial_number);
	sprintf((char*) SN, "device%s", (const char*) Userdata.Serial_number);
	//printf("\nSNo:%s\n",Userdata.Serial_number);
	/*MQTT configuration setting*/
	const esp_mqtt_client_config_t mqtt_cfg = { .uri =
			"mqtt://vguardbox.com:8883", .event_handle = mqtt_event_handler,
			.client_id = (const char*) SN,
			.lwt_topic = (const char*) lwtMessage, .lwt_msg = "shutdown",
			.lwt_msg_len = 8, .task_stack = 6 * 1024, .buffer_size = 2048,
			.keepalive = 120, .lwt_qos = 0, .lwt_retain = 1 };
	/*init the Mqtt settings*/
	DUPS_client = esp_mqtt_client_init(&mqtt_cfg);
}
#endif
/**
 * @brief Wifi even handler
 */

static esp_err_t wifi_event_handler(void *ctx, system_event_t *event) {
	switch (event->event_id) {
	case SYSTEM_EVENT_STA_START: //Start the wifi
		tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA, "VG_Inverter");
		esp_wifi_connect();
		break;
	case SYSTEM_EVENT_STA_GOT_IP: //Got Ip address
		if (No_of_battery <31){
		xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
		obtain_time();
		// update 'now' variable with current time
		time(&now);
		setenv("TZ", "IST-5:30", 1); // only Indian Standard time
		tzset();
		localtime_r(&now, &timeinfo);

		esp_mqtt_client_start(DUPS_client); //Start the mqtt service

		GetSystem_Mac();
		wifisystem_flag = true;
		wifi_connect = true;
		 }else{
			 wifisystem_flag = true;
		 }
		break;
	case SYSTEM_EVENT_STA_DISCONNECTED: //WiFi disconnection
		wifisystem_flag = false;
		esp_wifi_connect();
		wifi_connect = false;
		xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
		break;
	default:
		break;
	}
	return ESP_OK;
}
/*
 * @brief:WiFi connection function
 *
 */
void wifi_conn_init(void) {
	memset((char*) inverter_mod, 0, sizeof(inverter_mod));
	strcpy((char*) inverter_mod, "VG_Inverter");

	tcpip_adapter_init();
	wifi_event_group = xEventGroupCreate();
	ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL));
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

	  wifi_config_t wifi_config =
	      {
	          .sta = {
	              .ssid = "",
	              .password = ""}};


	  if( (No_of_battery <31)){

	strcpy((char *)wifi_config.sta.ssid,(const char*)Userdata.Wifi_ssid);
	strcpy((char *)wifi_config.sta.password,(const char*)Userdata.wifi_password);
	  }
	  else{
	strcpy((char *)wifi_config.sta.ssid,"VGUARD");
	strcpy((char *)wifi_config.sta.password,"12345678");
	  }
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
	esp_wifi_set_ps(WIFI_PS_NONE);
	ESP_ERROR_CHECK(esp_wifi_start());


	tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA, "VG_Inverter");
}
/*
 * @brief:Input and output intilialized
 *
 */
void IO_initialized() {
	gpio_pad_select_gpio (10);
	gpio_set_direction(10, GPIO_MODE_OUTPUT);

	gpio_pad_select_gpio (04);
	gpio_set_direction(04, GPIO_MODE_OUTPUT);
	gpio_set_level(04, 1);
//	gpio_pad_select_gpio (05);
//	gpio_set_direction(05, GPIO_MODE_OUTPUT);
//	gpio_set_level(05, 0);

	}
void app_main() {
	RESET_REASON reason = rtc_get_reset_reason(0);
	ESP_LOGI("MAIN TASK", "RST_ID:%d", reason);

	if(reason != RTC_SW_CPU_RESET) //Time update condition check
			{
		timeupdate = 0;
	} else {
		Server_Started = 1;
	}
	if (timeupdate > 1) {
		timeupdate = 0;
	}
	ESP_LOGI(MQTT_TAG, "[APP] Startup..FirmwareVersion...%s ", Version);

	IO_initialized(); //IO initialize

	Init_Uart(); //Initialize UART

	nvs_flash_init(); //Flash Initialize

	Init_Settings(); //Init Files


	Start_DUPS(); //Start the Dups Services


	InitKPI(); //Initialize KPI

	mqtt_app_start(); //MQTT start

	wifi_connect = true; //Disable Wi-Fi power save mode

	wifi_conn_init(); //Initialize Wi-Fi device.

}
