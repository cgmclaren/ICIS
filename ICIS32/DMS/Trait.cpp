/* Trait.cpp : Implements the functions to access and manipulate the TRAIT,SCALE,TMETHOD tables of ICIS
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
 Programmed by   :   Arllet M. Portugal
 Modified  (AMP) :   November 4, 2009
 **************************************************************/


#include "Trait.h"

extern LONG UACCESS,USERID;
extern CODBCdbc *_centralDMS,*_localDMS;  //Central and Local Database Connection
extern CODBCdbc *_centralDBC,*_localDBC;  //Central and Local Database Connection
extern stack<char*> _routines_stack;
extern bool HAS_SESSION;

//////////////////////////////////////////////////////////////////////////////
//  Finds the minimum  traitid
//////////////////////////////////////////////////////////////////////////////
LONG getNextTraitID(void)
{
static LONG traitID=0;

   //if (!traitID)
   //{
      CODBCdirectStmt local = _localDMS->DirectStmt("SELECT MIN(TRAITID) FROM TRAIT");
      local.Execute();
      local.Fetch();
      if (!local.NoData())
         traitID=local.Field(1).AsInteger();
   //}
   return --traitID;
}

//////////////////////////////////////////////////////////////////////////////
//  Finds the minimum  TID
//////////////////////////////////////////////////////////////////////////////
LONG getNextTID(void)
{
static LONG tID=0;

   if (!tID)
   {
      CODBCdirectStmt local = _localDMS->DirectStmt("SELECT MIN(TID) FROM TRAIT");
      local.Execute();
      local.Fetch();
      if (!local.NoData())
         tID=local.Field(1).AsInteger();
   }
   return --tID;
}



//////////////////////////////////////////////////////////////////////////////
//  Finds the minimum  scaleid
//////////////////////////////////////////////////////////////////////////////
LONG getNextScaleID(void)
{
static LONG scaleID=0;

   if (!scaleID)
   {
      CODBCdirectStmt local = _localDMS->DirectStmt("SELECT MIN(SCALEID) FROM SCALE");
      local.Execute();
      local.Fetch();
      if (!local.NoData())
         scaleID=local.Field(1).AsInteger();
   }
   return --scaleID;
}

//////////////////////////////////////////////////////////////////////////////
//  Finds the minimum  tmethodid
//////////////////////////////////////////////////////////////////////////////
LONG getNextTMethodID(void)
{
static LONG tmethID=0;

   if (!tmethID)
   {
      CODBCdirectStmt local = _localDMS->DirectStmt("SELECT MIN(TMETHID) FROM TMETHOD");
      local.Execute();
      local.Fetch();
      if (!local.NoData())
         tmethID=local.Field(1).AsInteger();
   }
   return --tmethID;
}


LONG addTrait( DMS_TRAIT* Tra, CHAR* szPTra)
{
#define SQL_ADDTRAIT "INSERT INTO TRAIT (TRAITID,TRNAME,TRABBR,TRDESC) VALUES (?,?,?,?)"
   PUSH(addTrait);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_ADDTRAIT);
   static   DMS_TRAIT n;
   static   PSTR pnext;
   static  int p;
   static BOOL first_time=TRUE;


   pnext = (szPTra);
   Tra->TRAITID = getNextTraitID();
   n = *Tra;
   local.BindParam(1, n.TRAITID); 
   local.BindParam(2, n.TRNAME, DMS_TRAIT_NAME);  
   local.BindParam(3, n.TRABBR, DMS_TRAIT_ABBR);  
   local.BindParam(4, pnext,sizeof(pnext));  
   *Tra = n;

   if (local.Execute())
      ret=DMS_SUCCESS;
   else
      ret=DMS_ERROR;

   POP();

#undef SQL_ADDTRAIT
}


// version 4
LONG addTrait_v4( DMS_TRAIT* Tra, CHAR* szPTra)
{
#define SQL_ADDTRAIT_v4 "INSERT INTO TRAIT (TID,TRAITID,TRNAME,TRABBR,TRDESC,SCALEID,TMETHID,TNSTAT) VALUES (?,?,?,?,?,?,?,?)"
   PUSH(addTrait);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_ADDTRAIT_v4);
   static   DMS_TRAIT n;
   static   PSTR pnext;
   static  int p;
   static BOOL first_time=TRUE;


   pnext = (szPTra);
   Tra->TID = getNextTID();
   if (Tra->TRAITID==0)
       Tra->TRAITID = getNextTraitID();
   n = *Tra;
   PadNull((CHAR *) n.TRNAME, DMS_TRAIT_NAME);
   PadNull((CHAR *) n.TRABBR, DMS_TRAIT_ABBR);
   if (first_time) {                       //2003/07/7
     local.BindParam(1,n.TID);
     local.BindParam(2, n.TRAITID); 
     local.BindParam(3, n.TRNAME, DMS_TRAIT_NAME-2);  
     local.BindParam(4, n.TRABBR, DMS_TRAIT_ABBR-2);  
     local.BindParam(5, pnext,sizeof(pnext));  
     local.BindParam(6, n.SCALEID);
     local.BindParam(7, n.TMETHID);
     local.BindParam(8, n.TNSTAT);
   }  
   *Tra = n;

   if (local.Execute())
      ret=DMS_SUCCESS;
   //else
   //   ret=DMS_ERROR;

   else {

	   if (ret!=-12) //GMS_UNIQUE_CONSTRAINT_ERROR
		   ret=GMS_ERROR;  
       // Handler for unique contraint error with the assumption that only TID has unique contraint 
	   //It is assumed if there is Session ID, there is one to one correspondence between traitid and tid

	   else if (HAS_SESSION) {
		   int i=0;
		   while ((ret==-12) && (i<100)) {
                 n.TRAITID = getNextTraitID();
				ret=local.Execute();
				++i;
		   }
		   if ((ret) &&(ret!=-12)) {
			   Tra->TRAITID =n.TRAITID ;
			   ret =DMS_SUCCESS;
		   }
		   else {
			   Tra->TRAITID = 0;
			   ret =DMS_ERROR;
		   }
	   }
   }


   POP();

#undef SQL_ADDTRAIT_v4
}

LONG addScale( DMS_SCALE* Scal)
{
#define SQL_ADDSCALE "INSERT INTO SCALE (SCALEID,SCTYPE,SCNAME, TRAITID) VALUES (?,?,?,?)"

   PUSH(addScale);
   static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_ADDSCALE);
   static   DMS_SCALE n;
   static   long nnext, ndlen, nnlen, tmp=0;
   static BOOL first_time=TRUE;

   Scal->SCALEID = getNextScaleID();
   n = *Scal;
   PadNull((CHAR *) n.SCNAME, DMS_SCALE_NAME);
   PadNull((CHAR *) n.SCTYPE, DMS_SCALE_TYPE);
   if (first_time) {                       //2003/07/7
      local.BindParam(1,n.SCALEID); 
      local.BindParam(2,n.SCTYPE, DMS_SCALE_TYPE-3);  
      local.BindParam(3,n.SCNAME, DMS_SCALE_NAME-2);  
      local.BindParam(4,n.TRAITID); 
   }

   if (local.Execute())
      ret=DMS_SUCCESS;
   else
      ret=DMS_ERROR;

   POP();

#undef SQL_ADDSCALE
}

LONG addTmethod( DMS_TMETHOD* Tme, CHAR* szPTme)
{
#define SQL_ADDTMETHOD "INSERT INTO TMETHOD (TMETHID,TMNAME,TMABBR,TMDESC,TRAITID) VALUES (?,?,?,?,?)"

   PUSH(addTmethod);
   static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_ADDTMETHOD);
   static   DMS_TMETHOD n;
   static   PSTR pnext;
   static char tmname[DMS_METHOD_NAME -2], tmabbr[DMS_METHOD_ABBR-2];
   static BOOL first_time=TRUE;

   Tme->TMETHID = getNextTMethodID(); 
   n = *Tme;
   pnext = (szPTme);
//   strcpy((char *) tmname, (char *) Tme->TMNAME);
//   strcpy((char *) tmabbr, (char *) Tme->TMABBR);
   PadNull((CHAR *) n.TMNAME, DMS_METHOD_NAME);
   PadNull((CHAR *) n.TMABBR, DMS_METHOD_ABBR);
   if (first_time) {                       //2003/07/7
     local.BindParam(1, n.TMETHID);   
     local.BindParam(2, n.TMNAME, strlen((CHAR *)n.TMNAME));  
     local.BindParam(3, n.TMABBR, strlen((CHAR *)n.TMABBR));  
     local.BindParam(4, pnext, sizeof(pnext));  
     local.BindParam(5, n.TRAITID); 
   }

   if (local.Execute())
      ret=DMS_SUCCESS;
   else
      ret=DMS_ERROR;

   POP();

#undef SQL_ADDTMETHOD
}

LONG getTrait(BOOL fLocal,DMS_TRAIT* Traits, UCHAR* szMDesc, LONG cMDesc, int fOpt)
{
#define SQL_GETTRAIT "SELECT  TRAITID,TRNAME,TRABBR,TRDESC \
							FROM TRAIT \
							WHERE (TRAITID=? OR 0=?) ORDER BY TRNAME" 

  LONG ret=DMS_SUCCESS;

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_GETTRAIT);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_GETTRAIT);
       CODBCbindedStmt *source;

static DMS_TRAIT _data;
static CHAR szDesc[DMS_MAX_STR];
static LONG traitid;
static BOOL first_time=TRUE;

//Parameters
static LONG _gid;
   if (fOpt == FIND_FIRST) {
      if (first_time)
	  {
		BINDPARAM(1, traitid);
		BINDPARAM(2, traitid);
												
        BIND(1, _data.TRAITID);		                   
        BINDS(2, _data.TRNAME, DMS_TRAIT_NAME);
		BINDS(3, _data.TRABBR, DMS_TRAIT_ABBR); 
		BINDS(4, szDesc, DMS_MAX_STR);

      first_time = FALSE;
	  }
     traitid=Traits->TRAITID;
     local.Execute();
     central.Execute();
   }

   source = (fLocal)?&local:&central;
   if (source->Fetch())
   {
      ret=GMS_SUCCESS;
	  *Traits = _data;
	  if (sizeof(szDesc) < cMDesc)
         strncpy((CHAR*) szMDesc,szDesc,sizeof(szDesc));
      else
         strncpy((CHAR*) szMDesc,szDesc,cMDesc);
   }
   else if (source->NoData())
      ret = DMS_NO_DATA;
   else 
      ret = DMS_ERROR;

   return ret;

