#pragma once

#include <string>

class Base64
{
public:
	std::string base64_encode(unsigned char const*, unsigned int len);
	std::string base64_decode(std::string const& s);
};