#include <GasesBoard.h>

bool GasesBoard::begin()
{
	if (alreadyStarted) return true;

	if (!I2Cdetect(sht31Address) ||
			!(I2Cdetect(Slot1.electrode_A.resistor.address)) ||
			!(I2Cdetect(Slot1.electrode_W.resistor.address)) ||
			!(I2Cdetect(Slot2.electrode_A.resistor.address)) ||
			!(I2Cdetect(Slot2.electrode_W.resistor.address)) ||
			!(I2Cdetect(Slot3.electrode_A.resistor.address)) ||
			!(I2Cdetect(Slot3.electrode_W.resistor.address))) return false;

	sht31.begin();

	// Set all potentiometers to 0
	setPot(Slot1.electrode_A, 0);
	setPot(Slot1.electrode_W, 0);
	setPot(Slot2.electrode_A, 0);
	setPot(Slot2.electrode_W, 0);
	setPot(Slot3.electrode_A, 0);
	setPot(Slot3.electrode_W, 0);

	alreadyStarted = true;

	return true;
}

float GasesBoard::getTemperature()
{
	sht31.update(true);
	return sht31.temperature;
}
float GasesBoard::getHumidity()
{
	sht31.update(true);
	return sht31.humidity;
}
uint32_t GasesBoard::getPot(Electrode wichElectrode)
{

	return ((255 - readI2C(wichElectrode.resistor.address, wichElectrode.resistor.channel)) * ohmsPerStep);
}
void GasesBoard::setPot(Electrode wichElectrode, uint32_t value)
{

	int data=0x00;
	if (value>100000) value = 100000;
	data = 255 - (int)(value/ohmsPerStep);		// POT's are connected 'upside down' (255 - step)

	writeI2C(wichElectrode.resistor.address, 16, 192);        	// select WR (volatile) registers in POT
	writeI2C(wichElectrode.resistor.address, wichElectrode.resistor.channel, data);
}
uint8_t GasesBoard::getPGAgain(MCP342X adc)
{
	uint8_t gainPGA = adc.getConfigRegShdw() & 0x3;
	return pow(2, gainPGA);
}
float GasesBoard::getElectrodeGain(Electrode wichElectrode)
{

	return (((getPot(wichElectrode) + 85) / 10000.0f) + 1) * getPGAgain(wichElectrode.adc);
}
// Returns electrode value in mV
double GasesBoard::getElectrode(Electrode wichElectrode)
{

	static int32_t result;

	// Gain can be changed before calling this funtion with: wichElectrode.gain = newGain (0->gain of 1, 1->gain of 2, 2->gain of 3 or 3->gain of 8)
	wichElectrode.adc.configure( MCP342X_MODE_ONESHOT | MCP342X_SIZE_18BIT | wichElectrode.gain);
	wichElectrode.adc.startConversion(wichElectrode.channel);
	wichElectrode.adc.getResult(&result);

	return (result * 0.015625) / getElectrodeGain(wichElectrode);
}
float GasesBoard::getPPM(gasesBoardSensor wichSlot)
{
    switch(wichSlot.calData.GAS) {

        case GB_CO: {
            // CO [ppm] = ((6.36 * WE - ZERO_CURR_W) - n*(6.36 * AE + ZERO_CURR_A)) / SENSITIVITY
            float resultCO = ((6.36 * getElectrode(wichSlot.electrode_W)) - wichSlot.calData.ZERO_CURR_W -
                  wichSlot.calData.ZERO_CURR_W/wichSlot.calData.ZERO_CURR_A * (6.36 * getElectrode(wichSlot.electrode_A) - wichSlot.calData.ZERO_CURR_A)) /
                  wichSlot.calData.SENSITIVITY[0];
	    resultCO =  max(0, resultCO);
	    return resultCO;
            break;

	} case GB_NO2: {
            // NO2 [ppm] = ((6.36 * WE - ZERO_CURR_W) - n*(6.36 * AE - ZERO_CURR_A)) / SENSITIVITY
            // NO2 [ppb] = ((6.36 * WE - ZERO_CURR_W) - n*(6.36 * AE - ZERO_CURR_A)) / SENSITIVITY
            float resultNO2 = (((6.36 * getElectrode(wichSlot.electrode_W) - wichSlot.calData.ZERO_CURR_W) -
                wichSlot.calData.ZERO_CURR_W/wichSlot.calData.ZERO_CURR_A * (6.36 * getElectrode(wichSlot.electrode_A)-wichSlot.calData.ZERO_CURR_A)) /
                wichSlot.calData.SENSITIVITY[0]) * 1000;
	    resultNO2 =  max(0, resultNO2);
	    return resultNO2;
            break;

	} case GB_NO2_O3: {
	    // O3 [ppm] =  ((6.36 * WE - ZERO_CURR_W) - n*(6.36 * AE - ZERO_CURR_A) - getPPM(NO2) * SENSITIVITY_NO2) / SENSITIVITY_O3
	    float resultO3 = ((6.36 * getElectrode(wichSlot.electrode_W) - wichSlot.calData.ZERO_CURR_W) -
		  ((wichSlot.calData.ZERO_CURR_W / wichSlot.calData.ZERO_CURR_A) * (6.36 * getElectrode(wichSlot.electrode_A) - wichSlot.calData.ZERO_CURR_A)) -
		  ((getPPM(Slot2) / 1000) * wichSlot.calData.SENSITIVITY[1])) /
		    wichSlot.calData.SENSITIVITY[0];
	    resultO3 =  max(0, resultO3);
	    return resultO3;
            break;
	}
    }
    return 0;
}
String GasesBoard::getUID()
{

	char data[24];
	uint8_t eeaddr = 0xf8;
	sprintf(data, "%02x:", readByte(eeaddr++));
	for(uint8_t pos = 0; pos<7; pos++){
		sprintf(data, "%s:%02x", data, readByte(eeaddr++));
	}
	return String(data);
}
bool GasesBoard::writeByte(uint8_t dataAddress, uint8_t data)
{
	WIRE.beginTransmission(eepromAddress);
	WIRE.write(dataAddress);
	WIRE.write(data);
	if (WIRE.endTransmission() == 0) return true;
	return false;
}
uint8_t GasesBoard::readByte(uint8_t dataAddress)
{
	WIRE.beginTransmission(eepromAddress);
	WIRE.write(dataAddress);
	if (WIRE.endTransmission(false)) return 0;
	if(!WIRE.requestFrom(eepromAddress, 1)) return 0;
	return WIRE.read();
}
bool GasesBoard::I2Cdetect(byte address)
{
	WIRE.beginTransmission(address);
	byte error = WIRE.endTransmission();

	if (error == 0) return true;
	else return false;
}
void GasesBoard::writeI2C(byte deviceaddress, byte instruction, byte data )
{
	WIRE.beginTransmission(deviceaddress);
	WIRE.write(instruction);
	WIRE.write(data);
	WIRE.endTransmission();
}
byte GasesBoard::readI2C(byte deviceaddress, byte instruction)
{
	byte  data = 0x0000;
	WIRE.beginTransmission(deviceaddress);
	WIRE.write(instruction);
	WIRE.endTransmission();
	WIRE.requestFrom(deviceaddress,1);
	unsigned long time = millis();
	while (!WIRE.available()) if ((millis() - time)>500) return 0x00;
	data = WIRE.read();
	return data;
}

