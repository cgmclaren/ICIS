/* User.cpp : Implements the functions to access and manipulate the USERS table of ICIS
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

#include "user.h"

extern LONG UACCESS,USERID,USTATUS;
extern CODBCdbc *_centralDBC,*_localDBC;  //Central and Local Database Connection
extern stack<char*> _routines_stack;

#if defined(WEB)
extern bool WEBSERVICE;  //True if calls are to Web Service
extern _bstr_t szURL;    // URL pointing to the WSDL of the Web Service
int uRcdNo; // Holds the record number to be fetched
//MethodBean *mb;  // Global container for the returned bean structure from the Web Service
int uidx=0;
GMS_User *beanUser;
typedef char char_255[255];

BOOL __stdcall BufferUser(L_User *InStruct,int sze)
{   int i; 
    beanUser = (GMS_User *)calloc( sze, sizeof(GMS_User));
	for (int i=0;i<sze;i++) {
		beanUser[i].adate = InStruct[i].adate;
		beanUser[i].cdate = InStruct[i].cdate;
		beanUser[i].instalid = InStruct[i].instalid;
		beanUser[i].personid = InStruct[i].personid;
		beanUser[i].uaccess = InStruct[i].uaccess;
        strncpy((char *)beanUser[i].uname,InStruct[i].uname,GMS_UNAME_SIZE);
        strncpy((char *)beanUser[i].upswd,InStruct[i].upswd,GMS_UPSWD_SIZE);
        beanUser[i].userid = InStruct[i].userid;
		beanUser[i].ustatus = InStruct[i].ustatus;
		beanUser[i].utype = InStruct[i].utype;
	}
	uidx = sze;


	return (true);
}


#endif //defined(WEB)


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG getNextUserID(void)
{
static LONG userID=0;

   userID=0;
//   if (!listID)
//   {
      CODBCdirectStmt local = _localDBC->DirectStmt("SELECT MIN(USERID) FROM USERS");
      local.Execute();
      local.Fetch();
      userID=local.Field(1).AsInteger();
      if (userID>0) userID=0;
//   }
   return --userID;
}

LONG DLL_INTERFACE GMS_getCurrentUser()
{
    return USERID;
}

LONG DLL_INTERFACE GMS_getUser(GMS_User *data, LONG fSearchOption)
{
   static BOOL fLocal=TRUE;

   PUSH(GMS_getUser);
   
   static GMS_User _data;

#if defined(WEB)
   IManagedInterface *cpi = NULL; 
   long userid = data->userid;
#endif


   if (fSearchOption==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal)
   {
      _data=*data;
      ret=getUser(LOCAL,*data,fSearchOption);
      if (ret!=GMS_SUCCESS)
      {
         fLocal = FALSE;
         *data=_data;

#if defined(WEB)
		if(WEBSERVICE==true)
			{
				uRcdNo = 0;
				fLocal = FALSE; 
				int str;
				_bstr_t szName;
				strcpy(szName, data->uname);

				//Enter Single Threaded Apartment (STA) - STA Thread
				CoInitialize(NULL);

				//Instantiate the COM object in the appropriate apartment
				HRESULT hr = CoCreateInstance(CLSID_LibraryImplementation,
					NULL, CLSCTX_INPROC_SERVER, IID_IManagedInterface, reinterpret_cast<void**>(&cpi)); 
				if(FAILED(hr))
				{
					MessageBox(NULL,"Could not create Instance, Web Service Failed!","ICIS V5",MB_OK);
				}
				str = cpi->LgetUser(userid, szName, szURL);

				if(str == -1)
				{
					MessageBox(NULL,"WebService Connection Lost. Unable to retrieve data","ICIS V5",MB_OK);
					return(GMS_ERROR);
				}

				if(str != 0)
					userid = beanUser[uRcdNo].userid;
  					data->adate = beanUser[uRcdNo].adate;
                    data->cdate = beanUser[uRcdNo].cdate;
					data->instalid = beanUser[uRcdNo].instalid;
					data->personid = beanUser[uRcdNo].personid;
					data->uaccess = beanUser[uRcdNo].uaccess;
                    strncpy((char *)data->uname, beanUser[uRcdNo].uname,GMS_UNAME_SIZE);
                    strncpy((char *)data->upswd, beanUser[uRcdNo].upswd, GMS_UPSWD_SIZE);
					data->userid = beanUser[uRcdNo].userid;
					data->ustatus = beanUser[uRcdNo].ustatus;
					data->utype = beanUser[uRcdNo].utype;
				    uRcdNo = uRcdNo + 1;
					ret = GMS_SUCCESS;
				if (userid != 0) {
  				    uRcdNo = 0; uidx=0;
				    CoUninitialize();
				}
			}
			else  
#endif

		 ret=getUser(CENTRAL,*data,FIND_FIRST);
      }
   }
   else

#if defined(WEB)
 		if(WEBSERVICE==true)
		{
			if(uRcdNo < uidx)
			{
				   // Mapping the C# Library data to the C data structure
					userid = beanUser[uRcdNo].userid;
  					data->adate = beanUser[uRcdNo].adate;
                    data->cdate = beanUser[uRcdNo].cdate;
					data->instalid = beanUser[uRcdNo].instalid;
					data->personid = beanUser[uRcdNo].personid;
					data->uaccess = beanUser[uRcdNo].uaccess;
                    strncpy((char *)data->uname, beanUser[uRcdNo].uname,GMS_UNAME_SIZE);
                    strncpy((char *)data->upswd, beanUser[uRcdNo].upswd, GMS_UPSWD_SIZE);
					data->userid = beanUser[uRcdNo].userid;
					data->ustatus = beanUser[uRcdNo].ustatus;
					data->utype = beanUser[uRcdNo].utype;
				    uRcdNo = uRcdNo + 1;
					ret = GMS_SUCCESS;

			}
			else{
				ret = GMS_NO_DATA;
				uRcdNo = 0; uidx=0;
				CoUninitialize();
			}
		} 
		else 
#endif //defined(WEB)

	   ret=getUser(CENTRAL,*data,fSearchOption);

   POP();

}

LONG getUser(BOOL fLocal, GMS_User &user, LONG fSearchOption)
{
#define SQL_GETUSER "select USERID,INSTALID,USTATUS,UACCESS,UTYPE,UNAME,\
   UPSWD,PERSONID,ADATE,CDATE \
   from USERS \
   where (USERID=?) or (0=? and UNAME=?)"

/*#define SQL_GETUSER "select userid,instalid,ustatus,uaccess,utype,uname,\
   upswd,personid,adate,cdate \
   from users \
   where (userid=?) or (0=? and uname=?)"
*/
   PUSH(getUser);
   LOG(fLocal?"LOCAL\n":"CENTRAL\n");

