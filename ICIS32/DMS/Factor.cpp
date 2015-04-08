/* Factor.cpp : Implements the functions to access and manipulate the FACTOR table of ICIS
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

#include "Factor.h"

extern LONG UACCESS,USERID;
extern CODBCdbc *_centralDMS,*_localDMS;  //Central and Local Database Connection
extern stack<char*> _routines_stack;


//////////////////////////////////////////////////////////////////////////////
//  Finds the minimum  labelid
//////////////////////////////////////////////////////////////////////////////
LONG getNextLabelID(void)
{
static LONG factorID=0;

   //if (!factorID)
   //{
      CODBCdirectStmt local = _localDMS->DirectStmt("SELECT MIN(LABELID) FROM FACTOR");
      local.Execute();
      local.Fetch();
      if (!local.NoData())
         factorID=local.Field(1).AsInteger();
   //}
   return --factorID;
}

//////////////////////////////////////////////////////////////////////////////
//  Finds the minimum  effectid
//////////////////////////////////////////////////////////////////////////////
LONG getNextRepresNo(void)
{
static LONG represNo=0;

   //if (!represNo)
   //{
      //CODBCdirectStmt local = _localDMS->DirectStmt("SELECT MIN(REPRESNO) FROM EFFECT");
      CODBCdirectStmt local = _localDMS->DirectStmt("SELECT MIN(REPRESNO) FROM REPRESTN");
      local.Execute();
      local.Fetch();
      if (!local.NoData())
         represNo=local.Field(1).AsInteger();
   //}
   return --represNo;
}



//////////////////////////////////////////////////////////////////////////////
//  Finds the minimum  effectid
//////////////////////////////////////////////////////////////////////////////
LONG getNextEffectID(LONG studyid)
{
//#define SQL_getEffectid " SELECT Min(EFFECT.EFFECTID) \
//                          FROM EFFECT, FACTOR  \
//                          WHERE (((EFFECT.FACTORID)=EFFECT.FACTORID) AND ((FACTOR.STUDYID)=?))"

static LONG effectid=0;

   //if (!effectid)
   //{
      CODBCdirectStmt local = _localDMS->DirectStmt("SELECT Min(EFFECTID)FROM STEFFECT ");
      local.Execute();
      local.Fetch();
      if (!local.NoData())
         effectid=local.Field(1).AsInteger();
  //}
   return --effectid;

#undef SQL_getEffectid
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG addRepresDefn(DMS_EFFECT data)
{
#define SQL_ADDEFFECT "\
   INSERT INTO EFFECT( EFFECTID, FACTORID, REPRESNO ) \
   values(?,?,?)"

   PUSH(addRepresDefn);

static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_ADDEFFECT);

static DMS_EFFECT _data;
static BOOL first_time=TRUE;

   if (first_time)
   {
      local.BindParam(1,_data.EFFECTID);
      local.BindParam(2,_data.FACTORID);
      local.BindParam(3,_data.REPRESNO);
      first_time=FALSE;
   }

   _data = data;

   if (local.Execute())
      ret=DMS_SUCCESS;
   else
      ret=DMS_ERROR;


   POP();

#undef SQL_ADDEFFECT
}




//////////////////////////////////////////////////////////////////////////////
//Input: REPRESNO and VARIATID
//////////////////////////////////////////////////////////////////////////////
LONG findVEffect(BOOL fLocal,DMS_VEFFECT &data)
{
#define SQL_FINDVEFFECT "\
   select REPRESNO, VARIATID from VEFFECT where (REPRESNO=?) and (VARIATID=?)" 
   
   PUSH(findVEffect);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_FINDVEFFECT);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_FINDVEFFECT);
       CODBCbindedStmt *source;

static DMS_VEFFECT _data;
static BOOL first_time=TRUE;


   if (first_time)
   {
         BINDPARAM(1,_data.REPRESNO);
         BINDPARAM(2,_data.VARIATEID);
         first_time=FALSE;
   }
   _data = data;
   local.Execute();
   central.Execute();

   source = (fLocal)?&local:&central;

   if (source->Fetch())
   {
      data = _data;
   }
   else if (source->NoData())
      ret=GMS_NO_DATA;
   else 
      ret= GMS_ERROR;

   POP();

}

//////////////////////////////////////////////////////////////////////////////
// findVEffect
// Created    : July 28, 2004
// Description: Find a record in the VEFFECT table. VEFFECT contains the variates in a data Representation 
//////////////////////////////////////////////////////////////////////////////

LONG DMS_findVEffect(DMS_VEFFECT data) {
	 PUSH(DMS_findVEffect);
     ret=findVEffect(LOCAL,data);
     if ((ret!=GMS_SUCCESS) && (data.REPRESNO <0 || data.VARIATEID <0)){
         ret=findVEffect(CENTRAL,data);
	 }
	 POP();
}


//////////////////////////////////////////////////////////////////////////////
// addVEffectRecord
// Created    : Oct. 14, 2002
// Description: Add a record in the VEFFECT table. VEFFECT contains the variates in a data Representation 
//////////////////////////////////////////////////////////////////////////////
LONG addVEffectRecord(DMS_VEFFECT data)
{
#define SQL_ADDVEFFECT "\
   INSERT INTO VEFFECT ( REPRESNO, VARIATID ) \
   values(?,?)"

   PUSH(addVEffectRecord);

static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_ADDVEFFECT);

static DMS_VEFFECT _data;
static BOOL first_time=TRUE;

   ret = DMS_findVEffect(data);
   if (ret == DMS_SUCCESS) 
	   POP2(DMS_EXIST);
   if (first_time)
   {
      local.BindParam(1,_data.REPRESNO);
      local.BindParam(2,_data.VARIATEID);
      first_time=FALSE;
   }

   _data = data;

   if (local.Execute())
      ret=DMS_SUCCESS;
   else
      ret=DMS_ERROR;


   POP();

#undef SQL_ADDEFFECT
}




//////////////////////////////////////////////////////////////////////////////
// Input:  Effect and Represno
// Output: Factor
//////////////////////////////////////////////////////////////////////////////
LONG getFactorEffect(BOOL fLocal,DMS_EFFECT &data, BOOL fSearchOption)
{
#define SQL_getFactorEffectC "\
   SELECT FACTORID FROM EFFECT \
   WHERE EFFECTID = ? AND REPRESNO=? ORDER BY FACTORID"
#define SQL_getFactorEffectL "\
   SELECT FACTORID FROM EFFECT \
   WHERE EFFECTID = ? AND REPRESNO=? ORDER BY FACTORID DESC"

   PUSH(GetFactorEffect);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_getFactorEffectC);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_getFactorEffectL);
       CODBCbindedStmt *source;

static DMS_EFFECT _data;
static BOOL first_time=TRUE;

   if (fSearchOption==FIND_FIRST){

      if (first_time)
      {
         BIND(1,_data.FACTORID);
         BINDPARAM(1, _data.EFFECTID);  
         BINDPARAM(2, _data.REPRESNO);  
         first_time=FALSE;
      }
      _data= data;

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

#undef SQL_getFactorEffectL
#undef SQL_getFactorEffectC
}



//////////////////////////////////////////////////////////////////////////////
// Input:  Factor 
// Output: Effect and Represno
//////////////////////////////////////////////////////////////////////////////
LONG getEffectFactor(BOOL fLocal,DMS_EFFECT &data, BOOL fSearchOption)
{
#define SQL_getEffectFactor "\
   SELECT EFFECTID, REPRESNO FROM EFFECT \
   WHERE FACTORID = ? "

   PUSH(getEffectFactor);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_getEffectFactor);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_getEffectFactor);
       CODBCbindedStmt *source;

static DMS_EFFECT _data;
static BOOL first_time=TRUE;

   if (fSearchOption==FIND_FIRST){

      if (first_time)
      {
         BIND(1,_data.EFFECTID);
         BIND(2, _data.REPRESNO);  
         BINDPARAM(1, _data.FACTORID);  
         first_time=FALSE;
      }
      _data= data;

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

#undef SQL_getEffectFactor
}


//////////////////////////////////////////////////////////////////////////////
// Input:  Representation
// Output: Observation Unit
//////////////////////////////////////////////////////////////////////////////
LONG getEffectOunit(BOOL fLocal,LONG represno, LONG *ounitid, BOOL fSearchOption)
{
#define SQL_getEffectOunit "\
       SELECT DISTINCT OINDEX.OUNITID \
       FROM OINDEX   \
       WHERE OINDEX.REPRESNO=? \
       ORDER BY OINDEX.OUNITID "
#define SQL_getEffectOunitL "\
       SELECT DISTINCT OINDEX.OUNITID \
       FROM OINDEX   \
       WHERE OINDEX.REPRESNO=? \
       ORDER BY OINDEX.OUNITID DESC "

   PUSH(getEffectOunit);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_getEffectOunit);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_getEffectOunitL);
       CODBCbindedStmt *source;

static LONG trepresno, tounitid;
static BOOL first_time=TRUE;

   if (fSearchOption==FIND_FIRST){

      if (first_time)
      {
         BIND(1,tounitid);
         BINDPARAM(1, trepresno);  
         first_time=FALSE;
      }
      trepresno = represno;

      local.Execute();
      central.Execute();
   }

   source = (fLocal)?&local:&central;

   if (source->Fetch())
      *ounitid = tounitid;
   else if (source->NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;

   POP();

#undef SQL_getEffectOunit
#undef SQL_getEffectOunitL

}


//////////////////////////////////////////////////////////////////////////////
// describeEffectOunit - retrieves the factors defining an observation unit for a given effectid
//////////////////////////////////////////////////////////////////////////////
LONG describeEffectOunit(BOOL fLocal, LONG ounitid, LONG represno,  LONG *factorid, LONG *levelno, LONG fSearchOption)
{
#define SQL_DESCRIBEOUNIT "\
        SELECT O.FACTORID, O.LEVELNO  FROM OINDEX O \
               WHERE O.REPRESNO = ? AND  O.OUNITID = ? " 
	PUSH(describeOunit);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_DESCRIBEOUNIT);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_DESCRIBEOUNIT);
       CODBCbindedStmt *source;

static long tounit,tfactorid, trepresno,tlevelno;
static BOOL first_time=TRUE;


   if (fSearchOption==FIND_FIRST)
   {
      if (first_time)
      {
         BIND(1, tfactorid);
         BIND(2, tlevelno);
		 BINDPARAM(1, trepresno);
         BINDPARAM(2,tounit);
 
         first_time=FALSE;
      }
      tounit = ounitid;
	  trepresno = represno;
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
// Input:  No of factors
// Output: Effect and Represno
//////////////////////////////////////////////////////////////////////////////
LONG getEffectCount(BOOL fLocal,LONG nfactor,LONG *represno, LONG *effectid,  BOOL fSearchOption)
{
#define SQL_getEffectCount "\
      SELECT EFFECT.REPRESNO, EFFECT.EFFECTID, Count(EFFECT.FACTORID)  \
      FROM EFFECT \
      GROUP BY EFFECT.REPRESNO, EFFECT.EFFECTID \
      HAVING Count(EFFECT.FACTORID)= ? "

   PUSH(getEffectCount);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_getEffectCount);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_getEffectCount);
       CODBCbindedStmt *source;

static LONG trepresno, teffectid, tnfactor;
static BOOL first_time=TRUE;

   if (fSearchOption==FIND_FIRST){
      tnfactor = nfactor;
      if (first_time)
      {
         BIND(1,trepresno);
         BIND(2, teffectid);  
         BINDPARAM(1, tnfactor);  
         first_time=FALSE;
      }

      local.Execute();
      central.Execute();
   }

   source = (fLocal)?&local:&central;

   if (source->Fetch()) {
	   *represno = trepresno;
       *effectid = teffectid;
   }
   else if (source->NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;

   POP();

#undef SQL_getEffectCount
}

/////////////////////////////////////////////////////////////////////////////////
// Input :  studyid
// Output:  effect record
/////////////////////////////////////////////////////////////////////////////////
LONG getEffect(BOOL fLocal,LONG studyid,LONG* effectid, LONG* repres, BOOL fSearchOption)
{
#define SQL_getEffect "\
     SELECT DISTINCT  EFFECT.EFFECTID, EFFECT.REPRESNO \
     FROM EFFECT, FACTOR  \
     WHERE (((FACTOR.STUDYID)=?) AND ((EFFECT.FACTORID)=FACTOR.LABELID)) \
     ORDER BY  EFFECT.EFFECTID,EFFECT.REPRESNO "

   PUSH(getEffectRecord);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_getEffect);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_getEffect);
       CODBCbindedStmt *source;

static LONG _effectid,_repres;
static LONG tstudyid;
static BOOL first_time=TRUE;

   if (fSearchOption==FIND_FIRST){

      if (first_time)
      {
         BIND(1, _effectid);  
         BIND(2,_repres);
		 BINDPARAM(1, tstudyid);
         first_time=FALSE;
      }
      tstudyid = studyid;
      local.Execute();
      central.Execute();
   }

   source = (fLocal)?&local:&central;

   if (source->Fetch()) {
       *effectid = _effectid;
	   *repres = _repres;
   }
   else if (source->NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;

   POP();

#undef SQL_getEffect
}

//////////////////////////////////////////////////////////////////////////////
// Input:  Study
// Output:  Represno,Effect,Factorid
//////////////////////////////////////////////////////////////////////////////
LONG getEffectRecord(BOOL fLocal,LONG studyid,DMS_EFFECT &data, BOOL fSearchOption)
{
#define SQL_getEffectRecord "\
     SELECT EFFECT.REPRESNO, EFFECT.EFFECTID, EFFECT.FACTORID \
     FROM EFFECT, FACTOR  \
     WHERE (((FACTOR.STUDYID)=?) AND ((EFFECT.FACTORID)=FACTOR.LABELID)) \
     ORDER BY EFFECT.REPRESNO, EFFECT.EFFECTID "

   PUSH(getEffectRecord);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_getEffectRecord);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_getEffectRecord);
       CODBCbindedStmt *source;

static DMS_EFFECT _data;
static LONG tstudyid;
static BOOL first_time=TRUE;

   if (fSearchOption==FIND_FIRST){

      if (first_time)
      {
         BIND(1, _data.REPRESNO);  
         BIND(2,_data.EFFECTID);
         BIND(3, _data.FACTORID);  
		 BINDPARAM(1, tstudyid);
         first_time=FALSE;
      }
      _data= data;
      tstudyid = studyid;
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

#undef SQL_getEffectRecord
}


/////////////////////////////////////////////////////////////////////////////////
// Input :  represno
// Output:  veffect record
/////////////////////////////////////////////////////////////////////////////////
LONG getVEffectRecord(BOOL fLocal, DMS_VEFFECT* data, BOOL fSearchOption)
{
#define SQL_getVEffectRecord "\
     SELECT  VEFFECT.REPRESNO, VEFFECT.VARIATID \
     FROM VEFFECT \
     WHERE VEFFECT.REPRESNO =? "

   PUSH(getVEffectRecord);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_getVEffectRecord);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_getVEffectRecord);
       CODBCbindedStmt *source;

static DMS_VEFFECT _data;
static LONG trepresno;
static BOOL first_time=TRUE;

   if (fSearchOption==FIND_FIRST){

      if (first_time)
      {
         BIND(1, _data.REPRESNO);  
         BIND(2,_data.VARIATEID);
		 BINDPARAM(1, trepresno);
         first_time=FALSE;
      }
      trepresno = data->REPRESNO;
      local.Execute();
      central.Execute();
   }

   source = (fLocal)?&local:&central;

   if (source->Fetch()) {
	   *data = _data;
   }
   else if (source->NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;

   POP();

#undef SQL_getVEffectRecord
}



//////////////////////////////////////////////////////////////////////////////
// Input:  Study
// Output:  Represno,Effect,Variatid
//////////////////////////////////////////////////////////////////////////////
LONG getEffectRecordV(BOOL fLocal,LONG studyid,DMS_EFFECT &data, BOOL fSearchOption)
{
#define SQL_getEVariateRecordC "\
       SELECT DISTINCT EFFECT.EFFECTID, VEFFECT.REPRESNO, VEFFECT.VARIATID \
        FROM VEFFECT, EFFECT, VARIATE \
         WHERE (((VEFFECT.REPRESNO)=EFFECT.REPRESNO) AND ((VEFFECT.VARIATID)=VARIATE.VARIATID) \
		 AND ((VARIATE.STUDYID)=?))  ORDER BY VEFFECT.VARIATID "
#define SQL_getEVariateRecordL "\
       SELECT DISTINCT EFFECT.EFFECTID, VEFFECT.REPRESNO, VEFFECT.VARIATID \
        FROM VEFFECT, EFFECT, VARIATE \
         WHERE (((VEFFECT.REPRESNO)=EFFECT.REPRESNO) AND ((VEFFECT.VARIATID)=VARIATE.VARIATID) \
		 AND ((VARIATE.STUDYID)=?)) ORDER BY VEFFECT.VARIATID DESC"

   PUSH(getEffectRecord1);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_getEVariateRecordC);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_getEVariateRecordL);
       CODBCbindedStmt *source;

static DMS_EFFECT _data;
static LONG tstudyid;
static BOOL first_time=TRUE;

   if (fSearchOption==FIND_FIRST){

      if (first_time)
      {
         BIND(1,_data.EFFECTID);
         BIND(2, _data.REPRESNO);  
         BIND(3, _data.FACTORID);  
		 BINDPARAM(1, tstudyid);
         first_time=FALSE;
      }
      _data= data;
      tstudyid = studyid;
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

#undef SQL_getEVariateRecordC
#undef SQL_getEVariateRecordL
}


//////////////////////////////////////////////////////////////////////////////
// Input:  Study
// Output:  Represno,Effect,Factorid
//////////////////////////////////////////////////////////////////////////////
LONG getEffectRecordV1(BOOL fLocal,LONG studyid,DMS_EFFECT &data, BOOL fSearchOption)
{
#define SQL_getEVariateRecord1 "\
     SELECT DISTINCT EFFECT.EFFECTID, EFFECT.REPRESNO,  DATA_C.VARIATID \
     FROM EFFECT, OINDEX, DATA_C, VARIATE \
     WHERE (((EFFECT.REPRESNO)=OINDEX.REPRESNO) AND ((OINDEX.OUNITID)=DATA_C.OUNITID) AND ((DATA_C.VARIATID)=VARIATE.VARIATID) AND ((VARIATE.STUDYID)=? )) \
     ORDER BY EFFECT.EFFECTID, EFFECT.REPRESNO  "

   PUSH(getEffectRecord1);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_getEVariateRecord1);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_getEVariateRecord1);
       CODBCbindedStmt *source;

static DMS_EFFECT _data;
static LONG tstudyid;
static BOOL first_time=TRUE;

   if (fSearchOption==FIND_FIRST){

      if (first_time)
      {
         BIND(1,_data.EFFECTID);
         BIND(2, _data.REPRESNO);  
         BIND(3, _data.FACTORID);  
		 BINDPARAM(1, tstudyid);
         first_time=FALSE;
      }
      _data= data;
      tstudyid = studyid;
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

#undef SQL_getEVariateRecord1
}


//////////////////////////////////////////////////////////////////////////////
// Input:  Study
// Output:  Represno,Effect,Factorid
//////////////////////////////////////////////////////////////////////////////
LONG getEffectRecordV2(BOOL fLocal,LONG studyid,DMS_EFFECT &data, BOOL fSearchOption)
{
#define SQL_getEVariateRecord2 "\
     SELECT DISTINCT EFFECT.EFFECTID, EFFECT.REPRESNO,  DATA_N.VARIATID \
     FROM EFFECT, OINDEX, DATA_N, VARIATE \
     WHERE (((EFFECT.REPRESNO)=OINDEX.REPRESNO) AND ((OINDEX.OUNITID)=DATA_N..OUNITID) AND ((DATA_N.VARIATID)=VARIATE.VARIATID) AND ((VARIATE.STUDYID)=? )) \
     ORDER BY  EFFECT.EFFECTID, EFFECT.REPRESNO "

   PUSH(getEffectRecord2);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_getEVariateRecord2);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_getEVariateRecord2);
       CODBCbindedStmt *source;

static DMS_EFFECT _data;
static LONG tstudyid;
static BOOL first_time=TRUE;

   if (fSearchOption==FIND_FIRST){

      if (first_time)
      {
         BIND(1,_data.EFFECTID);
         BIND(2, _data.REPRESNO);  
         BIND(3, _data.FACTORID);  
		 BINDPARAM(1, tstudyid);
         first_time=FALSE;
      }
      _data= data;
      tstudyid = studyid;
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

#undef SQL_getEVariateRecord2
}


//////////////////////////////////////////////////////////////////////////////
// Input:  Represno
// Output:  Represno,Effect,Factorid
//////////////////////////////////////////////////////////////////////////////
LONG getVariateCOfRepres(BOOL fLocal, DMS_EFFECT &data, BOOL fSearchOption)
{
#define SQL_getVariateCRepres "\
        SELECT DISTINCT EFFECT.EFFECTID, EFFECT.REPRESNO, DATA_C.VARIATID \
        FROM EFFECT, OINDEX, VARIATE, DATA_C \
        WHERE EFFECT.REPRESNO=? AND EFFECT.REPRESNO=OINDEX.REPRESNO AND \
		OINDEX.OUNITID=DATA_C.OUNITID AND DATA_C.VARIATID=VARIATE.VARIATID  \
        ORDER BY EFFECT.EFFECTID, EFFECT.REPRESNO "

   PUSH(getVariateCOfRepres);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_getVariateCRepres);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_getVariateCRepres);
       CODBCbindedStmt *source;

static DMS_EFFECT _data;
static LONG represno;
static BOOL first_time=TRUE;

   if (fSearchOption==FIND_FIRST){

      if (first_time)
      {
         BIND(1,_data.EFFECTID);
         BIND(2, _data.REPRESNO);  
         BIND(3, _data.FACTORID);  
		 BINDPARAM(1, represno);
         first_time=FALSE;
      }
      _data= data;
      represno = data.REPRESNO;
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

#undef SQL_getVariateCRepres
}


//////////////////////////////////////////////////////////////////////////////
// Input:  Represno
// Output:  Represno,Effect,Factorid
//////////////////////////////////////////////////////////////////////////////
LONG getVariateNOfRepres(BOOL fLocal, DMS_EFFECT &data, BOOL fSearchOption)
{
#define SQL_getVariateNRepres "\
        SELECT DISTINCT EFFECT.EFFECTID, EFFECT.REPRESNO, DATA_N.VARIATID \
        FROM EFFECT, OINDEX, VARIATE, DATA_N \
        WHERE EFFECT.REPRESNO=? AND EFFECT.REPRESNO=OINDEX.REPRESNO AND \
		OINDEX.OUNITID=DATA_N.OUNITID AND DATA_N.VARIATID=VARIATE.VARIATID  \
        ORDER BY EFFECT.EFFECTID, EFFECT.REPRESNO "

   PUSH(getVariateNOfRepres);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_getVariateNRepres);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_getVariateNRepres);
       CODBCbindedStmt *source;

static DMS_EFFECT _data;
static LONG represno;
static BOOL first_time=TRUE;

   if (fSearchOption==FIND_FIRST){

      if (first_time)
      {
         BIND(1,_data.EFFECTID);
         BIND(2, _data.REPRESNO);  
         BIND(3, _data.FACTORID);  
		 BINDPARAM(1, represno);
         first_time=FALSE;
      }
      _data= data;
      represno = data.REPRESNO;
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

#undef SQL_getVariateNRepres
}


//////////////////////////////////////////////////////////////////////////////
//Date Created: Dec 20, 2004
//Input: representation number
//Output: number of factor of the representation
//////////////////////////////////////////////////////////////////////////////
LONG countFactorOfRepres(BOOL fLocal, LONG represno, LONG *cntFactor)
{
#define SQL_countFactorOfRepres "\
        SELECT Count(EFFECT.FACTORID) AS CntFac \
        FROM EFFECT \
        where EFFECT.REPRESNO=?" 

   PUSH(countFactorOfRepres);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_countFactorOfRepres);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_countFactorOfRepres);
       CODBCbindedStmt *source;

static LONG _represno,_cntFactor;
static BOOL first_time=TRUE;

  if (first_time)
  {
      BIND(1,_cntFactor);
      BINDPARAM(1, _represno);
      first_time=FALSE;
   }
   _represno = represno;
   
   source = (fLocal)?&local:&central;
   source->Execute();
   if (source->Fetch())
      *cntFactor = _cntFactor;
   else if (source->NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;

   POP();

#undef SQL_countFactorOfRepres
}



//////////////////////////////////////////////////////////////////////////////
//Date Created: Dec 20, 2004
//Input: representation number
//Output: number of variate of the representation
//////////////////////////////////////////////////////////////////////////////
LONG countVariateOfRepres(BOOL fLocal, LONG represno, LONG *cntVar)
{
#define SQL_countVariateOfRepres "\
        SELECT Count(VEFFECT.VARIATID) AS CntVar \
        FROM VEFFECT \
        where VEFFECT.REPRESNO=?" 

   PUSH(countVariateOfRepres);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_countVariateOfRepres);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_countVariateOfRepres);
       CODBCbindedStmt *source;

static LONG _represno,_cntVar;
static BOOL first_time=TRUE;

  if (first_time)
  {
      BIND(1,_cntVar);
      BINDPARAM(1, _represno);
      first_time=FALSE;
   }
   _represno = represno;

   source = (fLocal)?&local:&central;
   source->Execute();
   if (source->Fetch())
      *cntVar = _cntVar;
   else if (source->NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;

   POP();

#undef SQL_countVariateOfRepres
}


//////////////////////////////////////////////////////////////////////////////
//Date Created: Dec 20, 2004
//Input: representation number
//Output: number of records in the OINDEX table of the specified representation number
//////////////////////////////////////////////////////////////////////////////
LONG countOunitOfRepres(BOOL fLocal, LONG represno, LONG *cntVar)
{
#define SQL_countVariateOfRepres "\
        SELECT Count(VEFFECT.VARIATID) AS CntVar \
        FROM VEFFECT \
        where VEFFECT.REPRESNO=?" 

   PUSH(countVariateOfRepres);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_countVariateOfRepres);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_countVariateOfRepres);
       CODBCbindedStmt *source;

static LONG _represno,_cntVar;
static BOOL first_time=TRUE;

  if (first_time)
  {
      BIND(1,_cntVar);
      BINDPARAM(1, _represno);
      first_time=FALSE;
   }
   _represno = represno;

   source = (fLocal)?&local:&central;
   source->Execute();
   if (source->Fetch())
      *cntVar = _cntVar;
   else if (source->NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;

   POP();

#undef SQL_countVariateOfRepres
}


//////////////////////////////////////////////////////////////////////////////
//Input: array of factors
//Output: represno, effectid
//////////////////////////////////////////////////////////////////////////////
LONG existEffect(LONG *lstfactor, LONG nfactor, LONG *represno, LONG *effectid)
{
	DMS_EFFECT teffect; 
	long tag[DMS_MAXNOFACTOR],i,texist;
	LONG retValue,trepres,teffectid;
    PUSH(existEffect);
    ret = DMS_NO_DATA;
	teffect.FACTORID = lstfactor[0];
    teffect.EFFECTID = 0;
	teffect.REPRESNO = *represno;
	trepres = 0;
	teffectid = 0;
	//retValue = DMS_getEffectFactor(&teffect,FIND_FIRST);
	retValue = DMS_getEffectCount(nfactor,&trepres,&teffectid,FIND_FIRST);
	while (retValue == DMS_SUCCESS) {
		teffect.EFFECTID = teffectid;
		teffect.REPRESNO = trepres;
		teffect.FACTORID = 0;
	  	 for (i=0;i<nfactor;i++) {
			   tag[i]= 0;
		 }
		 retValue  = DMS_getFactorEffect(&teffect,FIND_FIRST);
		 while (retValue ==DMS_SUCCESS) {
			for (i=0;i<nfactor;i++) {
				 if (lstfactor[i] == teffect.FACTORID) {
					tag[i] = 1;
					break;
				 }
			}
            
		    retValue  = DMS_getFactorEffect(&teffect,FIND_NEXT);
         }
		 texist = 1;

	  	 for (i=0;i<nfactor;i++) {
			   texist = texist && tag[i];
		 }

		 if (texist) {
			 ret = DMS_SUCCESS;
			 *represno = teffect.REPRESNO;
             *effectid = teffect.EFFECTID;
		 }
	     trepres = 0;
	     teffectid = 0;
  	     retValue = DMS_getEffectCount(nfactor,&trepres,&teffectid,FIND_NEXT);
//		 retValue  = DMS_getEffectFactor(&teffect,FIND_NEXT);
	}
	if (retValue == DMS_ERROR) ret = DMS_ERROR;
	POP();
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG addEffectOfStudy(long *effectid, long studyid)
{
#define SQL_ADDST_EFFECT  "INSERT into STEFFECT \
             (EFFECTID, STUDYID) \
              values (?, ?) "

   PUSH(addStEffect);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_ADDST_EFFECT);


static BOOL first_time=TRUE;
static LONG _effectid, _studyid;
   
   _effectid = getNextEffectID(studyid);
   _studyid = studyid;
   if (first_time)
   {
      local.BindParam(1,_effectid);
      local.BindParam(2,_studyid);
      first_time=FALSE;
   }

   if ((ret=local.Execute())==1) {
 	 *effectid = _effectid;
     ret=GMS_SUCCESS;
   }
   //else
   //   ret=GMS_ERROR;

    else  
	   if (ret!=GMS_UNIQUE_CONSTRAINT_ERROR) //GMS_UNIQUE_CONSTRAINT_ERROR
		   ret=GMS_ERROR;  
       // Handler for unique contraint error with the assumption that only EFFECTID has unique contraint 
	   else {
		   int i=0;
		   while ((ret==GMS_UNIQUE_CONSTRAINT_ERROR) && (i<100)) {
			    _effectid = getNextEffectID(studyid);
				ret=local.Execute();
				++i;
		   }
		   if ((ret) &&(ret!=GMS_UNIQUE_CONSTRAINT_ERROR)) {
			   *effectid = _effectid;
			   ret = DMS_SUCCESS;
		   }
		   else  {
			   *effectid= 0;
			   ret = DMS_ERROR;
		   }
	   }


   POP();

#undef SQL_ADDST_EFFECT
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG addReprestn(long *represno, long effectid)
{
#define SQL_ADDREPRESTN  "INSERT into REPRESTN \
             (REPRESNO, EFFECTID) \
              values (?, ?) "

   PUSH(addReprestn);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_ADDREPRESTN);


static BOOL first_time=TRUE;
static LONG _effectid, _represno;
   
   _represno = getNextRepresNo();
   _effectid = effectid;
   if (first_time)
   {
      local.BindParam(1,_represno);
      local.BindParam(2,_effectid);
      first_time=FALSE;
   }

   if ((ret=local.Execute())==1) {
 	 *represno = _represno;
     ret=GMS_SUCCESS;
   }
   //else
   //   ret=GMS_ERROR;

    else  
	   if (ret!=GMS_UNIQUE_CONSTRAINT_ERROR) //GMS_UNIQUE_CONSTRAINT_ERROR
		   ret=GMS_ERROR;  
       // Handler for unique contraint error with Represno having the unique constraint
	   else {
		   int i=0;
		   while ((ret==GMS_UNIQUE_CONSTRAINT_ERROR) && (i<100)) {
                _represno = getNextRepresNo();
				ret=local.Execute();
				++i;
		   }
		   if ((ret) &&(ret!=GMS_UNIQUE_CONSTRAINT_ERROR)) {
			   *represno = _represno;
			   ret = DMS_SUCCESS;
		   }
		   else  {
			   *represno= 0;
			   ret = DMS_ERROR;
		   }
	   }


   POP();

#undef SQL_ADDST_EFFECT
}

//////////////////////////////////////////////////////////////////////////////
//   
//////////////////////////////////////////////////////////////////////////////
LONG addEffect(LONG *lstfactor, LONG nfactor, LONG *represno, LONG *effectid)
{
    DMS_EFFECT teffect;
	LONG trepresno=0, teffectid=0,i=0,tstudyid=0;
	PUSH(addEffect);
    ret = existEffect(lstfactor,nfactor,&trepresno,&teffectid);
	if (ret == DMS_SUCCESS) {
		*represno = trepresno;
		*effectid = teffectid;
		ret = DMS_EXIST;
	}
	else {
        long rt;
		if (*effectid==0) {
		   rt = DMS_getStudyFactor(lstfactor[0],&tstudyid);
		   rt = addEffectOfStudy(effectid,tstudyid);
		   DMS_commitData;
		 //  *effectid = getNextEffectID(tstudyid);
		}
   	   //*represno = getNextRepresNo();
       rt = addReprestn(represno,*effectid);
	   if ((rt==DMS_SUCCESS) && (*effectid!=0)) {
	     for (i=0;i<nfactor;i++) {
		  teffect.EFFECTID = *effectid;
		  teffect.REPRESNO = *represno;
		  teffect.FACTORID = lstfactor[i];
  	      ret = addRepresDefn(teffect);
   	      DMS_commitData;
		 }
	    }
	   else
		   ret = DMS_ERROR;
	
    }
   POP();
}

//////////////////////////////////////////////////////////////////////////////
//Date Created: May 28, 2008
//Updates a list name record with status and hierarchy for the given listid
//////////////////////////////////////////////////////////////////////////////
LONG setRepresName(LONG represno, char *represname)
{
#define SQL_SETREPRESNAME "\
   update REPRESTN\
   set REPRESNAME=? \
   where REPRESNO=?"

   PUSH(setRepresName);


static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_SETREPRESNAME);

static long _represno;
static char _represname[150];
static BOOL first_time=TRUE;


   if (first_time)
   {
      local.BindParam(1,_represname,sizeof(_represname));
      local.BindParam(2,_represno);
      first_time=FALSE;
   }

   _represno = represno;
   strcpy(_represname,represname);

   if (local.Execute())
      ret=DMS_SUCCESS;
   else if (local.NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;
   POP();

#undef  SQL_SETREPRESNAME
}

//////////////////////////////////////////////////////////////////////////////
//Date Created: June 13, 2008
//Gets the representation name
//////////////////////////////////////////////////////////////////////////////
LONG getRepresName(LONG represno, char *represname)
{
#define SQL_GETREPRESNAME " \
   SELECT REPRESNAME FROM REPRESTN \
   where REPRESNO=?"

   PUSH(setRepresName);


static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_GETREPRESNAME);
static CODBCbindedStmt &central  = _centralDMS->BindedStmt(SQL_GETREPRESNAME);

static long _represno;
static char _represname[150];
static BOOL first_time=TRUE;

 
   if (first_time)
   {
      BINDS(1,_represname,sizeof(_represname));
      BINDPARAM(1,_represno);
      first_time=FALSE;
   }
   
   _represno = represno;
   //strcpy(_represname,represname);
   local.Execute();
   central.Execute();
   if (local.Fetch()){
      ret=DMS_SUCCESS;
      strcpy(represname,_represname);
   }
   else if (local.NoData())
	   if ( central.Fetch()) {
          strcpy(represname,_represname);
		  ret = DMS_SUCCESS;
	   }
	   else
          ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;
   POP();

#undef  SQL_SETREPRESNAME
}



//////////////////////////////////////////////////////////////////////////////
//Date Created: May 28, 2008
//Updates a list name record with status and hierarchy for the given listid
//////////////////////////////////////////////////////////////////////////////
LONG setEffectName(LONG id, char *name)
{
#define SQL_SETEffectName "\
   update STEFFECT\
   set EFFECTNAME=? \
   where EFFECTID=?"

   PUSH(setEffectName);


static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_SETEffectName );

static long _no;
static char _name[150];
static BOOL first_time=TRUE;


   if (first_time)
   {
      local.BindParam(1,_name,sizeof(_name));
      local.BindParam(2,_no);
      first_time=FALSE;
   }

   _no = id;
   strcpy(_name,name);

   if (local.Execute())
      ret=DMS_SUCCESS;
   else if (local.NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;
   POP();

#undef  SQL_SETEffectName
}

LONG getFactor(BOOL fLocal,DMS_FACTOR *data, int fOpt)
{
#define SQL_GETFACTOR1 \
  "SELECT LABELID, FACTORID, STUDYID, FNAME, TRAITID, SCALEID, TMETHID, LTYPE FROM FACTOR \
	WHERE ( 0=? or STUDYID = ?)  AND (0=? or LABELID=?) ORDER BY LABELID ASC" 
#define SQL_GETFACTOR2 \
  "SELECT LABELID, FACTORID, STUDYID, FNAME, TRAITID, SCALEID, TMETHID, LTYPE FROM FACTOR \
	WHERE ( 0=? or STUDYID = ?)  AND (0=? or LABELID=?) ORDER BY LABELID DESC" 


  LONG ret=DMS_SUCCESS;

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_GETFACTOR1);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_GETFACTOR2);
       CODBCbindedStmt *source;

static DMS_FACTOR _data;
static LONG studyid, factorid;
static BOOL first_time=TRUE;

//Parameters
   if (fOpt == FIND_FIRST) {
      if (first_time)
	  {
		BINDPARAM(1, studyid);
		BINDPARAM(2, studyid);
		BINDPARAM(3, factorid);
		BINDPARAM(4, factorid);
												
        BIND(1,_data.LABELID);
        BIND(2,_data.FACTORID);
        BIND(3,_data.STUDYID);
        BINDS(4,_data.FNAME,DMS_FACTOR_NAME);
	    BIND(5,_data.TRAITID);
		BIND(6,_data.SCALEID);
		BIND(7,_data.TMETHID);
        BINDS(8,_data.LTYPE,DMS_LEVEL_TYPE);

      first_time = FALSE;
	  }
     studyid = data->STUDYID;
	 factorid = data->LABELID;
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

#undef SQL_GETFACTOR1
#undef SQL_GETFACTOR2
}

////////////////////////////////////////////////////////////
//**** Gets the label names of a factor factor **********/
//Input: data.FACTORID
//Output: entire factor record
//Called by DMS_getFactorLabel
////////////////////////////////////////////////////////////
LONG getFactorLabel(BOOL fLocal,DMS_FACTOR *data, int fOpt)
{
#define SQL_GETFACTORLABEL1 \
  "SELECT LABELID, FACTORID, STUDYID, FNAME, TRAITID, SCALEID, TMETHID, LTYPE FROM FACTOR \
	WHERE (0=? or FACTORID=?) ORDER BY LABELID ASC" 

#define SQL_GETFACTORLABEL2 \
  "SELECT LABELID, FACTORID, STUDYID, FNAME, TRAITID, SCALEID, TMETHID, LTYPE FROM FACTOR \
	WHERE (0=? or FACTORID=?) ORDER BY LABELID DESC" 



  LONG ret=DMS_SUCCESS;

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_GETFACTORLABEL1);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_GETFACTORLABEL2);
       CODBCbindedStmt *source;

