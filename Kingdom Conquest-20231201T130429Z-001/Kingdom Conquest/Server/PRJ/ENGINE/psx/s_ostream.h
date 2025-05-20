// Copyright (C) Pixel Studio
// All rights reserved.
// 
// Author : Jiang Li
// Date   : 2005-8-31

#ifndef __OUTPUT_STREAM_H_G_SRC_SOURCE_PG_PSX__
#define __OUTPUT_STREAM_H_G_SRC_SOURCE_PG_PSX__

#pragma once

#include <psx/config.h>

__PG_PSX_SPACE_BEGIN__

enum ps_info_t
{
    psinfo_normal,
    psinfo_warning,
    psinfo_error,
    psinfo_link_error,
};

class pg_psx_public i_s_ostream : public psIOutputStream
{
public:
	virtual void Write(const wchar_t* text);
	virtual void WriteWarning(const wchar_t* text, const wchar_t* filename, int row, int col);
	virtual void WriteError(const wchar_t* text, const wchar_t* filename, int row, int col);
    virtual void WriteLinkError(const wchar_t* text);

    virtual void write(ps_info_t info, const wchar_t* txt, const wchar_t* fileName, int row, int col) = 0;
};

class pg_psx_public s_function_ostream : public i_s_ostream
{
public:
    typedef function<void(ps_info_t info, const wchar_t*, const wchar_t*, int row, int col)> write_fn_t;

    write_fn_t      on_write;
public:
    virtual void write(ps_info_t info, const wchar_t* txt, const wchar_t* fileName, int row, int col);
};

class pg_psx_public s_wstring_ostream : public i_s_ostream
{
protected:
    wstring::scoped_string_t string_buf;
public:
    virtual void write(ps_info_t info, const wchar_t* txt, const wchar_t* fileName, int row, int col);

    wstring::scoped_string_t& str();
};

__PG_PSX_SPACE_END__

#endif  // __OUTPUT_STREAM_H_G_SRC_SOURCE_PG_PSX__