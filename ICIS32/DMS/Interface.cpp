/* Interface.cpp : Implements the functions to access and manipulate the USERINTERFACE, UITAB tables of ICIS
 *                 to dynamically create a GUI
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
 Programmed by   :   Arllet M. Portugal, May Ann Sallan
 Date Created:  November 4, 2003
 Modified  (AMP) :   
 **************************************************************/

#include "Interface.h"

extern LONG UACCESS,USERID;
extern CODBCdbc *_centralDMS,*_localDMS;  //Central and Local Database Connection
extern stack<char*> _routines_stack;



////////////////////////////////////////////////////////////////
// UI_getUserInterface
// Created:  November 17, 2003
// Input:  User interface ID (UIID), UI class
///////////////////////////////////////////////////////////////
LONG getUserInterface( UI_CLASS *uiclass, LONG uiid, LONG fopt )
{
#define SQL_getUserInterface "\
        SELECT USERINTERFACE.UITABID as UITABID,  \
        UITAB.UICLASS as UICLASS,     \
        Count(USERINTERFACE.UITABID) AS NOITEMS \
        FROM USERINTERFACE, UITAB  \
        WHERE USERINTERFACE.UITABID = UITAB.UITABID AND UITAB.UICLASS<> ? \
        AND USERINTERFACE.UIID=? \
        GROUP BY USERINTERFACE.UITABID,UITAB.UICLASS  \
        ORDER BY USERINTERFACE.UITABID "

  PUSH(getUserInterface);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_getUserInterface);

static LONG _uiid;
static UI_CLASS _uiclass;
static BOOL first_time=TRUE;

       _uiclass = *uiclass;
	   _uiid = uiid;
	if (fopt == FIND_FIRST) {
      if (first_time)
      {
         local.BindParam(1, _uiclass.UICLASS,UI_CLASSNAME);  
         local.BindParam(2,_uiid);
		 local.Bind(1, _uiclass.TABID);
		 local.Bind(2, _uiclass.UICLASS,UI_CLASSNAME);
		 local.Bind(3, _uiclass.NO_ITEMS);

         first_time=FALSE;
      }

      local.Execute();
	}
   if (local.Fetch()) {
       *uiclass = _uiclass;
   }
   else if (local.NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;

   POP();

#undef SQL_getUserInterface

}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE UI_getUserInterface(UI_CLASS *uiclass, LONG uiid, LONG fopt)
{
   PUSH(UI_getUserInterface);
   strcpy((CHAR *) uiclass->UICLASS, "HEADER");
   ret=getUserInterface(uiclass, uiid, fopt);
   POP();
}



/** Original
////////////////////////////////////////////////////////////////
// UI_getUIHeader
// Created:  November 4, 2003
// Input:  User interface ID (UIID), UI class = HEADER
///////////////////////////////////////////////////////////////
LONG getUIHeader( UI_INTERFACE *data, LONG fOpt )
{
#define SQL_getUIHeader "\
	SELECT USERINTERFACE.UIFID as UIFID, \
     USERINTERFACE.UITABID as UITABID, \
     USERINTERFACE.UILWIDTH as UILWIDTH, \
     USERINTERFACE.UIWIDTH as UIWIDTH, \
     USERINTERFACE.UI_Y as UI_Y, \
     USERINTERFACE.UI_X as UI_X, \
     USERINTERFACE.FLDTYPE as FLDTYPE, \
     USERINTERFACE.STUDYID as STUDYID, \
     USERINTERFACE.FLDDEF as FLDDEF, \
     USERINTERFACE.UIACCESS as UIACCESS,  \
	 USERINTERFACE.UIID as UIID \
	FROM USERINTERFACE,UITAB WHERE UIID=? \
	  AND USERINTERFACE.UITABID=UITAB.UITABID AND \
	  UITAB.UICLASS='HEADER' \
	ORDER BY USERINTERFACE.UITABID, \
	  USERINTERFACE.UI_Y, USERINTERFACE.UI_X "



  PUSH(SQL_getUIHeader);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_getUIHeader);

static LONG _uiid;
static UI_INTERFACE _data;
static BOOL first_time=TRUE;

       _uiid = data->UIID;
	if (fOpt == FIND_FIRST) {
      if (first_time)
      {
         local.BindParam(1, _uiid);  
         local.Bind(1, _data.UIFID);  
         local.Bind(2,_data.UITABID);
         local.Bind(3,_data.UILWIDTH);
         local.Bind(4,_data.UIWIDTH);
         local.Bind(5,_data.UI_Y);
         local.Bind(6,_data.UI_X);
         local.Bind(7,_data.FLDTYPE,4);
         local.Bind(8,_data.STUDYID);
         local.Bind(9,_data.FLDDEF);
         local.Bind(10,_data.UIACCESS,4);
         local.Bind(11,_data.UIID);

         first_time=FALSE;
      }

      local.Execute();
	}
   if (local.Fetch()) {
       *data = _data;
   }
   else if (local.NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;

   POP();

#undef SQL_getUIHeader

}

****/



