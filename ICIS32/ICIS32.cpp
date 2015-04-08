/* ICIS32.cpp : Defines the entry point for the DLL application.
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

/*******************************************************************************
 Programmed by   :   Arllet M. Portugal, Olan Casumpang
 Modification:
 Jan 24, 2006 (AMP)  -   INSTALID - a variable to store the local installation id
 *******************************************************************************/

#include "icis32.h"
#include "user.h"
#include "instln.h"


extern stack<char*> _routines_stack;

#if defined(WEB)
_bstr_t szURL;  //URL of the web service to connect to
#endif

//LONG ret;
LONG UACCESS,USERID, USTATUS, hasSID, INSTALID;
bool GMSCLOSED;
CODBCenv *_env;
CODBCdbc *_centralDBC=NULL,*_localDBC=NULL,*_centralDMS=NULL,*_localDMS=NULL;  //Central and Local Database Connection
CODBCdbc  *_centralGEMS=NULL;                                                    //, *_localGEMS=NULL  
CODBCdbc *_localIMS=NULL;    //*_centralIMS=NULL,
CODBCdbc *_localIPMS=NULL;  //Local Intellectual Property Management System (IPMS) Database Connection
bool WEBSERVICE;            // True if the data from central database is retrieved through Web Service
bool AUTOCOMMIT, SHOW_PREFID, HAS_SESSION;

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
   CHAR szTemp[150];


   switch (ul_reason_for_call)
   {
      case DLL_PROCESS_ATTACH:
         _env = new CODBCenv;
		 GetEnvironmentVariable("TEMP" , szTemp, 155);
		 strcat(szTemp,"\\ICIS32.log");
		 DEBUG = 1;
         START_LOG(szTemp);   
		 GMSCLOSED = FALSE;


		 //Check if SetGen has AUTOCOMMIT=YES and DLL SETTING has SHOW_PRFID=YES
         CHAR szCommit[8];
         AUTOCOMMIT = FALSE;
	     char szIniFile[150];
	     GetEnvironmentVariable("TEMP", (LPSTR) szIniFile, 255);
	     strcat(szIniFile,"\\ICIS.INI");
         GetPrivateProfileString("SETGEN", "AUTOCOMMIT", "", szCommit, sizeof(szCommit), (LPCSTR) szIniFile);
         if ( strcmp((char *) szCommit, "YES") == 0)
             AUTOCOMMIT = TRUE;
		 ZeroMemory(szCommit,8);
         GetPrivateProfileString("DLL SETTINGS", "SHOW_PREFID", "", szCommit, sizeof(szCommit), (LPCSTR) szIniFile);
         if ( strcmp((char *) szCommit, "YES") == 0)
             SHOW_PREFID = TRUE;

		 //HAS_SESSION will be used to test if there is call to Session ID and more likely there is a multi-user environment
		 HAS_SESSION = FALSE;
		 long szlen;
        GetPrivateProfileString("DLL SETTINGS", "SESSION_ID", "", szCommit, sizeof(szCommit), szIniFile);
 	    if ((szlen = strlen(szCommit)) > 0 )
	    {
			HAS_SESSION = TRUE;   
	    }

		LOG( "ICIS32 DLL v5.5.1.0");

  //      DWORD dwHandle, dwLen;
  //      char *szVersion;
		//LPTSTR lpData;
		//VS_FIXEDFILEINFO *pFileInfo;
		//UINT BufLen;
		//dwLen = GetFileVersionInfoSize("ICIS32.DLL",&dwHandle);
		//if (dwLen) {
		//	lpData = (LPTSTR)  malloc(dwLen);
		//	if (GetFileVersionInfo("ICIS32.DLL",dwHandle,dwLen,lpData)) {
		//		if (VerQueryValue(lpData,"\\",(LPVOID*) &pFileInfo, (PUINT) &BufLen)) {
  //                 WORD major,minor,build,revision; 
		//		   major = (WORD) malloc (dwLen);
		//		   minor = (WORD) malloc (dwLen);
		//		   build = (WORD) malloc (dwLen);
		//		   revision = (WORD) malloc (dwLen);
		//		   LOG( "ICIS32 DLL v");
		//		   major = HIWORD(pFileInfo->dwFileVersionMS);
		//		   LOG((char*) major,".");
		//		   minor=LOWORD(pFileInfo->dwFileVersionMS);
		//		   LOG((char*) minor,".");
		//		   build=HIWORD(pFileInfo->dwFileVersionLS);
		//		   LOG((char*) build,".");
		//		   revision=LOWORD(pFileInfo->dwFileVersionLS);
		//		   LOG((char*) revision,".");
		//		   LOG( lpData, "\n");

		//		}
		//	}
		//	free(lpData);
		//}
        break;
      case DLL_PROCESS_DETACH:
         delete _env;
		 DEBUG= 1;
         END_LOG();
         break;
      case DLL_THREAD_ATTACH:
      case DLL_THREAD_DETACH:
         break;
   }
   return TRUE;
}

#if defined(WEB)
void CheckWebService(LPCSTR szIniFile)
{
	CHAR wsURL[128];
	GetPrivateProfileString("Central GMS","URL","",wsURL,sizeof(wsURL),szIniFile);
	//if(strcmp(wsURL,"http://i1ws0112a:8080/axis/services/GmsWs?wsdl") == 0)
	if(strcmp(wsURL,"") != 0)   
	{	
		WEBSERVICE = true; 
		szURL = (_bstr_t) wsURL;  //convert to all small case	
	}  
	else
	{WEBSERVICE = false; szURL="";}
}
#endif

LONG CheckGMS(void)
{

  PUSH(CheckGMS);
  CODBCdirectStmt &central = _centralDBC->DirectStmt("SELECT * FROM GERMPLSM");
  CODBCdirectStmt &local = _localDBC->DirectStmt("SELECT * FROM LISTDATA");
  BOOLEAN fetchCentral;
  long i;
  char  msg[50], szLength[10];

  fetchCentral = TRUE;

  hasSID = 0; 
  local = _localDBC->DirectStmt("SELECT * FROM GERMPLSM");
  local.Execute();
  for (i=1; i<= local.ColCount(); i++) {
     if (strcmp(local.Field(i).Name(), "SID")==0)   hasSID = 1;
  }
  
  //(AMP): commented out because mySQL has erratic behaviour for returning the length of a char field
  //local = _localDBC->DirectStmt("SELECT ENTRYCD FROM LISTDATA");
  //local.Execute();
  //i=local.Field(1).Size();
  //strcpy(msg,"Length of LISTDATA.ENTRYCD(local):  ");
  //_itoa(i,szLength,10);
  //strcat(msg,szLength);
  //strcat(msg,"\n");
  //LOG(msg);
  //if (i < 47 ) ret = GMS_ERROR;
  POP();

}
 
LONG DLL_INTERFACE GMS_hasCIDSID(void)
{   
	if (hasSID != 0)
		return GMS_SUCCESS;
	else
		return GMS_NO_DATA;
}

LONG DLL_INTERFACE GMS_openDatabase(LPCSTR szIniFile)
{
   LONG userid,ret;
#if defined(WEB)
   CheckWebService(szIniFile);
#endif
   ret =GMS_openDatabase2( szIniFile, "GUEST", "GUEST",&userid);
   return ret;
}


LONG setSessionID(char *szSession)
{

   PUSH(setSessionID);
   LOG("setSessionID fuction\n");  //AMP: Debug

   CODBCdirectStmt &central= _centralDBC->DirectStmt();
   CODBCdirectStmt &local  = _localDBC->DirectStmt();

 LOG("setSessionID fuction: Binding of procedure statement\n");  //AMP: Debug

 BOOL first_time=TRUE;
 char _szSession[25], szConnected[2];


   strcpy(_szSession,szSession);
   LOGF("_szSession: %s \n",_szSession);  //AMP: Debug
 

   local.ExecuteProc(_szSession,25,szConnected,2);
   central.ExecuteProc(_szSession,25,szConnected,2);
   if (strcmp(szConnected, "F")==0 ) {
      ret=GMS_SUCCESS;
   }
   else
      ret = GMS_ERROR;

   POP();
   return ret;
#undef SQL_setSessionID

}


