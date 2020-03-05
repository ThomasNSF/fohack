/** 
 */

#include "fallout.h"
#include "gameboard.h"

//========================================================================
namespace
{
    const char KEY_ESC(0x1b);     /* Escape */

    std::string FILLER_CHARS("\\/!@#$%^'\",.-_&*(){}[]");

    int generate_random_addr()
    {
        int address(0);

        for (int i = 0; i < 4; ++i)
        {
            address = address << 4;
            if (i == 3)
                address |= (rand() % 2) * 8;
            else
                address |= rand() % 0xF;
        }

        return address;
    }
}

//========================================================================
const int GameBoard::sFieldWidth(12);
const int GameBoard::sFieldHeight(17);

//------------------------------------------------------------------------
GameBoard::GameBoard() :
    mGameWindow(nullptr),
    mPanelHeader(nullptr),
    mPanelStatus(nullptr),
    mPanelFiller({ nullptr, nullptr }),
    mPanelField({ nullptr, nullptr }),
    mCompanyName("RED ROCKET GARAGE"),
    mTurnsRemaining(3),
    mCursor(),
    mExit(false)
{ }

GameBoard::~GameBoard()
{
    mGameWindow = nullptr;

    if (mPanelHeader)
        delwin(mPanelHeader);
    if (mPanelStatus)
        delwin(mPanelStatus);

    for (WINDOW *filler : mPanelFiller)
    {
		if (filler)
			delwin(filler);
    }

    for (WINDOW *field : mPanelField)
    {
		if (field)
			delwin(field);
    }
}

void GameBoard::Initialize()
{
    InitializeGameData();

    refresh();

    DisplayHeader();
    DisplayFiller();
    DisplayField();

    if (mPanelStatus)
        wrefresh(mPanelStatus);
}

void GameBoard::InitializeGameData()
{
    mCursor = std::make_shared<GameCursor>(sFieldWidth, sFieldHeight, false);

    int total_length(sFieldWidth * sFieldHeight * 2);
    mDisplayField.clear();
    
    mDisplayField.resize(total_length);

    for (char &c : mDisplayField)
    {
        c = FILLER_CHARS[rand() % FILLER_CHARS.size()];
    }
}

bool GameBoard::Play()
{
    while (!mExit)
    {
        int key(getch());

        if (key == ERR)
            continue;

        switch (key)
        {
        case KEY_ESC:
            mExit = true;
            break;

        case KEY_UP:
        case KEY_DOWN:
        case KEY_LEFT:
        case KEY_RIGHT:
            if (MoveCursor(key))
                DisplayField();
            else
                beep();
            break;

        case KEY_ENTER:
            break;

        default:
            beep();
            break;
        }
    }

    return true;
}

bool GameBoard::MoveCursor(int key)
{
    bool success(false);
    switch (key)
    {
    case KEY_UP:
        success = mCursor->advanceUp();
        break;
    case KEY_DOWN:
        success = mCursor->advanceDown();
        break;
    case KEY_LEFT:
        success = mCursor->advanceLeft();
        break;
    case KEY_RIGHT:
        success = mCursor->advanceRight();
        break;
    default:
        break;
    }

    return success;
}


void GameBoard::DisplayHeader()
{
    if (mPanelHeader)
    {
        mvwprintw(mPanelHeader, 0, 0, "%s TERMLINK PROTOCOL\nENTER PASSWORD NOW", mCompanyName.c_str());

        mvwprintw(mPanelHeader, 3, 0, "ATTEMPTS REMAINING: %d ", mTurnsRemaining);
        for (int i = 0; i < mTurnsRemaining; ++i)
        {
            waddch(mPanelHeader, '\xDB');
            waddch(mPanelHeader, ' ');
        }
        wrefresh(mPanelHeader);
    }
}

void GameBoard::DisplayFiller()
{
    if (mPanelFiller[0] && mPanelFiller[1])
    {
        int address(generate_random_addr());
        int limit(getmaxy(mPanelFiller[0]));
        int span(getmaxx(mPanelField[0]));

        for (int i = 0; i < (limit * 2); ++i)
        {
            mvwprintw(mPanelFiller[i / limit], i % limit, 0,
                "0X%04X", address);
            address += span;
        }
        for (WINDOW *filler : mPanelFiller)
        {
            wrefresh(filler);
        }
    }
}

