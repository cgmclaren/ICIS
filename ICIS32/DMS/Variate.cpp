/* Variate.cpp : Implements the functions to access and manipulate the VARIATE table of ICIS
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
 Programmed by   :   Arllet M. Portugal, Warren Constantino
 Modified  (AMP) :   
 **************************************************************/


#include "Variate.h"

extern LONG UACCESS,USERID;
extern CODBCdbc *_centralDMS,*_localDMS;  //Central and Local Database Connection
extern stack<char*> _routines_stack;

//////////////////////////////////////////////////////////////////////////////
//  Finds the minimum  variateid
//////////////////////////////////////////////////////////////////////////////
LONG getNextVariateID(void)
{
static LONG variatID=0;

   //if (!variatID)
   //{
      CODBCdirectStmt local = _localDMS->DirectStmt("SELECT MIN(VARIATID) FROM VARIATE");
      local.Execute();
      local.Fetch();
      if (!local.NoData())
         variatID=local.Field(1).AsInteger();
   //}
   return --variatID;
}


//////////////////////////////////////////////////////////////////////////////
//  Finds the minimum  attribute id
//////////////////////////////////////////////////////////////////////////////
LONG getNextAttributeID(void)
{
static LONG attrID=0;

   //if (!attrID)
   //{
      CODBCdirectStmt local = _localDMS->DirectStmt("SELECT MIN(DMSATID) FROM DMSATTR");
      local.Execute();
      local.Fetch();
      if (!local.NoData())
         attrID=local.Field(1).AsInteger();
   //}
   return --attrID;
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG findVariate(BOOL fLocal, CHAR *szName, DMS_VARIATE &data, LONG fSearchOption)
{
#define SQL_FINDVARIATE  \
       " select V.VariatID, V.STUDYID, V.VNAME, V.VTYPE, \
              V.TRAITID, V.SCALEID, V.TMETHID, V.DTYPE \
              from VARIATE V "

#define VNAME_EQ_SQL  " WHERE  V.VNAME = ? " 
#define VNAME_LIKE_SQL " WHERE  V.VNAME LIKE ? "
#define STUDYID_SQL " AND V.STUDYID = ? "

#define SQL_FINDVARIATE_ " order by V.VNAME asc "

   PUSH(findVariate);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_FINDVARIATE);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_FINDVARIATE);
       CODBCbindedStmt *source;
static CHAR strname[MAX_STR];
static DMS_VARIATE _data;
static BOOL name_wild;

static BOOL first_time=TRUE;

   if (fSearchOption==FIND_FIRST){
      string stmt_sql;
	  ZeroFill(strname,sizeof(strname));
      strcpy(strname,szName);
	  name_wild =strchr(strname,'_') || strchr(strname,'%');
	  stmt_sql=SQL_FINDVARIATE;
	  stmt_sql+=(name_wild?VNAME_LIKE_SQL:VNAME_EQ_SQL);
	  if (data.STUDYID != 0) 
		  stmt_sql+=STUDYID_SQL;
	  stmt_sql+=SQL_FINDVARIATE_;
	  central.SetSQLstr(stmt_sql.c_str());
	  local.SetSQLstr(stmt_sql.c_str());

 //     if (first_time)
 //     {
         BIND(1,_data.VARIATID);
         BIND(2,_data.STUDYID);
         BINDS(3,_data.VNAME,DMS_VARIATE_NAME);
         BINDS(4,_data.VTYPE,DMS_VARIATE_TYPE);
		 BIND(5,_data.TRAITID);
		 BIND(6,_data.SCALEID);
		 BIND(7,_data.TMETHID);
         BINDS(8,_data.DTYPE,DMS_DATA_TYPE);
         BINDPARAMS(1, strname, DMS_VARIATE_NAME-2);
	     if (data.STUDYID != 0) 
  		     BINDPARAM(2,data.STUDYID);
         first_time=FALSE;
  //    }
      local.Execute();
      central.Execute();
   }

   source = (fLocal)?&local:&central;
   ZeroMemory(&_data,sizeof(_data));

   if (source->Fetch())
      data = _data;
   else if (source->NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;

   POP();
#undef SQL_FINDVARIATE 
#undef VNAME_EQ_SQL 
#undef VNAME_LIKE_SQL 
#undef STUDYID_SQL 
#undef SQL_FINDVARIATE_
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG findVariateEq(BOOL fLocal, CHAR *szName, DMS_VARIATE &data, LONG fSearchOption)
{
#define SQL_FINDVARIATE_EQ  \
       " select V.VariatID, V.STUDYID, V.VNAME, V.VTYPE, \
              V.TRAITID, V.SCALEID, V.TMETHID, V.DTYPE \
              from VARIATE V WHERE  V.VNAME = ? AND (0=? OR V.STUDYID = ?) \
			  order by V.VNAME asc "

   PUSH(findVariateEq);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_FINDVARIATE_EQ);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_FINDVARIATE_EQ);
       CODBCbindedStmt *source;
