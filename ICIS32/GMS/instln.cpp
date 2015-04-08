/* INSTLN.cpp : Implements the functions to access and manipulate the INSTLN table of ICIS
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

#include "instln.h"

extern LONG UACCESS,USERID,USTATUS, INSTALID;
extern bool GMSCLOSED;
extern CODBCdbc *_centralDBC,*_localDBC;  //Central and Local Database Connection
extern stack<char*> _routines_stack;


#if defined(WEB)
extern bool WEBSERVICE;         //True if calls are to Web Service
extern _bstr_t szURL;           // URL pointing to the WSDL of the Web Service
int iRcdNo;                     // Holds the record number to be fetched
int iidx=0;
GMS_Installation *beanInstln;   //Global container for the installation records retrieved
typedef char char_255[255];
char_255 *beanIDesc;

BOOL __stdcall BufferInstln(L_Installation *InStruct,int sze)
{   int i; 
    beanInstln = (GMS_Installation *)calloc( sze, sizeof(GMS_Installation));
	beanIDesc = (char_255 *) calloc( sze, sizeof(char_255));
	for (int i=0;i<sze;i++) {
		beanInstln[i].instalid = InStruct[i].instalid;
		beanInstln[i].admin = InStruct[i].admin;
		beanInstln[i].udate = InStruct[i].udate;
		beanInstln[i].ugid = InStruct[i].ugid;
		beanInstln[i].ulocn = InStruct[i].ulocn;
		beanInstln[i].umethn = InStruct[i].umethn;
		beanInstln[i].ufldno = InStruct[i].ufldno;
		beanInstln[i].urefno = InStruct[i].urefno;
		beanInstln[i].upid = InStruct[i].upid;
        strncpy((char *)beanIDesc[i],InStruct[i].desc,strlen(InStruct[i].desc));



	}
	iidx = sze;


	return (true);
}


#endif //defined(WEB)


LONG DLL_INTERFACE GMS_getInstallation(GMS_Installation *data,LPSTR szDesc
               ,LONG nszDesc)
{
   static BOOL fLocal=TRUE;

   //MessageBox(NULL,"GMS getInstallation 1","Test",MB_OK);
   PUSH(GMS_getInstallation);
   LOGF("INSTALID = %d\n",data->instalid);

   if (data->instalid==0 || data->instalid==-1)
      ret= getInstallation(LOCAL, *data, szDesc,nszDesc);
   else
#if defined(WEB)
	   if (WEBSERVICE == true) {
		   IManagedInterface *cpi = NULL; 
		   L_Installation linstln;
		   CoInitialize(NULL);
		   HRESULT hr = CoCreateInstance(CLSID_LibraryImplementation,
					NULL, CLSCTX_INPROC_SERVER, IID_IManagedInterface, reinterpret_cast<void**>(&cpi)); 
		   if (FAILED(hr))
		   {
			   MessageBox(NULL,"Could not create Instance, Web Service Failed!","ICIS V5",MB_OK);
		   }
		   ZeroMemory(&linstln,sizeof(linstln));
 		   linstln.instalid = data->instalid;
           ret = cpi->LgetInstallation(&linstln,FIND_FIRST,szURL);
           CoUninitialize();
       
	   }
	   else
#endif
   //MessageBox(NULL,"GMS getInstallation  2","Test",MB_OK);
      ret= getInstallation(CENTRAL, *data, szDesc,nszDesc);
   POP();
}


LONG getInstallation(BOOL fLocal,GMS_Installation &instln,LPSTR szDesc,LONG nszDesc)
{
#define SQL_INSTLN "select INSTALID, ADMIN, UDATE, UGID, \
   ULOCN, UMETHN, UFLDNO, UREFNO, UPID,IDESC\
   from INSTLN \
   where (0=? or INSTALID=?)"
/*#define SQL_INSTLN "select instalid, admin, udate, ugid, \
   ulocn, umethn, ufldno, urefno, upid,idesc\
   from instln \
   where (0=? or instalid=?)"
*/
   PUSH(getInstallation);
   LOG(fLocal?"LOCAL\n":"CENTRAL\n");
   //MessageBox(NULL,"GMS getInstallation a","Test",MB_OK);
static CODBCbindedStmt &central= _centralDBC->BindedStmt(SQL_INSTLN);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_INSTLN);
       CODBCbindedStmt *source;