static DMS_FACTOR _data;
static LONG factorid;
static BOOL first_time=TRUE;

//Parameters
   if (fOpt == FIND_FIRST) {
      if (first_time)
	  {
		BINDPARAM(1, factorid);
		BINDPARAM(2, factorid);
												
        BIND(1,_data.LABELID);
        BIND(2,_data.FACTORID);
        BIND(3,_data.STUDYID);
        BINDS(4,_data.FNAME,DMS_FACTOR_NAME);
	    BIND(5,_data.TRAITID);
		BIND(6,_data.SCALEID);
		BIND(7,_data.TMETHID);
        BINDS(8,_data.LTYPE,DMS_LEVEL_TYPE);

      first_time = FALSE;
	  }
	 factorid = data->FACTORID;
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

#undef SQL_GETFACTORLABEL1
#undef SQL_GETFACTORLABEL2
}


/***  if a set of factors can be defined by more than one effect
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG addEffect(LONG *lstfactor, LONG nfactor, LONG *represno, LONG *effectid)
{
    DMS_EFFECT teffect;
	PUSH(addEffect);
    if (represno==0) {
       represno = getNextRepresno();
	   effectid = getNextEffectid();
	   addEffectRecord(teffect);
	}
	else {
       ret = existEffect(lstfactor,nfactor,represno,effectid);
	   if (ret == DMS_NO_DATA) {
	       affectid = getNextEffectid();
	       addEffectRecord(teffect);
	   }
	}

   POP();
}

****/