static CHAR strname[MAX_STR];
static DMS_VARIATE _data;
static BOOL name_wild;
static LONG styid;

static BOOL first_time=TRUE;

   if (fSearchOption==FIND_FIRST){
      string stmt_sql;
	  ZeroFill(strname,sizeof(strname));
      strcpy(strname,szName);
      styid = data.STUDYID;
	  
      if (first_time)
      {
         BIND(1,_data.VARIATID);
         BIND(2,_data.STUDYID);
         BINDS(3,_data.VNAME,DMS_VARIATE_NAME);
         BINDS(4,_data.VTYPE,DMS_VARIATE_TYPE);
		 BIND(5,_data.TRAITID);
		 BIND(6,_data.SCALEID);
		 BIND(7,_data.TMETHID);
         BINDS(8,_data.DTYPE,DMS_DATA_TYPE);
         BINDPARAMS(1, strname, DMS_VARIATE_NAME-2);
	     BINDPARAM(2,styid);
	     BINDPARAM(3,styid);
         first_time=FALSE;
      }
      local.Execute();
      central.Execute();
   }

   source = (fLocal)?&local:&central;
   ZeroMemory(&_data,sizeof(_data));

   if (source->Fetch())
      data = _data;
   else if (source->NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;

   POP();
#undef SQL_FINDVARIATE_EQ 
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG findVariateLike(BOOL fLocal, CHAR *szName, DMS_VARIATE &data, LONG fSearchOption)
{
#define SQL_FINDVARIATE_LIKE  \
       " select V.VariatID, V.STUDYID, V.VNAME, V.VTYPE, \
              V.TRAITID, V.SCALEID, V.TMETHID, V.DTYPE \
              from VARIATE V WHERE  V.VNAME LIKE ? AND (0=?  OR V.STUDYID = ?) \
			  order by V.VNAME asc "

   PUSH(findVariateLike);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_FINDVARIATE_LIKE);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_FINDVARIATE_LIKE);
       CODBCbindedStmt *source;

static CHAR strname[MAX_STR];
static DMS_VARIATE _data;
static BOOL name_wild;
static LONG styid;
static BOOL first_time=TRUE;

   if (fSearchOption==FIND_FIRST){
      string stmt_sql;
	  ZeroFill(strname,sizeof(strname));
      strcpy(strname,szName);
      styid = data.STUDYID;
	  
      if (first_time)
      {
         BIND(1,_data.VARIATID);
         BIND(2,_data.STUDYID);
         BINDS(3,_data.VNAME,DMS_VARIATE_NAME);
         BINDS(4,_data.VTYPE,DMS_VARIATE_TYPE);
		 BIND(5,_data.TRAITID);
		 BIND(6,_data.SCALEID);
		 BIND(7,_data.TMETHID);
         BINDS(8,_data.DTYPE,DMS_DATA_TYPE);
         BINDPARAMS(1, strname, DMS_VARIATE_NAME-2);
	     BINDPARAM(2,styid);
	     BINDPARAM(3,styid);
         first_time=FALSE;
      }
      local.Execute();
      central.Execute();
   }

   source = (fLocal)?&local:&central;
   ZeroMemory(&_data,sizeof(_data));

   if (source->Fetch())
      data = _data;
   else if (source->NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;

   POP();
#undef SQL_FINDVARIATE_LIKE 
}



//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG findVariateTMS(BOOL fLocal, DMS_VARIATE &data, LONG fSearchOption)
{
#define SQL_FINDVARIATE  \
       " select V.VariatID, V.STUDYID, V.VNAME, V.VTYPE, \
              V.TRAITID, V.SCALEID, V.TMETHID, V.DTYPE \
              from VARIATE V \
         WHERE  (V.STUDYID = ? or 0=?) AND (V.TRAITID=? OR 0=?) AND (V.SCALEID=? AND 0=?) and (T.TMETHID=? OR 0=?) \
         order by V.VNAME asc "

   PUSH(findVariateTMS);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_FINDVARIATE);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_FINDVARIATE);
       CODBCbindedStmt *source;
static CHAR strname[MAX_STR];
static DMS_VARIATE _data;
static BOOL name_wild;

static BOOL first_time=TRUE;

   if (fSearchOption==FIND_FIRST){
     if (first_time)
     {
         BIND(1,_data.VARIATID);
         BIND(2,_data.STUDYID);
         BINDS(3,_data.VNAME,DMS_VARIATE_NAME);
         BINDS(4,_data.VTYPE,DMS_VARIATE_TYPE);
		 BIND(5,_data.TRAITID);
		 BIND(6,_data.SCALEID);
		 BIND(7,_data.TMETHID);
         BINDS(8,_data.DTYPE,DMS_DATA_TYPE);
  		 BINDPARAM(1,data.STUDYID);
  		 BINDPARAM(2,data.STUDYID);
  		 BINDPARAM(3,data.TRAITID);
  		 BINDPARAM(4,data.TRAITID);
  		 BINDPARAM(5,data.SCALEID);
  		 BINDPARAM(6,data.SCALEID);
  		 BINDPARAM(7,data.TMETHID);
  		 BINDPARAM(8,data.TMETHID);
         first_time=FALSE;
      }
      local.Execute();
      central.Execute();
   }

   source = (fLocal)?&local:&central;
   ZeroMemory(&_data,sizeof(_data));

   if (source->Fetch())
      data = _data;
   else if (source->NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;

   POP();
#undef SQL_FINDVARIATE 
}



