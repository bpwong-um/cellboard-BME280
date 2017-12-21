/******************************************************************************
SparkFunBME280.cpp
BME280 Arduino and Teensy Driver
Marshall Taylor @ SparkFun Electronics
May 20, 2015
https://github.com/sparkfun/BME280_Breakout

Resources:
Uses Wire.h for i2c operation
Uses SPI.h for SPI operation

Development environment specifics:
Arduino IDE 1.6.4
Teensy loader 1.23

This code is released under the [MIT License](http://opensource.org/licenses/MIT).
Please review the LICENSE.md file included with this example. If you have any questions 
or concerns with licensing, please contact techsupport@sparkfun.com.
Distributed as-is; no warranty is given.
******************************************************************************/
//See SparkFunBME280.h for additional topology notes.

#include "BME280.h"
#include <math.h>


//****************************************************************************//
//
//  Settings and configuration
//
//****************************************************************************//

//Construct with these default settings if nothing is specified
BME280 bme280 = {
    .settings = {
        //Select interface mode
        .commInterface  = BME280_I2C_MODE,
    	//Select address for I2C.  Does nothing for SPI            
        .I2CAddress     = 0x77,
    	//Select CS pin for SPI.  Does nothing for I2C            
        .chipSelectPin  = 10,
        .runMode        = 3,
        .tempOverSample = 1,
        .pressOverSample= 1,
        .humidOverSample= 1
        },
    .calibration = {},
    .t_fine = 0,
    .temp_C     = -9999,
    .press_Pa   = -9999,
    .RH         = -9999
};

void BME280_start() {
    I2C_Start();
    I2C_Sleep();
}

void BME280_stop() {
    I2C_Stop();
}

//****************************************************************************//
//
//  Configuration section
//
//  This uses the stored SensorSettings to start the IMU
//  Use statements such as "mySensor.settings.commInterface = SPI_MODE;" to 
//  configure before calling .begin();
//
//****************************************************************************//
uint8_t BME280_init()
{
	//Check the settings structure values to determine how to setup the device
	uint8_t data_to_write = 0, device_address;  //Temporary variable
	I2C_Start();
    I2C_Sleep();
    
	//Reading all compensation data, range 0x88:A1, 0xE1:E7
	bme280.calibration.dig_T1 = ((uint16_t)((BME280_readRegister(BME280_DIG_T1_MSB_REG) << 8) + BME280_readRegister(BME280_DIG_T1_LSB_REG)));
	bme280.calibration.dig_T2 = ((int16_t)((BME280_readRegister(BME280_DIG_T2_MSB_REG) << 8) + BME280_readRegister(BME280_DIG_T2_LSB_REG)));
	bme280.calibration.dig_T3 = ((int16_t)((BME280_readRegister(BME280_DIG_T3_MSB_REG) << 8) + BME280_readRegister(BME280_DIG_T3_LSB_REG)));

	bme280.calibration.dig_P1 = ((uint16_t)((BME280_readRegister(BME280_DIG_P1_MSB_REG) << 8) + BME280_readRegister(BME280_DIG_P1_LSB_REG)));
	bme280.calibration.dig_P2 = ((int16_t)((BME280_readRegister(BME280_DIG_P2_MSB_REG) << 8) + BME280_readRegister(BME280_DIG_P2_LSB_REG)));
	bme280.calibration.dig_P3 = ((int16_t)((BME280_readRegister(BME280_DIG_P3_MSB_REG) << 8) + BME280_readRegister(BME280_DIG_P3_LSB_REG)));
	bme280.calibration.dig_P4 = ((int16_t)((BME280_readRegister(BME280_DIG_P4_MSB_REG) << 8) + BME280_readRegister(BME280_DIG_P4_LSB_REG)));
	bme280.calibration.dig_P5 = ((int16_t)((BME280_readRegister(BME280_DIG_P5_MSB_REG) << 8) + BME280_readRegister(BME280_DIG_P5_LSB_REG)));
	bme280.calibration.dig_P6 = ((int16_t)((BME280_readRegister(BME280_DIG_P6_MSB_REG) << 8) + BME280_readRegister(BME280_DIG_P6_LSB_REG)));
	bme280.calibration.dig_P7 = ((int16_t)((BME280_readRegister(BME280_DIG_P7_MSB_REG) << 8) + BME280_readRegister(BME280_DIG_P7_LSB_REG)));
	bme280.calibration.dig_P8 = ((int16_t)((BME280_readRegister(BME280_DIG_P8_MSB_REG) << 8) + BME280_readRegister(BME280_DIG_P8_LSB_REG)));
	bme280.calibration.dig_P9 = ((int16_t)((BME280_readRegister(BME280_DIG_P9_MSB_REG) << 8) + BME280_readRegister(BME280_DIG_P9_LSB_REG)));

	bme280.calibration.dig_H1 = ((uint8_t)(BME280_readRegister(BME280_DIG_H1_REG)));
	bme280.calibration.dig_H2 = ((int16_t)((BME280_readRegister(BME280_DIG_H2_MSB_REG) << 8) + BME280_readRegister(BME280_DIG_H2_LSB_REG)));
	bme280.calibration.dig_H3 = ((uint8_t)(BME280_readRegister(BME280_DIG_H3_REG)));
	bme280.calibration.dig_H4 = ((int16_t)((BME280_readRegister(BME280_DIG_H4_MSB_REG) << 4) + (BME280_readRegister(BME280_DIG_H4_LSB_REG) & 0x0F)));
	bme280.calibration.dig_H5 = ((int16_t)((BME280_readRegister(BME280_DIG_H5_MSB_REG) << 4) + ((BME280_readRegister(BME280_DIG_H4_LSB_REG) >> 4) & 0x0F)));
	bme280.calibration.dig_H6 = ((uint8_t)BME280_readRegister(BME280_DIG_H6_REG));

	//Set the oversampling control words.
	//config will only be writeable in sleep mode, so first insure that.
	BME280_writeRegister(BME280_CTRL_MEAS_REG, 0x00);
	
	//Set the config word
	data_to_write = (bme280.settings.tStandby << 0x5) & 0xE0;
	data_to_write |= (bme280.settings.filter << 0x02) & 0x1C;
	BME280_writeRegister(BME280_CONFIG_REG, data_to_write);
	
	//Set ctrl_hum first, then ctrl_meas to activate ctrl_hum
	data_to_write = bme280.settings.humidOverSample & 0x07; //all other bits can be ignored
	BME280_writeRegister(BME280_CTRL_HUMIDITY_REG, data_to_write);
	
	//set ctrl_meas
	//First, set temp oversampling
	data_to_write = (bme280.settings.tempOverSample << 0x05) & 0xE0;
	//Next, pressure oversampling
	data_to_write |= (bme280.settings.pressOverSample << 0x02) & 0x1C;
	//Last, set mode
	data_to_write |= (bme280.settings.runMode) & 0x03;
	//Load the byte
	BME280_writeRegister(BME280_CTRL_MEAS_REG, data_to_write);
	
    // dataToWrite is a placeholder for the value we will return
	device_address = BME280_readRegister(0xD0);
    return device_address;
}

