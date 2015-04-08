/* Gems.cpp : Implements the functions to access and manipulate the GEMS tables of ICIS
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
 **************************************************************/

#include "GEMS.h"


extern CODBCdbc *_centralGEMS;  //,*_localGEMS;    //Central and Local Database Connection
extern stack<char*> _routines_stack;





//////////////////////////////////////////////////////////////////////////////
//  Finds the maximum MDID from the GEMS_MARKER_DETECTOR
//  Created by: AMP (10/3/2006)
//  
//////////////////////////////////////////////////////////////////////////////
LONG getMaxMDID(void)
{
static LONG lngMDID=0;

   if (!lngMDID)
   {
      CODBCdirectStmt central = _centralGEMS->DirectStmt("SELECT MAX(MDID) FROM GEMS_MARKER_DETECTOR");
      central.Execute();
      central.Fetch();
      if (!central.NoData())
         lngMDID=central.Field(1).AsInteger();
   }
   return lngMDID;
}

//////////////////////////////////////////////////////////////////////////////
//added by WVC
// Gets the corresponding GEMS MID based on the marker name & object type 
//Input:  marker name & object type
//Output: GEMS MID
//Called by DMS_getGemsMID
//Modification:  AMP (10/3/2006)
//////////////////////////////////////////////////////////////////////////////

LONG getGemsMID(BOOL fLocal, GEMS_INFO &data, int fOpt)
{
#define SQL_GEMSMID \
		"SELECT GOBJID, GNVAL, GOBJTYPE FROM GEMS_NAMES \
		WHERE ((GEMS_NAMES.GNVAL=?) AND (GEMS_NAMES.GOBJTYPE=?))"
	
	LONG ret = DMS_SUCCESS;
	
	static CODBCbindedStmt &central = _centralGEMS->BindedStmt(SQL_GEMSMID);
//	static CODBCbindedStmt &local = _localGEMS->BindedStmt(SQL_GEMSMID);
	CODBCbindedStmt *source;
	
	static GEMS_INFO _data;
	static BOOL first_time = TRUE;
	
	if (fOpt == FIND_FIRST) {
		if (first_time) {
			
			//BINDPARAMS(1, _data.GNVAL, GEMS_MARKER_NAME);
			//BINDPARAMS(2, _data.GOBJTYPE, GEMS_TABLE_NAME);
			//
			//BIND(1, _data.GOBJID);
			//BINDS(2, _data.GNVAL, GEMS_MARKER_NAME);
			//BINDS(3, _data.GOBJTYPE, GEMS_TABLE_NAME);

			central.BindParam(1, _data.GNVAL, ICIS_MAX_SIZE);
			central.BindParam(2, _data.GOBJTYPE, ICIS_MAX_SIZE);
			
			central.Bind(1, _data.GOBJID);
			central.Bind(2, _data.GNVAL, ICIS_MAX_SIZE);
			central.Bind(3, _data.GOBJTYPE, ICIS_MAX_SIZE);
			
			first_time = FALSE;
		}
		_data = data;
//		local.Execute();
		central.Execute();
	}
	
//	source = (fLocal) ? &local : &central;
	source = &central;
	if (source->Fetch()) {
		ret = DMS_SUCCESS;
		data = _data;
	}
	else if (source->NoData())
		ret = DMS_NO_DATA;
	else 
		ret = DMS_ERROR;
	
	return ret;
	
#undef SQL_GEMSMID
}

//////////////////////////////////////////////////////////////////////////////
//added by WVC
//***** Gets the corresponding GEMS MID based on the marker name & object type *****/
//Input:  marker name & object type
//Output: GEMS MID
//Modification:  AMP (10/3/2006)
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GEMS_getGemsMID(GEMS_INFO *data, int fOpt)
{
	static BOOL fLocal = TRUE;
	
	PUSH(GEMS_getGemsMID);
	
	if (fOpt == FIND_FIRST) {
		fLocal = TRUE;
	}
	if (fLocal) {
		ret = getGemsMID(LOCAL, *data, fOpt);
		if (ret != DMS_SUCCESS){
			fLocal = FALSE;
			//ret = getGemsMID(CENTRAL, *data, FIND_NEXT);
		}
	}
	else
		ret = GMS_NO_DATA;
		//ret = getGemsMID(CENTRAL, *data, fOpt);
	
	POP();
}



