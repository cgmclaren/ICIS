/* Germplsm.h : Declares the functions to access and manipulate the GERMPLSM table of ICIS
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

#ifndef _GERMPLSM_H_
#define _GERMPLSM_H_

#include <time.h>
#include <string>
#include "icis32.h"
#include "codbc.h"
#include "misc.h"
#include "ODBCTools.h"
#include "names.h"
#include "atributs.h"


LONG getGermplasmRecord(BOOL fLocal,LONG id, GMS_Germplasm &grmplsm);
//LONG getGermplasmRecord(BOOL fLocal,LONG gid, GMS_Germplasm* grmplsm);

LONG getGermplasm(BOOL fLocal,LONG id, GMS_Germplasm &germ);
LONG getGermplasm2(BOOL fLocal,LONG id,GMS_Germplasm &germ,GMS_Name &name
                   ,LPSTR szName,LONG nszName);
LONG getGermplasm3(BOOL fLocal,LONG id,GMS_Germplasm &germ,GMS_Name &name
                   ,LPSTR szName,LONG nszName);
LONG setGermplasm(BOOL fLocal,GMS_Germplasm &germ);
LONG addGermplasmRecord(BOOL fLocal, GMS_Germplasm &germ,LONG *progenitors);
LONG addGermplasm(BOOL fLocal, GMS_Germplasm &germ,GMS_Name &name,CHAR *szName,LONG *progenitors);
LONG deleteGermplasm(BOOL fLocal,LONG gid);


LONG applyGermplasmChanges(GMS_Germplasm &germ);
LONG findGermplasm(BOOL fLocal,GMS_Germplasm &germ,LONG fSearchOption);
LONG findDescendant(BOOL fLocal,LONG gid,GMS_Germplasm &germ,LONG &pno,LONG fSearchOption);

LONG addProgenitorID(BOOL fLocal, LONG gid, LONG pno, LONG pid);
LONG getProgenitorID(BOOL fLocal,LONG gid, LONG pno,LONG &pid);
LONG setProgenitorID(LONG gid,LONG pno,LONG pid);
LONG deleteProgenitorID(BOOL fLocal,LONG gid);
LONG deleteOneProgenitor(BOOL fLocal,LONG gid,LONG pid);

LONG getGermplasm2_2(BOOL fLocal,LONG gid,GMS_Germplasm &germ);
LONG GenerationNo(LONG gid, LONG *c2, LONG *c1, LONG *c3, LONG *c4, LONG *LDM, LONG *CRM);
LONG getBackcrossDosage(GMS_Germplasm g3);


#endif