/* Copyright (C) 1993, 1994, 1995, Russell Lang.  All rights reserved.
  
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

/* gvcrc.h */
/* Common Resource header file */

#define GSVIEW_VERSION "1995-05-23  1.3"
#define EMX_NEEDED "0.9a"

#define ID_GSVIEW 42

#define IDD_ABOUT 	50
#define IDD_INPUT 	51
#define ID_ANSWER	52
#define ID_PROMPT	53
#define ID_HELP		54

#define IDM_FILEMENU	100
#define IDM_OPEN	101
#define IDM_SELECT	102
#define IDM_SAVEAS	103
#define IDM_EXTRACT	104
#define IDM_PSTOEPS	105
#define IDM_CLOSE	106
#define IDM_INFO	107
#define IDM_PRINT	110
#define IDM_PRINTTOFILE 111
#define IDM_SPOOL	112
#define IDM_EXIT	113
#define IDM_DROP	114

#define IDM_EDITMENU	150
#define IDM_COPYCLIP	151
#define IDM_PASTETO	152
#define IDM_CONVERT	153
#define IDM_ADDEPSMENU	154
#define IDM_MAKEEPSI	155
#define IDM_MAKEEPST4	156
#define IDM_MAKEEPST	157
#define IDM_MAKEEPSW	158
#define IDM_EXTEPSMENU	159
#define IDM_EXTRACTPS	160
#define IDM_EXTRACTPRE	161
#define IDM_TEXTEXTRACT	162
#define IDM_TEXTFIND	163
#define IDM_TEXTFINDNEXT 164

#define IDM_OPTIONMENU	  174
#define IDM_GSCOMMAND	  175
#define IDM_SOUNDS	  176
#define IDM_SETTINGS	  177
#define IDM_SAVESETTINGS  178
#define IDM_SAFER         179  
#define IDM_SAVEDIR	  180
#define IDM_BUTTONSHOW	  181
#define IDM_FITPAGE	  182
#define IDM_QUICK	  183
#define IDM_AUTOREDISPLAY 184
#define IDM_EPSFCLIP	  185
#define IDM_EPSFWARN	  186
#define IDM_IGNOREDSC	  187
#define IDM_SHOWBBOX      188

#define IDM_UNITMENU	190
#define IDM_UNITPT	191
#define IDM_UNITMM	192
#define IDM_UNITINCH	193

#define IDM_GSVERMENU	195
#define IDM_GS261	196
#define IDM_GS3		197

#define IDM_VIEWMENU    200
#define IDM_NEXT	201
#define IDM_NEXTSKIP	202
#define IDM_PREV	203
#define IDM_PREVSKIP	204
#define IDM_GOTO	205
#define IDM_REDISPLAY   206
#define IDM_SKIP	207

#define IDM_ORIENTMENU	220
#define IDM_PORTRAIT	221
#define IDM_LANDSCAPE   222
#define IDM_UPSIDEDOWN  223
#define IDM_SEASCAPE	224
#define IDM_SWAPLANDSCAPE 225

#define IDM_MEDIAMENU	250
#define IDM_RESOLUTION  251
#define IDM_ZOOM	252
#define IDM_ZOOMRES	253
#define IDM_MAGPLUS	254
#define IDM_MAGMINUS	255

#define IDM_DEPTHMENU	260
#define IDM_DEPTHDEF	261
#define IDM_DEPTH1	262
#define IDM_DEPTH4	263
#define IDM_DEPTH8	264
#define IDM_DEPTH16	265
#define IDM_DEPTH24	266

#define IDM_LETTER	301
#define IDM_LETTERSMALL	302
#define IDM_TABLOID	303
#define IDM_LEDGER	304
#define IDM_LEGAL	305
#define IDM_STATEMENT	306
#define IDM_EXECUTIVE	307
#define IDM_A3		308
#define IDM_A4		309
#define IDM_A4SMALL	310
#define IDM_A5		311
#define IDM_B4		312
#define IDM_B5		313
#define IDM_FOLIO	314
#define IDM_QUARTO	315
#define IDM_10X14	316
#define IDM_USERSIZE	317
#define IDM_MEDIALAST	318

#define IDM_HELPMENU	350
#define IDM_HELPCONTENT 351
#define IDM_HELPSEARCH	352
#define IDM_HELPKEYS	353
#define IDM_ABOUT	354
#define IDM_MISC	355

/* info dialog box */
#define IDD_INFO	400
#define INFO_FILE	401
#define INFO_TYPE	402
#define INFO_TITLE	403
#define INFO_DATE	404
#define INFO_BBOX	405
#define INFO_ORIENT	406
#define INFO_ORDER	407
#define INFO_DEFMEDIA	408
#define INFO_NUMPAGES	409
#define INFO_PAGE	410
#define INFO_BITMAP	411
#define INFO_ICON	412

#define ABOUT_ICON	451
#define ABOUT_VERSION	452

#define IDD_SOUND	500
#define SOUND_EVENT	501
#define SOUND_FILE	502
#define SOUND_PATH	503
#define SOUND_TEST	504

#define IDD_SPOOL	524
#define SPOOL_PORT	525

#define CANCEL_PCDONE	541

#define IDD_PAGE	550
#define IDD_MULTIPAGE	551
#define PAGE_LIST	552
#define PAGE_ALL	553
#define PAGE_ODD	554
#define PAGE_EVEN	555

#define IDD_DEVICE	560
#define DEVICE_NAME	561
#define DEVICE_RES	562
#define DEVICE_RESTEXT	563
#define DEVICE_PROP	564