//////////////////////////////////////////////////////////////////////////////
//added by WVC
//***** Gets the corresponding GEMS MVID based on the allele name, object type, & marker ID *****/
//Input:  allele name, object type, & marker ID
//Output: GEMS MVID
//Modification:  AMP (10/3/2006)
//Called by GEMS_getGemsMVID
//////////////////////////////////////////////////////////////////////////////
LONG getGemsMVID(BOOL fLocal, GEMS_INFO &data, int fOpt)
{
	#define SQL_GEMSMVID \
		"SELECT GEMS_NAMES.GOBJID, GEMS_NAMES.GNVAL, GEMS_NAMES.GOBJTYPE, GEMS_MV.MDID  \
		FROM GEMS_NAMES, GEMS_MV \
		WHERE ((GEMS_NAMES.GNVAL=?) AND (GEMS_NAMES.GOBJTYPE=?) AND \
		(GEMS_MV.MDID =?) AND (GEMS_NAMES.GOBJID = GEMS_MV.MVID))"

	
	LONG ret = DMS_SUCCESS;
	
	static CODBCbindedStmt &central = _centralGEMS->BindedStmt(SQL_GEMSMVID);
	CODBCbindedStmt *source;
	
	static GEMS_INFO _data;
	static BOOL first_time = TRUE;
	static LONG _objid;
	
	if (fOpt == FIND_FIRST) {
		if (first_time) {
			
			central.BindParam(1, _data.GNVAL, ICIS_MAX_SIZE);
			central.BindParam(2, _data.GOBJTYPE, ICIS_MAX_SIZE);
			central.BindParam(3, _data.MARKERID);
			
			central.Bind(1, _objid);
			central.Bind(2, _data.GNVAL, ICIS_MAX_SIZE);
			central.Bind(3, _data.GOBJTYPE, ICIS_MAX_SIZE);
			central.Bind(4, _data.MARKERID);


			first_time = FALSE;
		}
		_data = data;
		_objid = 0;
		central.Execute();
	}
	
	//source = (fLocal) ? &local : &central;
	source = &central;
	if (source->Fetch()) {
		ret = DMS_SUCCESS;
        data = _data;
		data.GOBJID = _objid;
	}
	else if (source->NoData())
		ret = DMS_NO_DATA;
	else 
		ret = DMS_ERROR;
	
	return ret;
	
	#undef SQL_GEMSMVID
}

//////////////////////////////////////////////////////////////////////////////
//added by WVC
//***** Gets the corresponding GEMS MVID based on the allele name, object type, & marker ID *****/
//Input:  allele name, object type, & marker ID
//Output: GEMS MVID
//Modification:  AMP (10/3/2006)
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GEMS_getGemsMVID(GEMS_INFO *data, int fOpt)
{
	static BOOL fLocal = TRUE;
	
	PUSH(GEMS_getGemsMVID);
	
	if (fOpt == FIND_FIRST) {
		fLocal = TRUE;
	}
	if (fLocal) {
		ret = getGemsMVID(LOCAL, *data, fOpt);
		if (ret != DMS_SUCCESS){
			fLocal = FALSE;
			//ret = getGemsMVID(CENTRAL, *data, FIND_NEXT);
		}
	}
	else
		ret = GMS_NO_DATA;
		//ret = getGemsMVID(CENTRAL, *data, fOpt);
	
	POP();
}


//////////////////////////////////////////////////////////////////////////////
// -  Adds a record in GEMS_NAMES table
//	-  returns 1 if successful, 0 if not
//Input:  GEMS marker name 
//Output: 
//Called by GEMS_addName(recName As GEMS_NAME)
//Modification:  AMP (10/3/2006)
//////////////////////////////////////////////////////////////////////////////

LONG addGEMSName(GEMS_NAME recName)
{
#define SQL_addGEMSName \
       "Insert into gems_names (gnid,gobjid,gobjtype,gntype,gnstat,gnuid,gnval,gnlocn,gndate,gnref ) \
		values (?,?,?,?,?,?,?,?,?,?)  "
	
	PUSH(addGEMSName);
	static CODBCbindedStmt &central = _centralGEMS->BindedStmt(SQL_addGEMSName);
//	static CODBCbindedStmt &local = _localGEMS->BindedStmt(SQL_GEMSMID);
	CODBCbindedStmt *source;
	
	static GEMS_NAME _recName;
	static BOOL first_time = TRUE;
	
	if (first_time) {
			
			//BINDPARAMS(1, _data.GNVAL, GEMS_MARKER_NAME);

			central.BindParam(1, _recName.gnid);
			central.BindParam(2, _recName.GOBJID);
			central.BindParam(3, _recName.GOBJTYPE, 255);
			central.BindParam(4, _recName.gntype);
			central.BindParam(5, _recName.gnstat);
			central.BindParam(6, _recName.gnuid);
			central.BindParam(7, _recName.GNVAL, 255);
			central.BindParam(8, _recName.gnlocn);
			central.BindParam(9, _recName.gndate);
			central.BindParam(10, _recName.gnref);

		
			first_time = FALSE;
	}
		
	
   _recName= recName;

   if (central.Execute())
      ret=GMS_SUCCESS;
   else
      ret=GMS_ERROR;

   POP();
	
#undef SQL_addGEMSName
}


/////////////////////////////////////////////////////////////
// Modified by WVC on 01/02/08: Added "+ 1" to GEMS_getMaxID
/////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GEMS_addGEMSName(GEMS_NAME *recName)
{
   PUSH(GEMS_addGEMSName);
   recName->gnid = GEMS_getMaxID("GEMS_NAMES","GNID") + 1;
   ret = addGEMSName(*recName);
   POP();
}


