/* Lot.cpp : Implements the functions to access and manipulate the IMS_LOT,IMS_TRANSACTION tables of ICIS
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
  Programmed by   :   Arllet M. Portugal, Beth Sison
 Modified  (AMP) :  Jan 20, 2004 (LOT and ITRANSACTION are renamed to IMS_LOT, IMS_TRANSACTION 
                 :  March 11,2005 (PERSONID is added in ITRANSACTION)
 **************************************************************/

#include "Lot.h"

extern LONG UACCESS,USERID;
extern CODBCdbc *_centralIMS,*_localIMS;  //Central and Local Database Connection
extern stack<char*> _routines_stack;




//////////////////////////////////////////////////////////////////////////////
//  Finds the minimum  lotid
//////////////////////////////////////////////////////////////////////////////
LONG getNextLotID(void)
{
static LONG lotID=0;
      lotID=0;
      CODBCdirectStmt local = _localIMS->DirectStmt("SELECT MAX(LOTID) FROM IMS_LOT");
      local.Execute();
      local.Fetch();
      if (!local.NoData()) {
         lotID=local.Field(1).AsInteger();
      }
      return ++lotID;
}


//////////////////////////////////////////////////////////////////////////////
//  Finds the maximun transaction id
// Modification: AMP (2008/09/24):  deleted the condition LOTID
//////////////////////////////////////////////////////////////////////////////
LONG getNextTransID(LONG lotid)
{
#define SQL_GETNEXTTRANSID "SELECT MAX(TRNID) FROM IMS_TRANSACTION" 
   PUSH(getNextEntryID);

static CODBCbindedStmt &local  = _localIMS->BindedStmt(SQL_GETNEXTTRANSID);
static BOOL first_time=TRUE;
static LONG transID=0,_lotid;

   if (first_time)
   {
      local.Bind(1,transID);
//      local.BindParam(1,_lotid);
 
      first_time=FALSE;
   }
   transID = 0;
   _lotid=lotid;
   local.Execute();
   local.Fetch();
   return ++transID;
}


//////////////////////////////////////////////////////////////////////////////
//  Tests whether an lot id exists or not
//////////////////////////////////////////////////////////////////////////////
LONG existLotID(LONG lotid)
{

      CODBCbindedStmt local = _localIMS->BindedStmt("SELECT LOTID FROM IMS_LOT WHERE LOTID = ?");

      local.BindParam(1,lotid);
      local.Execute();
      local.Fetch();

      if (!local.NoData())
         return DMS_SUCCESS;
	  else 
	     return DMS_NO_DATA; 

}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG addLot(IMS_LOT &data)
{
#define SQL_ADDLOT "\
    INSERT INTO IMS_LOT ( LOTID, USERID, ETYPE, EID, LOCID, SCALEID, STATUS, SOURCEID ,COMMENTS ) \
      values(?,?,?,?,?,?,?,?,?)"

   PUSH(addLot);

static CODBCbindedStmt &local  = _localIMS->BindedStmt(SQL_ADDLOT);

static IMS_LOT _data;
static BOOL first_time=TRUE;
static LONG gid;
static  GMS_Germplasm germ;

   ZeroMemory(&_data,sizeof(data));
   _data = data;
   gid = data.EID;
//   ZeroMemory(&germ,sizeof(germ));
//   ret=GMS_getGermplasm(gid,&germ);
//  if ( ret != GMS_SUCCESS) 
//     return IMS_INVALID_ID;
   
   if (first_time)
   {
      local.BindParam(1,_data.LOTID);
      local.BindParam(2,_data.USERID);
      local.BindParam(3,_data.ETYPE,sizeof(_data.ETYPE));
      local.BindParam(4,_data.EID);
      local.BindParam(5,_data.LOCID);
      local.BindParam(6,_data.SCALEID);
      local.BindParam(7,_data.STATUS);
      local.BindParam(8,_data.SOURCE);
      local.BindParam(9,_data.COMMENTS,sizeof(_data.COMMENTS)-1); 
      first_time=FALSE;
   }

   data.LOTID = getNextLotID();
   data.USERID = USERID;
   _data = data;
   if (local.Execute())
      ret=IMS_SUCCESS;
   else
      ret=IMS_ERROR;

   POP();

#undef SQL_ADDLOT
}


