/** @file DUPS_Communication.h
 *  @brief Function prototypes for the DUPS communication and Data collection handling.
 *
 *  This contains the Ble and Wifi connection pipeline handling, Creating many task to handle power save mode,
 *  date time sync and etc.
 *  Communication and eventually any macros, constants, or global variables you will need.
 *
 *  @author Duraisamy Pandurangan (V-Guard Industrial Ltd,R&D)
 *  @modified by Binu Antony (V-Guard Industrial Ltd,R&D)
 *  @modifed on:17-11-20
 */

#ifndef DUPS_COMMUNICATION_H_
#define DUPS_COMMUNICATION_H_
/**Header Includes************/
#include "mqtt_client.h"
#include "diagnostic.h"
#include "esp_bt.h"
#include <sys/time.h>
#include "esp_system.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"
#include "gatts_table_creat_demo.h"
#include <string.h>
#include "AES_CBC.h"
#include "kpiprocess.h"
#define  INPUT 				0
#define  OUTPUT 			1
#define  FACTORY_RESET_PIN	01//13 in ESP WROOM 02
#define  FACTORY_RST_COUNT	5
#define  FACTORY_PIN_SET	(1ULL<<FACTORY_RESET_PIN)
/**
 * Macro to check the outputs of TWDT functions and trigger an abort if an
 * incorrect code is returned.
 */
#define CHECK_ERROR_CODE(returned, expected) ({                        \
		if(returned != expected){                                  \
			printf("TWDT ERROR\n");                                \
			abort();                                               \
		}                                                          \
})

/*Function Declaration */
void Write_Data(int Registry, int Val, bool Write);
void wifi_conn_init(void);
void SendMessage(char *PublishMessage);
#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))
extern uint8_t Write_systemConf();
extern void factoryResetFlash();
extern void RX_hexbytes(uint8_t *bytes);
void factoryPINConfig(uint8_t mode);
void factoryResetCheck(void);
RTC_NOINIT_ATTR uint8_t timeupdate;
/* Preprocessor Declaration*/
#define WIFI_LED 10/*!< GPIO 10 is used for External Hardware Watchdog Control*/
#define ActualJson "{\"VG029\":{"/*!< Holds the Actual packet Json Root values */
#define InfoJson "{\"VG076\":{"/*!< Holds the Info packet Json Root values */
//Watchdog Timeout
#define TWDT_TIMEOUT_S          20 /*!< Watchdog Timeout duration in Sec */
/*Public Variable declaration****/
uint8_t factoryCount = 0;

bool SolarFCM_Flag1 = false, SolarFCM_Flag2 = false, Time_out_Flag = false;
int App_Timeout = -1;
enum timeCheckFlags {
	TIME_CHECK_RESET,
	TIME_CHECK_STARTED,
	TIME_CHECK_REQUESTED,
	TIME_CHECK_SUCCESS,
	TIME_CHECK_UPDATED
};
enum timeCheckFlags timeCheckMode = TIME_CHECK_RESET;
bool timeFlag = false, Data_log_flag = false, Flag15 = 0;
int Data_log_Count = 0, Flag15_Counter = 0, Flag15_Time = 0;
bool battery;/*!< Holds the Battery Read status */
bool wifi_connect = false, Device_syn = false, AlarmStat = false;
volatile bool Statusflag = false, TotalWrite = true;
bool wifisystem_flag = false, Lock_system = true, Today_Unit_OK = 0, Jig_Falg =
		0;
bool model = false, wdt_true = false, solarkpi = false, Secured = false,
		ReadAllParam = true, Learning;
uint8_t AlarmCount = 0, AlarmCount_184 = 0, AlarmCount_186 = 0;/*!< Holds the FCM double check verification status*/
int AlarmHour = 0, SolarHour = 0, Solar186_hour = 0;
uint8_t volatile bootcalled = 0;/*!< Holds the WiFi data polling service status*/
uint8_t Maintenance = 0;/*!< Holds the system Reboot status*/
static uint8_t forcecut = 0;/*!< Holds the Mains force cut status */
uint8_t No_of_battery = 31; // 20210917 increase the battery count to 31 from 10  10;/*!< Holds the inverter no of battery connected status */
uint8_t Req_address[2];/*!< Holds the inverter Request address LSB,MSB */
uint16_t AlarmStatus, SolarAlarmStatus;/*!< Hold the alarm and solar alarm status */
uint16_t Pr_AlarmStatus = 0, Pr_SolarAlarmStatus = 0;

uint16_t Today_Power_Failure_Count = 0;
uint16_t Today_Power_Failure_Duration = 0;

char inverter_model[10];/*!< Holds the Inverter model status*/
char inverter_mod[15], ModelCode[5], Serial[10], PgVersion[10],
		ManVendorCode[10];/*!< Holds Single read registry inverter details*/
char Trends_values_c[50], Trends_values_d[50];/*!< Holds the Trends charts values*/
char Pub_Topic[50];/*!< Holds the Data publish topic  */
char Wifi_address[13];/*!< Holds the WiFi Mac address */
int wifi_strength;/*!< Holds the WiFi signal strenth */
bool Restrt_falg = 0;
char *Info_Json, *Actual_Json, *BootData;/*!< Create the Cache Memory variable for Data pushing */
int read_data = 0;/*!< Holds the data reading status */
int Boot;/*!< Holds the Running reading KPI values */
int connection_error = 1; /*!< Holds the Server Connectivity status */
int Data_log = 0;/*!< Holds the data log bits status,KPI is VG041 */
int Min_Vol = 9, Max_Vol = 11; /*!< Holds the Battery power save mode min and maximum power save value, For default  Min 9V and Max 11V*/
int exist = 0;/*!< Holds the data in pipeline and yet to process */
int TotalUnit;
/************** RTC variable declaration**********/
time_t now;
struct tm timeinfo;/*!<Create the time variable for RTC global Access */
static TaskHandle_t task_handles[portNUM_PROCESSORS];/*!< Task handler for Watchdog Thread*/
static TaskHandle_t xLow_Power = NULL;/*!< Task handler for Xlow power Thread*/
static TaskHandle_t xPowerTask = NULL;/*!< Task handler for Power Task Thread*/
static TaskHandle_t xTest_Jig_Task = NULL;
esp_mqtt_client_handle_t DUPS_client = NULL; /*!<Create the DUPS client Struct member for global Access */
/**
 *Enumeration of KPIList
 */