//Strictly resets.  Run BME280_init() afterwards
void BME280_reset()
{
	BME280_writeRegister(BME280_RST_REG, 0xB6);
	
}

//****************************************************************************//
//
//  Pressure Section
//
//****************************************************************************//
float BME280_readFloatPressure( void )
{

	// Returns pressure in Pa as unsigned 32 bit integer in Q24.8 format (24 integer bits and 8 fractional bits).
	// Output value of “24674867” represents 24674867/256 = 96386.2 Pa = 963.862 hPa
	int32_t adc_P = ((uint32_t)BME280_readRegister(BME280_PRESSURE_MSB_REG) << 12) | ((uint32_t)BME280_readRegister(BME280_PRESSURE_LSB_REG) << 4) | ((BME280_readRegister(BME280_PRESSURE_XLSB_REG) >> 4) & 0x0F);
	
	int64_t var1, var2, p_acc;
	var1 = ((int64_t) bme280.t_fine) - 128000;
	var2 = var1 * var1 * (int64_t) bme280.calibration.dig_P6;
	var2 = var2 + ((var1 * (int64_t) bme280.calibration.dig_P5)<<17);
	var2 = var2 + (((int64_t) bme280.calibration.dig_P4)<<35);
	var1 = ((var1 * var1 * (int64_t) bme280.calibration.dig_P3)>>8) + ((var1 * (int64_t) bme280.calibration.dig_P2)<<12);
	var1 = (((((int64_t)1)<<47)+var1))*((int64_t) bme280.calibration.dig_P1)>>33;
	if (var1 == 0)
	{
		return 0; // avoid exception caused by division by zero
	}
	p_acc = 1048576 - adc_P;
	p_acc = (((p_acc<<31) - var2)*3125)/var1;
	var1 = (((int64_t) bme280.calibration.dig_P9) * (p_acc>>13) * (p_acc>>13)) >> 25;
	var2 = (((int64_t) bme280.calibration.dig_P8) * p_acc) >> 19;
	p_acc = ((p_acc + var1 + var2) >> 8) + (((int64_t) bme280.calibration.dig_P7)<<4);
	
    bme280.press_Pa = (float)p_acc / 256.0;
    return bme280.press_Pa;
	
}

float BME280_readFloatAltitudeMeters( void )
{
	float heightOutput = 0;
	
	heightOutput = ((float)-45846.2)*(pow(((float) BME280_readFloatPressure()/(float) P_SURFACE), 0.190263) - (float)1);
	return heightOutput;
	
}

float BME280_readFloatAltitudeFeet( void )
{
	float heightOutput = 0;
	
	heightOutput = BME280_readFloatAltitudeMeters() * 3.28084;
	return heightOutput;
	
}

