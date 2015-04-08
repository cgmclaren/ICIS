#include "ipms.h"

extern LONG UACCESS,USERID,USTATUS;
extern CODBCdbc *_localIPMS;  //Central and Local Database Connection
extern stack<char*> _routines_stack;


LONG IPMS_getMTA(BOOL fLocal,DMS_SCALE* Scales, long fOpt)
{
  return true;
}



/**************************
//////////////////////////////////////////////////////////////////////////////
//  Finds the maximum  ipms_id
//////////////////////////////////////////////////////////////////////////////
LONG getNextIPMSID(void)
{
static LONG ipms_ID=0;
      ipms_ID=0;
      CODBCdirectStmt local = _localIPMS->DirectStmt("SELECT MAX(IPMS_ID) FROM IPMS");
      local.Execute();
      local.Fetch();
      if (!local.NoData()) {
         ipms_ID=local.Field(1).AsInteger();
      }
      return ++ipms_ID;
}



//////////////////////////////////////////////////////////////////////////////
//  Finds the maximum  protection id
//////////////////////////////////////////////////////////////////////////////
LONG getNextProtID(void)
{
static LONG _ID=0;
      _ID=0;
      CODBCdirectStmt local = _localIPMS->DirectStmt("SELECT MAX(PROT_ID) FROM PROTECTION");
      local.Execute();
      local.Fetch();
      if (!local.NoData()) {
         _ID=local.Field(1).AsInteger();
      }
      return ++_ID;
}


//////////////////////////////////////////////////////////////////////////////
// Adds a record in the IPMS table
//////////////////////////////////////////////////////////////////////////////
LONG addIPMS(IPMS_IPMS &data)
{
#define SQL_ADDIPMS "\
    INSERT INTO IPMS ( IPMS_ID, EFFECTIVE_DATE, EXPIRY_DATE, VERSION_ID, REMARK ) \
      values(?,?,?,?,?)"

   PUSH(addIPMS);

static CODBCbindedStmt &local  = _localIPMS->BindedStmt(SQL_ADDIPMS);

static IPMS_IPMS _data;
static BOOL first_time=TRUE;
static LONG gid;

   ZeroMemory(&_data,sizeof(data));
   _data = data;
   
   if (first_time)
   {
      local.BindParam(1,_data.IPMS_ID);
      local.BindParam(2,_data.EFFECTIVE_DATA);
      local.BindParam(3,_data.EXPIRY_DATE);
      local.BindParam(4,_data.VERSION_ID);
	  local.BindParam(5, _data.REMARK, sizeof(_data.REMARK));
      first_time=FALSE;
   }

   data.IPMS_ID = getNextIPMSID();
   _data = data;
   if (local.Execute())
      ret=IPMS_SUCCESS;
   else
      ret=IPMS_ERROR;

   POP();

#undef SQL_ADDIPMS
}

//////////////////////////////////////////////////////////////////////////////
// Adds a record in the Protection table
//////////////////////////////////////////////////////////////////////////////
LONG addProtection(IPMS_PROT &data)
{
#define SQL_ADDPROT "\
    INSERT INTO PROTECTION (PROT_ID, IPMS_ID, SUPERSEDED_BY, ACESSION_ID, TYPE, OWNER, CHECK_PRIMARY_DOC ) \
      values(?,?,?,?,?)"

   PUSH(addProtection);

static CODBCbindedStmt &local  = _localIPMS->BindedStmt(SQL_ADDPROT);

static IPMS_PROT _data;
static BOOL first_time=TRUE;
static LONG gid;

   ZeroMemory(&_data,sizeof(data));
   _data = data;
   
   if (first_time)
   {
      local.BindParam(1,_data.PROTID);
      local.BindParam(2,_data.IPMS_ID);
      local.BindParam(3,_data.SUPERSEDED_BY);
      local.BindParam(4,_data.ACCESSION_ID, sizeof(_data.ACCESSION_ID));
	  local.BindParam(5, _data.TYPE, sizeof(_data.TYPE));
	  local.BindParam(6, _data.OWNER);
	  local.BindParam(7, _data.CHECK_PRIM_DOC, sizeof(_data.CHECK_PRIM_DOC));

      first_time=FALSE;
   }

   data.IPMS_ID = getNextIPMSID();
   _data = data;
   if (local.Execute())
      ret=IPMS_SUCCESS;
   else
      ret=IPMS_ERROR;

   POP();

#undef SQL_ADDPROT
}



******************/