#undef SQL_GETTRAIT 
}



LONG getTrait_v4(BOOL fLocal,DMS_TRAIT* Traits, UCHAR* szMDesc, LONG cMDesc, int fOpt)
{
#define SQL_GETTRAIT_v4 "SELECT  TID, TRAITID, TRNAME,TRABBR, SCALEID, TMETHID, TRDESC, TNSTAT \
							FROM TRAIT \
							WHERE (TRAITID=? OR 0=?) ORDER BY TRNAME" 

  LONG ret=DMS_SUCCESS;

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_GETTRAIT_v4);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_GETTRAIT_v4);
       CODBCbindedStmt *source;

static DMS_TRAIT _data;
static CHAR szDesc[DMS_MAX_STR]; 
static LONG traitid;
static BOOL first_time=TRUE;

//Parameters
static LONG _gid;
   if (fOpt == FIND_FIRST) {
      if (first_time)
	  {
		BINDPARAM(1, traitid);
		BINDPARAM(2, traitid);
												
        BIND(1, _data.TID);		 
		BIND(2, _data.TRAITID);
        BINDS(3, _data.TRNAME, DMS_TRAIT_NAME);
		BINDS(4, _data.TRABBR, DMS_TRAIT_ABBR); 
		BIND(5, _data.SCALEID);
		BIND(6, _data.TMETHID);
		BINDS(7, szDesc, DMS_MAX_STR);  
		BIND(8, _data.TNSTAT);


      first_time = FALSE;
	  }
     traitid=Traits->TRAITID;
     local.Execute();
     central.Execute();
   }

   source = (fLocal)?&local:&central;
   if (source->Fetch())
   {
      ret=GMS_SUCCESS;
	  *Traits = _data;
	  if (sizeof(szDesc) < cMDesc)
         strncpy((CHAR*) szMDesc,szDesc,sizeof(szDesc));
      else
         strncpy((CHAR*) szMDesc,szDesc,cMDesc);
   }
   else if (source->NoData())
      ret = DMS_NO_DATA;
   else 
      ret = DMS_ERROR;

   return ret;

#undef SQL_GETTRAIT_v4
}

// for version 4
LONG getPrefTrait(BOOL fLocal,DMS_TRAIT* Traits, UCHAR* szMDesc, LONG cMDesc, int fOpt)
{
#define SQL_GETPREFTRAIT "SELECT  TID, TRAITID, TRNAME,TRABBR, TNSTAT, SCALEID, TMETHID, TRDESC \
							FROM TRAIT \
							WHERE (TRAITID=? OR 0=?) AND (TNSTAT=1) ORDER BY TRNAME" 

  LONG ret=DMS_SUCCESS;

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_GETPREFTRAIT);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_GETPREFTRAIT);
       CODBCbindedStmt *source;

static DMS_TRAIT _data;
static CHAR szDesc[MAX_STR];
static LONG traitid,tnstat;
static BOOL first_time=TRUE;

//Parameters
static LONG _gid;
   if (fOpt == FIND_FIRST) {
      if (first_time)
	  {
		BINDPARAM(1, traitid);
		BINDPARAM(2, tnstat);
												
        BIND(1, _data.TID);		 
		BIND(2, _data.TRAITID);
        BINDS(3, _data.TRNAME, DMS_TRAIT_NAME);
		BINDS(4, _data.TRABBR, DMS_TRAIT_ABBR); 
		BIND(5, _data.TNSTAT);
		BIND(6, _data.SCALEID);
		BIND(7, _data.TMETHID);
		BINDS(8, szDesc, MAX_STR);

      first_time = FALSE;
	  }
     traitid=Traits->TRAITID;
	 tnstat = 1;
     local.Execute();
     central.Execute();
   }

   source = (fLocal)?&local:&central;
   if (source->Fetch())
   {
      ret=GMS_SUCCESS;
	  *Traits = _data;
	  if (sizeof(szMDesc) < cMDesc)
         strncpy((CHAR*) szMDesc,szDesc,sizeof(szMDesc));
      else
         strncpy((CHAR*) szMDesc,szDesc,cMDesc);
   }
   else if (source->NoData())
      ret = DMS_NO_DATA;
   else 
      ret = DMS_ERROR;

   return ret;

#undef SQL_GETPREFTRAIT
}



LONG getScale(BOOL fLocal,DMS_SCALE* Scales, long fOpt)
{
#define SQL_GETSCALE "SELECT SCALEID,TRAITID,SCTYPE,SCNAME "\
							"FROM SCALE "\
							"WHERE (SCALEID=? OR 0=?)" 

  LONG ret=DMS_SUCCESS;

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_GETSCALE);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_GETSCALE);
       CODBCbindedStmt *source;

static DMS_SCALE _data;
static CHAR szDesc[MAX_STR];
static LONG scaleid;
static BOOL first_time=TRUE;

//Parameters
static LONG _gid;

   if (fOpt==FIND_FIRST) {
      if (first_time)
	  {
		BINDPARAM(1, scaleid);
		BINDPARAM(2, scaleid);
												
        BIND(1, _data.SCALEID);		                   
        BIND(2, _data.TRAITID);
		BINDS(3, _data.SCTYPE, DMS_SCALE_TYPE); 
		BINDS(4, _data.SCNAME, DMS_SCALE_NAME);

      first_time = FALSE;
	  }
      scaleid=Scales->SCALEID;
      local.Execute();
	  central.Execute();
   }

   source = (fLocal)?&local:&central;

   if (source->Fetch())
   {
      ret=DMS_SUCCESS;
	  *Scales = _data;
   }
   else if (source->NoData())
      ret = DMS_NO_DATA;
   else 
      ret = DMS_ERROR;

   return ret;//POP();

#undef SQL_GETSCALE 
}


LONG getScale2(BOOL fLocal,DMS_SCALE* Scales, long fOpt)
{
#define SQL_GETSCALE2 "SELECT SCALEID,TRAITID,SCTYPE,SCNAME FROM SCALE WHERE (TRAITID=?)" 

  LONG ret=DMS_SUCCESS;

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_GETSCALE2);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_GETSCALE2);
       CODBCbindedStmt *source;

static DMS_SCALE _data;
static CHAR szDesc[MAX_STR];
static LONG traitid;
static BOOL first_time=TRUE;

//Parameters
static LONG _gid;

   if (fOpt == FIND_FIRST) {
     if (first_time)
	 {
		BINDPARAM(1, traitid);
												
        BIND(1, _data.SCALEID);		                   
        BIND(2, _data.TRAITID);
		BINDS(3, _data.SCTYPE, DMS_SCALE_TYPE); 
		BINDS(4, _data.SCNAME, DMS_SCALE_NAME);

      first_time = FALSE;
	 }
     traitid=Scales->TRAITID;
     local.Execute();
	 central.Execute();
   }  
   source = (fLocal)?&local:&central;

   if (source->Fetch())
   {
      ret = DMS_SUCCESS;
	  *Scales = _data;
   }
   else if (source->NoData())
      ret = DMS_NO_DATA;
   else 
      ret = DMS_ERROR;

   return ret;//POP();

#undef SQL_GETSCALE2 
}



LONG getScaleCon(BOOL fLocal,DMS_SCALECON* Scales, long fOpt)
{
#define SQL_GETSCALECON "SELECT SCALEID,SLEVEL,ELEVEL "\
							"FROM SCALECON "\
							"WHERE (SCALEID=? OR 0=?)" 

  PUSH(getScaleCon);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_GETSCALECON);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_GETSCALECON);
       CODBCbindedStmt *source;

static DMS_SCALECON _data;
static LONG _scaleid;
static BOOL first_time=TRUE;


   if (fOpt==FIND_FIRST) {
      if (first_time)
	  {
		BINDPARAM(1, _scaleid);
		BINDPARAM(2, _scaleid);
												
        BIND(1, _data.SCALEID);		                   
        BIND(2, _data.SLEVEL);
		BIND(3, _data.ELEVEL); 

      first_time = FALSE;
	  }
      _scaleid=Scales->SCALEID;
      local.Execute();
	  central.Execute();
   }

   source = (fLocal)?&local:&central;

   if (source->Fetch())
   {
      ret=DMS_SUCCESS;
	  *Scales = _data;
   }
   else if (source->NoData())
      ret = DMS_NO_DATA;
   else 
      ret = DMS_ERROR;

   return ret;//POP();

#undef SQL_GETSCALE 
}



LONG getScaleDis(BOOL fLocal,DMS_SCALEDIS* Scales, long fOpt)
{
#define SQL_GETSCALEDIS "SELECT SCALEID,VALUE,VALDESC "\
							"FROM SCALEDIS "\
							"WHERE (SCALEID=? OR 0=?) " 

  PUSH(getScaleDis);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_GETSCALEDIS);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_GETSCALEDIS);
       CODBCbindedStmt *source;

static DMS_SCALEDIS _data;
static LONG _scaleid;
static BOOL first_time=TRUE;


   if (fOpt==FIND_FIRST) {
      if (first_time)
	  {
		BINDPARAM(1, _scaleid);
		BINDPARAM(2, _scaleid);
												
        BIND(1, _data.SCALEID);		                   
        BINDS(2, _data.VALUE, DMS_SCALE_VALUE);
		BINDS(3, _data.VALDESC, DMS_SCALE_DESC); 

      first_time = FALSE;
	  }
      _scaleid=Scales->SCALEID;
      local.Execute();
	  central.Execute();
   }

   source = (fLocal)?&local:&central;

   if (source->Fetch())
   {
      ret=DMS_SUCCESS;
	  *Scales = _data;
   }
   else if (source->NoData())
      ret = DMS_NO_DATA;
   else 
      ret = DMS_ERROR;

   POP();