//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_findVariateTMS(DMS_VARIATE *data, LONG fSearchOption)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_findVariateTMS);
   if (fSearchOption==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal){
      ret=findVariateTMS(LOCAL,*data,fSearchOption);
      if (ret!=GMS_SUCCESS){
         fLocal = FALSE; 
         ret=findVariateTMS(CENTRAL,*data,FIND_FIRST);
      }
   }
   else
      ret=findVariateTMS(CENTRAL,*data,fSearchOption);

   POP();

}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG findVariateDistinct(BOOL fLocal, CHAR *szName, DMS_VARIATE &data, LONG fSearchOption)
{
#define SQL_FINDVARIATEDISTINCT  \
       " select distinct V.VNAME, V.TRAITID, V.SCALEID, V.TMETHID \
         from VARIATE V WHERE  V.VNAME LIKE ? \
         order by V.VNAME asc "

   PUSH(findVariateDistinct);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_FINDVARIATEDISTINCT);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_FINDVARIATEDISTINCT);
       CODBCbindedStmt *source;
static CHAR strname[MAX_STR];
static DMS_VARIATE _data;

static BOOL first_time=TRUE;

   if (fSearchOption==FIND_FIRST){
	  ZeroFill(strname,sizeof(strname));
      strcpy(strname,szName);
      if (first_time)
      {
         BINDS(1,_data.VNAME,DMS_VARIATE_NAME);
		 BIND(2,_data.TRAITID);
		 BIND(3,_data.SCALEID);
		 BIND(4,_data.TMETHID);
         BINDPARAMS(1, strname, DMS_VARIATE_NAME-2);
         first_time=FALSE;
      }
      local.Execute();
      central.Execute();
   }

   source = (fLocal)?&local:&central;
   ZeroMemory(&_data,sizeof(_data));

   if (source->Fetch())
      data = _data;
   else if (source->NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;

   POP();
#undef SQL_FINDVARIATEDISTINCT 
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG addVariate(DMS_VARIATE &data)
{
#define SQL_ADDVARIATE "\
        insert into VARIATE \
             (VARIATID,VNAME,VTYPE,STUDYID,TRAITID, SCALEID, TMETHID,DTYPE) \
              values (?, ?, ?, ?, ?, ?, ?, ?) "
   PUSH(addVariate);

static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_ADDVARIATE);

static DMS_VARIATE _data;
static BOOL first_time=TRUE;

   _data = data; 
   ret = findVariateEq(data.STUDYID<0,data.VNAME,_data,FIND_FIRST);
   if (ret==DMS_SUCCESS) {
	   data = _data;                        //return the entire record of array
	   POP2(DMS_EXIST);
   }
   data.VARIATID = getNextVariateID();
   if (first_time)
   {
      local.BindParam(1,_data.VARIATID);
//20030227      local.BindParam(2,_data.VNAME,sizeof(_data.VNAME));
	  local.BindParam(2,_data.VNAME,DMS_VARIATE_NAME-2);
//20030227            local.BindParam(3,_data.VTYPE, sizeof(_data.VTYPE));
	  local.BindParam(3,_data.VTYPE, DMS_VARIATE_TYPE-2);
      local.BindParam(4,_data.STUDYID);
      local.BindParam(5,_data.TRAITID);
      local.BindParam(6,_data.SCALEID);
      local.BindParam(7,_data.TMETHID);
//20030227            local.BindParam(8,_data.DTYPE,sizeof(_data.DTYPE));
	  local.BindParam(8,_data.DTYPE,DMS_DATA_TYPE-3);
      first_time=FALSE;
   }

   _data= data;

   if ((ret=local.Execute())==1)
      ret=DMS_SUCCESS;
   //else
   //   ret=DMS_ERROR;

    else  
	   if (ret!=GMS_UNIQUE_CONSTRAINT_ERROR) //GMS_UNIQUE_CONSTRAINT_ERROR
		   ret=GMS_ERROR;  
       // Handler for unique contraint error with the assumption that only VARIATEID has unique contraint 
	   else {
		   int i=0;
		   while ((ret==GMS_UNIQUE_CONSTRAINT_ERROR) && (i<100)) {
			    _data.VARIATID = getNextVariateID();
				ret=local.Execute();
				++i;
		   }
		   if ((ret) &&(ret!=GMS_UNIQUE_CONSTRAINT_ERROR)) {
               data = _data;
			   ret = DMS_SUCCESS;
		   }
		   else  {
			   data.VARIATID = 0;
			   ret = DMS_ERROR;
		   }
	   }

   POP();

#undef SQL_ADDVARIATE
}



