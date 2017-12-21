/*******************************************************************************
* File Name: main.c
*
* Version: 1.20
*
* Description:
*  This example project demonstrates the basic operation of the I2C master
*  (SCB mode) component. The I2C master sends a packet with a command to
*  the I2C slave to control the RGB LED color. The packet with a status
*  is read back.
*  For more information refer to the example project datasheet.
*
********************************************************************************
* Copyright 2014-2015, Cypress Semiconductor Corporation. All rights reserved.
* This software is owned by Cypress Semiconductor Corporation and is protected
* by and subject to worldwide patent and copyright laws and treaties.
* Therefore, you may use this software only as provided in the license agreement
* accompanying the software package from which you obtained this software.
* CYPRESS AND ITS SUPPLIERS MAKE NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* WITH REGARD TO THIS SOFTWARE, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT,
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*******************************************************************************/

#include "BME280.h"
#include <project.h>


/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
*  The main function performs the following actions:
*   1. Turns off the RGB LED.
*   2. Starts the I2C master (SCB mode) component.
*   3. Enables global interrupts.
*   4. Sends command and status packets to the slave every 500mS.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
int main()
{
    

    
    CyGlobalIntEnable;

    BME280_start();  
    CyDelay(10u);
    BME280_init();    

    /***************************************************************************
    * Main polling loop
    ***************************************************************************/
    for(;;)
    {
        /* Send packet with command to the slave */        

        BME280_init();
        BME280_readTempC();
        BME280_readFloatPressure();
        BME280_readFloatHumidity();
        
        LED_Write(!LED_Read());
        WQ_Power_Write(LED_Read());                      
        
        CyDelay(1000u); /* Delay between commands */
    }
}

/* [] END OF FILE */
