/* Search.cpp : Implements the functions to query and retrieve data from the DMS of ICIS
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
 Modified  (AMP) :   
 **************************************************************/


#include "Search.h"

extern LONG UACCESS,USERID;
extern CODBCdbc *_centralDMS,*_localDMS;  //Central and Local Database Connection
extern stack<char*> _routines_stack;

LONG searchDataC(BOOL fLocal,LONG Trait, LONG Scale, LONG Method, LONG *Ounit, CHAR *DValueC, LONG cDataLen, LONG fopt)
{
#define SearchDC1_SQL "SELECT DATA_C.OUNITID, DATA_C.DVALUE FROM VARIATE, DATA_C \
        WHERE VARIATE.TRAITID=? AND DATA_C.VARIATID = VARIATE.VARIATID ORDER BY DATA_C.OUNITID"    //scale==0 and method==0
#define SearchDC2_SQL "SELECT DATA_C.OUNITID, DATA_C.DVALUE FROM VARIATE, DATA_C \
        WHERE VARIATE.TRAITID=? AND VARIATE.SCALEID=? AND DATA_C.VARIATID = VARIATE.VARIATID ORDER BY DATA_C.OUNITID"    //method==0
#define SearchDC3_SQL "SELECT DATA_C.OUNITID, DATA_C.DVALUE FROM VARIATE, DATA_C \
        WHERE VARIATE.TRAITID=? AND VARIATE.TMETHID=? AND DATA_C.VARIATID = VARIATE.VARIATID ORDER BY DATA_C.OUNITID"    //scale==0 
#define SearchDC4_SQL "SELECT DATA_C.OUNITID, DATA_C.DVALUE FROM VARIATE, DATA_C \
        WHERE VARIATE.TRAITID=? AND VARIATE.SCALEID = ? AND VARIATE.TMETHID=? AND  DATA_C.VARIATID = VARIATE.VARIATID ORDER BY DATA_C.OUNITID"

	PUSH(SearchDataC);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(SearchDC1_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SearchDC1_SQL);
       CODBCbindedStmt *source;

static LONG tounit;
static LONG lngTrait= Trait, lngScale=Scale, lngMeth=Method;
static CHAR szName[DMS_DATA_VALUE];
    string stmt_sql;

	*DValueC = '\0';
	if (fopt == FIND_FIRST) {
        lngTrait= Trait;
		lngScale=Scale;
		lngMeth=Method;
	if (Scale == 0) {
		if (Method == 0) {
		   stmt_sql=SearchDC1_SQL;
	       central.SetSQLstr(stmt_sql.c_str());
	       local.SetSQLstr(stmt_sql.c_str());
		   BINDPARAM(1, lngTrait);
		}
		else {
		   stmt_sql=SearchDC3_SQL;
	       central.SetSQLstr(stmt_sql.c_str());
	       local.SetSQLstr(stmt_sql.c_str());
		   BINDPARAM(1, lngTrait);
		   BINDPARAM(2, lngMeth);
		}
	}
	else if (Method == 0) {
	   stmt_sql=SearchDC2_SQL;
	   central.SetSQLstr(stmt_sql.c_str());
	   local.SetSQLstr(stmt_sql.c_str());
	   BINDPARAM(1, lngTrait);
	   BINDPARAM(2, lngScale);
	}
	else {
	   stmt_sql=SearchDC4_SQL;
	   central.SetSQLstr(stmt_sql.c_str());
	   local.SetSQLstr(stmt_sql.c_str());
	   BINDPARAM(1, lngTrait);
	   BINDPARAM(2, lngScale);
	   BINDPARAM(3, lngMeth);
	}
	BIND(1,tounit);
	BINDS(2,szName,DMS_DATA_VALUE-1);
    local.Execute();
    central.Execute();

	}

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*Ounit = tounit;
	    strncat(DValueC, szName, cDataLen);
		ret= DMS_SUCCESS;
	}
	else if (source->NoData())
		ret=  DMS_NO_DATA;
	else
		ret= DMS_ERROR;
    POP();
#undef SEARCHDC1_SQL
#undef SEARCHDC2_SQL
#undef SEARCHDC3_SQL
#undef SEARCHDC4_SQL
}

LONG searchDataC1(BOOL fLocal,LONG Trait, LONG Scale, LONG Method, LONG *Ounit, CHAR *DValueC, LONG cDataLen, LONG fopt)
{
#define SearchDC1_SQL "SELECT DATA_C.OUNITID, DATA_C.DVALUE FROM VARIATE, DATA_C \
        WHERE VARIATE.TRAITID=? AND DATA_C.VARIATID = VARIATE.VARIATID ORDER BY DATA_C.OUNITID"    //scale==0 and method==0

	PUSH(SearchDataC1);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(SearchDC1_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SearchDC1_SQL);
       CODBCbindedStmt *source;

static LONG tounit;
static LONG lngTrait= Trait, lngScale=Scale, lngMeth=Method;
static CHAR szName[DMS_DATA_VALUE];
static BOOL first_time=TRUE;
    string stmt_sql;

	*DValueC = '\0';
	if (fopt == FIND_FIRST) {
        lngTrait= Trait;
		lngScale=Scale;
		lngMeth=Method;
		if (first_time) {
          first_time = FALSE;
 	      BINDPARAM(1, lngTrait);
  	      BIND(1,tounit);
	      BINDS(2,szName,DMS_DATA_VALUE-1);
		}
        local.Execute();
        central.Execute();

	}

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*Ounit = tounit;
	    strncat(DValueC, szName, cDataLen);
		ret= DMS_SUCCESS;
	}
	else if (source->NoData())
		ret=  DMS_NO_DATA;
	else
		ret= DMS_ERROR;
    POP();
#undef SEARCHDC1_SQL
}



LONG searchDataC2(BOOL fLocal,LONG Trait, LONG Scale, LONG Method, LONG *Ounit, CHAR *DValueC, LONG cDataLen, LONG fopt)
{
#define SearchDC2_SQL "SELECT DATA_C.OUNITID, DATA_C.DVALUE FROM VARIATE, DATA_C \
        WHERE VARIATE.TRAITID=? AND VARIATE.SCALEID=? AND DATA_C.VARIATID = VARIATE.VARIATID ORDER BY DATA_C.OUNITID"    //method==0

	PUSH(SearchDataC2);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(SearchDC2_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SearchDC2_SQL);
       CODBCbindedStmt *source;

static LONG tounit;
static LONG lngTrait= Trait, lngScale=Scale, lngMeth=Method;
static CHAR szName[DMS_DATA_VALUE];
static BOOL first_time=TRUE;
    string stmt_sql;

	*DValueC = '\0';
	if (fopt == FIND_FIRST) {
       lngTrait= Trait;
	   lngScale=Scale;
	   lngMeth=Method;
		if (first_time) {
          first_time = FALSE;
	      BINDPARAM(1, lngTrait);
	      BINDPARAM(2, lngScale);
	      BIND(1,tounit);
	      BINDS(2,szName,DMS_DATA_VALUE-1);
		}
       local.Execute();
       central.Execute();

	}

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*Ounit = tounit;
	    strncat(DValueC, szName, cDataLen);
		ret= DMS_SUCCESS;
	}
	else if (source->NoData())
		ret=  DMS_NO_DATA;
	else
		ret= DMS_ERROR;
    POP();
#undef SEARCHDC2_SQL
}



LONG searchDataC3(BOOL fLocal,LONG Trait, LONG Scale, LONG Method, LONG *Ounit, CHAR *DValueC, LONG cDataLen, LONG fopt)
{
#define SearchDC3_SQL "SELECT DATA_C.OUNITID, DATA_C.DVALUE FROM VARIATE, DATA_C \
        WHERE VARIATE.TRAITID=? AND VARIATE.TMETHID=? AND DATA_C.VARIATID = VARIATE.VARIATID ORDER BY DATA_C.OUNITID"    //scale==0 

	PUSH(SearchDataC3);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(SearchDC3_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SearchDC3_SQL);
       CODBCbindedStmt *source;

static LONG tounit;
static LONG lngTrait= Trait, lngScale=Scale, lngMeth=Method;
static CHAR szName[DMS_DATA_VALUE];
static BOOL first_time=TRUE;
    string stmt_sql;

	*DValueC = '\0';
	if (fopt == FIND_FIRST) {
        lngTrait= Trait;
		lngScale=Scale;
		lngMeth=Method;
		if (first_time) {
          first_time = FALSE;
 	      BINDPARAM(1, lngTrait);
		  BINDPARAM(2, lngMeth);
	      BIND(1,tounit);
	      BINDS(2,szName,DMS_DATA_VALUE-1);
		}
        local.Execute();
        central.Execute();
	}

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*Ounit = tounit;
	    strncat(DValueC, szName, cDataLen);
		ret= DMS_SUCCESS;
	}
	else if (source->NoData())
		ret=  DMS_NO_DATA;
	else
		ret= DMS_ERROR;
    POP();
#undef SEARCHDC3_SQL
}


LONG searchDataC4(BOOL fLocal,LONG Trait, LONG Scale, LONG Method, LONG *Ounit, CHAR *DValueC, LONG cDataLen, LONG fopt)
{
#define SearchDC4_SQL "SELECT DATA_C.OUNITID, DATA_C.DVALUE FROM VARIATE, DATA_C \
        WHERE VARIATE.TRAITID=? AND VARIATE.SCALEID = ? AND VARIATE.TMETHID=? AND  DATA_C.VARIATID = VARIATE.VARIATID ORDER BY DATA_C.OUNITID"

	PUSH(SearchDataC4);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(SearchDC4_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SearchDC4_SQL);
       CODBCbindedStmt *source;

static LONG tounit;
static LONG lngTrait= Trait, lngScale=Scale, lngMeth=Method;
static CHAR szName[DMS_DATA_VALUE];
static BOOL first_time=TRUE;
    string stmt_sql;

	*DValueC = '\0';
	if (fopt == FIND_FIRST) {
        lngTrait= Trait;
		lngScale=Scale;
		lngMeth=Method;
		if (first_time) {
 	      BINDPARAM(1, lngTrait);
	      BINDPARAM(2, lngScale);
	      BINDPARAM(3, lngMeth);
 	      BIND(1,tounit);
	      BINDS(2,szName,DMS_DATA_VALUE-1);
          first_time = FALSE;
		}
        local.Execute();
        central.Execute();

	}

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*Ounit = tounit;
	    strncat(DValueC, szName, cDataLen);
		ret= DMS_SUCCESS;
	}
	else if (source->NoData())
		ret=  DMS_NO_DATA;
	else
		ret= DMS_ERROR;
    POP();
#undef SEARCHDC4_SQL
}



LONG searchDataN(BOOL fLocal,LONG Trait, LONG Scale, LONG Method, LONG *Ounit,  double *DValueN, LONG fopt)
{
#define SearchDN1_SQL "SELECT DATA_N.OUNITID, DATA_N.DVALUE FROM VARIATE, DATA_N \
        WHERE VARIATE.TRAITID=? AND DATA_N.VARIATID = VARIATE.VARIATID ORDER BY DATA_N.OUNITID"    //scale==0 and method==0
#define SearchDN2_SQL "SELECT DATA_N.OUNITID, DATA_N.DVALUE FROM VARIATE, DATA_N \
        WHERE VARIATE.TRAITID=? AND VARIATE.SCALEID=? AND DATA_N.VARIATID = VARIATE.VARIATID ORDER BY DATA_N.OUNITID"    //method==0
#define SearchDN3_SQL "SELECT DATA_N.OUNITID, DATA_N.DVALUE FROM VARIATE, DATA_N \
        WHERE VARIATE.TRAITID=? AND VARIATE.TMETHID=? AND DATA_N.VARIATID = VARIATE.VARIATID ORDER BY DATA_N.OUNITID"    //scale==0 
#define SearchDN4_SQL "SELECT DATA_N.OUNITID, DATA_N.DVALUE FROM VARIATE, DATA_N \
        WHERE VARIATE.TRAITID=? AND VARIATE.SCALEID = ? AND VARIATE.TMETHID=? AND DATA_N.VARIATID = VARIATE.VARIATID ORDER BY DATA_N.OUNITID"


	PUSH(SearchDataN);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(SearchDN1_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SearchDN1_SQL);
       CODBCbindedStmt *source;

static LONG tounit;
static double dvalue;
static LONG lngTrait= Trait, lngScale=Scale, lngMeth=Method;
static CHAR szName[DMS_DATA_VALUE];
    string stmt_sql;

	*DValueN = 0;
	if (fopt == FIND_FIRST) {
		lngTrait= Trait;
		lngScale=Scale;
		lngMeth=Method;
	if (Scale == 0) {
		if (Method == 0) {
		   stmt_sql=SearchDN1_SQL;
	       central.SetSQLstr(stmt_sql.c_str());
	       local.SetSQLstr(stmt_sql.c_str());
		   BINDPARAM(1, lngTrait);
		}
		else {
		   stmt_sql=SearchDN3_SQL;
	       central.SetSQLstr(stmt_sql.c_str());
	       local.SetSQLstr(stmt_sql.c_str());
		   BINDPARAM(1, lngTrait);
		   BINDPARAM(2, lngMeth);
		}
	}
	else if (Method == 0) {
	   stmt_sql=SearchDN2_SQL;
	   central.SetSQLstr(stmt_sql.c_str());
	   local.SetSQLstr(stmt_sql.c_str());
	   BINDPARAM(1, lngTrait);
	   BINDPARAM(2, lngScale);
	}
	else {
	   stmt_sql=SearchDN4_SQL;
	   central.SetSQLstr(stmt_sql.c_str());
	   local.SetSQLstr(stmt_sql.c_str());
	   BINDPARAM(1, lngTrait);
	   BINDPARAM(2, lngScale);
	   BINDPARAM(3, lngMeth);
	}
	BIND(1,tounit);
	BIND(2,dvalue);
    local.Execute();
    central.Execute();

	}

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*Ounit = tounit;
	    *DValueN = dvalue;
		ret= DMS_SUCCESS;
	}
	else if (source->NoData())
		ret=  DMS_NO_DATA;
	else
		ret= DMS_ERROR;
    POP();
#undef SEARCHDN1_SQL
#undef SEARCHDN2_SQL
#undef SEARCHDN3_SQL
#undef SEARCHDN4_SQL
}

LONG searchDataN1(BOOL fLocal,LONG Trait, LONG Scale, LONG Method, LONG *Ounit,  double *DValueN, LONG fopt)
{
#define SearchDN1_SQL "SELECT DATA_N.OUNITID, DATA_N.DVALUE FROM VARIATE, DATA_N \
        WHERE VARIATE.TRAITID=? AND DATA_N.VARIATID = VARIATE.VARIATID ORDER BY DATA_N.OUNITID"    //scale==0 and method==0


	PUSH(SearchDataN1);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(SearchDN1_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SearchDN1_SQL);
       CODBCbindedStmt *source;

static LONG tounit;
static double dvalue;
static LONG lngTrait= Trait, lngScale=Scale, lngMeth=Method;
static CHAR szName[DMS_DATA_VALUE];
static BOOL first_time = TRUE;
    string stmt_sql;

	*DValueN = 0;
	if (fopt == FIND_FIRST) {
		lngTrait= Trait;
		lngScale=Scale;
		lngMeth=Method;
		if (first_time) {
			first_time = FALSE;
 	        BINDPARAM(1, lngTrait);
	        BIND(1,tounit);
	        BIND(2,dvalue);
		}
        local.Execute();
        central.Execute();

	}

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*Ounit = tounit;
	    *DValueN = dvalue;
		ret= DMS_SUCCESS;
	}
	else if (source->NoData())
		ret=  DMS_NO_DATA;
	else
		ret= DMS_ERROR;
    POP();
#undef SEARCHDN1_SQL
}



LONG searchDataN2(BOOL fLocal,LONG Trait, LONG Scale, LONG Method, LONG *Ounit,  double *DValueN, LONG fopt)
{
#define SearchDN2_SQL "SELECT DATA_N.OUNITID, DATA_N.DVALUE FROM VARIATE, DATA_N \
        WHERE VARIATE.TRAITID=? AND VARIATE.SCALEID=? AND DATA_N.VARIATID = VARIATE.VARIATID ORDER BY DATA_N.OUNITID"    //method==0


	PUSH(SearchDataN2);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(SearchDN2_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SearchDN2_SQL);
       CODBCbindedStmt *source;

static LONG tounit;
static double dvalue;
static LONG lngTrait= Trait, lngScale=Scale, lngMeth=Method;
static CHAR szName[DMS_DATA_VALUE];
static BOOL first_time = TRUE;
    string stmt_sql;

	*DValueN = 0;
	if (fopt == FIND_FIRST) {
		lngTrait= Trait;
		lngScale=Scale;
		lngMeth=Method;
		if (first_time) {
			first_time = FALSE;
 	        BINDPARAM(1, lngTrait);
 	        BINDPARAM(2, lngScale);
	        BIND(1,tounit);
	        BIND(2,dvalue);
		}
        local.Execute();
        central.Execute();

	}

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*Ounit = tounit;
	    *DValueN = dvalue;
		ret= DMS_SUCCESS;
	}
	else if (source->NoData())
		ret=  DMS_NO_DATA;
	else
		ret= DMS_ERROR;
    POP();
#undef SEARCHDN2_SQL
}


LONG searchDataN3(BOOL fLocal,LONG Trait, LONG Scale, LONG Method, LONG *Ounit,  double *DValueN, LONG fopt)
{
#define SearchDN3_SQL "SELECT DATA_N.OUNITID, DATA_N.DVALUE FROM VARIATE, DATA_N \
        WHERE VARIATE.TRAITID=? AND VARIATE.TMETHID=? AND DATA_N.VARIATID = VARIATE.VARIATID ORDER BY DATA_N.OUNITID"    //scale==0 


	PUSH(SearchDataN3);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(SearchDN3_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SearchDN3_SQL);
       CODBCbindedStmt *source;

static LONG tounit;
static double dvalue;
static LONG lngTrait= Trait, lngScale=Scale, lngMeth=Method;
static CHAR szName[DMS_DATA_VALUE];
static BOOL first_time = TRUE;
    string stmt_sql;

	*DValueN = 0;
	if (fopt == FIND_FIRST) {
		lngTrait= Trait;
		lngScale=Scale;
		lngMeth=Method;
		if (first_time) {
			first_time = FALSE;
 	        BINDPARAM(1, lngTrait);
 	        BINDPARAM(2, lngMeth);
	        BIND(1,tounit);
	        BIND(2,dvalue);
		}
        local.Execute();
        central.Execute();

	}

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*Ounit = tounit;
	    *DValueN = dvalue;
		ret= DMS_SUCCESS;
	}
	else if (source->NoData())
		ret=  DMS_NO_DATA;
	else
		ret= DMS_ERROR;
    POP();
#undef SEARCHDN3_SQL
}


