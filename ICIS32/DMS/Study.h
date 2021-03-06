/* Study.h : Declares the functions to access and manipulate the STUDY table of ICIS
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

#ifndef _STUDY_H_
#define _STUDY_H_

#include "icis32.h"
#include "codbc.h"
#include "misc.h"

LONG findStudy(BOOL fLocal, CHAR *szName, DMS_STUDY &data, LONG fSearchOption);
LONG addStudy(DMS_STUDY &data, CHAR *szAval);
LONG  getStudy(BOOL fLocal ,DMS_STUDY* data, UCHAR* szMDesc, LONG cMDesc, int fOpt);

#endif