//****************************************************************************//
//
//  Humidity Section
//
//****************************************************************************//
float BME280_readFloatHumidity( void )
{
	
	// Returns humidity in %RH as unsigned 32 bit integer in Q22. 10 format (22 integer and 10 fractional bits).
	// Output value of “47445” represents 47445/1024 = 46. 333 %RH
	int32_t adc_H = ((uint32_t) BME280_readRegister(BME280_HUMIDITY_MSB_REG) << 8) | ((uint32_t) BME280_readRegister(BME280_HUMIDITY_LSB_REG));
	
	int32_t var1;
	var1 = (bme280.t_fine - ((int32_t)76800));
	var1 = (((((adc_H << 14) - (((int32_t)bme280.calibration.dig_H4) << 20) - (((int32_t)bme280.calibration.dig_H5) * var1)) +
	((int32_t)16384)) >> 15) * (((((((var1 * ((int32_t)bme280.calibration.dig_H6)) >> 10) * (((var1 * ((int32_t)bme280.calibration.dig_H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) *
	((int32_t)bme280.calibration.dig_H2) + 8192) >> 14));
	var1 = (var1 - (((((var1 >> 15) * (var1 >> 15)) >> 7) * ((int32_t)bme280.calibration.dig_H1)) >> 4));
	var1 = (var1 < 0 ? 0 : var1);
	var1 = (var1 > 419430400 ? 419430400 : var1);

    bme280.RH = (float) (var1>>12) / 1024.0;
	return bme280.RH;

}



//****************************************************************************//
//
//  Temperature Section
//
//****************************************************************************//

float BME280_readTempC( void )
{
	// Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123” equals 51.23 DegC.
	// t_fine carries fine temperature as global value

	//get the reading (adc_T);
	int32_t adc_T = ((uint32_t)BME280_readRegister(BME280_TEMPERATURE_MSB_REG) << 12) | ((uint32_t)BME280_readRegister(BME280_TEMPERATURE_LSB_REG) << 4) | ((BME280_readRegister(BME280_TEMPERATURE_XLSB_REG) >> 4) & 0x0F);

	//By datasheet, calibrate
	int64_t var1, var2;

	var1 = ((((adc_T>>3) - ((int32_t)bme280.calibration.dig_T1<<1))) * ((int32_t)bme280.calibration.dig_T2)) >> 11;
	var2 = (((((adc_T>>4) - ((int32_t)bme280.calibration.dig_T1)) * ((adc_T>>4) - ((int32_t)bme280.calibration.dig_T1))) >> 12) * ((int32_t)bme280.calibration.dig_T3)) >> 14;
	
    bme280.t_fine = var1 + var2;
	float output = (bme280.t_fine * 5 + 128) >> 8;

	output = output / 100;
	
    bme280.temp_C = output;
	return bme280.temp_C;
}

float BME280_readTempF( void )
{
	float output = BME280_readTempC();
	output = (output * 9) / 5 + 32;

	return output;
}

//****************************************************************************//
//
//  Utility
//
//****************************************************************************//
uint8_t BME280_readRegister(uint8_t offset)
{
	//Return value
	uint8_t result;
	uint8_t numBytes = 1;
    
    uint16  iter;
    
	switch (bme280.settings.commInterface) {

    	case BME280_I2C_MODE:
            I2C_Wakeup();
            I2C_MasterClearStatus(); /* Clear any previous status */
            
            // Write to register
            I2C_MasterWriteBuf(bme280.settings.I2CAddress, (uint8 *) &offset, 1, I2C_MODE_COMPLETE_XFER);
            for(iter = 0; iter < BME280_MAX_ITER; iter++)
            {
                if(0u != (I2C_MasterStatus() & I2C_MSTAT_WR_CMPLT))
                {
                    /* Transfer complete. Check Master status to make sure that transfer
                    completed without errors. */
                    break;
                }
            }        
            
            // Read Buffer        
            I2C_MasterReadBuf(bme280.settings.I2CAddress, &result, numBytes, I2C_MODE_COMPLETE_XFER);
            
            for(iter = 0; iter < BME280_MAX_ITER; iter++){
                if(0u != (I2C_MasterStatus() & I2C_MSTAT_RD_CMPLT))
                {
                    /* Transfer complete. Check Master status to make sure that transfer
                    completed without errors. */
                    break;
                }
                
            }
            I2C_Sleep();

    		break;

    	case BME280_SPI_MODE:
    		// TODO
    		break;

    	default:
    		break;
	}
	return result;
}


void BME280_writeRegister(uint8_t offset, uint8_t data)
{
    uint8 packet[] = {offset, data};
    uint16 iter;
    
	switch (bme280.settings.commInterface)
	{
    	case BME280_I2C_MODE:
    		//Write the byte
            I2C_Wakeup();
            I2C_MasterClearStatus(); /* Clear any previous status */
            
            // Write to register
            I2C_MasterWriteBuf(bme280.settings.I2CAddress, (uint8 *) &packet, 2, I2C_MODE_COMPLETE_XFER);
            for(iter = 0; iter < BME280_MAX_ITER; iter++)
            {
                if(0u != (I2C_MasterStatus() & I2C_MSTAT_WR_CMPLT))
                {
                    // Transfer complete. Check Master status to make sure that transfer
                    //completed without errors. 
                    break;               
                }
            }          
            
            I2C_Sleep();        

    		break;

    	case BME280_SPI_MODE:
    		//TODO
    		break;

    	default:
    		break;
	}
}
