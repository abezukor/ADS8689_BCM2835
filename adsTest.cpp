#include <bcm2835.h>
#include <unistd.h>
#include <time.h>
#include "ADS8689_BCM2835.hpp"
#include <limits>
#include <iostream>
#include <stdexcept>

int main()
{
	
	struct timespec delay;
	
	delay.tv_sec = 0;
	delay.tv_nsec = 10000;
	
	if (!bcm2835_init())
	{
		throw std::runtime_error("bcm2835_init failed. Are you running as root??\n");
		return 1;
	}
	
	//change tp bcm2835_aux_spi_begin() if using SPI1
	if (!bcm2835_spi_begin())
	{
		throw std::runtime_error("bcm2835_spi_begin failed. Are you running as root??\n");
		return 1;
	}
	
	//using SPI0 and CS0, comment arguments for SPI1 and CS2
	ADS8689 adc(ADS8689::SPI::SPI_0, ADS8689::Range::pm0625Vref, ADS8689::Reference::Internal, BCM2835_SPI_CS0);
	
	//double scaler = (4.096 * 0.625)/(std::numeric_limits<int>::max()/2);
	double min = std::numeric_limits<double>::max();
	double max = std::numeric_limits<double>::min();

	while(1)
	{
		//read adc
		double scaledval = adc.readADC();
		//printf("Voltage = %lf \n", scaledval);
		if(scaledval>max){
			max = scaledval;
			std::cout << "min: " << min << " max: " << max << " v: " << adc.readPlainADC() << std::endl;
		}
		if(scaledval<min){
			min = scaledval;
			std::cout << "min: " << min << " max: " << max << " v: " << adc.readPlainADC() << std::endl;
		}	
			nanosleep(&delay, NULL); 
	}
}
