/*
 *@file File.h
 *
 *@brief This library provides read and data from the SPIFFS.
 *
 *  Created on: Dec 29, 2017
 *      Author: Duraisamy P
  * @modified by Binu Antony (V-Guard Industrial Ltd,R&D)
 *  @modifed on:17-11-20
 */
/*Header includes*/
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "gatts_table_creat_demo.h"
#include "ini.h"


static const char *TAG = "SPI_FLASH"; /*!<Debug Display Tag*/

/*Function Declaration*/
void parse(const char* name, const char* string);
void factoryResetFlash(void);

/*Ini parsing support package Variable*/
int User;
char Prev_section[50];

typedef struct
{
	const char* data;
} configuration;

/*End of Ini File support variable*/

/**
 * @brief the check the partition table table is created or not and mount the SPIFFS in user space.
 * if Partition is not create ,then it will create partition and system will reboot.
 *
 * @return 1 Partition already created and mounted in user space
 */

int Mount_Flash(){


	ESP_LOGI(TAG, "Initializing SPIFFS--");
	/*Initializing SPIFFS*/
	esp_vfs_spiffs_conf_t conf = {
			.base_path = "/spiffs",
			.partition_label = NULL,
			.max_files = 5,
			.format_if_mount_failed = true
	};

	// Use settings defined above to initialize and mount SPIFFS filesystem.
	// Note: esp_vfs_spiffs_register is an all-in-one convenience function.
	esp_err_t ret = esp_vfs_spiffs_register(&conf);

	if (ret != ESP_OK) {
		if (ret == ESP_FAIL) {
			ESP_LOGE(TAG, "Failed to mount or format filesystem");
		} else if (ret == ESP_ERR_NOT_FOUND) {
			ESP_LOGE(TAG, "Failed to find SPIFFS partition");
		} else {
			ESP_LOGE(TAG, "Failed to initialize SPIFFS (%d)", ret);
		}
		/*Reboot the system*/
		esp_restart();

		return -1;
	}


	size_t total = 0, used = 0;
	/*Get the information about the flash and mount into user space*/
	ret = esp_spiffs_info(NULL, &total, &used);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "Failed to get SPIFFS partition information");
	} else {
		ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
	}

	return 1;
}
/*
 * @brief Read_SystemConf open file from SPIFFS.parse the data into variable
 * @param Filename
 * @return -2 Failed to open file
 * @return 1 Successfully open the file and parsed data into variable.
 *
 */
uint8_t Read_SystemConf(char* file){

	/*Mount the SPIFFS*/
	if(Mount_Flash() == 1){
		ESP_LOGI(TAG, "Opening file");
		FILE* f = fopen(file,"r");

		if (f == NULL) {
			ESP_LOGE(TAG, "Failed to open file for Reading");
			esp_vfs_spiffs_unregister(NULL);
			return -2;
		}

		char line[64];
		char *gaucCmpBuf;

		gaucCmpBuf = (char *)malloc(500 * sizeof(char));

		//  fgets(gaucCmpBuf, 300, f);
		// strip newline

		memset(line,0,sizeof(line));
		strcpy(gaucCmpBuf,"");
		/*Read the data line by line upto end of file*/
		while(fgets(line, sizeof(line), f)) {
			strcat(gaucCmpBuf,line);
			//printf("%s\n",line);
		}
		memset(line,0,sizeof(line));



		//	ESP_LOGI(TAG, "Parsing to : parse function:%s",gaucCmpBuf);
		/*parse file value into ini process*/
		parse("long continued", (const char *)gaucCmpBuf);

		free(gaucCmpBuf);
		/*Close the file*/
		fclose(f);
		/*unmount the SPIFFS*/
		esp_vfs_spiffs_unregister(NULL);

	}else{

		ESP_LOGE(TAG, "Unable to mount the Partition");

	}


	esp_vfs_spiffs_unregister(NULL);
	return 1;

}

/**
 * @brief This is callback Function is used to handled the ini parsing files
 *
 * return 1 for sucess
 * return 0 unknow selection error
 */

