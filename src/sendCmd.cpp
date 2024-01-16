//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------ 
#include "defines.h"
#include "config.h"
#include "setup.h"
#include "moveTracks.h"
#include "sendCmd.h"

#include <stdint.h>
#include <ArduinoLogger.h>                         // [Serial / Terminal]

//------------------------------------------------------------------------
// objects
//------------------------------------------------------------------------
TrackSpeeds speeds;                                                       // Speed from moveTrack.cpp
SerialCommand Command;
SerialFeedback Feedback_Serial0,
               NewFeedback_Serial0,
               Feedback_Serial1,
               NewFeedback_Serial1,
               Feedback_Serial2,
               NewFeedback_Serial2;


//------------------------------------------------------------------------
// variables 
//------------------------------------------------------------------------
uint8_t idx_Serial = 0;                                                   // Index for new data pointer
  uint16_t bufStartFrame_Serial;                                            // Buffer Start Frame
  byte *p_Serial;                                                           // Pointer declaration for the new received data
  byte incomingByte_Serial;
  byte incomingBytePrev_Serial;
unsigned long iTimeSend = 0;
bool enable = 0;                                                          // from blynk
byte controlMode = 0;                                                     // from blynk

//------------------------------------------------------------------------
// sending procedure
//------------------------------------------------------------------------ 
void sendSerial(int8_t serialPort, int16_t uEnableMotors, int16_t uControlMode, int16_t uSpeedMaster, int16_t uSpeedSlave) {
  
  HardwareSerial* serial;
    
    switch(serialPort) {
      case 0:
        serial = &Serial;
        break;
      case 1:
        serial = &Serial1;
        break;
      case 2:
        serial = &Serial2;
        break;
      case 3:
        //serial = &Serial3;
        break;
        // Add more cases if needed for additional serial ports
      default:
        inf << terminal_name_device << "Invalid serial port number " << serialPort << " does not exist."<< endl; // Invalid serial port number
        return;
    }

  // Create command
  Command.start           = (uint16_t)START_FRAME;    // Start Frame  
  Command.enableMotors    = (int16_t)uEnableMotors;   // Enable Motors
  Command.controlMode     = (int16_t)uControlMode;   // Enable Motors
  Command.speedMaster     = (int16_t)uSpeedMaster;    // Speed Master Board
  Command.speedSlave      = (int16_t)uSpeedSlave;     // Speed Slave Board
  Command.checksum        = (uint16_t)( Command.start ^ 
                                        Command.enableMotors ^ 
                                        Command.controlMode ^ 
                                        Command.speedMaster ^ 
                                        Command.speedSlave);
  
  //inf << Command.start << " , " << Command.enableMotors << " , " << Command.controlMode  << " , " << Command.speedMaster << " , " << Command.speedSlave << " , " << Command.checksum << " , " << uSerial << endl;
  
  // Write to Serial
  serial->write((uint8_t *) &Command, sizeof(Command)); 
}

