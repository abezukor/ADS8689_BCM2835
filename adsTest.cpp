#include <bcm2835.h>
#include <unistd.h>
#include <time.h>
#include "ADS8689_BCM2835.hpp"
#include <limits>
#include <iostream>
#include <stdexcept>

int main()
{
	//Initialize a delay
	struct timespec delay;
	//Set delay times
	delay.tv_sec = 0;
	delay.tv_nsec = 10000;
	
	//Initialize the ADC using SPI0, CS0, ±0.625*Vref range and an internal Reference.
	ADS8689 adc(ADS8689::SPI::SPI_0, ADS8689::ChipSelect::CS0, ADS8689::Range::pm0625Vref, ADS8689::Reference::Internal);
	
	//store min and max
	double min = std::numeric_limits<double>::max();
	double max = std::numeric_limits<double>::min();

	while(1)
	{
		//read adc
		double scaledval = adc.readADC();

		
		if(scaledval>max){
			max = scaledval;
		}
		if(scaledval<min){
			min = scaledval;
		}

		//print the scaled value and the max and min received voltages
		std::cout << "value: " << scaledval  << " received min: " << min << " received max: " << max << std::endl;
		
		//sleep for a bit
		nanosleep(&delay, NULL); 
	}
}
