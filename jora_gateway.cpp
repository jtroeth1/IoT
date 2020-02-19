//to compile:
//g++ -o jora_gateway Jora.h Jora.cpp jora_gateway.cpp -lwiringPi -lbcm2835

#include "Jora.h"

//triggered on interrupt
void onReceive(int packetSize){
	printf("Received packet: ");

	//read packet
	for(int i = 0; i < packetSize; i++){
		//p.payload[i] = (char)j.read();
		printf("%c", Jora.read());
	}
	printf("\n");
	//m.new_msg = true;
}

void setup(){
	printf("Setting up Jora Gateway...\n");

	wiringPiSetup();

	Jora.setPins(CS_PIN, RESET_PIN, IRQ_PIN);

	if(Jora.begin(DEFAULT_FREQ) < 0){
		printf("Jora begin failed!\n");
		while(true){	//stay here
			printf(".");
			delay(3000);
		}
	}

#ifndef ARDUINO
	Jora.setTxPower(14, PA_OUTPUT_PA_BOOST_PIN);	//gateway uses inAir9B (PA_BOOST)
#else
	Jora.setTxPower(1, PA_OUTPUT_RFO_PIN);
#endif
	
	Jora.onReceive(onReceive);
	Jora.receive();

#ifdef DEBUG
	printf("CH: %#07x\n", Jora.getFrequency());
#endif

	printf("Jora begin Success!\n");
}

//for basic receive testing
void receiver(){
	int packetSize = Jora.parsePacket();
	if(packetSize){
		printf("Received packet: ");
		while(Jora.available()){
			printf("%c", (char)Jora.read());
		}
		printf(" with RSSI %d\n", Jora.packetRssi());
	}
}

//for basic sender testing
void sender(int counter){
	printf("Sending packet: %d\n", counter);

	//Jora.beginPacket();
	//Jora.print("hello");
	//Jora.print(counter);
	//Jora.endPacket();

}

int main (int argc, char *argv[]){
	
	setup();
	printf("Starting Jora Gateway...\n");

	int counter = 0;


	while(true){
		//receiver();
		//sender(counter);
		counter++;
		//delay(1000);

		//printf("MAIN: _onReceive: %d\n", j.getOR());
		//delay(50);
	};	
	printf("Finished..\n");
	return (0);
}