static int handler(void* user, const char* section, const char* name,
		const char* value)
{
	configuration* pconfig = (configuration*)user;

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0



	if(strcmp(section,"Setting") == 0){
		if (MATCH("Setting", "BrokerAddress")) {
			pconfig->data = strdup(value);
			strcpy((char *)Userdata.BrokerAddress,pconfig->data);

		} else if (MATCH("Setting", "BrokerPort")) {

			pconfig->data = strdup(value);
			strcpy((char *)Userdata.BrokerPort,pconfig->data);

		} else if (MATCH("Setting", "ServerUname")) {

			pconfig->data = strdup(value);
			strcpy((char *)Userdata.serverUname,pconfig->data);

		}else if (MATCH("Setting", "Serverpass")) {
			pconfig->data = strdup(value);
			strcpy((char *)Userdata.serverpass,pconfig->data);

		} else if (MATCH("Setting", "SSID")) {
			pconfig->data = strdup(value);
			strcpy((char *)Userdata.Wifi_ssid,pconfig->data);

		}else if (MATCH("Setting", "WifiPass")) {
			pconfig->data = strdup(value);
			strcpy((char *)Userdata.wifi_password,pconfig->data);
		}else if (MATCH("Setting", "INT")) {
			pconfig->data = strdup(value);
			strcpy((char *)Userdata.interval,pconfig->data);
		}else if (MATCH("Setting", "PowerSave")) {
			Userdata.PowerSave = atoi(value);
		}else if (MATCH("Setting", "ResetTimer")) {
			Userdata.ResetTimer = atoi(value);
		}else if (MATCH("Setting", "Serial")) {
			pconfig->data = strdup(value);
			strcpy((char *)Userdata.Serial_number,pconfig->data);
		}else if (MATCH("Setting", "GPSlat")) {
			pconfig->data = strdup(value);
			strcpy((char *)Userdata.GPS_latitude,pconfig->data);
		}else if (MATCH("Setting", "GPSlon")) {
			pconfig->data = strdup(value);
			strcpy((char *)Userdata.GPS_Longitude,pconfig->data);
		}else {
			ESP_LOGI(TAG, "unknow selection:%s", pconfig->data);

			return 0;  /* unknown section/name, error */
		}

	}else if(strcmp(section,"Inverter") == 0){
		if (MATCH("Inverter", "Data_log")) {
			Inverter.Data_log = atoi(value);
		}else if (MATCH("Inverter", "Key")) {
			pconfig->data = strdup(value);
			strcpy((char *)Inverter.Key,pconfig->data);
		}else if (MATCH("Inverter", "IV")) {
			pconfig->data = strdup(value);
			strcpy((char *)Inverter.IV,pconfig->data);
		}else if (MATCH("Inverter", "TU")) {
			Inverter.Totalunit = atoi(value);
		}else {
			ESP_LOGI(TAG, "unknow selection:%s", pconfig->data);

			return 0;  /* unknown section/name, error */
		}
	}
	return 1;
}

/**
 *@brief  parse the string form of Ini based frame file.
 *Argument string ,
 *Argument Matching word
 *Return void
 */

void parse(const char* name, const char* string) {

	configuration config;

	*Prev_section = '\0';
	ini_parse_string(string, handler, &config);
}


/**
 *@brief this function is used to write the setting parameters data in Setting.ini file
 *@return -1 unable to open the partition
 *@return -2 unable to create a new file
 *@return 1 Successfully saved into SPIFFS
 */
