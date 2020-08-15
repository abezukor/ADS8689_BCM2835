/*----------------------------------------------------*\
 * This is a library to interface the ADS8689 ADC chip *
 * with a Raspberry Pi usign BCM2835 library           *
 *                                                     *
 * Author: Marco Duarte                                *
 \----------------------------------------------------*/
 
#ifndef ADS8689_BCM2835_h
#define ADS8689_BCM2835_h

#include <bcm2835.h>
#include <stdio.h>
#include <time.h>
#include <iostream>
#include <stdexcept>
#include <limits>

//addresses for register bits
//format: reg_name_MSB_LSB
//only unreserved sections are defined
//if a section includes reserved bits, writes will be ignored/reads return 0 at those bits

#define DEVICE_ID_REG_23_16   0x02 //Device ID
#define RST_PWCTRL_REG_7_0    0x04 //Reset and power control
#define RST_PWCTRL_REG_15_8   0x05 
#define SDI_CTL_REG_7_0       0x08 //SDI data input control
#define SDO_CTL_REG_7_0       0x0C //SDO-x data input control
#define SDO_CTL_REG_15_8      0x0D
#define DATAOUT_CTL_REG_7_0   0x10 //output data control
#define DATAOUT_CTL_REG_15_8  0x11
#define RANGE_SEL_REG_7_0     0x14 //input range selection control
#define ALARM_REG_7_0         0x20 //ALARM output register
#define ALARM_REG_15_8        0x21
#define ALARM_H_TH_REG_7_0    0x24 //ALARM high threshold and hystersis register
#define ALARM_H_TH_REG_15_8   0x25
#define ALARM_H_TH_REG_31_24  0x27
#define ALARM_L_TH_REG_7_0    0x28 //ALARM low threshold register
#define ALARM_L_TH_REG_15_8   0x29

//command op codes - LSB of all op codes (except NOP) is MSB of 9 bit address
#define NOP           0x00 //send to read current 16 bit value
#define CLEAR_HWORD   0xC0 //any bits marked 1 in data will be set to 0 in register at those positions
#define READ_HWORD    0xC8 //read 16 bits from register
#define READ          0x48 //read 8 bits from register
#define WRITE         0xD0 //write 16 bits to register
#define WRITE_MSBYTE  0xD2 //write 8 bits to register (MS Byte of data)
#define WRITE_LSBYTE  0xD4 //write 8 bits to register (LS Byte of data)
#define SET_HWORD     0xD8 //any bits marked 1 in data will be set to 1 in register at those positions

class ADS8689
{
	public:
		//SPI Options
		enum SPI{
			SPI_0 = 0,
			SPI_AUX = 1
		};

		//Chip Select Options
		enum ChipSelect {
			CS0 = BCM2835_SPI_CS0,
			CS1 = BCM2835_SPI_CS1,
			CS2 = BCM2835_SPI_CS2,
			CSManual = BCM2835_SPI_CS_NONE 
		};

		//Reference Options
		enum Reference{
			External = 0x0080,
			Internal = 0x0000,
		};

		//Range Options (pm is ±, p is unipolar)
		enum Range{
			pm3Vref = 0x0000, //±3*Vref
			pm25Vref = 0x0001, //±2.5*Vref
			pm15Vref = 0x0002, //±2.5*Vref
			pm125Vref = 0x0003, //±1.25*Vref
			pm0625Vref = 0x0004, //±0.625*Vref
			p3Vref = 0x0008, //3*Vref
			p25Vref = 0x0009, //2.5*Vref
			p15Vref = 0x000A, //1.5*Vref
			p125Vref = 0x000B //1.25*Vref
		};

		//Internal reference value. May be useful for data processing so its public
		const float internalReference = 4.096;

		//Constructors
		ADS8689(SPI, ChipSelect cs, Range range, Reference reference=Internal); //Internal Reference
		ADS8689(SPI, ChipSelect cs, Range range, double referenceVoltage, Reference reference=External); //External Reference

		uint16_t sendCommand(uint8_t op, uint8_t address, uint16_t data); //Send a Generic Command
		uint16_t readPlainADC(); //Read plane value from ADC
		double readADC(); //Read Scaled value from the ADC
		
	private:
		//local stored varibles.
		uint8_t spiModule; 
		uint8_t cs;
		Reference reference;
		Range range;
		bool setRange=false;
		double referenceVoltage;

};


#endif