enum KPILIST {
	LED_Disp = 1,/*!Solar inverter Writing Register 0x22,0X0C*/
	Serial_number = 3,/*! inverter Register 0xc8,0X0b , KPI = VG003*/
	Program_version = 10,/*! inverter Register 0xca,0X0b , KPI = VG010*/
	Manufacturing_vendor_code = 13,/*! inverter Register 0xce,0X0b , KPI = VG013*/
	Mains_Voltage = 14,/*! inverter Register 0x08,0x0C , KPI = VG014*/
	Output_voltage = 15,/*! inverter Register 0x0a,0x0C , KPI = VG015*/
	Battery_Voltage = 16,/*! inverter Register 0x06,0x0C , KPI = VG016*/
	Load_Current = 17,/*! inverter Register 0x0C,0x0C , KPI = VG017*/
	Charging_Current = 18,/*! inverter Register 0x10,0x0C , KPI = VG018*/
	Mains_frequency = 19,/*! inverter Register 0x12,0x0C, KPI = VG019*/
	Temperature_Value = 20,/*! inverter Register 0x0e,0x0C, KPI = VG020*/
	Mode = 21,/*! inverter Register 0x18,0x0C, KPI = VG021*/
	Battery_Type = 22,/*! inverter Register 0x1C,0x0C, KPI = VG022*/
	Charging = 23,/*! inverter Register 0x1A,0x0C, KPI = VG023*/
	Date_of_calibration = 24,/*! inverter Register 0xc4,0x0b, KPI = VG024*/
	Time_of_calibration = 25,/*! inverter Register 0xc2,0x0b, KPI = VG025*/
	Ah_count = 26,/*! inverter Register 0x3c,0x0C, KPI = VG026*/
	ALARMS = 33,/*! inverter Register 0x1e,0x0C, KPI = VG033*/
	Buzzer_settings = 34,/*! inverter Register 0x20,0x0C, KPI = VG033*/
	Voltage_regulator = 35,/*! inverter Register 0x24,0x0C, KPI = VG035*/
	Appliance_Mode = 36,/*! inverter Register 0x26,0x0C, KPI = VG036*/
	Mains_forced_cut_time = 37,/*! inverter Register 0x2a,0x0C, KPI = VG037*/
	Mains_force_cut_status = 38,/*! inverter Register 0x28,0x0C, KPI = VG038*/
	Inverter_Model = 42,/*! inverter Register0xb0,0x0b, KPI = VG038*/
	Slr_Clr_Day = 43,/*! inverter Register 0x82,0x0C, KPI = VG043*/
	PV_Voltage = 49,/*! inverter Register 0x78,0x0C, KPI = VG049*/
	Load_alarm_status = 50,/*! inverter Register 0x3a,0x0C, KPI = VG050*/
	Trends_values_1_d = 69,/*! inverter Register 0x50,0x0C, KPI = VG069 First place position  values*/
	Trends_values_1_c = 70,/*! inverter Register 0x40,0x0C, KPI = VG070 First place position  values*/
	Battery_remaining_alarm_status = 71,/*! inverter Register 0x3e,0x0C, KPI = VG071*/
	Extra_Backup_status = 72,/*! inverter Register 0x6e,0x0C, KPI = VG071*/
	Battery_type_change_timer_status = 73,/*! inverter Register 0x2e,0x0C, KPI = VG073*/
	Solar_Savings_Yesterday = 88,/*! inverter Register 0x7e,0x0C, KPI = VG088*/
	Solar_Savings_Total = 89,/*! inverter Register 0x7a,0x0C, KPI = VG089*/
	SWITCH_STAT = 94,/*! inverter Register 0x16,0x0C, KPI = VG094*/
	Present_status = 95,/*! inverter Register 0x1e,0x0C, KPI = VG095*/
	Load_percentage = 96,/*! inverter Register 0x2c,0x0C, KPI = VG096*/
	Battery_backup_time = 97,/*! inverter Register 0x38,0x0C, KPI = VG097*/
	Battery_percentage = 98,/*! inverter Register 0x3c,0x0C, KPI = VG098*/
	Turbo_Charging_status = 99,/*! inverter Register 0x30,0x0C, KPI = VG099*/
	Holiday_Mode_status = 100,/*! inverter Register 0x32,0x0C, KPI = VG100*/
	Battery_capacity = 104,/*! inverter Register 0xcc,0x0b, KPI = VG104*/
	Inverter_Time = 110,/*! inverter Register 0x60,0x0C, KPI=VG110*/
	No_Bat_cntd = 135,/*! inverter Register 0x74,0x0C, KPI = VG135*/
	Solar_Current = 141,/*! inverter Register 0x76,0x0C, KPI = VG141*/
	Total_AH = 144,/*! inverter Register 0xA0,0x0C, KPI = VG144*/
	AH_in = 146,/*! inverter Register 0x9E,0x0C, KPI = VG146*/
	Solar_Savings_Today = 174,/*! inverter Register 0x7c,0x0C, KPI = VG174*/
	Power_saver_switch = 183,/*! inverter Register 0x80,0x0C, KPI = VG183*/
	Solar_Alarms = 184,/*! inverter Register 0x90,0x0C, KPI = VG184*/
	Daily_Day_time_loadUsage = 185,/*! inverter Register 0x84,0x0C, KPI = VG185*/
	Cleaning_Reminder = 186,/*! inverter Register 0x8a,0x0C, KPI = VG186*/
	Total_days_of_installation = 197,/*! inverter Register 0x62,0x0C, KPI = VG197*/
	Total_overload_count = 198,/*! inverter Register 0x64,0x0C, KPI = VG198*/
	Total_Short_circuit_count = 199,/*! inverter Register 0x66,0x0C, KPI = VG199*/
	Total_battery_low_count = 200,/*! inverter Register 0x68,0x0C, KPI = VG200*/
	Water_Topping_remaining_days = 201,/*! inverter Register 0xFE,0x0B, KPI = VG201*/
	Mains_Charger_ON_OFF = 202,/*! inverter Register 0x70,0x0C, KPI = VG202*/
	Charging_Time = 206,/*! inverter Register 0x92,0x0C, KPI = VG206*/
	/* Add new Enum value more than maximum current server given value, Add it here*/
	//	Alarm_statusUpadate = 208, //For status update changes to report server
	//	SolarAlarm_statusUpadate = 209, //For status update changes to report server
	Reserved_KPI_6A = 211, /*! inverter Register 0x6A,0x0C,KPI  =211 */
	Solar_Reserved_Kpi_2 = 212,/*! inverter Register 0x94,0x0C, KPI = VG212*/
	Solar_Reserved_Kpi_3 = 213,/*! inverter Register 0x96,0x0C, KPI = VG213*/
	Solar_Reserved_Kpi_4 = 214,/*! inverter Register 0x98,0x0C, KPI = VG214*/
	Solar_Reserved_Kpi_5 = 215,/*! inverter Register 0x9A,0x0C, KPI = VG215*/
	Solar_Reserved_Kpi_6 = 216,/*! inverter Register 0x9C,0x0C, KPI = VG216*/
	Solar_Reserved_Kpi_9 = 217,/*! inverter Register 0xA2,0x0C, KPI = VG217*/
	Solar_Reserved_Kpi_10 = 218,/*! inverter Register 0xA4,0x0C, KPI = VG218*/
	Daily_Load_Start = 219,/*! inverter Register 0x86,0x0C, KPI = VG217*/
	Daily_Load_End = 220,/*! inverter Register 0x88,0x0C, KPI = VG218*/

	/*Add new Section Value End*/
	/*This KPI enum value automatically create . Do not assign Any values*/
	Trends_values_2_c,/*! inverter Register 0x42,0x0C, KPI = VG070,Second place position  values*/
	Trends_values_3_c,/*! inverter Register 0x44,0x0C, KPI = VG070,third place position  values*/
	Trends_values_4_c,/*! inverter Register 0x46,0x0C, KPI = VG070,fourth place position values*/
	Trends_values_5_c,/*! inverter Register 0x48,0x0C, KPI = VG070,Fifth place position values*/
	Trends_values_6_c,/*! inverter Register 0x4a,0x0C, KPI = VG070,Sixth place position values*/
	Trends_values_7_c,/*! inverter Register 0x4c,0x0C, KPI = VG070,Seventh place position values*/
	Trends_values_8_c,/*! inverter Register 0x4e,0x0C, KPI = VG070,Eighth place position values*/
	Trends_values_2_d,/*! inverter Register 0x52,0x0C, KPI = VG069,Second place position values*/
	Trends_values_3_d,/*! inverter Register 0x54,0x0C, KPI = VG069,Third place position values*/
	Trends_values_4_d,/*! inverter Register 0x56,0x0C, KPI = VG069,Fourth place position values*/
	Trends_values_5_d,/*! inverter Register 0x58,0x0C, KPI = VG069,Fifth place position values*/
	Trends_values_6_d,/*! inverter Register 0x5a,0x0C, KPI = VG069,Sixth place position values*/
	Trends_values_7_d,/*! inverter Register 0x5c,0x0C, KPI = VG069,Seventh place position values*/
	Trends_values_8_d,/*! inverter Register 0x5e,0x0C, KPI = VG069,Eighth place position values*/
	Trail_node/*<Not used*/
};
/** @brief Start the learn logic background,
 *  disable Write request as false, start the Switch read command to inverter.
 *  @return Void.
 */
void LearnLogic() {

	vTaskDelay(50 / portTICK_RATE_MS);
	Learning = true;
	WriteRequest = false;
	Req_address[0] = 0x16;
	Req_address[1] = 0x0c;
	char command[] = { 0xFF, 0xFF, 0xFF, 0x16, 0x0C, 0x01, 0xFF, 0xFF };
	uart_write_bytes(UART_NUM_1, (const char*) command, sizeof(command));
}
/** @brief Add the KPI values and inverter MSB,LSB register into the node element.
 * This function should call at the time of system initialization, Subsequent call has no effect.
 *
 *  @return Void.
 */