#undef SQL_GETSCALEDIS 
}


LONG getScaleDis2(BOOL fLocal,DMS_SCALEDIS* Scales, long fOpt)
{
#define SQL_GETSCALEDIS "SELECT SCALEID,VALUE,VALDESC "\
							"FROM SCALEDIS "\
							"WHERE (SCALEID=? AND VALUE = ? ) " 

  PUSH(getScaleDis);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_GETSCALEDIS);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_GETSCALEDIS);
       CODBCbindedStmt *source;

static DMS_SCALEDIS _data;
static LONG _scaleid;
static BOOL first_time=TRUE;


   if (fOpt==FIND_FIRST) {
      if (first_time)
	  {
		BINDPARAM(1, _scaleid);
		BINDPARAMS(2, _data.VALUE, DMS_SCALE_VALUE-1);
												
        BIND(1, _data.SCALEID);		                   
        BINDS(2, _data.VALUE, DMS_SCALE_VALUE);
		BINDS(3, _data.VALDESC, DMS_SCALE_DESC); 

      first_time = FALSE;
	  }
	  _data = *Scales;
      _scaleid=Scales->SCALEID;
      local.Execute();
	  central.Execute();
   }

   source = (fLocal)?&local:&central;

   if (source->Fetch())
   {
      ret=DMS_SUCCESS;
	  *Scales = _data;
   }
   else if (source->NoData())
      ret = DMS_NO_DATA;
   else 
      ret = DMS_ERROR;

   return ret;//POP();

#undef SQL_GETSCALEDIS 
}





LONG getScaleTab(BOOL fLocal,DMS_SCALETAB* Scales)
{
#define SQL_GETSCALEDIS "SELECT SCALEID,SSQL,MODULE "\
							"FROM SCALETAB "\
							"WHERE (SCALEID=?)" 

  PUSH(getScaleDis);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_GETSCALEDIS);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_GETSCALEDIS);
       CODBCbindedStmt *source;

static DMS_SCALETAB _data;
static LONG _scaleid;
static BOOL first_time=TRUE;


   if (first_time)
	  {
		BINDPARAM(1, _scaleid);
												
        BIND(1, _data.SCALEID);		                   
        BINDS(2, _data.SQLTEXT, DMS_SCALE_SQL-2);
		BINDS(3,_data.MODULE, 8);

      first_time = FALSE;
   }
      _scaleid=Scales->SCALEID;
      local.Execute();
	  central.Execute();

   source = (fLocal)?&local:&central;

   if (source->Fetch())
   {
      ret=DMS_SUCCESS;
	  *Scales = _data;
   }
   else if (source->NoData())
      ret = DMS_NO_DATA;
   else 
      ret = DMS_ERROR;

   return ret;//POP();

#undef SQL_GETSCALETAB
}


// Stores the result in a file
LONG executeScaleTab(DMS_SCALETAB scaletab, CHAR *fname, LONG *noCol)
{
  PUSH(executeScaleTab);
  CODBCdirectStmt &central = _centralDBC->DirectStmt("SELECT * FROM SCALETAB");
  CODBCdirectStmt &local = _localDBC->DirectStmt("SELECT * FROM SCALETAB");
  CHAR *szData, szFileName[252];
  FILE  *sFile;
  BOOLEAN fetchCentral;
  long i;

  fetchCentral = TRUE;
  if (strcmp((CHAR *)scaletab.MODULE ,(CHAR *)"GMS")==0) {
    central = _centralDBC->DirectStmt((LPCSTR) scaletab.SQLTEXT);
    local = _localDBC->DirectStmt((LPCSTR) scaletab.SQLTEXT);
  }
  if (strcmp((CHAR *)scaletab.MODULE ,(CHAR *)"GMSLOC")==0) {
    local = _localDBC->DirectStmt((LPCSTR) scaletab.SQLTEXT);
	fetchCentral = FALSE;
  }
  if (strcmp((CHAR *)scaletab.MODULE ,(CHAR *)"DMS")==0) {
    central =  _centralDMS->DirectStmt((LPCSTR) scaletab.SQLTEXT);
    local = _localDMS->DirectStmt((LPCSTR) scaletab.SQLTEXT);
  }
  if (strcmp((CHAR *)scaletab.MODULE ,(CHAR *)"DMSLOC")==0) {
    local = _localDMS->DirectStmt((LPCSTR) scaletab.SQLTEXT);
	fetchCentral = FALSE;
  }
  
  sFile=fopen(fname,"w+"); \
  if (sFile!=NULL)\
       setvbuf( sFile, NULL, _IONBF, 0 );
     
  if (fetchCentral) {
   central.Execute();
   *noCol = central.ColCount();
   for (i=1;i<=*noCol;i++) {
      szData = ((CHAR *)central.Field(i).Name());
	  fprintf(sFile,szData);
	  if (i!= *noCol) 
	    fprintf(sFile," \t");
	  else
	    fprintf(sFile," \n");
   }
   while (central.Fetch()) {
	   for (i=1;i<=*noCol;i++) {
		  szData = ((CHAR *) central.Field(i).AsString());
		     fprintf(sFile,szData);
		  if (i!= *noCol) 
		     fprintf(sFile," \t");
		  else
			 fprintf(sFile," \n");
	   }
   }
  }

   local.Execute();
   *noCol = local.ColCount();
   while (local.Fetch()) {
	   for (i=1;i<=*noCol;i++) {
		  szData = ((CHAR *) local.Field(i).AsString());
		     fprintf(sFile,szData);
		  if (i!= *noCol) 
		     fprintf(sFile," \t");
		  else
			 fprintf(sFile," \n");
	   }
   }

   if (sFile!=NULL) fclose(sFile);
   ret=DMS_SUCCESS;
   local.Free();
   central.Free();
   return ret;//POP();

#undef SQL_GETSCALETAB
}


// Stores the result in a file, delimitted by exclamation point
LONG executeScaleTab_exclam(DMS_SCALETAB scaletab, CHAR *fname, LONG *noCol)
{
  PUSH(executeScaleTab);
  CODBCdirectStmt &central = _centralDBC->DirectStmt("SELECT * FROM SCALETAB");
  CODBCdirectStmt &local = _localDBC->DirectStmt("SELECT * FROM SCALETAB");
  CHAR *szData, szFileName[252];
  FILE  *sFile;
  BOOLEAN fetchCentral;
  long i;

  fetchCentral = TRUE;
  if (strcmp((CHAR *)scaletab.MODULE ,(CHAR *)"GMS")==0) {
    central = _centralDBC->DirectStmt((LPCSTR) scaletab.SQLTEXT);
    local = _localDBC->DirectStmt((LPCSTR) scaletab.SQLTEXT);
  }
  if (strcmp((CHAR *)scaletab.MODULE ,(CHAR *)"GMSLOC")==0) {
    local = _localDBC->DirectStmt((LPCSTR) scaletab.SQLTEXT);
	fetchCentral = FALSE;
  }
  if (strcmp((CHAR *)scaletab.MODULE ,(CHAR *)"DMS")==0) {
    central =  _centralDMS->DirectStmt((LPCSTR) scaletab.SQLTEXT);
    local = _localDMS->DirectStmt((LPCSTR) scaletab.SQLTEXT);
  }
  if (strcmp((CHAR *)scaletab.MODULE ,(CHAR *)"DMSLOC")==0) {
    local = _localDMS->DirectStmt((LPCSTR) scaletab.SQLTEXT);
	fetchCentral = FALSE;
  }
  
  sFile=fopen(fname,"w+"); \
  if (sFile!=NULL)\
       setvbuf( sFile, NULL, _IONBF, 0 );
     
  if (fetchCentral) {
   central.Execute();
   *noCol = central.ColCount();
   for (i=1;i<=*noCol;i++) {
      szData = ((CHAR *)central.Field(i).Name());
	  fprintf(sFile,szData);
	  if (i!= *noCol) 
		  fprintf(sFile," !");
	  else
	    fprintf(sFile," \n");
   }
   while (central.Fetch()) {
	   for (i=1;i<=*noCol;i++) {
		  szData = ((CHAR *) central.Field(i).AsString());
		     fprintf(sFile,szData);
		  if (i!= *noCol) 
		     fprintf(sFile," !");
		  else
			 fprintf(sFile," \n");
	   }
   }
  }

   local.Execute();
   *noCol = local.ColCount();
   while (local.Fetch()) {
	   for (i=1;i<=*noCol;i++) {
		  szData = ((CHAR *) local.Field(i).AsString());
		     fprintf(sFile,szData);
		  if (i!= *noCol) 
		     fprintf(sFile," !");
		  else
			 fprintf(sFile," \n");
	   }
   }

   if (sFile!=NULL) fclose(sFile);
   ret=DMS_SUCCESS;
   local.Free();
   central.Free();
   return ret;//POP();

#undef SQL_GETSCALETAB
}



