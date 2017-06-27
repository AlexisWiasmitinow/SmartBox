/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h"
int alertStatus=0;
void updateLED(){
    CYBLE_GATTS_HANDLE_VALUE_NTF_T tempHandle;
    int ledState = (!Pin_LED_B_Read());
    if(CyBle_GetState() != CYBLE_STATE_CONNECTED)
        return;
    
    tempHandle.attrHandle = CYBLE_SMARTBOX_ALERT_CHAR_HANDLE;
    tempHandle.value.val = (uint8 *)&ledState;
    tempHandle.value.len = 1;
    CyBle_GattsWriteAttributeValue(&tempHandle,0,&cyBle_connHandle,0);
}

void blinkBlue(){
     Pin_LED_B_Write(0);
     CyDelay(500);
     Pin_LED_B_Write(1);
     CyDelay(500);
}

void BleCallBack(uint32 event, void* eventParam){
    //uint8 blinkStatus = 1;
    CYBLE_GATTS_WRITE_CMD_REQ_PARAM_T *wrReqParam;
    
    switch(event){
        //no connection
        case CYBLE_EVT_STACK_ON:
        case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
            CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
            Pin_LED_R_Write(0);
            Pin_LED_G_Write(1);
        break;
       //connection established
       case CYBLE_EVT_GATT_CONNECT_IND:
            Pin_LED_R_Write(1);
            Pin_LED_G_Write(0);
       break;
       //write request
       case CYBLE_EVT_GATTS_WRITE_REQ:
            Pin_LED_R_Write(1);
            Pin_LED_G_Write(1);
            wrReqParam = (CYBLE_GATTS_WRITE_CMD_REQ_PARAM_T *) eventParam;
            
            //blinkStatus = &wrReqParam;
            //LED write request
            if(wrReqParam->handleValPair.attrHandle == CYBLE_SMARTBOX_ALERT_CHAR_HANDLE){
                //check if writeable
                if(CYBLE_GATT_ERR_NONE == CyBle_GattsWriteAttributeValue(&wrReqParam->handleValPair,0,&cyBle_connHandle,CYBLE_GATT_DB_PEER_INITIATED)){
                    //Pin_LED_B_Write(!wrReqParam->handleValPair.value.val[0]);
                    CyBle_GattsWriteRsp(cyBle_connHandle);
                    
                    alertStatus = wrReqParam->handleValPair.value.val[0];
                    if(alertStatus == 0){
                        Blinker_Stop();
                        Pin_LED_B_Write(1);
                    }
                    else if (alertStatus == 1) {
                        Blinker_Start();
                        Pin_Beep_Enable_Write(0);
                        Pin_LED_B_Write(0);
                    }
                    else if (alertStatus == 2) {
                        Blinker_Start();
                        Pin_Beep_Enable_Write(1);
                        Pin_LED_B_Write(0);
                    }                    
                }
            }
            break;
      default:
            break;
    }
}
void readADC(){
    
    float x =0;
    uint16 y=0x0066;
    ADC_SAR_Seq_1_StartConvert();
    ADC_SAR_Seq_1_IsEndConversion(ADC_SAR_Seq_1_WAIT_FOR_RESULT);
    x=ADC_SAR_Seq_1_GetResult16(0);
    y=ADC_SAR_Seq_1_CountsTo_mVolts(0,x);
    
    
    //uint16 testBla=0x00FF;
    CYBLE_GATTS_HANDLE_VALUE_NTF_T tempHandle;
    if(CyBle_GetState() != CYBLE_STATE_CONNECTED)
        return;
    tempHandle.attrHandle = CYBLE_SMARTBOX_LEVEL_CHAR_HANDLE;
    //tempHandle.value.val = (uint8 *) &testBla;
    tempHandle.value.val = (uint8*)&y;
    tempHandle.value.len = 2;
    CyBle_GattsWriteAttributeValue(&tempHandle,0,&cyBle_connHandle,0);
}

void readButton(){
    int buttonStatus=0;
    buttonStatus=Pin_Button_Read();
    if (buttonStatus==0){
        Blinker_Stop();
        alertStatus=0;
        Pin_LED_B_Write(1);
    }
    if (alertStatus==0){
        CYBLE_GATTS_HANDLE_VALUE_NTF_T tempHandle;
        if(CyBle_GetState() != CYBLE_STATE_CONNECTED)
            return;
        tempHandle.attrHandle = CYBLE_SMARTBOX_ALERT_CHAR_HANDLE;
        tempHandle.value.val = 0x00;
        tempHandle.value.len = 1;
        CyBle_GattsWriteAttributeValue(&tempHandle,0,&cyBle_connHandle,0);
    }
}

int main()
{
    CyGlobalIntEnable; /* Enable global interrupts. */
    //float adcValue=0;
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    CyBle_Start(BleCallBack);
    ADC_SAR_Seq_1_Start();
    for(;;)
    {
        updateLED();
        readButton();
        readADC();
        CyBle_ProcessEvents();
        //CyBle_EnterLPM(CYBLE_BLESS_DEEPSLEEP);
       
    }
}

/* [] END OF FILE */