LONG searchDataN4(BOOL fLocal,LONG Trait, LONG Scale, LONG Method, LONG *Ounit,  double *DValueN, LONG fopt)
{
#define SearchDN4_SQL "SELECT DATA_N.OUNITID, DATA_N.DVALUE FROM VARIATE, DATA_N \
        WHERE VARIATE.TRAITID=? AND VARIATE.SCALEID = ? AND VARIATE.TMETHID=? AND DATA_N.VARIATID = VARIATE.VARIATID ORDER BY DATA_N.OUNITID"


	PUSH(SearchDataN4);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(SearchDN4_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SearchDN4_SQL);
       CODBCbindedStmt *source;

static LONG tounit;
static double dvalue;
static LONG lngTrait= Trait, lngScale=Scale, lngMeth=Method;
static CHAR szName[DMS_DATA_VALUE];
static BOOL first_time = TRUE;
    string stmt_sql;

	*DValueN = 0;
	if (fopt == FIND_FIRST) {
		lngTrait= Trait;
		lngScale=Scale;
		lngMeth=Method;
		if (first_time) {
			first_time = FALSE;
 	        BINDPARAM(1, lngTrait);
 	        BINDPARAM(2, lngScale);
 	        BINDPARAM(3, lngMeth);
	        BIND(1,tounit);
	        BIND(2,dvalue);
		}
        local.Execute();
        central.Execute();

	}

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*Ounit = tounit;
	    *DValueN = dvalue;
		ret= DMS_SUCCESS;
	}
	else if (source->NoData())
		ret=  DMS_NO_DATA;
	else
		ret= DMS_ERROR;
    POP();
#undef SEARCHDN4_SQL
}




LONG searchLevelN(BOOL fLocal,LONG Trait, LONG Scale, LONG Method, LONG *Ounit, double *DValueN, LONG fopt)
{
#define SearchLN1_SQL "SELECT OINDEX.OUNITID,  LEVEL_N.LVALUE FROM OINDEX, FACTOR,  LEVEL_N \
        WHERE FACTOR.TRAITID=? AND  LEVEL_N.LABELID = FACTOR.LABELID AND OINDEX.FACTORID =  LEVEL_N.FACTORID \
		AND OINDEX.LEVELNO =  LEVEL_N.LEVELNO ORDER BY OINDEX.OUNITID"    //scale==0 and method==0
#define SearchLN2_SQL "SELECT OINDEX.OUNITID,  LEVEL_N.LVALUE FROM OINDEX, FACTOR,  LEVEL_N \
        WHERE FACTOR.TRAITID=? AND FACTOR.SCALEID = ? AND  LEVEL_N.LABELID = FACTOR.LABELID AND \
		INDEX.FACTORID =  LEVEL_N.FACTORID AND OINDEX.LEVELNO =  LEVEL_N.LEVELNO ORDER BY OINDEX.OUNITID"   //METHOD ==0
#define SearchLN3_SQL "SELECT OINDEX.OUNITID,  LEVEL_N.LVALUE FROM OINDEX, FACTOR,  LEVEL_N \
        WHERE FACTOR.TRAITID=? AND FACTOR.TMETHID = ? AND  LEVEL_N.LABELID = FACTOR.LABELID AND \
		OINDEX.FACTORID =  LEVEL_N.FACTORID AND OINDEX.LEVELNO =  LEVEL_N.LEVELNO ORDER BY OINDEX.OUNITID"   //SCALE ==0
#define SearchLN4_SQL "SELECT OINDEX.OUNITID,  LEVEL_N.LVALUE FROM OINDEX, FACTOR,  LEVEL_N \
        WHERE FACTOR.TRAITID=? AND FACTOR.SCALEID =? AND FACTOR.TMETHID = ? AND  LEVEL_N.LABELID = FACTOR.LABELID AND \
		OINDEX.FACTORID =  LEVEL_N.FACTORID AND OINDEX.LEVELNO =  LEVEL_N.LEVELNO ORDER BY OINDEX.OUNITID"   

	PUSH(SearchLevelN);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(SearchLN1_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SearchLN1_SQL);
       CODBCbindedStmt *source;

static LONG tounit;
static double dvalue;
static LONG lngTrait= Trait, lngScale=Scale, lngMeth=Method;
static CHAR szName[DMS_DATA_VALUE];
    string stmt_sql;

	*DValueN = 0;
	if (fopt == FIND_FIRST) {
		lngTrait= Trait;
		lngScale=Scale;
		lngMeth=Method;
	if (Scale == 0) {
		if (Method == 0) {
		   stmt_sql=SearchLN1_SQL;
	       central.SetSQLstr(stmt_sql.c_str());
	       local.SetSQLstr(stmt_sql.c_str());
		   BINDPARAM(1, lngTrait);
		}
		else {
		   stmt_sql=SearchLN3_SQL;
	       central.SetSQLstr(stmt_sql.c_str());
	       local.SetSQLstr(stmt_sql.c_str());
		   BINDPARAM(1, lngTrait);
		   BINDPARAM(2, lngMeth);
		}
	}
	else if (Method == 0) {
	   stmt_sql=SearchLN2_SQL;
	   central.SetSQLstr(stmt_sql.c_str());
	   local.SetSQLstr(stmt_sql.c_str());
	   BINDPARAM(1, lngTrait);
	   BINDPARAM(2, lngScale);
	}
	else {
	   stmt_sql=SearchLN4_SQL;
	   central.SetSQLstr(stmt_sql.c_str());
	   local.SetSQLstr(stmt_sql.c_str());
	   BINDPARAM(1, lngTrait);
	   BINDPARAM(2, lngScale);
	   BINDPARAM(3, lngMeth);
	}
	BIND(1,tounit);
	BIND(2,dvalue);
    local.Execute();
    central.Execute();

	}

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*Ounit = tounit;
        *DValueN = dvalue;
		ret= DMS_SUCCESS;
	}
	else if (source->NoData())
		ret=  DMS_NO_DATA;
	else
		ret= DMS_ERROR;
    POP();
#undef SEARCHLN1_SQL
#undef SEARCHLN2_SQL
#undef SEARCHLN3_SQL
#undef SEARCHLN4_SQL
}


LONG searchLevelN4(BOOL fLocal,LONG Trait, LONG Scale, LONG Method, LONG *Ounit, double *DValueN, LONG fopt)
{
#define SearchLN4_SQL "SELECT OINDEX.OUNITID,  LEVEL_N.LVALUE FROM OINDEX, FACTOR,  LEVEL_N \
        WHERE FACTOR.TRAITID=? AND FACTOR.SCALEID =? AND FACTOR.TMETHID = ? AND  LEVEL_N.LABELID = FACTOR.LABELID AND \
		OINDEX.FACTORID =  LEVEL_N.FACTORID AND OINDEX.LEVELNO =  LEVEL_N.LEVELNO ORDER BY OINDEX.OUNITID"   //SCALE ==0

	PUSH(SearchLevelN4);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(SearchLN4_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SearchLN4_SQL);
       CODBCbindedStmt *source;

static LONG tounit;
static double dvalue;
static LONG lngTrait= Trait, lngScale=Scale, lngMeth=Method;
static CHAR szName[DMS_DATA_VALUE];
static first_time=TRUE;
    string stmt_sql;

	*DValueN = 0;
	if (fopt == FIND_FIRST) {
		lngTrait= Trait;
		lngScale=Scale;
		lngMeth=Method;
		if (first_time) {
			first_time = FALSE;
	        BINDPARAM(1, lngTrait);
	        BINDPARAM(2, lngScale);
	        BINDPARAM(3, lngMeth);
	        BIND(1,tounit);
	        BIND(2,dvalue);
        }
        local.Execute();
        central.Execute();
	}

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*Ounit = tounit;
        *DValueN = dvalue;
		ret= DMS_SUCCESS;
	}
	else if (source->NoData())
		ret=  DMS_NO_DATA;
	else
		ret= DMS_ERROR;
    POP();
#undef SEARCHLN4_SQL
}



LONG searchLevelN3(BOOL fLocal,LONG Trait, LONG Scale, LONG Method, LONG *Ounit, double *DValueN, LONG fopt)
{
#define SearchLN3_SQL "SELECT OINDEX.OUNITID,  LEVEL_N.LVALUE FROM OINDEX, FACTOR,  LEVEL_N \
        WHERE FACTOR.TRAITID=? AND FACTOR.TMETHID = ? AND  LEVEL_N.LABELID = FACTOR.LABELID AND \
		OINDEX.FACTORID =  LEVEL_N.FACTORID AND OINDEX.LEVELNO =  LEVEL_N.LEVELNO ORDER BY OINDEX.OUNITID"   //SCALE ==0

	PUSH(SearchLevelN3);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(SearchLN3_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SearchLN3_SQL);
       CODBCbindedStmt *source;

static LONG tounit;
static double dvalue;
static LONG lngTrait= Trait, lngScale=Scale, lngMeth=Method;
static CHAR szName[DMS_DATA_VALUE];
static first_time=TRUE;
    string stmt_sql;

	*DValueN = 0;
	if (fopt == FIND_FIRST) {
		lngTrait= Trait;
		lngScale=Scale;
		lngMeth=Method;
		if (first_time) {
			first_time = FALSE;
	        BINDPARAM(1, lngTrait);
	        BINDPARAM(2, lngMeth);
	        BIND(1,tounit);
	        BIND(2,dvalue);
        }
        local.Execute();
        central.Execute();
	}

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*Ounit = tounit;
        *DValueN = dvalue;
		ret= DMS_SUCCESS;
	}
	else if (source->NoData())
		ret=  DMS_NO_DATA;
	else
		ret= DMS_ERROR;
    POP();
#undef SEARCHLN3_SQL
}


LONG searchLevelN2(BOOL fLocal,LONG Trait, LONG Scale, LONG Method, LONG *Ounit, double *DValueN, LONG fopt)
{
#define SearchLN2_SQL "SELECT OINDEX.OUNITID,  LEVEL_N.LVALUE FROM OINDEX, FACTOR,  LEVEL_N \
        WHERE FACTOR.TRAITID=? AND FACTOR.SCALEID = ? AND  LEVEL_N.LABELID = FACTOR.LABELID AND \
		INDEX.FACTORID =  LEVEL_N.FACTORID AND OINDEX.LEVELNO =  LEVEL_N.LEVELNO ORDER BY OINDEX.OUNITID"   //METHOD ==0

	PUSH(SearchLevelN2);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(SearchLN2_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SearchLN2_SQL);
       CODBCbindedStmt *source;

static LONG tounit;
static double dvalue;
static LONG lngTrait= Trait, lngScale=Scale, lngMeth=Method;
static CHAR szName[DMS_DATA_VALUE];
static first_time=TRUE;
    string stmt_sql;

	*DValueN = 0;
	if (fopt == FIND_FIRST) {
		lngTrait= Trait;
		lngScale=Scale;
		lngMeth=Method;
		if (first_time) {
			first_time = FALSE;
	        BINDPARAM(1, lngTrait);
	        BINDPARAM(2, lngScale);
	        BIND(1,tounit);
	        BIND(2,dvalue);
        }
        local.Execute();
        central.Execute();
	}

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*Ounit = tounit;
        *DValueN = dvalue;
		ret= DMS_SUCCESS;
	}
	else if (source->NoData())
		ret=  DMS_NO_DATA;
	else
		ret= DMS_ERROR;
    POP();
#undef SEARCHLN2_SQL
}


LONG searchLevelN1(BOOL fLocal,LONG Trait, LONG Scale, LONG Method, LONG *Ounit, double *DValueN, LONG fopt)
{
#define SearchLN1_SQL "SELECT OINDEX.OUNITID,  LEVEL_N.LVALUE FROM OINDEX, FACTOR,  LEVEL_N \
        WHERE FACTOR.TRAITID=? AND  LEVEL_N.LABELID = FACTOR.LABELID AND OINDEX.FACTORID =  LEVEL_N.FACTORID \
		AND OINDEX.LEVELNO =  LEVEL_N.LEVELNO ORDER BY OINDEX.OUNITID"    //scale==0 and method==0

	PUSH(SearchLevelN1);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(SearchLN1_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SearchLN1_SQL);
       CODBCbindedStmt *source;

static LONG tounit;
static double dvalue;
static LONG lngTrait= Trait, lngScale=Scale, lngMeth=Method;
static CHAR szName[DMS_DATA_VALUE];
static first_time=TRUE;
    string stmt_sql;

	*DValueN = 0;
	if (fopt == FIND_FIRST) {
		lngTrait= Trait;
		lngScale=Scale;
		lngMeth=Method;
		if (first_time) {
			first_time = FALSE;
	        BINDPARAM(1, lngTrait);
	        BIND(1,tounit);
	        BIND(2,dvalue);
        }
        local.Execute();
        central.Execute();
	}

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*Ounit = tounit;
        *DValueN = dvalue;
		ret= DMS_SUCCESS;
	}
	else if (source->NoData())
		ret=  DMS_NO_DATA;
	else
		ret= DMS_ERROR;
    POP();
#undef SEARCHLN1_SQL
}


LONG searchLevelC(BOOL fLocal,LONG Trait, LONG Scale, LONG Method, LONG *Ounit, CHAR *DValueC, LONG cDataLen, LONG fopt)
{
#define SearchLC1_SQL "SELECT OINDEX.OUNITID, LEVEL_C.LVALUE FROM OINDEX, FACTOR, LEVEL_C \
        WHERE FACTOR.TRAITID=? AND LEVEL_C.LABELID = FACTOR.LABELID AND OINDEX.FACTORID = LEVEL_C.FACTORID \
		AND OINDEX.LEVELNO = LEVEL_C.LEVELNO ORDER BY OINDEX.OUNITID"    //scale==0 and method==0
#define SearchLC2_SQL "SELECT OINDEX.OUNITID, LEVEL_C.LVALUE FROM OINDEX, FACTOR, LEVEL_C \
        WHERE FACTOR.TRAITID=? AND FACTOR.SCALEID = ? AND LEVEL_C.LABELID = FACTOR.LABELID AND \
		OINDEX.FACTORID = LEVEL_C.FACTORID AND OINDEX.LEVELNO = LEVEL_C.LEVELNO ORDER BY OINDEX.OUNITID"   //METHOD ==0
#define SearchLC3_SQL "SELECT OINDEX.OUNITID, LEVEL_C.LVALUE FROM OINDEX, FACTOR, LEVEL_C \
        WHERE FACTOR.TRAITID=? AND FACTOR.TMETHID = ? AND LEVEL_C.LABELID = FACTOR.LABELID AND \
		OINDEX.FACTORID = LEVEL_C.FACTORID AND OINDEX.LEVELNO = LEVEL_C.LEVELNO ORDER BY OINDEX.OUNITID"   //SCALE ==0
#define SearchLC4_SQL "SELECT OINDEX.OUNITID, LEVEL_C.LVALUE FROM OINDEX, FACTOR, LEVEL_C \
        WHERE FACTOR.TRAITID=? AND FACTOR.SCALEID =? AND FACTOR.TMETHID = ? AND LEVEL_C.LABELID = FACTOR.LABELID AND \
		OINDEX.FACTORID = LEVEL_C.FACTORID AND OINDEX.LEVELNO = LEVEL_C.LEVELNO ORDER BY OINDEX.OUNITID"   


	PUSH(SearchLevelC);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(SearchLC1_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SearchLC1_SQL);
       CODBCbindedStmt *source;

static LONG tounit;
static LONG lngTrait= Trait, lngScale=Scale, lngMeth=Method;
static CHAR szName[DMS_DATA_VALUE-1];
    string stmt_sql;

	*DValueC = '\0';
	if (fopt == FIND_FIRST) {
    	lngTrait= Trait;
		lngScale=Scale;
		lngMeth=Method;

	if (Scale == 0) {
		if (Method == 0) {
		   stmt_sql=SearchLC1_SQL;
	       central.SetSQLstr(stmt_sql.c_str());
	       local.SetSQLstr(stmt_sql.c_str());
		   BINDPARAM(1, lngTrait);
		}
		else {
		   stmt_sql=SearchLC3_SQL;
	       central.SetSQLstr(stmt_sql.c_str());
	       local.SetSQLstr(stmt_sql.c_str());
		   BINDPARAM(1, lngTrait);
		   BINDPARAM(2, lngMeth);
		}
	}
	else if (Method == 0) {
	   stmt_sql=SearchLC2_SQL;
	   central.SetSQLstr(stmt_sql.c_str());
	   local.SetSQLstr(stmt_sql.c_str());
	   BINDPARAM(1, lngTrait);
	   BINDPARAM(2, lngScale);
	}
	else {
	   stmt_sql=SearchLC4_SQL;
	   central.SetSQLstr(stmt_sql.c_str());
	   local.SetSQLstr(stmt_sql.c_str());
	   BINDPARAM(1, lngTrait);
	   BINDPARAM(2, lngScale);
	   BINDPARAM(3, lngMeth);
	}
	BIND(1,tounit);
	BINDS(2,szName,DMS_DATA_VALUE);   //BINDS(2,szName,DMS_DATA_VALUE-1);  
    local.Execute();
    central.Execute();

	}

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*Ounit = tounit;
	    strncat(DValueC, szName, cDataLen);
		ret= DMS_SUCCESS;
	}
	else if (source->NoData())
		ret=  DMS_NO_DATA;
	else
		ret= DMS_ERROR;
    POP();
#undef SEARCHLC1_SQL
#undef SEARCHLC2_SQL
#undef SEARCHLC3_SQL
#undef SEARCHLC4_SQL
}


LONG searchLevelC1(BOOL fLocal,LONG Trait, LONG Scale, LONG Method, LONG *Ounit, CHAR *DValueC, LONG cDataLen, LONG fopt)
{
#define SearchLC1_SQL "SELECT OINDEX.OUNITID, LEVEL_C.LVALUE FROM OINDEX, FACTOR, LEVEL_C \
        WHERE FACTOR.TRAITID=? AND LEVEL_C.LABELID = FACTOR.LABELID AND OINDEX.FACTORID = LEVEL_C.FACTORID \
		AND OINDEX.LEVELNO = LEVEL_C.LEVELNO ORDER BY OINDEX.OUNITID"    //scale==0 and method==0


	PUSH(SearchLevelC1);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(SearchLC1_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SearchLC1_SQL);
       CODBCbindedStmt *source;

static LONG tounit;
static LONG lngTrait= Trait, lngScale=Scale, lngMeth=Method;
static CHAR szName[DMS_DATA_VALUE-1];
BOOL first_time=TRUE;
    string stmt_sql;

	*DValueC = '\0';
	if (fopt == FIND_FIRST) {
    	lngTrait= Trait;
		lngScale=Scale;
		lngMeth=Method;
        if (first_time) {
			first_time = FALSE;
	        BINDPARAM(1, lngTrait);
		}
	    BIND(1,tounit);
	    BINDS(2,szName,DMS_DATA_VALUE);   //BINDS(2,szName,DMS_DATA_VALUE-1);  
        local.Execute();
        central.Execute();

	}

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*Ounit = tounit;
	    strncat(DValueC, szName, cDataLen);
		ret= DMS_SUCCESS;
	}
	else if (source->NoData())
		ret=  DMS_NO_DATA;
	else
		ret= DMS_ERROR;
    POP();