//////////////////////////////////////////////////////////////////////////////
// Input:  Factorid 
// Output: Studyid
//////////////////////////////////////////////////////////////////////////////
LONG getStudyFactor(BOOL fLocal,LONG factorid, LONG *studyid)
{
#define SQL_getStudyFactor "\
   SELECT STUDYID FROM FACTOR \
   WHERE FACTORID = ? "

   PUSH(getStudyFactor);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_getStudyFactor);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_getStudyFactor);
       CODBCbindedStmt *source;

static LONG tfactorid,tstudyid;
static BOOL first_time=TRUE;


      if (first_time)
      {
         BIND(1,tstudyid);
         BINDPARAM(1,tfactorid);  
         first_time=FALSE;
      }
       tfactorid = factorid; 
      local.Execute();
      central.Execute();
   

   source = (fLocal)?&local:&central;

   if (source->Fetch())
      *studyid = tstudyid;
   else if (source->NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;

   POP();

#undef SQL_getStudyFactor
}




//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG addFactor(DMS_FACTOR &data)
{
#define SQL_ADDFACTOR "\
   insert into FACTOR (LABELID, FACTORID,FNAME,STUDYID,TRAITID,SCALEID,TMETHID,LTYPE)\
   values(?,?,?,?,?,?,?,?)"

   PUSH(addFactor);

static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_ADDFACTOR);

