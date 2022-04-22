#include <algorithm>
#include <string>
#include <sstream>
#include "HttpRequest.h"

std::string HttpRequest::GetHeader(std::string &header)
{
    auto returnString = std::string();

    // Do not accept empty string
    if (header.empty()) return returnString;

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

bool HttpRequest::CheckHeaderValue(std::string &header, std::string &value)
{
    // Do not accept empty strings
    if (header.empty() || value.empty()) return false;

    // Get the header value, if present
    auto valueFromHeader = this->GetHeader(header);
	if (valueFromHeader.empty()) return false;

    // Return the comparison between header value and value
	return valueFromHeader.compare(value) == 0;
}

bool HttpRequest::CheckHeaderContainsString(std::string &header, std::string &value)
{
    // Do not accept empty strings
    if (header.empty() || value.empty()) return false;
    
    // Get the header value, if present
    auto valueFromHeader = this->GetHeader(header);
	if (valueFromHeader.empty()) return false;

    // First find the position of searched value
    auto it = std::search(
        valueFromHeader.begin(), valueFromHeader.end(),
        value.begin(), value.end(),
        [](char ch1, char ch2) { return ch1 == ch2; }
    );
    
    // If what we found is not the end of string, true
    return it != valueFromHeader.end();
}

bool HttpRequest::CheckHeaderContainsValueBetweenCommas(std::string &header, std::string &value)
{
    // Do not accept empty strings
    if (header.empty() || value.empty()) return false;
    
    // Get the header value, if present
    auto valueFromHeader = this->GetHeader(header);
	if (valueFromHeader.empty()) return false;

    std::istringstream f(valueFromHeader);
    std::string s;
    while (std::getline(f, s, ',')) {
        // ltrim the string
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
        // rtrim the string
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());

        // Comparing the current value
        if (s.compare(value) == 0) return true;
    }
    
    // Nothing found
    return false;
}