#undef SEARCHLC1_SQL
}

LONG searchLevelC2(BOOL fLocal,LONG Trait, LONG Scale, LONG Method, LONG *Ounit, CHAR *DValueC, LONG cDataLen, LONG fopt)
{
#define SearchLC2_SQL "SELECT OINDEX.OUNITID, LEVEL_C.LVALUE FROM OINDEX, FACTOR, LEVEL_C \
        WHERE FACTOR.TRAITID=? AND FACTOR.SCALEID = ? AND LEVEL_C.LABELID = FACTOR.LABELID AND \
		OINDEX.FACTORID = LEVEL_C.FACTORID AND OINDEX.LEVELNO = LEVEL_C.LEVELNO ORDER BY OINDEX.OUNITID"   //METHOD ==0


	PUSH(SearchLevelC2);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(SearchLC2_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SearchLC2_SQL);
       CODBCbindedStmt *source;

static LONG tounit;
static LONG lngTrait= Trait, lngScale=Scale, lngMeth=Method;
static CHAR szName[DMS_DATA_VALUE-1];
BOOL first_time=TRUE;
    string stmt_sql;

	*DValueC = '\0';
	if (fopt == FIND_FIRST) {
    	lngTrait= Trait;
		lngScale=Scale;
		lngMeth=Method;
        if (first_time) {
			first_time = FALSE;
	        BINDPARAM(1, lngTrait);
	        BINDPARAM(2, lngScale);
		}
	    BIND(1,tounit);
	    BINDS(2,szName,DMS_DATA_VALUE);   //BINDS(2,szName,DMS_DATA_VALUE-1);  
        local.Execute();
        central.Execute();

	}

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*Ounit = tounit;
	    strncat(DValueC, szName, cDataLen);
		ret= DMS_SUCCESS;
	}
	else if (source->NoData())
		ret=  DMS_NO_DATA;
	else
		ret= DMS_ERROR;
    POP();
#undef SEARCHLC2_SQL
}


LONG searchLevelC3(BOOL fLocal,LONG Trait, LONG Scale, LONG Method, LONG *Ounit, CHAR *DValueC, LONG cDataLen, LONG fopt)
{
#define SearchLC3_SQL "SELECT OINDEX.OUNITID, LEVEL_C.LVALUE FROM OINDEX, FACTOR, LEVEL_C \
        WHERE FACTOR.TRAITID=? AND FACTOR.TMETHID = ? AND LEVEL_C.LABELID = FACTOR.LABELID AND \
		OINDEX.FACTORID = LEVEL_C.FACTORID AND OINDEX.LEVELNO = LEVEL_C.LEVELNO ORDER BY OINDEX.OUNITID"   //SCALE ==0


	PUSH(SearchLevelC3);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(SearchLC3_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SearchLC3_SQL);
       CODBCbindedStmt *source;

static LONG tounit;
static LONG lngTrait= Trait, lngScale=Scale, lngMeth=Method;
static CHAR szName[DMS_DATA_VALUE-1];
BOOL first_time=TRUE;
    string stmt_sql;

	*DValueC = '\0';
	if (fopt == FIND_FIRST) {
    	lngTrait= Trait;
		lngScale=Scale;
		lngMeth=Method;
        if (first_time) {
			first_time = FALSE;
	        BINDPARAM(1, lngTrait);
	        BINDPARAM(3, lngMeth);
		}
	    BIND(1,tounit);
	    BINDS(2,szName,DMS_DATA_VALUE);   //BINDS(2,szName,DMS_DATA_VALUE-1);  
        local.Execute();
        central.Execute();

	}

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*Ounit = tounit;
	    strncat(DValueC, szName, cDataLen);
		ret= DMS_SUCCESS;
	}
	else if (source->NoData())
		ret=  DMS_NO_DATA;
	else
		ret= DMS_ERROR;
    POP();
#undef SEARCHLC3_SQL
}




LONG searchLevelC4(BOOL fLocal,LONG Trait, LONG Scale, LONG Method, LONG *Ounit, CHAR *DValueC, LONG cDataLen, LONG fopt)
{
#define SearchLC4_SQL "SELECT OINDEX.OUNITID, LEVEL_C.LVALUE FROM OINDEX, FACTOR, LEVEL_C \
        WHERE FACTOR.TRAITID=? AND FACTOR.SCALEID =? AND FACTOR.TMETHID = ? AND LEVEL_C.LABELID = FACTOR.LABELID AND \
		OINDEX.FACTORID = LEVEL_C.FACTORID AND OINDEX.LEVELNO = LEVEL_C.LEVELNO ORDER BY OINDEX.OUNITID"   


	PUSH(SearchLevelC4);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(SearchLC4_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SearchLC4_SQL);
       CODBCbindedStmt *source;

static LONG tounit;
static LONG lngTrait= Trait, lngScale=Scale, lngMeth=Method;
static CHAR szName[DMS_DATA_VALUE-1];
BOOL first_time=TRUE;
    string stmt_sql;

	*DValueC = '\0';
	if (fopt == FIND_FIRST) {
    	lngTrait= Trait;
		lngScale=Scale;
		lngMeth=Method;
        if (first_time) {
			first_time = FALSE;
	        BINDPARAM(1, lngTrait);
	        BINDPARAM(2, lngScale);
	        BINDPARAM(3, lngMeth);
		}
	    BIND(1,tounit);
	    BINDS(2,szName,DMS_DATA_VALUE);   //BINDS(2,szName,DMS_DATA_VALUE-1);  
        local.Execute();
        central.Execute();
	}

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*Ounit = tounit;
	    strncat(DValueC, szName, cDataLen);
		ret= DMS_SUCCESS;
	}
	else if (source->NoData())
		ret=  DMS_NO_DATA;
	else
		ret= DMS_ERROR;
    POP();
#undef SEARCHLC4_SQL
}


//--------------------------------------------------------------------------
//   DMS_extractOunitDC
//      Given:  traitid, scaleid, methodid, data value
//      Output:  ounitid, data value value
//--------------------------------------------------------------------------
LONG extractOunitDC(BOOL fLocal,long studyid,char* chrVal, DMS_CharVALUE *CharValue,int fopt)
{
#define extractODC1_SQL "SELECT DATA_C.OUNITID, DATA_C.DVALUE FROM DATA_C, VARIATE \
        WHERE VARIATE.TRAITID=? AND VARIATE.SCALEID = ? AND VARIATE.TMETHID=? AND VARIATE.VARIATID = DATA_C.VARIATID "
#define extractODC2_SQL "SELECT DATA_C.OUNITID, DATA_C.DVALUE FROM DATA_C, VARIATE \
        WHERE VARIATE.TRAITID=? AND VARIATE.SCALEID = ? AND VARIATE.TMETHID=? AND \
		VARIATE.STUDYID = ? AND VARIATE.VARIATID = DATA_C.VARIATID "

#define EXTRACT_EQ_SQL  " AND RTRIM(DATA_C.DVALUE) = ?  ORDER BY DATA_C.OUNITID  " 
#define EXTRACT_LIKE_SQL " AND RTRIM(DATA_C.DVALUE) LIKE ?  ORDER BY DATA_C.OUNITID  "

	PUSH(extractOunitDC);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(extractODC1_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(extractODC1_SQL);
       CODBCbindedStmt *source;

static DMS_CharVALUE tNValue;
static LONG tstudy;
static char tchr[DMS_DATA_VALUE];
static BOOL cvalue_wild;
    string stmt_sql;
	ZeroFill(tchr,sizeof(tchr));

    tchr[0]='\0';
	strcpy(tchr,chrVal);

	*tNValue.cvalue  = '\0';
	strcat(tNValue.cvalue,CharValue->cvalue);
	tNValue.traitid = CharValue->traitid;
	tNValue.scaleid = CharValue->scaleid;
	tNValue.tmethid = CharValue->tmethid;

	if (fopt == FIND_FIRST) {
       strcpy(tchr,chrVal);
   	   cvalue_wild =strchr(tchr,'_') || strchr(tchr,'%');

       
       if (studyid == 0)
          stmt_sql= extractODC1_SQL;
	   else {
          stmt_sql= extractODC2_SQL;
	   }
 	   stmt_sql+=(cvalue_wild?EXTRACT_LIKE_SQL:EXTRACT_EQ_SQL);
	   central.SetSQLstr(stmt_sql.c_str());
	   local.SetSQLstr(stmt_sql.c_str());	

	   BINDPARAM(1,tNValue.traitid);
       BINDPARAM(2,tNValue.scaleid);
       BINDPARAM(3,tNValue.tmethid);
	   if (studyid == 0 ){
           BINDPARAMS(4,tchr,DMS_DATA_VALUE-1); 
	   }
	   else {
           BINDPARAM(4,tstudy);
           BINDPARAMS(5,tchr,DMS_DATA_VALUE-1); 
	   }

       BIND(1,tNValue.ounitid); 
	   BINDS(2,tNValue.cvalue,DMS_DATA_VALUE -1);
       local.Execute();
       central.Execute();
    }

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*CharValue = tNValue;
		ret= DMS_SUCCESS;
	}
	else if (source->NoData())
		ret= DMS_NO_DATA;

	else
		ret= DMS_ERROR;
    POP();
#undef extractODC1_SQL 
#undef extractODC2_SQL 
#undef EXTRACT_EQ_SQL
#undef EXTRACT_LIKE_SQL

}



//--------------------------------------------------------------------------
//   extractOunitDC1
//      Given:  traitid, scaleid, methodid, data value
//      Output:  ounitid, data value value
//--------------------------------------------------------------------------
LONG extractOunitDC1(BOOL fLocal,long studyid,char* chrVal, DMS_CharVALUE *CharValue,int fopt)
{
#define extractODC1_SQL "SELECT DATA_C.OUNITID, DATA_C.DVALUE FROM DATA_C, VARIATE \
        WHERE VARIATE.TRAITID=? AND VARIATE.SCALEID = ? AND VARIATE.TMETHID=? AND (VARIATE.STUDYID=? OR 0=?) \
		AND VARIATE.VARIATID = DATA_C.VARIATID \
        AND RTRIM(DATA_C.DVALUE) LIKE ?  ORDER BY DATA_C.OUNITID  " 

	PUSH(extractOunitDC);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(extractODC1_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(extractODC1_SQL);
       CODBCbindedStmt *source;

static DMS_CharVALUE tNValue;
static LONG tstudy;
static char tchr[DMS_DATA_VALUE];
static BOOL cvalue_wild;
static BOOL first_time=TRUE;
    string stmt_sql;
	ZeroFill(tchr,sizeof(tchr));

    tchr[0]='\0';
	strcpy(tchr,chrVal);

	*tNValue.cvalue  = '\0';
	strcat(tNValue.cvalue,CharValue->cvalue);
	tNValue.traitid = CharValue->traitid;
	tNValue.scaleid = CharValue->scaleid;
	tNValue.tmethid = CharValue->tmethid;

	if (fopt == FIND_FIRST) {
       strcpy(tchr,chrVal);
	   tstudy = studyid;
	   if (first_time) {
  	     BINDPARAM(1,tNValue.traitid);
         BINDPARAM(2,tNValue.scaleid);
         BINDPARAM(3,tNValue.tmethid);
		 BINDPARAM(4, tstudy);
		 BINDPARAM(5, tstudy);
         BINDPARAMS(6,tchr,DMS_DATA_VALUE-1); 
         BIND(1,tNValue.ounitid); 
  	     BINDS(2,tNValue.cvalue,DMS_DATA_VALUE -1);
       }
       local.Execute();
       central.Execute();
    }

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*CharValue = tNValue;
		ret= DMS_SUCCESS;
	}
	else if (source->NoData())
		ret= DMS_NO_DATA;
	else
		ret= DMS_ERROR;
    POP();
#undef extractODC1_SQL 

}

//--------------------------------------------------------------------------
//   extractOunitDC2
//      Given:  traitid, scaleid, methodid, data value, studyid
//      Output:  ounitid, data value value
//--------------------------------------------------------------------------
LONG extractOunitDC2(BOOL fLocal,long studyid,char* chrVal, DMS_CharVALUE *CharValue,int fopt)
{
#define extractODC2_SQL "SELECT DATA_C.OUNITID, DATA_C.DVALUE FROM DATA_C, VARIATE \
        WHERE VARIATE.TRAITID=? AND VARIATE.SCALEID = ? AND VARIATE.TMETHID=? AND \
		(VARIATE.STUDYID = ? OR  0=?) AND VARIATE.VARIATID = DATA_C.VARIATID \
		AND RTRIM(DATA_C.DVALUE) = ?  ORDER BY DATA_C.OUNITID "


	PUSH(extractOunitDC2);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(extractODC2_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(extractODC2_SQL);
       CODBCbindedStmt *source;

static DMS_CharVALUE tNValue;
static LONG tstudy;
static char tchr[DMS_DATA_VALUE];
static BOOL cvalue_wild;
static BOOL first_time=TRUE;
    string stmt_sql;
	ZeroFill(tchr,sizeof(tchr));

    tchr[0]='\0';
	strcpy(tchr,chrVal);

	*tNValue.cvalue  = '\0';
	strcat(tNValue.cvalue,CharValue->cvalue);
	tNValue.traitid = CharValue->traitid;
	tNValue.scaleid = CharValue->scaleid;
	tNValue.tmethid = CharValue->tmethid;

	if (fopt == FIND_FIRST) {
       strcpy(tchr,chrVal);
	   tstudy = studyid;
	   if (first_time) {
  	     BINDPARAM(1,tNValue.traitid);
         BINDPARAM(2,tNValue.scaleid);
         BINDPARAM(3,tNValue.tmethid);
		 BINDPARAM(4, tstudy);
		 BINDPARAM(5, tstudy);
         BINDPARAMS(6,tchr,DMS_DATA_VALUE-1); 
         BIND(1,tNValue.ounitid); 
  	     BINDS(2,tNValue.cvalue,DMS_DATA_VALUE -1);
       }
       local.Execute();
       central.Execute();
    }

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*CharValue = tNValue;
		ret= DMS_SUCCESS;
	}
	else if (source->NoData())
		ret= DMS_NO_DATA;
	else
		ret= DMS_ERROR;
    POP();
#undef extractODC2_SQL 

}



//--------------------------------------------------------------------------
//   DMS_extractOunitLC
//      Given:  traitid, scaleid, methodid, data value
//      Output:  ounitid, data value value
//--------------------------------------------------------------------------
LONG extractOunitLC(BOOL fLocal,long studyid,char* chrVal, DMS_CharVALUE *CharValue,int fopt)
{
#define extractOLC1_SQL "SELECT OINDEX.OUNITID,LEVEL_C.LVALUE FROM OINDEX, FACTOR, LEVEL_C \
        WHERE FACTOR.TRAITID=? AND FACTOR.SCALEID = ? AND FACTOR.TMETHID=? AND LEVEL_C.LABELID = FACTOR.LABELID  \
		AND  OINDEX.LEVELNO =  LEVEL_C.LEVELNO "
#define extractOLC2_SQL "SELECT OINDEX.OUNITID,LEVEL_C.LVALUE FROM OINDEX, FACTOR, LEVEL_C \
        WHERE FACTOR.TRAITID=? AND FACTOR.SCALEID = ? AND FACTOR.TMETHID=? AND FACTOR.STUDYID = ? AND LEVEL_C.LABELID = FACTOR.LABELID  \
		AND  OINDEX.LEVELNO =  LEVEL_C.LEVELNO "

#define EXTRACT_EQ_SQL  " AND {fn RTRIM(LEVEL_C.LVALUE)} = ?  ORDER BY OINDEX.OUNITID " 
#define EXTRACT_LIKE_SQL " AND {fn RTRIM(LEVEL_C.LVALUE)} LIKE ?  ORDER BY OINDEX.OUNITID "

	PUSH(extractOunitLC);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(extractOLC1_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(extractOLC1_SQL);
       CODBCbindedStmt *source;

static DMS_CharVALUE tNValue;
static LONG tstudy;
static char tchr[DMS_DATA_VALUE];
static BOOL cvalue_wild;
    string stmt_sql;
	ZeroFill(tchr,sizeof(tchr));

    tchr[0]='\0';
	strcpy(tchr,chrVal);

	*tNValue.cvalue  = '\0';
	strcat(tNValue.cvalue,CharValue->cvalue);
	tNValue.traitid = CharValue->traitid;
	tNValue.scaleid = CharValue->scaleid;
	tNValue.tmethid = CharValue->tmethid;

	if (fopt == FIND_FIRST) {
       strcpy(tchr,chrVal);
   	   cvalue_wild =strchr(tchr,'_') || strchr(tchr,'%');

       tstudy = studyid;
       if (studyid == 0)
          stmt_sql= extractOLC1_SQL;
	   else {
          stmt_sql= extractOLC2_SQL;
	   }
 	   stmt_sql+=(cvalue_wild?EXTRACT_LIKE_SQL:EXTRACT_EQ_SQL);
	   central.SetSQLstr(stmt_sql.c_str());
	   local.SetSQLstr(stmt_sql.c_str());	

	   BINDPARAM(1,tNValue.traitid);
       BINDPARAM(2,tNValue.scaleid);
       BINDPARAM(3,tNValue.tmethid);
	   if (studyid == 0 ){
           BINDPARAMS(4,tchr,DMS_DATA_VALUE-1); 
	   }
	   else {
           BINDPARAM(4,tstudy);
           BINDPARAMS(5,tchr,DMS_DATA_VALUE-1); 
	   }

       BIND(1,tNValue.ounitid); 
	   BINDS(2,tNValue.cvalue,DMS_DATA_VALUE);
       local.Execute();
       central.Execute();
    }

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*CharValue = tNValue;
		ret= DMS_SUCCESS;
	}
	else if (source->NoData())
		ret= DMS_NO_DATA;
	else
		ret= DMS_ERROR;
    POP();
#undef extractOLC1_SQL 
#undef extractOLC2_SQL 
#undef EXTRACT_EQ_SQL
#undef EXTRACT_LIKE_SQL
}


