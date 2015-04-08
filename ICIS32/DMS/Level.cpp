/* Level.cpp : Implements the functions to access and manipulate the LEVELN & LEVELC tables of ICIS
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

#include "Level.h"

extern LONG UACCESS,USERID;
extern CODBCdbc *_centralDMS,*_localDMS;  //Central and Local Database Connection
extern stack<char*> _routines_stack;




//////////////////////////////////////////////////////////////////////////////
//  Finds the minimum  Levelno from Level_C
//////////////////////////////////////////////////////////////////////////////
LONG getNextLevelNoC(void)
{
static LONG levelNo=0;

   //if (!levelNo)
   //{
      CODBCdirectStmt local = _localDMS->DirectStmt("SELECT MIN(LEVELNO) FROM LEVEL_C");
      local.Execute();
      local.Fetch();
      if (!local.NoData())
         levelNo=local.Field(1).AsInteger();
   //}
   return --levelNo;
}


//////////////////////////////////////////////////////////////////////////////
//  Finds the minimum  Levelno from Level_N
//////////////////////////////////////////////////////////////////////////////
LONG getNextLevelNoN(void)
{
static LONG levelNo=0;

   //if (!levelNo)
   //{
      CODBCdirectStmt local = _localDMS->DirectStmt("SELECT MIN(LEVELNO) FROM LEVEL_N");
      local.Execute();
      local.Fetch();
      if (!local.NoData())
         levelNo=local.Field(1).AsInteger();
   //}
   return --levelNo;
}

//////////////////////////////////////////////////////////////////////////////
//  Finds the minimum  Levelno
//////////////////////////////////////////////////////////////////////////////
LONG getNextLevelNo(void)
{
static LONG levelNo;

      CODBCdirectStmt local = _localDMS->DirectStmt("SELECT MIN(LEVELNO) FROM LEVELS");
      local.Execute();
      local.Fetch();
      if (!local.NoData())
         levelNo=local.Field(1).AsInteger();
     return --levelNo;   
}

//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
LONG findLevel(BOOL fLocal,DMS_LEVELC &data)
{

#define SQL_FINDLEVEL "\
    select L.FACTORID , L.LEVELNO from  LEVEL_C L \
   where L.LABELID = ? and L.LVALUE = ? "

   PUSH(findLevel);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_FINDLEVEL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_FINDLEVEL);
       CODBCbindedStmt *source;

static BOOL first_time=TRUE;
static DMS_LEVELC _data;

   _data = data;
  if (first_time)
   {
      BIND(1,_data.FACTORID);
      BIND(2,_data.LEVELNO);
      BINDPARAM(1,_data.LABELID);
      BINDPARAMS(2,_data.LVALUE,sizeof(_data.LVALUE)-1);
      first_time=FALSE;
   }


   source = (fLocal)?&local:&central;
   source->Execute();

   if (source->Fetch())
   {
      data = _data;
   }
   else if (source->NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;

   POP();

#undef SQL_FINDLEVEL
}


//////////////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////////////
LONG findLevelNo(BOOL fLocal,long factorid, long levelno)
{

#define SQL_FINDLEVELNO "\
    select L.FACTORID , L.LEVELNO from  LEVELS L \
   where  L.FACTORID = ? and L.LEVELNO = ? "

   PUSH(findLevel);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_FINDLEVELNO);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_FINDLEVELNO);
       CODBCbindedStmt *source;

static BOOL first_time=TRUE;
static long _factorid, _levelno;

   _factorid = factorid;
   _levelno = levelno;
  if (first_time)
   {
      BINDPARAM(1,_factorid);
      BINDPARAM(2,_levelno);
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

#undef SQL_FINDLEVELNO
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG findLevelN(BOOL fLocal,DMS_LEVELN &data)
{

#define SQL_FINDLEVELN "\
    select L.FACTORID , L.LEVELNO from  LEVEL_N L \
   where L.LABELID = ? and L.LVALUE = ? "

   PUSH(findLevelN);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_FINDLEVELN);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_FINDLEVELN);
       CODBCbindedStmt *source;

static BOOL first_time=TRUE;
static DMS_LEVELN _data;

   _data = data;

  if (first_time)
   {
      BIND(1,_data.FACTORID);
      BIND(2,_data.LEVELNO);
      BINDPARAM(1,_data.LABELID);
      BINDPARAM(2,_data.LVALUE);
      first_time=FALSE;
   }

   source = (fLocal)?&local:&central;
   source->Execute();

   if (source->Fetch())
   {
      data = _data;
/**	   data.LABELID = _data.LABELID;
	  data.FACTORID = _data.FACTORID;
	  data.LEVELNO = _data.LEVELNO;
	  data.LVALUE  = atof(_data.LVALUE);
**/
  }
   else if (source->NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;

   POP();

#undef SQL_FINDLEVELN
}



//////////////////////////////////////////////////////////////////////////////
//  findLabel - finds if a label record exists in the LEVEL_Ctable
//////////////////////////////////////////////////////////////////////////////
LONG findLabel(BOOL fLocal,DMS_LEVELC &data)
{

#define SQL_FINDLABEL "\
   SELECT LEVELNO from LEVEL_C WHERE LABELID=? AND FACTORID=? \
   AND LEVELNO=? AND LVALUE=? "

   PUSH(findLabel);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_FINDLABEL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_FINDLABEL);
       CODBCbindedStmt *source;

static BOOL first_time=TRUE;
static DMS_LEVELC _data;
static LONG tlevno;

   _data = data;
   if (first_time)
   {
      BIND(1,tlevno);
      BINDPARAM(1,_data.LABELID);
	  BINDPARAM(2,_data.FACTORID);
	  BINDPARAM(3,_data.LEVELNO);
      BINDPARAMS(4,_data.LVALUE,sizeof(_data.LVALUE)-1);
      first_time=FALSE;
   }


   source = (fLocal)?&local:&central;
   source->Execute();

   if (source->Fetch())
   {
      memcpy(&data,&_data,sizeof(_data));
   }
   else if (source->NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;

   POP();

#undef SQL_FINDLABEL
}


//////////////////////////////////////////////////////////////////////////////
//  findLabelN - finds if a numeric label record exists in the LEVEL_Ctable
//////////////////////////////////////////////////////////////////////////////
LONG findLabelN(BOOL fLocal,DMS_LEVELN &data)
{

#define SQL_FINDLABELN "\
   SELECT LEVELNO FROM FACTOR F, LEVEL_N L \
   where F.LABELID = L.LABELID AND L.LABELID=? AND L.FACTORID=? \
   AND L.LEVELNO=? AND L.LVALUE=? "

   PUSH(findLabel);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_FINDLABELN);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_FINDLABELN);
       CODBCbindedStmt *source;

static BOOL first_time=TRUE;
static DMS_LEVELN _data;
static LONG tlevno;

   if (first_time)
   {
      BIND(1,tlevno);
      BINDPARAM(1,_data.LABELID);
	  BINDPARAM(2,_data.FACTORID);
	  BINDPARAM(3,_data.LEVELNO);
      BINDPARAM(4,_data.LVALUE);
      first_time=FALSE;
   }

   _data = data;
   source = (fLocal)?&local:&central;
   source->Execute();

   if (source->Fetch())
   {
      data= _data;
   }
   else if (source->NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;

   POP();

#undef SQL_FINDLABELN
}



//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG addLevelNo(DMS_LEVELNO *data)
{
#define SQL_ADDEVELNO  "insert into LEVELS \
             (LEVELNO,FACTORID) \
              values (?, ?) "

   PUSH(addLevelNo);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_ADDEVELNO);

static DMS_LEVELNO _data;
static BOOL first_time=TRUE;
static LONG tlong;
   
   data->LEVELNO = getNextLevelNo();
   if (first_time)
   {
      local.BindParam(1,_data.LEVELNO);
      local.BindParam(2,_data.FACTORID);
      first_time=FALSE;
   }

   _data.FACTORID = data->FACTORID;
   _data.LEVELNO = data->LEVELNO;

   if ((ret=local.Execute())==1) {
      ret=GMS_SUCCESS;
      DMS_commitData; 
   }
   //else
   //   ret=GMS_ERROR;

    else  
	   if (ret!=GMS_UNIQUE_CONSTRAINT_ERROR) //GMS_UNIQUE_CONSTRAINT_ERROR
		   ret=GMS_ERROR;  
       // Handler for unique contraint error with the LEVELNO having unique contraint 
	   else {
		   int i=0;
		   while ((ret==GMS_UNIQUE_CONSTRAINT_ERROR) && (i<100)) {
			    _data.LEVELNO = getNextLevelNo();
				ret=local.Execute();
				DMS_commitData;
				++i;
		   }
		   if ((ret) &&(ret!=GMS_UNIQUE_CONSTRAINT_ERROR)) {
               *data = _data;
			   ret = DMS_SUCCESS;
		   }
		   else  {
			   data->LEVELNO = 0;
			   ret = DMS_ERROR;
		   }
	   }

   POP();

#undef SQL_ADDLEVELNO
}




//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG addLevel(DMS_LEVELC &data)
{
#define SQL_ADDLEVELC "insert into LEVEL_C (LABELID, FACTORID,LEVELNO,LVALUE) values (?,?,?,?)"

   PUSH(addLevelC);

static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_ADDLEVELC);

static DMS_LEVELC _data;
static BOOL first_time=TRUE;
static LONG rt;
DMS_LEVELNO dataLevel;


   _data = data;
   if (data.FACTORID == data.LABELID) {
       if (data.FACTORID < 0) {
         rt = findLevel(LOCAL,_data);
         if (rt==DMS_SUCCESS) {
	       data.LEVELNO = _data.LEVELNO;
		   data.FACTORID = _data.FACTORID;
	       POP2(DMS_EXIST);
		 }
	   }
       else{
         rt = findLevel(CENTRAL,_data);
         if (rt==DMS_SUCCESS) {
	       data.LEVELNO =_data.LEVELNO;
		   data.FACTORID = _data.FACTORID;
	       POP2(DMS_EXIST);
		 }
         else {
           rt = findLevel(LOCAL,_data);
           if (rt==DMS_SUCCESS) {
	         data.LEVELNO = data.LEVELNO;
	         POP2(DMS_EXIST);
		   }
		 }
	   }
 
	   dataLevel.FACTORID = data.FACTORID;
	   dataLevel.LEVELNO = 0;
	   rt = addLevelNo(&dataLevel);
	   if (rt == DMS_ERROR) 
  	         POP2(DMS_ERROR)
	   else 
            data.LEVELNO = dataLevel.LEVELNO;
//5.4 version       data.LEVELNO =getNextLevelNo();
   }
   else {
       if (data.LABELID < 0) {
	     rt = findLabel(LOCAL,_data);
		 if (rt==DMS_SUCCESS) POP2(DMS_EXIST);
		 if (rt==DMS_ERROR) POP2(DMS_ERROR);
	   }
	   else {
	     rt = findLabel(CENTRAL,_data);
		 if (rt==DMS_SUCCESS) POP2(DMS_EXIST);
		 if (rt==DMS_ERROR) POP2(DMS_ERROR);
	   }

   }

 
   //if (data.FACTORID == data.LABELID) {
	  // if (DMS_findLevelNo(data.FACTORID, data.LEVELNO) != DMS_SUCCESS)
	  // {
			//dataLevel.LEVELNO = 0;
			//rt = addLevelNo(&dataLevel);
			//if (rt == DMS_ERROR) 
  	//				POP2(DMS_ERROR)
			//else 
			//		data.LEVELNO = dataLevel.LEVELNO;
	  // }
   //}


   if (first_time)
   {

	  local.BindParam(1,_data.LABELID);
      local.BindParam(2,_data.FACTORID);
      local.BindParam(3,_data.LEVELNO);
      local.BindParam(4,_data.LVALUE,sizeof(_data.LVALUE)-1);

      first_time=FALSE;
   }
   _data = data;
   //memcpy(&_data,&data,sizeof(data));

   if (local.Execute())
      ret=GMS_SUCCESS;
   else
      ret=GMS_ERROR;


   POP();

#undef SQL_ADDLEVELC
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG addLevel2(DMS_LEVELC &data)
{
#define SQL_ADDLEVELC2 "insert into LEVEL_C (LABELID, FACTORID,LEVELNO,LVALUE) values (?,?,?,?)"

   PUSH(addLevelC2);

static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_ADDLEVELC2);

