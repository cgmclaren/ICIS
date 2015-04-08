/* Lot.h : Declares the functions to access and manipulate the IMS_LOT,IMS_TRANSACTION tables of ICIS
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
  Programmed by   :   Arllet M. Portugal, Beth Sison
 Modified  (AMP) :  Jan 20, 2004 (LOT and ITRANSACTION are renamed to IMS_LOT, IMS_TRANSACTION 
                 :  March 11,2005 (PERSONID is added in ITRANSACTION)
 **************************************************************/

#ifndef _LOT_H_
#define _LOT_H_

#include "icis32.h"
#include "codbc.h"
#include "misc.h"

LONG findLocation(BOOL fLocal, IMS_Location &location, LONG fSearchOption);
LONG addLocation(IMS_Location &location);
LONG getIMSLocation3(BOOL fLocal, IMS_Location &data, LONG fSearchOption);

#endif