//////////////////////////////////////////////////////////////////////////////
//Modified: May 3, 2004
//          January 24, 2006 (AMP) - INSTALID is assigned to the installation ID of the local database
//Formerly the GMS_openDatabase2
//Access is allowed only to users with ustatus = 1; If the Admin user status is secure, deny access to any local user
// 
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE G_openDatabase2(LPCSTR szIniFile, LPCSTR szUserName, LPCSTR szPassword, LONG *userid)
{  
GMS_User user, userAdmin;
GMS_Installation lInstln,cInstln;
CHAR szDSN[128], szUID[128]="Admin", szPWD[128]="", szDEBUG[128], szDEBUGNO[]="NO", szSessionID[25];
int szlen;

   GetPrivateProfileString("DLL SETTINGS", "DEBUG", "", szDEBUG, sizeof(szDEBUG), szIniFile);
#if defined(WEB)
   CheckWebService(szIniFile);
#endif

   DEBUG = 0;
   ZeroMemory(&user, sizeof(user));
   ZeroMemory(&userAdmin, sizeof(userAdmin));
   if ( strcmp((char *) szDEBUG, (char *) szDEBUGNO) != 0)
      DEBUG = 1;
   if (DEBUG)
     LOG("Debug is on\n")
   else
     LOG("Debug is off\n");
   PUSH(G_openDatabase2);
   //1. Open Central
   LOG("1. Open Central Database\n");
   GetPrivateProfileString("Central GMS", "DSN", "", szDSN, sizeof(szDSN), szIniFile);
   GetPrivateProfileString("Central GMS", "UID", "", szUID, sizeof(szUID), szIniFile);
   GetPrivateProfileString("Central GMS", "PWD", "", szPWD, sizeof(szPWD), szIniFile);
   _centralDBC = _env->Connect(szDSN,szUID,szPWD);
   if (!_centralDBC->Connected())
   {
      GMS_closeDatabase();
      POP2(GMS_ERROR);
   }
   //2. Open Local
   LOG("2.Open Local Database\n");
   GetPrivateProfileString("Local GMS", "DSN", "", szDSN, sizeof(szDSN), szIniFile);
   GetPrivateProfileString("Local GMS", "UID", "", szUID, sizeof(szUID), szIniFile);
   GetPrivateProfileString("Local GMS", "PWD", "", szPWD, sizeof(szPWD), szIniFile);
   _localDBC = _env->Connect(szDSN,szUID,szPWD);
   if (!_localDBC->Connected())
   {
      GMS_closeDatabase();
      POP2(GMS_ERROR);
   }

   //2.b Set Session Id
   GetPrivateProfileString("DLL SETTINGS", "SESSION_ID", "", szSessionID, sizeof(szSessionID), szIniFile);
   if ((szlen = strlen(szSessionID)) > 0 )
   {
     LOGF("SessionID: %d \n",szlen);  //AMP: Debug
     LOGF("SessionID: %s \n",szSessionID);  //AMP: Debug
 	 setSessionID(szSessionID);
   }

   //3. Verify Guest Account, else goto to next step
   LOG("3. Verify Guest Account\n");
   if (strcmp(szUserName, "GUEST")==0 && strcmp(szPassword, "GUEST")==0) 
   {
      //3.a. Set databases to readonly 
      _centralDBC->SetReadOnly(TRUE);
      _localDBC->SetReadOnly(TRUE);
      //3.b. Set USERID,UACCESS
      UACCESS = 20;
      USERID = -32768;     // Guest account (no need to put in the USERS table)
      *userid = USERID;
      POP2(GMS_SUCCESS);
   }


   //4. Get User Record matching szUser
   LOG("4. Get User Record matching szUser\n");
   UACCESS = 100; // Set User access to administrator so we can call GMS_getUser
   user.userid = 0;

   strncpy(user.uname, szUserName, GMS_UNAME_SIZE);
   user.uname[GMS_UNAME_SIZE-1]='\0';
   user.upswd[GMS_UPSWD_SIZE-1]='\0';

   //4.a. check user 'szUser' in local database first
   LOG("  a. check user 'szUser' in local database first\n");
   ret=getUser(LOCAL,user, FIND_FIRST);
   if (ret==GMS_SUCCESS)
   {
       if (user.userid<0 && user.uaccess>100)
           ret = GMS_ERROR;
       //4.b. check user in central then
       else if (user.userid>0)
       {
          LOG("  b. check user in central (user is in Local)\n");
          CHAR  upswd[GMS_UPSWD_SIZE];
          strcpy(upswd,user.upswd);
          ret=getUser(CENTRAL,user, FIND_FIRST);
          if (ret==GMS_SUCCESS)
             strcpy(user.upswd,upswd);
       }
   }
   else
   {
      //4.b. check user in central then
	   LOG("  b. check user in central \n");
      ret=getUser(CENTRAL,user, FIND_FIRST);
   }
   //MessageBox(NULL,"GMS opendatabase - getuser","Test",MB_OK);
   if (ret==GMS_NO_DATA) 
   {
      LOG("ERROR: GMS_INVALID_USER\n");
      GMS_closeDatabase();
      POP2(GMS_INVALID_USER);
   }
   else if (ret==GMS_ERROR) 
   {
      LOG("ERROR: Unexpected Error\n");
      GMS_closeDatabase();
   }
   else {
      // Todo:
      // 1. Check if the User.instalid = Local.instalid
      // 2. Check if the Local installaton is updated
      // 3. Check if Local(Installation)=Central(Installation)
	  if (user.ustatus != 1) {
           GMS_closeDatabase();
		   POP2(GMS_ERROR);
	  }
      if (strcmp(user.upswd,szPassword)==0) 
      {
         // Get the USER's Record
         USERID = user.userid;
         *userid = USERID;
         UACCESS = user.uaccess; 
		 USTATUS = user.ustatus;
         // Get Local Installaton Record
         ZeroMemory(&lInstln,sizeof(lInstln));
         lInstln.instalid = 0;
		 //MessageBox(NULL,"GMS opendatabase  get local instln","Test",MB_OK);
         ret = getInstallation(LOCAL,lInstln, NULL, 0);
		 INSTALID = lInstln.instalid;

         if (ret==GMS_NO_DATA)
         {
            LOG("ERROR: GMS_INVALID_INSTALLATION (local installation not found)\n");
            GMS_closeDatabase();
            POP2(GMS_INVALID_INSTALLATION);
         }
         else if (ret!=GMS_SUCCESS)
         {
            GMS_closeDatabase();
            POP();
         }
         if (user.instalid!=0 && user.instalid!=lInstln.instalid)
         {
            LOG("ERROR: GMS_INVALID_INSTALLATION (central user installation do not match local installation)\n");
            GMS_closeDatabase();
            POP2(GMS_INVALID_INSTALLATION);
         }
         else
         {
            if (lInstln.instalid==0)
            {
               LOG("ERROR: GMS_INVALID_INSTALLATION (local installation not found:2)\n");
               GMS_closeDatabase();
               POP2(GMS_INVALID_INSTALLATION);
            }
            else if (lInstln.instalid>0)
            {
               ZeroMemory(&cInstln,sizeof(cInstln));
               cInstln.instalid=lInstln.instalid;
  		       //MessageBox(NULL,"GMS opendatabase  get central instln","Test",MB_OK);
               ret = getInstallation(CENTRAL, cInstln, NULL,0);
               if (ret==GMS_NO_DATA)
               {
                  LOG("ERROR: GMS_INVALID_INSTALLATION (central installation not found)\n");
                  GMS_closeDatabase();
                  POP2(GMS_INVALID_INSTALLATION);
               }
               else if (ret!=GMS_SUCCESS)
               {
                  LOG("ERROR: unexpected Error\n");
                  GMS_closeDatabase();
                  POP2(GMS_ERROR);
               }

               if (memcmp(&cInstln,&lInstln,sizeof(cInstln)))
               {
		       //20071127 (AMP): commented out for using the INSTLN to store the next GID
               //   LOG(" WARNING: Read Only\n");
               //   _centralDBC->SetReadOnly(TRUE);
               //   UACCESS = 20;
               //   POP2(GMS_READ_ONLY);
               }
               else
               {
                  ZeroMemory(&userAdmin, sizeof(userAdmin));
				  userAdmin.userid = lInstln.admin;
                  ret=getUser(CENTRAL,userAdmin, FIND_FIRST);
				  if (userAdmin.ustatus == 2) {
                      GMS_closeDatabase();
					  POP2(GMS_ERROR);
				  }
 			
                  POP2(GMS_SUCCESS);
               }
            }
            else
            {
               POP2(GMS_SUCCESS);
            }
         }
      }
      else 
      {
         LOG("ERROR: GMS_INVALID_PASSWORD\n");
         GMS_closeDatabase();
         POP2(GMS_INVALID_PASSWORD);
      }
   }
   LOG("GMS USERID: %d\n",USERID);
   POP2(GMS_SUCCESS);
   POP();
}


//////////////////////////////////////////////////////////////////////////////
//Modified: May 3, 2004
//Formerly the GMS_openDatabase2
//Access is allowed only to users with ustatus = 2; The password is encrypted to have access to ICIS database
// 
//////////////////////////////////////////////////////////////////////////////

