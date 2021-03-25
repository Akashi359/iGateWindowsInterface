#include "ByteBuffer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

ByteBuffer* ByteBuffer::getByteBuffer(std::uint8_t* binary, int count) {
	ByteBuffer* temp = new ByteBuffer();
	temp->binary = (std::uint8_t*)malloc(count);
	if (temp->binary == nullptr) {
		errorNum = 
		sprintf(errorMsg, "malloc failed in call to getByteBuffer");
		errorNum = BAD_ALLOC;
		delete(temp);
		return nullptr;
	}
	for (int k = 0; k < count; k++)
		temp->binary[k] = binary[k];
	temp->count = count;
	return temp;
}

ByteBuffer* ByteBuffer::getByteBuffer(ByteBuffer* other) {
	ByteBuffer* temp = new ByteBuffer();
	temp->binary = (std::uint8_t*)malloc(other->count);
	if (temp->binary == nullptr) {
		sprintf(errorMsg, "malloc failed in call to getByteBuffer");
		errorNum = BAD_ALLOC;
		delete(temp);
		return nullptr;
	}
	for (int k = 0; k < other->count; k++)
		temp->binary[k] = other->binary[k];
	temp->count = other->count;
	return temp;
}

ByteBuffer* ByteBuffer::getByteBuffer(char* string, int radix, char delim) {
	if(radix != 8 || radix != 10 || radix != 16){
		sprintf(errorMsg, "radix must be one of 8, 10, or 16");
		errorNum = BAD_CALL;
		return nullptr;
	}

	int counter = 0;
	int runningSum;
	bool lastWasDelim = true;
	char* curr = string;
	char* endPtr;

	int currMaxSize = 16;
	std::uint8_t* binary = (std::uint8_t*)malloc(currMaxSize);
	if (binary == nullptr) {
		sprintf(errorMsg, "malloc failed in call to getByteBuffer");
		errorNum = BAD_ALLOC;
		return nullptr;
	}

	while (*curr != '\0') {
		if (*curr == delim) {
			curr++;
			if (!lastWasDelim) {
				lastWasDelim = true;
				if (counter > currMaxSize - 1) {
					currMaxSize = currMaxSize * 2;
					std::uint8_t* temp = (std::uint8_t*)realloc(binary, currMaxSize);
					if (temp == nullptr) {
						sprintf(errorMsg, "realloc failed in call to getByteBuffer");
						errorNum = BAD_ALLOC;
						free(binary);
						return nullptr;
					}
					binary = temp;
				}
				binary[counter] = runningSum;
				counter++;
			}
		}
		else {
			int currVal = getVal(*curr);
			if (currVal < 0 || currVal >= radix) {
				sprintf(errorMsg, "non-digit character found in call to getByteBuffer()");
				errorNum = BAD_DIGIT;
				free(binary);
				return nullptr;
			}
			if (lastWasDelim) {
				lastWasDelim = false;
				runningSum = currVal;
			}
			else {
				runningSum = runningSum * radix;
				runningSum += currVal;
				if (runningSum > 255) {
					sprintf(errorMsg, "found string of digits outside of range representable by a single byte in call to getByteBuffer()");
					errorNum = BAD_STRING;	
					free(binary);
					return nullptr;
				}
			}
		}
	}
	if (!lastWasDelim) {
		if (currMaxSize - 1 != counter) {
			std::uint8_t* temp = (std::uint8_t*)realloc(binary, counter + 1);
			if (temp == nullptr) {
				sprintf(errorMsg,"realloc failed in call to getByteBuffer");
				errorNum = BAD_ALLOC;
				free(binary);
				return nullptr;
			}
			binary = temp;
		}
		binary[counter] = runningSum;
		counter++;
	}
	else {
		if (currMaxSize != counter){
			std::uint8_t* temp = (std::uint8_t*)realloc(binary, counter);
			if (temp == nullptr) {
				sprintf(errorMsg,"realloc failed in call to getByteBuffer");
				errorNum = BAD_ALLOC;
				free(binary);
				return nullptr;
			}
			binary = temp;
		}
	}

	ByteBuffer* temp = new ByteBuffer();
	temp->binary = binary;
	temp->count = counter;
	return temp;
}

ByteBuffer::ByteBuffer() {
	binary = nullptr;
	count = 0;
	errorNum = 0;
}
ByteBuffer::~ByteBuffer() {
	free(binary);
}

std::uint8_t* ByteBuffer::getBinary() {
	return binary;
}

void ByteBuffer::getBinary(std::uint8_t* buffer) {
	for (int k = 0; k < count; k++)
		buffer[k] = binary[k];
}

int ByteBuffer::getCount() {
	return count;
}

char* ByteBuffer::getString(int radix, char delim) {
	return getString(radix, delim, -1);
}