static GMS_Installation _data;
static BOOL first_time=TRUE;
static char _szDesc[MAX_STR];
static LONG id;


   ZeroMemory(_szDesc, MAX_STR);
   if (GMSCLOSED) {
	   first_time=TRUE;
	   central = _centralDBC->BindedStmt(SQL_INSTLN);
       local  = _localDBC->BindedStmt(SQL_INSTLN);
   }
   if (first_time)
   {
      //MessageBox(NULL,"GMS getInstallation b","Test",MB_OK);
      BIND(1,_data.instalid);
      BIND(2,_data.admin);
      BIND(3,_data.udate);
      //MessageBox(NULL,"GMS getInstallation b","Test",MB_OK);
	  BIND(4,_data.ugid);
      BIND(5,_data.ulocn);
      BIND(6,_data.umethn);
      BIND(7,_data.ufldno);
      BIND(8,_data.urefno);
      BIND(9,_data.upid);
      BINDS(10,_szDesc,MAX_STR);
      //MessageBox(NULL,"GMS getInstallation c","Test",MB_OK);
      BINDPARAM(1,id);
      BINDPARAM(2,id);

      first_time=FALSE;
   }
   //MessageBox(NULL,"GMS getInstallation d","Test",MB_OK);
   id=instln.instalid; 
   source = (fLocal)?&local:&central;

   //if (szDesc) {
   //   if (nszDesc < MAX_STR) 
   ////      source->Bind(10,szDesc,nszDesc);
   //      BINDS(10,szDesc,nszDesc)
	  //else
   ////      source->Bind(10,szDesc,MAX_STR);
   //      BINDS(10,_szDesc,MAX_STR);
   //} 
  if (source->Execute() && source->Fetch())
   {
	   //MessageBox(NULL,"GMS getInstallation e","Test",MB_OK);
	   if (szDesc) {
			strncpy(szDesc,_szDesc, nszDesc);
			szDesc[nszDesc-1]='\0';
	   }
      instln = _data;
   }
  else if (source->NoData()){
      //MessageBox(NULL,"GMS getInstallation f","Test",MB_OK);
      ret = GMS_NO_DATA;
  }
   else 
      ret = GMS_ERROR;
   //MessageBox(NULL,SQL_INSTLN,"Test",MB_OK);
   POP();
   return ret;
}

LONG DLL_INTERFACE GMS_getInstallationEx(GMS_Installation *data,LPSTR szDesc
               ,LONG nszDesc,LONG fSearchOption)
{
   static BOOL fLocal=TRUE;

   PUSH(GMS_getInstallationEx);

   if (fSearchOption==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal){
      ret=getInstallationEx(LOCAL,*data,szDesc,nszDesc,fSearchOption);
      if (ret!=GMS_SUCCESS){
         fLocal = FALSE; 
         ret=getInstallationEx(CENTRAL,*data,szDesc,nszDesc,FIND_NEXT);
      }
   }
   else
      ret=getInstallationEx(CENTRAL,*data,szDesc,nszDesc,fSearchOption);
   POP();
}




LONG getInstallationEx(BOOL fLocal,GMS_Installation &instln,LPSTR szDesc
               ,LONG nszDesc,LONG fSearchOption)
{
/*#define SQL_INSTLNEX "select INSTALID, ADMIN, UDATE, UGID, \
   ULOCN, UMETHN, UFLDNO, UREFNO, UPID,IDESC\
   from INSTLN \
   where (0=? or INSTALID=?)"
   */
#define SQL_INSTLNEX "select instalid, admin, udate, ugid, \
   ulocn, umethn, ufldno, urefno, upid,idesc\
   from instln \
   where (0=? or instalid=?)"

   PUSH(getInstallationEx);

static CODBCbindedStmt &central= _centralDBC->BindedStmt(SQL_INSTLNEX);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_INSTLNEX);
       CODBCbindedStmt *source;

static GMS_Installation _data;
static BOOL first_time=TRUE;

static LONG id;

   if (fSearchOption==FIND_FIRST)
   {
      if (first_time)
      {
         BIND(1,_data.instalid);
         BIND(2,_data.admin);
         BIND(3,_data.udate);
         BIND(4,_data.ugid);
         BIND(5,_data.ulocn);
         BIND(6,_data.umethn);
         BIND(7,_data.ufldno);
         BIND(8,_data.urefno);
         BIND(9,_data.upid);
         BINDPARAM(1,id);
         BINDPARAM(2,id);
 
         first_time=FALSE;
      }
      if (szDesc)
         BINDS(10,szDesc,nszDesc);
	   id=instln.instalid;
      local.Execute();
      central.Execute();
   }

   source = (fLocal)?&local:&central;

   if (source->Fetch())
   {
      instln = _data;
      if (szDesc)
         szDesc[nszDesc-1]='\0';
   }
   else if (source->NoData())
      ret = GMS_NO_DATA;
   else 
      ret = GMS_ERROR;

   POP();
   return ret;
}