#ifdef gasesBoardTest
void GasesBoard::runTester(uint8_t wichSlot)
{

	Electrode wichElectrode_W;
	Electrode wichElectrode_A;

	switch(wichSlot) {
		case 1: {
				wichElectrode_W = Slot1.electrode_W;
				wichElectrode_A = Slot1.electrode_A;
				break;
			} case 2: {
				wichElectrode_W = Slot2.electrode_W;
				wichElectrode_A = Slot2.electrode_A;
				break;
			} case 3: {
				wichElectrode_W = Slot3.electrode_W;
				wichElectrode_A = Slot3.electrode_A;
				break;
			}
		default: break;
	}

	// Print headers
	SerialUSB.println("testW,readW,testA,readA");

	// Output from -1400 to +1400 nA
	for (int16_t i=-1400; i<1400; i++) {
		tester.setCurrent(tester.electrode_W, i);
		double currVoltW = getElectrode(wichElectrode_W);
		if (preVoltW != -99) if ((currVoltW - preVoltW) < threshold) maxErrorsW--;
		preVoltW = currVoltW;
		if (maxErrorsW == 0) SerialUSB.println("Working electrode fail !!!");

		tester.setCurrent(tester.electrode_A, i);
		double currVoltA = getElectrode(wichElectrode_A);
		if (preVoltA != -99) if ((currVoltA - preVoltA) < threshold) maxErrorsA--;
		preVoltA = currVoltA;
		if (maxErrorsA == 0) SerialUSB.println("Auxiliary electrode fail !!!");

		SerialUSB.print(tester.getCurrent(tester.electrode_W));
		SerialUSB.print(",");
		SerialUSB.print(currVoltW, 8);
		SerialUSB.print(",");
		SerialUSB.print(tester.getCurrent(tester.electrode_A));
		SerialUSB.print(",");
		SerialUSB.println(currVoltA, 8);
	}
}
void GasesBoard::setTesterCurrent(int16_t wichCurrent, uint8_t wichSlot)
{

	Electrode wichElectrode_W;
	Electrode wichElectrode_A;

	switch(wichSlot) {
		case 1: {
				wichElectrode_W = Slot1.electrode_W;
				wichElectrode_A = Slot1.electrode_A;
				break;
			} case 2: {
				wichElectrode_W = Slot2.electrode_W;
				wichElectrode_A = Slot2.electrode_A;
				break;
			} case 3: {
				wichElectrode_W = Slot3.electrode_W;
				wichElectrode_A = Slot3.electrode_A;
				break;
			}
		default: break;
	}

	SerialUSB.print("Setting test current to: ");
	SerialUSB.println(wichCurrent);

	tester.setCurrent(tester.electrode_W, wichCurrent);
	tester.setCurrent(tester.electrode_A, wichCurrent);

	SerialUSB.print("Tester Electrode W: ");
	SerialUSB.println(tester.getCurrent(tester.electrode_W));
	SerialUSB.print("Gases Board ");
	SerialUSB.print(wichSlot);
	SerialUSB.print("W: ");
	SerialUSB.println(getElectrode(wichElectrode_W));

	SerialUSB.print("Tester Electrode A: ");
	SerialUSB.println(tester.getCurrent(tester.electrode_A));
	SerialUSB.print("Gases Board ");
	SerialUSB.print(wichSlot);
	SerialUSB.print("A: ");
	SerialUSB.println(getElectrode(wichElectrode_A));
}
bool GasesBoard::autoTest()
{
	Electrode wichElectrode_W;
	Electrode wichElectrode_A;

	// Autoselect slot based on response (if none responds it fails)
	for (uint8_t i=1; i<4; i++) {
		switch(i) {
			case 1:	wichElectrode_W = Slot1.electrode_W; wichElectrode_A = Slot1.electrode_A; break;
			case 2:	wichElectrode_W = Slot2.electrode_W; wichElectrode_A = Slot2.electrode_A; break;
			case 3: wichElectrode_W = Slot3.electrode_W; wichElectrode_A = Slot3.electrode_A; break;
		}
		tester.setCurrent(tester.electrode_W, 0);
		double zeroVolt = getElectrode(wichElectrode_W);
		tester.setCurrent(tester.electrode_W, 500);
		double fiveVolt = getElectrode(wichElectrode_W);
		if ((fiveVolt - zeroVolt) > 5) {
			SerialUSB.println("Tesing slot " + String(i));
			break;
		}
	}
	uint8_t multiplier = 25;
	for (int16_t i=-1400; i<1400; i+=multiplier) {

		tester.setCurrent(tester.electrode_W, i);
		double currVoltW = getElectrode(wichElectrode_W);
		if (preVoltW != -99) if ((currVoltW - preVoltW) < threshold * multiplier) maxErrorsW--;
		preVoltW = currVoltW;
		if (maxErrorsW == 0) {
			SerialUSB.println("\r\nWorking electrode fail !!!");
			return false;
		}

		tester.setCurrent(tester.electrode_A, i);
		double currVoltA = getElectrode(wichElectrode_A);
		if (preVoltA != -99) if ((currVoltA - preVoltA) < threshold) maxErrorsA--;
		preVoltA = currVoltA;
		if (maxErrorsA == 0) {
			SerialUSB.println("\r\nAuxiliary electrode fail !!!");
			return false;
		}

		SerialUSB.print(".");
	}
	if (maxErrorsW > 0 && maxErrorsA > 0) {
		SerialUSB.println("\r\nTest OK");
		return true;
	}
	return false;
}
#endif