void InitKPI() {
	//Root Node KPI
	AddNewKPI(0x16, 0x0C, SWITCH_STAT, false, true);
	//ADD new KPI, Here
	AddNewKPI(0x92, 0x0C, Charging_Time, false, true);
	AddNewKPI(0xA0, 0x0C, Total_AH, false, true);
	AddNewKPI(0x9E, 0x0C, AH_in, false, true);
	AddNewKPI(0x6A, 0x0C, Reserved_KPI_6A, false, true);
	// End of New KPI added is completed
	AddNewKPI(0x1e, 0x0C, ALARMS, false, true);
	AddNewKPI(0x08, 0x0C, Mains_Voltage, false, true);
	AddNewKPI(0x0a, 0x0C, Output_voltage, false, true);
	AddNewKPI(0x06, 0x0C, Battery_Voltage, false, true);
	AddNewKPI(0x0C, 0x0C, Load_Current, false, true);
	AddNewKPI(0x10, 0x0C, Charging_Current, false, true);
	AddNewKPI(0x12, 0x0C, Mains_frequency, false, true);
	AddNewKPI(0x0e, 0x0C, Temperature_Value, false, true);
	AddNewKPI(0x18, 0x0C, Mode, false, true);
	AddNewKPI(0x1C, 0x0C, Battery_Type, false, true);
	AddNewKPI(0x1A, 0x0C, Charging, false, true);
	AddNewKPI(0xc4, 0x0b, Date_of_calibration, false, true);
	AddNewKPI(0xc2, 0x0b, Time_of_calibration, false, true);
	AddNewKPI(0x3c, 0x0C, Ah_count, false, true);
	AddNewKPI(0x20, 0x0C, Buzzer_settings, false, true);
	AddNewKPI(0x24, 0x0C, Voltage_regulator, false, true);
	AddNewKPI(0x26, 0x0C, Appliance_Mode, false, true);
	AddNewKPI(0x2a, 0x0C, Mains_forced_cut_time, false, true);
	AddNewKPI(0x28, 0x0C, Mains_force_cut_status, false, true);
	AddNewKPI(0x82, 0x0C, Slr_Clr_Day, true, true);
	AddNewKPI(0x3a, 0x0C, Load_alarm_status, false, true);
	AddNewKPI(0x3e, 0x0C, Battery_remaining_alarm_status, false, true);
	AddNewKPI(0x6e, 0x0C, Extra_Backup_status, false, true);
	AddNewKPI(0x2e, 0x0C, Battery_type_change_timer_status, false, true);
	AddNewKPI(0x2c, 0x0C, Load_percentage, false, true);
	AddNewKPI(0x38, 0x0C, Battery_backup_time, false, true);
	AddNewKPI(0x30, 0x0C, Turbo_Charging_status, false, true);
	AddNewKPI(0x32, 0x0C, Holiday_Mode_status, false, true);
	AddNewKPI(0xcc, 0x0b, Battery_capacity, false, true);
	AddNewKPI(0x60, 0x0C, Inverter_Time, false, true);
	AddNewKPI(0x62, 0x0C, Total_days_of_installation, false, true);
	AddNewKPI(0x64, 0x0C, Total_overload_count, false, true);
	AddNewKPI(0x66, 0x0C, Total_Short_circuit_count, false, true);
	AddNewKPI(0x68, 0x0C, Total_battery_low_count, false, true);
	AddNewKPI(0xFE, 0x0B, Water_Topping_remaining_days, false, true);
	AddNewKPI(0x70, 0x0C, Mains_Charger_ON_OFF, false, true);
	// solar KPI adding into node element
	if (solarkpi == true) {
		AddNewKPI(0x78, 0x0C, PV_Voltage, true, true);
		AddNewKPI(0x7e, 0x0C, Solar_Savings_Yesterday, true, true);
		AddNewKPI(0x7a, 0x0C, Solar_Savings_Total, true, true);
		AddNewKPI(0x76, 0x0C, Solar_Current, true, true);
		AddNewKPI(0x7c, 0x0C, Solar_Savings_Today, true, true);
		AddNewKPI(0x80, 0x0C, Power_saver_switch, true, true);
		AddNewKPI(0x90, 0x0C, Solar_Alarms, true, true);
		AddNewKPI(0x84, 0x0C, Daily_Day_time_loadUsage, true, true);
		AddNewKPI(0x8a, 0x0C, Cleaning_Reminder, true, true);

		AddNewKPI(0x94, 0x0C, Solar_Reserved_Kpi_2, true, true);
		AddNewKPI(0x96, 0x0C, Solar_Reserved_Kpi_3, true, true);
		AddNewKPI(0x98, 0x0C, Solar_Reserved_Kpi_4, true, true);
		AddNewKPI(0x9A, 0x0C, Solar_Reserved_Kpi_5, true, true);
		AddNewKPI(0x9C, 0x0C, Solar_Reserved_Kpi_6, true, true);
		AddNewKPI(0xA2, 0x0C, Solar_Reserved_Kpi_9, true, true);
		AddNewKPI(0xA4, 0x0C, Solar_Reserved_Kpi_10, true, true);

		AddNewKPI(0x86, 0x0C, Daily_Load_Start, true, true);
		AddNewKPI(0x88, 0x0C, Daily_Load_End, true, true);

	}
	//Trends KPI adding into node element
	AddNewKPI(0x40, 0x0C, Trends_values_1_c, false, true);
	AddNewKPI(0x42, 0x0C, Trends_values_2_c, false, true);
	AddNewKPI(0x44, 0x0C, Trends_values_3_c, false, true);
	AddNewKPI(0x46, 0x0C, Trends_values_4_c, false, true);
	AddNewKPI(0x48, 0x0C, Trends_values_5_c, false, true);
	AddNewKPI(0x4a, 0x0C, Trends_values_6_c, false, true);
	AddNewKPI(0x4c, 0x0C, Trends_values_7_c, false, true);
	AddNewKPI(0x4e, 0x0C, Trends_values_8_c, false, true);
	AddNewKPI(0x50, 0x0C, Trends_values_1_d, false, true);
	AddNewKPI(0x52, 0x0C, Trends_values_2_d, false, true);
	AddNewKPI(0x54, 0x0C, Trends_values_3_d, false, true);
	AddNewKPI(0x56, 0x0C, Trends_values_4_d, false, true);
	AddNewKPI(0x58, 0x0C, Trends_values_5_d, false, true);
	AddNewKPI(0x5a, 0x0C, Trends_values_6_d, false, true);
	AddNewKPI(0x5c, 0x0C, Trends_values_7_d, false, true);
	AddNewKPI(0x5e, 0x0C, Trends_values_8_d, false, true);
	//This is end of the Trial node.
	AddNewKPI(0, 0, Trail_node, false, true);
	LearnLogic();
}
/** @brief Read KPI function is call every new JSon Message creation for Single read register,
 *  *  @return Void.
 */
void ReadKPI() {
	char *data = (char*) malloc(100);
	// sprintf(data,"\"VG012\":\"%s\",",(const char*)Version);	// Wi-Fi firmware Version
	// strcpy(BootData,data);
	sprintf(data, "\"VG011\":\"%d\",", wifi_strength);	// Wi-Fi sinal strength
	strcpy(BootData, data);
	sprintf(data, "\"VG012\":\"%s\",", (const char*) Version);// Wi-Fi firmware Version
	strcat(BootData, data);
	sprintf(data, "\"VG132\":\"%s\",", (const char*) Wifi_address); // Wi-Fi module hardware mac address
	strcat(BootData, data);
	sprintf(data, "\"VG136\":\"%s\",", (const char*) Userdata.Wifi_ssid); //Wifi SSID Name
	strcat(BootData, data);
	sprintf(data, "\"VG195\":\"%s,%s\",", (const char*) Userdata.GPS_latitude,
			(const char*) Userdata.GPS_Longitude);	//Get Latitude and Longitude
	strcat(BootData, data);
	sprintf(data, "\"VG042\":\"%d\",", (int) strtol(ModelCode, NULL, 16)); //Inverter Module code
	strcat(BootData, data);
	sprintf(data, "\"VG135\":\"%d\",", No_of_battery); //No of Battery connected to the inverter
	strcat(BootData, data);
	sprintf(data, "\"VG003\":\"%s\",", Serial); // Last 4 Digit S.No
	strcat(BootData, (const char*) data);
	sprintf(data, "\"VG010\":\"%s\",", PgVersion); //Inverter Program Version
	strcat(BootData, (const char*) data);
	sprintf(data, "\"VG013\":\"%s\",", ManVendorCode); //Inverter Manufacture vendor code
	strcat(BootData, (const char*) data);
	//WiFi module time
	char strftime_buf[20];
	memset(strftime_buf,'\0',sizeof(strftime_buf));
	strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
	sprintf(data, "\"VG109\":\"%s\",", (const char*) strftime_buf);
	strcat(BootData, (const char*) data);
	//Abbreviation flag in DUPS
	sprintf(data, "\"VG041\":\"%d\",", Inverter.Data_log);
	strcat(BootData, (const char*) data);
	free(data);
	return;
}
/** @brief Writes the Wi-Fi Uart Rx data into the arguments bytes.Create the JSon format and
 * Push the Actual data to server ,while end of last uart request  .
 *
 *  @param inverter response UART TX data which is recently requested to inverter.
 *  @return Void.
 */