static DMS_LEVELC _data;
static BOOL first_time=TRUE;
static LONG rt;


   _data = data;
   if (data.FACTORID == data.LABELID) {
     data.LEVELNO =getNextLevelNo();
   }

   if (first_time)
   {

	  local.BindParam(1,_data.LABELID);
      local.BindParam(2,_data.FACTORID);
      local.BindParam(3,_data.LEVELNO);
      local.BindParam(4,_data.LVALUE,sizeof(_data.LVALUE)-1);

      first_time=FALSE;
   }
   _data = data;
   //memcpy(&_data,&data,sizeof(data));

   if (local.Execute())
      ret=GMS_SUCCESS;
   else
      ret=GMS_ERROR;


   POP();

#undef SQL_ADDLEVELC2
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG addLevelN(DMS_LEVELN &data)
{
#define SQL_ADDLEVELN "insert into LEVEL_N (LABELID, FACTORID,LEVELNO,LVALUE) values (?,?,?,?)"

   PUSH(addLevelN);

static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_ADDLEVELN);

static DMS_LEVELN _data;
static BOOL first_time=TRUE;
static LONG rt;
	   DMS_LEVELNO dataLevel;



   _data = data;
   if (data.FACTORID == data.LABELID) {
       if (data.FACTORID < 0) {
         rt = findLevelN(LOCAL,_data);
         if (rt==DMS_SUCCESS) {
	       data.LEVELNO = _data.LEVELNO;
		   data.FACTORID = _data.FACTORID;
	       POP2(DMS_EXIST);
		 }
	   }
       else{
         rt = findLevelN(CENTRAL,_data);
         if (rt==DMS_SUCCESS) {
	       data.LEVELNO =_data.LEVELNO;
		   data.FACTORID = _data.FACTORID;
	       POP2(DMS_EXIST);
		 }
         else {
           rt = findLevelN(LOCAL,_data);
           if (rt==DMS_SUCCESS) {
	         data.LEVELNO = data.LEVELNO;
	         POP2(DMS_EXIST);
		   }
		 }
	   }
	   dataLevel.FACTORID = data.FACTORID;
	   dataLevel.LEVELNO = 0;
	   rt = addLevelNo(&dataLevel);
	   if (rt == DMS_ERROR) 
  	       POP2(DMS_ERROR)
	   else
           data.LEVELNO = dataLevel.LEVELNO;

//version 5.4       data.LEVELNO =getNextLevelNo();
   }
   else {
       if (data.LABELID < 0) {
	     rt = findLabelN(LOCAL,_data);
		 if (rt==DMS_SUCCESS) POP2(DMS_EXIST);
		 if (rt==DMS_ERROR) POP2(DMS_ERROR);
	   }
	   else {
	     rt = findLabelN(CENTRAL,_data);
		 if (rt==DMS_SUCCESS) POP2(DMS_EXIST);
		 if (rt==DMS_ERROR) POP2(DMS_ERROR);
	   }

   }

  //if (data.FACTORID == data.LABELID) {
	 //  if (DMS_findLevelNo(data.FACTORID, data.LEVELNO) != DMS_SUCCESS)
	 //  {
		//	dataLevel.LEVELNO = 0;
		//	rt = addLevelNo(&dataLevel);
		//	if (rt == DMS_ERROR) 
  //					POP2(DMS_ERROR)
		//	else 
		//			data.LEVELNO = dataLevel.LEVELNO;
	 //  }
  // }
 
   if (first_time)
   {

	  local.BindParam(1,_data.LABELID);
      local.BindParam(2,_data.FACTORID);
      local.BindParam(3,_data.LEVELNO);
      local.BindParam(4,_data.LVALUE);

      first_time=FALSE;
   }
   _data = data;
   //memcpy(&_data,&data,sizeof(data));

   if (local.Execute())
      ret=GMS_SUCCESS;
   else
      ret=GMS_ERROR;


   POP();

#undef SQL_ADDLEVELN
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG addLevelN2(DMS_LEVELN &data)
{
#define SQL_ADDLEVELN2 "insert into LEVEL_N (LABELID, FACTORID,LEVELNO,LVALUE) values (?,?,?,?)"

   PUSH(addLevelN2);

static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_ADDLEVELN2);

static DMS_LEVELN _data;
static BOOL first_time=TRUE;
static LONG rt;



   _data = data;
   if (data.FACTORID == data.LABELID) {
       data.LEVELNO =getNextLevelNo();
   }
   
   if (first_time)
   {

	  local.BindParam(1,_data.LABELID);
      local.BindParam(2,_data.FACTORID);
      local.BindParam(3,_data.LEVELNO);
      local.BindParam(4,_data.LVALUE);

      first_time=FALSE;
   }
   _data = data;
   //memcpy(&_data,&data,sizeof(data));

   if (local.Execute())
      ret=GMS_SUCCESS;
   else
      ret=GMS_ERROR;


   POP();

#undef SQL_ADDLEVELN2
}


//////////////////////////////////////////////////////////////////////////////
//    getLevel - retrieves the record with the specified labelid and levelno
//////////////////////////////////////////////////////////////////////////////
LONG getLevel(BOOL fLocal, DMS_LEVELC &data)
{
#define SQL_GETLEVEL "\
     select L.LVALUE from LEVEL_C L  \
	 where L.LABELID = ? and L.FACTORID=? and L.LEVELNO = ? "

   PUSH(getLevel);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_GETLEVEL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_GETLEVEL);
       CODBCbindedStmt *source;

static DMS_LEVELC _data;
    
   _data = data;
   BINDS(1,_data.LVALUE,DMS_DATA_VALUE);
   BINDPARAM(1,_data.LABELID);
   BINDPARAM(2, _data.FACTORID);
   BINDPARAM(3,_data.LEVELNO);
 
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

#undef SQL_GETLEVEL
}


//////////////////////////////////////////////////////////////////////////////
//    getMaxLevel - retrieves the record with the maximum value for the specified trait
//  Date Created: Mar 25, 2008
//  Created by: AMP
//  Date Modified: 
//////////////////////////////////////////////////////////////////////////////
LONG getMaxLevel(BOOL fLocal, LONG traitid, CHAR *lvalue)
{
#define SQL_GETLEVEL "\
		SELECT Max(LEVEL_C.LVALUE) AS MaxOfLVALUE \
		FROM FACTOR INNER JOIN LEVEL_C ON FACTOR.LABELID = LEVEL_C.LABELID \
		GROUP BY FACTOR.TRAITID \
		HAVING (((FACTOR.TRAITID)=?))"

   PUSH(getLevel);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_GETLEVEL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_GETLEVEL);
       CODBCbindedStmt *source;

static CHAR _lvalue[GMS_MAX_NVAL];
static LONG _traitid;
    
   _traitid = traitid;
   BINDS(1,_lvalue,ICIS_MAX_SIZE );
   BINDPARAM(1, _traitid);

   local.Execute();
   central.Execute();

   source = (fLocal)?&local:&central;

   if (source->Fetch()){
       strncpy(lvalue,_lvalue,ICIS_MAX_SIZE);
   }
   else if (source->NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;

   POP();

#undef SQL_GETLEVEL
}



//////////////////////////////////////////////////////////////////////////////
//    getLevelN - retrieves the record with the specified labelid and levelno
//////////////////////////////////////////////////////////////////////////////
LONG getLevelN(BOOL fLocal, DMS_LEVELN &data)
{
#define SQL_GETLEVEL "\
     select L.LVALUE from LEVEL_N L  \
	 where L.LABELID = ? and L.FACTORID=? and L.LEVELNO = ? "

   PUSH(getLevel);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_GETLEVEL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_GETLEVEL);
       CODBCbindedStmt *source;

static DMS_LEVELN _data;
    
   _data = data;
   BIND(1,_data.LVALUE);
   BINDPARAM(1,_data.LABELID);
   BINDPARAM(2, _data.FACTORID);
   BINDPARAM(3,_data.LEVELNO);
 
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

#undef SQL_GETLEVEL
}

//////////////////////////////////////////////////////////////////////////////
//    getLabel - retrieves the label record with the specified factorid and levelno
//////////////////////////////////////////////////////////////////////////////
LONG getLabelLevel(BOOL fLocal, DMS_LEVELC &data, LONG fSearchOption )
{
#define SQL_GETLABEL "\
     select L.LABELID, L.LVALUE from LEVEL_C L  \
	 where L.FACTORID = ? and L.LEVELNO = ? "

   PUSH(getLabel);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_GETLABEL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_GETLABEL);
       CODBCbindedStmt *source;

static DMS_LEVELC _data;
static BOOL first_time=TRUE;
 
  if (fSearchOption==FIND_FIRST){
     _data = data;
     if (first_time){
        BIND(1, _data.LABELID);
        BINDS(2,_data.LVALUE,DMS_DATA_VALUE);
        BINDPARAM(1,_data.FACTORID);
        BINDPARAM(2,_data.LEVELNO);
        first_time=FALSE;
	  }
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

#undef SQL_GETLABEL
}


//////////////////////////////////////////////////////////////////////////////
//    getLabel - retrieves the label record with the specified factorid and levelno
//////////////////////////////////////////////////////////////////////////////
LONG getLabelLevelN(BOOL fLocal, DMS_LEVELN &data, LONG fSearchOption )
{
#define SQL_GETLABEL "\
     select L.LABELID, L.LVALUE from LEVEL_N L  \
	 where L.FACTORID = ? and L.LEVELNO = ? "

   PUSH(getLabel);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_GETLABEL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_GETLABEL);
       CODBCbindedStmt *source;

static DMS_LEVELN _data;
static BOOL first_time=TRUE;
 
  if (fSearchOption==FIND_FIRST){
     _data = data;
     if (first_time){
        BIND(1, _data.LABELID);
        BIND(2,_data.LVALUE);
        BINDPARAM(1,_data.FACTORID);
        BINDPARAM(2,_data.LEVELNO);
        first_time=FALSE;
	  }
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

#undef SQL_GETLABEL
}


//////////////////////////////////////////////////////////////////////////////
//    getLabelN - retrieves the label records with the specified labelid and levelno
//////////////////////////////////////////////////////////////////////////////
LONG getLabelN(BOOL fLocal, DMS_LEVELN &data, LONG fSearchOption)
{
#define SQL_GETLABELN "\
     select L.LABELID, L.LVALUE from LEVEL_N L  \
	 where L.FACTORID = ? and L.LEVELNO = ? "

   PUSH(getLabelN);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_GETLABELN);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_GETLABELN);
       CODBCbindedStmt *source;

static DMS_LEVELN _data;
static BOOL first_time=TRUE;
 
  if (fSearchOption==FIND_FIRST){
     _data = data;
     if (first_time){
        BIND(1, _data.LABELID);
        BIND(2,_data.LVALUE);
        BINDPARAM(1,_data.FACTORID);
        BINDPARAM(2,_data.LEVELNO);
        first_time=FALSE;
	  }
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

#undef SQL_GETLABELN
}


//////////////////////////////////////////////////////////////////////////////
//    getAllLevel - retrieves all records with the given labelid
//////////////////////////////////////////////////////////////////////////////
LONG getAllLevel(BOOL fLocal, DMS_LEVELC &data, LONG fSearchOption )
{
#define SQL_GETALLLEVEL "\
       select L.FACTORID, L.LEVELNO, LVALUE from LEVEL_C L  where L.LABELID = ? "
   PUSH(getLabel);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_GETALLLEVEL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_GETALLLEVEL);
       CODBCbindedStmt *source;

static DMS_LEVELC _data;
static BOOL first_time=TRUE;
 
  if (fSearchOption==FIND_FIRST){
      _data = data;
      if (first_time){
   
        BINDPARAM(1, _data.LABELID);
        BIND(1,_data.FACTORID);
        BIND(2,_data.LEVELNO);
		BINDS(3,_data.LVALUE, sizeof(_data.LVALUE)-1);
        first_time=FALSE;
	  }
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

#undef SQL_GETALLLEVEL
}