static DMS_FACTOR _data;
static BOOL first_time=TRUE;

   _data = data;
   ret = findFactorEq(data.STUDYID<0,data.FNAME,_data,FIND_FIRST);
   if (ret==DMS_SUCCESS) {
	   data = _data;                        //return the entire record of array
	   POP2(DMS_EXIST);
   }
   data.LABELID = getNextLabelID();
   if (data.FACTORID ==0)
	   data.FACTORID = data.LABELID;
   if (first_time)
   {
      local.BindParam(1,_data.LABELID);
      local.BindParam(2,_data.FACTORID);
//20030227:delete      local.BindParam(3,_data.FNAME,sizeof(_data.FNAME));
      local.BindParam(3,_data.FNAME,DMS_FACTOR_NAME - 2);
      local.BindParam(4,_data.STUDYID);
      local.BindParam(5,_data.TRAITID);
      local.BindParam(6,_data.SCALEID);
      local.BindParam(7,_data.TMETHID);
//20030227:delete            local.BindParam(8,_data.LTYPE,sizeof(_data.LTYPE));
      local.BindParam(8,_data.LTYPE,DMS_LEVEL_TYPE-3); 
      first_time=FALSE;
   }

   _data = data;

   if ((ret=local.Execute())==1)
      ret=GMS_SUCCESS;
   //else
   //   ret=GMS_ERROR;

    else  
	   if (ret!=GMS_UNIQUE_CONSTRAINT_ERROR) //GMS_UNIQUE_CONSTRAINT_ERROR
		   ret=GMS_ERROR;  
       // Handler for unique contraint error with the assumption that only LABELID has unique contraint 
	   else {
		   int i=0;
		   while ((ret==GMS_UNIQUE_CONSTRAINT_ERROR) && (i<100)) {
			    _data.LABELID = getNextLabelID();
				ret=local.Execute();
				++i;
		   }
		   if ((ret) &&(ret!=GMS_UNIQUE_CONSTRAINT_ERROR)) {
               data = _data;
			   ret = DMS_SUCCESS;
		   }
		   else  {
			   data.LABELID = 0;
			   ret = DMS_ERROR;
		   }
	   }


   POP();

