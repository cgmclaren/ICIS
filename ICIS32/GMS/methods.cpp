/* Methods.cpp : Implements the functions to access and manipulate the METHODS table of ICIS
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

#include "methods.h"

extern LONG UACCESS,USERID;
extern CODBCdbc *_centralDBC,*_localDBC;  //Central and Local Database Connection
extern stack<char*> _routines_stack;

#if defined(WEB)
extern bool WEBSERVICE;  //True if calls are to Web Service
extern _bstr_t szURL;    // URL pointing to the WSDL of the Web Service
int mRcdNo; // Holds the record number to be fetched
//MethodBean *mb;  // Global container for the returned bean structure from the Web Service
int midx=0;
GMS_Method *beanMethod;
typedef char char_255[255];
char_255 *beanDesc;

BOOL __stdcall BufferMethod(MethodBean *InStruct,int sze)
{   int i; 
    beanMethod = (GMS_Method *)calloc( sze, sizeof(GMS_Method));
//	beanDesc = (char_255 *) calloc( sze, sizeof(char_255));
//	mb = new MethodBean[sze];
//	mb = NULL;
	for (int i=0;i<sze;i++) {
		beanMethod[i].mid = InStruct[i].mid;
        strncpy((char *)beanMethod[i].mtype,InStruct[i].mtype,4);
		strncpy((char *)beanMethod[i].mgrp,InStruct[i].mgrp,4);
		strncpy((char *)beanMethod[i].mcode,InStruct[i].mcode,12);
		strncpy((char *)beanMethod[i].mname,InStruct[i].mname,52);
		beanMethod[i].mref= InStruct[i].mref;
		beanMethod[i].mprgn= InStruct[i].mprgn;
		beanMethod[i].mfprg= InStruct[i].mfprg;
		beanMethod[i].mattr= InStruct[i].mattr;
		beanMethod[i].geneq= InStruct[i].geneq;
		beanMethod[i].muid= InStruct[i].muid;
		beanMethod[i].lmid= InStruct[i].lmid;
		beanMethod[i].mdate= InStruct[i].mdate;
// 	    strncpy(beanDesc[i],InStruct[i].mdesc,255);
	}
//	mb = InStruct;
	midx = sze;


	return (true);
}


#endif //defined(WEB)

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_getMethod(GMS_Method *data, LPSTR szDesc, LONG nszDesc
               , LONG fSearchOption)
{
   static BOOL fLocal=TRUE;
   long methid;
   CHAR *szName;
#if defined(WEB)
   IManagedInterface *cpi = NULL; 
#endif
   PUSH(GMS_getMethod);

   if (fSearchOption==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal){
      ret=getMethod(LOCAL,*data,szDesc,nszDesc,fSearchOption);
      if (ret!=GMS_SUCCESS){
         fLocal = FALSE; 
#if defined(WEB)
		if(WEBSERVICE==true)
			{
				mRcdNo = 0;
				fLocal = FALSE; 
				int str;
				//Enter Single Threaded Apartment (STA) - STA Thread
				CoInitialize(NULL);

//MessageBox(NULL, "CoCreateInstance of the Web service","getMethod",MB_OK);
				//Instantiate the COM object in the appropriate apartment
				HRESULT hr = CoCreateInstance(CLSID_LibraryImplementation,
					NULL, CLSCTX_INPROC_SERVER, IID_IManagedInterface, reinterpret_cast<void**>(&cpi)); 

				if(FAILED(hr))
				{
					MessageBox(NULL,"Could not create Instance, Web Service Failed!","ICIS V5",MB_OK);
				}
                methid = data->mid;
				str = cpi->LgetMethod(methid,szURL);

				if(str == -1)
				{
					MessageBox(NULL,"WebService Connection Lost. Unable to retrieve data","ICIS V5",MB_OK);
					return(GMS_ERROR);
				}

				if(str != 0) data->mid =  beanMethod[mRcdNo].mid; 

//MessageBox(NULL,"getMethod is initialized in ICIS32.DLL!","ICIS V5",MB_OK);

//char tbuffer[65];
//_itoa( mRcdNo, tbuffer, 10 );
//MessageBox(NULL, tbuffer,"Record No",MB_OK);
					strcpy(data->mtype, beanMethod[mRcdNo].mtype);
					strcpy(data->mgrp, beanMethod[mRcdNo].mgrp);
					strcpy(data->mcode, beanMethod[mRcdNo].mcode);
					strcpy(data->mname, beanMethod[mRcdNo].mname);
					data->mref = beanMethod[mRcdNo].mref;
					data->mprgn = beanMethod[mRcdNo].mprgn;
					data->mfprg = beanMethod[mRcdNo].mfprg;
					data->mattr = beanMethod[mRcdNo].mattr;
					data->geneq = beanMethod[mRcdNo].geneq;
					data->muid = beanMethod[mRcdNo].muid;
					data->lmid = beanMethod[mRcdNo].lmid;
					data->mdate = beanMethod[mRcdNo].mdate;
				    mRcdNo = mRcdNo + 1;
					ret = GMS_SUCCESS;
//MessageBox(NULL,beanDesc[mRcdNo],"ICIS V5",MB_OK);
//  					strncpy(szDesc,beanDesc[mRcdNo],255);

//MessageBox(NULL,"getMethod first record!","ICIS V5",MB_OK);
				if (methid != 0) {
  				    mRcdNo = 0; midx=0;
				    CoUninitialize();
				}

			}
			else  
#endif
		 ret=getMethod(CENTRAL,*data,szDesc,nszDesc,FIND_NEXT);
      }
   }
   else

#if defined(WEB)
 		if(WEBSERVICE==true)
		{
			if(mRcdNo < midx)
			{
				// Mapping the C# Library data to the C data structure
					strcpy(data->mtype, beanMethod[mRcdNo].mtype);
					strcpy(data->mgrp, beanMethod[mRcdNo].mgrp);
					strcpy(data->mcode, beanMethod[mRcdNo].mcode);
					strcpy(data->mname, beanMethod[mRcdNo].mname);
					data->mref = beanMethod[mRcdNo].mref;
					data->mprgn = beanMethod[mRcdNo].mprgn;
					data->mfprg = beanMethod[mRcdNo].mfprg;
					data->mattr = beanMethod[mRcdNo].mattr;
					data->geneq = beanMethod[mRcdNo].geneq;
					data->muid = beanMethod[mRcdNo].muid;
					data->lmid = beanMethod[mRcdNo].lmid;
					data->mdate = beanMethod[mRcdNo].mdate;
 				    mRcdNo = mRcdNo + 1;
// 					strncpy(szDesc,beanDesc[mRcdNo],255);

					ret = GMS_SUCCESS;
					if (mRcdNo > 287 )
						MessageBox(NULL,"getMethod","ICIS V5",MB_OK);

//				if((data->mid == 0) || (data->mid > 1000000))
//					ret = GMS_NO_DATA;
			}
			else{
				ret = GMS_NO_DATA;
				mRcdNo = 15; midx=0;
				CoUninitialize();
//				cpi->Release();
			}
		} 
		else 
#endif //defined(WEB)
	        ret=getMethod(CENTRAL,*data,szDesc,nszDesc,fSearchOption);
   POP();
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG getMethod(BOOL fLocal, GMS_Method &data, CHAR *szDesc, LONG nszDesc
               , LONG fSearchOption)
{
#define SQL_METHOD "\
   select MID, MREF, MPRGN, MFPRG, MATTR\
     , GENEQ, MUID, LMID, MDATE\
     , MTYPE, MCODE, MNAME, MGRP\
     , MDESC\
   from METHODS\
   where 0=? or MID=?" 

   PUSH(getMethod);

static CODBCbindedStmt &central= _centralDBC->BindedStmt(SQL_METHOD);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_METHOD);
       CODBCbindedStmt *source;

static GMS_Method _data;
static BOOL first_time=TRUE;

static LONG mid;
static CHAR _szDesc[0xFFF];

   if (fSearchOption==FIND_FIRST){
      if (first_time)
      {
         BIND(1,_data.mid);
         BIND(2,_data.mref);
         BIND(3,_data.mprgn);
         BIND(4,_data.mfprg);
         BIND(5,_data.mattr);
         BIND(6,_data.geneq);
         BIND(7,_data.muid);
         BIND(8,_data.lmid);
         BIND(9,_data.mdate);
         BINDS(10,_data.mtype,GMS_MTYPE_SIZE);
         BINDS(11,_data.mcode,GMS_MCODE_SIZE);
         BINDS(12,_data.mname,GMS_MNAME_SIZE);
         BINDS(13,_data.mgrp,GMS_MGRP_SIZE);
         BINDS(14,_szDesc,0xFFF);
	      BINDPARAM(1,mid);
	      BINDPARAM(2,mid);
 
 
         first_time=FALSE;
      }

	   mid=data.mid;
      local.Execute();
      central.Execute();
   }
   
   source = (fLocal)?&local:&central;

   ZeroMemory(&_data,sizeof(_data));
   if (source->Fetch())
   {
      data = _data;
      if (szDesc)
      {
         strncpy(szDesc,_szDesc,nszDesc);
         //strncpy(szDesc,source->Field(14).AsString(),nszDesc);
         szDesc[nszDesc-1]='\0';
         //PadRight(szDesc,nszDesc);
      }
      //PadRight(data.mtype,GMS_MTYPE_SIZE);
      //PadRight(data.mcode,GMS_MCODE_SIZE);
      //PadRight(data.mname,GMS_MNAME_SIZE);
      //PadRight(data.mgrp,GMS_MGRP_SIZE);
   }
   else if (source->NoData())
      ret=GMS_NO_DATA;
   else 
      ret=GMS_ERROR;

   POP();

#undef SQL_METHOD
}
