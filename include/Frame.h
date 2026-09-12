#ifndef FRAME_H
#define FRAME_H

//#include <Arduino.h>
#include <cstdint>
#include <cstddef>

#define MAX_PAYLOAD 64

#define ui8 uint8_t
#define ui16 uint16_t

enum DataType : ui8
{
    BYTE_T  = 0x01,
    WORD_T  = 0x02,
    FLOAT_T = 0x03,
    DATA_T  = 0x04,
    ACK_T   = 0x06,
    NACK_T  = 0x15
};

struct __attribute__((__packed__)) Header
{
    ui8  sof;
    ui8  seq;
    ui8  type;
    ui16 len;
};

//frame do protocolo:
//[1 byte]  preamb/sync
//[1 byte]  sequencia (stop and wait)
//[1 byte]  tipo de dado -> 0: byte, 1: word, ...
//[n bytes] payload
//[2 bytes] crc ou checksum
class Frame
{
public:
    Header header;
    ui8 payload[MAX_PAYLOAD];
    ui16 checksum;

    Frame();
    ui16 calcularChecksum();
    void montar(ui8 seq, ui8 type, const ui8 *data, ui16 len);
};

#endif