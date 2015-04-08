/**************************************************************
 Programmed by   :   Arllet M. Portugal
 Modified  (AMP) :   
 **************************************************************/

#include "Persons.h"

extern LONG UACCESS,USERID;
extern CODBCdbc *_centralDMS,*_localDMS;  //Central and Local Database Connection
extern stack<char*> _routines_stack;


//////////////////////////////////////////////////////////////////////////////
//  Finds the minimum  labelid
//////////////////////////////////////////////////////////////////////////////
LONG getNextPersonID(void)
{
static LONG personID=0;

   if (!personID)
   {
      CODBCdirectStmt local = _localDMS->DirectStmt("SELECT MIN(PERSONID) FROM PERSONS");
      local.Execute();
      local.Fetch();
      if (!local.NoData())
         personID=local.Field(1).AsInteger();
   }
   return --personID;
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG addPerson(SUP_PERSONS &data)
{
#define SQL_ADDPERSON "INSERT INTO PERSONS (PERSONID, FNAME, LNAME, IONAME, INSTITID, PTITLE, PONAME, PLANGU, " \
                    " PPHONE, PEXTENT, PFAX, PEMAIL, PROLE, SPERSON, EPERSON, PSTATUS, CONTACT ) VALUES " \
		            " (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?) "
	
	PUSH(addPersons);

static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_ADDPERSON);

static SUP_PERSONS _data;
static BOOL first_time=TRUE;

   data.PERSONID = getNextPersonID();
   _data = data; 
   if (first_time)
   {
      local.BindParam(1,_data.PERSONID);
	  local.BindParam(2,_data.FNAME, 20);
	  local.BindParam(3,_data.LNAME,20);
      local.BindParam(4,_data.IONAME,15);
	  local.BindParam(5,_data.INSTITID);
	  local.BindParam(6,_data.PTITLE,25);
	  local.BindParam(7,_data.PONAME,50);
	  local.BindParam(8,_data.PLANGU);
      local.BindParam(9,_data.PPHONE,50);
      local.BindParam(10,_data.PEXTENT,20);
      local.BindParam(11,_data.PFAX,20);
	  local.BindParam(12,_data.PEMAIL,40);
	  local.BindParam(13,_data.PROLE);
	  local.BindParam(14,_data.SPERSON);
	  local.BindParam(15,_data.EPERSON);
	  local.BindParam(16,_data.PSTATUS);
	  local.BindParam(17,_data.CONTACT,255);

      first_time=FALSE;
   }


   if (local.Execute())
      ret=DMS_SUCCESS;
   else
      ret=DMS_ERROR;


   POP();

#undef SQL_ADDPERSON
}



//////////////////////////////////////////////////////////////////////////////
// 
// 
//////////////////////////////////////////////////////////////////////////////
LONG getPerson(BOOL fLocal,SUP_PERSONS &data, int fOpt)
{
	#define SQL_GETPERSON "SELECT PERSONS.PERSONID, PERSONS.FNAME, PERSONS.LNAME,  PERSONS.IONAME, PERSONS.INSTITID, PERSONS.PTITLE,  " \
                          " PERSONS.PONAME, PERSONS.PLANGU, PERSONS.PPHONE, PERSONS.PEXTENT, PERSONS.PFAX , " \
						  " PERSONS.PEMAIL, PERSONS.PROLE, PERSONS.SPERSON, PERSONS.EPERSON, PERSONS.PSTATUS, PERSONS.CONTACT " \
	                      " FROM  PERSONS  " \
                          " WHERE PERSONS.PERSONID = ? OR 0=?  " \
						  " ORDER BY PERSONS.LNAME"
  
	PUSH(getPerson);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_GETPERSON);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_GETPERSON);
       CODBCbindedStmt *source;

static SUP_PERSONS _data;
static LONG personid;
static BOOL first_time=TRUE;

