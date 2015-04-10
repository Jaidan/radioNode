/*
   TODO we should store the NodeId and network information in EEPROM
   Defaults should be provided by the define, but we should be able to alter it
   by changing a real value stored in EEPROM
   
   Maybe a handshake negotiation to allow auto joining the network.  Use a magic
   nodeid for the initial registration and allow the gateway to pick a nodeid.
*/
#include "Arduino.h"
#include "radioNode.h"

#define GATEWAYID 15 // Reserved ID for gateways

#ifdef DEBUG
const PROGMEM char debugListening[] = "\nListening at %d Mhz...";
const PROGMEM char debugReadRadioCount[] = "#[%u][%d] ";
const PROGMEM char debugReadRadioRXRSSI[] =  "   [RX_RSSI:%u";
const PROGMEM char debugConnTest[] =  " Pinging node %u - ACK...";
#endif
const PROGMEM char debugTemperature[] =  "Radio Temp is %dC, %dF";

void RadioNode::setupRadio(const uint8_t frequency, const uint8_t nodeId,
        const uint8_t network, const bool highPower, const char *encryptKeyPtr)
{
    radio.initialize(frequency, nodeId, network);
    if (highPower)
    radio.encrypt(encryptKeyPtr);
#ifdef DEBUG
    char buff[50];
    sprintf_P(
        buff, debugListening,
        frequency == RF69_433MHZ ? 433 : frequency == RF69_868MHZ ? 868 : 915
      );
    Serial.println(buff);
#endif
}

void RadioNode::readRadio(RadioHeader *header, char *body)
{
#ifdef DEBUG
    char buff[50];
    sprintf_P(buff, debugReadRadioCount, ++packetCount, radio.SENDERID);
    Serial.print(buff);
    for (byte i = 0; i < radio.DATALEN; i++) {
        char c = (char)radio.DATA[i];
        Serial.print(c);
    }
    sprintf_P(buff, debugReadRadioRXRSSI, radio.RSSI);
    Serial.println(buff);
#endif

    memcpy(header, (const void *)radio.DATA, sizeof(RadioHeader));
    memcpy(body, (const void *)&radio.DATA[LHEADER], radio.DATALEN - LHEADER);

    if (radio.ACKRequested()) {
        radio.sendACK();
#ifdef DEBUG
        Serial.print(F(" - ACK sent."));
#endif

        // When a node requests an ACK, respond to the ACK
        // and also send a packet requesting an ACK (every 3rd one only)
        // This way both TX/RX NODE functions are tested on 1 end at the GATEWAY
        if (ackCount++ % 3 == 0)
          testConnection();
        }
#ifdef DEBUG
    Serial.println();
#endif
    // TODO implement delayless blink
    // blink(LED, 3);  Blinking an LED on radio read would be nice...
    // but how to do it without a delay for this case?
    // Turn on LED -> Track last reception millis -> if currentMillis - lastMillis then LED off 
}

void RadioNode::testConnection()
{
  uint8_t theNodeId = radio.SENDERID;
#ifdef DEBUG
  char buff[50];
  sprintf_P(buff, debugConnTest, theNodeId);
  Serial.print(buff);
#endif
  delay(3); //need this when sending right after reception .. ?
  if (radio.sendWithRetry(theNodeId, "ACK TEST", 8, 0))  // 0 = only 1 attempt, no retries
    Serial.print(F("ok!"));
  else Serial.print(F("nothing"));
}

void RadioNode::enableEncryption()
{
    radio.encrypt((const char *)RadioNode::encryptKey);
    Serial.println(F("Encryption Enabled"));
}

void RadioNode::disableEncryption()
{
    radio.encrypt(NULL);
    Serial.println(F("Encryption disabled"));
}

void RadioNode::dumpTemp()
{
    byte temp = radio.readTemperature(-1); // -1 = user cal factor, adjust for correct ambient
    byte fTemp = 1.8 * temp + 32; // 9/5=1.8
    char buff[50];
    sprintf_P(buff, debugTemperature, temp, fTemp);
    Serial.println(buff);
}

void RadioNode::dumpRegisters()
{
    radio.readAllRegs();
}

void RadioNode::executeCommand(char input)
{
    switch (input) {
      case RN_DUMP_REGISTERS:
        RadioNode::dumpRegisters();
        break;
      case RN_ENABLE_ENCRYPTION:
        RadioNode::enableEncryption();
        break;
      case RN_DISABLE_ENCRYPTION:
        RadioNode::disableEncryption();
        break;
      case RN_DUMP_TEMP:
        RadioNode::dumpTemp();
        break;
    }
}

void RadioNode::sendData(const void* data, uint8_t toAddress, uint8_t lenData)
{
    radio.sendWithRetry(toAddress, data, lenData);
}

void RadioNode::sendData(const void* data, uint8_t lenData)
{
    // Default to the gateway
    RadioNode::sendData(data, GATEWAYID, lenData);
}

RFM69 RadioNode::getRadio()
{
    return radio;
}

uint16_t RadioNode::getAckCount()
{
    return ackCount;
}

uint16_t RadioNode::getPacketCount()
{
    return packetCount;
}

uint16_t RadioNode::ackCount = 0;
uint32_t RadioNode::packetCount = 0;
RFM69 RadioNode::radio = RFM69();
char RadioNode::encryptKey[RN_KEYLEN];
