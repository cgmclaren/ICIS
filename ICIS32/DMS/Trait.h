/* Trait.h : Declares the functions to access and manipulate the TRAIT,SCALE,TMETHOD tables of ICIS
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
 Programmed by   :   Arllet M. Portugal
 Modified  (AMP) :   November 4, 2003
 **************************************************************/


#ifndef _TRAIT_H_
#define _TRAIT_H_

#include "icis32.h"
#include "codbc.h"
#include "misc.h"
#include <stdio.h>

LONG executeScaleTab(DMS_SCALETAB scaletab,CHAR *fname, LONG *noCol);
LONG executeScaleTab_exclam(DMS_SCALETAB scaletab,CHAR *fname, LONG *noCol);
LONG executeScaleTab2(DMS_SCALETAB scaletab,CHAR *value, LONG nVal);
LONG executeScaleTab3(DMS_SCALETAB scaletab,CHAR *value, LONG nVal, long param1, LONG fopt);
LONG getScale(BOOL fLocal,DMS_SCALE* Scales, long fOpt);
LONG getTmethod(BOOL fLocal,DMS_TMETHOD* Tmethods, UCHAR* szMDesc, LONG cMDesc, int fOpt);
LONG getTrait_v4(BOOL fLocal,DMS_TRAIT* Traits, UCHAR* szMDesc, LONG cMDesc, int fOpt);



#endif