// Returns the retrieved value
LONG executeScaleTab2(DMS_SCALETAB scaletab,CHAR *value, LONG nVal)
{

  PUSH(executeScaleTab);
  CODBCdirectStmt &central = _centralDMS->DirectStmt("SELECT * FROM SCALETAB");
  CODBCdirectStmt &local = _localDMS->DirectStmt("SELECT * FROM SCALETAB");
  CHAR *szData, *szLocData;
  BOOLEAN fetchCentral;

  fetchCentral = TRUE;
  if (strcmp((CHAR *)scaletab.MODULE ,(CHAR *)"GMS")==0) {
    central = _centralDBC->DirectStmt((LPCSTR) scaletab.SQLTEXT);
    local = _localDBC->DirectStmt((LPCSTR) scaletab.SQLTEXT);
  }
  if (strcmp((CHAR *)scaletab.MODULE ,(CHAR *)"GMSLOC")==0) {
    local = _localDBC->DirectStmt((LPCSTR) scaletab.SQLTEXT);
	fetchCentral = FALSE;
  }
  if (strcmp((CHAR *)scaletab.MODULE ,(CHAR *)"DMS")==0) {
    central =  _centralDMS->DirectStmt((LPCSTR) scaletab.SQLTEXT);
    local = _localDMS->DirectStmt((LPCSTR) scaletab.SQLTEXT);
}
  if (strcmp((CHAR *)scaletab.MODULE ,(CHAR *)"DMSLOC")==0) {
    local = _localDMS->DirectStmt((LPCSTR) scaletab.SQLTEXT);
	fetchCentral = FALSE;
  }
  //MessageBox(NULL,scaletab.MODULE,"ICIS V5-Module",MB_OK);
  //MessageBox(NULL,scaletab.SQLTEXT,"ICIS V5-SQL",MB_OK);
  
  //ZeroMemory(szData,nVal);
  if (fetchCentral) {
   central.Execute();
   if (central.Fetch()) {
	 szData = ((CHAR *)central.Field(1).AsString());
     strncpy(value,szData, nVal);

     //MessageBox(NULL,(CHAR *)central.Field(1).AsString(),"ICIS V5-central Fetch",MB_OK);
     //MessageBox(NULL,(CHAR *)central.SQLstr(),"ICIS V5-central Fetch",MB_OK);
	 
   }
//   if (central.NoData())
//	   MessageBox(NULL,"No Data","ICIS V5-central Fetch",MB_OK);
  }


  local.Execute();
  if (local.Fetch()){
	  if (!local.NoData()) {
			szLocData = ((CHAR *) local.Field(1).AsString());
			if (strlen(szLocData) > 0) {
				strncpy(value, szLocData, nVal);
 // 			    MessageBox(NULL,szData,"ICIS V5-local fetch",MB_OK);
			}
	}
  }
//  MessageBox(NULL,value,"ICIS V5- value",MB_OK);
  
  ret=DMS_SUCCESS;
  return ret;//POP();

#undef SQL_GETSCALETAB2
}


//////////////////////////////////////////////////////////////////////////////
// Execute an SQL statement and returns the retrieved value
// The SQL is stored in the DMS_SCALETAB structure,
// value is the returned value and nVal is the length
// The result of the statement is Bind if fopt = 0
//////////////////////////////////////////////////////////////////////////////

LONG executeScaleTab3(DMS_SCALETAB scaletab,CHAR *value, LONG nVal, long param1, LONG fopt)
{

  PUSH(executeScaleTab3);
  static CODBCbindedStmt &central= _centralDBC->BindedStmt();
  static CODBCbindedStmt &local  = _localDBC->BindedStmt();
  static CODBCbindedStmt &centralDMS= _centralDMS->BindedStmt();
  static CODBCbindedStmt &localDMS  = _localDMS->BindedStmt();
  static LONG _param1;
  static CHAR _value[ICIS_MAX_SIZE ];

  CHAR *szData;
  BOOLEAN fetchCentral=TRUE;

  if (fopt== FIND_FIRST) {
	string central_sql,local_sql,strtemp;

	local_sql=  scaletab.SQLTEXT;
	central_sql= scaletab.SQLTEXT;
    fetchCentral = TRUE;

	if (strcmp((CHAR *)scaletab.MODULE ,(CHAR *)"GMS")==0) {
		central= _centralDBC->BindedStmt((LPCSTR)central_sql.c_str());            
		local  = _localDBC->BindedStmt((LPCSTR)local_sql.c_str());              
	}
	if (strcmp((CHAR *)scaletab.MODULE ,(CHAR *)"GMSLOC")==0) {
		local  = _localDBC->BindedStmt((LPCSTR)local_sql.c_str());              
		fetchCentral = FALSE;
	}
	if (strcmp((CHAR *)scaletab.MODULE ,(CHAR *)"DMS")==0) {
		central= _centralDMS->BindedStmt((LPCSTR)central_sql.c_str());            
		local  = _localDMS->BindedStmt((LPCSTR)local_sql.c_str());              
	}
	if (strcmp((CHAR *)scaletab.MODULE ,(CHAR *)"DMSLOC")==0) {
		local  = _localDMS->BindedStmt((LPCSTR)local_sql.c_str());              
		fetchCentral = FALSE;
	}

    BINDS(1,_value,ICIS_MAX_SIZE);
    BINDPARAM(1,_param1);
  }


  _param1 = param1;

  ZeroMemory(value,nVal);   //initialize the string input parameter
  if (fetchCentral) {
   central.Execute();
   if (central.Fetch()) {
     strncpy(value,_value, nVal);
   }
  }

   local.Execute();
   if (local.Fetch()) {
     strncpy(value,_value, nVal);
   }
   
   ret=DMS_SUCCESS;
   return ret;//POP();

#undef SQL_GETSCALETAB3
}


LONG getTmethod(BOOL fLocal,DMS_TMETHOD* Tmethods, UCHAR* szMDesc, LONG cMDesc, int fOpt)
{
#define SQL_GETMETHOD "SELECT TMETHID,TRAITID,TMNAME,TMABBR,TMDESC FROM TMETHOD WHERE (TMETHID=? OR 0=?)"

  LONG ret=GMS_SUCCESS;//PUSH(getGermplasmRecord);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_GETMETHOD);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_GETMETHOD);
       CODBCbindedStmt *source;

static DMS_TMETHOD _data;
static CHAR szDesc[DMS_MAX_STR]; 
static LONG methid;
static BOOL first_time=TRUE;

//Parameters
static LONG _gid;

   if (fOpt == FIND_FIRST) {
      if (first_time)
	  {
		BINDPARAM(1, methid);
		BINDPARAM(2, methid);
												
        BIND(1, _data.TMETHID);		                   
        BIND(2, _data.TRAITID);		                   
        BINDS(3, _data.TMNAME, DMS_METHOD_NAME);
		BINDS(4, _data.TMABBR, DMS_METHOD_ABBR); 
		BINDS(5, szDesc, 1000); 
        first_time = FALSE;
	  }
      methid=Tmethods->TMETHID;
      local.Execute();
	  central.Execute();
   }


   source = (fLocal)?&local:&central;

   if (source->Fetch())
   {
      ret=DMS_SUCCESS;
	  *Tmethods = _data;
	  if (sizeof(szDesc) < cMDesc)
         strncpy((CHAR*) szMDesc,szDesc,sizeof(szDesc));
      else
         strncpy((CHAR*) szMDesc,szDesc,cMDesc);
   }
   else if (source->NoData())
      ret = DMS_NO_DATA;
   else 
      ret = DMS_ERROR;

   return ret;//POP();

#undef SQL_GETMETHOD 
}


LONG getTmethod2(BOOL fLocal,DMS_TMETHOD* Tmethods, UCHAR* szMDesc, LONG cMDesc, int fOpt)
{
#define SQL_GETMETHOD "SELECT TMETHID,TRAITID,TMNAME,TMABBR,TMDESC FROM TMETHOD WHERE (TRAITID=?)"

  LONG ret=GMS_SUCCESS;//PUSH(getGermplasmRecord);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_GETMETHOD);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_GETMETHOD);
       CODBCbindedStmt *source;

static DMS_TMETHOD _data;
static CHAR szDesc[DMS_MAX_STR];
static LONG traitid;
static BOOL first_time=TRUE;

//Parameters
static LONG _gid;

   if (fOpt == FIND_FIRST) { 
      if (first_time)
	  {
		BINDPARAM(1, traitid);
												
        BIND(1, _data.TMETHID);		                   
        BIND(2, _data.TRAITID);		                   
        BINDS(3, _data.TMNAME, DMS_METHOD_NAME);
		BINDS(4, _data.TMABBR, DMS_METHOD_ABBR); 
		BINDS(5, szDesc, 1000);

        first_time = FALSE;
	  }
      traitid=Tmethods->TRAITID;
      local.Execute();
	  central.Execute();
   
   } 

   source = (fLocal)?&local:&central;

   if (source->Fetch())
   {
      ret=DMS_SUCCESS;
	  *Tmethods = _data;
	  if (sizeof(szDesc) < cMDesc)
         strncpy((CHAR*) szMDesc,szDesc,sizeof(szDesc));
      else
         strncpy((CHAR*) szMDesc,szDesc,cMDesc);
    }
   else if (source->NoData())
      ret = DMS_NO_DATA;
   else 
      ret = DMS_ERROR;

   return ret;//POP();

#undef SQL_GETMETHOD 
}



LONG findTrait(BOOL fLocal, CHAR *szName, DMS_TRAIT &data, LONG fSearchOption)
{
#define SQL_FINDTRAIT " select TRAITID, TRNAME, TRABBR \
             from TRAIT  "

#define TRNAME_EQ_SQL  " WHERE  {fn UCASE(TRAIT.TRNAME)} = ? " 
#define TRNAME_LIKE_SQL " WHERE  {fn UCASE(TRAIT.TRNAME)} LIKE ? "

#define SQL_FINDTRAIT_ " order by TRNAME asc"

   PUSH(findTrait);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_FINDTRAIT);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_FINDTRAIT);
       CODBCbindedStmt *source;
static CHAR trname[MAX_STR];
static DMS_TRAIT _data;
static BOOL name_wild;

static BOOL first_time=TRUE;

   if (fSearchOption==FIND_FIRST){
      string stmt_sql;
	  ZeroFill(trname,sizeof(trname));
      strcpy(trname,szName);
	  UpperCase((CHAR *) trname, DMS_TRAIT_NAME);
	  name_wild =strchr(trname,'_') || strchr(trname,'%');
	  stmt_sql=SQL_FINDTRAIT;
	  stmt_sql+=(name_wild?TRNAME_LIKE_SQL:TRNAME_EQ_SQL);
	  stmt_sql+=SQL_FINDTRAIT_;
	  central.SetSQLstr(stmt_sql.c_str());
	  local.SetSQLstr(stmt_sql.c_str());

//      if (first_time)
//    {
         BIND(1,_data.TRAITID);
         BINDS(2,_data.TRNAME,DMS_TRAIT_NAME);
		 BINDS(3,_data.TRABBR,DMS_TRAIT_ABBR);
         BINDPARAMS(1, trname, DMS_TRAIT_NAME);
        first_time=FALSE;
//    }
    
      local.Execute();
      central.Execute();
   }

   source = (fLocal)?&local:&central;

   ZeroMemory(&_data,sizeof(_data));
   if (source->Fetch())
      data = _data;
   else if (source->NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;

   POP();
#undef SQL_FINDTRAIT
#undef TRNAME_EQ_SQL 
#undef TRNAME_LIKE_SQL 
}





