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

/* gvcfile.h */

/* This is a quick and dirty implementation of CFile, providing
 * only those methods used by the non MFC GSview so we don't need 
 * to import all of MFC
 */

#ifndef _Windows
#ifndef UINT
#define UINT unsigned int
#endif
#ifndef BOOL
#define BOOL int
#endif
#ifndef LONG
#define LONG long
#endif
#ifndef DWORD
#define DWORD unsigned long
#endif
#endif
#ifndef LPCTSTR
#define LPCTSTR const char *
#ifndef GENERIC_READ
#define GENERIC_READ (0x80000000L)
#endif
#ifndef FILE_SHARE_READ
#define FILE_SHARE_READ 0x00000001
#endif
#endif

class CFile {
    public:
	int m_hFile;
	CFile(void);
	CFile(int hFile);

        enum OpenFlags {modeRead = 0x0000, modeWrite = 0x0001,
	    shareExclusive=0x0010, shareDenyWrite=0x0020, 
	    modeCreate=0x1000};
	BOOL Open(LPCTSTR lpszFileName, UINT nOpenFlags);
	void Close();	// no exceptions

	UINT Read(void *lpBuf, UINT nCount);		// no exceptions
	UINT CFile::Write(void *lpBuf, UINT nCount);	// no exceptions

        enum {begin, current, end};
	LONG Seek(LONG lOff, UINT nFrom);

	static void assert(char *file, int line);
};

#ifndef ASSERT
#ifdef DEBUG
#define ASSERT(f) if (!(f)) CFile::assert(__FILE__, __LINE__)
#else
#define ASSERT(f)
#endif
#endif