G_openDatabase3(LPCSTR szIniFile, LPCSTR szUserName, LPCSTR szPassword, LONG *userid)
{  
GMS_User user;
GMS_Installation lInstln,cInstln;

CHAR szDSN[128], szUID[128]="Admin", szPWD[128]="", szDEBUG[128], szDEBUGNO[]="NO" , szSessionID[25];
int szlen;
CHAR szEncryptPWD[36], szEncryptPWD2[36];

   GetPrivateProfileString("DLL SETTINGS", "DEBUG", "", szDEBUG, sizeof(szDEBUG), szIniFile);
   DEBUG = 0;
   if ( strcmp((char *) szDEBUG, (char *) szDEBUGNO) != 0)
      DEBUG = 1;
   PUSH(G_openDatabase3);
   //1. Open Central
   LOG("1. Open Central Database\n");
   GetPrivateProfileString("Central GMS", "DSN", "", szDSN, sizeof(szDSN), szIniFile);
   GetPrivateProfileString("Central GMS", "UID", "", szUID, sizeof(szUID), szIniFile);
   GetPrivateProfileString("Central GMS", "PWD", "", szPWD, sizeof(szPWD), szIniFile);

   _centralDBC = _env->Connect(szDSN,szUID,szPWD);
   if (!_centralDBC->Connected())
   {
      GMS_closeDatabase();
      POP2(GMS_ERROR);
   }
   //2. Open Local
   LOG("2.Open Local Database\n");
   GetPrivateProfileString("Local GMS", "DSN", "", szDSN, sizeof(szDSN), szIniFile);
   GetPrivateProfileString("Local GMS", "UID", "", szUID, sizeof(szUID), szIniFile);
   GetPrivateProfileString("Local GMS", "PWD", "", szPWD, sizeof(szPWD), szIniFile);
   _localDBC = _env->Connect(szDSN,szUID,szPWD);
   if (!_localDBC->Connected())
   {
      GMS_closeDatabase();
      POP2(GMS_ERROR);
   }

   //2.b Set Session Id
   GetPrivateProfileString("DLL SETTINGS", "SESSION_ID", "", szSessionID, sizeof(szSessionID), szIniFile);
   if ((szlen = strlen(szSessionID)) > 0 )
   {
     LOGF("SessionID: %d \n",szlen);  //AMP: Debug
     LOGF("SessionID: %s \n",szSessionID);  //AMP: Debug
 	 setSessionID(szSessionID);
   }

   //3. Verify Guest Account, else goto to next step
   LOG("3. Verify Guest Account\n");
   if (strcmp(szUserName, "GUEST")==0 && strcmp(szPassword, "GUEST")==0) 
   {
      //3.a. Set databases to readonly 
      _centralDBC->SetReadOnly(TRUE);
      _localDBC->SetReadOnly(TRUE);
      //3.b. Set USERID,UACCESS
      UACCESS = 20;
      USERID = -32768;     // Guest account (no need to put in the USERS table)
      *userid = USERID;
      POP2(GMS_SUCCESS);
   }

   //4. Get User Record matching szUser
   LOG("4. Get User Record matching szUser\n");
   UACCESS = 100; // Set User access to administrator so we can call GMS_getUser
   user.userid = 0;

   strncpy(user.uname, szUserName, GMS_UNAME_SIZE);
   user.uname[GMS_UNAME_SIZE-1]='\0';

   //4.a. check user 'szUser' in local database first
   LOG("  a. check user 'szUser' in local database first\n");
   ret=getUser(LOCAL,user, FIND_FIRST);

   if (ret==GMS_SUCCESS)
   {
       if (user.userid<0 && user.uaccess>100)
           ret = GMS_ERROR;
       //4.b. check user in central then
       else if (user.userid>0)
       {
          LOG("  b. check user in central (user is in Local)\n");
          CHAR  upswd[GMS_UPSWD_SIZE];
          strcpy(upswd,user.upswd);
          ret=getUser(CENTRAL,user, FIND_FIRST);
          if (ret==GMS_SUCCESS)
             strcpy(user.upswd,upswd);
       }
   }
   else
   {
      //4.b. check user in central then
	   LOG("  b. check user in central \n");
      ret=getUser(CENTRAL,user, FIND_FIRST);
   }



   // check if password match the one in the database
   if (ret == GMS_SUCCESS) {
       encryptPWD((CHAR *)szPassword,(CHAR *)szEncryptPWD2);  
	   strncpy((CHAR *)szEncryptPWD,user.upswd, sizeof(user.upswd));
      if (strcmp(szEncryptPWD, szEncryptPWD2)!=0) {
         ret = GMS_NO_DATA;
	  }
   }
   
   if (ret==GMS_NO_DATA) 
   {
      LOG("ERROR: GMS_INVALID_USER\n");
      GMS_closeDatabase();
      POP2(GMS_INVALID_USER);
   }
   else if (ret==GMS_ERROR) 
   {
      LOG("ERROR: Unexpected Error\n");
      GMS_closeDatabase();
   }
   else {
      // Todo:
      // 1. Check if the User.instalid = Local.instalid
      // 2. Check if the Local installaton is updated
      // 3. Check if Local(Installation)=Central(Installation)
	  if (user.ustatus != 2) POP2(GMS_ERROR);
      if (strcmp(user.upswd,szEncryptPWD2)==0) 
      {
         // Get the USER's Record
         USERID = user.userid;
         *userid = USERID;
		 USTATUS = user.ustatus;
         UACCESS = user.uaccess; 
         // Get Local Installaton Record
         ZeroMemory(&lInstln,sizeof(lInstln));
         lInstln.instalid = 0;
         ret = getInstallation(LOCAL,lInstln, NULL, 0);
 		 INSTALID = lInstln.instalid;

         if (ret==GMS_NO_DATA)
         {
            LOG("ERROR: GMS_INVALID_INSTALLATION (local installation not found)\n");
            GMS_closeDatabase();
            POP2(GMS_INVALID_INSTALLATION);
         }
         else if (ret!=GMS_SUCCESS)
         {
            GMS_closeDatabase();
            POP();
         }
         if (user.instalid!=0 && user.instalid!=lInstln.instalid)
         {
            LOG("ERROR: GMS_INVALID_INSTALLATION (central user installation do not match local installation)\n");
            GMS_closeDatabase();
            POP2(GMS_INVALID_INSTALLATION);
         }
         else
         {
            if (lInstln.instalid==0)
            {
               LOG("ERROR: GMS_INVALID_INSTALLATION (local installation not found:2)\n");
               GMS_closeDatabase();
               POP2(GMS_INVALID_INSTALLATION);
            }
            else if (lInstln.instalid>0)
            {
               ZeroMemory(&cInstln,sizeof(cInstln));
               cInstln.instalid=lInstln.instalid;
               ret = getInstallation(CENTRAL, cInstln, NULL,0);
               if (ret==GMS_NO_DATA)
               {
                  LOG("ERROR: GMS_INVALID_INSTALLATION (central installation not found)\n");
                  GMS_closeDatabase();
                  POP2(GMS_INVALID_INSTALLATION);
               }
               else if (ret!=GMS_SUCCESS)
               {
                  LOG("ERROR: unexpected Error\n");
                  GMS_closeDatabase();
                  POP2(GMS_ERROR);
               }
               if (memcmp(&cInstln,&lInstln,sizeof(cInstln)))
               {
  		       //20071127 (AMP): commented out for using the INSTLN to store the next GID
                  //LOG(" WARNING: Read Only\n");
                  //_centralDBC->SetReadOnly(TRUE);
                  //UACCESS = 20;
                  //POP2(GMS_READ_ONLY);
               }
               else
               {
                  POP2(GMS_SUCCESS);
               }
            }
            else
            {
               POP2(GMS_SUCCESS);
            }
         }
      }
      else 
      {
         LOG("ERROR: GMS_INVALID_PASSWORD\n");
         GMS_closeDatabase();
         POP2(GMS_INVALID_PASSWORD);
      }
   }
   POP2(GMS_SUCCESS);
}

//////////////////////////////////////////////////////////////////////////////
//Modified: May 3, 2004
//Formerly the GMS_openDatabase2
//Access is allowed only to users with ustatus = 3; 
//Password is encrypted to have acess to ICIS database; User name is encrypted to have access to central GMS DB
// 
//////////////////////////////////////////////////////////////////////////////

