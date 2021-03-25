#pragma once
#include <cstdint>

class ByteBuffer {
public:
	const static int BAD_ALLOC = 1;	//out of memory
	const static int BAD_DIGIT = 2; //found a character that is not recognized as a valid digit
	const static int BAD_STRING = 3; //found a string of digits that is out of the range of a single byte
	const static int BAD_CALL = 4; //performed a call to getMessage or getAddress on a bytebuffer that wasn't a full message.

	/*
		getByteBuffer(): returns a bytebuffer based off of the arguments provided.
			sets errorNum to BAD_CALL if radix is not 8, 10, or 16.		
			sets errorNum to BAD_DIGIT if a character is outside the range of 0 to radix-1, or greater than 0xf.
			sets errorNum to BAD_STRING if a string of digits has value greater than 255.
	*/
	static ByteBuffer* getByteBuffer(char*, int radix, char delim);
	static ByteBuffer* getByteBuffer(std::uint8_t*, int);
	static ByteBuffer* getByteBuffer(ByteBuffer*);
	~ByteBuffer(); // I. destructor

	/*
		Takes a destination bytebuffer and a command bytebuffer and returns a formatted, crc'd message bytebuffer.
			sets errorNum to BAD_CALL if the dest bytebuffer is too small to hold an actual destination.
			sets errorNum to BAD_CALL if the command bytebuffer is too large to hold an actual command
	*/
	static ByteBuffer* getMessage(volatile ByteBuffer* dest, ByteBuffer* command);

	/*
		Takes a formmatted message bytebuffer and returns a destination bytebuffer
			sets errorNum to BAD_CALL if the message isn't long enough to have a complete destination, but doesn't really verify that the message is formmatted correctly.
	*/
	ByteBuffer* getAddress();

	/*
		Allocates and returns the contents of the bytebuffer as a uint8_t array.
	*/
	std::uint8_t* getBinary();

	/*
		Writes the contents of the byteBuffer to the argument as a uint8_t array.

		Does not provide bounds checking of any kind.

		Does not append a nullbyte unless the underlying bytebuffer already has one.
	*/
	void getBinary(std::uint8_t*);

	/*
		Returns the number of bytes in the byte buffer.
	*/
	int getCount();

	/*
		Allocates and returns the contents of the bytebuffer as a c-string.
			sets errorNum to BAD_CALL if radix is not 8, 10, or 16.
			portNum overload prefixes the contents of the bytebuffer with "COMX: " where X is the portnum.
	*/
	char* getString(int radix, char delim);
	char* getString(int radix, char delim, int portNum);

	/*
		Verifies that the contents of a c-string are a series of byte values with the indicated radix and indicated delimiter.
		Returns the number of bytes in the c-string on success.
			sets errorNum to BAD_CALL if radix is not 8, 10, or 16.			
			sets errorNum to BAD_DIGIT if a character is outside the range of 0 to radix-1, or greater than 0xf.
			sets errorNum to BAD_STRING if a string of digits has value greater than 255.
	*/
	static int verifyString(char*, int radix, char delim);

	static const char* getErrorMsg();
	static const int getErrorNum();
private:
	std::uint8_t* binary;
	int count;

	thread_local static char errorMsg[128];
	thread_local static int errorNum;

	ByteBuffer();
	ByteBuffer(const ByteBuffer& other);// II. copy constructor
	ByteBuffer& operator=(const ByteBuffer& other);// III. copy assignment
};