#undef SQL_ADDFACTOR
}



//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG findFactor(BOOL fLocal, CHAR *szName, DMS_FACTOR &data, LONG fSearchOption)
{
#define SQL_FINDFACTOR  \
"SELECT LABELID, FACTORID, STUDYID, FNAME, TRAITID, SCALEID, TMETHID, LTYPE FROM FACTOR "

#define FNAME_EQ_SQL  " WHERE  FNAME = ? " 
#define FNAME_LIKE_SQL " WHERE  FNAME LIKE ? "
#define STUDYID_SQL " AND STUDYID = ? "

#define SQL_FINDFACTOR_ " order by FNAME asc "

   PUSH(findFactor);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_FINDFACTOR);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_FINDFACTOR);
       CODBCbindedStmt *source;
static CHAR fname[MAX_STR];
static DMS_FACTOR _data;
static BOOL fname_wild;

static BOOL first_time=TRUE;

   if (fSearchOption==FIND_FIRST){
      string stmt_sql;
	  ZeroFill(fname,sizeof(fname));
      strcpy(fname,szName);
	  fname_wild =strchr(fname,'_') || strchr(fname,'%');
	  stmt_sql=SQL_FINDFACTOR;
	  stmt_sql+=(fname_wild?FNAME_LIKE_SQL:FNAME_EQ_SQL);
	  if (data.STUDYID != 0) 
		  stmt_sql+=STUDYID_SQL;
	  stmt_sql+=SQL_FINDFACTOR_;
	  central.SetSQLstr(stmt_sql.c_str());
	  local.SetSQLstr(stmt_sql.c_str());

 //     if (first_time)
 //     {
         BIND(1,_data.LABELID);
         BIND(2,_data.FACTORID);
         BIND(3,_data.STUDYID);
         BINDS(4,_data.FNAME,DMS_FACTOR_NAME);
		 BIND(5,_data.TRAITID);
		 BIND(6,_data.SCALEID);
		 BIND(7,_data.TMETHID);
         BINDS(8,_data.LTYPE,DMS_LEVEL_TYPE);
         BINDPARAMS(1, fname, DMS_FACTOR_NAME-2);         
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
#undef SQL_FINDFACTOR 
#undef FNAME_EQ_SQL 
#undef FNAME_LIKE_SQL 
#undef STUDYID_SQL 
#undef SQL_FINDFACTOR_
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG findFactorEq(BOOL fLocal, CHAR *szName, DMS_FACTOR &data, LONG fSearchOption)
{
#define SQL_FINDFACTOR_EQ  \
    "SELECT LABELID, FACTORID, STUDYID, FNAME, TRAITID, SCALEID, TMETHID, LTYPE \
	 FROM FACTOR  WHERE  FNAME = ? \
	 AND (0=? OR STUDYID=?) order by FNAME asc "

   PUSH(findFactorEq);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_FINDFACTOR_EQ);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_FINDFACTOR_EQ);
       CODBCbindedStmt *source;
static CHAR fname[MAX_STR];
static DMS_FACTOR _data;
static BOOL fname_wild;
static LONG styid;

static BOOL first_time=TRUE;

   if (fSearchOption==FIND_FIRST){
      string stmt_sql;
	  ZeroFill(fname,sizeof(fname));
      strcpy(fname,szName);
      styid = data.STUDYID;
      if (first_time)
      {
         BIND(1,_data.LABELID);
         BIND(2,_data.FACTORID);
         BIND(3,_data.STUDYID);
         BINDS(4,_data.FNAME,DMS_FACTOR_NAME);
		 BIND(5,_data.TRAITID);
		 BIND(6,_data.SCALEID);
		 BIND(7,_data.TMETHID);
         BINDS(8,_data.LTYPE,DMS_LEVEL_TYPE);
         BINDPARAMS(1, fname, DMS_FACTOR_NAME-2);         
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
#undef SQL_FINDFACTOR_EQ 
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG findFactorLike(BOOL fLocal, CHAR *szName, DMS_FACTOR &data, LONG fSearchOption)
{
#define SQL_FINDFACTOR_LIKE  \
"SELECT LABELID, FACTORID, STUDYID, FNAME, TRAITID, SCALEID, TMETHID, LTYPE FROM FACTOR   \
   WHERE FNAME LIKE ? AND (0=? OR STUDYID=?) order by FNAME asc "

   PUSH(findFactorLike);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_FINDFACTOR_LIKE);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_FINDFACTOR_LIKE);
       CODBCbindedStmt *source;
static CHAR fname[MAX_STR];
static DMS_FACTOR _data;
static BOOL fname_wild;
static LONG styid;

static BOOL first_time=TRUE;

   if (fSearchOption==FIND_FIRST){
      string stmt_sql;
	  ZeroFill(fname,sizeof(fname));
      strcpy(fname,szName);
      styid = data.STUDYID;

      if (first_time)
      {
         BIND(1,_data.LABELID);
         BIND(2,_data.FACTORID);
         BIND(3,_data.STUDYID);
         BINDS(4,_data.FNAME,DMS_FACTOR_NAME);
		 BIND(5,_data.TRAITID);
		 BIND(6,_data.SCALEID);
		 BIND(7,_data.TMETHID);
         BINDS(8,_data.LTYPE,DMS_LEVEL_TYPE);
         BINDPARAMS(1, fname, DMS_FACTOR_NAME-2);         
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
#undef SQL_FINDFACTOR_LIKE 
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG findFactorDistinct(BOOL fLocal, CHAR *szName, DMS_FACTOR &data, LONG fSearchOption)
{
#define SQL_FINDFACTORDISTINCT  \
"SELECT DISTINCT  FNAME, TRAITID, SCALEID, TMETHID FROM FACTOR  \
	WHERE  FNAME LIKE ?  \
    order by FNAME asc "

   PUSH(findFactorDistinct);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_FINDFACTORDISTINCT);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_FINDFACTORDISTINCT);
       CODBCbindedStmt *source;
static CHAR fname[MAX_STR];
static DMS_FACTOR _data;
static BOOL fname_wild;

static BOOL first_time=TRUE;

   if (fSearchOption==FIND_FIRST){
	  ZeroFill(fname,sizeof(fname));
      strcpy(fname,szName);

      if (first_time)
      {
         BINDS(1,_data.FNAME,DMS_FACTOR_NAME);
		 BIND(2,_data.TRAITID);
		 BIND(3,_data.SCALEID);
		 BIND(4,_data.TMETHID);
         BINDPARAMS(1, fname, DMS_FACTOR_NAME-2);         
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
#undef SQL_FINDFACTORDISTINCT 
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_findFactorDistinct(CHAR *szSearchName, DMS_FACTOR *data, LONG fSearchOption)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_findFactorDistinct);
   if (fSearchOption==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal){
      ret=findFactorDistinct(LOCAL,szSearchName,*data,fSearchOption);
      if (ret!=GMS_SUCCESS){
         fLocal = FALSE; 
         ret=findFactorDistinct(CENTRAL,szSearchName,*data,FIND_NEXT);
      }
   }
   else
      ret=findFactorDistinct(CENTRAL,szSearchName,*data,fSearchOption);

   POP();
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG setFactor(DMS_FACTOR &data)
{
#define SQL_SETFACTOR "\
   update FACTOR set FACTORID=?,FNAME=?, STUDYID=?, TRAITID=?, \
   SCALEID=,TMETHID=?,LTYPE=? where LABELID=? "

   PUSH(setFactor);

static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_SETFACTOR);

static DMS_FACTOR _data;
static BOOL first_time=TRUE;

   _data = data;
   if (first_time)
   {
      local.BindParam(1,_data.FACTORID);
      local.BindParam(2,_data.FNAME,DMS_FACTOR_NAME - 2);
      local.BindParam(3,_data.STUDYID);
      local.BindParam(4,_data.TRAITID);
      local.BindParam(5,_data.SCALEID);
      local.BindParam(6,_data.TMETHID);
      local.BindParam(7,_data.LTYPE,DMS_LEVEL_TYPE-3); 
      local.BindParam(8,_data.LABELID);
      first_time=FALSE;
   }

   _data = data;

   if (local.Execute())
      ret=GMS_SUCCESS;
   else
      ret=GMS_ERROR;


   POP();

#undef SQL_SETFACTOR
}



/*****************************************************************************/

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_findFactor_(CHAR *szSearchName, DMS_FACTOR *data, LONG fSearchOption)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_findFactor);
   if (fSearchOption==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal){
      ret=findFactor(LOCAL,szSearchName,*data,fSearchOption);
      if (ret!=GMS_SUCCESS){
         fLocal = FALSE; 
         ret=findFactor(CENTRAL,szSearchName,*data,FIND_FIRST);
      }
   }
   else
      ret=findFactor(CENTRAL,szSearchName,*data,fSearchOption);

   POP();
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_findFactor(CHAR *szSearchName, DMS_FACTOR *data, LONG fSearchOption)
{
   static BOOL fLocal=TRUE;
   static BOOL name_wild=FALSE;

   PUSH(DMS_findFactor);
   if (fSearchOption==FIND_FIRST){
      fLocal=TRUE;
	  name_wild =strchr(szSearchName,'_') || strchr(szSearchName,'%');
   }
   if (name_wild) {
     if (fLocal){
        ret=findFactorLike(LOCAL,szSearchName,*data,fSearchOption);
        if (ret!=GMS_SUCCESS){
           fLocal = FALSE; 
           ret=findFactorLike(CENTRAL,szSearchName,*data,FIND_NEXT);
		}
	 }
     else
        ret=findFactorLike(CENTRAL,szSearchName,*data,fSearchOption);
   }
   else {
     if (fLocal){
        ret=findFactorEq(LOCAL,szSearchName,*data,fSearchOption);
        if (ret!=GMS_SUCCESS){
           fLocal = FALSE; 
           ret=findFactorEq(CENTRAL,szSearchName,*data,FIND_NEXT);
		}
	 }
     else
        ret=findFactorEq(CENTRAL,szSearchName,*data,fSearchOption);
   }
   POP();

}



//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_addFactor(DMS_FACTOR* Fac)
{
   PUSH(DMS_addFactor);
   ret = addFactor(*Fac);
   DMS_commitData;
   POP();
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getFactor(DMS_FACTOR *data,  int fOpt)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_getFactor);

   if (fOpt==FIND_FIRST)
   {
      fLocal=TRUE;
   }
   if (fLocal){
      ret=getFactor(LOCAL,data,fOpt);
      if (ret!=DMS_SUCCESS){
         fLocal = FALSE; 
         ret=getFactor(CENTRAL,data, FIND_NEXT);
      }
   }
   else
      ret=getFactor(CENTRAL,data,fOpt);
   POP();

}