//////////////////////////////////////////////////////////////////////////////
// Retrieves a variate record based on the passed VariatID.  
// If VariatID=0, then all records will be retrieve.
//////////////////////////////////////////////////////////////////////////////
LONG getVariate(BOOL fLocal,DMS_VARIATE *data, int fOpt)
{
#define SQL_GETVARIATE \
  "SELECT V.VariatID, V.STUDYID, V.VNAME, V.VTYPE, \
              V.TRAITID, V.SCALEID, V.TMETHID, V.DTYPE \
              from VARIATE V \
	WHERE ( 0=? or V.STUDYID = ?)  AND (0=? or V.VariatID=?)" 


  LONG ret=DMS_SUCCESS;

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_GETVARIATE);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_GETVARIATE);
       CODBCbindedStmt *source;

static DMS_VARIATE _data;
static LONG studyid, variateid;
static BOOL first_time=TRUE;

//Parameters
   if (fOpt == FIND_FIRST) {
      if (first_time)
	  {
		BINDPARAM(1, studyid);
		BINDPARAM(2, studyid);
		BINDPARAM(3, variateid);
		BINDPARAM(4, variateid);
												
        BIND(1,_data.VARIATID);
        BIND(2,_data.STUDYID);
        BINDS(3,_data.VNAME,DMS_VARIATE_NAME);
        BINDS(4,_data.VTYPE,DMS_VARIATE_TYPE);
	    BIND(5,_data.TRAITID);
		BIND(6,_data.SCALEID);
		BIND(7,_data.TMETHID);
        BINDS(8,_data.DTYPE,DMS_DATA_TYPE);

      first_time = FALSE;
	  }
     studyid = data->STUDYID;
	 variateid = data->VARIATID;
     local.Execute();
     central.Execute();
   }

   source = (fLocal)?&local:&central;
   if (source->Fetch())
   {
      ret=GMS_SUCCESS;
	  *data = _data;
   }
   else if (source->NoData())
      ret = DMS_NO_DATA;
   else 
      ret = DMS_ERROR;

   return ret;

#undef SQL_GETVARIATE
}

//////////////////////////////////////////////////////////////////////////////
//Updates a record in VARIATE table based on the given VARIATID
// Input Parameter:  VARIATE structure 
// Note:  STUDYID is non-updateable
// Date:  Mar 18, 2003
//////////////////////////////////////////////////////////////////////////////
LONG setVariate(DMS_VARIATE &data)
{
#define SQL_SETVARIATE "\
        Update VARIATE \
        set VNAME = ?,VTYPE=?,TRAITID =?, SCALEID=?, TMETHID=?,DTYPE=? \
		where VARIATID=? "
   PUSH(setVariate);

static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_SETVARIATE);

static DMS_VARIATE _data;
static BOOL first_time=TRUE;

  if (first_time)
   {
	  local.BindParam(1,_data.VNAME,DMS_VARIATE_NAME-2);
	  local.BindParam(2,_data.VTYPE, DMS_VARIATE_TYPE-2);
      local.BindParam(3,_data.TRAITID);
      local.BindParam(4,_data.SCALEID);
      local.BindParam(5,_data.TMETHID);
	  local.BindParam(6,_data.DTYPE,DMS_DATA_TYPE-3);
      local.BindParam(7,_data.VARIATID);
      first_time=FALSE;
   }

   _data= data;

   if (local.Execute())
      ret=DMS_SUCCESS;
   else
      ret=DMS_ERROR;


   POP();

#undef SQL_SETVARIATE
}


LONG addDMSATTR( DMS_DMSATTR* Dtr, CHAR* szDesc)
{
#define SQL_ADDDMSATTR \
	" INSERT INTO DMSATTR (DMSATID,DMSATYPE,DMSATAB,DMSATREC,DMSATVAL) VALUES (?,?,?,?,?)"
  PUSH(addDMSATTR);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_ADDDMSATTR);

static DMS_DMSATTR _data;
static BOOL first_time=TRUE;