//Parameters
   if (fOpt == FIND_FIRST) {
      personid = data.PERSONID;
      if (first_time)
	  {
		BINDPARAM(1, personid);
		BINDPARAM(2, personid);
												
        BIND(1,_data.PERSONID);
  	    BINDS(2,_data.FNAME, 20);
	    BINDS(3,_data.LNAME,20);
        BINDS(4,_data.IONAME,15);
	    BIND(5,_data.INSTITID);
	    BINDS(6,_data.PTITLE,25);
	    BINDS(7,_data.PONAME,50);
	    BIND(8,_data.PLANGU);
        BINDS(9,_data.PPHONE,50);
        BINDS(10,_data.PEXTENT,20);
        BINDS(11,_data.PFAX,20);
	    BINDS(12,_data.PEMAIL,40);
	    BIND(13,_data.PROLE);
	    BIND(14,_data.SPERSON);
	    BIND(15,_data.EPERSON);
	    BIND(16,_data.PSTATUS);
	    BINDS(17,_data.CONTACT,255);

        first_time = FALSE;
	  }
     local.Execute();
     central.Execute();
   }

   source = (fLocal)?&local:&central;
   if (source->Fetch())
   {
      ret=GMS_SUCCESS;
	  data = _data;
   }
   else if (source->NoData())
      ret = DMS_NO_DATA;
   else 
      ret = DMS_ERROR;

   POP();

#undef SQL_GETPERSON
}



//////////////////////////////////////////////////////////////////////////////
// 
// 
//////////////////////////////////////////////////////////////////////////////
LONG getPersonInst(BOOL fLocal,long institid, char fname[24], char lname[52], char insacr[24],char title[28], long* personid, long fOpt)
{
	#define SQL_GETPERSONInst "SELECT PERSONS.FNAME, PERSONS.LNAME,  INSTITUT.INSACR, PERSONS.PTITLE, PERSONS.PERSONID  " \
	                      " FROM INSTITUT , PERSONS  " \
                          " WHERE (INSTITUT.PINSID=? or 0=?) AND  INSTITUT.INSTITID = PERSONS.INSTITID " \
						  " ORDER BY PERSONS.LNAME"
  
	PUSH(getPersonInst);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_GETPERSONInst);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_GETPERSONInst);
       CODBCbindedStmt *source;

static SUP_PERSONS _data;
static LONG tinstitid=0,tpersonid=0;
static char cfname[24], clname[52],  cinsacr[24],  ctitle[28];
static char lfname[24], llname[52],  linsacr[24],  ltitle[28];
static BOOL first_time=TRUE;

//Parameters
   if (fOpt == FIND_FIRST) {
      tinstitid = institid;
      if (first_time)
	  {
		BINDPARAM(1, institid);
		BINDPARAM(2, institid);
												
  	    BINDS(1,cfname, 20);
		BINDS(2,clname,52);
		BINDS(3,cinsacr,24);
		BINDS(4,ctitle,28);
		BIND(5,tpersonid);


        first_time = FALSE;
	  }
     local.Execute();
     central.Execute();
   }

   source = (fLocal)?&local:&central;
   if (source->Fetch())
   {
      ret=DMS_SUCCESS;
      strncpy(fname, cfname, 24);
	  strncpy(lname, clname, 52);
	  strncpy(insacr,cinsacr,24);
	  strncpy(title, ctitle, 28);
	  *personid = tpersonid;
   }
   else if (source->NoData())
      ret = DMS_NO_DATA;
   else 
      ret = DMS_ERROR;

   POP();

#undef SQL_GETPERSONInst
}


LONG getInstitutePrnt(BOOL fLocal,SUP_INSTITUT &data, long fOpt)
{
	#define SQL_GETINSTITUTPRNT "SELECT INSTITID, INSNAME, INSACR, INSTYPE, STREET, POSTBOX, CITY, STATEID, " \
	                      " CPOSTAL, CNTRYID, APHONE, AFAX, AEMAIL, WEBURL, SINS, EINS, ICHANGE " \
	                      " FROM  INSTITUT  " \
                          " WHERE  PINSID = ? or 0 = ?"
  
	PUSH(getPersonInst);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_GETINSTITUTPRNT);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_GETINSTITUTPRNT);
       CODBCbindedStmt *source;