#define IDD_PROP	570
#define PROP_NAME	571
#define PROP_VALUE	572

#define IDD_BBOX	590
#define BB_PROMPT	591
#define BB_CLICK	592

/* file filters */
#define FILTER_PSALL	0
#define FILTER_PS	1
#define FILTER_EPS	2
#define FILTER_EPI	3
#define FILTER_ALL	4
#define FILTER_BMP	5
#define FILTER_TIFF	6
#define FILTER_WMF	7
#define FILTER_TXT	8

/* string constants */
#define IDS_FILTER	601
#define IDS_TITLE	602
#define IDS_HELPFILE	603
#define IDS_WRONGGS	604
#define IDS_BUSY	605
#define IDS_FILENOTFOUND 606
#define IDS_PRINTBUSY	607

#define IDS_FILE	610
#define IDS_NOFILE	611
#define IDS_PAGE	612
#define IDS_NOPAGE	613
#define IDS_LANDSCAPE	614
#define IDS_PORTRAIT	615
#define IDS_ASCEND	616
#define IDS_DESCEND	617
#define IDS_SPECIAL	618
#define IDS_EPSF	619
#define IDS_EPSI	620
#define IDS_EPST	621
#define IDS_EPSW	622
#define IDS_DSC		623
#define IDS_NOTDSC	624
#define IDS_PDF         625
#define IDS_IGNOREDSC   626
#define IDS_PAGEINFO	627

#define IDS_OUTPUTFILE	630
#define IDS_PRINTINGALL	631
#define IDS_PRINTFILE	632
#define IDS_NOSPOOL	633
#define IDS_SELECTPAGE	634
#define IDS_SELECTPAGES	635
#define IDS_TIMEOUT	636
#define IDS_NOTIMER	637
#define IDS_NOTOPEN	638
#define IDS_CANNOTRUN	639
#define IDS_TOOLONG	640
#define IDS_NOMORE	642
#define IDS_GSCOMMAND	643
#define IDS_RES		644
#define IDS_ZOOMRES	645
#define IDS_NOZOOM	646
#define	IDS_USERWIDTH	647
#define	IDS_USERHEIGHT	648
#define IDS_BADEPS	649
#define IDS_NOPREVIEW	650
#define IDS_NOTDFNAME   651
#define IDS_PIPE_EOPEN	652
#define IDS_PIPE_EMEM	653
#define IDS_CANCELDONE	654
#define IDS_BADCLI      655
#define IDS_TEXTFIND	656
#define IDS_TEXTNOTFIND	657

#define IDS_SOUNDNAME	670
#define IDS_SNDPAGE	671
#define IDS_SNDNOPAGE	672
#define IDS_SNDNONUMBER 673
#define IDS_SNDNOTOPEN	674
#define IDS_SNDERROR	675
#define IDS_SNDTIMEOUT	676
#define IDS_SNDSTART	677
#define IDS_SNDEXIT	678
#define IDS_SOUNDNOMM	679
#define IDS_NONE	680
#define IDS_SPKR	681

/* help topics */
#define IDS_TOPICROOT	701
#define IDS_TOPICOPEN	702
#define IDS_TOPICPRINT	703
#define IDS_TOPICEDIT	704
#define IDS_TOPICGSCMD	705
#define IDS_TOPICSOUND	706
#define IDS_TOPICMEDIA  707
#define IDS_TOPICPSTOEPS 708
#define	IDS_TOPICGOTO    709
#define IDS_TOPICINSTALL 710
#define IDS_TOPICTEXT    711
#define IDS_TOPICKEYS    712

/* ps_to_eps */
#define IDS_BBPROMPT	750
#define IDS_BBPROMPT1	751
#define IDS_BBPROMPT2	752
#define IDS_BBPROMPT3	753
#define IDS_EPSONEPAGE	754
#define IDS_EPSQPAGES	755
#define IDS_EPSNOBBOX	756
#define IDS_EPSREAD     757

/* wait messages */
#define IDS_WAIT	770
#define IDS_WAITREAD	771
#define IDS_WAITWRITE	772
#define IDS_WAITDRAW	773
#define IDS_WAITGSOPEN	774
#define IDS_WAITGSCLOSE	775
#define IDS_WAITPRINT	776
#define IDS_WAITSEARCH	777

/* filter strings */
#define IDS_FILTER_BASE	800
#define IDS_FILTER_PSALL IDS_FILTER_BASE+FILTER_PSALL
#define IDS_FILTER_PS	IDS_FILTER_BASE+FILTER_PS
#define IDS_FILTER_EPS	IDS_FILTER_BASE+FILTER_EPS
#define IDS_FILTER_EPI	IDS_FILTER_BASE+FILTER_EPI
#define IDS_FILTER_ALL	IDS_FILTER_BASE+FILTER_ALL
#define IDS_FILTER_BMP	IDS_FILTER_BASE+FILTER_BMP
#define IDS_FILTER_TIFF	IDS_FILTER_BASE+FILTER_TIFF
#define IDS_FILTER_WMF	IDS_FILTER_BASE+FILTER_WMF
#define IDS_FILTER_TXT	IDS_FILTER_BASE+FILTER_TXT


/* RCDATA resources */
#define IDR_ORIENT	900
#define IDR_ORIENT3	901
#define IDR_EPSFWARN	903
#define IDR_DEVICES	904
#define IDR_PORTS	905
#define IDR_BUTTON	906

/* cursors */
#define IDP_CROSSHAIR  910