void GameBoard::DisplayField()
{
    if (mPanelField[0] && mPanelField[1])
    {
        int span(getmaxx(mPanelField[0]));
        int limit(getmaxy(mPanelField[0]));

        int field_length(span * limit);
        std::string::iterator itStart(mDisplayField.begin());
        std::string sub1(itStart, itStart + field_length);
        std::string sub2(itStart + field_length, mDisplayField.end());

        mvwprintw(mPanelField[0], 0, 0, "%s", sub1.c_str());
        mvwprintw(mPanelField[1], 0, 0, "%s", sub2.c_str());

        if (mCursor)
        { 
            WINDOW *field(mPanelField[ mCursor->getField() ]);

            int posy(mCursor->getY());
            int posx(mCursor->getX());

            wattrset(field, A_REVERSE);
            mvwaddch(field, posy, posx, mDisplayField[mCursor->getPosition()]);
            wattroff(field, A_REVERSE);
        }
        for (WINDOW *field : mPanelField)
        {
            wrefresh(field);
        }
    }
}


//------------------------------------------------------------------------
GameBoard::ptr_t GameBoard::BuildGameBoard(WINDOW *mainwindow)
{
    GameBoard::ptr_t bw(std::make_shared<GameBoard>());

    bw->mGameWindow = mainwindow;

    bw->mPanelHeader = newwin(5, 40, 0, 0);
    bw->mPanelFiller[0] = newwin(17, 6, 5, 0);
    bw->mPanelFiller[1] = dupwin(bw->mPanelFiller[0]);
    mvwin(bw->mPanelFiller[1], 5, 20);
    bw->mPanelField[0] = newwin(sFieldHeight, sFieldWidth, 5, 7);
    bw->mPanelField[1] = dupwin(bw->mPanelField[0]);
    mvwin(bw->mPanelField[1], 5, 27);
	//bw->mPanelStatus ;

    return bw;
}

//========================================================================
GameBoard::GameCursor::GameCursor(int span, int limit, bool wrap):
    mPosition(0),
    mSpan(span),
    mLimit(limit)
{
}

bool GameBoard::GameCursor::advanceLeft()
{
    int field(getField());
    int x(getX());
    int y(getY());

    if (x == 0)
    {
        if (field > 0)
        {
            x = mSpan - 1;
            --field;
        }
        else
        {
            return false;
        }
    }
    else
    {
        --x;
    }

    setPosition(field, x, y);
    return true;
}

bool GameBoard::GameCursor::advanceRight()
{
    int field(getField());
    int x(getX());
    int y(getY());

    if (x >= (mSpan - 1))
    {
        if (field == 0)
        {
            x = 0;
            ++field;
        }
        else
        {
            return false;
        }
    }
    else
    {
        ++x;
    }

    setPosition(field, x, y);
    return true;
}

bool GameBoard::GameCursor::advanceUp()
{
    int field(getField());
    int x(getX());
    int y(getY());

    if (y < 1)
        return false;

    --y;
    setPosition(field, x, y);
    return true;
}

bool GameBoard::GameCursor::advanceDown()
{
    int field(getField());
    int x(getX());
    int y(getY());

    if (y >= (mLimit - 1))
        return false;

    ++y;
    setPosition(field, x, y);
    return true;
}

void GameBoard::GameCursor::setPosition(int field, int x, int y)
{
    mPosition = (field * (mSpan * mLimit)) + (y * mSpan) + x;
}

int GameBoard::GameCursor::getField() const
{
    return (mPosition / (mSpan * mLimit));
}

int GameBoard::GameCursor::getX() const
{
    int field_length(mSpan * mLimit);

    int subrange(mPosition % field_length);

    return (subrange % mSpan);
}

int GameBoard::GameCursor::getY() const
{
    int field_length(mSpan * mLimit);

    int subrange(mPosition % field_length);

    return (subrange / mSpan);
}