//--------------------------------------------------------------------------
//   DMS_extractOunitLC1
//      Given:  traitid, scaleid, methodid, data value, Like operator
//      Output:  ounitid, data value value
//--------------------------------------------------------------------------
LONG extractOunitLC1(BOOL fLocal,long studyid,char* chrVal, DMS_CharVALUE *CharValue,int fopt)
{
#define extractOLC1_SQL "SELECT OINDEX.OUNITID,LEVEL_C.LVALUE FROM OINDEX, FACTOR, LEVEL_C \
        WHERE FACTOR.TRAITID=? AND FACTOR.SCALEID = ? AND FACTOR.TMETHID=? AND (FACTOR.STUDYID = ? OR 0=?)  AND LEVEL_C.LABELID = FACTOR.LABELID  \
		AND  OINDEX.LEVELNO =  LEVEL_C.LEVELNO  AND {fn RTRIM(LEVEL_C.LVALUE)} LIKE ?  ORDER BY OINDEX.OUNITID "

	PUSH(extractOunitLC1);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(extractOLC1_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(extractOLC1_SQL);
       CODBCbindedStmt *source;

static DMS_CharVALUE tNValue;
static LONG tstudy;
static char tchr[DMS_DATA_VALUE];
static BOOL cvalue_wild;
	ZeroFill(tchr,sizeof(tchr));

    tchr[0]='\0';
	strcpy(tchr,chrVal);

	*tNValue.cvalue  = '\0';
	strcat(tNValue.cvalue,CharValue->cvalue);
	tNValue.traitid = CharValue->traitid;
	tNValue.scaleid = CharValue->scaleid;
	tNValue.tmethid = CharValue->tmethid;

	if (fopt == FIND_FIRST) {
       strcpy(tchr,chrVal);
       tstudy = studyid;

	   BINDPARAM(1,tNValue.traitid);
       BINDPARAM(2,tNValue.scaleid);
       BINDPARAM(3,tNValue.tmethid);
       BINDPARAM(4,tstudy);
       BINDPARAM(5,tstudy);
       BINDPARAMS(6,tchr,DMS_DATA_VALUE-1); 

       BIND(1,tNValue.ounitid); 
	   BINDS(2,tNValue.cvalue,DMS_DATA_VALUE);
       local.Execute();
       central.Execute();
    }

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*CharValue = tNValue;
		ret= DMS_SUCCESS;
	}
	else if (source->NoData())
		ret= DMS_NO_DATA;
	else
		ret= DMS_ERROR;
    POP();
#undef extractOLC1_SQL 
}


//--------------------------------------------------------------------------
//   DMS_extractOunitLC2
//      Given:  traitid, scaleid, methodid, data value, = operator
//      Output:  ounitid, data value value
//--------------------------------------------------------------------------
LONG extractOunitLC2(BOOL fLocal,long studyid,char* chrVal, DMS_CharVALUE *CharValue,int fopt)
{
#define extractOLC2_SQL "SELECT OINDEX.OUNITID,LEVEL_C.LVALUE FROM OINDEX, FACTOR, LEVEL_C \
        WHERE FACTOR.TRAITID=? AND FACTOR.SCALEID = ? AND FACTOR.TMETHID=? AND (FACTOR.STUDYID = ? OR 0=?)  AND LEVEL_C.LABELID = FACTOR.LABELID  \
		AND  OINDEX.LEVELNO =  LEVEL_C.LEVELNO  AND {fn RTRIM(LEVEL_C.LVALUE)} = ?  ORDER BY OINDEX.OUNITID "

	PUSH(extractOunitLC2);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(extractOLC2_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(extractOLC2_SQL);
       CODBCbindedStmt *source;

static DMS_CharVALUE tNValue;
static LONG tstudy;
static char tchr[DMS_DATA_VALUE];
static BOOL cvalue_wild;
	ZeroFill(tchr,sizeof(tchr));

    tchr[0]='\0';
	strcpy(tchr,chrVal);

	*tNValue.cvalue  = '\0';
	strcat(tNValue.cvalue,CharValue->cvalue);
	tNValue.traitid = CharValue->traitid;
	tNValue.scaleid = CharValue->scaleid;
	tNValue.tmethid = CharValue->tmethid;

	if (fopt == FIND_FIRST) {
       strcpy(tchr,chrVal);
       tstudy = studyid;

	   BINDPARAM(1,tNValue.traitid);
       BINDPARAM(2,tNValue.scaleid);
       BINDPARAM(3,tNValue.tmethid);
       BINDPARAM(4,tstudy);
       BINDPARAM(5,tstudy);
       BINDPARAMS(6,tchr,DMS_DATA_VALUE-1); 

       BIND(1,tNValue.ounitid); 
	   BINDS(2,tNValue.cvalue,DMS_DATA_VALUE);
       local.Execute();
       central.Execute();
    }

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*CharValue = tNValue;
		ret= DMS_SUCCESS;
	}
	else if (source->NoData())
		ret= DMS_NO_DATA;
	else
		ret= DMS_ERROR;
    POP();
#undef extractOLC2_SQL 
}



//--------------------------------------------------------------------------
//   DMS_extractOunitDN
//      Given:  traitid, scaleid, methodid, data value, operator 
//      Output:  ounitid, data value value
//--------------------------------------------------------------------------
LONG extractOunitDN(BOOL fLocal,long studyid,double dblValue,DMS_NumVALUE *NumValue, int  oprtr, int fopt)
{
#define extractODN1_SQL "SELECT DATA_N.OUNITID, DATA_N.DVALUE FROM DATA_N, VARIATE  \
	        WHERE VARIATE.TRAITID=? AND VARIATE.SCALEID = ? AND VARIATE.TMETHID=? AND \
	        VARIATE.VARIATID = DATA_N.VARIATID AND DATA_N.DVALUE  "
#define extractODN2_SQL "SELECT DATA_N.OUNITID, DATA_N.DVALUE FROM DATA_N, VARIATE  \
	        WHERE VARIATE.TRAITID=? AND VARIATE.SCALEID = ? AND VARIATE.TMETHID=? AND \
	        VARIATE.STUDYID = ? AND VARIATE.VARIATID = DATA_N.VARIATID AND DATA_N.DVALUE  "

#define EXTRACT_EQ_SQL  " = ? ORDER BY DATA_N.OUNITID "
#define EXTRACT_LT_SQL  " < ? ORDER BY DATA_N.OUNITID "
#define EXTRACT_GT_SQL  " > ? ORDER BY DATA_N.OUNITID "
#define EXTRACT_GTE_SQL " >= ? ORDER BY DATA_N.OUNITID "
#define EXTRACT_LTE_SQL " <= ? ORDER BY DATA_N.OUNITID "


	PUSH(extractOunitDN);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(extractODN1_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(extractODN1_SQL);
       CODBCbindedStmt *source;

static DMS_NumVALUE tNValue;
static LONG tstudy;
    string stmt_sql;

	tNValue.nvalue  = NumValue->nvalue;
	tNValue.traitid = NumValue->traitid;
	tNValue.scaleid = NumValue->scaleid;
	tNValue.tmethid = NumValue->tmethid;

	if (fopt == FIND_FIRST) {

       
       if (studyid == 0)
          stmt_sql= extractODN1_SQL;
	   else {
          stmt_sql= extractODN2_SQL;
	   }
       if (oprtr == DMS_EQ ) stmt_sql+=EXTRACT_EQ_SQL;
  	   if (oprtr == DMS_LT ) stmt_sql+=EXTRACT_LT_SQL;
	   if (oprtr == DMS_GT ) stmt_sql+=EXTRACT_GT_SQL;
       if (oprtr == DMS_GTE) stmt_sql+=EXTRACT_GTE_SQL;
	   if (oprtr == DMS_LTE) stmt_sql+=EXTRACT_GTE_SQL;

	   central.SetSQLstr(stmt_sql.c_str());
	   local.SetSQLstr(stmt_sql.c_str());	

	   BINDPARAM(1,tNValue.traitid);
       BINDPARAM(2,tNValue.scaleid);
       BINDPARAM(3,tNValue.tmethid);
	   if (studyid == 0 ){
           BINDPARAM(4,dblValue); 
	   }
	   else {
           BINDPARAM(4,tstudy);
           BINDPARAM(5,dblValue); 
	   }

       BIND(1,tNValue.ounitid); 
	   BIND(2,tNValue.nvalue);
       local.Execute();
       central.Execute();
    }

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*NumValue = tNValue;
		ret = DMS_SUCCESS;
	}
	else if (source->NoData())
		ret = DMS_NO_DATA;
	else
		ret = DMS_ERROR;
    POP();

#undef extractODN1_SQL 
#undef extractODN2_SQL 
#undef EXTRACT_EQ_SQL  
#undef EXTRACT_LT_SQL  
#undef EXTRACT_GT_SQL  
#undef EXTRACT_GTE_SQL 
#undef EXTRACT_LTE_SQL 

}


//--------------------------------------------------------------------------
//   DMS_extractOunitDNEq
//      Given:  traitid, scaleid, methodid, data value, operator (= )
//      Output:  ounitid, data value value
//--------------------------------------------------------------------------
LONG extractOunitDN_EQ(BOOL fLocal,long studyid,double dblValue,DMS_NumVALUE *NumValue, int  oprtr, int fopt)
{
#define extractODNEq_SQL "SELECT DATA_N.OUNITID, DATA_N.DVALUE FROM DATA_N, VARIATE  \
	        WHERE VARIATE.TRAITID=? AND VARIATE.SCALEID = ? AND VARIATE.TMETHID=? AND \
	        (VARIATE.STUDYID = ? OR 0=?) AND VARIATE.VARIATID = DATA_N.VARIATID AND \
			DATA_N.DVALUE  = ? ORDER BY DATA_N.OUNITID "


	PUSH(extractOunitDNEq);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(extractODNEq_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(extractODNEq_SQL);

       CODBCbindedStmt *source;

static DMS_NumVALUE tNValue;
static DOUBLE _dblValue;
static LONG tstudy;
static BOOL first_time=TRUE;
    string stmt_sql;

	tNValue.nvalue  = NumValue->nvalue;
	tNValue.traitid = NumValue->traitid;
	tNValue.scaleid = NumValue->scaleid;
	tNValue.tmethid = NumValue->tmethid;

	if (fopt == FIND_FIRST) {

       
		if (first_time) {
	       BINDPARAM(1,tNValue.traitid);
           BINDPARAM(2,tNValue.scaleid);
           BINDPARAM(3,tNValue.tmethid);
           BINDPARAM(4,tstudy);
           BINDPARAM(5,tstudy);
           BINDPARAM(6,_dblValue); 
           BIND(1,tNValue.ounitid); 
	       BIND(2,tNValue.nvalue);
	   }
       _dblValue = dblValue;
       tstudy = studyid; 
       local.Execute();
       central.Execute();
    }

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*NumValue = tNValue;
		ret = DMS_SUCCESS;
	}
	else if (source->NoData())
		ret = DMS_NO_DATA;
	else
		ret = DMS_ERROR;
    POP();

#undef extractODNEq_SQL 

}



//--------------------------------------------------------------------------
//   DMS_extractOunitDN_LT
//      Given:  traitid, scaleid, methodid, data value, operator (<= )
//      Output:  ounitid, data value value
//--------------------------------------------------------------------------
LONG extractOunitDN_LT(BOOL fLocal,long studyid,double dblValue,DMS_NumVALUE *NumValue, int  oprtr, int fopt)
{
#define extractODNLT_SQL "SELECT DATA_N.OUNITID, DATA_N.DVALUE FROM DATA_N, VARIATE  \
	        WHERE VARIATE.TRAITID=? AND VARIATE.SCALEID = ? AND VARIATE.TMETHID=? AND \
	        (VARIATE.STUDYID = ? OR 0=?) AND VARIATE.VARIATID = DATA_N.VARIATID AND \
			DATA_N.DVALUE  < ? ORDER BY DATA_N.OUNITID "


	PUSH(extractOunitDNLT);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(extractODNLT_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(extractODNLT_SQL);

       CODBCbindedStmt *source;

static DMS_NumVALUE tNValue;
static DOUBLE _dblValue;
static LONG tstudy;
static BOOL first_time=TRUE;
    string stmt_sql;

	tNValue.nvalue  = NumValue->nvalue;
	tNValue.traitid = NumValue->traitid;
	tNValue.scaleid = NumValue->scaleid;
	tNValue.tmethid = NumValue->tmethid;

	if (fopt == FIND_FIRST) {

       
		if (first_time) {
	       BINDPARAM(1,tNValue.traitid);
           BINDPARAM(2,tNValue.scaleid);
           BINDPARAM(3,tNValue.tmethid);
           BINDPARAM(4,tstudy);
           BINDPARAM(5,tstudy);
           BINDPARAM(6,_dblValue); 
           BIND(1,tNValue.ounitid); 
	       BIND(2,tNValue.nvalue);
	   }
       _dblValue = dblValue;  
       tstudy = studyid; 
       local.Execute();
       central.Execute();
    }

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*NumValue = tNValue;
		ret = DMS_SUCCESS;
	}
	else if (source->NoData())
		ret = DMS_NO_DATA;
	else
		ret = DMS_ERROR;
    POP();

#undef extractODNLT_SQL 

}


//--------------------------------------------------------------------------
//   DMS_extractOunitDN_GT
//      Given:  traitid, scaleid, methodid, data value, operator (<= )
//      Output:  ounitid, data value value
//--------------------------------------------------------------------------
LONG extractOunitDN_GT(BOOL fLocal,long studyid,double dblValue,DMS_NumVALUE *NumValue, int  oprtr, int fopt)
{
#define extractODNGT_SQL "SELECT DATA_N.OUNITID, DATA_N.DVALUE FROM DATA_N, VARIATE  \
	        WHERE VARIATE.TRAITID=? AND VARIATE.SCALEID = ? AND VARIATE.TMETHID=? AND \
	        (VARIATE.STUDYID = ? OR 0=?) AND VARIATE.VARIATID = DATA_N.VARIATID AND \
			DATA_N.DVALUE  > ? ORDER BY DATA_N.OUNITID "


	PUSH(extractOunitDNGT);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(extractODNGT_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(extractODNGT_SQL);

       CODBCbindedStmt *source;

static DMS_NumVALUE tNValue;
static DOUBLE _dblValue;
static LONG tstudy;
static BOOL first_time=TRUE;
    string stmt_sql;

	tNValue.nvalue  = NumValue->nvalue;
	tNValue.traitid = NumValue->traitid;
	tNValue.scaleid = NumValue->scaleid;
	tNValue.tmethid = NumValue->tmethid;

	if (fopt == FIND_FIRST) {

       
		if (first_time) {
	       BINDPARAM(1,tNValue.traitid);
           BINDPARAM(2,tNValue.scaleid);
           BINDPARAM(3,tNValue.tmethid);
           BINDPARAM(4,tstudy);
           BINDPARAM(5,tstudy);
           BINDPARAM(6,_dblValue); 
           BIND(1,tNValue.ounitid); 
	       BIND(2,tNValue.nvalue);
	   }
       _dblValue = dblValue;
       tstudy = studyid; 
       local.Execute();
       central.Execute();
    }

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*NumValue = tNValue;
		ret = DMS_SUCCESS;
	}
	else if (source->NoData())
		ret = DMS_NO_DATA;
	else
		ret = DMS_ERROR;
    POP();

#undef extractODNGT_SQL 

}


//--------------------------------------------------------------------------
//   DMS_extractOunitDN_GE
//      Given:  traitid, scaleid, methodid, data value, operator (<= )
//      Output:  ounitid, data value value
//--------------------------------------------------------------------------
LONG extractOunitDN_GTE(BOOL fLocal,long studyid,double dblValue,DMS_NumVALUE *NumValue, int  oprtr, int fopt)
{
#define extractODNGE_SQL "SELECT DATA_N.OUNITID, DATA_N.DVALUE FROM DATA_N, VARIATE  \
	        WHERE VARIATE.TRAITID=? AND VARIATE.SCALEID = ? AND VARIATE.TMETHID=? AND \
	        (VARIATE.STUDYID = ? OR 0=?) AND VARIATE.VARIATID = DATA_N.VARIATID AND \
			DATA_N.DVALUE  >= ? ORDER BY DATA_N.OUNITID "


	PUSH(extractOunitDNGE);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(extractODNGE_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(extractODNGE_SQL);

       CODBCbindedStmt *source;

static DMS_NumVALUE tNValue;
static LONG tstudy;
static DOUBLE _dblValue;
static BOOL first_time=TRUE;
    string stmt_sql;

	tNValue.nvalue  = NumValue->nvalue;
	tNValue.traitid = NumValue->traitid;
	tNValue.scaleid = NumValue->scaleid;
	tNValue.tmethid = NumValue->tmethid;

	if (fopt == FIND_FIRST) {

       
		if (first_time) {
	       BINDPARAM(1,tNValue.traitid);
           BINDPARAM(2,tNValue.scaleid);
           BINDPARAM(3,tNValue.tmethid);
           BINDPARAM(4,tstudy);
           BINDPARAM(5,tstudy);
           BINDPARAM(6,_dblValue); 
           BIND(1,tNValue.ounitid); 
	       BIND(2,tNValue.nvalue);
	   }
       tstudy = studyid;
	   _dblValue = dblValue;
       local.Execute();
       central.Execute();
    }

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*NumValue = tNValue;
		ret = DMS_SUCCESS;
	}
	else if (source->NoData())
		ret = DMS_NO_DATA;
	else
		ret = DMS_ERROR;
    POP();

#undef extractODNGE_SQL 

}


//--------------------------------------------------------------------------
//   DMS_extractOunitDN_LE
//      Given:  traitid, scaleid, methodid, data value, operator (<= )
//      Output:  ounitid, data value value
//--------------------------------------------------------------------------
LONG extractOunitDN_LTE(BOOL fLocal,long studyid,double dblValue,DMS_NumVALUE *NumValue, int  oprtr, int fopt)
{
#define extractODNLE_SQL "SELECT DATA_N.OUNITID, DATA_N.DVALUE FROM DATA_N, VARIATE  \
	        WHERE VARIATE.TRAITID=? AND VARIATE.SCALEID = ? AND VARIATE.TMETHID=? AND \
	        (VARIATE.STUDYID = ? OR 0=?) AND VARIATE.VARIATID = DATA_N.VARIATID AND \
			DATA_N.DVALUE  <= ? ORDER BY DATA_N.OUNITID "


	PUSH(extractOunitDNLE);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(extractODNLE_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(extractODNLE_SQL);

       CODBCbindedStmt *source;

static DMS_NumVALUE tNValue;
static DOUBLE _dblValue;
static LONG tstudy;
static BOOL first_time=TRUE;
    string stmt_sql;

	tNValue.nvalue  = NumValue->nvalue;
	tNValue.traitid = NumValue->traitid;
	tNValue.scaleid = NumValue->scaleid;
	tNValue.tmethid = NumValue->tmethid;

	if (fopt == FIND_FIRST) {

       
		if (first_time) {
	       BINDPARAM(1,tNValue.traitid);
           BINDPARAM(2,tNValue.scaleid);
           BINDPARAM(3,tNValue.tmethid);
           BINDPARAM(4,tstudy);
           BINDPARAM(5,tstudy);
           BINDPARAM(6,_dblValue); 
           BIND(1,tNValue.ounitid); 
	       BIND(2,tNValue.nvalue);
	   }
       _dblValue = dblValue;
       tstudy = studyid; 
       local.Execute();
       central.Execute();
    }

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*NumValue = tNValue;
		ret = DMS_SUCCESS;
	}
	else if (source->NoData())
		ret = DMS_NO_DATA;
	else
		ret = DMS_ERROR;
    POP();

#undef extractODNLE_SQL 

}



