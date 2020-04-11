/** 
 */

#include "fallout.h"
#include "gameboard.h"
#include <algorithm>
#include <cctype>

//========================================================================
namespace
{
    const char KEY_ESC(0x1b);     /* Escape */
    const char KEY_RETURN(0x0a);  /* Return */

    //const std::string FILLER_CHARS("\\/!@#$%^'\",.-_&*(){}[]<>");
    const std::string FILLER_CHARS("\\\\//!!@@##$$%%^^''\"\",--_&&*((){{}[[]<<>");
    const std::string CLOSING_CHARS(")}]>");
    std::map<char, char> MATCHING_BRACE({
        {')', '('},
        {'}', '{'},
        {']', '['},
        {'>', '<'} });

    int generate_random_addr()
    {
        int address(0);

        for (int i = 0; i < 4; ++i)
        {
            address = address << 4;
            if (i == 3)
                address |= (std::rand() % 2) * 8;
            else
                address |= std::rand() % 0xF;
        }

        return address;
    }
}

//========================================================================
const int GameBoard::sFieldWidth(12);
const int GameBoard::sFieldHeight(17);
const int GameBoard::sMaxTurns(4);

//------------------------------------------------------------------------
GameBoard::GameBoard(WINDOW *mainwindow, const FalloutWords::ptr_t &words, 
        const OptionsData::ptr_t &opts):
    mGameWindow(mainwindow),
    mPanelHeader(nullptr),
    mPanelStatus(nullptr),
    mPanelFiller({ nullptr, nullptr }),
    mPanelField({ nullptr, nullptr }),
    mCompanyName(),
    mTurnsRemaining(sMaxTurns),
    mPasswordIndex(-1),
    mCursor(),
    mExit(false),
    mWin(false),
    mWords(words),
    mOpts(opts)
{ 
    mGameWindow = mainwindow;

    mCompanyName = opts->mTerminalName;

    mPanelHeader = newwin(5, 40, 0, 0);
    mPanelFiller[0] = newwin(17, 6, 5, 0);
    mPanelFiller[1] = dupwin(mPanelFiller[0]);
    mvwin(mPanelFiller[1], 5, 20);
    mPanelField[0] = newwin(sFieldHeight, sFieldWidth, 5, 7);
    mPanelField[1] = dupwin(mPanelField[0]);
    mvwin(mPanelField[1], 5, 27);
    mPanelStatus = newwin(17, 20, 5, 40);
    scrollok(mPanelStatus, true);
    immedok(mPanelStatus, true);
    wmove(mPanelStatus, 16, 0);
}

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

void GameBoard::initialize()
{
    mTurnsRemaining = 4;
    mWin = false;
    mExit = false;

    wclear(mPanelHeader);
    wclear(mPanelFiller[0]);
    wclear(mPanelFiller[1]);
    wclear(mPanelField[0]);
    wclear(mPanelField[1]);
    wclear(mPanelStatus);

    initializeGameData();

    refresh();

    displayHeader();
    displayFiller();
    displayField();
    displayStatus();

    if (mPanelStatus)
        wrefresh(mPanelStatus);
}

void GameBoard::setPlayDifficulty(int difficulty)
{
    if (!difficulty)
        mPlayDifficulty = (std::rand() % 3) + 1;
    else
        mPlayDifficulty = difficulty;
}

void GameBoard::initializeGameData()
{
    mCursor = std::make_shared<GameCursor>(sFieldWidth, sFieldHeight, false, mDisplayData);

    int total_length(sFieldWidth * sFieldHeight * 2);
    mDisplayField.clear();
    
    mDisplayField.resize(total_length);

    for (char &c : mDisplayField)
    {
        c = FILLER_CHARS[std::rand() % FILLER_CHARS.size()];
    }

    mDisplayData.clear();
    mDisplayData.resize(total_length, 0);

    setPlayDifficulty(mOpts->mDifficulty);
    initializeWords();
    if (mOpts->mPowerups)
        initializeDuds();
}

void GameBoard::initializeWords()
{
    int total_length(sFieldWidth * sFieldHeight * 2);

    mPasswords.clear();

    const FalloutWords::string_set_t &wordset(mWords->selectWordSet(mPlayDifficulty));
    size_t wordlength(wordset.begin()->length());

    FalloutWords::string_vec_t list(wordset.begin(), wordset.end());

    std::random_shuffle(list.begin(), list.end());

    size_t maxwords(9);

    size_t wordcount(std::min(list.size(), maxwords));
    size_t span(total_length / wordcount);
    size_t padding(((span - 2) - wordlength) / 2);
  
    size_t count(0);
    for (const std::string &word : list)
    {
        if (count >= wordcount)
            break;
        size_t start((std::rand() % padding) + (count * span));
        mPasswords.push_back(word);
        std::string::iterator it_start(mDisplayField.begin() + start);
        std::vector<int>::iterator it_markers(mDisplayData.begin() + start);

        std::copy(word.begin(), word.end(), it_start);
        std::fill(it_markers, it_markers + wordlength, int(count + 1));
        ++count;
    }

    mPasswordIndex = std::rand() % mPasswords.size();
}

