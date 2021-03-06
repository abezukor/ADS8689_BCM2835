/*----------------------------------------------------*\
 * This is a library to interface the ADS8689 ADC chip *
 * with a Raspberry Pi usign BCM2835 library           *
 *                                                     *
 * Author: Marco Duarte                                *
 * Improvments: Abe Zukor							   *
 \----------------------------------------------------*/
 
#include "ADS8689_BCM2835.hpp"

//main constructor
ADS8689::ADS8689(SPI spiModule, ChipSelect cs, Range range, Reference reference)
{
	//Setting classwide varibles
	this->cs = cs;
	this->spiModule = spiModule;
	this->range = range;
	this->reference = reference;

	if(reference==Reference::Internal){
		this->referenceVoltage = internalReference;
		this->setRange = true;
	} else if (!setRange){
		throw std::runtime_error("No Reference Voltage set.\n");
	}
	
	//initializeing SPI
	//Initializes BCM library
	if (!bcm2835_init())
	{
		throw std::runtime_error("bcm2835_init failed. Are you running as root??\n");
	}
	//change tp bcm2835_aux_spi_begin() if using SPI1
	bool spi_begin = false;
	switch(this->spiModule){
		case SPI_0:
			spi_begin = bcm2835_spi_begin();
			break;
		case SPI_AUX:
			spi_begin = bcm2835_aux_spi_begin();
			break;
		default:
			break;
	}
	if (!spi_begin)
	{
		throw std::runtime_error("bcm2835_spi_begin failed. Are you running as root??\n");
	}

	//wait a bit for initialazation to finish
	struct timespec delay;
	delay.tv_sec = 1;
	delay.tv_nsec = 10005;
	nanosleep(&delay, NULL);
	delay.tv_sec = 0;
	
	//set SPI Settings
	switch(this->spiModule){
		case SPI_AUX: 
		{
			uint16_t clockDivider = bcm2835_aux_spi_CalcClockDivider(3125000);
			bcm2835_aux_spi_setClockDivider(clockDivider);
			break;
		}
		case SPI_0:
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
			break;
		}
		default:
			throw std::runtime_error("Choose SPI_AUX or SPI_0\n");
			break;
	}
	
	//set the ADS to the requested interval
	uint16_t got = 0;
	uint32_t command = range | reference;
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
//Alternate constructor for external reference. Uses Constructor delagation.
ADS8689::ADS8689(SPI spiModule, ChipSelect cs, Range range, double referenceVoltage, Reference reference) : 
	ADS8689::ADS8689(spiModule, cs, range, reference)
{
	this->referenceVoltage = referenceVoltage;
	this->setRange = true;
}

//Send a command
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

//you will actually get the value of the adc on any command
//but nop makes sure it doesn't execute anything and you just get the value
//it's best to send all configuration commands before sampling
uint16_t ADS8689::readPlainADC()
{
	return sendCommand(NOP, 0x00, 0x0000);
}

//Uses read planeadc but also does scaling math
double ADS8689::readADC(){

	double val = (double) readPlainADC();


	double scalefactor = 0.0;

	//Find the scalefactor from the range
	switch(this->range)
	{
		case pm3Vref: case p3Vref:
			scalefactor = this->referenceVoltage*3/std::numeric_limits<uint16_t>::max();
			break;
		case pm25Vref: case p25Vref:
			scalefactor = this->referenceVoltage*25/std::numeric_limits<uint16_t>::max();
			break;
		case pm15Vref: case p15Vref:
			scalefactor = this->referenceVoltage*1.5/std::numeric_limits<uint16_t>::max();
			break;
		case pm125Vref: case p125Vref:
			scalefactor = this->referenceVoltage*1.25/std::numeric_limits<uint16_t>::max();
			break;
		case pm0625Vref:
			scalefactor = this->referenceVoltage*0.625/std::numeric_limits<uint16_t>::max();
			break;
		default:
			throw std::runtime_error("Invalid Range \n");
	}

	//Check if unipolar or bi-polar. If bipolar, double range and shift down by half the max value so its centered at zero
	switch(this->reference)
	{
		case pm3Vref: case pm25Vref: case pm15Vref: case pm125Vref: case pm0625Vref:
			val = val-0.5*std::numeric_limits<uint16_t>::max();
			scalefactor *= 2;
			break;
		case p3Vref: case p25Vref: case p15Vref: case p125Vref:
			//scalefactor = this->referenceVoltage*1.25/std::numeric_limits<uint16_t>::max();
			break;
		default:
			throw std::runtime_error("Invalid Range \n");
	}

	//Calculate and refturn the scaled value.
	return val*scalefactor;
}