char* ByteBuffer::getString(int radix, char delim, int portNum) {
	const char* specifier;
	switch (radix) {
	case 8: specifier = "%o "; break;
	case 10: specifier = "%u "; break;
	case 16: specifier = "%X "; break;
	default: sprintf(errorMsg,"radix must be one of 8, 10, or 16"); errorNum = BAD_CALL; return nullptr;
	}

	//calculate and allocate buffer size
	int maxcharspernumber = 0;
	for (int u = 255; u > 0; maxcharspernumber++)
		u = u / radix;
	char prefix[128];
	int prefixSize = 0;
	if (portNum >= 0)
		prefixSize = sprintf(prefix, "COM%d: ", portNum);
	char* buffer = (char*)malloc(sizeof(char)*((maxcharspernumber + 1)*count+1+prefixSize));
	if (buffer == nullptr && count != 0) {
		sprintf(errorMsg,"malloc failed in call to getString");
		errorNum = BAD_ALLOC;
		return nullptr;
	}

	//populate buffer
	if (portNum >= 0)
		strcpy(buffer, prefix);
	int currPos = prefixSize;
	for (int k = 0; k < count; k++) {
		int temp = sprintf(buffer + currPos, specifier, binary[k]);
		currPos += temp;
	}
	buffer[currPos-1] = '\0';
	char* temp = (char*)realloc(buffer, sizeof(char)*currPos);
	if (temp == nullptr) {
		free(buffer);
		sprintf(errorMsg,"realloc() failed in call to getString()");
		errorNum = BAD_ALLOC;
		return nullptr;
	}
	return temp;
}

int ByteBuffer::verifyString(char* string, int radix, char delim) {
	if (radix != 8 || radix != 10 || radix != 16) {
		sprintf(errorMsg, "radix must be one of 8, 10, or 16");
		errorNum = BAD_CALL;
		return -1;
	}

	int counter = 0;
	int runningSum;
	bool lastWasDelim = true;
	char* curr = string;
	char* endPtr;

	while (*curr != '\0') {
		if (*curr == delim) {
			curr++;
			if (!lastWasDelim) {
				counter++;
				lastWasDelim = true;
			}
		}
		else{
			int currVal = getVal(*curr);
			if (currVal < 0 || currVal >= radix) {
				sprintf(errorMsg, "non-digit character found in call to verifyString()");
				errorNum = BAD_DIGIT;
				return -1;
			}
			if (lastWasDelim) {
				lastWasDelim = false;
				runningSum = currVal;
			}
			else{
				runningSum = runningSum * radix;
				runningSum += currVal;
				if (runningSum > 255) {
					sprintf(errorMsg, "found string of digits outside of range representable by a single byte in call to getByteBuffer()");
					errorNum = BAD_STRING;
					return -1;
				}
			}
		}
	}
	if (!lastWasDelim)
		counter++;
	return counter;
}

int getVal(char curr) {
	if (curr >= '0' && curr <= '9')
		return curr - '0';
	else if (curr >= 'a' && curr <= 'f')
		return curr - 'a' + 10;
	else if (curr >= 'A' && curr <= 'F')
		return curr - 'A' + 10;
	else return -1;
}


ByteBuffer* ByteBuffer::getAddress() {
	if (this->count < 17) {
		sprintf(errorMsg, "byte buffer is too small to be holding a message");
		errorNum = BAD_CALL;
		return nullptr;
	}
	ByteBuffer* temp = new ByteBuffer();
	uint8_t* buffer = (uint8_t*)malloc(8);
	if (buffer == nullptr) {
		sprintf(errorMsg, "call to malloc() failed in call to getAddress()");
		errorNum = BAD_ALLOC;
		delete(temp);
		return nullptr;
	}
	for (int k = 0; k < 8; k++)
		buffer[k] = this->binary[k + 8];
	temp->binary = buffer;
	temp->count = 8;
	return temp;
}

/*
	Assumes the ByteBuffer is 8 bits of dest followed by a command.
*/
ByteBuffer* ByteBuffer::getMessage(volatile ByteBuffer* dest, ByteBuffer* command) {
	if (dest->count != 8){
		sprintf(errorMsg, "dest buffer is the wrong size to be holding a valid destination");
		errorNum = BAD_CALL;
		return nullptr;
	}

	if (dest->count + command->count + 2 + 4 > 255) {
		sprintf(errorMsg, "command buffer is too large to be holding a valid command");
		errorNum = BAD_CALL;
		return nullptr;
	}

	unsigned totalSize = 2 + 1 + 2 + 4 + dest->count + command->count + 2;

	uint8_t* buffer = (uint8_t*)malloc(totalSize * sizeof(uint8_t));
	if (buffer == nullptr) {
		sprintf(errorMsg,"call to malloc() failed in getMessage()");
		errorNum = BAD_ALLOC;
		return nullptr;
	}
	buffer[0] = 0xff;
	buffer[1] = 0x5e;
	buffer[2] = 2 + 4 + dest->count + command->count;
	buffer[3] = 'c';
	buffer[4] = 'd';
	for (unsigned k = 5; k <= 8; k++)
		buffer[k] = 0;
	//9 to 16 for dest
	for (unsigned k = 0; k < 8; k++)
		buffer[k+9] = dest->binary[k];
	//17 to n for command
	for (unsigned k = 0; k < command->count; k++)
		buffer[k + 17] = command->binary[k];
	//totalsize - 2 and totalsize - 1 for crc
	getCRC(buffer + 3, totalSize - 5, buffer + totalSize - 2);

	ByteBuffer* temp = new ByteBuffer();
	temp->binary = buffer;
	temp->count = totalSize;
	return temp;
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

const char* ByteBuffer::getErrorMsg() {
	return errorMsg;
}


const int ByteBuffer::getErrorNum() {
	return errorNum;
}