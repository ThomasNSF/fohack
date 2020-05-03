
#include "lockoutwindow.h"
#include <iostream>
#include <fstream>
#include <boost/algorithm/string.hpp>

#include <thread>
#include <chrono>

//========================================================================
namespace
{
    bool kbhit()
    {
        int ch = getch();

        if (ch != ERR) 
        {
            ungetch(ch);
            return true;
        }
        return false;
    }
}
