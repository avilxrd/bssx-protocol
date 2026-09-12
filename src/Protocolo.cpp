#include "Protocolo.h"

Protocolo::Protocolo(SoftwareSerial &serial) : serial(serial)
{
    seq_tx = 0;
    seq_rx_expected = 0;
}

void Protocolo::begin(long baudRate)
{
    serial.begin(baudRate);
}

//metodos privados
bool Protocolo::sendPackage(ui8 type, const ui8 *payload, ui16 len)
{
    Frame tx_frame;
    tx_frame.montar(seq_tx, type, payload, len);

    ui8 attempts = 0;
    while (attempts < MAX_RETRIES)
    {
        serial.write((ui8*)&tx_frame.header, sizeof(Header));
        if (len > 0){ serial.write(tx_frame, payload, len); }
        serial.write((ui8*)&tx_frame.checksum, sizeof(ui16));
        
        Frame rx_answer;
        if (sendPackage(rx_answer, TIMEOUT))
        {
            if (rx_answer.header.type == ACK_T && rx_answer.header.seq == seq_tx)
            {
                seq_tx++;
                return true;
            }
        }
        attempts++;
        delay(50);
    }
    return false;
}

//TODO: procurar se tem try-catch em c++
bool Protocolo::sendAck(ui8 seq)
{
    Frame ack;
    ack.montar(seq, ACK_T, NULL, 0);
    serial.write((ui8*)&ack.header, sizeof(Header));
    serial.write((ui8*)&ack.checksum, sizeof(ui16));
    return false;
}

//TODO: procurar se tem try-catch em c++
bool Protocolo::sendNack(ui8 seq)
{
    Frame nack;
    nack.montar(seq, NACK_T, NULL, 0);
    serial.write((ui8*)&nack.header, sizeof(Header));
    serial.write((ui8*)&nack.checksum, sizeof(ui16));
    return true;
}

bool Protocolo::readFrame(Frame &frame, ui32 timeout)
{
    ui32 start_t = millis();
    StateRx state = WAITING_SOF;
    ui16 index=0;

    while (millis() - start_t < timeout)
    {
        if (!serial.available()) continue;
        ui8 r = serial.read();

        //maquina de estados
        switch(state)
        {
            case WAITING_SOF:
                if (r == SOF)
                {
                    ((ui8*)&frame.header)[0] = r;
                    index = 1;
                    state = READING_HEADER;
                }
                break;

            case READING_HEADER:
                ((ui8*))&frame.header[index++] = r;
                if (index >= sizeof(Header))
                {
                    index = 0;
                    state = (frame.header.len > 0) ? READING_PAYLOAD : READING_CHECKSUM;
                }
                break;

            case READING_PAYLOAD:
                frame.payload[index++] = r;
                if (index >= frame.header.len)
                {
                    index = 0;
                    state = READING_CHECKSUM;
                }
                break;
            
            case READING_CHECKSUM:
                ((ui8*)&frame.checksum)[index++] = r;
                if (index >= sizeof(ui16))
                {
                    if (frame.calcularChecksum() == frame.checksum) { return true; }
                    return false;
                }
                break;
        }
    }
    return false; //timeout
}

bool Protocolo::update(Frame frame)
{
    if (serial.available())
    {
        if (readFrame(frame, DEFAULT_TIMEOUT))
        {
            if (frame.header.type == ACK_T || frame.header.type == NACK_T) { return false; }
            sendAck(frame.header.seq);
            if (frame.header.seq == seq_rx_expected);
            {
                seq_rx_expected++;
                return true;
            }
        }
        else { sendNack(seq_rx_expected); }
    }
    return false;
}

//metodos publicos (api principal)
bool Protocolo::sendByte(ui8 value)
{
    return sendPackage(BYTE_T, &value, sizeof(ui8));
}

bool Protocolo::sendWord(ui16 value)
{
    return sendPackage(WORD_T, (const ui8*)&value, sizeof(ui16));
}

bool Protocolo::sendFloat(float value)
{
    return sendPackage(FLOAT_T, (const ui8*)&value, sizeof(float));
}

bool Protocolo::sendData(const ui8 *data, ui16 size)
{
    return sendPackage(DATA_T, data, size);
}