G_openDatabase4(LPCSTR szIniFile, LPCSTR szUserName, LPCSTR szPassword, LONG *userid)
{  
GMS_User user;
GMS_Installation lInstln,cInstln;

CHAR szDSN[128], szUID[128]="Admin", szPWD[128]="", szDEBUG[128], szDEBUGNO[]="NO", szSessionID[25];
int szlen;
CHAR szEncryptPWD[36], szEncryptPWD2[36], szEncryptUser[36];

   GetPrivateProfileString("DLL SETTINGS", "DEBUG", "", szDEBUG, sizeof(szDEBUG), szIniFile);
   DEBUG = 0;
   if ( strcmp((char *) szDEBUG, (char *) szDEBUGNO) != 0)
      DEBUG = 1;
   PUSH(G_openDatabase4);
   //1. Open Central
   LOG("1. Open Central Database\n");
   //Encrypt user

   GetPrivateProfileString("Central GMS", "DSN", "", szDSN, sizeof(szDSN), szIniFile);
   GetPrivateProfileString("Central GMS", "UID", "", szUID, sizeof(szUID), szIniFile);
   GetPrivateProfileString("Central GMS", "PWD", "", szPWD, sizeof(szPWD), szIniFile);

   /*** Encrypts the username
   encryptPWD((CHAR *)szUID,(CHAR *)szEncryptUser);  
   _centralDBC = _env->Connect(szDSN,szEncryptUser, szPWD);
   ****/

   _centralDBC = _env->Connect(szDSN,szUID, szPWD);

   if (!_centralDBC->Connected())
   {
      GMS_closeDatabase();
      POP2(GMS_ERROR);
   }
   //2. Open Local
   LOG("2.Open Local Database\n");
   GetPrivateProfileString("Local GMS", "DSN", "", szDSN, sizeof(szDSN), szIniFile);
   GetPrivateProfileString("Local GMS", "UID", "", szUID, sizeof(szUID), szIniFile);
   GetPrivateProfileString("Local GMS", "PWD", "", szPWD, sizeof(szPWD), szIniFile);

   encryptPWD((CHAR *)szUID,(CHAR *)szEncryptUser);  
   _localDBC = _env->Connect(szDSN,szEncryptUser,szPWD);
   if (!_localDBC->Connected())
   {
      GMS_closeDatabase();
      POP2(GMS_ERROR);
   }

   //2.b Set Session Id
   GetPrivateProfileString("DLL SETTINGS", "SESSION_ID", "", szSessionID, sizeof(szSessionID), szIniFile);
   if ((szlen = strlen(szSessionID)) > 0 )
   {
     LOGF("SessionID: %d \n",szlen);  //AMP: Debug
     LOGF("SessionID: %s \n",szSessionID);  //AMP: Debug
 	 setSessionID(szSessionID);
   }

   //3. Verify Guest Account, else goto to next step
   LOG("3. Verify Guest Account\n");
   if (strcmp(szUserName, "GUEST")==0 && strcmp(szPassword, "GUEST")==0) 
   {
      //3.a. Set databases to readonly 
      _centralDBC->SetReadOnly(TRUE);
      _localDBC->SetReadOnly(TRUE);
      //3.b. Set USERID,UACCESS
      UACCESS = 20;
      USERID = -32768;     // Guest account (no need to put in the USERS table)
      *userid = USERID;
      POP2(GMS_SUCCESS);
   }

   //4. Get User Record matching szUser
   LOG("4. Get User Record matching szUser\n");
   UACCESS = 100; // Set User access to administrator so we can call GMS_getUser
   user.userid = 0;

   strncpy(user.uname, szUserName, GMS_UNAME_SIZE);
   user.uname[GMS_UNAME_SIZE-1]='\0';

   //4.a. check user 'szUser' in local database first
   LOG("  a. check user 'szUser' in local database first\n");
   ret=getUser(LOCAL,user, FIND_FIRST);

   if (ret==GMS_SUCCESS)
   {
       if (user.userid<0 && user.uaccess>100)
           ret = GMS_ERROR;
       //4.b. check user in central then
       else if (user.userid>0)
       {
          LOG("  b. check user in central (user is in Local)\n");
          CHAR  upswd[GMS_UPSWD_SIZE];
          strcpy(upswd,user.upswd);
          ret=getUser(CENTRAL,user, FIND_FIRST);
          if (ret==GMS_SUCCESS)
             strcpy(user.upswd,upswd);
       }
   }
   else
   {
      //4.b. check user in central then
	   LOG("  b. check user in central \n");
      ret=getUser(CENTRAL,user, FIND_FIRST);
   }


   // check if password match the one in the database
   if (ret == GMS_SUCCESS) {
       encryptPWD((CHAR *)szPassword,(CHAR *)szEncryptPWD2);  
	   strncpy((CHAR *)szEncryptPWD,user.upswd, sizeof(user.upswd));
      if (strcmp(szEncryptPWD, szEncryptPWD2)!=0) {
         ret = GMS_NO_DATA;
	  }
   }

   if (ret==GMS_NO_DATA) 
   {
      LOG("ERROR: GMS_INVALID_USER\n");
      GMS_closeDatabase();
      POP2(GMS_INVALID_USER);
   }
   else if (ret==GMS_ERROR) 
   {
      LOG("ERROR: Unexpected Error\n");
      GMS_closeDatabase();
   }
   else {
      // Todo:
      // 1. Check if the User.instalid = Local.instalid
      // 2. Check if the Local installaton is updated
      // 3. Check if Local(Installation)=Central(Installation)
      if (strcmp(user.upswd,szEncryptPWD2)==0) 
      {
         // Get the USER's Record
         USERID = user.userid;
         *userid = USERID;
         UACCESS = user.uaccess; 
		 USTATUS = user.ustatus;
         // Get Local Installaton Record
         ZeroMemory(&lInstln,sizeof(lInstln));
         lInstln.instalid = 0;
         ret = getInstallation(LOCAL,lInstln, NULL, 0);
 		 INSTALID = lInstln.instalid;

         if (ret==GMS_NO_DATA)
         {
            LOG("ERROR: GMS_INVALID_INSTALLATION (local installation not found)\n");
            GMS_closeDatabase();
            POP2(GMS_INVALID_INSTALLATION);
         }
         else if (ret!=GMS_SUCCESS)
         {
            GMS_closeDatabase();
            POP();
         }
         if (user.instalid!=0 && user.instalid!=lInstln.instalid)
         {
            LOG("ERROR: GMS_INVALID_INSTALLATION (central user installation do not match local installation)\n");
            GMS_closeDatabase();
            POP2(GMS_INVALID_INSTALLATION);
         }
         else
         {
            if (lInstln.instalid==0)
            {
               LOG("ERROR: GMS_INVALID_INSTALLATION (local installation not found:2)\n");
               GMS_closeDatabase();
               POP2(GMS_INVALID_INSTALLATION);
            }
            else if (lInstln.instalid>0)
            {
               ZeroMemory(&cInstln,sizeof(cInstln));
               cInstln.instalid=lInstln.instalid;
               ret = getInstallation(CENTRAL, cInstln, NULL,0);
               if (ret==GMS_NO_DATA)
               {
                  LOG("ERROR: GMS_INVALID_INSTALLATION (central installation not found)\n");
                  GMS_closeDatabase();
                  POP2(GMS_INVALID_INSTALLATION);
               }
               else if (ret!=GMS_SUCCESS)
               {
                  LOG("ERROR: unexpected Error\n");
                  GMS_closeDatabase();
                  POP2(GMS_ERROR);
               }

               if (memcmp(&cInstln,&lInstln,sizeof(cInstln)))
               {
  		          //20071127 (AMP): commented out for using the INSTLN to store the next GID
                  //LOG(" WARNING: Read Only\n");
                  //_centralDBC->SetReadOnly(TRUE);
                  //UACCESS = 20;
                  //POP2(GMS_READ_ONLY);
               }
               else
               {
                  POP2(GMS_SUCCESS);
               }
            }
            else
            {
               POP2(GMS_SUCCESS);
            }
         }
      }
      else 
      {
         LOG("ERROR: GMS_INVALID_PASSWORD\n");
         GMS_closeDatabase();
         POP2(GMS_INVALID_PASSWORD);
      }
   }
   POP2(GMS_SUCCESS);
}



