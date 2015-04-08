/* Study.cpp : Implements the functions to access and manipulate the STUDY table of ICIS
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


#include "Study.h"

extern LONG UACCESS,USERID;
extern CODBCdbc *_centralDMS,*_localDMS;  //Central and Local Database Connection
extern stack<char*> _routines_stack;

//////////////////////////////////////////////////////////////////////////////
//  Finds the minimum  studyid
//////////////////////////////////////////////////////////////////////////////
LONG getNextStudyID(void)
{
static LONG studyID=0;

   //if (!studyID)
   //{
      CODBCdirectStmt local = _localDMS->DirectStmt("SELECT MIN(STUDYID) FROM STUDY");
      local.Execute();
      local.Fetch();
      if (!local.NoData())
         studyID=local.Field(1).AsInteger();
   //}
   return --studyID;
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG findStudy(BOOL fLocal, CHAR *szName, DMS_STUDY &data, LONG fSearchOption)
{
#define SQL_FINDSTUDY  \
	"select S.STUDYID, S.SNAME, S.PMKEY, S.TITLE, S.INVESTID, \
              S.STYPE, S.SDATE, S.EDATE, S.USERID, S.SSTATUS, S.SHIERARCHY \
             from STUDY S "

#define SNAME_EQ_SQL  " WHERE  S.SNAME = ? " 
#define SNAME_LIKE_SQL " WHERE  S.SNAME LIKE ? "

#define SQL_FINDSTUDY_ " order by S.SNAME asc "

   PUSH(findStudy);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_FINDSTUDY);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_FINDSTUDY);
       CODBCbindedStmt *source;
static CHAR strname[MAX_STR];

static DMS_STUDY _data;
static BOOL name_wild;

static BOOL first_time=TRUE;

   if (fSearchOption==FIND_FIRST){
      ZeroMemory(&_data,sizeof(_data));
      string stmt_sql;
	  ZeroFill(strname,sizeof(strname));
      strcpy(strname,szName);
	  name_wild =strchr(strname,'_') || strchr(strname,'%');
	  stmt_sql=SQL_FINDSTUDY;
	  stmt_sql+=(name_wild?SNAME_LIKE_SQL:SNAME_EQ_SQL);
	  stmt_sql+=SQL_FINDSTUDY_;
	  central.SetSQLstr(stmt_sql.c_str());
	  local.SetSQLstr(stmt_sql.c_str());

 //     if (first_time)
 //     {
         BIND(1,_data.STUDYID);
         BINDS(2,_data.SNAME, DMS_STUDY_NAME);
         BIND(3,_data.PMKEY);
         BINDS(4,_data.TITLE,DMS_STUDY_TITLE);
		 BIND(5,_data.INVESTID);
		 BINDS(6,_data.STYPE, DMS_STUDY_STYPE);
		 BIND(7,_data.SDATE);
         BIND(8,_data.EDATE);
         BIND(9,_data.USERID);
         BIND(10,_data.SSTATUS);
         BIND(11,_data.SHIERARCHY);
         BINDPARAMS(1, strname, DMS_STUDY_NAME);
         first_time=FALSE;
 //    }

      local.Execute();
      central.Execute();
   }

   source = (fLocal)?&local:&central;

   if (source->Fetch())
      data = _data;
   else if (source->NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;

   POP();
#undef SQL_FINDSTUDY
#undef SNAME_EQ_SQL 
#undef SNAME_LIKE_SQL 
#undef SQL_FINDSTUDY_
}



//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG findStudyEq(BOOL fLocal, CHAR *szName, DMS_STUDY &data, LONG fSearchOption)
{
#define SQL_FINDSTUDY_EQ  \
	"select S.STUDYID, S.SNAME, S.PMKEY, S.TITLE, S.INVESTID, \
              S.STYPE, S.SDATE, S.EDATE, S.USERID, S.SSTATUS, S.SHIERARCHY \
             from STUDY S  WHERE  S.SNAME = ? order by S.SNAME asc "

   PUSH(findStudyEq);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_FINDSTUDY_EQ);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_FINDSTUDY_EQ);
       CODBCbindedStmt *source;
static CHAR strname[MAX_STR];

static DMS_STUDY _data;
static BOOL name_wild;

static BOOL first_time=TRUE;

   if (fSearchOption==FIND_FIRST){
      ZeroMemory(&_data,sizeof(_data));
	  ZeroFill(strname,sizeof(strname));
      strcpy(strname,szName);

      if (first_time)
      {
         BIND(1,_data.STUDYID);
         BINDS(2,_data.SNAME, DMS_STUDY_NAME);
         BIND(3,_data.PMKEY);
         BINDS(4,_data.TITLE,DMS_STUDY_TITLE);
		 BIND(5,_data.INVESTID);
		 BINDS(6,_data.STYPE, DMS_STUDY_STYPE);
		 BIND(7,_data.SDATE);
         BIND(8,_data.EDATE);
         BIND(9,_data.USERID);
         BIND(10,_data.SSTATUS);
         BIND(11,_data.SHIERARCHY);
		 BINDPARAMS(1, strname, DMS_STUDY_NAME);
         first_time=FALSE;
     }

      local.Execute();
      central.Execute();
   }

   source = (fLocal)?&local:&central;

   if (source->Fetch())
      data = _data;
   else if (source->NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;

   POP();
#undef SQL_FINDSTUDY_EQ
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG findStudyLike(BOOL fLocal, CHAR *szName, DMS_STUDY &data, LONG fSearchOption)
{
#define SQL_FINDSTUDY_LIKE  \
	"select S.STUDYID, S.SNAME, S.PMKEY, S.TITLE, S.INVESTID, \
              S.STYPE, S.SDATE, S.EDATE, S.USERID, S.SSTATUS, S.SHIERARCHY  \
             from STUDY S  WHERE  S.SNAME LIKE ? order by S.SNAME asc "

   PUSH(findStudyEq);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_FINDSTUDY_LIKE);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_FINDSTUDY_LIKE);
       CODBCbindedStmt *source;
static CHAR strname[MAX_STR];

static DMS_STUDY _data;
static BOOL name_wild;

static BOOL first_time=TRUE;

   if (fSearchOption==FIND_FIRST){
      ZeroMemory(&_data,sizeof(_data));
	  ZeroFill(strname,sizeof(strname));
      strcpy(strname,szName);

      if (first_time)
      {
         BIND(1,_data.STUDYID);
         BINDS(2,_data.SNAME, DMS_STUDY_NAME);
         BIND(3,_data.PMKEY);
         BINDS(4,_data.TITLE,DMS_STUDY_TITLE);
		 BIND(5,_data.INVESTID);
		 BINDS(6,_data.STYPE, DMS_STUDY_STYPE);
		 BIND(7,_data.SDATE);
         BIND(8,_data.EDATE);
         BIND(9,_data.USERID);
         BIND(10,_data.SSTATUS);
         BIND(11,_data.SHIERARCHY);
         BINDPARAMS(1, strname, DMS_STUDY_NAME);
         first_time=FALSE;
     }

      local.Execute();
      central.Execute();
   }

   source = (fLocal)?&local:&central;

   if (source->Fetch())
      data = _data;
   else if (source->NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;

   POP();
#undef SQL_FINDSTUDY_LIKE
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG addStudy(DMS_STUDY &data, CHAR *szAval)
{
#define SQL_ADDSTUDY "\
     INSERT INTO STUDY \
             (STUDYID, SNAME, PMKEY, TITLE, OBJECTIV, INVESTID, \
              STYPE, SDATE, EDATE, USERID, SSTATUS, SHIERARCHY ) \
             values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?) "
   PUSH(addSTUDY);

static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_ADDSTUDY);

static DMS_STUDY _data;
static CHAR szObj[MAX_STR];
static BOOL first_time=TRUE;

   ret = DMS_findStudy(data.SNAME,&_data,FIND_FIRST);
   if (ret==DMS_NO_DATA) {
    data.STUDYID = getNextStudyID();
	strncpy((char *) szObj, szAval, strlen(szAval));
    if (first_time)
    {
      local.BindParam(1,_data.STUDYID);
      local.BindParam(2,_data.SNAME, sizeof(_data.SNAME));
      local.BindParam(3,_data.PMKEY);
      local.BindParam(4,_data.TITLE, sizeof(_data.TITLE)-1);
      //local.BindParam(5, szAval,strlen(szAval)+1);
	  local.BindParam(5, szObj,strlen(szObj));
      local.BindParam(6,_data.INVESTID);
      local.BindParam(7,_data.STYPE, sizeof(_data.STYPE));
      local.BindParam(8,_data.SDATE); 
      local.BindParam(9,_data.EDATE);
	  local.BindParam(10,_data.USERID);
      first_time=FALSE;
	}
    _data= data;
	_data.USERID = USERID;

    if (local.Execute())
      ret=DMS_SUCCESS;
    else
      ret=DMS_ERROR;

   }
   else {
	   if (ret==DMS_SUCCESS) {   
   	     data = _data;                        //return the entire record of array
	     POP2(DMS_EXIST);
	   }
	   else  POP2(DMS_ERROR);
   }
   POP();

#undef SQL_ADDSTUDY
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG setStudy(DMS_STUDY &data, CHAR *szAval)
{
#define SQL_SETSTUDY "\
        UPDATE STUDY \
        SET   PMKEY=?, TITLE=?, OBJECTIV=?, INVESTID=?, \
              STYPE=?, SDATE=?, EDATE=?, USERID=?, SNAME=?, SSTATUS=?, SHIERARCHY=? ) \
        WHERE STUDYID = ? "

   PUSH(setSTUDY);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_SETSTUDY);

static DMS_STUDY _data;
static CHAR szObj[MAX_STR];
static BOOL first_time=TRUE;

    _data = data;
    strncpy((char *) szObj, szAval, strlen(szAval));
    if (first_time)
    {
      local.BindParam(1,_data.PMKEY);
      local.BindParam(2,_data.TITLE, sizeof(_data.TITLE)-1);
	  local.BindParam(3, szObj,strlen(szObj));
      local.BindParam(4,_data.INVESTID);
      local.BindParam(5,_data.STYPE, sizeof(_data.STYPE));
      local.BindParam(6,_data.SDATE); 
      local.BindParam(7,_data.EDATE);
      local.BindParam(8,_data.STUDYID);
      local.BindParam(9,_data.USERID);
      local.BindParam(10,_data.SNAME, sizeof(_data.SNAME));
	  local.BindParam(11,_data.SSTATUS);
	  local.BindParam(12, _data.SHIERARCHY);
      first_time=FALSE;
    }
   if (local.Execute())
      ret=DMS_SUCCESS;
   else if (local.NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;
   POP();


#undef SQL_SETSTUDY
}



long deleteStudy(long studyid)
{
#define SQL_DeleteStudy "DELETE FROM OINDEX WHERE OINDEX.FACTORID IN (SELECT FACTORID FROM FACTOR WHERE STUDYID = ?)"

    PUSH(DeleteStudy); 
    long  tmp=0;
    CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_DeleteStudy); 
	local.BindParam(1,studyid);
    if (local.Execute())
      ret=DMS_SUCCESS;
    else
      ret=DMS_ERROR;

    local.SetSQLstr("DELETE FROM DMSATTR  WHERE DMSATTR.DMSATYPE=802 AND DMSATTR.DMSATREC IN (SELECT VARIATID FROM VARIATE WHERE STUDYID = ?) ");
	local.BindParam(1,studyid);
    if (local.Execute())
      ret=DMS_SUCCESS;
    else
      ret=DMS_ERROR;

    local.SetSQLstr("DELETE FROM DMSATTR  WHERE DMSATTR.DMSATYPE=801 AND DMSATTR.DMSATREC IN (SELECT LABELID FROM FACTOR WHERE STUDYID = ?)");
	local.BindParam(1,studyid);
    if (local.Execute())
      ret=DMS_SUCCESS;
    else
      ret=DMS_ERROR;


    local.SetSQLstr("DELETE FROM LEVEL_C WHERE FACTORID IN (SELECT FACTORID FROM FACTOR WHERE STUDYID = ?) ");
	local.BindParam(1,studyid);
    if (local.Execute())
      ret=DMS_SUCCESS;
    else
      ret=DMS_ERROR;

	local.SetSQLstr("DELETE FROM LEVEL_N WHERE FACTORID IN (SELECT FACTORID FROM FACTOR WHERE STUDYID = ?) ");
	local.BindParam(1,studyid);
    if (local.Execute())
      ret=DMS_SUCCESS;
    else
      ret=DMS_ERROR;


    local.SetSQLstr("DELETE FROM EFFECT WHERE FACTORID IN (SELECT LABELID FROM FACTOR WHERE STUDYID = ?) ");
	local.BindParam(1,studyid);
    if (local.Execute())
      ret=DMS_SUCCESS;
    else
      ret=DMS_ERROR;


    local.SetSQLstr("DELETE FROM FACTOR WHERE STUDYID = ? ");
	local.BindParam(1,studyid);
    if (local.Execute())
      ret=DMS_SUCCESS;
    else
      ret=DMS_ERROR;

    local.SetSQLstr("DELETE FROM DATA_N WHERE VARIATID IN (SELECT VARIATID FROM VARIATE WHERE STUDYID = ?) ");
	local.BindParam(1,studyid);
    if (local.Execute())
      ret=DMS_SUCCESS;
    else
      ret=DMS_ERROR;

    local.SetSQLstr("DELETE  FROM DATA_C WHERE VARIATID IN (SELECT VARIATID FROM VARIATE WHERE STUDYID = ?) ");
	local.BindParam(1,studyid);
    if (local.Execute())
      ret=DMS_SUCCESS;
    else
      ret=DMS_ERROR;

    local.SetSQLstr("DELETE FROM VEFFECT WHERE VARIATID IN (SELECT VARIATID FROM VARIATE WHERE STUDYID = ?) ");
	local.BindParam(1,studyid);
    if (local.Execute())
      ret=DMS_SUCCESS;
    else
      ret=DMS_ERROR;




    local.SetSQLstr("DELETE FROM VARIATE WHERE STUDYID = ? ");
	local.BindParam(1,studyid);
    if (local.Execute())
      ret=DMS_SUCCESS;
    else
      ret=DMS_ERROR;

    local.SetSQLstr("DELETE FROM STUDY WHERE STUDYID = ? ");
	local.BindParam(1,studyid);
    if (local.Execute())
      ret=DMS_SUCCESS;
    else
      ret=DMS_ERROR;

	POP();
}

/********
*	getStudy - function to get all Study info from the central 
*				and local database.
*********/

