#include "Jora.h"
/*
#ifdef ARDUINO
#define printf	Definitions::prf
#endif
*/

JoraClass::JoraClass():
#ifdef ARDUINO
	_spiSettings(LORA_DEFAULT_SPI_FREQUENCY, MSBFIRST, SPI_MODE0),
	_spi(&LORA_DEFAULT_SPI),
#endif
	_cs(CS_PIN),
	_reset(RESET_PIN),
	_irq(IRQ_PIN),
	_frequency(0),
	_packetIndex(0),
	_implicitHeaderMode(0),
	_onReceive(NULL)
{

}

//Sets up the onReceive ISR
void JoraClass::onReceive(void(*callback)(int))
{
	_onReceive = callback;
	
	if (callback) {
		pinMode(_irq, INPUT);
		writeRegister(REG_DIO_MAPPING_1, 0x00);
#ifdef ARDUINO
#ifdef SPI_HAS_NOTUSINGINTERRUPT
		SPI.usingInterrupt(digitalPinToInterrupt(_irq));
#endif
		attachInterrupt(digitalPinToInterrupt(_irq), JoraClass::onDio0Rise, RISING);
	}
	else {
		detachInterrupt(digitalPinToInterrupt(_irq));
#ifdef SPI_HAS_NOTUSINGINTERRUPT
		SPI.notUsingInterrupt(digitalPinToInterrupt(_irq));
#endif
	}
#else
		//for pi
		if(wiringPiISR(_irq, INT_EDGE_RISING, &JoraClass::onDio0Rise) < 0){	//was &JoraClass
			_logger.log("failed to setup ISR\n");
			while(true){	//stay here
				_logger.log(".");
				delay(3000);
			}

		}else{
			_logger.log("ISR setup Success!\n");
		}	
	}
#endif
}

int JoraClass::parsePacket(int size)
{
  int packetLength = 0;
  int irqFlags = readRegister(REG_IRQ_FLAGS);
	//_logger.log("irgFlags: %d\n", irqFlags);
	//_logger.log("Mode: %d\n", readRegister(REG_OP_MODE));
  if (size > 0) {
    implicitHeaderMode();

    writeRegister(REG_PAYLOAD_LENGTH, size & 0xff);
  } else {
    explicitHeaderMode();
  }

  // clear IRQ's
  writeRegister(REG_IRQ_FLAGS, irqFlags);

  if ((irqFlags & IRQ_RX_DONE_MASK) && (irqFlags & IRQ_PAYLOAD_CRC_ERROR_MASK) == 0) {
    // received a packet
    _packetIndex = 0;

    // read packet length
    if (_implicitHeaderMode) {
      packetLength = readRegister(REG_PAYLOAD_LENGTH);
    } else {
      packetLength = readRegister(REG_RX_NB_BYTES);
    }
	
    // set FIFO address to current RX address
    writeRegister(REG_FIFO_ADDR_PTR, readRegister(REG_FIFO_RX_CURRENT_ADDR));

    // put in standby mode
    idle();
  } else if (readRegister(REG_OP_MODE) != (MODE_LONG_RANGE_MODE | MODE_RX_SINGLE)) {
    // not currently in RX mode

    // reset FIFO address
    writeRegister(REG_FIFO_ADDR_PTR, 0);

    // put in single RX mode
    writeRegister(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_RX_SINGLE);
  }

  return packetLength;
}

int JoraClass::packetRssi()
{
  return (readRegister(REG_PKT_RSSI_VALUE) - (_frequency < 868E6 ? 164 : 157));
}

int JoraClass::beginPacket(int implicitHeader)
{
  // put in standby mode
  idle();
 
  if (implicitHeader) {
	  implicitHeaderMode();
  } else {
    explicitHeaderMode();
  }
 
  // reset FIFO address and payload length
  writeRegister(REG_FIFO_ADDR_PTR, 0);
  writeRegister(REG_PAYLOAD_LENGTH, 0);
 
  return 1;
}

void JoraClass::flush()
{
}

int JoraClass::available()
{
	return (readRegister(REG_RX_NB_BYTES) - _packetIndex);
}

