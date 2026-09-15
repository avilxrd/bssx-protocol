# Documentação do protocolo de comunicação serial
Artur Fardin Corrêa, Miguel Avila de Oliveira e Nicolas Monteiro Longo

## Visão Geral
O protocolo desenvolvido é orientado à frames de tamanho variável, utilizando o esquema `stop-and-wait ARQ`. Cada frame é composto por campos de header, payload e checksum.

| Campo | Descrição | Tamanho |
| --- | --- | --- |
| SOF | Início do quadro | 1 byte |
| SEQ | Número de sequência do quadro | 1 byte |
| TYPE | Tipo de dado que está sendo enviado | 1 byte |
| LEN | Tamanho do payload | 1 byte |
| PAYLOAD | Dados que estão sendo enviados | (n) bytes |
| CHECKSUM | Checksum para evitar redundância | 2 bytes |

## Implementação
Em código, os campos descritos acima foram implementados da seguinte maneira.

### Header
O cabeçalho do protocolo foi implementado através de uma `struct` compactada utilizando a diretiva `__attribute__((__packed__))` do compilador, para evitar a inserção de bytes extras de alinhamento.
```c++
typedef uint8_t  ui8;
typedef uint16_t ui16;
typedef uint32_t ui32;

struct __attribute__((__packed__)) Header
{
    ui8 sof;
    ui8 seq;
    ui8 type;
    ui8 len;
};
```

### Type
O campo que indica o tipo de dado enviado é abstraído por uma `enum`, onde os bytes especiais são associados a nomes mais legíveis. Os quadros `ACK` e `NACK` seguem o padrão `ASCII`.

```c++
enum DataType: ui8
{
    BYTE_T  = 0x01,
    WORD_T  = 0x02,
    FLOAT_T = 0x03,
    DATA_T  = 0x04,
    ACK_T   = 0x06, //ASCII
    NACK_T  = 0x15  //ASCII
};
```

### Payload
O campo de dados válidos é implementado através de um `buffer` fixo de 64 bytes.
```c++
#define MAX_PAYLOAD 64

typedef uint8_t  ui8;

ui8 payload[MAX_PAYLOAD];
```

## Decisões de Projeto
Algumas explicações sobre decisões tomadas no desenvolvimento.

### Frame Compactado
Garante que os dados enviados serão exatamente o esperado (a não ser por corrupção), pois o compilador não adiciona bytes de alinhamento, o que evita o envio de bytes adicionais sem necessidade e facilita o `debug` do código para detecção de erros.

### Checksum
Mesmo sem a necessidade, implementamos um mecanismo de `checksum` por conta da sua simplicidade e utilidade. Com esse mecanismo o código é capaz de detectar corrupção de dados no meio.

### Quadros ACK/NACK
Ao invés de criar um mecanismo à parte para confirmações, utilizamos a mesma estrutura do quadro e utilizando os tipos de quadro ACK/NACK.

### Stop-and-Wait ARQ
Este mecanismo foi escolhido por conta de sua simplicidade. Não havia necessidade de implementar janelas maiores ou outros mecanismos como o `go-back-n` ou o `selective-repeat` Além da dificuldade desnecessária, o hardware possui limitação de memória e processamento.

## Máquina de Transmissão (TX)
O envio de qualquer tipo de dado utiliza o método privado `sendPackage`, que implementa o mecanismo `stop-and-wait`. O funcionamento segue da seguinte maneira:
1. O frame é montado com o método `Frame::montar()`, ou seja, são preenchidos os campos de header, payload e checksum.
2. Envia o pacote pelo `SoftwareSerial`.
3. Espera até receber um `ACK` ou estourar o `timeout` (`stop-and-wait`).
- Caso o envio tenha algum problema, reenvia o pacote até no máximo 3 tentativas. Ao estourar as tentativas, retorna `false` e o envio é considerado mal sucedido.
4. Se a resposta for um `ACK`, verifica se o número de sequência é o esperado.
5. Se o número de sequência for o esperado, incrementa `seq` e retorna `true` considerando o envio bem sucedido.

## Máquina de Recepção (RX)
A leitura de um frame é realizada por uma máquina de estados que consome os bytes que chegam pela serial um a um, sem bloquear. Os estados possíveis são: 
| State | Descrição |
| --- | --- |
| WAITING_SOF | Descarta bytes até encontrar um marcador de início de quadro |
| READING_HEADER | Acumula os bytes restantes de header |
| READING_PAYLOAD | Acumula os bytes de payload |
| READING_CHECKSUM | Acumula os bytes de checksum |

O método `readFrame()` só retorna `true` se um frame completo chegar dentro do timeout e com o `checksum` calculad compatível.

## Confirmação e detecção de duplicados
O método `Frame::update()` é chamado para tratar os frames que chegam do outro arduino. 
1. Se não há nenhum dado disponível, não faz nada.
2. Se um frame chega e o checksum é válido, o método trata de acordo com o tipo do frame.
- `ACK` ou `NACK`: o `update()` não processa, pois o próprio método `sendPackage()` já trata.
- `Frame de dado`: envia um `ACK` de volta com o mesmo índice de sequência.
3. Se o `seq` recebido for o esperado incrementa o contador e sinaliza que há um dado novo para a aplicação processar. Se o `seq` nao for o esperado, envia o `ACK` e o emissor enviará o próximo quadro.
4. Se o frame chegar corrompido ou o timeout estoura envia um `NACK` pedindo a retransmissão.

## Fluxo Principal
A cada segundo a placa tenta enviar um valor `float` de exemplo para a outra e informa ao usuário (via `serial monitor`) se a transmissão foi confirmada ou falhou.

```c++
SoftwareSerial sw_serial(10,11);
Protocolo protocolo(sw_serial);

void setup()
{
    Serial.begin(9600);
    protocolo.begin(9600);
}

void loop()
{
    if (protocolo.sendFloat(FLOAT)) 
    { 
        Serial.println("sucesso (ack recebido)"); 
    }
    else 
    { 
        Serial.println("falha no envio (timeout ou nack)"); 
    }
    delay(1000);
}
```