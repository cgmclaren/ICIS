/* INSTLN.h : Declares the functions to access and manipulate the INSTLN table of ICIS
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

#ifndef _INSTALN_H_
#define _INSTALN_H_

#include "icis32.h"
#include "codbc.h"
#include "misc.h"
#include "encrypt.h"

LONG getInstallation(BOOL fLocal,GMS_Installation &instln,LPSTR szDesc,LONG nszDesc);
LONG getInstallationEx(BOOL fLocal,GMS_Installation &instln,LPSTR szDesc,LONG nszDesc,LONG fSearchOption);

#endif