//////////////////////////////////////////////////////////////////////////////
// Finds the maximum ID of any GEMS table
// Modified by WVC on 01/02/08: Replaced "return ++maxID" with "return maxID"
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GEMS_getMaxID(CHAR *szTblName, CHAR *szFieldName)
{
#define SQL_GEMSgetMaxID "SELECT MAX("

static LONG maxID=0;
char _szField[MAX_STR], _szTblName[GEMS_TABLE_NAME];
string central_sql, strtemp;
    //  strcpy(_szField,"SELECT MAX(");
      strcpy(_szField,szFieldName);
	  strcpy(_szTblName,szTblName);
      strcat( _szField, (CHAR *) ") FROM " );
      strcat( _szField, _szTblName);
	  strtemp = _szField;
      central_sql=SQL_GEMSgetMaxID;
      central_sql+=strtemp ;

      CODBCdirectStmt central = _centralGEMS->DirectStmt((LPCSTR)central_sql.c_str());
      central.Execute();
      central.Fetch();
      if (!central.NoData())
         maxID=central.Field(1).AsInteger();
      return maxID;
}


//////////////////////////////////////////////////////////////////////////////
// -  Adds a record in GEMS_MARKER_DETECTOR table
//	-  returns 1 if successful, 0 if not
//Input:  GEMS marker detector record
//Output: 
//Called by GEMS_addMD(GEMS_MARKER_DETECTOR)
//Modification:  AMP (04/16/2007)
//////////////////////////////////////////////////////////////////////////////

LONG addMD(GEMS_MARKER_DETECTOR data)
{
#define SQL_addMD \
       "Insert into gems_marker_detector (mdid,matype,fprimer,rprimer,mauid,maref) \
		values (?,?,?,?,?,?)  "
	
	PUSH(addMD);
	static CODBCbindedStmt &central = _centralGEMS->BindedStmt(SQL_addMD);
//	static CODBCbindedStmt &local = _localGEMS->BindedStmt(SQL_GEMSMID);
	CODBCbindedStmt *source;
	
	static GEMS_MARKER_DETECTOR _data;
	static BOOL first_time = TRUE;
	
	if (first_time) {

			central.BindParam(1, _data.mdid);
			central.BindParam(2, _data.matype, LEN_MATYPE );
			central.BindParam(3, _data.fprimer, LEN_PRIMER);
			central.BindParam(4, _data.rprimer, LEN_PRIMER);
			central.BindParam(5, _data.mauid);
			central.BindParam(6, _data.maref);
		
			first_time = FALSE;
	}
		
	
   _data= data;

   if (central.Execute())
      ret=GMS_SUCCESS;
   else
      ret=GMS_ERROR;

   POP();
	
#undef SQL_addMD
}



/////////////////////////////////////////////////////////////
// Modified by WVC on 01/02/08: Added "+ 1" to GEMS_getMaxID
/////////////////////////////////////////////////////////////
LONG DLL_INTERFACE  GEMS_addMD(GEMS_MARKER_DETECTOR *recTblMD ) 
{
   PUSH(GEMS_addMD);
   recTblMD->mdid = GEMS_getMaxID("gems_marker_detector","mdid") + 1;
   ret = addMD(*recTblMD);
   POP();
}


//////////////////////////////////////////////////////////////////////////////
// -  Adds a record in GEMS_MV table
//	-  returns 1 if successful, 0 if not
//Input:  GEMS MV record
//Output: 
//Called by GEMS_addMV(GEMS_MV)
//Modification:  AMP (04/16/2007)
//////////////////////////////////////////////////////////////////////////////

LONG addMV(GEMS_MV data)
{
#define SQL_addMV \
		"Insert into gems_mv (mvid,locusid,mdid,mvtype,mwt,lmvid,mvuid,mvref) \
	     values (?,?,?,?,?,?,?,?)"
	PUSH(addMV);
	static CODBCbindedStmt &central = _centralGEMS->BindedStmt(SQL_addMV);
//	static CODBCbindedStmt &local = _localGEMS->BindedStmt(SQL_GEMSMID);
	CODBCbindedStmt *source;
	
	static GEMS_MV _data;
	static BOOL first_time = TRUE;
	
	if (first_time) {
			

			central.BindParam(1, _data.mvid);
			central.BindParam(2, _data.markerid);
			central.BindParam(3, _data.mdid);
			central.BindParam(4, _data.mvtype);
			central.BindParam(5, _data.mwt);
			central.BindParam(6, _data.lmvid);
			central.BindParam(7, _data.mvuid);
			central.BindParam(8, _data.mvref);
		
			first_time = FALSE;
	}
		
	
   _data= data;

   if (central.Execute())
      ret=GMS_SUCCESS;
   else
      ret=GMS_ERROR;

   POP();
	
#undef SQL_addMV
}


/////////////////////////////////////////////////////////////
// Modified by WVC on 01/02/08: Added "+ 1" to GEMS_getMaxID
/////////////////////////////////////////////////////////////
LONG DLL_INTERFACE  GEMS_addMV(GEMS_MV *recTblMV)
{
   PUSH(GEMS_addMV);
   recTblMV->mvid = GEMS_getMaxID("gems_mv","mvid") + 1;
   ret = addMV(*recTblMV);
   POP();
}


//////////////////////////////////////////////////////////////////////////////
// -  Adds a record in GEMS_COMPONENT table
//	-  returns 1 if successful, 0 if not
//Input:  GEMS_COMPONENT record
//Output: 
//Called by GEMS_addComponent(GEMS_COMPONENT)
//Modification:  AMP (04/16/2007)
//////////////////////////////////////////////////////////////////////////////