//////////////////////////////////////////////////////////////////////////////
//Modified: May 3, 2004
//Formerly the GMS_openDatabase2
//Access is allowed only to users with ustatus = 3; 
//Password is encrypted to have acess to ICIS database; Password is encrypted to have access to  GMS DB
// 
//////////////////////////////////////////////////////////////////////////////
G_openDatabase5(LPCSTR szIniFile, LPCSTR szUserName, LPCSTR szPassword, LONG *userid)
{  
GMS_User user;
GMS_Installation lInstln,cInstln;

CHAR szDSN[128], szUID[128]="Admin", szPWD[128]="", szDEBUG[128], szDEBUGNO[]="NO", szSessionID[25];
int szlen;
CHAR szEncryptPWD[36], szEncryptPWD2[36], szEncryptUser[36];

   GetPrivateProfileString("DLL SETTINGS", "DEBUG", "", szDEBUG, sizeof(szDEBUG), szIniFile);
   DEBUG = 0;
   if ( strcmp((char *) szDEBUG, (char *) szDEBUGNO) != 0)
      DEBUG = 1;
   PUSH(G_openDatabase5);
   //1. Open Central
   LOG("1. Open Central Database\n");

   GetPrivateProfileString("Central GMS", "DSN", "", szDSN, sizeof(szDSN), szIniFile);
   GetPrivateProfileString("Central GMS", "UID", "", szUID, sizeof(szUID), szIniFile);
   GetPrivateProfileString("Central GMS", "PWD", "", szPWD, sizeof(szPWD), szIniFile);

   /*** Encrypt the password ****/
   encryptPWD((CHAR *)szPWD,(CHAR *)szEncryptUser);  
   _centralDBC = _env->Connect(szDSN,szUID, szEncryptUser);


   if (!_centralDBC->Connected())
   {
      GMS_closeDatabase();
      POP2(GMS_ERROR);
   }
   //2. Open Local
   LOG("2.Open Local Database\n");
   GetPrivateProfileString("Local GMS", "DSN", "", szDSN, sizeof(szDSN), szIniFile);
   GetPrivateProfileString("Local GMS", "UID", "", szUID, sizeof(szUID), szIniFile);
   GetPrivateProfileString("Local GMS", "PWD", "", szPWD, sizeof(szPWD), szIniFile);

   /*** Encrypt the password ****/
   encryptPWD((CHAR *)szPWD,(CHAR *)szEncryptUser);  
   _localDBC = _env->Connect(szDSN,szUID,szEncryptUser);
   if (!_localDBC->Connected())
   {
      GMS_closeDatabase();
      POP2(GMS_ERROR);
   }

   //3. Verify Guest Account, else goto to next step
   LOG("3. Verify Guest Account\n");
   if (strcmp(szUserName, "GUEST")==0 && strcmp(szPassword, "GUEST")==0) 
   {
      //3.a. Set databases to readonly 
      _centralDBC->SetReadOnly(TRUE);
      _localDBC->SetReadOnly(TRUE);
      //3.b. Set USERID,UACCESS
      UACCESS = 20;
      USERID = -32768;     // Guest account (no need to put in the USERS table)
      *userid = USERID;
      POP2(GMS_SUCCESS);
   }

   //2.b Set Session Id
   GetPrivateProfileString("DLL SETTINGS", "SESSION_ID", "", szSessionID, sizeof(szSessionID), szIniFile);
   if ((szlen = strlen(szSessionID)) > 0 )
   {
     LOGF("SessionID: %d \n",szlen);  //AMP: Debug
     LOGF("SessionID: %s \n",szSessionID);  //AMP: Debug
 	 setSessionID(szSessionID);
   }



   //4. Get User Record matching szUser
   LOG("4. Get User Record matching szUser\n");
   UACCESS = 100; // Set User access to administrator so we can call GMS_getUser
   user.userid = 0;

   strncpy(user.uname, szUserName, GMS_UNAME_SIZE);
   user.uname[GMS_UNAME_SIZE-1]='\0';

   //4.a. check user 'szUser' in local database first
   LOG("  a. check user 'szUser' in local database first\n",szUserName);
   ret=getUser(LOCAL,user, FIND_FIRST);

   if (ret==GMS_SUCCESS)
   {
       if (user.userid<0 && user.uaccess>100)
           ret = GMS_ERROR;
       //4.b. check user in central then
       else if (user.userid>0)
       {
          LOG("  b. check user in central (user is in Local)\n");
          CHAR  upswd[GMS_UPSWD_SIZE];
          strcpy(upswd,user.upswd);
          ret=getUser(CENTRAL,user, FIND_FIRST);
          if (ret==GMS_SUCCESS)
             strcpy(user.upswd,upswd);
       }
   }
   else
   {
      //4.b. check user in central then
	   LOG("  b. check user in central \n");
      ret=getUser(CENTRAL,user, FIND_FIRST);
   }


   // check if password match the one in the database
   if (ret == GMS_SUCCESS) {
       encryptPWD((CHAR *)szPassword,(CHAR *)szEncryptPWD2);  
	   strncpy((CHAR *)szEncryptPWD,user.upswd, sizeof(user.upswd));
      if (strcmp(szEncryptPWD, szEncryptPWD2)!=0) {
         ret = GMS_NO_DATA;
	  }
   }

   if (ret==GMS_NO_DATA) 
   {
      LOG("ERROR: GMS_INVALID_USER\n");
      GMS_closeDatabase();
      POP2(GMS_INVALID_USER);
   }
   else if (ret==GMS_ERROR) 
   {
      LOG("ERROR: Unexpected Error\n");
      GMS_closeDatabase();
   }
   else {
      // Todo:
      // 1. Check if the User.instalid = Local.instalid
      // 2. Check if the Local installaton is updated
      // 3. Check if Local(Installation)=Central(Installation)
      if (strcmp(user.upswd,szEncryptPWD2)==0) 
      {
         // Get the USER's Record
         USERID = user.userid;
         *userid = USERID;
         UACCESS = user.uaccess; 
		 USTATUS = user.ustatus;
         LOG("USER ID \n", user.userid);
         // Get Local Installaton Record
         ZeroMemory(&lInstln,sizeof(lInstln));
         lInstln.instalid = 0;
         ret = getInstallation(LOCAL,lInstln, NULL, 0);
 		 INSTALID = lInstln.instalid;

         if (ret==GMS_NO_DATA)
         {
            LOG("ERROR: GMS_INVALID_INSTALLATION (local installation not found)\n");
            GMS_closeDatabase();
            POP2(GMS_INVALID_INSTALLATION);
         }
         else if (ret!=GMS_SUCCESS)
         {
            GMS_closeDatabase();
            POP();
         }
         if (user.instalid!=0 && user.instalid!=lInstln.instalid)
         {
            LOG("ERROR: GMS_INVALID_INSTALLATION (central user installation do not match local installation)\n");
            GMS_closeDatabase();
            POP2(GMS_INVALID_INSTALLATION);
         }
         else
         {
            if (lInstln.instalid==0)
            {
               LOG("ERROR: GMS_INVALID_INSTALLATION (local installation not found:2)\n");
               GMS_closeDatabase();
               POP2(GMS_INVALID_INSTALLATION);
            }
            else if (lInstln.instalid>0)
            {
               ZeroMemory(&cInstln,sizeof(cInstln));
               cInstln.instalid=lInstln.instalid;
               ret = getInstallation(CENTRAL, cInstln, NULL,0);
               if (ret==GMS_NO_DATA)
               {
                  LOG("ERROR: GMS_INVALID_INSTALLATION (central installation not found)\n");
                  GMS_closeDatabase();
                  POP2(GMS_INVALID_INSTALLATION);
               }
               else if (ret!=GMS_SUCCESS)
               {
                  LOG("ERROR: unexpected Error\n");
                  GMS_closeDatabase();
                  POP2(GMS_ERROR);
               }
               if (memcmp(&cInstln,&lInstln,sizeof(cInstln)))
               {
  		       //20071127 (AMP): commented out for using the INSTLN to store the next GID
                  //LOG(" WARNING: Read Only\n");
                  //_centralDBC->SetReadOnly(TRUE);
                  //UACCESS = 20;
                  //POP2(GMS_READ_ONLY);
               }
               else
               {
                  POP2(GMS_SUCCESS);
               }
            }
            else
            {
               POP2(GMS_SUCCESS);
            }
         }
      }
      else 
      {
         LOG("ERROR: GMS_INVALID_PASSWORD\n");
         GMS_closeDatabase();
         POP2(GMS_INVALID_PASSWORD);
      }
   }
   POP2(GMS_SUCCESS);
}


LONG deleteGMSNull(void)
{
    PUSH(deleteGMSNull); 
    CODBCbindedStmt &local  = _localDBC->BindedStmt("DELETE FROM GERMPLSM WHERE GID=0"); 
    if (local.Execute())
      ret=DMS_SUCCESS;
    else
      ret=DMS_ERROR;

	local.SetSQLstr("DELETE FROM NAMES WHERE GID=0 AND NID =0 ");
    if (local.Execute())
      ret=DMS_SUCCESS;
    else
      ret=DMS_ERROR;

    local.SetSQLstr("DELETE FROM LISTDATA WHERE LISTID=0 AND GID=0");
    if (local.Execute())
      ret=DMS_SUCCESS;
    else
      ret=DMS_ERROR;


    local.SetSQLstr("DELETE FROM LISTNMS WHERE LISTID=0  ");
    if (local.Execute())
      ret=DMS_SUCCESS;
    else
      ret=DMS_ERROR;
    POP();
}






LONG DLL_INTERFACE GMS_openDatabase2(LPCSTR szIniFile, LPCSTR szUserName, LPCSTR szPassword, LONG *userid)
{
    LPCSTR szDB;
	PUSH(GMS_openDatabase2);
    CHAR szSecurity[5];
	int security = 1;
	//MessageBox(NULL,"GMS opendatabase 1","Test",MB_OK);
    GetPrivateProfileString("DLL SETTINGS", "SECURITY", "", szSecurity, sizeof(szSecurity), szIniFile);
#if defined(WEB)
	CheckWebService(szIniFile); 
#endif
	//MessageBox(NULL,"GMS opendatabase 2","Test",MB_OK);
 	if (strlen(szSecurity) > 0 ) security = atoi(szSecurity);
	if (security ==1) 
		ret = G_openDatabase2(szIniFile, szUserName, szPassword, userid);
	else if (security ==2)
		ret = G_openDatabase3(szIniFile, szUserName, szPassword, userid);
	else if (security == 3)
		ret = G_openDatabase4(szIniFile, szUserName, szPassword, userid);
	else if (security == 4)
		ret = G_openDatabase5(szIniFile, szUserName, szPassword, userid);
	//MessageBox(NULL,"GMS opendatabase 3","Test",MB_OK);
    if (ret==GMS_SUCCESS) ret = CheckGMS();
	if (ret==GMS_ERROR) ret = GMS_INVALID_DBSTRUCTURE;
    if (_localDBC)
    {

       szDB = _localDBC->DatabaseName();
	   	//MessageBox(NULL,"GMS opendatabase 4","Test",MB_OK);
       if (strcmp(szDB, "MySQL")==0 ) 
          deleteGMSNull();
	   if ((strcmp(szDB, "PostgreSQL")==0 )  && (ret== GMS_INVALID_DBSTRUCTURE)) 
		   ret = GMS_SUCCESS;

	}
   	//MessageBox(NULL,"GMS opendatabase 5","Test",MB_OK);
    

    POP();
}