void Boot_data(uint8_t *bytes) {
	read_data = 0;	//Clear the Read data variable
	if ((connection_error > 0 || ble_connect == 1 || bootcalled == 0)
			&& (Learning == false))	//Check the Server connection , Ble connection and Learn mode status
		return;
	if (bytes[3] == 0x16 && bytes[4] == 0x0c) //Setting Starting Request Address
		Boot = SWITCH_STAT;
	if ((Req_address[0] != bytes[3] && Req_address[1] != bytes[4])) { //Check the Request inverter address and Response address
		return;
	}
	if ((bytes[0] != 0xFF || bytes[1] != 0xFF || bytes[2] != 0xFF
			|| bytes[5] != 0x01) || (bytes[6] == 0XAA && bytes[7] == 0X55)) { //Check the inverter response frame

		if (Learning == false) {
			bootcalled = 0; //Disable Start message
			Data_log = 0; //clear the data log

			vTaskDelay(5000 / portTICK_RATE_MS); //Set Task delay 5 Sec
			Data_log = Inverter.Data_log;
			bootcalled = 1; //Enable start message
		} else {
			AddtoBlockList(Boot);
			LearnLogic();
		}
		return;
	}
	vTaskDelay(20 / portTICK_RATE_MS); //Set Task delay 20 Milli seconds
	char command[] = { 0xFF, 0xFF, 0xFF, 0x16, 0x0C, 0x01, 0xFF, 0xFF };
	/*
	 * Check the Learning mode status.if learn mode do not decode.
	 */
	if (Learning == false) {

		uint16_t rcv_var = (uint16_t)(((bytes[7]) << 8) | (bytes[6])); //convert invert response to uint16
		float con_rcv = 0;
		char *data = (char*) malloc(50);
		switch (Boot) { //Switch the Request KPI
		case SWITCH_STAT: // Power SWitch ON-OFF KPI
			ReadKPI();
			sprintf(data, "\"VG%03d\":\"%d\",", Boot, rcv_var);
			strcat(BootData, data);
			break;
		case Battery_Voltage: //Battery_Voltage KPI
			con_rcv = (float) rcv_var / 100;
			sprintf(data, "\"VG%03d\":\"%.2f\",", Boot, con_rcv);
			strcat(BootData, data);
			break;
		case ALARMS: //ALARMS KPI
			if (CHECK_BIT(rcv_var,3) == 0)
				forcecut = 1;
			else
				forcecut = 0;
			sprintf(data, "\"VG%03d\":\"%d\",", Boot, rcv_var);
			strcat(BootData, data);
			sprintf(data, "\"VG%03d\":\"%d\",", Present_status, rcv_var);
			strcat(BootData, data);
			break;
		case Charging_Current: //Charging current
		case Output_voltage: //Output_voltage KPI
		case Mains_frequency: //Mains_frequency KPI
		case Mains_Voltage: //Mains_Voltage KPI
			con_rcv = (float) rcv_var / 10;
			if (Boot == Mains_Voltage && forcecut == 1)
				con_rcv = 0;
			sprintf(data, "\"VG%03d\":\"%.2f\",", Boot, con_rcv);
			strcat(BootData, data);
			break;
		case Ah_count: //Ah_count as well as Battery_percentage  KPI
			sprintf(data, "\"VG%03d\":\"%d\",", Boot, rcv_var);
			strcat(BootData, data);
			sprintf(data, "\"VG%03d\":\"%d\",", Battery_percentage, rcv_var); //Battery Percentage KPI
			strcat(BootData, data);
			break;
		case Trends_values_1_c: 	//Trends_values_1_c  KPI
			sprintf(data, "\"VG%03d\":\"%d,", Boot, rcv_var);
			Today_Power_Failure_Count = rcv_var;
			strcat(BootData, data);
			if (ReadAllParam == false) { //Check Read all param status flag
				strcat(BootData, Trends_values_c);
				Boot = Trends_values_8_c;
			}
			break;
		case Trends_values_1_d: //Trends_values_1_d  KPI
			sprintf(data, "\"VG%03d\":\"%d,", Boot, rcv_var);
			Today_Power_Failure_Duration = rcv_var;
			strcat(BootData, data);

			if (ReadAllParam == false) {
				strcat(BootData, Trends_values_d);
				Boot = Trends_values_8_d; //Set End of Node to data push to server.
			}
			break;
		case Trends_values_2_c ... Trends_values_7_c: //Trends_values_2_c  from to Trends_values_7_c
			sprintf(data, "%d,", rcv_var);
			if (Boot == Trends_values_2_c)
				strcpy((char*) Trends_values_c, data);
			else
				strcat((char*) Trends_values_c, data);
			break;
		case Trends_values_8_c: //Trends_values_8_c  KPI
			sprintf(data, "%d\",", rcv_var);
			strcat((char*) Trends_values_c, data);
			strcat(BootData, Trends_values_c);
			break;
		case Trends_values_2_d ... Trends_values_7_d: //Trends_values_2_d  from to Trends_values_7_d
			sprintf(data, "%d,", rcv_var);
			if (Boot == Trends_values_2_d)
				strcpy((char*) Trends_values_d, data);
			else
				strcat((char*) Trends_values_d, data);

			break;

		case Trends_values_8_d: //Trends_values_8_d  KPI
			ReadAllParam = false;
			sprintf(data, "%d", rcv_var);
			strcat((char*) Trends_values_d, data);
			strcat(BootData, Trends_values_d);
			break;

		default: //Defualt KPI value passed by Boot variable.
			sprintf(data, "\"VG%03d\":\"%d\",", Boot, rcv_var);
			strcat(BootData, data);
			break;
		}
		free(data); //Clear allocated local buffer
	}
	struct KPIProcess *GetValue = Head;
	while (GetValue != NULL) {
		if (GetValue->KPIval == Boot) {
			GetValue = GetValue->next;  //Load the next registry
			Boot = GetValue->KPIval;
			command[3] = GetValue->MSB;
			command[4] = GetValue->LSB;
			Boot = GetValue->KPIval;
			break;
		}
		GetValue = GetValue->next;
	}
	if (GetValue->next == NULL) {
		if (Learning == false) {
			strcat(BootData, "\"\n}\n}"); //End of Root Json
			strcpy(Info_Json, InfoJson); //Copy the Info Message Tag to Info Cache memory
			strcat(Info_Json, BootData); //strcat the Boot message to Info Cache memory
			if (strcmp(Info_Json, "NA") == 0) {
				SendMessage(Info_Json);
			}
//			if(Restrt){
//				SendMessage(Info_Json);
//				Restrt=0;
//			}

			strcpy(Actual_Json, ActualJson); //Copy the Info Message Tag to Actual Cache memory
			strcat(Actual_Json, BootData); //strcat the Boot message to Actual Cache memory
			if (Data_log_flag) {
				if (Data_log_Count) {
					if (SolarFCM_Flag2 == false) {
						SendMessage(Actual_Json); //Send actual message to server data log
					} else {
						SolarFCM_Flag2 = false;
					}
					Data_log_Count--;
					if (Flag15 == true) {
						bootcalled = 0;
						Data_log_flag = 0;
					}
				}
			} else {
				SendMessage(Actual_Json); //Send actual message to server App Connected
			}

			if (Statusflag == true) {
				Statusflag = false;
				bootcalled = 0;
			} else {
				//Display Registry writing while Wifi connection data handled
				if (solarkpi == true) {
					WriteRequest = true;
					Write_Data(LED_Disp, 13, true); //LED_Disp(0x0D);
				}
				vTaskDelay(50 / portTICK_RATE_MS); //5000
				Boot = SWITCH_STAT; //Set Request KPI is power switch status
				command[3] = 0x16;
				command[4] = 0x0c;
				WriteRequest = false;
			}
		} else {
			Learning = false;
		}
	}
	Req_address[0] = command[3];
	Req_address[1] = command[4];
	uart_write_bytes(UART_NUM_1, (const char*) command, sizeof(command)); //write data to uart buffer
}
/** @brief Decode the inverter data with the specific address of LSB and MSB position.
 *  @param argument is invalid, the function has no effect.
 *  @return Void.
 */