////////////////////////////////////////////////////////////////
// UI_getUIHeader
// Created:  November 4, 2003
// Input:  User interface ID (UIID), UI class = HEADER
///////////////////////////////////////////////////////////////
LONG getUIHeader( UI_INTERFACE *data, LONG fOpt )
{
#define SQL_getUIHeader "\
	SELECT USERINTERFACE.UIFID as UIFID, \
     USERINTERFACE.UITABID as UITABID, \
     USERINTERFACE.UILWIDTH as UILWIDTH, \
     USERINTERFACE.UIWIDTH as UIWIDTH, \
     USERINTERFACE.UI_Y as UI_Y, \
     USERINTERFACE.UI_X as UI_X, \
     USERINTERFACE.FLDTYPE as FLDTYPE, \
     USERINTERFACE.FLDDEF as FLDDEF, \
     USERINTERFACE.UIACCESS as UIACCESS,  \
	 USERINTERFACE.UIID as UIID \
	FROM USERINTERFACE,UITAB WHERE UIID=? \
	  AND USERINTERFACE.UITABID=UITAB.UITABID AND \
	  UITAB.UICLASS='HEADER' \
	ORDER BY USERINTERFACE.UITABID, \
	  USERINTERFACE.UI_Y, USERINTERFACE.UI_X "



  PUSH(SQL_getUIHeader);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_getUIHeader);

static LONG _uiid;
static UI_INTERFACE _data;
static BOOL first_time=TRUE;

       _uiid = data->UIID;
	if (fOpt == FIND_FIRST) {
      if (first_time)
      {
         local.BindParam(1, _uiid);  
         local.Bind(1, _data.UIFID);  
         local.Bind(2,_data.UITABID);
         local.Bind(3,_data.UILWIDTH);
         local.Bind(4,_data.UIWIDTH);
         local.Bind(5,_data.UI_Y);
         local.Bind(6,_data.UI_X);
         local.Bind(7,_data.FLDTYPE,4);
         local.Bind(8,_data.FLDDEF, DMS_VARIATE_NAME);
         local.Bind(9,_data.UIACCESS,4);
         local.Bind(10,_data.UIID);

         first_time=FALSE;
      }

      local.Execute();
	}
   if (local.Fetch()) {
       *data = _data;
   }
   else if (local.NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;

   POP();

#undef SQL_getUIHeader

}



//////////////////////////////////////////////////////////////////////////////
// Original UI_getUIHeader
// FLDDEF in the USERINTERFACE is the ID in VARIATE, FACTOR or UDLFDS
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE UI_getUIHeader( UI_INTERFACE *data,  LONG fOpt)
{
   PUSH(UI_getUIHeader);
   ret=getUIHeader( data, fOpt );
   POP();
}




