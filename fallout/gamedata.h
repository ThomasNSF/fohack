/**
 */

#ifndef FALLOUT_GAMEDATA_H
#define FALLOUT_GAMEDATA_H

#include <memory>
#include <vector>
#include <set>
#include <map>

class FalloutWords
{
public:
    typedef std::shared_ptr<FalloutWords>   ptr_t;
    typedef std::vector<std::string>    string_vec_t;

    FalloutWords()
    {}

    ~FalloutWords()
    {}

    bool                loadWordList(const std::string &filename);
    void                dump();

private:
    typedef std::set<std::string>       string_set_t;
    typedef std::map<size_t, string_set_t> string_length_map_t;

    string_length_map_t mMasterLists;
};

#endif // !FALLOUT_GAMEDATA_H