uint8_t Write_systemConf(){

	if(Mount_Flash() !=1)
		return -1;

	FILE* f;

	struct stat st,old;
	/*Check the status of the file :system.ini*/
	uint8_t filestat= stat("/spiffs/system.ini", &st);
	/*check the status of the fileV1.0 supports:sytem.ini*/
	uint8_t filestat_old= stat("/spiffs/sytem.ini", &old);

	if (filestat == 0 || filestat_old ==0 ) {

		if(stat("/spiffs/system.ini", &st) == 0)
			f = fopen("/spiffs/system.ini", "w");
		else
			f = fopen("/spiffs/sytem.ini", "w"); // For Support Released field firmware version 1.0.

		if (f == NULL) {
			ESP_LOGE(TAG, "Failed to Create file for writing");
			esp_vfs_spiffs_unregister(NULL);
			return -2;
		}
		char *gaucCmpBuf,*temp;;

		gaucCmpBuf = (char *)malloc(500 * sizeof(char));
		temp = (char *)malloc(20 * sizeof(char));

		strcpy((char *)gaucCmpBuf,"[Setting]");

		strcat((char *)gaucCmpBuf,"\nBrokerAddress = ");
		strcat((char *)gaucCmpBuf,(const char *)Userdata.BrokerAddress);

		strcat((char *)gaucCmpBuf,"\nBrokerPort = ");
		strcat((char *)gaucCmpBuf,(const char *)Userdata.BrokerPort);

		strcat((char *)gaucCmpBuf,"\nServerUname = ");
		strcat((char *)gaucCmpBuf,(const char *)Userdata.serverUname);

		strcat((char *)gaucCmpBuf,"\nServerpass = ");
		strcat((char *)gaucCmpBuf,(const char *)Userdata.serverpass);

		strcat((char *)gaucCmpBuf,"\nSSID = ");
		strcat((char *)gaucCmpBuf,(const char *)Userdata.Wifi_ssid);

		strcat((char *)gaucCmpBuf,"\nWifiPass = ");
		strcat((char *)gaucCmpBuf,(const char *)Userdata.wifi_password);

		strcat((char *)gaucCmpBuf,"\nSerial = ");
		strcat((char *)gaucCmpBuf,(const char *)Userdata.Serial_number);

		strcat((char *)gaucCmpBuf,"\nINT = ");
		strcat((char *)gaucCmpBuf,(const char *)Userdata.interval);

		strcat((char *)gaucCmpBuf,"\nPowerSave = ");
		sprintf((char *)temp,"%d",Userdata.PowerSave);

		strcat((char *)gaucCmpBuf,temp);

		strcat((char *)gaucCmpBuf,"\nResetTimer = ");
		sprintf((char *)temp,"%d",Userdata.ResetTimer);

		strcat((char *)gaucCmpBuf,temp);

		strcat((char *)gaucCmpBuf,"\nGPSlon = ");
		sprintf((char *)temp,"%s",Userdata.GPS_Longitude);

		strcat((char *)gaucCmpBuf,temp);

		strcat((char *)gaucCmpBuf,"\nGPSlat = ");
		sprintf((char *)temp,"%s",Userdata.GPS_latitude);

		strcat((char *)gaucCmpBuf,temp);


		fprintf(f,gaucCmpBuf);
		free(gaucCmpBuf);

		fclose(f);



	}else{  //Set defualt system values in Flash memory

		ESP_LOGE(TAG, "Creating new file ");

		f = fopen("/spiffs/system.ini", "w");
		if (f == NULL) {
			ESP_LOGE(TAG, "Failed to Create file for writing");
			esp_vfs_spiffs_unregister(NULL);
			return -2;
		}

		char *gaucCmpBuf;

		gaucCmpBuf = (char *)malloc(500 * sizeof(char));

		strcpy((char *)gaucCmpBuf,"[Setting]");

		strcat((char *)gaucCmpBuf,"\nBrokerAddress = ");
		strcat((char *)gaucCmpBuf,"vguardbox.com");

		strcat((char *)gaucCmpBuf,"\nBrokerPort = ");
		strcat((char *)gaucCmpBuf,"8883");

		strcat((char *)gaucCmpBuf,"\nServerUname = ");
		strcat((char *)gaucCmpBuf,"vguard");

		strcat((char *)gaucCmpBuf,"\nServerpass = ");
		strcat((char *)gaucCmpBuf,"vguard1234");

		strcat((char *)gaucCmpBuf,"\nSSID = ");

		strcat((char *)gaucCmpBuf,"\nWifiPass = ");

		strcat((char *)gaucCmpBuf,"\nSerial =  ");

		strcat((char *)gaucCmpBuf,"\nINT = 100");

		strcat((char *)gaucCmpBuf,"\nPowerSave = OFF");

		strcat((char *)gaucCmpBuf,"\nGPSlon = NA");

		strcat((char *)gaucCmpBuf,"\nGPSlat = NA");


		fprintf(f,gaucCmpBuf);
		fclose(f);
		free(gaucCmpBuf);

	}
	// All done, unmount partition and disable SPIFFS
	esp_vfs_spiffs_unregister(NULL);

	return 1;

}
/**
 *@brief write inverter information into the flash.
 *@return -1 unable to mount the partition
 *@return -2 unable to create a new file
 *@return 1 successfully written into the flash
 */