LONG addComponent(GEMS_COMPONENT data)
{
#define SQL_addComponent \
	"Insert into gems_comp (cid,condid,comid,comval,pid,comtype, comgrp,comuid,comref) \
      values (?,?,?,?,?,?,?,?,?) "

	PUSH(addComponent);
	static CODBCbindedStmt &central = _centralGEMS->BindedStmt(SQL_addComponent);
//	static CODBCbindedStmt &local = _localGEMS->BindedStmt(SQL_GEMSMID);
	CODBCbindedStmt *source;
	
	static GEMS_COMPONENT _data;
	static BOOL first_time = TRUE;
	
	if (first_time) {
			

			central.BindParam(1, _data.cid);
			central.BindParam(2, _data.condid);
			central.BindParam(3, _data.comid);
			central.BindParam(4, _data.comval, ICIS_MAX_SIZE);
			central.BindParam(5, _data.pid);
            central.BindParam(6, _data.comtype, ICIS_MAX_SIZE);
            central.BindParam(7, _data.comgrp, ICIS_MAX_SIZE);
			central.BindParam(8, _data.comuid);
			central.BindParam(9, _data.comref);
		
			first_time = FALSE;
	}
		
	
   _data= data;

   if (central.Execute())
      ret=GMS_SUCCESS;
   else
      ret=GMS_ERROR;

   POP();
	
#undef SQL_addComponent
}

/////////////////////////////////////////////////////////////
// Modified by WVC on 01/02/08: Added "+ 1" to GEMS_getMaxID
/////////////////////////////////////////////////////////////
LONG DLL_INTERFACE  GEMS_addComponent(GEMS_COMPONENT *recTblComp)
{
   PUSH(GEMS_addComponent);
   recTblComp->cid = GEMS_getMaxID("gems_comp","cid") + 1;
   ret = addComponent(*recTblComp);
   POP();

}


//////////////////////////////////////////////////////////////////////////////
// -  Adds a record in GEMS_PROPERTY table
//	-  returns 1 if successful, 0 if not
//Input:  GEMS_PROP record
//Output: 
//Called by GEMS_addProp
//Created:  AMP (05/17/2007)
//////////////////////////////////////////////////////////////////////////////

LONG addProp(GEMS_PROP &data)
{
#define SQL_addProp \
	"Insert into gems_prop (pid,propid,propname,scaleid,methid,propgrp) \
      values (?,?,?,?,?,?) "


	PUSH(addProp);
	static CODBCbindedStmt &central = _centralGEMS->BindedStmt(SQL_addProp);
	CODBCbindedStmt *source;
	
	static GEMS_PROP _data;
	static BOOL first_time = TRUE;
	
	if (first_time) {
			

			central.BindParam(1, _data.pid);
			central.BindParam(2, _data.propid);
			central.BindParam(3, _data.PropName, ICIS_MAX_SIZE);
			central.BindParam(4, _data.scaleid);
			central.BindParam(5, _data.methid);
			central.BindParam(6, _data.Propgrp, ICIS_MAX_SIZE);
		
			first_time = FALSE;
	}
		
	
   _data= data;

   if (central.Execute())
      ret=GMS_SUCCESS;
   else
      ret=GMS_ERROR;

   POP();
	
#undef SQL_addProp
}

LONG DLL_INTERFACE  GEMS_addProp(GEMS_PROP *data)
{
   PUSH(GEMS_addProp);
   ret = addProp(*data);
   POP();
}




//////////////////////////////////////////////////////////////////////////////
// -  Adds a record in GEMS_PD table
//	-  returns 1 if successful, 0 if not
//Input:  GEMS_PD record
//Output: 
//Called by GEMS_addPD(GEMS_PD)
//Modification:  AMP (04/16/2007)
//////////////////////////////////////////////////////////////////////////////

LONG addPD(GEMS_PD data)
{
#define SQL_addPD \
	"Insert into gems_Pd (pdid,condid,mdid)  \
    values (?,?,?) "

	PUSH(addPD);
	static CODBCbindedStmt &central = _centralGEMS->BindedStmt(SQL_addPD);
	CODBCbindedStmt *source;
	
	static GEMS_PD _data;
	static BOOL first_time = TRUE;
	
	if (first_time) {
			

			central.BindParam(1, _data.pdid);
			central.BindParam(2, _data.condid);
			central.BindParam(3, _data.mdid);
		
			first_time = FALSE;
	}
		
   _data= data;

   if (central.Execute())
      ret=GMS_SUCCESS;
   else
      ret=GMS_ERROR;

   POP();
	
#undef SQL_addPD
}


/////////////////////////////////////////////////////////////
// Modified by WVC on 01/02/08: Added "+ 1" to GEMS_getMaxID
/////////////////////////////////////////////////////////////
LONG DLL_INTERFACE  GEMS_addPD(GEMS_PD *recTblPd ) 
{
   PUSH(GEMS_addPD);
   recTblPd->pdid = GEMS_getMaxID("gems_Pd","pdid") + 1;
   ret = addPD(*recTblPd);
   POP();
}