///////////////////////////////////////////////////////////////////////////////
// Version 4
LONG findTrait_v4(BOOL fLocal, CHAR *szName, DMS_TRAIT &data, LONG fSearchOption)
{
#define SQL_FINDTRAIT4 " select TID, TRAITID, TRNAME, TRABBR, TNSTAT, SCALEID, TMETHID \
             from TRAIT  "

#define TRNAME_EQ_SQL  " WHERE  {fn UCASE(TRAIT.TRNAME)} = ? " 
#define TRNAME_LIKE_SQL " WHERE  {fn UCASE(TRAIT.TRNAME)} LIKE ? "

#define SQL_FINDTRAIT_ " order by TRNAME asc"

   PUSH(findTrait_v4);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_FINDTRAIT4);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_FINDTRAIT4);
       CODBCbindedStmt *source;
static CHAR trname[MAX_STR];
static DMS_TRAIT _data;
static BOOL name_wild;

static BOOL first_time=TRUE;

   if (fSearchOption==FIND_FIRST){
      string stmt_sql;
	  ZeroFill(trname,sizeof(trname));
      strcpy(trname,szName);
	  UpperCase((CHAR *) trname, DMS_TRAIT_NAME);
	  name_wild =strchr(trname,'_') || strchr(trname,'%');
	  stmt_sql=SQL_FINDTRAIT4;
	  stmt_sql+=(name_wild?TRNAME_LIKE_SQL:TRNAME_EQ_SQL);
	  stmt_sql+=SQL_FINDTRAIT_;
	  central.SetSQLstr(stmt_sql.c_str());
	  local.SetSQLstr(stmt_sql.c_str());

//      if (first_time)
//    {
         BIND(1,_data.TID);
         BIND(2,_data.TRAITID);
         BINDS(3,_data.TRNAME,DMS_TRAIT_NAME);
		 BINDS(4,_data.TRABBR,DMS_TRAIT_ABBR);
         BIND(5,_data.TNSTAT);
         BIND(6,_data.SCALEID);
         BIND(7,_data.TMETHID);
         BINDPARAMS(1, trname, DMS_TRAIT_NAME-2);
        first_time=FALSE;
//    }
    
      local.Execute();
      central.Execute();
   }

   source = (fLocal)?&local:&central;

   ZeroMemory(&_data,sizeof(_data));
   if (source->Fetch())
      data = _data;
   else if (source->NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;

   POP();
#undef SQL_FINDTRAIT4
#undef TRNAME_EQ_SQL4 
#undef TRNAME_LIKE_SQL4 
}

LONG findTraitEq(BOOL fLocal, CHAR *szName, DMS_TRAIT &data, LONG fSearchOption)
{
#define SQL_FINDTRAIT_EQ " select TID, TRAITID, TRNAME, TRABBR, TNSTAT, SCALEID, TMETHID \
             from TRAIT  WHERE  {fn UCASE(TRAIT.TRNAME)} = ? order by TRNAME asc"

   PUSH(findTraitEq);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_FINDTRAIT_EQ);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_FINDTRAIT_EQ);
       CODBCbindedStmt *source;
static CHAR trname[MAX_STR];
static DMS_TRAIT _data;
static BOOL name_wild;

static BOOL first_time=TRUE;

   if (fSearchOption==FIND_FIRST){
      string stmt_sql;
	  ZeroFill(trname,sizeof(trname));
      strcpy(trname,szName);
	  UpperCase((CHAR *) trname, DMS_TRAIT_NAME);

      if (first_time)
	  {
         BIND(1,_data.TID);
         BIND(2,_data.TRAITID);
         BINDS(3,_data.TRNAME,DMS_TRAIT_NAME);
		 BINDS(4,_data.TRABBR,DMS_TRAIT_ABBR);
         BIND(5,_data.TNSTAT);
         BIND(6,_data.SCALEID);
         BIND(7,_data.TMETHID);
         BINDPARAMS(1, trname, DMS_TRAIT_NAME-2);
        first_time=FALSE;
	  }
    
      local.Execute();
      central.Execute();
   }

   source = (fLocal)?&local:&central;

   ZeroMemory(&_data,sizeof(_data));
   if (source->Fetch()) {
      data = _data;
	  ret = DMS_SUCCESS;
   }
   else if (source->NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;

   POP();
#undef SQL_FINDTRAIT_EQ
}


LONG findTraitLike(BOOL fLocal, CHAR *szName, DMS_TRAIT &data, LONG fSearchOption)
{
#define SQL_FINDTRAIT_LIKE " select TID, TRAITID, TRNAME, TRABBR, TNSTAT, SCALEID, TMETHID \
             from TRAIT  WHERE  {fn UCASE(TRAIT.TRNAME)} LIKE ? order by TRNAME asc"

   PUSH(findTraitEq);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_FINDTRAIT_LIKE);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_FINDTRAIT_LIKE);
       CODBCbindedStmt *source;
static CHAR trname[MAX_STR];
static DMS_TRAIT _data;
static BOOL name_wild;

static BOOL first_time=TRUE;

   if (fSearchOption==FIND_FIRST){
      string stmt_sql;
	  ZeroFill(trname,sizeof(trname));
      strcpy(trname,szName);
	  UpperCase((CHAR *) trname, DMS_TRAIT_NAME);

      if (first_time)
	  {
         BIND(1,_data.TID);
         BIND(2,_data.TRAITID);
         BINDS(3,_data.TRNAME,DMS_TRAIT_NAME);
		 BINDS(4,_data.TRABBR,DMS_TRAIT_ABBR);
         BIND(5,_data.TNSTAT);
         BIND(6,_data.SCALEID);
         BIND(7,_data.TMETHID);
         BINDPARAMS(1, trname, DMS_TRAIT_NAME-2);
        first_time=FALSE;
	  }
    
      local.Execute();
      central.Execute();
   }

   source = (fLocal)?&local:&central;

   ZeroMemory(&_data,sizeof(_data));
   if (source->Fetch())
      data = _data;
   else if (source->NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;

   POP();
#undef SQL_FINDTRAIT_LIKE
}



LONG findScale(BOOL fLocal, CHAR *szName, DMS_SCALE &data, LONG fSearchOption)
{
#define SQL_FINDSCALE "select SCALEID, TRAITID, SCNAME, SCTYPE \
             from SCALE "  

#define SCNAME_EQ_SQL  " where  TRAITID = ? AND {fn UCASE(SCNAME)} = ? " 
#define SCNAME_LIKE_SQL " where  TRAITID = ? AND {fn UCASE(SCNAME)} LIKE ? "

#define SQL_FINDSCALE_ " order by SCNAME asc "

   PUSH(findScale);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_FINDSCALE);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_FINDSCALE);
       CODBCbindedStmt *source;
static CHAR scname[MAX_STR];
static LONG traitid;
static DMS_SCALE _data;
static BOOL name_wild;

static BOOL first_time=TRUE;

   if (fSearchOption==FIND_FIRST){
      string stmt_sql;
	  traitid = data.TRAITID;
	  ZeroFill(scname,sizeof(scname));
      strcpy(scname,szName);
	  UpperCase((CHAR *) scname, DMS_SCALE_NAME);
	  name_wild =strchr(scname,'_') || strchr(scname,'%');
	  stmt_sql=SQL_FINDSCALE;
	  stmt_sql+=(name_wild?SCNAME_LIKE_SQL:SCNAME_EQ_SQL);
	  stmt_sql+=SQL_FINDSCALE_;
	  central.SetSQLstr(stmt_sql.c_str());
	  local.SetSQLstr(stmt_sql.c_str());

//    if (first_time)
//    {
         BIND(1,_data.SCALEID);
		 BIND(2, _data.TRAITID);
         BINDS(3,_data.SCNAME,DMS_SCALE_NAME);
		 BINDS(4,_data.SCTYPE,DMS_SCALE_TYPE);
         BINDPARAM(1, traitid);
         BINDPARAMS(2, scname, DMS_SCALE_NAME-2);
         first_time=FALSE;
 //   }

      local.Execute();
      central.Execute();
   }

   source = (fLocal)?&local:&central;

   ZeroMemory(&_data,sizeof(_data));
   if (source->Fetch())
      data = _data;
   else if (source->NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;


   POP();
#undef SQL_FINDSCALE
#undef SCNAME_EQ_SQL 
#undef SCNAME_LIKE_SQL 
}


LONG findScaleEq(BOOL fLocal, CHAR *szName, DMS_SCALE &data, LONG fSearchOption)
{
#define SQL_FINDSCALE_EQ "select SCALEID, TRAITID, SCNAME, SCTYPE \
             from SCALE where  TRAITID = ? AND {fn UCASE(SCNAME)} = ? order by SCNAME asc "

   PUSH(findScale);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_FINDSCALE_EQ);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_FINDSCALE_EQ);
       CODBCbindedStmt *source;
static CHAR scname[MAX_STR];
static LONG traitid;
static DMS_SCALE _data;
static BOOL name_wild;

static BOOL first_time=TRUE;

   if (fSearchOption==FIND_FIRST){
      string stmt_sql;
	  traitid = data.TRAITID;
	  ZeroFill(scname,sizeof(scname));
      strcpy(scname,szName);
	  UpperCase((CHAR *) scname, DMS_SCALE_NAME);

      if (first_time)
	  {
        BIND(1,_data.SCALEID);
        BIND(2, _data.TRAITID);
        BINDS(3,_data.SCNAME,DMS_SCALE_NAME);
	    BINDS(4,_data.SCTYPE,DMS_SCALE_TYPE);
        BINDPARAM(1, traitid);
        BINDPARAMS(2, scname, DMS_SCALE_NAME-2);
        first_time=FALSE;
	  }

      local.Execute();
      central.Execute();
   }

   source = (fLocal)?&local:&central;

   ZeroMemory(&_data,sizeof(_data));
   if (source->Fetch())
      data = _data;
   else if (source->NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;


   POP();
#undef SQL_FINDSCALE_EQ
}