void System_function(uint8_t *bytes) {

	switch (bytes[4]) {
	case 0x0C:
		switch (bytes[3]) { //Check LSB inverter address
		case 0x74: //No of Battery is connected Register.
			battery = false;
			No_of_battery = (uint16_t)(((bytes[7]) << 8) | (bytes[6]));
			if (No_of_battery == 2) //check if 24v battery system
				Max_Vol = 20;
			else if (No_of_battery == 3) //check if 36v battery system
				Max_Vol = 29;
			break;
		case 0x60: //Inverter Time
			if (solarkpi == true && Lock_system == false
					&& TotalWrite == true) {
				if (ble_connect == 0 && bootcalled == 0) {
					uint16_t invert_time = ((bytes[7]) << 8) | (bytes[6]);
					if (invert_time > 450 && invert_time < 1800)
						Write_Data(Solar_Savings_Total, 0, false);
				}
			}
			break;
		case 0x7A:
			if (Learning == false) {
				if (bytes[6] == 0XAA && bytes[7] == 0X55)
					break;
				if (Lock_system == false) {
					TotalUnit = ((bytes[7]) << 8) | (bytes[6]);
					if (TotalUnit < Inverter.Totalunit) {
						TotalUnit = Inverter.Totalunit;
						Write_Data(Solar_Savings_Total, TotalUnit, true);
					} else if (Inverter.Totalunit == 0 || TotalWrite == true) {
						if (TotalWrite == true) {
							Inverter.Totalunit = TotalUnit;
							Write_Inverter();
						}
						TotalWrite = false;
					}
				}
			}
			break;
		case 0x7C:
			Inverter.Today_unit = ((bytes[7]) << 8) | (bytes[6]);
			Today_Unit_OK = 1;
			break;

		case 0x1E: //ALARM Register
			wdt_true = true;
			if (ble_connect == 0 && bootcalled == 0) { //check if  direct and Internet mode connectivity
				uint16_t rcv_var = (uint16_t)(((bytes[7]) << 8) | (bytes[6]));
				/*
				 *Check the ALARM BIT for FCM
				 */
				if (CHECK_BIT(rcv_var,0) != 0 || CHECK_BIT(rcv_var,1) != 0
						|| CHECK_BIT(rcv_var,2) != 0
						|| CHECK_BIT(rcv_var,6) != 0 ||
						CHECK_BIT(rcv_var,7) != 0 || CHECK_BIT(rcv_var,8) != 0
						|| CHECK_BIT(rcv_var,9) != 0
						|| CHECK_BIT(rcv_var,12) != 0) {
					if (AlarmCount == 1 && (AlarmStatus != rcv_var)
							&& connection_error == 0) { //check if Alarm count already set to 1
						char FCM_Topic[50];
						char fcm[20];
						memset(FCM_Topic, 0, sizeof(FCM_Topic));
						sprintf(FCM_Topic, "device/dups/%s/fcm/%s", ModelCode,
								(const char*) Userdata.Serial_number);
						memset(fcm, 0, sizeof(fcm));
						sprintf(fcm, "VG033:%d", rcv_var);
						/*
						 * Send FCM to server
						 */
						AlarmCount = 0;

						if (Pr_AlarmStatus != rcv_var) {
							esp_mqtt_client_publish(DUPS_client, FCM_Topic, fcm,
									strlen((char*) fcm), 0, 0);
						}

					} else { //Set Alarm count is 1
						vTaskDelay(30 / portTICK_RATE_MS);
						Write_Data(ALARMS, 0, false); //Request for Alarm count
						AlarmCount = 1;
						break;
					}

				} else { //Clear the Alarm count value
					AlarmCount = 0;

				}
				if (solarkpi == true) { //If solar inverter check Solar Alarm
					vTaskDelay(30 / portTICK_RATE_MS); //5000
					Write_Data(Solar_Alarms, 0, false);
				}
				if (AlarmStatus != rcv_var) {
					Pr_AlarmStatus = AlarmStatus;
					AlarmStatus = rcv_var;

					if (solarkpi == true)
						AlarmStat = true;
					else
						Statusflag = true;
				} else {
					AlarmStatus = rcv_var;
				}
			}
			break;
		case 0x90: //Solar Alarm FCM register
			wdt_true = true;
			if (ble_connect == 0 && bootcalled == 0) { //check if  direct and Internet mode connectivity

				uint16_t rcv_var = (uint16_t)(((bytes[7]) << 8) | (bytes[6]));

				/*
				 *Check the SOLAR ALARM BIT for FCM
				 */
				if (CHECK_BIT(rcv_var,1) != 0 || CHECK_BIT(rcv_var,2) != 0
						|| CHECK_BIT(rcv_var,3) != 0) {
					if (AlarmCount_184 == 1 && SolarAlarmStatus != rcv_var
							&& connection_error == 0) { //check if Solar Alarm count already set to 1
						char FCM_Topic[50];
						char fcm[20];
						memset(FCM_Topic, 0, sizeof(FCM_Topic));
						sprintf(FCM_Topic, "device/dups/%s/fcm/%s", ModelCode,
								(const char*) Userdata.Serial_number);
						memset(fcm, 0, sizeof(fcm));
						sprintf(fcm, "VG184:%d", rcv_var);
						/*
						 * Send Solar FCM to server
						 */
						AlarmCount_184 = 0;
						if (Pr_SolarAlarmStatus != rcv_var) {
							esp_mqtt_client_publish(DUPS_client, FCM_Topic, fcm,
									strlen((char*) fcm), 0, 0);
						}

					} else { //Set Alarm count is 1
						AlarmCount_184 = 1;
						vTaskDelay(30 / portTICK_RATE_MS);
						Write_Data(Solar_Alarms, 0, false);
						break;
					}

				} else { //Clear the Alarm count value

					AlarmCount_184 = 0;

				}

				if (SolarAlarmStatus != rcv_var) {
					Pr_SolarAlarmStatus = rcv_var;
					SolarAlarmStatus = rcv_var;
					AlarmStat = true;
				} else {
					SolarAlarmStatus = rcv_var;
				}

				if (solarkpi == true) {
					vTaskDelay(30 / portTICK_RATE_MS);
					Write_Data(Cleaning_Reminder, 0, false);
				}
			}

			break;
		case 0x8A: //Solar Cleaning Flag Register
			wdt_true = true;
			if (ble_connect == 0 && bootcalled == 0) { //check if direct and Internet mode connectivity

				uint16_t rcv_var = (uint16_t)(((bytes[7]) << 8) | (bytes[6]));

				if (rcv_var == 1) { //Check solar Cleaning flag Bit
					if (AlarmCount_186 == 1 && Solar186_hour != -2
							&& connection_error == 0) //check if Solar Alarm cleaning  already set to 1
									{
						char FCM_Topic[50];
						char fcm[20];
						memset(FCM_Topic, 0, sizeof(FCM_Topic));

						sprintf(FCM_Topic, "device/dups/%s/fcm/%s", ModelCode,
								(const char*) Userdata.Serial_number);
						memset(fcm, 0, sizeof(fcm));
						sprintf(fcm, "VG186:%d", rcv_var);
						/*
						 * Send Solar FCM to server
						 */
						AlarmCount_186 = 0;
						Solar186_hour = -2; //timeinfo.tm_hour;
						esp_mqtt_client_publish(DUPS_client, FCM_Topic, fcm,
								strlen((char*) fcm), 0, 0);
						if (AlarmStat == true) {
							AlarmStat = false;
							Statusflag = true;
						}
						break;
					} else { //Set Alarm count is 1
						AlarmCount_186 = 1;
						vTaskDelay(30 / portTICK_RATE_MS);
						Write_Data(Cleaning_Reminder, 0, false);
					}

				} else { //Clear the Alarm count value
					AlarmCount_186 = 0;

					if (AlarmStat == true) {
						AlarmStat = false;
						Statusflag = true;
					}
				}

			}
			break;
		case 0x06: //Checking Battery Checking Profile
			/*
			 * Set the Watch dog flag as true ,when the battery voltage read.
			 */
			wdt_true = true;
			uint16_t batteryvol = (uint16_t)(bytes[7] << 8 | (bytes[6]));
			batteryvol /= 100;
			if (No_of_battery != 0)
				batteryvol /= No_of_battery;
			if (batteryvol < Min_Vol && batteryvol != 0) { //Checking Battery low condition
				if (Userdata.ResetTimer != 1) { // Previous power save mode in controller
					Hours = timeinfo.tm_hour * 60 + timeinfo.tm_min;
					Userdata.ResetTimer = 1; //Reset ultra power saver timer
				}
				if ((Hours + Sleep_Min)
						< (timeinfo.tm_hour * 60 + timeinfo.tm_min)) { //Check current WiFi module time with ultra power save time
					if (Userdata.PowerSave != 1) { //if is not ultra power save mode,
						Userdata.PowerSave = 1;/*! Setting to ultra power save mode*/
						Write_systemConf();
					}
				}
			} else if (batteryvol * No_of_battery >= Max_Vol) { //Check the battery voltage with ultra power save mode recovery battery voltage
				if (Userdata.PowerSave == 1) { //if system already ultra power save mode
					if (Userdata.ResetTimer != 2) { // check if  Previous ultra power save mode timer in controller
						Hours = timeinfo.tm_hour * 60 + timeinfo.tm_min;
						Userdata.ResetTimer = 2; /*! Reset ultra power saver timer*/
					}
					if ((Hours + Sleep_Min)
							< (timeinfo.tm_hour * 60 + timeinfo.tm_min)) { //Check current WiFi module time with recovery ultra power save time
						if (Userdata.PowerSave != 0) {
							Userdata.PowerSave = 0; /*! Reset ultra power save mode*/
							Write_systemConf();
						}

					}
				} else {
					Userdata.ResetTimer = 0; /*! set normal power save mode*/
				}
			}
			break;
		default:
			break;
		}
		break;
	case 0x0B: //inverter Type Register
		switch (bytes[3]) {
		case 0xB0: // Model Selection
			sprintf((char*) ModelCode, "%02X%02X", bytes[7], bytes[6]);
			ModelCode[4] = '\0';
			if (bytes[7] == 0XAB && bytes[6] == 0XCD) {
				Jig_Falg = 1;
			} else {
				Jig_Falg = 0;
			}

			memset(inverter_model, 0, sizeof(inverter_model));
			model = true;
			if (ModelCode[0] == 'C') //Check if Solar type
				solarkpi = true; //Set inverter model is solar
			else
				solarkpi = false; //Set inverter model is non-solar
			break;
		case 0xc8: // Serial Number Register
		{
			int strlength = strlen((char*) Userdata.Serial_number);
			if (strlength > 4) { //Check the Saved flash serial number should be more than size of 4
				char SN[5];
				strncpy((char*) SN,
						(const char*) Userdata.Serial_number + (strlength - 4),
						4);
				SN[4] = '\0';

				if (bytes[7] == 0XEE && bytes[6] == 0XEE) {
					printf("Re Configure Mode\n");
					Reconfig_Flag = 2;
				} else {
					if (Reconfig_Flag > 0) {
						Reconfig_Flag--;
					}
				}

				int SerialNo = atoi(SN);
				if ((uint16_t)(((bytes[7]) << 8) | (bytes[6])) == SerialNo) { //compare the inverter S.No and Wi-Fi configured s.no
					Lock_system = false; //Serial number matched.
				} else {
					Lock_system = true; //Serial number not matched. Internet control disabled
				}
				sprintf((char*) Serial, "%d",
						(uint16_t)(((bytes[7]) << 8) | (bytes[6])));
			} else { //wi-fi module not configure with S.No number.
				printf("Sn Not configured\n");
				Reconfig_Flag = 2;
			}
		}
			break;
		case 0xca: //Inverter ProgramVersion
			sprintf((char*) PgVersion, "%d",
					(uint16_t)(((bytes[7]) << 8) | (bytes[6])));
			break;
		case 0xce: //Manufacture vendor Code
			sprintf((char*) ManVendorCode, "%d",
					(uint16_t)(((bytes[7]) << 8) | (bytes[6])));
			break;

		default:
			break;
		}
		break;
	default:
		break;
	}
	return;
}
/** @brief inverter 8byte buffer fill call back function. function is triggered uart interrupt handler
 *
 *  @param argument is inverter response request data.
 *  @return Void.
 */
inline static void pr_hexbytes(uint8_t *bytes) {

//		printf("%02X%02X%02X%02X%02X%02X%02X%02X---Boot:%d---Solar:%d--WriteReq:%d--Learn:%d--BootCalled:%d\n",bytes[0],bytes[1],bytes[2],bytes[3],bytes[4],bytes[5],bytes[6],bytes[7],Boot,solarkpi,WriteRequest,Learning,bootcalled);
	while (exist == 1) { //Wait until the last receive data is process
		vTaskDelay(1 / portTICK_RATE_MS); //5000
	}

	exist = 1; //Enable data process flag

	if (ble_connect == 0) {
		if (bytes[3] == 0x22 && bytes[4] == 0x0C) { //Check if display writing registry
			exist = 0; ////Clear the data process flag && Enable Inverter firmware V1.0 Supports
			WriteRequest = false; //Disable the write request flag
			return;
		}
	}

	System_function(bytes); //Send the data to system decode function

	if (WriteRequest == false) { //Check Request Type Read or Write

		if (ble_connect == 1) //Check ble is Connected
				{
			RX_hexbytes(bytes); //Write data to ble transmit buffer
			Hours = timeinfo.tm_hour * 60 + timeinfo.tm_min; //Clear the power save timer
			Ble_Wake = Hours; //Copy the current time into ble wake up time
		} else {

			Boot_data(bytes); //process the inverter response

		}

	} else {
		WriteRequest = false; //Disable the write request flag
	}
	exist = 0; //Clear the data process flag
	return;
}

