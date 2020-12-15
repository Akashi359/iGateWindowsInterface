#include "Message.h"
#include <stdlib.h>
#include <cstring>
#include <cstdio>


/*
	Converts a string of numbers into a byte array of numbers.
	Param:
		-dest:	destination array to store the byte array of numbers. Must be large enough.
		-src:	source char array holding a c string of numbers to be converted.
		-n:		max count of numbers from src to convert. If <= 0, this function instead stops when a null char is found in src.
		-delim:	char indicating the delimiter between consecutive numbers in src.
		-radix: the numerical base that the numbers in src are represented in.
*/
bool strntob(uint8_t* dest, const char* src, unsigned n, char delim, unsigned radix) {
	unsigned srcSize = strlen(src);
	char* temp = (char*)malloc(sizeof(char)*(srcSize + 1));
	strcpy(temp, src);

	unsigned startIndex = 0;
	unsigned destPos = 0;
	bool lastCharWasDelim = true;

	for (unsigned k = 0; k < srcSize + 1 && (n <= 0 || destPos < n); k++) {
		if (temp[k] == delim || temp[k] == '\0') {
			if (!lastCharWasDelim) {
				temp[k] = '\0';
				unsigned converted = strtol(temp + startIndex, NULL, radix);
				if (converted > 255)
					return false;
				dest[destPos++] = converted;
			}
			lastCharWasDelim = true;
		}
		else {
			if (lastCharWasDelim) {
				startIndex = k;
			}
			lastCharWasDelim = false;
		}
	}
	return true;
}

/*
	Returns the amount of numbers in a null terminated c string of numbers.
	Radix must be 10 or 16. Returns -2 otherwise.
	Returns -1 if the c string contains any characters besides digits or delims.
*/
int count(const char* buffer, char delim, unsigned radix) {
	unsigned len = strlen(buffer);
	bool lastCharWasDelim = true;
	int counter = 0;
	if (radix != 10 && radix != 16)
		return -1;
	for (unsigned k = 0; k < len; k++) {
		if ( (buffer[k] >= '0' && buffer[k] <= '9') ||
			 (radix == 16 && buffer[k] >= 'a' && buffer[k] <= 'f') ||
			 (radix == 16 && buffer[k] >= 'A' && buffer[k] <= 'F') ) {
			if (lastCharWasDelim)
				counter++;
			lastCharWasDelim = false;
		}
		else if (buffer[k] == delim)
			lastCharWasDelim = true;
		else
			return -2;
	}
	return counter;
}

void getCRC(uint8_t *buf, int len, uint8_t *value) {
	unsigned int crcval, t;
	int i;
	crcval = 0xffff; // CCITT CRC
	for (i = 0; i < len; ++i) {
		t = crcval ^ (unsigned int)buf[i];
		t = (t ^ (t << 4)) & 0xff;
		crcval = (crcval >> 8) ^ (t << 8) ^ (t << 3) ^ (t >> 4);
	}
	crcval = ~crcval;
	value[0] = (uint8_t)(crcval & 0x00ff); // return LSB (sent first)
	value[1] = (uint8_t)((crcval & 0xff00) >> 8); // return MSB (sent second)
}

size_t getMessage(const char* dest, const char* command, int commandLen, uint8_t*& message) {
	unsigned totalSize = 2 + 1 + 2 + 4 + 8 + commandLen + 2;

	uint8_t* buffer = (uint8_t*)malloc((totalSize + 1) * sizeof(uint8_t));
	buffer[0] = 0xff;
	buffer[1] = 0x5e;
	buffer[2] = 2 + 4 + 8 + commandLen;
	buffer[3] = 'c';
	buffer[4] = 'd';
	for (unsigned k = 5; k <= 8; k++)
		buffer[k] = 0;
	//9 to 16 for dest
	if (!strntob(buffer + 9, dest, 0, ' ', 10)) {
		free(buffer);
		return -1;
	}
	//17 to n for command
	if (!strntob(buffer + 17, command, 0, ' ', 10)) {
		free(buffer);
		return -2;
	}
	//totalsize - 2 and totalsize - 1 for crc
	getCRC(buffer + 3, totalSize - 5, buffer + totalSize - 2);
	buffer[totalSize] = '\0';
	message = buffer;
	return totalSize;
}

char* getAddress(const char* message) {
	unsigned strSize = strlen(message);
	unsigned numSpaces = 0;
	unsigned startIndex;
	for (startIndex = 0; startIndex < strSize; startIndex++) {
		if (numSpaces == 9)
			break;
		if (message[startIndex] == ' ')
			numSpaces++;
	}
	if (startIndex == strSize)
		return NULL;
	uint8_t byteBuffer[8];
	if (!strntob(byteBuffer, message + startIndex, 8, ' ', 16))
		return NULL;

	char* retVal = (char*)malloc(sizeof(char) * 41);
	startIndex = 0;
	for (unsigned k = 0; k < 8; k++)
		startIndex += sprintf(retVal + startIndex, "%d", byteBuffer[k]);

	return (char*)realloc(retVal, startIndex + 1);
}

/*
	"(B)yte (n) (to) (str)ing"
*/
bool bntostr(char* dest, uint8_t* src, unsigned n, char delim, unsigned radix) {
	char format[6] = "%02x ";
	format[4] = delim;
	switch (radix) {
	case 'd':
	case 'i':
	case 'u':
	case 10:
		format[3] = 'u';
		break;
	case 'h':
	case 'x':
	case 16:
		format[3] = 'x';
		break;
	default:
		return false;
	}
	for (unsigned k = 0; k < n; k++)
		sprintf(dest + (3 * k), format, src[k]);
	return true;
}

char* bntostr(uint8_t* src, unsigned n, char delim, unsigned radix) {
	char* dest = (char*)malloc((3 * n + 1) * sizeof(char));
	if (dest == NULL)
		return NULL;
	dest[n] = '\0';
	if (!bntostr(dest, src, n, delim, radix)) {
		free(dest);
		return NULL;
	}
	return dest;
}