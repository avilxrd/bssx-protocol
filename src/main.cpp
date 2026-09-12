#include <SoftwareSerial.h>
#include "Protocolo.h"

SoftwareSerial sw_serial(10, 11); //rx e tx
Protocolo protocolo(sw_serial);

void setup()
{
    Serial.begin(9600);
    protocolo.begin(9600);
}

#define FLOAT_EX 2.5
#define BYTE_EX 0b01010001 
#define WORD_EX 0x87D5

void loop()
{
    Serial.println("enviando dados")
    if (protocolo.sendFloat(FLOAT_EX)) { Serial.println("sucesso (ack recebido)"); }
    else { Serial.println("falha no envio (timeout)") }
    delay(1000);
}