Gases_SHT31::Gases_SHT31(TwoWire *localWire)
{
	_Wire = localWire;
}
bool Gases_SHT31::begin()
{
	_Wire->begin();
	_Wire->beginTransmission(address);
	byte error = _Wire->endTransmission();
	if (error != 0) return false;

	delay(1); 		// In case the device was off
	sendComm(SOFT_RESET); 	// Send reset command
	delay(50); 		// Give time to finish reset
	update(true);

	return true;
}
bool Gases_SHT31::stop()
{
	// It will go to idle state by itself after 1ms
	return true;
}
bool Gases_SHT31::update(bool wait)
{
	uint32_t elapsed = millis() - lastTime;
	if (elapsed < timeout) delay(timeout - elapsed);

	uint8_t readbuffer[6];
	sendComm(SINGLE_SHOT_HIGH_REP);

	_Wire->requestFrom(address, (uint8_t)6);
	// Wait for answer (datasheet says 15ms is the max)
	uint32_t started = millis();
	while(_Wire->available() != 6) {
		if (millis() - started > timeout) return 0;
	}

	// Read response
	for (uint8_t i=0; i<6; i++) readbuffer[i] = _Wire->read();

	uint16_t ST, SRH;
	ST = readbuffer[0];
	ST <<= 8;
	ST |= readbuffer[1];

	// Check Temperature crc
	if (readbuffer[2] != crc8(readbuffer, 2)) return false;
	SRH = readbuffer[3];
	SRH <<= 8;
	SRH |= readbuffer[4];

	// check Humidity crc
	if (readbuffer[5] != crc8(readbuffer+3, 2)) return false;
	double temp = ST;
	temp *= 175;
	temp /= 0xffff;
	temp = -45 + temp;
	temperature = (float)temp;

	double shum = SRH;
	shum *= 100;
	shum /= 0xFFFF;
	humidity = (float)shum;

	lastTime = millis();

	return true;
}
void Gases_SHT31::sendComm(uint16_t comm)
{
	_Wire->beginTransmission(address);
	_Wire->write(comm >> 8);
	_Wire->write(comm & 0xFF);
	_Wire->endTransmission();
}
uint8_t Gases_SHT31::crc8(const uint8_t *data, int len)
{

	/* CRC-8 formula from page 14 of SHT spec pdf */

	/* Test data 0xBE, 0xEF should yield 0x92 */

	/* Initialization data 0xFF */
	/* Polynomial 0x31 (x8 + x5 +x4 +1) */
	/* Final XOR 0x00 */

	const uint8_t POLYNOMIAL(0x31);
	uint8_t crc(0xFF);

	for ( int j = len; j; --j ) {
		crc ^= *data++;
		for ( int i = 8; i; --i ) {
			crc = ( crc & 0x80 )
				? (crc << 1) ^ POLYNOMIAL
				: (crc << 1);
		}
	}
	return crc;
}

