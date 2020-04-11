
#ifndef SCREENSAVE_H
#define SCREENSAVE_H

#include <iostream>
#include <memory>
#include <algorithm>
#include <curses.h>

#include <boost/program_options.hpp>

struct OptionsData
{
    typedef std::shared_ptr<OptionsData> ptr_t;

    std::string   mTextFile;
    bool          mWaitForKey;
    float         mTimeoutSeconds;
};

#endif // !SCREENSAVE_H

