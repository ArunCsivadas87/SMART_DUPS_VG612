#include "kpiprocess.h"

/*Check Duplicate schedule
 */
bool KPIProcess_Duplicate(uint8_t MSB,uint8_t LSB,int KPIval){

    struct KPIProcess *CheckKPIProcess = Head;
    while (CheckKPIProcess !=NULL) {
#ifdef Debug
        printf("parse:%d---%d\n",KPIval,CheckKPIProcess->KPIval);
#endif
        if(CheckKPIProcess->KPIval == KPIval){// if((CHECK_DATE(KPIval,i) && CHECK_DATE(CheckKPIProcess->KPIval,i))!=0){
#ifdef Debug
            printf("KPIProcess already available Same date and Time\n");
#endif
            return  true;

        }

        CheckKPIProcess = CheckKPIProcess->next;

    }
#ifdef Debug
    printf("Dupilcate KPIProcess not Available\n");
#endif
    return false;
}
/*
 *Creating New KPIProcess
 */
int AddNewKPI(uint8_t MSB,uint8_t LSB,int KPIval,bool SolarKPI,bool KPIProcessEnable ){

    if(KPIProcess_Duplicate(MSB,LSB,KPIval) == true)
        return  -1;
    struct KPIProcess *NewKPIProcess = NULL;
    NewKPIProcess = (struct KPIProcess*)malloc(sizeof (struct KPIProcess));
    NewKPIProcess->MSB = MSB;
    NewKPIProcess->LSB = LSB;
    NewKPIProcess->KPIval = KPIval;
    NewKPIProcess->SolarKPI =SolarKPI;
    NewKPIProcess->KPIProcessEnable = KPIProcessEnable;
    NewKPIProcess->next = NULL;

    //Check if Head Node is create or Not

    if(Head == NULL){
        Head = NewKPIProcess;
    }else {  //Check if already KPIProcess is created
        struct KPIProcess *KPIProcessItem = Head;

        while (KPIProcessItem->next !=NULL) {
            KPIProcessItem = KPIProcessItem->next;
        }
        KPIProcessItem->next = NewKPIProcess;
    }
    return 1;
}
/*
 *Delete the KPIProcess ,
 */

int Delete_KPI(int KPIval){

    if(Head == NULL){
#ifdef Debug
        printf("KPIProcess Not Configured\n");
#endif
        return  -1;
    }else{
        struct KPIProcess *DeleteKPIProcess = Head;
        struct KPIProcess *PreviousKPIProcess = Head;

        while (DeleteKPIProcess !=NULL) {
            if(KPIval == DeleteKPIProcess->KPIval){
                if(Head == DeleteKPIProcess)
                    Head = DeleteKPIProcess->next;
                PreviousKPIProcess->next  = DeleteKPIProcess->next;
                free(DeleteKPIProcess);
//#ifdef Debug
                printf("Success fully Deleted KPIProcessd---%d\n",KPIval);
//#endif
                return 1;
            }
            PreviousKPIProcess = DeleteKPIProcess;
            DeleteKPIProcess = DeleteKPIProcess->next;
        }

    }
#ifdef Debug
    printf("Unable to find the KPIProcess");
#endif
    return  -2;
}
/*Display the Total No of KPIProcess
 */
int Display_KPIProcess(void){

    uint8_t command[] ={0xFF,0xFF,0xFF,0x16,0x0C,0x01,0xFF,0xFF};

    if(Head == NULL)
        return -1;
    struct KPIProcess *Display = Head;
    int KPIProcessCount=1;
    while(Display!=NULL){
#ifdef Debug
        printf("\nKPIProcess ID:%d", Display->KPIval);
        command[3] = Display->MSB;command[4] = Display->LSB;
        printf("%02X%02X%02X%02X%02X%02X%02X%02X\n",command[0],command[1],
                command[2],command[3],command[4],command[5],command[6],command[7]);
#endif

        Display = Display->next;
    }
    return 1;
}

/*Get the no of KPIProcess available in system
 */

int KPIProcess_Count(){

    if(Head == NULL)
        return 0;
    struct KPIProcess *Display = Head;
    int KPIProcessCount=0;
    while(Display!=NULL){
        Display = Display->next;
        KPIProcessCount++;
    }
    return KPIProcessCount;
}

/* This is function is used to add the register into block list
 *
 */
void AddtoBlockList(int KPI){

    if(Head != NULL)
    {
        struct KPIProcess *GetValue = Head;
        while (GetValue !=NULL) {
            if(GetValue->KPIval == KPI){
                GetValue->blocklist++;
            	printf("Added blocklist:%d--Count:%d\n",GetValue->KPIval,GetValue->blocklist);
                if(GetValue->blocklist>1){
                Delete_KPI(KPI);
                break;

                }
            }
            GetValue = GetValue->next;
        }
    }

}

/* Get KPIProcess Value by id
 */

void GetKPIProcess_ValueById(int KPIProcessID, uint8_t *MSB,uint8_t *LSBr,int *KPIval){

    *MSB = 0;
    *LSBr = 0;
    *KPIval = 0;
    uint8_t command[] ={0xFF,0xFF,0xFF,0x16,0x0C,0x01,0xFF,0xFF};

    if(Head != NULL)
    {
        struct KPIProcess *GetValue = Head;
        while (GetValue !=NULL) {
            if(GetValue->KPIval == KPIProcessID){
                *MSB = GetValue->MSB;
                *LSBr = GetValue->LSB;
                *KPIval = GetValue->KPIval;
                command[3] = GetValue->MSB;command[4] = GetValue->LSB;

                /*printf("%02X%02X%02X%02X%02X%02X%02X%02X\n",command[0],command[1],
                        command[2],command[3],command[4],command[5],command[6],command[7]);*/
                break;
            }
            GetValue = GetValue->next;
        }

    }

    return;
}
