/* Germplsm.cpp : Implements the functions to access and manipulate the GERMPLSM table of ICIS
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

#include "germplsm.h"
#include "changes.h"

extern LONG UACCESS,USERID, hasSID;
extern CODBCdbc *_centralDBC,*_localDBC;  //Central and Local Database Connection
extern stack<char*> _routines_stack;
extern bool AUTOCOMMIT, SHOW_PREFID;

#if defined(WEB)
extern bool WEBSERVICE;  //True if calls are to Web Service
extern _bstr_t szURL;    // URL pointing to the WSDL of the Web Service
#endif

LONG getNextGID(BOOL fLocal)
{

#define SQL_getGID "select MIN(GID) from GERMPLSM"

static  CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_getGID); 

static LONG localGID=0, nextGID;
  
   if (fLocal)
   {
      
	  if (!localGID)
      {
		CODBCdirectStmt source1 = _localDBC->DirectStmt("select MIN(UGID) from INSTLN") ;
		source1.Execute();
		source1.Fetch();
		localGID = source1.Field(1).AsInteger();

	    local.Bind(1,nextGID);
      }
	  
      local.Execute();
      local.Fetch();
      if (local.NoData())
         nextGID=-1;
	  
    //CODBCdirectStmt  source2 = _localDBC->DirectStmt("select MIN(GID) from GERMPLSM") ;      	  
    //source2.Execute();
   //   source2.Fetch();
      localGID=min(localGID , nextGID);
	  --localGID;

   }
 	return localGID;
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG addGermplasmRecord(BOOL fLocal, GMS_Germplasm &germ,LONG *progenitors)
{
#define SQL_ADDGERMPLSMREC "\
   insert into GERMPLSM\
   (GID,METHN,GNPGS,GPID1,\
   GPID2,GERMUID,LGID,GLOCN,\
   GDATE,GREF,GRPLCE,MGID)\
   values ( ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"

   PUSH(addGermplasmRecord);

//static CODBCbindedStmt &central= _centralDBC->BindedStmt(SQL_ADDGERMPLSMREC);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_ADDGERMPLSMREC);
       CODBCbindedStmt *source;

static GMS_Germplasm _data;
static BOOL first_time=TRUE;

   if (first_time)
   {
	  local.BindParam(1,_data.gid);
      local.BindParam(2,_data.methn);
      local.BindParam(3,_data.gnpgs);
      local.BindParam(4,_data.gpid1);
      local.BindParam(5,_data.gpid2);
      local.BindParam(6,_data.guid);
      local.BindParam(7,_data.lgid);
      local.BindParam(8,_data.glocn);
      local.BindParam(9,_data.gdate);
      local.BindParam(10,_data.gref);
      local.BindParam(11,_data.grplce);
      local.BindParam(12,_data.ggid);
 
      first_time=FALSE;
   }

   //source = (fLocal)?&local:&central;
   source = &local; 
   germ.gid=getNextGID(fLocal);  //commented on 20071018
   //germ.gid=0;
   germ.guid = USERID;

   _data = germ;

// if (source->Execute())
   if ((ret=local.Execute())==1)   //2007711 (AMP): assigned to ret
   {
      ret=GMS_SUCCESS;
   }
   else
   {
	   if (ret!=-12) //GMS_UNIQUE_CONSTRAINT_ERROR
		   ret=GMS_ERROR;  
       // Handler for unique contraint error with the assumption that only GID has unique contraint 
	   else {
		   int i=0;
		   while ((ret==-12) && (i<100)) {
                germ.gid=getNextGID(fLocal);  //commented on 20071018
				_data.gid = germ.gid;
				ret=source->Execute();
				++i;
		   }
		   if ((ret) &&(ret!=-12))  ret = GMS_SUCCESS;
		   else  {
			   germ.gid = 0;
			   ret = GMS_ERROR;
		   }
	   }
   }

   if (ret=GMS_SUCCESS) {
		if (_data.gnpgs>2)
			{
				if (progenitors == NULL)
				{
					deleteGermplasm(fLocal,_data.gid);
					ret =GMS_ERROR; 
				}
				else{
					for (LONG pno=3;pno<=_data.gnpgs;pno++) 
					if ( (ret=addProgenitorID(fLocal,_data.gid,pno,progenitors[pno-3]))!=GMS_SUCCESS)
					{
						deleteGermplasm(fLocal,_data.gid);
						ret =GMS_ERROR; 
					}
				}
			}
	}

   POP();

#undef SQL_ADDGERMPLSMREC
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_addGermplasm(GMS_Germplasm *data,GMS_Name *name,CHAR *szName,LONG *progenitors)
{
   PUSH(GMS_addGermplasm);
   ret = addGermplasm(LOCAL,*data,*name,szName,progenitors);
   POP();
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG addGermplasm(BOOL fLocal, GMS_Germplasm &germ,GMS_Name &name,CHAR *szName,LONG *progenitors)
{
   PUSH(addGermplasm);
   if ((ret=addGermplasmRecord(fLocal,germ,progenitors))==GMS_SUCCESS)
   {
      
      name.gid=germ.gid;
      name.nstat=1;
	  if (AUTOCOMMIT) {
		  GMS_commitData();
	  }
      if ( (ret=addName(fLocal,name,szName,GMS_PREFFERED))!=GMS_SUCCESS)
      {
         deleteGermplasm(fLocal,germ.gid);
	  }
	  else
   		if (AUTOCOMMIT) 
		   GMS_commitData();
	  
 
   }
   else
	   LOG("addGermplasm Error: %d\n",ret);
   POP();
}

//////////////////////////////////////////////////////////////////////////////
// GMS_getGermplasmRecord
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_getGermplasmRecord(LONG gid, GMS_Germplasm *data)
{
   PUSH(GMS_getGermplasmRecord);
#if defined(WEB)
   if ((WEBSERVICE==true) && (gid > 0)) {
		L_Germplasm lgerms;
		IManagedInterface *gGR = NULL;
		//Enter Single Threaded Apartment (STA) - STA Thread
		CoInitialize(NULL);
		//Instantiate the COM object in the appropriate apartment
		HRESULT hr = CoCreateInstance(CLSID_LibraryImplementation,NULL, CLSCTX_INPROC_SERVER, IID_IManagedInterface, reinterpret_cast<void**>(&gGR)); 
		if(FAILED(hr))
		{
			MessageBox(NULL,"Could not create Instance, Web Service Failed!","ICIS V5",MB_OK);
		}

		ret = gGR->LgetGermplasmRecord(gid,&lgerms,szURL);  
		if (ret==GMS_SUCCESS)
		{
			data->gdate = lgerms.GDATE;   
			data->gid = lgerms.GGID; 
			data->glocn = lgerms.GLOCN;
			data->gnpgs = lgerms.GNPGS;
			data->gpid1 = lgerms.GPID1;
			data->gpid2 = lgerms.GPID2;
			data->gref = lgerms.GREF;
			data->grplce = lgerms.GRPLCE;
			data->guid = lgerms.GERMUID; 
			data->lgid = lgerms.LGID;
			data->methn = lgerms.METHN; 
		}
		CoUninitialize();
		gGR->Release();
   }
   else
#endif

      ret=getGermplasmRecord((gid <0),gid,*data);
  
   POP();
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG getGermplasmRecord(BOOL fLocal,LONG gid, GMS_Germplasm &grmplsm)
{
#define SQL_GETGRMPLSMREC "\
  select GERMPLSM.METHN, GERMPLSM.GNPGS, GERMPLSM.GPID1\
  ,GERMPLSM.GPID2, GERMPLSM.GERMUID, GERMPLSM.LGID, GERMPLSM.GLOCN\
  ,GERMPLSM.GDATE,GERMPLSM.GREF, GERMPLSM.GRPLCE, GERMPLSM.MGID \
  from GERMPLSM \
  where GID=?"

  LONG ret=GMS_SUCCESS;//PUSH(getGermplasmRecord);

static CODBCbindedStmt &central= _centralDBC->BindedStmt(SQL_GETGRMPLSMREC);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_GETGRMPLSMREC);
       CODBCbindedStmt *source;

static GMS_Germplasm _data;
static BOOL first_time=TRUE;

//Parameters
static LONG _gid;

   if (first_time)
   {
      BIND(1,_data.methn);
      BIND(2,_data.gnpgs);
      BIND(3,_data.gpid1);
      BIND(4,_data.gpid2);
      BIND(5,_data.guid);
      BIND(6,_data.lgid);
      BIND(7,_data.glocn);
      BIND(8,_data.gdate);
      BIND(9,_data.gref);
      BIND(10,_data.grplce);
      BIND(11,_data.ggid);
      BINDPARAM(1,_gid);

      first_time = FALSE;
   }

   source = (fLocal)?&local:&central;

   _gid=gid;
   source->Execute();
   if (source->Fetch())
   {
      ret=GMS_SUCCESS;
      grmplsm = _data;
      grmplsm.gid = _gid;
      if (grmplsm.grplce==_gid)
         ret = GMS_NO_DATA;
	  else {
          if (grmplsm.gid > 0)
			   applyGermplasmChanges(grmplsm);  
	  }
   }
   else if (source->NoData())
      ret = GMS_NO_DATA;
   else 
      ret = GMS_ERROR;

   return ret;//POP();

#undef SQL_GETGRMPLSMREC
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG applyGermplasmChanges(GMS_Germplasm &germ)
{

   LONG ret=GMS_SUCCESS;//PUSH(applyGermplasmChanges);

   LONG replacement;
   LONG _newvalue;

   //Deleted or Replaced
   if (GMS_SUCCESS==getReplacementGermplasmFromChanges(germ.gid,replacement))
   {
      // Deleted
      if (germ.gid==replacement)
         ret=GMS_NO_DATA;
      //Replaced
      else
      {
         ret=getGermplasmRecord(replacement<0,replacement,germ);
         if (ret==GMS_SUCCESS)
            ret=applyGermplasmChanges(germ);
      }
   }
   else
   {
	   //if germplasm ID is positive, get CHANGES records.
	   if (germ.gid >=0) {   
			//Check each field
			getGermplasmFieldFromChanges(germ.gid,"METHN",germ.methn);
			getGermplasmFieldFromChanges(germ.gid,"GLOCN",germ.glocn);
			getGermplasmFieldFromChanges(germ.gid,"GNPGS",germ.gnpgs);
			getGermplasmFieldFromChanges(germ.gid,"GPID1",germ.gpid1);
  			//*_newvalue = germ.gpid1;
			// getGermplasmFieldFromChanges(germ.gid,"GPID1",*_newvalue);
			// germ.gpid1 = *_newvalue;

			getGermplasmFieldFromChanges(germ.gid,"GPID2",germ.gpid2);
			getGermplasmFieldFromChanges(germ.gid,"GDATE",germ.gdate);
			getGermplasmFieldFromChanges(germ.gid,"GREF",germ.gref);
			ret=GMS_SUCCESS;
	   }
   }

   return ret;//POP();
}

//////////////////////////////////////////////////////////////////////////////
// GMS_getGermplasm
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_getGermplasm(LONG gid, GMS_Germplasm *data)
{
   PUSH(GMS_getGermplasm);
   ret = getGermplasm((gid <0),gid,*data);
   POP();
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG getGermplasm(BOOL fLocal,LONG gid, GMS_Germplasm &germ)
{
   //PUSH(getGermplasm);
   //LONG ret=getGermplasmRecord(fLocal,gid,germ);
   LONG ret=GMS_getGermplasmRecord(gid,&germ);
   if (ret==GMS_SUCCESS) 
   {
      if (germ.grplce!=0)
         return getGermplasm(germ.grplce<0,germ.grplce,germ);
      ret=applyGermplasmChanges(germ);
      if (!(ret==GMS_SUCCESS || ret==GMS_NO_DATA))
      {
         if (germ.grplce==germ.gid)
            ret=GMS_NO_DATA;
         else if (germ.grplce!=0)
            ret=getGermplasm(fLocal,germ.grplce,germ);
         else
            ret = GMS_SUCCESS;
      }
   }
   //POP();
   return ret;
}



//////////////////////////////////////////////////////////////////////////////
// GMS_getGermplasm2   
// Deactivated: Oct. 11, 2002
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_getGermplasm2_(LONG gid, GMS_Germplasm *data, GMS_Name *name
                   , LPSTR szName, LONG nszName)
{
   PUSH(GMS_getGermplasm2);

   //ret=getGermplasm(gid<0,gid,*data);
   //if (ret==GMS_SUCCESS)
  // {
   //   name->gid=data->gid;
   //   ret=getPrefName(LOCAL,*name,szName,nszName,FIND_FIRST);
   //   if (ret!=GMS_SUCCESS && gid>0)
   //      ret=getPrefName(CENTRAL,*name,szName,nszName,FIND_FIRST);
  // }
   ret=getGermplasm2(gid<0,gid,*data,*name,szName,nszName);
   if (gid>0)
      getPrefName(LOCAL,*name,szName,nszName,FIND_FIRST);
   POP();
}

//////////////////////////////////////////////////////////////////////////////
// GMS_getGermplasm2
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_getGermplasm2(LONG gid, GMS_Germplasm *data, GMS_Name *name
                   , LPSTR szName, LONG nszName)
{
   LONG ret2,nid;
   BOOLEAN blCentralName, noPrefName=false;
   PUSH(GMS_getGermplasm2);

   //ret=getGermplasm(gid<0,gid,*data);
   //if (ret==GMS_SUCCESS)
  // {
   //   name->gid=data->gid;
   //   ret=getPrefName(LOCAL,*name,szName,nszName,FIND_FIRST);
   //   if (ret!=GMS_SUCCESS && gid>0)
   //      ret=getPrefName(CENTRAL,*name,szName,nszName,FIND_FIRST);
  // }
   ret=getGermplasm2_2(gid<0,gid,*data);
   if (ret==GMS_SUCCESS)
   {
     name->gid=data->gid;
	 blCentralName = FALSE;
     ret2=getPrefName(LOCAL,*name,szName,nszName,FIND_FIRST);
     if (ret2!=GMS_SUCCESS && data->gid>0) {
        ret2=getPrefName(CENTRAL,*name,szName,nszName,FIND_FIRST);
		blCentralName = TRUE;
     }
	 //if there is no preferred name, check for preferred ID
	 if (ret2==GMS_NO_DATA) {
		 name->gid=data->gid;
		 name->nstat = 8;  
         ret2=GMS_getName(name,szName,nszName,FIND_FIRST);
		 noPrefName=true;
	 }
	 else {
		 noPrefName=false;
		 // Preferred ID  will be attached with the preferred name
		 if (SHOW_PREFID) {
			 char szPrefID[GMS_NVAL_SIZE];
			 long origNstat = name->nstat;
			 name->gid=data->gid;
			 name->nstat = 8;  
	         ret2=GMS_getName(name,szPrefID,GMS_NVAL_SIZE,FIND_FIRST);
			 if (ret2==GMS_SUCCESS) {
			    string newName;
				newName = szName;
				newName = newName + ", " + szPrefID;
				strncpy(szName,(char*) newName.c_str(), newName.length());
				//strcat(szName,",");
				//strcat(szName,szPrefID);
			 }
			 name->nstat = origNstat;
		 }
	 }
     if (ret2 == GMS_SUCCESS)  
	 {
      nid = name->nid;
	  getNameFieldFromChanges(nid,"NSTAT",name->nstat);
      if (((name->nstat != 1) && (!noPrefName)) || ((name->nstat !=8)&& (noPrefName) )) {
		  name->ntype =0;
		  name->nstat = 0;
          ret2 = GMS_getName(name,szName,nszName,FIND_FIRST);
		  while (ret2) {
			  if (name->nstat == 1) {
                  break;                 
			  }
			  if ((noPrefName)  && (name->nstat ==8)) {
				  break;
			  }
             ret2 = GMS_getName(name,szName,nszName,FIND_NEXT);
		  }
		  if (ret2 == GMS_NO_DATA)  {
              ZeroMemory(name,sizeof(name));
		      ZeroMemory(szName, sizeof(szName));
		  }
	  }
	  else {
	    getNameFieldFromChanges(nid,"NTYPE",name->ntype);
	    getNameFieldFromChanges(nid,"NUID",name->nuid);
	    //getNameFieldFromChanges(nid,"NVAL",data->nval);
	    getNameFieldFromChanges(nid,"NLOCN",name->nlocn);
	    getNameFieldFromChanges(nid,"NDATE",name->ndate);
	    getNameFieldFromChanges(nid,"NREF",name->nref);

	  }

	 }


   }

   POP();
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG getGermplasm2(BOOL fLocal,LONG gid,GMS_Germplasm &germ,GMS_Name &name
                   ,LPSTR szName,LONG nszName)
{
	PUSH(getGermplasm2);

#if defined(WEB)
	if((WEBSERVICE==true) && (fLocal==false))
	{
		L_Germplasm lgerms;
		IManagedInterface *gGR = NULL;
		//Enter Single Threaded Apartment (STA) - STA Thread
		CoInitialize(NULL);

		//Instantiate the COM object in the appropriate apartment
		HRESULT hr = CoCreateInstance(CLSID_LibraryImplementation,
			NULL, CLSCTX_INPROC_SERVER, IID_IManagedInterface, reinterpret_cast<void**>(&gGR)); 

		if(FAILED(hr))
		{
			MessageBox(NULL,"Could not create Instance, Web Service Failed!","ICIS V5",MB_OK);
		}
		int val;

		val = gGR->LgetGermplasmRecord(gid,&lgerms,szURL);  

		if((val==-1) || (val == 0))
		{		
			if(val == -1) {
				MessageBox(NULL,"WebService Connection Lost. Unable to retrieve data","ICIS V5",MB_ICONINFORMATION);
				ret = GMS_ERROR; 
				exit(EXIT_SUCCESS);
			}
			else
			{
				ret = GMS_NO_DATA;
				CoUninitialize();
				gGR->Release();
				ret = GMS_NO_DATA;
			}
		}
		else
		{
			germ.gdate = lgerms.GDATE;   
			germ.gid = lgerms.GGID; 
			germ.glocn = lgerms.GLOCN;
			germ.gnpgs = lgerms.GNPGS;
			germ.gpid1 = lgerms.GPID1;
			germ.gpid2 = lgerms.GPID2;
			germ.gref = lgerms.GREF;
			germ.grplce = lgerms.GRPLCE;
			germ.guid = lgerms.GERMUID; 
			germ.lgid = lgerms.LGID;
			germ.methn = lgerms.METHN; 

			ret = GMS_SUCCESS;
		}
	}
	else
#endif
	{
	//2001-10-15: delete NAMES.NEXT
#define SQL_GETGRMPLSM2 "\
  select GERMPLSM.METHN, GERMPLSM.GNPGS, GERMPLSM.GPID1\
  ,GERMPLSM.GPID2, GERMPLSM.GERMUID, GERMPLSM.LGID, GERMPLSM.GLOCN\
  ,GERMPLSM.GDATE,GERMPLSM.GREF, GERMPLSM.GRPLCE, GERMPLSM.MGID\
  ,NAMES.NTYPE, NAMES.NSTAT, NAMES.NUID, NAMES.NVAL, NAMES.NLOCN\
  ,NAMES.NDATE, NAMES.NREF, NAMES.NID \
  from GERMPLSM,NAMES\
  where GERMPLSM.GID=?\
    and NAMES.GID=?\
    and NAMES.NSTAT=1"

    PUSH(getGermplasm2);
static CODBCbindedStmt &central= _centralDBC->BindedStmt(SQL_GETGRMPLSM2);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_GETGRMPLSM2);
       CODBCbindedStmt *source;
static BOOL first_time=TRUE;

static GMS_Name _name;
static GMS_Germplasm _germ;
static CHAR _nval[GMS_MAX_NVAL+1],_next[MAX_STR];


   if (first_time)
   {
      BIND(1,_germ.methn);
      BIND(2,_germ.gnpgs);
      BIND(3,_germ.gpid1);
      BIND(4,_germ.gpid2);
      BIND(5,_germ.guid);
      BIND(6,_germ.lgid);
      BIND(7,_germ.glocn);
      BIND(8,_germ.gdate);
      BIND(9,_germ.gref);
      BIND(10,_germ.grplce);
	  BIND(11,_germ.ggid);
      BIND(12,_name.ntype);
      BIND(13,_name.nstat);
      BIND(14,_name.nuid);
      BINDS(15,_nval,GMS_MAX_NVAL+1);
      BIND(16,_name.nlocn);
      BIND(17,_name.ndate);
      BIND(18,_name.nref);
	  BIND(19,_name.nid);
     // BINDS(18,_next,MAX_STR);
      BINDPARAM(1,_germ.gid);
      BINDPARAM(2,_germ.gid);

      first_time = FALSE;
   }

   source = (fLocal)?&local:&central;

   _germ.gid=gid;
   source->Execute();
   _next[0]='\0';
   _nval[0]='\0';
   if (source->Fetch()){
      if (_germ.grplce==0)// Not Deleted Nor Replaced
      {
         string namestr;
         namestr = _nval;
         namestr +=_next;
         germ = _germ;
         name = _name;
         name.gid=_germ.gid;
         strncpy(szName,namestr.c_str(),nszName);
         szName[nszName-1]='\0';
    
         // Check Local Changes to the Central Germplasm Record
         if (germ.gid>0)
         {
            LONG replacement;
            if (getReplacementGermplasmFromChanges(germ.gid,replacement)==GMS_SUCCESS)
            {
               if (replacement==germ.gid)
               {
                  POP2(GMS_NO_DATA);
               }
               ret = getGermplasm2(replacement<0,replacement,germ,name,szName,nszName);
            }
            if (applyGermplasmChanges(germ)==GMS_SUCCESS)
                  _germ = germ;
         }
      }
      else if (_germ.grplce==gid) //Deleted
      {
         germ.gid=name.gid=0;
         ret = GMS_NO_DATA;
      }
      else //Replaced
         ret = getGermplasm2(_germ.grplce<0,_germ.grplce,germ,name,szName,nszName);
   }
   else if (source->NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;
	}
   POP();

#undef SQL_GETGRMPLSM2
}


//////////////////////////////////////////////////////////////////////////////
//Created: Oct 10, 2002
// Description:  Similar to getGermplasm2 except no NAME structure
//////////////////////////////////////////////////////////////////////////////
LONG getGermplasm2_2(BOOL fLocal,LONG gid,GMS_Germplasm &germ)
{
	//2001-10-15: delete NAMES.NEXT
#define SQL_GETGRMPLSM2_2 "\
  select GERMPLSM.METHN, GERMPLSM.GNPGS, GERMPLSM.GPID1\
  ,GERMPLSM.GPID2, GERMPLSM.GERMUID, GERMPLSM.LGID, GERMPLSM.GLOCN\
  ,GERMPLSM.GDATE,GERMPLSM.GREF, GERMPLSM.GRPLCE, GERMPLSM.MGID\
  from GERMPLSM \
  where GERMPLSM.GID=?"

    PUSH(getGermplasm2_2);
static CODBCbindedStmt &central= _centralDBC->BindedStmt(SQL_GETGRMPLSM2_2);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_GETGRMPLSM2_2);
       CODBCbindedStmt *source;
static BOOL first_time=TRUE;

static GMS_Germplasm _germ;


   if (first_time)
   {
      BIND(1,_germ.methn);
      BIND(2,_germ.gnpgs);
      BIND(3,_germ.gpid1);
      BIND(4,_germ.gpid2);
      BIND(5,_germ.guid);
      BIND(6,_germ.lgid);
      BIND(7,_germ.glocn);
      BIND(8,_germ.gdate);
      BIND(9,_germ.gref);
      BIND(10,_germ.grplce);
	  BIND(11,_germ.ggid);
     // BINDS(18,_next,MAX_STR);
      BINDPARAM(1,_germ.gid);

      first_time = FALSE;
   }

   source = (fLocal)?&local:&central;

   _germ.gid=gid;
   source->Execute();
   if (source->Fetch()){
      if (_germ.grplce==0)// Not Deleted Nor Replaced
      {
         string namestr;
         germ = _germ;

         // Check Local Changes to the Central Germplasm Record
         if (germ.gid>0)
         {
            LONG replacement;
            if (getReplacementGermplasmFromChanges(germ.gid,replacement)==GMS_SUCCESS)
            {
               if (replacement==germ.gid)
               {
                  POP2(GMS_NO_DATA);
               }
               ret = getGermplasm2_2(replacement<0,replacement,germ);
            }
            if (applyGermplasmChanges(germ)==GMS_SUCCESS)
                  _germ = germ;
         }
      }
      else if (_germ.grplce==gid) //Deleted
      {
         germ.gid=0;
         ret = GMS_NO_DATA;
      }
      else //Replaced
         ret = getGermplasm2_2(_germ.grplce<0,_germ.grplce,germ);
   }
   else if (source->NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;

   POP();

#undef SQL_GETGRMPLSM2_2
}


//////////////////////////////////////////////////////////////////////////////
// GMS_getGermplasm3
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_getGermplasm3(LONG gid, GMS_Germplasm *data, GMS_Name *name
                   , LPSTR szName, LONG nszName)
{
   PUSH(GMS_getGermplasm3);

   //ret=getGermplasm(gid<0,gid,*data);
   //if (ret==GMS_SUCCESS)
  // {
   //   name->gid=data->gid;
   //   ret=getPrefName(LOCAL,*name,szName,nszName,FIND_FIRST);
   //   if (ret!=GMS_SUCCESS && gid>0)
   //      ret=getPrefName(CENTRAL,*name,szName,nszName,FIND_FIRST);
  // }
   ret=getGermplasm3(gid<0,gid,*data,*name,szName,nszName);
   if (gid>0)
      getPrefName(LOCAL,*name,szName,nszName,FIND_FIRST);
   POP();
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG getGermplasm3(BOOL fLocal,LONG gid,GMS_Germplasm &germ,GMS_Name &name
                   ,LPSTR szName,LONG nszName)
{
#define SQL_GETGRMPLSM3 "\
  select GERMPLSM.METHN, GERMPLSM.GNPGS, GERMPLSM.GPID1\
  ,GERMPLSM.GPID2, GERMPLSM.GERMUID, GERMPLSM.LGID, GERMPLSM.GLOCN\
  ,GERMPLSM.GDATE,GERMPLSM.GREF, GERMPLSM.GRPLCE, GERMPLSM.MGID\
  ,NAMES.NTYPE, NAMES.NSTAT, NAMES.NUID, NAMES.NVAL, NAMES.NLOCN\
  ,NAMES.NDATE, NAMES.NREF, NAMES.NID\
  from GERMPLSM,NAMES\
  where GERMPLSM.GID=?\
    and NAMES.GID=?\
    and NAMES.NSTAT=1"

    PUSH(getGermplasm3);
static CODBCbindedStmt &central= _centralDBC->BindedStmt(SQL_GETGRMPLSM3);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_GETGRMPLSM3);
       CODBCbindedStmt *source;
static BOOL first_time=TRUE;

static GMS_Name _name;
static GMS_Germplasm _germ;
static CHAR _nval[GMS_MAX_NVAL+1],_next[MAX_STR];


   if (first_time)
   {
      BIND(1,_germ.methn);
      BIND(2,_germ.gnpgs);
      BIND(3,_germ.gpid1);
      BIND(4,_germ.gpid2);
      BIND(5,_germ.guid);
      BIND(6,_germ.lgid);
      BIND(7,_germ.glocn);
      BIND(8,_germ.gdate);
      BIND(9,_germ.gref);
      BIND(10,_germ.ggid);
      BIND(11,_germ.grplce);
      BIND(12,_name.ntype);
      BIND(13,_name.nstat);
      BIND(14,_name.nuid);
      BINDS(15,_nval,GMS_MAX_NVAL+1);
      BIND(16,_name.nlocn);
      BIND(17,_name.ndate);
      BIND(18,_name.nref);
      BIND(19,_name.nid);
      BINDPARAM(1,_germ.gid);
      BINDPARAM(2,_germ.gid);

      first_time = FALSE;
   }

   source = (fLocal)?&local:&central;

   _germ.gid=gid;
   source->Execute();
   _next[0]='\0';
   _nval[0]='\0';
   if (source->Fetch()){
      if (_germ.grplce==0)// Not Deleted Nor Replaced
      {
         string namestr;
         namestr = _nval;
         namestr +=_next;
         germ = _germ;
         name = _name;
         name.gid=_germ.gid;
         strncpy(szName,namestr.c_str(),nszName);
         szName[nszName-1]='\0';
    
         // Check Local Changes to the Central Germplasm Record
         if (germ.gid>0)
         {
            LONG replacement;
            if (getReplacementGermplasmFromChanges(germ.gid,replacement)==GMS_SUCCESS)
            {
               if (replacement==germ.gid)
               {
                  POP2(GMS_NO_DATA);
               }
               ret = getGermplasm3(replacement<0,replacement,germ,name,szName,nszName);
            }
            if (applyGermplasmChanges(germ)==GMS_SUCCESS)
                  _germ = germ;
         }
      }
      else if (_germ.grplce==gid) //Deleted
      {
         germ.gid=name.gid=0;
         ret = GMS_NO_DATA;
      }
      else //Replaced
         ret = getGermplasm3(_germ.grplce<0,_germ.grplce,germ,name,szName,nszName);
   }
   else if (source->NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;

   POP();

#undef SQL_GETGRMPLSM3
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_setGermplasm(GMS_Germplasm *germ)
{
   PUSH(GMS_setGermplasm);

   if (UACCESS<30) 
      ret = GMS_NO_ACCESS;
   else if (germ->gid>=0)
      ret =GMS_ERROR;
   else
      ret= setGermplasm(TRUE, *germ);
   POP();

}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG setGermplasm(BOOL fLocal,GMS_Germplasm &germ)
{
#define SQL_SETGRMPLSMREC "\
  update GERMPLSM\
  set GERMPLSM.METHN=?, GERMPLSM.GNPGS=?, GERMPLSM.GPID1=?\
    ,GERMPLSM.GPID2=?, GERMPLSM.GERMUID=?, GERMPLSM.LGID=?, GERMPLSM.GLOCN=?\
    ,GERMPLSM.GDATE=?, GERMPLSM.GREF=?, GERMPLSM.GRPLCE=?, GERMPLSM.MGID=? \
  where GERMPLSM.GID=?"

  PUSH(setGermplasm);

static CODBCbindedStmt &central= _centralDBC->BindedStmt(SQL_SETGRMPLSMREC);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_SETGRMPLSMREC);
       CODBCbindedStmt *source;

static GMS_Germplasm _data;
static BOOL first_time=TRUE;

//Parameters
static LONG _gid;
   if (first_time)
   {
      BINDPARAM(1,_data.methn);
      BINDPARAM(2,_data.gnpgs);
      BINDPARAM(3,_data.gpid1);
      BINDPARAM(4,_data.gpid2);
      BINDPARAM(5,_data.guid);
      BINDPARAM(6,_data.lgid);
      BINDPARAM(7,_data.glocn);
      BINDPARAM(8,_data.gdate);
      BINDPARAM(9,_data.gref);
      BINDPARAM(10,_data.grplce);
      BINDPARAM(11,_data.ggid);
      BINDPARAM(12,_data.gid);

      first_time = FALSE;
   }

   source = (fLocal)?&local:&central;

   _data=germ;
   if (source->Execute())
      ret = GMS_SUCCESS;
   else if (source->NoData())
      ret = GMS_NO_DATA;
   else 
      ret = GMS_ERROR;

   POP();

#undef SQL_SETGRMPLSMREC
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_deleteGermplasm(LONG gid)
{
   PUSH(GMS_deleteGermplasm);
   if (UACCESS<30) 
      ret= GMS_NO_ACCESS;
   else if (gid>=0)
      ret= GMS_ERROR;
   else
      ret= deleteGermplasm(gid<0,gid);
   POP();

}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG deleteGermplasm(BOOL fLocal,LONG gid)
{
#define SQL_DELETEGERMPLSM "\
   delete from GERMPLSM where GERMPLSM.GID=?"

   PUSH(deleteGermplasm);

static CODBCbindedStmt &central= _centralDBC->BindedStmt(SQL_DELETEGERMPLSM);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_DELETEGERMPLSM);
       CODBCbindedStmt *source;

static LONG _gid;
static BOOL first_time=TRUE;

   if (first_time)
   {
      BINDPARAM(1,_gid);

      first_time=FALSE;
   }

   source = (fLocal)?&local:&central;

   _gid=gid;

   if (source->Execute())
      ret = GMS_SUCCESS;
   else if (source->NoData())
      ret = GMS_NO_DATA;
   else 
      ret = GMS_ERROR;

   POP();

#undef SQL_DELETEGERMPLSM
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_findGermplasm(GMS_Germplasm *data,LONG fSearchOption)
{
   static BOOL fLocal=TRUE;

   PUSH(GMS_findGermplasm);

   if (fSearchOption==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal){
      ret=findGermplasm(LOCAL,*data,fSearchOption);
      if (ret!=GMS_SUCCESS){
         fLocal = FALSE; 
         ret=findGermplasm(CENTRAL,*data,FIND_NEXT);
      }
   }
   else
      ret=findGermplasm(CENTRAL,*data,fSearchOption);
   POP();
}

//////////////////////////////////////////////////////////////////////////////
//Description: Retrieves germplasm record that meets the specified input parameter
//Input: Any of the field of the germplasm data structure
//Date Modified: Sept 5, 2006
//////////////////////////////////////////////////////////////////////////////
LONG findGermplasm(BOOL fLocal,GMS_Germplasm &germ,LONG fSearchOption)
{
#define SQL_FGERM_SELECT_FMT "select GID from GERMPLSM "
#define SQL_FGERM_WHERE_FMT " and %s=?"

   PUSH(findGermplasm);

static CODBCbindedStmt &central= _centralDBC->BindedStmt(SQL_FGERM_SELECT_FMT);  
static CODBCbindedStmt &local= _localDBC->BindedStmt(SQL_FGERM_SELECT_FMT);  
       CODBCbindedStmt *source;

static string strSQL="", prevstrSQL="";
static BOOL changed=TRUE;
static BOOL first_time=TRUE,fOK;
static BOOL fgpid1,fgpid2,fglocn,fguid,fgdate,fgnpgs,fgref,fgrplce,fmethn,flgid;
static LONG _gid,gpid1,gpid2,glocn,guid,gdate,gnpgs,gref,methn,lgid,ggid,  noSQL=0;
int result;


   if (fSearchOption==FIND_FIRST)
   {
         strSQL="select GID from GERMPLSM where";
         if ((gpid1=germ.gpid1)!=0)                                  
            strSQL = strSQL+" AND gpid1=?";
         if ((gpid2 =germ.gpid2)!=0)                                 
            strSQL = strSQL+" AND gpid2=?";
         if ((glocn =germ.glocn)!=0)                                   
            strSQL = strSQL+" AND glocn=?";                          
         if ((guid  =germ.guid)!=0)                                  
            strSQL = strSQL+" AND germuid =?";                          
         if ((gdate =germ.gdate)!=0)                                 
            strSQL = strSQL+" AND gdate=?";                           
         if ((gnpgs =germ.gnpgs)!=0)                                 
			 strSQL = strSQL+" AND gnpgs=?";                          
         if ((gref  =germ.gref)!=0)                                  
            strSQL = strSQL+" AND gref =?";                          
         if ((methn =germ.methn)!=0)                                 
            strSQL = strSQL+" AND methn=?";                          
         if ((lgid  =germ.lgid)!=0)                                  
            strSQL = strSQL+" AND lgid =?";
         if ((ggid  =germ.ggid)!=0)                                  
            strSQL = strSQL+" AND MGID =?";


         strSQL.erase(strSQL.find(" AND ",0),4);

		 //FOR DEBUG
		 //char error_msg[200];
		 //MessageBox(NULL,(LPSTR)strSQL.c_str(), "Debug Information", MB_OK+MB_ICONINFORMATION);

         result = strSQL.compare(prevstrSQL );
		 if (result!=0) {
			 prevstrSQL=strSQL;
             central = _centralDBC->BindedStmt((LPCSTR)strSQL.c_str());     //AMP:  added
             local   = _localDBC->BindedStmt((LPCSTR)strSQL.c_str());       //AMP:  added
		 }


         // Bind all Colums and Parameter
         BIND(1,_gid);
         int i=1;
         if (gpid1!=0)
         {
            BINDPARAM(i,gpid1);i+=1;
         }
         if (gpid2!=0)
         {
            BINDPARAM(i,gpid2);i+=1;
         }
         if (glocn!=0)
         {
            BINDPARAM(i,glocn);i+=1;
         }
         if (guid!=0 )
         {
            BINDPARAM(i,guid);i+=1;
         }
         if (gdate!=0)
         {
            BINDPARAM(i,gdate);i+=1;
         }
         if (gnpgs!=0)
         {
            BINDPARAM(i,gnpgs);i+=1;
         }
         if (gref!=0 )
         {
            BINDPARAM(i,gref);i+=1;
         }
         if (methn!=0)
         {
            BINDPARAM(i,methn);i+=1;
         }
         if (lgid!=0 )
         {
            BINDPARAM(i,lgid);i+=1;
         }
         if (ggid!=0 )
         {

            BINDPARAM(i,ggid);i+=1;
         }

		 //FOR DEBUG
         //sprintf((LPSTR)error_msg ,"\nNumber of parameter:%ld\n",i);
         //MessageBox(NULL,(LPSTR)error_msg, "Debug Information", MB_OK+MB_ICONINFORMATION);


      local.Execute();
      central.Execute();
   }
 
   source = (fLocal)?&local:&central;


   while (TRUE)
   {
      if (fOK=source->Fetch())
      {
         ret=getGermplasm((_gid<0),_gid,germ);
         if (ret==GMS_NO_DATA || _gid!=germ.gid) //possibility: Replaced Germplasm
            continue;
         else
            break;
      }
      else { 
		  ret=GMS_NO_DATA;
		  break;
	  }
   }

   if (fOK)
      ret = GMS_SUCCESS;
   else if (source->NoData())
      ret = GMS_NO_DATA;
   else 
      ret = GMS_ERROR;


   POP();


#undef SQL_FGERM_SELECT_FMT 
#undef SQL_FGERM_WHERE_FMT 
}



/* 2001-10-15
LONG findGermplasm(BOOL fLocal,GMS_Germplasm &germ,LONG fSearchOption)
{
#define SQL_FGERM_SELECT_FMT "select GID from GERMPLSM where"
#define SQL_FGERM_WHERE_FMT " and %s=?"

   PUSH(findGermplasm);


static CODBCbindedStmt &central = _centralDBC->BindedStmt();
static CODBCbindedStmt &local   = _localDBC->BindedStmt();
       CODBCbindedStmt *source;

static string strSQL="";
static BOOL changed=TRUE;
static BOOL first_time=TRUE,fOK;
static BOOL fgpid1,fgpid2,fglocn,fguid,fgdate,fgnpgs,fgref,fgrplce,fmethn,flgid;
static LONG _gid,gpid1,gpid2,glocn,guid,gdate,gnpgs,gref,methn,lgid;

   if (fSearchOption==FIND_FIRST)
   {
      changed = Changed(fgpid1,(gpid1=germ.gpid1)!=0) |
                Changed(fgpid2,(gpid1=germ.gpid1)!=0) |
                Changed(fglocn,(glocn=germ.glocn)!=0) |
                Changed(fguid ,(guid =germ.guid )!=0) |
                Changed(fgdate,(gdate=germ.gdate)!=0) |
                Changed(fgnpgs,(gnpgs=germ.gnpgs)!=0) |
                Changed(fgref ,(gref =germ.gref )!=0) |
                Changed(fmethn,(methn=germ.methn)!=0) |
                Changed(flgid ,(lgid =germ.lgid )!=0);      
      if (first_time){
         changed=TRUE;
         first_time=FALSE;
      }

      if (changed)
      {
         strSQL="select GID from GERMPLSM where";
         if (fgpid1)
            strSQL = strSQL+" AND gpid1=?";
         if (fgpid2= (gpid2 =germ.gpid2)!=0)
            strSQL = strSQL+" AND gpid2=?";
         if (fglocn= (glocn =germ.glocn)!=0)
            strSQL = strSQL+" AND glocn=?";
         if (fguid = (guid  =germ.guid)!=0)
            strSQL = strSQL+" AND guid =?";
         if (fgdate= (gdate =germ.gdate)!=0)
            strSQL = strSQL+" AND gdate=?";
         if (fgnpgs= (gnpgs =germ.gnpgs)!=0)
            strSQL = strSQL+" AND gnpgs=?";
         if (fgref = (gref  =germ.gref)!=0)
            strSQL = strSQL+" AND gref =?";
         if (fmethn= (methn =germ.methn)!=0)
            strSQL = strSQL+" AND methn=?";
         if (flgid = (lgid  =germ.lgid)!=0)
            strSQL = strSQL+" AND lgid =?";
         if (!(fgpid1||fgpid2||fglocn||fguid||
               fgdate||fgnpgs||fgref||fgrplce||fmethn||flgid))
         {
            POP2(GMS_ERROR);
         }
         //strSQL.Delete(strSQL.Find(" AND ",0),4);
         strSQL.erase(strSQL.find(" AND ",0),4);

         central.SetSQLstr((LPCSTR)strSQL.c_str());
         local.SetSQLstr((LPCSTR)strSQL.c_str());

         // Bind all Colums and Parameter
         BIND(1,_gid);
         int i=1;
         if (fgpid1)
         {
            BINDPARAM(i,gpid1);i+=1;
         }
         if (fgpid2)
         {
            BINDPARAM(i,gpid2);i+=1;
         }
         if (fglocn)
         {
            BINDPARAM(i,glocn);i+=1;
         }
         if (fguid )
         {
            BINDPARAM(i,guid);i+=1;
         }
         if (fgdate)
         {
            BINDPARAM(i,gdate);i+=1;
         }
         if (fgnpgs)
         {
            BINDPARAM(i,gnpgs);i+=1;
         }
         if (fgref )
         {
            BINDPARAM(i,gref);i+=1;
         }
         if (fmethn)
         {
            BINDPARAM(i,methn);i+=1;
         }
         if (flgid )
         {
            BINDPARAM(i,lgid);i+=1;
         }

         changed=FALSE;
      }

      local.Execute();
      central.Execute();
   }
 
   source = (fLocal)?&local:&central;


   while (TRUE)
   {
      if (fOK=source->Fetch())
      {
         ret=getGermplasm((_gid<0),_gid,germ);
         if (ret==GMS_NO_DATA) //possibility: Replaced Germplasm
            continue;
         else
            break;
      }
      else break;
   }

   if (fOK)
      ret = GMS_SUCCESS;
   else if (source->NoData())
      ret = GMS_NO_DATA;
   else 
      ret = GMS_ERROR;

   POP();


#undef SQL_FGERM_SELECT_FMT 
#undef SQL_FGERM_WHERE_FMT 
}
*/

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_findDescendant(LONG gid,GMS_Germplasm *data,LONG *pno,LONG fSearchOption)
{
   static BOOL fLocal=TRUE;

   PUSH(GMS_findDescendant);

   if (fSearchOption==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal){
      ret=findDescendant(LOCAL,gid,*data,*pno,fSearchOption);
      if (ret!=GMS_SUCCESS){
         fLocal = FALSE; 
         ret=findDescendant(CENTRAL,gid,*data,*pno,FIND_NEXT);
      }
   }
   else
      ret=findDescendant(CENTRAL,gid,*data,*pno,fSearchOption);
   POP();
}