//////////////////////////////////////////////////////////////////////////////
//Gets the labels of a factor
//Input: data.FACTORID
//Output: the entire factor record
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getFactorLabel(DMS_FACTOR *data,  int fOpt)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_getFactorLabel);

   if (fOpt==FIND_FIRST)
   {
      fLocal=TRUE;
   }
   if (fLocal){
      ret=getFactorLabel(LOCAL,data,fOpt);
      if (ret!=DMS_SUCCESS){
         fLocal = FALSE; 
         ret=getFactorLabel(CENTRAL,data, FIND_NEXT);
      }
   }
   else
      ret=getFactorLabel(CENTRAL,data,fOpt);

   POP();

}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getStudyFactor(LONG factorid, LONG *studyid)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_getStudyFactor);
   ret=getStudyFactor(LOCAL,factorid, studyid);
   if (ret== DMS_NO_DATA){
         ret=getStudyFactor(CENTRAL,factorid, studyid);
   }
   POP();

}


//////////////////////////////////////////////////////////////////////////////
// Add a list of factors to the effect table.  A new represno will be assigned to those list of factors. 
// If effectid=0, a new effectid will be assigned  
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_addEffect(LONG *lstfactor, LONG nfactor, LONG *represno, LONG *effectid)
{
   PUSH(DMS_addEffect);
   ret = addEffect(lstfactor, nfactor, represno, effectid);
   POP();
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_addVEffect(DMS_VEFFECT *data)
{
   PUSH(DMS_addVEffect);
   ret = addVEffectRecord(*data);
   POP();
}


//////////////////////////////////////////////////////////////////////////////
//Gets the different effect & representation no of a given factorid
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getEffectFactor(DMS_EFFECT *data, BOOL fSearchOption)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_getEffectFactor);
   if (fSearchOption==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal){
      ret=getEffectFactor(LOCAL,*data,fSearchOption);
      if (ret!=DMS_SUCCESS){
         fLocal = FALSE; 
         ret=getEffectFactor(CENTRAL,*data,FIND_FIRST);
      }
   }
   else
      ret=getEffectFactor(CENTRAL,*data,fSearchOption);

   POP();

}

//////////////////////////////////////////////////////////////////////////////
//  Gets the observation units of an effect 
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getEffectOunit(LONG represno, LONG *ounitid, BOOL fSearchOption)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_getEffectOunit);
   if (fSearchOption==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal){
      ret=getEffectOunit(LOCAL,represno, ounitid,fSearchOption);
      if (ret!=DMS_SUCCESS){
         fLocal = FALSE; 
         ret=getEffectOunit(CENTRAL,represno, ounitid,FIND_FIRST);
      }
   }
   else
      ret=getEffectOunit(CENTRAL,represno, ounitid,fSearchOption);

   POP();

}


//////////////////////////////////////////////////////////////////////////////
// DMS_describeEffectOunit - describes an observation unit of a given represno of an effect 
//                     by combination of factor  levels
// Parameters:
//   Input    ounitid-  observation unit
//            represno - represno of an effect
//            nsrc - initially the maximum allowed no of factors/ the length of source; 
//   Output
//            source - an array or pointer of a data structure containing factorid and levelno
//            nsrc = the number of factors that describes the observation unit for the specified effect
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_describeEffectOunit(LONG ounitid, LONG represno, struct source *dsource, LONG *nsrc,LONG fopt )
{
#define NULLSTR ""
	long tret,varid,i=0;
	long tval=0;
	char *szData;

	szData = NULLSTR;

	if (ounitid < 0) {
	  tret = describeEffectOunit(TRUE,ounitid,represno,&varid,&tval,FIND_FIRST);      //local DMS
      while (tret==DMS_SUCCESS && i < *nsrc){
         dsource[i].facid = varid;
		 dsource[i].levelno = tval;
	     tret = describeEffectOunit(TRUE,ounitid,represno,&varid,&tval,FIND_NEXT);
         i++;
	  }
	  if (tret == DMS_ERROR) return DMS_ERROR;
	}
	else {
	  tret = describeEffectOunit(FALSE,ounitid,represno,&varid,&tval,FIND_FIRST);    //central DMS
      while (tret==DMS_SUCCESS && i < *nsrc){
         dsource[i].facid = varid;
		 dsource[i].levelno = tval;
	     tret = describeEffectOunit(FALSE,ounitid,represno,&varid,&tval,FIND_NEXT);
         i++;
	  }
	  if (tret == DMS_ERROR) return DMS_ERROR;
	}
	if (i>0) {
	   *nsrc = i;
       return DMS_SUCCESS;
	}
	  else return DMS_NO_DATA;
}


//////////////////////////////////////////////////////////////////////////////
// Input:  Effect and Represno
// Output: Factor
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getFactorEffect(DMS_EFFECT *data, BOOL fSearchOption)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_getEffectFactor);
   if (fSearchOption==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal){
      ret=getFactorEffect(LOCAL,*data,fSearchOption);
      if (ret!=DMS_SUCCESS){
         fLocal = FALSE; 
         ret=getFactorEffect(CENTRAL,*data,FIND_FIRST);
      }
   }
   else
      ret=getFactorEffect(CENTRAL,*data,fSearchOption);

   POP();

}

//////////////////////////////////////////////////////////////////////////////
// Gets the record of the given effectid
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getEffectRecord(LONG studyid, DMS_EFFECT *data, BOOL fSearchOption)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_getEffectRecord);
   if (fSearchOption==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal){
      ret=getEffectRecord(LOCAL,studyid,*data,fSearchOption);
      if (ret!=DMS_SUCCESS){
         fLocal = FALSE; 
         ret=getEffectRecord(CENTRAL,studyid,*data,FIND_FIRST);
      }
   }
   else
      ret=getEffectRecord(CENTRAL,studyid,*data,fSearchOption);

   POP();

}


//////////////////////////////////////////////////////////////////////////////
// Gets the record of the given effectid
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getVEffectRecord(DMS_VEFFECT *data, BOOL fSearchOption)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_getVEffectRecord);
   if (fSearchOption==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal){
      ret=getVEffectRecord(LOCAL,data,fSearchOption);
      if (ret!=DMS_SUCCESS){
         fLocal = FALSE; 
         ret=getVEffectRecord(CENTRAL,data,FIND_FIRST);
      }
   }
   else
      ret=getVEffectRecord(CENTRAL,data,fSearchOption);

   POP();

}


LONG DLL_INTERFACE DMS_getEffect(LONG studyid,LONG* effectid, LONG* repres, BOOL fSearchOption)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_getEffect);
   if (fSearchOption==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal){
      ret=getEffect(LOCAL,studyid,effectid,repres,fSearchOption);
      if (ret!=DMS_SUCCESS){
         fLocal = FALSE; 
         ret=getEffect(CENTRAL,studyid,effectid,repres,FIND_FIRST);
      }
   }
   else
      ret=getEffect(CENTRAL,studyid,effectid,repres,fSearchOption);

   POP();
}


//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getEffectRecordV1(LONG studyid, DMS_EFFECT *data, BOOL fSearchOption)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_getEffectRecordV1);
   if (fSearchOption==FIND_FIRST){
         fLocal=TRUE;
   }
       if (fLocal){
         ret=getEffectRecordV1(LOCAL,studyid,*data,fSearchOption);
         if (ret!=DMS_SUCCESS){
           fLocal = FALSE; 
           ret=getEffectRecordV1(CENTRAL,studyid,*data,FIND_FIRST);
		 }
	   }
       else {
         ret=getEffectRecordV1(CENTRAL,studyid,*data,fSearchOption);
	   }

   POP();

}


//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getEffectRecordV2(LONG studyid, DMS_EFFECT *data, BOOL fSearchOption)
{
   static BOOL fLocal2=TRUE;

   PUSH(DMS_getEffectRecordV2);
   if (fSearchOption==FIND_FIRST){
         fLocal2=TRUE;
   }
       if (fLocal2){
         ret=getEffectRecordV2(LOCAL,studyid,*data,fSearchOption);
         if (ret!=DMS_SUCCESS){
           fLocal2 = FALSE; 
           ret=getEffectRecordV2(CENTRAL,studyid,*data,FIND_FIRST);
		 }
	   }
       else {
         ret=getEffectRecordV2(CENTRAL,studyid,*data,fSearchOption);
	   }

   POP();

}


