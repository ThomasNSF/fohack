// screensave.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "screensave.h"
#include "textscreen.h"

namespace
{
    WINDOW *gWindow(nullptr);

    namespace bpo = boost::program_options;

    void initialize_curses()
    {
        gWindow = initscr();

        cbreak();              /* direct input (no newline required)... */
        noecho();              /* ... without echoing */
        curs_set(0);           /* hide cursor (if possible) */
        nodelay(gWindow, TRUE);  /* don't wait for input... */
        halfdelay(10);         /* ...well, no more than a second, anyway */
        keypad(gWindow, TRUE);   /* enable cursor keys */
        wtimeout(gWindow, 0);
        //set_escdelay(0);

        std::srand(static_cast<unsigned int>(time(nullptr)));
    }

    void shutdown_curses()
    {
        endwin();
        gWindow = nullptr;
    }
}

int main(int argc, char **argv)
{
    std::srand(static_cast<unsigned int>(time(nullptr)));
    initialize_curses();

    TextScreen test;

    test.loadScreenText("./data/rr-cropped.txt");
    //test.loadScreenText("./data/pipboy.txt");

    test.play(gWindow);
    getchar();

    shutdown_curses();

    return 0;
}