//////////////////////////////////////////////////////////////////////////////
// -  Adds a record in GEMS_PD_Comp table
//	-  returns 1 if successful, 0 if not
//Input:  GEMS_PD_Comp record
//Output: 
//Called by GEMS_addPD(GEMS_PD)
//Modification:  AMP (04/16/2007)
//////////////////////////////////////////////////////////////////////////////

LONG addPDComp(GEMS_PD_COMP data)
{
#define SQL_addPDComp \
	"Insert into gems_pd_comp (pd_comp,pdid,cid)   \
    values (?,?,?,?) "

	PUSH(addPDComp);
	static CODBCbindedStmt &central = _centralGEMS->BindedStmt(SQL_addPDComp);
	CODBCbindedStmt *source;
	
	static GEMS_PD_COMP _data;
	static BOOL first_time = TRUE;
	
	if (first_time) {
			

			central.BindParam(1, _data.pd_comp);
			central.BindParam(2, _data.pdid);
			central.BindParam(3, _data.cid);
		
			first_time = FALSE;
	}
		
   _data= data;

   if (central.Execute())
      ret=GMS_SUCCESS;
   else
      ret=GMS_ERROR;

   POP();
	
#undef SQL_addPDComp
}


/////////////////////////////////////////////////////////////
// Modified by WVC on 01/02/08: Added "+ 1" to GEMS_getMaxID
/////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GEMS_addPdComp(GEMS_PD_COMP *recTblPdComp ) 
{
   PUSH(GEMS_addPdComp);
   recTblPdComp->pd_comp = GEMS_getMaxID("gems_pd_comp","pd_comp") + 1;
   ret = addPDComp(*recTblPdComp);
   POP();
}


LONG getCIDComp(LONG fLocal, LONG *cid, LONG condid, LONG fopt)
{
	#define SQL_getCIDComp \
		"Select cid gems_comp  where condid = ? "
	
	LONG ret = GEMS_SUCCESS;
	
	static CODBCbindedStmt &central = _centralGEMS->BindedStmt(SQL_getCIDComp);
	//static CODBCbindedStmt &local = _localGEMS->BindedStmt(SQL_GEMSMVID);
	CODBCbindedStmt *source;
	
	static long _cid, _condid;
	static BOOL first_time = TRUE;
	
	if (fopt == FIND_FIRST) {
		if (first_time) {
			
            central.Bind(1, _cid);
			central.BindParam(1, _condid);


			first_time = FALSE;
		}
		_condid = condid;
		central.Execute();
	}
	
	//source = (fLocal) ? &local : &central;
	source = &central;
	if (source->Fetch()) {
		ret = DMS_SUCCESS;
		*cid = _cid;
	}
	else if (source->NoData())
		ret = DMS_NO_DATA;
	else 
		ret = DMS_ERROR;
	
	return ret;
	
	#undef SQL_getCIDComp
}

/////////////////////////////////////////////////////////////
// Modified by WVC on 01/02/08: Added "+ 1" to GEMS_getMaxID
/////////////////////////////////////////////////////////////
LONG DLL_INTERFACE  GEMS_getPdComp(PD_COMP_INFO *pdCompInfo, LONG fopt)
{
	static BOOL fLocal = TRUE;
	LONG _cid, _condid;
	GEMS_PD_COMP recPdComp;
	PUSH(GEMS_getPdComp);
	_condid = pdCompInfo->cid;
	ret = getCIDComp(LOCAL,&_cid, _condid,0);
	while (ret==GEMS_SUCCESS)
	{
	     ret = getCIDComp(LOCAL,&_cid, _condid,0);    
		 recPdComp.pdid = pdCompInfo->pdid;
         recPdComp.pd_comp = GEMS_getMaxID("gems_pd_comp", "pd_comp") + 1;
         //adds a new record in the GEMS_PD_COMP table
         ret = GEMS_addPdComp(&recPdComp)  ; 
		 _cid = 0;
   	     ret = getCIDComp(LOCAL,&_cid, _condid,1);
	}
	
	POP();
}



LONG getProtocolID(LONG fLocal, GEMS_INFO *recProcInfo , LONG fopt)
{
	#define SQL_getCIDComp \
		"Select gobjid from gems_names where gobjtype = \"protocol\" and gnval = ?"
	LONG ret = GEMS_SUCCESS;
	
	static CODBCbindedStmt &central = _centralGEMS->BindedStmt(SQL_getCIDComp);
	//static CODBCbindedStmt &local = _localGEMS->BindedStmt(SQL_GEMSMVID);

	CODBCbindedStmt *source;
	
	static GEMS_INFO _recProcInfo;
	static BOOL first_time = TRUE;
	
	if (fopt == FIND_FIRST) {
		if (first_time) {
			
            central.Bind(1, _recProcInfo.GOBJID);
			central.BindParam(1, _recProcInfo.GOBJTYPE, ICIS_MAX_SIZE);

			first_time = FALSE;
		}
		_recProcInfo = *recProcInfo;
		central.Execute();
	}
	
	//source = (fLocal) ? &local : &central;
	source = &central;
	if (source->Fetch()) {
        recProcInfo->GOBJID = _recProcInfo.GOBJID;
		ret = GEMS_SUCCESS;
	}
	else if (source->NoData())
		ret = GMS_NO_DATA;
	else 
		ret = GEMS_ERROR;
	
	return ret;
	
	#undef SQL_getCIDComp
}