LONG  getStudy(BOOL fLocal ,DMS_STUDY* data, UCHAR* szMDesc, LONG cMDesc, int fOpt)
{

#define SQL_GETSTUDY \
  "SELECT  STUDYID,SNAME,PMKEY,TITLE,OBJECTIV,INVESTID,STYPE,SDATE,EDATE, USERID, SSTATUS, SHIERARCHY  "\
							"FROM STUDY "\
							"WHERE (STUDYID=? OR 0=?) AND (SHIERARCHY =? OR 0=?)"

  PUSH(getStudy);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_GETSTUDY);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_GETSTUDY);
       CODBCbindedStmt *source;

static DMS_STUDY _data;
static LONG studyid, shierar;
static BOOL first_time=TRUE;
static CHAR szDesc[MAX_STR];

//Parameters
   if (fOpt == FIND_FIRST) {
      if (first_time)
	  {
		BINDPARAM(1, studyid);
		BINDPARAM(2, studyid);
		BINDPARAM(3, shierar);
		BINDPARAM(4, shierar);
												
         BIND(1,_data.STUDYID);
         BINDS(2,_data.SNAME, DMS_STUDY_NAME);
         BIND(3,_data.PMKEY);
         BINDS(4,_data.TITLE,DMS_STUDY_TITLE);
	 	 BINDS(5, szDesc, MAX_STR);
		 BIND(6,_data.INVESTID);
		 BINDS(7,_data.STYPE, DMS_STUDY_STYPE);
		 BIND(8,_data.SDATE);
         BIND(9,_data.EDATE);
         BIND(10,_data.USERID);
         BIND(11,_data.SSTATUS);
         BIND(12,_data.SHIERARCHY);

      first_time = FALSE;
	  }
	 shierar = data->SHIERARCHY;
     studyid = data->STUDYID;
     local.Execute();
     central.Execute();
   }

   source = (fLocal)?&local:&central;
   if (source->Fetch())
   {
      ret=GMS_SUCCESS;
	  *data = _data;
	  if (sizeof(szDesc) < cMDesc)
         strncpy((CHAR*) szMDesc,szDesc,sizeof(szDesc));
      else
         strncpy((CHAR*) szMDesc,szDesc,cMDesc);
   }   
   else if (source->NoData())
      ret = DMS_NO_DATA;
   else 
      ret = DMS_ERROR;

   POP();

