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

/* gvxres.cpp */
/* Act like we have Windows/OS/2 resources, but really use const data */

#include "gvx.h"
#include "gvxlang.h"

#ifdef STANDALONE
#define DEBUG
#define LANGUAGE IDM_LANGEN
#else
#define LANGUAGE option.language
#endif

/* Find string from id using binary search */
int find_string(int id, STRING_ENTRY *st, int stlen)
{
    int i, step;
    i = stlen;
    step = 1;
    while (i) {
	step <<= 1;
	i >>= 1;
    }
    step >>= 1;
    i = stlen / 2;

    while (step) {
	step >>= 1;
	if (st[i].id == id)
	    return i;
	else if (st[i].id < id) {
	    i += step;
	    if (i >= stlen)
		i = stlen-1;
	}
	else {
	    i -= step;
	    if (i < 0)
		i = 0;
	}
    }
    return -1;
}

const char *get_string(int id)
{
    int i;
    STRING_ENTRY *st;
    int stlen;

    switch (LANGUAGE) {
	case IDM_LANGDE:
	    st = string_de;
	    stlen = string_de_len;
	    break;
	case IDM_LANGFR:
	    st = string_fr;
	    stlen = string_fr_len;
	    break;
	case IDM_LANGES:
	    st = string_es;
	    stlen = string_es_len;
	    break;
	case IDM_LANGIT:
	    st = string_it;
	    stlen = string_it_len;
	    break;
	case IDM_LANGEN:
	default:
	    st = string_en;
	    stlen = string_en_len;
    }

    i = find_string(id, st, stlen);
    if (i<0) {
	g_print("STRING ID %d not found\n", id);
	return NULL;
    }
    return st[i].str;
}

int load_string(int id, char *str, int len)
{
    const char *p = get_string(id);
    int count;
    if (p == NULL)
	return 0;
    count = strlen(p);
    if (count >= len) {
	count = len-1;
	str[count] = '\0';
    }
    strncpy(str, p, count+1);
    return count;
}

/* If debugging, call this during initialisation to make sure 
 * that the string table is not corrupted.
 */
void check_string_order(STRING_ENTRY *st, int stlen)
{
    int i, prev;
    prev = 0;
    for (i=0; i<stlen; i++) {
	if (st[i].id <= prev)
	    gs_addmessf("string table is not in order: %d=%s\n", 
		st[i].id, st[i].str);
	prev = st[i].id;	
    }
}

#ifdef STANDALONE
int main(int argc, char *argv[])
{
    check_string_order(string_en, length_en);

    printf("IDS_FILE=%s\n", get_string(IDS_FILE));
    return 0;
}
#endif