//////////////////////////////////////////////////////////////////////////////
////    getAllLevelN 
//////////////////////////////////////////////////////////////////////////////
LONG getAllLevelN(BOOL fLocal, DMS_LEVELN &data, LONG fSearchOption)
{
#define SQL_GETALLLEVEL_N "\
       select L.FACTORID, L.LEVELNO, L.LVALUE from LEVEL_N L  where L.LABELID = ? \
	   ORDER BY L.LEVELNO"
   PUSH(getLabel);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_GETALLLEVEL_N);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_GETALLLEVEL_N);
       CODBCbindedStmt *source;

static DMS_LEVELN _data;
static BOOL first_time=TRUE;
 
  if (fSearchOption==FIND_FIRST){
      _data = data;
      if (first_time){
   
        BINDPARAM(1, _data.LABELID);
        BIND(1,_data.FACTORID);
        BIND(2,_data.LEVELNO);
		BIND(3,_data.LVALUE);
        first_time=FALSE;
	  }
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

#undef SQL_GETALLLEVEL_N
}


//////////////////////////////////////////////////////////////////////////////
//    getAllLabel - retrieves all records with the given factorid from both Level_C and Level_N
//////////////////////////////////////////////////////////////////////////////
LONG getAllLabelNC(BOOL fLocal, DMS_LEVELC &data, LONG fSearchOption )
{
#define SQL_GETALLLABEL_NC_C "\
        SELECT levelno, labelid,  {fn COnvert( LVALUE,SQL_VARCHAR) }   from Level_N  where factorid=?   union  \
        select   levelno,labelid, lvalue from Level_C where factorid=? order by levelno ASC,labelid ASC "

#define SQL_GETALLLABEL_NC_L "\
        SELECT levelno, labelid,  {fn COnvert( LVALUE,SQL_VARCHAR) }   from Level_N  where factorid=?   union  \
        select   levelno,labelid, lvalue from Level_C where factorid=? order by levelno DESC,labelid DESC"


#define SQL_GETALLLABEL_NC_C_MYSQL "\
        SELECT levelno, labelid,  CAST(LVALUE as CHAR)    from Level_N  where factorid=?                union  \
        select   levelno,labelid, lvalue from Level_C where factorid=? order by levelno ASC,labelid ASC "

#define SQL_GETALLLABEL_NC_L_MYSQL "\
        SELECT levelno, labelid,  CAST(LVALUE AS CHAR)    from Level_N  where factorid=?               union  \
        select   levelno,labelid, lvalue from Level_C where factorid=? order by levelno DESC,labelid DESC"

       LPCSTR szDB;
		
		PUSH(getAllLabel);

static CODBCbindedStmt &central= _centralDMS->BindedStmt();
static CODBCbindedStmt &local  = _localDMS->BindedStmt();

       CODBCbindedStmt *source;
static string central_sql,local_sql;

static DMS_LEVELC _data;
static BOOL first_time=TRUE;


  if (fSearchOption==FIND_FIRST){
      _data = data;
      if (first_time){
        szDB = _localDMS->DatabaseName();
        if (strcmp(szDB, "MySQL")==0 ) {
           local_sql=SQL_GETALLLABEL_NC_L_MYSQL;
		}
        else
           local_sql=SQL_GETALLLABEL_NC_L;
        szDB = _centralDMS->DatabaseName();
        if (strcmp(szDB, "MySQL")==0 ) {
          central_sql=SQL_GETALLLABEL_NC_C_MYSQL;
		}
        else
		  central_sql= SQL_GETALLLABEL_NC_C;
   
        central.SetSQLstr(central_sql.c_str());
        local.SetSQLstr(local_sql.c_str());

        BINDPARAM(1, _data.FACTORID);
        BINDPARAM(2, _data.FACTORID);
        BIND(2,_data.LABELID);
        BIND(1,_data.LEVELNO);
		BINDS(3,_data.LVALUE, sizeof(_data.LVALUE)-1);
        first_time=FALSE;
	  }
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

#undef SQL_GETALLLABEL_NC_C
#undef SQL_GETALLLABEL_NC_L
}


//////////////////////////////////////////////////////////////////////////////
//    getAllLabel - retrieves all records with the given labelid
//////////////////////////////////////////////////////////////////////////////
LONG getAllLabel(BOOL fLocal, DMS_LEVELC &data, LONG fSearchOption )
{
#define SQL_GETALLLABEL "\
       select L.LABELID, L.LEVELNO, LVALUE from LEVEL_C L  where L.FACTORID = ? \
	   ORDER BY L.LEVELNO, L.LABELID"
      PUSH(getAllLabel);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_GETALLLABEL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_GETALLLABEL);
       CODBCbindedStmt *source;

static DMS_LEVELC _data;
static BOOL first_time=TRUE;
 
  if (fSearchOption==FIND_FIRST){
      _data = data;
      if (first_time){
   
        BINDPARAM(1, _data.FACTORID);
        BIND(1,_data.LABELID);
        BIND(2,_data.LEVELNO);
		BINDS(3,_data.LVALUE, sizeof(_data.LVALUE)-1);
        first_time=FALSE;
	  }
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

#undef SQL_GETALLLABEL
}


//////////////////////////////////////////////////////////////////////////////
////    getAllLabelN 
//////////////////////////////////////////////////////////////////////////////
LONG getAllLabelN(BOOL fLocal, DMS_LEVELN &data, LONG fSearchOption)
{
#define SQL_GETALLLABEL_N "\
       select L.LABELID, L.LEVELNO, L.LVALUE from LEVEL_N L  where L.FACTORID = ? \
	   ORDER BY L.LEVELNO, L.LABELID"
   PUSH(getAllLabelN);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_GETALLLABEL_N);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_GETALLLABEL_N);
       CODBCbindedStmt *source;

static DMS_LEVELN _data;
static BOOL first_time=TRUE;
 
  if (fSearchOption==FIND_FIRST){
      _data = data;
      if (first_time){
   
        BINDPARAM(1, _data.FACTORID);
        BIND(1,_data.LABELID);
        BIND(2,_data.LEVELNO);
		BIND(3,_data.LVALUE);
        first_time=FALSE;
	  }
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

#undef SQL_GETALLLABEL_N
}


//////////////////////////////////////////////////////////////////////////////
// getOunitLevelC - retrieves all observation units of the given label
//////////////////////////////////////////////////////////////////////////////
LONG getOunitLevelC(BOOL fLocal, DMS_LEVELC *Lev, long *ounitid, long fSearchOption)
{
#define SQL_GETOUNITLEVEL "SELECT OINDEX.OUNITID, FACTOR.FACTORID, LEVEL_C.LEVELNO, LEVEL_C.LVALUE" \
                          " FROM LEVEL_C, FACTOR, OINDEX "\
                          " WHERE (((FACTOR.LABELID)=?) AND ((FACTOR.LABELID)=LEVEL_C.LABELID) AND ((LEVEL_C.FACTORID)=OINDEX.FACTORID) AND ((LEVEL_C.LEVELNO)=OINDEX.LEVELNO))" \
						  " ORDER BY OINDEX.OUNITID "
#define SQL_GETOUNITLEVEL_L "SELECT OINDEX.OUNITID, FACTOR.FACTORID, LEVEL_C.LEVELNO, LEVEL_C.LVALUE" \
                          " FROM LEVEL_C, FACTOR, OINDEX "\
                          " WHERE (((FACTOR.LABELID)=?) AND ((FACTOR.LABELID)=LEVEL_C.LABELID) AND ((LEVEL_C.FACTORID)=OINDEX.FACTORID) AND ((LEVEL_C.LEVELNO)=OINDEX.LEVELNO))" \
						  " ORDER BY OINDEX.OUNITID DESC"

   PUSH(getOunitLevelC);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_GETOUNITLEVEL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_GETOUNITLEVEL_L);
       CODBCbindedStmt *source;

static long tounit;
static DMS_LEVELC tlevc;
static BOOL first_time=TRUE;


   if (fSearchOption==FIND_FIRST)
   {
      if (first_time)
      {
         BIND(1,tounit);
		 BIND(2, tlevc.FACTORID);
		 BIND(3, tlevc.LEVELNO)
		 BINDS(4, tlevc.LVALUE, sizeof(tlevc.LVALUE)-1);
         BINDPARAM(1,tlevc.LABELID);
 
         first_time=FALSE;
      }
      tlevc =  *Lev;
      local.Execute();
      central.Execute();
   }
   source = (fLocal)?&local:&central;

   if (source->Fetch()) {
      *ounitid = tounit;
      *Lev = tlevc;
   }
   else if (source->NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;

   POP();

#undef SQL_GETOUNITLEVEL
#undef SQL_GETOUNITLEVEL_L

}


//////////////////////////////////////////////////////////////////////////////
// getOunitLevelC - retrieves all observation units of the given label
//////////////////////////////////////////////////////////////////////////////
LONG getOunitLevelN(BOOL fLocal, DMS_LEVELN *Lev, long *ounitid, long fSearchOption)
{
#define SQL_GETOUNITLEVELN "SELECT Distinct OINDEX.OUNITID, FACTOR.FACTORID, LEVEL_N.LEVELNO, LEVEL_N.LVALUE" \
                          " FROM LEVEL_N, FACTOR, OINDEX "\
                          " WHERE (((FACTOR.LABELID)=?) AND ((FACTOR.LABELID)=LEVEL_N.LABELID) AND ((LEVEL_N.FACTORID)=OINDEX.FACTORID) AND ((LEVEL_N.LEVELNO)=OINDEX.LEVELNO))" \
						  " ORDER BY OINDEX.OUNITID "
#define SQL_GETOUNITLEVELN_L "SELECT Distinct OINDEX.OUNITID, FACTOR.FACTORID, LEVEL_N.LEVELNO, LEVEL_N.LVALUE" \
                          " FROM LEVEL_N, FACTOR, OINDEX "\
                          " WHERE (((FACTOR.LABELID)=?) AND ((FACTOR.LABELID)=LEVEL_N.LABELID) AND ((LEVEL_N.FACTORID)=OINDEX.FACTORID) AND ((LEVEL_N.LEVELNO)=OINDEX.LEVELNO))" \
						  " ORDER BY OINDEX.OUNITID DESC"
   PUSH(getOunitLevelN);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_GETOUNITLEVELN);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_GETOUNITLEVELN_L);
       CODBCbindedStmt *source;

static long tounit;
static DMS_LEVELN tlevn;
static BOOL first_time=TRUE;


   if (fSearchOption==FIND_FIRST)
   {
      if (first_time)
      {
         BIND(1,tounit);
		 BIND(2, tlevn.FACTORID);
		 BIND(3, tlevn.LEVELNO)
		 BIND(4, tlevn.LVALUE);
         BINDPARAM(1,tlevn.LABELID);
 
         first_time=FALSE;
      }
      tlevn =  *Lev;
      local.Execute();
      central.Execute();
   }
   source = (fLocal)?&local:&central;

   if (source->Fetch()) {
      *ounitid = tounit;
      *Lev = tlevn;
   }
   else if (source->NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;

   POP();

#undef SQL_GETOUNITLEVELN
#undef SQL_GETOUNITLEVELN_L

}



//////////////////////////////////////////////////////////////////////////////
// extractOunitLVNEq - retrieves observation units whose level value of the given label is equal to the specified value
//////////////////////////////////////////////////////////////////////////////
LONG extractOunitLVN_EQ(BOOL fLocal, DMS_OINDEX *Ounit, long labelid, double *lvalue, long fSearchOption)
{
#define SQL_ExtractOunitLVNEq "SELECT OINDEX.OUNITID, OINDEX.FACTORID, OINDEX.LEVELNO, OINDEX.REPRESNO,  LEVEL_N.LVALUE \
                               FROM OINDEX , LEVEL_N     \
							   WHERE OINDEX.LEVELNO = LEVEL_N.LEVELNO AND OINDEX.REPRESNO=? AND LEVEL_N.LABELID = ? AND LEVEL_N.LVALUE = ?"


   PUSH(extractOunitLVNEq);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_ExtractOunitLVNEq);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_ExtractOunitLVNEq);
       CODBCbindedStmt *source;