/*****
////////////////////////////////////////////////////////////////
// UI_getUIField
// Created:  November 4, 2003
// Input:  User interface ID (UIID), UITABID
///////////////////////////////////////////////////////////////
LONG getUIField( UI_INTERFACE *data, LONG fOpt )
{
#define SQL_getUIField "\
	SELECT USERINTERFACE.UIFID as UIFID, \
     USERINTERFACE.UITABID as UITABID, \
     USERINTERFACE.UILWIDTH as UILWIDTH, \
     USERINTERFACE.UIWIDTH as UIWIDTH, \
     USERINTERFACE.UI_Y as UI_Y, \
     USERINTERFACE.UI_X as UI_X, \
     USERINTERFACE.FLDTYPE as FLDTYPE, \
     USERINTERFACE.STUDYID as STUDYID, \
     USERINTERFACE.FLDDEF as FLDDEF, \
     USERINTERFACE.UIACCESS as UIACCESS,  \
	 USERINTERFACE.UIID as UIID \
	FROM USERINTERFACE,UITAB \
    WHERE USERINTERFACE.UIID = ? AND USERINTERFACE.UITABID=UITAB.UITABID AND  \
	      UITAB.UICLASS<>'HEADER' AND UITAB.UITABID=?   \
    ORDER BY USERINTERFACE.UI_Y, USERINTERFACE.UI_X "


  PUSH(SQL_getUIField);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_getUIField);

static LONG _uiid, _uitabid;
static UI_INTERFACE _data;
static BOOL first_time=TRUE;

       _uiid = data->UIID;
	   _uitabid = data->UITABID;
	if (fOpt == FIND_FIRST) {
      if (first_time)
      {
         local.BindParam(1, _uiid);  
         local.BindParam(2, _uitabid);  
         local.Bind(1, _data.UIFID);  
         local.Bind(2,_data.UITABID);
         local.Bind(3,_data.UILWIDTH);
         local.Bind(4,_data.UIWIDTH);
         local.Bind(5,_data.UI_Y);
         local.Bind(6,_data.UI_X);
         local.Bind(7,_data.FLDTYPE,4);
         local.Bind(8,_data.STUDYID);
         local.Bind(9,_data.FLDDEF);
         local.Bind(10,_data.UIACCESS,4);
         local.Bind(11,_data.UIID);

         first_time=FALSE;
      }

      local.Execute();
	}
   if (local.Fetch()) {
       *data = _data;
   }
   else if (local.NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;

   POP();

#undef SQL_getUIField

}

****/

////////////////////////////////////////////////////////////////
// UI_getUIField
// Created:  November 4, 2003
// Input:  User interface ID (UIID), UITABID
///////////////////////////////////////////////////////////////
LONG getUIField( UI_INTERFACE *data, LONG fOpt )
{
#define SQL_getUIField "\
	SELECT USERINTERFACE.UIFID as UIFID, \
     USERINTERFACE.UITABID as UITABID, \
     USERINTERFACE.UILWIDTH as UILWIDTH, \
     USERINTERFACE.UIWIDTH as UIWIDTH, \
     USERINTERFACE.UI_Y as UI_Y, \
     USERINTERFACE.UI_X as UI_X, \
     USERINTERFACE.FLDTYPE as FLDTYPE, \
     USERINTERFACE.FLDDEF as FLDDEF, \
     USERINTERFACE.UIACCESS as UIACCESS,  \
	 USERINTERFACE.UIID as UIID \
	FROM USERINTERFACE,UITAB \
    WHERE USERINTERFACE.UIID = ? AND USERINTERFACE.UITABID=UITAB.UITABID AND  \
	      UITAB.UICLASS<>'HEADER' AND UITAB.UITABID=?   \
    ORDER BY USERINTERFACE.UI_Y, USERINTERFACE.UI_X "


  PUSH(SQL_getUIField);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_getUIField);

static LONG _uiid, _uitabid;
static UI_INTERFACE _data;
static BOOL first_time=TRUE;

       _uiid = data->UIID;
	   _uitabid = data->UITABID;
	if (fOpt == FIND_FIRST) {
      if (first_time)
      {
         local.BindParam(1, _uiid);  
         local.BindParam(2, _uitabid);  
         local.Bind(1, _data.UIFID);  
         local.Bind(2,_data.UITABID);
         local.Bind(3,_data.UILWIDTH);
         local.Bind(4,_data.UIWIDTH);
         local.Bind(5,_data.UI_Y);
         local.Bind(6,_data.UI_X);
         local.Bind(7,_data.FLDTYPE,4);
         local.Bind(8,_data.FLDDEF, DMS_VARIATE_NAME);
         local.Bind(9,_data.UIACCESS,4);
         local.Bind(10,_data.UIID);

         first_time=FALSE;
      }

      local.Execute();
	}
   if (local.Fetch()) {
       *data = _data;
   }
   else if (local.NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;

   POP();

#undef SQL_getUIField

}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE UI_getUIField( UI_INTERFACE *data , LONG fOpt)
{
   PUSH(UI_getUIField);
   ret=getUIField( data, fOpt );
   POP();
}



