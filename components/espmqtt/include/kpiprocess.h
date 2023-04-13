/** @file Kpiprocess.h
 *  @brief Supports add kpi ,delete kpi and block list
 *
 *  @author Duraisamy Pandurangan (V-Guard Industrial Ltd,R&D)
 *  @Created Mar 25,2019
 */
#ifndef KPIPROCESS_H
#define KPIPROCESS_H
/*Header Include*/
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
/*Preprocessor*/
//#define Debug <!*<Debug */

/**
 * Structure Represents the KPI process data
 */
struct KPIProcess{
    uint8_t MSB;/*!<Hold the Inverter MSB address*/
    uint8_t LSB;/*!<Hold the Inverter LSB address*/
    int KPIval;/*!<Hold the KPI value*/
    bool SolarKPI;/*!<Hold the solar flag status*/
    bool KPIProcessEnable;/*!<Hold the kpi processor status*/
    uint8_t blocklist;/*!<Hold the block list count*/
    struct KPIProcess *next;/*!<Hold the new structure node*/
};

struct KPIProcess *Head;/*!<Create the header node for KPIprocess*/

/**
 * @brief Add KPI into node element
 * @param MSB - inverter MSB address
 * @param LSB - inverter MSB address
 * @param KPIVal - Server KPI value to process
 * @param SolarKPI - Add the KPI into solar System
 * @param KPIProcessEnable - KPI process status
 *
 * @return  1 - Successfully added
 * @return -1 - Duplicate KPI
 */
int AddNewKPI(uint8_t MSB,uint8_t LSB,int KPIval,bool SolarKPI,bool KPIProcessEnable );
/**
 * @brief Delete KPI from node element
 * @param KPI - delete KPI value
 *
 * @return  1 - Deleted
 * @return -1 - KPI not available
 */
int Delete_KPI(int KPIval);
/**
 * @brief Display KPI from node element
 * @return  1 - shown in consol
 * @return -1 - KPI not available
 */
int Display_KPIProcess(void);
/**
 * @brief Count KPI from node element
 * @return Total no of KPI available in node
 */
int KPIProcess_Count();
/**
 * @brief Get Value  from node element
 * @param KPIprocessID , Value to fetch from node
 * @param MSB -Msb value copied respective KPIprocessID and store address
 * @param LSB -Lsb value copied respective KPIprocessID and store address
 * @param KPIval - KPI value copied respective KPIprocessID and store address
 *
 * @return Void
 */
void GetKPIProcess_ValueById(int KPIProcessID, uint8_t *MSB,uint8_t *LSBr,int *KPIval);
/**
 * @brief Add KPI into Block list
 *
 * @param KPI value. Note (adding block list 2 times,automatically deleted from the node
 * @return void
 */
void AddtoBlockList(int KPI);
#endif // KPIPROCESS_H