LONG DLL_INTERFACE  GEMS_getProtocolID(GEMS_INFO *recProcInfo , LONG fopt)
{
   PUSH(GEMS_getProtocolID);
   ret = getProtocolID(LOCAL, recProcInfo, fopt);
   POP();
}



LONG findPID(PROP_INFO *recProcInfo )
{ 

#define SQL_findPID \
	   "SELECT gems_prop.pid \
         FROM (gems_prop INNER JOIN gems_scale ON gems_prop.scaleid = gems_scale.scaleid) \
         INNER JOIN gems_method ON gems_prop.methid = gems_method.methid \
         WHERE gems_prop.propname= ? \
         AND gems_method.mname = ? \
         AND gems_scale.scname = ? \
         AND gems_prop.propgrp = ? "

	LONG ret = GEMS_SUCCESS;
	
	static CODBCbindedStmt &central = _centralGEMS->BindedStmt(SQL_findPID);

	CODBCbindedStmt *source;
	
	static PROP_INFO _recProcInfo;
	static BOOL first_time = TRUE;
	

		if (first_time) {
			
            central.Bind(1, _recProcInfo.pid);
			central.BindParam(1, _recProcInfo.pname, ICIS_MAX_SIZE);
			central.BindParam(2, _recProcInfo.mname, ICIS_MAX_SIZE);
			central.BindParam(3, _recProcInfo.sname, ICIS_MAX_SIZE);
			central.BindParam(4, _recProcInfo.pgrpname, ICIS_MAX_SIZE);

			first_time = FALSE;
		}
		_recProcInfo = *recProcInfo;
		central.Execute();

	
	//source = (fLocal) ? &local : &central;
	source = &central;
	if (source->Fetch()) {
        recProcInfo->pid = _recProcInfo.pid;
		ret = GEMS_SUCCESS;
	}
	else if (source->NoData())
		ret = GMS_NO_DATA;
	else 
		ret = GEMS_ERROR;
	
	return ret;
	
	#undef SQL_getCIDComp

}


LONG DLL_INTERFACE GEMS_findPID(PROP_INFO *PropInfo) 
{
   PUSH(GEMS_getProtocolID);
   ret = findPID(PropInfo);
   POP();
}

LONG findPropID(GEMS_PROP *data)
{
#define SQL_findPropID \
	"SELECT gems_prop.pid,gems_prop.propid \
     FROM gems_method INNER JOIN gems_prop ON gems_method.methid = gems_prop.methid \
     WHERE gems_prop.propname= ? \
     AND gems_prop.propgrp = ?"

    LONG ret = GEMS_SUCCESS;
	
	static CODBCbindedStmt &central = _centralGEMS->BindedStmt(SQL_findPropID );

	CODBCbindedStmt *source;
	
	static GEMS_PROP _data;
	static BOOL first_time = TRUE;
	

		if (first_time) {
			
            central.Bind(1, _data.pid);
			central.Bind(2, _data.propid);
			central.BindParam(1, _data.PropName,ICIS_MAX_SIZE);
			central.BindParam(2, _data.Propgrp, ICIS_MAX_SIZE);

			first_time = FALSE;
		}
		_data = *data;
		central.Execute();

	
	//source = (fLocal) ? &local : &central;
	source = &central;
	if (source->Fetch()) {
        data->pid = _data.pid;
		data->propid = _data.propid;
		ret = GEMS_SUCCESS;
	}
	else if (source->NoData())
		ret = GMS_NO_DATA;
	else 
		ret = GEMS_ERROR;
	
	return ret;

#undef SQL_findPropID
}

LONG DLL_INTERFACE GEMS_findPropid(GEMS_PROP *recTblProp)
{
   PUSH(GEMS_findPropid);
   ret = findPropID(recTblProp);
   POP();
}

LONG gemsFindMethod( GEMS_METHOD *data)
{
#define SQL_FindMethod \
	"SELECT methid,mabbr, mdesc from gems_method WHERE gems_method.mname= ? "
    LONG ret = GEMS_SUCCESS;
	
	static CODBCbindedStmt &central = _centralGEMS->BindedStmt(SQL_FindMethod );

	CODBCbindedStmt *source;
	
	static GEMS_METHOD _data;
	static BOOL first_time = TRUE;
	

		if (first_time) {
			
            central.Bind(1, _data.methid);
            central.Bind(2, _data.mabbr, ICIS_MAX_SIZE);
            central.Bind(3, _data.mdesc, ICIS_MAX_SIZE);
			central.BindParam(1, _data.mname, ICIS_MAX_SIZE);

			first_time = FALSE;
		}
		_data = *data;
		central.Execute();

	
	//source = (fLocal) ? &local : &central;
	source = &central;
	if (source->Fetch()) {
		*data = _data;
		ret = GEMS_SUCCESS;
	}
	else if (source->NoData())
		ret = GMS_NO_DATA;
	else 
		ret = GEMS_ERROR;
	
	return ret;

#undef SQL_FindMethod

}

