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

/* gvwreg.cpp */

/* GSview Windows registration */

/* before changing this file, please see gvcreg.cpp */

#include "gvwin.h"

#define REG_KEY_NAME "Software\\Ghostgum\\GSview"
#define REGISTRATION_RECEIPT "Receipt"
#define REGISTRATION_NUMBER "Number"
#define REGISTRATION_NAME "Name"


/* display error message if reading or writing a registry key fails */
void registry_error(HKEY root, const char *name, const char *value, 
    BOOL bRead, LONG rc)
{
    LPVOID lpMessageBuffer;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
	FORMAT_MESSAGE_FROM_SYSTEM,
	NULL, rc,
	MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), /* user default language */
	(LPTSTR) &lpMessageBuffer, 0, NULL);
    if (lpMessageBuffer) {
	gs_addmess( bRead ? "Failed to read registry\n" :
	    "Failed to write registry\n");
	if (root == HKEY_CLASSES_ROOT)
	    gs_addmess("HKEY_CLASSES_ROOT\\");
	else if (root == HKEY_LOCAL_MACHINE)
	    gs_addmess("HKEY_LOCAL_MACHINE\\");
	else if (root == HKEY_CURRENT_USER)
	    gs_addmess("HKEY_CURRENT_USER\\");
	else
	    gs_addmess("???\\");
	gs_addmess(name);
	gs_addmess("\n");
	if (value)
	    gs_addmessf("  %s\n", value);
	gs_addmessf("Error=%d\n", rc);
	gs_addmess((LPTSTR)lpMessageBuffer);
	gs_addmess("\r\n");
	LocalFree(LocalHandle(lpMessageBuffer));
    }
    gs_showmess();
}

BOOL
write_registration(unsigned int reg_receipt, unsigned int reg_number,
  char *reg_name)
{
    LONG rc;
    HKEY hkey;
    DWORD dwValue;
    HKEY root;
    char *name;
    char *value;
   
    if (is_win32s) {
	char profile[MAXSTR];
	char *section = INISECTION;
	PROFILE *prf;
	prf = profile_open(szIniFile);
	profile_write_string(prf, section, "RegistrationName", reg_name);
	sprintf(profile, "%u", reg_receipt);
	profile_write_string(prf, section, "RegistrationReceipt", profile);
	sprintf(profile, "%u", (reg_number ^ 0xffff));
	profile_write_string(prf, section, "RegistrationNumber", profile);
	profile_close(prf);
    }
    else {
	root = HKEY_LOCAL_MACHINE;
	name = REG_KEY_NAME;
	value = NULL;
	if ((rc = RegOpenKeyEx(root, name, 0, 
		KEY_ALL_ACCESS, &hkey)) != ERROR_SUCCESS) {
	    /* failed to open key, so try to create it */
	    rc = RegCreateKey(root, name, &hkey);
	}

	if (rc == ERROR_SUCCESS) {
	    dwValue = (DWORD)reg_receipt;
	    value = REGISTRATION_RECEIPT;
	    rc = RegSetValueEx(hkey, value, 0, REG_DWORD,
			(CONST BYTE *)&dwValue, sizeof(DWORD));

	    dwValue = (DWORD)(reg_number ^ 0xffff);
	    if (rc == ERROR_SUCCESS) {
	        value = REGISTRATION_NUMBER;
		rc = RegSetValueEx(hkey, value, 0, REG_DWORD,
			(CONST BYTE *)&dwValue, sizeof(DWORD));
	    }

	    if (rc == ERROR_SUCCESS) {
	        value = REGISTRATION_NAME;
		rc = RegSetValueEx(hkey, value, 0, REG_SZ,
			(CONST BYTE *)reg_name, lstrlen(reg_name)+1);
	    }
	    RegCloseKey(hkey);
	}
	
	if (rc != ERROR_SUCCESS) {
	    registry_error(root, name, value, FALSE, rc);
	    return FALSE;
	}
    }
    return TRUE;
}