#undef SQL_GETSTUDY

}


/********
*	getStudy2 - function to get all Study info from the central 
*				and local database for the log in user
*********/

LONG  getStudy2(BOOL fLocal ,DMS_STUDY* data, UCHAR* szMDesc, LONG cMDesc, int fOpt)
{

#define SQL_GETSTUDY2 \
  "SELECT  STUDYID,SNAME,PMKEY,TITLE,OBJECTIV,INVESTID,STYPE,SDATE,EDATE, USERID, SSTATUS, SHIERARCHY "\
							"FROM STUDY "\
							"WHERE (USERID=? OR 0=?)"

  PUSH(getStudy2);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_GETSTUDY2);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_GETSTUDY2);
       CODBCbindedStmt *source;

static DMS_STUDY _data;
static LONG userid;
static BOOL first_time=TRUE;
static CHAR szDesc[MAX_STR];

//Parameters
   if (fOpt == FIND_FIRST) {
      if (first_time)
	  {
		BINDPARAM(1, userid);
		BINDPARAM(2, userid);
												
         BIND(1,_data.STUDYID);
         BINDS(2,_data.SNAME, DMS_STUDY_NAME);
         BIND(3,_data.PMKEY);
         BINDS(4,_data.TITLE,DMS_STUDY_TITLE);
	 	 BINDS(5, szDesc, MAX_STR);
		 BIND(6,_data.INVESTID);
		 BINDS(7,_data.STYPE, DMS_STUDY_STYPE);
		 BIND(8,_data.SDATE);
         BIND(9,_data.EDATE);
         BIND(10,_data.USERID);
         BIND(11,_data.SSTATUS);
         BIND(12,_data.SHIERARCHY);

      first_time = FALSE;
	  }
     userid = USERID;
     local.Execute();
     central.Execute();
   }

   source = (fLocal)?&local:&central;
   if (source->Fetch())
   {
      ret=GMS_SUCCESS;
	  *data = _data;
	  if (sizeof(szDesc) < cMDesc)
         strncpy((CHAR*) szMDesc,szDesc,sizeof(szDesc));
      else
         strncpy((CHAR*) szMDesc,szDesc,cMDesc);
   }   
   else if (source->NoData())
      ret = DMS_NO_DATA;
   else 
      ret = DMS_ERROR;

   POP();