void DLL_INTERFACE GMS_closeDatabase(void)
{
   LPCSTR szDB;
   LOG(">GMS_closeDatabase\n");
   if (_centralDBC)
   {
      delete _centralDBC;
      _centralDBC=NULL;
   }
   if (_localDBC)
   {
      szDB = _localDBC->DatabaseName();
      if (strcmp(szDB, "MySQL")==0 ) 
	       deleteGMSNull();
      delete _localDBC;
      _localDBC=NULL;
   }
   GMSCLOSED = TRUE;
   LOG("<GMS_closeDatabase\n");
}

void DLL_INTERFACE GMS_rollbackData(void)
{
   _centralDBC->Rollback();
   _localDBC->Rollback();
}

void DLL_INTERFACE GMS_commitData(void)
{
   LOG(">GMS_commitData\n");
   _centralDBC->Commit();
   _localDBC->Commit();
   LOG("<GMS_commitData\n");
}

void DLL_INTERFACE GMS_autoCommit(LONG autoCommit)
{
   LOG(">GMS_autoCommit\n");
   _centralDBC->SetAutoCommit(autoCommit);
   _localDBC->SetAutoCommit(autoCommit);
   LOG("<GMS_autoCommit\n");
}

LONG setSessionID_DMS(char *szSession)
{

   PUSH(setSessionID);
   LOG("setSessionID fuction\n");  //AMP: Debug

   CODBCdirectStmt &central= _centralDMS->DirectStmt();
   CODBCdirectStmt &local  = _localDMS->DirectStmt();

 LOG("setSessionID fuction: Binding of procedure statement\n");  //AMP: Debug

 BOOL first_time=TRUE;
 char _szSession[25], szConnected[2];


   strcpy(_szSession,szSession);
   LOGF("_szSession: %s \n",_szSession);  //AMP: Debug
 
   local.ExecuteProc(_szSession,25,szConnected,2);
   central.ExecuteProc(_szSession,25,szConnected,2);

   if (strcmp(szConnected, "F")==0 ) {
      ret=GMS_SUCCESS;
   }
   else
      ret = GMS_ERROR;

   POP();
   return ret;

}


LONG DLL_INTERFACE D_openDatabase(LPCSTR szIniFile)
{  

   CHAR szDSN[128], szUID[128]="Admin", szPWD[128]="", szDEBUG[128], szDEBUGNO[]="NO";;
   GetPrivateProfileString("DLL SETTINGS", "DEBUG", "", szDEBUG, sizeof(szDEBUG), szIniFile);
   DEBUG = 0;
   if ( strcmp((char *) szDEBUG, (char *) szDEBUGNO) != 0)
      DEBUG = 1;
   PUSH(DMS_openDatabase);
  //1. Open Central
   LOG("1. Open Central Database\n");
   GetPrivateProfileString("Central DMS", "DSN", "", szDSN, sizeof(szDSN), szIniFile);
   GetPrivateProfileString("Central DMS", "UID", "", szUID, sizeof(szUID), szIniFile);
   GetPrivateProfileString("Central DMS", "PWD", "", szPWD, sizeof(szPWD), szIniFile);
   _centralDMS = _env->Connect(szDSN,szUID,szPWD);
   if (!_centralDMS->Connected())
   {
      DMS_closeDatabase();
      POP2(GMS_ERROR);
   }
   //2. Open Local
   LOG("2.Open Local Database\n");
   GetPrivateProfileString("Local DMS", "DSN", "", szDSN, sizeof(szDSN), szIniFile);
   GetPrivateProfileString("Local DMS", "UID", "", szUID, sizeof(szUID), szIniFile);
   GetPrivateProfileString("Local DMS", "PWD", "", szPWD, sizeof(szPWD), szIniFile);
   _localDMS = _env->Connect(szDSN,szUID,szPWD);
   if (!_localDMS->Connected())
   {
      DMS_closeDatabase();
      POP2(GMS_ERROR);
   }
   POP2(GMS_SUCCESS);
}

// ****  User name is encrypted ********/
LONG DLL_INTERFACE D_openDatabase2(LPCSTR szIniFile)
{  

   CHAR szDSN[128], szUID[128]="Admin", szPWD[128]="", szDEBUG[128], szDEBUGNO[]="NO";;
   CHAR  szEncryptUser[36];

   GetPrivateProfileString("DLL SETTINGS", "DEBUG", "", szDEBUG, sizeof(szDEBUG), szIniFile);
   DEBUG = 0;
   if ( strcmp((char *) szDEBUG, (char *) szDEBUGNO) != 0)
      DEBUG = 1;
   PUSH(DMS_openDatabase);
  //1. Open Central
   LOG("1. Open Central Database\n");
   GetPrivateProfileString("Central DMS", "DSN", "", szDSN, sizeof(szDSN), szIniFile);
   GetPrivateProfileString("Central DMS", "UID", "", szUID, sizeof(szUID), szIniFile);
   GetPrivateProfileString("Central DMS", "PWD", "", szPWD, sizeof(szPWD), szIniFile);

   /*** Encrypt the password ****/
   encryptPWD((CHAR *)szUID,(CHAR *)szEncryptUser);  
   _centralDMS = _env->Connect(szDSN, szEncryptUser, szPWD);

   
//   _centralDMS = _env->Connect(szDSN,szUID,szPWD);
   if (!_centralDMS->Connected())
   {
      DMS_closeDatabase();
      POP2(GMS_ERROR);
   }
   //2. Open Local
   LOG("2.Open Local Database\n");
   GetPrivateProfileString("Local DMS", "DSN", "", szDSN, sizeof(szDSN), szIniFile);
   GetPrivateProfileString("Local DMS", "UID", "", szUID, sizeof(szUID), szIniFile);
   GetPrivateProfileString("Local DMS", "PWD", "", szPWD, sizeof(szPWD), szIniFile);

   encryptPWD((CHAR *)szUID,(CHAR *)szEncryptUser);  
   _localDMS = _env->Connect(szDSN,szEncryptUser,szPWD);
   
   //_localDMS = _env->Connect(szDSN,szUID,szPWD);
   if (!_localDMS->Connected())
   {
      DMS_closeDatabase();
      POP2(GMS_ERROR);
   }
   POP2(GMS_SUCCESS);
}


// ****  Password is encrypted ********/
LONG DLL_INTERFACE D_openDatabase3(LPCSTR szIniFile)
{  

   CHAR szDSN[128], szUID[128]="Admin", szPWD[128]="", szDEBUG[128], szDEBUGNO[]="NO";;
   CHAR  szEncryptUser[36];

   GetPrivateProfileString("DLL SETTINGS", "DEBUG", "", szDEBUG, sizeof(szDEBUG), szIniFile);
   DEBUG = 0;
   if ( strcmp((char *) szDEBUG, (char *) szDEBUGNO) != 0)
      DEBUG = 1;
   PUSH(DMS_openDatabase);
  //1. Open Central
   LOG("1. Open Central Database\n");
   GetPrivateProfileString("Central DMS", "DSN", "", szDSN, sizeof(szDSN), szIniFile);
   GetPrivateProfileString("Central DMS", "UID", "", szUID, sizeof(szUID), szIniFile);
   GetPrivateProfileString("Central DMS", "PWD", "", szPWD, sizeof(szPWD), szIniFile);

   /*** Encrypt the password ****/
   encryptPWD((CHAR *)szPWD,(CHAR *)szEncryptUser);  
   _centralDMS = _env->Connect(szDSN,szUID, szEncryptUser);

   
//   _centralDMS = _env->Connect(szDSN,szUID,szPWD);
   if (!_centralDMS->Connected())
   {
      DMS_closeDatabase();
      POP2(GMS_ERROR);
   }
   //2. Open Local
   LOG("2.Open Local Database\n");
   GetPrivateProfileString("Local DMS", "DSN", "", szDSN, sizeof(szDSN), szIniFile);
   GetPrivateProfileString("Local DMS", "UID", "", szUID, sizeof(szUID), szIniFile);
   GetPrivateProfileString("Local DMS", "PWD", "", szPWD, sizeof(szPWD), szIniFile);

   encryptPWD((CHAR *)szPWD,(CHAR *)szEncryptUser);  
   _localDMS = _env->Connect(szDSN,szUID,szEncryptUser);
   
   //_localDMS = _env->Connect(szDSN,szUID,szPWD);
   if (!_localDMS->Connected())
   {
      DMS_closeDatabase();
      POP2(GMS_ERROR);
   }
   POP2(GMS_SUCCESS);
}




