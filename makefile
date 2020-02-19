all:
	g++ -o jora_gateway Jora.h Jora.cpp jora_gateway.cpp -lwiringPi -lbcm2835

jora_gateway: jora_gateway.o wiringPi.o jora.o
	g++ -lrt -lpthread jora_gateway.o wiringPi.o 

jora_gateway.o: jora_gateway.cpp
	g++ $(CFLAGS) -DRASPBERRY -DRASPBERRY2 -DIS_RCV_GATEWAY -c jora_gateway.cpp -o jora_gateway.o

wiringPi.o: wiringPi.h
	g++ wiringPi.o

jora.o: jora.h
	g++ -c jora.cpp -o jora.o

lib: wiringPi.o jora_gateway.o

clean:
	rm *.o jora_*gateway
