// Fallout.cpp : Defines the entry point for the application.
//

#include "fallout.h"
#include "gamedata.h"
#include "gameboard.h"
#include <boost/program_options.hpp>
#include <time.h>

WINDOW *gWindow(nullptr);


namespace
{
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

        srand(static_cast<unsigned int>(time(nullptr)));
	}

	void shutdown_curses()
	{
		endwin();
		gWindow = nullptr;
	}

    class OptionsLoader
    {
    public:
        OptionsLoader():
            mOptions("Allowed Options")
        {
            mOptions.add_options()
                ("help,H",                                    
                    "Produce this help message")
                ("company",     bpo::value<std::string>()->default_value("RED ROCKET GARAGE"),  
                    "Set company name in terminal")
                ("wordfile",    bpo::value<std::string>(),  
                    "Word file")
                ("wordcheck",   
                    "Load word file, display its contents and exit.")
                ("boosts",      bpo::value<bool>()->default_value(true),         
                    "Include boosts/dud removal")
                ("difficulty",   bpo::value<int>()->default_value(0),          
                    "Set difficulty\n"
                        "\t0 = Random");
        }

        OptionsData::ptr_t load(int argc, char **argv)
        {
            bpo::variables_map vm;
            try
            { 
                bpo::store(bpo::parse_command_line(argc, argv, mOptions), vm);
                bpo::notify(vm);
            }
            catch(std::exception &e)
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

            if (vm.count("company"))
                opts->mTerminalName = vm["company"].as<std::string>();

            if (vm.count("wordfile"))
                opts->mDataFile = vm["wordfile"].as<std::string>();

            opts->mCheckOnly = (vm.count("wordcheck") > 0);

            if (vm.count("boosts"))
                opts->mPowerups = vm["boosts"].as<bool>();
            else
                opts->mPowerups = true;

            if (vm.count("difficulty"))
                opts->mDifficulty = vm["difficulty"].as<int>();
            else
                opts->mDifficulty = 0;

            return opts;
        }

        void usage(const std::string &name)
        {
            std::cout << name << std::endl <<
                "Fallout-like hacking mini-game." << std::endl <<
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


    FalloutWords::ptr_t words(std::make_shared<FalloutWords>());

    if (!words->loadWordList(opts->mDataFile))
    {
        return -1;
    }

    if (opts->mCheckOnly)
    {
        words->dump();
        return -1;
    }

	initialize_curses();

    GameBoard::ptr_t board = std::make_shared<GameBoard>(gWindow, words, opts);

    while(true)
    {
        board->initialize();
        bool win = board->play();

        board->writeStatus("\nPLAY AGAIN? [Y/N]");
        int ch;
        while(true)
        {
            ch = getch();
            if ((ch == 'Y') || (ch == 'y') || (ch == 'N') || (ch == 'n'))
                break;
            beep(); 
        }

        if ((ch == 'N') || (ch == 'n'))
            break;
    }

    board.reset();
	shutdown_curses();
	return 0;
}
