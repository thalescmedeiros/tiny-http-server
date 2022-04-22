#include <algorithm>
#include "HttpResponse.h"

std::string HttpResponse::GetHeader(std::string header)
{
	auto returnString = std::string();

    // Do not accept empty string
    auto h = this->headers;
    if (header.empty() || headers.empty()) return returnString;

    // First find the position of the beginning
    auto it_start = std::search(
        headers.begin(), headers.end(),
        header.begin(), header.end(),
        [](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); }
    );
    if (it_start == headers.end()) return returnString;

    // The matching shall be valid only if it's the beginning
    // of all headers or if before we had a \n character
    if (!(it_start == headers.begin() || *(std::prev(it_start)) == '\n')) return returnString;

    // Advancing the iterator to after header
    std::advance(it_start, header.size());

    // If we arrive to the end, not good
    if (it_start == headers.end()) return returnString;

    // Check if next char is :
    if (*it_start != ':') return returnString;

    // Advancing the iterator until we have no more spaces after :
    while (++it_start != headers.end() && *it_start == ' ');
    
    // Advancing the end iterator until we find \r \n or end of string
    auto it_end = it_start;
    while (++it_end != headers.end() && *it_end != '\r' && *it_end != '\n');

    // Creates a string between the 2 positions
    returnString = std::string(it_start, it_end);

    return returnString;
}

bool HttpResponse::ContainsHeader(std::string header)
{
	// Do not accept empty strings
    if (header.empty() || headers.empty()) return false;

    // Get the header value, if present
    auto valueFromHeader = this->GetHeader(header);
	return !valueFromHeader.empty();
}

std::string HttpResponse::GetReasonPhrase()
{
	if (this->statusCode == 101) return "Switching Protocols";
	if (this->statusCode == 200) return "OK";
	if (this->statusCode == 304) return "Not Modified";
	if (this->statusCode == 400) return "Bad Request";
	if (this->statusCode == 401) return "Unauthorized";
	if (this->statusCode == 404) return "Not Found";
	if (this->statusCode == 405) return "Method Not Allowed";
	if (this->statusCode == 500) return "Internal Server Error";
	if (this->statusCode == 503) return "Server Too Busy";
	return "OK";
}