static CODBCbindedStmt &central= _centralDBC->BindedStmt(SQL_GETUSER);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_GETUSER);
       CODBCbindedStmt *source;

static GMS_User _data;
static BOOL first_time=TRUE;
//Parameters
static LONG userid;
static CHAR uname[GMS_UNAME_SIZE];
   source = (fLocal)?&local:&central;
   ZeroMemory(&_data,sizeof(_data));
   ZeroFill(uname,sizeof(uname));
   strncpy(uname,user.uname,sizeof(user.uname));

   if (fSearchOption==FIND_FIRST)
   {
//     if (first_time)
 //     {
         BIND(1,_data.userid    );
         BIND(2,_data.instalid  );
         BIND(3,_data.ustatus   );
         BIND(4,_data.uaccess   );
         BIND(5,_data.utype     );
         BINDS(6,_data.uname,sizeof(_data.uname));
         BINDS(7,_data.upswd,sizeof(_data.upswd));
         BIND(8,_data.personid  );
         BIND(9,_data.adate     );
         BIND(10,_data.cdate     );
         BINDPARAM(1,userid);
         BINDPARAM(2,userid);
         BINDPARAMS(3,uname,GMS_UNAME_ASIZE);
         first_time = FALSE;

 //     }
      //Set Parameters
      userid=user.userid;
	  local.Execute();
	  central.Execute();
//      source->Execute();
   }


   if (source->Fetch())
      user = _data;
   else if (source->NoData())
      ret = GMS_NO_DATA;
   else 
      ret = GMS_ERROR;

   POP();
}



//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG addUser(GMS_User &data)
{
#define SQL_ADDUSER "\
   INSERT INTO USERS ( USERID,  INSTALID, USTATUS, UACCESS, UTYPE, UNAME, \
    UPSWD, PERSONID, ADATE, CDATE ) \
    values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"

   PUSH(addUser);
static CODBCbindedStmt &local  = _localDBC->BindedStmt(SQL_ADDUSER);

static GMS_User _data;
static BOOL first_time=TRUE;

   if (first_time)
   {
	  local.BindParam(1,_data.userid);
      local.BindParam(2,_data.instalid);
	  local.BindParam(3,_data.ustatus);
	  local.BindParam(4,_data.uaccess);
	  local.BindParam(5,_data.utype);
	  local.BindParam(6,_data.uname, sizeof(_data.uname));
	  local.BindParam(7,_data.upswd, sizeof(_data.upswd));
	  local.BindParam(8,_data.personid);
	  local.BindParam(9,_data.adate);
	  local.BindParam(10,_data.cdate);
      first_time=FALSE;
   }
   
   data.userid = getNextUserID();

   _data=data;

   if (local.Execute())
      ret=GMS_SUCCESS;   
   else 
      ret=GMS_ERROR;
   POP();

#undef SQL_ADDUSER
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_addUser(GMS_User *data, LPCSTR ename, long security)
{
	PUSH(GMS_addUser);
	char szName[30], szEncrypt[36];
	//user to be added should not have priviledge greater than the current user adding him
	if (data->uaccess > UACCESS)  POP2(GMS_ERROR);   
	
	//Only user with access >= 80 can add a user
	if ((security==1) && (UACCESS>=80) && (USTATUS <2))
		ret = addUser(*data);
	//if security is greater than 1, then encryption is applied but can only be done by user with privilege = 150
    else if ( (security > 1) && ( ((UACCESS >=100) && (USTATUS ==2)) || (UACCESS ==150))  ) {
  	    strncpy((char *)szName, (char *) data->upswd,sizeof(data->upswd));
        ret = GMS_encryptPWD(szName,szEncrypt);
		if (ret == GMS_SUCCESS) {
  	       strcpy((char *) data->upswd,(char *)szEncrypt);
 		   ret = addUser(*data);
  		   if ((ret == GMS_SUCCESS) && (security==3) && (UACCESS==150)) {
              ZeroMemory(szName,sizeof(szName)); 
              ZeroMemory(szEncrypt,sizeof(szEncrypt)); 
      	      strncpy((char *)szName, (char *) data->uname,sizeof(data->uname));
              if ( GMS_encryptPWD(szName,szEncrypt) ==1) 
                   strncpy((char *) ename,szEncrypt,sizeof(szEncrypt));
		   }
		}
	}
	else ret= GMS_NO_ACCESS;
	POP();
}