LONG findScaleLike(BOOL fLocal, CHAR *szName, DMS_SCALE &data, LONG fSearchOption)
{
#define SQL_FINDSCALE_LIKE "select SCALEID, TRAITID, SCNAME, SCTYPE \
             from SCALE where  TRAITID = ? AND {fn UCASE(SCNAME)} = ? order by SCNAME asc "

   PUSH(findScale);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_FINDSCALE_LIKE);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_FINDSCALE_LIKE);
       CODBCbindedStmt *source;
static CHAR scname[MAX_STR];
static LONG traitid;
static DMS_SCALE _data;
static BOOL name_wild;

static BOOL first_time=TRUE;

   if (fSearchOption==FIND_FIRST){
      string stmt_sql;
	  traitid = data.TRAITID;
	  ZeroFill(scname,sizeof(scname));
      strcpy(scname,szName);
	  UpperCase((CHAR *) scname, DMS_SCALE_NAME);

      if (first_time)
	  {
        BIND(1,_data.SCALEID);
        BIND(2, _data.TRAITID);
        BINDS(3,_data.SCNAME,DMS_SCALE_NAME);
	    BINDS(4,_data.SCTYPE,DMS_SCALE_TYPE);
        BINDPARAM(1, traitid);
        BINDPARAMS(2, scname, DMS_SCALE_NAME-2);
        first_time=FALSE;
	  }

      local.Execute();
      central.Execute();
   }

   source = (fLocal)?&local:&central;

   ZeroMemory(&_data,sizeof(_data));
   if (source->Fetch())
      data = _data;
   else if (source->NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;


   POP();
#undef SQL_FINDSCALE_LIKE
}



LONG findTmethod(BOOL fLocal, CHAR *szName, DMS_TMETHOD &data, LONG fSearchOption)
{
#define SQL_FINDMETHOD "select TMETHID, TRAITID, TMNAME, TMABBR \
             from TMETHOD "  

#define TMNAME_EQ_SQL  " where  TRAITID = ? AND {fn UCASE(TMNAME)} = ? " 
#define TMNAME_LIKE_SQL " where  TRAITID = ? AND {fn UCASE(TMNAME)} LIKE ? "

#define SQL_FINDMETHOD_ " order by TMNAME asc "

   PUSH(findMethod);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_FINDMETHOD);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_FINDMETHOD);
       CODBCbindedStmt *source;
static CHAR scname[MAX_STR];
static LONG traitid;
static DMS_TMETHOD _data;
static BOOL name_wild;

static BOOL first_time=TRUE;

   if (fSearchOption==FIND_FIRST){
      string stmt_sql;
 	  traitid = data.TRAITID;
 	  ZeroFill(scname,sizeof(scname));
      strcpy(scname,szName);
  	  UpperCase((CHAR *) scname, DMS_METHOD_NAME);
      name_wild =strchr(scname,'_') || strchr(scname,'%');
	  stmt_sql=SQL_FINDMETHOD;
	  stmt_sql+=(name_wild?TMNAME_LIKE_SQL:TMNAME_EQ_SQL);
	  stmt_sql+=SQL_FINDMETHOD_;
	  central.SetSQLstr(stmt_sql.c_str());
	  local.SetSQLstr(stmt_sql.c_str());

 //   if (first_time)
 //   {
         BIND(1,_data.TMETHID);
		 BIND(2, _data.TRAITID);
         BINDS(3,_data.TMNAME,DMS_METHOD_NAME);
		 BINDS(4,_data.TMABBR,DMS_METHOD_ABBR);
         BINDPARAM(1, traitid);
         BINDPARAMS(2, scname, DMS_METHOD_NAME-2);
         first_time=FALSE;
 //   }

      local.Execute();
      central.Execute();
   }

   source = (fLocal)?&local:&central;
   ZeroMemory(&_data,sizeof(_data));

   if (source->Fetch())
      data = _data;
   else if (source->NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;

   POP();
#undef SQL_FINDMETHOD
#undef TMNAME_EQ_SQL 
#undef TMNAME_LIKE_SQL 
}



LONG findTmethodEq(BOOL fLocal, CHAR *szName, DMS_TMETHOD &data, LONG fSearchOption)
{
#define SQL_FINDMETHOD_EQ "select TMETHID, TRAITID, TMNAME, TMABBR \
             from TMETHOD  where  TRAITID = ? AND {fn UCASE(TMNAME)} = ?  order by TMNAME asc "

   PUSH(findMethodEq);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_FINDMETHOD_EQ);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_FINDMETHOD_EQ);
       CODBCbindedStmt *source;

static CHAR scname[MAX_STR];
static LONG traitid;
static DMS_TMETHOD _data;
static BOOL name_wild;

static BOOL first_time=TRUE;

   if (fSearchOption==FIND_FIRST){
      string stmt_sql;
 	  traitid = data.TRAITID;
 	  ZeroFill(scname,sizeof(scname));
      strcpy(scname,szName);
  	  UpperCase((CHAR *) scname, DMS_METHOD_NAME);

      if (first_time)
      {
         BIND(1,_data.TMETHID);
		 BIND(2, _data.TRAITID);
         BINDS(3,_data.TMNAME,DMS_METHOD_NAME);
		 BINDS(4,_data.TMABBR,DMS_METHOD_ABBR);
         BINDPARAM(1, traitid);
         BINDPARAMS(2, scname, DMS_METHOD_NAME-2);
         first_time=FALSE;
	  }

      local.Execute();
      central.Execute();
   }

   source = (fLocal)?&local:&central;
   ZeroMemory(&_data,sizeof(_data));

   if (source->Fetch())
      data = _data;
   else if (source->NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;

   POP();
#undef SQL_FINDMETHOD_EQ
}



LONG findTmethodLike(BOOL fLocal, CHAR *szName, DMS_TMETHOD &data, LONG fSearchOption)
{
#define SQL_FINDMETHOD_LIKE "select TMETHID, TRAITID, TMNAME, TMABBR \
             from TMETHOD  where  TRAITID = ? AND {fn UCASE(TMNAME)} LIKE ?  order by TMNAME asc "

   PUSH(findMethodLike);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_FINDMETHOD_LIKE);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_FINDMETHOD_LIKE);
       CODBCbindedStmt *source;

static CHAR scname[MAX_STR];
static LONG traitid;
static DMS_TMETHOD _data;
static BOOL name_wild;

static BOOL first_time=TRUE;

   if (fSearchOption==FIND_FIRST){
      string stmt_sql;
 	  traitid = data.TRAITID;
 	  ZeroFill(scname,sizeof(scname));
      strcpy(scname,szName);
  	  UpperCase((CHAR *) scname, DMS_METHOD_NAME);

      if (first_time)
      {
         BIND(1,_data.TMETHID);
		 BIND(2, _data.TRAITID);
         BINDS(3,_data.TMNAME,DMS_METHOD_NAME);
		 BINDS(4,_data.TMABBR,DMS_METHOD_ABBR);
         BINDPARAM(1, traitid);
         BINDPARAMS(2, scname, DMS_METHOD_NAME-2);
         first_time=FALSE;
	  }

      local.Execute();
      central.Execute();
   }

   source = (fLocal)?&local:&central;
   ZeroMemory(&_data,sizeof(_data));

   if (source->Fetch())
      data = _data;
   else if (source->NoData())
      ret=DMS_NO_DATA;
   else 
      ret=DMS_ERROR;

   POP();
#undef SQL_FINDMETHOD_LIKE
}



/**************************************************************************/
LONG DLL_INTERFACE DMS_addTrait(DMS_TRAIT* Trt, CHAR* szPTra)
{
   CHAR szName[DMS_TRAIT_NAME];
   DMS_TRAIT ttrait;
   PUSH(DMS_addTrait);
   ttrait = *Trt;
   PadRight((CHAR *) Trt->TRNAME,DMS_TRAIT_NAME);
   ZeroFill(szName, sizeof(szName));
   strncpy(szName,(CHAR *) Trt->TRNAME, sizeof(Trt->TRNAME));
 //  ret=DMS_findTrait((CHAR *)szName, &ttrait, FIND_FIRST);
 //  if (ret == DMS_NO_DATA)
      ret = addTrait_v4(Trt, szPTra);
 //  else if (ret == DMS_SUCCESS)
 //     ret = DMS_EXIST;
   POP();
}




LONG DLL_INTERFACE DMS_addScale(DMS_SCALE* Scal)
{
   CHAR szName[DMS_TRAIT_NAME];
   DMS_SCALE tscale;
   PUSH(DMS_addScale);
   tscale = *Scal;
   PadRight((CHAR *) Scal->SCNAME, DMS_SCALE_NAME);
   ZeroFill(szName, sizeof(szName));
   strncpy(szName,(CHAR *) Scal->SCNAME, sizeof(Scal->SCNAME));
 //  ret = DMS_findScale((CHAR *)szName,&tscale,FIND_FIRST);
 //  if (ret == DMS_NO_DATA)
      ret = addScale(Scal);
 //  else if (ret==DMS_SUCCESS)
//	   ret = DMS_EXIST;
   POP();
}


LONG DLL_INTERFACE DMS_addTmethod(DMS_TMETHOD* Tmeth, CHAR* szPTme)
{
   CHAR szName[DMS_METHOD_NAME];
   DMS_TMETHOD ttmeth;
   PUSH(DMS_addTmethod);
   ttmeth = *Tmeth;
   PadRight((CHAR *) Tmeth->TMNAME,DMS_METHOD_NAME);
   ZeroFill(szName, DMS_METHOD_NAME);
   strncpy(szName, (CHAR *) Tmeth->TMNAME, DMS_METHOD_NAME);
//   ret = DMS_findTmethod((CHAR *) szName,&ttmeth,FIND_FIRST);
//   if (ret == DMS_NO_DATA)
      ret = addTmethod(Tmeth,szPTme);
//   else if (ret == DMS_SUCCESS)
//	   ret = DMS_EXIST;
   POP();
}


