/* Changes.h : Declares the functions to access and manipulate the CHANGES table of ICIS
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
 **************************************************************/

#ifndef _CHANGES_H_
#define _CHANGES_H_

#include "icis32.h"
#include "codbc.h"
#include "misc.h"

LONG addChanges(GMS_Changes &data, CHAR *szDesc);
LONG deleteChanges(GMS_Changes &data);
LONG getChanges(BOOL fLocal, LONG cid,GMS_Changes &data, CHAR *szDesc, LONG nszDesc);
LONG findChanges(BOOL fLocal,GMS_Changes &data, LPSTR szDesc, LONG nszDesc
               , LONG fSearchOption);
LONG findDeletedGermplasmFromChanges(LONG gid);
LONG getReplacementGermplasmFromChanges(LONG gid,LONG &to);
LONG getGermplasmFieldFromChanges(LONG gid,LPCSTR szField,LONG &to);
LONG getGermplasmFieldsFromChanges(LONG gid,LONG &cfrom,LONG &cto
               , LPSTR szField,LONG nszField,LONG &cdate,LONG &ctime,LONG fSearchOptions);
LONG getNameFieldFromChanges(LONG nid,LPCSTR szField,LONG &to);


#endif
