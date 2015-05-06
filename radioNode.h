#ifndef radioNode_h
#define radioNode_h

#include "Arduino.h"
#include <RFM69.h>

#define RN_KEYLEN 16

#define RN_DUMP_REGISTERS 'r'
#define RN_ENABLE_ENCRYPTION 'E'
#define RN_DISABLE_ENCRYPTION 'e'
#define RN_DUMP_TEMP 't'
#define LHEADER sizeof(RadioHeader)

#define LBODY RF69_MAX_DATA_LEN - LHEADER

// Possible bug if we hit this exact limit due to no null termination

typedef struct {
    uint8_t id; // Should be 2 nibbles containing both node and sensor ids
    uint8_t packetType;
} RadioHeader;

class RadioNode
{
    public:
        static void setupRadio(const uint8_t frequency, const uint8_t nodeId, const uint8_t network, const bool highPower, const char *encryptKey);
        static void readRadio(RadioHeader *header, char *body);
        static void sendData(const RadioHeader *header, const void *body, uint8_t toAddress, uint8_t lenBody);
        static void sendData(const RadioHeader *header, const void *body, uint8_t lenBody);
        static uint16_t getAckCount();
        static uint16_t getPacketCount();
        static void testConnection();
        static void enableEncryption();
        static void disableEncryption();
        static void dumpTemp();
        static void dumpRegisters();
        static void executeCommand(char input);
        static RFM69 getRadio();

    private:
        static RFM69 radio;
        static uint16_t ackCount;
        static uint32_t packetCount;
        static char encryptKey[RN_KEYLEN];
        static RadioHeader readHeader();
};
#endif
