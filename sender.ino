#include <SoftwareSerial.h>
#include "Protocolo.h"

// --- Comunicação com o receiver ---
SoftwareSerial sw_serial(10, 11);
Protocolo protocolo(sw_serial);

// --- Pinos dos botões (usando pull-up interno: botão liga o pino ao GND) ---
#define BTN_BYTE  2
#define BTN_FLOAT 3
#define BTN_DATA  4

// --- Valores de exemplo enviados por cada botão ---
#define BYTE_EX  0b01010001
#define FLOAT_EX 2.5f
const char DATA_EX[] = "OK123";   // "outro dado" via sendData()

// --- Debounce ---
#define DEBOUNCE_MS 200

bool lastByteState  = HIGH;
bool lastFloatState = HIGH;
bool lastDataState  = HIGH;

unsigned long lastByteTime  = 0;
unsigned long lastFloatTime = 0;
unsigned long lastDataTime  = 0;

void setup()
{
    Serial.begin(9600);
    protocolo.begin(9600);

    pinMode(BTN_BYTE,  INPUT_PULLUP);
    pinMode(BTN_FLOAT, INPUT_PULLUP);
    pinMode(BTN_DATA,  INPUT_PULLUP);

    Serial.println("Sender pronto.");
}

void loop()
{
    checkButtonByte();
    checkButtonFloat();
    checkButtonData();
}

// Envia um byte quando o botão é pressionado
void checkButtonByte()
{
    bool state = digitalRead(BTN_BYTE);

    if (state == LOW && lastByteState == HIGH && (millis() - lastByteTime) > DEBOUNCE_MS)
    {
        lastByteTime = millis();

        Serial.println("Enviando byte...");
        if (protocolo.sendByte(BYTE_EX)) { Serial.println("Byte enviado (ack recebido)"); }
        else { Serial.println("Falha no envio do byte (timeout)"); }
    }

    lastByteState = state;
}

// Envia um float quando o botão é pressionado
void checkButtonFloat()
{
    bool state = digitalRead(BTN_FLOAT);

    if (state == LOW && lastFloatState == HIGH && (millis() - lastFloatTime) > DEBOUNCE_MS)
    {
        lastFloatTime = millis();

        Serial.println("Enviando float...");
        if (protocolo.sendFloat(FLOAT_EX)) { Serial.println("Float enviado (ack recebido)"); }
        else { Serial.println("Falha no envio do float (timeout)"); }
    }

    lastFloatState = state;
}

// Envia um dado genérico (sendData) quando o botão é pressionado
void checkButtonData()
{
    bool state = digitalRead(BTN_DATA);

    if (state == LOW && lastDataState == HIGH && (millis() - lastDataTime) > DEBOUNCE_MS)
    {
        lastDataTime = millis();

        Serial.println("Enviando dado (sendData)...");
        ui16 len = sizeof(DATA_EX) - 1; // sem o '\0'
        if (protocolo.sendData((const ui8*)DATA_EX, len)) { Serial.println("Dado enviado (ack recebido)"); }
        else { Serial.println("Falha no envio do dado (timeout)"); }
    }

    lastDataState = state;
}
//ß
