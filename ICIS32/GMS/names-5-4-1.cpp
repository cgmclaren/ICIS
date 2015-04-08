/* Names.cpp : Implements the functions to access and manipulate the NAMES table of ICIS
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
 Modified  (AMP) :   11/18/2003   {setName is added}
 **************************************************************/



#include "..\\\tools\\wildcards.h"
#include "names.h"
#include "misc.h"
#include "changes.h"

extern LONG UACCESS,USERID;
extern CODBCdbc *_centralDBC,*_localDBC;  //Central and Local Database Connection
extern stack<char*> _routines_stack;

bool check=false;

#if defined(WEB)
extern bool WEBSERVICE;  //True if calls are to Web Service
extern _bstr_t szURL;    // URL pointing to the WSDL of the Web Service
int RcdNo; // Holds the record number to be fetched
NetBean *nb;  // Global container for the returned bean structure from the Web Service
int idx=0;
typedef struct TGMS_GermNameBean{

	LONG          gid;
    LONG          methn;
    LONG          gnpgs;
    LONG          gpid1;
    LONG          gpid2;
    LONG          guid;
    LONG          lgid;
    LONG          glocn;
    LONG          gdate;
    LONG          gref;
    LONG          grplce;
	LONG          ggid;              //GID of the germplasm where it is related by management purpose which is the MGID field

	LONG          nid;
    LONG          ntype;
    LONG          nstat;
    LONG          nuid;
    LONG          nlocn;
    LONG          ndate;
    LONG          nref;
    char          nval[255];

}           GMS_NameGerm;

GMS_NameGerm *beanNameGerm;



BOOL __stdcall BufferStruct(NetBean *InStruct,int sze)
{
//ge	nb = new NetBean[sze];
//ge	nb = NULL;
//ge	nb = InStruct;
	idx = sze;
	free(beanNameGerm);
    beanNameGerm = (GMS_NameGerm *)calloc( sze, sizeof(GMS_NameGerm));
	for(int i=0;i<sze;i++)
	{
		beanNameGerm[i].nid = InStruct[i].NID;
		// name.gid = InStruct[i].NGID; Already assigned 
		beanNameGerm[i].ntype = InStruct[i].NTYPE;
		beanNameGerm[i].nstat = InStruct[i].NSTAT;
		beanNameGerm[i].nuid = InStruct[i].NUID;					
		strncpy( beanNameGerm[i].nval, InStruct[i].NVAL,strlen(InStruct[i].NVAL));
		//MessageBox(NULL,InStruct[i].NVAL,"Test",MB_OK);
		beanNameGerm[i].nlocn = InStruct[i].NLOCN;
		beanNameGerm[i].ndate = InStruct[i].NDATE;
		beanNameGerm[i].nref = InStruct[i].NREF;

		beanNameGerm[i].gdate = InStruct[i].GDATE;
		beanNameGerm[i].gid = InStruct[i].GGID; 
		beanNameGerm[i].glocn = InStruct[i].GLOCN;
		beanNameGerm[i].gnpgs = InStruct[i].GNPGS;
		beanNameGerm[i].gpid1 = InStruct[i].GPID1;
		beanNameGerm[i].gpid2 = InStruct[i].GPID2;
		beanNameGerm[i].gref = InStruct[i].GREF;
		beanNameGerm[i].grplce = InStruct[i].GRPLCE;
		beanNameGerm[i].guid = InStruct[i].GERMUID; 
		beanNameGerm[i].lgid = InStruct[i].LGID;
		beanNameGerm[i].methn = InStruct[i].METHN; 
	} 

	return (true);
}
#endif //defined(WEB)