static long _repres, _labelid;
static double _lvalue, _plvalue;
static DMS_OINDEX _oindex;
static BOOL first_time=TRUE;


   if (fSearchOption==FIND_FIRST)
   {
      if (first_time)
      {
         BIND(1,_oindex.OUNITID);
		 BIND(2, _oindex.FACTORID);
		 BIND(3, _oindex.LEVELNO)
		 BIND(4, _oindex.REPRESNO);
		 BIND(5,_lvalue);
         BINDPARAM(1,_repres);
		 BINDPARAM(2, _labelid);
         BINDPARAM(3,_plvalue);

 
         first_time=FALSE;
      }
	  _plvalue = *lvalue;
	  _repres = Ounit->REPRESNO;
	  _labelid = labelid;

      local.Execute();
      central.Execute();
   }
   source = (fLocal)?&local:&central;

   if (source->Fetch()) {
      *Ounit = _oindex;
      *lvalue = _lvalue;
   }
   else if (source->NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;

   POP();

#undef SQL_ExtractOunitLVNEq

}


//////////////////////////////////////////////////////////////////////////////
// extractOunitLVN_GT - retrieves observation units whose value to a given labelid is greater than the given level value
// Input:    a representation no and level value
//////////////////////////////////////////////////////////////////////////////
LONG extractOunitLVN_GT(BOOL fLocal, DMS_OINDEX *Ounit, long labelid, double *lvalue, long fSearchOption)
{
#define SQL_ExtractOunitLVNGT "SELECT OINDEX.OUNITID, OINDEX.FACTORID, OINDEX.LEVELNO, OINDEX.REPRESNO,  LEVEL_N.LVALUE \
                               FROM OINDEX , LEVEL_N     \
							   WHERE OINDEX.LEVELNO = LEVEL_N.LEVELNO AND OINDEX.REPRESNO=? AND LEVEL_N.LABELID = ? AND LEVEL_N.LVALUE > ?  \
							   ORDER BY LEVEL_N.LVALUE ASC"


   PUSH(extractOunitLVNGT);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_ExtractOunitLVNGT);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_ExtractOunitLVNGT);
       CODBCbindedStmt *source;

static long _repres, _labelid;
static double _lvalue, _plvalue;
static DMS_OINDEX _oindex;
static BOOL first_time=TRUE;


   if (fSearchOption==FIND_FIRST)
   {
      if (first_time)
      {
         BIND(1,_oindex.OUNITID);
		 BIND(2, _oindex.FACTORID);
		 BIND(3, _oindex.LEVELNO)
		 BIND(4, _oindex.REPRESNO);
		 BIND(5,_lvalue);
         BINDPARAM(1,_repres);
		 BINDPARAM(2, _labelid);
         BINDPARAM(3,_plvalue);

 
         first_time=FALSE;
      }
	  _plvalue = *lvalue;
	  _repres = Ounit->REPRESNO;
	  _labelid = labelid;

      local.Execute();
      central.Execute();
   }
   source = (fLocal)?&local:&central;

   if (source->Fetch()) {
      *Ounit = _oindex;
      *lvalue = _lvalue;
   }
   else if (source->NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;

   POP();

#undef SQL_ExtractOunitLVNGT

}


//////////////////////////////////////////////////////////////////////////////
// extractOunitLVN_GTE - retrieves observation units whose value to a given labelid is greater than or equal the given level value
// Input:    a representation no and level value
//////////////////////////////////////////////////////////////////////////////
LONG extractOunitLVN_GTE(BOOL fLocal, DMS_OINDEX *Ounit, long labelid, double *lvalue, long fSearchOption)
{
#define SQL_ExtractOunitLVNGTE "SELECT OINDEX.OUNITID, OINDEX.FACTORID, OINDEX.LEVELNO, OINDEX.REPRESNO,  LEVEL_N.LVALUE \
                               FROM OINDEX , LEVEL_N     \
							   WHERE OINDEX.LEVELNO = LEVEL_N.LEVELNO AND OINDEX.REPRESNO=? AND LEVEL_N.LABELID = ? AND LEVEL_N.LVALUE >= ? \
							   ORDER BY LEVEL_N.LVALUE ASC"


   PUSH(extractOunitLVNGTE);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_ExtractOunitLVNGTE);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_ExtractOunitLVNGTE);
       CODBCbindedStmt *source;

static long _repres, _labelid;
static double _lvalue, _plvalue;
static DMS_OINDEX _oindex;
static BOOL first_time=TRUE;


   if (fSearchOption==FIND_FIRST)
   {
      if (first_time)
      {
         BIND(1,_oindex.OUNITID);
		 BIND(2, _oindex.FACTORID);
		 BIND(3, _oindex.LEVELNO)
		 BIND(4, _oindex.REPRESNO);
		 BIND(5,_lvalue);
         BINDPARAM(1,_repres);
		 BINDPARAM(2, _labelid);
         BINDPARAM(3,_plvalue);

 
         first_time=FALSE;
      }
	  _plvalue = *lvalue;
	  _repres = Ounit->REPRESNO;
	  _labelid = labelid;

      local.Execute();
      central.Execute();
   }
   source = (fLocal)?&local:&central;

   if (source->Fetch()) {
      *Ounit = _oindex;
      *lvalue = _lvalue;
   }
   else if (source->NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;

   POP();

#undef SQL_ExtractOunitLVNGTE

}


//////////////////////////////////////////////////////////////////////////////
// extractOunitLVN_LT - retrieves observation units whose value to a given labelid is less than the given level value
// Input:    a representation no and level value
//////////////////////////////////////////////////////////////////////////////
LONG extractOunitLVN_LT(BOOL fLocal, DMS_OINDEX *Ounit, long labelid, double *lvalue, long fSearchOption)
{
#define SQL_ExtractOunitLVNLT "SELECT OINDEX.OUNITID, OINDEX.FACTORID, OINDEX.LEVELNO, OINDEX.REPRESNO,  LEVEL_N.LVALUE \
                               FROM OINDEX , LEVEL_N     \
							   WHERE OINDEX.LEVELNO = LEVEL_N.LEVELNO AND OINDEX.REPRESNO=? AND LEVEL_N.LABELID = ? AND LEVEL_N.LVALUE < ? \
							   ORDER BY LEVEL_N.LVALUE DESC"


   PUSH(extractOunitLVNLT);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_ExtractOunitLVNLT);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_ExtractOunitLVNLT);
       CODBCbindedStmt *source;

static long _repres, _labelid;
static double _lvalue, _plvalue;
static DMS_OINDEX _oindex;
static BOOL first_time=TRUE;


   if (fSearchOption==FIND_FIRST)
   {
      if (first_time)
      {
         BIND(1,_oindex.OUNITID);
		 BIND(2, _oindex.FACTORID);
		 BIND(3, _oindex.LEVELNO)
		 BIND(4, _oindex.REPRESNO);
		 BIND(5,_lvalue);
         BINDPARAM(1,_repres);
		 BINDPARAM(2, _labelid);
         BINDPARAM(3,_plvalue);

 
         first_time=FALSE;
      }
	  _plvalue = *lvalue;
	  _repres = Ounit->REPRESNO;
	  _labelid = labelid;

      local.Execute();
      central.Execute();
   }
   source = (fLocal)?&local:&central;

   if (source->Fetch()) {
      *Ounit = _oindex;
      *lvalue = _lvalue;
   }
   else if (source->NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;

   POP();

#undef SQL_ExtractOunitLVNLT

}

//////////////////////////////////////////////////////////////////////////////
// extractOunitLVN_LTE - retrieves observation units whose value to a given labelid is less than or equal the given level value
// Input:    a representation no and level value
//////////////////////////////////////////////////////////////////////////////
LONG extractOunitLVN_LTE(BOOL fLocal, DMS_OINDEX *Ounit, long labelid, double *lvalue, long fSearchOption)
{
#define SQL_ExtractOunitLVNLTE "SELECT OINDEX.OUNITID, OINDEX.FACTORID, OINDEX.LEVELNO, OINDEX.REPRESNO,  LEVEL_N.LVALUE \
                               FROM OINDEX , LEVEL_N     \
							   WHERE OINDEX.LEVELNO = LEVEL_N.LEVELNO AND OINDEX.REPRESNO=? AND LEVEL_N.LABELID = ? AND LEVEL_N.LVALUE <= ? \
							   ORDER BY LEVEL_N.LVALUE DESC"


   PUSH(extractOunitLVNLTE);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_ExtractOunitLVNLTE);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_ExtractOunitLVNLTE);
       CODBCbindedStmt *source;

static long _repres, _labelid;
static double _lvalue, _plvalue;
static DMS_OINDEX _oindex;
static BOOL first_time=TRUE;


   if (fSearchOption==FIND_FIRST)
   {
      if (first_time)
      {
         BIND(1,_oindex.OUNITID);
		 BIND(2, _oindex.FACTORID);
		 BIND(3, _oindex.LEVELNO)
		 BIND(4, _oindex.REPRESNO);
		 BIND(5,_lvalue);
         BINDPARAM(1,_repres);
		 BINDPARAM(2, _labelid);
         BINDPARAM(3,_plvalue);

 
         first_time=FALSE;
      }
	  _plvalue = *lvalue;
	  _repres = Ounit->REPRESNO;
	  _labelid = labelid;

      local.Execute();
      central.Execute();
   }
   source = (fLocal)?&local:&central;

   if (source->Fetch()) {
      *Ounit = _oindex;
      *lvalue = _lvalue;
   }
   else if (source->NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;

   POP();

#undef SQL_ExtractOunitLVNLTE

}


//////////////////////////////////////////////////////////////////////////////
// extractOunitLVC_LT - retrieves observation units whose level value of the given label is less than the specified value
//////////////////////////////////////////////////////////////////////////////
LONG extractOunitLVC_LT(BOOL fLocal, DMS_OINDEX *Ounit, long labelid, char *lvalue, long fSearchOption)
{
#define SQL_ExtractOunitLVC_LT "SELECT OINDEX.OUNITID, OINDEX.FACTORID, OINDEX.LEVELNO, OINDEX.REPRESNO,  LEVEL_C.LVALUE \
                               FROM OINDEX , LEVEL_C     \
							   WHERE OINDEX.LEVELNO = LEVEL_C.LEVELNO AND OINDEX.REPRESNO=? AND LEVEL_C.LABELID = ? AND LEVEL_C.LVALUE < ? \
							   ORDER BY LEVEL_C.LVALUE DESC"


   PUSH(extractOunitLVC_LT);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_ExtractOunitLVC_LT);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_ExtractOunitLVC_LT);
       CODBCbindedStmt *source;

static long _repres, _labelid;
static char _lvalue[DMS_DATA_VALUE], _plvalue[DMS_DATA_VALUE];
static DMS_OINDEX _oindex;
static BOOL first_time=TRUE;


   if (fSearchOption==FIND_FIRST)
   {
      if (first_time)
      {
         BIND(1,_oindex.OUNITID);
		 BIND(2, _oindex.FACTORID);
		 BIND(3, _oindex.LEVELNO)
		 BIND(4, _oindex.REPRESNO);
		 BINDS(5,_lvalue, DMS_DATA_VALUE -1);
         BINDPARAM(1,_repres);
		 BINDPARAM(2, _labelid);
         BINDPARAMS(3,_plvalue, DMS_DATA_VALUE -1 );

 
         first_time=FALSE;
      }
	  strcpy(_plvalue, lvalue);
	  _repres = Ounit->REPRESNO;
	  _labelid = labelid;

      local.Execute();
      central.Execute();
   }
   source = (fLocal)?&local:&central;

   if (source->Fetch()) {
      *Ounit = _oindex;
      strcpy(lvalue,  _lvalue);
   }
   else if (source->NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;

   POP();

#undef SQL_ExtractOunitLVC_LT

}


//////////////////////////////////////////////////////////////////////////////
// extractOunitLVC_GT - retrieves observation units whose level value of the given label is greater than the specified value
//////////////////////////////////////////////////////////////////////////////
LONG extractOunitLVC_GT(BOOL fLocal, DMS_OINDEX *Ounit, long labelid, char *lvalue, long fSearchOption)
{
#define SQL_ExtractOunitLVC_GT "SELECT OINDEX.OUNITID, OINDEX.FACTORID, OINDEX.LEVELNO, OINDEX.REPRESNO,  LEVEL_C.LVALUE \
                               FROM OINDEX , LEVEL_C     \
							   WHERE OINDEX.LEVELNO = LEVEL_C.LEVELNO AND OINDEX.REPRESNO=? AND LEVEL_C.LABELID = ? AND LEVEL_C.LVALUE > ? \
							   ORDER BY LEVEL_C.LVALUE ASC"


   PUSH(extractOunitLVC_GT);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_ExtractOunitLVC_GT);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_ExtractOunitLVC_GT);
       CODBCbindedStmt *source;