LONG DLL_INTERFACE DMS_openDatabase(LPCSTR szIniFile)
{  
	PUSH(DMS_openDatabase);
    CHAR szSecurity[5],szSessionID[25];;
	int security = 1, szlen;
    GetPrivateProfileString("DLL SETTINGS", "SECURITY", "", szSecurity, sizeof(szSecurity), szIniFile);
	if (strlen(szSecurity) > 0 ) security = atoi(szSecurity);
	if (security ==4)   //Password is encrypted
		ret = D_openDatabase3(szIniFile);
	else if  (security ==3)   //Username is entrypted
		ret = D_openDatabase2(szIniFile);
	else
  	    ret = D_openDatabase(szIniFile);

    GetPrivateProfileString("DLL SETTINGS", "SESSION_ID", "", szSessionID, sizeof(szSessionID), szIniFile);
 	if ((szlen = strlen(szSessionID)) > 0 )
	{
       LOGF("SessionID: %d \n",szlen);  //AMP: Debug
       LOGF("SessionID: %s \n",szSessionID);  //AMP: Debug
	   setSessionID_DMS(szSessionID);
	}


    POP();

}


LONG deleteDMSNull(void)
{
    PUSH(deleteDMSNull); 
    CODBCbindedStmt &local  = _localDMS->BindedStmt("DELETE FROM OINDEX WHERE OUNITID=0 AND LEVELNO=0"); 
    if (local.Execute())
      ret=DMS_SUCCESS;
    else
      ret=DMS_ERROR;

	local.SetSQLstr("DELETE FROM DMSATTR  WHERE DMSATID=0 ");
    if (local.Execute())
      ret=DMS_SUCCESS;
    else
      ret=DMS_ERROR;

    //local.SetSQLstr("DELETE FROM DMSATTR  WHERE DMSATID=0");
    //if (local.Execute())
    //  ret=DMS_SUCCESS;
    //else
    //  ret=DMS_ERROR;


    local.SetSQLstr("DELETE FROM LEVEL_C WHERE LABELID =0 AND LEVELNO=0  ");
    if (local.Execute())
      ret=DMS_SUCCESS;
    else
      ret=DMS_ERROR;

	local.SetSQLstr("DELETE FROM LEVEL_N WHERE LABELID =0 AND LEVELNO=0  ");
    if (local.Execute())
      ret=DMS_SUCCESS;
    else
      ret=DMS_ERROR;


    local.SetSQLstr("DELETE FROM EFFECT WHERE FACTORID =0 ");
    if (local.Execute())
      ret=DMS_SUCCESS;
    else
      ret=DMS_ERROR;


    local.SetSQLstr("DELETE FROM FACTOR WHERE LABELID = 0 AND STUDYID=0");
    if (local.Execute())
      ret=DMS_SUCCESS;
    else
      ret=DMS_ERROR;

    local.SetSQLstr("DELETE FROM DATA_N WHERE VARIATID =0 AND OUNITID=0  ");
    if (local.Execute())
      ret=DMS_SUCCESS;
    else
      ret=DMS_ERROR;

    local.SetSQLstr("DELETE  FROM DATA_C WHERE VARIATID =0 AND OUNITID=0 ");
    if (local.Execute())
      ret=DMS_SUCCESS;
    else
      ret=DMS_ERROR;

    local.SetSQLstr("DELETE FROM VEFFECT WHERE VARIATID =0 ");
    if (local.Execute())
      ret=DMS_SUCCESS;
    else
      ret=DMS_ERROR;

    local.SetSQLstr("DELETE FROM VARIATE WHERE VARIATID = 0 AND STUDYID=0 ");
    if (local.Execute())
      ret=DMS_SUCCESS;
    else
      ret=DMS_ERROR;

    local.SetSQLstr("DELETE FROM STUDY WHERE STUDYID = 0 ");
    if (local.Execute())
      ret=DMS_SUCCESS;
    else
      ret=DMS_ERROR;

    local.SetSQLstr("DELETE FROM TRAIT WHERE TRAITID = 0 ");
    if (local.Execute())
      ret=DMS_SUCCESS;
    else
      ret=DMS_ERROR;

    local.SetSQLstr("DELETE FROM SCALE WHERE SCALEID = 0 ");
    if (local.Execute())
      ret=DMS_SUCCESS;
    else
      ret=DMS_ERROR;

    local.SetSQLstr("DELETE FROM TMETHOD WHERE TMETHID = 0 ");
    if (local.Execute())
      ret=DMS_SUCCESS;
    else
      ret=DMS_ERROR;


    POP();
}

void DLL_INTERFACE DMS_closeDatabase(void)
{
   LPCSTR szDB;
   LOG(">DMS_closeDatabase\n");
   if (_centralDMS)
   {
      delete _centralDMS;
      _centralDMS=NULL;
   }
   if (_localDMS)
   {
      szDB = _localDMS->DatabaseName();
      if (strcmp(szDB, "MySQL")==0 ) 
	       deleteDMSNull();
      delete _localDMS;
      _localDMS=NULL;
   }
   LOG("<DMS_closeDatabase\n");
}


void DLL_INTERFACE DMS_rollbackData(void)
{
   _centralDMS->Rollback();
   _localDMS->Rollback();
}

void DLL_INTERFACE DMS_commitData(void)
{
   LOG(">DMS_commitData\n");
   _centralDMS->Commit();
   _localDMS->Commit();
   LOG("<DMS_commitData\n");
}

void DLL_INTERFACE DMS_autoCommit(BOOL autoCommit)
{
   LOG(">DMS_autoCommit\n");
   _centralDMS->SetAutoCommit(autoCommit);
   _localDMS->SetAutoCommit(autoCommit);
   LOG("<DMS_autoCommit\n");
}


LONG deleteIMSNull(void)
{
    PUSH(deleteIMSNull); 
    CODBCbindedStmt &local  = _localIMS->BindedStmt("DELETE FROM IMS_LOT WHERE LOTID=0"); 
    if (local.Execute())
      ret=DMS_SUCCESS;
    else
      ret=DMS_ERROR;

	local.SetSQLstr("DELETE FROM IMS_TRANSACTION WHERE TRNID=0 AND LOTID =0 ");
    if (local.Execute())
      ret=DMS_SUCCESS;
    else
      ret=DMS_ERROR;

    POP();
}



void DLL_INTERFACE IMS_closeDatabase(void)
{
   LPCSTR szDB;
   LOG(">IMS_closeDatabase\n");
   if (_localIMS)
   {
      szDB = _localIMS->DatabaseName();
      if (strcmp(szDB, "MySQL")==0 ) 
	       deleteIMSNull();
      delete _localIMS;
      _localIMS=NULL;
   }
   LOG("<IMS_closeDatabase\n");
}






LONG DLL_INTERFACE I_openDatabase(LPCSTR szIniFile)
{  
   PUSH(IMS_openDatabase);

   CHAR szDSN[128], szUID[128]="Admin", szPWD[128]="";
   LOG("2.Open IMS Database\n");
   GetPrivateProfileString("IMS", "DSN", "", szDSN, sizeof(szDSN), szIniFile);
   GetPrivateProfileString("IMS", "UID", "", szUID, sizeof(szUID), szIniFile);
   GetPrivateProfileString("IMS", "PWD", "", szPWD, sizeof(szPWD), szIniFile);
#if defined(WEB)
   CheckWebService(szIniFile); 
#endif
   _localIMS = _env->Connect(szDSN,szUID,szPWD);
   if (!_localIMS->Connected())
   {
      IMS_closeDatabase();
      POP2(GMS_ERROR);
   }
   POP2(GMS_SUCCESS);
}



// ****  User name is encrypted ********/
LONG DLL_INTERFACE I_openDatabase2(LPCSTR szIniFile)
{  

   CHAR szDSN[128], szUID[128]="Admin", szPWD[128]="", szDEBUG[128], szDEBUGNO[]="NO";;
   CHAR  szEncryptUser[36];

   GetPrivateProfileString("CONFIGURATION", "DEBUG", "", szDEBUG, sizeof(szDEBUG), szIniFile);
   DEBUG = 0;
   if ( strcmp((char *) szDEBUG, (char *) szDEBUGNO) != 0)
      DEBUG = 1;
   PUSH(I_openDatabase2);
  //1. Open database
   LOG("1. Open IMS Database\n");
   GetPrivateProfileString("IMS", "DSN", "", szDSN, sizeof(szDSN), szIniFile);
   GetPrivateProfileString("IMS", "UID", "", szUID, sizeof(szUID), szIniFile);
   GetPrivateProfileString("IMS", "PWD", "", szPWD, sizeof(szPWD), szIniFile);

   /*** Encrypt the user ****/
   encryptPWD((CHAR *)szUID,(CHAR *)szEncryptUser);  
   _localIMS = _env->Connect(szDSN, szEncryptUser, szPWD);
  
   if (!_localIMS->Connected())
   {
      IMS_closeDatabase();
      POP2(GMS_ERROR);
   }
   POP2(GMS_SUCCESS);
}



