
#ifndef SCREENSAVE_H
#define SCREENSAVE_H

#include <iostream>
#include <memory>
#include <algorithm>
#include <curses.h>

#include <boost/program_options.hpp>
#include "boost/filesystem.hpp"

namespace bfs = boost::filesystem;

struct OptionsData
{
    typedef std::shared_ptr<OptionsData> ptr_t;

    bfs::path   mTextFile;
    bool                mWaitForKey;
    float               mTimeoutSeconds;
};

#endif // !SCREENSAVE_H