void GameBoard::initializeDuds()
{
    int    dud_count(0);
    size_t end_pos(mDisplayField.size());
    size_t start_pos(std::string::npos);

    while(true)
    {
        end_pos = mDisplayField.find_last_of(CLOSING_CHARS, end_pos);
        if (end_pos == std::string::npos)
            break;

        char opening_brace = MATCHING_BRACE[ mDisplayField[end_pos] ];
        start_pos = mDisplayField.find_last_of(opening_brace, end_pos);

        if (start_pos != std::string::npos)
        {   // found the matching open brace
            std::string dud_span(mDisplayField.substr(start_pos, (end_pos - start_pos)+1));

            if (dud_span.size() < sFieldWidth)
            {
                if (std::find_if(dud_span.begin(), dud_span.end(),
                        [](const char &c) { return std::isalpha(c); }) == dud_span.end())
                {   // no aplhabetic chars in the span
                    dud_count++;
                    for (size_t i = start_pos; i <= end_pos; ++i)
                    {
                        mDisplayData[i] = -dud_count;
                    }
                    end_pos = start_pos;
                    continue;
                }
            }
        }

        // nothing was found... back end_pos up by one and try again.
        if (!end_pos)
            break;
        --end_pos;
    }
}

bool GameBoard::play()
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
            if (moveCursor(key))
                displayField();
            else
                beep();
            break;

        case KEY_RETURN:
        case KEY_ENTER:
            handleEnter();
            break;

        default:
            beep();
            break;
        }
    }

    return mWin;
}

bool GameBoard::moveCursor(int key)
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

bool GameBoard::handleEnter()
{
    if (!mCursor->isOnRange())
        return false;

    int selected(mCursor->getRangeValue());
    if (selected > 0)
    {
        handlePasswordGuess(selected);
    } 
    else if (selected < 0)
    {
        handleDudRemoval(selected);
    }

    if (!mExit)
        writeStatus("ENTER PASSWORD NOW\n> ");
    return true;
}

void GameBoard::handlePasswordGuess(int selected)
{
    writeStatus(mPasswords[selected - 1]);
    writeStatus("\n");        
    int likeness(calculateLikeness(mPasswords[selected - 1]));
    std::stringstream result;
    if (likeness < mPasswords[selected - 1].size())
    {
        failGuess(selected);
        result << "LIKENESS=" << likeness << "\n\n";
        result << "ENTRY DENIED!\n";
    }
    else
    {
        result << "ENTRY GRANTED!\n";
        mWin = true;
        mExit = true;
    }
    writeStatus(result.str());
}

void GameBoard::handleDudRemoval(int selected)
{
    clearSelection(selected);
    writeStatus("\n");

    if ((std::rand() % 20) == 0)
    {   // 5% chance to restore turns
        mTurnsRemaining = sMaxTurns;
        writeStatus("TURNS RESET\n");
        displayHeader();
    }
    else
    {
        std::set<int>   outstanding;
        for (const int &value : mDisplayData)
        {   // collect unselected duds
            if ((value > 0) && ((value - 1) != mPasswordIndex))
                outstanding.insert(value);
        }
        if (!outstanding.empty())
        {
            size_t duds(outstanding.size());
            size_t dud_remove(std::rand() % duds);

            auto it_remove = outstanding.begin();
            while (dud_remove--)
                ++it_remove;

            int dud_index = (*it_remove);

            clearSelection(dud_index, true);
            writeStatus("DUD REMOVED\n");
        }
    }
}

void GameBoard::displayHeader()
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
        wclrtoeol(mPanelHeader);
        wrefresh(mPanelHeader);
    }
}

void GameBoard::displayFiller()
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

void GameBoard::displayField()
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

#if 0
        for (int i = 0; i < mDisplayData.size(); ++i)
        {
            if (mDisplayData[i] != 0)
                continue;
            std::stringstream convert;
            convert << mDisplayData[i];

            WINDOW *field(mPanelField[mCursor->convertToField(i)]);

            int posy(mCursor->convertToY(i));
            int posx(mCursor->convertToX(i));

            mvwaddch(field, posy, posx, convert.str()[0]);
        }
