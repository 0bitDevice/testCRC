#include <stdio.h>
#include <stdint.h>

typedef unsigned char uint8;
typedef unsigned short uint16;

typedef signed short sint16;


extern const unsigned char crc8_2F_Table[];
static uint8 CRC_encode_tData(uint8 *ptr, uint8 f_size)
{
	volatile uint16 offset ;
	volatile uint16 Dividend=0;

	uint16 i = f_size -1;
	uint16 j;

	for (j= 0; j<i; j++)
	{
		offset = ((uint16)(*ptr & 0xf0))<< 4;
		Dividend = crc8_2F_Table[Dividend | offset ];
		offset = ((uint16)(*ptr & 0x0f))<< 8;
		Dividend = crc8_2F_Table[Dividend | offset ];
		ptr++;
	}
	Dividend = crc8_2F_Table[Dividend];
	Dividend = crc8_2F_Table[Dividend];
	return (Dividend);
}

static uint8 CRC_encode_Data(uint8 *ptr, uint8 f_size)
{
	uint8 	size = f_size;
//	static uint8 	Poly = 0x2F;       // Z8+Z5+Z3+Z2+Z+1
	static uint8 	Poly = 0x1D;       // z8+z4+z3+z2+ 1
	static uint8 	NUM_BITS_DATA_PACKET = 8;
	static uint8 	CRC_CHECK_BIT  = 0x80;

	unsigned volatile char Dividend=0;

	uint16 j;
	sint16 k;

	for (j= 0; j<size-1; j++)
	{
		for (k=NUM_BITS_DATA_PACKET-1; k>=0; k--)
		{
			if (Dividend & CRC_CHECK_BIT)
			{
				Dividend = (Dividend << 1) ;
				Dividend |= (*ptr >> k) & 0x01;
				Dividend ^= Poly;
			}
			else
			{
				Dividend = (Dividend << 1) ;
				Dividend |= (*ptr >>k) & 0x01;
			}
		}
		ptr++;
	}

	for (k=0; k<NUM_BITS_DATA_PACKET; k++)
	{
		if (Dividend & CRC_CHECK_BIT)
		{
			Dividend = (Dividend << 1) ;
			Dividend ^= Poly;
		}
		else
		{
			Dividend = (Dividend << 1);
		}
	}

	return (Dividend);
}

typedef enum {
	REF_4BIT = 4,
	REF_5BIT = 5,
	REF_6BIT = 6,
	REF_7BIT = 7,
	REF_8BIT = 8,
	REF_16BIT = 16,
	REF_32BIT = 32
}REFLECTED_MODE;

uint32_t ReflectedData(uint32_t data, REFLECTED_MODE mode)
{
	data = ((data & 0xffff0000) >> 16) | ((data & 0x0000ffff) << 16);
	data = ((data & 0xff00ff00) >> 8) | ((data & 0x00ff00ff) << 8);
	data = ((data & 0xf0f0f0f0) >> 4) | ((data & 0x0f0f0f0f) << 4);
	data = ((data & 0xcccccccc) >> 2) | ((data & 0x33333333) << 2);
	data = ((data & 0xaaaaaaaa) >> 1) | ((data & 0x55555555) << 1);

	switch (mode)
	{
	case REF_32BIT:
		return data;
	case REF_16BIT:
		return (data >> 16) & 0xffff;
	case REF_8BIT:
		return (data >> 24) & 0xff;
	case REF_7BIT:
		return (data >> 25) & 0x7f;
	case REF_6BIT:
		return (data >> 26) & 0x7f;
	case REF_5BIT:
		return (data >> 27) & 0x1f;
	case REF_4BIT:
		return (data >> 28) & 0x0f;
	}
	return 0;
}

uint8_t CheckCrc8(uint8_t poly, uint8_t init, bool refIn, bool refOut, uint8_t xorOut,
	const uint8_t *buffer, uint32_t length)
{
	uint32_t i = 0;
	uint8_t crc = init;

	while (length--)
	{
		if (refIn == true)
		{
			crc ^= ReflectedData(*buffer++, REF_8BIT);
		}
		else
		{
			crc ^= *buffer++;
		}

		for (i = 0; i < 8; i++)
		{
			if (crc & 0x80)
			{
				crc <<= 1;
				crc ^= poly;
			}
			else
			{
				crc <<= 1;
			}
		}
	}

	if (refOut == true)
	{
		crc = ReflectedData(crc, REF_8BIT);
	}

	return crc ^ xorOut;
}

int main()
{
    uint8 wrCmd[5] = {0x1E, 0x80, 0x36, 0x08, 0x01};
//    CRC_encode_Data(wrCmd, 6);
    uint8 res = CheckCrc8(0x2F, 0x00, 0, 0, 0x00, wrCmd, 5);
    return 0;
}
