#include "Frame.h"

Frame::Frame()
{
    header.sof  = 0x7E;
    header.seq  = 0;
    header.type = 0;
    header.len  = 0;
    checksum    = 0;
};

//calcula o checksum do frame
ui16 Frame::calcularChecksum()
{
    ui16 sum=0;
    ui8  *hptr = (ui8*)&header;

    //soma os bytes do header
    for (size_t i=0; i<sizeof(Header); i++){ sum += hptr[i]; }

    //soma os bytes do payload
    for (ui16 i=0; i<header.len; i++) { sum += payload[i]; }

    return sum;
}

//
void Frame::montar(ui8 seq, ui8 type, const ui8 *data, ui16 len)
{
    header.sof  = 0x7E;
    header.seq  = seq;
    header.type = type;
    header.len  = len;

    for (ui16 i=0; i<len; i++){ payload[i] = data[i]; }

    checksum = calcularChecksum();
}