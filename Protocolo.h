#ifndef PROTOCOLO_H
#define PROTOCOLO_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include "Frame.h"

#define DEFAULT_TIMEOUT 1000

typedef uint8_t ui8;
typedef uint32_t ui32;

class Protocolo
{
private:
    SoftwareSerial &serial;
    ui8 seq_tx;
    ui8 seq_rx_expected;

    const ui32 TIMEOUT = 1000;
    const ui8  MAX_RETRIES = 3;

    //possiveis estados do receptor
    enum StateRx{ WAITING_SOF, READING_HEADER, READING_PAYLOAD, READING_CHECKSUM };

    //[TX] envia o pacote e aguarda o ack (stop-and-wait)
    bool sendPackage(ui8 type, const ui8 *payload, ui16 len);
    bool sendAck(ui8 seq);
    bool sendNack(ui8 seq);
    bool readFrame(Frame &frame, ui32 timeout); 


public:
    Protocolo(SoftwareSerial &serial);
    void begin(long baudRate);

    bool sendByte(ui8 value);
    bool sendWord(ui16 value);
    bool sendFloat(float value);
    bool sendData(const ui8 *data, ui16 size);

    bool update(Frame &frame);
};

#endif