/** @brief This  function is used to read and  Write_Request to the inverter,
 *	@param Registry inverter read or write KPI.
 *	@param Val writing value to inverter.
 *  @param write is true then request type is write else converted to read request.
 *  @return Void.
 */
void Write_Data(int Registry, int Val, bool Write) {

	char Writecommand[] = { 0xFF, 0x01, 0x00, 0x00, 0x0C, 0x00, 0xFF, 0xFF };

	if (Write == true) {
		Writecommand[1] = (uint8_t) Val;
	} else {
		Writecommand[1] = 0xFF;
		Writecommand[2] = 0xFF;
		Writecommand[5] = 0x01;
	}
	switch (Registry) {

	case LED_Disp:
		Writecommand[3] = 0x22;
		break;

	case No_Bat_cntd:
		Writecommand[3] = 0x74;
		break;

	case Serial_number:
		Writecommand[3] = 0xc8;
		Writecommand[4] = 0x0b;
		break;
	case Inverter_Model:
		Writecommand[3] = 0xb0;
		Writecommand[4] = 0x0b;
		break;
	case Program_version:
		Writecommand[3] = 0xca;
		Writecommand[4] = 0x0b;
		break;

	case Manufacturing_vendor_code:
		Writecommand[3] = 0xce;
		Writecommand[4] = 0x0b;
		break;
	case Battery_Voltage:
		Writecommand[3] = 0x06;
		break;
	case ALARMS:
		Writecommand[3] = 0x1E;
		break;
	case Solar_Alarms:
		Writecommand[3] = 0x90;
		break;
	case Cleaning_Reminder:
		Writecommand[3] = 0x8a;
		break;
	case SWITCH_STAT:
		Writecommand[3] = 0x16;
		break;
	case Mode:
		Writecommand[3] = 0x18;
		break;
	case Buzzer_settings:
		Writecommand[3] = 0x20;
		break;
	case Output_voltage:
		Writecommand[3] = 0x24;
		break;
	case Slr_Clr_Day:
		Writecommand[3] = 0x82;
		Writecommand[2] = (uint8_t) Val >> 8;
		break;
	case Appliance_Mode:
		Writecommand[3] = 0x26;
		break;
	case Mains_forced_cut_time:
		Writecommand[3] = 0x2a;
		if (Val == 30) // for 30 mins
			Writecommand[1] = 0x1E;
		else if (Val == 60) // for 60 mins
			Writecommand[1] = 0x3C;
		else if (Val == 120) { // for 120 mins
			Writecommand[1] = 0x78;
		} else if (Val == 180) { // for 180 mins
			Writecommand[1] = 0xB4;
		} else if (Val == 240) { // for 240 mins
			Writecommand[1] = 0xF0;
		} else {
			Writecommand[1] = 0x00; // clear the mains force cut
		}
		break;
	case Mains_force_cut_status:
		Writecommand[3] = 0x28;
		break;

	case Turbo_Charging_status:
		Writecommand[3] = 0x30;
		break;
	case Holiday_Mode_status:
		Writecommand[3] = 0x32;
		break;
	case Load_alarm_status:
		Writecommand[3] = 0x3a;
		if (Val == 100) { // for 100%
			Writecommand[1] = 0xF4;
			Writecommand[2] = 0x01;
		}
		break;
	case Battery_remaining_alarm_status:
		Writecommand[3] = 0x3E;
		Writecommand[1] = 0x64;
		Writecommand[2] = 0x00;
		if (Val == 1) { // if check battery remaining alarm status
			Writecommand[1] = 0x4C;
			Writecommand[2] = 0x04;
		}
		break;
	case Time_of_calibration:
		Writecommand[3] = 0x34;
		break;

	case Inverter_Time:
		Writecommand[3] = 0x60;
		break;

	case Daily_Day_time_loadUsage:
		Writecommand[3] = 0x84;
		break;
	case Extra_Backup_status: //write
		Writecommand[1] = 0xE8;
		Writecommand[2] = 0x03;
		Writecommand[3] = 0x6E;
		break;
	case Charging:
		Writecommand[3] = 0x2E;
		break;
	case Mains_Charger_ON_OFF:
		Writecommand[3] = 0x70; //0x18 replaced with 0x70
		break;
	case Solar_Savings_Total:
		if (Write == true) {
			Writecommand[1] = (uint16_t) Val;
			Writecommand[2] = (uint16_t) Val >> 8;
		}
		Writecommand[3] = 0x7A;
		break;
		//---------------------------------//
	case Daily_Load_Start:
		if (Write == true) {
			Writecommand[1] = (uint16_t) Val;
			Writecommand[2] = (uint16_t) Val >> 8;
		}
		Writecommand[3] = 0x86;
		break;
		//---------------------------------//

	default:
		printf("invalid Register Requested\n");
		break;
	}

	WriteRequest = true;
	uart_write_bytes(UART_NUM_1, (const char*) Writecommand, 8);
}

//void FloatToStr(float number, char *str ){
//		memset(str,'\0',10);
//		int temp1=0,temp2;
//		temp1=(int)number;
//		temp2=(int)((number-(float)temp1)*1000);
//		sprintf(str,"%d.%d",temp1,temp2);
//}

/** @brief This function is set the inverter time.
 *	@param Time inverter time.
 *  @return Void.
 */
void initialize_sntp(void) {
	printf("\nInitializing SNTP\n");
	sntp_setoperatingmode (SNTP_OPMODE_POLL);
	sntp_setservername(0, "pool.ntp.org"); //
	sntp_init();
}

void obtain_time(void) {
	static bool tok = 0;
	if (!tok) {
		initialize_sntp();
		tok = 1;
	}

	// wait for time to be set

	int retry = 0;
	const int retry_count = 10;
	timeCheckMode = TIME_CHECK_REQUESTED;
	do {
		printf("Waiting for system time to be set... \n");
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		time(&now);
		localtime_r(&now, &timeinfo);
	} while (timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count);

}
void SetSystem_time(int Time) {

	char sync[] = { 0xFF, 0x01, 0x00, 0x36, 0x0C, 0x00, 0xFF, 0xFF };
	uint8_t data;
	data = (uint16_t) Time;
	sync[1] = data;
	data = (uint16_t) Time >> 8;
	sync[2] = data;
	uart_write_bytes(UART_NUM_1, (const char*) sync, sizeof(sync));
	WriteRequest = true;
	vTaskDelay(100 / portTICK_RATE_MS);
	Write_Data(Time_of_calibration, 1, true);
}
/** @brief The function is execute while power thread create.
 *	@param passed by thread
 *  @return Void.
 */
static void Power(void *pvParameters) {
	//Initialize local timer variable
	int timer = 0, wifi_timer = 0;
	bool wifi = false, ble_flag = true;
	//Wait for 15 Sec while WiFi connectivity established
	vTaskDelay(15000 / portTICK_RATE_MS); //
	while (ble_connect == 0) { //Continue untill if Ble is not connected
		vTaskDelay(1000 / portTICK_RATE_MS);
		if (Userdata.PowerSave == 1) { //Check if Ultra power save mode is enabled
			if (ble_connect == 0) { //check if Ble is not connected
				timer++; //increase the Ble timer
				if (timer == 1 && ble_flag == true) { //if timer set Seconds one and ble is already turned on
					ble_connect = 0;
					ble_if_close();
					ble_flag = false;
				} else if (timer > 10) { //Ble timer is more than 10 Seconds
					if (ble_flag == false) { //If Ble disabled
						Config_GATT(); //Enable the Ble
						ble_flag = true;
					}
					timer = 0; //Clear the Ble Timer
				}
			}
		} else { //If Ble Wi-Fi model normal Battery Voltage
			if (ble_connect == 0) { //check if Ble is not connected
				timer++; //increase the Ble timer
				if (timer == 3 && ble_flag == true) { //if timer set three Seconds and ble is already turned on
					ble_connect = 0;
					ble_if_close();
					ble_flag = false;

				} else if (timer > 6) { //Ble timer is more than 6
					if (ble_flag == false) { //If Ble disabled
						Config_GATT(); //Enable the Ble
						ble_flag = true;
					}
					timer = 0;
				}
			}
		}
		if (wifisystem_flag == false) { //If wifi is not connected with router
			wifi_timer++; //increase the WiFi timer

			if (wifi_timer == 16) { //If Wifi Timer more than sixteen Seconds
				esp_wifi_stop(); //Turn off the WiFi driver
				wifi = false;
			} else if (wifi_timer > 60) { //If Wifi Timer more than 60(1-Mins) Seconds
				wifi = true;
				esp_wifi_start(); //Turn on Wifi driver
				//	tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA ,"VG_Inverter");//Set inverter name to wifi adapter
				wifi_timer = 0;
			}
		}
	}
	if (wifisystem_flag == false) { //Check the Wi-Fi connectivity
		esp_wifi_start(); //Turn on Wifi driver
	}
	Ble_Wake = timeinfo.tm_hour * 60 + timeinfo.tm_min; //Set Current Inverter time to Ble Wake up time
	wdt_true = true; //Clear the watch dog flag
	xPowerTask = NULL; //Set the power task to Null address
	vTaskDelete (NULL); //Delete the power task
}

