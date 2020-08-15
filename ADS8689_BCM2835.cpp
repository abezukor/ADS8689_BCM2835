/*----------------------------------------------------*\
 * This is a library to interface the ADS8689 ADC chip *
 * with a Raspberry Pi usign BCM2835 library           *
 *                                                     *
 * Author: Marco Duarte                                *
 \----------------------------------------------------*/
 
#include "ADS8689_BCM2835.hpp"

uint16_t ADS8689::sendCommand(uint8_t op, uint8_t address, uint16_t data)
{
	char buff[4] = {0};
	
	buff[0] = op/*(op | (address >> 7))*/;
	buff[1] = address/*(address << 1)*/;
	buff[2] = (data >> 8);
	buff[3] = data;
	
	//printf("Sending: %.2x %.2x %.2x %.2x\n", buff[0], buff[1], buff[2], buff[3]);
	
	if(spiModule == SPI_AUX)
		bcm2835_aux_spi_transfern(buff, sizeof(buff));
	else
		bcm2835_spi_transfern(buff, sizeof(buff));
		
	//printf("Got: %.2x %.2x %.2x %.2x\n", buff[0], buff[1], buff[2], buff[3]);
		
	return ((buff[0] << 8) | buff[1]); 
}

void ADS8689::begin(SPI spiModule, uint8_t cs, Range range, Reference reference)
{
	this->cs = cs;
	this->spiModule = spiModule;
	
	struct timespec delay;
	delay.tv_sec = 1;
	delay.tv_nsec = 10005;
	
	nanosleep(&delay, NULL);
	
	delay.tv_sec = 0;
	
	if(this->spiModule == SPI_AUX)
	{
		uint16_t clockDivider = bcm2835_aux_spi_CalcClockDivider(3125000);
		bcm2835_aux_spi_setClockDivider(clockDivider);
	}
	else if(this->spiModule == SPI_0)
	{
		if(this->cs != BCM2835_SPI_CS0 && this->cs != BCM2835_SPI_CS1)
		{
			throw std::runtime_error("Choose BCM2835_SPI_CS0 or BCM2835_SPI_CS1\n");
		}
		
		bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
		bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
		bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_128);
		bcm2835_spi_chipSelect(this->cs);
		bcm2835_spi_setChipSelectPolarity(this->cs, LOW);
	}
	else
	{
		throw std::runtime_error("Choose SPI_AUX or SPI_0\n");
	}
	
	//set adc to use internal VREF and range to 1.25*VREF

	uint16_t got = 0;
	
	uint64_t command = range | reference;

	do
	{
		sendCommand(WRITE, RANGE_SEL_REG_7_0, command);
		nanosleep(&delay, NULL);
		sendCommand(READ_HWORD, RANGE_SEL_REG_7_0, 0x0000);
		got = sendCommand(NOP, 0x00, 0x0000);
		//std::clog << "Got: " << got >> 8 << " " << (got & 0x00FF) << std::endl;
		nanosleep(&delay, NULL);
	}while(got != command);
	
	sendCommand(NOP, 0x00, 0x0000);
	
	return;
}

//you will actually get the value of the adc on any command
//but nop makes sure it doesn't execute anything and you just get the value
//it's best to send all configuration commands before sampling
uint16_t ADS8689::readADC()
{
	return sendCommand(NOP, 0x00, 0x0000);
}