//--------------------------------------------------------------------------
//   DMS_extractOunitLN
//      Given:  traitid, scaleid, methodid, data value, operator 
//      Output:  ounitid, data value value
//--------------------------------------------------------------------------
LONG extractOunitLN(BOOL fLocal,long studyid,double dblValue,DMS_NumVALUE *NumValue, int  oprtr, int fopt)
{
#define extractOLN1_SQL "SELECT OINDEX.OUNITID,  LEVEL_N.LVALUE  FROM OINDEX, FACTOR,  LEVEL_N \
	       WHERE FACTOR.TRAITID=? AND FACTOR.SCALEID = ? AND FACTOR.TMETHID=? AND  LEVEL_N.LABELID = FACTOR.LABELID \
	       AND OINDEX.FACTORID =  LEVEL_N.FACTORID AND OINDEX.LEVELNO =  LEVEL_N.LEVELNO AND LEVEL_N.LVALUE  "
#define extractOLN2_SQL "SELECT OINDEX.OUNITID,  LEVEL_N.LVALUE FROM OINDEX, FACTOR,  LEVEL_N \
	       WHERE FACTOR.TRAITID=? AND FACTOR.SCALEID = ? AND FACTOR.TMETHID=? AND FACTOR.STUDYID = ? AND  LEVEL_N.LABELID = FACTOR.LABELID \
	       AND OINDEX.FACTORID =  LEVEL_N.FACTORID AND OINDEX.LEVELNO =  LEVEL_N.LEVELNO AND  LEVEL_N.LVALUE   "

#define EXTRACT_EQ_SQL  " = ?  ORDER BY OINDEX.OUNITID "
#define EXTRACT_LT_SQL  " < ?  ORDER BY OINDEX.OUNITID "
#define EXTRACT_GT_SQL  " > ?  ORDER BY OINDEX.OUNITID "
#define EXTRACT_GTE_SQL " >= ?  ORDER BY OINDEX.OUNITID "
#define EXTRACT_LTE_SQL " <= ?  ORDER BY OINDEX.OUNITID "


	PUSH(extractOunitLN);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(extractOLN1_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(extractOLN1_SQL);
       CODBCbindedStmt *source;

static DMS_NumVALUE tNValue;
static DOUBLE _dblValue;
static LONG tstudy;
    string stmt_sql;

	tNValue.nvalue  = NumValue->nvalue;
	tNValue.traitid = NumValue->traitid;
	tNValue.scaleid = NumValue->scaleid;
	tNValue.tmethid = NumValue->tmethid;

	if (fopt == FIND_FIRST) {
       tstudy = studyid;
       _dblValue = dblValue;
       if (studyid == 0)
          stmt_sql= extractOLN1_SQL;
	   else {
          stmt_sql= extractOLN2_SQL;
	   }
       if (oprtr == DMS_EQ ) stmt_sql+=EXTRACT_EQ_SQL;
  	   if (oprtr == DMS_LT ) stmt_sql+=EXTRACT_LT_SQL;
	   if (oprtr == DMS_GT ) stmt_sql+=EXTRACT_GT_SQL;
       if (oprtr == DMS_GTE) stmt_sql+=EXTRACT_GTE_SQL;
	   if (oprtr == DMS_LTE) stmt_sql+=EXTRACT_GTE_SQL;

	   central.SetSQLstr(stmt_sql.c_str());
	   local.SetSQLstr(stmt_sql.c_str());	

	   BINDPARAM(1,tNValue.traitid);
       BINDPARAM(2,tNValue.scaleid);
       BINDPARAM(3,tNValue.tmethid);
	   if (studyid == 0 ){
           BINDPARAM(4,_dblValue); 
	   }
	   else {
           BINDPARAM(4,tstudy);
           BINDPARAM(5,_dblValue); 
	   }

       BIND(1,tNValue.ounitid); 
	   BIND(2,tNValue.nvalue);
       local.Execute();
       central.Execute();
    }

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*NumValue = tNValue;
		ret = DMS_SUCCESS;
	}
	else if (source->NoData())
		ret = DMS_NO_DATA;
	else
		ret = DMS_ERROR;
    POP();

#undef extractOLN1_SQL
#undef extractOLN2_SQL 
#undef EXTRACT_EQ_SQL  
#undef EXTRACT_LT_SQL  
#undef EXTRACT_GT_SQL  
#undef EXTRACT_GTE_SQL 
#undef EXTRACT_LTE_SQL 

}


//--------------------------------------------------------------------------
//   DMS_extractOunitLN
//      Given:  traitid, scaleid, methodid, data value, operator 
//      Output:  ounitid, data value value
//--------------------------------------------------------------------------
LONG extractOunitLN_EQ(BOOL fLocal,long studyid,double dblValue,DMS_NumVALUE *NumValue, int  oprtr, int fopt)
{
#define extractOLN1_SQL "SELECT OINDEX.OUNITID,  LEVEL_N.LVALUE  FROM OINDEX, FACTOR,  LEVEL_N \
	       WHERE FACTOR.TRAITID=? AND FACTOR.SCALEID = ? AND FACTOR.TMETHID=? AND (FACTOR.STUDYID = ? OR 0=? )AND  LEVEL_N.LABELID = FACTOR.LABELID \
	       AND OINDEX.FACTORID =  LEVEL_N.FACTORID AND OINDEX.LEVELNO =  LEVEL_N.LEVELNO \
		   AND LEVEL_N.LVALUE = ?  ORDER BY OINDEX.OUNITID "

	PUSH(extractOunitLN);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(extractOLN1_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(extractOLN1_SQL);
       CODBCbindedStmt *source;

static DMS_NumVALUE tNValue;
static LONG tstudy;
static double dblVal_;
static first_time=TRUE;
    string stmt_sql;

	tNValue.nvalue  = NumValue->nvalue;
	tNValue.traitid = NumValue->traitid;
	tNValue.scaleid = NumValue->scaleid;
	tNValue.tmethid = NumValue->tmethid;

	if (fopt == FIND_FIRST) {
       tstudy = studyid;
	   dblVal_ = dblValue;
       if (first_time) {
	     BINDPARAM(1,tNValue.traitid);
         BINDPARAM(2,tNValue.scaleid);
         BINDPARAM(3,tNValue.tmethid);
         BINDPARAM(4,tstudy);
		 BINDPARAM(5, tstudy); 
         BINDPARAM(6,dblVal_); 
		 first_time = FALSE;
       }
       
       BIND(1,tNValue.ounitid); 
	   BIND(2,tNValue.nvalue);
       local.Execute();
       central.Execute();
    }

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*NumValue = tNValue;
		ret = DMS_SUCCESS;
	}
	else if (source->NoData())
		ret = DMS_NO_DATA;
	else
		ret = DMS_ERROR;
    POP();

#undef extractOLN1_SQL

}




//--------------------------------------------------------------------------
//   DMS_extractOunitLN_GT
//      Given:  traitid, scaleid, methodid, data value, operator >
//      Output:  ounitid, data value value
//--------------------------------------------------------------------------
LONG extractOunitLN_GT(BOOL fLocal,long studyid,double dblValue,DMS_NumVALUE *NumValue, int  oprtr, int fopt)
{
#define extractOLNGT_SQL "SELECT OINDEX.OUNITID,  LEVEL_N.LVALUE  FROM OINDEX, FACTOR,  LEVEL_N \
	       WHERE FACTOR.TRAITID=? AND FACTOR.SCALEID = ? AND FACTOR.TMETHID=? AND (FACTOR.STUDYID = ? OR 0=? )AND  LEVEL_N.LABELID = FACTOR.LABELID \
	       AND OINDEX.FACTORID =  LEVEL_N.FACTORID AND OINDEX.LEVELNO =  LEVEL_N.LEVELNO \
		   AND LEVEL_N.LVALUE > ?  ORDER BY OINDEX.OUNITID "

	PUSH(extractOunitLN_GT);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(extractOLNGT_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(extractOLNGT_SQL);
       CODBCbindedStmt *source;

static DMS_NumVALUE tNValue;
static LONG tstudy;
static first_time=TRUE;
static double _dblValue;
    string stmt_sql;

	tNValue.nvalue  = NumValue->nvalue;
	tNValue.traitid = NumValue->traitid;
	tNValue.scaleid = NumValue->scaleid;
	tNValue.tmethid = NumValue->tmethid;

	if (fopt == FIND_FIRST) {
       tstudy = studyid;
	   _dblValue = dblValue;
       if (first_time) {
	     BINDPARAM(1,tNValue.traitid);
         BINDPARAM(2,tNValue.scaleid);
         BINDPARAM(3,tNValue.tmethid);
         BINDPARAM(4,tstudy);
		 BINDPARAM(5, tstudy); 
         BINDPARAM(6,_dblValue); 
		 first_time = FALSE;
       }
       
       BIND(1,tNValue.ounitid); 
	   BIND(2,tNValue.nvalue);
       local.Execute();
       central.Execute();
    }

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*NumValue = tNValue;
		ret = DMS_SUCCESS;
	}
	else if (source->NoData())
		ret = DMS_NO_DATA;
	else
		ret = DMS_ERROR;
    POP();

#undef extractOLNGT_SQL
}



//--------------------------------------------------------------------------
//   DMS_extractOunitLN_LT
//      Given:  traitid, scaleid, methodid, data value, operator >
//      Output:  ounitid, data value value
//--------------------------------------------------------------------------
LONG extractOunitLN_LT(BOOL fLocal,long studyid,double dblValue,DMS_NumVALUE *NumValue, int  oprtr, int fopt)
{
#define extractOLNLT_SQL "SELECT OINDEX.OUNITID,  LEVEL_N.LVALUE  FROM OINDEX, FACTOR,  LEVEL_N \
	       WHERE FACTOR.TRAITID=? AND FACTOR.SCALEID = ? AND FACTOR.TMETHID=? AND (FACTOR.STUDYID = ? OR 0=? )AND  LEVEL_N.LABELID = FACTOR.LABELID \
	       AND OINDEX.FACTORID =  LEVEL_N.FACTORID AND OINDEX.LEVELNO =  LEVEL_N.LEVELNO \
		   AND LEVEL_N.LVALUE < ?  ORDER BY OINDEX.OUNITID "

	PUSH(extractOunitLN_LT);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(extractOLNLT_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(extractOLNLT_SQL);
       CODBCbindedStmt *source;

static DMS_NumVALUE tNValue;
static LONG tstudy;
static first_time=TRUE;
static double _dblValue;
    string stmt_sql;

	tNValue.nvalue  = NumValue->nvalue;
	tNValue.traitid = NumValue->traitid;
	tNValue.scaleid = NumValue->scaleid;
	tNValue.tmethid = NumValue->tmethid;

	if (fopt == FIND_FIRST) {
       tstudy = studyid;
	   _dblValue = dblValue;
       if (first_time) {
	     BINDPARAM(1,tNValue.traitid);
         BINDPARAM(2,tNValue.scaleid);
         BINDPARAM(3,tNValue.tmethid);
         BINDPARAM(4,tstudy);
		 BINDPARAM(5, tstudy); 
         BINDPARAM(6,_dblValue); 
		 first_time = FALSE;
       }
       
       BIND(1,tNValue.ounitid); 
	   BIND(2,tNValue.nvalue);
       local.Execute();
       central.Execute();
    }

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*NumValue = tNValue;
		ret = DMS_SUCCESS;
	}
	else if (source->NoData())
		ret = DMS_NO_DATA;
	else
		ret = DMS_ERROR;
    POP();

#undef extractOLNLT_SQL
}


//--------------------------------------------------------------------------
//   DMS_extractOunitLN_LTE
//      Given:  traitid, scaleid, methodid, data value, operator >
//      Output:  ounitid, data value value
//--------------------------------------------------------------------------
LONG extractOunitLN_LTE(BOOL fLocal,long studyid,double dblValue,DMS_NumVALUE *NumValue, int  oprtr, int fopt)
{
#define extractOLNLTE_SQL "SELECT OINDEX.OUNITID,  LEVEL_N.LVALUE  FROM OINDEX, FACTOR,  LEVEL_N \
	       WHERE FACTOR.TRAITID=? AND FACTOR.SCALEID = ? AND FACTOR.TMETHID=? AND (FACTOR.STUDYID = ? OR 0=? )AND  LEVEL_N.LABELID = FACTOR.LABELID \
	       AND OINDEX.FACTORID =  LEVEL_N.FACTORID AND OINDEX.LEVELNO =  LEVEL_N.LEVELNO \
		   AND LEVEL_N.LVALUE <= ?  ORDER BY OINDEX.OUNITID "

	PUSH(extractOunitLN_LTE);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(extractOLNLTE_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(extractOLNLTE_SQL);
       CODBCbindedStmt *source;

static DMS_NumVALUE tNValue;
static LONG tstudy;
static first_time=TRUE;
static double _dblValue;
    string stmt_sql;


	tNValue.nvalue  = NumValue->nvalue;
	tNValue.traitid = NumValue->traitid;
	tNValue.scaleid = NumValue->scaleid;
	tNValue.tmethid = NumValue->tmethid;

	if (fopt == FIND_FIRST) {
       tstudy = studyid;
	   _dblValue = dblValue;
       if (first_time) {
	     BINDPARAM(1,tNValue.traitid);
         BINDPARAM(2,tNValue.scaleid);
         BINDPARAM(3,tNValue.tmethid);
         BINDPARAM(4,tstudy);
		 BINDPARAM(5, tstudy); 
         BINDPARAM(6,_dblValue); 
		 first_time = FALSE;
       }
       
       BIND(1,tNValue.ounitid); 
	   BIND(2,tNValue.nvalue);
       local.Execute();
       central.Execute();
    }

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*NumValue = tNValue;
		ret = DMS_SUCCESS;
	}
	else if (source->NoData())
		ret = DMS_NO_DATA;
	else
		ret = DMS_ERROR;
    POP();

#undef extractOLNLTE_SQL
}



//--------------------------------------------------------------------------
//   DMS_extractOunitLN_GTE
//      Given:  traitid, scaleid, methodid, data value, operator >
//      Output:  ounitid, data value value
//--------------------------------------------------------------------------
LONG extractOunitLN_GTE(BOOL fLocal,long studyid,double dblValue,DMS_NumVALUE *NumValue, int  oprtr, int fopt)
{
#define extractOLNGTE_SQL "SELECT OINDEX.OUNITID,  LEVEL_N.LVALUE  FROM OINDEX, FACTOR,  LEVEL_N \
	       WHERE FACTOR.TRAITID=? AND FACTOR.SCALEID = ? AND FACTOR.TMETHID=? AND (FACTOR.STUDYID = ? OR 0=? )AND  LEVEL_N.LABELID = FACTOR.LABELID \
	       AND OINDEX.FACTORID =  LEVEL_N.FACTORID AND OINDEX.LEVELNO =  LEVEL_N.LEVELNO \
		   AND LEVEL_N.LVALUE >= ?  ORDER BY OINDEX.OUNITID "

	PUSH(extractOunitLN_GTE);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(extractOLNGTE_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(extractOLNGTE_SQL);
       CODBCbindedStmt *source;

static DMS_NumVALUE tNValue;
static LONG tstudy;
static first_time=TRUE;
static double _dblValue;
     string stmt_sql;

	tNValue.nvalue  = NumValue->nvalue;
	tNValue.traitid = NumValue->traitid;
	tNValue.scaleid = NumValue->scaleid;
	tNValue.tmethid = NumValue->tmethid;

	if (fopt == FIND_FIRST) {
       tstudy = studyid;
	   _dblValue = dblValue;
       if (first_time) {
	     BINDPARAM(1,tNValue.traitid);
         BINDPARAM(2,tNValue.scaleid);
         BINDPARAM(3,tNValue.tmethid);
         BINDPARAM(4,tstudy);
		 BINDPARAM(5, tstudy); 
         BINDPARAM(6,_dblValue); 
		 first_time = FALSE;
       }
       
       BIND(1,tNValue.ounitid); 
	   BIND(2,tNValue.nvalue);
       local.Execute();
       central.Execute();
    }

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*NumValue = tNValue;
		ret = DMS_SUCCESS;
	}
	else if (source->NoData())
		ret = DMS_NO_DATA;
	else
		ret = DMS_ERROR;
    POP();

#undef extractOLNGTE_SQL
}




//--------------------------------------------------------------------------
//   DMS_searchDataNOfOunit
//      Given:  ounitid, traitid, scaleid, methodid, 
//      Output: data value value
//      Date Created:  Dec 20, 2005
//--------------------------------------------------------------------------
LONG searchDataNOfOunit(BOOL fLocal, DMS_NumVALUE *NumValue)
{
#define getDataNOfOunit_SQL "SELECT DATA_N.DVALUE FROM DATA_N, VARIATE  \
	        WHERE VARIATE.TRAITID=? AND VARIATE.SCALEID = ? AND VARIATE.TMETHID=? AND \
	        VARIATE.VARIATID = DATA_N.VARIATID AND DATA_N.OUNITID = ? "


	PUSH(getDataNOfOunit);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(getDataNOfOunit_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(getDataNOfOunit_SQL);

       CODBCbindedStmt *source;

static DMS_NumVALUE tNValue;
static DOUBLE _dblValue;
static LONG tstudy;
static BOOL first_time=TRUE;
    string stmt_sql;

	tNValue.ounitid  = NumValue->ounitid;
	tNValue.traitid = NumValue->traitid;
	tNValue.scaleid = NumValue->scaleid;
	tNValue.tmethid = NumValue->tmethid;

	if (first_time) {
       BINDPARAM(1,tNValue.traitid);
       BINDPARAM(2,tNValue.scaleid);
       BINDPARAM(3,tNValue.tmethid);
       BINDPARAM(4,tNValue.ounitid); 
       BIND(1,tNValue.nvalue);
     }
     local.Execute();
     central.Execute();

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*NumValue = tNValue;
		ret = DMS_SUCCESS;
	}
	else if (source->NoData())
		ret = DMS_NO_DATA;
	else
		ret = DMS_ERROR;
    POP();

#undef getDataNOfOunit_SQL

}

//--------------------------------------------------------------------------
//   DMS_searchLevelNOfOunit
//      Given:  ounitid, traitid, scaleid, methodid, 
//      Output: level  value from LEVEL_N table
//      Date Created:  Dec 20, 2005
//--------------------------------------------------------------------------
LONG searchLevelNOfOunit(BOOL fLocal, DMS_NumVALUE *NumValue)
{
#define getLevelNOfOunit_SQL "SELECT LEVEL_N.LVALUE \
				FROM LEVEL_N , FACTOR, OINDEX  \
				WHERE LEVEL_N.LABELID = FACTOR.LABELID AND LEVEL_N.LEVELNO = OINDEX.LEVELNO \
				 AND FACTOR.TRAITID=? AND FACTOR.SCALEID=? AND FACTOR.TMETHID=? AND OINDEX.OUNITID=? "


	PUSH(getLevelNOfOunit);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(getLevelNOfOunit_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(getLevelNOfOunit_SQL);

       CODBCbindedStmt *source;

static DMS_NumVALUE tNValue;
static DOUBLE _dblValue;
static LONG tstudy;
static BOOL first_time=TRUE;
    string stmt_sql;

	tNValue.ounitid  = NumValue->ounitid;
	tNValue.traitid = NumValue->traitid;
	tNValue.scaleid = NumValue->scaleid;
	tNValue.tmethid = NumValue->tmethid;

	if (first_time) {
       BINDPARAM(1,tNValue.traitid);
       BINDPARAM(2,tNValue.scaleid);
       BINDPARAM(3,tNValue.tmethid);
       BINDPARAM(4,tNValue.ounitid); 
       BIND(1,tNValue.nvalue);
     }
     local.Execute();
     central.Execute();

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*NumValue = tNValue;
		ret = DMS_SUCCESS;
	}
	else if (source->NoData())
		ret = DMS_NO_DATA;
	else
		ret = DMS_ERROR;
    POP();

#undef getLevelNOfOunit_SQL

}

