/*
 * VS.EXE archive numbers: the tables main opens with cd_archive_open
 * (src/vs/main.c). VS numbers its own; COM\STAT.T, COM\CRT.T and COM\ICON.T
 * are ST.EXE's files under other numbers.
 */
#ifndef VS_ARCHIVES_H
#define VS_ARCHIVES_H

#define VS_ARCHIVE_VS_CRT 0    /* VS\VS_CRT.T */
#define VS_ARCHIVE_VS_ETC 5    /* VS\VS_ETC.T */
#define VS_ARCHIVE_STAT 8      /* COM\STAT.T */
#define VS_ARCHIVE_CRT 10      /* COM\CRT.T */
#define VS_ARCHIVE_ICON 11     /* COM\ICON.T */

#endif
