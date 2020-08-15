build:
	g++ -std=c++11 -c ADS8689_BCM2835.cpp -l bcm2835
	ar rvs ADS8689_BCM2835.a ADS8689_BCM2835.o

build-test:
	g++ -std=c++11 adsTest.cpp ADS8689_BCM2835.a -l bcm2835 -o test
	
run-test:
	./test

clean:
	rm ADS8689_BCM2835.a ADS8689_BCM2835.o test