//--------------------------------------------------------------------------
//   searchDataCOfOunit
//      Given:  ounitid, traitid, scaleid, methodid, 
//      Output: data value value
//      Date Created:  Dec 20, 2005
//--------------------------------------------------------------------------
LONG searchDataCOfOunit(BOOL fLocal, DMS_CharVALUE *CharValue)
{
#define getDataCOfOunit_SQL "SELECT DATA_C.DVALUE FROM DATA_C, VARIATE  \
	        WHERE VARIATE.TRAITID=? AND VARIATE.SCALEID = ? AND VARIATE.TMETHID=? AND \
	        VARIATE.VARIATID = DATA_C.VARIATID AND DATA_C.OUNITID = ? "


	PUSH(getDataCOfOunit);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(getDataCOfOunit_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(getDataCOfOunit_SQL);

       CODBCbindedStmt *source;

static DMS_CharVALUE tValue;
static DOUBLE _dblValue;
static LONG tstudy;
static BOOL first_time=TRUE;
    string stmt_sql;

	tValue.ounitid  = CharValue->ounitid;
	tValue.traitid = CharValue->traitid;
	tValue.scaleid = CharValue->scaleid;
	tValue.tmethid = CharValue->tmethid;

	if (first_time) {
       BINDPARAM(1,tValue.traitid);
       BINDPARAM(2,tValue.scaleid);
       BINDPARAM(3,tValue.tmethid);
       BINDPARAM(4,tValue.ounitid); 
       BINDS(1,tValue.cvalue,DMS_DATA_VALUE-1);
     }
     local.Execute();
     central.Execute();

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*CharValue = tValue;
		ret = DMS_SUCCESS;
	}
	else if (source->NoData())
		ret = DMS_NO_DATA;
	else
		ret = DMS_ERROR;
    POP();

#undef getDataCOfOunit_SQL

}



//--------------------------------------------------------------------------
//   DMS_searchLevelCOfOunit
//      Given:  ounitid, traitid, scaleid, methodid, 
//      Output: data value value
//      Date Created:  Dec 20, 2005
//--------------------------------------------------------------------------
LONG searchLevelCOfOunit(BOOL fLocal, DMS_CharVALUE *CharValue)
{
#define getLevelCOfOunit_SQL " SELECT LEVEL_C.LVALUE \
             FROM LEVEL_C , FACTOR, OINDEX \
	         WHERE LEVEL_C.LABELID = FACTOR.LABELID AND LEVEL_C.LEVELNO = OINDEX.LEVELNO \
              AND FACTOR.TRAITID=? AND FACTOR.SCALEID=? AND FACTOR.TMETHID=? AND OINDEX.OUNITID=? "


	PUSH(getLevelCOfOunit);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(getLevelCOfOunit_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(getLevelCOfOunit_SQL);

       CODBCbindedStmt *source;

static DMS_CharVALUE tValue;
static DOUBLE _dblValue;
static LONG tstudy;
static BOOL first_time=TRUE;
    string stmt_sql;

	tValue.ounitid  = CharValue->ounitid;
	tValue.traitid = CharValue->traitid;
	tValue.scaleid = CharValue->scaleid;
	tValue.tmethid = CharValue->tmethid;

	if (first_time) {
       BINDPARAM(1,tValue.traitid);
       BINDPARAM(2,tValue.scaleid);
       BINDPARAM(3,tValue.tmethid);
       BINDPARAM(4,tValue.ounitid); 
       BINDS(1,tValue.cvalue,DMS_DATA_VALUE-1);
     }
     local.Execute();
     central.Execute();

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*CharValue = tValue;
		ret = DMS_SUCCESS;
	}
	else if (source->NoData())
		ret = DMS_NO_DATA;
	else
		ret = DMS_ERROR;
    POP();

#undef getLevelCOfOunit_SQL

}


LONG defineOunitLN(BOOL fLocal,DMS_NumVALUE *FactorN,int fopt)
{

#define DefineOFN_SQL "SELECT FACTOR.TRAITID, FACTOR.SCALEID, FACTOR.TMETHID, LEVEL_N.LVALUE  \
        FROM FACTOR, OINDEX,  LEVEL_N WHERE  OINDEX.OUNITID = ?  \
		AND OINDEX.LEVELNO =  LEVEL_N.LEVELNO   AND FACTOR.LABELID =  LEVEL_N.LABELID "

	PUSH(defineOunitLN);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(DefineOFN_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(DefineOFN_SQL);
       CODBCbindedStmt *source;
static DMS_NumVALUE tNValue;

	tNValue.ounitid = FactorN->ounitid;

    if (fopt == FIND_FIRST) {

       BINDPARAM(1,tNValue.ounitid);
       BIND(1,tNValue.traitid);
       BIND(2,tNValue.scaleid);
       BIND(3,tNValue.tmethid);
       BIND(4,tNValue.nvalue); 
	   local.Execute();
	   central.Execute();
    }
    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*FactorN = tNValue;
		ret = DMS_SUCCESS;
	}
	else if (source->NoData())
		ret = DMS_NO_DATA;
	else
		ret = DMS_ERROR;
    POP();
#undef DefineOFN_SQL
}


LONG defineOunitLC(BOOL fLocal,DMS_CharVALUE *FactorC, int fopt)
{
#define DefineOFC_SQL "SELECT FACTOR.TRAITID, FACTOR.SCALEID, FACTOR.TMETHID, LEVEL_C.LVALUE \
        FROM FACTOR, OINDEX, LEVEL_C WHERE  OINDEX.OUNITID = ?  \
		AND OINDEX.LEVELNO = LEVEL_C.LEVELNO   AND FACTOR.LABELID = LEVEL_C.LABELID "

	PUSH(defineOunitLC);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(DefineOFC_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(DefineOFC_SQL);
       CODBCbindedStmt *source;

long nlval=SQL_NO_TOTAL;
static DMS_CharVALUE tNValue;

	tNValue.ounitid = FactorC->ounitid;
    *FactorC->cvalue = '\0';

    if (fopt == FIND_FIRST) {
       BINDPARAM(1,tNValue.ounitid);
       BIND(1,tNValue.traitid);
       BIND(2,tNValue.scaleid);
       BIND(3,tNValue.tmethid);
       BINDS(4,tNValue.cvalue,DMS_DATA_VALUE -1); 
	   local.Execute();
	   central.Execute();
    }
    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*FactorC = tNValue;
		ret = DMS_SUCCESS;
	}
	else if (source->NoData())
		ret = DMS_NO_DATA;
	else
		ret = DMS_ERROR;
    POP();

#undef DefineOFN_SQL
}


LONG defineOunitDN(BOOL fLocal,DMS_NumVALUE *VariateN,int fopt)
{
#define DefineOVN_SQL "SELECT VARIATE.TRAITID, VARIATE.SCALEID, VARIATE.TMETHID, DATA_N.DVALUE \
        FROM VARIATE, DATA_N WHERE  DATA_N.OUNITID = ? AND VARIATE.VARIATID = DATA_N.VARIATID "

	PUSH(defineOunitDN);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(DefineOVN_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(DefineOVN_SQL);
       CODBCbindedStmt *source;
    static DMS_NumVALUE tNValue;
	tNValue.ounitid = VariateN->ounitid;

    if (fopt == FIND_FIRST) {
       BINDPARAM(1,tNValue.ounitid);
       BIND(1,tNValue.traitid);
       BIND(2,tNValue.scaleid);
       BIND(3,tNValue.tmethid);
       BIND(4,tNValue.nvalue); 
	   local.Execute();
	   central.Execute();
    }
    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*VariateN = tNValue;
		ret = DMS_SUCCESS;
	}
	else if (source->NoData())
		ret = DMS_NO_DATA;
	else
		ret = DMS_ERROR;
    POP();
#undef DefineOVN_SQL

}


LONG defineOunitDC(BOOL fLocal,DMS_CharVALUE *VariateC, int fopt)
{
#define DefineOVC_SQL "SELECT VARIATE.TRAITID, VARIATE.SCALEID, VARIATE.TMETHID, DATA_C.DVALUE \
        FROM VARIATE, DATA_C WHERE  DATA_C.OUNITID = ? AND VARIATE.VARIATID = DATA_C.VARIATID "

	PUSH(defineOunitDN);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(DefineOVC_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(DefineOVC_SQL);
       CODBCbindedStmt *source;

	long nlval=SQL_NO_TOTAL;
    static DMS_CharVALUE tNValue;

	tNValue.ounitid = VariateC->ounitid;
    *VariateC->cvalue = '\0';

    if (fopt == FIND_FIRST) {
       BINDPARAM(1,tNValue.ounitid);
       BIND(1,tNValue.traitid);
       BIND(2,tNValue.scaleid);
       BIND(3,tNValue.tmethid);
       BINDS(4,tNValue.cvalue, DMS_DATA_VALUE-1); 
	   local.Execute();
	   central.Execute();
    }
    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*VariateC = tNValue;
		ret = DMS_SUCCESS;
	}
	else if (source->NoData())
		ret = DMS_NO_DATA;
	else
		ret = DMS_ERROR;
    POP();
#undef DefineOVC_SQL

}


LONG findStEffect(BOOL fLocal,LONG studyid, LONG *Represno)
{
#define SQL_findStEffect "SELECT E.REPRESNO, Count(E.REPRESNO) AS CntEffect \
                      FROM FACTOR  F, EFFECT  E \
                      WHERE (((F.LABELID)=E.FACTORID) AND ((F.STUDYID)=?)) \
                      GROUP BY E.REPRESNO  \
                      HAVING (((Count(E.REPRESNO))=1))"
	PUSH(findStEffect);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(SQL_findStEffect);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(SQL_findStEffect);
       CODBCbindedStmt *source;
static long tRepresno, tcntRepres, tstudyid;
static BOOL first_time=TRUE;

   if (first_time) {
	  BIND(1,tRepresno);
	  BIND(2,tcntRepres);
	  BINDPARAM(1, tstudyid);
   }
   local.Execute();
   central.Execute();

   source = (fLocal)?&local:&central;
   tstudyid = studyid;

   source->Execute();
   if (source->Fetch())
   {
      ret=DMS_SUCCESS;
	  *Represno = tRepresno;
   }
   else if (source->NoData())
      ret = GMS_NO_DATA;
   else 
      ret = GMS_ERROR;

   POP();

#undef SQL_findStEffect

}



LONG findEffectOunit(BOOL fLocal,LONG represno, LONG *ounitid ,int fopt)
{

#define FindEffectOunit_SQL "SELECT DISTINCT OUNITID from OINDEX where REPRESNO = ?  "

	PUSH(findEffectOunit);

static CODBCbindedStmt &central= _centralDMS->BindedStmt(FindEffectOunit_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(FindEffectOunit_SQL);
       CODBCbindedStmt *source;

static LONG tounit, trepresno;
    BOOL first_time = TRUE;

    if (fopt == FIND_FIRST) {
		if (first_time) {
            BIND(1,tounit); 
            BINDPARAM(1,trepresno);
			first_time = FALSE;
		}
		trepresno = represno;
        local.Execute();
	    central.Execute();
    }


    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*ounitid = tounit;
		ret = DMS_SUCCESS;
	}
	else if (source->NoData())
		ret = DMS_NO_DATA;
	else
		ret = DMS_ERROR;
    POP();
#undef FindEffectOunit_SQL

}



// Description  : Finds the header unit of the study; the header unit contains the global factor of the study
//
LONG findStOunit(BOOL fLocal, DMS_CharVALUE *CharValue,int fopt)
{

#define FindStO_SQL "SELECT OINDEX.OUNITID FROM OINDEX, FACTOR, LEVEL_C \
        WHERE FACTOR.TRAITID=? AND LEVEL_C.FACTORID = FACTOR.FACTORID \
		AND LTRIM(RTRIM(LEVEL_C.LVALUE)) LIKE ? AND OINDEX.FACTORID = LEVEL_C.FACTORID AND OINDEX.LEVELNO = LEVEL_C.LEVELNO  \
		AND OINDEX.REPRESNO = ? "

	PUSH(findStOunit);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(FindStO_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(FindStO_SQL);
       CODBCbindedStmt *source;

    LONG npval = SQL_NTS;
    DMS_CharVALUE tNValue;

	*tNValue.cvalue  = '\0';
	strcat(tNValue.cvalue,CharValue->cvalue);
	tNValue.traitid = CharValue->traitid;
	tNValue.scaleid = CharValue->scaleid;
	tNValue.tmethid = CharValue->tmethid;

    if (fopt == FIND_FIRST) {
       BINDPARAM(1,tNValue.traitid);
       BINDPARAMS(2,tNValue.cvalue,DMS_DATA_VALUE-1);  
       BIND(1,tNValue.ounitid); 
       local.Execute();
	   central.Execute();
    }
    source = (fLocal)?&local:&central;
	if (source->Fetch()){
		*CharValue = tNValue;
		ret = DMS_SUCCESS;
	}
	else if (source->NoData())
		ret = DMS_NO_DATA;
	else
		ret = DMS_ERROR;
    POP();
#undef FindStO_SQL

}



//--------------------------------------------------------------------------
//   DMS_searchDataC
//      Given:  traitid, scaleid, methodid
//      Output:  ounitid, level value
//--------------------------------------------------------------------------
LONG DLL_INTERFACE DMS_searchDataC_(long Trait, long Scale, long Method, long *Ounit, char *DValueC, long cDataLen, long fopt)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_searchDataC);
   if (fopt==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal){
      ret=searchDataC(LOCAL,Trait, Scale, Method, Ounit,DValueC,cDataLen, fopt);
      if (ret!=DMS_SUCCESS){
         fLocal = FALSE; 
         ret=searchDataC(CENTRAL,Trait, Scale, Method, Ounit,DValueC,cDataLen, FIND_FIRST);
      }
   }
   else
      ret=searchDataC(CENTRAL,Trait, Scale, Method, Ounit,DValueC,cDataLen, fopt);

   POP();

}

//--------------------------------------------------------------------------
//   DMS_searchDataC
//      Given:  traitid, scaleid, methodid
//      Output:  ounitid, level value
//   Uses the searchDataC1, searchDataC2, searchDataC3, searchDataC4
//--------------------------------------------------------------------------
LONG DLL_INTERFACE DMS_searchDataC(long Trait, long Scale, long Method, long *Ounit, char *DValueC, long cDataLen, long fopt)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_searchDataC);
   if (fopt==FIND_FIRST)
         fLocal=TRUE;
   ret =0;
   if ((Trait!=0) && (Scale==0) && (Method==0)) {
     if (fLocal){
        ret=searchDataC1(LOCAL,Trait, Scale, Method, Ounit,DValueC,cDataLen, fopt);
        if (ret!=DMS_SUCCESS){
           fLocal = FALSE; 
           ret=searchDataC1(CENTRAL,Trait, Scale, Method, Ounit,DValueC,cDataLen, FIND_NEXT);
		}
	 }
     else
        ret=searchDataC1(CENTRAL,Trait, Scale, Method, Ounit,DValueC,cDataLen, fopt);
   }

   if ((Trait!=0) && (Scale!=0) && (Method==0)) {
     if (fLocal){
        ret=searchDataC2(LOCAL,Trait, Scale, Method, Ounit,DValueC,cDataLen, fopt);
        if (ret!=DMS_SUCCESS){
           fLocal = FALSE; 
           ret=searchDataC2(CENTRAL,Trait, Scale, Method, Ounit,DValueC,cDataLen, FIND_NEXT);
		}
	 }
     else
        ret=searchDataC2(CENTRAL,Trait, Scale, Method, Ounit,DValueC,cDataLen, fopt);
   }

   if ((Trait!=0) && (Scale==0) && (Method!=0)) {
     if (fLocal){
        ret=searchDataC3(LOCAL,Trait, Scale, Method, Ounit,DValueC,cDataLen, fopt);
        if (ret!=DMS_SUCCESS){
           fLocal = FALSE; 
           ret=searchDataC3(CENTRAL,Trait, Scale, Method, Ounit,DValueC,cDataLen, FIND_NEXT);
		}
	 }
     else
        ret=searchDataC3(CENTRAL,Trait, Scale, Method, Ounit,DValueC,cDataLen, fopt);
   }

   if ((Trait!=0) && (Scale!=0) && (Method!=0)) {
     if (fLocal){
        ret=searchDataC4(LOCAL,Trait, Scale, Method, Ounit,DValueC,cDataLen, fopt);
        if (ret!=DMS_SUCCESS){
           fLocal = FALSE; 
           ret=searchDataC4(CENTRAL,Trait, Scale, Method, Ounit,DValueC,cDataLen, FIND_NEXT);
		}
	 }
     else
        ret=searchDataC4(CENTRAL,Trait, Scale, Method, Ounit,DValueC,cDataLen, fopt);
   }


   POP();

}



//--------------------------------------------------------------------------
//   DMS_searchDataN
//      Given:  traitid, scaleid, methodid
//      Output:  ounitid, data value
//--------------------------------------------------------------------------
LONG DLL_INTERFACE DMS_searchDataN_(long Trait, long Scale, long Method, long *Ounit, double *DValueN, long fopt)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_searchDataN);
   if (fopt==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal){
      ret=searchDataN(LOCAL,Trait, Scale, Method, Ounit,DValueN,  fopt);
      if (ret!=DMS_SUCCESS){
         fLocal = FALSE; 
         ret=searchDataN(CENTRAL,Trait, Scale, Method, Ounit,DValueN,  FIND_FIRST);
      }
   }
   else
      ret=searchDataN(CENTRAL,Trait, Scale, Method, Ounit,DValueN, fopt);
   POP();

}


//--------------------------------------------------------------------------
//   DMS_searchDataN
//      Given:  traitid, scaleid, methodid
//      Output:  ounitid, data value
//   Uses the searchDataN1, searchDataN2, searchDataN3, searchDataN4
//--------------------------------------------------------------------------

