/* Bibrefs.cpp : Implements the functions to access and manipulate the BIBREFS table of ICIS
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

#include "bibrefs.h"

extern LONG UACCESS,USERID;
extern CODBCdbc *_centralDBC,*_localDBC;  //Central and Local Database Connection
extern stack<char*> _routines_stack;


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG getNextREFID(void)
{
static LONG refID=0;

   //if (!refID)
   //{
      CODBCdirectStmt local = _localDBC->DirectStmt("SELECT MIN(REFID) FROM BIBREFS");
      local.Execute();
      local.Fetch();
      refID=local.Field(1).AsInteger();
   //   if (refID<0) refID=0;
   //}
   return --refID;
}

   
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG getBibrefs(BOOL fLocal, GMS_Bibrefs &data,LONG fSearchOption)
{
#define SQL_GETBIBREFS	"\
  SELECT BIBREFS.REFID, BIBREFS.PUBTYPE, BIBREFS.PUBDATE, BIBREFS.AUTHORS, BIBREFS.EDITORS, \
    BIBREFS.ANALYT, BIBREFS.MONOGR, BIBREFS.SERIES, BIBREFS.VOLUME, BIBREFS.ISSUE,  \
    BIBREFS.PAGECOL, BIBREFS.PUBLISH, BIBREFS.PUCITY, BIBREFS.PUCNTRY \
  FROM BIBREFS \
  WHERE (BIBREFS.REFID=? OR 0=?) "

   PUSH(getBibrefs);

static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_GETBIBREFS);
static CODBCbindedStmt &central  = _centralDBC->BindedStmt(SQL_GETBIBREFS);
       CODBCbindedStmt *source;

static GMS_Bibrefs _data;
static BOOL first_time=TRUE;

static LONG id;

   if (first_time)
   {
      BIND(1,_data.refid);
      BIND(2,_data.pubtype);
      BIND(3,_data.pubdate);
      BINDS(4,_data.authors,sizeof(_data.authors));
      BINDS(5,_data.editors,sizeof(_data.editors));
      BINDS(6,_data.analyt,sizeof(_data.analyt));
      BINDS(7,_data.monog,sizeof(_data.monog));
      BINDS(8,_data.series,sizeof(_data.series));
      BINDS(9,_data.volume,sizeof(_data.volume));
      BINDS(10,_data.issue,sizeof(_data.issue));
      BINDS(11,_data.pagecol,sizeof(_data.pagecol));
      BINDS(12,_data.publish,sizeof(_data.publish));
      BINDS(13,_data.pubcity,sizeof(_data.pubcity));
      BINDS(14,_data.pubcntry,sizeof(_data.pubcntry));
	  BINDPARAM(1,id);
	  BINDPARAM(2,id);
 
      first_time=FALSE;
   }

   if (fSearchOption==FIND_FIRST){
	   id=data.refid;
      local.Execute();
	  central.Execute();
   }
   ZeroMemory(&_data,sizeof(_data));
   source = (fLocal)?&local:&central;

   if (source->Fetch())
      data = _data;
   else if (local.NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;

   POP();

#undef SQL_GETBIBREFS
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_getBibrefs(GMS_Bibrefs *data,LONG fSearchOption)
{

   PUSH(GMS_getBibrefs);
   static BOOL fLocal=TRUE;


   if (fSearchOption==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal){
      ret = getBibrefs(LOCAL,*data,fSearchOption);
      if (ret!=GMS_SUCCESS){
         fLocal = FALSE; 
         ret = getBibrefs(CENTRAL,*data,FIND_NEXT);
      }
   }
   else
      ret = getBibrefs(CENTRAL,*data,fSearchOption);
   
   POP();
}




//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG addBibrefs(GMS_Bibrefs &data)
{
#define SQL_ADDBIBREFS "\
   insert into BIBREFS\
   (REFID, PUBTYPE, PUBDATE, AUTHORS, EDITORS, \
    ANALYT, MONOGR, SERIES, VOLUME, ISSUE,  \
    PAGECOL, PUBLISH, PUCITY, PUCNTRY)\
   values (?, ?, ?, ?, ?, ?, ?)"

   PUSH(addBibrefs);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_ADDBIBREFS);

static GMS_Bibrefs _data;
static BOOL first_time=TRUE;

   if (first_time)
   {
      local.BindParam(1,_data.refid);
      local.BindParam(2,_data.pubtype);
      local.BindParam(3,_data.pubdate);
      local.BindParam(4,_data.authors,sizeof(_data.authors));
      local.BindParam(5,_data.editors,sizeof(_data.authors));
      local.BindParam(6,_data.analyt,sizeof(_data.authors));
      local.BindParam(7,_data.monog,sizeof(_data.authors));
      local.BindParam(8,_data.series,sizeof(_data.authors));
      local.BindParam(9,_data.volume,sizeof(_data.authors));
      local.BindParam(10,_data.issue,sizeof(_data.authors));
      local.BindParam(11,_data.pagecol,sizeof(_data.authors));
      local.BindParam(12,_data.publish,sizeof(_data.authors));
      local.BindParam(13,_data.pubcity,sizeof(_data.authors));
      local.BindParam(14,_data.pubcntry,sizeof(_data.authors));

 
      first_time=FALSE;
   }
   
   data.refid= getNextREFID();

   _data=data;

   if ((ret=local.Execute())==1)
      ret=GMS_SUCCESS;   
   else 
	   if (ret!=GMS_UNIQUE_CONSTRAINT_ERROR) //GMS_UNIQUE_CONSTRAINT_ERROR
		   ret=GMS_ERROR;  
       // Handler for unique contraint error with the assumption that only AID has unique contraint 
	   else {
		   int i=0;
		   while ((ret==GMS_UNIQUE_CONSTRAINT_ERROR) && (i<100)) {
				_data.refid = getNextREFID();
				ret=local.Execute();
				++i;
		   }
		   if ((ret) &&(ret!=GMS_UNIQUE_CONSTRAINT_ERROR)) {
               data.refid = _data.refid;
			   ret = GMS_SUCCESS;
		   }
		   else  {
			   data.refid = 0;
			   ret = GMS_ERROR;
		   }
	   }
   POP();

#undef SQL_ADDBIBREFS
}



//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_addBibrefs(GMS_Bibrefs *data)
{
   return addBibrefs(*data);
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG setBibrefs(GMS_Bibrefs &data)
{
#define SQL_SETBIBREFS "\
   update BIBREFS\
   set REFID =?, PUBTYPE=?, PUBDATE=?, AUTHORS=?, EDITORS=?, \
    ANALYT=?, MONOGR=?, SERIES=?, VOLUME=?, ISSUE=?,  \
    PAGECOL=?, PUBLISH=?, PUCITY=?, PUCNTRY=?)\
   where REFID = ?"

   PUSH(setBibrefs);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_SETBIBREFS);

static GMS_Bibrefs _data;
static BOOL first_time=TRUE;

   if (first_time)
   {
      local.BindParam(1,_data.refid);
      local.BindParam(2,_data.pubtype);
      local.BindParam(3,_data.pubdate);
      local.BindParam(4,_data.authors,sizeof(_data.authors));
      local.BindParam(5,_data.editors,sizeof(_data.authors));
      local.BindParam(6,_data.analyt,sizeof(_data.authors));
      local.BindParam(7,_data.monog,sizeof(_data.authors));
      local.BindParam(8,_data.series,sizeof(_data.authors));
      local.BindParam(9,_data.volume,sizeof(_data.authors));
      local.BindParam(10,_data.issue,sizeof(_data.authors));
      local.BindParam(11,_data.pagecol,sizeof(_data.authors));
      local.BindParam(12,_data.publish,sizeof(_data.authors));
      local.BindParam(13,_data.pubcity,sizeof(_data.authors));
      local.BindParam(14,_data.pubcntry,sizeof(_data.authors));
      local.BindParam(15,_data.refid);

 
      first_time=FALSE;
   }
   
   _data=data;

   if (local.Execute())
      ret = GMS_SUCCESS;
   else if (local.NoData())
      ret = GMS_NO_DATA;
   else 
      ret = GMS_ERROR;


   POP();

#undef SQL_SETBIBREFS
}

//////////////////////////////////////////////////////////////////////////////
//Date Created: October 22, 2008 (AMP)
//Description: Updates the record in BIBREFS table for the specified REFID  
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_setBibrefs(GMS_Bibrefs *data)
{
   return setBibrefs(*data);
}

