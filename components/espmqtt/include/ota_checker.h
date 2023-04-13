/** @file ota_checker.h
 *  @brief This files download the ota file from the server using rest call,
 *  OTA stream data stored in SPIFFS.
 *  @author Duraisamy Pandurangan (V-Guard Industrial Ltd,R&D)
 *  @Created Mar 2,2018
 */

/************************Headers includes*********************/
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_ota_ops.h"

#include "nvs.h"
#include "nvs_flash.h"
#include "esp_spiffs.h"

//#include "File.h"
/*************************Defines*******************/
//http://vguardbox.com:8080
//#define TestServer
#define EXAMPLE_SERVER_IP  "10.1.62.87"/*!< Hold the test server ip*/
#define EXAMPLE_SERVER_PORT "8080"/*!<Hold the test server port*/
//#define EXAMPLE_FILENAME ""
#define BUFFSIZE 1024/*!<Hold the OTA buffer size*/
#define text_ota_BUFFSIZE 1024/*!<OTA Text buffer text size*/
/*Variable Declaration*/
static const char *OTA_TAG = "OTA";/*!<Display Debug console log TAG*/
static char ota_write_data[BUFFSIZE + 1] = { 0 };/*!<Hold the OTA buffer*/
static char text_ota[BUFFSIZE + 1] = { 0 };/*!<OTA Text buffer text size*/
int binary_file_length = 0;/*!< hold the download binary file size*/
int percentage = 10;/*!<hold the download file percentage DIsp*/

static int socket_id = -1;/*!<Hold the server socket connection id*/
char ServerIpadress[20];/*!<Hold the production server address*/

/*************Function declarations*****************/
//static void initialise_wifi(void);
static bool connect_to_http_server();
static bool read_past_http_header(char text_ota[], int total_len, esp_ota_handle_t update_handle);
static esp_err_t event_handler(void *ctx, system_event_t *event);
static int read_until(char *buffer, char delim, int len);
static void ota_check(char FileName[]);



ip_addr_t Url_ip_Addr;

unsigned int Signal_check_all[100][6],Signal_check_all_max_cnt=0;

 void dns_found_cb(const char *name, const ip_addr_t *ipaddr, void *callback_arg)
 {
     Url_ip_Addr = *ipaddr;

     sprintf(ServerIpadress, "%i.%i.%i.%i",
                    ip4_addr1(&Url_ip_Addr.u_addr.ip4),
                    ip4_addr2(&Url_ip_Addr.u_addr.ip4),
                    ip4_addr3(&Url_ip_Addr.u_addr.ip4),
                    ip4_addr4(&Url_ip_Addr.u_addr.ip4) );
                printf("\n%s\n",ServerIpadress);
 }
 void Get_IP_From_Url() {
     dns_init();
     IP_ADDR4(&Url_ip_Addr, 0, 0, 0, 0);
     printf("\nDNS Checking Start\n");



    err_t er = dns_gethostbyname("vguardbox.com", &Url_ip_Addr, dns_found_cb,
     NULL);
     if (er==0) {
         sprintf(ServerIpadress, "%i.%i.%i.%i",
                        ip4_addr1(&Url_ip_Addr.u_addr.ip4),
                        ip4_addr2(&Url_ip_Addr.u_addr.ip4),
                        ip4_addr3(&Url_ip_Addr.u_addr.ip4),
                        ip4_addr4(&Url_ip_Addr.u_addr.ip4) );
                  //  printf("\n%s\n",ServerIpadress);
         printf("IP : %s\n", ServerIpadress);
     }



 }






/*
 * @Brief connect to http server
 */
static bool connect_to_http_server()
{
#ifdef TestServer
	ESP_LOGI(OTA_TAG, "Server IP: %s Server Port:%s", EXAMPLE_SERVER_IP, EXAMPLE_SERVER_PORT);
#else
	//ESP_LOGI(OTA_TAG, "Server IP: %s Server Port:%s", (char *)ServerIpadress, EXAMPLE_SERVER_PORT);
#endif


	int  http_connect_flag = -1;
	struct sockaddr_in sock_info;

	socket_id = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_id == -1) {
		ESP_LOGE(OTA_TAG, "Create socket failed!");
		return false;
	}

	// set connect info
	memset(&sock_info, 0, sizeof(struct sockaddr_in));
	sock_info.sin_family = AF_INET;


#ifdef TestServer

	//For Testing Server
	sock_info.sin_addr.s_addr = inet_addr((char *)EXAMPLE_SERVER_IP);
