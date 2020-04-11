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

    class OptionsLoader
    {
    public:
        OptionsLoader() :
            mOptions("Allowed Options")
        {
            mOptions.add_options()
                ("help,H",
                    "Produce this help message")
                ("text-file", bpo::value<std::string>(),
                    "ASCII text file to display")
                ("wait-for-key",
                    "Wait for key press when done.")
                ("lockout-time", bpo::value<int>()->default_value(0),
                    "Number of seconds to lock terminal");
        }

        OptionsData::ptr_t load(int argc, char **argv)
        {
            bpo::variables_map vm;
            try
            {
                bpo::store(bpo::parse_command_line(argc, argv, mOptions), vm);
                bpo::notify(vm);
            }
            catch (std::exception &e)
            {
                std::cerr << "Bad command line:" << std::endl;
                std::cerr << e.what() << std::endl;
                usage(argv[0]);
                return OptionsData::ptr_t();
            }

            if (vm.count("help"))
            {
                usage(argv[0]);
                return OptionsData::ptr_t();
            }

            OptionsData::ptr_t opts(std::make_shared<OptionsData>());

            if (vm.count("text-file"))
                opts->mTextFile = vm["text-file"].as<std::string>();

            opts->mWaitForKey = (vm.count("wait-for-key") != 0);

            if (vm.count("lockout-time"))
                opts->mTimeoutSeconds = static_cast<float>(vm["lockout-time"].as<int>());
            else
                opts->mTimeoutSeconds = 0.f;

            return opts;
        }

        void usage(const std::string &name)
        {
            std::cout << name << std::endl <<
                "\"Rain\" a text file to the terminal." << std::endl <<
                std::endl <<
                mOptions << "\n";
        }
    private:
        bpo::options_description mOptions;
    };

}

int main(int argc, char **argv)
{
    OptionsData::ptr_t opts;

    {
        OptionsLoader loader;

        opts = loader.load(argc, argv);
        if (!opts)
            return -1;
    }

    std::srand(static_cast<unsigned int>(time(nullptr)));
    initialize_curses();

    TextScreen test;

    test.loadScreenText(opts->mTextFile);

    test.play(gWindow);

    if (opts->mWaitForKey)
        getchar();

    shutdown_curses();

    return 0;
}
