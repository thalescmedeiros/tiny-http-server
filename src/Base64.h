#ifndef H_Base64
#define H_Base64

#include <string>

class Base64
{
public:
	static std::string Encode(const unsigned char* data, int input_length);
	static char* Decode(const unsigned char* data, int input_length, int* output_length);
	static void Cleanup();
};

#endif
