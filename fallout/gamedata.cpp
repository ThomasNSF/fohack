/** 
 */

#include "fallout.h"
#include "gamedata.h"
#include <iostream>
#include <fstream>
#include <boost/algorithm/string.hpp>

//========================================================================
bool FalloutWords::loadWordList(const std::string &filename)
{
    std::ifstream wordlist(filename);

    if (wordlist.fail())
    {
        std::cerr << "Unable to open \"" << filename << "\"" << std::endl;
        return false;
    }
    
    size_t count(0);
    std::string word;
    while (wordlist >> word)
    {
        size_t length(word.size());
        if (length > 3)
        {
            ++count;
            boost::to_upper(word);
            mMasterLists[length].insert(word);
            std::cout << ".";
        }
        else
        {
            std::cout << "X";
        }
    }

    std::cerr << std::endl << "Loaded " << count << " words." << std::endl;

    size_t total(0);
    for (auto it = mMasterLists.begin(); it != mMasterLists.end(); )
    {
        if ((*it).second.size() < 10)
        {
            std::cerr << "Discarding " << (*it).second.size() <<
                " words of length " << (*it).first << std::endl;
            mMasterLists.erase(it++);
        }
        else
        {
            total += ((*it).second.size());
            std::cerr << (*it).second.size() << " words of length " << (*it).first << std::endl;
            ++it;
        }
    }

    std::cerr << "Dictionary contains " << total << " words." << std::endl;

    return true;
}

void FalloutWords::dump()
{
    for (const auto &it : mMasterLists)
    {
        std::cerr << "Size: " << it.first << " count: " << it.second.size() << std::endl <<
            "===================================" << std::endl;

        for (const std::string &word : it.second)
        {
            std::cerr << word << " ";
        }
        std::cerr << std::endl << "===================================" << std::endl << std::endl;
    }
}
//------------------------------------------------------------------------