//////////////////////////////////////////////////////////////////////////////
//
// 06-Dec-2000 - Fixed the bug which it returns replaced germplasm. As a consequence
//               it will return twice, since it will be picked-up in CHANGES table
//
//////////////////////////////////////////////////////////////////////////////
LONG findDescendant(BOOL fLocal,LONG gid,GMS_Germplasm &germ,LONG &pno,LONG fSearchOption)
{

#define SQL_FINDDESCENDANT1\
   "select PNO,GERMPLSM.GID from GERMPLSM,PROGNTRS where GERMPLSM.GID=PROGNTRS.GID and PID=? order by PNO desc"
#define SQL_FINDDESCENDANT2\
   "select 2 as SOURCE,GID from GERMPLSM where GPID2=? order by GID desc"
#define SQL_FINDDESCENDANT3\
   "select 1 as SOURCE,GID from GERMPLSM where GPID1=? order by GID desc"
#define SQL_FINDDESCENDANT4\
   "select 2 as SOURCE,CRECORD from CHANGES where CTABLE='GERMPLSM' and CFIELD='GPID2' and CSTATUS=0 and CTO=? order by CRECORD desc"
#define SQL_FINDDESCENDANT5\
   "select 1 as SOURCE,CRECORD from CHANGES where CTABLE='GERMPLSM' and CFIELD='GPID1' and CSTATUS=0 and CTO=? order by CRECORD desc"

try{

    PUSH(findDescendant);

static CODBCbindedStmt &central1= _centralDBC->BindedStmt(SQL_FINDDESCENDANT1);
static CODBCbindedStmt &central2= _centralDBC->BindedStmt(SQL_FINDDESCENDANT2);
static CODBCbindedStmt &central3= _centralDBC->BindedStmt(SQL_FINDDESCENDANT3);
static CODBCbindedStmt &local1  = _localDBC->BindedStmt(SQL_FINDDESCENDANT1);
static CODBCbindedStmt &local2  = _localDBC->BindedStmt(SQL_FINDDESCENDANT2);
static CODBCbindedStmt &local3  = _localDBC->BindedStmt(SQL_FINDDESCENDANT3);
static CODBCbindedStmt &local4  = _localDBC->BindedStmt(SQL_FINDDESCENDANT4);
static CODBCbindedStmt &local5  = _localDBC->BindedStmt(SQL_FINDDESCENDANT5);
static CODBCbindedStmt *central[3]={&central1,&central2,&central3};
static CODBCbindedStmt *local[5]={&local1,&local2,&local3,&local4,&local5};
static int icentral,ilocal;

       CODBCbindedStmt *source;


static LONG _source,_id,_gid,_gpid1,_gpid2;
static BOOL first_time=TRUE,find_changes;

   if (fSearchOption==FIND_FIRST){
      if (first_time)
      {
         central1.Bind(1,_source);
         central2.Bind(1,_source);
         central3.Bind(1,_source);
           local1.Bind(1,_source);
           local2.Bind(1,_source);
           local3.Bind(1,_source);
           local4.Bind(1,_source);
           local5.Bind(1,_source);
         central1.Bind(2,_gid);
         central2.Bind(2,_gid);
         central3.Bind(2,_gid);
           local1.Bind(2,_gid);
           local2.Bind(2,_gid);
           local3.Bind(2,_gid);
           local4.Bind(2,_gid);
           local5.Bind(2,_gid);
         central1.BindParam(1,_id);
         central2.BindParam(1,_id);
         central3.BindParam(1,_id);
           local1.BindParam(1,_id);
           local2.BindParam(1,_id);
           local3.BindParam(1,_id);
           local4.BindParam(1,_id);
           local5.BindParam(1,_id);
 
         first_time=FALSE;
      }

	   _id=gid;
         central1.Execute();
         central2.Execute();
         central3.Execute();
           local1.Execute();
           local2.Execute();
           local3.Execute();
           local4.Execute();
           local5.Execute();
         icentral=ilocal=0;
   }

   source=(fLocal)?local[ilocal]:central[icentral];

   while (TRUE)
   {
      if (source->Fetch()){
         ret=getGermplasm((_gid<0),_gid,germ);
         if (ret==GMS_NO_DATA || _gid!=germ.gid) //Fixed..possibility: Replaced Germplasm
            continue;
         pno=_source;
         if (_source==1 && germ.gpid1!=_id)
            continue;
         else if (_source==2 && germ.gpid2!=_id)
            continue;
      }
      else
      {
         if (source->NoData())
         {
            if (fLocal)
            {
               ilocal++;
               if (ilocal<5)
               {
                  source = local[ilocal];
                  continue;
               }
            }
            else
            {
               icentral++;
               if (icentral<3)
               {
                  source = central[icentral];
                  continue;
               }
            }
            ret=GMS_NO_DATA;
         }
         else 
            ret=GMS_ERROR;

      }
      break;
   }
   POP();
}catch(...)
{
   MessageBox(NULL,"Error inside findescendant","ERROR",MB_OK|MB_ICONERROR|MB_APPLMODAL);
   POP2(GMS_ERROR);
}

#undef SQL_FINDDESCENDANT1
#undef SQL_FINDDESCENDANT2
#undef SQL_FINDDESCENDANT3
#undef SQL_FINDDESCENDANT4
#undef SQL_FINDDESCENDANT5
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_getProgenitorID(LONG gid, LONG pno,LONG *pid)
{
   PUSH(GMS_deleteGermplasm);
    ret = getProgenitorID(gid<0,gid,pno,*pid);
   POP();
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG getProgenitorID(BOOL fLocal,LONG gid, LONG pno,LONG &pid)
{
#define SQL_PROGNTRS "\
  select PID from PROGNTRS\
  where GID=? and PNO=?"

  PUSH(getProgenitorID);
static CODBCbindedStmt &central= _centralDBC->BindedStmt(SQL_PROGNTRS);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_PROGNTRS);
       CODBCbindedStmt *source;

static BOOL first_time=TRUE;

//Parameters
static LONG _pid,_gid,_pno;

   if (first_time)
   {
      BIND(1,_pid);
      BINDPARAM(1,_gid);
      BINDPARAM(2,_pno);

      first_time = FALSE;
   }

   source = (fLocal)?&local:&central;

   if (pno<=0) 
      ret = GMS_ERROR;
   else if (pno<=2) 
   {
      GMS_Germplasm germ;
      if ((ret=getGermplasmRecord((gid<0), gid, germ))==GMS_SUCCESS)
         pid = (pno==1) ? germ.gpid1 : germ.gpid2;
   }
   else
   {
      _gid=gid;
      _pno=pno;
      source->Execute();
      if (source->Fetch())
         pid = _pid;
      else if (source->NoData())
         ret = GMS_NO_DATA;
      else 
         ret = GMS_ERROR;
   }
   POP();


#undef SQL_PROGNTRS

}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_addProgenitorID(LONG gid,LONG pno,LONG pid)
{
   PUSH(GMS_addProgenitorID);

   if (UACCESS<30) 
      ret = GMS_NO_ACCESS;
   else if (gid>=0)
      ret = GMS_ERROR;
   else
      ret= addProgenitorID(LOCAL,gid,pno,pid);

   POP();
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG addProgenitorID(BOOL fLocal, LONG gid, LONG pno, LONG pid)
{
#define SQL_ADDPROGENITOR "\
    insert into PROGNTRS\
   (GID,PNO,PID)\
   values (?, ?, ?)"

   PUSH(addProgenitor);

static CODBCbindedStmt &central= _centralDBC->BindedStmt(SQL_ADDPROGENITOR);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_ADDPROGENITOR);
       CODBCbindedStmt *source;

static LONG _gid,_pno,_pid;
static BOOL first_time=TRUE;

   if (first_time)
   {
      BINDPARAM(1,_gid);
      BINDPARAM(2,_pno);
      BINDPARAM(3,_pid);

      first_time=FALSE;
   }

   source = (fLocal)?&local:&central;

   _gid=gid;
   _pno=pno;
   _pid=pid;

   if (!source->Execute())
      ret = GMS_ERROR;

   return ret;

#undef SQL_ADDPROGENITOR
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_setProgenitorID(LONG gid,LONG pno,LONG pid)
{
   PUSH(GMS_setProgenitorID)
   if (UACCESS<30)
      ret = GMS_NO_ACCESS;
   else if (gid>=0)
      ret = GMS_ERROR;
   else
      ret= setProgenitorID(gid,pno,pid);
   POP();
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG setProgenitorID(LONG gid,LONG pno,LONG pid)
{
#define SQL_SETPROGENITORID "\
   update GERMPLSM set GPID1=? where GID=?"

   PUSH(setProgenitorID);

static CODBCbindedStmt &pid1  = _localDBC->BindedStmt("update GERMPLSM set GPID1=? where GID=?");
static CODBCbindedStmt &pid2  = _localDBC->BindedStmt("update GERMPLSM set GPID2=? where GID=?");
//static CODBCbindedStmt &opid  = _localDBC->BindedStmt("update PROGNTRS set PID=?,PNO=? where GID=? ");
static CODBCbindedStmt &opid  = _localDBC->BindedStmt("update PROGNTRS set PID=? where GID=? and PNO = ?");
	   CODBCbindedStmt *source;

static LONG _gid,_pno,_pid;
static BOOL first_time=TRUE, blnExec;

   if (first_time)
   {
      pid1.BindParam(1,_pid);
      pid1.BindParam(2,_gid);
      pid2.BindParam(1,_pid);
      pid2.BindParam(2,_gid);

      opid.BindParam(1,_pid);
      opid.BindParam(2,_gid);
      opid.BindParam(3,_pno);

      first_time=FALSE;
   }

   _pid = pid;
   _gid = gid;
   _pno = pno;
   if (pno==1)
      source=&pid1;
   else if (pno==2)
      source=&pid2;
   else
      source=&opid;

   if (source->Execute())
     ret =GMS_SUCCESS;
   else if (source->NoData())
      ret=GMS_NO_DATA;
   else
      ret = GMS_ERROR;
   POP();
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_deleteProgenitorID(LONG gid)
{
   PUSH(GMS_deleteProgenitorID)
   if (UACCESS<30) 
      ret = GMS_NO_ACCESS;
   else if (gid>=0)
      ret = GMS_ERROR;
   else
      ret= deleteProgenitorID(LOCAL,gid);
   POP();
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_deleteOneProgenitor(LONG gid, LONG pid)
{
   PUSH(GMS_deleteProgenitorID)
   if (UACCESS<30) 
      ret = GMS_NO_ACCESS;
   else if (gid>=0)
      ret = GMS_ERROR;
   else
      ret= deleteOneProgenitor(LOCAL, gid, pid);
   POP();
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG deleteProgenitorID(BOOL fLocal,LONG gid)
{
#define SQL_DELETEPROGENITORID "\
   delete from PROGNTRS where gid=?"

   PUSH(deleteProgenitorID);

static CODBCbindedStmt &central= _centralDBC->BindedStmt(SQL_DELETEPROGENITORID);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_DELETEPROGENITORID);
       CODBCbindedStmt *source;

static LONG _gid;
static BOOL first_time=TRUE;

   if (first_time)
   {
      BINDPARAM(1,_gid);

      first_time=FALSE;
   }

   source = (fLocal)?&local:&central;
   _gid=gid;

   if (source->Execute())
      ret = GMS_SUCCESS;
   else if (source->NoData())
      ret = GMS_NO_DATA;
   else 
      ret = GMS_ERROR;

   POP();

#undef SQL_DELETEPROGENITORID
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG deleteOneProgenitor(BOOL fLocal,LONG gid,LONG pid)
{
#define SQL_DELETEONEPROG "\
   delete from PROGNTRS where gid=? and pid=?"

   PUSH(deleteProgenitorID);

static CODBCbindedStmt &central= _centralDBC->BindedStmt(SQL_DELETEONEPROG);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_DELETEONEPROG);
       CODBCbindedStmt *source;

static LONG _gid, _pid;
static BOOL first_time=TRUE;

   if (first_time)
   {
      BINDPARAM(1,_gid);
      BINDPARAM(2,_pid);

      first_time=FALSE;
   }

   source = (fLocal)?&local:&central;
   _gid=gid;
   _pid=pid;

   if (source->Execute())
      ret = GMS_SUCCESS;
   else if (source->NoData())
      ret = GMS_NO_DATA;
   else 
      ret = GMS_ERROR;

   POP();

#undef SQL_DELETEONEPROG
}



//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG findCIDSID(BOOL fLocal,LONG* CID, LONG* SID, LONG* GID, LONG fOpt)
{
#define SQL_findCIDSID  "\
	   SELECT GERMPLSM.CID, GERMPLSM.SID, GERMPLSM.GID  \
       FROM GERMPLSM                                   \
       WHERE (GERMPLSM.CID=?) AND (GERMPLSM.GRPLCE=0) "
#define SQL_findCIDSID_GID  "\
	   SELECT GERMPLSM.CID, GERMPLSM.SID, GERMPLSM.GID  \
       FROM GERMPLSM                                   \
       WHERE (GERMPLSM.GID=?) AND (GERMPLSM.GRPLCE=0)  "


   PUSH(findCIDSID);

static CODBCbindedStmt &central= _centralDBC->BindedStmt(SQL_findCIDSID);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_findCIDSID);
       CODBCbindedStmt *source;

static LONG _gid, _cid, _sid, _id;
static BOOL first_time=TRUE;

    _id = *CID;
	if (fOpt == FIND_FIRST) {
		if (first_time)
		{
			BINDPARAM(1,_id);
			BIND(1,_cid);
			BIND(2,_sid);
			BIND(3,_gid);
			first_time=FALSE;
		}
		_cid = *CID;
		_sid = *SID;
		_gid = *GID;

		local.Execute();
		central.Execute();
	}
   source = (fLocal)?&local:&central;

   if (source->Fetch()) {
	  *CID = _cid;
	  *SID = _sid;
	  *GID = _gid;
      ret = GMS_SUCCESS;
   }
   else if (source->NoData())
      ret = GMS_NO_DATA;
   else 
      ret = GMS_ERROR;

   POP();


#undef SQL_findCIDSID
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG getCIDSID(LONG* CID, LONG* SID, LONG* GID)
{
#define SQL_getCIDSID_GID  "\
	   SELECT GERMPLSM.CID, GERMPLSM.SID, GERMPLSM.GID  \
       FROM GERMPLSM                                   \
       WHERE (GERMPLSM.GID=?)AND (GERMPLSM.GRPLCE=0)  "


   PUSH(getCIDSID);

static CODBCbindedStmt &central= _centralDBC->BindedStmt(SQL_getCIDSID_GID);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_getCIDSID_GID);

static LONG _gid, _cid, _sid, _id;
static BOOL first_time=TRUE, with_data;

    _id = *GID;
	if (first_time)
		{
			BINDPARAM(1,_id);
			BIND(1,_cid);
			BIND(2,_sid);
			BIND(3,_gid);
			first_time=FALSE;
		}
	_cid = *CID;
 	_sid = *SID;
	_gid = *GID;

	local.Execute();
	central.Execute();
    if (local.Fetch()) {
		*CID = _cid;
		*SID = _sid;
		*GID = _gid;
		ret = GMS_SUCCESS;
	}
    else if (local.NoData())  {
		if (central.Fetch()) {
  		   *CID = _cid;
		   *SID = _sid;
		   *GID = _gid;
		   ret = GMS_SUCCESS;
		}
		else if (central.NoData()) {
			ret = GMS_NO_DATA;
		}
		else ret = GMS_ERROR;
    }
	else ret = GMS_ERROR;

   POP();


#undef SQL_getCIDSID_GID
}




//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG findSID(LONG CID, LONG SID, LONG* GID)
{
#define SQL_findSID  "\
	   SELECT GERMPLSM.GID  \
       FROM GERMPLSM                                   \
       WHERE (GERMPLSM.CID=?)  AND (GERMPLSM.SID=?) AND (GERMPLSM.GRPLCE=0)  "

   PUSH(findSID);

static CODBCbindedStmt &central= _centralDBC->BindedStmt(SQL_findSID);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_findSID);

static LONG _gid, _cid, _sid;
static BOOL first_time=TRUE, cntrl_no_data;

     cntrl_no_data=FALSE;
	if (first_time)
	{
			BINDPARAM(1,_cid);
			BINDPARAM(2,_sid);

			BIND(1,_gid);
			first_time=FALSE;
	}
   _cid = CID;
   _sid = SID;
   _gid = 0;
   local.Execute();
   central.Execute();
   
   local.Fetch();
   if (local.NoData())  {
	   central.Fetch();
	   if (central.NoData()) cntrl_no_data = TRUE;
   }

   if (_gid != 0) {
	  *GID = _gid;
      ret = GMS_SUCCESS;
   }
   else if (local.NoData() && cntrl_no_data)
      ret = GMS_NO_DATA;
   else 
      ret = GMS_ERROR;

   POP();


#undef SQL_findSID
}



//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_findCIDSID(LONG *CID, LONG *SID, LONG *GID, LONG fOpt)
{
	static BOOL fLocal=TRUE, getCID=FALSE;

   PUSH(GMS_findCIDSID);
   if (hasSID==0) POP2(GMS_INVALID_DBSTRUCTURE);
   if ((*CID ==0 && *SID!=0  && *GID!=0 ) || (*CID!=0 && *GID!=0)) {
	   POP2(GMS_ERROR);
   }
   if (fOpt == FIND_SPECIFIC)
       ret = findSID(*CID,*SID,  GID);
   else {
   	   if (fOpt==FIND_FIRST) {
	       fLocal=TRUE;
           getCID = TRUE;
		   if (*GID != 0) {
			   ret = getCIDSID(CID,SID,GID);
			   getCID = FALSE;
		   }
	   }
       if (getCID) {
   	      if (fLocal) {
	         ret=findCIDSID(LOCAL, CID, SID, GID, fOpt);
  	         if (ret!=GMS_SUCCESS){
			    fLocal = FALSE; 
  	            ret=findCIDSID(CENTRAL, CID, SID, GID, FIND_NEXT);
			 }
		  }
		  else
		      ret=findCIDSID(CENTRAL, CID, SID, GID, fOpt);
	   }
   }
   POP();
}


//////////////////////////////////////////////////////////////////////////////
//Created: Aug. 17, 2004
// Description:  Retrieves the germplasm record for the given UniqueID
//////////////////////////////////////////////////////////////////////////////
LONG findUniqueID(LONG uid,LONG lgid, GMS_Germplasm &germ)
{
#define SQL_GETGRMPLSM2_2 "\
  select GERMPLSM.METHN, GERMPLSM.GNPGS, GERMPLSM.GPID1\
  ,GERMPLSM.GPID2, GERMPLSM.GERMUID, GERMPLSM.LGID, GERMPLSM.GLOCN\
  ,GERMPLSM.GDATE,GERMPLSM.GREF, GERMPLSM.GRPLCE\
  from GERMPLSM \
  where GERMPLSM.GERMUID=? and GERMPLSM.LGID = ?"

    PUSH(getGermplasm2_2);
static CODBCbindedStmt &central= _centralDBC->BindedStmt(SQL_GETGRMPLSM2_2);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_GETGRMPLSM2_2);
static BOOL first_time=TRUE, with_data;

static GMS_Germplasm _germ;


   if (first_time)
   {
      BIND(1,_germ.methn);
      BIND(2,_germ.gnpgs);
      BIND(3,_germ.gpid1);
      BIND(4,_germ.gpid2);
      BIND(5,_germ.guid);
      BIND(6,_germ.lgid);
      BIND(7,_germ.glocn);
      BIND(8,_germ.gdate);
      BIND(9,_germ.gref);
      BIND(10,_germ.grplce);
      BINDPARAM(1,_germ.guid);
      BINDPARAM(2,_germ.lgid);

      first_time = FALSE;
   }


   _germ.guid= uid;
   _germ.lgid = lgid;

   local.Execute();
   central.Execute();

   with_data = FALSE;
    if (local.Fetch()) {
		with_data = TRUE;
		ret = GMS_SUCCESS;
	}
    else if (local.NoData())  {
		if (central.Fetch()) {
  		  with_data = TRUE;
		  ret = GMS_SUCCESS;
		}
		else if (central.NoData()) {
			with_data = FALSE;
			ret = GMS_NO_DATA;
		}
		else ret = GMS_ERROR;
    }
	else ret = GMS_ERROR;

   
   if (with_data){
      if (_germ.grplce==0)// Not Deleted Nor Replaced
      {
         germ = _germ;

         // Check Local Changes to the Central Germplasm Record
         if (germ.gid>0)
         {
            LONG replacement;
            if (getReplacementGermplasmFromChanges(germ.gid,replacement)==GMS_SUCCESS)
            {
               if (replacement==germ.gid)
               {
                  POP2(GMS_NO_DATA);
               }
               ret = getGermplasm2_2(replacement<0,replacement,germ);
            }
            if (applyGermplasmChanges(germ)==GMS_SUCCESS)
                  _germ = germ;
         }
      }
      else if (_germ.grplce==_germ.gid) //Deleted
      {
         germ.gid=0;
         ret = GMS_NO_DATA;
      }
      else //Replaced
         ret = getGermplasm2_2(_germ.grplce<0,_germ.grplce,germ);
   }

   POP();

#undef SQL_GETGRMPLSM2_2
}


//////////////////////////////////////////////////////////////////////////////
// Date Created: Aug. 17, 2004
// Retrieves the germplasm record of the given Unique ID = UID LGID
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_findUniqueID(LONG UID, LONG LGID, GMS_Germplasm *data)
{
   static BOOL fLocal=TRUE;

   PUSH(GMS_findUniqueID);
      ret=findUniqueID(UID, LGID , *data);
   POP();
}



LONG GenerationNo2(LONG gid, CHAR *genNo){
	PUSH(GenerationNo);
	GMS_Germplasm gg,gg2;
	CHAR *_genNo;
	LONG grpGID, srcGID;
	LONG c1,c2,err,nthhaploid, err2;
	BOOLEAN bBulk;
	string mtype, gidstrc, cur_method, grp_method, method_name, szDouble, szHaploid;
	GMS_Method gm;
	char szDesc[75];
	_genNo = "\0";
    strcpy(_genNo,"(0:0)");
	cur_method = "\0";
	grp_method = "\0";
	srcGID = gid;
	bBulk = TRUE;
	nthhaploid = 0;
    c1 = 0;
	c2 = 0;
	err = GMS_getGermplasm(gid,&gg);
	gm.mid= gg.methn;
	err2 = GMS_getMethod(&gm,szDesc,75,0);
	cur_method =  gm.mcode;
	method_name = gm.mname;
	TOUPPER(method_name);
	if (gg.gnpgs < 0) {
		grpGID = gg.gpid1;
		ZeroMemory(&gg2,sizeof(gg2));
		err2 = GMS_getGermplasm(grpGID,&gg2);
		ZeroMemory(&gm,sizeof(gm));
		GMS_getMethod(&gm,szDesc,75,0);
		grp_method = gm.mcode;
        szDouble = strstr( (CHAR *) grp_method.c_str(), (CHAR *) "DOUBLE" );
		szHaploid = strstr( grp_method.c_str(), "HAPLOID");
        if (( szDouble.c_str() != NULL ) && ( szHaploid.c_str() != NULL )) {
            nthhaploid++;
            c1 = 0;
			c2 = 0;
			gg.gnpgs = 0;
        }
	}
    else {
		grpGID = gid;
		grp_method = cur_method;
	}
    while (gg.gnpgs < 0) {
		if (err != GMS_SUCCESS) {
			if (err == GMS_NO_DATA) ret = GMS_NO_DATA;
			break;
		}
        else {
			if ((gg.gpid1==0) && (gg.gpid2==0)) {
				c1 = 0;
				c2 = 0;
				break;
			}
		}
		///need to check the circularity
        srcGID = gg.gpid2;
        

		ZeroMemory(&gm,sizeof(gm));
		gm.mid = gg.methn;
		GMS_getMethod(&gm,szDesc,75,0);

	}

    POP();
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_computeGenerationNo(LONG gid, LONG *c2, LONG *c1, LONG *c3, LONG *c4, LONG *LDM, LONG *CRM)
{
   PUSH(GMS_computeGenerationNo)
      ret= GenerationNo(gid,  c2, c1,c3,c4, LDM, CRM);
   POP();
}


//////////////////////////////////////////////////////////////////////////////
// Output:
//     c2 - number of generations since last Single Plant selection
//     c1 - Total no of generation
//     c3 - number of management generation
//     c4 - number of backcross generation
//////////////////////////////////////////////////////////////////////////////

LONG GenerationNo(LONG gid, LONG *c2, LONG *c1, LONG *c3, LONG *c4, LONG *LDM, LONG *CRM)
{
    GMS_Germplasm germ, germ2;
	LONG taux,aux, _gid;
	GMS_Method gm;
	char szDesc[75];

	PUSH(GenerationNo);
	*c1 = 0;
	*c2 = 0;
	*c3 = 0;
	*c4 = 0;
	*LDM = 0;
	*CRM = 0;
	ZeroMemory(&germ, sizeof(germ));
	taux = GMS_getGermplasm(gid,&germ); 

	if (germ.gnpgs >= 0) {      //method is generative
		*c1 = 1;
		*c2 = 0;
		*LDM = 0;
		*CRM = germ.methn;
		germ2 = germ;
	}
	else {
		if (germ.gpid1 == 0 && germ.gpid2 ==0)  {
			/* for inbreeding crops, c1 = 0 should be interpreted as infinite
			   for outbreeding crops, c1 = 0 should be interpreted as 1 */
			*c1 = 0;           
			*c2 = 0;
            *LDM = germ.methn;
			*CRM = 0;
		}
        else  {
            *LDM = germ.methn;
			germ2 = germ;
			while (germ2.gnpgs < 0) {
				if ((germ2.gpid2 ==0) ) {
					*c2 = 0;
					*c1 = 0;
					*CRM=0;
					if (germ2.gpid1!=0) {
                      _gid = germ2.gpid1;
					   aux = GMS_getGermplasm(_gid,&germ2);
					   *CRM = germ2.methn;
					}
					break;
				}
				else {
					gm.mid = germ2.methn;
					ZeroMemory(szDesc,75);
 		            GMS_getMethod(&gm,szDesc,75,0);
					if (strcmp(gm.mtype, "MAN") == 0)  
						 *c3 = *c3 + 1;
					else *c1 = *c1 + 1;
  				    if ((*c2 == 0) &&  ((germ2.methn == 208) || (germ2.methn == 505) || (germ2.methn == 205) || (germ2.methn == 504)) )    
                         *c2 = *c1;
					if (((germ2.methn == 202) || (germ2.methn == 502)) && (*c2>=0)) *c2 = -*c1;
  			        _gid = germ2.gpid2;
					aux = GMS_getGermplasm(_gid,&germ2);
					*CRM = germ2.methn;

						
				}
			}
		if ((*c1 > 0 ) )  {
				*c1 = *c1 + 1;
				if (*c2 > 0) *c2 = *c1 - *c2  ; 
				if (*c2 <0)  *c2 = -(*c1 + *c2);
			}  
		else if ((*c3>0) && (*CRM!=0))  *c1 =1;	
		}
	}

	//backcross
	if ((*CRM==107) || (*CRM==4)) {
		*c4 = getBackcrossDosage(germ2);
	}
	ret = taux;
    POP();
#undef GEN_TYPE
}


LONG getBackcrossDosage(GMS_Germplasm g3)
{
	GMS_Germplasm g1,g2;
	LONG ret,dosage, recp;

         dosage=0;
         recp=0;
         //traverse the tree until none of the parents are backcross
         while (g3.methn==4 || g3.methn==GMS_BACKCROSS){
            dosage++;
    		if (dosage > 100) {
                return GMS_ERROR;
			}

            ret = GMS_getGermplasm(g3.gpid1,&g1);RETURN_ON_ERROR(ret);
            ret = GMS_getGermplasm(g3.gpid2,&g2);RETURN_ON_ERROR(ret);
            // Parent 2 is the recurrent, because g1 is a derivative
            if (g1.gnpgs>0 && g2.gnpgs < 0){
               g3 = g1;
               if (!recp) recp=g2.gid;
            }
            // Parent 1 is the recurrent
            else if (g2.gnpgs>0 && g1.gnpgs < 0){
               g3= g2;
               if (!recp) recp=g1.gid;
            }
            // No derivative
            else if(g1.gnpgs>0 && g2.gnpgs>0){
               //Check Parents of Parent2
               if (g2.gpid1 == g1.gid || g2.gpid2 == g1.gid){
                  // Parent 1 is the Recurrent
                  g3=g2;
                  if (!recp) recp=g1.gid;
               }
               else if (g1.gpid1 == g2.gid || g1.gpid2 == g2.gid){
                  // Parent 2 is the Recurrent
                  g3=g1;
                  if (!recp) recp=g2.gid;
               }
               else{
                  return GMS_ERROR;
               }
            }
            else{
               return GMS_ERROR;
            }
         }
         if (recp==g1.gid)
            g3 = g2;
         else if (recp==g2.gid)
            g3 = g1;
         else{
            return GMS_ERROR;
         }
         dosage++;
		 return dosage;
}

LONG GenerationNoOrig(LONG gid, LONG *c2, LONG *c1, LONG *LDM, LONG *CRM)
{
    GMS_Germplasm germ, germ2;
	LONG aux, _gid;
	GMS_Method gm;
	char szDesc[75];

	PUSH(GenerationNo);
	*c1 = 0;
	*c2 = 0;
	*LDM = 0;
	*CRM = 0;
	ZeroMemory(&germ, sizeof(germ));
	aux = GMS_getGermplasm(gid,&germ); 
	if (germ.gnpgs >= 0) {      //method is generative
		*c1 = 1;
		*c2 = 0;
		*LDM = 0;
		*CRM = germ.methn;
	}
	else {
		if (germ.gpid1 == 0 && germ.gpid2 ==0)  {
			/* for inbreeding crops, c1 = 0 should be interpreted as infinite
			   for outbreeding crops, c1 = 0 should be interpreted as 1 */
			*c1 = 0;           
			*c2 = 0;
            *LDM = germ.methn;
			*CRM = 0;
		}
        else  {
            *LDM = germ.methn;
			germ2 = germ;
			while (germ2.gnpgs < 0) {
				if ((germ2.gpid2 ==0) || ((germ2.methn == 202) || (germ2.methn == 502))) {
					*c1 = 0;
					*c2 = 0;
                    _gid = germ2.gpid1;
					aux = GMS_getGermplasm(_gid,&germ2);
					*CRM = germ2.methn;
					break;
				}
				else {
                    *c1 = *c1 + 1;
					gm.mid = germ.methn;
					ZeroMemory(szDesc,75);
 		            GMS_getMethod(&gm,szDesc,75,0);
  				    if ((*c2 == 0) &&  ((germ2.methn == 208) || (germ2.methn == 505) || (germ2.methn == 205) || (germ2.methn == 504)) ) {   //&& (strcmp(gm.mtype, "GEN")!=0)
                         *c2 = *c1;
					}
  			        _gid = germ2.gpid2;
					aux = GMS_getGermplasm(_gid,&germ2);
					*CRM = germ2.methn;

						
				}
			}
		if ((*c1 > 0 ) )  {
				*c1 = *c1 + 1;
				if (*c2 > 0) 	*c2 = *c1 - *c2  ; 
			}  
			
		}
	}
	ret = aux;
    POP();
#undef GEN_TYPE
}


//////////////////////////////////////////////////////////////////////////////
// getMgmntNeighbor
// Description  : Retrieves the management neighbors of the given management group germplasm; 
//                called by the function GMS_getMgmntNeighbor
// Input        : mgid which is the management root germplasm
// Output       : germplasm data record of the management neighbor
// Date Created : August 23, 2006
//////////////////////////////////////////////////////////////////////////////
LONG getMgmntNeighbor(BOOL fLocal,LONG gid, GMS_Germplasm &grmplsm, LONG fopt)
{
#define SQL_GETMGMTREC "\
  select GERMPLSM.METHN, GERMPLSM.GNPGS, GERMPLSM.GPID1\
  ,GERMPLSM.GPID2, GERMPLSM.GERMUID, GERMPLSM.LGID, GERMPLSM.GLOCN\
  ,GERMPLSM.GDATE,GERMPLSM.GREF, GERMPLSM.GRPLCE, GERMPLSM.MGID, GERMPLSM.GID \
  from GERMPLSM \
where MGID=? AND GERMPLSM.GRPLCE <> GERMPLSM.GID"

  LONG ret=GMS_SUCCESS;//PUSH(getGermplasmRecord);

static CODBCbindedStmt &central= _centralDBC->BindedStmt(SQL_GETMGMTREC);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_GETMGMTREC);
       CODBCbindedStmt *source;

static GMS_Germplasm _data;
static BOOL first_time=TRUE;

//Parameters
static LONG _mgid;

   if ( fopt==FIND_FIRST){
	   if (first_time)
	   {
	      BIND(1,_data.methn);
	      BIND(2,_data.gnpgs);
	      BIND(3,_data.gpid1);
	      BIND(4,_data.gpid2);
	      BIND(5,_data.guid);
	      BIND(6,_data.lgid);
	      BIND(7,_data.glocn);
	      BIND(8,_data.gdate);
	      BIND(9,_data.gref);
	      BIND(10,_data.grplce);
	      BIND(11,_data.ggid);
		  BIND(12,_data.gid);
	      BINDPARAM(1,_mgid);
	      first_time = FALSE;
	   }
	   _mgid=gid;
       local.Execute();
	   central.Execute();
   }
	
   source = (fLocal)?&local:&central;

   if (source->Fetch())
   {
      ret=GMS_SUCCESS;
      grmplsm = _data;
   }
   else if (source->NoData())
      ret = GMS_NO_DATA;
   else 
      ret = GMS_ERROR;

   return ret;//POP();

#undef SQL_GETMGMTMREC
}




//////////////////////////////////////////////////////////////////////////////
// GMS_getMgmntNeighbor
// Description  : Retrieves the management neighbors of the given management group germplasm
// Input        : mgid which is the management root germplasm
// Output       : germplasm data record, name record and the preferred name of the management neighbor
// Date Created : August 23, 2006
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_getMgmntNeighbor(LONG mgid, GMS_Germplasm *data, GMS_Name *name
                   , LPSTR szName, LONG nszName, LONG fopt)
{
   LONG ret2,nid;
   BOOLEAN blCentralName;
   static BOOL fLocal=TRUE;

   PUSH(GMS_getMgmntNeighbor);

   // Get the record of the management neighbor
   if (fopt==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal){
      ret=getMgmntNeighbor(LOCAL,mgid,*data,fopt);
      if (ret!=GMS_SUCCESS){
         fLocal = FALSE; 
	     ret=getMgmntNeighbor(CENTRAL,mgid,*data,FIND_NEXT);
      }
   }
   else
	     ret=getMgmntNeighbor(CENTRAL,mgid,*data,fopt);


   if (ret==GMS_SUCCESS)
   {
     name->gid=data->gid;
	 blCentralName = FALSE;
     ret2=getPrefName(LOCAL,*name,szName,nszName,FIND_FIRST);
     if (ret2!=GMS_SUCCESS && data->gid>0) {
        ret2=getPrefName(CENTRAL,*name,szName,nszName,FIND_FIRST);
		blCentralName = TRUE;
     }
	 //determine if a name is deleted
     if (ret2 == GMS_SUCCESS)  
	 {
      nid = name->nid;
	  getNameFieldFromChanges(nid,"NSTAT",name->nstat);
      if ((name->nstat != 1)) {
		  name->ntype =0;
		  name->nstat = 0;
          ret2 = GMS_getName(name,szName,nszName,FIND_FIRST);
		  while (ret2) {
			  if (name->nstat == 1) {
                  break;                 
			  }
             ret2 = GMS_getName(name,szName,nszName,FIND_NEXT);
		  }
		  if (ret2 == GMS_NO_DATA)  {
              ZeroMemory(name,sizeof(name));
		      ZeroMemory(szName, sizeof(szName));
		  }
	  }
	  else {
	    getNameFieldFromChanges(nid,"NTYPE",name->ntype);
	    getNameFieldFromChanges(nid,"NUID",name->nuid);
	    //getNameFieldFromChanges(nid,"NVAL",data->nval);
	    getNameFieldFromChanges(nid,"NLOCN",name->nlocn);
	    getNameFieldFromChanges(nid,"NDATE",name->ndate);
	    getNameFieldFromChanges(nid,"NREF",name->nref);

	  }

	 }


   }

   POP();
}


//////////////////////////////////////////////////////////////////////////////
//Description: Finds the immediate lines derived from the specified GID
//////////////////////////////////////////////////////////////////////////////
LONG findDerivative(LONG GID, LONG* DerivedGID, LONG fOpt)
{
#define SQL_findDerivative  "\
	   SELECT GERMPLSM.GID  \
       FROM GERMPLSM                                   \
       WHERE (GERMPLSM.GPID2=?)  AND (GERMPLSM.GNPGS<0) AND (GERMPLSM.GRPLCE=0)"

	//	SELECT GERMPLSM.GID \
	//	FROM GERMPLSM, METHODS \
	//	WHERE (((GERMPLSM.GPID2)=?) AND ((GERMPLSM.GNPGS)<0) AND ((GERMPLSM.GRPLCE)=0) AND ((METHODS.MTYPE)='DER') AND ((GERMPLSM.METHN)=METHODS.MID))"


   PUSH(findDerivative);

static CODBCbindedStmt &central= _centralDBC->BindedStmt(SQL_findDerivative);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_findDerivative);

static LONG _gid, _derivedGid;
static BOOL first_time=TRUE, cntrl_no_data=FALSE, cntrl_fetch=FALSE;

	_gid = GID;
	if (fOpt == FIND_FIRST)
	{
		_gid = GID;
		BINDPARAM(1,_gid);
		BIND(1,_derivedGid);

		
        local.Execute();
        central.Execute();
   }
 
//   source = (fLocal)?&local:&central;
   local.Fetch();
   if (local.NoData())  {
	   central.Fetch();
	   if (central.NoData()) cntrl_no_data = TRUE;
   }

   if (_derivedGid!= 0) {
	  *DerivedGID = _derivedGid;
	  _derivedGid = 0;
      ret = GMS_SUCCESS;
   }
   else if (local.NoData() && cntrl_no_data)
      ret = GMS_NO_DATA;
   else 
      ret = GMS_ERROR;

   POP();

#undef SQL_findDerivative
}

//////////////////////////////////////////////////////////////////////////////
// GMS_findDerivative
// Description  : Gets the branches of the derivatives of the given line
// Input        : 
// Output       : array of GIDs of the derivative neighbors
// Date Created : April 3, 2007
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_findDerivative(LONG gid, LONG lngNStep, LONG *idList,LONG *idLstSz, LONG fOpt)
{
   LONG ret1, ret2, gid2, _dgid, _nstep, derivativeList[500], cntList=0,j;
   static long i=0, _idLstSz;
   PUSH(GMS_findDerivative);
   
   if (fOpt == FIND_FIRST) {
	   i=0;       // if first time, set the index of the array to 0
	   _idLstSz = *idLstSz;
   }
   if (i<_idLstSz) idList[i++] = gid;  //add the current gid to the list of derivatives
   if ((lngNStep > 0) && (i<_idLstSz)) {                // if n step forward > 0, then get the succeeding derived lines
		ret1 = findDerivative(gid,&_dgid,FIND_FIRST);
		while (ret1 == GMS_SUCCESS)
		{ 
           derivativeList[cntList] = _dgid;
 	       ret1 = findDerivative(gid,&_dgid,FIND_NEXT);
           cntList++;
		}
		for (j=0;j<cntList;j++)
		{
	      gid2 = derivativeList[j];
	      _nstep = lngNStep - 1;
		  ret2 = GMS_findDerivative(gid2, _nstep, idList, idLstSz, FIND_NEXT); // get the succeeding derivatives of the immediate derived line
		}
   }
   else {
	   if (*idLstSz > 0) 
		   ret = GMS_SUCCESS;
	   else
	      ret = GMS_NO_DATA;
   }
  *idLstSz = i;
   POP();
}

//////////////////////////////////////////////////////////////////////////////
// GMS_findDerivative2
// Description  : Gets the branches and level of the derivatives of a given line 
// Input        : 
// Output       : array of GIDs of the derivative neighbors
// Date Created : Jan 4, 2008
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_findDerivative2(LONG gid, LONG lngNStep, LONG *idList,LONG *levelList, LONG *idLstSz, LONG fOpt)
{
   LONG ret1, ret2, gid2, _dgid, _nstep, derivativeList[500], cntList=0,j;
   static long idx, _idLstSz, _lngNStep=0;
   PUSH(GMS_findDerivative);
   
   if (fOpt == FIND_FIRST) {
	   idx=0;       // if first time, set the index of the array to 0
	   _idLstSz = *idLstSz;
	   _lngNStep = lngNStep;
   }
   if (idx<_idLstSz) {
	   idx++;
	   idList[idx] = gid;  //add the current gid to the list of derivatives
	   levelList[idx] =  _lngNStep - lngNStep + 1;
   }
   if ((lngNStep > 0) && (idx<_idLstSz)) {                // if n step forward > 0, then get the succeeding derived lines
		ret1 = findDerivative(gid,&_dgid,FIND_FIRST);
		while (ret1 == GMS_SUCCESS)
		{ 
           derivativeList[cntList] = _dgid;
 	       ret1 = findDerivative(gid,&_dgid,FIND_NEXT);
           cntList++;
		}
		for (j=0;j<cntList;j++)
		{
	      gid2 = derivativeList[j];
	      _nstep = lngNStep - 1;
		  ret2 = GMS_findDerivative2(gid2, _nstep, idList, levelList, idLstSz, FIND_NEXT); // get the succeeding derivatives of the immediate derived line
		}
   }
   else {
	   if (*idLstSz > 0) 
		   ret = GMS_SUCCESS;
	   else
	      ret = GMS_NO_DATA;
   }
  *idLstSz = idx;
   POP();
}

//////////////////////////////////////////////////////////////////////////////
// GMS_getDerivativeNeighbor
// Description  : Retrieves the derivative neighbors  m-step backward and n-step forward of the given germplasm
// Input        : GID, m-step backward, n-step forward, blnDH - true if Double Haploid method is to be excluded
// Output       : array of GIDs of the derivative neightbors
// Date Created : April 3, 2007
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_getDerivativeNeighbor(LONG gid, LONG lngMStep,LONG lngNStep,LONG *idList,LONG *idLstSz, LONG blnDH)
{
   LONG ret2, lngTgid, lngDgid, idx, lngTotalStep;
   BOOLEAN blCentralName;
   GMS_Germplasm strucGerm;
   static BOOL fLocal=TRUE;

   PUSH(GMS_getDerivativeNeighbor);

   lngTgid = gid;
   lngDgid = gid;
   // get the root GID m-step backward
   for (int i=1; i<=lngMStep; i++)
   {
		//ret2 = getGermplasmRecord(lngTgid<0,lngTgid, strucGerm);
	   ret2 = getGermplasm2_2(lngTgid<0,lngTgid, strucGerm);
		if ((ret2 == GMS_SUCCESS) && (strucGerm.gnpgs <0)) {
   	        lngDgid = lngTgid;
			lngTgid = strucGerm.gpid2;
		}
   }
   
   lngTotalStep = lngNStep + lngMStep;
   ret = GMS_findDerivative(lngDgid, lngTotalStep, idList, idLstSz, FIND_FIRST) ;  

   POP();
}


//////////////////////////////////////////////////////////////////////////////
// GMS_getDerivativeNeighbor
// Description  : Retrieves the derivative neighbors  m-step backward and n-step forward of the given germplasm
// Input        : GID, m-step backward, n-step forward, blnDH - true if Double Haploid method is to be excluded
// Output       : array of GIDs of the derivative neightbors and their corresponding level in the tree
// Date Created : April 3, 2007
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_getDerivativeNeighbor2(LONG gid, LONG lngMStep,LONG lngNStep,LONG *idList, LONG *levelList, LONG *idLstSz, LONG blnDH)
{
	//_mStep - actual step backward, might be less than lngMStep if cross is just few step genereation from the line
   LONG ret2, lngTgid, lngDgid, _mStep, lngTotalStep;
   BOOLEAN blCentralName;
   GMS_Germplasm strucGerm;
   static BOOL fLocal=TRUE;

   PUSH(GMS_getDerivativeNeighbor);

   lngTgid = gid;
   lngDgid = gid;
   // get the root GID m-step backward
   _mStep = 0;
   for (int i=1; i<=lngMStep; i++)
   {
		//ret2 = getGermplasmRecord(lngTgid<0,lngTgid, strucGerm);
	   ret2 = getGermplasm2_2(lngTgid<0,lngTgid, strucGerm);
		if ((ret2 == GMS_SUCCESS) && (strucGerm.gnpgs <0)) {
   	        lngDgid = lngTgid;
			lngTgid = strucGerm.gpid2;
			_mStep = i-1;
		}
   }
   
   lngTotalStep = lngNStep + _mStep;
   ret = GMS_findDerivative2(lngDgid, lngTotalStep, idList, levelList, idLstSz, FIND_FIRST) ;  
   for (int i=1; i<=*idLstSz; i++)
   {    if ((levelList[i]<=_mStep) && (levelList[i]>=1))
            levelList[i]= levelList[i]-_mStep-1; 
        else if ((levelList[i]<=lngTotalStep+1) && (levelList[i]>(_mStep+1)))
           levelList[i]= levelList[i]-(_mStep + 1);
        else if ((levelList[i]==_mStep+1))
           levelList[i]=0;
   }
   POP();
}
