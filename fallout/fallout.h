// Fallout.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <iostream>
#include <memory>
#include <algorithm>
#include <curses.h>

#include <boost/program_options.hpp>

//========================================================================
struct OptionsData
{
    typedef std::shared_ptr<OptionsData> ptr_t;

    std::string     mTerminalName;
    std::string     mDataFile;
    int             mDifficulty;
    bool            mPowerups;
    bool            mCheckOnly;
    bool            mSinglePlay;
    bool            mPlayUntilWin;
};

extern WINDOW *gWindow;
