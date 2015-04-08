/* atributs.cpp : Implements the functions  to access and manipulate the ATRIBUTS table of ICIS
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
 Programmed by   :   Olan Casumpang, Arllet M. Portugal
 Modified  (AMP) :   5/27/2002   {Adding AID in the ATTRIBUTE data structure}
 **************************************************************/

#include "atributs.h"

extern LONG UACCESS,USERID;
extern CODBCdbc *_centralDBC,*_localDBC;  //Central and Local Database Connection
extern stack<char*> _routines_stack;



LONG getNextAID(BOOL fLocal)
{
   static LONG localAID=0,centralAID=0;
   if (fLocal )
   {
	   if (!localAID) {
        CODBCdirectStmt source1 = _localDBC->DirectStmt("select MIN(UAID) from INSTLN") ;
        source1.Execute();
        source1.Fetch();
		localAID=source1.Field(1).AsInteger();
	   }
       CODBCdirectStmt source2 = _localDBC->DirectStmt("select MIN(AID) from ATRIBUTS") ;
       source2.Execute();
       source2.Fetch();
       localAID=min(localAID,source2.Field(1).AsInteger());
   }
   //else if (!fLocal && !centralAID) //Next Central AID
   //{
   //   CODBCdirectStmt source = _centralDBC->DirectStmt("select MAX(AID) from ATRIBUTS") ;
   //   source.Execute();
   //   source.Fetch();
   //   centralAID=source.Field(1).AsInteger();
   //}
   //return (fLocal)? --localAID:++centralAID;  //central GID is also returned
   return  --localAID;
}


