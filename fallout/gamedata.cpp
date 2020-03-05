/** 
 */

#include "fallout.h"
#include "gamedata.h"

//========================================================================

FalloutWords::ptr_t FalloutWords::CreateMasterWordLists()
{
    FalloutWords::ptr_t words(std::make_shared<FalloutWords>());



    return words;
}

bool FalloutWords::LoadWordList(const std::string &filename)
{
    return false;
}

//------------------------------------------------------------------------
