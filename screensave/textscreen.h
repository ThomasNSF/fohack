
#ifndef TEXTSCREEN_H
#define TEXTSCREEN_H

#include "screensave.h"

class TextScreen
{
public:

                    TextScreen(const OptionsData::ptr_t &opts);
                    ~TextScreen() = default;

    bool            loadScreenText(const std::string &);

    void            play(WINDOW *pwin);
private:
    typedef std::vector<std::string>    text_vect_t;
    class ColumnDef
    {
    public:
        typedef std::shared_ptr<ColumnDef>  ptr_t;

        ColumnDef(int column_number, const std::string &column_text):
            mColumnNumber(column_number),
            mColumnText(column_text),
            mTargetRow(0),
            mCurrentRow(0)
        {
            mTargetRow = (int)mColumnText.size() - 1;
        }

        bool        process(WINDOW *pwin, int offset_x, int offset_y);
        bool        isDone() const;
        void        reset();
    private:
        int         mColumnNumber;
        std::string mColumnText;
        int         mTargetRow;
        int         mCurrentRow;
    };

    typedef std::map<int, ColumnDef::ptr_t> column_map_t;

    void                buildColumns(const text_vect_t &columns);

    column_map_t        mColumns;
    OptionsData::ptr_t  mOpts;
    int                 mMaxColumn;

    static const int    sSpacing;
    static const int    sMaxInflight;
};

#endif // !TEXTSCREEN_H
