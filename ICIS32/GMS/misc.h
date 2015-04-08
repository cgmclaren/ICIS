/* misc.h : Declares the utility functions used by other functions of the ICIS32.DLL
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

#ifndef _MISC_H_
#define _MISC_H_

#include "icis32.h"
#include "codbc.h"
#include "odbctools.h"

#include <time.h>
#include <sys/timeb.h>
#include <sys/types.h>

#include <map>
#include <vector>
#include <string>

typedef vector<string> stringarray;
typedef vector<string>::iterator istringarray;
typedef map<string,string> dictionary;
typedef map<string,string>::iterator idictionary;
typedef vector<LONG> LONGarray;

   class CGermplasmTree{
   private:
      CGermplasmTree *FChild;
      int FLevel;
      string FName;
      GMS_Germplasm FGerm;
      vector <CGermplasmTree *> FParent; 
      BOOL FLandrace;
   public:
      CGermplasmTree(CGermplasmTree *descendant,LONG gid);
      ~CGermplasmTree();

      LONG Expand(LONG gid,int level);
      BOOL Derivative(){return FGerm.gnpgs < 0;}
      CGermplasmTree *Group(); 
      GMS_Germplasm &Data(){return FGerm;}
      CGermplasmTree *Parent(int index);
      CGermplasmTree *Child(){return FChild;}
      LPCSTR Name(){return (LPCSTR)FName.c_str();}
      LONG Level(){return FLevel;}
      BOOL Landrace(){return FLandrace;}
   };

LONG crossExpansion(LONG gid, LONG level,LONG ntype,CHAR* szBuffer,LONG bufsize,LONG *idList,LONG idLstSz);
void replaceString(string szInput, string* szOutput,string szfind, string szReplace, long nszLength);
void spellAbbr(string* szInput, string szAbbr, string szLongName, long nszLength);
void DellBlank(string* szInput) ;

CHAR* Trim(CHAR* str);
CHAR* PadRight(CHAR *str, int size);
void PadNull(CHAR *str, int size) ;
BOOL Changed(BOOL &ol,BOOL ne);
void UpperCase(CHAR *str, int size) ;

#endif