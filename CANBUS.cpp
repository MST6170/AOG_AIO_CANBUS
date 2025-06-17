// copied from https://github.com/lansalot/AgOpenGPS_Boards/blob/CANBUSEnable/TeensyModules/V4.1/Firmware/Autosteer_gps_teensy_v4_1/CANBUS.cpp
// and changed the ID and Byte for AutosteerButton on NH FR780
// OK, this is a template class for CANBUS
// Until we get proper CANBUS support in AOG, this will have to do
// You'll need to check your board version and use the correct #define in the FlexCAN_T4 line

#include "CANBUS.h"

// AIO 2.x version board, pins 16 and 17, use CAN3
// AIO 2.x version board, pins 18 and 19, use CAN2
// AIO 4.x version board, pins 16 and 17, use CAN3
// 
// Change  vvvv
//FlexCAN_T4<CAN3, RX_SIZE_256, TX_SIZE_256> TeensyCAN;
extern FlexCAN_T4<CAN3, RX_SIZE_1024, TX_SIZE_1024> Can0; //added
// Change  ^^^^

const int8_t filterID = 0;
const int8_t byteOffset = 1;
const int8_t ANDValue = 2;
uint32_t lastCANCommand; // Need to delay processing CANBUS messages for a bit, so we don't process multiple messages

// UNCOMMENT ONLY ONE CANINFO LINE BELOW !!!


int CANInfo[3] = { 0x18FFBB31 , 0, 0x04 }; // NH, steer button on right panel


// ============================================
// You don't need to change anything below here
// ============================================



bool debugCANBUS = false;
CAN_message_t CANBUSData;

void CAN_Setup() {
	// we're only listening, so no need to claim an address?
	lastCANCommand = millis();
	Can0.begin();
	Can0.setBaudRate(250000); // , LISTEN_ONLY);
	Can0.enableFIFO();
	Can0.setFIFOFilter(REJECT_ALL);
	delay(1000);
	if (CANInfo[filterID] > 0xffff) {
		Serial.print("Setting extended filter 0x"); Serial.println(CANInfo[filterID], HEX);
		Can0.setFIFOFilter(0, CANInfo[filterID], EXT);
	}
	else {
		Serial.print("Setting standard filter 0x"); Serial.println(CANInfo[filterID], HEX);
		Can0.setFIFOFilter(0, CANInfo[filterID], STD);
	}
	if (debugCANBUS) Serial.println("Initialised CANBUS");
	delay(3000);
}

void CANBUS_Receive() {
	if (Can0.read(CANBUSData)) {
		// if we receive a filtered message, do the bit checking
		// There should be no need to check a CANBUS ID because we're filtering on it - should only receive filtered messages
		if (debugCANBUS) {
			Serial.print("CANBUS ID: ");
			Serial.print(CANBUSData.id, HEX); Serial.print("  ");
			Serial.print("Byte of interest at offset "); Serial.print(CANInfo[byteOffset]); Serial.print("  Value: ");
			Serial.println(CANBUSData.buf[CANInfo[byteOffset]], HEX);
		}
		if ((CANBUSData.buf[CANInfo[byteOffset]] & CANInfo[ANDValue]) && millis() - lastCANCommand > 1000) {
			lastCANCommand = millis();
			if (debugCANBUS) Serial.println("CANBUS bit state: true");
			// not 100% these are correct, lemme know !
			// wasn't correct - thanks to Valentin and BSE for letting me know :D
			if (currentState == 1)
			{
				currentState = 0;
				steerSwitch = 0;
				if (debugCANBUS) Serial.println("Engaging steering");
			}
			else
			{
				if (debugCANBUS) Serial.println("Disengaging steering");
				currentState = 1;
				steerSwitch = 1;
			}
		}
	}
}
