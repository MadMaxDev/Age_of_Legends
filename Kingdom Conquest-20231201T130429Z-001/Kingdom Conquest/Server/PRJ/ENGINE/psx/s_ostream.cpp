// Copyright (C) Pixel Studio
// All rights reserved.
// 
// Author : Jiang Li
// Date   : 2005-8-31

#include "commoninc.h"
#include <psx/s_ostream.h>
#include <guic/text/convert.h>

__PG_PSX_SPACE_BEGIN__

void i_s_ostream::Write(const wchar_t* text)
{
    write(psinfo_normal, text, 0, 0, 0);
}

void i_s_ostream::WriteWarning(const wchar_t* text, const wchar_t* filename, int row, int col)
{
    write(psinfo_normal, text, filename, row, col);
}

void i_s_ostream::WriteError(const wchar_t* text, const wchar_t* filename, int row, int col)
{
    write(psinfo_error, text, filename, row, col);
}

void i_s_ostream::WriteLinkError(const wchar_t* text)
{
    write(psinfo_link_error, text, 0, 0, 0);
}

// s_function_ostream

void s_function_ostream::write(ps_info_t info, const wchar_t* txt, const wchar_t* fileName, int row, int col)
{
    if(on_write)
    {
        on_write(info, txt, fileName, row, col);
    }
}

// s_string_ostream

void s_wstring_ostream::write(ps_info_t info, const wchar_t* txt, const wchar_t* fileName, int row, int col)
{
    switch(info)
    {
    case psinfo_warning:
        {
            string_buf.append(fileName);
            wchar_t x[64];
            text::format(x, sizeof(x), L"(%d, %d):[warning]", row, col);
            string_buf.append(x);
            string_buf.append(txt);
        }
        break;
    case psinfo_error:
        {
            string_buf.append(fileName);
            wchar_t x[64];
            text::format(x, sizeof(x), L"(%d, %d):[error]", row, col);
            string_buf.append(x);
            string_buf.append(txt);
        }
        break;
    case psinfo_link_error:
        {
            string_buf.append(L"[link error]");
            string_buf.append(txt);
        }
        break;
    default:
        string_buf.append(txt);
    }
    string_buf.append(L"\r\n");
}

wstring::scoped_string_t& s_wstring_ostream::str()
{
    return string_buf;
}


__PG_PSX_SPACE_END__