/////////////////////////////////////////////////////////////////////////////////
// DMS_getEffectRecordV
// Date Created  : 2002/07/02
// Date Modified : 2002/10/14
// Modification  : The record is retrieved straight from the VEFFECT table.  No need to have
//                 two separate queries for numeric and character variate.                      
////////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getEffectRecordV(LONG studyid, DMS_EFFECT *data, BOOL fSearchOption)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_getEffectRecordV);
   if (fSearchOption==FIND_FIRST){
         fLocal=TRUE;
   }
       if (fLocal){
         ret=getEffectRecordV(LOCAL,studyid,*data,fSearchOption);
         if (ret!=DMS_SUCCESS){
           fLocal = FALSE; 
           ret=getEffectRecordV(CENTRAL,studyid,*data,FIND_FIRST);
		 }
	   }
       else {
         ret=getEffectRecordV(CENTRAL,studyid,*data,fSearchOption);
	   }

   POP();


}



//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getVariateCOfRepres( DMS_EFFECT *data, BOOL fSearchOption)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_getVariateCOfRepres);
   if (fSearchOption==FIND_FIRST){
         fLocal=TRUE;
   }
       if (fLocal){
         ret=getVariateCOfRepres(LOCAL,*data,fSearchOption);
         if (ret!=DMS_SUCCESS){
           fLocal = FALSE; 
           ret=getVariateCOfRepres(CENTRAL,*data,FIND_FIRST);
		 }
	   }
       else {
         ret=getVariateCOfRepres(CENTRAL,*data,fSearchOption);
	   }

   POP();

}


//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getVariateNOfRepres( DMS_EFFECT *data, BOOL fSearchOption)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_getVariateNOfRepres);
   if (fSearchOption==FIND_FIRST){
         fLocal=TRUE;
   }
       if (fLocal){
         ret=getVariateNOfRepres(LOCAL,*data,fSearchOption);
         if (ret!=DMS_SUCCESS){
           fLocal = FALSE; 
           ret=getVariateNOfRepres(CENTRAL,*data,FIND_FIRST);
		 }
	   }
       else {
         ret=getVariateNOfRepres(CENTRAL,*data,fSearchOption);
	   }

   POP();

}


LONG DLL_INTERFACE DMS_getVariateOfRepres(DMS_EFFECT *data, BOOL fSearchOption)
{
   PUSH(getVariateOfRepres);
   static BOOL fFirst=TRUE;

   if (fSearchOption==FIND_FIRST){
         fFirst=TRUE;
   }

   if (fFirst) {
      ret = DMS_getVariateCOfRepres(data,fSearchOption);
      if (ret == DMS_NO_DATA) {
          ret = DMS_getVariateNOfRepres(data,FIND_FIRST);
		  fFirst = FALSE;

	  }
   }
   else
          ret = DMS_getVariateNOfRepres(data,fSearchOption);
   POP();
}



//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getEffectCount(LONG nfactor,LONG *represno, LONG *effectid,  BOOL fSearchOption)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_getEffectCount);
   if (fSearchOption==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal){
      ret=getEffectCount(LOCAL,nfactor,represno,effectid,fSearchOption);
      if (ret!=DMS_SUCCESS){
         fLocal = FALSE; 
         ret=getEffectCount(CENTRAL,nfactor, represno, effectid,FIND_FIRST);
      }
   }
   else
      ret=getEffectCount(CENTRAL,nfactor, represno, effectid,fSearchOption);

   POP();

}

//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_countFactorOfRepres(LONG represno, LONG *cntFactor)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_countFactorOfRepres);
   if (represno < 0){
      ret=countFactorOfRepres(LOCAL,represno,cntFactor);
   }
   else
      ret=countFactorOfRepres(CENTRAL,represno, cntFactor);

   POP();

}


//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_countVariateOfRepres(LONG represno, LONG *cntVar)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_countVariateOfRepres);
   if (represno < 0){
      ret=countVariateOfRepres(LOCAL,represno,cntVar);
   }
   else
      ret=countVariateOfRepres(CENTRAL,represno, cntVar);

   POP();

}

