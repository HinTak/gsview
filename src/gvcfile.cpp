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

/* gvcfile.cpp */

/* This is a quick and dirty implementation of CFile for OS/2 and Unix, 
 * providing only those methods used by the non MFC GSview so we 
 * don't need MFC.
 */

#include <stdio.h>
#include <fcntl.h>
#ifdef UNIX
#include <unistd.h>
#define O_BINARY (0)
#else
#include <io.h>
#endif

#include "gvcfile.h"

CFile::CFile(void)
{
    m_hFile = -1;
}

CFile::CFile(int hFile)
{
   m_hFile = hFile;	// OS handle
}

BOOL CFile::Open(LPCTSTR lpszFileName, UINT nOpenFlags)
{
    ASSERT(m_hFile == -1);
    ASSERT(nOpenFlags == GENERIC_READ);
    m_hFile = open(lpszFileName, O_BINARY | O_RDONLY);
    if (m_hFile == -1)
	return 0;
    return 1;
}

void CFile::Close()
{
    ASSERT(m_hFile != -1);
    close(m_hFile);
    m_hFile = -1;
}


UINT CFile::Read(void *lpBuf, UINT nCount)
{
    ASSERT(m_hFile != -1);
    DWORD nBytesRead;
    if ((nBytesRead = read(m_hFile, lpBuf, nCount)) != 0)
	return nBytesRead;
    return 0;
}

LONG CFile::Seek(LONG lOff, UINT nFrom)
{
    ASSERT(m_hFile != -1);
    DWORD dwMoveMethod;
    switch(nFrom) {
	default:
	case begin:
	    dwMoveMethod = SEEK_SET;
	    break;
	case current:
	    dwMoveMethod = SEEK_CUR;
	    break;
	case end:
	    dwMoveMethod = SEEK_END;
	    break;
    }
    // return value on error is -1
    return lseek(m_hFile, lOff, dwMoveMethod);
}

void CFile::assert(char *file, int line) 
{
/*
  char buf[256];
  printf("Assert failed in file %s at line %d\n", file, line);
*/
}
