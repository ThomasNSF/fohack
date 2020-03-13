/**
 */

#ifndef FALLOUT_GAMEBOARD_H
#define FALLOUT_GAMEBOARD_H

#include <memory>
#include <array>
#include <vector>
#include <curses.h>

#include "fallout.h"
#include "gamedata.h"

class GameBoard
{
public:
    typedef std::shared_ptr<GameBoard> ptr_t;

    class GameCursor
    {
    public:
        typedef std::shared_ptr<GameCursor> ptr_t;

        GameCursor(int span, int limit, bool wrap, std::vector<int> &data);

        bool        advanceLeft();
        bool        advanceRight();
        bool        advanceUp();
        bool        advanceDown();

        void        setPosition(int pos) { mPosition = pos; }
        void        setPosition(int field, int x, int y);

        int         getPosition() const { return mPosition; }
        int         getField() const    { return convertToField(mPosition); }
        int         getX() const        { return convertToX(mPosition); }
        int         getY() const        { return convertToY(mPosition); }

        bool        isOnRange() const;
        int         getRangeValue() const;
        int         getRangeStart() const;
        int         getRangeEnd() const;

        int         convertToField(int position) const;
        int         convertToX(int position) const;
        int         convertToY(int position) const;

    private:
        std::vector<int> &  mFieldData;
        int                 mPosition;

        int                 mSpan;
        int                 mLimit;
    };

    GameBoard(WINDOW *mainwindow, const FalloutWords::ptr_t &words, const OptionsData::ptr_t &opts);
    ~GameBoard();

    void                    initialize();
    bool                    play();
    void                    writeStatus(const std::string &status);

    static const int        sFieldWidth;
    static const int        sFieldHeight;

private:
    bool                    moveCursor(int key);
    bool                    handleEnter();

    void                    displayHeader();
    void                    displayFiller();
    void                    displayField();
    void                    displayStatus();

    void                    writePreview(const std::string &status, bool restore_cursor = true);
    void                    clearPreview();
    bool                    previewUnderCursor(bool restore_cursor = true);

    void                    initializeGameData();
    void                    initializeWords();

    int                     calculateLikeness(const std::string &test);
    void                    failGuess(int selection);

    WINDOW *                mGameWindow;
    
    WINDOW *                mPanelHeader;
    WINDOW *                mPanelStatus;
    std::array<WINDOW *, 2> mPanelFiller;
    std::array<WINDOW *, 2> mPanelField;

    std::string             mCompanyName;
    int                     mTurnsRemaining;
    int                     mPasswordIndex;

    std::string             mDisplayField;
    std::vector<int>        mDisplayData;

    GameCursor::ptr_t       mCursor;
    bool                    mExit;
    bool                    mWin;

    FalloutWords::string_vec_t  mPasswords;

    FalloutWords::ptr_t     mWords; 
    OptionsData::ptr_t      mOpts;
};

#endif // !FALLOUT_GAMEBOARD_H