uint8_t Write_Inverter(){

	if(Mount_Flash() !=1)
		return -1;

	FILE* f;
	struct stat st;
	if (stat("/spiffs/Inverter.ini", &st) == 0) {

		ESP_LOGE(TAG, "File already available");

		f = fopen("/spiffs/Inverter.ini", "w");
		if (f == NULL) {
			ESP_LOGE(TAG, "Failed to Create file for writing");
			esp_vfs_spiffs_unregister(NULL);
			return -2;
		}
		char *gaucCmpBuf,*temp;;

		gaucCmpBuf = (char *)malloc(300 * sizeof(char));
		temp = (char *)malloc(20 * sizeof(char));

		strcpy((char *)gaucCmpBuf,"[Inverter]");


		strcat((char *)gaucCmpBuf,"\nData_log = ");
		sprintf((char *)temp,"%d",Inverter.Data_log);

		strcat((char *)gaucCmpBuf,temp);

		strcat((char *)gaucCmpBuf,"\nKey = ");
		strcat((char *)gaucCmpBuf,(const char *)Inverter.Key);

		strcat((char *)gaucCmpBuf,"\nIV = ");
		strcat((char *)gaucCmpBuf,(const char *)Inverter.IV);

		strcat((char *)gaucCmpBuf,"\nTU = ");
		sprintf((char *)temp,"%d",Inverter.Totalunit);
		strcat((char *)gaucCmpBuf,temp);

		fprintf(f,gaucCmpBuf);
		free(gaucCmpBuf);

		fclose(f);



	}else{  //Set defualt system values in Flash memory

		ESP_LOGE(TAG, "Creating new file ");

		f = fopen("/spiffs/Inverter.ini", "w");
		if (f == NULL) {
			ESP_LOGE(TAG, "Failed to Create file for writing");
			esp_vfs_spiffs_unregister(NULL);
			return -2;
		}

		char *gaucCmpBuf;

		gaucCmpBuf = (char *)malloc(300 * sizeof(char));

		strcpy((char *)gaucCmpBuf,"[Inverter]");


		strcat((char *)gaucCmpBuf,"\nData_log = 0");
		strcat((char *)gaucCmpBuf,"\nKey = NA");
		strcat((char *)gaucCmpBuf,"\nIV = NA");
		strcat((char *)gaucCmpBuf,"\nTU = 0");

		fprintf(f,gaucCmpBuf);
		fclose(f);
		free(gaucCmpBuf);

	}

	// All done, unmount partition and disable SPIFFS
	esp_vfs_spiffs_unregister(NULL);
	return 1;

}
/**********************************************************************************************************/
/**
 * @brief Function is for factory resetting flash. 
 * @return void
 */
void factoryResetFlash(void)
{
	esp_err_t ret;
	if(Mount_Flash()!=1)
		return;
	ret=esp_spiffs_format(NULL);
	if(ret==ESP_OK)
	{
		ESP_LOGI("FACTORY","SPIFFS FORMAT SUCCESS");
	}
	else
	{
		ESP_LOGI("FACTORY","SPIFFS FORMAT FAILED");
	}
	esp_vfs_spiffs_unregister(NULL);
	esp_restart();
}
/**********************************************************************************************************/
/**
 * @brief init the system from the SPIFFS, Create call for SPIFFS format,file creation and File read
 * * @return void
 */
void Init_Settings(void){


	/*Mount SPIFSS */
	if(Mount_Flash() !=1)
		return;//return -1;
	struct stat st;

	/*set Default Lat and Lon filesystem*/
	strcpy((char *)Userdata.GPS_latitude,"NA");
	strcpy((char *)Userdata.GPS_Longitude,"NA");
	Inverter.Data_log = 0;

	uint8_t filestat= stat("/spiffs/system.ini", &st);
	uint8_t filestat_old= stat("/spiffs/sytem.ini", &st);
	/*unmount the SPIFSS*/
	esp_vfs_spiffs_unregister(NULL);


	if ( filestat == 0){ //If latest DupsFirmware V1.1 files

		ESP_LOGI(TAG, "Reading Existing File configuration:%d", Read_SystemConf("/spiffs/system.ini"));

	}else if(filestat_old == 0){ // For Support Released field firmware version 1.0
		ESP_LOGI(TAG, "Reading Existing File configuration:%d", Read_SystemConf("/spiffs/sytem.ini"));

	}else{//Create new system files
		ESP_LOGI(TAG, "Create Defualt File configuration:%d", Write_systemConf());
		ESP_LOGI(TAG, "Reading Existing File configuration:%d", Read_SystemConf("/spiffs/system.ini"));


	}

	if(Mount_Flash() !=1)
		return;//return -1;

	filestat= stat("/spiffs/Inverter.ini", &st);
	esp_vfs_spiffs_unregister(NULL);

	if ( filestat == 0){

		ESP_LOGI(TAG, "Reading Existing File configuration:%d", Read_SystemConf("/spiffs/Inverter.ini"));

	}else{
		ESP_LOGI(TAG, "Create Defualt File configuration:%d", Write_Inverter());
		ESP_LOGI(TAG, "Reading Existing File configuration:%d", Read_SystemConf("/spiffs/Inverter.ini"));


	}
	return;
}

