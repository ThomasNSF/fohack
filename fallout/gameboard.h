/**
 */

#ifndef FALLOUT_GAMEBOARD_H
#define FALLOUT_GAMEBOARD_H

#include <memory>
#include <array>
#include <vector>
#include <curses.h>

class GameBoard
{
public:
    typedef std::shared_ptr<GameBoard> ptr_t;

    class GameCursor
    {
    public:
        typedef std::shared_ptr<GameCursor> ptr_t;

        GameCursor(int span, int limit, bool wrap);

        bool advanceLeft();
        bool advanceRight();
        bool advanceUp();
        bool advanceDown();

        void setPosition(int pos) { mPosition = pos; }
        void setPosition(int field, int x, int y);

        int getPosition() const { return mPosition; }
        int getField() const;
        int getX() const;
        int getY() const;

    private:
        int mPosition;

        int mSpan;
        int mLimit;
    };

    GameBoard();
    ~GameBoard();

    void                    initialize();
    bool                    play();

    WINDOW *                mGameWindow;
    
    WINDOW *                mPanelHeader;
    WINDOW *                mPanelStatus;
    std::array<WINDOW *, 2> mPanelFiller;
    std::array<WINDOW *, 2> mPanelField;

    static ptr_t BuildGameBoard(WINDOW *mainwindow);

    std::string             mCompanyName;
    int                     mTurnsRemaining;

    std::string             mDisplayField;
    std::vector<int>        mDisplayData;

    GameCursor::ptr_t       mCursor;

    static const int        sFieldWidth;
    static const int        sFieldHeight;

private:
    bool                    moveCursor(int key);

    void                    displayHeader();
    void                    displayFiller();
    void                    displayField();
    void                    initializeGameData();

    bool                    mExit;
};

#endif // !FALLOUT_GAMEBOARD_H