#else
	//For Production
	sock_info.sin_addr.s_addr = inet_addr((char *)ServerIpadress);
#endif

	sock_info.sin_port = htons(atoi(EXAMPLE_SERVER_PORT));

	// connect to http server
	http_connect_flag = connect(socket_id, (struct sockaddr *)&sock_info, sizeof(sock_info));
	if (http_connect_flag == -1) {
		ESP_LOGE(OTA_TAG, "Connect to server failed! errno=%d", errno);
		close(socket_id);
		return false;
	} else {
		ESP_LOGI(OTA_TAG, "Connected to server");
		return true;
	}
	return false;
}
/*
 *@brief Http header parser from the server,
 */
static bool read_past_http_header(char text_ota[], int total_len, esp_ota_handle_t update_handle)
{
	/* i means current position */
	int i = 0, i_read_len = 0;
	while (text_ota[i] != 0 && i < total_len) {
		i_read_len = read_until(&text_ota[i], '\n', total_len);
		// if we resolve \r\n line,we think packet header is finished
		if (i_read_len == 2) {
			int i_write_len = total_len - (i + 2);
			memset(ota_write_data, 0, BUFFSIZE);
			/*copy first http packet body to write buffer*/
			memcpy(ota_write_data, &(text_ota[i + 2]), i_write_len);

			esp_err_t err = esp_ota_write( update_handle, (const void *)ota_write_data, i_write_len);
			if (err != ESP_OK) {
				ESP_LOGE(OTA_TAG, "Error: esp_ota_write failed! err=0x%x", err);
				return false;
			} else {
				ESP_LOGI(OTA_TAG, "esp_ota_write header OK");
				binary_file_length += i_write_len;
			}
			return true;
		}
		i += i_read_len;
	}
	return false;
}

/**
 * @brief read the End of the OTA files.
 */
static int read_until(char *buffer, char delim, int len)
{
	//  /*TODO: delim check,buffer check,further: do an buffer length limited*/
	int i = 0;
	while (buffer[i] != delim && i < len) {
		++i;
	}
	return i + 1;
}
/**
 * @brief check the OTA file in the server.
 * @param OTAFile - download file from the server.
 */