BOOL
read_registration(unsigned int *preg_receipt, unsigned int *preg_number,
  char *reg_name, int reg_len)
{
    LONG rc;
    HKEY hkey;
    DWORD dwValue;
    DWORD cbData;
    DWORD keytype;
    HKEY root;
    char *name;
    char *value;
   
    if (is_win32s) {
	unsigned int i;
	char profile[MAXSTR];
	char *section = INISECTION;
	PROFILE *prf;
	prf = profile_open(szIniFile);
	profile_read_string(prf, section, "RegistrationReceipt", "", 
		profile, sizeof(profile));
	if (sscanf(profile,"%u", &i) == 1)
	    *preg_receipt = i;
	profile_read_string(prf, section, "RegistrationNumber", "", 
		profile, sizeof(profile));
	if (sscanf(profile,"%u", &i) == 1)
	    *preg_number = i ^ 0xffff;
	profile_read_string(prf, section, "RegistrationName", "", 
		reg_name, reg_len);
	profile_close(prf);
    }
    else {
	root = HKEY_LOCAL_MACHINE;
	name = REG_KEY_NAME;
	value = NULL;
	rc = RegOpenKeyEx(root, name, 0, KEY_READ, &hkey);

	if (rc == ERROR_SUCCESS) {
	    cbData = sizeof(dwValue);
	    keytype =  REG_DWORD;
	    if (rc == ERROR_SUCCESS) {
		value = REGISTRATION_RECEIPT;
		if ((rc = RegQueryValueEx(hkey, value, 0, &keytype, 
		    (LPBYTE)&dwValue, &cbData)) == ERROR_SUCCESS)
		    *preg_receipt = dwValue;
	    }

	    cbData = sizeof(dwValue);
	    keytype =  REG_DWORD;
	    if (rc == ERROR_SUCCESS) {
		value = REGISTRATION_NUMBER;
		if ((rc = RegQueryValueEx(hkey, value, 0, &keytype, 
		    (LPBYTE)&dwValue, &cbData)) == ERROR_SUCCESS)
		    *preg_number = dwValue ^ 0xffff;
	    }

	    cbData = reg_len;
	    keytype =  REG_SZ;
	    if (rc == ERROR_SUCCESS) {
		value = REGISTRATION_NAME;
		rc = RegQueryValueEx(hkey, value, 0, &keytype, 
		    (LPBYTE)reg_name, &cbData);
	    }

	    RegCloseKey(hkey);
	    if ((rc != ERROR_SUCCESS) && (rc != ERROR_FILE_NOT_FOUND))
		registry_error(root, name, value, TRUE, rc);
	}
	else {
	    if (rc != ERROR_FILE_NOT_FOUND)
		registry_error(root, name, value, TRUE, rc);

	}
	
	if (rc != ERROR_SUCCESS)
	    return FALSE;
    }
    return TRUE;
}

#ifdef __BORLANDC__
#pragma argsused
#endif
BOOL CALLBACK _export
RegDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message) {
        case WM_INITDIALOG:
	    centre_dialog(hDlg);
            return TRUE;
        case WM_COMMAND:
            switch(LOWORD(wParam)) {
		case IDOK:
		    {int reg_num;
		    int reg_receipt;
		    reg_receipt = GetDlgItemInt(hDlg, REGDLG_RECEIPT,
			NULL, FALSE);
		    reg_num = GetDlgItemInt(hDlg, REGDLG_NUMBER, NULL, FALSE);
		    char buf[MAXSTR];
		    GetDlgItemText(hDlg, REGDLG_NAME, buf, sizeof(buf));
		    if ((reg_receipt != 0) && 
			((unsigned int)reg_num == make_reg(reg_receipt)) &&
			(strlen(buf) > 0)) {
			strncpy(registration_name, buf, 
			    sizeof(registration_name));
			registration_receipt = reg_receipt;
			write_registration(registration_receipt,
			    reg_num, registration_name);
			EndDialog(hDlg, IDOK);
		    }
		    else {
			char buf[MAXSTR];
			load_string(IDS_INVALIDREG, buf, sizeof(buf)-1);
			MessageBox(hDlg, buf, szAppName, 
				MB_ICONEXCLAMATION | MB_OK);
		    }
		    }
                    return(TRUE);
                case IDCANCEL:
                    EndDialog(hDlg, IDCANCEL);
                    return(TRUE);
                case REGDLG_ONLINE:
		    ShellExecute(hDlg, NULL, 
			"http://www.ghostgum.com.au/index.htm", 
			NULL, NULL, SW_SHOWNORMAL);
                    return(TRUE);
		case ID_HELP:
		    get_help();
		    return(FALSE);
                default:
                    return(FALSE);
            }
        default:
            return(FALSE);
    }
}
#ifdef __BORLANDC__
#pragma argsused
#endif
BOOL CALLBACK _export
NagDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message) {
        case WM_INITDIALOG:
            SetDlgItemText(hDlg, NAG_NAME, registration_name);
	    if (registration_receipt != 0) {
		char buf[16];
		sprintf(buf, "%u", registration_receipt);
		SetDlgItemText(hDlg, NAG_RECEIPT, buf);
	    }
	    centre_dialog(hDlg);
            return TRUE;
        case WM_COMMAND:
            switch(LOWORD(wParam)) {
		case IDOK:
                    EndDialog(hDlg, IDOK);
                    return(TRUE);
                case NAG_REGISTER:
                    EndDialog(hDlg, NAG_REGISTER);
                    return(TRUE);
		case ID_HELP:
		    get_help();
		    return(FALSE);
                default:
                    return(FALSE);
            }
        default:
            return(FALSE);
    }
}


/* show registation nag screen */
BOOL registration_nag(void)
{
    nHelpTopic = IDS_TOPICREG;
    if (DialogBoxParam(hlanguage, "NagDlgBox", hwndimg, 
	NagDlgProc, (LPARAM)NULL) == NAG_REGISTER) {
	if (DialogBoxParam(hlanguage, "RegDlgBox", hwndimg, 
	    RegDlgProc, (LPARAM)NULL) == IDOK)
	    return TRUE;
    }
    return FALSE;
}