static long _repres, _labelid;
static char _lvalue[DMS_DATA_VALUE], _plvalue[DMS_DATA_VALUE];
static DMS_OINDEX _oindex;
static BOOL first_time=TRUE;


   if (fSearchOption==FIND_FIRST)
   {
      if (first_time)
      {
         BIND(1,_oindex.OUNITID);
		 BIND(2, _oindex.FACTORID);
		 BIND(3, _oindex.LEVELNO)
		 BIND(4, _oindex.REPRESNO);
		 BINDS(5,_lvalue, DMS_DATA_VALUE -1);
         BINDPARAM(1,_repres);
		 BINDPARAM(2, _labelid);
         BINDPARAMS(3,_plvalue, DMS_DATA_VALUE -1 );

 
         first_time=FALSE;
      }
	  strcpy(_plvalue, lvalue);
	  _repres = Ounit->REPRESNO;
	  _labelid = labelid;

      local.Execute();
      central.Execute();
   }
   source = (fLocal)?&local:&central;

   if (source->Fetch()) {
      *Ounit = _oindex;
      strcpy(lvalue,  _lvalue);
   }
   else if (source->NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;

   POP();

#undef SQL_ExtractOunitLVC_GT

}



//////////////////////////////////////////////////////////////////////////////
// extractOunitLVC_EQ - retrieves observation units whose level value of the given label is greater than the specified value
//////////////////////////////////////////////////////////////////////////////
LONG extractOunitLVC_EQ(BOOL fLocal, DMS_OINDEX *Ounit, long labelid, char *lvalue, long fSearchOption)
{
#define SQL_ExtractOunitLVC_EQ "SELECT OINDEX.OUNITID, OINDEX.FACTORID, OINDEX.LEVELNO, OINDEX.REPRESNO,  LEVEL_C.LVALUE \
                               FROM OINDEX , LEVEL_C     \
							   WHERE OINDEX.LEVELNO = LEVEL_C.LEVELNO AND OINDEX.REPRESNO=? AND LEVEL_C.LABELID = ? AND LEVEL_C.LVALUE = ?"


   PUSH(extractOunitLVC_EQ);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_ExtractOunitLVC_EQ);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_ExtractOunitLVC_EQ);
       CODBCbindedStmt *source;

static long _repres, _labelid;
static char _lvalue[DMS_DATA_VALUE], _plvalue[DMS_DATA_VALUE];
static DMS_OINDEX _oindex;
static BOOL first_time=TRUE;


   if (fSearchOption==FIND_FIRST)
   {
      if (first_time)
      {
         BIND(1,_oindex.OUNITID);
		 BIND(2, _oindex.FACTORID);
		 BIND(3, _oindex.LEVELNO)
		 BIND(4, _oindex.REPRESNO);
		 BINDS(5,_lvalue, DMS_DATA_VALUE -1);
         BINDPARAM(1,_repres);
		 BINDPARAM(2, _labelid);
         BINDPARAMS(3,_plvalue, DMS_DATA_VALUE -1 );

 
         first_time=FALSE;
      }
	  strcpy(_plvalue, lvalue);
	  _repres = Ounit->REPRESNO;
	  _labelid = labelid;

      local.Execute();
      central.Execute();
   }
   source = (fLocal)?&local:&central;

   if (source->Fetch()) {
      *Ounit = _oindex;
      strcpy(lvalue,  _lvalue);
   }
   else if (source->NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;

   POP();

#undef SQL_ExtractOunitLVC_EQ

}



/*******************************************************************************/
//////////////////////////////////////////////////////////////////////////////
//   DMS_addLevelC - adds a new character level
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_addLevelC(DMS_LEVELC* Level)
{ 
   PUSH(DMS_addLevelC);
   ret = addLevel(*Level);
   DMS_commitData;
   POP();
}



/*******************************************************************************/
//////////////////////////////////////////////////////////////////////////////
//   DMS_addLevelC - adds a new character level
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_addLevelC2(DMS_LEVELC* Level)
{
   PUSH(DMS_addLevelC2);
   ret = addLevel2(*Level);
   POP();
}


//////////////////////////////////////////////////////////////////////////////
//   DMS_addLevelN - adds a new numeric level
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_addLevelN(DMS_LEVELN* Level)
{
   PUSH(DMS_addLevelN);
   ret = addLevelN(*Level);
   DMS_commitData;
   POP();
}


//////////////////////////////////////////////////////////////////////////////
//   DMS_addLevelN - adds a new numeric level
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_addLevelN2(DMS_LEVELN* Level)
{
   PUSH(DMS_addLevelN2);
   ret = addLevelN2(*Level);
   POP();
}

//////////////////////////////////////////////////////////////////////////////
//      DMS_findLevelN: gets the factorid and levelno of a given labelid and level value;
//                      there is no find option
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_findLevelN(DMS_LEVELN* Level)
{

   PUSH(DMS_findLevelN);
   if (Level->LABELID <0) 
	   ret =  findLevelN(LOCAL,*Level);
   else {
	   ret =  findLevelN(LOCAL,*Level);
       if (ret==DMS_NO_DATA)
	      ret =  findLevelN(CENTRAL,*Level);
   }
   POP();
}


//////////////////////////////////////////////////////////////////////////////
//      DMS_findLevelN: gets the factorid and levelno of a given labelid and level value;
//                      there is no find option
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_findLevelNo(long factorid, long levelno)
{

   PUSH(DMS_findLevelNo);
   if (factorid <0) 
	   ret =  findLevelNo(LOCAL,factorid, levelno);
   else {
	   ret =  findLevelNo(LOCAL,factorid, levelno);
       if (ret==DMS_NO_DATA)
	      ret =  findLevelNo(CENTRAL,factorid, levelno);
   }
   POP();
}


//////////////////////////////////////////////////////////////////////////////
//      DMS_findLevelC: gets the factorid and levelno of a given labelid and level value;
//                      there is no find option
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_findLevelC(DMS_LEVELC* Level)
{

   PUSH(DMS_findLevelC)	
   if (Level->LABELID <0) 
	   ret =  findLevel(LOCAL,*Level);
   else {
	   ret =  findLevel(LOCAL,*Level);
	   if (ret == DMS_NO_DATA)
	      ret =  findLevel(CENTRAL,*Level);
   }
   POP();
}



//////////////////////////////////////////////////////////////////////////////
//      DMS_findLabelN: finds if a record exists
//                     given the labelid, factorid, levelno and numeric level value
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_findLabelN(DMS_LEVELN* Level)
{
   PUSH(DMS_findLabelN);
   if ((Level->LABELID <0) || (Level->LEVELNO <0))
	   ret =  findLabelN(LOCAL,*Level);
   else {
	   ret =  findLabelN(LOCAL,*Level);
       if (ret==DMS_NO_DATA)
	      ret =  findLabelN(CENTRAL,*Level);
   }
   POP();
}

//////////////////////////////////////////////////////////////////////////////
// DMS_findLabelC: finds if a record exists
//                given the labelid, factorid, levelno and numeric level value
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_findLabelC(DMS_LEVELC* Level)
{
   	 
   PUSH(DMS_findLabelC);
   if ((Level->LABELID <0) || (Level->LEVELNO <0))
	   ret =  findLabel(LOCAL,*Level);
   else {
	   ret =  findLabel(LOCAL,*Level);
	   if (ret == DMS_NO_DATA)
	      ret =  findLabel(CENTRAL,*Level);
   }
   POP();
}


//////////////////////////////////////////////////////////////////////////////
// DMS_getLevelC - gets the factorid and level value of a given labelid and levelno;
//                there is no find option since there should only be one value for a labelid & levelno
//        input : plevc.labelid, plevc.levelno
//        output: plevc.factorid, plevc.lvalue
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getLevelC(DMS_LEVELC* Level)
{
   	 
   PUSH(DMS_getLevelC);
   if ((Level->LABELID <0) || (Level->LEVELNO <0))
	   ret =  getLevel(LOCAL,*Level);
   else {
	   ret =  getLevel(LOCAL,*Level);
	   if (ret == DMS_NO_DATA)
	      ret =  getLevel(CENTRAL,*Level);
   }
   POP();
}


//////////////////////////////////////////////////////////////////////////////
// DMS_getLevelN - gets the factorid and level value of a given labelid and levelno;
//                there is no find option since there should only be one value for a labelid & levelno
//        input : plevn.labelid, plevn.levelno
//        output: plevn.factorid, plevn.lvalue
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getLevelN(DMS_LEVELN* Level)
{
   PUSH(DMS_getLevelN);
   if ((Level->LABELID <0) || (Level->LEVELNO <0))
	   ret =  getLevelN(LOCAL,*Level);
   else {
	   ret =  getLevelN(LOCAL,*Level);
       if (ret==DMS_NO_DATA)
	      ret =  getLevelN(CENTRAL,*Level);
   }
   POP();
}


//////////////////////////////////////////////////////////////////////////////
// DMS_getLabelC - gets all label values of a given factor level
//        input : Level.factorid, Level.levelno
//        output: Level.labelid, Level.lvalue
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getLabelC(DMS_LEVELC* Level, LONG fopt)
{
   	 
   static BOOL fLocal=TRUE;
   static DMS_LEVELC lev;
   PUSH(DMS_findStudy);
   if (fopt==FIND_FIRST){
       lev = *Level;
       fLocal=TRUE;
   }
   if (fLocal){
   	     ret =  getLabelLevel(LOCAL,lev, fopt);
         if (ret!=GMS_SUCCESS){
         fLocal = FALSE; 
         ret =  getLabelLevel(CENTRAL,lev,FIND_FIRST);
      }
   }
   else
      ret =  getLabelLevel(CENTRAL,lev,fopt);
   *Level = lev;
   POP();
}


//////////////////////////////////////////////////////////////////////////////
// DMS_getLabelN - gets all label values of a given factor level
//        input : plevn.factorid, plevn.levelno
//        output: plevn.labelid, plevn.lvalue
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getLabelN(DMS_LEVELN* Level, LONG fopt)
{
   PUSH(DMS_getLabelN);
   if ((Level->LABELID <0) || (Level->LEVELNO <0)) {
	   ret =  getLabelN(LOCAL,*Level, fopt);
       if (ret==DMS_NO_DATA)
	      ret =  getLabelN(CENTRAL,*Level, FIND_FIRST);
   }
   else {
	   ret =  getLabelN(CENTRAL,*Level, fopt);
   }
   POP();
}


//////////////////////////////////////////////////////////////////////////////
// DMS_getAllLevelN - gets all level values of a given label
//        input : plevn.labelid
//        output: plevn.factorid, plevn.levelno, plevn.lvalue
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getAllLevelN(DMS_LEVELN* Level, LONG fopt)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_getAllLevelN);

   if (fopt==FIND_FIRST)
   {
      fLocal=TRUE;
   }
   if (fLocal){
      ret=getAllLevelN(LOCAL,*Level,fopt);
      if (ret!=DMS_SUCCESS){
         fLocal = FALSE; 
         ret=getAllLevelN(CENTRAL,*Level,FIND_NEXT);
      }
   }
   else
      ret=getAllLevelN(CENTRAL,*Level,fopt);
   POP();

}


//////////////////////////////////////////////////////////////////////////////
// DMS_getAllLevelC - gets all level values of a given label
//        input : plevc.labelid
//        output: plevc.factorid, plevc.levelno, plevc.lvalue
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getAllLevelC(DMS_LEVELC* Level, LONG fopt)
{
   static BOOL fLocal=TRUE;
   PUSH(DMS_getAllLevelC);
   if (fopt==FIND_FIRST)
   {
      fLocal=TRUE;
   }
   if (fLocal){
      ret=getAllLevel(LOCAL,*Level,fopt);
      if (ret!=DMS_SUCCESS){
         fLocal = FALSE; 
         ret=getAllLevel(CENTRAL,*Level,FIND_NEXT);
      }
   }
   else
      ret=getAllLevel(CENTRAL,*Level,fopt);
   POP();

}


