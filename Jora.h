#pragma once

#include "Logging.h"
#include "Definitions.h"

#ifndef ARDUINO
#include <stdio.h>
#include "bcm2835.h"	//"" not <> meaning we use the latest one in local folder
#include <unistd.h>
#include <wiringPi.h>
#else
#include <Arduino.h>
#include "SPI_J.h"
#endif

//pins - be careful with pin number (is it GPIO, header or wiringPi??)
//can be different!!
#ifdef ARDUINO
#define CS_PIN	10
#define RESET_PIN	4
#define IRQ_PIN	2
#else
#define CS_PIN	8	//gpio8 (ce0) (24th pin) wp10!!!
#define RESET_PIN	2	//gpio_gen2 gpio27 (13th pin) wp2
#define IRQ_PIN	0	//dio0 gpio_gen0 gpio17 (11th pin)	wp0
#endif

#define PA_OUTPUT_RFO_PIN	0
#define PA_OUTPUT_PA_BOOST_PIN	1

// registers
#define REG_FIFO                 0x00
#define REG_OP_MODE              0x01
#define REG_FRF_MSB              0x06
#define REG_FRF_MID              0x07
#define REG_FRF_LSB              0x08
#define REG_PA_CONFIG            0x09
#define REG_LNA                  0x0c
#define REG_FIFO_ADDR_PTR        0x0d
#define REG_FIFO_TX_BASE_ADDR    0x0e
#define REG_FIFO_RX_BASE_ADDR    0x0f
#define REG_FIFO_RX_CURRENT_ADDR 0x10
#define REG_IRQ_FLAGS            0x12
#define REG_RX_NB_BYTES          0x13
#define REG_PKT_SNR_VALUE        0x19
#define REG_PKT_RSSI_VALUE       0x1a
#define REG_MODEM_CONFIG_1       0x1d
#define REG_MODEM_CONFIG_2       0x1e
#define REG_PREAMBLE_MSB         0x20
#define REG_PREAMBLE_LSB         0x21
#define REG_PAYLOAD_LENGTH       0x22
#define REG_MODEM_CONFIG_3       0x26
#define REG_FREQ_ERROR_MSB       0x28
#define REG_FREQ_ERROR_MID       0x29
#define REG_FREQ_ERROR_LSB       0x2a
#define REG_RSSI_WIDEBAND        0x2c
#define REG_DETECTION_OPTIMIZE   0x31
#define REG_DETECTION_THRESHOLD  0x37
#define REG_SYNC_WORD            0x39
#define REG_DIO_MAPPING_1        0x40
#define REG_VERSION              0x42

// modes
#define MODE_LONG_RANGE_MODE     0x80
#define MODE_SLEEP               0x00
#define MODE_STDBY               0x01
#define MODE_TX                  0x03
#define MODE_RX_CONTINUOUS       0x05
#define MODE_RX_SINGLE           0x06

// PA config
#define PA_BOOST                 0x80

// IRQ masks
#define IRQ_TX_DONE_MASK           0x08
#define IRQ_PAYLOAD_CRC_ERROR_MASK 0x20
#define IRQ_RX_DONE_MASK           0x40

#define MAX_PKT_LENGTH           255

//macros
#define bitRead(val, bit)	(((val) >> (bit)) & 0x01)
#define bitSet(val, bit)	((val) |= ((1UL) << (bit)))
#define bitClear(val, bit)	((val) &= ~((1UL) << (bit)))

//comms
#define LORA_DEFAULT_SPI	SPI
#define LORA_DEFAULT_SPI_FREQUENCY	8E6

//sx config
#define DEFAULT_FREQ	915E6

//#define DEBUG
#ifdef ARDUINO 
class JoraClass : Stream {
#else
class JoraClass {
#endif
public:

	JoraClass();

	Logger _logger;

	int begin(long freq);
	void end();

	int parsePacket(int size = 0);
	int packetRssi();
	float packetSnr();
	long packetFrequencyError();

	//from print
	virtual size_t write(uint8_t byte);
	virtual size_t write(const uint8_t *buffer, size_t size);

	//from stream
	int available();
	int read();
	int peek();
	void flush();
	void readRawRegFIFO();

	int beginPacket(int implicitHeader = false);
	int endPacket();
	void sendPayload();

	int getRegVal(uint8_t addr);

	void onReceive(void(*callback)(int));
	void receive(int size= 0);

	int getOR();

	void idle();
	void sleep();

	void setTxPower(int level, int outputPin = PA_OUTPUT_PA_BOOST_PIN);
	void setFrequency(uint64_t freq);
	long getFrequency();
	//void setSpreadingFactor(int sf);
	void setSignalBandwidth(long sbw);
	//void setCodingRate4(int denominator);
	//void setPreambleLength(long len);
	//void setSyncWord(int sw);
	//void enableCrc();
	//void disableCrc();

	void setPins(int cs = CS_PIN, int reset = RESET_PIN, int dio0 = IRQ_PIN);
	uint8_t setSPI();
	//void setSPIFreq(uint32_t freq);

	//void dumpRegisters(Stream& out);

	char txbuf[2];
	char rxbuf[2];

	int lastISRTime = 0;

	bool newMessage;	//simply set by ISR
	void handleMessage();	//called when ready to deal with new message

private:
	void explicitHeaderMode();
	void implicitHeaderMode();

	void handleDio0Rise();
	int getSpreadingFactor();
	long getSignalBandwidth();

	void setLdoFlag();

	uint8_t readRegister(uint8_t addr);
	void writeRegister(uint8_t addr, uint8_t val);
	uint8_t singleTransfer(uint8_t addr, uint8_t val);

	static void onDio0Rise();

private:
	int _cs;
	int _reset;
	int _irq;
	uint64_t _frequency;
	int _packetIndex;
	int _implicitHeaderMode;
	void (*_onReceive)(int);

#ifdef ARDUINO
	SPISettings _spiSettings;
	SPIClass* _spi;
#endif

};

extern JoraClass Jora;