int JoraClass::read()
{
	if (!available()) {
		return -1;
	}

	_packetIndex++;
	uint8_t r = readRegister(REG_FIFO);
	//_logger.log("%d", r);
	return r;//readRegister(REG_FIFO);
}

int JoraClass::peek()
{
	if (!available()) {
		return -1;
	}

	// store current FIFO address
	int currentAddress = readRegister(REG_FIFO_ADDR_PTR);

	// read
	uint8_t b = readRegister(REG_FIFO);

	// restore FIFO address
	writeRegister(REG_FIFO_ADDR_PTR, currentAddress);

	return b;
}

size_t JoraClass::write(uint8_t byte)
{
	return write(&byte, sizeof(byte));
}

size_t JoraClass::write(const uint8_t *buffer, size_t size)
{
  int currentLength = readRegister(REG_PAYLOAD_LENGTH);

  // check size
  if ((currentLength + size) > MAX_PKT_LENGTH) {
    size = MAX_PKT_LENGTH - currentLength;
  }

  // write data
  for (size_t i = 0; i < size; i++) {
    writeRegister(REG_FIFO, buffer[i]);
	//_logger.log("%d", buffer[i]);
  }

  // update length
  writeRegister(REG_PAYLOAD_LENGTH, currentLength + size);

  return size;
}

void setPayload(uint8_t byte){
	//set standby mode to write
  //writeRegister(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_STDBY);

	//setting addr pointer in FIFO buf
	//writeRegister(REG_FIFO_ADDR_PTR, 0x80);
	
	//write REG_FIFO with data
//	writeRegister(REG_FIFO, byte);

}

int JoraClass::endPacket()
{	
  // put in TX mode
	//Serial.println("Placing in tx mode: ");Serial.println(MODE_LONG_RANGE_MODE | MODE_TX)
  writeRegister(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_TX);

  // wait for TX done
  while ((readRegister(REG_IRQ_FLAGS) & IRQ_TX_DONE_MASK) == 0) {
    //yield();
	delay(1);	//ont have yield
  }
  
  // clear IRQ's
  writeRegister(REG_IRQ_FLAGS, IRQ_TX_DONE_MASK);

  return 1;
}

void JoraClass::sendPayload(){
	//set TX mode
	//writeRegister(REG_OP_MODE, MODE_TX);

	//endPacket();
	//set back to receive mode??
	//idle();
}

void JoraClass::receive(int size)
{
  if (size > 0) {
    implicitHeaderMode();

    writeRegister(REG_PAYLOAD_LENGTH, size & 0xff);
  } else {
    explicitHeaderMode();
  }

  writeRegister(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_RX_CONTINUOUS);
}

void JoraClass::setPins(int cs, int reset, int irq)
{
  _cs = cs;
  _reset = reset;
  _irq= irq;
}

void JoraClass::explicitHeaderMode()
{
  _implicitHeaderMode = 0;

  writeRegister(REG_MODEM_CONFIG_1, readRegister(REG_MODEM_CONFIG_1) & 0xfe);
}

void JoraClass::implicitHeaderMode()
{
  _implicitHeaderMode = 1;

  writeRegister(REG_MODEM_CONFIG_1, readRegister(REG_MODEM_CONFIG_1) | 0x01);
}

//for pi because ISR just goes off flat chat when a new message comes in
void JoraClass::handleMessage() {
	
	int irqFlags = readRegister(REG_IRQ_FLAGS);
	
	//_logger.log("irqFlags: %d\n", irqFlags);

	writeRegister(REG_IRQ_FLAGS, irqFlags);

	if ((irqFlags & IRQ_PAYLOAD_CRC_ERROR_MASK) == 0) {
		// received a packet
		_packetIndex = 0;

		// read packet length
		int packetLength = _implicitHeaderMode ? readRegister(REG_PAYLOAD_LENGTH) : readRegister(REG_RX_NB_BYTES);

		// set FIFO address to current RX address
		writeRegister(REG_FIFO_ADDR_PTR, readRegister(REG_FIFO_RX_CURRENT_ADDR));

		//_logger.log("_onReceive: %d\n", getOR());

		//_logger.log("onReceive: %d\n", _onReceive);
		if (_onReceive) {
			//_logger.log("onReceive > 0\n");
			_onReceive(packetLength);
		}

		// reset FIFO address
		writeRegister(REG_FIFO_ADDR_PTR, 0);

		newMessage = false;
	}
}

