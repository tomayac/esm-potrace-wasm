#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "hexutils.h"

static const uint8_t digitToHex[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

uint8_t* char_to_hex(uint8_t c)
{
	unsigned int upper_digit = c / 16;
	unsigned int lower_digit = c % 16;
	uint8_t* hex_value = (uint8_t*)malloc(3);
	hex_value[0] = digitToHex[upper_digit];
	hex_value[1] = digitToHex[lower_digit];
	hex_value[2] = '\0';
	return hex_value;
}

uint8_t* rgb_to_hex(uint8_t r, uint8_t g, uint8_t b)
{
	uint8_t* color = (uint8_t*)malloc(7);
	color[6] = '\0';

	uint8_t* rHex = char_to_hex(r);
	color[0] = rHex[0];
	color[1] = rHex[1];
	uint8_t* gHex = char_to_hex(g);
	color[2] = gHex[0];
	color[3] = gHex[1];
	uint8_t* bHex = char_to_hex(b);
	color[4] = bHex[0];
	color[5] = bHex[1];
	free(rHex);
	free(gHex);
	free(bHex);
	return color;
}