/** @brief The function is execute while Low_Power thread create.
 *	@param passed by thread
 *  @return Void.
 */
static void Low_Power(void *pvParameters) {

	uint8_t fcm = 0, Count = 0, Energy_count = 0;
	int minuteTime = 0;
	bool timeCheck = false;
	wifi_ap_record_t wifidata;
	while (1) {

		if (ble_connect == 0) { //Check the Ble is not connected
			battery = true;

			if (bootcalled == 0 /*&& Bootconf == false*/) { //Check if internet is not connected
				if (solarkpi == true) {
					Energy_count++;
					if (Energy_count > 30) {
						Write_Data(Inverter_Time, 0, false); //Request the battery voltage
						vTaskDelay(300 / portTICK_RATE_MS);
					}
				}
				/****************************************************************************************************************************************/
				/*Newly added for time update*/
				minuteTime = (timeinfo.tm_hour * 60) + timeinfo.tm_min;
				if ((minuteTime > 130) && (minuteTime < 1380))/*Between 2:10AM and 11:00PM*/
				{
					timeCheck = true;
				} else {
					timeCheck = false;
				}
				if (timeCheck == true)/*If time update is in range*/
				{
					if ((timeupdate == 0)
							&& (timeCheckMode == TIME_CHECK_RESET)) {
						if (connection_error == 0) {
							char taskTopic[50];
							sprintf(taskTopic,
									"device/webapi/nous/timestamp/%s",
									(const char*) Userdata.Serial_number);
							esp_mqtt_client_subscribe(DUPS_client, taskTopic,
									0); //Subscribe the time topic

							timeFlag = false; //Initiate time update
							timeCheckMode = TIME_CHECK_STARTED;

						}
					}
					if ((timeFlag == true) && (timeupdate == 0)
							&& (timeCheckMode == TIME_CHECK_SUCCESS)) //Updating time to inverter
							{

						if (timeinfo.tm_year < (2016 - 1900)) {
							timeFlag = false; //Initiate time update
						} else {
							SetSystem_time(
									((timeinfo.tm_hour * 3600)
											+ (timeinfo.tm_min * 60)
											+ timeinfo.tm_sec) / 2);
							timeupdate = 1;/*Time update flag locked*/
							timeCheckMode = TIME_CHECK_RESET;
							printf("System Time Updated\n");
						}
						vTaskDelay(300 / portTICK_RATE_MS);
					}
				}

//				printf("Hr:%d Min :%d\n", timeinfo.tm_hour, timeinfo.tm_min);

				if (Flag15_Counter >= 15) {
					Data_log_Count = 1;
					Data_log_flag = true;
					Flag15 = true;
					bootcalled = 1;
					//printf("\nEVERY 15 Minute \n");
					Statusflag = false;
					Flag15_Counter = 0;
				} else {

					if (timeinfo.tm_min != Flag15_Time) {
						Flag15_Counter++;
						//printf("\nCount=%d\n",Flag15_Counter);
						Flag15_Time = timeinfo.tm_min;
					}
				}

				if ((timeinfo.tm_hour == 1) && (timeinfo.tm_min > 0)
						&& (timeinfo.tm_min < 59) && (timeupdate == 1))/*Time update flag release at 1AM*/
						{
					timeupdate = 0;
					timeCheckMode = TIME_CHECK_RESET;
				}

				//************************************
				if ((timeinfo.tm_hour == 20) && (timeinfo.tm_min >= 0)
						&& (timeinfo.tm_min < 10)) {

//				if ((timeinfo.tm_min >=0)
//						&& (timeinfo.tm_min < 10)){

//				if ((timeinfo.tm_min > 0) && (timeinfo.tm_min < 5)) {
					//printf("TP1\n");
//				if(timeinfo.tm_min %2 ==0)
//				{

					if (SolarFCM_Flag1 == false && connection_error == 0) {

						if (Today_Unit_OK) {
//								printf("TP2\n");
							SolarFCM_Flag1 = true;

							char FCM_Topic[50];
							char fcm[50];//ar  fcm[10];
							memset(FCM_Topic, 0, sizeof(FCM_Topic));
							sprintf(FCM_Topic, "device/dups/%s/fcm/%s",
									ModelCode,
									(const char*) Userdata.Serial_number);
							memset(fcm, 0, sizeof(fcm));
//							printf("\nFCM=%s\n",FCM_Topic);
							double solarToday, solartotal;

							if ((No_of_battery == 1)
									|| ((strstr(ModelCode, "C002"))
									|| (strstr(ModelCode, "C001")))) {
//								printf("\n**TP3\n");
								solarToday =
										(double) ((double) Inverter.Today_unit
												/ 10000);
								solartotal = (double) ((double) TotalUnit / 10);

							} else {

								solarToday =
										(double) ((double) Inverter.Today_unit
												/ 1000);
								solartotal = (double) TotalUnit;

//								printf("\n**VG089:%.1f,VG174:%.2f\n",solarToday,TotalUnit);
							}

							int Dur_Hour = (int) (Today_Power_Failure_Duration
									/ 1800);
							int Dur_Min = (((float) Today_Power_Failure_Duration
									/ 1800) - Dur_Hour) * 60;
							if (solarToday >= 2) {

								sprintf(fcm,
										"VG089:%.1f,VG174:%.2f,power_today: %d,%.2d:%.2d",
										solartotal, solarToday,
										Today_Power_Failure_Count, Dur_Hour,
										Dur_Min);
							} else {
								sprintf(fcm,
										"VG089: ,VG174: ,power_today: %d,%.2d:%.2d",
										Today_Power_Failure_Count, Dur_Hour,
										Dur_Min);
							}
							esp_mqtt_client_publish(DUPS_client, FCM_Topic, fcm,
									strlen((char*) fcm), 0, 0);

							Data_log_Count = 1;
							Data_log_flag = true;
							Flag15 = true;
							bootcalled = 1;
							//printf("\nEVERY 15 Minute \n");
							Statusflag = false;
							Flag15_Counter = 0;

						} else {
							Data_log_Count = 1;
							Data_log_flag = true;
							Flag15 = true;
							bootcalled = 1;
							//printf("\nEVERY 15 Minute \n");
							Statusflag = false;
							SolarFCM_Flag2 = true;
						}
					}
				} else {
					SolarFCM_Flag1 = false;
				}
				//************************************

				/****************************************************************************************************************************************/
				fcm++;
				if (fcm > 5) {
					Write_Data(Battery_Voltage, 0, false); //Request the battery voltage
					vTaskDelay(300 / portTICK_RATE_MS);
					fcm = 0;
				}
				if (connection_error == 0) {
					Write_Data(ALARMS, 0, false);
				}

			} else {
				Flag15_Time = timeinfo.tm_min;

			}

		} else {
			battery = false;

			if (timeinfo.tm_min != Flag15_Time) {
				if (Flag15_Counter < 15) {
					Flag15_Counter++;
				}
				//printf("\nCount=%d\n",Flag15_Counter);
				Flag15_Time = timeinfo.tm_min;
			}
		}
		/*Date and sync..*/
		if ((connection_error == 0) && (timeFlag == false)) {
			Count++;
			if (Count > 150)/*Retrying time update*/
			{
				printf("date and time sync...\n");
				char Timestamp[50];
				Count = 0;

				obtain_time();
				// update 'now' variable with current time
				time(&now);
				setenv("TZ", "IST-5:30", 1); // only Indian Standard time
				tzset();
				localtime_r(&now, &timeinfo);
				if (timeinfo.tm_year < (2016 - 1900)) {
					printf("date and time sync From Nouse\n");
					sprintf(Timestamp, "gmtindia*%s*yyyyMMddHHmmss",
							(const char*) Userdata.Serial_number);
					esp_mqtt_client_publish(DUPS_client, "device/webapi/nous",
							Timestamp, strlen((char*) Timestamp), 0, 0);
					timeFlag = false;
					if (timeCheckMode == TIME_CHECK_STARTED) {
						timeCheckMode = TIME_CHECK_REQUESTED;
					}
				} else {
					char strftime_buf[64];
					strftime(strftime_buf, sizeof(strftime_buf), "%c",
							&timeinfo);
					printf("Time is: %s", strftime_buf);
					timeFlag = true;

					if (timeCheckMode == TIME_CHECK_REQUESTED) {
						timeCheckMode = TIME_CHECK_SUCCESS;
					}

				}

			}
		}
		if (connection_error == 0) {
			if (esp_wifi_sta_get_ap_info(&wifidata) == 0) {
				wifi_strength = wifidata.rssi;
			}
		}
		vTaskDelay(2000 / portTICK_RATE_MS);
	}
	vTaskDelete (NULL);
}

/** @brief The function is execute while DUPS_task thread create.
 *	@param passed by thread
 *  @return Void.
 */
