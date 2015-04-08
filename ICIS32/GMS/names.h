/* Names.h : Declares the functions to access and manipulate the NAMES table of ICIS
 * Copyright (C) 2005 IRRI
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA.
 */

/**************************************************************
 Programmed by   :   Arllet M. Portugal, Olan Casumpang
 Modified  (AMP) :   11/18/2003   {setName is added}
 **************************************************************/

#ifndef _NAMES_H_
#define _NAMES_H_

#include "icis32.h"
#include "codbc.h"
#include "ODBCTools.h"
#include "misc.h"
#include "germplsm.h"
#include "atributs.h"

typedef LONG (*TGETNAMEPTR)(BOOL,GMS_Name &,LPSTR ,LONG ,LONG );
typedef LONG (*TFINDNAMEPTR)(BOOL,CHAR*,GMS_Name&,GMS_Germplasm&,CHAR*,LONG,LONG);


LONG addName(BOOL fLocal, GMS_Name &name,CHAR *szName,LONG stat);
LONG getPrefName(BOOL fLocal,GMS_Name &name,LPSTR szName,LONG nszName,LONG fSearchOption);
LONG getNameEx(BOOL fLocal,GMS_Name &name,LPSTR szName,LONG nszName,LONG fSearchOption);
LONG setPreferredName(BOOL fLocal,GMS_Name &name,CHAR *szName);
LONG setPreferredAbbr(BOOL fLocal,GMS_Name &name,CHAR *szName);
//LONG setPreferred(BOOL fLocal,GMS_Name &name,CHAR *szName,LONG nstat);

LONG findName(BOOL fLocal,CHAR *szSearchName, GMS_Name &name, GMS_Germplasm &germ
               , CHAR *szName, LONG nszName, LONG fSearchOption);
LONG findName2(BOOL fLocal,CHAR *szSearchName, GMS_Name &name, GMS_Germplasm &germ
               , CHAR *szName, LONG nszName, LONG fSearchOption);
LONG findName3(BOOL fLocal,CHAR *szSearchName, GMS_Name &name, GMS_Germplasm &germ
               , CHAR *szName, LONG nszName, LONG fSearchOption);

LONG listNames(LONG gid,CHAR* szBuffer, LONG nszBuffer);
LONG listNamesEx(LONG gid,CHAR* szBuffer, LONG nszBuffer,LPCSTR szDelimeter);
LONG standardName(CHAR* szInput, CHAR* szOutput,LONG nszOutput);

LONG getNameEx2(BOOL fLocal,GMS_Name &name, LPSTR szName, LONG nszName
               , LONG fSearchOption);

LONG SoundEx(CHAR* szInput, CHAR* szOutput,LONG nszOutput);

#endif