void JoraClass::handleDio0Rise()
{
	newMessage = true;
#ifdef ARDUINO

	int irqFlags = readRegister(REG_IRQ_FLAGS);

	//_logger.log("irqFlags: %d\n", irqFlags);

	writeRegister(REG_IRQ_FLAGS, irqFlags);

	if ((irqFlags & IRQ_PAYLOAD_CRC_ERROR_MASK) == 0) {
		// received a packet
		_packetIndex = 0;

		// read packet length
		int packetLength = _implicitHeaderMode ? readRegister(REG_PAYLOAD_LENGTH) : readRegister(REG_RX_NB_BYTES);

		// set FIFO address to current RX address
		writeRegister(REG_FIFO_ADDR_PTR, readRegister(REG_FIFO_RX_CURRENT_ADDR));

		//_logger.log("_onReceive: %d\n", getOR());

		//_logger.log("onReceive: %d\n", _onReceive);
		if (_onReceive) {
			//_logger.log("onReceive > 0\n");
			_onReceive(packetLength);
		}

		// reset FIFO address
		writeRegister(REG_FIFO_ADDR_PTR, 0);

		newMessage = false;
	}
#endif
}

void JoraClass::onDio0Rise()
{
  Jora.handleDio0Rise();
}

#ifndef ARDUINO
uint8_t JoraClass::setSPI(){
	_logger.log(Definitions::debug, "Starting SPI...\n");

	if(!bcm2835_init()){
		_logger.log(Definitions::debug, "bcm2835_init failed. Are you running as root??\n");
		return -1;
	}else if(!bcm2835_spi_begin()){
		_logger.log(Definitions::debug, "bcm2835_spi_begin failed\n");
		return -1;
	}else{
		_logger.log(Definitions::debug, "bcm2835 initialized!\n");

		//all defaults
		bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
		bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
		bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_64);	//65536

		return 0;
	}

}
#endif

int JoraClass::begin(long freq){

	pinMode(_cs, OUTPUT);
	digitalWrite(_cs, HIGH);

#ifndef ARDUINO
	if(_reset != -1){
		_logger.log(Definitions::debug, "Resetting Jora...\n");
		pinMode(_reset, OUTPUT);
		digitalWrite(_reset, LOW);
		delay(10);
		digitalWrite(_reset, HIGH);
		delay(10);
	}

	if(setSPI() < 0){
		_logger.log(Definitions::debug, "setSPI failed!");
		return -1;
	}
#else

	if (_reset != -1) {
		pinMode(_reset, OUTPUT);

		// perform reset
		digitalWrite(_reset, LOW);
		delay(10);
		digitalWrite(_reset, HIGH);
		delay(10);
	}

	// start SPI
	_spi->begin();
#endif

	uint8_t version = readRegister(REG_VERSION);
	if(version != 0x12){
		return -1;
	}

#ifdef DEBUG
	_logger.log(Definitions::debug, "Semtech version: %#02x\n", version);
#endif

	sleep();

	setFrequency(freq);	//set lora radio freq

	//set base addresses
	writeRegister(REG_FIFO_TX_BASE_ADDR, 0);
	writeRegister(REG_FIFO_RX_BASE_ADDR, 0);

	//set LNA boost
	writeRegister(REG_LNA, readRegister(REG_LNA) | 0x03);

	//set auto AGC
	writeRegister(REG_MODEM_CONFIG_3, 0x04);

	//set output power to 14 dBm
	setTxPower(17);	//default - can be overwritten later

	idle();

	return 0;
}

void JoraClass::setFrequency(uint64_t frequency)
{
	_logger.log(Definitions::debug, "Setting radio freq...\n");
  _frequency = frequency;

  uint64_t frf = ((uint64_t)frequency << 19) / 32000000;

  writeRegister(REG_FRF_MSB, (uint8_t)(frf >> 16));
  writeRegister(REG_FRF_MID, (uint8_t)(frf >> 8));
  writeRegister(REG_FRF_LSB, (uint8_t)(frf >> 0));

}

