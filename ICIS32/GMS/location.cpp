/* Location.cpp : Implements the functions to access and manipulate the LOCATION table of ICIS
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

#include "location.h"
extern LONG UACCESS,USERID;
extern CODBCdbc *_centralDBC,*_localDBC;  //Central and Local Database Connection
extern stack<char*> _routines_stack;
#if defined(WEB)
  #include "BeanTransfer.h"
  extern bool WEBSERVICE;  //True if calls are to Web Service
  extern _bstr_t szURL;    // URL pointing to the WSDL of the Web Service

  GMS_Location *getLocation2Bean;
  int getLocation2Idx;



  BOOL _stdcall BufferLocation(LocationBean *InStruct, int codeFunction, int sze)
  {
	  switch (codeFunction){
		  case 1:     //GMS_getLocation2
			  free(getLocation2Bean);
			  getLocation2Idx = sze;
			  getLocation2Bean = (GMS_Location *)calloc(sze,sizeof(GMS_Location));
			  for (int i=0; i <sze; i++)
			  {
				  getLocation2Bean[i].locid = InStruct[i].locid;
				  getLocation2Bean[i].ltype = InStruct[i].ltype;
				  getLocation2Bean[i].nllp = InStruct[i].nllp;
				  getLocation2Bean[i].snl3id = InStruct[i].snl3id;
				  getLocation2Bean[i].snl2id = InStruct[i].snl2id;
				  getLocation2Bean[i].snl1id = InStruct[i].snl1id;
				  strncpy(getLocation2Bean[i].lname, InStruct[i].lname, strlen(InStruct[i].lname));
				  strncpy(getLocation2Bean[i].labbr, InStruct[i].labbr, strlen(InStruct[i].labbr));
			  }
			  return (true);
			  break;
		  case 2:
			  break;
		  case 3:
			  break;
	  }
  }

#endif
//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_getLocation(GMS_Location *data, LONG fSearchOption)
{
   static BOOL fLocal=TRUE;
   long locid ;
#if defined(WEB)
   IManagedInterface *cpi = NULL; 
   LocationBean lbean;
#endif

   PUSH(GMS_getLocation);
   locid = data->locid;
   if (fSearchOption==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal){
      ret=getLocation(LOCAL,*data,fSearchOption);
      if (ret!=GMS_SUCCESS){
         fLocal = FALSE; 

#if defined(WEB)
		 if (WEBSERVICE) {
				CoInitialize(NULL);
				//Instantiate the COM object in the appropriate apartment
				HRESULT hr = CoCreateInstance(CLSID_LibraryImplementation,
					NULL, CLSCTX_INPROC_SERVER, IID_IManagedInterface, reinterpret_cast<void**>(&cpi)); 

				if(FAILED(hr))
				{
					MessageBox(NULL,"Could not create Instance, Web Service Failed!","ICIS V5",MB_OK);
				}
                ZeroMemory(&lbean,sizeof(lbean));
				lbean.locid = locid;
				ret = cpi->LgetLocation(CENTRAL,&lbean,szURL);
				if (ret == GMS_SUCCESS) 
                    transferLocBean(lbean,data);
				if ((locid != 0) || (ret == GMS_NO_DATA))
				    CoUninitialize();
		 }
		 else
#endif
      
			 ret=getLocation(CENTRAL,*data,FIND_NEXT);
      }
   }
   else
      ret=getLocation(CENTRAL,*data,fSearchOption);
   POP();
}




//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG getLocation(BOOL fLocal, GMS_Location &data, LONG fSearchOption)
{
#define SQL_GETLOCATION "\
   select LTYPE,NLLP,\
     LRPLCE,LNAME,LABBR,SNL3ID,SNL2ID,SNL1ID,CNTRYID\
   from LOCATION \
   where LOCATION.LOCID=?\
   order by LOCATION.LOCID"

   PUSH(getLocation);


static CODBCbindedStmt &central= _centralDBC->BindedStmt(SQL_GETLOCATION);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_GETLOCATION);
       CODBCbindedStmt *source;

static GMS_Location _data;
static BOOL first_time=TRUE;

static LONG locid;

   if (fSearchOption==FIND_FIRST)
   {
      if (first_time)
      {
         BIND(1,_data.ltype);
         BIND(2,_data.nllp);
         BIND(3,_data.lrplce);
         BINDS(4,_data.lname,GMS_LNAME_SIZE);
         BINDS(5,_data.labbr,GMS_LABBR_SIZE);
         BIND(6,_data.snl3id);
         BIND(7,_data.snl2id);
         BIND(8,_data.snl1id);
         BIND(9,_data.cntryid);
	      BINDPARAM(1,locid);
 
         first_time=FALSE;
      }

	   locid=data.locid;
      local.Execute();
      central.Execute();
   }

   source = (fLocal)?&local:&central;

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
LONG DLL_INTERFACE GMS_getLocation2(GMS_Location *data, LONG fSearchOption)
{
   static BOOL fLocal=TRUE;
#if defined(WEB)
   IManagedInterface *cpi = NULL; 
   LocationBean lbean;
   static long recNo;
#endif

   PUSH(GMS_getLocation2);
   if (fSearchOption==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal){
      ret=getLocation2(LOCAL,*data,fSearchOption);
      if (ret!=GMS_SUCCESS){
         fLocal = FALSE; 

#if defined(WEB)
		 if (WEBSERVICE) {
			    recNo = 0;
				CoInitialize(NULL);
				//Instantiate the COM object in the appropriate apartment
				HRESULT hr = CoCreateInstance(CLSID_LibraryImplementation,
					NULL, CLSCTX_INPROC_SERVER, IID_IManagedInterface, reinterpret_cast<void**>(&cpi)); 
                if(FAILED(hr))
				{
					MessageBox(NULL,"Could not create Instance, Web Service Failed!","ICIS V5",MB_OK);
				}
                ZeroMemory(&lbean,sizeof(lbean));
				assignLocBean(&lbean,*data);
				ret = cpi->LgetLocation2(CENTRAL,lbean,szURL);
				if (ret == GMS_SUCCESS) {
                   data->locid = getLocation2Bean[recNo].locid;
				   data->ltype = getLocation2Bean[recNo].ltype;
				   data->nllp	= getLocation2Bean[recNo].nllp;
				   strcpy(data->labbr,getLocation2Bean[recNo].labbr);
				   strcpy(data->lname,getLocation2Bean[recNo].lname);
				   data->snl3id =getLocation2Bean[recNo].snl3id;
				   data->snl2id =getLocation2Bean[recNo].snl2id;
				   data->snl1id =getLocation2Bean[recNo].snl1id;
                   data->cntryid = getLocation2Bean[recNo].cntryid;
				   data->lrplce =getLocation2Bean[recNo].lrplce;
			       recNo = recNo + 1;
				}
				CoUninitialize();
		 }
		 else
#endif

		 ret=getLocation2(CENTRAL,*data,FIND_NEXT);
      }
   }
   else

#if defined(WEB)
   		 if (WEBSERVICE) 
		 {
           if (recNo< getLocation2Idx )
		   {
               data->locid = getLocation2Bean[recNo].locid;
			   data->ltype = getLocation2Bean[recNo].ltype;
			   data->nllp	= getLocation2Bean[recNo].nllp;
			   strcpy(data->labbr,getLocation2Bean[recNo].labbr);
			   strcpy(data->lname,getLocation2Bean[recNo].lname);
			   data->snl3id =getLocation2Bean[recNo].snl3id;
			   data->snl2id =getLocation2Bean[recNo].snl2id;
			   data->snl1id =getLocation2Bean[recNo].snl1id;
               data->cntryid = getLocation2Bean[recNo].cntryid;
			   data->lrplce =getLocation2Bean[recNo].lrplce;
			   recNo = recNo + 1;
			   ret = GMS_SUCCESS;
		   }
		   else{
			   ret = GMS_NO_DATA;
			   recNo = 0;
			   free(getLocation2Bean);
		   }
		 }
		 else
#endif
	   
	   ret=getLocation2(CENTRAL,*data,fSearchOption);
   POP();
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG	getLocation2(BOOL fLocal,GMS_Location &data, LONG fSearchOption)
{
#define SQL_GETLOCATION2 "\
   select LOCID, LTYPE,NLLP,\
     LRPLCE,LNAME,LABBR,SNL3ID,SNL2ID,SNL1ID,CNTRYID\
   from LOCATION where"

   PUSH(getLocation2);

static CODBCbindedStmt &central= _centralDBC->BindedStmt();
static CODBCbindedStmt &local  = _localDBC->BindedStmt();
       CODBCbindedStmt *source;

static GMS_Location _data;

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
         strSQL=SQL_GETLOCATION2;
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

         central = _centralDBC->BindedStmt((LPCSTR)strSQL.c_str());     //AMP:  added
         local   = _localDBC->BindedStmt((LPCSTR)strSQL.c_str());       //AMP:  added

 //AMP:deleted 2003/06/05        local.SetSQLstr((LPCSTR)strSQL.c_str());
 //AMP:deleted 2003/06/05        central.SetSQLstr((LPCSTR)strSQL.c_str());


         BIND(1,_data.locid  );
         BIND(2,_data.ltype  );
         BIND(3,_data.nllp   );
         BIND(4,_data.lrplce );
         BINDS(5,_data.lname,GMS_LNAME_SIZE);
         BINDS(6,_data.labbr,GMS_LABBR_SIZE);
         BIND(7,_data.snl3id );
         BIND(8,_data.snl2id );
         BIND(9,_data.snl1id );
         BIND(10,_data.cntryid);

         int i=1;
         if (flocid)
         {
            BINDPARAM(i,locid);i+=1;
         }
         if (fltype)
         {
            BINDPARAM(i,ltype);i+=1;
         }
         if (fnllp)
         {
            BINDPARAM(i,nllp);i+=1;
         }
         if (flrplce)
         {
            BINDPARAM(i,lrplce);i+=1;
         }
         if (fsnl3id)
         {
            BINDPARAM(i,snl3id);i+=1;
         }
         if (fsnl2id)
         {
            BINDPARAM(i,snl2id);i+=1;
         }
         if (fsnl1id)
         {
            BINDPARAM(i,snl1id);i+=1;
         }
         if (fcntryid)
         {
            BINDPARAM(i,cntryid);i+=1;
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
      central.Execute();
   }
 
   source = (fLocal)?&local:&central;


   ZeroMemory(&_data,sizeof(_data));
   if (source->Fetch())
      data = _data;
   else if (source->NoData())
      ret = GMS_NO_DATA;
   else 
      ret = GMS_ERROR;

   POP();

#undef SQL_GETLOCATION2
}



//////////////////////////////////////////////////////////////////////////////
//  Created: October 22, 2004
//  Description: Retrieves location records based on the specified SNL3ID, SNL2ID, SNL1ID
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_getLocation3(GMS_Location *data, LONG fSearchOption)
{
   static BOOL fLocal=TRUE;

   PUSH(GMS_getLocation3);
   if (fSearchOption==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal){
      ret=getLocation3(LOCAL,*data,fSearchOption);
      if (ret!=GMS_SUCCESS){
         fLocal = FALSE; 
         ret=getLocation3(CENTRAL,*data,FIND_NEXT);
      }
   }
   else
      ret=getLocation3(CENTRAL,*data,fSearchOption);
   POP();
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG getLocation3(BOOL fLocal, GMS_Location &data, LONG fSearchOption)
{
#define SQL_GETLOCATION3  "\
   select LTYPE,NLLP,\
     LRPLCE,LNAME,LABBR,SNL3ID,SNL2ID,SNL1ID,CNTRYID, LOCID \
   from LOCATION \
   where SNL3ID = ? AND SNL2ID= ? AND SNL1ID =  ? AND LTYPE= ?\
   order by LOCID"

   PUSH(getLocation3);


static CODBCbindedStmt &central= _centralDBC->BindedStmt(SQL_GETLOCATION3);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_GETLOCATION3);
       CODBCbindedStmt *source;

static GMS_Location _data;
static BOOL first_time=TRUE;

static LONG _snl3id, _snl2id, _snl1id, _ltype;

   if (fSearchOption==FIND_FIRST)
   {
      if (first_time)
      {
         BIND(1,_data.ltype);
         BIND(2,_data.nllp);
         BIND(3,_data.lrplce);
         BINDS(4,_data.lname,GMS_LNAME_SIZE);
         BINDS(5,_data.labbr,GMS_LABBR_SIZE);
         BIND(6,_data.snl3id);
         BIND(7,_data.snl2id);
         BIND(8,_data.snl1id);
         BIND(9,_data.cntryid);
		 BIND(10,_data.locid);
	     BINDPARAM(1,_snl3id);
	     BINDPARAM(2,_snl2id);
	     BINDPARAM(3,_snl1id);
	     BINDPARAM(4,_ltype);
 
         first_time=FALSE;
      }


	  _snl3id=data.snl3id;
	  _snl2id = data.snl2id;
      _snl1id = data.snl1id; 
	  _ltype = data.ltype;
      local.Execute();
      central.Execute();
   }

   source = (fLocal)?&local:&central;

   ZeroMemory(&_data,sizeof(_data));
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


//////////////////////////////////////////////////////////////////////////////
//Date Created: October 25, 2005
//Programmer by: AMP
//Description:  Retrieves the location record given any of the SNL3ID, SNL21ID or SNL1ID
//////////////////////////////////////////////////////////////////////////////
LONG getLocation4(BOOL fLocal, GMS_Location &data, LONG fSearchOption)
{
#define SQL_GETLOCATION "\
   select LTYPE,NLLP,\
     LRPLCE,LNAME,LABBR,SNL3ID,SNL2ID,SNL1ID,CNTRYID, LOCID \
   from LOCATION \
    where (SNL3ID = ? OR 0=?) AND (SNL2ID= ? OR 0=?) AND (SNL1ID =  ? OR 0=?) AND LTYPE= ?\
   order by LOCID"

   PUSH(getLocation3);


static CODBCbindedStmt &central= _centralDBC->BindedStmt(SQL_GETLOCATION);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_GETLOCATION);
       CODBCbindedStmt *source;

static GMS_Location _data;
static BOOL first_time=TRUE;

static LONG _snl3id, _snl2id, _snl1id, _ltype;

   if (fSearchOption==FIND_FIRST)
   {
      if (first_time)
      {
         BIND(1,_data.ltype);
         BIND(2,_data.nllp);
         BIND(3,_data.lrplce);
         BINDS(4,_data.lname,GMS_LNAME_SIZE);
         BINDS(5,_data.labbr,GMS_LABBR_SIZE);
         BIND(6,_data.snl3id);
         BIND(7,_data.snl2id);
         BIND(8,_data.snl1id);
         BIND(9,_data.cntryid);
		 BIND(10,_data.locid);
	     BINDPARAM(1,_snl3id);
	     BINDPARAM(2,_snl3id);
	     BINDPARAM(3,_snl2id);
	     BINDPARAM(4,_snl2id);
	     BINDPARAM(5,_snl1id);
	     BINDPARAM(6,_snl1id);
	     BINDPARAM(7,_ltype);
 
         first_time=FALSE;
      }


	  _snl3id=data.snl3id;
	  _snl2id = data.snl2id;
      _snl1id = data.snl1id; 
	  _ltype = data.ltype;
      local.Execute();
      central.Execute();
   }

   source = (fLocal)?&local:&central;

   ZeroMemory(&_data,sizeof(_data));
   if (source->Fetch())
   {
      data = _data;
   }
   else if (source->NoData())
      ret = GMS_NO_DATA;
   else 
      ret = GMS_ERROR;

   POP();

#undef SQL_GETLOCATION4
}




//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_findLocation(GMS_Location *data, LONG fSearchOption)
{
   static BOOL fLocal=TRUE;

   PUSH(GMS_findLocation);

   if (fSearchOption==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal){
      ret=findLocation(LOCAL,*data,fSearchOption);
      if (ret!=GMS_SUCCESS){
         fLocal = FALSE; 
         ret=findLocation(CENTRAL,*data,FIND_NEXT);
      }
   }
   else
      ret=findLocation(CENTRAL,*data,fSearchOption);
   POP();
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG findLocation(BOOL fLocal, GMS_Location &data, LONG fSearchOption)
{
#define SQL_FINDLOCATION "\
   select LOCID,LTYPE,NLLP,\
     LRPLCE,LNAME,LABBR,SNL3ID,SNL2ID,SNL1ID,CNTRYID\
   from LOCATION \
   where (LNAME like ?) and (0=? or LTYPE=?)\
   order by LOCATION.LOCID"

   PUSH(findLocation);

static CODBCbindedStmt &central= _centralDBC->BindedStmt(SQL_FINDLOCATION);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_FINDLOCATION);
       CODBCbindedStmt *source;

static GMS_Location _data;
static CHAR lname[GMS_LNAME_SIZE];
static LONG ltype;

static BOOL first_time=TRUE;


   if (fSearchOption==FIND_FIRST){
      if (first_time)
      {
         BIND(1,_data.locid);
         BIND(2,_data.ltype);
         BIND(3,_data.nllp);
         BIND(4,_data.lrplce);
         BINDS(5,_data.lname,GMS_LNAME_SIZE);
         BINDS(6,_data.labbr,GMS_LABBR_SIZE);
         BIND(7,_data.snl3id);
         BIND(8,_data.snl2id);
         BIND(9,_data.snl1id);
         BIND(10,_data.cntryid);
	      BINDPARAMS(1,lname,GMS_LNAME_SIZE);
	      BINDPARAM(2,ltype);
	      BINDPARAM(3,ltype);
 
         first_time=FALSE;
      }

	   ltype=data.ltype;
      strcpy(lname,data.lname);
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
#undef SQL_FINDLOCATION
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_addLocation(GMS_Location *data)
{
   if (UACCESS<30)
      return GMS_NO_ACCESS;
   LONG ret= addLocation(*data);
   return ret;
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG getNextLOCID(BOOL fLocal)
{
   static LONG localLOCID=0,centralLOCID=0;
   if (fLocal )
   {
	   if ( !localLOCID){
         CODBCdirectStmt source1 = _localDBC->DirectStmt("SELECT MIN(ULOCN) FROM INSTLN") ;
         source1.Execute();
         source1.Fetch();
		 localLOCID=source1.Field(1).AsInteger();
      }
      CODBCdirectStmt source2 = _localDBC->DirectStmt("SELECT MIN(LOCID) FROM LOCATION") ;
      source2.Execute();
      source2.Fetch();
      localLOCID=min(localLOCID,source2.Field(1).AsInteger());
   }
   //else if (!fLocal && !centralLOCID)
   //{
   //   CODBCdirectStmt source = _centralDBC->DirectStmt("SELECT MAX(LOCID) FROM LOCATION") ;
   //   source.Execute();
   //   source.Fetch();
   //   centralLOCID=source.Field(1).AsInteger();
   //}
   //return (fLocal)? --localLOCID:++centralLOCID;
   return  --localLOCID;
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG getNextLDID(BOOL fLocal)
{
   static LONG localLDID=0,centralLDID=0;
   if (fLocal && !localLDID)
   {
      CODBCdirectStmt source1 = _localDBC->DirectStmt("SELECT MIN(ULDID) FROM INSTLN") ;
      source1.Execute();
      source1.Fetch();
      CODBCdirectStmt source2 = _localDBC->DirectStmt("SELECT MIN(LDID) FROM LOCDES") ;
      source2.Execute();
      source2.Fetch();
      localLDID=min(source1.Field(1).AsInteger(),source2.Field(1).AsInteger());
   }
   else if (!fLocal && !centralLDID)
   {
      CODBCdirectStmt source = _centralDBC->DirectStmt("SELECT MAX(LDID) FROM LOCDES") ;
      source.Execute();
      source.Fetch();
      centralLDID=source.Field(1).AsInteger();
   }
   return (fLocal)? --localLDID:++centralLDID;
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG	addLocation(GMS_Location &location)
{
#define SQL_ADDLOCATION "\
  insert into LOCATION\
  (LOCID, LTYPE,NLLP\
  ,LRPLCE,LNAME,LABBR\
  ,SNL3ID,SNL2ID,SNL1ID\
  ,CNTRYID)\
  values (?,?,?,?,?,?,?,?,?,?)"

    PUSH(addLocation)

static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_ADDLOCATION);
       
static GMS_Location _data;
static BOOL first_time=TRUE;
   
   if (first_time)
   {
      local.BindParam(1,_data.locid);
      local.BindParam(2,_data.ltype);
      local.BindParam(3,_data.nllp);
      local.BindParam(4,_data.lrplce);
      local.BindParam(5,_data.lname,GMS_LNAME_SIZE);
      local.BindParam(6,_data.labbr,GMS_LABBR_SIZE);
      local.BindParam(7,_data.snl3id);
      local.BindParam(8,_data.snl2id);
      local.BindParam(9,_data.snl1id);
      local.BindParam(10,_data.cntryid);

      first_time=FALSE;
   }
   location.locid = getNextLOCID(TRUE);
   _data=location;
   if ((ret=local.Execute())==1)
      return GMS_SUCCESS;
   else if (local.NoData())
      return GMS_NO_DATA;
   else 
	   if (ret!=GMS_UNIQUE_CONSTRAINT_ERROR) //GMS_UNIQUE_CONSTRAINT_ERROR
		   ret=GMS_ERROR;  
       // Handler for unique contraint error with the assumption that only NID has unique contraint 
	   else {
		   int i=0;
		   while ((ret==GMS_UNIQUE_CONSTRAINT_ERROR) && (i<100)) {
				_data.locid = getNextLOCID(TRUE);
				ret=local.Execute();
				++i;
		   }
		   if ((ret) &&(ret!=GMS_UNIQUE_CONSTRAINT_ERROR)) {
			   location.locid = _data.locid;
			   ret = GMS_SUCCESS;
		   }
		   else {
			   location.locid = 0;
			   ret = GMS_ERROR;
		   }
	   }

	   POP();
#undef SQL_ADDLOCATION
}