//------------------------------------------------------------------------
// receiver procedure
//------------------------------------------------------------------------ 
void receiveSerial(int8_t serialPort) {
  
  // Class //
  HardwareSerial* serial;
  SerialFeedback* serialFeedback;
  SerialFeedback* newSerialFeedback;

    // Select the appropriate serial port based on the input parameter
    switch(serialPort) {
        case 0:
            serial = &Serial;
            serialFeedback = &Feedback_Serial0;
            newSerialFeedback = &NewFeedback_Serial0;
            break;
        case 1:
            serial = &Serial1;
            serialFeedback = &Feedback_Serial1;
            newSerialFeedback = &NewFeedback_Serial1;
            break;
        case 2:
            serial = &Serial2;
            serialFeedback = &Feedback_Serial2;
            newSerialFeedback = &NewFeedback_Serial2;
            break;
        case 3:
            //serial = &Serial3;
            break;
        // Add more cases if needed for additional serial ports
        default:
            inf << terminal_name_device << "Invalid serial port number " << serialPort << " does not exist."<< endl; // Invalid serial port number
            return;
    }
    // Check for new data availability in the Serial buffer
    if (serial->available()) {
        incomingByte_Serial 	= serial->read();                                   // Read the incoming byte
        bufStartFrame_Serial	= ((uint16_t)(incomingByte_Serial) << 8) | incomingBytePrev_Serial;       // Construct the start frame
    } else {
      return;
    }

  // If DEBUG_RX is defined print all incoming bytes
  #ifdef DEBUG_SERIAL1_RX
        Serial.print(serial->read());
        return;
    #endif

    // Copy received data
    if (bufStartFrame_Serial == START_FRAME) {	                    // Initialize if new data is detected
        p_Serial       = (byte *)newSerialFeedback; // &
        *p_Serial++    = incomingBytePrev_Serial;
        *p_Serial++    = incomingByte_Serial;
        idx_Serial     = 2;	
    } else if (idx_Serial >= 2 && idx_Serial < sizeof(SerialFeedback)) {  // Save the new received data
        *p_Serial++    = incomingByte_Serial; 
        idx_Serial++;
    }	
    
    // Check if we reached the end of the package
    if (idx_Serial == sizeof(SerialFeedback)) {
        uint16_t checksum;
        checksum = (uint16_t)(newSerialFeedback->start ^ 
                              newSerialFeedback->cmd1 ^ 
                              newSerialFeedback->cmd2 ^ 
                              newSerialFeedback->speedR_meas ^ 
                              newSerialFeedback->speedL_meas ^ 
                              newSerialFeedback->batVoltage ^ 
                              newSerialFeedback->boardTempMaster ^ 
                              newSerialFeedback->boardTempSlave ^ 
                              newSerialFeedback->enableFinMaster ^ 
                              newSerialFeedback->enableFinSlave ^ 
                              newSerialFeedback->chargeStatus ^ 
                              newSerialFeedback->cmdLed);

        // Check validity of the new data
        if (newSerialFeedback->start == START_FRAME && checksum == newSerialFeedback->checksum) {
          // Copy the new data
          memcpy(serialFeedback, newSerialFeedback, sizeof(SerialFeedback));
          #ifdef PRINT_SERIAL_DATA
          // Print data to built-in Serial
          Serial.print("Serial"); Serial.print(serialPort);
          Serial.print(": 1: ");  Serial.print(serialFeedback->cmd1);
          Serial.print(" 2: ");  Serial.print(serialFeedback->cmd2);
          Serial.print(" 3: ");  Serial.print(serialFeedback->speedR_meas);
          Serial.print(" 4: ");  Serial.print(serialFeedback->speedL_meas);
          Serial.print(" 5: ");  Serial.print(serialFeedback->batVoltage);
          Serial.print(" 6: ");  Serial.print(serialFeedback->boardTempMaster);
          Serial.print(" 7: ");  Serial.print(serialFeedback->boardTempSlave);
          Serial.print(" 8: ");  Serial.print(serialFeedback->enableFinMaster);
          Serial.print(" 9: ");  Serial.print(serialFeedback->enableFinSlave);
          Serial.print(" 10: ");  Serial.print(serialFeedback->chargeStatus);
          Serial.print(" 11: ");  Serial.println(serialFeedback->cmdLed);
          #endif
        } else {
          #ifdef PRINT_SERIAL_DATA
          Serial.print("Software Serial"); Serial.print(serialPort); Serial.println(" Non-valid data skipped");
          #endif
        }
      idx_Serial = 0;    // Reset the index (it prevents to enter in this if condition in the next cycle)
    }

    // Update previous states
    incomingBytePrev_Serial = incomingByte_Serial; 
}

//------------------------------------------------------------------------
// procedures send command setup
//------------------------------------------------------------------------ 
void setupSendCmd() {
  Serial1.begin(HOVER_SERIAL_BAUD, SERIAL_8N1, 27, 26 );
  delay(100);
  Serial2.begin(HOVER_SERIAL_BAUD, SERIAL_8N1, 16, 17 );
  delay(100);
}

//------------------------------------------------------------------------
// procedures send command loop
//------------------------------------------------------------------------ 
void loopSendCmd() {
  
  // Send commands 
  unsigned long timeNow = millis();
  if (timeNow - iTimeSend >= TIME_SEND) {
    iTimeSend = timeNow;
    // Uart1//
    sendSerial(1, enable, controlMode, -speeds.leftSpeed, speeds.leftSpeed); 
    //                                         LP                LT    
    // Uart2 //
    sendSerial(2, enable, controlMode, speeds.rightSpeed, -speeds.rightSpeed);
    //                                         PP                PT
  }

  // Receive commands
  receiveSerial(1);
  // delay(1); 
  receiveSerial(2);
  // delay(1);
}

//------------------------------------------------------------------------
// end file
//------------------------------------------------------------------------ 