#undef SQL_GETSTUDY2

}


/*****************************************************************************/

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_findStudy_(CHAR *szSearchName, DMS_STUDY *data, LONG fSearchOption)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_findStudy);
   if (fSearchOption==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal){
      ret=findStudy(LOCAL,szSearchName,*data,fSearchOption);
      if (ret!=GMS_SUCCESS){
         fLocal = FALSE; 
         ret=findStudy(CENTRAL,szSearchName,*data,FIND_NEXT);
      }
   }
   else
      ret=findStudy(CENTRAL,szSearchName,*data,fSearchOption);

   POP();

}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_findStudy(CHAR *szSearchName, DMS_STUDY *data, LONG fSearchOption)
{
   static BOOL fLocal=TRUE;
   static BOOL name_wild=FALSE;

   PUSH(DMS_findStudy);
   if (fSearchOption==FIND_FIRST){
      fLocal=TRUE;
	  name_wild =strchr(szSearchName,'_') || strchr(szSearchName,'%');
   }
   if (name_wild) {
     if (fLocal){
      ret=findStudyLike(LOCAL,szSearchName,*data,fSearchOption);
      if (ret!=GMS_SUCCESS){
         fLocal = FALSE; 
         ret=findStudyLike(CENTRAL,szSearchName,*data,FIND_NEXT);
      }
	 }
     else
      ret=findStudyLike(CENTRAL,szSearchName,*data,fSearchOption);
   }
   else {
     if (fLocal){
      ret=findStudyEq(LOCAL,szSearchName,*data,fSearchOption);
      if (ret!=GMS_SUCCESS){
         fLocal = FALSE; 
         ret=findStudyEq(CENTRAL,szSearchName,*data,FIND_NEXT);
      }
	 }
     else
      ret=findStudyEq(CENTRAL,szSearchName,*data,fSearchOption);
   }

   
   POP();

}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_addStudy(DMS_STUDY  *study, CHAR *szDesc)
{
   PUSH(DMS_addStudy);
   ret = addStudy(*study, szDesc);
   POP();
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_setStudy(DMS_STUDY  *study, CHAR *szDesc)
{
   PUSH(DMS_setStudy);
   ret = setStudy(*study, szDesc);
   POP();
}


LONG DLL_INTERFACE DMS_deleteStudy(long styid)
{
    PUSH(DMS_deleteStudy);
	if (styid > 0)
	{
		return DMS_INVALID_ID;  
	}
	ret = deleteStudy(styid);
	POP();
}


LONG DLL_INTERFACE DMS_getStudy(DMS_STUDY* Study, UCHAR* szMDesc, LONG cMDesc, int fOpt)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_getStudy);
   if (fOpt==FIND_FIRST)
   {
      fLocal=TRUE;
   }
   if (fLocal){
      ret = getStudy(LOCAL, Study, szMDesc, cMDesc, fOpt);
      if (ret!=DMS_SUCCESS){
         fLocal = FALSE; 
         ret = getStudy(CENTRAL, Study, szMDesc, cMDesc, FIND_NEXT);
      }
   }
   else
      ret = getStudy(CENTRAL, Study, szMDesc, cMDesc, fOpt);
	POP();
}

LONG DLL_INTERFACE DMS_getStudy2(DMS_STUDY* Study, UCHAR* szMDesc, LONG cMDesc, int fOpt)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_getStudy2);
   if (fOpt==FIND_FIRST)
   {
      fLocal=TRUE;
   }
   if (fLocal){
      ret = getStudy2(LOCAL, Study, szMDesc, cMDesc, fOpt);
      if (ret!=DMS_SUCCESS){
         fLocal = FALSE; 
         ret = getStudy2(CENTRAL, Study, szMDesc, cMDesc, FIND_NEXT);
      }
   }
   else
      ret = getStudy2(CENTRAL, Study, szMDesc, cMDesc, fOpt);
	POP();
}