static CHAR pnext[MAX_STR];

  
   strncpy((CHAR *)pnext, szDesc , strlen(szDesc));

   Dtr->DMSATID = getNextAttributeID();
   _data = *Dtr;
 //  _data.DMSATAB[DMS_ATTR_TABLE-2] = '\0';

   if (first_time)
   {
      local.BindParam(1,_data.DMSATID);
      local.BindParam(2,_data.DMSATYPE);
      local.BindParam(3,_data.DMSATAB, DMS_ATTR_TABLE-2);
      local.BindParam(4,_data.DMSATREC);
      first_time=FALSE;
   }

  local.BindParam(5,szDesc, strlen(szDesc) );
   
   //if (local.Execute())
   //   ret=DMS_SUCCESS;
   //else
   //   ret=DMS_ERROR;


    if ((ret=local.Execute())==1)
      ret=DMS_SUCCESS;
    else  
	   if (ret!=GMS_UNIQUE_CONSTRAINT_ERROR) //GMS_UNIQUE_CONSTRAINT_ERROR
		   ret=GMS_ERROR;  
       // Handler for unique contraint error with the assumption that only STUDYID has unique contraint 
	   else {
		   int i=0;
		   while ((ret==GMS_UNIQUE_CONSTRAINT_ERROR) && (i<100)) {
                _data.DMSATID = getNextAttributeID();
				ret=local.Execute();
				++i;
		   }
		   if ((ret) &&(ret!=GMS_UNIQUE_CONSTRAINT_ERROR)) {
               *Dtr = _data;
			   ret = DMS_SUCCESS;
		   }
		   else  {
			   Dtr->DMSATID  = 0;
			   ret = DMS_ERROR;
		   }
	   }


   POP();
}


LONG getDMSATTR(BOOL fLocal, DMS_DMSATTR* Dtr, CHAR* szDesc, LONG lenszDesc, LONG fOpt)
{
#define SQL_GETDMSATTR \
  " SELECT DMSATID,DMSATYPE,DMSATAB,DMSATREC,DMSATVAL FROM DMSATTR \
	WHERE DMSATAB=? AND DMSATYPE = ? AND  (DMSATREC=? or 0=?)"
  PUSH(getDMSATTR);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_GETDMSATTR);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_GETDMSATTR);
       CODBCbindedStmt *source;

static DMS_DMSATTR _data;
static CHAR szVal[DMS_MAX_STR], szTab[DMS_ATTR_TABLE];
static LONG _recid, _udfldid;
static BOOL first_time=TRUE;

//Parameters
   if (fOpt == FIND_FIRST) {
      if (first_time)
	  {
		BINDPARAMS(1, szTab,DMS_ATTR_TABLE);
		BINDPARAM(2, _udfldid);
		BINDPARAM(3, _recid);
		BINDPARAM(4, _recid);
												
        BIND(1, _data.DMSATID);		                   
        BIND(2, _data.DMSATYPE);
		BINDS(3, _data.DMSATAB, DMS_ATTR_TABLE); 
        BIND(4, _data.DMSATREC);
		BINDS(5, szVal, DMS_MAX_STR);

      first_time = FALSE;
	  }
     _recid=Dtr->DMSATREC;
	 _udfldid = Dtr->DMSATYPE;
     strncpy(szTab,Dtr->DMSATAB,DMS_ATTR_TABLE);
     local.Execute();
     central.Execute();
   }

   source = (fLocal)?&local:&central;
   if (source->Fetch())
   {
      ret=GMS_SUCCESS;
      strncpy(szDesc, (CHAR*) szVal,lenszDesc);
	  *Dtr= _data;
   }
   else if (source->NoData())
      ret = DMS_NO_DATA;
   else 
      ret = DMS_ERROR;

   POP();

#undef SQL_GETDMSATTR
}


//////////////////////////////////////////////////////////////////////////////
//Date Created: June 14, 2005
// Decription: Deletes a record from DMSATTR for the given attribute type and record number
//////////////////////////////////////////////////////////////////////////////
LONG deleteDMSATTR(BOOL fLocal, long atype, long atrec)

{
#define SQL_deleteDMSATTR "\
   delete from DMSATTR\
   where DMSATYPE=? and DMSATREC=?"
   PUSH(deleteDMSATTR);

static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_deleteDMSATTR);

static BOOL first_time=TRUE;

static LONG _atype,_atrec;

   if (first_time)
   {
      local.BindParam(1,_atype);
      local.BindParam(2,_atrec);
 
      first_time=FALSE;
   }

   _atype=atype;
   _atrec=atrec;

   if (local.Execute())
      ret=GMS_SUCCESS;
   else if (local.NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;
   POP();

#undef SQL_deleteDMSATTR
}


//////////////////////////////////////////////////////////////////////////////
//Date Created: June 14, 2005
// Decription: Deletes a record from DMSATTR for the given attribute type and record number
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_deleteDMSATTR(LONG atype,LONG atrec)
{
   PUSH(GMS_deleteDMSATTR);
   ret=deleteDMSATTR(LOCAL,atype,atrec);
   POP();
}


