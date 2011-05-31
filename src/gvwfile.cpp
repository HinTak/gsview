/* Copyright (C) 2000, Ghostgum Software Pty Ltd.  All rights reserved.
  
  This file is part of GSview.
  
  This program is distributed with NO WARRANTY OF ANY KIND.  No author
  or distributor accepts any responsibility for the consequences of using it,
  or for whether it serves any particular purpose or works at all, unless he
  or she says so in writing.  Refer to the GSview Free Public Licence 
  (the "Licence") for full details.
  
  Every copy of GSview must include a copy of the Licence, normally in a 
  plain ASCII text file named LICENCE.  The Licence grants you the right 
  to copy, modify and redistribute GSview, but only under certain conditions 
  described in the Licence.  Among other things, the Licence requires that 
  the copyright notice and this notice be preserved on all copies.
*/

/* gvwfile.cpp */

/* This is a quick and dirty implementation of CFile for Windows, 
 * providing only those methods used by the non MFC GSview so we 
 * don't need to import all of MFC
 */

#define STRICT
#include <windows.h>
#include <stdio.h>

#include "gvcfile.h"

CFile::CFile(void)
{
    m_hFile = 0;
}

CFile::CFile(int hFile)
{
   m_hFile = hFile;
}

BOOL CFile::Open(LPCTSTR lpszFileName, UINT nOpenFlags)
{
    ASSERT(m_hFile == 0);
    ASSERT(nOpenFlags == GENERIC_READ);
    DWORD dwAccess = GENERIC_READ;
    DWORD dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
    DWORD dwCreate = OPEN_EXISTING;
    if ((nOpenFlags & 0xf) == modeRead)
	dwAccess = GENERIC_READ;
    if ((nOpenFlags & 0xf) == modeWrite)
	dwAccess = GENERIC_WRITE;
    if ((nOpenFlags & 0xf0) == shareDenyWrite)
	dwShareMode = FILE_SHARE_READ;
    if ((nOpenFlags & 0xf0) == shareExclusive)
	dwShareMode = 0;
    if ((nOpenFlags & 0xf00) == modeCreate)
	dwCreate = CREATE_ALWAYS;
 
    m_hFile = (UINT)CreateFile(lpszFileName, dwAccess,
	dwShareMode, NULL, dwCreate, FILE_ATTRIBUTE_NORMAL,
	NULL);
    if ((HANDLE)m_hFile == INVALID_HANDLE_VALUE) {
	m_hFile = 0;
	return FALSE;
    }
    return TRUE;
}

void CFile::Close()
{
    ASSERT(m_hFile != 0);
    CloseHandle((HANDLE)m_hFile);
    m_hFile = 0;
}


UINT CFile::Read(void *lpBuf, UINT nCount)
{
    ASSERT(m_hFile != 0);
    DWORD nBytesRead;
    if (ReadFile((HANDLE)m_hFile, lpBuf, nCount, &nBytesRead, NULL))
	return nBytesRead;
    return 0;
}

UINT CFile::Write(void *lpBuf, UINT nCount)
{
    ASSERT(m_hFile != 0);
    DWORD nBytesWritten;
    if (WriteFile((HANDLE)m_hFile, lpBuf, nCount, &nBytesWritten, NULL))
	return nBytesWritten;
    return 0;
}

LONG CFile::Seek(LONG lOff, UINT nFrom)
{
    ASSERT(m_hFile != 0);
    DWORD dwMoveMethod;
    LONG lMoveHigh = 0;
    switch(nFrom) {
	default:
	case begin:
	    dwMoveMethod = FILE_BEGIN;
	    break;
	case current:
	    dwMoveMethod = FILE_CURRENT;
	    break;
	case end:
	    dwMoveMethod = FILE_END;
	    break;
    }
    // return value on error is 0xffffffff
    return SetFilePointer((HANDLE)m_hFile, lOff, &lMoveHigh, dwMoveMethod);
}

void CFile::assert(char *file, int line) 
{
  printf("Assert failed in file %s at line %d\n", file, line);
}