LONG getNextNID(BOOL fLocal)
{
   static LONG localNID=0,centralNID=0;
   if (fLocal && !localNID)
   {
      CODBCdirectStmt source1 = _localDBC->DirectStmt("select MIN(UNID) from INSTLN") ;
      source1.Execute();
      source1.Fetch();
      CODBCdirectStmt source2 = _localDBC->DirectStmt("select MIN(NID) from NAMES") ;
      source2.Execute();
      source2.Fetch();
      localNID=min(source1.Field(1).AsInteger(),source2.Field(1).AsInteger());
   }
   else if (!fLocal && !centralNID)
   {
      CODBCdirectStmt source = _centralDBC->DirectStmt("select MAX(NID) from NAMES") ;
      source.Execute();
      source.Fetch();
      centralNID=source.Field(1).AsInteger();
   }
   return (fLocal)? --localNID:++centralNID;
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_addName(GMS_Name *name,CHAR *szName,LONG stat)
{
   PUSH(GMS_addName);
   if (UACCESS<30)
      ret = GMS_NO_ACCESS;
   else
      ret = addName(TRUE,*name,szName,stat);
   POP();
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG addName(BOOL fLocal, GMS_Name &name,CHAR *szName,LONG stat)
{
#define SQL_ADDNAME "\
   insert into NAMES\
   (NID, GID,NTYPE,NSTAT,\
  NUID,NLOCN,NDATE,NREF,NVAL)\
  VALUES (?,?,?,?,?,?,?,?,?)"

  PUSH(addName);

//static CODBCbindedStmt &central= _centralDBC->BindedStmt(SQL_ADDNAME);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_ADDNAME);
       CODBCbindedStmt *source;

static GMS_Name _data;
static CHAR nval[GMS_MAX_NVAL],*pnext;
static BOOL first_time=TRUE;
static long nid=0;

   if (first_time)
   {
      local.BindParam(1,nid);
      local.BindParam(2,_data.gid);
      local.BindParam(3,_data.ntype);
      local.BindParam(4,_data.nstat);
      local.BindParam(5,_data.nuid);
      local.BindParam(6,_data.nlocn);
      local.BindParam(7,_data.ndate);
      local.BindParam(8,_data.nref);
	  local.BindParam(9, nval,ICIS_MAX_SIZE );
      first_time=FALSE;
   }

   source = &local;
   nid = getNextNID(fLocal);
   name.nuid=USERID;
   _data = name;
   _data.nid = nid;
   // Copy to nval
   strncpy(nval,szName,GMS_NVAL_SIZE);
   nval[GMS_NVAL_SIZE]='\0';

   // Copying to NEXT is deleted

   if (source->Execute()) {
      name.nid = nid;
      ret = GMS_SUCCESS;
	  }
   else 
      ret = GMS_ERROR;

   POP();

#undef SQL_ADDNAME
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_getName(GMS_Name *data, LPSTR szName, LONG nszName
               , LONG fSearchOption)
{
   static BOOL fLocal=TRUE;
//   static TGETNAMEPTR getnameptr=&getNameEx;

   LONG ret=GMS_SUCCESS, nid=0,to=0, gid=0, nstat=0;//PUSH(GMS_getName);

   if (fSearchOption==FIND_FIRST)
   {
      fLocal=TRUE;
      //if (data->nstat) getnameptr = &getPrefName;
      //else getnameptr = &getNameEx;
   }
   if (fLocal){
      ret=getNameEx(LOCAL,*data,szName,nszName,fSearchOption);
      if (ret!=GMS_SUCCESS){
         fLocal = FALSE; 
         ret=getNameEx(CENTRAL,*data,szName,nszName,FIND_NEXT);
      }
   }
   else
      ret=getNameEx(CENTRAL,*data,szName,nszName,fSearchOption);
   if (ret == GMS_SUCCESS)
   {
      nid = data->nid;
	  nstat= data->nstat;
	  getNameFieldFromChanges(nid,"NSTAT",nstat);
      if (nstat == 9) {
		  //ret = GMS_NO_DATA;
		  ZeroMemory(szName, sizeof(szName));
          ret=GMS_getName(data,szName,nszName,FIND_NEXT);		  
	  }
	  else {
	  getNameFieldFromChanges(nid,"NTYPE",data->ntype);
	  getNameFieldFromChanges(nid,"NUID",data->nuid);
	  //getNameFieldFromChanges(nid,"NVAL",data->nval);
	  getNameFieldFromChanges(nid,"NLOCN",data->nlocn);
	  getNameFieldFromChanges(nid,"NDATE",data->ndate);
	  getNameFieldFromChanges(nid,"NREF",data->nref);
	  getNameFieldFromChanges(nid,"NSTAT",data->nstat);

	  }


   }
   return ret;//POP();

}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_getName2(GMS_Name *data, LPSTR szName, LONG nszName
               , LONG fSearchOption)
{
   static BOOL fLocal=TRUE;
//   static TGETNAMEPTR getnameptr=&getNameEx;

   LONG ret=GMS_SUCCESS, nid=0,to=0, gid=0, nstat=0;//PUSH(GMS_getName);

   if (fSearchOption==FIND_FIRST)
   {
      fLocal=TRUE;
   }
   if (fLocal){
      ret=getNameEx2(LOCAL,*data,szName,nszName,fSearchOption);
      if (ret!=GMS_SUCCESS){
         fLocal = FALSE; 
         ret=getNameEx2(CENTRAL,*data,szName,nszName,FIND_NEXT);
      }
   }
   else
      ret=getNameEx2(CENTRAL,*data,szName,nszName,fSearchOption);
   if (ret == GMS_SUCCESS)
   {
      nid = data->nid;
	  nstat= data->nstat;
	  getNameFieldFromChanges(nid,"NSTAT",nstat);
      if (nstat == 9) {
		  //ret = GMS_NO_DATA;
		  ZeroMemory(szName, sizeof(szName));
          ret=GMS_getName(data,szName,nszName,FIND_NEXT);		  
	  }
	  else {
	  getNameFieldFromChanges(nid,"NTYPE",data->ntype);
	  getNameFieldFromChanges(nid,"NUID",data->nuid);
	  //getNameFieldFromChanges(nid,"NVAL",data->nval);
	  getNameFieldFromChanges(nid,"NLOCN",data->nlocn);
	  getNameFieldFromChanges(nid,"NDATE",data->ndate);
	  getNameFieldFromChanges(nid,"NREF",data->nref);
	  getNameFieldFromChanges(nid,"NSTAT",data->nstat);

	  }


   }
   return ret;//POP();

}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG getPrefName(BOOL fLocal,GMS_Name &name, LPSTR szName, LONG nszName
               , LONG fSearchOption)
{
#define SQL_GETPREFNAME "\
   select GID, NTYPE, NSTAT, NUID, NVAL, NLOCN, NDATE, NREF, NID \
   from NAMES \
   where GID=? and NSTAT=1 "

   PUSH(getName);

static CODBCbindedStmt &central= _centralDBC->BindedStmt(SQL_GETPREFNAME);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_GETPREFNAME);
       CODBCbindedStmt *source;

static GMS_Name _data;
static CHAR _nval[GMS_NVAL_SIZE+1];
static BOOL first_time=TRUE;

//Parameters
static LONG gid,nstat,ntype;
   _nval[0]='\0'; 
   if (fSearchOption==FIND_FIRST)
   {
      if (first_time)
      {
         BIND(1,_data.gid);
         BIND(2,_data.ntype);
         BIND(3,_data.nstat);
         BIND(4,_data.nuid);
         BINDS(5,_nval,ICIS_MAX_SIZE );
         BIND(6,_data.nlocn);
         BIND(7,_data.ndate);
         BIND(8,_data.nref);
		 BIND(9,_data.nid);
         BINDPARAM(1,gid);

         first_time = FALSE;
      }
   
      gid=name.gid;
      local.Execute();
      central.Execute();
   }

   source = (fLocal)?&local:&central;

    if (source->Fetch())
   {
     name = _data;
     string namestr;
     namestr=_nval;
     strncpy(szName,namestr.c_str(),nszName);
     szName[nszName-1]='\0';
   }
   else if (source->NoData()) {
     string namestr;
     namestr=_nval;
     strncpy(szName,namestr.c_str(),nszName);
     szName[nszName-1]='\0';
     ret=GMS_NO_DATA;
   }
   else 
      ret=GMS_ERROR;

   POP();

#undef SQL_GETPREFNAME
}

LONG getNameEx(BOOL fLocal,GMS_Name &name, LPSTR szName, LONG nszName
               , LONG fSearchOption)
{
#define SQL_GETNAMEEX "\
   select GID, NTYPE, NSTAT, NUID, NVAL, NLOCN, NDATE, NREF, NID \
   from NAMES \
   where GID=? and((?<>0 and NSTAT=?) or (?=0 and (?=0 or ?=NTYPE))) and NSTAT <> 9 \
    order by NTYPE ASC, NSTAT ASC"     //AMP: DESC"

   PUSH(getName);

static CODBCbindedStmt &central= _centralDBC->BindedStmt(SQL_GETNAMEEX);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_GETNAMEEX);
       CODBCbindedStmt *source;

static GMS_Name _data;
static CHAR _nval[GMS_NVAL_SIZE+1],_next[MAX_STR];
static BOOL first_time=TRUE;

//Parameters
static LONG gid,nstat,ntype;

   if (fSearchOption==FIND_FIRST)
   {
      if (first_time)
      {
 	      BIND(1,_data.gid);
         BIND(2,_data.ntype);
         BIND(3,_data.nstat);
         BIND(4,_data.nuid);
         BINDS(5,_nval,ICIS_MAX_SIZE );
         BIND(6,_data.nlocn);
         BIND(7,_data.ndate);
         BIND(8,_data.nref);
		 BIND(9,_data.nid);
         BINDPARAM(1,gid);
         BINDPARAM(2,nstat);
         BINDPARAM(3,nstat);
         BINDPARAM(4,nstat);
         BINDPARAM(5,ntype);
         BINDPARAM(6,ntype);


         first_time = FALSE;
      }
   
      gid=name.gid;
      nstat=name.nstat;
      ntype=name.ntype;
      local.Execute();
      central.Execute();
   }

   source = (fLocal)?&local:&central;

   _next[0]='\0';
   if (source->Fetch())
   {
      name = _data;
      string namestr;
      namestr=_nval;
      namestr+=_next;
      strncpy(szName,namestr.c_str(),nszName);
      szName[nszName-1]='\0';
      //PadRight(szName,nszName);
   }
   else if (source->NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;

   POP();

#undef SQL_GETNAMEEX
}

//////////////////////////////////////////////////////////////////////
//  getNameEx2 - gets all the names regardless if they are deleted
//
/////////////////////////////////////////////////////////////////////
LONG getNameEx2(BOOL fLocal,GMS_Name &name, LPSTR szName, LONG nszName
               , LONG fSearchOption)
{
#define SQL_GETNAMEEX "\
   select GID, NTYPE, NSTAT, NUID, NVAL, NLOCN, NDATE, NREF, NID \
   from NAMES \
   where GID=? and((?<>0 and NSTAT=?) or (?=0 and (?=0 or ?=NTYPE))) \
   order by NTYPE ASC, NSTAT DESC"

   PUSH(getName);

static CODBCbindedStmt &central= _centralDBC->BindedStmt(SQL_GETNAMEEX);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_GETNAMEEX);
       CODBCbindedStmt *source;

static GMS_Name _data;
static CHAR _nval[GMS_NVAL_SIZE+1],_next[MAX_STR];
static BOOL first_time=TRUE;

//Parameters
static LONG gid,nstat,ntype;

   if (fSearchOption==FIND_FIRST)
   {
      if (first_time)
      {
 	      BIND(1,_data.gid);
         BIND(2,_data.ntype);
         BIND(3,_data.nstat);
         BIND(4,_data.nuid);
         BINDS(5,_nval,ICIS_MAX_SIZE );
         BIND(6,_data.nlocn);
         BIND(7,_data.ndate);
         BIND(8,_data.nref);
		 BIND(9,_data.nid);
         BINDPARAM(1,gid);
         BINDPARAM(2,nstat);
         BINDPARAM(3,nstat);
         BINDPARAM(4,nstat);
         BINDPARAM(5,ntype);
         BINDPARAM(6,ntype);


         first_time = FALSE;
      }
   
      gid=name.gid;
      nstat=name.nstat;
      ntype=name.ntype;
      local.Execute();
      central.Execute();
   }

   source = (fLocal)?&local:&central;

   _next[0]='\0';
   if (source->Fetch())
   {
      name = _data;
      string namestr;
      namestr=_nval;
      namestr+=_next;
      strncpy(szName,namestr.c_str(),nszName);
      szName[nszName-1]='\0';
      //PadRight(szName,nszName);
   }
   else if (source->NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;

   POP();

#undef SQL_GETNAMEEX2
}


//////////////////////////////////////////////////////////////////////////////
//
// Changes:
//   Removed the NUID Filter
//////////////////////////////////////////////////////////////////////////////
LONG  setPreferred(GMS_Name &name,CHAR *szName,LONG nstat)
{
#define SQL_SETPREFF "\
  update NAMES set NSTAT=? where GID=? and NSTAT=?"
#define SQL_SETPREFF2 "\
  update NAMES set NSTAT=? where GID=? and NVAL=?"
  //and NUID=?"

    PUSH(setPreferred);

static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_SETPREFF);
static CODBCbindedStmt &local2  = _localDBC->BindedStmt(SQL_SETPREFF2);

static LONG _nstat,_gid,_nstatFrom;//_nuid,
static CHAR _nval[GMS_NVAL_SIZE+1];

SQLINTEGER nrow;
BOOL fOK,fExists;
//Parameters
static LONG gid,ntype;
static BOOL first_time=TRUE;

   if (first_time)
   {
      local.BindParam(1,_nstat);
      local.BindParam(2,_gid);
      local.BindParam(3,_nstatFrom);

      local2.BindParam(1,_nstat);
      local2.BindParam(2,_gid);
      local2.BindParam(3,_nval,ICIS_MAX_SIZE );
      //local2.BindParam(4,_nuid);
      first_time=FALSE;
   }
   LOGF("Name to change = %s\n",szName);
   //if (strlen(szName)>GMS_ABBR_LEN || name.nstat== (LONG)(2 / nstat))
   // if existing is a preferred name or abbreviation already then don't allow
   if ((name.nstat== (LONG)(2 / nstat)) || (name.nstat == 9))
   {
      POP2(GMS_ERROR);
   }

   // Set from nstat to -nstat to revert back later if there's any problem
   nrow=0;
   _nstat=-nstat;
   _gid=name.gid;
   _nstatFrom=nstat;
   if (local.Execute())
      SQLRowCount(local.Handle(),&nrow);
   fExists=nrow!=0;
   LOGF("fExists = %d -Name exists\n",fExists);

   // Set Preferred Name/Abbreviation
   _nstat=nstat;
   _gid=name.gid;
   //_nuid=USERID;
   strncpy(_nval,szName,GMS_NVAL_SIZE+1);_nval[GMS_NVAL_SIZE]='\0';strupr(_nval);

   fOK=local2.Execute();
   LOGF("fOK = %d - Old Preferred Changed to alternative name\n",fOK);
   if (fExists)
   {
      if (fOK)
      {
         // Make permanent change
         _nstat=0;
         _gid=name.gid;
         _nstatFrom=-nstat;
      }
      else
      {
         // Maybe in the central, check if it exists
         GMS_Name _name;
         GMS_Germplasm _germ;
         CHAR _szName[MAX_STR];
         LONG _ret;
         _ret=findName(CENTRAL,szName,_name,_germ,_szName,MAX_STR,FIND_FIRST);
         while (_ret==GMS_SUCCESS && _name.gid==name.gid && _name.nstat!=nstat)
            _ret=findName(CENTRAL,szName,_name,_germ,_szName,MAX_STR,FIND_NEXT);

         //Copy Central to Local name record
         if (_ret==GMS_SUCCESS)
         {
            LOG("Name is in the Central Database\n");
			_name.nstat = nstat;
            addName(LOCAL,_name,_szName,nstat);
            _nstat=0;
            _gid=name.gid;
            _nstatFrom=-nstat;
         }
         // revert back
         else
         {
            LOG("Reverting back to the old Preferred Name\n");
            _nstat=nstat;
            _gid=name.gid;
            _nstatFrom=-nstat;
         }
      }
      local.Execute();
   }
   
   if (!fOK) 
      ret=GMS_ERROR;
   else 
   {
      SQLRowCount(local2.Handle(),&nrow);
      if (nrow==0)
         ret = GMS_NO_DATA;
      else
         name.nstat=nstat;
   }

   POP();


#undef SQL_SETPREFF
#undef SQL_SETPREFF2
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE  GMS_setPreferredName(GMS_Name *name,CHAR *szName)
{
   PUSH(GMS_setPreferredName);
   LOGF(" GID = %d\n",name->gid);
   if (UACCESS<30) // Read-only central DB and local db
      ret=GMS_NO_ACCESS;
//   else if (name->gid>=0)
//      ret=GMS_ERROR;
   else
      ret=setPreferred(*name,szName,1);
   POP();
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE  GMS_setPreferredAbbr(GMS_Name *name,CHAR *szName)
{
   PUSH(GMS_setPreferredAbbr);
   if (UACCESS<30)
      ret = GMS_NO_ACCESS;
   else if (name->gid>=0)
      ret = GMS_ERROR;
   else
      //ret=setPreferred(TRUE,*name,szName,2);//ret=setPreferredAbbr(TRUE,*name,szName);
      ret=setPreferred(*name,szName,2);
   POP();

}



//////////////////////////////////////////////////////////////////////////////
//Date Modified: Nov 20, 2003
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_findName(CHAR *szSearchName, GMS_Name *name, GMS_Germplasm *germ
               , CHAR *szName, LONG nszName, LONG fSearchOption)
{
   static BOOL fLocal=TRUE;
   LONG nid;

   PUSH(GMS_findName);
   if (fSearchOption==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal){
      ret=findName(LOCAL,szSearchName,*name,*germ,szName,nszName,fSearchOption);
      if (ret!=GMS_SUCCESS){
         fLocal = FALSE; 
         ret=findName(CENTRAL,szSearchName,*name,*germ,szName,nszName,FIND_NEXT);
      }
   }
   else
      ret=findName(CENTRAL,szSearchName,*name,*germ,szName,nszName,fSearchOption);

   //determine if a name is deleted; if so, return no data
   if (ret == GMS_SUCCESS)  
   {
      nid = name->nid;
	  getNameFieldFromChanges(nid,"NSTAT",name->nstat);
      if (name->nstat == 9) {
		  ret = GMS_NO_DATA;
          ZeroMemory(name,sizeof(name));
		  ZeroMemory(szName, sizeof(szName));
		  ZeroMemory(germ, sizeof(germ));
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

   POP();

}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG findName(BOOL fLocal,CHAR *szSearchName, GMS_Name &name, GMS_Germplasm &germ
               , CHAR *szName, LONG nszName, LONG fSearchOption)
{
#define SQL_FINDNAME_CENTRAL "\
 select \
   NAMES.GID, NAMES.NTYPE, NAMES.NSTAT, NAMES.NUID, NAMES.NVAL\
  ,NAMES.NLOCN, NAMES.NDATE, NAMES.NREF, NAMES.NID  \
  ,GERMPLSM.GID,GERMPLSM.METHN, GERMPLSM.GNPGS, GERMPLSM.GPID1\
  ,GERMPLSM.GPID2, GERMPLSM.GERMUID, GERMPLSM.LGID, GERMPLSM.GLOCN\
  ,GERMPLSM.GDATE,GERMPLSM.GREF, GERMPLSM.GRPLCE, GERMPLSM.MGID\
 from GERMPLSM,NAMES\
 where (GERMPLSM.GID=NAMES.GID) and NAMES.NSTAT <> 9 and "

#define SQL_FINDNAME_LOCAL "\
 select NAMES.GID, NAMES.NTYPE, NAMES.NSTAT, NAMES.NUID, NAMES.NVAL\
   ,NAMES.NLOCN, NAMES.NDATE, NAMES.NREF, NAMES.NID  \
 from NAMES  \
 where NAMES.NSTAT <> 9 and "

#define SQL_FINDNAME_ \
  " order by NAMES.NVAL "

#define NVAL_EQ_SQL     " (NAMES.NVAL = ?) "
#define NVAL_LIKE_SQL   " (NAMES.NVAL like ?) "


   PUSH(findName);

static CODBCbindedStmt &central= _centralDBC->BindedStmt();
static CODBCbindedStmt &local  = _localDBC->BindedStmt();
       CODBCbindedStmt *source;

static GMS_Name _name;
static GMS_Germplasm _germ;
static CHAR _nval[GMS_NVAL_SIZE+1],_next[MAX_STR];
static BOOL bind=TRUE;

static BOOL changed=TRUE,long1=FALSE;
static BOOL wild=FALSE;
static BOOL first_time=TRUE,match;
// These values determines a string if it has a wild character
static BOOL nval1_wild,next1_wild;

//Parameters
static LONG nstat,len;
static CHAR central_sql[544],local_sql[320];
static CHAR nval1[MAX_STR],nval2[MAX_STR],nval3[MAX_STR];
// These char pointers points automatically to the 31st character
static CHAR *pnext1=nval1+GMS_NVAL_SIZE+1;
static string strName1;
string strName;


BOOL  fOK;


   if (fSearchOption==FIND_FIRST)
   {
      ZeroFill(nval1,sizeof(nval1));

      nstat=name.nstat;

      // Original Name uppercased
      strcpy(nval1, szSearchName);
      strupr(nval1);


      changed = Changed(long1,strlen(nval1)>GMS_NVAL_SIZE);

      if (first_time)
      {
         changed=TRUE;
         first_time=FALSE;
      }

      next1_wild=0;

      strName1=nval1;

      if (long1)
      {
         memmove(nval1+GMS_NVAL_SIZE+1,nval1+GMS_NVAL_SIZE,strlen(nval1)-GMS_NVAL_SIZE);
         nval1[GMS_NVAL_SIZE]='\0';
         next1_wild=strchr(pnext1,'_') || strchr(pnext1,'%');
      }


      if (Changed(nval1_wild,strchr(nval1,'_') || strchr(nval1,'%')))
         changed=TRUE;

      if (changed)
      {
         string central_sql,local_sql,strtemp;

         central_sql=SQL_FINDNAME_CENTRAL;
         central_sql+="(";
         local_sql=SQL_FINDNAME_LOCAL;
         local_sql+="(";

         strtemp+=(nval1_wild?NVAL_LIKE_SQL:NVAL_EQ_SQL);

         local_sql+=strtemp+")"+SQL_FINDNAME_;
         central_sql+=strtemp+")"+SQL_FINDNAME_;

         central.SetSQLstr(central_sql.c_str());
         local.SetSQLstr(local_sql.c_str());

         BIND(1,_name.gid);
         BIND(2,_name.ntype);
         BIND(3,_name.nstat);
         BIND(4,_name.nuid);
         BINDS(5,_nval,ICIS_MAX_SIZE );
         BIND(6,_name.nlocn);
         BIND(7,_name.ndate);
         BIND(8,_name.nref);
		 BIND(9,_name.nid);
         //BINDS(9,_next,MAX_STR);
         central.Bind(10,_germ.gid);
         central.Bind(11,_germ.methn);
         central.Bind(12,_germ.gnpgs);
         central.Bind(13,_germ.gpid1);
         central.Bind(14,_germ.gpid2);
         central.Bind(15,_germ.guid);
         central.Bind(16,_germ.lgid);
         central.Bind(17,_germ.glocn);
         central.Bind(18,_germ.gdate);
         central.Bind(19,_germ.gref);
         central.Bind(20,_germ.grplce);
         central.Bind(21,_germ.ggid);
         BINDPARAMS(1,nval1,GMS_NVAL_SIZE);

         changed = FALSE;
      }

      local.Execute();
      central.Execute();
   }

   source = (fLocal)?&local:&central;

   while (TRUE)
   {
	  _nval[0]='\0';
	  _next[0]='\0';
      if (fOK=source->Fetch())
      {
         name = _name;

         strName = _nval;
         //strName += _next;

         //Check the next as necessary
         match=wildcardfit ((char*)strName1.c_str(),(char*)strName.c_str());
         if (!match)
            continue;
         if (fLocal)
         {
            ret = getGermplasmRecord(_name.gid<0, _name.gid, _germ);
            if (ret!=GMS_SUCCESS)// && ret!=GMS_NO_DATA
            {
               POP();
            }
            if (_germ.grplce==0 && _name.gid==_germ.gid)
               break;
         }
         else
         {
            if (_germ.grplce!=0)
               continue;
            name.gid=_germ.gid;
         }
         // Check if it has been replaced or deleted then ignore it.
         LONG replacement;
         if (getReplacementGermplasmFromChanges(_name.gid,replacement)!=GMS_SUCCESS)
            break;
      }
      else break;
   }

   germ=_germ;

   ret=GMS_SUCCESS;

   if (fOK){
      strncpy(szName,strName.c_str(),nszName);
      szName[nszName-1]='\0';
   }
   else if (source->NoData())
      ret = GMS_NO_DATA;
   else 
      ret = GMS_ERROR;

   POP();


#undef SQL_FINDNAME_CENTRAL
#undef SQL_FINDNAME_LOCAL
#undef NVAL_SQL
#undef NVAL_LIKE_SQL
#undef NEXT_SQL
#undef NEXT_LIKE_SQL
}

			   /*
{
#define SQL_FINDNAME_CENTRAL "\
 select \
   NAMES.GID, NAMES.NTYPE, NAMES.NSTAT, NAMES.NUID, NAMES.NVAL\
  ,NAMES.NLOCN, NAMES.NDATE, NAMES.NREF, NAMES.NEXT \
  ,GERMPLSM.GID,GERMPLSM.METHN, GERMPLSM.GNPGS, GERMPLSM.GPID1\
  ,GERMPLSM.GPID2, GERMPLSM.GUID, GERMPLSM.LGID, GERMPLSM.GLOCN\
  ,GERMPLSM.GDATE,GERMPLSM.GREF, GERMPLSM.GRPLCE\
 from GERMPLSM,NAMES\
 where (GERMPLSM.GID=NAMES.GID) and (GERMPLSM.GRPLCE=0) and "

#define SQL_FINDNAME_LOCAL "\
 select NAMES.GID, NAMES.NTYPE, NAMES.NSTAT, NAMES.NUID, NAMES.NVAL\
   ,NAMES.NLOCN, NAMES.NDATE, NAMES.NREF, NAMES.NEXT \
 from NAMES  \
 where "

   PUSH(findName);

#define NVAL_LIKE_SQL "(NAMES.NVAL LIKE ?)"
#define NVAL_EQ_SQL "(NAMES.NVAL = ?)"
#define SQL_FINDNAME_ " order by NAMES.NVAL"

static CODBCbindedStmt &central= _centralDBC->BindedStmt();
static CODBCbindedStmt &local  = _localDBC->BindedStmt();
       CODBCbindedStmt *source;

static GMS_Name _name;
static GMS_Germplasm _germ;
static CHAR _nval[GMS_NVAL_SIZE+1],_next[MAX_STR];
static BOOL bind=TRUE;

static BOOL changed=TRUE,_long=FALSE;
static BOOL first_time=TRUE;
static BOOL nval_wild=FALSE;


//Parameters
static LONG nstat,len;
static CHAR nval[MAX_STR];
static CHAR *pnext=nval+GMS_NVAL_SIZE+1;

BOOL  fOK;


   if (fSearchOption==FIND_FIRST)
   {
      ZeroFill(nval,sizeof(nval));

      nstat=name.nstat;

      // Original Name uppercased
      strcpy(nval, szSearchName);
      strupr(nval);

	   //^ - xor operator
      changed= (_long ^ (_long=strlen(nval)>GMS_NVAL_SIZE)); 

      if (_long)
      {
         memmove(nval+GMS_NVAL_SIZE+1,nval+GMS_NVAL_SIZE,strlen(nval)-GMS_NVAL_SIZE);
         nval[GMS_NVAL_SIZE]='\0';
      }
      else
         if (Changed(nval_wild,strchr(nval,'_') || strchr(nval,'%')))
            changed = TRUE;

      if (first_time)
      {
         changed=TRUE;
         first_time=FALSE;
      }

      if (changed)
      {
         string central_sql,local_sql,strtemp;
         central_sql=SQL_FINDNAME_CENTRAL;
         local_sql=SQL_FINDNAME_LOCAL;
         strtemp+=(nval_wild?NVAL_LIKE_SQL:NVAL_EQ_SQL);

         local_sql+=strtemp+SQL_FINDNAME_;
         central_sql+=strtemp+SQL_FINDNAME_;

         central.SetSQLstr(central_sql.c_str());
         local.SetSQLstr(local_sql.c_str());

         BIND(1,_name.gid);
         BIND(2,_name.ntype);
         BIND(3,_name.nstat);
         BIND(4,_name.nuid);
         BINDS(5,_nval,GMS_NVAL_SIZE+1);
         BIND(6,_name.nlocn);
         BIND(7,_name.ndate);
         BIND(8,_name.nref);
         BINDS(9,_next,MAX_STR);
         central.Bind(10,_germ.gid);
         central.Bind(11,_germ.methn);
         central.Bind(12,_germ.gnpgs);
         central.Bind(13,_germ.gpid1);
         central.Bind(14,_germ.gpid2);
         central.Bind(15,_germ.guid);
         central.Bind(16,_germ.lgid);
         central.Bind(17,_germ.glocn);
         central.Bind(18,_germ.gdate);
         central.Bind(19,_germ.gref);
         central.Bind(20,_germ.grplce);

         BINDPARAMS(1,nval,GMS_NVAL_SIZE);

         changed = FALSE;
      }

      local.Execute();
      central.Execute();
   }

   source = (fLocal)?&local:&central;

   while (TRUE)
   {
      if (fOK=source->Fetch())
      {
         name = _name;
         if (fLocal)
         {
            ret = getGermplasmRecord(_name.gid<0, _name.gid, _germ);
            if (ret!=GMS_SUCCESS)
            {
               POP();
            }
            if (_germ.grplce==0 && _name.gid==_germ.gid)
               break;
         }
         else
         {
            if (_germ.grplce!=0) 
               continue;
            name.gid=_germ.gid;
         }
         // Check if it has been replaced or deleted then ignore it.
         LONG replacement;
         if (getReplacementGermplasmFromChanges(_name.gid,replacement)!=GMS_SUCCESS)
            break;
      }
      else break;
   }

   germ=_germ;

   ret=GMS_SUCCESS;

   if (fOK){
      string namestr;
      namestr = _nval;
      namestr += _next;
      strncpy(szName,namestr.c_str(),nszName);
      szName[nszName-1]='\0';
      //PadRight(szName,nszName);
   }
   else if (source->NoData())
      ret = GMS_NO_DATA;
   else 
      ret = GMS_ERROR;

   POP();


#undef SQL_FINDNAME_CENTRAL
#undef SQL_FINDNAME_LOCAL
#undef NVAL_EQ_SQL
#undef NVAL_LIKE_SQL
#undef SQL_FINDNAME_
}
*/
//////////////////////////////////////////////////////////////////////////////
//Changes:  2005/09/27 - when the searched string has a wild card and the Nstat of the found name is deleted,
//                       it will return GMS_INVALID_DATA(-11)
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_findName2(CHAR *szSearchName, GMS_Name *name, GMS_Germplasm *germ
               , CHAR *szName, LONG nszName, LONG fSearchOption)
{
	static bool fLocal=TRUE;
   LONG nid;
   static BOOL _wild;
#if defined(WEB)
    IManagedInterface *cpi = NULL;
#endif

   PUSH(GMS_findName2);
   if (fSearchOption==FIND_FIRST){
         fLocal=TRUE;
  	     _wild= strchr(szSearchName,'_') || strchr(szSearchName,'%');
   }
   if (fLocal){
      ret=findName2(LOCAL,szSearchName,*name,*germ,szName,nszName,fSearchOption);
      if (ret!=GMS_SUCCESS){
         fLocal = FALSE; 

#if defined(WEB)
			if(WEBSERVICE==true)
			{
				RcdNo = 0;
				fLocal = FALSE; 
				check=true;
				int str;
				//Enter Single Threaded Apartment (STA) - STA Thread
				CoInitialize(NULL);
//                MessageBox(NULL,"findName2 initialized!","ICIS V5",MB_OK);

				//Instantiate the COM object in the appropriate apartment
				HRESULT hr = CoCreateInstance(CLSID_LibraryImplementation,
					NULL, CLSCTX_INPROC_SERVER, IID_IManagedInterface, reinterpret_cast<void**>(&cpi)); 

				if(FAILED(hr))
				{
					MessageBox(NULL,"Could not create Instance, Web Service Failed!","ICIS V5",MB_OK);
				}

				str = cpi->LfindName2(szSearchName,szURL);
				
				if(str == -1)
				{
					MessageBox(NULL,"WebService Connection Lost. Unable to retrieve data","ICIS V5",MB_OK);
					return(GMS_ERROR);
				}

//ge				if(str != 0) name->gid = nb[RcdNo].NGID; 
				if(str != 0) name->gid = beanNameGerm[RcdNo].gid; 
				if((str == 0) || (name->gid == 0)){
					ret=GMS_NO_DATA;
					RcdNo=15;  
					CoUninitialize();
					cpi->Release();  //or 16					
				}
				else
				{
					// Mapping the C# Library data to the C data structure //
					name->nid = beanNameGerm[RcdNo].nid;
					name->gid = beanNameGerm[RcdNo].gid; 
					name->ntype = beanNameGerm[RcdNo].ntype;
					name->nstat = beanNameGerm[RcdNo].nstat;
					name->nuid = beanNameGerm[RcdNo].nuid;					
					strcpy(szName, beanNameGerm[RcdNo].nval);
					name->nlocn = beanNameGerm[RcdNo].nlocn;
					name->ndate = beanNameGerm[RcdNo].ndate;
					name->nref = beanNameGerm[RcdNo].nref;

					germ->gdate = beanNameGerm[RcdNo].gdate;
					germ->gid = beanNameGerm[RcdNo].gid ; 
					germ->glocn = beanNameGerm[RcdNo].glocn;
					germ->gnpgs = beanNameGerm[RcdNo].gnpgs;
					germ->gpid1 = beanNameGerm[RcdNo].gpid1;
					germ->gpid2 = beanNameGerm[RcdNo].gpid2;
					germ->gref = beanNameGerm[RcdNo].gref;
					germ->grplce = beanNameGerm[RcdNo].grplce;
					germ->guid = beanNameGerm[RcdNo].guid; 
					germ->lgid = beanNameGerm[RcdNo].lgid;
					germ->methn = beanNameGerm[RcdNo].methn;  

					ret = GMS_SUCCESS;
				}
			}
			else
#endif
         ret=findName2(CENTRAL,szSearchName,*name,*germ,szName,nszName,FIND_NEXT);
      }
	}
	else
	{
#if defined(WEB)
		if(WEBSERVICE==true)
		{
			check=true;
			if(RcdNo < idx)
			{
				RcdNo = RcdNo + 1;
					// Mapping the C# Library data to the C data structure //
					name->nid = beanNameGerm[RcdNo].nid;
					name->gid = beanNameGerm[RcdNo].gid;
					name->ntype = beanNameGerm[RcdNo].ntype;
					name->nstat = beanNameGerm[RcdNo].nstat;
					name->nuid = beanNameGerm[RcdNo].nuid;					
					strcpy(szName, beanNameGerm[RcdNo].nval);
					name->nlocn = beanNameGerm[RcdNo].nlocn;
					name->ndate = beanNameGerm[RcdNo].ndate;
					name->nref = beanNameGerm[RcdNo].nref;

					germ->gdate = beanNameGerm[RcdNo].gdate;
					germ->gid = beanNameGerm[RcdNo].gid ; 
					germ->glocn = beanNameGerm[RcdNo].glocn;
					germ->gnpgs = beanNameGerm[RcdNo].gnpgs;
					germ->gpid1 = beanNameGerm[RcdNo].gpid1;
					germ->gpid2 = beanNameGerm[RcdNo].gpid2;
					germ->gref = beanNameGerm[RcdNo].gref;
					germ->grplce = beanNameGerm[RcdNo].grplce;
					germ->guid = beanNameGerm[RcdNo].guid; 
					germ->lgid = beanNameGerm[RcdNo].lgid;
					germ->methn = beanNameGerm[RcdNo].methn;  
				ret = GMS_SUCCESS;

				if((germ->gid == 0)) {              // || (germ->gid > 1000000)){
					ret = GMS_NO_DATA;
  				    RcdNo = 0; idx=0;
					free(beanNameGerm);
				    CoUninitialize();
				}
			}
			else{
				ret = GMS_NO_DATA;
				RcdNo = 15; idx=0;
				CoUninitialize();
			}

		}
   else
#endif //WEB
      ret=findName2(CENTRAL,szSearchName,*name,*germ,szName,nszName,fSearchOption);
	}
	if(check==false)
	{
   if (ret == GMS_SUCCESS)  
   {
      nid = name->nid;
	  getNameFieldFromChanges(nid,"NSTAT",name->nstat);
      if (name->nstat == 9) {
		  if (_wild )
			  ret= GMS_INVALID_DATA;
		  else
		      ret = GMS_NO_DATA;
          ZeroMemory(name,sizeof(name));
		  ZeroMemory(szName, sizeof(szName));
		  ZeroMemory(germ, sizeof(germ));
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
// 04-Dec-2000 - added a call to applyGermplasmChanges to reflect changes
// 18-Nov-2003 - NSTAT <>9
//
//////////////////////////////////////////////////////////////////////////////
LONG findName2(BOOL fLocal,CHAR *szSearchName, GMS_Name &name, GMS_Germplasm &germ
               , CHAR *szName, LONG nszName, LONG fSearchOption)
{
#define SQL_FINDNAME2_CENTRAL "\
 select \
   NAMES.GID, NAMES.NTYPE, NAMES.NSTAT, NAMES.NUID, NAMES.NVAL\
  ,NAMES.NLOCN, NAMES.NDATE, NAMES.NREF, NAMES.NID \
  ,GERMPLSM.GID,GERMPLSM.METHN, GERMPLSM.GNPGS, GERMPLSM.GPID1\
  ,GERMPLSM.GPID2, GERMPLSM.GERMUID, GERMPLSM.LGID, GERMPLSM.GLOCN\
  ,GERMPLSM.GDATE,GERMPLSM.GREF, GERMPLSM.GRPLCE, GERMPLSM.MGID\
 from GERMPLSM,NAMES\
 where (GERMPLSM.GID=NAMES.GID) and NAMES.NSTAT <> 9 and"

#define SQL_FINDNAME2_LOCAL "\
 select NAMES.GID, NAMES.NTYPE, NAMES.NSTAT, NAMES.NUID, NAMES.NVAL\
   ,NAMES.NLOCN, NAMES.NDATE, NAMES.NREF, NAMES.NID \
 from NAMES  \
 where NAMES.NSTAT <> 9 and "

#define SQL_FINDNAME2_ \
  " and (0=? or NAMES.NSTAT=?)\
 order by NAMES.NVAL"

#define NVAL2_EQ_SQL     " (NAMES.NVAL = ?) "
#define NVAL2_LIKE_SQL   " (NAMES.NVAL like ?) "
#define NVAL1_EQ_SQL     " (NAMES.NVAL)  = ?"
#define NVAL1_LIKE_SQL   " NAMES.NVAL  LIKE ?"


   PUSH(findName2);

static CODBCbindedStmt &central= _centralDBC->BindedStmt();
static CODBCbindedStmt &local  = _localDBC->BindedStmt();
       CODBCbindedStmt *source;

static GMS_Name _name;
static GMS_Germplasm _germ;
static CHAR _nval[GMS_NVAL_SIZE+1],_next[MAX_STR];
static BOOL bind=TRUE;

static BOOL changed=TRUE,long1=FALSE,long2=FALSE,long3=FALSE,long4=FALSE;
static BOOL wild=FALSE;
static BOOL first_time=TRUE,match;
// These values determines a string if it has a wild character
static BOOL nval1_wild,next1_wild,
            nval2_wild,next2_wild,
            nval3_wild,next3_wild,
            nval4_wild;

//Parameters
static LONG nstat,len;
static CHAR nval1[MAX_STR],nval2[MAX_STR],nval3[MAX_STR],nval4[MAX_STR];
// These char pointers points automatically to the 31st character
static CHAR *pnext1=nval1+GMS_NVAL_SIZE+1,
            *pnext2=nval2+GMS_NVAL_SIZE+1,
            *pnext3=nval3+GMS_NVAL_SIZE+1;
static string strName1,strName2,strName3;
string strName;


BOOL  fOK;


   if (fSearchOption==FIND_FIRST)
   {
      ZeroFill(nval1,sizeof(nval1));
      ZeroFill(nval2,sizeof(nval2));
      ZeroFill(nval3,sizeof(nval3));
	  ZeroFill(nval4,sizeof(nval4));

      nstat=name.nstat;

      // Original Name uppercased
      strcpy(nval1, szSearchName);
      strupr(nval1);
	  strcpy(nval4,szSearchName);

      // Name with all spaces deleted
      CHAR *p = nval1;
      int i =0;
      while (*p) 
         if (*(p++)!=' ') 
            nval2[i++]=*(p-1);
      nval2[i]='\0';

      // Standardized Name
      standardName(nval1,nval3,MAX_STR);

      changed = Changed(long1,strlen(nval1)>GMS_NVAL_SIZE) |
                Changed(long2,strlen(nval2)>GMS_NVAL_SIZE) |
                Changed(long3,strlen(nval3)>GMS_NVAL_SIZE);
                Changed(long4,strlen(nval4)>GMS_NVAL_SIZE);

      if (first_time)
      {
         changed=TRUE;
         first_time=FALSE;
      }

      next1_wild=next2_wild=next3_wild;

      strName1=nval1;
      strName2=nval2;
      strName3=nval3;

      if (long1)
      {
         memmove(nval1+GMS_NVAL_SIZE+1,nval1+GMS_NVAL_SIZE,strlen(nval1)-GMS_NVAL_SIZE);
         nval1[GMS_NVAL_SIZE]='\0';
         next1_wild=strchr(pnext1,'_') || strchr(pnext1,'%');
      }

      if (long2)
      {
         memmove(nval2+GMS_NVAL_SIZE+1,nval2+GMS_NVAL_SIZE,strlen(nval2)-GMS_NVAL_SIZE);
         nval2[GMS_NVAL_SIZE]='\0';
         next2_wild=strchr(pnext2,'_') || strchr(pnext2,'%');
      }
      if (long3)
      {
         memmove(nval3+GMS_NVAL_SIZE+1,nval3+GMS_NVAL_SIZE,strlen(nval3)-GMS_NVAL_SIZE);
         nval3[GMS_NVAL_SIZE]='\0';
         next3_wild=strchr(pnext3,'_') || strchr(pnext3,'%');
      }

      if (Changed(nval1_wild,strchr(nval1,'_') || strchr(nval1,'%')) |
                Changed(nval2_wild,strchr(nval2,'_') || strchr(nval2,'%')) |
                Changed(nval3_wild,strchr(nval3,'_') || strchr(nval3,'%')) |
				Changed(nval4_wild,strchr(nval3,'_') || strchr(nval4,'%')))
         changed=TRUE;

      if (changed)
      {
         string central_sql,local_sql,strtemp;

         central_sql=SQL_FINDNAME2_CENTRAL;
         central_sql+="(";
         local_sql=SQL_FINDNAME2_LOCAL;
         local_sql+="(";

         strtemp+=(nval1_wild?NVAL1_LIKE_SQL:NVAL1_EQ_SQL);
         strtemp+=" or ";
         strtemp+=(nval2_wild?NVAL2_LIKE_SQL:NVAL2_EQ_SQL);
         strtemp+=" or ";
         strtemp+=(nval3_wild?NVAL2_LIKE_SQL:NVAL2_EQ_SQL);
         strtemp+=" or ";
		 strtemp+=(nval4_wild?NVAL2_LIKE_SQL:NVAL2_EQ_SQL);

         local_sql+=strtemp+")"+SQL_FINDNAME2_;
         central_sql+=strtemp+")"+SQL_FINDNAME2_;

         central= _centralDBC->BindedStmt((LPCSTR)central_sql.c_str());            //4/29/2003 (AMP):  reassign the statement handle if previous statement is different from the current one
         local  = _localDBC->BindedStmt((LPCSTR)local_sql.c_str());              //4/29/2003 (AMP):  reassign the statement handle if previous statement is different from the current one

//AMP: deleted 2006/06/05         central.SetSQLstr(central_sql.c_str());
//AMP: deleted 2006/06/05         local.SetSQLstr(local_sql.c_str());

         BIND(1,_name.gid);
         BIND(2,_name.ntype);
         BIND(3,_name.nstat);
         BIND(4,_name.nuid);
         BINDS(5,_nval,255);
         BIND(6,_name.nlocn);
         BIND(7,_name.ndate);
         BIND(8,_name.nref);
		 BIND(9,_name.nid);
         //delete: BINDS(9,_next,MAX_STR);
         central.Bind(10,_germ.gid);
         central.Bind(11,_germ.methn);
         central.Bind(12,_germ.gnpgs);
         central.Bind(13,_germ.gpid1);
         central.Bind(14,_germ.gpid2);
         central.Bind(15,_germ.guid);
         central.Bind(16,_germ.lgid);
         central.Bind(17,_germ.glocn);
         central.Bind(18,_germ.gdate);
         central.Bind(19,_germ.gref);
         central.Bind(20,_germ.grplce);
         central.Bind(21,_germ.ggid);
         BINDPARAMS(1,nval1,255);
         BINDPARAMS(2,nval2,255);
         BINDPARAMS(3,nval3,255);
         BINDPARAMS(4,nval4,255);
         BINDPARAM(5,nstat);
         BINDPARAM(6,nstat);

         changed = FALSE;
      }

      local.Execute();
      central.Execute();
   }

   source = (fLocal)?&local:&central;

   while (TRUE)
   {
	  _nval[0]='\0';
	  _next[0]='\0';
      if (fOK=source->Fetch())
      {
         name = _name;

         strName = _nval;
         //delete: strName += _next;

         //Check the next as necessary
         //delete: match=wildcardfit ((char*)strName1.c_str(),(char*)strName.c_str())|
         //      wildcardfit ((char*)strName2.c_str(),(char*)strName.c_str())|
         //      wildcardfit ((char*)strName3.c_str(),(char*)strName.c_str());
         //if (!match)
         //   continue;
         if (fLocal)
         {
            ret = getGermplasmRecord(_name.gid<0, _name.gid, _germ);
            if (ret!=GMS_SUCCESS && ret!=GMS_NO_DATA)
            {
               POP();
            }
            if (_germ.grplce==0 && _name.gid==_germ.gid)
               break;
         }
         else
         {
            if (_germ.grplce!=0)
               continue;
            name.gid=_germ.gid;
         }
         // Check if it has been replaced or deleted then ignore it.
         LONG replacement;
         if (getReplacementGermplasmFromChanges(_name.gid,replacement)!=GMS_SUCCESS)
            break;
      }
      else break;
   }

   germ=_germ;
   applyGermplasmChanges(germ);  // added 04-Dec-2000

   ret=GMS_SUCCESS;

   if (fOK){
      strncpy(szName,strName.c_str(),nszName);
      szName[nszName-1]='\0';
   }
   else if (source->NoData())
      ret = GMS_NO_DATA;
   else 
      ret = GMS_ERROR;

   POP();


#undef SQL_FINDNAME2_CENTRAL
#undef SQL_FINDNAME2_LOCAL
#undef NVAL2_SQL
#undef NVAL2_LIKE_SQL
#undef NEXT2_SQL
#undef NEXT2_LIKE_SQL
#undef BUILDEXP
}



/*  Original: 2003/07/25
//////////////////////////////////////////////////////////////////////////////
//
// 04-Dec-2000 - added a call to applyGermplasmChanges to reflect changes
//
//////////////////////////////////////////////////////////////////////////////
LONG findName2(BOOL fLocal,CHAR *szSearchName, GMS_Name &name, GMS_Germplasm &germ
               , CHAR *szName, LONG nszName, LONG fSearchOption)
{
#define SQL_FINDNAME2_CENTRAL "\
 select \
   NAMES.GID, NAMES.NTYPE, NAMES.NSTAT, NAMES.NUID, NAMES.NVAL\
  ,NAMES.NLOCN, NAMES.NDATE, NAMES.NREF, NAMES.NID \
  ,GERMPLSM.GID,GERMPLSM.METHN, GERMPLSM.GNPGS, GERMPLSM.GPID1\
  ,GERMPLSM.GPID2, GERMPLSM.GERMUID, GERMPLSM.LGID, GERMPLSM.GLOCN\
  ,GERMPLSM.GDATE,GERMPLSM.GREF, GERMPLSM.GRPLCE\
 from GERMPLSM,NAMES\
 where (GERMPLSM.GID=NAMES.GID) and "

#define SQL_FINDNAME2_LOCAL "\
 select NAMES.GID, NAMES.NTYPE, NAMES.NSTAT, NAMES.NUID, NAMES.NVAL\
   ,NAMES.NLOCN, NAMES.NDATE, NAMES.NREF, NAMES.NID \
 from NAMES  \
 where "

#define SQL_FINDNAME2_ \
  " and (0=? or NAMES.NSTAT=?)\
 order by NAMES.NVAL"

#define NVAL2_EQ_SQL     " (NAMES.NVAL = ?) "
#define NVAL2_LIKE_SQL   " (NAMES.NVAL like ?) "


   PUSH(findName2);

static CODBCbindedStmt &central= _centralDBC->BindedStmt();
static CODBCbindedStmt &local  = _localDBC->BindedStmt();
       CODBCbindedStmt *source;

static GMS_Name _name;
static GMS_Germplasm _germ;
static CHAR _nval[GMS_NVAL_SIZE+1],_next[MAX_STR];
static BOOL bind=TRUE;

static BOOL changed=TRUE,long1=FALSE,long2=FALSE,long3=FALSE;
static BOOL wild=FALSE;
static BOOL first_time=TRUE,match;
// These values determines a string if it has a wild character
static BOOL nval1_wild,next1_wild,
            nval2_wild,next2_wild,
            nval3_wild,next3_wild;

//Parameters
static LONG nstat,len;
static CHAR nval1[MAX_STR],nval2[MAX_STR],nval3[MAX_STR];
// These char pointers points automatically to the 31st character
static CHAR *pnext1=nval1+GMS_NVAL_SIZE+1,
            *pnext2=nval2+GMS_NVAL_SIZE+1,
            *pnext3=nval3+GMS_NVAL_SIZE+1;
static string strName1,strName2,strName3;
string strName;


BOOL  fOK;


   if (fSearchOption==FIND_FIRST)
   {
      ZeroFill(nval1,sizeof(nval1));
      ZeroFill(nval2,sizeof(nval2));
      ZeroFill(nval3,sizeof(nval3));

      nstat=name.nstat;

      // Original Name uppercased
      strcpy(nval1, szSearchName);
      strupr(nval1);

      // Name with all spaces deleted
      CHAR *p = nval1;
      int i =0;
      while (*p) 
         if (*(p++)!=' ') 
            nval2[i++]=*(p-1);
      nval2[i]='\0';

      // Standardized Name
      standardName(nval1,nval3,MAX_STR);

      changed = Changed(long1,strlen(nval1)>GMS_NVAL_SIZE) |
                Changed(long2,strlen(nval2)>GMS_NVAL_SIZE) |
                Changed(long3,strlen(nval3)>GMS_NVAL_SIZE);

      if (first_time)
      {
         changed=TRUE;
         first_time=FALSE;
      }

      next1_wild=next2_wild=next3_wild;

      strName1=nval1;
      strName2=nval2;
      strName3=nval3;

      if (long1)
      {
         memmove(nval1+GMS_NVAL_SIZE+1,nval1+GMS_NVAL_SIZE,strlen(nval1)-GMS_NVAL_SIZE);
         nval1[GMS_NVAL_SIZE]='\0';
         next1_wild=strchr(pnext1,'_') || strchr(pnext1,'%');
      }

      if (long2)
      {
         memmove(nval2+GMS_NVAL_SIZE+1,nval2+GMS_NVAL_SIZE,strlen(nval2)-GMS_NVAL_SIZE);
         nval2[GMS_NVAL_SIZE]='\0';
         next2_wild=strchr(pnext2,'_') || strchr(pnext2,'%');
      }
      if (long3)
      {
         memmove(nval3+GMS_NVAL_SIZE+1,nval3+GMS_NVAL_SIZE,strlen(nval3)-GMS_NVAL_SIZE);
         nval3[GMS_NVAL_SIZE]='\0';
         next3_wild=strchr(pnext3,'_') || strchr(pnext3,'%');
      }

      if (Changed(nval1_wild,strchr(nval1,'_') || strchr(nval1,'%')) |
                Changed(nval2_wild,strchr(nval2,'_') || strchr(nval2,'%')) |
                Changed(nval3_wild,strchr(nval3,'_') || strchr(nval3,'%')))
         changed=TRUE;

      if (changed)
      {
         string central_sql,local_sql,strtemp;

         central_sql=SQL_FINDNAME2_CENTRAL;
         central_sql+="(";
         local_sql=SQL_FINDNAME2_LOCAL;
         local_sql+="(";

         strtemp+=(nval1_wild?NVAL2_LIKE_SQL:NVAL2_EQ_SQL);
         strtemp+=" or ";
         strtemp+=(nval2_wild?NVAL2_LIKE_SQL:NVAL2_EQ_SQL);
         strtemp+=" or ";
         strtemp+=(nval3_wild?NVAL2_LIKE_SQL:NVAL2_EQ_SQL);

         local_sql+=strtemp+")"+SQL_FINDNAME2_;
         central_sql+=strtemp+")"+SQL_FINDNAME2_;

         central= _centralDBC->BindedStmt((LPCSTR)central_sql.c_str());            //4/29/2003 (AMP):  reassign the statement handle if previous statement is different from the current one
         local  = _localDBC->BindedStmt((LPCSTR)local_sql.c_str());              //4/29/2003 (AMP):  reassign the statement handle if previous statement is different from the current one

//AMP: deleted 2006/06/05         central.SetSQLstr(central_sql.c_str());
//AMP: deleted 2006/06/05         local.SetSQLstr(local_sql.c_str());

         BIND(1,_name.gid);
         BIND(2,_name.ntype);
         BIND(3,_name.nstat);
         BIND(4,_name.nuid);
         BINDS(5,_nval,GMS_NVAL_SIZE+1);
         BIND(6,_name.nlocn);
         BIND(7,_name.ndate);
         BIND(8,_name.nref);
		 BIND(9,_name.nid);
         //delete: BINDS(9,_next,MAX_STR);
         central.Bind(10,_germ.gid);
         central.Bind(11,_germ.methn);
         central.Bind(12,_germ.gnpgs);
         central.Bind(13,_germ.gpid1);
         central.Bind(14,_germ.gpid2);
         central.Bind(15,_germ.guid);
         central.Bind(16,_germ.lgid);
         central.Bind(17,_germ.glocn);
         central.Bind(18,_germ.gdate);
         central.Bind(19,_germ.gref);
         central.Bind(20,_germ.grplce);
         BINDPARAMS(1,nval1,GMS_NVAL_SIZE);
         BINDPARAMS(2,nval2,GMS_NVAL_SIZE);
         BINDPARAMS(3,nval3,GMS_NVAL_SIZE);
         BINDPARAM(4,nstat);
         BINDPARAM(5,nstat);

         changed = FALSE;
      }

      local.Execute();
      central.Execute();
   }

   source = (fLocal)?&local:&central;

   while (TRUE)
   {
	  _nval[0]='\0';
	  _next[0]='\0';
      if (fOK=source->Fetch())
      {
         name = _name;

         strName = _nval;
         //delete: strName += _next;

         //Check the next as necessary
         match=wildcardfit ((char*)strName1.c_str(),(char*)strName.c_str())|
               wildcardfit ((char*)strName2.c_str(),(char*)strName.c_str())|
               wildcardfit ((char*)strName3.c_str(),(char*)strName.c_str());
         if (!match)
            continue;
         if (fLocal)
         {
            ret = getGermplasmRecord(_name.gid<0, _name.gid, _germ);
            if (ret!=GMS_SUCCESS && ret!=GMS_NO_DATA)
            {
               POP();
            }
            if (_germ.grplce==0 && _name.gid==_germ.gid)
               break;
         }
         else
         {
            if (_germ.grplce!=0)
               continue;
            name.gid=_germ.gid;
         }
         // Check if it has been replaced or deleted then ignore it.
         LONG replacement;
         if (getReplacementGermplasmFromChanges(_name.gid,replacement)!=GMS_SUCCESS)
            break;
      }
      else break;
   }

   germ=_germ;
   applyGermplasmChanges(germ);  // added 04-Dec-2000

   ret=GMS_SUCCESS;

   if (fOK){
      strncpy(szName,strName.c_str(),nszName);
      szName[nszName-1]='\0';
   }
   else if (source->NoData())
      ret = GMS_NO_DATA;
   else 
      ret = GMS_ERROR;

   POP();


#undef SQL_FINDNAME2_CENTRAL
#undef SQL_FINDNAME2_LOCAL
#undef NVAL2_SQL
#undef NVAL2_LIKE_SQL
#undef NEXT2_SQL
#undef NEXT2_LIKE_SQL
#undef BUILDEXP
}
*/



//////////////////////////////////////////////////////////////////////////////
//
// Added a filter on Location
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_findName3(CHAR *szSearchName, GMS_Name *name, GMS_Germplasm *germ
               , CHAR *szName, LONG nszName, LONG fSearchOption)
{
   static TFINDNAMEPTR findname=&findName3;
   static BOOL fLocal=TRUE;
   LONG nid;

   PUSH(GMS_findName3);
   if (fSearchOption==FIND_FIRST)
   {
      fLocal=TRUE;
      findname = name->nlocn||name->ntype?&findName3:&findName2;
   }
   if (fLocal){
      ret=findname(LOCAL,szSearchName,*name,*germ,szName,nszName,fSearchOption);
      if (ret!=GMS_SUCCESS){
         fLocal = FALSE; 
#if defined(WEB)
		 if(WEBSERVICE==true)
			{
				RcdNo = 0;
				fLocal = FALSE; 
				check=true;
				int str;
				IManagedInterface *cpi = NULL;
				//Enter Single Threaded Apartment (STA) - STA Thread
				CoInitialize(NULL);

				//Instantiate the COM object in the appropriate apartment
				HRESULT hr = CoCreateInstance(CLSID_LibraryImplementation,
					NULL, CLSCTX_INPROC_SERVER, IID_IManagedInterface, reinterpret_cast<void**>(&cpi)); 

				if(FAILED(hr))
				{
					MessageBox(NULL,"Could not create Instance, Web Service Failed!","ICIS V5",MB_OK);
				}

				str = cpi->LfindName2(szSearchName,szURL);
				if(str == -1)
				{
					MessageBox(NULL,"WebService Connection Lost. Unable to retrieve data","ICIS V5",MB_OK);
					return(GMS_ERROR);
				}
				if(str != 0) name->gid = nb[RcdNo].NGID; 
				if((str == 0) || (name->gid == 0)){
					ret=GMS_NO_DATA;
					RcdNo=15;  
					CoUninitialize();
					cpi->Release();  //or 16					
					return ret;
				}
				else
				{
					/* Mapping the C# Library data to the C data structure */
					name->nid = nb[RcdNo].NID;
					// name.gid = nb[RcdNo].NGID; Already assigned 
					name->ntype = nb[RcdNo].NTYPE;
					name->nstat = nb[RcdNo].NSTAT;
					name->nuid = nb[RcdNo].NUID;
					szName = nb[RcdNo].NVAL;
					name->nlocn = nb[RcdNo].NLOCN;
                    name->ndate = nb[RcdNo].NDATE;
					name->nref = nb[RcdNo].NREF;

					germ->gdate = nb[RcdNo].GDATE;
					germ->gid = nb[RcdNo].GGID; 
					germ->glocn = nb[RcdNo].GLOCN;
					germ->gnpgs = nb[RcdNo].GNPGS;
					germ->gpid1 = nb[RcdNo].GPID1;
					germ->gpid2 = nb[RcdNo].GPID2;
					germ->gref = nb[RcdNo].GREF;
					germ->grplce = nb[RcdNo].GRPLCE;
					germ->guid = nb[RcdNo].GERMUID; 
					germ->lgid = nb[RcdNo].LGID;
					germ->methn = nb[RcdNo].METHN; 
					
					ret = GMS_SUCCESS;
					return ret;
				}
			}
			else
#endif
         ret=findname(CENTRAL,szSearchName,*name,*germ,szName,nszName,FIND_NEXT);
      }
	}
	else
	{
#if defined(WEB)
		if(WEBSERVICE==true)
		{
			check=true;
			if(RcdNo < idx)
			{
				RcdNo = RcdNo + 1;
				/* Mapping the C# Library data to the C data structure */
				name->nid = nb[RcdNo].NID;
				name->gid = nb[RcdNo].NGID;
				name->ntype = nb[RcdNo].NTYPE;
				name->nstat = nb[RcdNo].NSTAT;
				name->nuid = nb[RcdNo].NUID;
				szName = nb[RcdNo].NVAL;
				name->nlocn = nb[RcdNo].NLOCN;
				name->ndate = nb[RcdNo].NDATE;
				name->nref = nb[RcdNo].NREF;

				germ->gdate = nb[RcdNo].GDATE;
				germ->gid = nb[RcdNo].GGID; 
				germ->glocn = nb[RcdNo].GLOCN;
				germ->gnpgs = nb[RcdNo].GNPGS;
				germ->gpid1 = nb[RcdNo].GPID1;
				germ->gpid2 = nb[RcdNo].GPID2;
				germ->gref = nb[RcdNo].GREF;
				germ->grplce = nb[RcdNo].GRPLCE;
				germ->guid = nb[RcdNo].GERMUID; 
				germ->lgid = nb[RcdNo].LGID;
				germ->methn = nb[RcdNo].METHN; 

				ret = GMS_SUCCESS;

				if((germ->gid == 0) || (germ->gid > 1000000))
					ret = GMS_NO_DATA;
				return ret;
			}
			else{
				ret = GMS_NO_DATA;
				RcdNo = 15; idx=0;
				CoUninitialize();
				return ret;
			}
		}
   else
#endif
      ret=findname(CENTRAL,szSearchName,*name,*germ,szName,nszName,fSearchOption);
	}

	if(check==false)
	{
	//determine if a name is deleted; if so, return no data
   if (ret == GMS_SUCCESS)  
   {
      nid = name->nid;
	  getNameFieldFromChanges(nid,"NSTAT",name->nstat);
      if (name->nstat == 9) {
		  ret = GMS_NO_DATA;
          ZeroMemory(name,sizeof(name));
		  ZeroMemory(szName, sizeof(szName));
		  ZeroMemory(germ, sizeof(germ));
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
LONG findName3(BOOL fLocal,CHAR *szSearchName, GMS_Name &name, GMS_Germplasm &germ
               , CHAR *szName, LONG nszName, LONG fSearchOption)
{
#define SQL_FINDNAME3_CENTRAL "\
 select \
   NAMES.GID, NAMES.NTYPE, NAMES.NSTAT, NAMES.NUID, NAMES.NVAL\
  ,NAMES.NLOCN, NAMES.NDATE, NAMES.NREF, NAMES.NID  \
  ,GERMPLSM.GID,GERMPLSM.METHN, GERMPLSM.GNPGS, GERMPLSM.GPID1\
  ,GERMPLSM.GPID2, GERMPLSM.GERMUID, GERMPLSM.LGID, GERMPLSM.GLOCN\
  ,GERMPLSM.GDATE,GERMPLSM.GREF, GERMPLSM.GRPLCE, GERMPLSM.MGID\
 from GERMPLSM,NAMES\
 where (GERMPLSM.GID=NAMES.GID) and NAMES.NSTAT <> 9 and "

#define SQL_FINDNAME3_LOCAL "\
 select NAMES.GID, NAMES.NTYPE, NAMES.NSTAT, NAMES.NUID, NAMES.NVAL\
   ,NAMES.NLOCN, NAMES.NDATE, NAMES.NREF, NAMES.NID  \
 from NAMES  \
 where NAMES.NSTAT <> 9 and "

#define SQL_FINDNAME3_ \
  " and (0=? or NAMES.NSTAT=?) and (0=? or NAMES.NTYPE=?) and (0=? or NAMES.NLOCN=?)\
 order by NAMES.NVAL"

#define NVAL3_EQ_SQL     " (NAMES.NVAL = ?) "
#define NVAL3_LIKE_SQL   " (NAMES.NVAL like ?) "


   PUSH(findNAME3);

static CODBCbindedStmt &central= _centralDBC->BindedStmt();
static CODBCbindedStmt &local  = _localDBC->BindedStmt();
       CODBCbindedStmt *source;

static GMS_Name _name;
static GMS_Germplasm _germ;
static CHAR _nval[GMS_NVAL_SIZE+1],_next[MAX_STR];
static BOOL bind=TRUE;

static BOOL changed=TRUE,long1=FALSE,long2=FALSE,long3=FALSE;
static BOOL wild=FALSE;
static BOOL first_time=TRUE,match;
// These values determines a string if it has a wild character
static BOOL nval1_wild,next1_wild,
            NVAL3_wild,NEXT3_wild,
            nval3_wild,next3_wild;

//Parameters
static LONG nstat,ntype,nlocn,len;
static CHAR nval1[MAX_STR],NVAL3[MAX_STR],nval3[MAX_STR];
// These char pointers points automatically to the 31st character
static CHAR *pnext1=nval1+GMS_NVAL_SIZE+1,
            *pNEXT3=NVAL3+GMS_NVAL_SIZE+1,
            *pnext3=nval3+GMS_NVAL_SIZE+1;
static string strName1,strNAME3,strName3;
string strName;


BOOL  fOK;


   if (fSearchOption==FIND_FIRST)
   {
      ZeroFill(nval1,sizeof(nval1));
      ZeroFill(NVAL3,sizeof(NVAL3));
      ZeroFill(nval3,sizeof(nval3));

      nstat=name.nstat;
      ntype=name.ntype;
      nlocn=name.nlocn;

      // Original Name uppercased
      strcpy(nval1, szSearchName);
      strupr(nval1);

      // Name with all spaces deleted
      CHAR *p = nval1;
      int i =0;
      while (*p) 
         if (*(p++)!=' ') 
            NVAL3[i++]=*(p-1);
      NVAL3[i]='\0';

      // Standardized Name
      standardName(nval1,nval3,MAX_STR);

      changed = Changed(long1,strlen(nval1)>GMS_NVAL_SIZE) |
                Changed(long2,strlen(NVAL3)>GMS_NVAL_SIZE) |
                Changed(long3,strlen(nval3)>GMS_NVAL_SIZE);

      if (first_time)
      {
         changed=TRUE;
         first_time=FALSE;
      }

      next1_wild=NEXT3_wild=next3_wild;

      strName1=nval1;
      strNAME3=NVAL3;
      strName3=nval3;

      if (long1)
      {
         memmove(nval1+GMS_NVAL_SIZE+1,nval1+GMS_NVAL_SIZE,strlen(nval1)-GMS_NVAL_SIZE);
         nval1[GMS_NVAL_SIZE]='\0';
         //next1_wild=strchr(pnext1,'_') || strchr(pnext1,'%');
      }

      if (long2)
      {
         memmove(NVAL3+GMS_NVAL_SIZE+1,NVAL3+GMS_NVAL_SIZE,strlen(NVAL3)-GMS_NVAL_SIZE);
         NVAL3[GMS_NVAL_SIZE]='\0';
         //NEXT3_wild=strchr(pNEXT3,'_') || strchr(pNEXT3,'%');
      }
      if (long3)
      {
         memmove(nval3+GMS_NVAL_SIZE+1,nval3+GMS_NVAL_SIZE,strlen(nval3)-GMS_NVAL_SIZE);
         nval3[GMS_NVAL_SIZE]='\0';
         //next3_wild=strchr(pnext3,'_') || strchr(pnext3,'%');
      }

      if (Changed(nval1_wild,strchr(nval1,'_') || strchr(nval1,'%')) |
                Changed(NVAL3_wild,strchr(NVAL3,'_') || strchr(NVAL3,'%')) |
                Changed(nval3_wild,strchr(nval3,'_') || strchr(nval3,'%')))
         changed=TRUE;

      if (changed)
      {
         string central_sql,local_sql,strtemp;

         central_sql=SQL_FINDNAME3_CENTRAL;
         central_sql+="(";
         local_sql=SQL_FINDNAME3_LOCAL;
         local_sql+="(";

         strtemp+=(nval1_wild?NVAL3_LIKE_SQL:NVAL3_EQ_SQL);
         strtemp+=" or ";
         strtemp+=(NVAL3_wild?NVAL3_LIKE_SQL:NVAL3_EQ_SQL);
         strtemp+=" or ";
         strtemp+=(nval3_wild?NVAL3_LIKE_SQL:NVAL3_EQ_SQL);

         local_sql+=strtemp+")"+SQL_FINDNAME3_;
         central_sql+=strtemp+")"+SQL_FINDNAME3_;

         central.SetSQLstr(central_sql.c_str());
         local.SetSQLstr(local_sql.c_str());

         BIND(1,_name.gid);
         BIND(2,_name.ntype);
         BIND(3,_name.nstat);
         BIND(4,_name.nuid);
         BINDS(5,_nval,GMS_NVAL_SIZE+1);
         BIND(6,_name.nlocn);
         BIND(7,_name.ndate);
         BIND(8,_name.nref);
			BIND(9,_name.nid);
         //delete:BINDS(9,_next,MAX_STR);
         central.Bind(10,_germ.gid);
         central.Bind(11,_germ.methn);
         central.Bind(12,_germ.gnpgs);
         central.Bind(13,_germ.gpid1);
         central.Bind(14,_germ.gpid2);
         central.Bind(15,_germ.guid);
         central.Bind(16,_germ.lgid);
         central.Bind(17,_germ.glocn);
         central.Bind(18,_germ.gdate);
         central.Bind(19,_germ.gref);
         central.Bind(20,_germ.grplce);
         central.Bind(21,_germ.ggid);
         BINDPARAMS(1,nval1,GMS_NVAL_SIZE);
         BINDPARAMS(2,NVAL3,GMS_NVAL_SIZE);
         BINDPARAMS(3,nval3,GMS_NVAL_SIZE);
         BINDPARAM(4,nstat);
         BINDPARAM(5,nstat);
         BINDPARAM(6,ntype);
         BINDPARAM(7,ntype);
         BINDPARAM(8,nlocn);
         BINDPARAM(9,nlocn);

         changed = FALSE;
      }

      local.Execute();
      central.Execute();
   }

   source = (fLocal)?&local:&central;

   while (TRUE)
   {
	  _nval[0]='\0';
	  _next[0]='\0';
      if (fOK=source->Fetch())
      {
         name = _name;

         strName = _nval;
         strName += _next;

         //Check the next as necessary
         match=wildcardfit ((char*)strName1.c_str(),(char*)strName.c_str())|
               wildcardfit ((char*)strNAME3.c_str(),(char*)strName.c_str())|
               wildcardfit ((char*)strName3.c_str(),(char*)strName.c_str());
         if (!match)
            continue;
         if (fLocal)
         {
            ret = getGermplasmRecord(_name.gid<0, _name.gid, _germ);
            if (ret!=GMS_SUCCESS && ret!=GMS_NO_DATA)
            {
               POP();
            }
            if (_germ.grplce==0 && _name.gid==_germ.gid)
               break;
         }
         else
         {
            if (_germ.grplce!=0)
               continue;
            name.gid=_germ.gid;
         }
         // Check if it has been replaced or deleted then ignore it.
         LONG replacement;
         if (getReplacementGermplasmFromChanges(_name.gid,replacement)!=GMS_SUCCESS)
            break;
      }
      else break;
   }

   germ=_germ;
   applyGermplasmChanges(germ);  // added 04-Dec-2000

   ret=GMS_SUCCESS;

   if (fOK){
      strncpy(szName,strName.c_str(),nszName);
      szName[nszName-1]='\0';
   }
   else if (source->NoData())
      ret = GMS_NO_DATA;
   else 
      ret = GMS_ERROR;

   POP();


#undef SQL_FINDNAME3_CENTRAL
#undef SQL_FINDNAME3_LOCAL
#undef NVAL3_SQL
#undef NVAL3_LIKE_SQL
#undef NEXT3_SQL
#undef NEXT3_LIKE_SQL
#undef BUILDEXP
}




//////////////////////////////////////////////////////////////////////////////
//Input:  GID, NSTAT
//////////////////////////////////////////////////////////////////////////////
LONG setName(BOOL fLocal,GMS_Name &name, CHAR *szName, LONG nszName)
{
#define SQL_SETNAMEREC "\
  update NAMES\
  set NTYPE=?, NSTAT=?, NUID=?\
    , NVAL=?, NLOCN = ?, NDATE = ?, NREF=? \
  where NID=? "

  PUSH(setName);

static CODBCbindedStmt &central= _centralDBC->BindedStmt(SQL_SETNAMEREC);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_SETNAMEREC);
       CODBCbindedStmt *source;

static GMS_Name _data;
static CHAR nval[GMS_MAX_NVAL],*pnext;
static BOOL first_time=TRUE;

//Parameters
static LONG _gid;
   if (first_time)
   {
      BINDPARAM(1,_data.ntype);
      BINDPARAM(2,_data.nstat);
      BINDPARAM(3,_data.nuid);
	  BINDPARAMS(4, nval, ICIS_MAX_SIZE );
      BINDPARAM(5,_data.nlocn);
      BINDPARAM(6,_data.ndate);
      BINDPARAM(7,_data.nref);
      BINDPARAM(8,_data.nid);
 

      first_time = FALSE;
   }

   source = (fLocal)?&local:&central;
   strncpy(nval,szName,GMS_NVAL_SIZE);
   nval[GMS_NVAL_SIZE]='\0';
   _data=name;

   if (source->Execute())
      ret = GMS_SUCCESS;
   else if (source->NoData())
      ret = GMS_NO_DATA;
   else 
      ret = GMS_ERROR;

   POP();

#undef SQL_SETNAMEREC
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_setName(GMS_Name *name, CHAR *szName, LONG nszName)
{
   PUSH(GMS_setName);

   if (UACCESS<30) 
      ret = GMS_NO_ACCESS;
   else if (name->nid>=0)
      ret =GMS_ERROR;
   else
      ret= setName(TRUE, *name, szName, nszName);
   POP();

}


//////////////////////////////////////////////////////////////////////////////
//::GMS_listNames
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_listNames(LONG gid,CHAR* szBuffer, LONG nszBuffer)
{
   LONG ret = listNames(gid,szBuffer,nszBuffer);
   return ret;
}

//////////////////////////////////////////////////////////////////////////////
//::GMS_listNames
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_listNamesEx(LONG gid,CHAR* szBuffer, LONG nszBuffer,LPCSTR szDelimeter)
{
   LONG ret = listNamesEx(gid,szBuffer,nszBuffer,szDelimeter);
   return ret;
}

//////////////////////////////////////////////////////////////////////////////
//::listNames
//////////////////////////////////////////////////////////////////////////////
LONG listNames(LONG gid,CHAR* szBuffer, LONG nszBuffer)
{
   return listNamesEx(gid,szBuffer,nszBuffer,NULL);
}
//////////////////////////////////////////////////////////////////////////////
//::listNames2
//////////////////////////////////////////////////////////////////////////////
LONG listNamesEx(LONG gid,CHAR* szBuffer, LONG nszBuffer,LPCSTR szDelimeter)
{
   LONG ret;
   GMS_Name n;
   CHAR szName[MAX_STR],*p;
   stringarray abuffer;
   
   *szBuffer='\0';
   p=szBuffer;
   n.gid=gid;

   // Preferred Name first
   n.ntype=0;
   n.nstat=1;
   ret=GMS_getName(&n,szName,MAX_STR,FIND_FIRST);
   if (ret!=GMS_SUCCESS)
      return ret;
   abuffer.push_back(szName);

   // Alternative name next
   n.ntype=0;
   n.nstat=2;
   ret=GMS_getName(&n,szName,MAX_STR,FIND_FIRST);
   if (ret!=GMS_SUCCESS && ret!=GMS_NO_DATA)
      return ret;
   if (ret!=GMS_NO_DATA)
      abuffer.push_back(szName);
   // Then the rest, ignoring Preferred and Alternative names
   n.ntype=0;
   n.nstat=0;
   ret=GMS_getName(&n,szName,MAX_STR,FIND_FIRST);
   if (ret!=GMS_SUCCESS)
      return ret;
   int secondPref=0,secondAbbr=0;
   while (ret==GMS_SUCCESS)// && (p-szBuffer) < nszBuffer)
   {
	   if ((n.nstat==1) && !secondPref) 
      {
         ret=GMS_getName(&n,szName,MAX_STR,FIND_NEXT);
         secondPref=1;
		   continue;
	   }
	   if ((n.nstat==2) && !secondAbbr) 
      {
         ret=GMS_getName(&n,szName,MAX_STR,FIND_NEXT);
         secondAbbr=1;
		   continue;
	   }
      abuffer.push_back(szName);
      ret=GMS_getName(&n,szName,MAX_STR,FIND_NEXT); 
   }
   // Put it Together
   if (abuffer.size())
   {
      UINT acount=abuffer.size()
         ,lenstr,lenDelim;
      UINT i=0,j;
      vector<char> buffer;
      if (szDelimeter)
         lenDelim=strlen(szDelimeter);
      else
         lenDelim=1;

      lenstr=0;
      for (i=0;i<acount;i++)
         lenstr+=abuffer[i].length()+lenDelim;
      lenstr+=1;

      //Merger here
      buffer.reserve(lenstr);
      for (i=0;i<acount;i++)
      {
         for (j=0;j<abuffer[i].length();j++)
            buffer.push_back(abuffer[i][j]);
         if (szDelimeter)
            for (j=0;j<lenDelim;j++)
               buffer.push_back(szDelimeter[j]);
         else
            buffer.push_back('\0');
      }
      //buffer.erase(buffer.length()-lenDelim,buffer.length()-1);
      //strncpy(szBuffer,buffer.c_str(),nszBuffer);
      for (i=0;(i<buffer.size()) && (i<(UINT)nszBuffer-1);i++)
         szBuffer[i]=buffer[i];
      szBuffer[i]=0;
   }
   else
      return GMS_NO_DATA;
   return GMS_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_standardName(CHAR* szInput, CHAR* szOutput,LONG nszOutput)
{
   LONG ret = standardName(szInput,szOutput,nszOutput);
   return ret;
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG standardName(CHAR* szInput, CHAR* szOutput,LONG nszOutput)
{
CHAR token[]="-.'[]+";
unsigned int i,len;
string input=szInput;

   PUSH(standardName);

   //1. Trim leading and trailing spaces, and capitalize all letter 
   //input=" "+input+" ";
   TRIM(input);
   TOUPPER(input);
   input+=" ";

   // L-N -> L^N only when there is exactly one '-' 
   //            and L is not preceeded by a space ' '
   if ((i=input.find_first_of('-'))==input.find_last_of('-') && i!=-1 
        && (i<=0 || isalpha(input[i-1])) 
        && (i>=input.length()-1 ||isdigit(input[i+1])) 
        && ((i>=2 && input[i-2]!=' ') || i<2) 
       )
       input.replace(i,1," ");

   i=1;
   while (i+1 < (len=input.length())){
      //2. L( and N( to L ( or N (
      if (input[i]=='(' && isalnum(input[i-1])){
         input.insert(i," ");

	  }
      //3. )L and )N to ) L  or ) N
      //else if (input[i]==')' && isalnum(input[i+1])){
      else if (input[i-1]==')' && isalnum(input[i])){
         input.insert(i+1," ");
         i+=2;
      }
      //4. Change L. to L^
      if (input[i]=='.' && isalpha(input[i-1])){
         input.replace(i,1," ");
	  }
      //5. Change LN to L^N except for SLN
      else if (isalpha(input[i-1]) && isdigit(input[i]) && (i<2 || input.substr(i-2,1).find_first_of(token)==-1)){
         input.insert(i,1,' ');
      }
      //6. Change NL to N^L except for SNL
      else if (isdigit(input[i-1]) && isalpha(input[i]) && (i<2 || input.substr(i-2,1).find_first_of(token)==-1)){
          input.insert(i,1,' ');   //if (i > 1) 
      }
      //7. Change LL-LL to LL^LL
      else if (input[i]=='-' && i>1 && i+2<len 
              && isalpha(input[i-1]) && isalpha(input[i+1])
              && isalpha(input[i-2]) && isalpha(input[i+2])
              ){
         input.replace(i++,1," ");
	   }
      else if (input[i]==' '){
         //8. Change ^0N to ^N
		  if ((i+2<len) && input[i+1]=='0' && isdigit(input[i+2]))
            input.erase(i+1,1);
         //9. Change ^^ to ^
		  else if (input[i+1]==' ')
            input.erase(i,1);
         //10. Change ^) to )
		  else if (input[i+1]==')')
            input.erase(i,1);
         //11. Change (^ to (
		  else if (input[i-1]=='(')
           input.erase(i,1);
         //12. Change ^/ to /
		  else if (input[i+1]=='/')
           input.erase(i,1);
         //13. Change /^ to /
		  else if (input[i-1]=='/')
           input.erase(i,1);
         else i++;
      }
	  else i++;
   }




   TRIMRIGHT(input);
   FillMemory(szOutput,nszOutput,0);
   strncpy(szOutput,(LPCTSTR)input.c_str(),min((LONG)nszOutput-1,(LONG)input.length()-1+1));
   if ((int)strlen(szOutput)<input.length()-1) 
      ret = GMS_MORE_DATA;
   POP();

}



LONG getNameRecord(BOOL fLocal,GMS_Name &name, LPSTR szName, LONG nszName)
{
#define SQL_GETNAMEREC "\
   select GID, NTYPE, NSTAT, NUID, NVAL, NLOCN, NDATE, NREF, NID \
   from NAMES \
   where NID = ? AND NSTAT <> 9"

   PUSH(getNameRec);

static CODBCbindedStmt &central= _centralDBC->BindedStmt(SQL_GETNAMEREC);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_GETNAMEREC);
       CODBCbindedStmt *source;

static GMS_Name _data;
static CHAR _nval[GMS_NVAL_SIZE+1],_next[MAX_STR];
static BOOL first_time=TRUE;

//Parameters
static LONG nid;

      if (first_time)
      {
 	      BIND(1,_data.gid);
         BIND(2,_data.ntype);
         BIND(3,_data.nstat);
         BIND(4,_data.nuid);
         BINDS(5,_nval,GMS_NVAL_SIZE+1);
         BIND(6,_data.nlocn);
         BIND(7,_data.ndate);
         BIND(8,_data.nref);
		 BIND(9,_data.nid);
         BINDPARAM(1,nid);


         first_time = FALSE;
      }
   
      nid=name.nid;

   source = (fLocal)?&local:&central;
   source->Execute();

   _next[0]='\0';
   if (source->Fetch())
   {
      name = _data;
      string namestr;
      namestr=_nval;
      namestr+=_next;
      strncpy(szName,namestr.c_str(),nszName);
      szName[nszName-1]='\0';
	  ret = GMS_SUCCESS;
   }
   else if (source->NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;

   POP();

#undef SQL_GETNAMEREC
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_getNameRecord(GMS_Name *data, LPSTR szName, LONG nszName)
{
   static BOOL fLocal=TRUE;

   PUSH(GMS_getNameRecord);
   LONG  nid=0,to=0, gid=0, nstat=0;


   nid = data->nid;
   if (nid<0)
   {
      ret=getNameRecord(LOCAL,*data,szName,nszName);
   }
   else
      ret=getNameRecord(CENTRAL,*data,szName,nszName);
   if (ret == GMS_SUCCESS)
   {
      nid = data->nid;
	  nstat= data->nstat;
	  getNameFieldFromChanges(nid,"NSTAT",nstat);
      if (nstat == 9) {
		  ret = GMS_NO_DATA;
		  ZeroMemory(szName, sizeof(szName));
	  }
	  else {
	      getNameFieldFromChanges(nid,"NTYPE",data->ntype);
	      getNameFieldFromChanges(nid,"NUID",data->nuid);
	      getNameFieldFromChanges(nid,"NLOCN",data->nlocn);
	      getNameFieldFromChanges(nid,"NDATE",data->ndate);
	      getNameFieldFromChanges(nid,"NREF",data->nref);
          ret = GMS_SUCCESS;
	  }

   }
   
   POP();

}



//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_getSoundEx(CHAR* szInput, CHAR* szOutput,LONG nszOutput)
{
   LONG ret = SoundEx(szInput,szOutput,nszOutput);
   return ret;
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG SoundEx(CHAR* szInput, CHAR* szOutput,LONG nszOutput)
{
	string input=szInput,buff,firstLetter,output;
	string szFind,szReplace;

	long i,len;
    TRIM(input);
    TOUPPER(input);
//	szFind = "  ";
//	szReplace = " ";
//	replaceString(input,&output,szFind,szReplace,2);

	szFind = " ";
	szReplace = "\0";
	replaceString(input,&output,szFind,szReplace,1);

    input = output;


    szFind= "CHIANUNG";
	szReplace = "CIANUNG";
	replaceString(input,&output,szFind,szReplace,8);
    input = output;

    szFind= "TAICHUNG";
	szReplace = "TAICUNG";
	replaceString(input,&output,szFind,szReplace,8);
    input = output;

    spellAbbr(&input,"BBT","BLUEBONNET",3);
    spellAbbr(&input,"CH","CIANUNG",3);
    spellAbbr(&input,"CP","CENTURYPATNA",3);
    spellAbbr(&input,"DGWD","DEE-GEO-WEO-GEN",3);
    spellAbbr(&input,"HBJ","HAIBIGANJ",3);
    spellAbbr(&input,"IGT","I-GEO-TZE",3);
    spellAbbr(&input,"KDM","KHAODAWKMALI",3);
    spellAbbr(&input,"LMN","LEBMUENAHNG",3);
    spellAbbr(&input,"PBMN","PATEBLANCMN",3);
    spellAbbr(&input,"MN","MUEYNAWNG",3);
    spellAbbr(&input,"NSPT","NIAWSANPAHTAWNG",3);
    spellAbbr(&input,"TN","TAICHUNGNATIVE",3);
    spellAbbr(&input,"WC","WARRANGALCULTURE",3);

    firstLetter = input.substr(0,1);
	buff=input.substr(1,input.length());


	replaceString(buff,&output,"A","~",1);
    buff = output;
	replaceString(buff,&output,"E","~",1);
    buff = output;
	replaceString(buff,&output,"I","~",1);
    buff = output;
	replaceString(buff,&output,"O","~",1);
    buff = output;
	replaceString(buff,&output,"U","~",1);
    buff = output;
	replaceString(buff,&output,"W","~",1);
    buff = output;
   	replaceString(buff,&output,"H","~",1);
    buff = output;
	replaceString(buff,&output,"Y","~",1);
    buff = output;

    szFind="~";
    szReplace="\0";
    replaceString(buff,&output,szFind,szReplace,1);
    buff = output;

	replaceString(buff,&output,"V","B",1);
    buff = output;
	replaceString(buff,&output,"P","F",1);
    buff = output;
	replaceString(buff,&output,"K","C",1);
    buff = output;
	replaceString(buff,&output,"Q","C",1);
    buff = output;
	replaceString(buff,&output,"Z","S",1);
    buff = output;
	replaceString(buff,&output,"X","S",1);
    buff = output;
	replaceString(buff,&output,"J","G",1);
    buff = output;
	replaceString(buff,&output,"T","D",1);
    buff = output;
	replaceString(buff,&output,"N","M",1);
    buff = output;

	replaceString(buff,&output,"BB","B",2);
    buff = output;
	replaceString(buff,&output,"FF","F",2);
    buff = output;
	replaceString(buff,&output,"CC","C",2);
    buff = output;
	replaceString(buff,&output,"SS","S",2);
    buff = output;
	replaceString(buff,&output,"GG","G",2);
    buff = output;
	replaceString(buff,&output,"DD","D",2);
    buff = output;
	replaceString(buff,&output,"LL","L",2);
    buff = output;
	replaceString(buff,&output,"MM","M",2);
    buff = output;
	replaceString(buff,&output,"RR","R",2);
    buff = output;


    firstLetter.append(buff);
	if (strlen(firstLetter.c_str()) < nszOutput)
	   strncpy(szOutput, (char *) firstLetter.c_str(),strlen(firstLetter.c_str()));
	else
	   strncpy(szOutput, (char *) firstLetter.c_str(),nszOutput);
    return GMS_SUCCESS;
}
