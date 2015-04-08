/* Level.h : Declares the functions to access and manipulate the LEVELN & LEVELC tables of ICIS
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
 Modified  (AMP) :   
 **************************************************************/

#ifndef _LEVEL_H_
#define _LEVEL_H_

#include "icis32.h"
#include "codbc.h"
#include "misc.h"

LONG getCondLevel(BOOL fLocal, DMS_CONDLEVEL *data, int fOpt);
LONG searchLevelN(BOOL fLocal,LONG trait, LONG scale, LONG method, LONG &ounit, double *level, LONG fSearchOption);
LONG searchLevelC(BOOL fLocal,LONG trait, LONG scale, LONG method, LONG &ounit, CHAR *szlevel,LONG nszlevel, LONG fSearchOption);
LONG DLL_INTERFACE DMS_findLevelNo(long factorid, long levelno);

#endif