LONG DLL_INTERFACE GEMS_findMethod( GEMS_METHOD *recTblMethod)
{
   PUSH(GEMS_findMethod);
   ret = gemsFindMethod(recTblMethod);
   POP();
}


LONG gemsAddMethod(GEMS_METHOD data)
{
#define SQL_addMethod \
		"Insert into gems_method (methid,mname,mabbr,mdesc) \
         values (?,?,?,?) "

	PUSH(gemsAddMethod);
	static CODBCbindedStmt &central = _centralGEMS->BindedStmt(SQL_addMethod);
//	static CODBCbindedStmt &local = _localGEMS->BindedStmt(SQL_GEMSMID);
	CODBCbindedStmt *source;
	
	static GEMS_METHOD _data;
	static BOOL first_time = TRUE;
	
	if (first_time) {
			central.BindParam(1, _data.methid);
			central.BindParam(2, _data.mname, ICIS_MAX_SIZE);
			central.BindParam(3, _data.mabbr, ICIS_MAX_SIZE);
			central.BindParam(4, _data.mdesc, ICIS_MAX_SIZE);
		
			first_time = FALSE;
	}
		
	
   _data= data;

   if (central.Execute())
      ret=GMS_SUCCESS;
   else
      ret=GMS_ERROR;

   POP();
	
#undef SQL_gemsAddMethod
}

/////////////////////////////////////////////////////////////
// Modified by WVC on 01/02/08: Added "+ 1" to GEMS_getMaxID
/////////////////////////////////////////////////////////////
LONG DLL_INTERFACE  GEMS_addMethod(GEMS_METHOD *tblMethod) 
{
   PUSH(GEMS_addMethod);
   tblMethod->methid = GEMS_getMaxID("gems_method","methid") + 1;
   ret = gemsAddMethod(*tblMethod);
   POP();
}


LONG gemsFindScale(GEMS_SCALE *data)
{
#define SQL_findScale \
		"SELECT scaleid from gems_scale \
	    WHERE gems_scale.scname= ? \
        AND gems_scale.propid = ?"

	static CODBCbindedStmt &central = _centralGEMS->BindedStmt(SQL_findScale );

	CODBCbindedStmt *source;
	PUSH(gemsFindScale);
	static GEMS_SCALE _data;
	static BOOL first_time = TRUE;
	

		if (first_time) {
			
            central.Bind(1, _data.scaleid);
            central.BindParam(1, _data.scname, ICIS_MAX_SIZE);
            central.BindParam(2, _data.propid);

			first_time = FALSE;
		}
		_data = *data;
		central.Execute();

	
	//source = (fLocal) ? &local : &central;
	source = &central;
	if (source->Fetch()) {
		*data = _data;
		ret = GEMS_SUCCESS;
	}
	else if (source->NoData())
		ret = GMS_NO_DATA;
	else 
		ret = GEMS_ERROR;
	
	POP();

#undef SQL_findScale

}

LONG DLL_INTERFACE  GEMS_findScale(GEMS_SCALE *tblScale)
{
   PUSH(GEMS_findScale);
   ret = gemsFindScale(tblScale);
   POP();
}

LONG gemsAddScale(GEMS_SCALE data)
{
#define SQL_gemsAddScale \
	"Insert into gems_scale (scaleid,scname,propid,sctype) \
    values (?,?,?,?)"
	PUSH(gemsAddScale);
	static CODBCbindedStmt &central = _centralGEMS->BindedStmt(SQL_gemsAddScale);
//	static CODBCbindedStmt &local = _localGEMS->BindedStmt(SQL_GEMSMID);
	CODBCbindedStmt *source;
	
	static GEMS_SCALE _data;
	static BOOL first_time = TRUE;
	
	if (first_time) {
			central.BindParam(1, _data.scaleid);
			central.BindParam(2, _data.scname, ICIS_MAX_SIZE);
			central.BindParam(3, _data.propid);
			central.BindParam(4, _data.sctype, ICIS_MAX_SIZE);
		
			first_time = FALSE;
	}
		
	
   _data= data;

   if (central.Execute())
      ret=GMS_SUCCESS;
   else
      ret=GMS_ERROR;

   POP();
	
#undef SQL_gemsAddScale
}


/////////////////////////////////////////////////////////////
// Modified by WVC on 01/02/08: Added "+ 1" to GEMS_getMaxID
/////////////////////////////////////////////////////////////
LONG DLL_INTERFACE  GEMS_addScale(GEMS_SCALE *tblScale) 
{
   PUSH(GEMS_addScale);
   tblScale->scaleid = GEMS_getMaxID("gems_scale","scaleid") + 1;
   ret = gemsAddScale(*tblScale);
   POP();
}