LONG DLL_INTERFACE DMS_searchDataN(long Trait, long Scale, long Method, long *Ounit, double *DValueN, long fopt)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_searchDataN);
   if (fopt==FIND_FIRST)
         fLocal=TRUE;

   if ((Trait!=0) && (Scale==0) && (Method==0)) {
     if (fLocal){
        ret=searchDataN1(LOCAL,Trait, Scale, Method, Ounit,DValueN,  fopt);
        if (ret!=DMS_SUCCESS){
           fLocal = FALSE; 
           ret=searchDataN1(CENTRAL,Trait, Scale, Method, Ounit,DValueN,  FIND_FIRST);
		}
	 }
     else
        ret=searchDataN1(CENTRAL,Trait, Scale, Method, Ounit,DValueN, fopt);
   }

   if ((Trait!=0) && (Scale!=0) && (Method==0)) {
     if (fLocal){
        ret=searchDataN2(LOCAL,Trait, Scale, Method, Ounit,DValueN,  fopt);
        if (ret!=DMS_SUCCESS){
           fLocal = FALSE; 
           ret=searchDataN2(CENTRAL,Trait, Scale, Method, Ounit,DValueN,  FIND_FIRST);
		}
	 }
     else
        ret=searchDataN2(CENTRAL,Trait, Scale, Method, Ounit,DValueN, fopt);
   }

   if ((Trait!=0) && (Scale==0) && (Method!=0)) {
     if (fLocal){
        ret=searchDataN3(LOCAL,Trait, Scale, Method, Ounit,DValueN,  fopt);
        if (ret!=DMS_SUCCESS){
           fLocal = FALSE; 
           ret=searchDataN3(CENTRAL,Trait, Scale, Method, Ounit,DValueN,  FIND_FIRST);
		}
	 }
     else
        ret=searchDataN3(CENTRAL,Trait, Scale, Method, Ounit,DValueN, fopt);
   }

   if ((Trait!=0) && (Scale!=0) && (Method!=0)) {
     if (fLocal){
        ret=searchDataN4(LOCAL,Trait, Scale, Method, Ounit,DValueN,  fopt);
        if (ret!=DMS_SUCCESS){
           fLocal = FALSE; 
           ret=searchDataN4(CENTRAL,Trait, Scale, Method, Ounit,DValueN,  FIND_FIRST);
		}
	 }
     else
        ret=searchDataN4(CENTRAL,Trait, Scale, Method, Ounit,DValueN, fopt);
   }


   POP();

}



//--------------------------------------------------------------------------
//   DMS_searchLevelC
//      Given:  traitid, scaleid, methodid
//      Output:  ounitid, level value
//--------------------------------------------------------------------------
LONG DLL_INTERFACE DMS_searchLevelC_(long Trait, long Scale, long Method, long *Ounit, char *FLevelC, long cLevelLen, long fopt)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_searchLevelC);
   if (fopt==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal){
      ret=searchLevelC(LOCAL,Trait, Scale, Method, Ounit,FLevelC,cLevelLen,  fopt);
      if (ret!=DMS_SUCCESS){
         fLocal = FALSE; 
         ret=searchLevelC(CENTRAL,Trait, Scale, Method, Ounit,FLevelC, cLevelLen, FIND_NEXT);
      }
   }
   else
      ret=searchLevelC(CENTRAL,Trait, Scale, Method, Ounit,FLevelC,cLevelLen, fopt);
   POP();

}

//--------------------------------------------------------------------------
//   DMS_searchLevelC
//      Given:  traitid, scaleid, methodid
//      Output:  ounitid, level value
//--------------------------------------------------------------------------
LONG DLL_INTERFACE DMS_searchLevelC(long Trait, long Scale, long Method, long *Ounit, char *FLevelC, long cLevelLen, long fopt)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_searchLevelC);
   if (fopt==FIND_FIRST)
         fLocal=TRUE;
   ret =0;
   if ((Trait!=0) && (Scale==0) && (Method==0)) {
     if (fLocal){
        ret=searchLevelC1(LOCAL,Trait, Scale, Method, Ounit,FLevelC,cLevelLen,  fopt);
        if (ret!=DMS_SUCCESS){
           fLocal = FALSE; 
           ret=searchLevelC1(CENTRAL,Trait, Scale, Method, Ounit,FLevelC, cLevelLen, FIND_NEXT);
		}
	 }
     else
        ret=searchLevelC1(CENTRAL,Trait, Scale, Method, Ounit,FLevelC,cLevelLen, fopt);
   }

   if ((Trait!=0) && (Scale!=0) && (Method==0)) {
     if (fLocal){
        ret=searchLevelC2(LOCAL,Trait, Scale, Method, Ounit,FLevelC,cLevelLen,  fopt);
        if (ret!=DMS_SUCCESS){
           fLocal = FALSE; 
           ret=searchLevelC2(CENTRAL,Trait, Scale, Method, Ounit,FLevelC, cLevelLen, FIND_NEXT);
		}
	 }
     else
        ret=searchLevelC2(CENTRAL,Trait, Scale, Method, Ounit,FLevelC,cLevelLen, fopt);
   }

   if ((Trait!=0) && (Scale==0) && (Method!=0)) {
     if (fLocal){
        ret=searchLevelC3(LOCAL,Trait, Scale, Method, Ounit,FLevelC,cLevelLen,  fopt);
        if (ret!=DMS_SUCCESS){
           fLocal = FALSE; 
           ret=searchLevelC3(CENTRAL,Trait, Scale, Method, Ounit,FLevelC, cLevelLen, FIND_NEXT);
		}
	 }
     else
        ret=searchLevelC3(CENTRAL,Trait, Scale, Method, Ounit,FLevelC,cLevelLen, fopt);
   }

   if ((Trait!=0) && (Scale!=0) && (Method!=0)) {
     if (fLocal){
        ret=searchLevelC4(LOCAL,Trait, Scale, Method, Ounit,FLevelC,cLevelLen,  fopt);
        if (ret!=DMS_SUCCESS){
           fLocal = FALSE; 
           ret=searchLevelC4(CENTRAL,Trait, Scale, Method, Ounit,FLevelC, cLevelLen, FIND_NEXT);
		}
	 }
     else
        ret=searchLevelC4(CENTRAL,Trait, Scale, Method, Ounit,FLevelC,cLevelLen, fopt);
   }

   POP();

}


//--------------------------------------------------------------------------
//   DMS_searchLevelN
//      Given:  traitid, scaleid, methodid
//      Output:  ounitid, data value
//--------------------------------------------------------------------------
LONG DLL_INTERFACE DMS_searchLevelN_(long Trait, long Scale, long Method, long *Ounit, double *FLevelN, long fopt)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_searchLevelN);
   if (fopt==FIND_FIRST)
         fLocal=TRUE;
   if (fLocal){
      ret=searchLevelN(LOCAL,Trait, Scale, Method, Ounit,FLevelN,  fopt);
      if (ret!=DMS_SUCCESS){
         fLocal = FALSE; 
         ret=searchLevelN(CENTRAL,Trait, Scale, Method, Ounit,FLevelN,  FIND_FIRST);
      }
   }
   else
      ret=searchLevelN(CENTRAL,Trait, Scale, Method, Ounit,FLevelN, fopt);
   POP();

}


//--------------------------------------------------------------------------
//   DMS_searchLevelN
//      Given:  traitid, scaleid, methodid
//      Output:  ounitid, data value
//--------------------------------------------------------------------------
LONG DLL_INTERFACE DMS_searchLevelN(long Trait, long Scale, long Method, long *Ounit, double *FLevelN, long fopt)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_searchLevelC);
   if (fopt==FIND_FIRST)
         fLocal=TRUE;
   if ((Trait!=0) && (Scale==0) && (Method==0)) {
       if (fLocal){
         ret=searchLevelN1(LOCAL,Trait, Scale, Method, Ounit,FLevelN,  fopt);
         if (ret!=DMS_SUCCESS){
           fLocal = FALSE; 
           ret=searchLevelN1(CENTRAL,Trait, Scale, Method, Ounit,FLevelN,  FIND_FIRST);
		 }
	   } 
       else
         ret=searchLevelN1(CENTRAL,Trait, Scale, Method, Ounit,FLevelN, fopt);
   }

   if ((Trait!=0) && (Scale!=0) && (Method==0)) {
       if (fLocal){
         ret=searchLevelN2(LOCAL,Trait, Scale, Method, Ounit,FLevelN,  fopt);
         if (ret!=DMS_SUCCESS){
           fLocal = FALSE; 
           ret=searchLevelN2(CENTRAL,Trait, Scale, Method, Ounit,FLevelN,  FIND_FIRST);
		 }
	   } 
       else
         ret=searchLevelN2(CENTRAL,Trait, Scale, Method, Ounit,FLevelN, fopt);
   }

   if ((Trait!=0) && (Scale==0) && (Method!=0)) {
       if (fLocal){
         ret=searchLevelN3(LOCAL,Trait, Scale, Method, Ounit,FLevelN,  fopt);
         if (ret!=DMS_SUCCESS){
           fLocal = FALSE; 
           ret=searchLevelN3(CENTRAL,Trait, Scale, Method, Ounit,FLevelN,  FIND_FIRST);
		 }
	   } 
       else
         ret=searchLevelN3(CENTRAL,Trait, Scale, Method, Ounit,FLevelN, fopt);
   }

   if ((Trait!=0) && (Scale!=0) && (Method!=0)) {
       if (fLocal){
         ret=searchLevelN4(LOCAL,Trait, Scale, Method, Ounit,FLevelN,  fopt);
         if (ret!=DMS_SUCCESS){
           fLocal = FALSE; 
           ret=searchLevelN4(CENTRAL,Trait, Scale, Method, Ounit,FLevelN,  FIND_FIRST);
		 }
	   } 
       else
         ret=searchLevelN4(CENTRAL,Trait, Scale, Method, Ounit,FLevelN, fopt);
   }

   POP();

}


 //--------------------------------------------------------------------------
 //   DMS_extractOunitDC
 //      Desc :  Extracts the observation unit with the specified data value for the given trait, scale and methof
 //      Given:  traitid, scaleid, methodid, operator (<,>,=,<>,>=,<=), character level value, studyid
 //      Output:  ounitid
 //--------------------------------------------------------------------------
LONG DLL_INTERFACE DMS_extractOunitDC_(long studyid,char* chrVal, DMS_CharVALUE *CharValue,int fopt)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_extractOunitDC);
	if (studyid == 0) {
	if (fopt == FIND_FIRST) {
	    fLocal = TRUE;
	}
	if (fLocal) {
	   ret=extractOunitDC(LOCAL,studyid,chrVal,CharValue,fopt);
	   if (ret == DMS_NO_DATA) {
	     fLocal = FALSE;
	     ret=extractOunitDC(CENTRAL,studyid,chrVal,CharValue,FIND_FIRST);
	   }
	}
    else {
	   ret=extractOunitDC(CENTRAL,studyid,chrVal,CharValue,fopt);
	}
	}

	else {
		if (studyid < 0) {
	     ret=extractOunitDC(LOCAL,studyid,chrVal,CharValue,fopt);
		}
		else
		{
	     ret=extractOunitDC(CENTRAL,studyid,chrVal,CharValue,fopt);
		}
	}
	POP()
}


 //--------------------------------------------------------------------------
 //   DMS_extractOunitDC
 //      Desc :  Extracts the observation unit with the specified data value for the given trait, scale and methof
 //      Given:  traitid, scaleid, methodid, operator (<,>,=,<>,>=,<=), character level value, studyid
 //      Output:  ounitid
 //--------------------------------------------------------------------------
LONG DLL_INTERFACE DMS_extractOunitDC(long studyid,char* chrVal, DMS_CharVALUE *CharValue,int fopt)
{
   static BOOL fLocal=TRUE;
   static BOOL name_wild=FALSE;

   PUSH(DMS_findFactor);
   if (fopt==FIND_FIRST){
      fLocal=TRUE;
	  name_wild =strchr(chrVal,'_') || strchr(chrVal,'%');
   }
   if (name_wild) {
	if (fLocal) {
	   ret=extractOunitDC1(LOCAL,studyid,chrVal,CharValue,fopt);
	   if (ret == DMS_NO_DATA) {
	     fLocal = FALSE;
	     ret=extractOunitDC1(CENTRAL,studyid,chrVal,CharValue,FIND_NEXT);
	   }
	}
    else {
 	   ret=extractOunitDC1(CENTRAL,studyid,chrVal,CharValue,fopt);
	}
   }
   else {
	if (fLocal) {
	   ret=extractOunitDC2(LOCAL,studyid,chrVal,CharValue,fopt);
	   if (ret == DMS_NO_DATA) {
	     fLocal = FALSE;
	     ret=extractOunitDC2(CENTRAL,studyid,chrVal,CharValue,FIND_NEXT);
	   }
	}
    else {
 	   ret=extractOunitDC2(CENTRAL,studyid,chrVal,CharValue,fopt);
	}
   }

   POP();

}
 

 //--------------------------------------------------------------------------
 //   DMS_extractOunitLC
 //      Desc :  Extracts the observation unit with the specified data value for the given trait, scale and methof
 //      Given:  traitid, scaleid, methodid, operator (<,>,=,<>,>=,<=), character level value, studyid
 //      Output:  ounitid
 //--------------------------------------------------------------------------
LONG DLL_INTERFACE DMS_extractOunitLC_(long studyid,char* chrVal, DMS_CharVALUE *CharValue,int fopt)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_extractOunitLC);
	if (studyid == 0) {
	if (fopt == FIND_FIRST) {
	    fLocal = TRUE;
	}
	if (fLocal) {
	   ret=extractOunitLC(LOCAL,studyid,chrVal,CharValue,fopt);
	   if (ret == DMS_NO_DATA) {
	     fLocal = FALSE;
	     ret=extractOunitLC(CENTRAL,studyid,chrVal,CharValue,FIND_FIRST);
	   }
	}
    else {
	   ret=extractOunitLC(CENTRAL,studyid,chrVal,CharValue,fopt);
	}
	}

	else {
		if (studyid < 0) {
	     ret=extractOunitLC(LOCAL,studyid,chrVal,CharValue,fopt);
		}
		else
		{
	     ret=extractOunitLC(CENTRAL,studyid,chrVal,CharValue,fopt);
		}
	}
	POP()
}


//--------------------------------------------------------------------------
 //   DMS_extractOunitLC
 //      Desc :  Extracts the observation unit with the specified data value for the given trait, scale and methof
 //      Given:  traitid, scaleid, methodid, operator (<,>,=,<>,>=,<=), character level value, studyid
 //      Output:  ounitid
 //--------------------------------------------------------------------------
LONG DLL_INTERFACE DMS_extractOunitLC(long studyid,char* chrVal, DMS_CharVALUE *CharValue,int fopt)
{
   static BOOL fLocal=TRUE;
   static BOOL name_wild=FALSE;

   PUSH(DMS_extractOunitLC);
   if (fopt==FIND_FIRST){
      fLocal=TRUE;
	  name_wild =strchr(chrVal,'_') || strchr(chrVal,'%');
   }
   if (name_wild) {
	if (fLocal) {
	   ret=extractOunitLC1(LOCAL,studyid,chrVal,CharValue,fopt);
	   if (ret == DMS_NO_DATA) {
	     fLocal = FALSE;
	     ret=extractOunitLC1(CENTRAL,studyid,chrVal,CharValue,FIND_NEXT);
	   }
	}
    else {
 	   ret=extractOunitLC1(CENTRAL,studyid,chrVal,CharValue,fopt);
	}
   }
   else {
	if (fLocal) {
	   ret=extractOunitLC2(LOCAL,studyid,chrVal,CharValue,fopt);
	   if (ret == DMS_NO_DATA) {
	     fLocal = FALSE;
	     ret=extractOunitLC2(CENTRAL,studyid,chrVal,CharValue,FIND_NEXT);
	   }
	}
    else {
 	   ret=extractOunitLC2(CENTRAL,studyid,chrVal,CharValue,fopt);
	}
   }

   POP();

}


LONG DLL_INTERFACE DMS_extractOunitLN_(long studyid,double dblValue, DMS_NumVALUE *NumValue,int oprtr, int fopt)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_extractOunitLN);
	if (studyid == 0) {
	if (fopt == FIND_FIRST) {
	    fLocal = TRUE;
	}
	if (fLocal) {
	   ret=extractOunitLN(LOCAL,studyid,dblValue,NumValue,oprtr,fopt);
	   if (ret == DMS_NO_DATA) {
         fLocal = FALSE;
	     ret=extractOunitLN(CENTRAL,studyid,dblValue, NumValue,oprtr,FIND_FIRST);
	   }
	}
    else {
	   ret=extractOunitLN(CENTRAL,studyid,dblValue, NumValue,oprtr,fopt);
	}
	}

	else {
		if (studyid < 0) {
	      ret=extractOunitLN(LOCAL,studyid,dblValue,NumValue,oprtr,fopt);
		}
		else
		{
	      ret=extractOunitLN(CENTRAL,studyid,dblValue, NumValue,oprtr,fopt);
		}
	}
	POP();
}


LONG DLL_INTERFACE DMS_extractOunitLN(long studyid,double dblValue, DMS_NumVALUE *NumValue,int oprtr, int fopt)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_extractOunitDN);
	if (fopt == FIND_FIRST) {
	    fLocal = TRUE;
	}

    if (oprtr == DMS_EQ ) {
   	   if (fLocal) {
	      ret=extractOunitLN_EQ(LOCAL,studyid,dblValue,NumValue,oprtr,fopt);
	      if (ret == DMS_NO_DATA) {
            fLocal = FALSE;
	        ret=extractOunitLN_EQ(CENTRAL,studyid,dblValue, NumValue,oprtr,FIND_FIRST);
		  }
	   }
       else 
	      ret=extractOunitLN_EQ(CENTRAL,studyid,dblValue, NumValue,oprtr,fopt);
	}

    if (oprtr == DMS_LT ) {
   	   if (fLocal) {
	      ret=extractOunitLN_LT(LOCAL,studyid,dblValue,NumValue,oprtr,fopt);
	      if (ret == DMS_NO_DATA) {
            fLocal = FALSE;
	        ret=extractOunitLN_LT(CENTRAL,studyid,dblValue, NumValue,oprtr,FIND_FIRST);
		  }
	   }
       else 
	      ret=extractOunitLN_LT(CENTRAL,studyid,dblValue, NumValue,oprtr,fopt);
	}

	if (oprtr == DMS_GT ) {
   	   if (fLocal) {
	      ret=extractOunitLN_GT(LOCAL,studyid,dblValue,NumValue,oprtr,fopt);
	      if (ret == DMS_NO_DATA) {
            fLocal = FALSE;
	        ret=extractOunitLN_GT(CENTRAL,studyid,dblValue, NumValue,oprtr,FIND_FIRST);
		  }
	   }
       else 
	      ret=extractOunitLN_GT(CENTRAL,studyid,dblValue, NumValue,oprtr,fopt);
	}

    if (oprtr == DMS_GTE) {
   	   if (fLocal) {
	      ret=extractOunitLN_GTE(LOCAL,studyid,dblValue,NumValue,oprtr,fopt);
	      if (ret == DMS_NO_DATA) {
            fLocal = FALSE;
	        ret=extractOunitLN_GTE(CENTRAL,studyid,dblValue, NumValue,oprtr,FIND_FIRST);
		  }
	   }
       else 
	      ret=extractOunitLN_GTE(CENTRAL,studyid,dblValue, NumValue,oprtr,fopt);
	}

	if (oprtr == DMS_LTE) {
	   	   if (fLocal) {
	      ret=extractOunitLN_LTE(LOCAL,studyid,dblValue,NumValue,oprtr,fopt);
	      if (ret == DMS_NO_DATA) {
            fLocal = FALSE;
	        ret=extractOunitLN_LTE(CENTRAL,studyid,dblValue, NumValue,oprtr,FIND_FIRST);
		  }
	   }
       else 
	      ret=extractOunitLN_LTE(CENTRAL,studyid,dblValue, NumValue,oprtr,fopt);
	}


	POP();
}