//////////////////////////////////////////////////////////////////////////////
// DMS_getAllLabelN - gets all label values of a given factor
//        input : plevn.factorid
//        output: plevn.labelid, plevn.levelno, plevn.lvalue
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getAllLabelN(DMS_LEVELN* Level, LONG fopt)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_getAllLabelN);

   if (fopt==FIND_FIRST)
   {
      fLocal=TRUE;
   }
   if (fLocal){
      ret=getAllLabelN(LOCAL,*Level,fopt);
      if (ret!=DMS_SUCCESS){
         fLocal = FALSE; 
         ret=getAllLabelN(CENTRAL,*Level,FIND_NEXT);
      }
   }
   else
      ret=getAllLabelN(CENTRAL,*Level,fopt);
   POP();

}


//////////////////////////////////////////////////////////////////////////////
// DMS_getAllLabelC - gets all level values of a given label
//        input : plevc.factorid
//        output: plevc.labelid, plevc.levelno, plevc.lvalue
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getAllLabelC(DMS_LEVELC* Level, LONG fopt)
{
   static BOOL fLocal=TRUE;
   PUSH(DMS_getAllLabelC);
   if (fopt==FIND_FIRST)
   {
      fLocal=TRUE;
   }
   if (fLocal){
      ret=getAllLabel(LOCAL,*Level,fopt);
      if (ret!=DMS_SUCCESS){
         fLocal = FALSE; 
         ret=getAllLabel(CENTRAL,*Level,FIND_NEXT);
      }
   }
   else
      ret=getAllLabel(CENTRAL,*Level,fopt);
   POP();

}


//////////////////////////////////////////////////////////////////////////////
// DMS_getAllLabel - gets all level values of a given factorid
//        input : Level.factorid
//        output: Level.labelid, Level.levelno, Level.lvalue
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getAllLabel(DMS_LEVELC* Level, LONG fopt)
{
   static BOOL fLocal=TRUE;
   PUSH(DMS_getAllLabelC);
   if (fopt==FIND_FIRST)
   {
      fLocal=TRUE;
   }
   if (fLocal){
      ret=getAllLabelNC(LOCAL,*Level,fopt);
      if (ret!=DMS_SUCCESS){
         fLocal = FALSE; 
         ret=getAllLabelNC(CENTRAL,*Level,FIND_NEXT);
      }
   }
   else
      ret=getAllLabelNC(CENTRAL,*Level,fopt);
   POP();

}


LONG DLL_INTERFACE DMS_getOunitLevelC(DMS_LEVELC *LevC, long *ounitid, long fopt)
//Parameter: Labelid
{
	PUSH(DMS_getOunitLevelC); 
	if (LevC->LABELID<0) 
		ret=getOunitLevelC(LOCAL,LevC,ounitid,fopt);
	else
		ret=getOunitLevelC(CENTRAL,LevC,ounitid,fopt);
	POP();
}

LONG DLL_INTERFACE DMS_getOunitLevelN(DMS_LEVELN *LevN, long *ounitid, long fopt)
//Parameter: Labelid
{

   PUSH(DMS_getOunitLevelN);
   if (LevN->LABELID<0) 
	   ret=getOunitLevelN(LOCAL,LevN,ounitid, fopt);
   else
 	 ret=getOunitLevelN(CENTRAL,LevN,ounitid, fopt);
   POP();
}





LONG DLL_INTERFACE DMS_extractOunitByLNVal(DMS_OINDEX *Ounit, long labelid, double *lvalue,  int oprtr, int fopt)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_extractOunitDN);
	if (fopt == FIND_FIRST) {
	    fLocal = TRUE;
	}

    if (oprtr == DMS_EQ ) {
   	   if (fLocal) {
	      ret=extractOunitLVN_EQ(LOCAL,Ounit,labelid,lvalue,fopt);
	      if (ret == DMS_NO_DATA) {
            fLocal = FALSE;
	        ret=extractOunitLVN_EQ(CENTRAL,Ounit,labelid,lvalue,fopt);
		  }
	   }
       else 
	      ret=extractOunitLVN_EQ(CENTRAL,Ounit,labelid,lvalue,fopt);
	}

    if (oprtr == DMS_LT ) {
   	   if (fLocal) {
	      ret=extractOunitLVN_LT(LOCAL,Ounit,labelid,lvalue,fopt);
	      if (ret == DMS_NO_DATA) {
            fLocal = FALSE;
	        ret=extractOunitLVN_LT(CENTRAL,Ounit,labelid,lvalue,FIND_FIRST);
		  }
	   }
       else 
	      ret=extractOunitLVN_LT(CENTRAL,Ounit,labelid,lvalue,fopt);
	}

	if (oprtr == DMS_GT ) {
   	   if (fLocal) {
	      ret=extractOunitLVN_GT(LOCAL,Ounit,labelid,lvalue,fopt);
	      if (ret == DMS_NO_DATA) {
            fLocal = FALSE;
	        ret=extractOunitLVN_GT(CENTRAL,Ounit,labelid,lvalue,FIND_FIRST);
		  }
	   }
       else 
	      ret=extractOunitLVN_GT(CENTRAL,Ounit,labelid,lvalue,fopt);
	}

    if (oprtr == DMS_GTE) {
   	   if (fLocal) {
	      ret=extractOunitLVN_GTE(LOCAL,Ounit,labelid,lvalue,fopt);
	      if (ret == DMS_NO_DATA) {
            fLocal = FALSE;
	        ret=extractOunitLVN_GTE(CENTRAL,Ounit,labelid,lvalue,FIND_FIRST);
		  }
	   }
       else 
	      ret=extractOunitLVN_GTE(CENTRAL,Ounit,labelid,lvalue,fopt);
	}

	if (oprtr == DMS_LTE) {
	   	   if (fLocal) {
	      ret=extractOunitLVN_LTE(LOCAL,Ounit,labelid,lvalue,fopt);
	      if (ret == DMS_NO_DATA) {
            fLocal = FALSE;
	        ret=extractOunitLVN_LTE(CENTRAL,Ounit,labelid,lvalue,FIND_FIRST);
		  }
	   }
       else 
	      ret=extractOunitLVN_LTE(CENTRAL,Ounit,labelid,lvalue,fopt);
	}


	POP();
}



//--------------------------------------------------------------------------
 //--------------------------------------------------------------------------
LONG DLL_INTERFACE DMS_extractOunitByLCVal(DMS_OINDEX *Ounit, long labelid, char *lvalue,  int oprtr, int fopt)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_extractOunitLCVal);
	if (fopt == FIND_FIRST) {
	    fLocal = TRUE;
	}

    if (oprtr == DMS_EQ ) {
   	   if (fLocal) {
	      ret=extractOunitLVC_EQ(LOCAL,Ounit,labelid,lvalue,fopt);
	      if (ret == DMS_NO_DATA) {
            fLocal = FALSE;
	        ret=extractOunitLVC_EQ(CENTRAL,Ounit,labelid,lvalue,fopt);
		  }
	   }
       else 
	      ret=extractOunitLVC_EQ(CENTRAL,Ounit,labelid,lvalue,fopt);
	}

    if (oprtr == DMS_LT ) {
   	   if (fLocal) {
	      ret=extractOunitLVC_LT(LOCAL,Ounit,labelid,lvalue,fopt);
	      if (ret == DMS_NO_DATA) {
            fLocal = FALSE;
	        ret=extractOunitLVC_LT(CENTRAL,Ounit,labelid,lvalue,FIND_FIRST);
		  }
	   }
       else 
	      ret=extractOunitLVC_LT(CENTRAL,Ounit,labelid,lvalue,fopt);
	}

	if (oprtr == DMS_GT ) {
   	   if (fLocal) {
	      ret=extractOunitLVC_GT(LOCAL,Ounit,labelid,lvalue,fopt);
	      if (ret == DMS_NO_DATA) {
            fLocal = FALSE;
	        ret=extractOunitLVC_GT(CENTRAL,Ounit,labelid,lvalue,FIND_FIRST);
		  }
	   }
       else 
	      ret=extractOunitLVC_GT(CENTRAL,Ounit,labelid,lvalue,fopt);
	}



	POP();

}




