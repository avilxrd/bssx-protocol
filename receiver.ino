#include <SoftwareSerial.h>
#include "Protocolo.h"

SoftwareSerial sw_serial(10, 11);
Protocolo protocolo(sw_serial);

Frame rx_frame;

void setup()
{
    Serial.begin(9600);
    protocolo.begin(9600);

    Serial.println("Receiver pronto. Aguardando dados...");
}

void loop()
{
    // update() cuida do ack/nack internamente; retorna true quando
    // um frame novo (não repetido) foi recebido com sucesso
    if (protocolo.update(rx_frame))
    {
        mostrarFrame(rx_frame);
    }
}

void mostrarFrame(Frame &frame)
{
    Serial.print("Seq: ");
    Serial.print(frame.header.seq);
    Serial.print(" | Tipo: ");

    switch (frame.header.type)
    {
        case BYTE_T:
        {
            ui8 valor = frame.payload[0];
            Serial.print("BYTE | Valor: ");
            Serial.println(valor, BIN);
            break;
        }

        case WORD_T:
        {
            ui16 valor;
            memcpy(&valor, frame.payload, sizeof(ui16));
            Serial.print("WORD | Valor: 0x");
            Serial.println(valor, HEX);
            break;
        }

        case FLOAT_T:
        {
            float valor;
            memcpy(&valor, frame.payload, sizeof(float));
            Serial.print("FLOAT | Valor: ");
            Serial.println(valor);
            break;
        }

        case DATA_T:
        {
            Serial.print("DATA | Valor: ");
            for (ui16 i = 0; i < frame.header.len; i++)
            {
                Serial.print((char)frame.payload[i]);
            }
            Serial.println();
            break;
        }

        default:
            Serial.println("Tipo desconhecido");
            break;
    }
}