/*****************************************************************************/

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_findVariate(CHAR *szSearchName, DMS_VARIATE *data, LONG fSearchOption)
{
   static BOOL fLocal=TRUE;
   static BOOL name_wild=FALSE;

   PUSH(DMS_findVariate);
   if (fSearchOption==FIND_FIRST){
         fLocal=TRUE;
	  name_wild =strchr(szSearchName,'_') || strchr(szSearchName,'%');
   }
   if (name_wild) {
     if (fLocal){
        ret=findVariateLike(LOCAL,szSearchName,*data,fSearchOption);
        if (ret!=GMS_SUCCESS){
           fLocal = FALSE; 
         ret=findVariateLike(CENTRAL,szSearchName,*data,FIND_NEXT);
		}
	 }
     else
        ret=findVariateLike(CENTRAL,szSearchName,*data,fSearchOption);
   }
   else {
     if (fLocal){
        ret=findVariateEq(LOCAL,szSearchName,*data,fSearchOption);
        if (ret!=GMS_SUCCESS){
           fLocal = FALSE; 
         ret=findVariateEq(CENTRAL,szSearchName,*data,FIND_NEXT);
		}
	 }
     else
        ret=findVariateEq(CENTRAL,szSearchName,*data,fSearchOption);

   }
   POP();

}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_findVariate_(CHAR *szSearchName, DMS_VARIATE *data, LONG fSearchOption)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_findVariate);
   if (fSearchOption==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal){
      ret=findVariate(LOCAL,szSearchName,*data,fSearchOption);
      if (ret!=GMS_SUCCESS){
         fLocal = FALSE; 
         ret=findVariate(CENTRAL,szSearchName,*data,FIND_FIRST);
      }
   }
   else
      ret=findVariate(CENTRAL,szSearchName,*data,fSearchOption);

   POP();

}


//////////////////////////////////////////////////////////////////////////////
//      DMS_addVariate
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_addVariate(DMS_VARIATE* data)
{
   PUSH(DMS_addVariate);
   ret = addVariate(*data);
   DMS_commitData;

   POP();
}



LONG DLL_INTERFACE DMS_addAttr( DMS_DMSATTR* Dtr, CHAR* szDesc)
{
   PUSH(DMS_addDMSATTR);
   ret = addDMSATTR(Dtr, szDesc);
   POP();
}

//////////////////////////////////////////////////////////////////////////////
//  Input: data.studyid
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getVariate(DMS_VARIATE *data,  int fOpt)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_getVariate);

   if (fOpt==FIND_FIRST)
   {
      fLocal=TRUE;
   }
   if (fLocal){
      ret=getVariate(LOCAL,data,fOpt);
      if (ret!=DMS_SUCCESS){
         fLocal = FALSE; 
         ret=getVariate(CENTRAL,data, FIND_NEXT);
      }
   }
   else
      ret=getVariate(CENTRAL,data,fOpt);
   POP();

}


//////////////////////////////////////////////////////////////////////////////
//  Input : data.DMSATAB, data.DMSATYPE, data.DMSATREC
//  Remark: szDesc must be a null terminated string
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getDMSAttr(DMS_DMSATTR* data, CHAR* szDesc, LONG nszDesc,  int fOpt)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_getDMSAttr);

   if (fOpt==FIND_FIRST)
   {
      fLocal=TRUE;
   }
   if (fLocal){
      ret=getDMSATTR(LOCAL,data,szDesc,nszDesc, fOpt);
      if (ret!=DMS_SUCCESS){
         fLocal = FALSE; 
         ret=getDMSATTR(CENTRAL,data, szDesc, nszDesc,FIND_NEXT);
      }
   }
   else
      ret=getDMSATTR(CENTRAL,data,szDesc,nszDesc, fOpt);
   POP();

}


//////////////////////////////////////////////////////////////////////////////
//added by WVC
//***** Gets all variates belonging to one representation *****/
//Input:  data.STUDYID, data.REPRESNO
//Output: entire variate record
//Called by DMS_getSRVariate
//////////////////////////////////////////////////////////////////////////////
LONG getSRVariate(BOOL fLocal, DMS_SRVARIATE *data, int fOpt)
{
	#define SQL_GETSRVARIATE1 \
		"SELECT STUDYID, REPRESNO, VARIATE.VARIATID, VNAME, VTYPE, TRAITID, SCALEID, TMETHID, DTYPE \
		FROM VARIATE, VEFFECT \
		WHERE (((VEFFECT.VARIATID)=VARIATE.VARIATID) AND ((VEFFECT.REPRESNO)=?) \
		AND ((VARIATE.STUDYID)=?)) ORDER BY VARIATE.VARIATID ASC"
	
	#define SQL_GETSRVARIATE2 \
		"SELECT STUDYID, REPRESNO, VARIATE.VARIATID, VNAME, VTYPE, TRAITID, SCALEID, TMETHID, DTYPE \
		FROM VARIATE, VEFFECT \
		WHERE (((VEFFECT.VARIATID)=VARIATE.VARIATID) AND ((VEFFECT.REPRESNO)=?) \
		AND ((VARIATE.STUDYID)=?)) ORDER BY VARIATE.VARIATID DESC"
	
	LONG ret = DMS_SUCCESS;
	
	static CODBCbindedStmt &central = _centralDMS->BindedStmt(SQL_GETSRVARIATE1);
	static CODBCbindedStmt &local = _localDMS->BindedStmt(SQL_GETSRVARIATE2);
	CODBCbindedStmt *source;
	
	static DMS_SRVARIATE _data;
	static BOOL first_time = TRUE;
	
	if (fOpt == FIND_FIRST) {
		if (first_time) {
			BINDPARAM(1, _data.REPRESNO);
			BINDPARAM(2, _data.STUDYID);
			
			BIND(1, _data.STUDYID);
			BIND(2, _data.REPRESNO);
			BIND(3, _data.VARIATID);
			BINDS(4, _data.VNAME, DMS_VARIATE_NAME);
			BINDS(5, _data.VTYPE, DMS_VARIATE_TYPE);
			BIND(6, _data.TRAITID);
			BIND(7, _data.SCALEID);
			BIND(8, _data.TMETHID);
			BINDS(9, _data.DTYPE, DMS_DATA_TYPE);
			
			first_time = FALSE;
		}
		_data.STUDYID = data->STUDYID;
		_data.REPRESNO = data->REPRESNO;
		local.Execute();
		central.Execute();
	}
	
	source = (fLocal) ? &local : &central;
	if (source->Fetch()) {
		ret = GMS_SUCCESS;
		*data = _data;
	}
	else if (source->NoData())
		ret = DMS_NO_DATA;
	else 
		ret = DMS_ERROR;
	
	return ret;
	
	#undef SQL_GETSRVARIATE1
	#undef SQL_GETSRVARIATE2
}

