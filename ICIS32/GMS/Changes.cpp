/* Changes.cpp : Implements the functions to access and manipulate the CHANGES table of ICIS
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

#include "changes.h"

extern LONG UACCESS,USERID;
extern CODBCdbc *_centralDBC,*_localDBC;  //Central and Local Database Connection
extern stack<char*> _routines_stack;



//////////////////////////////////////////////////////////////////////////////
//
//GMS_Changes Structure:
//    LONG cid
//    LONG crecord
//    LONG cfrom
//    LONG cto
//    LONG cdate
//    LONG ctime
//    LONG cuid
//    LONG cref
//    CHAR ctable[GMS_MAX_TABLE_NAME]
//    CHAR cfield[GMS_MAX_FIELD_NAME]
//    CHAR cgroup[GMS_MAX_GROUP_NAME]
// where,
//    GMS_MAX_TABLE_NAME    16
//    GMS_MAX_FIELD_NAME    16
//    GMS_MAX_GROUP_NAME    20
//
//Syntax:
//   long GMS_addChanges(changes,szDesc)
//
//Parameter    Type				Comment
//---------    ----				------
//changes	   GMS_Changes *	(in/out) returns the CID,USERID, TIME, & DATE
//szDesc		   char *			(in) 
//
//Usage:
//   Add a Changes record. 
//Comment:
//   The fields crecord,cfrom,cto,ctable,cfield should be properly 
//    set/initialize. The routine will assign values to cid,cuid,cdate,
//    and ctime  fields
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_addChanges(GMS_Changes *data, CHAR *szDesc)
{
   PUSH(GMS_addChanges);
   if (UACCESS<30)
      ret= GMS_NO_ACCESS;
   else 
      ret= addChanges(*data,szDesc);
   POP();
}


LONG getNextCID(void)
{
   static LONG localCID=0;

   if (!localCID)
   {
      CODBCdirectStmt source1 = _localDBC->DirectStmt("SELECT MIN(UCID) FROM INSTLN") ;
      source1.Execute();
      source1.Fetch();
      localCID=source1.Field(1).AsInteger();
   }
   CODBCdirectStmt source2 = _localDBC->DirectStmt("SELECT MIN(CID) FROM CHANGES") ;
   source2.Execute();
   source2.Fetch();
   localCID=min(localCID,source2.Field(1).AsInteger());
  
   return --localCID;
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG addChanges(GMS_Changes &data, CHAR *szDesc)
{
#define SQL_ADDCHANGES "\
   insert into CHANGES\
   (CID,CRECORD,CTO,CFROM,CDATE,CTIME,CUID,CREF,CTABLE,CFIELD,CGROUP,CDESC,CSTATUS) \
   VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?,0)"

   PUSH(addChanges);

static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_ADDCHANGES);

static GMS_Changes _data;
static BOOL first_time=TRUE;
static CHAR _szDesc[255+1]; 

   if (first_time)
   {
      local.BindParam(1,_data.cid);
      local.BindParam(2,_data.crecord);
      local.BindParam(3,_data.cto);
      local.BindParam(4,_data.cfrom);
      local.BindParam(5,_data.cdate);
      local.BindParam(6,_data.ctime);
      local.BindParam(7,_data.cuid);
      local.BindParam(8,_data.cref);
      local.BindParam(9,_data.ctable,GMS_MAX_TABLE_NAME);
      local.BindParam(10,_data.cfield,GMS_MAX_FIELD_NAME);
      local.BindParam(11,_data.cgroup,GMS_MAX_GROUP_NAME);
      local.BindParam(12,_szDesc,255);
 
      first_time=FALSE;
   }
   data.cid = getNextCID();
   data.ctime = GMS_getTime();
   data.cdate = GMS_getDate();
   data.cuid = USERID;
   _data = data;
   if (szDesc)
   {
      strncpy(_szDesc,szDesc,255);
      _szDesc[255]='\0';
   }
   else
      _szDesc[0]='\0';
   

   if ((ret=local.Execute()) ==1)
	   ret = GMS_SUCCESS;
   else
   {
	   if (ret!=GMS_UNIQUE_CONSTRAINT_ERROR) //GMS_UNIQUE_CONSTRAINT_ERROR
		   ret=GMS_ERROR;  
       // Handler for unique contraint error with the assumption that only NID has unique contraint 
	   else {
		   int i=0;
		   while ((ret==GMS_UNIQUE_CONSTRAINT_ERROR) && (i<100)) {
				_data.cid = getNextCID();
				ret=local.Execute();
				++i;
		   }
		   if ((ret) &&(ret!=GMS_UNIQUE_CONSTRAINT_ERROR)) {
			   data.cid = _data.cid;
			   ret = GMS_SUCCESS;
		   }
		   else {
			   data.cid = 0;
			   ret = GMS_ERROR;
		   }
	   }
   }
   POP();

#undef SQL_ADDCHANGES
}


//////////////////////////////////////////////////////////////////////////////
//Syntax:
//   long GMS_deleteChanges(changes)
//
//Parameter    Type				Comment
//---------    ----				------
//changes	   GMS_Changes *	(in)
//
//Comment:
//   Records that can be deleted can be based on any combination of field
//   values of cid,crecord,cdate,ctime,cuid,cref,ctable,cfield, and cgroup. 
//   It is important to set all unneccessary fields to either 0 for numeric 
//   field, or null (empty string, byte(0)) for character field.
//Sample C Code:
//   LONG ret;
//   GMS_Changes c; 
//     ZeroMemory(&c,sizeof(c));  //Set all to field values to byte(0)
//     // Delete Changes Records made on GPID1 of GERMPLSM 
//     //  table dated 7 July 2000 at 11:30:20
//     strcpy(c.ctable,"GERMPLSM");
//     strcpy(c.cfield,"GPID1");
//     strcpy(c.cgroup,"2000071811302000");
//
//     ret=GMS_deleteChanges(&c);
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_deleteChanges(GMS_Changes &data)
{
   PUSH(GMS_deleteChanges);
   if (UACCESS<30) 
      ret=GMS_NO_ACCESS;
   else
      ret = deleteChanges(data);
   POP();
   return ret;

}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG deleteChanges(GMS_Changes &data)
{
#define SQL_DELETECHANGES "\
   delete from CHANGES where "

   PUSH(deleteChanges);

static CODBCbindedStmt &local  = _localDBC->BindedStmt();
       CODBCbindedStmt *source;


static string strSQL="";
static BOOL changed=TRUE;
static BOOL first_time=TRUE,fOK;
static BOOL fcid,fcrecord,fcdate,fctime,fcuid,fcref,fctable,fcfield,fcgroup;
static LONG cid,crecord,cdate,ctime,cuid,cref;
static CHAR ctable[GMS_MAX_TABLE_NAME],cfield[GMS_MAX_FIELD_NAME],cgroup[GMS_MAX_GROUP_NAME];

//   if (fSearchOption==FIND_FIRST)
   {
      changed = Changed(fcid     ,(cid      =data.cid)     !=0)            |
                Changed(fcrecord ,(crecord  =data.crecord) !=0)            |
                Changed(fcdate   ,(cdate    =data.cdate)   !=0)            |
                Changed(fctime   ,(ctime    =data.ctime)   !=0)            |
                Changed(fcuid    ,(cuid     =data.cuid)    !=0)            |
                Changed(fcref    ,(cref     =data.cref)    !=0)            |
                Changed(fctable  ,strlen(strcpy(ctable,data.ctable))!=0)   |
                Changed(fcfield  ,strlen(strcpy(cfield,data.cfield))!=0)   |
                Changed(fcgroup  ,strlen(strcpy(cgroup,data.cgroup))!=0);

      if (first_time)
      {
         changed=TRUE;
         first_time=FALSE;
      }

      if (changed)
      {
         strSQL=SQL_DELETECHANGES;
         if (fcid)
            strSQL = strSQL+" AND cid=?";
         if (fcrecord)
            strSQL = strSQL+" AND crecord=?";
         if (fcdate)
            strSQL = strSQL+" AND cdate=?";
         if (fctime)
            strSQL = strSQL+" AND ctime=?";
         if (fcuid)
            strSQL = strSQL+" AND cuid=?";
         if (fcref)
            strSQL = strSQL+" AND cref=?";
         if (fctable)
            strSQL = strSQL+" AND ctable=?";
         if (fcfield)
            strSQL = strSQL+" AND cfield=?";
         if (fcgroup)
            strSQL = strSQL+" AND cgroup=?";
         if (!(fcid||fcrecord||fcdate||fctime||fcuid||fcref||fctable||fcfield||fcgroup))
         {
            POP2(GMS_ERROR);
         }
         strSQL.erase(strSQL.find(" AND ",0),4);
         strSQL+= " and CSTATUS=0 order by CDATE desc, CTIME desc";

         //central.SetSQLstr((LPCSTR)strSQL.c_str());
         local.SetSQLstr((LPCSTR)strSQL.c_str());

         int i=1;
         if (fcid)
         {
            local.BindParam(i,cid);i+=1;
         }
         if (fcrecord)
         {
            local.BindParam(i,crecord);i+=1;
         }
         if (fcdate)
         {
            local.BindParam(i,cdate);i+=1;
         }
         if (fctime)
         {
            local.BindParam(i,ctime);i+=1;
         }
         if (fcuid)
         {
            local.BindParam(i,cuid);i+=1;
         }
         if (fcref)
         {
            local.BindParam(i,cref);i+=1;
         }
         if (fctable)
         {
            local.BindParam(i,ctable,GMS_MAX_TABLE_NAME);i+=1;
         }
         if (fcfield)
         {
            local.BindParam(i,cfield,GMS_MAX_FIELD_NAME);i+=1;
         }
         if (fcgroup)
         {
            local.BindParam(i,cgroup,GMS_MAX_GROUP_NAME);i+=1;
         }
      }
   }
 
   source = &local;//(fLocal)?&local:&central;

   cid      = data.cid;
   crecord  = data.crecord;
   cdate    = data.cdate;
   ctime    = data.ctime;
   cuid     = data.cuid;
   cref     = data.cref;
   strcpy(ctable,data.ctable);
   strcpy(cfield,data.cfield);
   strcpy(cgroup,data.cgroup);

   if (source->Execute())
      ret = GMS_SUCCESS;
   else if (source->NoData())
      ret = GMS_NO_DATA;
   else 
      ret = GMS_ERROR;

   POP();

#undef SQL_DELETECHANGES
}

//////////////////////////////////////////////////////////////////////////////
//Syntax:
//   long GMS_getChanges(cid,changes,szDesc,nszDesc)
//
//Parameter    Type				Comment
//---------    ----				------
//cid          long           (in)Changes Record ID
//changes	   GMS_Changes *	(out)
//szDesc       char *         (out) Description (can be NULL)
//nszDesc      long           (out) Max length of the szDesc can hold
//
//Usage:
//   Find a particular Changes made based on some criteria and returns the
//      changes structure record.
//////////////////////////////////////////////////////////////////////////////

LONG DLL_INTERFACE GMS_getChanges(LONG cid,GMS_Changes *data, LPSTR szDesc, LONG nszDesc)
{
   PUSH(GMS_getChanges);
   ret=getChanges(cid<0,cid,*data,szDesc,nszDesc);
   POP();
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG getChanges(BOOL fLocal, LONG cid,GMS_Changes &data, CHAR *szDesc, LONG nszDesc)
{
#define SQL_GETCHANGES "\
   select CRECORD, CFROM, CTO, CDATE, CTIME, CUID, CREF, CTABLE, CFIELD, CGROUP,CDESC\
   from CHANGES\
   where CID=? and CSTATUS=0 \
   order by CDATE desc, CTIME desc"


   PUSH(getChange);

static CODBCbindedStmt &central= _centralDBC->BindedStmt(SQL_GETCHANGES);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_GETCHANGES);
       CODBCbindedStmt *source;

static GMS_Changes _data;
static BOOL first_time=TRUE;
static CHAR _szDesc[MAX_STR];


//   if (fSearchOption==FIND_FIRST)
   {
      if (first_time)
      {
         BIND(1,_data.crecord);
         BIND(2,_data.cfrom);
         BIND(3,_data.cto);
         BIND(4,_data.cdate);
         BIND(5,_data.ctime);
         BIND(6,_data.cuid);
         BIND(7,_data.cref);
         BINDS(8,_data.ctable,GMS_MAX_TABLE_NAME);
         BINDS(9,_data.cfield,GMS_MAX_FIELD_NAME);
         BINDS(10,_data.cgroup,GMS_MAX_GROUP_NAME);
         BINDS(11,_szDesc,MAX_STR);
	      BINDPARAM(1,_data.cid);
 
         first_time=FALSE;
      }
      _data.cid=cid;

      local.Execute();
      central.Execute();
   }
 
   source = (fLocal)?&local:&central;

   _szDesc[0]='\0';
   if (source->Fetch())
   {
      if (szDesc)
      {
         strncpy(szDesc,_szDesc,nszDesc);
         szDesc[nszDesc-1]='\0';
      }
      data = _data;
      ret=GMS_SUCCESS;
   }
   else if (source->NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;

   POP();

#undef SQL_GETCHANGES
}


//////////////////////////////////////////////////////////////////////////////
//Syntax:
//   long GMS_findChanges(changes,szDesc,nszDesc)
//
//Parameter    Type				Comment
//---------    ----				------
//changes	   GMS_Changes *	(in/out)
//szDesc       char *         (out) Description (can be NULL to ignore return)
//nszDesc      long           (out) Max length of the szDesc can hold
//fSearchOption long          (in) FIND_FIRST(0), FIND_NEXT(1)
//
//Usage:
//   Find a particular Changes made based on some criteria and returns the
//      changes structure record.
//
//Comment:
//   Records that can be deleted can be based on any combination of field
//   values of crecord,cdate,ctime,cuid,cref,ctable,cfield, and cgroup. 
//   The changes structure fields must be properly initialize.
//
//////////////////////////////////////////////////////////////////////////////

LONG DLL_INTERFACE GMS_findChanges(GMS_Changes *data, LPSTR szDesc, LONG nszDesc
               , LONG fSearchOption)
{
   PUSH(GMS_findChanges);

   static BOOL fLocal=TRUE;

   if (fSearchOption==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal)
   {
      ret=findChanges(LOCAL,*data,szDesc,nszDesc,fSearchOption);
      if (ret!=GMS_SUCCESS){
         fLocal = FALSE; 
         ret=findChanges(CENTRAL,*data,szDesc,nszDesc,FIND_NEXT);
      }
   }
   else
      ret=findChanges(CENTRAL,*data,szDesc,nszDesc,fSearchOption);

   POP();

}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG findChanges(BOOL fLocal,GMS_Changes &data, LPSTR szDesc, LONG nszDesc
               , LONG fSearchOption)
{
#define SQL_FINDCHANGES "\
   select CID\
   from CHANGES where "

   PUSH(findChanges);

static CODBCbindedStmt &central= _centralDBC->BindedStmt();
static CODBCbindedStmt &local  = _localDBC->BindedStmt();
       CODBCbindedStmt *source;

static LONG _cid;

static string strSQL="";
static BOOL changed=TRUE;
static BOOL first_time=TRUE,fOK;
static BOOL fcrecord=0,fcdate=0,fctime=0,fcuid=0,fctable=0,fcfield=0,fcgroup=0;
static LONG crecord=0,cdate=0,ctime=0,cuid=0;
static CHAR ctable[GMS_MAX_TABLE_NAME]="",cfield[GMS_MAX_FIELD_NAME]="",cgroup[GMS_MAX_GROUP_NAME]="";

   if (fSearchOption==FIND_FIRST)
   {
      changed  = Changed(fcrecord ,(crecord  =data.crecord) !=0)
               | Changed(fcdate   ,(cdate    =data.cdate)  !=0)
               | Changed(fctime   ,(ctime    =data.ctime)   !=0)
               | Changed(fcuid    ,(cuid     =data.cuid)    !=0)
               | Changed(fctable  ,strlen(strcpy(ctable,data.ctable))!=0)
               | Changed(fcfield  ,strlen(strcpy(cfield,data.cfield))!=0)
               | Changed(fcgroup  ,strlen(strcpy(cgroup,data.cgroup))!=0);

      if (first_time)
         changed=TRUE;

      if (changed)
      {
         strSQL=SQL_FINDCHANGES;
         if (fcrecord)
            strSQL = strSQL+" AND crecord=?";
         if (fcdate)
            strSQL = strSQL+" AND cdate=?";
         if (fctime)
            strSQL = strSQL+" AND ctime=?";
         if (fcuid)
            strSQL = strSQL+" AND cuid=?";
         if (fctable)
            strSQL = strSQL+" AND ctable=?";
         if (fcfield)
            strSQL = strSQL+" AND cfield=?";
         if (fcgroup)
            strSQL = strSQL+" AND cgroup=?";
         if (!(fcrecord||fcdate||fctime||fcuid||fctable||fcfield||fcgroup))
         {
            POP2(GMS_ERROR);
         }
         strSQL.erase(strSQL.find(" AND ",0),4);
         strSQL+= " and CSTATUS=0 order by CDATE desc, CTIME desc";

         central.SetSQLstr((LPCSTR)strSQL.c_str());
         local.SetSQLstr((LPCSTR)strSQL.c_str());

         int i=1;
         if (fcrecord)
         {
            BINDPARAM(i,crecord);i+=1;
         }
         if (fcdate)
         {
            BINDPARAM(i,cdate);i+=1;
         }
         if (fctime)
         {
            BINDPARAM(i,ctime);i+=1;
         }
         if (fcuid)
         {
            BINDPARAM(i,cuid);i+=1;
         }
         if (fctable)
         {
            BINDPARAMS(i,ctable,GMS_MAX_TABLE_NAME);i+=1;
         }
         if (fcfield)
         {
            BINDPARAMS(i,cfield,GMS_MAX_FIELD_NAME);i+=1;
         }
         if (fcgroup)
         {
            BINDPARAMS(i,cgroup,GMS_MAX_GROUP_NAME);i+=1;
         }

         if (first_time)
         {
            BIND(1,_cid);
            first_time=FALSE;
         }
      }
      local.Execute();
      central.Execute();
   }
 
   source = fLocal?&local:&central;

   if (source->Fetch())
      ret = getChanges(fLocal,_cid,data,szDesc,nszDesc);
   else if (source->NoData())
      ret = GMS_NO_DATA;
   else 
      ret = GMS_ERROR;

   POP();
#undef SQL_FINDCHANGES
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG findChanges2(BOOL fLocal,GMS_Changes &data, LPSTR szDesc, LONG nszDesc
               , LONG fSearchOption)
{
#define SQL_FINDCHANGES "\
   select CID, CDESC \
   from CHANGES where "

   PUSH(findChanges);

static CODBCbindedStmt &central= _centralDBC->BindedStmt();
static CODBCbindedStmt &local  = _localDBC->BindedStmt();
       CODBCbindedStmt *source;

static LONG _cid;

static string strSQL="";
static BOOL changed=TRUE;
static BOOL first_time=TRUE,fOK;
static BOOL fcrecord=0,fcfrom=0,fcto=0,fcuid=0,fctable=0,fcfield=0,fcgroup=0;
static LONG crecord=0,cto=0,cfrom=0,cuid=0;
static CHAR ctable[GMS_MAX_TABLE_NAME]="",cfield[GMS_MAX_FIELD_NAME]="",cgroup[GMS_MAX_GROUP_NAME]="";
//static CHAR _szDesc[MAX_STR];

   if (fSearchOption==FIND_FIRST)
   {
      changed  = Changed(fcrecord ,(crecord  =data.crecord) !=0)
               | Changed(fcfrom   ,(cfrom    =data.cfrom)  !=0)
               | Changed(fcto     ,(cto    =data.cto)   !=0)
               | Changed(fcuid    ,(cuid     =data.cuid)    !=0)
               | Changed(fctable  ,strlen(strcpy(ctable,data.ctable))!=0)
               | Changed(fcfield  ,strlen(strcpy(cfield,data.cfield))!=0)
               | Changed(fcgroup  ,strlen(strcpy(cgroup,data.cgroup))!=0);

      if (first_time)
         changed=TRUE;

      if (changed)
      {
         strSQL=SQL_FINDCHANGES;
         if (fcrecord)
            strSQL = strSQL+" AND crecord=?";
         if (fcfrom)
            strSQL = strSQL+" AND cfrom=?";
         if (fcto)
            strSQL = strSQL+" AND cto=?";
         if (fcuid)
            strSQL = strSQL+" AND cuid=?";
         if (fctable)
            strSQL = strSQL+" AND ctable=?";
         if (fcfield)
            strSQL = strSQL+" AND cfield=?";
         if (fcgroup)
            strSQL = strSQL+" AND cgroup=?";
         if (!(fcrecord||fcfrom||fcto||fcuid||fctable||fcfield||fcgroup))
         {
            POP2(GMS_ERROR);
         }
         strSQL.erase(strSQL.find(" AND ",0),4);
         strSQL+= " and CSTATUS=0 order by CDATE desc, CTIME desc";

         central.SetSQLstr((LPCSTR)strSQL.c_str());
         local.SetSQLstr((LPCSTR)strSQL.c_str());

         int i=1;
         if (fcrecord)
         {
            BINDPARAM(i,crecord);i+=1;
         }
         if (fcfrom)
         {
            BINDPARAM(i,cfrom);i+=1;
         }
         if (fcto)
         {
            BINDPARAM(i,cto);i+=1;
         }
         if (fcuid)
         {
            BINDPARAM(i,cuid);i+=1;
         }
         if (fctable)
         {
            BINDPARAMS(i,ctable,GMS_MAX_TABLE_NAME);i+=1;
         }
         if (fcfield)
         {
            BINDPARAMS(i,cfield,GMS_MAX_FIELD_NAME);i+=1;
         }
         if (fcgroup)
         {
            BINDPARAMS(i,cgroup,GMS_MAX_GROUP_NAME);i+=1;
         }

         if (first_time)
         {
            BIND(1,_cid);
            //BINDS(2,_szDesc,MAX_STR);

            first_time=FALSE;
         }
      }
      local.Execute();
      central.Execute();
   }
 
   source = fLocal?&local:&central;

   //_szDesc[0]='\0';
   if (source->Fetch())
      ret = getChanges(fLocal,_cid,data,szDesc,nszDesc);
   else if (source->NoData())
      ret = GMS_NO_DATA;
   else 
      ret = GMS_ERROR;

   POP();
#undef SQL_FINDCHANGES2
}



//////////////////////////////////////////////////////////////////////////////
//Syntax:
//   long GMS_findChanges(changes,szDesc,nszDesc)
//
//Parameter    Type				Comment
//---------    ----				------
//changes	   GMS_Changes *	(in/out)
//szDesc       char *         (out) Description (can be NULL to ignore return)
//nszDesc      long           (out) Max length of the szDesc can hold
//fSearchOption long          (in) FIND_FIRST(0), FIND_NEXT(1)
//
//Usage:
//   Find a particular Changes made based on some criteria (including CTO and CFROM fields) and returns the
//      changes structure record.
//
//   The changes structure fields must be properly initialize.
//
//////////////////////////////////////////////////////////////////////////////

LONG DLL_INTERFACE GMS_findChanges2(GMS_Changes *data, LPSTR szDesc, LONG nszDesc
               , LONG fSearchOption)
{
   PUSH(GMS_findChanges2);

   static BOOL fLocal=TRUE;

   if (fSearchOption==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal)
   {
      ret=findChanges2(LOCAL,*data,szDesc,nszDesc,fSearchOption);
      if (ret!=GMS_SUCCESS){
         fLocal = FALSE; 
         ret=findChanges2(CENTRAL,*data,szDesc,nszDesc,FIND_NEXT);
      }
   }
   else
      ret=findChanges2(CENTRAL,*data,szDesc,nszDesc,fSearchOption);

   POP();

}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG findDeletedGermplasmFromChanges(LONG gid)
{
#define SQL_FINDDELETEDGERMFROMCHGS "\
   select CID\
   from CHANGES where CTABLE='GERMPLSM' and CFIELD='GID' and CRECORD=? and CRECORD=CFROM and CFROM=CTO and CSTATUS=0 \
   order by CDATE desc, CTIME desc"

   LONG ret=GMS_SUCCESS;//PUSH(findDeletedGermplasmFromChanges);

static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_FINDDELETEDGERMFROMCHGS);
       CODBCbindedStmt *source;

static LONG _gid,_cid;

static BOOL first_time=TRUE;

   if (first_time)
   {
      local.Bind(1,_cid);
      local.BindParam(1,_gid);
      first_time=FALSE;
   }
   _gid=gid;
   local.Execute();

   source = &local;//(fLocal)?&local:&central;

   if (source->Fetch())
      ret = GMS_SUCCESS;
   else if (source->NoData())
      ret = GMS_NO_DATA;
   else 
      ret = GMS_ERROR;

   return ret;//POP();
#undef SQL_FINDDELETEDGERMFROMCHGS
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG getReplacementGermplasmFromChanges(LONG gid,LONG &to)
{
#define SQL_GETREPGERMFROMCHGS "\
   select CTO\
   from CHANGES where CTABLE='GERMPLSM' and CFIELD='GID' and CRECORD=? and CRECORD=CFROM and CSTATUS=0 \
   order by CDATE desc, CTIME desc"

   LONG ret=GMS_SUCCESS;//PUSH(getReplacementGermplasmFromChanges);

static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_GETREPGERMFROMCHGS);
       CODBCbindedStmt *source;

static LONG _gid,_to;

static BOOL first_time=TRUE;

   if (first_time)
   {
      local.Bind(1,_to);
      local.BindParam(1,_gid);
      first_time=FALSE;
   }
   _gid=gid;
   local.Execute();

   source = &local;

   if (source->Fetch())
   {
      to=_to;
      ret = GMS_SUCCESS;
   }
   else if (source->NoData())
      ret = GMS_NO_DATA;
   else 
      ret = GMS_ERROR;

   return ret;//POP();
#undef SQL_GETREPGERMFROMCHGS
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG getGermplasmFieldFromChanges(LONG gid,LPCSTR szField,LONG &to)
{
#define SQL_GETGERMFIELDFROMCHGS "\
   select CTO\
   from CHANGES where CRECORD=? and CTABLE='GERMPLSM' and CFIELD=? and CSTATUS=0 \
   order by CDATE desc, CTIME desc"

   LONG ret=GMS_SUCCESS;//PUSH(getGermplasmFieldFromChanges);

static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_GETGERMFIELDFROMCHGS);
       CODBCbindedStmt *source;

static LONG _gid,_to;
static CHAR _field[GMS_MAX_FIELD_NAME]="";

static BOOL first_time=TRUE;

   if (first_time)
   {
      local.Bind(1,_to);
      local.BindParam(1,_gid);
      local.BindParam(2,_field,GMS_MAX_FIELD_NAME);
      first_time=FALSE;
   }

   _gid=gid;
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
#undef SQL_GETGERMFIELDFROMCHGS
}


LONG getGermplasmFieldsFromChanges(LONG gid,LONG &cfrom,LONG &cto,LPSTR szField,LONG nszField,LONG &cdate,LONG &ctime,LONG fSearchOptions)
{
#define SQL_GETGERMFIELDSFROMCHGS "\
   select CFROM,CTO,CDATE,CTIME,CFIELD\
   from CHANGES where CTABLE='GERMPLSM' and CRECORD=? and CSTATUS=0 \
   order by CDATE desc, CTIME desc"

   LONG ret=GMS_SUCCESS;//PUSH(getGermplasmFieldFromChanges);

static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_GETGERMFIELDSFROMCHGS);

static LONG _gid,_cfrom,_cto,_cdate,_ctime;
static CHAR _cfield[GMS_MAX_FIELD_NAME]="";

static BOOL first_time=TRUE;

   if (fSearchOptions==FIND_FIRST)
   {
      if (first_time)
      {
         local.Bind(1,_cfrom);
         local.Bind(2,_cto);
         local.Bind(3,_cdate);
         local.Bind(4,_ctime);
         local.Bind(5,_cfield,GMS_MAX_FIELD_NAME);
         local.BindParam(1,_gid);
         first_time=FALSE;
      }
      _gid=gid;
      local.Execute();
   }


   _cfield[0]='\0';
   if (local.Fetch())
   {
      cfrom=_cfrom;
      cto=_cto;
      cdate=_cdate;
      ctime=_ctime;
      strncpy(szField,_cfield,nszField);
      szField[nszField-1]='\0';
      ret = GMS_SUCCESS;
   }
   else if (local.NoData())
      ret = GMS_NO_DATA;
   else 
      ret = GMS_ERROR;

   return ret;//POP();
#undef SQL_GETGERMFIELDSFROMCHGS
}


//////////////////////////////////////////////////////////////////////////////
//Created:  Nov 19, 2003
//Description:  Get the change made to the a record in the names table
// Input:  NID, FIELD NAME
//////////////////////////////////////////////////////////////////////////////
LONG getNameFieldFromChanges(LONG nid,LPCSTR szField,LONG &to)
{
#define SQL_GETGERMFIELDFROMCHGS "\
   select CTO\
   from CHANGES where CRECORD=? and CTABLE='NAMES' and CFIELD=? and CSTATUS=0 \
   order by CDATE desc, CTIME desc"

   LONG ret=GMS_SUCCESS;//PUSH(getGermplasmFieldFromChanges);

static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_GETGERMFIELDFROMCHGS);
       CODBCbindedStmt *source;

static LONG _nid,_to;
static CHAR _field[GMS_MAX_FIELD_NAME]="";

static BOOL first_time=TRUE;

   if (first_time)
   {
      local.Bind(1,_to);
      local.BindParam(1,_nid);
      local.BindParam(2,_field,GMS_MAX_FIELD_NAME);
      first_time=FALSE;
   }

   _nid=nid;
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
#undef SQL_GETGERMFIELDFROMCHGS
}
