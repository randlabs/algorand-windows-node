//
//     Algorand Node for Windows -- Service  implementation
//     Copyright (C) 2021   Rand Labs 
//
//     This program is free software: you can redistribute it and/or modify
//     it under the terms of the GNU Affero General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     This program is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU Affero General Public License for more details.
//
//     You should have received a copy of the GNU Affero General Public License
//     along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
#ifndef __DPRINTF_H__
#define __DPRINTF_H__

#include <Windows.h>
#include <strsafe.h>

inline void dprintfW(const wchar_t* fmt, ...)
{
	const size_t MAX_MSG = 255;

	va_list args;
	va_start(args, fmt);

	wchar_t msg[MAX_MSG];
	StringCbVPrintfW(msg, MAX_MSG * sizeof(wchar_t), fmt, args);
	OutputDebugStringW(msg);

	va_end(args);
}

inline void dprintfA(const char* fmt, ...)
{
	const size_t MAX_MSG = 255;

	va_list args;
	va_start(args, fmt);

	char msg[MAX_MSG];
	StringCbVPrintfA(msg, MAX_MSG, fmt, args);
	OutputDebugStringA(msg);

	va_end(args);
}

#ifdef _DEBUG
#define _TRACE dprintf(L"At %s line %d",__FUNCTIONW__,__LINE__)
#else
#define _TRACE
#endif

#endif // __DPRINTF_H__