LONG DLL_INTERFACE GMS_encryptPWD(LPCSTR szPwd, LPCSTR szEncrypt)
{
  PUSH(GMS_ecnryptPWD)
  if (((UACCESS >=100) && (USTATUS ==2)) || (UACCESS ==150))
	  ret = encryptPWD(szPwd, szEncrypt);
  else ret = GMS_ERROR;
  POP()
}



//////////////////////////////////////////////////////////////////////////////
//getGMSStatus - called by DMS_getDMSStatus
//Input: Intallation ID
//Output: the value of DMSStatus of the INSTALN table
//         1 - if data is being loaded in DMS
//Date Created: January 24, 2006
//Programmed by AMP
//Modification:
//////////////////////////////////////////////////////////////////////////////

LONG getDMSStatus(BOOL fLocal,LONG instid,LONG *dmsstat)
{
#define SQL_DMSSTATUS "select DMS_STATUS\
   from INSTLN \
   where (0=? or INSTALID=?)"

   PUSH(getDMSStatus);
   LOG(fLocal?"LOCAL\n":"CENTRAL\n");

static CODBCbindedStmt &central= _centralDBC->BindedStmt(SQL_DMSSTATUS);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_DMSSTATUS);
       CODBCbindedStmt *source;

static BOOL first_time=TRUE;

static LONG id, _dmsstat;

   if (first_time)
   {
      BIND(1,_dmsstat);

      BINDPARAM(1,id);
      BINDPARAM(2,id);

      first_time=FALSE;
   }
   id=instid; 

   source = (fLocal)?&local:&central;

   if (source->Execute() && source->Fetch())
      *dmsstat = _dmsstat;
   else if (source->NoData())
      ret = GMS_NO_DATA;
   else 
      ret = GMS_ERROR;

   POP();
   return ret;
#undef SQL_DMSSTATUS
}



//////////////////////////////////////////////////////////////////////////////
//GMS_getDMSStatus 
//Input: Intallation ID
//Output: the value of DMSStatus of the INSTALN table
//         1 - if data is being loaded in DMS
//Date Created: January 24, 2006
//Programmed by AMP
//Modification:
//////////////////////////////////////////////////////////////////////////////


LONG DLL_INTERFACE GMS_getDMSStatus(LONG *dmsstat)
{
   static BOOL fLocal=TRUE;

   PUSH(GMS_getDMSStatus);
      ret=getDMSStatus(LOCAL,INSTALID,dmsstat);
   POP();
}



//////////////////////////////////////////////////////////////////////////////
//setGMSStatus - called by DMS_setDMSStatus
//Input: Intallation ID, DMS_STATUS
//        updates the DMS_STATUS field of the local INSTALID table
//Date Created: January 24, 2006
//Programmed by AMP
//Modification:
//////////////////////////////////////////////////////////////////////////////

LONG setDMSStatus(BOOL fLocal,LONG instid,LONG dmsstat)
{
#define SQL_setDMSSTATUS "update INSTLN \
   set DMS_STATUS=? \
   where INSTALID=?"

   PUSH(setDMSStatus);
   LOG(fLocal?"LOCAL\n":"CENTRAL\n");

static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_setDMSSTATUS);
       CODBCbindedStmt *source;

static BOOL first_time=TRUE;

static LONG id, _dmsstat;

   if (first_time)
   {
      local.BindParam(1,_dmsstat);
      local.BindParam(2,id);

      first_time=FALSE;
   }
   id=instid; 
   _dmsstat = dmsstat;
   LOG("  setDMSStatus - binding of parameter \n");

   if (local.Execute()) {
      ret = GMS_SUCCESS;
      LOG("  execution is successful \n");
	  }
   else if (local.NoData()) {
      ret = GMS_NO_DATA;
      LOG("  no data for installation \n");
	  }
   else 
      ret = GMS_ERROR;

   POP();
   return ret;
#undef SQL_setDMSSTATUS
}



//////////////////////////////////////////////////////////////////////////////
//GMS_setDMSStatus 
//Input: Intallation ID, dms status 
//         1 - if data is being loaded in DMS
//Date Created: January 24, 2006
//Programmed by AMP
//Modification:
//////////////////////////////////////////////////////////////////////////////


LONG DLL_INTERFACE GMS_setDMSStatus(LONG dmsstat)
{
   static BOOL fLocal=TRUE;

   PUSH(GMS_setDMSStatus);
      ret=setDMSStatus(LOCAL,INSTALID,dmsstat);
   POP();
}

//////////////////////////////////////////////////////////////////////////////
//Date Created: Nov 15, 2007
//    Added by: WVC
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_getCurrentInstln(void)
{
   PUSH(GMS_getCurrentInstln);
      ret = INSTALID;
   POP();
}