#endif

        if (mCursor)
        { 
            if (mCursor->isOnRange())
            {
                for (int i = mCursor->getRangeStart(); i < mCursor->getRangeEnd(); ++i)
                {
                    WINDOW *field(mPanelField[mCursor->convertToField(i)]);

                    int posy(mCursor->convertToY(i));
                    int posx(mCursor->convertToX(i));

                    wattrset(field, A_REVERSE);
                    mvwaddch(field, posy, posx, mDisplayField[i]);
                    wattroff(field, A_REVERSE);
               }

                previewUnderCursor();
            }
            else
            {
                WINDOW *field(mPanelField[mCursor->getField()]);

                int posy(mCursor->getY());
                int posx(mCursor->getX());

                wattrset(field, A_REVERSE);
                mvwaddch(field, posy, posx, mDisplayField[mCursor->getPosition()]);
                wattroff(field, A_REVERSE);
                clearPreview();
            }
        }
        for (WINDOW *field : mPanelField)
        {
            wrefresh(field);
        }
    }
}

void GameBoard::displayStatus()
{
    writeStatus("ENTER PASSWORD NOW\n> ");
}

void GameBoard::writeStatus(const std::string &status)
{
    wprintw(mPanelStatus, status.c_str());
}

void GameBoard::writePreview(const std::string &preview, bool restore_cursor)
{
    int posx;
    int posy;

    getyx(mPanelStatus, posy, posx);
    wprintw(mPanelStatus, preview.c_str());
    if (restore_cursor)
        wmove(mPanelStatus, posy, posx);
}

void GameBoard::clearPreview()
{
    wclrtoeol(mPanelStatus);
}

bool GameBoard::previewUnderCursor(bool restore_cursor)
{
    int selected = mCursor->getRangeValue();
    
    if (!selected)
    {
        clearPreview();
        return false;
    }
    std::string preview;
    if (selected > 0)
        preview = mPasswords[selected - 1];
    else
    {
        auto it_start = std::find(mDisplayData.begin(), mDisplayData.end(), selected);
        auto it_end = std::find_if_not(it_start, mDisplayData.end(), [selected](int test) { return test == selected; });

        size_t pos_start = std::distance(mDisplayData.begin(), it_start);
        size_t pos_end = std::distance(mDisplayData.begin(), it_end);

        preview = mDisplayField.substr(pos_start, (pos_end - pos_start));
    }
    writePreview(preview);
    return true;
}

int GameBoard::calculateLikeness(const std::string &test)
{
    if (mPasswordIndex < 0)
        return 0;

    int likeness(0);
    std::string password(mPasswords[mPasswordIndex]);

    for (int i = 0; i < password.size(); ++i)
    {
        if (password[i] == test[i])
            ++likeness;
    }

    return likeness;
}

void GameBoard::failGuess(int selection)
{
    clearSelection(selection);
    
    --mTurnsRemaining;
    displayHeader();
    if (!mTurnsRemaining)
        mExit = true;
}

void GameBoard::clearSelection(int selection, bool clear_text )
{
    size_t index(0);
    for (int &value : mDisplayData)
    {
        if (value == selection)
        {
            value = 0;
            if (clear_text)
                mDisplayField[index] = '.';
        }
        ++index;
    }
    if (clear_text)
        displayField();
}

//========================================================================
GameBoard::GameCursor::GameCursor(int span, int limit, bool wrap, std::vector<int> &data):
    mPosition(0),
    mSpan(span),
    mLimit(limit),
    mFieldData(data)
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

int GameBoard::GameCursor::convertToField(int position) const
{
    return (position / (mSpan * mLimit));
}

int GameBoard::GameCursor::convertToX(int position) const
{
    int field_length(mSpan * mLimit);

    int subrange(position % field_length);

    return (subrange % mSpan);
}

int GameBoard::GameCursor::convertToY(int position) const
{
    int field_length(mSpan * mLimit);

    int subrange(position % field_length);

    return (subrange / mSpan);
}

bool GameBoard::GameCursor::isOnRange() const
{
    return (mFieldData[mPosition] != 0);
}

int GameBoard::GameCursor::getRangeValue() const
{
    return (mFieldData[mPosition]);
}

int GameBoard::GameCursor::getRangeStart() const
{
    if (!isOnRange())
        return mPosition;

    int key(mFieldData[mPosition]);
    std::vector<int>::iterator it_cur(mFieldData.begin() + mPosition);

    std::vector<int>::iterator start(std::find(mFieldData.begin(), it_cur + 1, key));
    return int(std::distance(mFieldData.begin(), start));
}

int GameBoard::GameCursor::getRangeEnd() const
{
    if (!isOnRange())
        return mPosition;

    int key(mFieldData[mPosition]);
    std::vector<int>::iterator it_cur(mFieldData.begin() + mPosition);

    std::vector<int>::iterator end(std::find_if_not(it_cur, mFieldData.end(), 
        [key](const int &test) { return test == key; }));

    return mPosition + int(std::distance(it_cur, end));
}