//////////////////////////////////////////////////////////////////////////////
//Modified: April 23, 2004
//Retrieves the attributes of a given GID and changes made to the attributes from the CHANGES table
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_getAttribute(GMS_Attribute *data, LPSTR szDesc, LONG nszDesc
               , LONG fSearchOption)
{
   PUSH(GMS_getAttribute);

   static BOOL fLocal=TRUE;
   LONG aid,atype;

   if (fSearchOption==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal)
   {
      ret=getAttribute(LOCAL,*data,szDesc,nszDesc,fSearchOption);
      if (ret!=GMS_SUCCESS){
         fLocal = FALSE; 
         ret=getAttribute(CENTRAL,*data,szDesc,nszDesc,FIND_NEXT);
      }
   }
   else
      ret=getAttribute(CENTRAL,*data,szDesc,nszDesc,fSearchOption);

   if (ret == GMS_SUCCESS)
   {
      aid = data->aid;
	  atype= data->atype;
	  getAtributFieldFromChanges(aid,"ATYPE",atype);
      if (atype == 999) {   //the atribute is deleted 
		  ZeroMemory(szDesc, sizeof(szDesc));
          ret=GMS_getAttribute(data,szDesc,nszDesc,FIND_NEXT);		  
	  }
	  else {
	  getAtributFieldFromChanges(aid,"AUID",data->auid);
	  getAtributFieldFromChanges(aid,"ALOCN",data->alocn);
	  getAtributFieldFromChanges(aid,"ADATE",data->adate);
	  getAtributFieldFromChanges(aid,"AREF",data->aref);
	  }

   }
   POP();
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG getAttribute(BOOL fLocal, GMS_Attribute &data, CHAR *szDesc, LONG nszDesc
               , LONG fSearchOption)
{
#define SQL_GETATRIBUTS1 "\
   select GID, ATYPE, AUID, ALOCN, ADATE, AREF, AVAL, AID\
   from ATRIBUTS\
   where GID=?\
     and (0=? or ATYPE=?)\
   order by AID ASC"
#define SQL_GETATRIBUTS2 "\
   select GID, ATYPE, AUID, ALOCN, ADATE, AREF, AVAL, AID\
   from ATRIBUTS\
   where GID=?\
     and (0=? or ATYPE=?)\
   order by AID DESC"


   PUSH(getAttribute);

static CODBCbindedStmt &central= _centralDBC->BindedStmt(SQL_GETATRIBUTS1);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_GETATRIBUTS2);
       CODBCbindedStmt *source;

static GMS_Attribute _data;
static BOOL first_time=TRUE;

static LONG gid,atype;
static CHAR _szDesc[MAX_STR];

   if (fSearchOption==FIND_FIRST)
   {
      if (first_time)
      {
         BIND(1,_data.gid);
         BIND(2,_data.atype);
         BIND(3,_data.auid);
         BIND(4,_data.alocn);
         BIND(5,_data.adate);
         BIND(6,_data.aref);
         BINDS(7,_szDesc,MAX_STR);
		 BIND(8,_data.aid);
	      BINDPARAM(1,gid);
	      BINDPARAM(2,atype);
	      BINDPARAM(3,atype);
 
         first_time=FALSE;
      }
      gid=data.gid;
      atype=data.atype;
      local.Execute();
      central.Execute();
   }

   source = (fLocal)?&local:&central;

   _szDesc[0]='\0';
   if (source->Fetch()){
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
      ret=GMS_ERROR;

   POP();

#undef SQL_GETATRIBUTS
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG get_Aval_Adate_Replaced(LONG gid, CHAR *szDesc, LONG nszDesc
               , LONG &adate, LONG fSearchOption)
{
#define SQL_AVALADATE_REPLACED "\
   select ADATE, AVAL\
   from ATRIBUTS\
   where GID=?\
     and ATYPE=101\
   order by ADATE"

   PUSH(get_Aval_Adate_Replaced);

static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_AVALADATE_REPLACED);
      
static LONG _gid,_adate;
static BOOL first_time=TRUE;
static CHAR _szDesc[MAX_STR];

   if (first_time)
   {
      local.Bind(1,_adate);
      local.Bind(2,_szDesc,MAX_STR);
	   local.BindParam(1,_gid);
      first_time=FALSE;
   }

   if (fSearchOption==FIND_FIRST){
	   _gid=gid;
      local.Execute();
   }
   _szDesc[0]='\0';
   if (local.Fetch()){
      adate=_adate;
      strncpy(szDesc,_szDesc,nszDesc);
      szDesc[nszDesc-1]='\0';
   }
   else if (local.NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;

   POP();

#undef SQL_AVALADATE_REPLACED
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG get_Aval_Adate_Changed(LONG gid, CHAR *szDesc, LONG nszDesc
               , LONG &adate, LONG fSearchOption)
{
#define SQL_AVALADATE_CHANGED "\
   select ADATE, AVAL\
   from ATRIBUTS\
   where GID=?\
     and ATYPE=102\
   order by ADATE"

   PUSH(get_Aval_Adate_Changed);

static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_AVALADATE_CHANGED);
      
static LONG _gid,_adate;
static CHAR _szDesc[MAX_STR];

   //if (first_time)
   if (fSearchOption==FIND_FIRST)
   {
      local.Bind(1,_adate);
      local.Bind(2,_szDesc,MAX_STR);
	   local.BindParam(1,_gid);
      //first_time=FALSE;
   }

   if (fSearchOption==FIND_FIRST)
   {
	   _gid=gid;
      local.Execute();
   }
   _szDesc[0]='\0';
   if (local.Fetch()){
      adate=_adate;
      strncpy(szDesc,_szDesc,nszDesc);
      szDesc[nszDesc-1]='\0';
      //PadRight(szDesc,nszDesc);
   }
   else if (local.NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;

   POP();

#undef SQL_AVALADATE_CHANGED
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG get_Aval_Adate(LONG gid, LONG atype, CHAR *szDesc, LONG nszDesc
               , LONG &adate, LONG fSearchOption)
{
   if (atype==101)
      return get_Aval_Adate_Replaced(gid,szDesc,nszDesc,adate,fSearchOption);
   else
      return get_Aval_Adate_Changed(gid,szDesc,nszDesc,adate,fSearchOption);
}

LONG get_Aval_Adate2(LONG gid, LONG atype, CHAR *szDesc, LONG nszDesc
               , LONG &adate, LONG fSearchOption)
{
#define SQL_AVALADATE "\
   select ADATE, AVAL\
   from ATRIBUTS\
   where GID=? and ATYPE=?\
   order by ADATE DESC"

   PUSH(get_Aval_Adate);

static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_AVALADATE);
      
static LONG _gid,_adate,_atype;
static CHAR _szDesc[128];
static BOOL first_time=TRUE;

   if (first_time)
   {
      local.Bind(1,_adate);
      local.Bind(2,_szDesc,128);
	   local.BindParam(1,_gid);
	   local.BindParam(2,_atype);
      first_time=FALSE;
   }

   if (fSearchOption==FIND_FIRST)
   {
	   _gid=gid;
      _atype = atype;
      local.Execute();
   }
   _szDesc[0]='\0';
   if (local.Fetch())
   {
      adate=_adate;
      strncpy(szDesc,_szDesc,nszDesc);
      szDesc[nszDesc-1]='\0';

   }
   else if (local.NoData())
      ret = GMS_NO_DATA;
   else 
      ret = GMS_ERROR;

   POP();

#undef SQL_AVALADATE_CHANGED
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_addAttribute(GMS_Attribute *data, CHAR *szAval)
{
   PUSH(GMS_addAttribute);
   if (UACCESS<30)
      ret= GMS_NO_ACCESS;
   else 
      ret= addAttribute(*data,szAval);
   POP();
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG addAttribute(GMS_Attribute &atributs, CHAR *szAval)
{
#define SQL_ADDATRIBUTS "\
   insert into ATRIBUTS\
   (AID, GID,ATYPE,AUID,ALOCN,ADATE,AREF,AVAL) \
   VALUES (?, ?, ?, ?, ?, ?, ?, ?)"

   PUSH(addAttribute);

static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_ADDATRIBUTS);

static GMS_Attribute _data;
static BOOL first_time=TRUE;
static LONG _time;
static long aid=0;

   if (first_time)
   {
      local.BindParam(1,aid);
      local.BindParam(2,_data.gid);
      local.BindParam(3,_data.atype);
      local.BindParam(4,_data.auid);
      local.BindParam(5,_data.alocn);
      local.BindParam(6,_data.adate);
      //delete: local.BindParam(7,_time);
      local.BindParam(7,_data.aref);
 
      first_time=FALSE;
   }
   
   aid = getNextAID(TRUE);
   _data = atributs;
   _data.aid = aid;
   _data.auid = USERID;
   _data.adate =GMS_getDate();
   //_time=GMS_getTime();

   local.BindParam(8,szAval,strlen(szAval)+1);
   
   if ((ret=local.Execute())==1) {
     atributs= _data;
     ret=GMS_SUCCESS;
   }
   else
	   if (ret!=GMS_UNIQUE_CONSTRAINT_ERROR) //GMS_UNIQUE_CONSTRAINT_ERROR
		   ret=GMS_ERROR;  
       // Handler for unique contraint error with the assumption that only AID has unique contraint 
	   else {
		   int i=0;
		   while ((ret==GMS_UNIQUE_CONSTRAINT_ERROR) && (i<100)) {
				_data.aid = getNextAID(TRUE);
				ret=local.Execute();
				++i;
		   }
		   if ((ret) &&(ret!=GMS_UNIQUE_CONSTRAINT_ERROR)) {
               atributs= _data;
			   ret = GMS_SUCCESS;
		   }
		   else  {
			   atributs.aid = 0;
			   ret = GMS_ERROR;
		   }
	   }


   POP();

#undef SQL_ADDATRIBUTS
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_setAttribute(GMS_Attribute *data, CHAR *szAval)
{
   PUSH(GMS_setAttribute);
   if (UACCESS<30)
      ret= GMS_NO_ACCESS;
   else 
      ret= setAttribute(*data,szAval);
   POP();
}



//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG setAttribute(GMS_Attribute &atributs, CHAR *szAval)
{
#define SQL_SETATRIBUTS "\
   UPDATE ATRIBUTS\
    set GID = ?,ATYPE=? , AUID=?, ALOCN=?, ADATE=?, AREF=?, AVAL=?  \
	WHERE AID = ?"

   PUSH(setAttribute);

static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_SETATRIBUTS);

static GMS_Attribute _data;
static BOOL first_time=TRUE;
static LONG _time;
static long aid=0;

   if (first_time)
   {
      local.BindParam(1,_data.gid);
      local.BindParam(2,_data.atype);
      local.BindParam(3,_data.auid);
      local.BindParam(4,_data.alocn);
      local.BindParam(5,_data.adate);
      local.BindParam(6,_data.aref);
       local.BindParam(8,_data.aid);

      first_time=FALSE;
   }
   
   _data = atributs;
   _data.auid = USERID;
   _data.adate =atributs.adate;
   
   local.BindParam(7,szAval,strlen(szAval)+1);
   
   if (local.Execute())
      ret = GMS_SUCCESS;
   else if (local.NoData())
      ret = GMS_NO_DATA;
   else 
      ret = GMS_ERROR;


   POP();

#undef SQL_SETATRIBUTS
}


//////////////////////////////////////////////////////////////////////////////
//Created:  April 23, 2004
//Description:  Get the change made to the a record in the attributes table
// Input:  NID, FIELD NAME
//////////////////////////////////////////////////////////////////////////////
LONG getAtributFieldFromChanges(LONG aid,LPCSTR szField,LONG &to)
{
#define SQL_GETFIELDFROMCHGS "\
   select CTO\
   from CHANGES where CRECORD=? and CTABLE='ATRIBUTS' and CFIELD=? and CSTATUS=0 \
   order by CDATE desc, CTIME desc"

   LONG ret=GMS_SUCCESS;

static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_GETFIELDFROMCHGS);
       CODBCbindedStmt *source;

static LONG _aid,_to;
static CHAR _field[GMS_MAX_FIELD_NAME]="";

static BOOL first_time=TRUE;

   if (first_time)
   {
      local.Bind(1,_to);
      local.BindParam(1,_aid);
      local.BindParam(2,_field,GMS_MAX_FIELD_NAME);
      first_time=FALSE;
   }

   _aid=aid;
   strcpy(_field,szField);

   local.Execute();

   source = &local;//(fLocal)?&local:&central;

   if (source->Fetch())
   {
      to=_to;
      ret = GMS_SUCCESS;//getChanges(fLocal,_cid,data,szDesc,nszDesc);
   }
   else if (source->NoData())
      ret = GMS_NO_DATA;
   else 
      ret = GMS_ERROR;

   return ret;//POP();
#undef SQL_GETFIELDFROMCHGS
}


//////////////////////////////////////////////////////////////////////////////
//Seach for germplasm based on the given attribute value and type
//////////////////////////////////////////////////////////////////////////////
LONG findAttribute(BOOL fLocal,GMS_Attribute &atributs, LPSTR szDesc,LONG nszDesc
               ,LONG fSearchOption)
{
#define SQL_findAttribute "\
   select GID, ATYPE, AUID, ALOCN, ADATE, AREF, AVAL, AID\
   from ATRIBUTS \
   where ((ATYPE=? or 0=?) AND NOT (ATYPE = 999) AND AVAL=?)\
   order by GID ASC"   
   PUSH(findAttribute);

static CODBCbindedStmt &central= _centralDBC->BindedStmt(SQL_findAttribute);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_findAttribute);
       CODBCbindedStmt *source;

static GMS_Attribute _data;
static BOOL first_time=TRUE;
static LONG atype;
static CHAR _szDesc[0xFFF];

   if (fSearchOption==FIND_FIRST)
   {
      if (first_time)
      {
         BIND(1,_data.gid);
         BIND(2,_data.atype);
         BIND(3,_data.auid);
         BIND(4,_data.alocn);
         BIND(5,_data.adate);
         BIND(6,_data.aref);
         BINDS(7,_szDesc,MAX_STR);
		 BIND(8,_data.aid);
	      BINDPARAM(1,atype);
	      BINDPARAM(2,atype);
	      BINDPARAMS(3,_szDesc,nszDesc);
 
         first_time=FALSE;
      }
	  strcpy((CHAR *) _szDesc, szDesc); 
	  atype = atributs.atype;
      local.Execute();
      central.Execute();
   }

   source = (fLocal)?&local:&central;

   ZeroMemory(&_data,sizeof(_data));
//   _szDesc[0]='\0';
   if (source->Fetch())
   {
      atributs = _data;
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

#undef SQL_findAttribute
}

//////////////////////////////////////////////////////////////////////////////
//Seach for germplasm based on the given term with wild card and attribute type
//Date Created: Feb 12, 2008 (AMP)
//////////////////////////////////////////////////////////////////////////////
LONG findAttribute2(BOOL fLocal,GMS_Attribute &atributs, LPSTR szDesc,LONG nszDesc
               ,LONG fSearchOption)
{
#define SQL_findAttribute "\
   select GID, ATYPE, AUID, ALOCN, ADATE, AREF, AVAL, AID\
   from ATRIBUTS \
   where ((ATYPE=? or 0=?) AND NOT (ATYPE = 999) AND AVAL LIKE ?)\
   order by GID ASC"   
   PUSH(findAttribute);

static CODBCbindedStmt &central= _centralDBC->BindedStmt(SQL_findAttribute);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_findAttribute);
       CODBCbindedStmt *source;

static GMS_Attribute _data;
static BOOL first_time=TRUE;
static LONG atype;
static CHAR _szDesc[0xFFF];

   if (fSearchOption==FIND_FIRST)
   {
      if (first_time)
      {
         BIND(1,_data.gid);
         BIND(2,_data.atype);
         BIND(3,_data.auid);
         BIND(4,_data.alocn);
         BIND(5,_data.adate);
         BIND(6,_data.aref);
         BINDS(7,_szDesc,MAX_STR);
		 BIND(8,_data.aid);
	      BINDPARAM(1,atype);
	      BINDPARAM(2,atype);
	      BINDPARAMS(3,_szDesc,nszDesc);
 
         first_time=FALSE;
      }
	  strcpy((CHAR *) _szDesc, szDesc); 
	  atype = atributs.atype;
      local.Execute();
      central.Execute();
   }

   source = (fLocal)?&local:&central;

   ZeroMemory(&_data,sizeof(_data));
//   _szDesc[0]='\0';
   if (source->Fetch())
   {
      atributs = _data;
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

#undef SQL_findAttribute
}

//////////////////////////////////////////////////////////////////////////////
//Created: May 15, 2007
//Modified: Feb 12, 2008 (AMP) - considers wild card characters
//Seach for germplasm based on the given attribute value and type
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_findAttribute(GMS_Attribute *data, LPSTR szDesc, LONG nszDesc
               , LONG fSearchOption)
{
   PUSH(GMS_findAttribute);

   static BOOL fLocal=TRUE, fWild=FALSE;
   LONG aid,atype;

   if (fSearchOption==FIND_FIRST)
         fLocal=TRUE;

   if (strchr(szDesc,'%') || fWild==TRUE){
	   fWild = TRUE;
		if (fLocal)
		{
			ret=findAttribute2(LOCAL,*data,szDesc,nszDesc,fSearchOption);
			if (ret!=GMS_SUCCESS){
				fLocal = FALSE; 
				ret=findAttribute2(CENTRAL,*data,szDesc,nszDesc,FIND_NEXT);
			}
		}
		else
			ret=findAttribute2(CENTRAL,*data,szDesc,nszDesc,fSearchOption);
		if (ret!=GMS_SUCCESS) 
			fWild = FALSE;

   }
   else {
	if (fLocal)
	{
		ret=findAttribute(LOCAL,*data,szDesc,nszDesc,fSearchOption);
		if (ret!=GMS_SUCCESS){
			fLocal = FALSE; 
			ret=findAttribute(CENTRAL,*data,szDesc,nszDesc,FIND_NEXT);
		}
	}
	else
		ret=findAttribute(CENTRAL,*data,szDesc,nszDesc,fSearchOption);
   }
   if (ret == GMS_SUCCESS)
   {
      aid = data->aid;
	  atype= data->atype;
	  getAtributFieldFromChanges(aid,"ATYPE",atype);
	  getAtributFieldFromChanges(aid,"AUID",data->auid);
	  getAtributFieldFromChanges(aid,"ALOCN",data->alocn);
	  getAtributFieldFromChanges(aid,"ADATE",data->adate);
	  getAtributFieldFromChanges(aid,"AREF",data->aref);

   }
   POP();
}



