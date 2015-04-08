/* Data.h : Declares the functions to access and manipulate the DATA table of ICIS
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
 Programmed by   :   Arllet M. Portugal, Warren Constantino
 Modified  (AMP) :   
 **************************************************************/

#ifndef _DATA_H_
#define _DATA_H_

#include "icis32.h"
#include "codbc.h"
#include "misc.h"

LONG searchDataN(BOOL fLocal,LONG trait, LONG scale, LONG method, LONG &ounit, double *data, LONG fSearchOption);
LONG searchDataC(BOOL fLocal,LONG trait, LONG scale, LONG method, LONG &ounit, CHAR *szdata,LONG nszdata, LONG fSearchOption);
LONG describeOunit(BOOL fLocal, LONG ounitid,  LONG *factorid, LONG *levelno, LONG fSearchOption);
LONG describeOunitRepres(BOOL fLocal, LONG repres, LONG ounitid,  LONG *factorid, LONG *levelno, LONG fSearchOption);
LONG DLL_INTERFACE DMS_findObsunit(long ounitid, long effectid);
LONG findObsunit(BOOL fLocal,long ounitid, long effectid);
LONG getConsData(BOOL fLocal, DMS_CONSDATA *data, int fOpt);


#endif