//////////////////////////////////////////////////////////////////////////////
//added by WVC
//***** Gets all variates belonging to one representation *****/
//Input:  data.STUDYID, data.REPRESNO
//Output: entire variate record
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getSRVariate(DMS_SRVARIATE *data, int fOpt)
{
	static BOOL fLocal = TRUE;
	
	PUSH(DMS_getSRVariate);
	
	if (fOpt == FIND_FIRST) {
		fLocal = TRUE;
	}
	if (fLocal) {
		ret = getSRVariate(LOCAL, data, fOpt);
		if (ret != DMS_SUCCESS){
			fLocal = FALSE;
			ret = getSRVariate(CENTRAL, data, FIND_NEXT);
		}
	}
	else
		ret = getSRVariate(CENTRAL, data, fOpt);
	
	POP();
	
}


//////////////////////////////////////////////////////////////////////////////
//added by WVC
//***** Gets all unique variate names & their ontology  *****/
//Input:  none
//Output: variate, property, scale, & method names
//Called by DMS_getUVariate
// Modified:  20060116 (AMP) - changes the returned data structure to DMS_VARIATE
//////////////////////////////////////////////////////////////////////////////
LONG getUVariate(BOOL fLocal, DMS_UVARIATE *data, int fOpt)
{
	#define SQL_UVARIATE \
		"SELECT DISTINCT VNAME, TRAITID, SCALEID, TMETHID \
		FROM VARIATE ORDER BY VNAME ASC"
	
	LONG ret = DMS_SUCCESS;
	
	static CODBCbindedStmt &central = _centralDMS->BindedStmt(SQL_UVARIATE);
	static CODBCbindedStmt &local = _localDMS->BindedStmt(SQL_UVARIATE);
	CODBCbindedStmt *source;
	
	static DMS_UVARIATE _data;
	static BOOL first_time = TRUE;
	
	if (fOpt == FIND_FIRST) {
		if (first_time) {
			
			BINDS(1, _data.VNAME, DMS_VARIATE_NAME);
			BIND(2, _data.TRAITID);
			BIND(3, _data.SCALEID);
			BIND(4, _data.TMETHID);
			
			first_time = FALSE;
		}
		local.Execute();
		central.Execute();
	}
	
	source = (fLocal) ? &local : &central;
	if (source->Fetch()) {
		ret = GMS_SUCCESS;
		*data = _data;
	}
	else if (source->NoData())
		ret = DMS_NO_DATA;
	else 
		ret = DMS_ERROR;
	
	return ret;
	
	#undef SQL_UVARIATE
}

//////////////////////////////////////////////////////////////////////////////
//added by WVC
//***** Gets all unique variate names & their ontology *****/
//Input:  none
//Output: variate, property, scale, & method names
// Modified:  20060116 (AMP) - changes the returned data structure to DMS_VARIATE
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getUVariate(DMS_UVARIATE *data, int fOpt)
{
	static BOOL fLocal = TRUE;
	
	PUSH(DMS_getUVariate);
	
	if (fOpt == FIND_FIRST) {
		fLocal = TRUE;
	}
	if (fLocal) {
		ret = getUVariate(LOCAL, data, fOpt);
		if (ret != DMS_SUCCESS){
			fLocal = FALSE;
			ret = getUVariate(CENTRAL, data, FIND_NEXT);
		}
	}
	else
		ret = getUVariate(CENTRAL, data, fOpt);
	
	POP();
	
}