LONG DLL_INTERFACE DMS_extractOunitDN_(long studyid,double dblValue, DMS_NumVALUE *NumValue,int oprtr, int fopt)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_extractOunitDN);
	if (studyid == 0) {
	if (fopt == FIND_FIRST) {
	    fLocal = TRUE;
	}
	if (fLocal) {
	   ret=extractOunitDN(LOCAL,studyid,dblValue,NumValue,oprtr,fopt);
	   if (ret == DMS_NO_DATA) {
         fLocal = FALSE;
	     ret=extractOunitDN(CENTRAL,studyid,dblValue, NumValue,oprtr,FIND_FIRST);
	   }
	}
    else {
	   ret=extractOunitDN(CENTRAL,studyid,dblValue, NumValue,oprtr,fopt);
	}
	}

	else {
		if (studyid < 0) {
	      ret=extractOunitDN(LOCAL,studyid,dblValue,NumValue,oprtr,fopt);
		}
		else
		{
	      ret=extractOunitDN(CENTRAL,studyid,dblValue, NumValue,oprtr,fopt);
		}
	}
	POP();
}



LONG DLL_INTERFACE DMS_extractOunitDN(long studyid,double dblValue, DMS_NumVALUE *NumValue,int oprtr, int fopt)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_extractOunitDN);
	if (fopt == FIND_FIRST) {
	    fLocal = TRUE;
	}

    if (oprtr == DMS_EQ ) {
   	   if (fLocal) {
	      ret=extractOunitDN_EQ(LOCAL,studyid,dblValue,NumValue,oprtr,fopt);
	      if (ret == DMS_NO_DATA) {
            fLocal = FALSE;
	        ret=extractOunitDN_EQ(CENTRAL,studyid,dblValue, NumValue,oprtr,FIND_FIRST);
		  }
	   }
       else 
	      ret=extractOunitDN_EQ(CENTRAL,studyid,dblValue, NumValue,oprtr,fopt);
	}

    if (oprtr == DMS_LT ) {
   	   if (fLocal) {
	      ret=extractOunitDN_LT(LOCAL,studyid,dblValue,NumValue,oprtr,fopt);
	      if (ret == DMS_NO_DATA) {
            fLocal = FALSE;
	        ret=extractOunitDN_LT(CENTRAL,studyid,dblValue, NumValue,oprtr,FIND_FIRST);
		  }
	   }
       else 
	      ret=extractOunitDN_LT(CENTRAL,studyid,dblValue, NumValue,oprtr,fopt);
	}

	if (oprtr == DMS_GT ) {
   	   if (fLocal) {
	      ret=extractOunitDN_GT(LOCAL,studyid,dblValue,NumValue,oprtr,fopt);
	      if (ret == DMS_NO_DATA) {
            fLocal = FALSE;
	        ret=extractOunitDN_GT(CENTRAL,studyid,dblValue, NumValue,oprtr,FIND_FIRST);
		  }
	   }
       else 
	      ret=extractOunitDN_GT(CENTRAL,studyid,dblValue, NumValue,oprtr,fopt);
	}

    if (oprtr == DMS_GTE) {
   	   if (fLocal) {
	      ret=extractOunitDN_GTE(LOCAL,studyid,dblValue,NumValue,oprtr,fopt);
	      if (ret == DMS_NO_DATA) {
            fLocal = FALSE;
	        ret=extractOunitDN_GTE(CENTRAL,studyid,dblValue, NumValue,oprtr,FIND_FIRST);
		  }
	   }
       else 
	      ret=extractOunitDN_GTE(CENTRAL,studyid,dblValue, NumValue,oprtr,fopt);
	}

	if (oprtr == DMS_LTE) {
	   	   if (fLocal) {
	      ret=extractOunitDN_LTE(LOCAL,studyid,dblValue,NumValue,oprtr,fopt);
	      if (ret == DMS_NO_DATA) {
            fLocal = FALSE;
	        ret=extractOunitDN_LTE(CENTRAL,studyid,dblValue, NumValue,oprtr,FIND_FIRST);
		  }
	   }
       else 
	      ret=extractOunitDN_LTE(CENTRAL,studyid,dblValue, NumValue,oprtr,fopt);
	}


	POP();
}


//--------------------------------------------------------------------------
//   DMS_searchDataNOfOunit
//      Given:  ounitid, traitid, scaleid, methodid, 
//      Output: data value value
//      Date Created:  Dec 20, 2005
//--------------------------------------------------------------------------
LONG DLL_INTERFACE DMS_searchDataNOfOunit(DMS_NumVALUE *data)
{

   PUSH(DMS_searchDataNOfOunit);
	if (data->ounitid < 0)  {
	   ret=searchDataNOfOunit(LOCAL,data);
	}
	else {
	   ret=searchDataNOfOunit(CENTRAL,data);
	}
	POP();

}


//--------------------------------------------------------------------------
//   DMS_searchDataCOfOunit
//      Given:  ounitid, traitid, scaleid, methodid, 
//      Output: data value value from the DATA_C table
//      Date Created:  Dec 20, 2005
//--------------------------------------------------------------------------
LONG DLL_INTERFACE DMS_searchDataCOfOunit(DMS_CharVALUE *data)
{

   PUSH(DMS_searchDataCOfOunit);
	if (data->ounitid < 0)  {
	   ret=searchDataCOfOunit(LOCAL,data);
	}
	else {
	   ret=searchDataCOfOunit(CENTRAL,data);
	}
	POP();

}

//--------------------------------------------------------------------------
//   DMS_searchLevelNOfOunit
//      Given:  ounitid, traitid, scaleid, methodid, 
//      Output: level value from LEVEL_N
//      Date Created:  Jan 3, 2006
//--------------------------------------------------------------------------
LONG DLL_INTERFACE DMS_searchLevelNOfOunit(DMS_NumVALUE *data)
{

   PUSH(DMS_searchLevelNOfOunit);
	if (data->ounitid < 0)  {
	   ret=searchLevelNOfOunit(LOCAL,data);
	}
	else {
	   ret=searchLevelNOfOunit(CENTRAL,data);
	}
	POP();

}


//--------------------------------------------------------------------------
//   DMS_searchLevelCOfOunit
//      Given:  ounitid, traitid, scaleid, methodid, 
//      Output: data value value from the LEVEL_C table
//      Date Created:  Jan 3, 2006
//--------------------------------------------------------------------------
LONG DLL_INTERFACE DMS_searchLevelCOfOunit(DMS_CharVALUE *data)
{

   PUSH(DMS_searchLevelCOfOunit);
	if (data->ounitid < 0)  {
	   ret=searchLevelCOfOunit(LOCAL,data);
	}
	else {
	   ret=searchLevelCOfOunit(CENTRAL,data);
	}
	POP();

}


LONG DLL_INTERFACE DMS_defineOunitLN(DMS_NumVALUE *FactorN,int fopt)
{

   PUSH(DMS_defineOunitLN);
	if (FactorN->ounitid < 0) {
	   ret=defineOunitLN(LOCAL,FactorN,fopt);
	}
	else {
	   ret=defineOunitLN(CENTRAL,FactorN,fopt);
	}
	POP();

}


LONG DLL_INTERFACE DMS_defineOunitLC(DMS_CharVALUE *FactorC, int fopt)
{

   PUSH(DMS_defineOunitLC);
	if (FactorC->ounitid < 0) {
	   ret=defineOunitLC(LOCAL,FactorC,fopt);
	}
	else {
	   ret=defineOunitLC(CENTRAL,FactorC,fopt);
	}
	POP();
}


LONG DLL_INTERFACE DMS_defineOunitDN(DMS_NumVALUE *VariateN,int fopt)
{

    PUSH(DMS_defineOunitDN);
	if (VariateN->ounitid < 0) {
	   ret=defineOunitDN(LOCAL,VariateN,fopt);
	}
	else {
	   ret=defineOunitDN(CENTRAL,VariateN,fopt);
	}
	POP();
}


LONG DLL_INTERFACE DMS_defineOunitDC(DMS_CharVALUE *VariateC, int fopt)
{
   PUSH(DMS_defineOunitDC);

   if (VariateC->ounitid < 0) {
	   ret=defineOunitDC(LOCAL,VariateC,fopt);
	}
	else {
	   ret=defineOunitDC(CENTRAL,VariateC,fopt);
    }
	POP();
}


LONG DLL_INTERFACE DMS_findStOunit_(DMS_CharVALUE *CharValue,int fopt)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_findStOunit);
	if (fopt == FIND_FIRST) {
		fLocal = TRUE;
	}
	if (fLocal) {
	   ret=findStOunit(LOCAL,CharValue,fopt);
	   if (ret == DMS_NO_DATA) {
	     fLocal = FALSE;
	     ret=findStOunit(CENTRAL,CharValue,FIND_FIRST);
	   }
	}
    else {
	   ret=findStOunit(CENTRAL,CharValue,fopt);
	}
	POP();
}


LONG DLL_INTERFACE DMS_findStEffect(long studyid,long *represno)
{
   static BOOL fLocal=TRUE;

   PUSH(DMS_findStEffect);
   ret=findStEffect(LOCAL,studyid,represno);
   if (ret == DMS_NO_DATA) {
     fLocal = FALSE;
    ret=findStEffect(CENTRAL,studyid,represno);
   }

	POP();
}



LONG DLL_INTERFACE DMS_findStOunit(DMS_CharVALUE *CharValue,int fopt)
{
#define NULLSTRING ""

   static BOOL fLocal=TRUE;
   long represno,ounitid=0;     
   CHAR szName[MAX_STR];
   DMS_STUDY study;
   PUSH(DMS_findStOunit);
   study.STUDYID =0;
   ZeroMemory(&study,sizeof(study));
   strncpy(szName,CharValue->cvalue, sizeof(CharValue->cvalue));
   ret = DMS_findStudy((CHAR *)szName, &study, FIND_FIRST);
   if (ret ==DMS_SUCCESS) {
	   if (study.STUDYID < 0){
          ret=findStEffect(LOCAL, study.STUDYID, &represno);
	      if (ret == DMS_SUCCESS)
			  ret = findEffectOunit(LOCAL,represno,&ounitid,FIND_FIRST);
	   }
	   else {
          ret=findStEffect(CENTRAL, study.STUDYID, &represno);
	      if (ret == DMS_SUCCESS)
			  ret = findEffectOunit(CENTRAL,represno,&ounitid,FIND_FIRST);
	   }
      CharValue->ounitid = ounitid;
   }

   POP();
}



/////////////////////////////////////////////////////////////////////////////////
/////  The following fuctions will the get the data values with the specified 
/////  Trait, Scale and Method and obsunit

////////////////////////////////////////////////////////////////////////////////
// Date Created: Nov 18, 2004
// Input: Trait, Ounitid
// Output: value 
// Description: Get the character data value with the given TMS trait and index ounitid
////////////////////////////////////////////////////////////////////////////////
LONG getDataCWithTMSIdx1(BOOL fLocal,LONG Trait, LONG Ounit, CHAR *DValueC, LONG cDataLen, LONG fopt)
{
#define DataCWithTMSIdx1_SQL "SELECT DATA_C.DVALUE FROM VARIATE, DATA_C \
        WHERE VARIATE.TRAITID=? AND DATA_C.OUNITID = ? AND DATA_C.VARIATID = VARIATE.VARIATID"    //scale==0 and method==0

	PUSH(getDataCWithTMSIdx1);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(DataCWithTMSIdx1_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(DataCWithTMSIdx1_SQL);
       CODBCbindedStmt *source;

static LONG tounit;
static LONG lngTrait= Trait;
static CHAR szName[DMS_DATA_VALUE];
static BOOL first_time=TRUE;
    string stmt_sql;

	*DValueC = '\0';
	if (fopt == FIND_FIRST) {
        lngTrait= Trait;
        tounit = Ounit; 
		if (first_time) {
          first_time = FALSE;
 	      BINDPARAM(1, lngTrait);
  	      BINDPARAM(2,tounit);
	      BINDS(1,szName,DMS_DATA_VALUE-1);
		}
        local.Execute();
        central.Execute();

	}

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
	    strncat(DValueC, szName, cDataLen);
		ret= DMS_SUCCESS;
	}
	else if (source->NoData())
		ret=  DMS_NO_DATA;
	else
		ret= DMS_ERROR;
    POP();
#undef DataCWithTMSIdx1
}

////////////////////////////////////////////////////////////////////////////////
// Date Created: Nov 18, 2004
// Input: Trait, Ounitid
// Output: value 
// Description: Get the character data value with the given TMS trait and index ounitid
////////////////////////////////////////////////////////////////////////////////
LONG getDataCWithTMSIdx2(BOOL fLocal,LONG Trait, LONG Scale, LONG Ounit, CHAR *DValueC, LONG cDataLen, LONG fopt)
{
#define DataCWithTMSIdx2_SQL "SELECT  DATA_C.DVALUE FROM VARIATE, DATA_C \
        WHERE VARIATE.TRAITID=? AND VARIATE.SCALEID=? AND DATA_C.OUNITID = ? AND DATA_C.VARIATID = VARIATE.VARIATID ORDER BY DATA_C.OUNITID"    //method==0

	PUSH(getDataCWithTMSIdx2);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(DataCWithTMSIdx2_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(DataCWithTMSIdx2_SQL);
       CODBCbindedStmt *source;

static LONG tounit;
static LONG lngTrait= Trait, lngScale=Scale;
static CHAR szName[DMS_DATA_VALUE];
static BOOL first_time=TRUE;
    string stmt_sql;

	*DValueC = '\0';
	if (fopt == FIND_FIRST) {
       lngTrait= Trait;
	   lngScale=Scale;
		if (first_time) {
          first_time = FALSE;
	      BINDPARAM(1, lngTrait);
	      BINDPARAM(2, lngScale);
	      BINDPARAM(3,tounit);
	      BINDS(1,szName,DMS_DATA_VALUE-1);
		}
       local.Execute();
       central.Execute();

	}

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
	    strncat(DValueC, szName, cDataLen);
		ret= DMS_SUCCESS;
	}
	else if (source->NoData())
		ret=  DMS_NO_DATA;
	else
		ret= DMS_ERROR;
    POP();
#undef DataCWithTMSIdx2
}


////////////////////////////////////////////////////////////////////////////////
// Date Created: Nov 18, 2004
// Input: Trait, Ounitid
// Output: value 
// Description: Get the character data value with the given TMS trait and index ounitid
////////////////////////////////////////////////////////////////////////////////
LONG getDataCWithTMSIdx3(BOOL fLocal,LONG Trait, LONG Method, LONG Ounit, CHAR *DValueC, LONG cDataLen, LONG fopt)
{
#define DataCWithTMSIdx3_SQL "SELECT  DATA_C.DVALUE FROM VARIATE, DATA_C \
        WHERE VARIATE.TRAITID=? AND VARIATE.TMETHID=? AND DATA_C.OUNITID AND DATA_C.VARIATID = VARIATE.VARIATID ORDER BY DATA_C.OUNITID"    //scale==0 

	PUSH(getDataCWithTMSIdx3);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(DataCWithTMSIdx3_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(DataCWithTMSIdx3_SQL);
       CODBCbindedStmt *source;

static LONG tounit;
static LONG lngTrait= Trait, lngMeth=Method;
static CHAR szName[DMS_DATA_VALUE];
static BOOL first_time=TRUE;
    string stmt_sql;

	*DValueC = '\0';
	if (fopt == FIND_FIRST) {
        lngTrait= Trait;
		lngMeth=Method;
		tounit = Ounit;
		if (first_time) {
          first_time = FALSE;
 	      BINDPARAM(1, lngTrait);
		  BINDPARAM(2, lngMeth);
	      BINDPARAM(3,tounit);
	      BINDS(1,szName,DMS_DATA_VALUE-1);
		}
        local.Execute();
        central.Execute();
	}

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
	    strncat(DValueC, szName, cDataLen);
		ret= DMS_SUCCESS;
	}
	else if (source->NoData())
		ret=  DMS_NO_DATA;
	else
		ret= DMS_ERROR;
    POP();
#undef DataCWithTMSIdx3_SQL
}


LONG getDataCWithTMSIdx4(BOOL fLocal,LONG Trait, LONG Scale, LONG Method, LONG Ounit, CHAR *DValueC, LONG cDataLen, LONG fopt)
{
#define DataCWithTMSIdx4_SQL "SELECT  DATA_C.DVALUE FROM VARIATE, DATA_C \
        WHERE VARIATE.TRAITID=? AND VARIATE.SCALEID = ? AND VARIATE.TMETHID=? AND DATA_C.OUNITID  AND  DATA_C.VARIATID = VARIATE.VARIATID ORDER BY DATA_C.OUNITID"

	PUSH(getDataCWithTMSIdx4);
static CODBCbindedStmt &central= _centralDMS->BindedStmt(DataCWithTMSIdx4_SQL);
static CODBCbindedStmt &local  = _localDMS->BindedStmt(DataCWithTMSIdx4_SQL);
       CODBCbindedStmt *source;

static LONG tounit;
static LONG lngTrait= Trait, lngScale=Scale, lngMeth=Method;
static CHAR szName[DMS_DATA_VALUE];
static BOOL first_time=TRUE;
    string stmt_sql;

	*DValueC = '\0';
	if (fopt == FIND_FIRST) {
        lngTrait= Trait;
		lngScale=Scale;
		lngMeth=Method;
		tounit = Ounit;
		if (first_time) {
 	      BINDPARAM(1, lngTrait);
	      BINDPARAM(2, lngScale);
	      BINDPARAM(3, lngMeth);
 	      BIND(4,tounit);
	      BINDS(1,szName,DMS_DATA_VALUE-1);
          first_time = FALSE;
		}
        local.Execute();
        central.Execute();

	}

    source = (fLocal)?&local:&central;
	if (source->Fetch()){
	    strncat(DValueC, szName, cDataLen);
		ret= DMS_SUCCESS;
	}
	else if (source->NoData())
		ret=  DMS_NO_DATA;
	else
		ret= DMS_ERROR;
    POP();
#undef DataCWithTMSIdx
}


