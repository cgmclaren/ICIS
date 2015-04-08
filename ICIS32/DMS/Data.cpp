/* Data.cpp : Implements the functions to access and manipulate the DATA table of ICIS
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

#include "Data.h"

extern LONG UACCESS,USERID;
extern CODBCdbc *_centralDMS,*_localDMS;  //Central and Local Database Connection
extern stack<char*> _routines_stack;


//////////////////////////////////////////////////////////////////////////////
//  getNextOunitID - Gets the next observation unit id
//////////////////////////////////////////////////////////////////////////////
LONG getNextOunitID(BOOL fLocal)
{
   long localOID=0,centralOID=0;
   if (fLocal )
   {
      CODBCdirectStmt source1 = _localDMS->DirectStmt("select MIN(OUNITID) from OBSUNIT") ;
      source1.Execute();
      source1.Fetch();
      localOID=source1.Field(1).AsInteger();
   }
   else if (!fLocal )
   {
      CODBCdirectStmt source = _centralDMS->DirectStmt("select MAX(OUNITID) from OINDEX") ;
      source.Execute();
      source.Fetch();
      centralOID=source.Field(1).AsInteger();
   }
   return (fLocal)? --localOID:++centralOID;
}


//////////////////////////////////////////////////////////////////////////////
//  Tests whether an observation unit exists or not
//////////////////////////////////////////////////////////////////////////////
LONG existOunitid(LONG tounit)
{

      CODBCbindedStmt local = _localDMS->BindedStmt("SELECT OUNITID FROM OINDEX WHERE OUNITID = ?");
      CODBCbindedStmt central = _centralDMS->BindedStmt("SELECT OUNITID FROM OINDEX WHERE OUNITID = ?");

      local.BindParam(1,tounit);
      local.Execute();
      local.Fetch();

      central.BindParam(1,tounit);
      central.Execute();
      central.Fetch();

      if (!local.NoData())
         return DMS_SUCCESS;
	  else if (!central.NoData())
		 return DMS_SUCCESS;
	  else 
	     return DMS_NO_DATA; 

}


//////////////////////////////////////////////////////////////////////////////
//  Tests whether an variate id exists or not
//////////////////////////////////////////////////////////////////////////////
LONG existVariatid(LONG tid)
{

      CODBCbindedStmt local = _localDMS->BindedStmt("SELECT VARIATID FROM VARIATE WHERE VARIATID = ?");
      CODBCbindedStmt central = _centralDMS->BindedStmt("SELECT VARIATID FROM VARIATE WHERE VARIATID = ?");

      local.BindParam(1,tid);
      local.Execute();
      local.Fetch();

      central.BindParam(1,tid);
      central.Execute();
      central.Fetch();

      if (!local.NoData())
         return DMS_SUCCESS;
	  else if (!central.NoData())
		 return DMS_SUCCESS;
	  else 
	     return DMS_NO_DATA; 

}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG addObsUnit(DMS_OBSUNIT *data)
{
#define SQL_ADDOBSUNIT  "insert into OBSUNIT \
             (OUNITID,EFFECTID) \
              values (?, ?) "

   PUSH(addObsUnit);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_ADDOBSUNIT);

static DMS_OBSUNIT _data;
static BOOL first_time=TRUE;
static LONG tlong;
   
   data->OUNITID = getNextOunitID(LOCAL);
   if (first_time)
   {
      local.BindParam(1,_data.OUNITID);
      local.BindParam(2,_data.EFFECTID);
      first_time=FALSE;
   }
   _data = *data;


   if ((ret=local.Execute())==1) {
	  LOG( "Within Unique Constraint Error (ounitid, effectid) ");
	  LOGF("Ounitid: %d \n",data->OUNITID);  //AMP: Debug
	  LOGF("effectid: %d \n", data->EFFECTID); 
	  LOGF("Return: %d \n", ret); 
	  DMS_commitData;
      ret=GMS_SUCCESS;
   }
   //else
   //   ret=GMS_ERROR;

    else  
	   if (ret!=GMS_UNIQUE_CONSTRAINT_ERROR) //GMS_UNIQUE_CONSTRAINT_ERROR
		   ret=GMS_ERROR;  
       // Handler for unique contraint error with the assumption that only OUNITID has unique contraint 
	   else {
		   int i=0;
		   while ((ret==GMS_UNIQUE_CONSTRAINT_ERROR) && (i<100)) {
			    _data.OUNITID = getNextOunitID(LOCAL);
				ret=local.Execute();
				LOG( "Within Unique Constraint Error (ounitid, effectid) ");
				LOGF("Ounitid: %d \n",_data.OUNITID);  //AMP: Debug
				LOGF("effectid: %d \n", _data.EFFECTID); 
				LOGF("Return: %d \n", ret); 
				++i;
		   }
		   if ((ret) &&(ret!=GMS_UNIQUE_CONSTRAINT_ERROR)) {
               *data = _data;				
			   DMS_commitData;
			   ret = DMS_SUCCESS;
		   }
		   else  {
			   data->OUNITID= 0;
			   ret = DMS_ERROR;
		   }
	   }


   POP();

#undef SQL_ADDOBSUNIT
}


//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
LONG findObsunit(BOOL fLocal,long ounitid, long effectid)
{

#define SQL_FINDOBSUNIT "\
    select OUNITID , EFFECTID from  OBSUNIT \
   where  OUNTIID = ? and EFFECTID = ? "

   PUSH(findLevel);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_FINDOBSUNIT);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_FINDOBSUNIT);
       CODBCbindedStmt *source;

static BOOL first_time=TRUE;
static long _ounitid, _effectid;

   _ounitid = ounitid;
   _effectid = effectid;
  if (first_time)
   {
      BINDPARAM(1,_ounitid);
      BINDPARAM(2,_effectid);
      first_time=FALSE;
   }


   source = (fLocal)?&local:&central;
   source->Execute();

   if (source->Fetch())
   {
      ret=DMS_SUCCESS;
   }
   else if (source->NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;

   POP();

#undef SQL_FINDOBSUNIT
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG addOindex(DMS_OINDEX data)
{
#define SQL_ADDOINDEX "insert into OINDEX \
             (OUNITID,FACTORID,LEVELNO,REPRESNO) \
              values (?, ?, ?, ?) "

   PUSH(addOindex);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_ADDOINDEX);

static DMS_OINDEX _data;
static BOOL first_time=TRUE;
static LONG tlong;
   
   if (first_time)
   {
      local.BindParam(1,_data.OUNITID);
      local.BindParam(2,_data.FACTORID);
      local.BindParam(3,_data.LEVELNO);
	  local.BindParam(4,_data.REPRESNO);
      first_time=FALSE;
   }

   memcpy(&_data,&data,sizeof(data));

   if (local.Execute())
      ret=GMS_SUCCESS;
   else
      ret=GMS_ERROR;

   POP();

#undef SQL_ADDOINDEX
}





//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG addDataC(DMS_DATAC &data)
{
#define SQL_ADDDATAC "INSERT INTO DATA_C (OUNITID,VARIATID,DVALUE) VALUES (?,?,?)"

   PUSH(addDataC);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_ADDDATAC);

static DMS_DATAC _data;
static BOOL first_time=TRUE;
static LONG tlong;
   
   tlong = data.OUNITID;
   ret = existOunitid( tlong);
   if (ret!=DMS_SUCCESS) {
       POP2(DMS_INVALID_ID);
   }
   tlong = data.VARIATID;
   ret = existVariatid( tlong);
   if (ret!=DMS_SUCCESS) {
	   POP2(DMS_INVALID_ID);
   }
   if (first_time)
   {
      local.BindParam(1,_data.OUNITID);
      local.BindParam(2,_data.VARIATID);
      local.BindParam(3,_data.DVALUE,sizeof(_data.DVALUE)-1);
      first_time=FALSE;
   }

   _data= data;

   if (local.Execute())
      ret=GMS_SUCCESS;
   else
      ret=GMS_ERROR;

   POP();

#undef SQL_ADDDATAC
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG addDataC2(DMS_DATAC &data)
{
#define SQL_ADDDATAC2 "INSERT INTO DATA_C (OUNITID,VARIATID,DVALUE) VALUES (?,?,?)"

   PUSH(addDataC2);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_ADDDATAC2);

static DMS_DATAC _data;
static BOOL first_time=TRUE;
static LONG tlong;
   
   tlong = data.OUNITID;
   if (first_time)
   {
      local.BindParam(1,_data.OUNITID);
      local.BindParam(2,_data.VARIATID);
      local.BindParam(3,_data.DVALUE,sizeof(_data.DVALUE)-1);
      first_time=FALSE;
   }

   _data= data;

   if (local.Execute())
      ret=GMS_SUCCESS;
   else
      ret=GMS_ERROR;

   POP();

#undef SQL_ADDDATAC2
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG updateDataC(DMS_DATAC &data)
{
#define SQL_UPDATEDATAC "UPDATE DATA_C SET DVALUE= ? WHERE OUNITID =? AND VARIATID=? "

   PUSH(updateDataC);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_UPDATEDATAC);
static CODBCbindedStmt &central = _centralDMS->BindedStmt(SQL_UPDATEDATAC);

static DMS_DATAC _data;
static BOOL first_time=TRUE;

   if (first_time)
   {
      BINDPARAMS(1,_data.DVALUE,sizeof(_data.DVALUE)-1);
      BINDPARAM(2,_data.OUNITID);
      BINDPARAM(3,_data.VARIATID);
      first_time=FALSE;
   }

   _data = data; 
   if (local.Execute())
      ret=GMS_SUCCESS;
   else
      if (central.Execute())
         ret = GMS_SUCCESS;
      else
         ret=GMS_ERROR;

   POP();

#undef SQL_UPDATEDATAC
}


//////////////////////////////////////////////////////////////////////////////
//deleteDataC  - deletes a record from DataC table based on the give observation unitid and variate id
//               if observation unit in the one only specified (variatid=0), all records for that observation unit are deleted
//Parameters:  
//      In:  data.OUNITID, data.VARIATID
//Programmed:  May15, 2003 
//Version:     4.0.2 
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
LONG deleteDataC(LONG ounit, LONG variatid)
{
#define SQL_DELETEDATAC "DELETE FROM DATA_C  WHERE OUNITID =? AND (VARIATID=? or 0=?) "

   PUSH(deleteDataC);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_DELETEDATAC);
//static CODBCbindedStmt &central = _centralDMS->BindedStmt(SQL_DELETEDATAC);

static LONG tmpounit=0,tmpvariatid=0;
static BOOL first_time=TRUE;

   if (first_time)
   {
      local.BindParam(1,tmpounit);
      local.BindParam(2,tmpvariatid);
      local.BindParam(3,tmpvariatid);
      first_time=FALSE;
   }

   tmpounit = ounit; 
   tmpvariatid = variatid;
   if (local.Execute())
      ret=GMS_SUCCESS;
   else
      ret=GMS_ERROR;

   POP();

#undef SQL_DELETEDATAC
}




//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG addDataN(DMS_DATAN &data)
{
#define SQL_ADDDATAN "INSERT INTO DATA_N (OUNITID,VARIATID,DVALUE) VALUES (?,?,?)"

   PUSH(addDataN);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_ADDDATAN);

static DMS_DATAN _data;
static BOOL first_time=TRUE;
static LONG tlong;
   
   tlong = data.OUNITID;
   ret = existOunitid( tlong);
   if (ret!=DMS_SUCCESS) {
       POP2(DMS_INVALID_ID);
   }
   tlong = data.VARIATID;
   ret = existVariatid(tlong);
   if (ret!=DMS_SUCCESS) {
	   POP2(DMS_INVALID_ID);
   }
   _data= data;
   if (first_time)
   {
      local.BindParam(1,_data.OUNITID);
      local.BindParam(2,_data.VARIATID);
      local.BindParam(3,_data.DVALUE);
      first_time=FALSE;
   }


   if (local.Execute())
      ret=GMS_SUCCESS;
   else
      ret=GMS_ERROR;

   POP();

#undef SQL_ADDDATAN
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG addDataN2(DMS_DATAN &data)
{
#define SQL_ADDDATAN2 "INSERT INTO DATA_N (OUNITID,VARIATID,DVALUE) VALUES (?,?,?)"

   PUSH(addDataN2);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_ADDDATAN2);

static DMS_DATAN _data;
static BOOL first_time=TRUE;
static LONG tlong;
   
   tlong = data.OUNITID;
   _data= data;
   if (first_time)
   {
      local.BindParam(1,_data.OUNITID);
      local.BindParam(2,_data.VARIATID);
      local.BindParam(3,_data.DVALUE);
      first_time=FALSE;
   }


   if (local.Execute())
      ret=GMS_SUCCESS;
   else
      ret=GMS_ERROR;

   POP();

#undef SQL_ADDDATAN2
}



//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG updateDataN(DMS_DATAN &data)
{
#define SQL_UPDATEDATAN "UPDATE DATA_N SET DVALUE= ? WHERE OUNITID =? AND VARIATID=? "

   PUSH(updateDataN);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_UPDATEDATAN);
static CODBCbindedStmt &central = _centralDMS->BindedStmt(SQL_UPDATEDATAN);

static DMS_DATAN _data;
static BOOL first_time=TRUE;

   if (first_time)
   {
      BINDPARAM(1,_data.DVALUE);
      BINDPARAM(2,_data.OUNITID);
      BINDPARAM(3,_data.VARIATID);
      first_time=FALSE;
   }

   _data = data; 
   if (local.Execute())
      ret=GMS_SUCCESS;
   else
      if (central.Execute())
         ret = GMS_SUCCESS;
      else
         ret=GMS_ERROR;

   POP();

#undef SQL_UPDATEDATAN
}


//////////////////////////////////////////////////////////////////////////////
//deleteDataN  - deletes a record from DataN table based on the give observation unitid and variate id
//               if observation unit in the one only specified (variatid=0), all records for that observation unit are deleted
//Parameters:  
//      In:  data.OUNITID, data.VARIATID
//Programmed:  May15, 2003 
//Version:     4.0.2 
//////////////////////////////////////////////////////////////////////////////
LONG deleteDataN(LONG ounit, LONG variatid)
{
#define SQL_DELETEDATAN "DELETE FROM DATA_N  WHERE OUNITID =? AND (VARIATID=? or 0=?) "

   PUSH(deleteDataN);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_DELETEDATAN);

static LONG _ounit,_variatid;
static BOOL first_time=TRUE;

   if (first_time)
   {
      local.BindParam(1,_ounit);
      local.BindParam(2,_variatid);
      local.BindParam(3,_variatid);
      first_time=FALSE;
   }

   _ounit = ounit; 
   _variatid = variatid;
    if (local.Execute())
      ret=GMS_SUCCESS;
   else
         ret=GMS_ERROR;

   POP();

#undef SQL_DELETEDATAN
}



//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG getDataC(BOOL fLocal, DMS_DATAC &data)
{
#define SQL_GETDATAC "\
   SELECT DVALUE FROM DATA_C WHERE OUNITID = ?  AND VARIATID = ? "

   PUSH(getDataC);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_GETDATAC);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_GETDATAC);
       CODBCbindedStmt *source;

static DMS_DATAC _data;
    
   _data = data;
   BINDS(1,_data.DVALUE,DMS_DATA_VALUE-1);
   BINDPARAM(1,_data.OUNITID);
   BINDPARAM(2,_data.VARIATID);
 
   local.Execute();
   central.Execute();

   source = (fLocal)?&local:&central;

   if (source->Fetch()){
      data = _data;
   }
   else if (source->NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;

   POP();

#undef SQL_GETDATAC
}




//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG getDataN(BOOL fLocal, DMS_DATAN &data)
{
#define SQL_GETDATAN "\
   SELECT DVALUE FROM DATA_N WHERE OUNITID = ?  AND VARIATID = ? "

   PUSH(getDataN);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_GETDATAN);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_GETDATAN);
       CODBCbindedStmt *source;

static DMS_DATAN _data;
    
   _data = data;
   BIND(1,_data.DVALUE);
   BINDPARAM(1,_data.OUNITID);
   BINDPARAM(2,_data.VARIATID);
 
   local.Execute();
   central.Execute();

   source = (fLocal)?&local:&central;

   if (source->Fetch()){
      data = _data;
   }
   else if (source->NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;

   POP();

#undef SQL_GETDATAN
}


//////////////////////////////////////////////////////////////////////////////
//Retrieves the observation units and values of a given variate
//////////////////////////////////////////////////////////////////////////////
LONG getOunitDataC(BOOL fLocal, DMS_DATAC &data, LONG fOpt )
{
#define SQL_GETDATAC "\
   SELECT OUNITID, VARIATID, DVALUE FROM DATA_C WHERE  VARIATID = ? "

   PUSH(getOunitDataC);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_GETDATAC);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_GETDATAC);
       CODBCbindedStmt *source;

static DMS_DATAC _data;
    
   _data = data;
   if (fOpt = FIND_FIRST) {
     BIND(1,_data.OUNITID);
     BIND(2,_data.VARIATID);
     BINDS(3,_data.DVALUE,DMS_DATA_VALUE-1);
     BINDPARAM(1,_data.VARIATID);
 
     local.Execute();
     central.Execute();
   }

   source = (fLocal)?&local:&central;

   if (source->Fetch()){
      data = _data;
   }
   else if (source->NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;

   POP();

#undef SQL_GETDATAC
}




//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG setDataN(DMS_DATAN &data)
{
#define SQL_SETDATAN "UPDATE DATA_N  \
	set DVALUE=? WHERE OUNITID=? AND VARIATID=?"

   PUSH(setDataN);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_SETDATAN);

static DMS_DATAN _data;
static BOOL first_time=TRUE;
   
   _data= data;
   if (first_time)
   {
      local.BindParam(1,_data.DVALUE);
      local.BindParam(2,_data.OUNITID);
      local.BindParam(3,_data.VARIATID);
      first_time=FALSE;
   }


   if (local.Execute())
      ret=GMS_SUCCESS;
   else
      ret=GMS_ERROR;

   POP();

#undef SQL_SETDATAN
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG setDataC(DMS_DATAC &data)
{
#define SQL_SETDATAC "UPDATE DATA_C \
	    set DVALUE=? WHERE OUNITID=?AND VARIATID=?"

   PUSH(setDataC);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_SETDATAC);

static DMS_DATAC _data;
static BOOL first_time=TRUE;

   if (first_time)
   {
      local.BindParam(1,_data.DVALUE,sizeof(_data.DVALUE)-1);
      local.BindParam(2,_data.OUNITID);
      local.BindParam(3,_data.VARIATID);
      first_time=FALSE;
   }

   _data= data;

   if (local.Execute())
      ret=GMS_SUCCESS;
   else
      ret=GMS_ERROR;

   POP();

#undef SQL_SETDATAC
}



////////////////////////////////////////////////////////////////////////////////
////
////////////////////////////////////////////////////////////////////////////////
//LONG getObsunit(BOOL fLocal, DMS_OBSUNIT &data,  LONG fSearchOption)
//{
//#define SQL_GETOBSUNIT "\
//        SELECT DISTINCT O.OUNITID FROM OINDEX O,FACTOR F \
//               WHERE F.STUDYID = ? AND O.FACTORID = F.FACTORID \
//               ORDER BY O.OUNITID"
//#define SQL_GETOBSUNITL "\
//        SELECT DISTINCT O.OUNITID FROM OINDEX O,FACTOR F \
//               WHERE F.STUDYID = ? AND O.FACTORID = F.FACTORID \
//               ORDER BY O.OUNITID DESC"
//
//   PUSH(getObsunit);
//
//static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_GETOBSUNIT);
//static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_GETOBSUNITL);
//       CODBCbindedStmt *source;
//
//static DMS_OBSUNIT _data;
//static BOOL first_time=TRUE;
//
//
//   if (fSearchOption==FIND_FIRST)
//   {
//      if (first_time)
//      {
//         BIND(1,_data.OUNITID);
//         BINDPARAM(1,_data.STUDYID);
// 
//         first_time=FALSE;
//      }
//	  _data = data;
//      local.Execute();
//      central.Execute();
//   }
//
//   source = (fLocal)?&local:&central;
//
//   if (source->Fetch())
//      data = _data;
//   else if (source->NoData())
//      ret=DMS_NO_DATA;
//   else 
//      ret=DMS_ERROR;
//
//   POP();
//
//#undef SQL_GETOBSUNIT
//#undef SQL_GETOBSUNITL
//}


//////////////////////////////////////////////////////////////////////////////
// getOunit - retrieves all observation units of the specified levelno
//////////////////////////////////////////////////////////////////////////////
LONG getOunit(BOOL fLocal, LONG *ounitid,  LONG levelno, LONG fSearchOption)
{
#define SQL_getOunit "\
        SELECT  O.OUNITID FROM OINDEX O \
               WHERE O.LEVELNO = ? "

   PUSH(getOunit);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_getOunit);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_getOunit);
       CODBCbindedStmt *source;

static long tounit, tlevelno;
static BOOL first_time=TRUE;


   if (fSearchOption==FIND_FIRST)
   {
      if (first_time)
      {
         BIND(1,tounit);
         BINDPARAM(1,tlevelno);
 
         first_time=FALSE;
      }
      tlevelno = levelno;

      local.Execute();
      central.Execute();
   }
   source = (fLocal)?&local:&central;

   if (source->Fetch())
      *ounitid = tounit;
   else if (source->NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;

   POP();

#undef SQL_getOunit
}



//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG getOindex(BOOL fLocal, LONG studyid, DMS_OINDEX &data,  LONG fSearchOption)
{
#define SQL_GETOBSUNIT "\
        SELECT DISTINCT O.OUNITID, O.FACTORID, O.LEVELNO, O.REPRESNO FROM OINDEX O,FACTOR F \
               WHERE F.STUDYID = ? AND O.FACTORID = F.FACTORID \
               ORDER BY O.OUNITID"
#define SQL_GETOBSUNITL "\
        SELECT DISTINCT O.OUNITID, O.FACTORID, O.LEVELNO, O.REPRESNO FROM OINDEX O,FACTOR F \
               WHERE F.STUDYID = ? AND O.FACTORID = F.FACTORID \
               ORDER BY O.OUNITID DESC"

   PUSH(getObsunit);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_GETOBSUNIT);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_GETOBSUNITL);
       CODBCbindedStmt *source;

static DMS_OINDEX _data;
static LONG _studyid;
static BOOL first_time=TRUE;


   if (fSearchOption==FIND_FIRST)
   {
      if (first_time)
      {
         BIND(1,_data.OUNITID);
		 BIND(2,_data.FACTORID);
		 BIND(3,_data.LEVELNO);
		 BIND(4,_data.REPRESNO);
         BINDPARAM(1,_studyid);
 
         first_time=FALSE;
      }
	  _studyid = studyid;
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

#undef SQL_GETOINDEX
#undef SQL_GETOINDEXL
}


//////////////////////////////////////////////////////////////////////////////
// getCount - retrieves all observation units of the specified levelno
//////////////////////////////////////////////////////////////////////////////
LONG getCount(BOOL fLocal, LONG *ounitid,  LONG studyid, LONG nfac, LONG fSearchOption)
{
#define SQL_GETCOUNT "\
           SELECT O.OUNITID, COUNT(O.OUNITID) FROM OINDEX O, FACTOR F \
               WHERE  O.FACTORID = F.LABELID AND F.STUDYID = ? \
               GROUP BY O.OUNITID HAVING Count(O.OUNITID) = ? " 
   PUSH(getCount);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_GETCOUNT);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_GETCOUNT);
       CODBCbindedStmt *source;

static long tounit, tstudyid, tnfac;
static BOOL first_time=TRUE;


   if (fSearchOption==FIND_FIRST)
   {
      if (first_time)
      {
         BIND(1,tounit);
         BINDPARAM(1, tstudyid);
         BINDPARAM(2, tnfac); 
 
         first_time=FALSE;
      }

      local.Execute();
      central.Execute();
   }
   tstudyid = studyid;
   tnfac = nfac;
   source = (fLocal)?&local:&central;

   if (source->Fetch())
      *ounitid = tounit;
   else if (source->NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;

   POP();

#undef SQL_GETCOUNT
}

//////////////////////////////////////////////////////////////////////////////
// describeOunit - retrieves the factors defining an observation unit
//////////////////////////////////////////////////////////////////////////////
LONG describeOunit(BOOL fLocal, LONG ounitid,  LONG *factorid, LONG *levelno, LONG fSearchOption)
{
#define SQL_DESCRIBEOUNIT "\
        SELECT O.FACTORID, O.LEVELNO  FROM OINDEX O \
               WHERE  O.OUNITID = ? " 
	PUSH(describeOunit);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_DESCRIBEOUNIT);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_DESCRIBEOUNIT);
       CODBCbindedStmt *source;

static long tounit,tfactorid, tlevelno;
static BOOL first_time=TRUE;


   if (fSearchOption==FIND_FIRST)
   {
      if (first_time)
      {
         BIND(1, tfactorid);
         BIND(2, tlevelno);
         BINDPARAM(1,tounit);
 
         first_time=FALSE;
      }
      tounit = ounitid;
      local.Execute();
      central.Execute();
   }
   source = (fLocal)?&local:&central;

   if (source->Fetch()){
      ret = DMS_SUCCESS;
      *factorid = tfactorid;
      *levelno = tlevelno;
   }
   else if (source->NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;

   POP();

#undef SQL_DESCRIBEOUNIT
}

//////////////////////////////////////////////////////////////////////////////
// describeOunit - retrieves the factors defining an observation unit
//////////////////////////////////////////////////////////////////////////////
LONG describeOunitRepres(BOOL fLocal, LONG repres, LONG ounitid,  LONG *factorid, LONG *levelno, LONG fSearchOption)
{
#define SQL_DESCRIBEOUNITREPRES "\
        SELECT O.FACTORID, O.LEVELNO  FROM OINDEX O \
               WHERE  REPRESNO = ? AND O.OUNITID = ? " 
	PUSH(describeOunitRepres);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_DESCRIBEOUNITREPRES);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_DESCRIBEOUNITREPRES);
       CODBCbindedStmt *source;

static long tounit,tfactorid, tlevelno, trepres;
static BOOL first_time=TRUE;


   if (fSearchOption==FIND_FIRST)
   {
      if (first_time)
      {
         BIND(1, tfactorid);
         BIND(2, tlevelno);
         BINDPARAM(1,trepres);
         BINDPARAM(2,tounit);
 
         first_time=FALSE;
      }
      tounit = ounitid;
	  trepres = repres;
      local.Execute();
      central.Execute();
   }
   source = (fLocal)?&local:&central;

   if (source->Fetch()){
      ret = DMS_SUCCESS;
      *factorid = tfactorid;
      *levelno = tlevelno;
   }
   else if (source->NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;

   POP();

#undef SQL_DESCRIBEOUNITREPRES
}


/*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
//  getFactorNDataN - Gets the data of the specified factor value
//                  - Called by DMS_getFactorNDataN function
//////////////////////////////////////////////////////////////////////////////
LONG getDataNOfFactorN(BOOL fLocal, LONG labelid, LONG variateid, double lvalue,  double *datan)
{
#define SQL_getFactorNDataN " \
          SELECT DATA_N.DVALUE \
          FROM FACTOR, LEVEL_N, OINDEX, DATA_N, VARIATE \
          WHERE ( LEVEL_N.LVALUE = ? AND  ((FACTOR.LABELID)=?) AND ((VARIATE.VARIATID)=?) AND  \
		  ((VARIATE.VARIATID)=DATA_N.Variatid) AND ((DATA_N.OUNITID)=OINDEX.ounitid) AND ((LEVEL_N.LEVELNO)=OINDEX.LEVELNO) AND ((LEVEL_N.LABELID)=FACTOR.LABELID)) "

   PUSH(getFactorNDataN);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_getFactorNDataN);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_getFactorNDataN);
       CODBCbindedStmt *source;

static double _lvalue, _datan;
static LONG _labelid, _variateid;
static BOOL first_time=TRUE;
    
    _labelid = labelid;
	_variateid = variateid;
	_lvalue = lvalue;
	_datan = 0;
	if (first_time) {
		BIND(1,_datan);
		BINDPARAM(1,_lvalue);
		BINDPARAM(2, _labelid);
		BINDPARAM(3,_variateid);
	}
 
   local.Execute();
   central.Execute();

   source = (fLocal)?&local:&central;

   if (source->Fetch()){
      *datan = _datan;
   }
   else if (source->NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;

   POP();

#undef SQL_getFactorNDataN
}


/*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
//  getFactorNDataC - Gets the data of the specified factor value
//                  - Called by DMS_getFactorNDataN function
//////////////////////////////////////////////////////////////////////////////
LONG getDataCOfFactorN(BOOL fLocal, LONG labelid, LONG variateid, double lvalue,  CHAR *datac)
{
#define SQL_getFactorNDataC " \
          SELECT DATA_C.DVALUE \
          FROM FACTOR, LEVEL_N, OINDEX, DATA_C, VARIATE \
          WHERE ( LEVEL_N.LVALUE = ? AND  ((FACTOR.LABELID)=?) AND ((VARIATE.VARIATID)=?) AND  \
		  ((VARIATE.VARIATID)=DATA_C.Variatid) AND ((DATA_C.OUNITID)=OINDEX.ounitid) AND ((LEVEL_N.LEVELNO)=OINDEX.LEVELNO) AND ((LEVEL_N.LABELID)=FACTOR.LABELID)) "

   PUSH(getDataCOfFactorN);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_getFactorNDataC);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_getFactorNDataC);
       CODBCbindedStmt *source;

static double _lvalue;
static CHAR _datac[255];
static LONG _labelid, _variateid;
static BOOL first_time=TRUE;
    
    _labelid = labelid;
	_variateid = variateid;
	_lvalue = lvalue;
	if (first_time) {
		BINDS(1,_datac, 255);
		BINDPARAM(1,_lvalue);
		BINDPARAM(2, _labelid);
		BINDPARAM(3,_variateid);
	}
 
   local.Execute();
   central.Execute();

   source = (fLocal)?&local:&central;

   if (source->Fetch()){
      strncpy(datac,_datac,strlen(_datac));
   }
   else if (source->NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;

   POP();

#undef SQL_getFactorNDataC
}




/*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
//  DMS_minOunit - Gets the minimum observation unit 
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_minOunit(LONG *ounitid)
{
   PUSH(DMS_minOunit);
   *ounitid=getNextOunitID(LOCAL);
   ret = DMS_SUCCESS;
   POP();
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_addObsUnit(DMS_OBSUNIT* dat)
{
   PUSH(DMS_addObsUnit);
   LOG( "Before addObsunit (ounitid, effectid) ");
   LOGF("Ounitid: %d \n",dat->OUNITID);  //AMP: Debug
   LOGF("effectid: %d \n", dat->EFFECTID); 

   ret = addObsUnit(dat);
   LOG( "After addObsunit (ounitid, effectid) ");
   LOGF("Ounitid: %d \n",dat->OUNITID);  //AMP: Debug
   LOGF("effectid: %d \n", dat->EFFECTID); 
   LOGF("Return: %d \n", ret); 

   DMS_commitData;
   POP();
}

//////////////////////////////////////////////////////////////////////////////
//      DMS_findObsunit: determines if the combination of effect and ounitid exists
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_findObsunit(long ounitid, long effectid)
{

   PUSH(DMS_findObsunit);
   if (ounitid <0) 
	   ret =  findObsunit(LOCAL,ounitid, effectid);
   else {
	   ret =  findObsunit(LOCAL,ounitid, effectid);
       if (ret==DMS_NO_DATA)
	      ret =  findObsunit(CENTRAL,ounitid, effectid);
   }
   POP();
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_addOindex(DMS_OINDEX * dat)
{
   PUSH(DMS_addOindex);
   ret = addOindex(*dat);
   POP();
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_addDataN(DMS_DATAN * dat)
{
   PUSH(DMS_addDataN);
   ret = addDataN(*dat);
   POP();
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_addDataN2(DMS_DATAN * dat)
{
   PUSH(DMS_addDataN2);
   ret = addDataN2(*dat);
   POP();
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_deleteDataN(LONG ounit, LONG variatid)
{
   PUSH(DMS_deleteDataN);
   ret = deleteDataN(ounit, variatid);
   POP();
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_updateDataN(DMS_DATAN * dat)
{
   PUSH(DMS_updateDataN);
   ret = updateDataN(*dat);
   POP();
}



//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_addDataC(DMS_DATAC * dat)
{
   PUSH(DMS_addDataC);
   ret = addDataC(*dat);
   POP();
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_addDataC2(DMS_DATAC * dat)
{
   PUSH(DMS_addDataC2);
   ret = addDataC2(*dat);
   POP();
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_deleteDataC(LONG ounit, LONG variatid)
{
   PUSH(DMS_deleteDataC);
   ret = deleteDataC(ounit, variatid);
   POP();
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_updateDataC(DMS_DATAC * dat)
{
   PUSH(DMS_updateDataC);
   ret = updateDataC(*dat);
   POP();
}


//////////////////////////////////////////////////////////////////////////////
// DMS_getDataC - gets the character data value of a given ounitid and variatid;
//                there is no find option since there should only be one value for a variatid & ouinitid
//        input : pdatc.ounitid, pdatc.variatid
//        output: pdatc.dvalue
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getDataC(DMS_DATAC * dat)
{
   PUSH(DMS_getDataC);
   if (dat->OUNITID <0 || dat->VARIATID<0) 
		ret=getDataC(LOCAL,*dat);
   else ret=getDataC(CENTRAL,*dat);
   POP();
}


//////////////////////////////////////////////////////////////////////////////
// DMS_getDataN - gets the numeric data value of a given ounitid and variatid;
//                there is no find option since there should only be one value for a variatid & ouinitid
//        input : pdatc.ounitid, pdatc.variatid
//        output: pdatc.dvalue
//////////////////////////////////////////////////////////////////////////////
  LONG DLL_INTERFACE DMS_getDataN(DMS_DATAN * dat)
{
   PUSH(DMS_getDataN);
	if (dat->OUNITID <0 || dat->VARIATID<0) 
		ret=getDataN(LOCAL,*dat);
	else ret=getDataN(CENTRAL,*dat);
    POP();
}



//////////////////////////////////////////////////////////////////////////////
// DMS_getOunitDataC - gets the observation units of a variate;
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getOunitDataC(DMS_DATAC * dat , LONG fSearchOption)

{
    PUSH(DMS_getOunitDataC);
   static BOOL fLocal=TRUE;

   if (fSearchOption==FIND_FIRST)
         fLocal=TRUE;
   if (dat->VARIATID < 0 ) {
      ret=getOunitDataC(LOCAL,*dat,fSearchOption);
   }
   else {
	if (fLocal)
	{
      ret=getOunitDataC(LOCAL,*dat,fSearchOption);
      if (ret!=DMS_SUCCESS){
         fLocal = FALSE; 
         ret=getOunitDataC(CENTRAL,*dat,FIND_NEXT);
      }
	}
	else
      ret=getOunitDataC(CENTRAL,*dat,fSearchOption);
   }
   POP();
}




//////////////////////////////////////////////////////////////////////////////
// DMS_getObsunit - gets the observation units of study;
//////////////////////////////////////////////////////////////////////////////
//LONG DLL_INTERFACE DMS_getObsunit(LONG studyid, DMS_OBSUNIT *dat, LONG fSearchOption)
//
//{
//    PUSH(DMS_getObsunit);
//   static BOOL fLocal=TRUE;
//
//   if (fSearchOption==FIND_FIRST)
//         fLocal=TRUE;
//   if (fLocal)
//   {
//      ret=getObsunit(LOCAL,*dat,fSearchOption);
//      if (ret!=DMS_SUCCESS){
//         fLocal = FALSE; 
//         ret=getObsunit(CENTRAL,*dat,FIND_NEXT);
//      }
//   }
//   else
//      ret=getObsunit(CENTRAL,*dat,fSearchOption);
//   POP();
//}


//////////////////////////////////////////////////////////////////////////////
// DMS_getOIndex - gets the OINDEX records for a give study id
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getOIndex(LONG styid, DMS_OINDEX *dat, LONG fSearchOption)

{
    PUSH(DMS_getOindex);
   static BOOL fLocal=TRUE;

   if (fSearchOption==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal)
   {
      ret=getOindex(LOCAL,styid, *dat,fSearchOption);
      if (ret!=DMS_SUCCESS){
         fLocal = FALSE; 
         ret=getOindex(CENTRAL,styid, *dat,FIND_NEXT);
      }
   }
   else
      ret=getOindex(CENTRAL,styid, *dat,fSearchOption);
   POP();
}


LONG DLL_INTERFACE DMS_describeOunit(LONG ounitid, struct source *dsource, LONG *nsrc,LONG fopt )
//LONG DLL_INTERFACE DMS_describeOunit(LONG studyid, LONG *varlist, char *varval[MAXVALUEC], LONG nsrc,LONG fopt )
{
    PUSH(DMS_describeOunit);
	long tret,varid,i=0;
	long tval=0;

	if (ounitid < 0) {
	  tret = describeOunit(LOCAL,ounitid,&varid,&tval,FIND_FIRST);
      while (tret==DMS_SUCCESS && i < *nsrc){
         dsource[i].facid = varid;
		 dsource[i].levelno = tval;
	     tret = describeOunit(LOCAL,ounitid,&varid,&tval,FIND_NEXT);
         i++;
	  }
	  if (tret == DMS_ERROR) return DMS_ERROR;
	}
	else {
	  tret = describeOunit(CENTRAL,ounitid,&varid,&tval,FIND_FIRST);
      while (tret==DMS_SUCCESS && i < *nsrc){
         dsource[i].facid = varid;
		 dsource[i].levelno = tval;
	     tret = describeOunit(CENTRAL,ounitid,&varid,&tval,FIND_NEXT);
         i++;
	  }
	  if (tret == DMS_ERROR) return DMS_ERROR;
	}
	if (i>0) {
	   *nsrc = i;
       ret= DMS_SUCCESS;
	}
	  else ret= DMS_NO_DATA;
   POP();
}


LONG DLL_INTERFACE DMS_existCombination(long studyid, struct source *dsource, long *ounitid, LONG nsource)
{
    PUSH(DMS_existCombination);
	long tag[50],i,ntsrc=50,varid,tval,texist,tlev=dsource[0].levelno, nlev,tret,tounitid;
	ret = 0;
    nlev = 0;     //July 26, 2000 
	if (studyid > 0) {
       tret = getOunit(CENTRAL, &tounitid, tlev, FIND_FIRST);  
 	   while (tret == DMS_SUCCESS) {
		for (i=0;i<nsource;i++) {
			tag[i]= 0;
		}
		nlev = 0;
        tret = describeOunit(CENTRAL,tounitid,&varid,&tval,FIND_FIRST);
		while (tret == DMS_SUCCESS)  {
			nlev++;
			for (i=0;i<nsource;i++) {
				  if ((dsource[i].facid == varid) && (dsource[i].levelno == tval)) {
					tag[i] = 1;
					break;
				  }
			}
            tret = describeOunit(CENTRAL,tounitid,&varid,&tval,FIND_NEXT);
		}
        texist = 1;
		for (i=0;i<nsource;i++) {
			texist = texist && tag[i];
		}
		if ((texist==1) && (nlev == nsource))  {
			*ounitid = tounitid; 
			ret= DMS_SUCCESS;
            tret = DMS_EXIST;
		}
        else tret = getOunit(CENTRAL, &tounitid, tlev, FIND_NEXT);  

	   }
	}
	else {
       tret = getOunit(LOCAL, &tounitid, tlev, FIND_FIRST);  
	   while (tret == DMS_SUCCESS) {
		for (i=0;i<nsource;i++) {
			tag[i]= 0;
		}
		nlev=0;
        tret = describeOunit(LOCAL,tounitid,&varid,&tval,FIND_FIRST);
		while (tret == DMS_SUCCESS)  {
			nlev++;
			for (i=0;i<nsource;i++) {
				 if ((dsource[i].facid == varid) && (dsource[i].levelno == tval)) {
					tag[i] = 1;
					break;
				 }
			}
            tret = describeOunit(LOCAL,tounitid,&varid,&tval,FIND_NEXT);
		}
        texist = 1;
		for (i=0;i<nsource;i++) {
			texist = texist && tag[i];
		}
        if (fFile!=NULL) fprintf(fFile,"\n");
		if ((texist==1) && (nlev == nsource))  {
			*ounitid = tounitid; 
			ret= DMS_SUCCESS;
			tret = DMS_EXIST;
		}
        else tret = getOunit(LOCAL, &tounitid, tlev,FIND_NEXT);  
	   }
	}
    if (ret!=DMS_SUCCESS) {
	   if (tret== DMS_NO_DATA)  ret= DMS_NO_DATA;
	   else ret= DMS_ERROR;
    }
	POP();
}


LONG DLL_INTERFACE DMS_getOunitEq(long *ounitid, long studyid, long nsource, long fopt)
{
	PUSH(DMS_getOunitEq);
	if (studyid > 0) 
       ret= getCount(CENTRAL, ounitid, studyid, nsource,fopt);
	else
       ret= getCount(LOCAL, ounitid, studyid, nsource, fopt);
	return ret;

}



//////////////////////////////////////////////////////////////////////////////
// DMS_getFactorNDataN - gets data of the given factor level value and variate
//        input : labelid, variate id, level value
//        output: datan
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getDataNOfFactorN(LONG labelid, LONG variateid, double lvalue,  double *datan)
{
   PUSH(DMS_getDataNOfFactorN);
   if ((labelid <0) || (variateid <0))
	   ret =  getDataNOfFactorN(LOCAL, labelid, variateid, lvalue,  datan);
   else {
	   ret =  getDataNOfFactorN(LOCAL, labelid, variateid, lvalue,  datan);
       if (ret==DMS_NO_DATA)
	      ret =  getDataNOfFactorN(CENTRAL, labelid, variateid, lvalue,  datan);
   }
   POP();
}

//////////////////////////////////////////////////////////////////////////////
// DMS_getFactorNDataC - gets character data of the given factor level value and variate
//        input : labelid, variate id, level value
//        output: datac
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getDataCOfFactorN(LONG labelid, LONG variateid, double lvalue,  CHAR *datac)
{
   PUSH(DMS_getDataCOfFactorN);
   if ((labelid <0) || (variateid <0))
	   ret =  getDataCOfFactorN(LOCAL, labelid, variateid, lvalue,  datac);
   else {
	   ret =  getDataCOfFactorN(LOCAL, labelid, variateid, lvalue,  datac);
       if (ret==DMS_NO_DATA)
	      ret =  getDataCOfFactorN(CENTRAL, labelid, variateid, lvalue,  datac);
   }
   POP();
}




//////////////////////////////////////////////////////////////////////////////
//added by WVC
//modified by AMP (2005/12/06) : Use a different SQL statement for Oracle
//            AMP (2008/05) : The SQL for mySQL and PostGres was corrected
//***** Gets all data values of the constants belonging to one representation *****/
//Input:  data.REPRESNO
//Output: constant data record
//Called by DMS_getConsData
//////////////////////////////////////////////////////////////////////////////
LONG getConsData(BOOL fLocal, DMS_CONSDATA *data, int fOpt)
{
	#define SQL_GETCONSDATA1 \
		"SELECT REPRESNO, VEFFECT.VARIATID, DVALUE \
		FROM VEFFECT, DATA_N \
		WHERE (((VEFFECT.VARIATID)=DATA_N.VARIATID) AND ((VEFFECT.REPRESNO)=?)) \
		UNION \
		(SELECT REPRESNO, VEFFECT.VARIATID, DVALUE \
		FROM VEFFECT, DATA_C \
		WHERE (((VEFFECT.VARIATID)=DATA_C.VARIATID) AND ((VEFFECT.REPRESNO)=?))) \
		ORDER BY VEFFECT.VARIATID ASC"
	#define SQL_GETCONSDATA1_Oracle \
		"SELECT V.REPRESNO, V.VARIATID, {fn CONVERT(D.DVALUE,SQL_VARCHAR)}   \
		FROM VEFFECT V, DATA_N D\
		WHERE (((V.VARIATID)=D.VARIATID) AND ((V.REPRESNO)=?)) \
		UNION \
		(SELECT V.REPRESNO, V.VARIATID, D.DVALUE \
		FROM VEFFECT V, DATA_C D\
		WHERE (((V.VARIATID)=D.VARIATID) AND ((V.REPRESNO)=?))) \
		ORDER BY VARIATID ASC"
	#define SQL_GETCONSDATA1_Postgres \
	"SELECT V.REPRESNO, V.VARIATID,  {fn CAST(D.DVALUE as char)}   \
		FROM VEFFECT V, DATA_N D\
		WHERE (((V.VARIATID)=D.VARIATID) AND ((V.REPRESNO)=?)) \
		UNION \
		SELECT V.REPRESNO, V.VARIATID, D.DVALUE \
		FROM VEFFECT V, DATA_C D\
		WHERE (((V.VARIATID)=D.VARIATID) AND ((V.REPRESNO)=?)) \
		ORDER BY VARIATID ASC"



	#define SQL_GETCONSDATA2 \
		"SELECT REPRESNO, VEFFECT.VARIATID, DVALUE \
		FROM VEFFECT, DATA_N \
		WHERE (((VEFFECT.VARIATID)=DATA_N.VARIATID) AND ((VEFFECT.REPRESNO)=?)) \
		UNION \
		(SELECT REPRESNO, VEFFECT.VARIATID, DVALUE \
		FROM VEFFECT, DATA_C \
		WHERE (((VEFFECT.VARIATID)=DATA_C.VARIATID) AND ((VEFFECT.REPRESNO)=?))) \
		ORDER BY VEFFECT.VARIATID DESC"
	#define SQL_GETCONSDATA2_Oracle \
		"SELECT V.REPRESNO, V.VARIATID, {fn CONVERT(D.DVALUE,SQL_VARCHAR)}   \
		FROM VEFFECT V, DATA_N D\
		WHERE (((V.VARIATID)=D.VARIATID) AND ((V.REPRESNO)=?)) \
		UNION \
		(SELECT V.REPRESNO, V.VARIATID, D.DVALUE \
		FROM VEFFECT V, DATA_C D\
		WHERE (((V.VARIATID)=D.VARIATID) AND ((V.REPRESNO)=?))) \
		ORDER BY VARIATID DESC"
	#define SQL_GETCONSDATA2_Postgres \
	"SELECT V.REPRESNO, V.VARIATID, {fn CAST(D.DVALUE as char)}   \
		FROM VEFFECT V, DATA_N D\
		WHERE (((V.VARIATID)=D.VARIATID) AND ((V.REPRESNO)=?)) \
		UNION \
		SELECT V.REPRESNO, V.VARIATID, D.DVALUE \
		FROM VEFFECT V, DATA_C D\
		WHERE (((V.VARIATID)=D.VARIATID) AND ((V.REPRESNO)=?)) \
		ORDER BY VARIATID DESC"


	LONG ret = DMS_SUCCESS;
	
	static CODBCbindedStmt &central = _centralDMS->BindedStmt();
	static CODBCbindedStmt &local = _localDMS->BindedStmt();
	CODBCbindedStmt *source;
    LPCSTR szDB;	
    static string central_sql,local_sql;
	
	static DMS_CONSDATA _data;
	static BOOL first_time = TRUE;
	
	if (fOpt == FIND_FIRST) {
		if (first_time) {


	        szDB = _localDMS->DatabaseName();
	        if (strncmp(szDB, "Oracle",6)==0 ) {
	           local_sql=SQL_GETCONSDATA2_Oracle;
			}
		    else if ((strcmp(szDB, "PostgreSQL")==0 ) || (strcmp(szDB, "MySQL")==0 ) )
	           local_sql=SQL_GETCONSDATA2_Postgres;
			else
			   local_sql=SQL_GETCONSDATA2;

			szDB = _centralDMS->DatabaseName();
	        if (strncmp(szDB, "Oracle",6)==0 ) {
	           central_sql=SQL_GETCONSDATA1_Oracle;
			}
		    else if ((strcmp(szDB, "PostgreSQL")==0 ) || (strcmp(szDB, "MySQL")==0 ) )
	           central_sql=SQL_GETCONSDATA1_Postgres;
			else
			   central_sql=SQL_GETCONSDATA1;
   
			central.SetSQLstr(central_sql.c_str());
			local.SetSQLstr(local_sql.c_str());


			BINDPARAM(1, _data.REPRESNO);
			BINDPARAM(2, _data.REPRESNO);
			
			BIND(1, _data.REPRESNO);
			BIND(2, _data.VARIATID);
			BINDS(3, _data.DVALUE, DMS_DATA_VALUE);
			
			first_time = FALSE;
		}
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
	
	#undef SQL_GETCONSDATA1
	#undef SQL_GETCONSDATA1_Oracle
	#undef SQL_GETCONSDATA1_Postgres

	#undef SQL_GETCONSDATA2
	#undef SQL_GETCONSDATA2_Oracle
	#undef SQL_GETCONSDATA2_Postgres
}

//////////////////////////////////////////////////////////////////////////////
//added by WVC
//***** Gets all data values of the constants belonging to one representation *****/
//Input:  data.REPRESNO
//Output: constant data record
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getConsData(DMS_CONSDATA *data, int fOpt)
{
	static BOOL fLocal = TRUE;
	
	PUSH(DMS_getConsData);
	
	if (fOpt == FIND_FIRST) {
		fLocal = TRUE;
	}
	if (fLocal) {
		ret = getConsData(LOCAL, data, fOpt);
		if (ret != DMS_SUCCESS){
			fLocal = FALSE;
			ret = getConsData(CENTRAL, data, FIND_NEXT);
		}
	}
	else
		ret = getConsData(CENTRAL, data, fOpt);
	
	POP();
	
}

//////////////////////////////////////////////////////////////////////////////
//added by WVC
//modified by AMP (2005/12/06) : Use a different SQL statement for Oracle
//            AMP (2008/05) : modified the SQL for mySQL and Postgres
//***** Gets all data values belonging to one observation unit *****/
//Input:  data.REPRESNO, data.OUNITID
//Output: entire observation unit data record
//Called by DMS_getOunitData
//////////////////////////////////////////////////////////////////////////////
LONG getOunitData(BOOL fLocal, DMS_OUNITDATA *data, int fOpt)
{
	#define SQL_GETOUNITDATA1 \
		"SELECT REPRESNO, OUNITID, VEFFECT.VARIATID, DVALUE \
		FROM VEFFECT, DATA_N \
		WHERE (((VEFFECT.VARIATID)=DATA_N.VARIATID) \
		AND ((VEFFECT.REPRESNO)=?) AND ((DATA_N.OUNITID)=?)) \
		UNION \
		(SELECT REPRESNO, OUNITID, VEFFECT.VARIATID, DVALUE \
		FROM VEFFECT, DATA_C \
		WHERE (((VEFFECT.VARIATID)=DATA_C.VARIATID) \
		AND ((VEFFECT.REPRESNO)=?) AND ((DATA_C.OUNITID)=?))) \
		ORDER BY VEFFECT.VARIATID ASC"


	#define SQL_GETOUNITDATA1_Oracle \
	"SELECT V.REPRESNO, D.OUNITID, V.VARIATID, {fn CONVERT(D.DVALUE,SQL_VARCHAR)}    \
		FROM VEFFECT V, DATA_N D\
		WHERE (((V.VARIATID)=D.VARIATID) \
		AND ((V.REPRESNO)=?) AND ((D.OUNITID)=?)) \
		UNION \
		SELECT V.REPRESNO, D.OUNITID, V.VARIATID, D.DVALUE \
		FROM VEFFECT V, DATA_C D \
		WHERE (((V.VARIATID)=D.VARIATID) \
		AND ((V.REPRESNO)=?) AND ((D.OUNITID)=?)) \
		ORDER BY VARIATID ASC"
	#define SQL_GETOUNITDATA1_Postgres \
	"SELECT V.REPRESNO, D.OUNITID, V.VARIATID, {fn CAST(D.DVALUE as CHAR) }   \
		FROM VEFFECT V, DATA_N D\
		WHERE (((V.VARIATID)=D.VARIATID) \
		AND ((V.REPRESNO)=?) AND ((D.OUNITID)=?)) \
		UNION \
		SELECT V.REPRESNO, D.OUNITID, V.VARIATID, D.DVALUE \
		FROM VEFFECT V, DATA_C D \
		WHERE (((V.VARIATID)=D.VARIATID) \
		AND ((V.REPRESNO)=?) AND ((D.OUNITID)=?)) \
		ORDER BY VARIATID ASC"
		

	#define SQL_GETOUNITDATA2 \
		"SELECT REPRESNO, OUNITID, VEFFECT.VARIATID, DVALUE \
		FROM VEFFECT, DATA_N \
		WHERE (((VEFFECT.VARIATID)=DATA_N.VARIATID) \
		AND ((VEFFECT.REPRESNO)=?) AND ((DATA_N.OUNITID)=?)) \
		UNION \
		(SELECT REPRESNO, OUNITID, VEFFECT.VARIATID, DVALUE \
		FROM VEFFECT, DATA_C \
		WHERE (((VEFFECT.VARIATID)=DATA_C.VARIATID) \
		AND ((VEFFECT.REPRESNO)=?) AND ((DATA_C.OUNITID)=?))) \
		ORDER BY VEFFECT.VARIATID DESC"

	//	    "SELECT V.REPRESNO, D.OUNITID, V.VARIATID, {fn CONVERT(D.DVALUE,SQL_VARCHAR)}    
   #define SQL_GETOUNITDATA2_Oracle \
   "SELECT V.REPRESNO, D.OUNITID, V.VARIATID, trim(to_char(D.DVALUE,'999999999.99999')) \
		FROM VEFFECT V, DATA_N D\
		WHERE (((V.VARIATID)=D.VARIATID) \
		AND ((V.REPRESNO)=?) AND ((D.OUNITID)=?)) \
		UNION \
		SELECT V.REPRESNO, D.OUNITID, V.VARIATID, D.DVALUE \
		FROM VEFFECT V, DATA_C D \
		WHERE (((V.VARIATID)=D.VARIATID) \
		AND ((V.REPRESNO)=?) AND ((D.OUNITID)=?)) \
		ORDER BY VARIATID DESC"	

  #define SQL_GETOUNITDATA2_Postgres \
	"SELECT V.REPRESNO, D.OUNITID, D.VARIATID,{fn CAST(D.DVALUE as CHAR)}    \
		FROM VEFFECT V, DATA_N D\
		WHERE (((V.VARIATID)=D.VARIATID) \
		AND ((V.REPRESNO)=?) AND ((D.OUNITID)=?)) \
		UNION \
		SELECT V.REPRESNO, D.OUNITID, D.VARIATID, D.DVALUE \
		FROM VEFFECT V, DATA_C D \
		WHERE (((V.VARIATID)=D.VARIATID) \
		AND ((V.REPRESNO)=?) AND ((D.OUNITID)=?)) \
		ORDER BY VARIATID DESC"
	


	LONG ret = DMS_SUCCESS;
	
	static CODBCbindedStmt &central = _centralDMS->BindedStmt();
	static CODBCbindedStmt &local = _localDMS->BindedStmt();
	CODBCbindedStmt *source;

	LPCSTR szDB;
    static string central_sql,local_sql;
	static DMS_OUNITDATA _data;
	static BOOL first_time = TRUE;
	
	if (fOpt == FIND_FIRST) {
		if (first_time) {
	        szDB = _localDMS->DatabaseName();
	        if (strncmp(szDB, "Oracle",6)==0 ) {
	           local_sql=SQL_GETOUNITDATA2_Oracle;
			}
		    else if  ((strcmp(szDB, "MySQL")==0) || (strcmp(szDB, "PostgreSQL")==0) )
	           local_sql=SQL_GETOUNITDATA2_Postgres;
			else
			   local_sql=SQL_GETOUNITDATA2;

			szDB = _centralDMS->DatabaseName();
	        if (strncmp(szDB, "Oracle",6)==0 ) {
	           central_sql=SQL_GETOUNITDATA1_Oracle;
			}
		    else if  ((strcmp(szDB, "MySQL")==0) || (strcmp(szDB, "PostgreSQL")==0) )
	           central_sql=SQL_GETOUNITDATA1_Postgres;
			else
			   central_sql=SQL_GETOUNITDATA1;
   
			central.SetSQLstr(central_sql.c_str());
			local.SetSQLstr(local_sql.c_str());

			LOG(local_sql.c_str());
			BINDPARAM(1, _data.REPRESNO);
			BINDPARAM(2, _data.OUNITID);
			BINDPARAM(3, _data.REPRESNO);
			BINDPARAM(4, _data.OUNITID);
			
			BIND(1, _data.REPRESNO);
			BIND(2, _data.OUNITID);
			BIND(3, _data.VARIATID);
			BINDS(4, _data.DVALUE, DMS_DATA_VALUE);
			
			first_time = FALSE;
		}
		_data.REPRESNO = data->REPRESNO;
		_data.OUNITID = data->OUNITID;
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
	
	#undef SQL_GETOUNITDATA1
	#undef SQL_GETOUNITDATA2
	#undef SQL_GETOUNITDATA1_Oracle
	#undef SQL_GETOUNITDATA2_Postgres
	#undef SQL_GETOUNITDATA1_Oracle
	#undef SQL_GETOUNITDATA2_Postgres
}
//////////////////////////////////////////////////////////////////////////////
//added by WVC
//***** Gets all data values belonging to one observation unit *****/
//Input:  data.REPRESNO, data.OUNITID
//Output: entire observation unit data record
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getOunitData(DMS_OUNITDATA *data, int fOpt)
{
	static BOOL fLocal = TRUE;
	
	PUSH(DMS_getOunitData);
	
	if (fOpt == FIND_FIRST) {
		fLocal = TRUE;
	}
	if (fLocal) {
		ret = getOunitData(LOCAL, data, fOpt);
		if (ret != DMS_SUCCESS){
			fLocal = FALSE;
			ret = getOunitData(CENTRAL, data, FIND_NEXT);
		}
	}
	else
		ret = getOunitData(CENTRAL, data, fOpt);
	
	POP();
	
}