LONG DLL_INTERFACE DMS_findTrait_( CHAR *szName, DMS_TRAIT *Trait, LONG fopt)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_findTrait);
   if (fopt==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal){
      ret=findTrait_v4(LOCAL,szName, *Trait, fopt);
      if (ret!=GMS_SUCCESS){
         fLocal = FALSE; 
	     ret=findTrait_v4(CENTRAL,szName, *Trait,  FIND_NEXT);
      }
   }
   else
      ret=findTrait_v4(CENTRAL,szName, *Trait, fopt);

   POP();

} 


LONG DLL_INTERFACE DMS_findTrait( CHAR *szName, DMS_TRAIT *Trait, LONG fopt)
{
   static BOOL fLocal=TRUE;
   static BOOL name_wild=FALSE;

   PUSH(DMS_findTrait);
   if (fopt==FIND_FIRST){
	   fLocal=TRUE;
	  name_wild =strchr(szName,'_') || strchr(szName,'%');
   }
   if (name_wild) {
     if (fLocal){
       ret=findTraitLike(LOCAL,szName, *Trait, fopt);
       if (ret!=GMS_SUCCESS){
         fLocal = FALSE; 
	     ret=findTraitLike(CENTRAL,szName, *Trait,  FIND_NEXT);
       }
	 }
     else
      ret=findTraitLike(CENTRAL,szName, *Trait, fopt);
   } 
   else {
     if (fLocal){
       ret=findTraitEq(LOCAL,szName, *Trait, fopt);
       if (ret!=GMS_SUCCESS){
         fLocal = FALSE; 
	     ret=findTraitEq(CENTRAL,szName, *Trait,  FIND_NEXT);
       }
	 }
     else
      ret=findTraitEq(CENTRAL,szName, *Trait, fopt);
   }
   POP();

} 



LONG DLL_INTERFACE DMS_findScale_( CHAR *szName,DMS_SCALE *SCALE, LONG fopt)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_findScale);
   if (fopt==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal){
      ret=findScale(LOCAL,szName, *SCALE, fopt);
      if (ret!=GMS_SUCCESS){
         fLocal = FALSE; 
	     ret=findScale(CENTRAL,szName, *SCALE, FIND_NEXT);
      }
   }
   else
      ret=findScale(CENTRAL,szName, *SCALE,fopt);

   POP();

}



LONG DLL_INTERFACE DMS_findScale( CHAR *szName,DMS_SCALE *SCALE, LONG fopt)
{
   static BOOL fLocal=TRUE;
   static BOOL name_wild=FALSE;

   PUSH(DMS_findScale);
   if (fopt==FIND_FIRST){
         fLocal=TRUE;
	  name_wild =strchr(szName,'_') || strchr(szName,'%');
   }

   if (name_wild) {
     if (fLocal){
       ret=findScaleEq(LOCAL,szName, *SCALE, fopt);
       if (ret!=GMS_SUCCESS){
         fLocal = FALSE; 
	     ret=findScaleEq(CENTRAL,szName, *SCALE, FIND_NEXT);
       }
	 }
     else
       ret=findScaleEq(CENTRAL,szName, *SCALE,fopt);
   }
   else {
     if (fLocal){
       ret=findScaleLike(LOCAL,szName, *SCALE, fopt);
       if (ret!=GMS_SUCCESS){
         fLocal = FALSE; 
	     ret=findScaleLike(CENTRAL,szName, *SCALE, FIND_NEXT);
       }
	 }
     else
       ret=findScaleLike(CENTRAL,szName, *SCALE,fopt);
   }

   POP();

}


LONG DLL_INTERFACE DMS_findTmethod_( CHAR *szName, DMS_TMETHOD *Tmeth, LONG fopt)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_findTmethod);
   if (fopt==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal){
      ret=findTmethod(LOCAL,szName, *Tmeth, fopt);
      if (ret!=GMS_SUCCESS){
         fLocal = FALSE; 
	     ret=findTmethod(CENTRAL,szName, *Tmeth, FIND_NEXT);
      }
   }
   else
      ret=findTmethod(CENTRAL,szName, *Tmeth, fopt);

   POP();
}

LONG DLL_INTERFACE DMS_findTmethod( CHAR *szName, DMS_TMETHOD *Tmeth, LONG fopt)
{
   static BOOL fLocal=TRUE;
   static BOOL name_wild=FALSE;

   PUSH(DMS_findTmethod);
   if (fopt==FIND_FIRST) {
       fLocal=TRUE;
 	   name_wild =strchr(szName,'_') || strchr(szName,'%');
   }

   if (name_wild) {
     if (fLocal){
        ret=findTmethodLike(LOCAL,szName, *Tmeth, fopt);
        if (ret!=GMS_SUCCESS){
           fLocal = FALSE; 
	       ret=findTmethodLike(CENTRAL,szName, *Tmeth, FIND_NEXT);
		}
	 }
     else
        ret=findTmethodLike(CENTRAL,szName, *Tmeth, fopt);
   }
   else {
     if (fLocal){
        ret=findTmethodEq(LOCAL,szName, *Tmeth, fopt);
        if (ret!=GMS_SUCCESS){
           fLocal = FALSE; 
	       ret=findTmethodEq(CENTRAL,szName, *Tmeth, FIND_NEXT);
		}
	 }
     else
        ret=findTmethodEq(CENTRAL,szName, *Tmeth, fopt);
   }

   POP();

}

//////////////////////////////////////////////////////////////////////////////
// DMS_getTrait
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getTrait(DMS_TRAIT* Traits, UCHAR* szMDesc, LONG cMDesc, int fOpt)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_getTrait);

   if (fOpt==FIND_FIRST)
   {
      fLocal=TRUE;
   }
   if (fLocal){
      ret=getTrait_v4(LOCAL,Traits,szMDesc, cMDesc, fOpt);
      if (ret!=DMS_SUCCESS){
         fLocal = FALSE; 
         ret=getTrait_v4(CENTRAL,Traits,szMDesc,cMDesc, FIND_NEXT);
      }
   }
   else
      ret=getTrait_v4(CENTRAL,Traits,szMDesc,cMDesc,fOpt);
   POP();

}




//////////////////////////////////////////////////////////////////////////////
// DMS_getPrefTrait impemented for version 4
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getPrefTrait(DMS_TRAIT* Traits, UCHAR* szMDesc, LONG cMDesc, int fOpt)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_getPrefTrait);

   if (fOpt==FIND_FIRST)
   {
      fLocal=TRUE;
   }
   if (fLocal){
      ret=getPrefTrait(LOCAL,Traits,szMDesc, cMDesc, fOpt);
      if (ret!=DMS_SUCCESS){
         fLocal = FALSE; 
         ret=getPrefTrait(CENTRAL,Traits,szMDesc,cMDesc, FIND_NEXT);
      }
   }
   else
      ret=getPrefTrait(CENTRAL,Traits,szMDesc,cMDesc,fOpt);
   POP();

}


//////////////////////////////////////////////////////////////////////////////
// DMS_getScale
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getScale(DMS_SCALE* Scales, long fOpt)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_getScale);

   if (fOpt==FIND_FIRST)
   {
      fLocal=TRUE;
   }
   if (fLocal){
      ret=getScale(LOCAL,Scales, fOpt);
      if (ret!=DMS_SUCCESS){
         fLocal = FALSE; 
         ret=getScale(CENTRAL,Scales,FIND_NEXT);
      }
   }
   else
      ret=getScale(CENTRAL,Scales,fOpt);
   POP();

}


LONG DLL_INTERFACE DMS_getScaleCon(DMS_SCALECON* Scales, long fOpt)
// Get the start and end values of a scale
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_getScaleCon);

   if (fOpt==FIND_FIRST)
   {
      fLocal=TRUE;
   }
   if (fLocal){
      ret=getScaleCon(LOCAL,Scales, fOpt);
      if (ret!=DMS_SUCCESS){
         fLocal = FALSE; 
         ret=getScaleCon(CENTRAL,Scales,FIND_NEXT);
      }
   }
   else
      ret=getScaleCon(CENTRAL,Scales,fOpt);
   POP();
}

LONG DLL_INTERFACE DMS_getScaleDis(DMS_SCALEDIS* Scales, long fOpt)
// Get the start and end values of a scale
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_getScaleDis);

   if (fOpt==FIND_FIRST)
   {
      fLocal=TRUE;
   }
   if (fLocal){
      ret=getScaleDis(LOCAL,Scales, fOpt);
      if (ret!=DMS_SUCCESS){
         fLocal = FALSE; 
         ret=getScaleDis(CENTRAL,Scales,FIND_NEXT);
      }
   }
   else
      ret=getScaleDis(CENTRAL,Scales,fOpt);
   POP();
}


LONG DLL_INTERFACE DMS_getScaleDis2(DMS_SCALEDIS* Scales, long fOpt)
// Get the start and end values of a scale
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_getScaleDis2);

   if (fOpt==FIND_FIRST)
   {
      fLocal=TRUE;
   }
   if (fLocal){
      ret=getScaleDis2(LOCAL,Scales, fOpt);
      if (ret!=DMS_SUCCESS){
         fLocal = FALSE; 
         ret=getScaleDis2(CENTRAL,Scales,FIND_NEXT);
      }
   }
   else
      ret=getScaleDis2(CENTRAL,Scales,fOpt);
   POP();
}


LONG DLL_INTERFACE DMS_getScaleTab(LONG scaleid, CHAR *fname, LONG *noCol)
{
   static BOOL fLocal=TRUE;
   DMS_SCALETAB scaletab;
   PUSH(DMS_getScaleTab);
   scaletab.SCALEID = scaleid;
   ret = getScaleTab(LOCAL,&scaletab);
   if (ret == DMS_NO_DATA)
	   ret = getScaleTab(CENTRAL,&scaletab);
   ret = executeScaleTab(scaletab,fname,noCol);
   POP();
}


