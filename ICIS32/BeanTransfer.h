/* BeanTransfer.h : Declares the functions that transfer data from a C# data structure to C++ data structure
 *                    They are used for the interoperability of .NET and C++ for web service access
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

#ifndef _BEANTRANSFER_H
#define _BEANTRANSFER_H

#include "icis32.h"

#if defined(WEB)
void transferLocBean(LocationBean bn, GMS_Location *loc);
void assignLocBean(LocationBean *bn, GMS_Location loc);
#endif

#endif