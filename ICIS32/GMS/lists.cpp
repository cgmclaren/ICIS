/* lists.cpp : Implements the functions to access and manipulate the CHANGES table of ICIS
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

#include "lists.h"

extern LONG UACCESS,USERID;
extern CODBCdbc *_centralDBC,*_localDBC;
extern stack<char*> _routines_stack;
extern bool AUTOCOMMIT;

#if defined(WEB)
extern bool WEBSERVICE;         //True if calls are to Web Service
extern _bstr_t szURL;           // URL pointing to the WSDL of the Web Service
#endif



//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_getListName(GMS_ListName *data,LONG fSearchOption)
{

   static BOOL fLocal=TRUE;
   PUSH(GMS_getListName);

   if (fSearchOption==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal){
      ret=getListName2(LOCAL, *data,fSearchOption);
      if (ret!=GMS_SUCCESS){
         fLocal = FALSE; 

#if defined(WEB)
	   if (WEBSERVICE == true) {
		   IManagedInterface *cpi = NULL; 
		   L_ListName ldata;
		   CoInitialize(NULL);
		   HRESULT hr = CoCreateInstance(CLSID_LibraryImplementation,
					NULL, CLSCTX_INPROC_SERVER, IID_IManagedInterface, reinterpret_cast<void**>(&cpi)); 
		   if (FAILED(hr))
		   {
			   MessageBox(NULL,"Could not create Instance, Web Service Failed!","ICIS V5",MB_OK);
		   }
		   ZeroMemory(&ldata,sizeof(ldata));
 		   ldata.listid = data->listid;
           ret = cpi->LgetListName(&ldata,szURL);
           CoUninitialize();
       
	   }
	   else
#endif

         ret=getListName2(CENTRAL,*data,FIND_NEXT);
      }
   }
   else
#if defined(WEB)
	   if (WEBSERVICE == true) {
		   ret = GMS_NO_DATA;
	   }      
#endif

      ret=getListName2(CENTRAL,*data,fSearchOption);

   POP();
}



//////////////////////////////////////////////////////////////////////////////
// Input: Liststatus
// Date Created:  Jan 31, 2005
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_getListByStatus(GMS_ListName *data,LONG fSearchOption)
{

   static BOOL fLocal=TRUE;
   PUSH(GMS_getListName);

   if (fSearchOption==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal){
      ret=getListByStatus(LOCAL, *data,fSearchOption);
      if (ret!=GMS_SUCCESS){
         fLocal = FALSE; 
         ret=getListByStatus(CENTRAL,*data,FIND_NEXT);
      }
   }
   else
      ret=getListByStatus(CENTRAL,*data,fSearchOption);

   POP();
}


//////////////////////////////////////////////////////////////////////////////
// Date Created: October 18, 2004
// Description: Retrieve record from list name with the list status and list hierarchy
//              Data are retrieved from either local or central database 
// Input Parameter: LISTID, LHIERARCHY
// Date Modified: Dec 7, 2004
//////////////////////////////////////////////////////////////////////////////
LONG getListName2(BOOL fLocal, GMS_ListName &data,LONG fSearchOption)
{
#define SQL_GETLISTNAME2	"\
   select LISTID,LISTNAME,LISTDATE,LISTTYPE,LISTUID,LISTDESC, LISTSTATUS, LHIERARCHY\
   from LISTNMS\
   where ( (0=? OR LISTNMS.LISTID=?) AND (0=? OR LHIERARCHY=?) AND LISTSTATUS <> 9) " 
//   where (0=? or LISTNMS.LISTID=?) " 

   PUSH(getListName2);

static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_GETLISTNAME2);
static CODBCbindedStmt &central  = _centralDBC->BindedStmt(SQL_GETLISTNAME2);
       CODBCbindedStmt *source; 
static GMS_ListName _data;
static BOOL first_time=TRUE;

static LONG id, _hier;

   if (first_time)
   {
      BIND(1,_data.listid);
      BINDS(2,_data.listname,sizeof(_data.listname));
      BIND(3,_data.listdate);
      BINDS(4,_data.listtype,sizeof(_data.listtype));
      BIND(5,_data.listuid);
      BINDS(6,_data.listdesc,ICIS_MAX_SIZE);  //sizeof(_data.listdesc)
      BIND(7,_data.liststatus);
	  BIND(8,_data.lhierarchy);
	  BINDPARAM(1,id);
	  BINDPARAM(2,id);
	  BINDPARAM(3,_hier);
	  BINDPARAM(4,_hier);
 
      first_time=FALSE;
   }

   if (fSearchOption==FIND_FIRST){
	   id=data.listid;
	   _hier = data.lhierarchy;
      local.Execute();
	  central.Execute();
   }
   source = (fLocal)?&local:&central;

   ZeroMemory(&_data,sizeof(_data));
   if (source->Fetch())
      data = _data;
   else if (source->NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;

   POP();

#undef SQL_GETLISTNAME2
}

//////////////////////////////////////////////////////////////////////////////
// Date Created: January 31, 2005
// Description: Retrieve record from list name with the list status and list hierarchy
//              Data are retrieved from either local or central database 
// Input Parameter:  LISTSTATUS
//////////////////////////////////////////////////////////////////////////////
LONG getListByStatus(BOOL fLocal, GMS_ListName &data,LONG fSearchOption)
{
#define SQL_GETLISTSTATUS	"\
   select LISTID,LISTNAME,LISTDATE,LISTTYPE,LISTUID,LISTDESC, LISTSTATUS, LHIERARCHY\
   from LISTNMS\
   where  ( LISTSTATUS=? OR (0=?)) AND (LHIERARCHY= ? OR (0=?)) " 

   PUSH(getListName2);

static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_GETLISTSTATUS);
static CODBCbindedStmt &central  = _centralDBC->BindedStmt(SQL_GETLISTSTATUS);
       CODBCbindedStmt *source; 
static GMS_ListName _data;
static BOOL first_time=TRUE;

static LONG id, _status, _hier;

   if (first_time)
   {
      BIND(1,_data.listid);
      BINDS(2,_data.listname,sizeof(_data.listname));
      BIND(3,_data.listdate);
      BINDS(4,_data.listtype,sizeof(_data.listtype));
      BIND(5,_data.listuid);
      BINDS(6,_data.listdesc,sizeof(_data.listdesc));
      BIND(7,_data.liststatus);
	  BIND(8,_data.lhierarchy);
	  BINDPARAM(1,_status);
	  BINDPARAM(2,_status);
	  BINDPARAM(3,_hier);
	  BINDPARAM(4,_hier);
      first_time=FALSE;
   }

   if (fSearchOption==FIND_FIRST){
	   _status = data.liststatus;
	   _hier = data.lhierarchy;
      local.Execute();
	  central.Execute();
   }
   source = (fLocal)?&local:&central;

   ZeroMemory(&_data,sizeof(_data));
   if (source->Fetch())
      data = _data;
   else if (source->NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;

   POP();

#undef SQL_GETLISTSTATUS
}



//////////////////////////////////////////////////////////////////////////////
// Find a given list name 
// LHIERARCHY is the hierarchy of the list
// Date Programmed:  July 16,2003
// Date Modified: Dec 6. 2004 (data are retrieved from local and central database
//////////////////////////////////////////////////////////////////////////////
LONG findListName2(BOOL fLocal, CHAR *szName,GMS_ListName &data,LONG fSearchOption)
{
#define SQL_FINDLISTNAME	"\
   select LISTID,LISTNAME,LISTDATE,LISTTYPE,LISTUID,LISTDESC, LISTSTATUS, LHIERARCHY\
   from LISTNMS\
   where (LISTNAME=?) AND LISTSTATUS <> 9 " 

   PUSH(findListName2);

static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_FINDLISTNAME);
static CODBCbindedStmt &central  = _centralDBC->BindedStmt(SQL_FINDLISTNAME);
       CODBCbindedStmt *source;
static GMS_ListName _data;
static BOOL first_time=TRUE;
static char _szName[GMS_LIST_NAME];

static LONG id;

   
   if (first_time)
   {
      local.Bind(1,_data.listid);
      local.Bind(2,_data.listname,sizeof(_data.listname));
      local.Bind(3,_data.listdate);
      local.Bind(4,_data.listtype,sizeof(_data.listtype));
      local.Bind(5,_data.listuid);
      local.Bind(6,_data.listdesc,ICIS_MAX_SIZE); //sizeof(_data.listdesc)
      local.Bind(7,_data.liststatus);
	  local.Bind(8,_data.lhierarchy);
      local.BindParam(1,_szName,GMS_LIST_NAME);

 
      first_time=FALSE;
   }

   if (fSearchOption==FIND_FIRST){
      strcpy(_szName, szName);
      local.Execute();
	  central.Execute();
   }
   source = (fLocal)?&local:&central;
   ZeroMemory(&_data,sizeof(_data));
   if (source->Fetch())
      data = _data;
   else if (source->NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;

   POP();

#undef SQL_FINDLISTNAME2
}




//////////////////////////////////////////////////////////////////////////////
//Description:  Finds if a list name exists in the database ;return GMS_SUCCESS if found
//Programmed:   July 21, 2003
// Modified:    Dec 2, 2004 (Uses the findListName2 which includes LISTSTATUS and LHIERARCHY
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_findListName(CHAR *szName,GMS_ListName &data,LONG fSearchOption)
{
   static BOOL fLocal=TRUE;
   PUSH(GMS_findListName);


   if (fSearchOption==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal){
      ret=findListName2(LOCAL,szName, data,fSearchOption);
      if (ret!=GMS_SUCCESS){
         fLocal = FALSE; 
         ret=findListName2(CENTRAL,szName, data,FIND_NEXT);
      }
   }
   else
      ret=findListName2(CENTRAL,szName, data,fSearchOption);

   POP();
}






//////////////////////////////////////////////////////////////////////////////
//  Determines whether a list name exists or not
// LHIERARCHY is the hierarchy of the list
// Date Programmed:  Dec 7,2004
//////////////////////////////////////////////////////////////////////////////
LONG existList(BOOL fLocal, GMS_ListName &data)
{
#define SQL_EXISTLISTNAME	"\
   select LISTID,LISTNAME,LISTDATE,LISTTYPE,LISTUID,LISTDESC, LISTSTATUS, LHIERARCHY\
   from LISTNMS\
   where (LISTNMS.LISTNAME=? and LHIERARCHY=? AND LISTSTATUS <> 9)" 

   PUSH(existListName);

static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_EXISTLISTNAME);
static CODBCbindedStmt &central  = _centralDBC->BindedStmt(SQL_EXISTLISTNAME);
       CODBCbindedStmt *source;

static GMS_ListName _data;
static BOOL first_time=TRUE;
static char _szName[GMS_LIST_NAME];

static LONG pid;

   
//   if (first_time)
//   {
      BIND(1,_data.listid);
      BINDS(2,_data.listname,sizeof(_data.listname));
      BIND(3,_data.listdate);
      BINDS(4,_data.listtype,sizeof(_data.listtype));
      BIND(5,_data.listuid);
      BINDS(6,_data.listdesc,sizeof(_data.listdesc));
      BIND(7,_data.liststatus);
	  BIND(8,_data.lhierarchy);
      BINDPARAMS(1,_data.listname,GMS_LIST_NAME);
      BINDPARAM(2,pid);
 
      first_time=FALSE;
//   }
   ZeroMemory(_data.listname,GMS_LIST_NAME); 
   strncpy(_data.listname,data.listname, strlen(data.listname));
   pid = data.lhierarchy;
   source = (fLocal)?&local:&central;

   source->Execute();
   
   ZeroMemory(&_data,sizeof(_data));
   if (source->Fetch())
      data = _data;
   else if (source->NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;

   POP();

#undef SQL_FINDLISTNAME2
}


LONG DLL_INTERFACE GMS_existList(GMS_ListName *data)
{

   PUSH(DMS_existList);
   ret=existList(LOCAL,*data);
   if (ret!=DMS_SUCCESS){
      ret=existList(CENTRAL,*data);
   }

   POP();

}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_setListName(GMS_ListName *data)
{
   PUSH(GMS_setListName2);
   ret=setListName2(*data);
   POP();
}


//////////////////////////////////////////////////////////////////////////////
//Date Created: Oct 14.2004
//Updates a list name record with status and hierarchy for the given listid
//////////////////////////////////////////////////////////////////////////////
LONG setListName2(GMS_ListName &data)
{
#define SQL_SETLISTNAME "\
   update LISTNMS\
   set LISTNAME=?,LISTDATE=?,LISTTYPE=?,LISTDESC=?, LISTSTATUS=?, LHIERARCHY=? \
   where LISTNMS.LISTID=?"

   PUSH(setListName);


static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_SETLISTNAME);

static GMS_ListName _data;
static BOOL first_time=TRUE;

static LONG id;

   if (first_time)
   {
      local.BindParam(1,_data.listname,sizeof(_data.listname));
      local.BindParam(2,_data.listdate);
      local.BindParam(3,_data.listtype,sizeof(_data.listtype));
      //local.BindParam(4,_data.listuid);
      local.BindParam(4,_data.listdesc,ICIS_MAX_SIZE);   //sizeof(_data.listdesc)
	  local.BindParam(5,_data.liststatus);
	  local.BindParam(6,_data.lhierarchy);
      local.BindParam(7,_data.listid);
 
      first_time=FALSE;
   }

   _data = data;

   if (local.Execute())
      ret=GMS_SUCCESS;
   else if (local.NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;
   POP();

#undef SQL_SETLISTNAME
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG getNextLID(void)
{
static LONG listID=0;

if (!listID) {
   CODBCdirectStmt source1 = _localDBC->DirectStmt("SELECT MIN(ULISTID) FROM INSTLN") ;
   source1.Execute();
   source1.Fetch();
   listID=source1.Field(1).AsInteger();
}

   CODBCdirectStmt source2 = _localDBC->DirectStmt("SELECT MIN(LISTID) FROM LISTNMS") ;
   source2.Execute();
   source2.Fetch();
   listID=min(listID,source2.Field(1).AsInteger());
   return --listID;

}





//////////////////////////////////////////////////////////////////////////////
//Created: October 14, 2004
//Add a list with status and hierarchy level
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_addListName(GMS_ListName *data)
{
   PUSH(GMS_addListName2);
   ret=addListName2(*data);
   POP();
}

//////////////////////////////////////////////////////////////////////////////
//Created: October 14, 2004
//Add a list with status and hierarchy level
//////////////////////////////////////////////////////////////////////////////
LONG addListName2(GMS_ListName &data)
{
#define SQL_ADDLISTNAME2 "\
   insert into LISTNMS\
   (LISTID,LISTNAME,LISTDATE,LISTTYPE,LISTUID,LISTDESC, LISTSTATUS, LHIERARCHY)\
   values (?, ?, ?, ?, ?, ?, ?, ?)"

   PUSH(addListName);

static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_ADDLISTNAME2);

static GMS_ListName _data;
static BOOL first_time=TRUE;
static char _szName[GMS_LIST_NAME];

static LONG id;



strncpy(_szName, data.listname, strlen(data.listname));
ZeroMemory(&_data,sizeof(_data));
_data.lhierarchy = data.lhierarchy;
strcpy(_data.listname, data.listname);
ret = GMS_existList(&_data);
if (ret == GMS_NO_DATA ) {

   ZeroMemory(&_data,sizeof(_data));
   if (first_time)
   {
      local.BindParam(1,_data.listid);
      local.BindParam(2,_data.listname,sizeof(_data.listname));
      local.BindParam(3,_data.listdate);
      local.BindParam(4,_data.listtype,sizeof(_data.listtype));
      local.BindParam(5,_data.listuid);
      local.BindParam(6,_data.listdesc,ICIS_MAX_SIZE );
      local.BindParam(7,_data.liststatus);
	  local.BindParam(8,_data.lhierarchy);

      first_time=FALSE;
   }

   data.listid = getNextLID();
   data.listuid = USERID;
   _data = data;
   LOGF("listid = %d",data.listid);

   if ((ret=local.Execute())==1)
      ret=GMS_SUCCESS;
   else 

   	   if (ret!=GMS_UNIQUE_CONSTRAINT_ERROR) //GMS_UNIQUE_CONSTRAINT_ERROR
		   ret=GMS_ERROR;  
       // Handler for unique contraint error with the assumption that only AID has unique contraint 
	   else {
		   int i=0;
		   while ((ret==GMS_UNIQUE_CONSTRAINT_ERROR) && (i<100)) {
                _data.listid = getNextLID();
				ret=local.Execute();
				++i;
		   }
		   if ((ret) &&(ret!=GMS_UNIQUE_CONSTRAINT_ERROR)) {
               data.listid= _data.listid;
			   ret = GMS_SUCCESS;
		   }
		   else  {
			   data.listid = 0;
			   ret = GMS_ERROR;
		   }
	   }

}
else {
	data.listid = _data.listid;
    ret = GMS_NAME_EXISTS;
}

   POP();

#undef SQL_ADDLISTNAME2
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_deleteListName(LONG lid)
{
   return deleteListName(lid);
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG deleteListName(LONG lid)
{
//#define SQL_DELETELISTNAME "delete from LISTNMS where LISTNMS.LISTID=?"
#define SQL_DELETELISTNAME "Update LISTNMS \
      set LISTSTATUS=9 \
      where LISTID=? "

static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_DELETELISTNAME);

static BOOL first_time=TRUE;

static LONG id;

   if (first_time)
   {
      local.BindParam(1,id);
 
      first_time=FALSE;
   }

   id=lid;

   if (local.Execute())
      return GMS_SUCCESS;
   else if (local.NoData())
      return GMS_NO_DATA;
   else 
      return GMS_ERROR;

#undef SQL_DELETELISTNAME
}





//////////////////////////////////////////////////////////////////////////////
// Date Created: Nov 2004
// Given: LISTID and LRECID
// LRECID is the system assigned ID and ENTRYID contains the sorting order
// Date Modified: Dec 7, 2004 (data are retrived from local & central database
//////////////////////////////////////////////////////////////////////////////
LONG getListData1_v2(BOOL fLocal,GMS_ListData &data,LONG fSearchOption)
{
#define SQL_GETLISTDATA1_v2 "\
   select LISTID,GID,LRECID,ENTRYCD,SOURCE,DESIG,GRPNAME, ENTRYID\
   from LISTDATA\
   where LISTID=? and (0=? or LRECID=?) AND LRSTATUS <> 9 \
   order by ENTRYID" 

   PUSH(getListData1);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_GETLISTDATA1_v2);
static CODBCbindedStmt &central  = _centralDBC->BindedStmt(SQL_GETLISTDATA1_v2);
       CODBCbindedStmt *source; 
static GMS_ListData _data;
static BOOL first_time=TRUE;

static LONG lid,eid;

   if (first_time)
   {
      BIND(1,_data.listid);
      BIND(2,_data.gid);
      BIND(3,_data.lrecid);
      BINDS(4,_data.entrycd,ICIS_MAX_SIZE );  //sizeof(_data.entrycd)
      BINDS(5,_data.source,ICIS_MAX_SIZE);
      BINDS(6,_data.desig,ICIS_MAX_SIZE);
      BINDS(7,_data.grpname,ICIS_MAX_SIZE);
      BIND(8,_data.entryid);
      BINDPARAM(1,lid);
	  BINDPARAM(2,eid);
	  BINDPARAM(3,eid);
 
      first_time=FALSE;
   }

   if (fSearchOption==FIND_FIRST){
      lid=data.listid;
	  eid=data.lrecid;
      local.Execute();
	  central.Execute();
   }
   source = (fLocal)?&local:&central;

   ZeroMemory(&_data,sizeof(_data));
   if (source->Fetch())
      data = _data;
   else if (source->NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;

   POP();

#undef SQL_GETLISTDATA1_v2
}


//////////////////////////////////////////////////////////////////////////////
// Date Created: Nov 2004
// Given: LISTID and GID
// LRECID is the system assigned ID and ENTRYID contains the sorting order
// Date Modified: Dec 7, 2004 (data are retrived from local & central database
//////////////////////////////////////////////////////////////////////////////
LONG getListData2_v2(BOOL fLocal,GMS_ListData &data,LONG fSearchOption)
{
#define SQL_GETLISTDATA2_v2 "\
   select LISTID,GID,LRECID,ENTRYCD,SOURCE,DESIG,GRPNAME, ENTRYID\
   from LISTDATA\
   where LISTID=? and GID=? AND LRSTATUS <> 9\
   order by ENTRYID"

   PUSH(getListData2);

static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_GETLISTDATA2_v2);
static CODBCbindedStmt &central  = _centralDBC->BindedStmt(SQL_GETLISTDATA2_v2);
       CODBCbindedStmt *source;
	   
static GMS_ListData _data;
static BOOL first_time=TRUE;

static LONG lid,gid;

   if (first_time)
   {
      BIND(1,_data.listid);
      BIND(2,_data.gid);
      BIND(3,_data.lrecid);
      BINDS(4,_data.entrycd,ICIS_MAX_SIZE);
      BINDS(5,_data.source,ICIS_MAX_SIZE);
      BINDS(6,_data.desig,ICIS_MAX_SIZE);
      BINDS(7,_data.grpname,ICIS_MAX_SIZE);
      BIND(8,_data.entryid);
      BINDPARAM(1,lid);
      BINDPARAM(2,gid);
 
      first_time=FALSE;
   }

   if (fSearchOption==FIND_FIRST){
      lid=data.listid;
	   gid=data.gid;
      local.Execute();
	  central.Execute();
   }
   source = (fLocal)?&local:&central;
   ZeroMemory(&_data,sizeof(_data));
   if (source->Fetch())
      data = _data;
   else if (source->NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;

   POP();

#undef SQL_GETLISTDATA2_v2
}



//////////////////////////////////////////////////////////////////////////////
// Date Created: Nov 2004
// Given: LISTID and ENTRYID
// LRECID is the system assigned ID and ENTRYID contains the sorting order
// Date Modified: Dec 7, 2004 (data are retrived from local & central database
//////////////////////////////////////////////////////////////////////////////
LONG getListData3_v2(BOOL fLocal, GMS_ListData &data,LONG fSearchOption)
{
#define SQL_GETLISTDATA3_v2 "\
   select LISTID,GID,LRECID,ENTRYCD,SOURCE,DESIG,GRPNAME, ENTRYID\
   from LISTDATA\
   where LISTID=? and  ENTRYID=? AND LRSTATUS <> 9 \
   order by ENTRYID" 

   PUSH(getListData3_v2);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_GETLISTDATA3_v2);
static CODBCbindedStmt &central  = _centralDBC->BindedStmt(SQL_GETLISTDATA3_v2);
       CODBCbindedStmt *source;  
static GMS_ListData _data;
static BOOL first_time=TRUE;

static LONG lid,eid;

   if (first_time)
   {
      BIND(1,_data.listid);
      BIND(2,_data.gid);
      BIND(3,_data.lrecid);
      BINDS(4,_data.entrycd,ICIS_MAX_SIZE);
      BINDS(5,_data.source,ICIS_MAX_SIZE);
      BINDS(6,_data.desig,ICIS_MAX_SIZE);
      BINDS(7,_data.grpname,ICIS_MAX_SIZE);
      BIND(8,_data.entryid);
      BINDPARAM(1,lid);
	  BINDPARAM(2,eid);
 
      first_time=FALSE;
   }

   if (fSearchOption==FIND_FIRST){
      lid=data.listid;
	  eid=data.entryid;
      local.Execute();
	  central.Execute();
   }

   source = (fLocal)?&local:&central;
   ZeroMemory(&_data,sizeof(_data));
   if (source->Fetch())
      data = _data;
   else if (source->NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;

   POP();

#undef SQL_GETLISTDATA3_v2
}

//////////////////////////////////////////////////////////////////////////////
// Date Created: Feb 15, 2006
// Given: LISTID and DESIG
//////////////////////////////////////////////////////////////////////////////
LONG getListDataByDesig(BOOL fLocal, GMS_ListData &data,LONG fSearchOption)
{
#define SQL_GETLISTDATA3_v2 "\
   select LISTID,GID,LRECID,ENTRYCD,SOURCE,DESIG,GRPNAME, ENTRYID\
   from LISTDATA\
   where LISTID=? and  DESIG= ? AND LRSTATUS <> 9 \
   order by ENTRYID" 

   PUSH(getListData3_v2);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_GETLISTDATA3_v2);
static CODBCbindedStmt &central  = _centralDBC->BindedStmt(SQL_GETLISTDATA3_v2);
       CODBCbindedStmt *source;  
static GMS_ListData _data;
static BOOL first_time=TRUE;

static LONG lid;
static CHAR desig[GMS_LIST_DESC];

   if (first_time)
   {
      strncpy(desig,data.desig,GMS_LIST_DESC);
      BIND(1,_data.listid);
      BIND(2,_data.gid);
      BIND(3,_data.lrecid);
      BINDS(4,_data.entrycd,ICIS_MAX_SIZE);
      BINDS(5,_data.source,ICIS_MAX_SIZE);
      BINDS(6,_data.desig,ICIS_MAX_SIZE);
      BINDS(7,_data.grpname,ICIS_MAX_SIZE);
      BIND(8,_data.entryid);
      BINDPARAM(1,lid);
	  BINDPARAMS(2,desig,GMS_LIST_DESC);
 
      first_time=FALSE;
   }

   if (fSearchOption==FIND_FIRST){
      lid=data.listid;
	  strncpy(desig,data.desig,GMS_LIST_DESC);
      local.Execute();
	  central.Execute();
   }

   source = (fLocal)?&local:&central;
   ZeroMemory(&_data,sizeof(_data));
   if (source->Fetch())
      data = _data;
   else if (source->NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;

   POP();

#undef SQL_GETLISTDATA3_v2
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG getListData_v2(GMS_ListData &data,LONG fSearchOption)
{
   static int fFlag=TRUE;
   static BOOL fLocal;
   PUSH(getListData);

   if (fSearchOption==FIND_FIRST) {
       fLocal=TRUE;
	   if (data.gid!=0) fFlag = 2; 
	   else if (data.entryid !=0) fFlag = 3;
	   else fFlag = 1;

   }

   if (fFlag==1) {
      if (fLocal){
         ret=getListData1_v2(LOCAL, data,fSearchOption);
         if (ret!=GMS_SUCCESS){
           fLocal = FALSE; 
           ret=getListData1_v2(CENTRAL,data,FIND_NEXT);
		 }
	  }
      else
        ret=getListData1_v2(CENTRAL, data,fSearchOption);
   }	   

   if (fFlag==2) {
      if (fLocal){
         ret=getListData2_v2(LOCAL, data,fSearchOption);
         if (ret!=GMS_SUCCESS){
           fLocal = FALSE; 
           ret=getListData2_v2(CENTRAL,data,FIND_NEXT);
		 }
	  }
      else
        ret=getListData2_v2(CENTRAL, data,fSearchOption);
   }

   if (fFlag==3) {
      if (fLocal){
         ret=getListData3_v2(LOCAL, data,fSearchOption);
         if (ret!=GMS_SUCCESS){
           fLocal = FALSE; 
           ret=getListData3_v2(CENTRAL,data,FIND_NEXT);
		 }
	  }
      else
        ret=getListData3_v2(CENTRAL, data,fSearchOption);
   }
   
   POP();

}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_getListData(GMS_ListData *data,LONG fSearchOption)
{
   PUSH(GMS_getListData2)
   ret=getListData_v2(*data,fSearchOption);
   POP();
}


//////////////////////////////////////////////////////////////////////////////
// Date Created: Jan 2006
// Given: LISTID and LRECID
// LRECID is the system assigned ID and ENTRYID contains the sorting order
//////////////////////////////////////////////////////////////////////////////
LONG getListDataStatus(BOOL fLocal,GMS_LDStatus &data,LONG fSearchOption)
{
#define SQL_GETLISTDATASTATUS "\
   select LISTID,GID,LRECID,ENTRYCD,SOURCE,DESIG,GRPNAME, ENTRYID, LRSTATUS\
   from LISTDATA\
   where LISTID=? and (0=? or LRECID=?) AND LRSTATUS <> 9 \
   order by ENTRYID" 

   PUSH(getListDataStatus);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_GETLISTDATASTATUS);
static CODBCbindedStmt &central  = _centralDBC->BindedStmt(SQL_GETLISTDATASTATUS);
       CODBCbindedStmt *source; 
static GMS_LDStatus _data;
static BOOL first_time=TRUE;

static LONG lid,eid;

   if (first_time)
   {
      BIND(1,_data.listid);
      BIND(2,_data.gid);
      BIND(3,_data.lrecid);
      BINDS(4,_data.entrycd,ICIS_MAX_SIZE );
      BINDS(5,_data.source,ICIS_MAX_SIZE);
      BINDS(6,_data.desig,ICIS_MAX_SIZE );
      BINDS(7,_data.grpname,ICIS_MAX_SIZE );
      BIND(8,_data.entryid);
	  BIND(9, _data.status);
      BINDPARAM(1,lid);
	  BINDPARAM(2,eid);
	  BINDPARAM(3,eid);
 
      first_time=FALSE;
   }

   if (fSearchOption==FIND_FIRST){
      lid=data.listid;
	  eid=data.lrecid;
      local.Execute();
	  central.Execute();
   }
   source = (fLocal)?&local:&central;

   ZeroMemory(&_data,sizeof(_data));
   if (source->Fetch())
      data = _data;
   else if (source->NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;

   POP();

#undef SQL_GETLISTDATASTATUS
}


//////////////////////////////////////////////////////////////////////////////
//Date Created: January 18, 2006
// Retrieves the list data record with status except for deleted records
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_getListData2(GMS_LDStatus *data,LONG fSearchOption)
{
   PUSH(GMS_getListData2)
    static BOOL fLocal=TRUE;


   if (fSearchOption==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal){
	  ret=getListDataStatus(LOCAL, *data,fSearchOption);
      if (ret!=GMS_SUCCESS){
         fLocal = FALSE; 
	     ret=getListDataStatus(CENTRAL, *data,fSearchOption);
      }
   }
   else
	     ret=getListDataStatus(CENTRAL, *data,fSearchOption);

   POP();

}


//////////////////////////////////////////////////////////////////////////////
//Date Created: February 15, 2006
// Retrieves the list data record for the given listid and desig
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_getListDataByDesig(GMS_ListData *data,LONG fSearchOption)
{
   PUSH(GMS_getListDataByDesig)
    static BOOL fLocal=TRUE;


   if (fSearchOption==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal){
	  ret=getListDataByDesig(LOCAL, *data,fSearchOption);
      if (ret!=GMS_SUCCESS){
         fLocal = FALSE; 
	     ret=getListDataByDesig(CENTRAL, *data,fSearchOption);
      }
   }
   else
	     ret=getListDataByDesig(CENTRAL, *data,fSearchOption);

   POP();

}

//////////////////////////////////////////////////////////////////////////////
//Description:  Finds all the list data records given a GID; 
//              LRECID is the system assigned ID
//Programmed:   Dec 2, 2004
//Date Modified: Dec 7, 2004 (data are retrived from local & central database
//////////////////////////////////////////////////////////////////////////////
LONG findListData2(BOOL fLocal, GMS_ListData &data,LONG fSearchOption)
{
#define SQL_FINDLISTDATA2 "\
   select LISTID,GID,ENTRYID,ENTRYCD,SOURCE,DESIG,GRPNAME, LRECID\
   from LISTDATA\
   where GID=? AND LRSTATUS<>9\
   order by LISTID"

   PUSH(findListDatav2);

static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_FINDLISTDATA2);
static CODBCbindedStmt &central  = _centralDBC->BindedStmt(SQL_FINDLISTDATA2);
       CODBCbindedStmt *source;

static GMS_ListData _data;
static BOOL first_time=TRUE;

static LONG gid;

  ZeroMemory(&_data,sizeof(_data));
  if (first_time)
   {
      BIND(1,_data.listid);
      BIND(2,_data.gid);
      BIND(3,_data.entryid);
      BINDS(4,_data.entrycd,ICIS_MAX_SIZE );
      BINDS(5,_data.source,ICIS_MAX_SIZE );
      BINDS(6,_data.desig,ICIS_MAX_SIZE );
      BINDS(7,_data.grpname,ICIS_MAX_SIZE );
	  BIND(8, _data.lrecid);
      BINDPARAM(1,gid);
 
      first_time=FALSE;
   }

   if (fSearchOption==FIND_FIRST){
      gid=data.gid;
      local.Execute();
	  central.Execute();
   }
   source = (fLocal)?&local:&central;

   if (source->Fetch())
      data = _data;
   else if (source->NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;

   POP();

#undef SQL_FINDLISTDATA2
}






//////////////////////////////////////////////////////////////////////////////
//Description:  Finds the all the list data records for a given GID
//Programmed:   July 21, 2003
//Data Modified: Dec 6, 2004 (data are retrieved from central and local database
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_findListData(GMS_ListData &data,LONG fSearchOption)
{
   PUSH(GMS_findListData);

   static BOOL fLocal=TRUE;


   if (fSearchOption==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal){
      ret=findListData2(LOCAL,data,fSearchOption);
      if (ret!=GMS_SUCCESS){
         fLocal = FALSE; 
         ret=findListData2(CENTRAL,data,FIND_NEXT);
      }
   }
   else
      ret=findListData2(CENTRAL,data,fSearchOption);

   POP();

}


//////////////////////////////////////////////////////////////////////////////
//Gets the next system assigned ID of an entry in a list
// The system assigned ID is stored in LRECID
// AMP (20080423): LRECID is globally assigned
//////////////////////////////////////////////////////////////////////////////
LONG getNextLRECID(LONG lid)
{
//#define SQL_GETNEXTENTRYID "SELECT MAX(LRECID) FROM LISTDATA WHERE LISTID=?"
#define SQL_GETNEXTENTRYID "SELECT MIN(LRECID) FROM LISTDATA"
   PUSH(getNextEntryID);

static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_GETNEXTENTRYID);
static BOOL first_time=TRUE;
static LONG _eid,_lid;

   if (first_time)
   {
      local.Bind(1,_eid);
//      local.BindParam(1,_lid);
 
      first_time=FALSE;
   }
   _eid = 0;
   _lid=lid;
   local.Execute();
   if (local.Fetch()){
      ret=_eid-1;
   }
   else if (local.NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;
   POP();

#undef SQL_GETNEXTENTRYID
}

//////////////////////////////////////////////////////////////////////////////
//Gets the next system assigned ID of an entry in a list
// The system assigned ID is stored in LRECID
//////////////////////////////////////////////////////////////////////////////
LONG getNextEntryID(LONG lid)
{
#define SQL_GETNEXTENTRYID "SELECT MAX(ENTRYID) FROM LISTDATA WHERE LISTID=? AND LRSTATUS<>9"
   PUSH(getNextEntryID);

static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_GETNEXTENTRYID);
static BOOL first_time=TRUE;
static LONG _eid,_lid;

   if (first_time)
   {
      local.Bind(1,_eid);
      local.BindParam(1,_lid);
 
      first_time=FALSE;
   }
   _eid = 0;
   _lid=lid;
   local.Execute();
   if (local.Fetch()){
      ret=_eid+1;
   }
   else if (local.NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;
   POP();

#undef SQL_GETNEXTENTRYID
}


//////////////////////////////////////////////////////////////////////////////
//Gets the next order of an entry in a list
// EntryID contains the sort order of the entries
//////////////////////////////////////////////////////////////////////////////
LONG getNextEntryIDv1(LONG lid)
{
#define SQL_GETNEXTENTRYID "SELECT MAX(ENTRYID) FROM LISTDATA WHERE LISTDATA.LISTID=?"
   PUSH(getNextEntryID);

static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_GETNEXTENTRYID);
static BOOL first_time=TRUE;
static LONG _eid,_lid;

   if (first_time)
   {
      local.Bind(1,_eid);
      local.BindParam(1,_lid);
 
      first_time=FALSE;
   }
   _eid = 0;
   _lid=lid;
   local.Execute();
   if (local.Fetch()){
      ret=_eid+1;
   }
   else if (local.NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;
   POP();

#undef SQL_GETNEXTENTRYID
}



//////////////////////////////////////////////////////////////////////////////
//Gets the next order of an entry in a list
// EntryID contains the sort order of the entries
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_getMaxEntryID(long listid)
{
   PUSH(GMS_getMaxEntryID);
   long entryid;
   entryid= getNextEntryID(listid);
   if (entryid == 0) entryid = 1;
   ret=entryid;
  
   POP();
}




//////////////////////////////////////////////////////////////////////////////
// Counts the number of entries in a list excluding textline and deleted entries 
// Date Created:  January 20, 2006
//////////////////////////////////////////////////////////////////////////////
LONG countListEntry(LONG lid)
{
#define SQL_COUNTLISTENTRIES "SELECT COUNT(ENTRYID) FROM LISTDATA WHERE LISTID=? AND LRSTATUS = 0"
#define SQL_COUNTLISTENTRIES "SELECT COUNT(ENTRYID) FROM LISTDATA WHERE LISTID=? AND LRSTATUS = 0"
PUSH(countListEntry);

static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_COUNTLISTENTRIES);
static CODBCbindedStmt &central  = _centralDBC->BindedStmt(SQL_COUNTLISTENTRIES);
       CODBCbindedStmt *source;

static BOOL first_time=TRUE;
static LONG _eid,_lid;

   if (first_time)
   {
      BIND(1,_eid);
      BINDPARAM(1,_lid);
 
      first_time=FALSE;
   }
   _eid = 0;
   _lid=lid;
   source = (lid <0)?&local:&central;
   source->Execute();
   if (source->Fetch()){
      ret=_eid;
   }
   else if (source->NoData()) {
	  ret=GMS_NO_DATA;
   }
   else 
      ret=GMS_ERROR;
   POP();

#undef SQL_COUNTLISTENTRIES
}


//////////////////////////////////////////////////////////////////////////////
// Counts the number of entries in a list excluding textline and deleted entries 
// Date Created:  January 20, 2006
// 
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_countListEntry(long listid)
{
   PUSH(GMS_countListEntry);
   long entryid;
   entryid= countListEntry(listid);
   ret=entryid;
  
   POP();
}


//////////////////////////////////////////////////////////////////////////////
//Date Modified: Dec 7, 2004 (Only local list can be updated)
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_setListData(GMS_ListData *data)
{
   PUSH(GMS_setListData2);
   if (data->listid <0)
      ret=setListData2(*data);
   else ret = GMS_NO_ACCESS;
   POP();
}


//////////////////////////////////////////////////////////////////////////////
//Date Created: Oct 14.2004
//Updates a list data record with entry order for the given lrecid and listid
//////////////////////////////////////////////////////////////////////////////
LONG setListData2(GMS_ListData &data)
{
#define SQL_SETLISTDATA2 "\
   update LISTDATA\
   set GID=?, ENTRYCD=?, SOURCE=?, DESIG=?,  GRPNAME=?, ENTRYID=?\
   where LISTDATA.LISTID=? and LISTDATA.LRECID=?"

   PUSH(setListData);

static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_SETLISTDATA2);

static GMS_ListData _data;
static BOOL first_time=TRUE;

   if (first_time)
   {
      local.BindParam(1,_data.gid);
      local.BindParam(2,_data.entrycd,ICIS_MAX_SIZE);
      local.BindParam(3,_data.source,ICIS_MAX_SIZE);
      local.BindParam(4,_data.desig,ICIS_MAX_SIZE);
      local.BindParam(5,_data.grpname,ICIS_MAX_SIZE);
      local.BindParam(6,_data.entryid);
      local.BindParam(7,_data.listid);
	  local.BindParam(8,_data.lrecid);
 
      first_time=FALSE;
   }

   _data =data;

   if (local.Execute())
      ret=GMS_SUCCESS;
   else if (local.NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;
   POP();


#undef SQL_SETLISTDATA2
}




//////////////////////////////////////////////////////////////////////////////
// Created: October 14, 2004
// Add list data with entry order
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_addListData(GMS_ListData *data)
{
   PUSH(GMS_addListData);
   if (data->listid < 0) 
      ret= addListData2(*data);
   else
	  ret= GMS_NO_ACCESS;
   if (ret && (AUTOCOMMIT)) {
		  GMS_commitData();
   }

   POP();
}


//////////////////////////////////////////////////////////////////////////////
// Created: October 14, 2004
// LRECID is the system assigned ID
//////////////////////////////////////////////////////////////////////////////
LONG addListData2(GMS_ListData &data)
{
#define SQL_ADDLISTDATA2 "\
   insert into LISTDATA\
   (LISTID,GID,LRECID,ENTRYCD,SOURCE,DESIG,GRPNAME,ENTRYID,LRSTATUS)\
   values (?, ?, ?, ?, ?, ?, ?,?,0)"

   PUSH(addListData);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_ADDLISTDATA2);

static GMS_ListData _data;
static BOOL first_time=TRUE;

   if (first_time)
   {
      local.BindParam(1,_data.listid);
      local.BindParam(2,_data.gid);
      local.BindParam(3,_data.lrecid);
      local.BindParam(4,_data.entrycd,GMS_LIST_ENTCD);
      local.BindParam(5,_data.source,ICIS_MAX_SIZE-1);
      local.BindParam(6,_data.desig,ICIS_MAX_SIZE-1);
      local.BindParam(7,_data.grpname,ICIS_MAX_SIZE-1);
      local.BindParam(8,_data.entryid);
      first_time=FALSE;
   }
   
   data.lrecid = getNextLRECID(data.listid);
   if (data.lrecid == 0 )  data.lrecid = -1;
   if (data.entryid ==0) data.entryid =  getNextEntryID(data.listid);

   _data=data;

   if ((ret=local.Execute())==1)
      ret=GMS_SUCCESS;   
//   else 
//      ret=GMS_ERROR;
    else  
	   if (ret!=GMS_UNIQUE_CONSTRAINT_ERROR) //GMS_UNIQUE_CONSTRAINT_ERROR
		   ret=GMS_ERROR;  
       // Handler for unique contraint error with the assumption that only STUDYID has unique contraint 
	   else {
		   int i=0;
		   while ((ret==GMS_UNIQUE_CONSTRAINT_ERROR) && (i<100)) {
				_data.lrecid = getNextLRECID(data.listid);
				ret=local.Execute();
				++i;
		   }
		   if ((ret) &&(ret!=GMS_UNIQUE_CONSTRAINT_ERROR)) {
               data = _data;
			   ret = DMS_SUCCESS;
		   }
		   else  {
			   data.lrecid= 0;
			   ret = DMS_ERROR;
		   }
	   }

	   POP();

#undef SQL_ADDLISTDATA2
}

//////////////////////////////////////////////////////////////////////////////
//Date Created: Dec 7, 2004
// Entry is deleted by changng the status of the record to 9
//////////////////////////////////////////////////////////////////////////////
LONG deleteListData2(LONG lid, LONG lrecid)
{
#define SQL_DELETELISTDATA "\
   update LISTDATA\
   set LRSTATUS=9 \
   where LISTDATA.LISTID=?   and (0=? or LRECID=?) "
// where LISTDATA.LISTID=?   and (0=? or LISTDATA.ENTRYID=?)"
   PUSH(deleteListData);

static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_DELETELISTDATA);

static BOOL first_time=TRUE;

static LONG _lid,_eid;

   if (first_time)
   {
	  local.BindParam(1,_lid);
      local.BindParam(1,_lid);
      local.BindParam(2,_eid);
      local.BindParam(3,_eid);
 
      first_time=FALSE;
   }

   _lid=lid;
   _eid = lrecid;

   if (local.Execute())
      ret=GMS_SUCCESS;
   else if (local.NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;
   POP();

#undef SQL_DELETELISTDATA
}


//////////////////////////////////////////////////////////////////////////////
//Date Modified: March 15, 2005
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_deleteListData(LONG lid,LONG lrecid)
{
   PUSH(GMS_deleteListData);
   ret=deleteListData2(lid,lrecid);
   POP();
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG deleteListData(LONG lid,LONG entryid)
{
#define SQL_DELETELISTDATA "\
   delete from LISTDATA\
   where LISTDATA.LISTID=? and (0=? or LISTDATA.ENTRYID=?)"
   PUSH(deleteListData);

static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_DELETELISTDATA);

static BOOL first_time=TRUE;

static LONG _lid,eid;

   if (first_time)
   {
      local.BindParam(1,_lid);
      local.BindParam(2,eid);
      local.BindParam(3,eid);
 
      first_time=FALSE;
   }

   _lid=lid;
   eid=entryid;

   if (local.Execute())
      ret=GMS_SUCCESS;
   else if (local.NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;
   POP();

#undef SQL_DELETELISTDATA
}


//////////////////////////////////////////////////////////////////////////////
// Created: January 18, 2006
// LRECID is the system assigned ID, LRSTATUS = 5 for textline
//////////////////////////////////////////////////////////////////////////////
LONG addListText(GMS_ListData &data)
{
#define SQL_ADDLISTTEXT "\
   insert into LISTDATA\
   (LISTID,GID,LRECID,ENTRYCD,SOURCE,DESIG,GRPNAME,ENTRYID,LRSTATUS)\
   values (?, ?, ?, ?, ?, ?, ?,?,5)"

   PUSH(addListData);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_ADDLISTTEXT);

static GMS_ListData _data;
static BOOL first_time=TRUE;

   if (first_time)
   {
      local.BindParam(1,_data.listid);
      local.BindParam(2,_data.gid);
      local.BindParam(3,_data.lrecid);
      local.BindParam(4,_data.entrycd,ICIS_MAX_SIZE);
      local.BindParam(5,_data.source,ICIS_MAX_SIZE);
      local.BindParam(6,_data.desig,ICIS_MAX_SIZE);
      local.BindParam(7,_data.grpname,ICIS_MAX_SIZE);
      local.BindParam(8,_data.entryid);
      first_time=FALSE;
   }
   
   data.lrecid = getNextLRECID(data.listid);
   if (data.lrecid == 0 )  data.lrecid =1;
   if (data.entryid ==0) data.entryid = data.lrecid;

   _data=data;

   if (local.Execute())
      ret=GMS_SUCCESS;   
   else 
      ret=GMS_ERROR;
   POP();

#undef SQL_ADDLISTTEXT
}

//////////////////////////////////////////////////////////////////////////////
// Created: January 18, 2006
// Add list data with entry order, LRSTATUS = 5 for text like
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_addListText(GMS_ListData *data)
{
   //PUSH(GMS_addListData);
   if (data->listid < 0) 
      return addListText(*data);
   else
	  return GMS_NO_ACCESS;
   //POP();
}



//**********************************************************************//
//******** OLD LISTDATA AND LISTNAME FUNCTIONS ************************//
/**

typedef struct TGMS_ListName{
    LONG          listid;
    CHAR          listname[GMS_LIST_NAME];
    LONG          listdate;
    CHAR          listtype[GMS_LIST_TYPE];
    LONG          listuid;
    CHAR          listdesc[GMS_LIST_DESC];
}           GMS_ListName;

typedef struct TGMS_ListData{
    LONG          listid,
                  gid,
                  entryid;
    CHAR          entrycd[GMS_LIST_ENTCD],
                  source[GMS_LIST_SOURC];
    CHAR          desig[GMS_LIST_DESIG];
    CHAR          grpname[GMS_LIST_GRPNM];
}           GMS_ListData;

//////////////////////////////////////////////////////////////////////////////
//Old getListName with no Hierarchy of lists
//////////////////////////////////////////////////////////////////////////////
LONG getListName(GMS_ListName &data,LONG fSearchOption)
{
#define SQL_GETLISTNAME	"\
   select LISTID,LISTNAME,LISTDATE,LISTTYPE,LISTUID,LISTDESC\
   from LISTNMS\
   where (0=? or LISTNMS.LISTID=?)" 

   PUSH(getListName);

static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_GETLISTNAME);

static GMS_ListName _data;
static BOOL first_time=TRUE;

static LONG id;

   if (first_time)
   {
      local.Bind(1,_data.listid);
      local.Bind(2,_data.listname,sizeof(_data.listname));
      local.Bind(3,_data.listdate);
      local.Bind(4,_data.listtype,sizeof(_data.listtype));
      local.Bind(5,_data.listuid);
      local.Bind(6,_data.listdesc,sizeof(_data.listdesc));
	  local.BindParam(1,id);
	  local.BindParam(2,id);
 
      first_time=FALSE;
   }

   if (fSearchOption==FIND_FIRST){
	   id=data.listid;
      local.Execute();
   }
   ZeroMemory(&_data,sizeof(_data));
   if (local.Fetch())
      data = _data;
   else if (local.NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;

   POP();

#undef SQL_GETLISTNAME
}


//////////////////////////////////////////////////////////////////////////////
//find a given list name
//Date Programmed:  July 16,2003
//////////////////////////////////////////////////////////////////////////////
LONG findListName(CHAR *szName,GMS_ListName &data,LONG fSearchOption)
{
#define SQL_FINDLISTNAME	"\
   select LISTID,LISTNAME,LISTDATE,LISTTYPE,LISTUID,LISTDESC\
   from LISTNMS\
   where (LISTNMS.LISTNAME=?)" 

   PUSH(findListName);

static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_FINDLISTNAME);

static GMS_ListName _data;
static BOOL first_time=TRUE;
static char _szName[GMS_LIST_NAME];

static LONG id;

   
   if (first_time)
   {
      local.Bind(1,_data.listid);
      local.Bind(2,_data.listname,sizeof(_data.listname));
      local.Bind(3,_data.listdate);
      local.Bind(4,_data.listtype,sizeof(_data.listtype));
      local.Bind(5,_data.listuid);
      local.Bind(6,_data.listdesc,sizeof(_data.listdesc));
      local.BindParam(1,_szName,GMS_LIST_NAME);
 
      first_time=FALSE;
   }

   if (fSearchOption==FIND_FIRST){
      strcpy(_szName, szName);
      local.Execute();
   }
   ZeroMemory(&_data,sizeof(_data));
   if (local.Fetch())
      data = _data;
   else if (local.NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;

   POP();

#undef SQL_FINDLISTNAME
}


//////////////////////////////////////////////////////////////////////////////
// old setListName with no List hierarchy
//////////////////////////////////////////////////////////////////////////////
LONG setListName(GMS_ListName &data)
{
#define SQL_SETLISTNAME "\
   update LISTNMS\
   set LISTNAME=?,LISTDATE=?,LISTTYPE=?,LISTUID=?,LISTDESC=? \
   where LISTNMS.LISTID=?"

   PUSH(setListName);


static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_SETLISTNAME);

static GMS_ListName _data;
static BOOL first_time=TRUE;

static LONG id;

   if (first_time)
   {
      local.BindParam(1,_data.listname,sizeof(_data.listname));
      local.BindParam(2,_data.listdate);
      local.BindParam(3,_data.listtype,sizeof(_data.listtype));
      local.BindParam(4,_data.listuid);
      local.BindParam(5,_data.listdesc,sizeof(_data.listdesc));
      local.BindParam(6,_data.listid);
 
      first_time=FALSE;
   }

   _data = data;

   if (local.Execute())
      ret=GMS_SUCCESS;
   else if (local.NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;
   POP();

#undef SQL_SETLISTNAME
}

//////////////////////////////////////////////////////////////////////////////
//Modified: July 17, 2003
//Modification:  Checks first if a list name exists, if so, returns GMS_EXISTS else the list name record is added to the database
// No Hierarchy of List
//////////////////////////////////////////////////////////////////////////////
LONG addListName(GMS_ListName &data)
{
#define SQL_ADDLISTNAME "\
   insert into LISTNMS\
   (LISTID,LISTNAME,LISTDATE,LISTTYPE,LISTUID,LISTDESC)\
   values (?, ?, ?, ?, ?, ?)"

   PUSH(addListName);

static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_ADDLISTNAME);

static GMS_ListName _data;
static BOOL first_time=TRUE;
static char _szName[GMS_LIST_NAME];

static LONG id;

if (strlen(data.listname) < 12)
    strcpy(_szName,data.listname);
else
    strncpy(_szName,data.listname,11);

ret = findListName((CHAR *)_szName,_data,0);
if (ret == GMS_NO_DATA ) {

   ZeroMemory(&_data,sizeof(_data));
   if (first_time)
   {
      local.BindParam(1,_data.listid);
      local.BindParam(2,_data.listname,sizeof(_data.listname));
      local.BindParam(3,_data.listdate);
      local.BindParam(4,_data.listtype,sizeof(_data.listtype));
      local.BindParam(5,_data.listuid);
      local.BindParam(6,_data.listdesc,sizeof(_data.listdesc));
 
      first_time=FALSE;
   }

   data.listid = getNextLID();
   data.listuid = USERID;
   _data = data;
   LOGF("listid = %d",data.listid);

   if (local.Execute())
      ret=GMS_SUCCESS;
   else 
      ret=GMS_ERROR;
}
else ret = GMS_NAME_EXISTS;

   POP();

#undef SQL_ADDLISTNAME
}



  //////////////////////////////////////////////////////////////////////////////
// Old GetListData given ListID and EntryID
//////////////////////////////////////////////////////////////////////////////
LONG getListData1(GMS_ListData &data,LONG fSearchOption)
{
#define SQL_GETLISTDATA1 "\
   select LISTID,GID,ENTRYID,ENTRYCD,SOURCE,DESIG,GRPNAME\
   from LISTDATA\
   where LISTID=? and (0=? or ENTRYID=?)\
   order by ENTRYID" 

   PUSH(getListData1);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_GETLISTDATA1);

static GMS_ListData _data;
static BOOL first_time=TRUE;

static LONG lid,eid;

   if (first_time)
   {
      local.Bind(1,_data.listid);
      local.Bind(2,_data.gid);
      local.Bind(3,_data.entryid);
      local.Bind(4,_data.entrycd,sizeof(_data.entrycd));
      local.Bind(5,_data.source,sizeof(_data.source));
      local.Bind(6,_data.desig,sizeof(_data.desig));
      local.Bind(7,_data.grpname,sizeof(_data.grpname));
      local.BindParam(1,lid);
	   local.BindParam(2,eid);
	   local.BindParam(3,eid);
 
      first_time=FALSE;
   }

   if (fSearchOption==FIND_FIRST){
      lid=data.listid;
	   eid=data.entryid;
      local.Execute();
   }
   ZeroMemory(&_data,sizeof(_data));
   if (local.Fetch())
      data = _data;
   else if (local.NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;

   POP();

#undef SQL_GETLISTDATA1
}

//////////////////////////////////////////////////////////////////////////////
// Old GetListData given ListID and GID
//////////////////////////////////////////////////////////////////////////////
LONG getListData2(GMS_ListData &data,LONG fSearchOption)
{
#define SQL_GETLISTDATA2 "\
   select LISTID,GID,ENTRYID,ENTRYCD,SOURCE,DESIG,GRPNAME\
   from LISTDATA\
   where LISTID=? and GID=?\
   order by ENTRYID"

   PUSH(getListData2);

static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_GETLISTDATA2);

static GMS_ListData _data;
static BOOL first_time=TRUE;

static LONG lid,gid;

   if (first_time)
   {
      local.Bind(1,_data.listid);
      local.Bind(2,_data.gid);
      local.Bind(3,_data.entryid);
      local.Bind(4,_data.entrycd,sizeof(_data.entrycd));
      local.Bind(5,_data.source,sizeof(_data.source));
      local.Bind(6,_data.desig,sizeof(_data.desig));
      local.Bind(7,_data.grpname,sizeof(_data.grpname));
      local.BindParam(1,lid);
	   local.BindParam(2,gid);
 
      first_time=FALSE;
   }

   if (fSearchOption==FIND_FIRST){
      lid=data.listid;
	   gid=data.gid;
      local.Execute();
   }
   ZeroMemory(&_data,sizeof(_data));
   if (local.Fetch())
      data = _data;
   else if (local.NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;

   POP();

#undef SQL_GETLISTDATA2
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG getListData(GMS_ListData &data,LONG fSearchOption)
{
   static BOOL fFlag=TRUE;
   PUSH(getListData);

   if (fSearchOption==FIND_FIRST)
      fFlag = data.gid==0;

   if (fFlag)
      ret=getListData1(data,fSearchOption);
   else
      ret=getListData2(data,fSearchOption);
   POP();

}


//////////////////////////////////////////////////////////////////////////////
//Description:  Finds all the list data records given a GID ;
//Programmed:   July 21, 2003
//////////////////////////////////////////////////////////////////////////////
LONG findListData(GMS_ListData &data,LONG fSearchOption)
{
#define SQL_FINDLISTDATA "\
   select LISTID,GID,ENTRYID,ENTRYCD,SOURCE,DESIG,GRPNAME\
   from LISTDATA\
   where GID=?\
   order by LISTID"

   PUSH(findListData);

static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_FINDLISTDATA);

static GMS_ListData _data;
static BOOL first_time=TRUE;

static LONG gid;

  ZeroMemory(&_data,sizeof(_data));
  if (first_time)
   {
      local.Bind(1,_data.listid);
      local.Bind(2,_data.gid);
      local.Bind(3,_data.entryid);
      local.Bind(4,_data.entrycd,sizeof(_data.entrycd));
      local.Bind(5,_data.source,sizeof(_data.source));
      local.Bind(6,_data.desig,sizeof(_data.desig));
      local.Bind(7,_data.grpname,sizeof(_data.grpname));
      local.BindParam(1,gid);
 
      first_time=FALSE;
   }

   if (fSearchOption==FIND_FIRST){
      gid=data.gid;
      local.Execute();
   }
   if (local.Fetch())
      data = _data;
   else if (local.NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;

   POP();

#undef SQL_FINDLISTDATA
}

//////////////////////////////////////////////////////////////////////////////
// Old SetListData with no LRECID
//////////////////////////////////////////////////////////////////////////////
LONG setListData(GMS_ListData &data)
{
#define SQL_SETLISTDATA "\
   update LISTDATA\
   set GID=?, ENTRYCD=?, SOURCE=?, DESIG=?,  GRPNAME=?\
   where LISTDATA.LISTID=? and LISTDATA.ENTRYID=?"

   PUSH(setListData);

static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_SETLISTDATA);

static GMS_ListData _data;
static BOOL first_time=TRUE;

   if (first_time)
   {
      local.BindParam(1,_data.gid);
      local.BindParam(2,_data.entrycd,sizeof(_data.entrycd));
      local.BindParam(3,_data.source,sizeof(_data.source));
      local.BindParam(4,_data.desig,sizeof(_data.desig));
      local.BindParam(5,_data.grpname,sizeof(_data.grpname));
      local.BindParam(6,_data.listid);
	   local.BindParam(7,_data.entryid);
 
      first_time=FALSE;
   }

   _data =data;

   if (local.Execute())
      ret=GMS_SUCCESS;
   else if (local.NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;
   POP();


#undef SQL_SETLISTDATA
}

//////////////////////////////////////////////////////////////////////////////
// Old AddListData
//////////////////////////////////////////////////////////////////////////////
LONG addListData(GMS_ListData &data)
{
#define SQL_ADDLISTDATA "\
   insert into LISTDATA\
   (LISTID,GID,ENTRYID,ENTRYCD,SOURCE,DESIG,GRPNAME)\
   values (?, ?, ?, ?, ?, ?, ?)"

   PUSH(addListData);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_ADDLISTDATA);

static GMS_ListData _data;
static BOOL first_time=TRUE;

   if (first_time)
   {
      local.BindParam(1,_data.listid);
      local.BindParam(2,_data.gid);
      local.BindParam(3,_data.entryid);
      local.BindParam(4,_data.entrycd,sizeof(_data.entrycd));
      local.BindParam(5,_data.source,sizeof(_data.source));
      local.BindParam(6,_data.desig,sizeof(_data.desig));
      local.BindParam(7,_data.grpname,sizeof(_data.grpname));
 
      first_time=FALSE;
   }
   
   data.entryid = getNextEntryIDv1(data.listid);
   _data=data;

   if (local.Execute())
      ret=GMS_SUCCESS;   
   else 
      ret=GMS_ERROR;
   POP();

#undef SQL_ADDLISTDATA
}
**/
