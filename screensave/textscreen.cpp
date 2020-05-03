
#include "textscreen.h"
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

//========================================================================
const int TextScreen::sSpacing(5);
const int TextScreen::sMaxInflight(5);

//========================================================================
TextScreen::TextScreen(const OptionsData::ptr_t &opts):
    mOpts(opts)
{}

//-------------------------------------------------------------------------
bool TextScreen::loadScreenText(const std::string &filepath)
{
    std::ifstream infile(filepath);

    if (infile.fail())
    {
        std::cerr << "Unable to load \"" << filepath << "\"" << std::endl;
        return false;
    }

    text_vect_t lines;

    size_t max_length(0);
    std::string line;

    while (std::getline(infile, line))
    {
        if (max_length < line.length())
            max_length = line.length();

        lines.push_back(line);
        //std::cerr << line << std::endl;
    }
   
    //std::cerr << max_length << std::endl;
    text_vect_t text_columns;
    text_columns.resize(max_length);

    for (const auto &line : lines)
    {
        for (int index(0); index < line.size(); ++index)
        {
            text_columns[index] += line[index];
        }
    }

    for (auto & line : text_columns)
    {
        boost::trim_right(line);
    }

    buildColumns(text_columns);
    //-------------------------------------------
    return true;
}

void TextScreen::buildColumns(const text_vect_t &columns)
{
    mColumns.clear();
    mMaxColumn = 0;

    for (int col = 0; col < columns.size(); ++col)
    {
        if (columns[col].size() > mMaxColumn)
            mMaxColumn = columns[col].size();
        ColumnDef::ptr_t columndef(std::make_shared<ColumnDef>(col, columns[col]));
        mColumns[col] = columndef;
    }
}

namespace
{
    template<class T>
    typename T::iterator select_random_column(T &collection)
    {
        if (collection.empty())
            return collection.end();
        size_t index(std::rand() % collection.size());

        typename T::iterator it(collection.begin());

        while (index--)
            ++it;

        return it;
    }
}

void TextScreen::play(WINDOW *pwin)
{
    column_map_t remaining(mColumns);
    column_map_t inflight;
    int spacing_count(0);

    int window_x(0);
    int window_y(0);
    getmaxyx(pwin, window_y, window_x);

    int offset_x((window_x - mColumns.size()) / 2);
    int offset_y((window_y - mMaxColumn) / 2);

    while (!remaining.empty() || !inflight.empty())
    {
        --spacing_count;
        if ((inflight.size() < sMaxInflight) && (spacing_count < 0))
        {
            spacing_count = sSpacing;
            column_map_t::iterator it1 = select_random_column(remaining);
            if (it1 != remaining.end())
            {
                if (!(*it1).second->isDone())
                {
                    inflight[(*it1).first] = (*it1).second;
                }
                else
                {
                    spacing_count = 0;
                }
                remaining.erase(it1);
            }
        }

        column_map_t::iterator it2 = inflight.begin(); 
        while(it2 != inflight.end())
        {
            if ((*it2).second->process(pwin, offset_x, offset_y))
            {
                ++it2;
            }
            else
            {
                if (!(*it2).second->isDone())
                {
                    remaining[(*it2).first] = (*it2).second;
                }
                inflight.erase(it2++);
            }
        }
        wrefresh(pwin);
        //std::this_thread::sleep_for(std::chrono::milliseconds(10));

#if 0
        if ((mOpts->mTimeoutSeconds <= 1.0f) && kbhit())
            break;
#endif
    }
}

//========================================================================
bool TextScreen::ColumnDef::process(WINDOW *pwin, int offset_x, int offset_y)
{
    if (mCurrentRow)
    {
        mvwaddch(pwin, (mCurrentRow - 1) + offset_y, mColumnNumber + offset_x, ' ');
        //std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    mvwaddch(pwin, mCurrentRow + offset_y, (int)(mColumnNumber + offset_x), mColumnText[mTargetRow]);
    
    ++mCurrentRow;
    if (mCurrentRow < mTargetRow)
        return true;

    mCurrentRow = 0;
    --mTargetRow;
    while ((mTargetRow >= 0) && (mColumnText[mTargetRow] == ' '))
    {
        --mTargetRow;
    }

    return false;
}

bool TextScreen::ColumnDef::isDone() const
{
    return (mTargetRow < 0);
}

void TextScreen::ColumnDef::reset()
{
    mCurrentRow = 0;
    mTargetRow = (int)mColumnText.size() - 1;
}