LONG DLL_INTERFACE IMS_addLot(IMS_LOT *lot)
{
	PUSH(IMS_addLot);
    static LONG gid;
	ret = addLot(*lot);
	POP();
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG closeLot(LONG lotid)
{
#define SQL_CLOSELOT "\
            UPDATE IMS_LOT SET STATUS = 1 \
           WHERE LOTID=?" 

   PUSH(closeLot);

static CODBCbindedStmt &local  = _localIMS->BindedStmt(SQL_CLOSELOT);

static LONG _lotid;
static BOOL first_time=TRUE;

   _lotid = lotid;
   
   if (first_time)
   {
      local.BindParam(1,_lotid);
   }

   if (local.Execute())
      ret=IMS_SUCCESS;
   else
      ret=IMS_ERROR;

   POP();

#undef SQL_CLOSELOT
}



//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE IMS_closeLot(LONG lotid)
{
   PUSH(IMS_closeLot);
   ret = closeLot(lotid);
   POP();
}



//////////////////////////////////////////////////////////////////////////////
// Date Created: Nov 4, 2004
// Data Modified:  March 11, 2005
// Description:  Updates the ITRANSACTION table except LOTID and TRNID
//////////////////////////////////////////////////////////////////////////////
LONG setLot( IMS_LOT lot)
{
#define SQL_SETLOT "\
     UPDATE IMS_LOT \
	 SET IMS_LOT.USERID=?, IMS_LOT.ETYPE=?,IMS_LOT.EID=?,IMS_LOT.LOCID=?, \
	     IMS_LOT.SCALEID=?,IMS_LOT.STATUS = ?, IMS_LOT.COMMENTS = ?, IMS_LOT.SOURCEID = ?  \
     WHERE  LOTID=? "

   PUSH(setLot);

static CODBCbindedStmt &local  = _localIMS->BindedStmt(SQL_SETLOT);

static IMS_LOT _data;
static BOOL first_time=TRUE;

   _data = lot;
   
   if (first_time)
   {
      local.BindParam(1,_data.USERID);
      local.BindParam(2,_data.ETYPE, sizeof(_data.ETYPE));
      local.BindParam(3,_data.EID);
      local.BindParam(4,_data.LOCID);
      local.BindParam(5,_data.SCALEID); 
      local.BindParam(6,_data.STATUS); 
      local.BindParam(7,_data.COMMENTS,sizeof(_data.COMMENTS)-1);
      local.BindParam(8,_data.SOURCE);
      local.BindParam(9,_data.LOTID);
   }
    
   if (local.Execute())
      ret=IMS_SUCCESS;
   else
      ret=IMS_ERROR;

   POP();

#undef SQL_SETLOT
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE IMS_setLot(IMS_LOT* lot)
{
   PUSH(IMS_setLot);
   ret = setLot(*lot);
   POP();
}



//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG addTransaction(IMS_TRANSACTION &data)
{
#define SQL_ADDTRANSACTION "\
      INSERT INTO IMS_TRANSACTION ( TRNID, USERID, LOTID, TRNDATE, TRNSTAT, TRNQTY, COMMENTS, CMTDATA, PERSONID) \
      values(?,?,?,?,?,?,?,?,?)"

   PUSH(addTransaction);

static CODBCbindedStmt &local  = _localIMS->BindedStmt(SQL_ADDTRANSACTION);

static IMS_TRANSACTION _data;
static BOOL first_time=TRUE;
   long   _lotid;

   _data = data;
   _lotid = data.LOTID;
   if (existLotID(_lotid) != IMS_SUCCESS) 
	   return IMS_INVALID_ID;
   
   if (first_time)
   {
      local.BindParam(1,_data.TRNID);
      local.BindParam(2,_data.TRNUSER);
      local.BindParam(3,_data.LOTID);
      local.BindParam(4,_data.TRNDATE);
      local.BindParam(5,_data.TRNSTAT);
      local.BindParam(6,_data.TRNQTY);
      local.BindParam(7,_data.COMMENTS,255); 
      local.BindParam(8,_data.CMTDATA); 
	  local.BindParam(9,_data.PERSONID);
      first_time=FALSE;
   }

   data.TRNID = getNextTransID(_lotid);
   data.TRNUSER = USERID;
   _data = data;
   if (local.Execute())
      ret=IMS_SUCCESS;
   else
      ret=IMS_ERROR;

   POP();

#undef SQL_ADDTRANSACTION
}


//////////////////////////////////////////////////////////////////////////////
//Date Created: Nov 4, 2004
//Date Modified: March 11, 2005
//
//////////////////////////////////////////////////////////////////////////////
LONG addTransaction2(IMS_TRANSACTION &data)
{
#define SQL_ADDTRANSACTION "\
      INSERT INTO IMS_TRANSACTION ( TRNID, USERID, LOTID, TRNDATE, TRNSTAT, TRNQTY, \
	              COMMENTS, CMTDATA, SOURCETYPE, SOURCEID, RECORDID, PREVAMOUNT, PERSONID) \
      values(?,?,?,?,?,?,?,?,?,?,?,?,?)"

   PUSH(addTransaction);

static CODBCbindedStmt &local  = _localIMS->BindedStmt(SQL_ADDTRANSACTION);

static IMS_TRANSACTION _data;
static BOOL first_time=TRUE;
   long   _lotid;

   ZeroMemory(&_data,sizeof(_data));
   _data = data;
   _lotid = data.LOTID;
   if (existLotID(_lotid) != IMS_SUCCESS) 
	   return IMS_INVALID_ID;
   
   if (first_time)
   {
      local.BindParam(1,_data.TRNID);
      local.BindParam(2,_data.TRNUSER);
      local.BindParam(3,_data.LOTID);
      local.BindParam(4,_data.TRNDATE);
      local.BindParam(5,_data.TRNSTAT);
      local.BindParam(6,_data.TRNQTY);
      local.BindParam(7,_data.COMMENTS,255); 
      local.BindParam(8,_data.CMTDATA); 
	  local.BindParam(9, _data.SOURCETYPE, IMS_SOURCETYPE);
	  local.BindParam(10, _data.SOURCEID);
	  local.BindParam(11, _data.RECORDID);
	  local.BindParam(12, _data.PREVAMOUNT);
	  local.BindParam(13,_data.PERSONID);
      first_time=FALSE;
   }

   data.TRNID = getNextTransID(_lotid);
   data.TRNUSER = USERID;
   _data = data;
   if (local.Execute())
      ret=IMS_SUCCESS;
   else
      ret=IMS_ERROR;

   POP();

#undef SQL_ADDTRANSACTION2
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE IMS_addTransaction(IMS_TRANSACTION* trans)
{
   PUSH(IMS_addTransaction);
   ret = addTransaction2(*trans);
   POP();
}


//////////////////////////////////////////////////////////////////////////////
//Modified: Oct 8, 2004
//Modification:  Lot that is closed (STATUS = 1) is not shown
//////////////////////////////////////////////////////////////////////////////
LONG findLotRecord(BOOL fLocal,IMS_LOT &data,LONG fSearchOption)
{
#define SQL_LOT_SELECT_FMT "SELECT LOTID, USERID, ETYPE, EID, LOCID, SCALEID, STATUS, COMMENTS, SOURCEID FROM IMS_LOT where "
#define SQL_LOT_WHERE_FMT " and %s=?"

   PUSH(findLotRecord);

static CODBCbindedStmt &local= _localIMS->BindedStmt();  

static string strSQL="", prevstrSQL="";
static BOOL changed=TRUE;
static BOOL first_time=TRUE,fOK;
static IMS_LOT _lot;
static BOOL fetype,feid,flocid;
static CHAR _etype[16];
static LONG _eid,_locid,_scaleid, noSQL=0;


   if (fSearchOption==FIND_FIRST)
   {
         strSQL="SELECT LOTID, USERID, ETYPE, EID, LOCID, SCALEID, STATUS, COMMENTS, SOURCEID FROM IMS_LOT where STATUS <> 1 ";
         if ((_eid=data.EID)!=0)                                  
            strSQL = strSQL+" AND EID=?";
         if ((_locid =data.LOCID)!=0)                                 
            strSQL = strSQL+" AND LOCID=?";
         if ((_scaleid =data.SCALEID)!=0)                                 
            strSQL = strSQL+" AND SCALEID=?";
         if (data.ETYPE[0]!='\0') {                                   
			 strcpy(_etype,data.ETYPE);
            strSQL = strSQL+" AND ETYPE=?";                          
         }

//         result = strSQL.compare(prevstrSQL );
//		 if (result!=0) {
//			 prevstrSQL=strSQL;
             local   = _localIMS->BindedStmt((LPCSTR)strSQL.c_str());       
//         }

         // Bind all Colums and Parameter
         local.Bind(1,_lot.LOTID);
		 local.Bind(2,_lot.USERID);
		 local.Bind(3,_lot.ETYPE, sizeof(_lot.ETYPE));
		 local.Bind(4,_lot.EID);
		 local.Bind(5,_lot.LOCID);
		 local.Bind(6,_lot.SCALEID);
		 local.Bind(7,_lot.STATUS);
		 local.Bind(8,_lot.COMMENTS, sizeof(_lot.COMMENTS));
		 local.Bind(9,_lot.SOURCE);


         int i=1;
         if (_eid!=0)
         {
            local.BindParam(i,_eid);i+=1;
         }
         if (_locid!=0)
         {
            local.BindParam(i,_locid);i+=1;
         }
         if (_scaleid!=0)
         {
            local.BindParam(i,_scaleid);i+=1;
		 }
         if (_etype[0]!='\0')
         {
            local.BindParam(i,_etype,sizeof(_etype));i+=1;
         }

         local.Execute();
   }
 

   if (local.Fetch()) {
      data = _lot;
      ret = IMS_SUCCESS;
	  }
   else if (local.NoData())
      ret = IMS_NO_DATA;
   else 
      ret = IMS_ERROR;

   POP();


#undef SQL_LOT_SELECT_FMT 
#undef SQL_LOT_WHERE_FMT 
}



//////////////////////////////////////////////////////////////////////////////
//Create: Jan 12, 2005
// Input: EID & ETYPE
//Modification:  Lot that is closed (STATUS = 1) is not shown
//////////////////////////////////////////////////////////////////////////////
LONG findLotRecord1(BOOL fLocal,IMS_LOT &data,LONG fSearchOption)
{
#define SQL_findLotRecord1 "SELECT LOTID, USERID, ETYPE, EID, LOCID, SCALEID, STATUS, COMMENTS, SOURCEID FROM IMS_LOT where STATUS <> 1  \
                    AND  EID = ? AND  ETYPE = ?" 


   PUSH(findLotRecord1);

static CODBCbindedStmt &local= _localIMS->BindedStmt(SQL_findLotRecord1);  

static string strSQL="", prevstrSQL="";
static BOOL changed=TRUE;
static BOOL first_time=TRUE,fOK;
static IMS_LOT _lot;
static BOOL fetype,feid,flocid;
static CHAR _etype[16];
static LONG _eid,_locid,_scaleid, noSQL=0;


   if (fSearchOption==FIND_FIRST)
   {
         // Bind all Colums and Parameter
         local.Bind(1,_lot.LOTID);
		 local.Bind(2,_lot.USERID);
		 local.Bind(3,_lot.ETYPE, sizeof(_lot.ETYPE));
		 local.Bind(4,_lot.EID);
		 local.Bind(5,_lot.LOCID);
		 local.Bind(6,_lot.SCALEID);
		 local.Bind(7,_lot.STATUS);
		 local.Bind(8,_lot.COMMENTS, sizeof(_lot.COMMENTS));
		 local.Bind(9,_lot.SOURCE);

        _eid = data.EID;
 	    strcpy(_etype,data.ETYPE);
        local.BindParam(1,_eid);
        local.BindParam(2,_etype,sizeof(_etype));

         local.Execute();
   }
 

   if (local.Fetch()) {
      data = _lot;
      ret = IMS_SUCCESS;
	  }
   else if (local.NoData())
      ret = IMS_NO_DATA;
   else 
      ret = IMS_ERROR;

   POP();


#undef SQL_findLotRecord1
#undef SQL_findLotRecord1
}


//////////////////////////////////////////////////////////////////////////////
//Create: Jan 12, 2005
// Input: EID & LOCID
//Modification:  Lot that is closed (STATUS = 1) is not shown
//////////////////////////////////////////////////////////////////////////////
LONG findLotRecord2(BOOL fLocal,IMS_LOT &data,LONG fSearchOption)
{
#define SQL_findLotRecord2 "SELECT LOTID, USERID, ETYPE, EID, LOCID, SCALEID, STATUS, COMMENTS, SOURCEID FROM IMS_LOT where STATUS <> 1  \
                    AND  EID = ? AND ETYPE= ?  AND  LOCID = ?" 


   PUSH(findLotRecord2);

static CODBCbindedStmt &local= _localIMS->BindedStmt(SQL_findLotRecord2);  

static string strSQL="", prevstrSQL="";
static BOOL changed=TRUE;
static BOOL first_time=TRUE,fOK;
static IMS_LOT _lot;
static BOOL fetype,feid,flocid;
static CHAR _etype[16];
static LONG _eid,_locid,_scaleid, noSQL=0;


   if (fSearchOption==FIND_FIRST)
   {
         // Bind all Colums and Parameter
         local.Bind(1,_lot.LOTID);
		 local.Bind(2,_lot.USERID);
		 local.Bind(3,_lot.ETYPE, sizeof(_lot.ETYPE));
		 local.Bind(4,_lot.EID);
		 local.Bind(5,_lot.LOCID);
		 local.Bind(6,_lot.SCALEID);
		 local.Bind(7,_lot.STATUS);
		 local.Bind(8,_lot.COMMENTS, sizeof(_lot.COMMENTS));
		 local.Bind(9,_lot.SOURCE);


        _eid = data.EID;
		_locid = data.LOCID;
 	    strcpy(_etype,data.ETYPE);
        local.BindParam(1,_eid);
        local.BindParam(2,_etype,sizeof(_etype));
        local.BindParam(3,_locid);

         local.Execute();
   }
 

   if (local.Fetch()) {
      data = _lot;
      ret = IMS_SUCCESS;
	  }
   else if (local.NoData())
      ret = IMS_NO_DATA;
   else 
      ret = IMS_ERROR;

   POP();


#undef SQL_findLotRecord2
#undef SQL_findLotRecord2
}


//////////////////////////////////////////////////////////////////////////////
//Create: Jan 12, 2005
// Input: EID & SCALEID
//Modification:  Lot that is closed (STATUS = 1) is not shown
//////////////////////////////////////////////////////////////////////////////
LONG findLotRecord3(BOOL fLocal,IMS_LOT &data,LONG fSearchOption)
{
#define SQL_findLotRecord3 "SELECT LOTID, USERID, ETYPE, EID, LOCID, SCALEID, STATUS, COMMENTS, SOURCEID FROM IMS_LOT where STATUS <> 1  \
                   AND   EID = ? AND ETYPE = ? AND  SCALEID = ?" 


   PUSH(findLotRecord3);

static CODBCbindedStmt &local= _localIMS->BindedStmt(SQL_findLotRecord3);  

static string strSQL="", prevstrSQL="";
static BOOL changed=TRUE;
static BOOL first_time=TRUE,fOK;
static IMS_LOT _lot;
static BOOL fetype,feid,flocid;
static CHAR _etype[16];
static LONG _eid,_locid,_scaleid, noSQL=0;


   if (fSearchOption==FIND_FIRST)
   {
         // Bind all Colums and Parameter
         local.Bind(1,_lot.LOTID);
		 local.Bind(2,_lot.USERID);
		 local.Bind(3,_lot.ETYPE, sizeof(_lot.ETYPE));
		 local.Bind(4,_lot.EID);
		 local.Bind(5,_lot.LOCID);
		 local.Bind(6,_lot.SCALEID);
		 local.Bind(7,_lot.STATUS);
		 local.Bind(8,_lot.COMMENTS, sizeof(_lot.COMMENTS));
		 local.Bind(9,_lot.SOURCE);


        _eid = data.EID;
		_scaleid = data.SCALEID;
 	    strcpy(_etype,data.ETYPE);

        local.BindParam(1,_eid);
        local.BindParam(2,_etype,sizeof(_etype));
        local.BindParam(3,_scaleid);

         local.Execute();
   }
 

   if (local.Fetch()) {
      data = _lot;
      ret = IMS_SUCCESS;
	  }
   else if (local.NoData())
      ret = IMS_NO_DATA;
   else 
      ret = IMS_ERROR;

   POP();


#undef SQL_findLotRecord3
#undef SQL_findLotRecord3
}


//////////////////////////////////////////////////////////////////////////////
//Create: Jan 12, 2005
// Input: EID & LOCID & SCALEID
//Modification:  Lot that is closed (STATUS = 1) is not shown
//////////////////////////////////////////////////////////////////////////////
LONG findLotRecord4(BOOL fLocal,IMS_LOT &data,LONG fSearchOption)
{
#define SQL_findLotRecord4 "SELECT LOTID, USERID, ETYPE, EID, LOCID, SCALEID, STATUS, COMMENTS, SOURCEID FROM IMS_LOT where STATUS <> 1  \
                   AND   EID = ? AND ETYPE = ? AND LOCID=? AND  SCALEID = ?" 


   PUSH(findLotRecord4);

static CODBCbindedStmt &local= _localIMS->BindedStmt(SQL_findLotRecord4);  

static string strSQL="", prevstrSQL="";
static BOOL changed=TRUE;
static BOOL first_time=TRUE,fOK;
static IMS_LOT _lot;
static BOOL fetype,feid,flocid;
static CHAR _etype[16];
static LONG _eid,_locid,_scaleid, noSQL=0;


   if (fSearchOption==FIND_FIRST)
   {
         // Bind all Colums and Parameter
         local.Bind(1,_lot.LOTID);
		 local.Bind(2,_lot.USERID);
		 local.Bind(3,_lot.ETYPE, sizeof(_lot.ETYPE));
		 local.Bind(4,_lot.EID);
		 local.Bind(5,_lot.LOCID);
		 local.Bind(6,_lot.SCALEID);
		 local.Bind(7,_lot.STATUS);
		 local.Bind(8,_lot.COMMENTS, sizeof(_lot.COMMENTS));
		 local.Bind(9,_lot.SOURCE);


		 _eid = data.EID;
		_scaleid = data.SCALEID;
		_locid = data.LOCID;
 	    strcpy(_etype,data.ETYPE);
        local.BindParam(1,_eid);
        local.BindParam(2,_etype,sizeof(_etype));
        local.BindParam(3,_locid);
        local.BindParam(4,_scaleid);

        local.Execute();
   }
 

   if (local.Fetch()) {
      data = _lot;
      ret = IMS_SUCCESS;
	  }
   else if (local.NoData())
      ret = IMS_NO_DATA;
   else 
      ret = IMS_ERROR;

   POP();


#undef SQL_findLotRecord4
#undef SQL_findLotRecord4
}



//////////////////////////////////////////////////////////////////////////////
//Create: Jan 12, 2005
// Input: LOCID
//Modification:  Lot that is closed (STATUS = 1) is not shown
//////////////////////////////////////////////////////////////////////////////
LONG findLotRecord5(BOOL fLocal,IMS_LOT &data,LONG fSearchOption)
{
#define SQL_findLotRecord5 "SELECT LOTID, USERID, ETYPE, EID, LOCID, SCALEID, STATUS, COMMENTS, SOURCEID FROM IMS_LOT where STATUS <> 1  \
                   AND LOCID=? " 


   PUSH(findLotRecord5);

static CODBCbindedStmt &local= _localIMS->BindedStmt(SQL_findLotRecord5);  

static string strSQL="", prevstrSQL="";
static BOOL changed=TRUE;
static BOOL first_time=TRUE,fOK;
static IMS_LOT _lot;
static BOOL fetype,feid,flocid;
static CHAR _etype[16];
static LONG _eid,_locid,_scaleid, noSQL=0;


   if (fSearchOption==FIND_FIRST)
   {
         // Bind all Colums and Parameter
         local.Bind(1,_lot.LOTID);
		 local.Bind(2,_lot.USERID);
		 local.Bind(3,_lot.ETYPE, sizeof(_lot.ETYPE));
		 local.Bind(4,_lot.EID);
		 local.Bind(5,_lot.LOCID);
		 local.Bind(6,_lot.SCALEID);
		 local.Bind(7,_lot.STATUS);
		 local.Bind(8,_lot.COMMENTS, sizeof(_lot.COMMENTS));
		 local.Bind(9,_lot.SOURCE);


		_locid = data.LOCID;
        local.BindParam(1,_locid);

        local.Execute();
   }
 

   if (local.Fetch()) {
      data = _lot;
      ret = IMS_SUCCESS;
	  }
   else if (local.NoData())
      ret = IMS_NO_DATA;
   else 
      ret = IMS_ERROR;

   POP();


#undef SQL_findLotRecord5
#undef SQL_findLotRecord5
}

//////////////////////////////////////////////////////////////////////////////
//Create: Jan 12, 2005
// Input: LOCID
//Modification:  Lot that is closed (STATUS = 1) is not shown
//////////////////////////////////////////////////////////////////////////////
LONG findLotRecord6(BOOL fLocal,IMS_LOT &data,LONG fSearchOption)
{
#define SQL_findLotRecord6 "SELECT LOTID, USERID, ETYPE, EID, LOCID, SCALEID, STATUS, COMMENTS, SOURCEID FROM IMS_LOT  \
						WHERE STATUS <> 1 AND LOTID=? " 

   PUSH(findLotRecord6);

static CODBCbindedStmt &local= _localIMS->BindedStmt(SQL_findLotRecord6);  

static BOOL changed=TRUE;
static BOOL first_time=TRUE,fOK;
static IMS_LOT _lot;
static LONG _lotid, noSQL=0;


   if (fSearchOption==FIND_FIRST)
   {
         // Bind all Colums and Parameter
         local.Bind(1,_lot.LOTID);
		 local.Bind(2,_lot.USERID);
		 local.Bind(3,_lot.ETYPE, sizeof(_lot.ETYPE));
		 local.Bind(4,_lot.EID);
		 local.Bind(5,_lot.LOCID);
		 local.Bind(6,_lot.SCALEID);
		 local.Bind(7,_lot.STATUS);
		 local.Bind(8,_lot.COMMENTS, sizeof(_lot.COMMENTS));
		 local.Bind(9,_lot.SOURCE);


		_lotid = data.LOTID;
        local.BindParam(1,_lotid);

		local.Execute();
   }
 

   if (local.Fetch()) {
      data = _lot;
      ret = IMS_SUCCESS;
	  }
   else if (local.NoData())
      ret = IMS_NO_DATA;
   else 
      ret = IMS_ERROR;

   POP();


#undef SQL_findLotRecord6
#undef SQL_findLotRecord6
}

//////////////////////////////////////////////////////////////////////////////
//Date Created: Oct 4, 2004
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE IMS_findLotRecord_(IMS_LOT *data,LONG fSearchOption)
{
   static BOOL fLocal=TRUE;

   PUSH(IMS_findLotRecord);

   ret=findLotRecord(LOCAL,*data,fSearchOption);

   POP();
}



//////////////////////////////////////////////////////////////////////////////
//Date Created: Oct 4, 2004
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE IMS_findLotRecord(IMS_LOT *data,LONG fSearchOption)
{
   static BOOL fLocal=TRUE;
   static int qtype;
   
   PUSH(IMS_findLotRecord);
   if (fSearchOption==0) {
	qtype = 0;
    if ((data->EID !=0) && (data->ETYPE[0]!='\0'))                                 
       qtype = 1;
    if ((data->EID !=0) && (data->LOCID !=0))                                 
       qtype = 2;
    if ((data->EID !=0) && (data->SCALEID!=0))                                 
       qtype = 3;
    if ((data->EID !=0) && (data->SCALEID!=0) && (data->LOCID !=0))
       qtype = 4; 
	if ((data->EID ==0) && (data->LOCID !=0))                                 
       qtype = 5;
	if ((data->LOTID !=0))  
       qtype = 6;
   } 
   switch (qtype)
   {
      case 1:
         ret=findLotRecord1(LOCAL,*data,fSearchOption);
	     break;
      case 2:
         ret=findLotRecord2(LOCAL,*data,fSearchOption);
	     break;
      case 3:
         ret=findLotRecord3(LOCAL,*data,fSearchOption);
	     break;
      case 4:
         ret=findLotRecord4(LOCAL,*data,fSearchOption);
	     break;
      case 5:
         ret=findLotRecord5(LOCAL,*data,fSearchOption);
	     break;
      case 6:
         ret=findLotRecord6(LOCAL,*data,fSearchOption);
         break;
      case 0:
         ret = DMS_NO_DATA; 
	     break;
   }
   POP();
}





//////////////////////////////////////////////////////////////////////////////
//Create: Jan 12, 2005
// Input: EID & LOCID
//Modification:  Lot that is closed (STATUS = 1) is not shown
//////////////////////////////////////////////////////////////////////////////
LONG countLot(BOOL fLocal,LONG eid,CHAR etype[16], LONG* cntLot)
{
#define SQL_countLot "SELECT COUNT(*) AS CntEID FROM IMS_LOT where STATUS <> 1  \
                    AND  EID = ? AND ETYPE= ? " 


   PUSH(countLot);

static CODBCbindedStmt &local= _localIMS->BindedStmt(SQL_countLot);  

static BOOL first_time=TRUE;
static CHAR _etype[16];
static LONG _cntLot, _eid;


   if (first_time)
   {
         // Bind all Colums and Parameter
        local.Bind(1,_cntLot);

        local.BindParam(1,_eid);
        local.BindParam(2,_etype,sizeof(_etype));
		first_time = FALSE;

   }
   _eid = eid;
   strcpy(_etype,etype);
 
   local.Execute();
 
   if (local.Fetch()) {
      *cntLot = _cntLot;
      ret = IMS_SUCCESS;
	  }
   else if (local.NoData())
      ret = IMS_NO_DATA;
   else 
      ret = IMS_ERROR;

   POP();


#undef SQL_countLot
#undef SQL_countLot
}



//////////////////////////////////////////////////////////////////////////////
//Date Created: Jan 13, 2005
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE IMS_countLot(LONG EID,CHAR ETYPE[16], LONG* cntLot)
{
   static BOOL fLocal=TRUE;

   PUSH(IMS_countLot);

   ret=countLot(LOCAL,EID, ETYPE, cntLot);

   POP();
}



//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG findTransactionRecord(BOOL fLocal,LONG lotid, IMS_TRANSACTION &data,LONG fSearchOption)
{
#define SQL_TRANS_SELECT "SELECT TRNID, USERID, LOTID, TRNDATE, TRNSTAT, TRNQTY, COMMENTS, CMTDATA \
                            FROM IMS_TRANSACTION where LOTID=? "

   PUSH(findTransactionRecord);

static CODBCbindedStmt &local= _localIMS->BindedStmt(SQL_TRANS_SELECT);  ; 
static string strSQL="", prevstrSQL="";
static BOOL changed=TRUE;
static BOOL first_time=TRUE,fOK;
static IMS_TRANSACTION _data;
static LONG _lotid=0;


   ZeroFill(_data.COMMENTS,sizeof(_data.COMMENTS));

   if (fSearchOption==FIND_FIRST)
   {

         // Bind all Colums and Parameter
         local.Bind(1,_data.TRNID);
         local.Bind(2,_data.TRNUSER);
         local.Bind(3,_data.LOTID);
         local.Bind(4,_data.TRNDATE);
         local.Bind(5,_data.TRNSTAT);
         local.Bind(6,_data.TRNQTY);
         local.Bind(7,_data.COMMENTS, 255); 
         local.Bind(8,_data.CMTDATA); 

         local.BindParam(1, _lotid);
		 _lotid = lotid;
         local.Execute();
 //        central.Execute();
   }
 

   if (local.Fetch()) {
      data = _data;
      ret = IMS_SUCCESS;
	  }
   else if (local.NoData())
      ret = IMS_NO_DATA;
   else 
      ret = IMS_ERROR;

   POP();


#undef SQL_TRANS_SELECT

}


//////////////////////////////////////////////////////////////////////////////
//Date Created: Nov 4, 2004
//Date Modified: March 11, 2005
//Description: Finds transactions for a given lotid  (SOURCETYPE, SOURCEID, RECORDID included)
//////////////////////////////////////////////////////////////////////////////
LONG findTransactionRecord2(BOOL fLocal,LONG lotid, IMS_TRANSACTION &data,LONG fSearchOption)
{
#define SQL_TRANS_SELECT "SELECT TRNID, USERID, LOTID, TRNDATE, TRNSTAT, TRNQTY, COMMENTS, CMTDATA, SOURCETYPE, SOURCEID, RECORDID,PREVAMOUNT, PERSONID \
                            FROM IMS_TRANSACTION where LOTID=? "

   PUSH(findTransactionRecord);

static CODBCbindedStmt &local= _localIMS->BindedStmt(SQL_TRANS_SELECT);  ; 
static string strSQL="", prevstrSQL="";
static BOOL changed=TRUE;
static BOOL first_time=TRUE,fOK;
static IMS_TRANSACTION _data;
static LONG _lotid=0;


   ZeroFill(_data.COMMENTS,sizeof(_data.COMMENTS));
   ZeroFill(_data.SOURCETYPE,IMS_SOURCETYPE);

   if (fSearchOption==FIND_FIRST)
   {

         // Bind all Colums and Parameter
         local.Bind(1,_data.TRNID);
         local.Bind(2,_data.TRNUSER);
         local.Bind(3,_data.LOTID);
         local.Bind(4,_data.TRNDATE);
         local.Bind(5,_data.TRNSTAT);
         local.Bind(6,_data.TRNQTY);
         local.Bind(7,_data.COMMENTS, 255); 
         local.Bind(8,_data.CMTDATA); 
         local.Bind(9, _data.SOURCETYPE, IMS_SOURCETYPE);
         local.Bind(10, _data.SOURCEID);
         local.Bind(11, _data.RECORDID);
		 local.Bind(12, _data.PREVAMOUNT);
		 local.Bind(13,_data.PERSONID);
         local.BindParam(1, _lotid);
		 _lotid = lotid;
         local.Execute();
 //        central.Execute();
   }
 

   if (local.Fetch()) {
      data = _data;
      ret = IMS_SUCCESS;
	  }
   else if (local.NoData())
      ret = IMS_NO_DATA;
   else 
      ret = IMS_ERROR;

   POP();


#undef SQL_TRANS_SELECT

}

//////////////////////////////////////////////////////////////////////////////
//Date Modified: Nov 4, 2004
//Description: Finds transactions for a given lotid 
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE IMS_findTransactionRecord(LONG lotid,IMS_TRANSACTION *data,LONG fSearchOption)
{
   static BOOL fLocal=TRUE;

   PUSH(IMS_findTransactionRecord);

   ret=findTransactionRecord2(LOCAL,lotid, *data,fSearchOption);
   POP();
}



//////////////////////////////////////////////////////////////////////////////
//Date created: October 11, 2004
//Description: Retrieves all reserve transactions
//depracated:  Mar 2005
//////////////////////////////////////////////////////////////////////////////
//LONG getTransactionReserve(LONG flagAll, IMS_TRANSACTION &data, IMS_LOT &lot, LONG fSearchOption)
//{
//
//#define SQL_TRANS_RESERVE "SELECT IMS_TRANSACTION.TRNID, IMS_TRANSACTION.USERID, IMS_TRANSACTION.LOTID, IMS_TRANSACTION.TRNDATE, \
//	             IMS_TRANSACTION.TRNSTAT, IMS_TRANSACTION.TRNQTY, IMS_TRANSACTION.COMMENTS, IMS_TRANSACTION.CMTDATA, \
//                 IMS_LOT.USERID, IMS_LOT.ETYPE, IMS_LOT.EID, IMS_LOT.LOCID, IMS_LOT.SCALEID, IMS_LOT.STATUS, IMS_LOT.COMMENTS, IMS_LOT.SOURCEID \
//                 FROM IMS_TRANSACTION, IMS_LOT \
//				 WHERE  IMS_TRANSACTION.LOTID = IMS_LOT.LOTID AND IMS_TRANSACTION.TRNSTAT=0 and IMS_TRANSACTION.TRNQTY < 0 AND IMS_TRANSACTION.USERID =?  "
//#define SQL_TRANS_RESERVE2 "SELECT IMS_TRANSACTION.TRNID, IMS_TRANSACTION.USERID, IMS_TRANSACTION.LOTID, IMS_TRANSACTION.TRNDATE, \
//	             IMS_TRANSACTION.TRNSTAT, IMS_TRANSACTION.TRNQTY, IMS_TRANSACTION.COMMENTS, IMS_TRANSACTION.CMTDATA, \
//                 IMS_LOT.USERID, IMS_LOT.ETYPE, IMS_LOT.EID, IMS_LOT.LOCID, IMS_LOT.SCALEID, IMS_LOT.STATUS, IMS_LOT.COMMENTS, IMS_LOT.SOURCEID \
//                 FROM IMS_TRANSACTION, IMS_LOT \
//				 WHERE  IMS_TRANSACTION.LOTID = IMS_LOT.LOTID AND IMS_TRANSACTION.TRNSTAT=0 and IMS_TRANSACTION.TRNQTY < 0   "
//
//   PUSH(getTransactionReserve);
//
//static CODBCbindedStmt &local= _localIMS->BindedStmt(SQL_TRANS_RESERVE);  ; 
//static string strSQL="", prevstrSQL="";
//static BOOL changed=TRUE;
//static BOOL first_time=TRUE,fOK;
//static IMS_TRANSACTION _data;
//static IMS_LOT _lot;
//static LONG _userid=0;
//
//
//   ZeroFill(_data.COMMENTS,sizeof(_data.COMMENTS));
//
//   if (fSearchOption==FIND_FIRST)
//   {
// 
//	   if (flagAll == 1) {
//             local  = _localIMS->BindedStmt(SQL_TRANS_RESERVE2);              //4/29/2003 (AMP):  reassign the statement handle if previous statement is different from the current one
//	   }
//	   else {
//             local  = _localIMS->BindedStmt(SQL_TRANS_RESERVE);              //4/29/2003 (AMP):  reassign the statement handle if previous statement is different from the current one
//		     _userid = USERID;
//             local.BindParam(1, _userid);
//	   }
//
//         // Bind all Colums and Parameter
//         local.Bind(1,_data.TRNID);
//         local.Bind(2,_data.TRNUSER);
//         local.Bind(3,_data.LOTID);
//         local.Bind(4,_data.TRNDATE);
//         local.Bind(5,_data.TRNSTAT);
//         local.Bind(6,_data.TRNQTY);
//         local.Bind(7,_data.COMMENTS, 255); 
//         local.Bind(8,_data.CMTDATA); 
//		 local.Bind(9,_lot.USERID);
//		 local.Bind(10,_lot.ETYPE, sizeof(_lot.ETYPE));
//		 local.Bind(11,_lot.EID);
//		 local.Bind(12,_lot.LOCID);
//		 local.Bind(13,_lot.SCALEID);
//		 local.Bind(14,_lot.STATUS);
//		 local.Bind(15,_lot.COMMENTS, sizeof(_lot.COMMENTS));
//		 local.Bind(16,_lot.SOURCE);
//
//
//         local.Execute();
// //        central.Execute();
//   }
// 
//
//   if (local.Fetch()) {
//	   _lot.LOTID = _data.LOTID;
//       lot = _lot;
//      data = _data;
//      ret = IMS_SUCCESS;
//	  }
//   else if (local.NoData())
//      ret = IMS_NO_DATA;
//   else 
//      ret = IMS_ERROR;
//
//   POP();
//
//
//#undef SQL_TRANS_RESERVE
//#undef SQL_TRANS_RESERVE2
//
//}

//////////////////////////////////////////////////////////////////////////////
//Data created: Nov 4, 2005
//Date modified: March 11, 2005
//Description: Retrieves all reserve transactions for the given user if  flagAll is 0 
//             and the specified sourcetype (LIST or STUDY) and source id (listid or studyid)
//////////////////////////////////////////////////////////////////////////////
LONG getTransactionReserve(LONG flagAll, IMS_TRANSACTION &data, IMS_LOT &lot, LONG fSearchOption)
{

#define SQL_TRANS_RESERVE "SELECT IMS_TRANSACTION.TRNID, IMS_TRANSACTION.USERID, IMS_TRANSACTION.LOTID, IMS_TRANSACTION.TRNDATE, \
	             IMS_TRANSACTION.TRNSTAT, IMS_TRANSACTION.TRNQTY, IMS_TRANSACTION.COMMENTS, IMS_TRANSACTION.CMTDATA, \
                 IMS_LOT.USERID, IMS_LOT.ETYPE, IMS_LOT.EID, IMS_LOT.LOCID, IMS_LOT.SCALEID, IMS_LOT.STATUS, IMS_LOT.COMMENTS, IMS_LOT.SOURCEID, \
				 IMS_TRANSACTION.SOURCETYPE, IMS_TRANSACTION.SOURCEID, IMS_TRANSACTION.RECORDID, IMS_TRANSACTION.PREVAMOUNT, IMS_TRANSACTION.PERSONID  \
                 FROM IMS_TRANSACTION, IMS_LOT \
				 WHERE  IMS_TRANSACTION.LOTID = IMS_LOT.LOTID AND IMS_TRANSACTION.TRNSTAT=0 and IMS_TRANSACTION.TRNQTY < 0 AND IMS_TRANSACTION.USERID =? \
				 AND IMS_TRANSACTION.SOURCETYPE= ? AND IMS_TRANSACTION.SOURCEID =? "
#define SQL_TRANS_RESERVE2 "SELECT IMS_TRANSACTION.TRNID, IMS_TRANSACTION.USERID, IMS_TRANSACTION.LOTID, IMS_TRANSACTION.TRNDATE, \
	             IMS_TRANSACTION.TRNSTAT, IMS_TRANSACTION.TRNQTY, IMS_TRANSACTION.COMMENTS, IMS_TRANSACTION.CMTDATA, \
                 IMS_LOT.USERID, IMS_LOT.ETYPE, IMS_LOT.EID, IMS_LOT.LOCID, IMS_LOT.SCALEID, IMS_LOT.STATUS, IMS_LOT.COMMENTS, IMS_LOT.SOURCEID, \
				 IMS_TRANSACTION.SOURCETYPE, IMS_TRANSACTION.SOURCEID, IMS_TRANSACTION.RECORDID, IMS_TRANSACTION.PREVAMOUNT, IMS_TRANSACTION.PERSONID \
                 FROM IMS_TRANSACTION, IMS_LOT \
				 WHERE  IMS_TRANSACTION.LOTID = IMS_LOT.LOTID AND IMS_TRANSACTION.TRNSTAT=0 and IMS_TRANSACTION.TRNQTY < 0 \
				 AND IMS_TRANSACTION.SOURCETYPE= ? AND IMS_TRANSACTION.SOURCEID =? "

   PUSH(getTransactionReserve2);

static CODBCbindedStmt &local= _localIMS->BindedStmt(SQL_TRANS_RESERVE);  ; 
static string strSQL="", prevstrSQL="";
static BOOL changed=TRUE;
static BOOL first_time=TRUE,fOK;
static IMS_TRANSACTION _data;
static IMS_LOT _lot;
static LONG _userid=0, _sourceid;
static char _srctype[IMS_SOURCETYPE];

   ZeroFill(_data.COMMENTS,sizeof(_data.COMMENTS));

   if (fSearchOption==FIND_FIRST)
   {
       strncpy(_srctype,data.SOURCETYPE, IMS_SOURCETYPE);
	   _sourceid = data.SOURCEID;
	   if (flagAll == 1) {
             local  = _localIMS->BindedStmt(SQL_TRANS_RESERVE2);              //4/29/2003 (AMP):  reassign the statement handle if previous statement is different from the current one
             local.BindParam(1, _srctype, IMS_SOURCETYPE);
			 local.BindParam(2, _sourceid);
	   }
	   else {
             local  = _localIMS->BindedStmt(SQL_TRANS_RESERVE);              //4/29/2003 (AMP):  reassign the statement handle if previous statement is different from the current one
		     _userid = USERID;
             local.BindParam(1, _userid);
             local.BindParam(2, _srctype, IMS_SOURCETYPE);
			 local.BindParam(3, _sourceid);
	   }


        // Bind all Colums and Parameter
         local.Bind(1,_data.TRNID);
         local.Bind(2,_data.TRNUSER);
         local.Bind(3,_data.LOTID);
         local.Bind(4,_data.TRNDATE);
         local.Bind(5,_data.TRNSTAT);
         local.Bind(6,_data.TRNQTY);
         local.Bind(7,_data.COMMENTS, 255); 
         local.Bind(8,_data.CMTDATA); 
		 local.Bind(9,_lot.USERID);
		 local.Bind(10,_lot.ETYPE, sizeof(_lot.ETYPE));
		 local.Bind(11,_lot.EID);
		 local.Bind(12,_lot.LOCID);
		 local.Bind(13,_lot.SCALEID);
		 local.Bind(14,_lot.STATUS);
		 local.Bind(15,_lot.COMMENTS, sizeof(_lot.COMMENTS));
		 local.Bind(16,_lot.SOURCE);
         local.Bind(17, _data.SOURCETYPE, IMS_SOURCETYPE);
		 local.Bind(18, _data.SOURCEID);
		 local.Bind(19, _data.RECORDID);
		 local.Bind(20, _data.PREVAMOUNT);
		 local.Bind(21,_data.PERSONID);


         local.Execute();
 //        central.Execute();
   }
 

   if (local.Fetch()) {
	   _lot.LOTID = _data.LOTID;
       lot = _lot;
      data = _data;
      ret = IMS_SUCCESS;
	  }
   else if (local.NoData())
      ret = IMS_NO_DATA;
   else 
      ret = IMS_ERROR;

   POP();


#undef SQL_TRANS_RESERVE
#undef SQL_TRANS_RESERVE2

}


//////////////////////////////////////////////////////////////////////////////
//Date created: October 11, 2004
//Description: Retrieves all reserve transactions
// Date Modified: Nov 4, 2004   ( SOURCETYPE, SOURCEID, RECORDID are added in the ITRANSACTION table)
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE IMS_getTransactionReserve(LONG flagAll,IMS_TRANSACTION *data,IMS_LOT *lot,LONG fSearchOption)
{
   static BOOL fLocal=TRUE;

   PUSH(IMS_getTransactionReserve);

   ret=getTransactionReserve(flagAll, *data,*lot,fSearchOption);
   POP();
}


//////////////////////////////////////////////////////////////////////////////
//Date Created: Sept 28, 2005
// Data Modified:  
//Description: Retrieves reserve transaction for the specified person
//////////////////////////////////////////////////////////////////////////////
LONG getReserveByPerson(IMS_TRANSACTION &data, IMS_LOT &lot, LONG fSearchOption)
{
#define SQL_TRANSDEPOSIT_PERSON "SELECT IMS_TRANSACTION.TRNID, IMS_TRANSACTION.USERID, IMS_TRANSACTION.LOTID, IMS_TRANSACTION.TRNDATE, \
	             IMS_TRANSACTION.TRNSTAT, IMS_TRANSACTION.TRNQTY, IMS_TRANSACTION.COMMENTS, IMS_TRANSACTION.CMTDATA, \
                 IMS_LOT.USERID, IMS_LOT.ETYPE, IMS_LOT.EID, IMS_LOT.LOCID, IMS_LOT.SCALEID, IMS_LOT.STATUS, IMS_LOT.COMMENTS, IMS_LOT.SOURCEID, \
                 IMS_TRANSACTION.SOURCETYPE, IMS_TRANSACTION.SOURCEID, IMS_TRANSACTION.RECORDID, IMS_TRANSACTION.PREVAMOUNT, IMS_TRANSACTION.PERSONID \
                 FROM IMS_TRANSACTION, IMS_LOT \
				 WHERE  IMS_TRANSACTION.LOTID = IMS_LOT.LOTID AND IMS_TRANSACTION.TRNSTAT=0 and IMS_TRANSACTION.TRNQTY < 0 AND IMS_TRANSACTION.PERSONID =? "
   PUSH(getDepositByPerson);

static CODBCbindedStmt &local= _localIMS->BindedStmt(SQL_TRANSDEPOSIT_PERSON);  ; 
static string strSQL="", prevstrSQL="";
static BOOL changed=TRUE;
static BOOL first_time=TRUE,fOK;
static IMS_TRANSACTION _data;
static IMS_LOT _lot;
static LONG _personid=0;

   ZeroFill(_data.COMMENTS,sizeof(_data.COMMENTS));

   if (fSearchOption==FIND_FIRST)
   {
 	    _personid = data.PERSONID;
        local.BindParam(1, _personid);

          // Bind all Colums and Parameter
         local.Bind(1,_data.TRNID);
         local.Bind(2,_data.TRNUSER);
         local.Bind(3,_data.LOTID);
         local.Bind(4,_data.TRNDATE);
         local.Bind(5,_data.TRNSTAT);
         local.Bind(6,_data.TRNQTY);
         local.Bind(7,_data.COMMENTS, 255); 
         local.Bind(8,_data.CMTDATA); 
		 local.Bind(9,_lot.USERID);
		 local.Bind(10,_lot.ETYPE, sizeof(_lot.ETYPE));
		 local.Bind(11,_lot.EID);
		 local.Bind(12,_lot.LOCID);
		 local.Bind(13,_lot.SCALEID);
		 local.Bind(14,_lot.STATUS);
		 local.Bind(15,_lot.COMMENTS, sizeof(_lot.COMMENTS));
		 local.Bind(16,_lot.SOURCE);
         local.Bind(17, _data.SOURCETYPE, IMS_SOURCETYPE);
		 local.Bind(18, _data.SOURCEID);
		 local.Bind(19, _data.RECORDID);
         local.Bind(20, _data.PREVAMOUNT);
		 local.Bind(21, _data.PERSONID);
         local.Execute();
   }
 

   if (local.Fetch()) {
	   _lot.LOTID = _data.LOTID;
       lot = _lot;
       data = _data;
       ret = IMS_SUCCESS;
	  }
   else if (local.NoData())
      ret = IMS_NO_DATA;
   else 
      ret = IMS_ERROR;

   POP();

#undef SQL_TRANS_DEPOSIT2

}


//////////////////////////////////////////////////////////////////////////////
//Date created: September, 29, 2005
//Description: Retrieves all reserve transactions by requestor (person)
// Input: PERSONID
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE IMS_getReserveByPerson(IMS_TRANSACTION *data,IMS_LOT *lot,LONG fSearchOption)
{
   static BOOL fLocal=TRUE;

   PUSH(IMS_getReserveByPerson);

   ret=getReserveByPerson( *data,*lot,fSearchOption);
   POP();
}

//////////////////////////////////////////////////////////////////////////////
//Depracated:  Mar 2005
//////////////////////////////////////////////////////////////////////////////
//LONG getTransactionDeposit(LONG flagAll, IMS_TRANSACTION &data, IMS_LOT &lot, LONG fSearchOption)
//{
//#define SQL_TRANS_DEPOSIT "SELECT IMS_TRANSACTION.TRNID, IMS_TRANSACTION.USERID, IMS_TRANSACTION.LOTID, IMS_TRANSACTION.TRNDATE, \
//	             IMS_TRANSACTION.TRNSTAT, IMS_TRANSACTION.TRNQTY, IMS_TRANSACTION.COMMENTS, IMS_TRANSACTION.CMTDATA, \
//                 IMS_LOT.USERID, IMS_LOT.ETYPE, IMS_LOT.EID, IMS_LOT.LOCID, IMS_LOT.SCALEID, IMS_LOT.STATUS, IMS_LOT.COMMENTS, IMS_LOT.SOURCEID \
//                 FROM IMS_TRANSACTION, IMS_LOT \
//				 WHERE  IMS_TRANSACTION.LOTID = IMS_LOT.LOTID AND IMS_TRANSACTION.TRNSTAT=0 and IMS_TRANSACTION.TRNQTY > 0 AND IMS_TRANSACTION.USERID =?  "
//#define SQL_TRANS_DEPOSIT2 "SELECT IMS_TRANSACTION.TRNID, IMS_TRANSACTION.USERID, IMS_TRANSACTION.LOTID, IMS_TRANSACTION.TRNDATE, \
//	             IMS_TRANSACTION.TRNSTAT, IMS_TRANSACTION.TRNQTY, IMS_TRANSACTION.COMMENTS, IMS_TRANSACTION.CMTDATA, \
//                 IMS_LOT.USERID, IMS_LOT.ETYPE, IMS_LOT.EID, IMS_LOT.LOCID, IMS_LOT.SCALEID, IMS_LOT.STATUS, IMS_LOT.COMMENTS, IMS_LOT.SOURCEID \
//                 FROM IMS_TRANSACTION, IMS_LOT \
//				 WHERE  IMS_TRANSACTION.LOTID = IMS_LOT.LOTID AND IMS_TRANSACTION.TRNSTAT=0 and IMS_TRANSACTION.TRNQTY > 0   "
//   PUSH(getTransactionDeposit);
//
//static CODBCbindedStmt &local= _localIMS->BindedStmt(SQL_TRANS_DEPOSIT);  ; 
//static string strSQL="", prevstrSQL="";
//static BOOL changed=TRUE;
//static BOOL first_time=TRUE,fOK;
//static IMS_TRANSACTION _data;
//static IMS_LOT _lot;
//static LONG _userid=0;
//
//
//   ZeroFill(_data.COMMENTS,sizeof(_data.COMMENTS));
//
//   if (fSearchOption==FIND_FIRST)
//   {
// 
//	   if (flagAll == 1) {
//             local  = _localIMS->BindedStmt(SQL_TRANS_DEPOSIT2);              //4/29/2003 (AMP):  reassign the statement handle if previous statement is different from the current one
//	   }
//	   else {
//             local  = _localIMS->BindedStmt(SQL_TRANS_DEPOSIT);              //4/29/2003 (AMP):  reassign the statement handle if previous statement is different from the current one
//		     _userid = USERID;
//             local.BindParam(1, _userid);
//	   }
//
//          // Bind all Colums and Parameter
//         local.Bind(1,_data.TRNID);
//         local.Bind(2,_data.TRNUSER);
//         local.Bind(3,_data.LOTID);
//         local.Bind(4,_data.TRNDATE);
//         local.Bind(5,_data.TRNSTAT);
//         local.Bind(6,_data.TRNQTY);
//         local.Bind(7,_data.COMMENTS, 255); 
//         local.Bind(8,_data.CMTDATA); 
//		 local.Bind(9,_lot.USERID);
//		 local.Bind(10,_lot.ETYPE, sizeof(_lot.ETYPE));
//		 local.Bind(11,_lot.EID);
//		 local.Bind(12,_lot.LOCID);
//		 local.Bind(13,_lot.SCALEID);
//		 local.Bind(14,_lot.STATUS);
//		 local.Bind(15,_lot.COMMENTS, sizeof(_lot.COMMENTS));
//		 local.Bind(16,_lot.SOURCE);
//
//
//         local.Execute();
// //        central.Execute();
//   }
// 
//
//   if (local.Fetch()) {
//	   _lot.LOTID = _data.LOTID;
//       lot = _lot;
//      data = _data;
//      ret = IMS_SUCCESS;
//	  }
//   else if (local.NoData())
//      ret = IMS_NO_DATA;
//   else 
//      ret = IMS_ERROR;
//
//   POP();
//
//
//#undef SQL_TRANS_DEPOSIT
//#undef SQL_TRANS_DEPOSIT2
//
//}


//////////////////////////////////////////////////////////////////////////////
//Date Created: Nov 4, 2004
// Data Modified:  March 11, 2005
//Description: Retrieves deposit transaction for the current user, Sourcetype (List or Study), Sourceid (Listid or studyid)
//////////////////////////////////////////////////////////////////////////////
LONG getTransactionDeposit(LONG flagAll, IMS_TRANSACTION &data, IMS_LOT &lot, LONG fSearchOption)
{
#define SQL_TRANS_DEPOSIT "SELECT IMS_TRANSACTION.TRNID, IMS_TRANSACTION.USERID, IMS_TRANSACTION.LOTID, IMS_TRANSACTION.TRNDATE, \
	             IMS_TRANSACTION.TRNSTAT, IMS_TRANSACTION.TRNQTY, IMS_TRANSACTION.COMMENTS, IMS_TRANSACTION.CMTDATA, \
                 IMS_LOT.USERID, IMS_LOT.ETYPE, IMS_LOT.EID, IMS_LOT.LOCID, IMS_LOT.SCALEID, IMS_LOT.STATUS, IMS_LOT.COMMENTS, IMS_LOT.SOURCEID, \
                 IMS_TRANSACTION.SOURCETYPE, IMS_TRANSACTION.SOURCEID, IMS_TRANSACTION.RECORDID, IMS_TRANSACTION.PREVAMOUNT, IMS_TRANSACTION.PERSONID \
                 FROM IMS_TRANSACTION, IMS_LOT \
				 WHERE  IMS_TRANSACTION.LOTID = IMS_LOT.LOTID AND IMS_TRANSACTION.TRNSTAT=0 and IMS_TRANSACTION.TRNQTY > 0 AND IMS_TRANSACTION.USERID =? \
				 AND IMS_TRANSACTION.SOURCETYPE= ? AND IMS_TRANSACTION.SOURCEID =? "
#define SQL_TRANS_DEPOSIT2 "SELECT IMS_TRANSACTION.TRNID, IMS_TRANSACTION.USERID, IMS_TRANSACTION.LOTID, IMS_TRANSACTION.TRNDATE, \
	             IMS_TRANSACTION.TRNSTAT, IMS_TRANSACTION.TRNQTY, IMS_TRANSACTION.COMMENTS, IMS_TRANSACTION.CMTDATA, \
                 IMS_LOT.USERID, IMS_LOT.ETYPE, IMS_LOT.EID, IMS_LOT.LOCID, IMS_LOT.SCALEID, IMS_LOT.STATUS, IMS_LOT.COMMENTS, IMS_LOT.SOURCEID, \
                 IMS_TRANSACTION.SOURCETYPE, IMS_TRANSACTION.SOURCEID, IMS_TRANSACTION.RECORDID, IMS_TRANSACTION.PREVAMOUNT, IMS_TRANSACTION.PERSONID \
                 FROM IMS_TRANSACTION, IMS_LOT \
				 WHERE  IMS_TRANSACTION.LOTID = IMS_LOT.LOTID AND IMS_TRANSACTION.TRNSTAT=0 and IMS_TRANSACTION.TRNQTY > 0 \
				 AND IMS_TRANSACTION.SOURCETYPE= ? AND IMS_TRANSACTION.SOURCEID =? "
   PUSH(getTransactionDeposit2);

static CODBCbindedStmt &local= _localIMS->BindedStmt(SQL_TRANS_DEPOSIT2);  ; 
static string strSQL="", prevstrSQL="";
static BOOL changed=TRUE;
static BOOL first_time=TRUE,fOK;
static IMS_TRANSACTION _data;
static IMS_LOT _lot;
static LONG _userid=0, _sourceid;
static char _srctype[IMS_SOURCETYPE];

   ZeroFill(_data.COMMENTS,sizeof(_data.COMMENTS));

   if (fSearchOption==FIND_FIRST)
   {
       strncpy(_srctype,data.SOURCETYPE, IMS_SOURCETYPE);
	   _sourceid = data.SOURCEID;
	   if (flagAll == 1) {
             local  = _localIMS->BindedStmt(SQL_TRANS_DEPOSIT2);              //4/29/2003 (AMP):  reassign the statement handle if previous statement is different from the current one
             local.BindParam(1, _srctype, IMS_SOURCETYPE);
			 local.BindParam(2, _sourceid);
	   }
	   else {
             local  = _localIMS->BindedStmt(SQL_TRANS_DEPOSIT);              //4/29/2003 (AMP):  reassign the statement handle if previous statement is different from the current one
		     _userid = USERID;
             local.BindParam(1, _userid);
             local.BindParam(2, _srctype, IMS_SOURCETYPE);
			 local.BindParam(3, _sourceid);
	   }

          // Bind all Colums and Parameter
         local.Bind(1,_data.TRNID);
         local.Bind(2,_data.TRNUSER);
         local.Bind(3,_data.LOTID);
         local.Bind(4,_data.TRNDATE);
         local.Bind(5,_data.TRNSTAT);
         local.Bind(6,_data.TRNQTY);
         local.Bind(7,_data.COMMENTS, 255); 
         local.Bind(8,_data.CMTDATA); 
		 local.Bind(9,_lot.USERID);
		 local.Bind(10,_lot.ETYPE, sizeof(_lot.ETYPE));
		 local.Bind(11,_lot.EID);
		 local.Bind(12,_lot.LOCID);
		 local.Bind(13,_lot.SCALEID);
		 local.Bind(14,_lot.STATUS);
		 local.Bind(15,_lot.COMMENTS, sizeof(_lot.COMMENTS));
		 local.Bind(16,_lot.SOURCE);
         local.Bind(17, _data.SOURCETYPE, IMS_SOURCETYPE);
		 local.Bind(18, _data.SOURCEID);
		 local.Bind(19, _data.RECORDID);
         local.Bind(20, _data.PREVAMOUNT);
		 local.Bind(21, _data.PERSONID);
         local.Execute();
 //        central.Execute();
   }
 

   if (local.Fetch()) {
	   _lot.LOTID = _data.LOTID;
       lot = _lot;
      data = _data;
      ret = IMS_SUCCESS;
	  }
   else if (local.NoData())
      ret = IMS_NO_DATA;
   else 
      ret = IMS_ERROR;

   POP();

#undef SQL_TRANSDEPOSIT_PERSON

}




//////////////////////////////////////////////////////////////////////////////
//Date created: Spetember 29, 2005
//Description: Retrieves all deposit transactions by requestor
// Input: PERSONID
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE IMS_getTransactionDeposit(LONG flagAll,IMS_TRANSACTION *data,IMS_LOT *lot,LONG fSearchOption)
{
   static BOOL fLocal=TRUE;

   PUSH(IMS_getTransactionDeposit);

   ret=getTransactionDeposit(flagAll, *data,*lot,fSearchOption);
   POP();
}



//////////////////////////////////////////////////////////////////////////////
//Date Created: Sept 28, 2005
// Data Modified:  
//Description: Retrieves deposit transaction for the specified person
//////////////////////////////////////////////////////////////////////////////
LONG getDepositByPerson(IMS_TRANSACTION &data, IMS_LOT &lot, LONG fSearchOption)
{
#define SQL_TRANSDEPOSIT_PERSON "SELECT IMS_TRANSACTION.TRNID, IMS_TRANSACTION.USERID, IMS_TRANSACTION.LOTID, IMS_TRANSACTION.TRNDATE, \
	             IMS_TRANSACTION.TRNSTAT, IMS_TRANSACTION.TRNQTY, IMS_TRANSACTION.COMMENTS, IMS_TRANSACTION.CMTDATA, \
                 IMS_LOT.USERID, IMS_LOT.ETYPE, IMS_LOT.EID, IMS_LOT.LOCID, IMS_LOT.SCALEID, IMS_LOT.STATUS, IMS_LOT.COMMENTS, IMS_LOT.SOURCEID, \
                 IMS_TRANSACTION.SOURCETYPE, IMS_TRANSACTION.SOURCEID, IMS_TRANSACTION.RECORDID, IMS_TRANSACTION.PREVAMOUNT, IMS_TRANSACTION.PERSONID \
                 FROM IMS_TRANSACTION, IMS_LOT \
				 WHERE  IMS_TRANSACTION.LOTID = IMS_LOT.LOTID AND IMS_TRANSACTION.TRNSTAT=0 and IMS_TRANSACTION.TRNQTY > 0 AND IMS_TRANSACTION.PERSONID =? "
   PUSH(getDepositByPerson);

static CODBCbindedStmt &local= _localIMS->BindedStmt(SQL_TRANSDEPOSIT_PERSON);  ; 
static string strSQL="", prevstrSQL="";
static BOOL changed=TRUE;
static BOOL first_time=TRUE,fOK;
static IMS_TRANSACTION _data;
static IMS_LOT _lot;
static LONG _personid=0;

   ZeroFill(_data.COMMENTS,sizeof(_data.COMMENTS));

   if (fSearchOption==FIND_FIRST)
   {
 	    _personid = data.PERSONID;
        local.BindParam(1, _personid);

          // Bind all Colums and Parameter
         local.Bind(1,_data.TRNID);
         local.Bind(2,_data.TRNUSER);
         local.Bind(3,_data.LOTID);
         local.Bind(4,_data.TRNDATE);
         local.Bind(5,_data.TRNSTAT);
         local.Bind(6,_data.TRNQTY);
         local.Bind(7,_data.COMMENTS, 255); 
         local.Bind(8,_data.CMTDATA); 
		 local.Bind(9,_lot.USERID);
		 local.Bind(10,_lot.ETYPE, sizeof(_lot.ETYPE));
		 local.Bind(11,_lot.EID);
		 local.Bind(12,_lot.LOCID);
		 local.Bind(13,_lot.SCALEID);
		 local.Bind(14,_lot.STATUS);
		 local.Bind(15,_lot.COMMENTS, sizeof(_lot.COMMENTS));
		 local.Bind(16,_lot.SOURCE);
         local.Bind(17, _data.SOURCETYPE, IMS_SOURCETYPE);
		 local.Bind(18, _data.SOURCEID);
		 local.Bind(19, _data.RECORDID);
         local.Bind(20, _data.PREVAMOUNT);
		 local.Bind(21, _data.PERSONID);
         local.Execute();
   }
 

   if (local.Fetch()) {
	   _lot.LOTID = _data.LOTID;
       lot = _lot;
       data = _data;
       ret = IMS_SUCCESS;
	  }
   else if (local.NoData())
      ret = IMS_NO_DATA;
   else 
      ret = IMS_ERROR;

   POP();

#undef SQL_TRANS_DEPOSIT2

}


//////////////////////////////////////////////////////////////////////////////
//Date created: September, 29, 2005
//Description: Retrieves all deposit transactions by requestor (person)
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE IMS_getDepositByPerson(IMS_TRANSACTION *data,IMS_LOT *lot,LONG fSearchOption)
{
   static BOOL fLocal=TRUE;

   PUSH(IMS_getDepositByPerson);

   ret=getDepositByPerson( *data,*lot,fSearchOption);
   POP();
}


//////////////////////////////////////////////////////////////////////////////
//Date Created: Nov 4, 2004
// Data Modified:  March 11, 2005
//Description: Retrieves deposit transaction for the specified source
//////////////////////////////////////////////////////////////////////////////
LONG getTransactionSource(LONG flagAll, IMS_TRANSACTION &data, IMS_LOT &lot, LONG fSearchOption)
{
#define SQL_TRANS_SOURCE "SELECT IMS_TRANSACTION.TRNID, IMS_TRANSACTION.USERID, IMS_TRANSACTION.LOTID, IMS_TRANSACTION.TRNDATE, \
	                IMS_TRANSACTION.TRNSTAT, IMS_TRANSACTION.TRNQTY, IMS_TRANSACTION.COMMENTS, IMS_TRANSACTION.CMTDATA, \
                    IMS_LOT.USERID, IMS_LOT.ETYPE, IMS_LOT.EID, IMS_LOT.LOCID, IMS_LOT.SCALEID, IMS_LOT.STATUS, IMS_LOT.COMMENTS, IMS_LOT.SOURCEID, \
                    IMS_TRANSACTION.SOURCETYPE, IMS_TRANSACTION.SOURCEID, IMS_TRANSACTION.RECORDID, IMS_TRANSACTION.PERSONID \
                 FROM IMS_TRANSACTION, LOT \
				 WHERE  IMS_TRANSACTION.LOTID = IMS_LOT.LOTID AND IMS_TRANSACTION.TRNSTAT=0 and IMS_TRANSACTION.TRNQTY > 0 AND \
				    IMS_TRANSACTION.SOURCETYPE=? AND IMS_TRANSACTION.SOURCEID = ?  "
   PUSH(getTransactionSource);

static CODBCbindedStmt &local= _localIMS->BindedStmt(SQL_TRANS_SOURCE);  ; 
static string strSQL="", prevstrSQL="";
static BOOL changed=TRUE;
static BOOL first_time=TRUE,fOK;
static IMS_TRANSACTION _data;
static IMS_LOT _lot;
static LONG _userid=0;


   ZeroFill(_data.COMMENTS,sizeof(_data.COMMENTS));

   if (fSearchOption==FIND_FIRST)
   {
 
          // Bind all Colums and Parameter
         local.Bind(1,_data.TRNID);
         local.Bind(2,_data.TRNUSER);
         local.Bind(3,_data.LOTID);
         local.Bind(4,_data.TRNDATE);
         local.Bind(5,_data.TRNSTAT);
         local.Bind(6,_data.TRNQTY);
         local.Bind(7,_data.COMMENTS, 255); 
         local.Bind(8,_data.CMTDATA); 
		 local.Bind(9,_lot.USERID);
		 local.Bind(10,_lot.ETYPE, sizeof(_lot.ETYPE));
		 local.Bind(11,_lot.EID);
		 local.Bind(12,_lot.LOCID);
		 local.Bind(13,_lot.SCALEID);
		 local.Bind(14,_lot.STATUS);
		 local.Bind(15,_lot.COMMENTS, sizeof(_lot.COMMENTS));
		 local.Bind(16,_lot.SOURCE);
         local.Bind(17, _data.SOURCETYPE, IMS_SOURCETYPE);
		 local.Bind(18, _data.SOURCEID);
		 local.Bind(19, _data.RECORDID);
		 local.Bind(20,_data.PERSONID);

         local.BindParam(1, _data.SOURCETYPE, IMS_SOURCETYPE);
		 local.BindParam(2, _data.SOURCEID);
         local.Execute();
 //        central.Execute();
   }
 

   if (local.Fetch()) {
	   _lot.LOTID = _data.LOTID;
       lot = _lot;
      data = _data;
      ret = IMS_SUCCESS;
	  }
   else if (local.NoData())
      ret = IMS_NO_DATA;
   else 
      ret = IMS_ERROR;

   POP();

#undef SQL_TRANS_SOURCE

}


//////////////////////////////////////////////////////////////////////////////
//Date created: Nov 4, 2004
//Description: Retrieves all ransactions for the specified source
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE IMS_getTransactionSource(LONG flagAll,IMS_TRANSACTION *data,IMS_LOT *lot,LONG fSearchOption)
{
   static BOOL fLocal=TRUE;

   PUSH(IMS_getTransactionSource);

   ret=getTransactionSource(flagAll, *data,*lot,fSearchOption);
   POP();
}


//////////////////////////////////////////////////////////////////////////////
// Description:  Updates the ITRANSACTION table except LOTID and TRNID
//////////////////////////////////////////////////////////////////////////////
LONG setTransaction( IMS_TRANSACTION trans)
{
#define SQL_SETTRANSLOT "\
      UPDATE IMS_TRANSACTION SET USERID=?,TRNDATE=?, TRNSTAT=?, TRNQTY=?,COMMENTS=?,CMTDATA=? \
      WHERE TRNID=? "     //LOTID=?,

   PUSH(setTransactionStatus);

static CODBCbindedStmt &local  = _localIMS->BindedStmt(SQL_SETTRANSLOT);

static IMS_TRANSACTION _data;
static BOOL first_time=TRUE;

   _data = trans;
   
   if (first_time)
   {
      local.BindParam(1,_data.TRNUSER);
//      local.BindParam(2,_data.LOTID);
      local.BindParam(2,_data.TRNDATE);
      local.BindParam(3,_data.TRNSTAT);
      local.BindParam(4,_data.TRNQTY);
      local.BindParam(5,_data.COMMENTS,sizeof(_data.COMMENTS)-1); 
      local.BindParam(6,_data.CMTDATA); 
      local.BindParam(7,_data.TRNID);
   }
    
   if (local.Execute())
      ret=IMS_SUCCESS;
   else
      ret=IMS_ERROR;

   POP();

#undef SQL_SETTRANSLOT
}


//////////////////////////////////////////////////////////////////////////////
// Date Created: Nov 4, 2004
// Data Modified:  March 11, 2005
// Description:  Updates the ITRANSACTION table except LOTID and TRNID
//////////////////////////////////////////////////////////////////////////////
LONG setTransaction2( IMS_TRANSACTION trans)
{
#define SQL_SETTRANSLOT2 "\
      UPDATE IMS_TRANSACTION SET USERID=?,TRNDATE=?, TRNSTAT=?, TRNQTY=?,   \
	         COMMENTS=?,CMTDATA=?, SOURCETYPE=?, SOURCEID=?, RECORDID=?, PREVAMOUNT=?, PERSONID=?    \
      WHERE TRNID=? AND LOTID=? "

   PUSH(setTransaction2);

static CODBCbindedStmt &local  = _localIMS->BindedStmt(SQL_SETTRANSLOT2);

static IMS_TRANSACTION _data;
static BOOL first_time=TRUE;

   _data = trans;
   
   if (first_time)
   {
      local.BindParam(1,_data.TRNUSER);
      local.BindParam(2,_data.TRNDATE);
      local.BindParam(3,_data.TRNSTAT);
      local.BindParam(4,_data.TRNQTY);
      local.BindParam(5,_data.COMMENTS,sizeof(_data.COMMENTS)-1); 
      local.BindParam(6,_data.CMTDATA); 
      local.BindParam(7,_data.SOURCETYPE,IMS_SOURCETYPE);
      local.BindParam(8,_data.SOURCEID);
      local.BindParam(9,_data.RECORDID);
	  local.BindParam(10, _data.PREVAMOUNT);
 	  local.BindParam(11,_data.PERSONID);
      local.BindParam(12,_data.TRNID);
      local.BindParam(13,_data.LOTID);
	  first_time = FALSE;
   }
    
   if (local.Execute())
      ret=IMS_SUCCESS;
   else
      ret=IMS_ERROR;

   POP();

#undef SQL_SETTRANSLOT2
}



//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE IMS_setTransaction(IMS_TRANSACTION* trans)
{
   PUSH(IMS_setTransaction);
   ret = setTransaction2(*trans);
   POP();
}



//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG calculateBalance(BOOL fLocal, LONG lotid, DOUBLE *tranqty)
{
#define SQL_CALCBALANCE "\
         SELECT Sum(TRNQTY)   FROM IMS_TRANSACTION \
         WHERE ((TRNSTAT=1) Or (TRNSTAT=0))  AND (LOTID=?)"
	
	PUSH(setTransactionStatus);

static CODBCbindedStmt &local  = _localIMS->BindedStmt(SQL_CALCBALANCE);

static LONG _id;
static DOUBLE _qty=0;
static BOOL first_time=TRUE;

   _id = lotid;
   _qty=0;
   if (first_time)
   {
      local.BindParam(1,_id);
	  local.Bind(1, _qty);
	  first_time = FALSE;
   }
   local.Execute();
   if (local.Fetch()) {
      *tranqty = _qty;
      ret=IMS_SUCCESS;
	  }
   else
      ret=IMS_ERROR;

   POP();

#undef SQL_CALCBALANCE
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG actualBalance(BOOL fLocal, LONG lotid, DOUBLE *tranqty)
{
#define SQL_ACTUALBALANCE "\
         SELECT Sum(TRNQTY)   FROM IMS_TRANSACTION \
         WHERE (TRNSTAT=1) AND (LOTID=?)"
	
	PUSH(setTransactionStatus);

static CODBCbindedStmt &local  = _localIMS->BindedStmt(SQL_ACTUALBALANCE);

static LONG _id;
static DOUBLE _qty=0;
static BOOL first_time=TRUE;

   _id = lotid;
   _qty=0;
   if (first_time)
   {
      local.BindParam(1,_id);
	  local.Bind(1, _qty);
	  first_time = FALSE;
   }
   local.Execute();

   if (local.Fetch()) {
      *tranqty = _qty;
      ret=IMS_SUCCESS;
	  }
   else
      ret=IMS_ERROR;

   POP();

#undef SQL_ACTUALBALANCE
}


//////////////////////////////////////////////////////////////////////////////
// IMS_calculateBalance - gets all values from field TRNQTY of the ITRANSACTION table for a given LotID
// Input: lotid
// Output: tranqty1 - available balance (committed & anticipated), trnqty2 - actual balance (committed)
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE IMS_calculateBalance(LONG lotid, DOUBLE *trnqty1, DOUBLE *trnqyt2)
{
   LONG ret1,ret2;
   PUSH(IMS_calculateBalance);
   ret1 = calculateBalance(LOCAL,lotid,trnqty1);
   ret2 = actualBalance(LOCAL,lotid,trnqyt2);
   ret = ret1 && ret2;
   POP();
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG getIMSLocation(BOOL fLocal, IMS_Location &data, LONG fSearchOption)
{
#define SQL_GETIMSLOCATION "\
   select LTYPE,NLLP,\
     LRPLCE,LNAME,LABBR,SNL3ID,SNL2ID,SNL1ID,CNTRYID\
   from IMS_LOCATION \
   where LOCID=?\
   order by LOCID"

   PUSH(getIMSLocation);


static CODBCbindedStmt &local  = _localIMS->BindedStmt(SQL_GETIMSLOCATION);
       CODBCbindedStmt *source;

static IMS_Location _data;
static BOOL first_time=TRUE;

static LONG locid;

   if (fSearchOption==FIND_FIRST)
   {
      if (first_time)
      {
         local.Bind(1,_data.ltype);
         local.Bind(2,_data.nllp);
         local.Bind(3,_data.lrplce);
         local.Bind(4,_data.lname,GMS_LNAME_SIZE);
         local.Bind(5,_data.labbr,GMS_LABBR_SIZE);
         local.Bind(6,_data.snl3id);
         local.Bind(7,_data.snl2id);
         local.Bind(8,_data.snl1id);
         local.Bind(9,_data.cntryid);
	     local.BindParam(1,locid);
 
         first_time=FALSE;
      }

	   locid=data.locid;
      local.Execute();
 //     central.Execute();
   }

 // source = (fLocal)?&local:&central;
   source = &local;

   ZeroMemory(&_data,sizeof(_data));
   if (source->Fetch())
   {
      data = _data;
      data.locid = locid;
   }
   else if (source->NoData())
      ret = GMS_NO_DATA;
   else 
      ret = GMS_ERROR;

   POP();

#undef SQL_GETLOCATION
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE IMS_getLocation(IMS_Location *data, LONG fSearchOption)
{
   static BOOL fLocal=TRUE;

   PUSH(GMS_getLocation);
   if (fSearchOption==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal){
      ret=getIMSLocation(LOCAL,*data,fSearchOption);
//      if (ret!=GMS_SUCCESS){
//         fLocal = FALSE; 
//         ret=getIMSLocation(CENTRAL,*data,FIND_NEXT);
//      }
   }
//   else
//      ret=getLocation(CENTRAL,*data,fSearchOption);
   POP();
}




//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG	getIMSLocation2(BOOL fLocal,IMS_Location &data, LONG fSearchOption)
{
#define SQL_GETIMSLOCATION2 "\
   select LOCID, LTYPE,NLLP,\
     LRPLCE,LNAME,LABBR,SNL3ID,SNL2ID,SNL1ID,CNTRYID\
   from IMS_LOCATION where"

   PUSH(getIMSLocation2);

//static CODBCbindedStmt &central= _centralDBC->BindedStmt();
static CODBCbindedStmt &local  = _localIMS->BindedStmt();
       CODBCbindedStmt *source;

static IMS_Location _data;

string strSQL="";
static BOOL changed=TRUE;
static BOOL first_time=TRUE,fOK;
static LONG locid,ltype,nllp,lrplce,snl3id,snl2id,snl1id,cntryid;
static BOOL flocid,fltype,fnllp,flrplce,fsnl3id,fsnl2id,fsnl1id,fcntryid;

   if (fSearchOption==FIND_FIRST)
   {
      changed = Changed(flocid  ,(locid  =data.locid)  !=0) |
                Changed(fltype  ,(ltype  =data.ltype)  !=0) |
                Changed(fnllp   ,(nllp   =data.nllp)   !=0) |
                Changed(flrplce ,(lrplce =data.lrplce) !=0) |
                Changed(fsnl3id ,(snl3id =data.snl3id) !=0) |
                Changed(fsnl2id ,(snl2id =data.snl2id) !=0) |
                Changed(fsnl1id ,(snl1id =data.snl1id) !=0) |
                Changed(fcntryid,(cntryid=data.cntryid)!=0);

      if (first_time)
      {
         changed=TRUE;
         first_time=FALSE;
      }

      if (changed)
      {
         strSQL=SQL_GETIMSLOCATION2;
         if (flocid)
            strSQL = strSQL+" AND locid=?";
         if (fltype)
            strSQL = strSQL+" AND ltype=?";
         if (fnllp)
            strSQL = strSQL+" AND nllp=?";
         if (flrplce)
            strSQL = strSQL+" AND lrplce=?";
         if (fsnl3id)
            strSQL = strSQL+" AND snl3id=?";
         if (fsnl2id)
            strSQL = strSQL+" AND snl2id=?";
         if (fsnl1id)
            strSQL = strSQL+" AND snl1id=?";
         if (fcntryid)
            strSQL = strSQL+" AND cntryid=?";
         if (!(flocid||fltype||fnllp||flrplce||fsnl3id||fsnl2id||fsnl1id||fcntryid))
         {
            POP2(GMS_ERROR);
         }
         strSQL.erase(strSQL.find(" AND ",0),4);
         strSQL+= " order by LOCID";

 //        central = _centralDBC->BindedStmt((LPCSTR)strSQL.c_str());     //AMP:  added
         local   = _localIMS->BindedStmt((LPCSTR)strSQL.c_str());       //AMP:  added

 

         local.Bind(1,_data.locid  );
         local.Bind(2,_data.ltype  );
         local.Bind(3,_data.nllp   );
         local.Bind(4,_data.lrplce );
         local.Bind(5,_data.lname,GMS_LNAME_SIZE);
         local.Bind(6,_data.labbr,GMS_LABBR_SIZE);
         local.Bind(7,_data.snl3id );
         local.Bind(8,_data.snl2id );
         local.Bind(9,_data.snl1id );
         local.Bind(10,_data.cntryid);

         int i=1;
         if (flocid)
         {
            local.BindParam(i,locid);i+=1;
         }
         if (fltype)
         {
            local.BindParam(i,ltype);i+=1;
         }
         if (fnllp)
         {
            local.BindParam(i,nllp);i+=1;
         }
         if (flrplce)
         {
            local.BindParam(i,lrplce);i+=1;
         }
         if (fsnl3id)
         {
            local.BindParam(i,snl3id);i+=1;
         }
         if (fsnl2id)
         {
            local.BindParam(i,snl2id);i+=1;
         }
         if (fsnl1id)
         {
            local.BindParam(i,snl1id);i+=1;
         }
         if (fcntryid)
         {
            local.BindParam(i,cntryid);i+=1;
         }
      }
      locid = data.locid;
      ltype = data.ltype;
      nllp  = data.nllp;
      lrplce= data.lrplce;
      snl3id= data.snl3id;
      snl2id= data.snl2id;
      snl1id= data.snl1id;
      cntryid= data.cntryid;
      local.Execute();
//      central.Execute();
   }
 
//   source = (fLocal)?&local:&central;
   source = &local;

   ZeroMemory(&_data,sizeof(_data));
   if (source->Fetch())
      data = _data;
   else if (source->NoData())
      ret = GMS_NO_DATA;
   else 
      ret = GMS_ERROR;

   POP();

#undef SQL_GETIMSLOCATION2
}




//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE IMS_getLocation2(IMS_Location *data, LONG fSearchOption)
{
   static BOOL fLocal=TRUE;

   PUSH(IMS_getLocation2);
   if (fSearchOption==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal){
      ret=getIMSLocation2(LOCAL,*data,fSearchOption);
      if (ret!=GMS_SUCCESS){
         fLocal = FALSE; 
         ret=getIMSLocation2(CENTRAL,*data,FIND_NEXT);
      }
   }
   else
      ret=getIMSLocation2(CENTRAL,*data,fSearchOption);
   POP();
}



//////////////////////////////////////////////////////////////////////////////
//Date Created: May 30, 2005
// Data Modified:  May 30, 2005
//Description: Retrieves tota sum of TRNQTY of the given lot
//////////////////////////////////////////////////////////////////////////////
LONG getBalanceOfLot(LONG flocal, LONG lotid, double *sumTrans )
{
#define SQL_getBalanceOfLot "SELECT Sum([TRNQTY]),LOTID  \
                          FROM IMS_TRANSACTION WHERE TRNQTY<0 \
						  GROUP BY LOTID \
						  HAVING LOTID=? "

   PUSH(getBalanceOfLot);

static CODBCbindedStmt &local= _localIMS->BindedStmt(SQL_getBalanceOfLot);  ; 
static BOOL first_time=TRUE,fOK;
static LONG _lotid;
static double _sum;

   
   if (first_time)
   {
 
          // Bind all Colums and Parameter
         local.Bind(1,_sum);
         local.BindParam(1, _lotid);
   }
  _lotid = lotid;
  local.Execute();
 

   if (local.Fetch()) {
      *sumTrans = _sum;
      ret = IMS_SUCCESS;
	  }
   else if (local.NoData())
      ret = IMS_NO_DATA;
   else 
      ret = IMS_ERROR;

   POP();

#undef SQL_getBalanceOfLot

}

//////////////////////////////////////////////////////////////////////////////
//Date created: May 30, 2005
//Description: Retrieves all ransactions for the specified source
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE IMS_getBalanceOfLot(LONG lot, double *sumTrans)
{
   static BOOL fLocal=TRUE;

   PUSH(IMS_getBalanceOfLot);

   ret=getBalanceOfLot(fLocal, lot,sumTrans);
   POP();
}


//////////////////////////////////////////////////////////////////////////////
//Date Created: May 30, 2005
// Data Modified:  June 17, 2005
//Description: Retrieves sum of TRNQTY, scale and location of  lots
//////////////////////////////////////////////////////////////////////////////
LONG reportBalance(LONG flocal, LONG *lotid, double *sumTrans, CHAR *szLocname, CHAR *szScale, long fSearchOption )
{
#define SQL_getSummaryTransaction "SELECT IMS_LOT.LOTID, SUM([IMS_TRANSACTION.TRNQTY]), LOCATION.LNAME, SCALE.SCNAME \
                                   FROM IMS_TRANSACTION, IMS_LOT, LOCATION, SCALE  \
                                   WHERE  IMS_LOT.LOCID = LOCATION.LOCID AND IMS_LOT.SCALEID = SCALE.SCALEID AND IMS_TRANSACTION.LOTID = IMS_LOT.LOTID \
                                   GROUP BY IMS_LOT.LOTID, LOCATION.LNAME, SCALE.SCNAME "

   PUSH(getSummaryTransaction);

static CODBCbindedStmt &local= _localIMS->BindedStmt(SQL_getSummaryTransaction);  ; 
static BOOL first_time=TRUE,fOK;
static LONG _lotid;
static double _sum;
static char _szLocname[GMS_LNAME_SIZE];
static char _szScale[DMS_SCALE_NAME];

  
	if (fSearchOption == FIND_FIRST) {
		if (first_time)
		{
 
          // Bind all Colums and Parameter
	         local.Bind(1,_lotid);
			 local.Bind(2,_sum);
			 local.Bind(3,_szLocname, GMS_LNAME_SIZE);
	         local.Bind(4,_szScale, DMS_SCALE_NAME);
		}
		local.Execute();
	}

   if (local.Fetch()) {
      *sumTrans = _sum;
      *lotid = _lotid;
	  strcpy(szLocname, _szLocname);
	  strcpy(szScale, _szScale);
      ret = IMS_SUCCESS;
	  }
   else if (local.NoData())
      ret = IMS_NO_DATA;
   else 
      ret = IMS_ERROR;

   POP();

#undef SQL_getSummaryTransaction

}


//////////////////////////////////////////////////////////////////////////////
//Date created: June 17, 2005
//Description: Retrieves the sum of the transaction quantity, location and scale for all lots
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE IMS_reportBalance( LONG *lotid, double *sumTrans, CHAR *szLocname, CHAR *szScale, long fSearchOption )
{
   static BOOL fLocal=TRUE;

   PUSH(IMS_reportBalanace);
 
   ret=reportBalance(LOCAL, lotid, sumTrans, szLocname, szScale, fSearchOption );
   POP();
}


//////////////////////////////////////////////////////////////////////////////
//Date Created: June 17, 2005
// Data Modified:  June 17, 2005
//Description: Retrieves the transaction information
//////////////////////////////////////////////////////////////////////////////
LONG reportAllTransaction(LONG flocal, LONG *lotid, LONG *cmtdate, double *quantity, CHAR *szLocname, CHAR *szScale,CHAR *szComment, long fSearchOption )
{
#define SQL_reportAllTransaction  "SELECT IMS_TRANSACTION.LOTID, IMS_TRANSACTION.CMTDATA, IMS_TRANSACTION.TRNQTY, LOCATION.LNAME, SCALE.SCNAME, IMS_TRANSACTION.COMMENTS \
									FROM IMS_TRANSACTION, SCALE,LOCATION, IMS_LOT \
									WHERE LOCATION.LOCID = IMS_LOT.LOCID AND SCALE.SCALEID = IMS_LOT.SCALEID AND IMS_TRANSACTION.LOTID = IMS_LOT.LOTID \
									AND IMS_TRANSACTION.TRNSTAT = 0 "

   PUSH(reportAllTransaction);

static CODBCbindedStmt &local= _localIMS->BindedStmt(SQL_reportAllTransaction);  ; 
static BOOL first_time=TRUE,fOK;
static LONG _lotid, _cmtdate;
static double _sum;
static char _szLocname[GMS_LNAME_SIZE], _szScale[DMS_SCALE_NAME], _szComment[255];

   
	if (fSearchOption == FIND_FIRST) {
		if (first_time)
		{
 
          // Bind all Colums and Parameter
	         local.Bind(1,_lotid);
			 local.Bind(2,_cmtdate);
			 local.Bind(3,_sum);
	         local.Bind(4,_szLocname, GMS_LNAME_SIZE);
		     local.Bind(5,_szScale, DMS_SCALE_NAME);
			 local.Bind(6,_szComment, 255);

		}
		local.Execute();
	}
 

   if (local.Fetch()) {
      *quantity = _sum;
      *lotid = _lotid;
      *cmtdate = _cmtdate;
	  strcpy(szComment, _szComment);
	  strcpy(szLocname, _szLocname);
	  strcpy(szScale, _szScale);
      ret = IMS_SUCCESS;
	  }
   else if (local.NoData())
      ret = IMS_NO_DATA;
   else 
      ret = IMS_ERROR;

   POP();

#undef SQL_reportAllTransaction

}


//////////////////////////////////////////////////////////////////////////////
//Date created: June 17, 2005
//Description: Retrieves the sum of the transaction quantity, location and scale for all lots
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE IMS_reportAllTransaction(LONG *lotid, LONG *cmtdate, double *quantity, CHAR *szLocname, CHAR *szScale,CHAR *szComment, long fSearchOption )
{
   static BOOL fLocal=TRUE;

   PUSH(IMS_reportAllTransaction);
 
   ret=reportAllTransaction(LOCAL, lotid, cmtdate, quantity, szLocname, szScale,szComment, fSearchOption );
   POP();
}


//////////////////////////////////////////////////////////////////////////////
//Date Created: June 17, 2005
// Data Modified:  June 17, 2005
//Description: 
//////////////////////////////////////////////////////////////////////////////
LONG reportLotByEntity(LONG flocal, CHAR *szETYPE, LONG *lotid, LONG *eid, CHAR *szLocname, CHAR *szScale,CHAR *szComment, long fSearchOption )
{
#define SQL_reportLotByEntity  "SELECT IMS_LOT.LOTID, IMS_LOT.EID, LOCATION.LNAME, SCALE.SCNAME, IMS_LOT.COMMENTS \
								FROM IMS_LOT,SCALE, LOCATION \
								WHERE IMS_LOT.SCALEID = SCALE.SCALEID AND IMS_LOT.LOCID = LOCATION.LOCID AND IMS_LOT.STATUS=0 AND IMS_LOT.ETYPE=? \
								ORDER BY IMS_LOT.LOTID "

   PUSH(reportLotByEntity);

static CODBCbindedStmt &local= _localIMS->BindedStmt(SQL_reportLotByEntity);  ; 
static BOOL first_time=TRUE,fOK;
static LONG _lotid, _eid;
static char _szLocname[GMS_LNAME_SIZE], _szScale[DMS_SCALE_NAME], _szComment[255], _szETYPE[IMS_ETYPE];

   
   strcpy(_szETYPE,szETYPE);
   if (fSearchOption == FIND_FIRST) {
	if (first_time)
	{
 
          // Bind all Colums and Parameter
         local.Bind(1,_lotid);
		 local.Bind(2,_eid);
         local.Bind(3,_szLocname, GMS_LNAME_SIZE);
         local.Bind(4,_szScale, DMS_SCALE_NAME);
         local.Bind(5,_szComment, 255);
		 local.BindParam(1,_szETYPE,IMS_ETYPE); 
	}
	local.Execute();
   }
 

   if (local.Fetch()) {
      *lotid = _lotid;
      *eid = _eid;
	  strcpy(szComment, _szComment);
	  strcpy(szLocname, _szLocname);
	  strcpy(szScale, _szScale);
      ret = IMS_SUCCESS;
	  }
   else if (local.NoData())
      ret = IMS_NO_DATA;
   else 
      ret = IMS_ERROR;

   POP();

#undef SQL_reportLotByEntity

}

//////////////////////////////////////////////////////////////////////////////
//Date created: June 17, 2005
//Description: Retrieves the sum of the transaction quantity, location and scale for all lots
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE IMS_reportLotByEntity( CHAR *szETYPE, LONG *lotid, LONG *eid, CHAR *szLocname, CHAR *szScale,CHAR *szComment, long fSearchOption )
{
   static BOOL fLocal=TRUE;

   PUSH(IMS_reportLotByEntity);
 
   ret=reportLotByEntity(LOCAL, szETYPE, lotid, eid, szLocname, szScale, szComment, fSearchOption );
   POP();
}


//////////////////////////////////////////////////////////////////////////////
//Date Created: June 17, 2005
// Data Modified:  June 17, 2005
//Description: 
//////////////////////////////////////////////////////////////////////////////
LONG reportLotByEntityGID(LONG flocal, CHAR *szETYPE, LONG *lotid, LONG *eid, CHAR *szLocname, CHAR *szScale,CHAR *szComment, long fSearchOption )
{
#define SQL_reportLotByEntity  "SELECT IMS_LOT.LOTID, IMS_LOT.EID, LOCATION.LNAME, SCALE.SCNAME, IMS_LOT.COMMENTS \
								FROM IMS_LOT,SCALE, LOCATION \
								WHERE IMS_LOT.SCALEID = SCALE.SCALEID AND IMS_LOT.LOCID = LOCATION.LOCID AND IMS_LOT.STATUS=0 AND IMS_LOT.ETYPE=? \
								ORDER BY IMS_LOT.LOTID "

   PUSH(reportLotByEntity);

static CODBCbindedStmt &local= _localIMS->BindedStmt(SQL_reportLotByEntity);  ; 
static BOOL first_time=TRUE,fOK;
static LONG _lotid, _eid;
static char _szLocname[GMS_LNAME_SIZE], _szScale[DMS_SCALE_NAME], _szComment[255], _szETYPE[IMS_ETYPE];

   
   strcpy(_szETYPE,szETYPE);
	if (fSearchOption == FIND_FIRST) {
		if (first_time)
		{
 
          // Bind all Colums and Parameter
         local.Bind(1,_lotid);
		 local.Bind(2,_eid);
         local.Bind(3,_szLocname, GMS_LNAME_SIZE);
         local.Bind(4,_szScale, DMS_SCALE_NAME);
         local.Bind(5,_szComment, 255);
		 local.BindParam(1,_szETYPE,IMS_ETYPE); 
		}
		local.Execute();
	}
 

   if (local.Fetch()) {
      *lotid = _lotid;
      *eid = _eid;
	  strcpy(szComment, _szComment);
	  strcpy(szLocname, _szLocname);
	  strcpy(szScale, _szScale);
      ret = IMS_SUCCESS;
	  }
   else if (local.NoData())
      ret = IMS_NO_DATA;
   else 
      ret = IMS_ERROR;

   POP();

#undef SQL_reportLotByEntity

}

//////////////////////////////////////////////////////////////////////////////
//Date created: June 17, 2005
//Description: Retrieves the sum of the transaction quantity, location and scale for all lots
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE IMS_reportLotByEntityGID( CHAR *szETYPE, LONG *lotid, LONG *eid, CHAR *szLocname, CHAR *szScale,CHAR *szComment, long fSearchOption )
{
   static BOOL fLocal=TRUE;

   PUSH(IMS_reportLotByEntity);
 
   ret=reportLotByEntityGID(LOCAL, szETYPE, lotid, eid, szLocname, szScale, szComment, fSearchOption );
   POP();
}


//////////////////////////////////////////////////////////////////////////////
//Date Created: June 17, 2005
// Data Modified:  June 17, 2005
//Description: Report on Lot by Entity Type ang GID
//////////////////////////////////////////////////////////////////////////////
LONG reportLotByEntityGIDFrom(LONG flocal, CHAR *szETYPE, LONG *lotid, LONG *eid, CHAR *szLocname, CHAR *szScale,CHAR *szComment, long fSearchOption )
{
#define SQL_reportLotByEntityGIDFrom  "SELECT IMS_LOT.LOTID, IMS_LOT.EID, LOCATION.LNAME, SCALE.SCNAME, IMS_LOT.COMMENTS \
								FROM IMS_LOT,SCALE, LOCATION \
								WHERE IMS_LOT.SCALEID = SCALE.SCALEID AND IMS_LOT.LOCID = LOCATION.LOCID AND IMS_LOT.STATUS=0 AND IMS_LOT.ETYPE=? \
								AND IMS_LOT.EID =? \
								ORDER BY IMS_LOT.LOTID "

   PUSH(reportLotByEntity);

static CODBCbindedStmt &local= _localIMS->BindedStmt(SQL_reportLotByEntityGIDFrom);  
static BOOL first_time=TRUE,fOK;
static LONG _lotid, _eid;
static char _szLocname[GMS_LNAME_SIZE], _szScale[DMS_SCALE_NAME], _szComment[255], _szETYPE[IMS_ETYPE];

   
	if (fSearchOption == FIND_FIRST) {
       strcpy(_szETYPE,szETYPE);
        _eid = *eid;   
	   if (first_time)
		{
 
          // Bind all Colums and Parameter
         local.Bind(1,_lotid);
		 local.Bind(2,_eid);
         local.Bind(3,_szLocname, GMS_LNAME_SIZE);
         local.Bind(4,_szScale, DMS_SCALE_NAME);
         local.Bind(5,_szComment, 255);
		 local.BindParam(1,_szETYPE,IMS_ETYPE); 
		 local.BindParam(2,_eid); 
		}
		local.Execute();
	}
 

   if (local.Fetch()) {
      *lotid = _lotid;
      *eid = _eid;
	  strcpy(szComment, _szComment);
	  strcpy(szLocname, _szLocname);
	  strcpy(szScale, _szScale);
      ret = IMS_SUCCESS;
	  }
   else if (local.NoData())
      ret = IMS_NO_DATA;
   else 
      ret = IMS_ERROR;

   POP();

#undef SQL_reportLotByEntityGIDFrom

}



//////////////////////////////////////////////////////////////////////////////
//Date created: June 23, 2005
//Description: Report on Lot by Entity Type ang GID
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE IMS_reportLotByEntityGIDFrom( CHAR *szETYPE, LONG *lotid, LONG *eid, CHAR *szLocname, CHAR *szScale,CHAR *szComment, long fSearchOption )
{
   static BOOL fLocal=TRUE;

   PUSH(IMS_reportLotByEntityFrom);
 
   ret=reportLotByEntityGIDFrom(LOCAL, szETYPE, lotid, eid, szLocname, szScale, szComment, fSearchOption );
   POP();
}


//////////////////////////////////////////////////////////////////////////////
//Date Created: June 23, 2005
// Data Modified:  June 23, 2005
//Description: Report on Lot by Entity Type ang range of GIDs
//////////////////////////////////////////////////////////////////////////////
LONG reportLotByEntityGIDRange(LONG flocal, CHAR *szETYPE, LONG eidFrom, LONG eidTo, LONG *lotid, LONG *eid, CHAR *szLocname, CHAR *szScale,CHAR *szComment, long fSearchOption )
{
#define SQL_reportLotByEntityGIDFrom  "SELECT IMS_LOT.LOTID, IMS_LOT.EID, LOCATION.LNAME, SCALE.SCNAME, IMS_LOT.COMMENTS \
								FROM IMS_LOT,SCALE, LOCATION \
								WHERE IMS_LOT.SCALEID = SCALE.SCALEID AND IMS_LOT.LOCID = LOCATION.LOCID AND IMS_LOT.STATUS=0 AND IMS_LOT.ETYPE=? \
                                AND IMS_LOT.EID >=? AND IMS_LOT.EID <= ?\
								ORDER BY IMS_LOT.LOTID "

   PUSH(reportLotByEntity);

static CODBCbindedStmt &local= _localIMS->BindedStmt(SQL_reportLotByEntityGIDFrom);  
static BOOL first_time=TRUE,fOK;
static LONG _lotid, _eid, _eidFrom, _eidTo;
static char _szLocname[GMS_LNAME_SIZE], _szScale[DMS_SCALE_NAME], _szComment[255], _szETYPE[IMS_ETYPE];

   
	if (fSearchOption == FIND_FIRST) {
       strcpy(_szETYPE,szETYPE);
        _eid = *eid;   
		_eidFrom = eidFrom;
		_eidTo = eidTo;
	   if (first_time)
		{
 
          // Bind all Colums and Parameter
         local.Bind(1,_lotid);
		 local.Bind(2,_eid);
         local.Bind(3,_szLocname, GMS_LNAME_SIZE);
         local.Bind(4,_szScale, DMS_SCALE_NAME);
         local.Bind(5,_szComment, 255);
		 local.BindParam(1,_szETYPE,IMS_ETYPE); 
		 local.BindParam(2,_eidFrom); 
		 local.BindParam(3,_eidTo); 
		}
		local.Execute();
	}
 

   if (local.Fetch()) {
      *lotid = _lotid;
      *eid = _eid;
	  strcpy(szComment, _szComment);
	  strcpy(szLocname, _szLocname);
	  strcpy(szScale, _szScale);
      ret = IMS_SUCCESS;
	  }
   else if (local.NoData())
      ret = IMS_NO_DATA;
   else 
      ret = IMS_ERROR;

   POP();

#undef SQL_reportLotByEntityGIDFrom

}



//////////////////////////////////////////////////////////////////////////////
//Date created: June 23, 2005
//Description: Report on Lot by Entity Type ang range of GIDs
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE IMS_reportLotByEntityGIDRange( CHAR *szETYPE, LONG eidFrom, LONG eidTo, LONG *lotid,  LONG *eid, CHAR *szLocname, CHAR *szScale,CHAR *szComment, long fSearchOption )
{
   static BOOL fLocal=TRUE;

   PUSH(IMS_reportLotByEntityGIDRange);
 
   ret=reportLotByEntityGIDRange(LOCAL, szETYPE, eidFrom, eidTo, lotid, eid, szLocname, szScale, szComment, fSearchOption );
   POP();
}



//////////////////////////////////////////////////////////////////////////////
//Date Created: June 23, 2005
// Data Modified:  June 23, 2005
//Description: Report on Lot by Entity Type ang range of GIDs
//////////////////////////////////////////////////////////////////////////////
LONG reportEmptyShelves(LONG flocal, LONG *lotid, LONG *eid, CHAR *szLocname, CHAR *szScale, long fSearchOption )
{
#define SQL_reportEmptyShelves  "SELECT IMS_LOT.LOTID, IMS_LOT.EID, LOCATION.LNAME, SCALE.SCNAME \
								FROM IMS_TRANSACTION ,IMS_LOT, SCALE, LOCATION  \
								WHERE IMS_LOT.SCALEID = SCALE.SCALEID AND IMS_TRANSACTION.LOTID = IMS_LOT.LOTID AND IMS_LOT.LOCID = LOCATION.LOCID AND IMS_TRANSACTION.TRNSTAT=1 \
								GROUP BY IMS_LOT.LOTID, IMS_LOT.EID, LOCATION.LNAME, SCALE.SCNAME \
								HAVING (((Sum(IMS_TRANSACTION.TRNQTY))=0)) \
								ORDER BY IMS_LOT.LOTID "

   PUSH(reportEmptyShelves);

static CODBCbindedStmt &local= _localIMS->BindedStmt(SQL_reportEmptyShelves);  
static BOOL first_time=TRUE,fOK;
static LONG _lotid, _eid;
static char _szLocname[GMS_LNAME_SIZE], _szScale[DMS_SCALE_NAME], _szComment[255];

   
	if (fSearchOption == FIND_FIRST) {
        _eid = *eid;   
	   if (first_time)
		{
 
          // Bind all Colums and Parameter
         local.Bind(1,_lotid);
		 local.Bind(2,_eid);
         local.Bind(3,_szLocname, GMS_LNAME_SIZE);
         local.Bind(4,_szScale, DMS_SCALE_NAME);
		}
		local.Execute();
	}
 

   if (local.Fetch()) {
      *lotid = _lotid;
      *eid = _eid;
	  strcpy(szLocname, _szLocname);
	  strcpy(szScale, _szScale);
      ret = IMS_SUCCESS;
	  }
   else if (local.NoData())
      ret = IMS_NO_DATA;
   else 
      ret = IMS_ERROR;

   POP();

#undef SQL_reportEmptyShelves

}


/////////////////////////////////////////////////////////////////////////////
//Date created: June 23, 2005
//Description: Report on Lot by Entity Type ang range of GIDs
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE IMS_reportEmptyShelves( LONG *lotid,  LONG *eid, CHAR *szLocname, CHAR *szScale, long fSearchOption )
{
   static BOOL fLocal=TRUE;

   PUSH(IMS_reportEmptyShelves);
 
   ret=reportEmptyShelves(LOCAL,  lotid, eid, szLocname, szScale,  fSearchOption );
   POP();
}


//////////////////////////////////////////////////////////////////////////////
// Date Created: June 23, 2005
// Data Modified:  June 23, 2005
// Description: Report on Lot by Entity Type ang range of GIDs
//////////////////////////////////////////////////////////////////////////////
LONG reportMaterialsDist(LONG flocal, LONG *lotid, LONG *eid, double *quantity, CHAR *szLocname, CHAR *szScale, CHAR *szComment, LONG *cmtdate, long fSearchOption )
{
#define SQL_reportMaterialsDist  " SELECT IMS_LOT.LOTID, IMS_LOT.EID, IMS_TRANSACTION.TRNQTY, SCALE.SCNAME, LOCATION.LNAME, IMS_TRANSACTION.COMMENTS,IMS_TRANSACTION.CMTDATA \
									FROM IMS_TRANSACTION, IMS_LOT,SCALE, LOCATION  \
									WHERE IMS_LOT.SCALEID = SCALE.SCALEID AND IMS_TRANSACTION.LOTID = IMS_LOT.LOTID AND IMS_LOT.LOCID = LOCATION.LOCID AND \
									IMS_TRANSACTION.TRNQTY<0  AND IMS_TRANSACTION.TRNSTAT=0 \
									ORDER BY IMS_LOT.LOTID "

   PUSH(reportMaterialsDist);

static CODBCbindedStmt &local= _localIMS->BindedStmt(SQL_reportMaterialsDist);  
static BOOL first_time=TRUE,fOK;
static LONG _lotid, _eid, _cmtdate;
static double _quantity;
static char _szLocname[GMS_LNAME_SIZE], _szScale[DMS_SCALE_NAME], _szComment[255];

   
	if (fSearchOption == FIND_FIRST) {
	   if (first_time)
		{
 
          // Bind all Colums and Parameter
         local.Bind(1,_lotid);
		 local.Bind(2,_eid);
		 local.Bind(3,_quantity);
         local.Bind(4,_szScale, DMS_SCALE_NAME);
         local.Bind(5,_szLocname, GMS_LNAME_SIZE);
         local.Bind(6,_szComment, 255);
         local.Bind(7,_cmtdate);
 
		}
		local.Execute();
	}
 

   if (local.Fetch()) {
      *lotid = _lotid;
      *eid = _eid;
	  strcpy(szLocname, _szLocname);
	  strcpy(szScale, _szScale);
	  strcpy(szComment, _szComment);
      *quantity = _quantity;
	  *cmtdate = _cmtdate;
      ret = IMS_SUCCESS;
	  }
   else if (local.NoData())
      ret = IMS_NO_DATA;
   else 
      ret = IMS_ERROR;

   POP();

#undef SQL_reportMaterialsDist

}



//////////////////////////////////////////////////////////////////////////////
//Date Created: June 8, 2006
// Data Modified:  June 8, 2006
//Description: Report by requestor
//////////////////////////////////////////////////////////////////////////////
LONG reportByRequestor(LONG flocal, LONG *lotid, LONG *eid, double *quantity, CHAR *szLocname, CHAR *szScale, CHAR *szComment, LONG *cmtdate, LONG *lngStat, LONG *lngPerson, long fSearchOption )
{
#define SQL_reportByRequestor  "SELECT IMS_LOT.LOTID, IMS_LOT.EID, IMS_TRANSACTION.TRNQTY, SCALE.SCNAME, LOCATION.LNAME, IMS_TRANSACTION.COMMENTS, IMS_TRANSACTION.CMTDATA, IMS_TRANSACTION.TRNSTAT, IMS_TRANSACTION.PERSONID \
               FROM IMS_TRANSACTION, IMS_LOT, SCALE, LOCATION \
               WHERE (((IMS_TRANSACTION.TRNQTY)<0) AND ((IMS_LOT.SCALEID)=[SCALE].[SCALEID]) AND ((IMS_TRANSACTION.LOTID)=[IMS_LOT].[LOTID]) AND ((IMS_LOT.LOCID)=[LOCATION].[LOCID])) \
               ORDER BY IMS_LOT.LOTID "

   PUSH(reportByRequestor );

static CODBCbindedStmt &local= _localIMS->BindedStmt(SQL_reportByRequestor);  
static BOOL first_time=TRUE,fOK;
static LONG _lotid, _eid, _cmtdate, _lngPerson, _lngStat;
static double _quantity;
static char _szLocname[GMS_LNAME_SIZE], _szScale[DMS_SCALE_NAME], _szComment[255];

   
	if (fSearchOption == FIND_FIRST) {
	   if (first_time)
		{
 
          // Bind all Colums and Parameter
         local.Bind(1,_lotid);
		 local.Bind(2,_eid);
		 local.Bind(3,_quantity);
         local.Bind(4,_szScale, DMS_SCALE_NAME);
         local.Bind(5,_szLocname, GMS_LNAME_SIZE);
         local.Bind(6,_szComment, 255);
         local.Bind(7,_cmtdate);
         local.Bind(8,_lngStat);
         local.Bind(9,_lngPerson);
 
		}
		local.Execute();
	}
 

   if (local.Fetch()) {
      *lotid = _lotid;
      *eid = _eid;
	  strcpy(szLocname, _szLocname);
	  strcpy(szScale, _szScale);
	  strcpy(szComment, _szComment);
      *quantity = _quantity;
	  *cmtdate = _cmtdate;
	  *lngStat = _lngStat;
	  *lngPerson = _lngPerson;
      ret = IMS_SUCCESS;
	  }
   else if (local.NoData())
      ret = IMS_NO_DATA;
   else 
      ret = IMS_ERROR;

   POP();

#undef SQL_reportByRequestor

}


/////////////////////////////////////////////////////////////////////////////
//Date created: June 23, 2005
//Description: Report on Lot by Entity Type ang range of GIDs
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE IMS_reportMaterialsDist( LONG *lotid, LONG *eid, double *quantity, CHAR *szLocname, CHAR *szScale, CHAR *szComment, LONG *cmtdate, long fSearchOption )
{
   static BOOL fLocal=TRUE;

   PUSH(IMS_reportMaterialsDist);
 
   ret=reportMaterialsDist(LOCAL, lotid, eid, quantity, szLocname, szScale, szComment, cmtdate, fSearchOption );
   POP();
}


/////////////////////////////////////////////////////////////////////////////
//Date created: June 5, 2006
//Description:  Report by Requestor
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE IMS_reportByRequestor( LONG *lotid, LONG *eid, double *quantity, CHAR *szLocname, CHAR *szScale, CHAR *szComment, LONG *cmtdate, LONG *lngStat, LONG *lngPerson, long fSearchOption )
{
   static BOOL fLocal=TRUE;

   PUSH(IMS_reportByRequestor);
 
   ret=reportByRequestor(LOCAL, lotid, eid, quantity, szLocname, szScale, szComment, cmtdate, lngStat, lngPerson, fSearchOption );
   POP();
}



//////////////////////////////////////////////////////////////////////////////
//Date Created: June 27, 2005
//Description: Report on Dormant Entries given a year
//Modification:  
// (AMP) June 6, 2006 - the SQL statement is modified to get the total balance and to consider the first transaction made as basis for date
//////////////////////////////////////////////////////////////////////////////
LONG reportDormantEntries(LONG flocal, LONG minYear, LONG *lotid, LONG *eid, double *quantity, CHAR *szLocname, CHAR *szScale, LONG *cmtdate, long fSearchOption )
{
//#define SQL_reportDormantEntries  " SELECT IMS_LOT.LOTID, IMS_LOT.EID, IMS_TRANSACTION.TRNQTY, SCALE.SCNAME, LOCATION.LNAME, IMS_TRANSACTION.CMTDATA \
//                                   FROM IMS_TRANSACTION INNER JOIN (LOCATION INNER JOIN (IMS_LOT INNER JOIN SCALE ON IMS_LOT.SCALEID = SCALE.SCALEID) ON \
//                                   LOCATION.LOCID = IMS_LOT.LOCID) ON IMS_TRANSACTION.LOTID = IMS_LOT.LOTID \
//                                   WHERE IMS_TRANSACTION.TRNQTY>0 AND IMS_TRANSACTION.TRNSTAT=1 AND {fn FLOOR(cmtdata/ 10000)}  <= ? \
//                                   ORDER BY IMS_LOT.LOTID "

	
#define SQL_reportDormantEntries  "	SELECT IMS_LOT.LOTID, IMS_LOT.EID, Sum(IMS_TRANSACTION.TRNQTY), SCALE.SCNAME, LOCATION.LNAME, Min({fn FLOOR(cmtdata/ 10000)})  \
FROM IMS_TRANSACTION INNER JOIN (LOCATION INNER JOIN (IMS_LOT INNER JOIN SCALE ON IMS_LOT.SCALEID = SCALE.SCALEID) ON LOCATION.LOCID = IMS_LOT.LOCID) ON IMS_TRANSACTION.LOTID = IMS_LOT.LOTID \
WHERE (((IMS_TRANSACTION.TRNSTAT)=1)) \
GROUP BY IMS_LOT.LOTID, IMS_LOT.EID, SCALE.SCNAME, LOCATION.LNAME \
HAVING (( (Sum(IMS_TRANSACTION.TRNQTY)) >0) AND (Min({fn FLOOR(cmtdata/ 10000)}) <=?)) \
ORDER BY IMS_LOT.LOTID"

   PUSH(reportDormantEntries);

static CODBCbindedStmt &local= _localIMS->BindedStmt(SQL_reportDormantEntries);  
static BOOL first_time=TRUE,fOK;
static LONG _lotid, _eid, _cmtdate, _minYear;
static double _quantity;
static char _szLocname[GMS_LNAME_SIZE], _szScale[DMS_SCALE_NAME];

   
	if (fSearchOption == FIND_FIRST) {
        _minYear = minYear;   
	   if (first_time)
		{
 
          // Bind all Colums and Parameter
         local.Bind(1,_lotid);
		 local.Bind(2,_eid);
		 local.Bind(3,_quantity);
         local.Bind(4,_szScale, DMS_SCALE_NAME);
         local.Bind(5,_szLocname, GMS_LNAME_SIZE);
         local.Bind(6,_cmtdate);
         local.BindParam(1, _minYear);
		}
		local.Execute();
	}
 

   if (local.Fetch()) {
      *lotid = _lotid;
      *eid = _eid;
	  strcpy(szLocname, _szLocname);
	  strcpy(szScale, _szScale);
      *quantity = _quantity;
	  *cmtdate = _cmtdate;
      ret = IMS_SUCCESS;
	  }
   else if (local.NoData())
      ret = IMS_NO_DATA;
   else 
      ret = IMS_ERROR;

   POP();

#undef SQL_reportDormantEntries

}

/////////////////////////////////////////////////////////////////////////////
//Date created: June 23, 2005
//Description: Report on Lot by Entity Type ang range of GIDs
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE IMS_reportDormantEntries(LONG minYear, LONG *lotid, LONG *eid, double *quantity, CHAR *szLocname, CHAR *szScale, LONG *cmtdate, long fSearchOption )
{   static BOOL fLocal=TRUE;

   PUSH(IMS_reportDormantEntries);
 
   ret=reportDormantEntries(LOCAL, minYear, lotid, eid, quantity, szLocname, szScale, cmtdate, fSearchOption );
   POP();
}


//////////////////////////////////////////////////////////////////////////////
//Date Created: June 27, 2005
// Data Modified:  June 27, 2005
//Description: Report on Dormant Entries given a year
//////////////////////////////////////////////////////////////////////////////
LONG reportMinimumAmount(LONG flocal, double minQty, LONG *lotid, LONG *eid, double *quantity, CHAR *szLocname, CHAR *szScale, long fSearchOption )
{
#define SQL_reportMinimumAmount  " SELECT IMS_LOT.LOTID, IMS_LOT.EID, SUM(IMS_TRANSACTION.TRNQTY), LOCATION.LNAME, SCALE.SCALEID \
                             FROM IMS_LOT, IMS_TRANSACTION, SCALE, LOCATION  \
                             WHERE IMS_LOT.LOTID = IMS_TRANSACTION.LOTID AND IMS_LOT.SCALEID = SCALE.SCALEID AND IMS_LOT.LOCID = LOCATION.LOCID \
                             AND IMS_TRANSACTION.TRNSTAT=1  \
                             GROUP BY IMS_LOT.LOTID, IMS_LOT.EID, LOCATION.LNAME, SCALE.SCALEID \
                             HAVING Sum(IMS_TRANSACTION.TRNQTY) < ? \
                             order by IMS_LOT.LOTID "


   PUSH(reportMinimumAmount);

static CODBCbindedStmt &local= _localIMS->BindedStmt(SQL_reportMinimumAmount);  
static BOOL first_time=TRUE,fOK;
static LONG _lotid, _eid, _cmtdate;
static double _quantity, _minQty;
static char _szLocname[GMS_LNAME_SIZE], _szScale[DMS_SCALE_NAME];

   
	if (fSearchOption == FIND_FIRST) {
        _minQty = minQty;   
	   if (first_time)
		{
 
          // Bind all Colums and Parameter
         local.Bind(1,_lotid);
		 local.Bind(2,_eid);
		 local.Bind(3,_quantity);
         local.Bind(4,_szLocname, GMS_LNAME_SIZE);
         local.Bind(5,_szScale, DMS_SCALE_NAME);
         local.BindParam(1, _minQty);
		}
		local.Execute();
	}
 

   if (local.Fetch()) {
      *lotid = _lotid;
      *eid = _eid;
	  strcpy(szLocname, _szLocname);
	  strcpy(szScale, _szScale);
      *quantity = _quantity;

      ret = IMS_SUCCESS;
	  }
   else if (local.NoData())
      ret = IMS_NO_DATA;
   else 
      ret = IMS_ERROR;

   POP();

#undef SQL_reportMinimumAmount

}

/////////////////////////////////////////////////////////////////////////////
//Date created: June 23, 2005
//Description: Report on Lot by Entity Type ang range of GIDs
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE IMS_reportMinimumAmount( double minQty, LONG *lotid, LONG *eid, double *quantity, CHAR *szLocname, CHAR *szScale, long fSearchOption )
{
   PUSH(IMS_reportMinimumAmount);
 
   ret=reportMinimumAmount(LOCAL,  minQty, lotid, eid, quantity, szLocname, szScale,  fSearchOption );
   POP();
}



//////////////////////////////////////////////////////////////////////////////
//Date Created: May 8, 2007
//////////////////////////////////////////////////////////////////////////////
LONG getIMSLabel(IMS_LABELINFO &data, LONG fSearchOption)
{
#define SQL_IMS_LABEL "SELECT ID,LABELINFO_ID, LABELITEMCOUNT,GROUP_PREFIX \
    FROM IMS_LABELINFO \
    WHERE IMS_LABELINFO.GROUP_PREFIX = ? "
   PUSH(getIMSLabel);

static CODBCbindedStmt &local= _localIMS->BindedStmt(SQL_IMS_LABEL);  ; 
static BOOL changed=TRUE;
static BOOL first_time=TRUE,fOK;
static IMS_LABELINFO _data;
static CHAR _grpPrefix[IMS_GROUP_PREFIX];

        ZeroFill(_grpPrefix,sizeof(_grpPrefix));

   if (fSearchOption==FIND_FIRST)
   {

         strcpy(_grpPrefix, data.GROUP_PREFIX);
         local.BindParam(1, _grpPrefix,IMS_GROUP_PREFIX);

          // Bind all Colums and Parameter
         local.Bind(1,_data.ID);
         local.Bind(2,_data.LABELINFO_ID);
         local.Bind(3,_data.LABELITEMCOUNT);
         local.Bind(4,_data.GROUP_PREFIX, sizeof(_data.GROUP_PREFIX));
         local.Execute();
   }
 

   if (local.Fetch()) {
       data = _data;
       ret = IMS_SUCCESS;
	  }
   else if (local.NoData())
      ret = IMS_NO_DATA;
   else 
      ret = IMS_ERROR;

   POP();

#undef SQL_IMS_LABEL

}


//////////////////////////////////////////////////////////////////////////////
//Date Created: May 8, 2007
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE IMS_geLabelInfo(IMS_LABELINFO *data, long fSearchOption )
{
   PUSH(IMS_reportMinimumAmount);
 
   ret=getIMSLabel(*data, fSearchOption);
   POP();
}



//////////////////////////////////////////////////////////////////////////////
//Date Created: May 8, 2007
//////////////////////////////////////////////////////////////////////////////
LONG getLabelOther(IMS_LABEL_OTHERINFO &data, LONG fSearchOption)
{
#define SQL_IMS_OTHERINFO "SELECT ID, OTHERINFO_ID, LABELINFO_ID, GROUP_PREFIX,TABLENAME, FIELDNAME \
    FROM IMS_LABEL_OTHERINFO \
WHERE GROUP_PREFIX = ? OR  GROUP_PREFIX =?  "

   PUSH(getLabelOther);

static CODBCbindedStmt &local= _localIMS->BindedStmt(SQL_IMS_OTHERINFO);  ; 
static BOOL changed=TRUE;
static BOOL first_time=TRUE,fOK;
static IMS_LABEL_OTHERINFO _data;
static CHAR _grpPrefix[IMS_GROUP_PREFIX], szAll[6];


   if (fSearchOption==FIND_FIRST)
   {

         ZeroFill(_grpPrefix,sizeof(_grpPrefix));
         strcpy(_grpPrefix, data.GROUP_PREFIX);
		 strcpy(szAll, "ALL");
         local.BindParam(1, _grpPrefix,IMS_GROUP_PREFIX);
         local.BindParam(2, szAll,6);

          // Bind all Colums and Parameter
		 local.Bind(1,_data.ID);
		 local.Bind(2,_data.OTHERINFO_ID);
		 local.Bind(3,_data.LABELINFO_ID);
         local.Bind(4,_data.GROUP_PREFIX, sizeof(_data.GROUP_PREFIX));
         local.Bind(5,_data.TABLENAME, sizeof(_data.TABLENAME));
         local.Bind(6,_data.FIELDNAME, sizeof(_data.FIELDNAME));
         local.Execute();
   }
 
   ZeroMemory(&_data,sizeof(_data));
   if (local.Fetch()) {
       data = _data;
       ret = IMS_SUCCESS;
	  }
   else if (local.NoData())
      ret = IMS_NO_DATA;
   else 
      ret = IMS_ERROR;

   POP();

#undef SQL_IMS_LABEL

}

//////////////////////////////////////////////////////////////////////////////
//Date Created: May 8, 2007
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE IMS_geLabelOtherInfo(IMS_LABEL_OTHERINFO *data, long fSearchOption )
{
   PUSH(IMS_reportMinimumAmount);
 
   ret=getLabelOther(*data, fSearchOption);
   POP();
}





/////////////////////////////////////////////////////////////////////////////
//Date Created: Nov 13, 2007
//    Added by: WVC
/////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE IMS_dllfindLocation(IMS_Location *data, LONG fSearchOption)
{
   static BOOL fLocal = TRUE;
   PUSH(IMS_dllfindLocation);
   if (fSearchOption == FIND_FIRST)
      fLocal = TRUE;
   if (fLocal){
      ret = findLocation(LOCAL, *data, fSearchOption);
   }
   POP();
}

//////////////////////////////////////////////////////////////////////////////
//Date Created: Nov 13, 2007
//    Added by: WVC
//////////////////////////////////////////////////////////////////////////////
LONG findLocation(BOOL fLocal, IMS_Location &data, LONG fSearchOption)
{
   #define SQL_FINDLOCATION "\
   select LOCID, LTYPE, NLLP, LRPLCE, LNAME, LABBR, SNL3ID, SNL2ID, SNL1ID, CNTRYID\
   from IMS_LOCATION\
   where (LNAME like ?) and (0=? or LTYPE=?)\
   order by IMS_LOCATION.LOCID"
   
   PUSH(findLocation);
   
   static CODBCbindedStmt &local = _localIMS->BindedStmt(SQL_FINDLOCATION);
          CODBCbindedStmt *source;
   static IMS_Location _data;
   static CHAR lname[GMS_LNAME_SIZE];
   static LONG ltype;
   static BOOL first_time = TRUE;
   
   if (fSearchOption == FIND_FIRST){
      if (first_time)
      {
         local.Bind(1, _data.locid);
         local.Bind(2, _data.ltype);
         local.Bind(3, _data.nllp);
         local.Bind(4, _data.lrplce);
         local.Bind(5, _data.lname, GMS_LNAME_SIZE);
         local.Bind(6, _data.labbr, GMS_LABBR_SIZE);
         local.Bind(7, _data.snl3id);
         local.Bind(8, _data.snl2id);
         local.Bind(9, _data.snl1id);
         local.Bind(10, _data.cntryid);
	     local.BindParam(1, lname, GMS_LNAME_SIZE);
	     local.BindParam(2, ltype);
	     local.BindParam(3, ltype);
         
         first_time = FALSE;
      }
      
	  ltype = data.ltype;
      strcpy(lname, data.lname);
      local.Execute();
   }
   
   source = &local;

   ZeroMemory(&_data, sizeof(_data));
   if (source->Fetch())
      data = _data;
   else if (source->NoData())
      ret = GMS_NO_DATA;
   else
      ret = GMS_ERROR;
   
   POP();
   #undef SQL_FINDLOCATION
}

//////////////////////////////////////////////////////////////////////////////
//Date Created: Nov 13, 2007
//    Added by: WVC
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE IMS_dlladdLocation(IMS_Location *data)
{
   if (UACCESS < 30)
      return GMS_NO_ACCESS;
   LONG ret = addLocation(*data);
   return ret;
}

//////////////////////////////////////////////////////////////////////////////
//Date Created: Nov 13, 2007
//    Added by: WVC
//////////////////////////////////////////////////////////////////////////////
LONG imsgetNextLOCID(BOOL fLocal)
{
   static LONG localLOCID = 0, centralLOCID = 0;
   if (fLocal && !localLOCID)
   {
      CODBCdirectStmt source1 = _localIMS->DirectStmt("SELECT MIN(ULOCN) FROM INSTLN");
      source1.Execute();
      source1.Fetch();
      CODBCdirectStmt source2 = _localIMS->DirectStmt("SELECT MIN(LOCID) FROM LOCATION");
      source2.Execute();
      source2.Fetch();
      localLOCID = min(source1.Field(1).AsInteger(), source2.Field(1).AsInteger());
   }
   return --localLOCID;
}

//////////////////////////////////////////////////////////////////////////////
//Date Created: Nov 13, 2007
//    Added by: WVC
//////////////////////////////////////////////////////////////////////////////
LONG addLocation(IMS_Location &location)
{
   #define SQL_ADDLOCATION "\
   insert into IMS_LOCATION\
   (LOCID, LTYPE, NLLP, LRPLCE, LNAME, LABBR, SNL3ID, SNL2ID, SNL1ID, CNTRYID)\
   values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
   
   static CODBCbindedStmt &local = _localIMS->BindedStmt(SQL_ADDLOCATION);
   static IMS_Location _data;
   static BOOL first_time = TRUE;
   
   if (first_time)
   {
      local.BindParam(1, _data.locid);
      local.BindParam(2, _data.ltype);
      local.BindParam(3, _data.nllp);
      local.BindParam(4, _data.lrplce);
      local.BindParam(5, _data.lname, GMS_LNAME_SIZE);
      local.BindParam(6, _data.labbr, GMS_LABBR_SIZE);
      local.BindParam(7, _data.snl3id);
      local.BindParam(8, _data.snl2id);
      local.BindParam(9, _data.snl1id);
      local.BindParam(10, _data.cntryid);
      
      first_time = FALSE;
   }
   location.locid = imsgetNextLOCID(TRUE);
   _data = location;
   if (local.Execute())
      return GMS_SUCCESS;
   else if (local.NoData())
      return GMS_NO_DATA;
   else 
      return GMS_ERROR;
   
   #undef SQL_ADDLOCATION
}

//////////////////////////////////////////////////////////////////////////////
//Date Created: Nov 13, 2007
//    Added by: WVC
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE IMS_getLocation3(IMS_Location *data, LONG fSearchOption)
{
   static BOOL fLocal = TRUE;
   PUSH(IMS_getLocation3);
   if (fSearchOption == FIND_FIRST)
      fLocal = TRUE;
   if (fLocal){
      ret = getIMSLocation3(LOCAL, *data, fSearchOption);
   }
   POP();
}

//////////////////////////////////////////////////////////////////////////////
//Date Created: Nov 13, 2007
//    Added by: WVC
//////////////////////////////////////////////////////////////////////////////
LONG getIMSLocation3(BOOL fLocal, IMS_Location &data, LONG fSearchOption)
{
   #define SQL_GETLOCATION3 "\
   select LTYPE, NLLP, LRPLCE, LNAME, LABBR, SNL3ID, SNL2ID, SNL1ID, CNTRYID, LOCID\
   from IMS_LOCATION\
   where SNL3ID = ? AND SNL2ID = ? AND SNL1ID = ? AND LTYPE = ?\
   order by LOCID"
   
   PUSH(getIMSLocation3);
   
   static CODBCbindedStmt &local = _localIMS->BindedStmt(SQL_GETLOCATION3);
          CODBCbindedStmt *source;
   static IMS_Location _data;
   static BOOL first_time = TRUE;
   static LONG _snl3id, _snl2id, _snl1id, _ltype;
   
   if (fSearchOption == FIND_FIRST)
   {
      if (first_time)
      {
         local.Bind(1, _data.ltype);
         local.Bind(2, _data.nllp);
         local.Bind(3, _data.lrplce);
         local.Bind(4, _data.lname, GMS_LNAME_SIZE);
         local.Bind(5, _data.labbr, GMS_LABBR_SIZE);
         local.Bind(6, _data.snl3id);
         local.Bind(7, _data.snl2id);
         local.Bind(8, _data.snl1id);
         local.Bind(9, _data.cntryid);
		 local.Bind(10, _data.locid);
	     local.BindParam(1, _snl3id);
	     local.BindParam(2, _snl2id);
	     local.BindParam(3, _snl1id);
	     local.BindParam(4, _ltype);
         
         first_time = FALSE;
      }
      
	  _snl3id = data.snl3id;
	  _snl2id = data.snl2id;
      _snl1id = data.snl1id; 
	  _ltype = data.ltype;
      local.Execute();
   }
   
   source = &local;
   
   ZeroMemory(&_data, sizeof(_data));
   if (source->Fetch())
   {
      data = _data;
   }
   else if (source->NoData())
      ret = GMS_NO_DATA;
   else 
      ret = GMS_ERROR;
   
   POP();
   #undef SQL_GETLOCATION3
}
