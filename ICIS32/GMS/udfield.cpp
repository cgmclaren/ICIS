/* udfield.cpp : Implements the functions to access and manipulate the UDFLDS table of ICIS
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

#include "udfield.h"

extern LONG UACCESS,USERID;
extern CODBCdbc *_centralDBC,*_localDBC;  //Central and Local Database Connection
extern stack<char*> _routines_stack;

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG getNextUDField(void)
{
static LONG fldno=0;

      CODBCdirectStmt local = _localDBC->DirectStmt("SELECT MIN(FLDNO) FROM UDFLDS");
      local.Execute();
      local.Fetch();
      fldno=local.Field(1).AsInteger();
      if (fldno>0) fldno=0;
//   }
   return --fldno;
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_getUDField(GMS_UDField *data, LPSTR szDesc, LONG nszDesc
               , LONG fSearchOption)
{
   static BOOL fLocal=TRUE;

   PUSH(GMS_getUDField);

   if (fSearchOption==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal){
      ret=getUDField(LOCAL,*data,szDesc,nszDesc,fSearchOption);
      if (ret!=GMS_SUCCESS){
         fLocal = FALSE; 
         ret=getUDField(CENTRAL,*data,szDesc,nszDesc,FIND_NEXT);
      }
   }
   else
      ret=getUDField(CENTRAL,*data,szDesc,nszDesc,fSearchOption);

   POP();

}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG getUDField(BOOL fLocal,GMS_UDField &data,LPSTR szDesc,LONG nszDesc
               ,LONG fSearchOption)
{
#define SQL_UDFIELD "\
   select FLDNO, FTABLE, FTYPE, FCODE, \
   FNAME, FFMT, LFLDNO,FUID,FDATE,FDESC, SCALEID \
	from UDFLDS\
   where (0=? or FLDNO=?)" 
   
   PUSH(getUDField);

static CODBCbindedStmt &central= _centralDBC->BindedStmt(SQL_UDFIELD);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_UDFIELD);
       CODBCbindedStmt *source;

static GMS_UDField _data;
static BOOL first_time=TRUE;

static LONG id;
static CHAR _szDesc[0xFFF];

   if (fSearchOption==FIND_FIRST)
   {
      if (first_time)
      {
         BIND(1,_data.fldno);
         BINDS(2,_data.ftable,GMS_FTABLE_SIZE);
         BINDS(3,_data.ftype,GMS_FTYPE_SIZE);
         BINDS(4,_data.fcode,GMS_FCODE_SIZE);
         BINDS(5,_data.fname,GMS_FNAME_SIZE);
         BINDS(6,_data.ffmt,GMS_FFMT_SIZE);
         BIND(7,_data.lfldno);
         BIND(8,_data.fuid);
         BIND(9,_data.fdate);
         BINDS(10,_szDesc,0xFFF);
		 BIND(11, _data.scaleid);
	      BINDPARAM(1,id);
	      BINDPARAM(2,id);
 
         first_time=FALSE;
      }
	   id=data.fldno;
      local.Execute();
      central.Execute();
   }

   source = (fLocal)?&local:&central;

   ZeroMemory(&_data,sizeof(_data));
   _szDesc[0]='\0';
   if (source->Fetch())
   {
      data = _data;
      if (szDesc)
      {
         strncpy(szDesc,_szDesc,nszDesc);
         szDesc[nszDesc-1]='\0';
      }
   }
   else if (source->NoData())
      ret=GMS_NO_DATA;
   else 
      ret= GMS_ERROR;

   POP();

}

//////////////////////////////////////////////////////////////////////////////
//Date Created: Sept 23, 2005
// Description:  Retrieves a record from UDFLDS table for the given table (FTABLE) and column (FTYPE)
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_getUDField2(GMS_UDField *data, LPSTR szDesc, LONG nszDesc
               , LONG fSearchOption)
{
   static BOOL fLocal=TRUE;

   PUSH(GMS_getUDField2);

   if (fSearchOption==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal){
      ret=getUDField2(LOCAL,*data,szDesc,nszDesc,fSearchOption);
      if (ret!=GMS_SUCCESS){
         fLocal = FALSE; 
         ret=getUDField2(CENTRAL,*data,szDesc,nszDesc,FIND_NEXT);
      }
   }
   else
      ret=getUDField2(CENTRAL,*data,szDesc,nszDesc,fSearchOption);

   POP();

}



//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG getUDField2(BOOL fLocal,GMS_UDField &data,LPSTR szDesc,LONG nszDesc
               ,LONG fSearchOption)
{
#define SQL_UDFIELD "\
   select FLDNO, FTABLE, FTYPE, FCODE, \
   FNAME, FFMT, LFLDNO,FUID,FDATE,FDESC, SCALEID \
	from UDFLDS\
   where (FTABLE=? and FTYPE=?)" 
   
   PUSH(getUDField2);

static CODBCbindedStmt &central= _centralDBC->BindedStmt(SQL_UDFIELD);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_UDFIELD);
       CODBCbindedStmt *source;

static GMS_UDField _data;
static BOOL first_time=TRUE;

static CHAR _szDesc[0xFFF];
static CHAR _ftable[GMS_FTABLE_SIZE], _ftype[GMS_FTYPE_SIZE];


   if (fSearchOption==FIND_FIRST)
   {
 	 strncpy((CHAR *) _ftable, data.ftable, sizeof(data.ftable)); 
	 strncpy((CHAR *) _ftype, data.ftype, sizeof(data.ftype)); 
     if (first_time)
      {
         BIND(1,_data.fldno);
         BINDS(2,_data.ftable,GMS_FTABLE_SIZE);
         BINDS(3,_data.ftype,GMS_FTYPE_SIZE);
         BINDS(4,_data.fcode,GMS_FCODE_SIZE);
         BINDS(5,_data.fname,GMS_FNAME_SIZE);
         BINDS(6,_data.ffmt,GMS_FFMT_SIZE);
         BIND(7,_data.lfldno);
         BIND(8,_data.fuid);
         BIND(9,_data.fdate);
         BINDS(10,_szDesc,0xFFF);
		 BIND(11, _data.scaleid);
	     BINDPARAMS(1,_ftable,GMS_FTABLE_SIZE);
	     BINDPARAMS(2,_ftype,GMS_FTYPE_SIZE);
 
         first_time=FALSE;
      }

      local.Execute();
      central.Execute();
   }

   source = (fLocal)?&local:&central;

   ZeroMemory(&_data,sizeof(_data));
   _szDesc[0]='\0';
   if (source->Fetch())
   {
      data = _data;
      if (szDesc)
      {
         strncpy(szDesc,_szDesc,nszDesc);
         szDesc[nszDesc-1]='\0';
      }
   }
   else if (source->NoData())
      ret=GMS_NO_DATA;
   else 
      ret= GMS_ERROR;

   POP();

}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_findUDField(GMS_UDField *data, LPSTR szDesc, LONG nszDesc
               , LONG fSearchOption)
{
   static BOOL fLocal=TRUE;

   PUSH(GMS_findUDField);

   if (fSearchOption==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal){
      ret=findUDField(LOCAL,*data,szDesc,nszDesc,fSearchOption);
      if (ret!=GMS_SUCCESS){
         fLocal = FALSE; 
         ret=findUDField(CENTRAL,*data,szDesc,nszDesc,FIND_NEXT);
      }
   }
   else
      ret=findUDField(CENTRAL,*data,szDesc,nszDesc,fSearchOption);

   POP();

}



//////////////////////////////////////////////////////////////////////////////
//Input: field name specified in the UDFLDS data structure
//////////////////////////////////////////////////////////////////////////////
LONG findUDField(BOOL fLocal,GMS_UDField &data,LPSTR szDesc,LONG nszDesc
               ,LONG fSearchOption)
{
#define SQL_UDFIELD_FIND "\
   select FLDNO, FTABLE, FTYPE, FCODE, \
   FNAME, FFMT, LFLDNO,FUID,FDATE,FDESC, SCALEID \
	from UDFLDS\
   where (FNAME=?)" 
   
   PUSH(getUDField);

static CODBCbindedStmt &central= _centralDBC->BindedStmt(SQL_UDFIELD_FIND);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_UDFIELD_FIND);
       CODBCbindedStmt *source;

static GMS_UDField _data;
static BOOL first_time=TRUE;

static CHAR _flddef[DMS_VARIATE_NAME];
static CHAR _szDesc[0xFFF];

   if (fSearchOption==FIND_FIRST)
   {
      if (first_time)
      {
         BIND(1,_data.fldno);
         BINDS(2,_data.ftable,GMS_FTABLE_SIZE);
         BINDS(3,_data.ftype,GMS_FTYPE_SIZE);
         BINDS(4,_data.fcode,GMS_FCODE_SIZE);
         BINDS(5,_data.fname,GMS_FNAME_SIZE);
         BINDS(6,_data.ffmt,GMS_FFMT_SIZE);
         BIND(7,_data.lfldno);
         BIND(8,_data.fuid);
         BIND(9,_data.fdate);
         BINDS(10,_szDesc,0xFFF);
		 BIND(11, _data.scaleid);
	     BINDPARAMS(1,_flddef,DMS_VARIATE_NAME);
 
         first_time=FALSE;
      }
	  strcpy((CHAR *) _flddef, data.fname); 
      local.Execute();
      central.Execute();
   }

   source = (fLocal)?&local:&central;

   ZeroMemory(&_data,sizeof(_data));
   _szDesc[0]='\0';
   if (source->Fetch())
   {
      data = _data;
      if (szDesc)
      {
         strncpy(szDesc,_szDesc,nszDesc);
         szDesc[nszDesc-1]='\0';
      }
   }
   else if (source->NoData())
      ret=GMS_NO_DATA;
   else 
      ret= GMS_ERROR;

   POP();

}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG addUDField(GMS_UDField &data,LPSTR szDesc, LONG nszDesc)
{
#define SQL_ADDUDFLDS "\
   INSERT INTO UDFLDS ( FLDNO, FTABLE, FTYPE, FCODE, \
   FNAME, FFMT, LFLDNO,FUID,FDATE, SCALEID,FDESC ) \
    values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"

   PUSH(addUdflds);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_ADDUDFLDS);

static GMS_UDField _data;
static BOOL first_time=TRUE;
static CHAR _szDesc[0xFFF];

   strncpy(_szDesc,szDesc,nszDesc);
   data.fldno = getNextUDField();
   _data = data;
   if (first_time)
   {
	  local.BindParam(1,_data.fldno);
      local.BindParam(2,_data.ftable, sizeof(_data.ftable));
	  local.BindParam(3,_data.ftype, sizeof(_data.ftype));
	  local.BindParam(4,_data.fcode, sizeof(_data.fcode));
	  local.BindParam(5,_data.fname, sizeof(_data.fname));
	  local.BindParam(6,_data.ffmt,ICIS_MAX_SIZE);
	  local.BindParam(7,_data.lfldno);
	  local.BindParam(8,_data.fuid);
	  local.BindParam(9,_data.fdate);
	  local.BindParam(10,_data.scaleid);
	  local.BindParam(11,_szDesc, ICIS_MAX_SIZE);
      first_time=FALSE;
   }
   
   if ((ret=local.Execute())==1)
      ret=GMS_SUCCESS;   
   else 
	   if (ret!=GMS_UNIQUE_CONSTRAINT_ERROR) //GMS_UNIQUE_CONSTRAINT_ERROR
		   ret=GMS_ERROR;  
       // Handler for unique contraint error with the assumption that only AID has unique contraint 
	   else {
		   int i=0;
		   while ((ret==GMS_UNIQUE_CONSTRAINT_ERROR) && (i<100)) {
                _data.fldno = getNextUDField();
				ret=local.Execute();
				++i;
		   }
		   if ((ret) &&(ret!=GMS_UNIQUE_CONSTRAINT_ERROR)) {
               data.fldno= _data.fldno;
			   ret = GMS_SUCCESS;
		   }
		   else  {
			   data.fldno = 0;
			   ret = GMS_ERROR;
		   }
	   }
   POP();

#undef SQL_ADDUDFLDS
}

//////////////////////////////////////////////////////////////////////////////
//The function adds a record in the UDFIELD table
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_addUDField(GMS_UDField &data,LPSTR szDesc, LONG nszDesc)
{
   return addUDField(data, szDesc, nszDesc);
}