static SUP_INSTITUT _data;
static LONG tinstitid=0;
static BOOL first_time=TRUE;

//Parameters
   if (fOpt == FIND_FIRST) {
      tinstitid = data.PINSID;
	  _data = data;
      if (first_time)
	  {
		BINDPARAM(1, tinstitid);
		BINDPARAM(2, tinstitid);
												
  	    BIND(1,_data.INSTITID);
		BINDS(2,_data.INSNAME,152);
		BINDS(3,_data.INSACR,24);
		BIND(4,_data.INSTYPE);
		BINDS(5,_data.STREET,128);
		BINDS(6,_data.POSTBOX,128);
		BINDS(7,_data.CITY,36);
		BIND(8,_data.STATEID);
		BINDS(9,_data.CPOSTAL,12);
		BIND(10,_data.CNTRYID);
		BINDS(11,_data.APHONE,28);
		BINDS(12, _data.AFAX,28);
		BINDS(13,_data.AEMAIL,44);
		BINDS(14,_data.WEBURL,64);
		BIND(15,_data.SINS);
		BIND(16,_data.EINS);
		BIND(17,_data.ICHANGE);


        first_time = FALSE;
	  }
     local.Execute();
     central.Execute();
   }

   source = (fLocal)?&local:&central;
   if (source->Fetch())
   {
      ret=DMS_SUCCESS;
      data = _data;   
   }
   else if (source->NoData())
      ret = DMS_NO_DATA;
   else 
      ret = DMS_ERROR;

   POP();

#undef SQL_GETINSTITUTPRNT
}




LONG DLL_INTERFACE SUP_addPerson( SUP_PERSONS* person)
{
	PUSH(SUP_addPerson);
	ret = addPerson(*person);
	POP();
}

LONG DLL_INTERFACE SUP_getPerson(SUP_PERSONS* data,long fOpt)
{
   static BOOL fLocal=TRUE;

   PUSH(SUP_getPerson);

   if (fOpt==FIND_FIRST)
   {
      fLocal=TRUE;
   }
   if (fLocal){
      ret=getPerson(LOCAL,*data,fOpt);
      if (ret!=DMS_SUCCESS){
         fLocal = FALSE; 
         ret=getPerson(CENTRAL,*data, FIND_NEXT);
      }
   }
   else
      ret=getPerson(CENTRAL,*data,fOpt);
   POP();
}


LONG DLL_INTERFACE SUP_getPersonInst(long institid, char fname[24], char lname[52], char insacr[24],char title[28], long* personid, long fOpt)
{
   static BOOL fLocal=TRUE;

   PUSH(SUP_getPerson);

   if (fOpt==FIND_FIRST)
   {
      fLocal=TRUE;
   }
   if (fLocal){
      ret=getPersonInst(LOCAL,institid,  fname, lname,  insacr,title, personid, fOpt);
      if (ret!=DMS_SUCCESS){
         fLocal = FALSE; 
         ret=getPersonInst(CENTRAL,institid,  fname, lname,  insacr,title, personid, fOpt);
      }
   }
   else
      ret=getPersonInst(CENTRAL,institid,  fname, lname,  insacr,title, personid, fOpt);
   POP();
}


LONG DLL_INTERFACE SUP_getInstitutePrnt(SUP_INSTITUT* instit, long fOpt)
{
   static BOOL fLocal=TRUE;

   PUSH(SUP_getPerson);

   if (fOpt==FIND_FIRST)
   {
      fLocal=TRUE;
   }
   if (fLocal){
      ret=getInstitutePrnt(LOCAL,*instit, fOpt);
      if (ret!=DMS_SUCCESS){
         fLocal = FALSE; 
         ret=getInstitutePrnt(CENTRAL,*instit, fOpt);
      }
   }
   else
      ret=getInstitutePrnt(CENTRAL,*instit, fOpt);
   POP();
}