// ****  Password is encrypted ********/
LONG DLL_INTERFACE I_openDatabase3(LPCSTR szIniFile)
{  

   CHAR szDSN[128], szUID[128]="Admin", szPWD[128]="", szDEBUG[128], szDEBUGNO[]="NO";;
   CHAR  szEncryptUser[36];

   GetPrivateProfileString("CONFIGURATION", "DEBUG", "", szDEBUG, sizeof(szDEBUG), szIniFile);
   DEBUG = 0;
   if ( strcmp((char *) szDEBUG, (char *) szDEBUGNO) != 0)
      DEBUG = 1;
   PUSH(I_openDatabase3);
  //1. Open database
   LOG("1. Open IMS Database\n");
   GetPrivateProfileString("IMS", "DSN", "", szDSN, sizeof(szDSN), szIniFile);
   GetPrivateProfileString("IMS", "UID", "", szUID, sizeof(szUID), szIniFile);
   GetPrivateProfileString("IMS", "PWD", "", szPWD, sizeof(szPWD), szIniFile);

   /*** Encrypt the password ****/
   encryptPWD((CHAR *)szPWD,(CHAR *)szEncryptUser);  
   _localIMS = _env->Connect(szDSN,szUID, szEncryptUser);

   
   if (!_localIMS->Connected())
   {
      IMS_closeDatabase();
      POP2(GMS_ERROR);
   }
   POP2(GMS_SUCCESS);
}


LONG DLL_INTERFACE IMS_openDatabase(LPCSTR szIniFile)
{  
	PUSH(IMS_openDatabase);
    CHAR szSecurity[5];
	int security = 1;
    GetPrivateProfileString("DLL SETTINGS", "SECURITY", "", szSecurity, sizeof(szSecurity), szIniFile);
	if (strlen(szSecurity) > 0 ) security = atoi(szSecurity);
	if (security ==4)   //Password is encrypted
		ret = I_openDatabase3(szIniFile);
	else if  (security ==3)   //Username is entrypted
		ret = I_openDatabase2(szIniFile);
	else
  	    ret = I_openDatabase(szIniFile);

    POP();

}

void DLL_INTERFACE IMS_autoCommit(BOOL autoCommit)
{
   LOG(">IMS_autoCommit\n");
//   _centralIMS->SetAutoCommit(autoCommit);
   _localIMS->SetAutoCommit(autoCommit);
   LOG("<IMS_autoCommit\n");
}


void DLL_INTERFACE IMS_commitData(void)
{
   LOG(">IMS_commitData\n");
   _localIMS->Commit();
   LOG("<GMS_commitData\n");
}


void DLL_INTERFACE IMS_rollbackData(void)
{
   _localIMS->Rollback();
}



//GEMS opendatabase
LONG DLL_INTERFACE GE_openDatabase(LPCSTR szIniFile)
{  

   PUSH(GE_openDatabase);
      CHAR szDSN[128], szUID[128]="Admin", szPWD[128]="", szDEBUG[128], szDEBUGNO[]="NO";;
   /**
   GetPrivateProfileString("CONFIGURATION", "DEBUG", "", szDEBUG, sizeof(szDEBUG), szIniFile);
   DEBUG = 0;
   if ( strcmp((char *) szDEBUG, (char *) szDEBUGNO) != 0)
      DEBUG = 1;
   **/

  //1. Open Central
   LOG("1. Open Central Database\n");
   GetPrivateProfileString("Central GEMS", "DSN", "", szDSN, sizeof(szDSN), szIniFile);
   GetPrivateProfileString("Central GEMS", "UID", "", szUID, sizeof(szUID), szIniFile);
   GetPrivateProfileString("Central GEMS", "PWD", "", szPWD, sizeof(szPWD), szIniFile);
   _centralGEMS = _env->Connect(szDSN,szUID,szPWD);
   if (!_centralGEMS->Connected())
   {
      GEMS_closeDatabase();
      POP2(GMS_ERROR);
   }
   //2. Open Local
   //LOG("2.Open Local Database\n");
   //GetPrivateProfileString("Local GEMS", "DSN", "", szDSN, sizeof(szDSN), szIniFile);
   //GetPrivateProfileString("Local GEMS", "UID", "", szUID, sizeof(szUID), szIniFile);
   //GetPrivateProfileString("Local GEMS", "PWD", "", szPWD, sizeof(szPWD), szIniFile);
   //_localGEMS = _env->Connect(szDSN,szUID,szPWD);
   //if (!_localGEMS->Connected())
   //{
   //   GEMS_closeDatabase();
   //   POP2(GMS_ERROR);
   //}
   POP2(GMS_SUCCESS);

   
}


// ****  User name is encrypted ********/
LONG DLL_INTERFACE GE_openDatabase2(LPCSTR szIniFile)
{  

   CHAR szDSN[128], szUID[128]="Admin", szPWD[128]="", szDEBUG[128], szDEBUGNO[]="NO";;
   CHAR  szEncryptUser[36];
   /**
   GetPrivateProfileString("DLL SETTINGS", "DEBUG", "", szDEBUG, sizeof(szDEBUG), szIniFile);
   DEBUG = 0;
   if ( strcmp((char *) szDEBUG, (char *) szDEBUGNO) != 0)
      DEBUG = 1;
   **/
   PUSH(GE_openDatabase2);
  //1. Open database
   LOG("1. Open GEMS Database\n");
   GetPrivateProfileString("Central GEMS", "DSN", "", szDSN, sizeof(szDSN), szIniFile);
   GetPrivateProfileString("Central GEMS", "UID", "", szUID, sizeof(szUID), szIniFile);
   GetPrivateProfileString("Central GEMS", "PWD", "", szPWD, sizeof(szPWD), szIniFile);

   /*** Encrypt the user ****/
   encryptPWD((CHAR *)szUID,(CHAR *)szEncryptUser);  
   _centralGEMS = _env->Connect(szDSN, szEncryptUser, szPWD);
  
   if (!_centralGEMS->Connected())
   {
      GEMS_closeDatabase();
      POP2(GMS_ERROR);
   }
   POP2(GMS_SUCCESS);
}


// ****  Password is encrypted ********/
LONG DLL_INTERFACE GE_openDatabase3(LPCSTR szIniFile)
{  

   CHAR szDSN[128], szUID[128]="Admin", szPWD[128]="", szDEBUG[128], szDEBUGNO[]="NO";;
   CHAR  szEncryptUser[36];
   /**
   GetPrivateProfileString("DLL SETTINGS", "DEBUG", "", szDEBUG, sizeof(szDEBUG), szIniFile);
   DEBUG = 0;
   if ( strcmp((char *) szDEBUG, (char *) szDEBUGNO) != 0)
      DEBUG = 1;
   **/
   PUSH(GE_openDatabase3);
  //1. Open database
   LOG("1. Open GEMS Database\n");
   GetPrivateProfileString("Central GEMS", "DSN", "", szDSN, sizeof(szDSN), szIniFile);
   GetPrivateProfileString("Central GEMS", "UID", "", szUID, sizeof(szUID), szIniFile);
   GetPrivateProfileString("Central GEMS", "PWD", "", szPWD, sizeof(szPWD), szIniFile);

   /*** Encrypt the password ****/
   encryptPWD((CHAR *)szPWD,(CHAR *)szEncryptUser);  
   _centralGEMS = _env->Connect(szDSN,szUID, szEncryptUser);

   
   if (!_centralGEMS->Connected())
   {
      GEMS_closeDatabase();
      POP2(GMS_ERROR);
   }
   POP2(GMS_SUCCESS);
}

LONG DLL_INTERFACE GEMS_openDatabase(LPCSTR szIniFile)
{  
	PUSH(GEMS_openDatabase);
    CHAR szSecurity[5];
	int security = 1;
    GetPrivateProfileString("DLL SETTINGS", "SECURITY", "", szSecurity, sizeof(szSecurity), szIniFile);
	if (strlen(szSecurity) > 0 ) security = atoi(szSecurity);
	if (security ==4)   //Password is encrypted
		ret = GE_openDatabase3(szIniFile);
	else if  (security ==3)   //Username is entrypted
		ret = GE_openDatabase2(szIniFile);
	else
  	    ret = GE_openDatabase(szIniFile);

    POP();

}

LONG deleteGEMSNull(BOOL flocal)
{
    PUSH(deleteGEMSNull); 
	POP();
}

void DLL_INTERFACE GEMS_closeDatabase(void)
{
   LPCSTR szDB;
   LOG(">GEMS_closeDatabase\n");
   if (_centralGEMS)
   {
      szDB = _centralGEMS->DatabaseName();
      if (strcmp(szDB, "MySQL")==0 ) 
	       deleteGEMSNull(CENTRAL);
      delete _centralGEMS;
      _centralGEMS=NULL;
   }
   //if (_localGEMS)
   //{
   //   szDB = _localGEMS->DatabaseName();
   //   if (strcmp(szDB, "MySQL")==0 ) 
	  //     deleteGEMSNull(LOCAL);
   //   delete _localGEMS;
   //   _localGEMS=NULL;
   //}
   LOG("<GEMS_closeDatabase\n");
}


void DLL_INTERFACE GEMS_autoCommit(BOOL autoCommit)
{
   LOG(">GEMS_autoCommit\n");
   _centralGEMS->SetAutoCommit(autoCommit);
   LOG("<GEMS_autoCommit\n");
}


void DLL_INTERFACE GEMS_commitData(void)
{
   LOG(">GEMS_commitData\n");
   _centralGEMS->Commit();
   LOG("<GEMS_commitData\n");
}