//////////////////////////////////////////////////////////////////////////////
//added by WVC
// modified by AMP (2005/12/5):  Assign different SQL statement depending on the database
//***** Gets all level values of the conditions belonging to one representation *****/
//Input:  data.REPRESNO, data.FACTORID
//Output: condition level record
//Called by DMS_getCondLevel
//////////////////////////////////////////////////////////////////////////////
LONG getCondLevel(BOOL fLocal, DMS_CONDLEVEL *data, int fOpt)
{
	#define SQL_GETCONDLEVEL1 \
		"SELECT REPRESNO, LEVEL_N.FACTORID, LABELID, LVALUE \
		FROM EFFECT, LEVEL_N \
		WHERE (((EFFECT.FACTORID)=LEVEL_N.FACTORID) AND ((EFFECT.REPRESNO)=?) AND ((LEVEL_N.FACTORID)=?)) \
		UNION \
		SELECT REPRESNO, LEVEL_C.FACTORID, LABELID, LVALUE \
		FROM EFFECT, LEVEL_C \
		WHERE (((EFFECT.FACTORID)=LEVEL_C.FACTORID) AND ((EFFECT.REPRESNO)=?) AND ((LEVEL_C.FACTORID)=?)) \
		ORDER BY LABELID ASC"
	#define SQL_GETCONDLEVEL_Oracle \
		"SELECT E.REPRESNO, L.FACTORID, L.LABELID, {fn CONVERT(L.LVALUE,SQL_VARCHAR)}  \
		FROM EFFECT E, LEVEL_N L\
		WHERE (((E.FACTORID)=L.FACTORID) AND ((E.REPRESNO)=?) AND ((L.FACTORID)=?)) \
		UNION \
		(SELECT E.REPRESNO, L.FACTORID, L.LABELID, L.LVALUE \
		FROM EFFECT E, LEVEL_C L \
		WHERE (((E.FACTORID)=L.FACTORID) AND ((E.REPRESNO)=?) AND ((L.FACTORID)=?))) \
		ORDER BY LABELID ASC"	
	#define SQL_GETCONDLEVEL_Postgres \
	    "SELECT REPRESNO, LEVEL_N.FACTORID, LABELID, CAST(LVALUE as char)  \
		FROM EFFECT, LEVEL_N \
		WHERE (((EFFECT.FACTORID)=LEVEL_N.FACTORID) AND ((EFFECT.REPRESNO)=?) AND ((LEVEL_N.FACTORID)=?)) \
		UNION \
		SELECT REPRESNO, LEVEL_C.FACTORID, LABELID, LVALUE \
		FROM EFFECT, LEVEL_C \
		WHERE (((EFFECT.FACTORID)=LEVEL_C.FACTORID) AND ((EFFECT.REPRESNO)=?) AND ((LEVEL_C.FACTORID)=?)) \
		ORDER BY LABELID ASC"	

    #define SQL_GETCONDLEVEL2 \
		"SELECT REPRESNO, LEVEL_N.FACTORID, LEVEL_N.LABELID, LVALUE \
		FROM EFFECT, LEVEL_N \
		WHERE (((EFFECT.FACTORID)=LEVEL_N.FACTORID) AND ((EFFECT.REPRESNO)=?) AND ((LEVEL_N.FACTORID)=?)) \
		UNION \
		SELECT REPRESNO, LEVEL_C.FACTORID, LEVEL_C.LABELID, LVALUE \
		FROM EFFECT, LEVEL_C \
		WHERE (((EFFECT.FACTORID)=LEVEL_C.FACTORID) AND ((EFFECT.REPRESNO)=?) AND ((LEVEL_C.FACTORID)=?)) \
		ORDER BY LABELID DESC"

	#define SQL_GETCONDLEVEL2_Oracle \
		"SELECT E.REPRESNO, L.FACTORID, L.LABELID, {fn CONVERT(L.LVALUE,SQL_VARCHAR)}  \
		FROM EFFECT E, LEVEL_N L\
		WHERE (((E.FACTORID)=L.FACTORID) AND ((E.REPRESNO)=?) AND ((L.FACTORID)=?)) \
		UNION \
		(SELECT E.REPRESNO, L.FACTORID, L.LABELID, L.LVALUE \
		FROM EFFECT E, LEVEL_C L \
		WHERE (((E.FACTORID)=L.FACTORID) AND ((E.REPRESNO)=?) AND ((L.FACTORID)=?))) \
		ORDER BY LABELID DESC"	
	#define SQL_GETCONDLEVEL2_Postgres \
	    "SELECT REPRESNO, LEVEL_N.FACTORID, LABELID, CAST(LVALUE as char)  \
		FROM EFFECT, LEVEL_N \
		WHERE (((EFFECT.FACTORID)=LEVEL_N.FACTORID) AND ((EFFECT.REPRESNO)=?) AND ((LEVEL_N.FACTORID)=?)) \
		UNION \
		SELECT REPRESNO, LEVEL_C.FACTORID, LABELID, LVALUE \
		FROM EFFECT, LEVEL_C \
		WHERE (((EFFECT.FACTORID)=LEVEL_C.FACTORID) AND ((EFFECT.REPRESNO)=?) AND ((LEVEL_C.FACTORID)=?)) \
		ORDER BY LABELID DESC"	


	LONG ret = DMS_SUCCESS;
    LPCSTR szDB;	
    static string central_sql,local_sql;
	static CODBCbindedStmt &central = _centralDMS->BindedStmt();
	static CODBCbindedStmt &local = _localDMS->BindedStmt();
	CODBCbindedStmt *source;
	
	static DMS_CONDLEVEL _data;
	static BOOL first_time = TRUE;
	
	if (fOpt == FIND_FIRST) {
		if (first_time) {

	        szDB = _localDMS->DatabaseName();
	        if (strncmp(szDB, "Oracle",6)==0 ) {
	           local_sql=SQL_GETCONDLEVEL2_Oracle;
			}
		    else if (strcmp(szDB, "PostgreSQL")==0 )
	           local_sql=SQL_GETCONDLEVEL2_Postgres;
			else
			   local_sql=SQL_GETCONDLEVEL2;

			szDB = _centralDMS->DatabaseName();
	        if (strncmp(szDB, "Oracle",6)==0 ) {
	           central_sql=SQL_GETCONDLEVEL_Oracle;
			}
		    else if (strcmp(szDB, "PostgreSQL")==0 )
	           central_sql=SQL_GETCONDLEVEL_Postgres;
			else
			   central_sql=SQL_GETCONDLEVEL1;
   
			central.SetSQLstr(central_sql.c_str());
			local.SetSQLstr(local_sql.c_str());


			BINDPARAM(1, _data.REPRESNO);
			BINDPARAM(2, _data.FACTORID);
			BINDPARAM(3, _data.REPRESNO);
			BINDPARAM(4, _data.FACTORID);
			
			BIND(1, _data.REPRESNO);
			BIND(2, _data.FACTORID);
			BIND(3, _data.LABELID);
			BINDS(4, _data.LVALUE, DMS_DATA_VALUE);
			
			first_time = FALSE;
		}
		_data.REPRESNO = data->REPRESNO;
		_data.FACTORID = data->FACTORID;
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
	
	#undef SQL_GETCONDLEVEL1
	#undef SQL_GETCONDLEVEL_Oracle
	#undef SQL_GETCONDLEVEL_Postgres
	#undef SQL_GETCONDLEVEL2
	#undef SQL_GETCONDLEVEL2_Oracle
	#undef SQL_GETCONDLEVEL2_Postgres
}

//////////////////////////////////////////////////////////////////////////////
//added by WVC
//***** Gets all level values of the conditions belonging to one representation *****/
//Input:  data.REPRESNO, data.FACTORID
//Output: condition level record
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getCondLevel(DMS_CONDLEVEL *data, int fOpt)
{
	static BOOL fLocal = TRUE;
	
	PUSH(DMS_getCondLevel);
	
	if (fOpt == FIND_FIRST) {
		fLocal = TRUE;
	}
	if (fLocal) {
		ret = getCondLevel(LOCAL, data, fOpt);
		if (ret != DMS_SUCCESS){
			fLocal = FALSE;
			ret = getCondLevel(CENTRAL, data, FIND_NEXT);
		}
	}
	else
		ret = getCondLevel(CENTRAL, data, fOpt);
	
	POP();
	
}

//////////////////////////////////////////////////////////////////////////////
//added by WVC
//***** Gets all level values belonging to one observation unit *****/
//Input:  data.REPRESNO, data.OUNITID
//Output: entire observation unit levels record
//Called by DMS_getOunitLevel
//////////////////////////////////////////////////////////////////////////////
LONG getOunitLevel(BOOL fLocal, DMS_OUNITLEVEL *data, int fOpt)
{
	#define SQL_GETOUNITLEVEL1 \
		"SELECT O.REPRESNO, O.OUNITID, L.LABELID, L.FACTORID, L.LEVELNO,  L.LVALUE \
		FROM OINDEX O, LEVEL_N L\
		WHERE (((O.LEVELNO)=L.LEVELNO) AND ((O.FACTORID)=L.FACTORID) \
		AND ((O.REPRESNO)=?) AND ((O.OUNITID)=?)) \
		UNION \
		(SELECT O.REPRESNO, O.OUNITID, L.LABELID, L.FACTORID, L.LEVELNO, L.LVALUE \
		FROM OINDEX O, LEVEL_C L \
		WHERE (((O.LEVELNO)=L.LEVELNO) AND ((O.FACTORID)=L.FACTORID) \
		AND ((O.REPRESNO)=?) AND ((O.OUNITID)=?))) \
		ORDER BY LABELID ASC"

	#define SQL_GETOUNITLEVEL1_Oracle \
		"SELECT O.REPRESNO, O.OUNITID, L.LABELID, L.FACTORID, L.LEVELNO,  {fn CONVERT(L.LVALUE,SQL_VARCHAR)} \
		FROM OINDEX O, LEVEL_N L\
		WHERE (((O.LEVELNO)=L.LEVELNO) AND ((O.FACTORID)=L.FACTORID) \
		AND ((O.REPRESNO)=?) AND ((O.OUNITID)=?)) \
		UNION \
		(SELECT O.REPRESNO, O.OUNITID, L.LABELID, L.FACTORID, L.LEVELNO, L.LVALUE \
		FROM OINDEX O, LEVEL_C L \
		WHERE (((O.LEVELNO)=L.LEVELNO) AND ((O.FACTORID)=L.FACTORID) \
		AND ((O.REPRESNO)=?) AND ((O.OUNITID)=?))) \
		ORDER BY LABELID ASC"

	#define SQL_GETOUNITLEVEL1_MySQL \
		"SELECT REPRESNO, OUNITID, LABELID, LEVEL_N.FACTORID, LEVEL_N.LEVELNO,  CAST(LVALUE as CHAR) \
		FROM OINDEX, LEVEL_N \
		WHERE (((OINDEX.LEVELNO)=LEVEL_N.LEVELNO) AND ((OINDEX.FACTORID)=LEVEL_N.FACTORID) \
		AND ((OINDEX.REPRESNO)=?) AND ((OINDEX.OUNITID)=?)) \
		UNION \
		(SELECT REPRESNO, OUNITID, LABELID, LEVEL_C.FACTORID, LEVEL_C.LEVELNO, LVALUE \
		FROM OINDEX, LEVEL_C \
		WHERE (((OINDEX.LEVELNO)=LEVEL_C.LEVELNO) AND ((OINDEX.FACTORID)=LEVEL_C.FACTORID) \
        AND ((OINDEX.REPRESNO)=?) AND ((OINDEX.OUNITID)=?))) \
		ORDER BY LABELID ASC"
		
	#define SQL_GETOUNITLEVEL2 \
		"SELECT O.REPRESNO, O.OUNITID, L.LABELID, L.FACTORID, L.LEVELNO,  {fn CONVERT(L.LVALUE,SQL_VARCHAR)} \
		FROM OINDEX O, LEVEL_N L \
		WHERE (((O.LEVELNO)=L.LEVELNO) AND ((O.FACTORID)=L.FACTORID) \
		AND ((O.REPRESNO)=?) AND ((O.OUNITID)=?)) \
		UNION \
		(SELECT O.REPRESNO, O.OUNITID, L.LABELID, L.FACTORID, L.LEVELNO, L.LVALUE \
		FROM OINDEX O, LEVEL_C L\
		WHERE (((O.LEVELNO)=L.LEVELNO) AND ((O.FACTORID)=L.FACTORID) \
		AND ((O.REPRESNO)=?) AND ((O.OUNITID)=?))) \
		ORDER BY LABELID DESC"


	#define SQL_GETOUNITLEVEL2_Oracle \
		"SELECT O.REPRESNO, O.OUNITID, L.LABELID, L.FACTORID, L.LEVELNO,  {fn CONVERT(L.LVALUE,SQL_VARCHAR)} \
		FROM OINDEX O, LEVEL_N L \
		WHERE (((O.LEVELNO)=L.LEVELNO) AND ((O.FACTORID)=L.FACTORID) \
		AND ((O.REPRESNO)=?) AND ((O.OUNITID)=?)) \
		UNION \
		(SELECT O.REPRESNO, O.OUNITID, L.LABELID, L.FACTORID, L.LEVELNO, L.LVALUE \
		FROM OINDEX O, LEVEL_C L\
		WHERE (((O.LEVELNO)=L.LEVELNO) AND ((O.FACTORID)=L.FACTORID) \
		AND ((O.REPRESNO)=?) AND ((O.OUNITID)=?))) \
		ORDER BY LABELID DESC"

	#define SQL_GETOUNITLEVEL2_MySQL \
		"SELECT REPRESNO, OUNITID, LABELID, LEVEL_N.FACTORID, LEVEL_N.LEVELNO, CAST(LVALUE as CHAR)  \
		FROM OINDEX, LEVEL_N \
		WHERE (((OINDEX.LEVELNO)=LEVEL_N.LEVELNO) AND ((OINDEX.FACTORID)=LEVEL_N.FACTORID) \
		AND ((OINDEX.REPRESNO)=?) AND ((OINDEX.OUNITID)=?)) \
		UNION \
		SELECT REPRESNO, OUNITID, LABELID, LEVEL_C.FACTORID, LEVEL_C.LEVELNO, LVALUE \
		FROM OINDEX, LEVEL_C \
		WHERE (((OINDEX.LEVELNO)=LEVEL_C.LEVELNO) AND ((OINDEX.FACTORID)=LEVEL_C.FACTORID) \
		AND ((OINDEX.REPRESNO)=?) AND ((OINDEX.OUNITID)=?)) \
		ORDER BY LABELID DESC"
			
	PUSH(getOunitLevel);

//	LONG ret = DMS_SUCCESS;
	
	static CODBCbindedStmt &central = _centralDMS->BindedStmt();
	static CODBCbindedStmt &local = _localDMS->BindedStmt();
	CODBCbindedStmt *source;
	LPCSTR szDB;
    static string central_sql,local_sql;
	static DMS_OUNITLEVEL _data;
	static BOOL first_time = TRUE;
    CHAR error_msg[SQL_MAX_MESSAGE_LENGTH+256]	;

	if (fOpt == FIND_FIRST) {
		if (first_time) {

	        szDB = _localDMS->DatabaseName();
			if ((strcmp(szDB, "MySQL")==0) || (strcmp(szDB, "PostgreSQL")==0) )  {
				local_sql=SQL_GETOUNITLEVEL2_MySQL;
                //sprintf((LPSTR)error_msg ,"\n%s",local_sql.c_str);
                //MessageBox(NULL,(LPSTR)error_msg , "GETOUNIT", MB_OK+MB_ICONINFORMATION);
			}
		   else {
			 if (strncmp(szDB, "Oracle",6)==0 ) 
				local_sql=SQL_GETOUNITLEVEL2_Oracle;
			 else  local_sql=SQL_GETOUNITLEVEL2;
		   }
		   
		   szDB = _centralDMS->DatabaseName();
		   if ((strcmp(szDB, "MySQL")==0) || (strcmp(szDB, "PostgreSQL")==0) )  {
			  central_sql=SQL_GETOUNITLEVEL1_MySQL;
			}
	        else if (strncmp(szDB, "Oracle",6)==0 ) {
              central_sql=SQL_GETOUNITLEVEL1_Oracle;
			}
			else
				central_sql= SQL_GETOUNITLEVEL1;
 
            central.SetSQLstr(central_sql.c_str());
            local.SetSQLstr(local_sql.c_str());


			BINDPARAM(1, _data.REPRESNO);
			BINDPARAM(2, _data.OUNITID);
			BINDPARAM(3, _data.REPRESNO);
			BINDPARAM(4, _data.OUNITID);
			
			BIND(1, _data.REPRESNO);
			BIND(2, _data.OUNITID);
			BIND(3, _data.LABELID);
			BIND(4, _data.FACTORID);
			BIND(5, _data.LEVELNO);
			BINDS(6, _data.LVALUE, DMS_DATA_VALUE);
			
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

	POP();
//	return ret;
	
	#undef SQL_GETOUNITLEVEL1
	#undef SQL_GETOUNITLEVEL2
	#undef SQL_GETOUNITLEVEL1_MySQL
	#undef SQL_GETOUNITLEVEL2_MySQL
	#undef SQL_GETOUNITLEVEL1_Oracle
	#undef SQL_GETOUNITLEVEL2_Oracle
}

//////////////////////////////////////////////////////////////////////////////
//added by WVC
//***** Gets all level values belonging to one observation unit *****/
//Input:  data.REPRESNO, data.OUNITID
//Output: entire observation unit levels record
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getOunitLevel(DMS_OUNITLEVEL *data, int fOpt)
{
	static BOOL fLocal = TRUE;
	
	PUSH(DMS_getOunitLevel);
	
	if (fOpt == FIND_FIRST) {
		fLocal = TRUE;
	}
	if (fLocal) {
		ret = getOunitLevel(LOCAL, data, fOpt);
		if (ret != DMS_SUCCESS){
			fLocal = FALSE;
			ret = getOunitLevel(CENTRAL, data, FIND_NEXT);
		}
	}
	else
		ret = getOunitLevel(CENTRAL, data, fOpt);
	
	POP();
	
}



//////////////////////////////////////////////////////////////////////////////
//added by WVC
//Modified  AMP(2005/12) : Modified the SQL for Oracle
//          AMP(2008/05) : modified the SQL for mySQL and PostGres
//***** Finds the entry corresponding to the label ID and level value *****/
//Input:  data.LABELID, data.LVALUE
//Output: entire level record
//Called by DMS_findPFLevel
//////////////////////////////////////////////////////////////////////////////
LONG findPFLevel(BOOL fLocal, DMS_LEVELC &data)
{
	#define SQL_FINDPFLEVEL \
		"SELECT LABELID, FACTORID, LEVELNO,LVALUE \
		FROM LEVEL_N \
WHERE (((LABELID)=?) AND ((LVALUE)=cstr(?))) \
		UNION \
		SELECT LABELID, FACTORID, LEVELNO, LVALUE \
		FROM LEVEL_C \
		WHERE (((LABELID)=?) AND ((LVALUE)=?))"

	//		WHERE (((LABELID)=?) AND ((LVALUE)=?)) \


	#define SQL_FINDPFLEVEL_Oracle \
		"SELECT LABELID, FACTORID, LEVELNO,{fn CONVERT(LVALUE,SQL_VARCHAR)}\
		FROM LEVEL_N \
		WHERE (((LABELID)=?) AND (({fn CONVERT(LVALUE,SQL_VARCHAR)})=?)) \
		UNION \
		SELECT LABELID, FACTORID, LEVELNO, LVALUE \
		FROM LEVEL_C \
		WHERE (((LABELID)=?) AND ((LVALUE)=?))"

	#define SQL_FINDPFLEVEL_MySQL \
		"SELECT LABELID, FACTORID, LEVELNO, CAST(LVALUE as CHAR) \
		FROM LEVEL_N \
		WHERE (((LABELID)=?) AND ((CAST(LVALUE as CHAR))=?)) \
		UNION \
		SELECT LABELID, FACTORID, LEVELNO, LVALUE \
		FROM LEVEL_C \
		WHERE (((LABELID)=?) AND ((LVALUE)=?))"


		LONG ret = DMS_SUCCESS;
	
	static CODBCbindedStmt &central = _centralDMS->BindedStmt(SQL_FINDPFLEVEL);
	static CODBCbindedStmt &local = _localDMS->BindedStmt(SQL_FINDPFLEVEL);
	CODBCbindedStmt *source;
	
	static DMS_LEVELC _data;
	static BOOL first_time = TRUE;
	
	LPCSTR szDB;
    static string central_sql,local_sql;

	if (first_time) {
	   szDB = _localDMS->DatabaseName();
	   if ((strcmp(szDB, "MySQL")==0) || (strcmp(szDB, "PostgreSQL")==0) )  {
		   local_sql=SQL_FINDPFLEVEL_MySQL;
	   }
	   else if (strncmp(szDB, "Oracle",6)==0 ) {
		   local_sql=SQL_FINDPFLEVEL_Oracle;
	   }
	   else local_sql=SQL_FINDPFLEVEL;

	   szDB = _centralDMS->DatabaseName();
	   if ((strcmp(szDB, "MySQL")==0) || (strcmp(szDB, "PostgreSQL")==0) )  {
		  central_sql=SQL_FINDPFLEVEL_MySQL;
	   }
	   else if (strncmp(szDB, "Oracle",6)==0 ) {
		  central_sql=SQL_FINDPFLEVEL_Oracle;
	   }
	   else central_sql=SQL_FINDPFLEVEL;
 
       local.SetSQLstr(local_sql.c_str());
       central.SetSQLstr(central_sql.c_str());

		BINDPARAM(1, _data.LABELID);
		BINDPARAMS(2, _data.LVALUE, DMS_DATA_VALUE-1);
		BINDPARAM(3, _data.LABELID);
		BINDPARAMS(4, _data.LVALUE, DMS_DATA_VALUE-1);
		
		BIND(1, _data.LABELID);
		BIND(2, _data.FACTORID);
		BIND(3, _data.LEVELNO);
		BINDS(4, _data.LVALUE, DMS_DATA_VALUE);
		
		first_time = FALSE;
	}
	_data = data;
	local.Execute();
	central.Execute();
	
	source = (fLocal) ? &local : &central;
	if (source->Fetch()) {
		ret = GMS_SUCCESS;
		data = _data;
	}
	else if (source->NoData())
		ret = DMS_NO_DATA;
	else 
		ret = DMS_ERROR;
	
	return ret;
	
	#undef SQL_FINDPFLEVEL
	#undef SQL_FINDPFLEVEL_MySQL
}

//////////////////////////////////////////////////////////////////////////////
//added by WVC
//***** Finds the entry corresponding to the label ID and level value *****/
//Input:  data.LABELID, data.LVALUE
//Output: entire level record
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_findPFLevel(DMS_LEVELC *data)
{
	PUSH(DMS_findPFLevel);
	
	if (data->LABELID < 0){
		ret = findPFLevel(LOCAL, *data);
		if (ret != DMS_SUCCESS){
			ret = findPFLevel(CENTRAL, *data);
		}
	}
	else
		ret = findPFLevel(CENTRAL, *data);
	
	POP();
	
}

//////////////////////////////////////////////////////////////////////////////
//added by WVC
//***** Finds the entry corresponding to the label ID, level no. and level value *****/
//Input:  data.LABELID, data.LEVELNO, data.LVALUE
//Output: entire level record
//Called by DMS_findLFLevel
//////////////////////////////////////////////////////////////////////////////
LONG findLFLevel(BOOL fLocal, DMS_LEVELC &data)
{
	#define SQL_FINDLFLEVEL \
		"SELECT LABELID, FACTORID, LEVELNO, LVALUE  \
		FROM LEVEL_N \
WHERE (((LABELID)=?) AND ((LEVELNO)=?) AND ((LVALUE)=cstr(?))) \
		UNION \
		SELECT LABELID, FACTORID, LEVELNO, LVALUE \
		FROM LEVEL_C \
		WHERE (((LABELID)=?) AND ((LEVELNO)=?) AND ((LVALUE)=?))"

//		WHERE (((LABELID)=?) AND ((LEVELNO)=?) AND ((LVALUE)=?)) \


	#define SQL_FINDLFLEVEL_Oracle \
		"SELECT LABELID, FACTORID, LEVELNO, {fn CONVERT(LVALUE,SQL_VARCHAR)} \
		FROM LEVEL_N \
		WHERE (((LABELID)=?) AND ((LEVELNO)=?) AND (({fn CONVERT(LVALUE,SQL_VARCHAR)})=?)) \
		UNION \
		SELECT LABELID, FACTORID, LEVELNO, LVALUE \
		FROM LEVEL_C \
		WHERE (((LABELID)=?) AND ((LEVELNO)=?) AND ((LVALUE)=?))"

	#define SQL_FINDLFLEVEL_MySQL \
		"SELECT LABELID, FACTORID, LEVELNO,  CAST(LVALUE as CHAR) \
		FROM LEVEL_N \
		WHERE (((LABELID)=?) AND ((LEVELNO)=?) AND ((CAST(LVALUE as CHAR))=?)) \
		UNION \
		SELECT LABELID, FACTORID, LEVELNO, LVALUE \
		FROM LEVEL_C \
		WHERE (((LABELID)=?) AND ((LEVELNO)=?) AND ((LVALUE)=?))"
	
	LONG ret = DMS_SUCCESS;
	
	static CODBCbindedStmt &central = _centralDMS->BindedStmt(SQL_FINDLFLEVEL);
	static CODBCbindedStmt &local = _localDMS->BindedStmt(SQL_FINDLFLEVEL);
	CODBCbindedStmt *source;
	
	static DMS_LEVELC _data;
	static BOOL first_time = TRUE;
	
	LPCSTR szDB;
    static string central_sql,local_sql;

	if (first_time) {
	   szDB = _localDMS->DatabaseName();
	   if ((strcmp(szDB, "MySQL")==0) || (strcmp(szDB, "PostgreSQL")==0) )  {
		   local_sql=SQL_FINDLFLEVEL_MySQL;
	   }
	   else if (strncmp(szDB, "Oracle",6)==0 ) {
		   local_sql=SQL_FINDLFLEVEL_Oracle;
	   }
	   else local_sql=SQL_FINDLFLEVEL;

	   szDB = _centralDMS->DatabaseName();
	   if ((strcmp(szDB, "MySQL")==0) || (strcmp(szDB, "PostgreSQL")==0) )  {
		  central_sql=SQL_FINDLFLEVEL_MySQL;
          central.SetSQLstr(central_sql.c_str());
	   }
	   else if (strncmp(szDB, "Oracle",6)==0 ) {
		  central_sql=SQL_FINDLFLEVEL_Oracle;
	   }
	   else central_sql=SQL_FINDLFLEVEL;
 
       local.SetSQLstr(local_sql.c_str());
       central.SetSQLstr(central_sql.c_str());

	    BINDPARAM(1, _data.LABELID);
	    BINDPARAM(2, _data.LEVELNO);
	    BINDPARAMS(3, _data.LVALUE, DMS_DATA_VALUE-1);
		BINDPARAM(4, _data.LABELID);
		BINDPARAM(5, _data.LEVELNO);
		BINDPARAMS(6, _data.LVALUE, DMS_DATA_VALUE-1);
		
		BIND(1, _data.LABELID);
		BIND(2, _data.FACTORID);
		BIND(3, _data.LEVELNO);
		BINDS(4, _data.LVALUE, DMS_DATA_VALUE);
		
		first_time = FALSE;
	}
	_data = data;
	local.Execute();
	central.Execute();
	
	source = (fLocal) ? &local : &central;
	if (source->Fetch()) {
		ret = GMS_SUCCESS;
		data = _data;
	}
	else if (source->NoData())
		ret = DMS_NO_DATA;
	else 
		ret = DMS_ERROR;
	
	return ret;
	
	#undef SQL_FINDLFLEVEL
	#undef SQL_FINDLFLEVEL_MySQL
	#undef SQL_FINDLFLEVEL_Oracle
}

//////////////////////////////////////////////////////////////////////////////
//added by WVC
//***** Finds the entry corresponding to the label ID, level no. and level value *****/
//Input:  data.LABELID, data.LEVELNO, data.LVALUE
//Output: entire level record
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_findLFLevel(DMS_LEVELC *data)
{
	PUSH(DMS_findLFLevel);
	
	if (data->LABELID < 0){
		ret = findLFLevel(LOCAL, *data);
		if (ret != DMS_SUCCESS){
			ret = findLFLevel(CENTRAL, *data);
		}
	}
	else
		ret = findLFLevel(CENTRAL, *data);
	
	POP();
}	




