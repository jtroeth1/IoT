#include <SPI.h>
#include <LoRa.h>

int counter = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("LoRa ping test");

  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
}

void loop() {

  //wait for message, then send it back
  int mSize = LoRa.parsePacket();
  if(mSize){    
    int message[mSize];
    Serial.print("Received: "); 
    while (LoRa.available()){
      for(int i = 0; i < mSize; i++){
        int b = LoRa.read(); 
        message[i] = b; 
        Serial.print((char)b);
      }
     }
     Serial.print(" (");Serial.print(mSize);Serial.println(")");

     //message[1] = 111;  //make reply pong
     Serial.print("Sending: ");
     LoRa.beginPacket();
     for(int i = 0; i < mSize; i++){           
      LoRa.write(message[i]);      
     }
     LoRa.endPacket();
     
     for(int i = 0; i < mSize; i++)
      Serial.print((char)message[i]);
     Serial.println("");
    
  }

}