long JoraClass::getFrequency(){

	uint8_t msb = readRegister(REG_FRF_MSB);
	uint8_t mid = readRegister(REG_FRF_MID);
	uint8_t lsb = readRegister(REG_FRF_LSB);

	uint32_t ch = ((uint32_t)msb << 16) + ((uint32_t)mid << 8) + lsb;
	
	return ch;
}

void JoraClass::sleep()
{
  writeRegister(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_SLEEP);
#ifdef DEBUG
  _logger.log(Definitions::debug,  "Mode: Sleep (%#02x)\n", readRegister(REG_OP_MODE));
#endif
}

void JoraClass::idle()
{	
  writeRegister(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_STDBY);
#ifdef DEBUG
  _logger.log("standby\n");
  //_logger.log(Definitions::debug, "Mode: Standby (%#02x)\n", readRegister(REG_OP_MODE));
#endif
}

void JoraClass::end(){
	sleep();	

#ifdef ARDUINO
	// stop SPI
	_spi->end();
#else
	bcm2835_spi_end();
	bcm2835_close();
#endif
}

void JoraClass::setTxPower(int level, int outputPin)
{
  if (PA_OUTPUT_RFO_PIN == outputPin) {
    // RFO
    if (level < 0) {
      level = 0;
    } else if (level > 14) {
      level = 14;
    }

    writeRegister(REG_PA_CONFIG, 0x70 | level);
  } else {
    // PA BOOST
    if (level < 2) {
      level = 2;
    } else if (level > 17) {
      level = 17;
    }

    writeRegister(REG_PA_CONFIG, PA_BOOST | (level - 2));
  }
}

/*
uint8_t JoraClass::singleTransfer(uint8_t addr, uint8_t val){

	dWrite(_cs, 0);
	delay(1);
	bitClear(addr, 7);	//set to read from reg
	
    	txbuf[0] = addr;
    	txbuf[1] = 0x00;	//val;
    
	dWrite(_cs, 0);
	bcm2835_spi_transfernb(txbuf, rxbuf, sizeof(txbuf));
	_logger.log("size_t: %d\n", sizeof(txbuf));
	dWrite(_cs, 1);

	return 0;	//data in rxbuf
}
*/

void JoraClass::writeRegister(uint8_t addr, uint8_t data){
#ifdef ARDUINO
	singleTransfer(addr | 0x80, data);
#else
	//singleTransfer(addr | 0x08, val);
	digitalWrite(_cs, LOW);
	delay(1);
	bitSet(addr, 7);	//bit 7 set to read from reg

	txbuf[0] = addr;
	txbuf[1] = data;

	digitalWrite(_cs, LOW);
	bcm2835_spi_transfernb(txbuf, rxbuf, sizeof(txbuf));
	digitalWrite(_cs, HIGH);
#endif
}

#ifdef ARDUINO
uint8_t JoraClass::singleTransfer(uint8_t address, uint8_t value)
{
	digitalWrite(_cs, LOW);

	_spi->beginTransaction(_spiSettings);
	
	//Serial.print(address, HEX); Serial.print(": "); Serial.println(value, HEX);

	_spi->transfer(address);
	
	uint8_t response = _spi->transfer(value);
	
	_spi->endTransaction();

	digitalWrite(_cs, HIGH);

	return response;
}
#endif

uint8_t JoraClass::readRegister(uint8_t addr){
#ifdef ARDUINO
	return singleTransfer(addr & 0x7f, 0x00);
#else

	digitalWrite(_cs, LOW);
	bitClear(addr, 7);
	
	txbuf[0] = addr;
	txbuf[1] = 0x00;

	digitalWrite(_cs, LOW);
	bcm2835_spi_transfernb(txbuf, rxbuf, sizeof(txbuf));
	digitalWrite(_cs, HIGH);
	//printf("%d", rxbuf[1]);
	return rxbuf[1];

	/*txbuf[0] = addr & 0x7F;
	txbuf[1] = 0x00;

  	bcm2835_gpio_write(CS_PIN,0);
	bcm2835_spi_transfernb( txbuf, rxbuf, sizeof(txbuf) );
	bcm2835_gpio_write(CS_PIN,1);

	return rxbuf[1];	//data is in rxbuf
	*/
#endif
}

int JoraClass::getRegVal(uint8_t addr){
	return readRegister(addr);
}

JoraClass Jora;






