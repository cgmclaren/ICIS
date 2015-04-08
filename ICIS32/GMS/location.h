/* Location.h : Declares the functions to access and manipulate the LOCATION table of ICIS
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

#ifndef _LOCATION_H_
#define _LOCATION_H_

#include "icis32.h"
#include "codbc.h"
#include "misc.h"
#include "ODBCTools.h"
#include "BeanTransfer.h"

LONG getLocation(BOOL fLocal,GMS_Location &data, LONG fSearchOption);
LONG getLocation2(BOOL fLocal,GMS_Location &data, LONG fSearchOption);
LONG getLocation3(BOOL fLocal, GMS_Location &data, LONG fSearchOption);
LONG findLocation(BOOL fLocal, GMS_Location &location, LONG fSearchOption);
LONG addLocation(GMS_Location &location);


#endif