static void DUPS_task(void *pvParameters) {
	//Subscribe this task to TWDT, then check if it is subscribed
	CHECK_ERROR_CODE(esp_task_wdt_add(NULL), ESP_OK);
	CHECK_ERROR_CODE(esp_task_wdt_status(NULL), ESP_OK);
	int Diagnostic = 0;
	factoryPINConfig(INPUT);
	while (1) {
		//Getting Time Update in flag
		time(&now);
		localtime_r(&now, &timeinfo);
		factoryResetCheck();
		//Watchdog Timer Reset
		if (wdt_true == true) {
			CHECK_ERROR_CODE(esp_task_wdt_reset(), ESP_OK); //Comment this line to trigger a TWDT timeout
			if (xPowerTask != NULL || ble_connect == 0) {
				wdt_true = false;
			}

		}
		//Create Power save mode handler service after Ble disconnect, Interval Time is 10 Mins
		if ((Ble_Wake + Sleep_Min) < (timeinfo.tm_hour * 60 + timeinfo.tm_min)
				&& (xPowerTask == NULL)) {
			xTaskCreate(Power, "Power", 2048, NULL, 5, &xPowerTask); //1024 Defualt task memccpy allocate
		}
		//maintenance Activity start
		Diagnostic = Date_timeNet();
		if (Diagnostic == 2) {
			ReadAllParam = true;
		} else if (Diagnostic == 1) {
			if (ble_connect == 0 && bootcalled == 0) { //Direct and wifi connection not established

				esp_restart();
				//				esp_sleep_enable_timer_wakeup(1000000); //Set sleep mode to wake up controller
				//				esp_deep_sleep_start();

			} else
				Maintenance = 1; //Direct connection established waiting for ble disconnection
			//Reboot the system for diagnostic
		} else if (Maintenance == 1 && ble_connect == 0 && bootcalled == 0) {
			// Maintenance activity started

			esp_restart();
			//			esp_sleep_enable_timer_wakeup(1000000); //Set sleep mode to wake up controller
			//			esp_deep_sleep_start();

		}
		//maintenance Activity End
		if (Statusflag == true)
			bootcalled = 1;

		if (bootcalled == 1 && read_data > 0) {

			if (ble_connect == 0 && connection_error == 0) {

				char command[] = { 0xFF, 0xFF, 0xFF, 0x16, 0x0C, 0x01, 0xFF,
						0xFF };
				Req_address[0] = 0x16;
				Req_address[1] = 0x0c;
				uart_write_bytes(UART_NUM_1, (const char*) command,
						sizeof(command));
				read_data = 0;
				WriteRequest = false;

			} else {
				//	Reading = 0;
				bootcalled = 0; //Disable data upload
			}
		}

		if (ble_connect == 1) {

			WriteRequest = false;
			bootcalled = 0;
			wdt_true = true;
			//printf("task xpower_ running...Or Ble Connected\n");
		} else if (bootcalled == 1) {

			read_data++;

			if (App_Timeout >= 0) {
				App_Timeout++;

				if (App_Timeout > (120 * 5)) { // if there is no any data came from sever last 15 than Minute Disconnect from server
					Time_out_Flag = 1;
					printf("\nTime Out detected\n");

				}
			}

		} else if (Data_log == 1 && connection_error == 0) {
			bootcalled = 1;
		}

		//Hardware Watchdog Reset
		gpio_set_level(10, 1);
		vTaskDelay(250 / portTICK_RATE_MS); //5000
		gpio_set_level(10, 0);
		vTaskDelay(250 / portTICK_RATE_MS); //5000

	}
	vTaskDelete (NULL);

}
/**
 *@brief Function for factory pin configuration
 *@param GPIO mode
 *@return void
 */
void factoryPINConfig(uint8_t mode) {
	gpio_config_t io_conf;
	io_conf.intr_type = GPIO_INTR_DISABLE;
	if (mode == INPUT) {

		io_conf.mode = GPIO_MODE_INPUT;
		io_conf.pin_bit_mask = FACTORY_PIN_SET;
		io_conf.pull_down_en = 0;
		io_conf.pull_up_en = 1;
	} else {
		io_conf.mode = GPIO_MODE_OUTPUT;
		io_conf.pin_bit_mask = FACTORY_PIN_SET;
		io_conf.pull_down_en = 0;
		io_conf.pull_up_en = 0;
	}
	gpio_config(&io_conf);
}
/**
 *@brief Function for checking factory reset
 *@param void
 *@return void
 */
void factoryResetCheck(void) {
	if (gpio_get_level(FACTORY_RESET_PIN) == 0) {
		factoryCount++;
		if (factoryCount >= FACTORY_RST_COUNT) {
			ESP_LOGI("FACTORY", "FACTORY RESET TIMEOUT");
			factoryCount = 0;
			factoryPINConfig(OUTPUT);
			gpio_set_level(FACTORY_RESET_PIN, 1);
			factoryResetFlash();
		}
	} else {
		factoryCount = 0;
	}
}
/**
 *
 * Start the Dups Function and create multiple service.
 */

static void Test_JIG_task(void *pvParameters) {
	wifi_ap_record_t wifidata;
	char Writecommand[] = { 0xFF, 0x00, 0x00, 0x40, 0x0C, 0x00, 0xFF, 0xFF };
	uint16_t Data = 0;
	wifi_conn_init();
	printf("JIG TASK Started\n");
	while (1) {

		if (Jig_Falg == 0) {
			Write_Data(Inverter_Model, 0, false);
		}
		if (wifisystem_flag == 1 && Jig_Falg == 1) {
			if (esp_wifi_sta_get_ap_info(&wifidata) == 0) {
				Data = 0;

				Data |= 1 << 5;

				wifi_strength = wifidata.rssi;
				Data = ((~wifi_strength + 1)/10) << 7 | Data;
				Writecommand[1] = (uint16_t) Data;
				Writecommand[2] = (uint16_t) Data >> 8;
				printf("SignalStrength %d--- %2X %2X\n", wifi_strength,
						Writecommand[1], Writecommand[2]);
				uart_write_bytes(UART_NUM_1, (const char*) Writecommand, 8);
			}
		}
		vTaskDelay(1000 / portTICK_RATE_MS);
		if ((No_of_battery < 31) && xTest_Jig_Task != NULL) {
			vTaskDelete (NULL);
			printf("JIG TASK STOPED\n");
		}
	}
}

void Start_DUPS() {

	time(&now);
	localtime_r(&now, &timeinfo);
	Hours = timeinfo.tm_hour * 60 + timeinfo.tm_min;
	//Set Write request flag
	WriteRequest = true;
	//Set All param reader flag
	ReadAllParam = true;
	//Set syn retry value is Zero.
	uint8_t syn_retry = 0;


	if (strlen((char*) Inverter.Key) > 15 && strlen((char*) Inverter.IV) > 15) //Check the AES inverter key and Iv length more than 16.
		Secured = true; //Set internet Encryption mode.
	//Enable supports for hardware testwith 3.3V supply

	Config_GATT();

	//Request inverter no of Batter is connected
	Write_Data(No_Bat_cntd, 0, false);
	vTaskDelay(300 / portTICK_RATE_MS); //5000
	while (No_of_battery > 30) { // 20210917 increase the battery count to 30 from 9 //continue if no of battery connected is more than 9
		Write_Data(No_Bat_cntd, 0, false);
		uint8_t timer = 0;
		while (timer < 5) {
			gpio_set_level(10, 1);
			vTaskDelay(500 / portTICK_RATE_MS); //5000
			gpio_set_level(10, 0);
			vTaskDelay(500 / portTICK_RATE_MS); //5000
			gpio_set_level(10, 1);
			timer++;

		}
		if (xTest_Jig_Task == NULL) {
			xTaskCreate(Test_JIG_task, "Test_JIG_task", 2048, NULL, 5,
					&xTest_Jig_Task);
		}
	}

	Maintenance = 0; //System Reboot flag
	vTaskDelay(100 / portTICK_RATE_MS);
	//Request Inverter model
	Write_Data(Inverter_Model, 0, false);
	vTaskDelay(100 / portTICK_RATE_MS);
	//Request inverter S.No
	Write_Data(Serial_number, 0, false);
	vTaskDelay(100 / portTICK_RATE_MS);
	//Request inverter Program version
	Write_Data(Program_version, 0, false);
	vTaskDelay(100 / portTICK_RATE_MS);
	//Request inverter Manufacture code
	Write_Data(Manufacturing_vendor_code, 0, false);
	vTaskDelay(100 / portTICK_RATE_MS);

	//Create the xLow_Power task with task size is 2 Kb and priority is 5
	xTaskCreate(Low_Power, "Low_Power", 2048, NULL, 5, &xLow_Power); //1024 Defualt task memccpy allocate
	//Create DUPS task and into the Watchdog Task Timer
	CHECK_ERROR_CODE(esp_task_wdt_init(TWDT_TIMEOUT_S, false), ESP_OK);
	esp_task_wdt_add(xTaskGetIdleTaskHandleForCPU(0));
	xTaskCreatePinnedToCore(DUPS_task, "DUPS_task", 2048, NULL, 10,
			&task_handles[0], 0);
	//Create Memory allocation for info,Actual and Boot data
	Info_Json = (char*) malloc(1200);
	Actual_Json = (char*) malloc(1200);
	BootData = (char*) malloc(1200);
	strcpy(Info_Json, "NA");
	strcpy(Actual_Json, "NA");
	Data_log = Inverter.Data_log;
	if (solarkpi == true) //Set inverter model is solar
		TotalUnit = Inverter.Totalunit;
	//Create the power task with task size is 2 Kb and priority is 5
	xTaskCreate(Power, "Power", 2048, NULL, 5, &xPowerTask); //1024 Defualt task memccpy allocate
}
#endif /*DUPS_COMMUNICATION_H_
 */