LONG DLL_INTERFACE DMS_getScaleTab2(LONG scaleid, CHAR *value, LONG nVal)
{
   static BOOL fLocal=TRUE;
   DMS_SCALETAB scaletab;
   PUSH(DMS_getScaleTab);
   scaletab.SCALEID = scaleid;
   ret = getScaleTab(LOCAL,&scaletab);
   if (ret == DMS_NO_DATA)
	   ret = getScaleTab(CENTRAL,&scaletab);
   ret = executeScaleTab2(scaletab,value, nVal);
   POP();
}




//////////////////////////////////////////////////////////////////////////////
// DMS_getTmethod
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getTmethod(DMS_TMETHOD* Tmethods, UCHAR* szMDesc, LONG cMDesc, int fOpt)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_getTmethod);

   if (fOpt==FIND_FIRST)
   {
      fLocal=TRUE;
   }
   if (fLocal){
      ret=getTmethod(LOCAL,Tmethods,szMDesc,cMDesc, fOpt);
      if (ret!=DMS_SUCCESS){
         fLocal = FALSE; 
         ret=getTmethod(CENTRAL,Tmethods,szMDesc,cMDesc, FIND_NEXT);
      }
   }
   else
      ret=getTmethod(CENTRAL,Tmethods,szMDesc,cMDesc,fOpt);
   POP();

}


LONG DLL_INTERFACE DMS_getScale2(DMS_SCALE* Scales, long fopt)
{
    static BOOL fLocal=TRUE;

    PUSH(DMS_getScale2);

    if (fopt==FIND_FIRST)
    {
      fLocal=TRUE;
    }
    if (fLocal){
 		 ret=getScale2(LOCAL,Scales,fopt);
      if (ret!=DMS_SUCCESS){
         fLocal = FALSE; 
  		 ret=getScale2(CENTRAL,Scales,FIND_NEXT);
      }
    }
    else
		ret=getScale2(CENTRAL,Scales,fopt);
    POP();
	

}


LONG DLL_INTERFACE DMS_getTmethod2(DMS_TMETHOD* Tmethods, UCHAR* szMDesc, LONG cMDesc, long fopt)
{
    PUSH(DMS_getTmethod2);

    static BOOL fLocal=TRUE;

    if (fopt==FIND_FIRST)
    {
      fLocal=TRUE;
    }
    if (fLocal){
 		 ret=getTmethod2(LOCAL,Tmethods,szMDesc, cMDesc, fopt);
      if (ret!=DMS_SUCCESS){
         fLocal = FALSE; 
		 ret=getTmethod2(CENTRAL,Tmethods,szMDesc, cMDesc, FIND_NEXT);
      }
    }
    else
		ret=getTmethod2(CENTRAL,Tmethods,szMDesc, cMDesc, fopt);
    POP();
	
}

//////////////////////////////////////////////////////////////////////////////
//added by WVC
//***** Gets all unique property names & their ontology  *****/
//Input:  none
//Output: property, scale, & method names
//Called by DMS_getUTrait
//////////////////////////////////////////////////////////////////////////////
LONG getUTrait(BOOL fLocal, DMS_UTRAIT *data, int fOpt)
{
	#define SQL_UTRAIT \
		"SELECT DISTINCT TRNAME, SCNAME, TMNAME \
		FROM TRAIT, SCALE, TMETHOD \
		WHERE (((TRAIT.TRAITID)=SCALE.TRAITID) \
		AND ((TRAIT.TRAITID)=TMETHOD.TRAITID)) ORDER BY TRNAME ASC"
	
	LONG ret = DMS_SUCCESS;
	
	static CODBCbindedStmt &central = _centralDMS->BindedStmt(SQL_UTRAIT);
	static CODBCbindedStmt &local = _localDMS->BindedStmt(SQL_UTRAIT);
	CODBCbindedStmt *source;
	
	static DMS_UTRAIT _data;
	static BOOL first_time = TRUE;
	
	if (fOpt == FIND_FIRST) {
		if (first_time) {
			
			BINDS(1, _data.TRNAME, DMS_TRAIT_NAME);
			BINDS(2, _data.SCNAME, DMS_SCALE_NAME);
			BINDS(3, _data.TMNAME, DMS_METHOD_NAME);
			
			first_time = FALSE;
		}
		local.Execute();
		central.Execute();
	}
	
	source = (fLocal) ? &local : &central;
	if (source->Fetch()) {
		ret = GMS_SUCCESS;
		*data = _data;
	}
	else if (source->NoData())
		ret = DMS_NO_DATA;
	else 
		ret = DMS_ERROR;
	
	return ret;
	
	#undef SQL_UTRAIT
}

//////////////////////////////////////////////////////////////////////////////
//added by WVC
//***** Gets all unique property names & their ontology *****/
//Input:  none
//Output: property, scale, & method names
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getUTrait(DMS_UTRAIT *data, int fOpt)
{
	static BOOL fLocal = TRUE;
	
	PUSH(DMS_getUTrait);
	
	if (fOpt == FIND_FIRST) {
		fLocal = TRUE;
	}
	if (fLocal) {
		ret = getUTrait(LOCAL, data, fOpt);
		if (ret != DMS_SUCCESS){
			fLocal = FALSE;
			ret = getUTrait(CENTRAL, data, FIND_NEXT);
		}
	}
	else
		ret = getUTrait(CENTRAL, data, fOpt);
	
	POP();
	
}





/*********
LONG getScaleTab2(BOOL fLocal,DMS_SCALETAB2* Scales)
{
#define SQL_GETSCALETAB2 "SELECT SCALEID,SQL,MODULE, PARAMETER, PTYPE "\
							"FROM SCALETAB "\
							"WHERE (SCALEID=?)" 

  PUSH(getScaleTab2);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_GETSCALETAB2);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_GETSCALETAB2);
       CODBCbindedStmt *source;

static DMS_SCALETAB2 _data;
static LONG _scaleid;
static BOOL first_time=TRUE;


   if (first_time)
	  {
		BINDPARAM(1, _scaleid);
												
        BIND(1, _data.SCALEID);		                   
        BINDS(2, _data.SQLTEXT, DMS_SCALE_SQL-2);
		BINDS(3,_data.MODULE, 5);
        BINDS(4, _data.PARAMETER, DMS_SCALETAB_PARAM);
        BINDS(5, _data.TYPE, DMS_SCALETAB_TYPE);


      first_time = FALSE;
   }
      _scaleid=Scales->SCALEID;
      local.Execute();
	  central.Execute();

   source = (fLocal)?&local:&central;

   if (source->Fetch())
   {
      ret=DMS_SUCCESS;
	  *Scales = _data;
   }
   else if (source->NoData())
      ret = DMS_NO_DATA;
   else 
      ret = DMS_ERROR;

   return ret;//POP();

#undef SQL_GETSCALETAB2
}


LONG executeScaleTab2(DMS_SCALETAB2 scaletab,CHAR *fname, LONG *noCol)
{

  PUSH(executeScaleTab);
  CODBCdirectStmt &central = _centralDBC->DirectStmt("SELECT * FROM SCALETAB");
  CODBCdirectStmt &local = _localDBC->DirectStmt("SELECT * FROM SCALETAB");
  CODBCparam param;
  CHAR *szData;
  FILE  *sFile;
  BOOLEAN fetchCentral;
  long i;

  fetchCentral = TRUE;
  if (strcmp((CHAR *)scaletab.MODULE ,(CHAR *)"GMS")==0) {
    central = _centralDBC->DirectStmt((LPCSTR) scaletab.SQLTEXT);
    local = _localDBC->DirectStmt((LPCSTR) scaletab.SQLTEXT);
	local.AddParam(&param,1);
	

  if (strcmp((CHAR *)scaletab.MODULE ,(CHAR *)"GMSLOC")==0) {
    local = _localDBC->DirectStmt((LPCSTR) scaletab.SQLTEXT);
	fetchCentral = FALSE;
  }
  if (strcmp((CHAR *)scaletab.MODULE ,(CHAR *)"DMS")==0) {
    central =  _centralDMS->DirectStmt((LPCSTR) scaletab.SQLTEXT);
    local = _localDMS->DirectStmt((LPCSTR) scaletab.SQLTEXT);
  }
  if (strcmp((CHAR *)scaletab.MODULE ,(CHAR *)"DMSLOC")==0) {
    local = _localDMS->DirectStmt((LPCSTR) scaletab.SQLTEXT);
	fetchCentral = FALSE;
  }

  sFile=fopen(fname,"w+"); \
  if (sFile!=NULL)\
       setvbuf( sFile, NULL, _IONBF, 0 );
     
  if (fetchCentral) {
   central.Execute();
   *noCol = central.ColCount();
   for (i=1;i<=*noCol;i++) {
      szData = ((CHAR *)central.Field(i).Name());
	  fprintf(sFile,szData);
	  if (i!= *noCol) 
	    fprintf(sFile," \t");
	  else
	    fprintf(sFile," \n");
   }
   while (central.Fetch()) {
	   for (i=1;i<=*noCol;i++) {
		  szData = ((CHAR *) central.Field(i).AsString());
		     fprintf(sFile,szData);
		  if (i!= *noCol) 
		     fprintf(sFile," \t");
		  else
			 fprintf(sFile," \n");
	   }
   }
  }

   local.Execute();
   *noCol = local.ColCount();
   while (local.Fetch()) {
	   for (i=1;i<=*noCol;i++) {
		  szData = ((CHAR *) local.Field(i).AsString());
		     fprintf(sFile,szData);
		  if (i!= *noCol) 
		     fprintf(sFile," \t");
		  else
			 fprintf(sFile," \n");
	   }
   }

   if (sFile!=NULL) fclose(sFile);
   ret=DMS_SUCCESS;
   return ret;//POP();

}


LONG DLL_INTERFACE DMS_getScaleTab2(LONG scaleid, CHAR *fname, LONG *noCol)
// Get the start and end values of a scale
{
   static BOOL fLocal=TRUE;
   DMS_SCALETAB2 scaletab;
   PUSH(DMS_getScaleTab2);
   scaletab.SCALEID = scaleid;
   ret = getScaleTab2(LOCAL,&scaletab);
   if (ret == DMS_NO_DATA)
	   ret = getScaleTab2(CENTRAL,&scaletab);
   ret = executeScaleTab2(scaletab,fname,noCol);
   POP();
}


**************/