//////////////////////////////////////////////////////////////////////////////
//Input: array of factors
//Output: represno, effectid
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_existEffect(LONG *lstfactor, LONG nfactor, LONG *represno, LONG *effectid)
{
	PUSH(existEffect);
	ret = existEffect(lstfactor,nfactor,represno,effectid);
	POP();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Description:  Determines if a combination of level factors exist in a represenation
//  Input:   represno
//  Output:  DMS_SUCCESS if found 
//  Created: July 4, 2003
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_existCombinationEffect(long represno, struct source *dsource, long *ounitid, LONG nsource)
{
    PUSH(DMS_existCombinationEffect);
	long tag[50],i,ntsrc=50,varid,tval,texist,tlev=dsource[0].levelno, nlev,tret,tounitid;
	ret = 0;
    nlev = 0;     //July 26, 2000 
    tret =  getEffectOunit(LOCAL, represno, &tounitid, FIND_FIRST);
    while (tret == DMS_SUCCESS) {
		for (i=0;i<nsource;i++) {
			tag[i]= 0;
		}
		nlev=0;
        tret = describeOunitRepres(LOCAL,represno, tounitid,&varid,&tval,FIND_FIRST);
		while (tret == DMS_SUCCESS)  {
			nlev++;
			for (i=0;i<nsource;i++) {
				 if ((dsource[i].facid == varid) && (dsource[i].levelno == tval)) {
					tag[i] = 1;
					break;
				 }
			}
            tret = describeOunitRepres(LOCAL,represno, tounitid,&varid,&tval,FIND_NEXT);
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
        else   tret =  getEffectOunit(LOCAL, represno, &tounitid, FIND_NEXT);  
	}
    if ((tret == DMS_NO_DATA) && (represno >0)) {
       tret =  getEffectOunit(CENTRAL, represno, &tounitid, FIND_FIRST);
 	   while (tret == DMS_SUCCESS) {
		for (i=0;i<nsource;i++) {
			tag[i]= 0;
		}
		nlev = 0;
        tret = describeOunitRepres(CENTRAL,represno,tounitid,&varid,&tval,FIND_FIRST);
		while (tret == DMS_SUCCESS)  {
			nlev++;
			for (i=0;i<nsource;i++) {
				  if ((dsource[i].facid == varid) && (dsource[i].levelno == tval)) {
					tag[i] = 1;
					break;
				  }
			}
            tret = describeOunitRepres(CENTRAL,represno,tounitid,&varid,&tval,FIND_NEXT);
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
        else tret =  getEffectOunit(CENTRAL, represno, &tounitid, FIND_NEXT);
      }
	}

    if (ret!=DMS_SUCCESS) {
	   if (tret== DMS_NO_DATA)  ret= DMS_NO_DATA;
	   else ret= DMS_ERROR;
    }
	POP();
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG findFactorTMS(BOOL fLocal, DMS_FACTOR &data, LONG fSearchOption)
{
#define SQL_FINDFACTOR  \
"SELECT LABELID, FACTORID, STUDYID, FNAME, TRAITID, SCALEID, TMETHID, LTYPE FROM FACTOR  \
 WHERE (STUDYID = ? OR 0=?) AND (TRAITID = ? or 0=?)  AND (SCALEID = ? or 0=?)  AND (TMETHID = ? or 0=?) \
 ORDER BY FNAME asc "

   PUSH(findFactor);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_FINDFACTOR);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_FINDFACTOR);
       CODBCbindedStmt *source;
static CHAR fname[MAX_STR];
static DMS_FACTOR _data;
static BOOL fname_wild;

static BOOL first_time=TRUE;

   if (fSearchOption==FIND_FIRST){
      if (first_time)
      {
         BIND(1,_data.LABELID);
         BIND(2,_data.FACTORID);
         BIND(3,_data.STUDYID);
         BINDS(4,_data.FNAME,DMS_FACTOR_NAME);
		 BIND(5,_data.TRAITID);
		 BIND(6,_data.SCALEID);
		 BIND(7,_data.TMETHID);
         BINDS(8,_data.LTYPE,DMS_LEVEL_TYPE);
         BINDPARAM(1, _data.STUDYID);         
         BINDPARAM(2, _data.STUDYID);         
         BINDPARAM(3, _data.TRAITID);         
         BINDPARAM(4, _data.TRAITID);         
         BINDPARAM(5, _data.SCALEID);         
         BINDPARAM(6, _data.SCALEID);         
         BINDPARAM(7, _data.TMETHID);         
         BINDPARAM(8, _data.TMETHID);         

		 if (data.STUDYID != 0) 
  		     BINDPARAM(2,data.STUDYID);
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
#undef SQL_FINDFACTOR 
}


//////////////////////////////////////////////////////////////////////////////
//Finds a factor with the given traitid, scaleid and tmethid and studyid
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_findFactorTMS(DMS_FACTOR *data, LONG fSearchOption)
{
   static BOOL fLocal=TRUE;
   static BOOL name_wild=FALSE;

   PUSH(DMS_findFactor);
   if (fSearchOption==FIND_FIRST){
      fLocal=TRUE;
   }
   if (fLocal){
        ret=findFactorTMS(LOCAL,*data,fSearchOption);
        if (ret!=GMS_SUCCESS){
           fLocal = FALSE; 
           ret=findFactorTMS(CENTRAL,*data,FIND_NEXT);
		}
   }
     else
        ret=findFactorTMS(CENTRAL,*data,fSearchOption);
   POP();

}



////////////////////////////////////////////////////////////
//added by WVC
//***** Gets all factors belonging to a representation (both TFs and LFs) *****/
//Input: data.STUDYID, data.REPRESNO
//Output: entire factor record
//Called by DMS_getSRFactor
////////////////////////////////////////////////////////////
LONG getSRFactor(BOOL fLocal, DMS_SRFACTOR *data, int fOpt)
{
	#define SQL_GETSRFACTOR1 \
		"SELECT STUDYID, REPRESNO, LABELID, FACTOR.FACTORID, FNAME, TRAITID, SCALEID, TMETHID, LTYPE \
		FROM FACTOR, EFFECT \
		WHERE (((EFFECT.FACTORID)=FACTOR.FACTORID) AND ((EFFECT.REPRESNO)=?) \
		AND ((FACTOR.STUDYID)=?)) ORDER BY LABELID ASC"

	#define SQL_GETSRFACTOR2 \
		"SELECT STUDYID, REPRESNO, LABELID, FACTOR.FACTORID, FNAME, TRAITID, SCALEID, TMETHID, LTYPE \
		FROM FACTOR, EFFECT \
		WHERE (((EFFECT.FACTORID)=FACTOR.FACTORID) AND ((EFFECT.REPRESNO)=?) \
		AND ((FACTOR.STUDYID)=?)) ORDER BY LABELID DESC"
	
	LONG ret = DMS_SUCCESS;
	
	static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_GETSRFACTOR1);
	static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_GETSRFACTOR2);
	CODBCbindedStmt *source;
	
	static DMS_SRFACTOR _data;
	static BOOL first_time = TRUE;
	
	if (fOpt == FIND_FIRST) {
		if (first_time) {
			BINDPARAM(1, _data.REPRESNO);
			BINDPARAM(2, _data.STUDYID);

			BIND(1, _data.STUDYID);
			BIND(2, _data.REPRESNO);
			BIND(3, _data.LABELID);
			BIND(4, _data.FACTORID);
			BINDS(5, _data.FNAME, DMS_FACTOR_NAME);
			BIND(6, _data.TRAITID);
			BIND(7, _data.SCALEID);
			BIND(8, _data.TMETHID);
			BINDS(9, _data.LTYPE, DMS_LEVEL_TYPE);
			
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
	
	#undef SQL_GETSRFACTOR1
	#undef SQL_GETSRFACTOR2
}

//////////////////////////////////////////////////////////////////////////////
//added by WVC
//***** Gets all factors belonging to a representation (both TFs and LFs) *****/
//Input: data.STUDYID, data.REPRESNO
//Output: entire factor record
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getSRFactor(DMS_SRFACTOR *data, int fOpt)
{
	static BOOL fLocal = TRUE;
	
	PUSH(DMS_getSRFactor);
	
	if (fOpt == FIND_FIRST) {
		fLocal = TRUE;
	}
	if (fLocal) {
		ret = getSRFactor(LOCAL, data, fOpt);
		if (ret != DMS_SUCCESS) {
			fLocal = FALSE; 
			ret = getSRFactor(CENTRAL, data, FIND_NEXT);
		}
	}
	else
		ret = getSRFactor(CENTRAL, data, fOpt);
	
	POP();
	
}




////////////////////////////////////////////////////////////
//added by WVC
//***** Gets all unique factor names & their ontology *****/
//Input: none
//Output: factor, property, scale, & method names
//Called by DMS_getUFactor
//Modified: 20060116 (AMP) - changed the returned data structure to DMS_FACTOR
////////////////////////////////////////////////////////////
LONG getUFactor(BOOL fLocal, DMS_UFACTOR *data, int fOpt)
{
	#define SQL_UFACTOR \
		"SELECT DISTINCT FNAME, TRAITID, SCALEID, TMETHID \
		FROM FACTOR ORDER BY FNAME ASC"
	
	LONG ret = DMS_SUCCESS;
	
	static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_UFACTOR);
	static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_UFACTOR);
	CODBCbindedStmt *source;
	
	static DMS_UFACTOR _data;
	static BOOL first_time = TRUE;
	
	if (fOpt == FIND_FIRST) {
		if (first_time) {
			
			BINDS(1, _data.FNAME, DMS_FACTOR_NAME);
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
	
	#undef SQL_UFACTOR
}

//////////////////////////////////////////////////////////////////////////////
//added by WVC
//***** Gets all unique factor names & their ontology *****/
//Input: none
//Output: factor, property, scale, & method names
//Modified: 20060116 (AMP) - changed the returned data structure to DMS_FACTOR
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getUFactor(DMS_UFACTOR *data, int fOpt)
{
	static BOOL fLocal = TRUE;
	
	PUSH(DMS_getUFactor);
	
	if (fOpt == FIND_FIRST) {
		fLocal = TRUE;
	}
	if (fLocal) {
		ret = getUFactor(LOCAL, data, fOpt);
		if (ret != DMS_SUCCESS) {
			fLocal = FALSE; 
			ret = getUFactor(CENTRAL, data, FIND_NEXT);
		}
	}
	else
		ret = getUFactor(CENTRAL, data, fOpt);
	
	POP();
	
}



////////////////////////////////////////////////////////////
//added by WVC
//***** Gets all factor names belonging to each representation (dataset) of a study *****/
//Input: data.STUDYID
//Output: data.REPRESNO, data.FNAME
//Called by DMS_getSRepres
////////////////////////////////////////////////////////////
LONG getSRepres(BOOL fLocal, DMS_SREPRES *data, int fOpt)
{
	#define SQL_GETSREPRES1 \
		"SELECT STUDYID, REPRESNO, FNAME \
		FROM FACTOR, EFFECT \
		WHERE (((FACTOR.STUDYID)=?) AND ((FACTOR.LABELID)=EFFECT.FACTORID)) \
		ORDER BY REPRESNO ASC"

	#define SQL_GETSREPRES2 \
		"SELECT STUDYID, REPRESNO, FNAME \
		FROM FACTOR, EFFECT \
		WHERE (((FACTOR.STUDYID)=?) AND ((FACTOR.LABELID)=EFFECT.FACTORID)) \
		ORDER BY REPRESNO DESC"
	
	LONG ret = DMS_SUCCESS;
	
	static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_GETSREPRES1);
	static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_GETSREPRES2);
	CODBCbindedStmt *source;
	
	static DMS_SREPRES _data;
	static BOOL first_time = TRUE;
	
	if (fOpt == FIND_FIRST) {
		if (first_time) {
			BINDPARAM(1, _data.STUDYID);

			BIND(1, _data.STUDYID);
			BIND(2, _data.REPRESNO);
			BINDS(3, _data.FNAME, DMS_FACTOR_NAME);
			
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
	
	#undef SQL_GETSREPRES1
	#undef SQL_GETSREPRES2
}

//////////////////////////////////////////////////////////////////////////////
//added by WVC
//***** Gets all factor names belonging to each representation (dataset) of a study *****/
//Input: data.STUDYID
//Output: data.REPRESNO, data.FNAME
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getSRepres(DMS_SREPRES *data, int fOpt)
{
	static BOOL fLocal = TRUE;
	
	PUSH(DMS_getSRepres);
	
	if (fOpt == FIND_FIRST) {
		fLocal = TRUE;
	}
	if (fLocal) {
		ret = getSRepres(LOCAL, data, fOpt);
		if (ret != DMS_SUCCESS) {
			fLocal = FALSE; 
			ret = getSRepres(CENTRAL, data, FIND_NEXT);
		}
	}
	else
		ret = getSRepres(CENTRAL, data, fOpt);
	
	POP();
	
}


////////////////////////////////////////////////////////////
//added by WVC
//***** Gets all factors belonging to a study *****/
//Input: data.STUDYID
//Output: entire factor record
//Called by DMS_getSFactor
////////////////////////////////////////////////////////////
LONG getSFactor(BOOL fLocal, DMS_SRFACTOR *data, int fOpt)
{
	#define SQL_GETSFACTOR1 \
		"SELECT STUDYID, REPRESNO, LABELID, FACTOR.FACTORID, FNAME, TRAITID, SCALEID, TMETHID, LTYPE \
		FROM FACTOR, EFFECT \
		WHERE (((EFFECT.FACTORID)=FACTOR.FACTORID) \
		AND ((FACTOR.STUDYID)=?)) ORDER BY  LABELID ASC"

	#define SQL_GETSFACTOR2 \
		"SELECT STUDYID, REPRESNO, LABELID, FACTOR.FACTORID, FNAME, TRAITID, SCALEID, TMETHID, LTYPE \
		FROM FACTOR, EFFECT \
		WHERE (((EFFECT.FACTORID)=FACTOR.FACTORID) \
		AND ((FACTOR.STUDYID)=?)) ORDER BY  LABELID DESC "
	
	LONG ret = DMS_SUCCESS;
	
	static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_GETSFACTOR1);
	static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_GETSFACTOR2);
	CODBCbindedStmt *source;
	
	static DMS_SRFACTOR _data;
	static BOOL first_time = TRUE;
	
	if (fOpt == FIND_FIRST) {
		if (first_time) {
			BINDPARAM(1, _data.STUDYID);

			BIND(1, _data.STUDYID);
			BIND(2, _data.REPRESNO);
			BIND(3, _data.LABELID);
			BIND(4, _data.FACTORID);
			BINDS(5, _data.FNAME, DMS_FACTOR_NAME);
			BIND(6, _data.TRAITID);
			BIND(7, _data.SCALEID);
			BIND(8, _data.TMETHID);
			BINDS(9, _data.LTYPE, DMS_LEVEL_TYPE);
			
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
	
	#undef SQL_GETSFACTOR1
	#undef SQL_GETSFACTOR2
}

//////////////////////////////////////////////////////////////////////////////
//added by WVC
//***** Gets all factors belonging to a study *****/
//Input: data.STUDYID
//Output: entire factor record
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getSFactor(DMS_SRFACTOR *data, int fOpt)
{
	static BOOL fLocal = TRUE;
	
	PUSH(DMS_getSFactor);
	
	if (fOpt == FIND_FIRST) {
		fLocal = TRUE;
	}
	if (fLocal) {
		ret = getSFactor(LOCAL, data, fOpt);
		if (ret != DMS_SUCCESS) {
			fLocal = FALSE; 
			ret = getSFactor(CENTRAL, data, FIND_NEXT);
		}
	}
	else
		ret = getSFactor(CENTRAL, data, fOpt);
	
	POP();
	
}


//////////////////////////////////////////////////////////////////////////////
//Description: Updates the name of the representation
//Input: the representation number (id) and the name of the representation (name)
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_setRepresName(LONG id, char *name)
{
   PUSH(GMS_setRepresName);
   ret=setRepresName(id, name);
   POP();
}

//////////////////////////////////////////////////////////////////////////////
//Description: Retrieves the name of the representation
// Date Created:  June 13, 2008
//Input: the representation number (id) and the name of the representation (name)
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getRepresName(LONG id, char *name)
{
   PUSH(GMS_getRepresName);
   ret=getRepresName(id, name);
   POP();
}



//////////////////////////////////////////////////////////////////////////////
//Description: Updates the name of the effect
//Input: the effect id (id) and the name of the effect (name)
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_setEffectName(LONG id, char *name)
{
   PUSH(GMS_setEffectName);
   ret=setEffectName(id, name);
   POP();
}