static void ota_check(char OTAfile[])
{

	binary_file_length = 0;
	percentage = 10;

	uint count = 0;
	esp_err_t err;
	esp_ota_handle_t update_handle = 0;
	const esp_partition_t *update_partition=NULL;

	printf("FileName:%s\n",OTAfile);
	err = nvs_flash_init();
	if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
		//OTA app partition table has a smaller NVS partition size than the non-OTA
		//partition table. This size mismatch may cause NVS initialization to fail.
		//If this happens, we erase NVS partition and initialize NVS again.
		ESP_ERROR_CHECK(nvs_flash_erase());
		err = nvs_flash_init();
	}
	ESP_ERROR_CHECK( err );
	//Initialize wifi
	//initialise_wifi();
	ESP_LOGI(OTA_TAG, "Starting OTA");
	const esp_partition_t *configured = esp_ota_get_boot_partition();
	const esp_partition_t *running = esp_ota_get_running_partition();

	if (configured != running) {
		ESP_LOGW(OTA_TAG, "Configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x",
				configured->address, running->address);
		ESP_LOGW(OTA_TAG, "(This can happen if either the OTA boot data or preferred boot image become corrupted somehow.)");
	}
	ESP_LOGI(OTA_TAG, "Running partition type %d subtype %d (offset 0x%08x)",
			running->type, running->subtype, running->address);

	/* Wait for the callback to set the CONNECTED_BIT in the
       event group.
	 */
	// xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,false, true, portMAX_DELAY);
	ESP_LOGI(OTA_TAG, "Connect to Wifi ! Start to Connect to Server....");

	/*connect to http server*/
	if (connect_to_http_server()) {
		ESP_LOGI(OTA_TAG, "Connected to http server");
	} else {
		esp_mqtt_client_publish(DUPS_client, Pub_Topic,"VG990:-3",8, 0, 0);
		ESP_LOGE(OTA_TAG, "Connect to http server failed!");
		close(socket_id);
		return;/*exit*/
	}
	/*send GET request to http server*/
	const char *GET_FORMAT =
			"GET %s HTTP/1.0\r\n"
			"Host: %s:%s\r\n"
			"User-Agent: esp-idf/1.0 esp32\r\n\r\n";

	char *http_request = NULL;
	int get_len = asprintf(&http_request, GET_FORMAT,(char *)OTAfile, ServerIpadress, EXAMPLE_SERVER_PORT);
	if (get_len < 0) {
		ESP_LOGE(OTA_TAG, "Failed to allocate memory for GET request buffer");
		close(socket_id);
		return;
	}
	int res = send(socket_id, http_request, get_len, 0);
	free(http_request);

	if (res < 0)
	{
		ESP_LOGE(OTA_TAG, "Send GET request to server failed");
		close(socket_id);
		return;
	}
	else
	{
		ESP_LOGI(OTA_TAG, "Send GET request to server succeeded");
	}
	update_partition = esp_ota_get_next_update_partition(NULL);
	ESP_LOGI(OTA_TAG, "Writing to partition subtype %d at offset 0x%x",
			update_partition->subtype, update_partition->address);
	assert(update_partition != NULL);

	err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
	if (err != ESP_OK) {
		esp_mqtt_client_publish(DUPS_client, Pub_Topic,"VG990:-3",8, 0, 0);
		ESP_LOGE(OTA_TAG, "esp_ota_begin failed, error=%d", err);
		close(socket_id);
		return;
	}
	ESP_LOGI(OTA_TAG, "esp_ota_begin succeeded");

	bool resp_body_start = false, flag = true;
	char OTA_message[50];

	struct timeval timeout;
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;

	int setsockopt_1 = setsockopt (socket_id, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
			                sizeof(timeout));

	/*deal with all receive packet*/
	while (flag)
	{


		count++;
		if(count == 0x30){

			percentage = binary_file_length/10240;

			sprintf((char *)OTA_message,"VG990:%d",percentage);
			if(percentage != 1)
			esp_mqtt_client_publish(DUPS_client, Pub_Topic,(char *)OTA_message,strlen((char *)OTA_message), 0, 0);
			count = 0;
		}




//			printf("sock:%d\n",setsockopt_1);


		int buff_len = recv(socket_id, text_ota, text_ota_BUFFSIZE, 0);

		if (buff_len < 0)
		{ /*receive error*/

			esp_mqtt_client_publish(DUPS_client, Pub_Topic,"VG990:-1",8, 0, 0);
			close(socket_id);
			flag = false;
			return;
		}
		else if (buff_len > 0 && !resp_body_start)
		{ /*deal with response header*/
			memcpy(ota_write_data, text_ota, buff_len);
			resp_body_start = read_past_http_header(text_ota, buff_len, update_handle);
		}
		else if (buff_len > 0 && resp_body_start)
		{ /*deal with response body*/
			memcpy(ota_write_data, text_ota, buff_len);
			err = esp_ota_write( update_handle, (const void *)ota_write_data, buff_len);
			if (err != ESP_OK)
			{
				esp_mqtt_client_publish(DUPS_client, Pub_Topic,"VG990:-2",8, 0, 0);

				ESP_LOGE(OTA_TAG, "Error: esp_ota_write failed! err=0x%x", err);
				close(socket_id);
				return;
			}
			binary_file_length += buff_len;
			ESP_LOGI(OTA_TAG, "Have written image length %d", binary_file_length);
		}
		else if (buff_len == 0)
		{  /*packet over*/
			flag = false;
			ESP_LOGI(OTA_TAG, "Connection closed, all packets received");
			close(socket_id);
		}
		else
		{
			ESP_LOGE(OTA_TAG, "Unexpected recv result");
		}
	}

	ESP_LOGI(OTA_TAG, "Total Write binary data length : %d", binary_file_length);


	if (esp_ota_end(update_handle) != ESP_OK)
	{
		esp_mqtt_client_publish(DUPS_client, Pub_Topic,"VG990:-4",8, 0, 0);

		ESP_LOGE(OTA_TAG, "esp_ota_end failed!");
		close(socket_id);
		return;
	}
	err = esp_ota_set_boot_partition(update_partition);
	if (err != ESP_OK)
	{
		esp_mqtt_client_publish(DUPS_client, Pub_Topic,"VG990:-5",8, 0, 0);
		ESP_LOGE(OTA_TAG, "esp_ota_set_boot_partition failed! err=0x%x", err);
		close(socket_id);
		return;
	}
	esp_mqtt_client_publish(DUPS_client, Pub_Topic,"VG990:1",7, 0, 0);

	ESP_LOGI(OTA_TAG, "Prepare to restart system!");
	esp_restart();
	return ;
}

