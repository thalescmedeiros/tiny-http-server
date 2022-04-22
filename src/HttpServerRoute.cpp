#include "HttpServerRoute.h"

bool HttpServerRoute::PathMatchesUrlPattern(std::string path)
{
    if (path.size() == 0 || url.size() == 0) return false;
    
    auto itPattern = url.begin();
    auto itPath = path.begin();

    // Wild card '*' represents any character, number or _ or -
    bool currentWildFound = false;
    while (itPattern != url.end() && itPath != path.end())
    {
        // Characters matching
        if (*itPattern != '*' && *itPattern == *itPath)
        {
            itPattern++;
            itPath++;
        }
        // Characters not matching
        if (*itPattern != '*' && *itPattern != *itPath)
            return false;

        // Pattern = * and Path IS a char:
        else if (*itPattern == '*'
            && ((*itPath >= '0' && *itPath <= '9')
                || (*itPath >= 'A' && *itPath <= 'Z')
                || (*itPath >= 'a' && *itPath <= 'z')
                || *itPath == '_' || *itPath == '-'))
        {
            if (!currentWildFound) currentWildFound = true;
            itPath++;
        }

        // Pattern = * and Path IS NOT a char:
        else if (*itPattern == '*')
        {
            // No characters found for the current wild card
            if (!currentWildFound) return false;
            else
            {
                *itPattern++;
            }
        }
    }

    if (*itPattern == '*' && currentWildFound) itPattern++;

    return itPattern == url.end() && itPath == path.end();
}