//////////////////////////////////////////////////////////////////////////////
//added by WVC
//***** Gets all variates belonging to a study *****/
//Input:  data.STUDYID
//Output: entire variate record
//Called by DMS_getSVariate
//////////////////////////////////////////////////////////////////////////////
LONG getSVariate(BOOL fLocal, DMS_SRVARIATE *data, int fOpt)
{
	#define SQL_GETSVARIATE1 \
		"SELECT STUDYID, REPRESNO, VARIATE.VARIATID, VNAME, VTYPE, TRAITID, SCALEID, TMETHID, DTYPE \
		FROM VARIATE, VEFFECT \
		WHERE (((VEFFECT.VARIATID)=VARIATE.VARIATID) \
		AND ((VARIATE.STUDYID)=?)) ORDER BY  VARIATE.VARIATID ASC "
	
	#define SQL_GETSVARIATE2 \
		"SELECT STUDYID, REPRESNO, VARIATE.VARIATID, VNAME, VTYPE, TRAITID, SCALEID, TMETHID, DTYPE \
		FROM VARIATE, VEFFECT \
		WHERE (((VEFFECT.VARIATID)=VARIATE.VARIATID) \
		AND ((VARIATE.STUDYID)=?)) ORDER BY VARIATE.VARIATID DESC "
	
	LONG ret = DMS_SUCCESS;
	
	static CODBCbindedStmt &central = _centralDMS->BindedStmt(SQL_GETSVARIATE1);
	static CODBCbindedStmt &local = _localDMS->BindedStmt(SQL_GETSVARIATE2);
	CODBCbindedStmt *source;
	
	static DMS_SRVARIATE _data;
	static BOOL first_time = TRUE;
	
	if (fOpt == FIND_FIRST) {
		if (first_time) {
			BINDPARAM(1, _data.STUDYID);
			
			BIND(1, _data.STUDYID);
			BIND(2, _data.REPRESNO);
			BIND(3, _data.VARIATID);
			BINDS(4, _data.VNAME, DMS_VARIATE_NAME);
			BINDS(5, _data.VTYPE, DMS_VARIATE_TYPE);
			BIND(6, _data.TRAITID);
			BIND(7, _data.SCALEID);
			BIND(8, _data.TMETHID);
			BINDS(9, _data.DTYPE, DMS_DATA_TYPE);
			
			first_time = FALSE;
		}
		_data.STUDYID = data->STUDYID;
		local.Execute();
		central.Execute();
	}
	
	source = (fLocal) ? &local : &central;
	if (source->Fetch()) {
		ret = GMS_SUCCESS;
		*data = _data;
	}
	else if (source->NoData())
		ret = DMS_NO_DATA;
	else 
		ret = DMS_ERROR;
	
	return ret;
	
	#undef SQL_GETSVARIATE1
	#undef SQL_GETSVARIATE2
}

//////////////////////////////////////////////////////////////////////////////
//added by WVC
//***** Gets all variates belonging to a study *****/
//Input:  data.STUDYID
//Output: entire variate record
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getSVariate(DMS_SRVARIATE *data, int fOpt)
{
	static BOOL fLocal = TRUE;
	
	PUSH(DMS_getSVariate);
	
	if (fOpt == FIND_FIRST) {
		fLocal = TRUE;
	}
	if (fLocal) {
		ret = getSVariate(LOCAL, data, fOpt);
		if (ret != DMS_SUCCESS){
			fLocal = FALSE;
			ret = getSVariate(CENTRAL, data, FIND_NEXT);
		}
	}
	else
		ret = getSVariate(CENTRAL, data, fOpt);
	
	POP();
	
}



/**
//Edit
LONG updateDMSATTR(BOOL fLocal, DMS_DMSATTR* Dtr, CHAR* szDesc, LONG lenszDesc)
{
#define SQL_UPDATEDMSATTR \
  "UPDATE DMSATTR SET DMSATTR.DMSATVAL = ?
   WHERE (((DMSATTR.DMSATYPE)=?) AND ((DMSATTR.DMSATREC)=?))"
  PUSH(getDMSATTR);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_UPDATEDMSATTR);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_UPDATEDMSATTR);
       CODBCbindedStmt *source;

static DMS_DMSATTR _data;
static CHAR szVal[DMS_MAX_STR], szTab[DMS_ATTR_TABLE];
static LONG _recid, _udfldid;
static BOOL first_time=TRUE;

//Parameters

    if (first_time)
    {
		BINDPARAMS(1, szVal,DMS_MAX_STR);
		BINDPARAM(2, _data.DMSATYPE);
    	BINDPARAM(3, _data.DMSATREC);

      first_time = FALSE;
    }
   local.BindParam(5,szDesc, strlen(szDesc) );
  
   local.Execute();
   central.Execute();

   source = (fLocal)?&local:&central;
   if (source->Fetch())
   {
      ret=GMS_SUCCESS;
      strncpy(szDesc, (CHAR*) szVal,lenszDesc);
	  *Dtr= _data;
   }
   else if (source->NoData())
      ret = DMS_NO_DATA;
   else 
      ret = DMS_ERROR;

   POP();

#undef SQL_UPDATEDMSATTR
}

**/