LONG findPdid(GEMS_PD *data)
{
#define SQL_findPdid \
		"SELECT pdid from gems_pd \
	     WHERE gems_pd.condid = ? AND gems_pd.mdid = ? "

	static CODBCbindedStmt &central = _centralGEMS->BindedStmt(SQL_findPdid);

	CODBCbindedStmt *source;
	PUSH(findPdid);
	static GEMS_PD _data;
	static BOOL first_time = TRUE;
	

		if (first_time) {
			
            central.Bind(1, _data.pdid);
            central.BindParam(1, _data.condid);
            central.BindParam(2, _data.mdid);

			first_time = FALSE;
		}
		_data = *data;
		central.Execute();

	
	//source = (fLocal) ? &local : &central;
	source = &central;
	if (source->Fetch()) {
		*data = _data;
		ret = GEMS_SUCCESS;
	}
	else if (source->NoData())
		ret = GMS_NO_DATA;
	else 
		ret = GEMS_ERROR;
	
	POP();

#undef SQL_findPdid
}

LONG DLL_INTERFACE GEMS_findPdid(GEMS_PD *tblPd)
{
   PUSH(GEMS_findPdid);
   ret = findPdid(tblPd);
   POP();
}



LONG addLocus(GEMS_LOCUS data)
{
#define SQL_addMD \
            "Insert into gems_locus(locusid,chr,pos) \
             values (?,?,?) "	

	PUSH(addLocus);
	static CODBCbindedStmt &central = _centralGEMS->BindedStmt(SQL_addMD);
//	static CODBCbindedStmt &local = _localGEMS->BindedStmt(SQL_GEMSMID);
	CODBCbindedStmt *source;
	
	static GEMS_LOCUS _data;
	static BOOL first_time = TRUE;
	
	if (first_time) {
			

			central.BindParam(1, _data.locusid);
			central.BindParam(2, _data.chr, ICIS_MAX_SIZE);
			central.BindParam(3, _data.pos, ICIS_MAX_SIZE);
		
			first_time = FALSE;
	}
		
	
   _data= data;

   if (central.Execute())
      ret=GMS_SUCCESS;
   else
      ret=GMS_ERROR;

   POP();
	
#undef SQL_addMD
}


/////////////////////////////////////////////////////////////
// Modified by WVC on 01/02/08: Added "+ 1" to GEMS_getMaxID
/////////////////////////////////////////////////////////////
LONG DLL_INTERFACE  GEMS_addLocus(GEMS_LOCUS *tblLocus)
{
   PUSH(GEMS_addLocus);
   tblLocus->locusid = GEMS_getMaxID("gems_locus","locusid") + 1;
   ret = addLocus(*tblLocus);
   POP();
}



LONG gemsGetNameID(GEMS_INFO *data)
{
	#define SQL_gemsGetNameID \
         "Select gobjid from gems_names where gobjtype = ? and gnval = ? "
	static CODBCbindedStmt &central = _centralGEMS->BindedStmt(SQL_gemsGetNameID);

	CODBCbindedStmt *source;
	PUSH(gemsGetNameID);
	static GEMS_INFO _data;
	static BOOL first_time = TRUE;
	

		if (first_time) {
			
            central.Bind(1, _data.GOBJID);
            central.BindParam(1, _data.GOBJTYPE, ICIS_MAX_SIZE);
            central.BindParam(2, _data.GNVAL, ICIS_MAX_SIZE);

			first_time = FALSE;
		}
		_data = *data;
		central.Execute();

	
	//source = (fLocal) ? &local : &central;
	source = &central;
	if (source->Fetch()) {
		*data = _data;
		ret = GEMS_SUCCESS;
	}
	else if (source->NoData())
		ret = GMS_NO_DATA;
	else 
		ret = GEMS_ERROR;
	
	POP();

#undef SQL_gemsGetNameID
}

LONG DLL_INTERFACE GEMS_getNameID(GEMS_INFO *data)
{
   PUSH(GEMS_getNameID);
   ret = gemsGetNameID(data);
   POP();
}

LONG getAlleleID( GEMS_INFO *data, LONG fopt)
{
	#define SQL_getAlleleID \
         "Select gobjid from gems_names where gobjtype = ? and gnval = ? "
	static CODBCbindedStmt &central = _centralGEMS->BindedStmt(SQL_getAlleleID);

	CODBCbindedStmt *source;
	PUSH(getAlleleID);
	static GEMS_INFO _data;
	static BOOL first_time = TRUE;
	
	if (fopt == FIND_FIRST) 
	{
		if (first_time) {
			
            central.Bind(1, _data.GOBJID);
            central.BindParam(1, _data.GOBJTYPE, ICIS_MAX_SIZE);
            central.BindParam(2, _data.GNVAL,ICIS_MAX_SIZE);

			first_time = FALSE;
		}
		_data = *data;
		central.Execute();

	}
	//source = (fLocal) ? &local : &central;
	source = &central;
	if (source->Fetch()) {
		*data = _data;
		ret = GEMS_SUCCESS;
	}
	else if (source->NoData())
		ret = GMS_NO_DATA;
	else 
		ret = GEMS_ERROR;
	
	POP();

#undef SQL_getAlleleID
}



LONG DLL_INTERFACE GEMS_getAlleleID( GEMS_INFO *recNameInfo, LONG fopt) 
{
   PUSH(GEMS_getAlleleID);
   ret = getAlleleID(recNameInfo, fopt);
   POP();

}


