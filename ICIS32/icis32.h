/* ICIS32.h : Declares the data structures and all the functions of the ICIS32.DLL
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

/***************** Modification ******************************
August 2006 (AMP): DMS_STUDY  data structure changed
**************************************************************/


#ifndef _ICIS32_H_
#define _ICIS32_H_

// use for conditional compilation of the web service code
//#ifndef WEB
//#define WEB    
//#include "Library.h"
//#endif


//#pragma warning(disable: 4786)
#pragma warning(disable: 4230) //anachronism used : modifiers/qualifiers interspersed, qualifier ignored
#pragma warning(disable: 4518) //'LONG ' : storage-class or type specifier(s) unexpected here; ignored

#include "CODBC.h"


              

#ifdef MAKE_DLL
   #define DLL_INTERFACE __declspec( dllexport ) _stdcall
#else
   //#define DLL_INTERFACE __declspec( dllimport ) 
   #define DLL_INTERFACE __declspec(dllimport) __stdcall 
#endif


#define ICIS_MAX_SIZE      255      

//////////////////////////////////////////////////////////////////////////////
// GMS Specific
//////////////////////////////////////////////////////////////////////////////

#define GMS_MAX_DATESTR	      12	// 11 is used 
#define GMS_NVAL_SIZE         255	// 255 is used 
#define GMS_MAX_NVAL          256	// 128 is used 
#define GMS_MAX_ABBREV	      12	// 9 is used 
#define GMS_ABBR_LEN           8
#define GMS_MAX_AVAL          256 

#define GMS_MTYPE_SIZE	       4	// 3 is really used 
#define GMS_MCODE_SIZE	      12	// 9 is really used 
#define GMS_MNAME_SIZE	      52	// 51 is really used 
#define GMS_MGRP_SIZE	       4	// 3 is really used 

#define GMS_FTABLE_SIZE	      28	// 25 is used 
#define GMS_FTYPE_SIZE	      16	// 13 is used 
#define GMS_FCODE_SIZE	      52	// 50 is used 
#define GMS_FNAME_SIZE	      52	// 50 is used 
#define GMS_FFMT_SIZE	     256	// 255 is used 

#define GMS_LNAME_SIZE	     64     // (61 is used) 
#define GMS_LABBR_SIZE        12	//8 used

#define GMS_USTATUS_SIZE	   12	// 11 is used 
#define GMS_UTYPE_SIZE	      32	// 31 is used 
#define GMS_UNAME_SIZE	      32	// 31 is used 
#define GMS_UNAME_ASIZE	      31	// 31 is used 
#define GMS_UPSWD_SIZE	      12    //36 for the encrypted password    //	// 11 is used 

//List Data
#define GMS_LIST_NAME         48 
#define GMS_LIST_TYPE         8 
#define GMS_LIST_DESC         256    //AMP2008-05-23: used to be 80
#define GMS_LIST_DESIG        256
#define GMS_LIST_GRPNM        256
#define GMS_LIST_SOURC        256   //AMP2008-06-26: 48
#define GMS_LIST_ENTCD        48    //AMP2004-02-16: used to be 11

#define GMS_MAX_TABLE_NAME    16
#define GMS_MAX_FIELD_NAME    16
#define GMS_MAX_GROUP_NAME    20 // 16 is used


enum GMS_RETURNS{
   GMS_UNIQUE_CONSTRAINT_ERROR=-12,
   GMS_INVALID_DATA=-11,
   GMS_INVALID_DBSTRUCTURE=-10,
   GMS_INVALID_INSTALLATION=-9,
   GMS_READ_ONLY,
   GMS_MORE_DATA,
   GMS_NOT_FOUND,
   GMS_NAME_EXISTS,
   GMS_INVALID_USER,
   GMS_INVALID_PASSWORD,
   GMS_NO_ACCESS,
   GMS_ERROR,
   GMS_NO_DATA,
   GMS_SUCCESS
};


//Global Declaration

#define GMS_NOT_PREFFERED	  0
#define GMS_PREFFERED		  1
#define GMS_ABBR		   	  2

#define GMS_UNKDERIVMETH	 31
#define GMS_BACKCROSS		107
#define GMS_REPLACED		   101
#define GMS_CHANGED			102

#define FIND_FIRST            0
#define FIND_NEXT		      1
#define FIND_SPECIFIC        2
#define LOCAL              TRUE
#define CENTRAL           FALSE


#define BIND(col,field) {\
   central.Bind((col),(field));\
	local.Bind((col),(field));}
#define BINDS(col,field,size) {\
   central.Bind((col),(field),(size));\
	local.Bind((col),(field),(size));}
#define BINDPARAM(col,field) {\
   central.BindParam((col),(field));\
	local.BindParam((col),(field));}
#define BINDPARAMS(col,field,size) {\
   central.BindParam((col),(field),(size));\
	local.BindParam((col),(field),(size));}

#define RETURN_ON_ERROR(ret) if ((ret)!=GMS_SUCCESS) return (ret)





typedef struct TGMS_Germplasm{
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
}           GMS_Germplasm;

typedef struct TGMS_Name{
	LONG          nid;
    LONG          gid;
    LONG          ntype;
    LONG          nstat;
    LONG          nuid;
    LONG          nlocn;
    LONG          ndate;
    LONG          nref;
}           GMS_Name;

typedef struct TGMS_Attribute{
	LONG          aid;
    LONG          gid;
    LONG          atype;
    LONG          auid;
    LONG          alocn;
    LONG          adate;
    LONG          aref;
}           GMS_Attribute;

typedef struct TGMS_Method{
    LONG          mid;
    LONG          mref;
    LONG          mprgn;
    LONG          mfprg;
    LONG          mattr;
    CHAR          mtype[GMS_MTYPE_SIZE];
    CHAR          mcode[GMS_MCODE_SIZE];
    CHAR          mname[GMS_MNAME_SIZE];
    CHAR          mgrp[GMS_MGRP_SIZE];
    LONG          geneq;
    LONG          muid;
    LONG          lmid;
    LONG          mdate;
}           GMS_Method;


typedef struct TGMS_UDField{
    LONG          fldno;
    CHAR          ftable[GMS_FTABLE_SIZE];
    CHAR          ftype[GMS_FTYPE_SIZE];
    CHAR          fcode[GMS_FCODE_SIZE];
    CHAR          fname[GMS_FNAME_SIZE];
    CHAR          ffmt[GMS_FFMT_SIZE];
    LONG          lfldno;
    LONG          fuid;
    LONG          fdate;
	LONG          scaleid;
}           GMS_UDField;



typedef struct TGMS_User{
    LONG          userid;
    LONG          instalid;
    LONG          ustatus;
    LONG          uaccess;
    LONG          utype;
    CHAR          uname[GMS_UNAME_SIZE];
    CHAR          upswd[GMS_UPSWD_SIZE];
    LONG          personid;
    LONG          adate;
    LONG          cdate;
}           GMS_User;

typedef struct TGMS_Installation{
    LONG          instalid;
    LONG          admin;
    LONG          udate;
    LONG          ugid;
    LONG          ulocn;
    LONG          umethn;
    LONG          ufldno;
    LONG          urefno;
    LONG          upid;
}           GMS_Installation;

// List files Structure



typedef struct TGMS_ListName2{
    LONG          listid;
    CHAR          listname[GMS_LIST_NAME];
    LONG          listdate;
    CHAR          listtype[GMS_LIST_TYPE];
    LONG          listuid;
    CHAR          listdesc[GMS_LIST_DESC];
	LONG          liststatus,                 //added: Oct 14, 2004
	              lhierarchy;                 //added: Oct 14, 2004 
}           GMS_ListName;


	typedef struct TGMS_Location{
    LONG          locid;
    LONG          ltype;
    LONG          nllp;
    CHAR          lname[GMS_LNAME_SIZE];
    CHAR          labbr[GMS_LABBR_SIZE];
    LONG	         snl3id;
    LONG          snl2id;
    LONG          snl1id;
    LONG          cntryid;
    LONG          lrplce;
}           GMS_Location;

typedef struct TGMS_ListData2{
    LONG          listid,
                  gid,
                  lrecid;
    CHAR          entrycd[GMS_LIST_ENTCD],
                  source[GMS_LIST_SOURC];
    CHAR          desig[GMS_LIST_DESIG];
    CHAR          grpname[GMS_LIST_GRPNM];
	LONG          entryid;                    //added: Oct 14, 2004
}           GMS_ListData;

typedef struct TGMS_LDStatus{    ///ListData with Status
    LONG          listid,
                  gid,
                  lrecid;
    CHAR          entrycd[GMS_LIST_ENTCD],
                  source[GMS_LIST_SOURC];
    CHAR          desig[GMS_LIST_DESIG];
    CHAR          grpname[GMS_LIST_GRPNM];
	LONG          entryid;
	LONG          status;
}           GMS_LDStatus;


typedef struct TGMS_Change{
    LONG          cid,
                  crecord,
                  cfrom,
                  cto,
                  cdate,
                  ctime,
                  cuid,
                  cref;
    CHAR          ctable[GMS_MAX_TABLE_NAME],
                  cfield[GMS_MAX_FIELD_NAME],
                  cgroup[GMS_MAX_GROUP_NAME];
}           GMS_Changes;


typedef struct TGMS_BIBREFS{
    LONG          refid;
    LONG          pubtype;
    LONG          pubdate;
    CHAR          authors[100];
    CHAR          editors[100];
    CHAR          analyt[255];
    CHAR          monog[255];
    CHAR          series[255];
	CHAR          volume[10];
	CHAR          issue[10];
	CHAR          pagecol[25];
	CHAR          publish[50];
	CHAR          pubcity[30];
	CHAR          pubcntry[75];
}           GMS_Bibrefs;

extern "C"{

//////////////////////////////////////////////////////////////////////////////
// Database interface
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_openDatabase(LPCSTR szIniFile);
LONG DLL_INTERFACE G_openDatabase2(LPCSTR szIniFile, LPCSTR szUserName
            , LPCSTR szPassword, LONG *userid);
LONG DLL_INTERFACE GMS_openDatabase2(LPCSTR szIniFile, LPCSTR szUserName, LPCSTR szPassword, LONG *userid);
void DLL_INTERFACE GMS_closeDatabase(void);
void DLL_INTERFACE GMS_rollbackData(void);
void DLL_INTERFACE GMS_commitData(void);
void DLL_INTERFACE GMS_autoCommit(LONG autoCommit);
LONG DLL_INTERFACE GMS_hasCIDSID(void);


//////////////////////////////////////////////////////////////////////////////
// Germplasm
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_addGermplasm(GMS_Germplasm *data,GMS_Name *name,CHAR *szName
                   , LONG *progenitors);
LONG DLL_INTERFACE GMS_getGermplasmRecord(LONG id, GMS_Germplasm *data);
LONG DLL_INTERFACE GMS_getGermplasm(LONG id, GMS_Germplasm *data);
LONG DLL_INTERFACE GMS_getGermplasm2(LONG id, GMS_Germplasm *data, GMS_Name *name
            , LPSTR szName, LONG nszName);
LONG DLL_INTERFACE GMS_getGermplasm3(LONG id, GMS_Germplasm *data, GMS_Name *name
            , LPSTR szName, LONG nszName);
LONG DLL_INTERFACE GMS_setGermplasm(GMS_Germplasm *germ);
LONG DLL_INTERFACE GMS_deleteGermplasm(LONG gid);

LONG DLL_INTERFACE GMS_findGermplasm(GMS_Germplasm *data,LONG fSearchOption);
LONG DLL_INTERFACE GMS_findDescendant(LONG gid,GMS_Germplasm *data,LONG *pno
            , LONG fSearchOption);
LONG DLL_INTERFACE GMS_findCIDSID(LONG *CID, LONG *SID, LONG *GID, LONG fOpt);
LONG DLL_INTERFACE GMS_findUniqueID(LONG UID, LONG LGID, GMS_Germplasm *data);


LONG DLL_INTERFACE GMS_addProgenitorID(LONG gid,LONG pno,LONG pid);
LONG DLL_INTERFACE GMS_getProgenitorID(LONG gid,LONG pno,LONG *pid);
LONG DLL_INTERFACE GMS_setProgenitorID(LONG gid,LONG pno,LONG pid);
LONG DLL_INTERFACE GMS_deleteProgenitorID(LONG gid);
LONG DLL_INTERFACE GMS_deleteOneProgenitor(LONG gid, LONG pid);
LONG DLL_INTERFACE GMS_computeGenerationNo(LONG gid, LONG *c2, LONG *c1, LONG *c3, LONG *c4, LONG *LDM, LONG *CRM);
LONG DLL_INTERFACE GMS_getMgmntNeighbor(LONG mgid, GMS_Germplasm *data, GMS_Name *name
                   , LPSTR szName, LONG nszName, LONG fopt);
LONG DLL_INTERFACE GMS_getDerivativeNeighbor(LONG gid, LONG lngMStep,LONG lngNStep,LONG *idList,LONG *idLstSz, LONG blnDH);
LONG DLL_INTERFACE GMS_getDerivativeNeighbor2(LONG gid, LONG lngMStep,LONG lngNStep,LONG *idList, LONG *levelList, LONG *idLstSz, LONG blnDH);


//////////////////////////////////////////////////////////////////////////////
// Name
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_addName(GMS_Name *name,CHAR *szName,LONG stat);
LONG DLL_INTERFACE GMS_getName(GMS_Name *data,LPSTR szName,LONG nszName,LONG fSearchOption);
LONG DLL_INTERFACE GMS_setPreferredName(GMS_Name *name,CHAR *szName);
LONG DLL_INTERFACE GMS_setPreferredAbbr(GMS_Name *name,CHAR *szName);

LONG DLL_INTERFACE GMS_findName(CHAR *szSearchName, GMS_Name *name, GMS_Germplasm *germ
               , CHAR *szName, LONG nszName, LONG fSearchOption);
LONG DLL_INTERFACE GMS_findName2(CHAR *szSearchName, GMS_Name *name, GMS_Germplasm *germ
               , CHAR *szName, LONG nszName, LONG fSearchOption);
LONG DLL_INTERFACE GMS_findName3(CHAR *szSearchName, GMS_Name *name, GMS_Germplasm *germ
               , CHAR *szName, LONG nszName, LONG fSearchOption);

LONG DLL_INTERFACE GMS_listNames(LONG gid,CHAR* szBuffer, LONG nszBuffer);
LONG DLL_INTERFACE GMS_listNamesEx(LONG gid,CHAR* szBuffer, LONG nszBuffer,LPCSTR szDelimeter);
LONG DLL_INTERFACE GMS_standardName(CHAR* szInput, CHAR* szOutput,LONG nszOutput);
LONG DLL_INTERFACE GMS_setName(GMS_Name *name, CHAR *szName, LONG nszName);
LONG DLL_INTERFACE GMS_getNameRecord(GMS_Name *data, LPSTR szName, LONG nszName);
LONG DLL_INTERFACE GMS_getSoundEx(CHAR* szInput, CHAR* szOutput,LONG nszOutput);
LONG DLL_INTERFACE GMS_getNextSequence(LONG ntype, LPSTR szPrefix, LONG *nextSequence);
LONG DLL_INTERFACE GMS_getNextName(LONG ntype, LPSTR szPrefix, LPSTR szNextName);

//////////////////////////////////////////////////////////////////////////////
// User
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_getUser(GMS_User *data,LONG fSearchOption);
LONG DLL_INTERFACE GMS_addUser(GMS_User *data, LPCSTR ename, long security);

//////////////////////////////////////////////////////////////////////////////
// Installation
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_getInstallation(GMS_Installation *data,LPSTR szDesc,LONG nszDesc);
LONG DLL_INTERFACE GMS_getInstallationEx(GMS_Installation *data,LPSTR szDesc,LONG nszDesc,LONG fSearchOption);
LONG DLL_INTERFACE GMS_encryptPWD(LPCSTR szPwd, LPCSTR szEncrypt);
LONG DLL_INTERFACE GMS_getDMSStatus(LONG *dmsstat);
LONG DLL_INTERFACE GMS_setDMSStatus(LONG dmsstat);
LONG DLL_INTERFACE GMS_getCurrentInstln(void);


//////////////////////////////////////////////////////////////////////////////
// UDField
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_getUDField(GMS_UDField *data,LPSTR szDesc,LONG nszDesc,LONG fSearchOption);
LONG DLL_INTERFACE GMS_findUDField(GMS_UDField *data, LPSTR szDesc, LONG nszDesc, LONG fSearchOption);
LONG DLL_INTERFACE GMS_getUDField2(GMS_UDField *data, LPSTR szDesc, LONG nszDesc, LONG fSearchOption);
LONG DLL_INTERFACE GMS_addUDField(GMS_UDField &data,LPSTR szDesc, LONG nszDesc);

//////////////////////////////////////////////////////////////////////////////
// Methods
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_getMethod(GMS_Method *data, LPSTR szDesc, LONG nszDesc
               , LONG fSearchOption);

//////////////////////////////////////////////////////////////////////////////
// Atributs
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_getAttribute(GMS_Attribute *data, LPSTR szDesc, LONG nszDesc
                  , LONG fSearchOption);
LONG DLL_INTERFACE GMS_addAttribute(GMS_Attribute *data, CHAR *szAval);
LONG DLL_INTERFACE GMS_setAttribute(GMS_Attribute *data, CHAR *szAval);
LONG DLL_INTERFACE GMS_findAttribute(GMS_Attribute *data, LPSTR szDesc, LONG nszDesc
               , LONG fSearchOption);


//////////////////////////////////////////////////////////////////////////////
// Changes
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_addChanges(GMS_Changes *data, CHAR *szDesc);
LONG DLL_INTERFACE GMS_deleteChanges(GMS_Changes &data);
LONG DLL_INTERFACE GMS_getChanges(LONG cid,GMS_Changes *data, LPSTR szDesc, LONG nszDesc);
LONG DLL_INTERFACE GMS_findChanges(GMS_Changes *data, LPSTR szDesc, LONG nszDesc
               , LONG fSearchOption);
LONG DLL_INTERFACE GMS_findChanges2(GMS_Changes *data, LPSTR szDesc, LONG nszDesc
               , LONG fSearchOption);

//////////////////////////////////////////////////////////////////////////////
// Location
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_getLocation(GMS_Location *data, LONG fSearchOption);
LONG DLL_INTERFACE GMS_getLocation2(GMS_Location *data, LONG fSearchOption);
LONG DLL_INTERFACE GMS_getLocation3(GMS_Location *data, LONG fSearchOption);
LONG DLL_INTERFACE GMS_findLocation(GMS_Location *data, LONG fSearchOption);
LONG DLL_INTERFACE GMS_addLocation(GMS_Location *data);


//////////////////////////////////////////////////////////////////////////////
// List 
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_getListName(GMS_ListName *data,LONG fSearchOption);
LONG DLL_INTERFACE GMS_setListName(GMS_ListName *l);
LONG DLL_INTERFACE GMS_addListName(GMS_ListName *data);
LONG DLL_INTERFACE GMS_deleteListName(LONG lid);
LONG DLL_INTERFACE GMS_getListByStatus(GMS_ListName *data,LONG fSearchOption);

LONG DLL_INTERFACE GMS_getListData(GMS_ListData *data,LONG fSearchOption);
LONG DLL_INTERFACE GMS_getListData2(GMS_LDStatus *data,LONG fSearchOption);   //Retrieves list data record with status info
LONG DLL_INTERFACE GMS_getListDataByDesig(GMS_ListData *data,LONG fSearchOption);  //Retrieves list data record for the given LISTID and DESIG
LONG DLL_INTERFACE GMS_addListData(GMS_ListData *data);
LONG DLL_INTERFACE GMS_addListText(GMS_ListData *data);
LONG DLL_INTERFACE GMS_setListData(GMS_ListData *data);
LONG DLL_INTERFACE GMS_deleteListData(LONG lid,LONG lrecid);
LONG DLL_INTERFACE GMS_countListEntry(long listid);


LONG DLL_INTERFACE GMS_findListData(GMS_ListData &data,LONG fSearchOption);
LONG DLL_INTERFACE GMS_findListName(CHAR *szName,GMS_ListName &data,LONG fSearchOption);


LONG DLL_INTERFACE GMS_getMaxEntryID(long listid);

//////////////////////////////////////////////////////////////////////////////
// Misc routines
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_generateTree(LONG gid, LONG level,CHAR *szBuffer, LONG len,LONG *idList, LONG nidList);
LONG DLL_INTERFACE GMS_crossExpansion(LONG gid, LONG level,LONG ntype,CHAR *szBuffer, LONG len,LONG *idList, LONG nidList);
LONG DLL_INTERFACE GMS_generateCrossName(LONG gid1, LONG gid2, LONG methn,CHAR* szBuffer, LONG bufsize);
LONG DLL_INTERFACE GMS_treeBuild(LONG gid,LONG level);
LONG DLL_INTERFACE GMS_treeDescendant(GMS_Germplasm *germ,LONG fWalk);
LONG DLL_INTERFACE GMS_treeProgenitor(LONG index,GMS_Germplasm *germ,LONG fWalk);

//////////////////////////////////////////////////////////////////////////////
// Date routines
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_validDate(LONG year,LONG month,LONG day);
LONG DLL_INTERFACE GMS_expandDate(LONG date,LONG *year ,LONG *month ,LONG *day);
LONG DLL_INTERFACE GMS_strToDate(CHAR *szDate);
LONG DLL_INTERFACE GMS_date(LONG year,LONG month,LONG day);
LONG DLL_INTERFACE GMS_dateToStr(LONG date,CHAR *szDate);
LONG DLL_INTERFACE GMS_getTime(void);
LONG DLL_INTERFACE GMS_getDate(void);



//////////////////////////////////////////////////////////////////////////////
// Bibrefs routines
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_getBibrefs(GMS_Bibrefs *data,LONG fSearchOption);
LONG DLL_INTERFACE GMS_addBibrefs(GMS_Bibrefs *data);
LONG DLL_INTERFACE GMS_setBibrefs(GMS_Bibrefs *data);



//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// DMS Specific
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
#define DMS_SUCCESS                   (1)
#define DMS_NO_DATA                   (0)
#define DMS_ERROR                    (-1)
#define DMS_EXIST                     (-5)
#define DMS_NOT_FOUND                 (-6)
#define DMS_INVALID_ID                (-9)

#define DMS_MAXNOFACTOR               50

#define   DMS_STUDY_NAME          52
#define   DMS_STUDY_TITLE         256
#define   DMS_STUDY_STYPE         4
#define   DMS_FACTOR_NAME         52
#define   DMS_LEVEL_TYPE          4
#define   DMS_DATA_VALUE          256
#define   DMS_VARIATE_NAME        52
#define   DMS_VARIATE_TYPE        4
#define   DMS_DATA_TYPE           4
#define   DMS_TRAIT_NAME          52
#define   DMS_TRAIT_ABBR          8
#define   DMS_SCALE_NAME          52
#define   DMS_SCALE_TYPE          4
#define   DMS_METHOD_NAME         52
#define   DMS_METHOD_ABBR         8
#define   DMS_ATTR_TABLE          12
#define   DMS_MAX_STR             1000
#define   DMS_SCALE_VALUE         24
#define   DMS_SCALE_DESC          256
#define   DMS_SCALE_SQL           252 

#define DMS_SCALETAB_PARAM 256
#define DMS_SCALETAB_TYPE 4

#define  DMS_EQ   0
#define	DMS_GT   1
#define	DMS_LT   2
#define	DMS_GTE  3 
#define	DMS_LTE  4
#define DMS_ASC  1
#define DMS_DESC 2

typedef struct {
	LONG STUDYID;
	CHAR SNAME[DMS_STUDY_NAME];
	LONG PMKEY;
	CHAR TITLE[DMS_STUDY_TITLE];
	LONG INVESTID;
	CHAR STYPE[DMS_STUDY_STYPE];
	LONG SDATE;
	LONG EDATE;
	LONG USERID;
	LONG SSTATUS;
	LONG SHIERARCHY;
}  DMS_STUDY;

typedef struct {
	LONG LABELID;
	LONG FACTORID;
	CHAR FNAME[DMS_FACTOR_NAME];
	LONG STUDYID;
	LONG TRAITID;
	LONG SCALEID;
	LONG TMETHID;
	CHAR LTYPE[DMS_LEVEL_TYPE];
}   DMS_FACTOR;

typedef struct {
	LONG VARIATID;
	CHAR VNAME[DMS_VARIATE_NAME];
	CHAR VTYPE[DMS_VARIATE_TYPE];
	LONG STUDYID;
	LONG TRAITID;
	LONG SCALEID;
	LONG TMETHID;
	CHAR DTYPE[DMS_DATA_TYPE];
}   DMS_VARIATE;

typedef struct {
	LONG LABELID;
	LONG FACTORID;
	LONG LEVELNO;
	double LVALUE;
}   DMS_LEVELN;

typedef struct {
	LONG LABELID;
	LONG FACTORID;
	LONG LEVELNO;
	CHAR LVALUE[DMS_DATA_VALUE];
}   DMS_LEVELC;


//revised in ICIS 5.5
typedef struct {
	LONG LEVELNO;
    LONG FACTORID; 
}   DMS_LEVELNO;

typedef struct {
	LONG OUNITID;
	LONG VARIATID;
	double DVALUE;
}   DMS_DATAN;

typedef struct {
	LONG OUNITID;
	LONG VARIATID;
	CHAR DVALUE[DMS_DATA_VALUE];
}   DMS_DATAC;

//revised in ICIS 5.5.
typedef struct {
	LONG OUNITID;
	LONG EFFECTID;
}   DMS_OBSUNIT;

typedef struct {
	LONG OUNITID;
	LONG FACTORID;
	LONG LEVELNO;
	LONG REPRESNO;
}   DMS_OINDEX;


typedef struct {
	LONG EFFECTID;
	LONG FACTORID;
	LONG REPRESNO;
} DMS_EFFECT;

typedef struct {
	LONG REPRESNO;
	LONG VARIATEID;
} DMS_VEFFECT;



typedef struct {
	LONG STUDYID;
	LONG REPRESNO;
	LONG LABELID;
	LONG FACTORID;
	CHAR FNAME[DMS_FACTOR_NAME];
	LONG TRAITID;
	LONG SCALEID;
	LONG TMETHID;
	CHAR LTYPE[DMS_LEVEL_TYPE];
} DMS_SRFACTOR;

typedef struct {
	LONG STUDYID;
	LONG REPRESNO;
	LONG VARIATID;
	CHAR VNAME[DMS_VARIATE_NAME];
	CHAR VTYPE[DMS_VARIATE_TYPE];
	LONG TRAITID;
	LONG SCALEID;
	LONG TMETHID;
	CHAR DTYPE[DMS_DATA_TYPE];
} DMS_SRVARIATE;

typedef struct {
	LONG REPRESNO;
	LONG FACTORID;
	LONG LABELID;
	CHAR LVALUE[DMS_DATA_VALUE];
} DMS_CONDLEVEL;

typedef struct {
	LONG REPRESNO;
	LONG VARIATID;
	CHAR DVALUE[DMS_DATA_VALUE];
} DMS_CONSDATA;

typedef struct {
	LONG REPRESNO;
	LONG OUNITID;
	LONG LABELID;
	LONG FACTORID;
	LONG LEVELNO;
	CHAR LVALUE[DMS_DATA_VALUE];
} DMS_OUNITLEVEL;

typedef struct {
	LONG REPRESNO;
	LONG OUNITID;
	LONG VARIATID;
	CHAR DVALUE[DMS_DATA_VALUE];
} DMS_OUNITDATA;

typedef struct {
    LONG            TRAITID;
    UCHAR           TRNAME[DMS_TRAIT_NAME];
    UCHAR           TRABBR[DMS_TRAIT_ABBR];
}         DMS_TRAIT_old;

typedef struct {
	LONG            TID;  
    LONG            TRAITID;
    UCHAR           TRNAME[DMS_TRAIT_NAME];
    UCHAR           TRABBR[DMS_TRAIT_ABBR];
	LONG            TNSTAT;
	LONG            SCALEID;
	LONG            TMETHID;
}         DMS_TRAIT;


typedef struct {
	LONG   SCALEID;
	LONG   TRAITID;
	UCHAR  SCNAME[DMS_SCALE_NAME];
	UCHAR  SCTYPE[DMS_SCALE_TYPE];
}  DMS_SCALE;


typedef struct {
	LONG   SCALEID;
	DOUBLE   SLEVEL;
	DOUBLE   ELEVEL;
}  DMS_SCALECON;

typedef struct {
	LONG   SCALEID;
	CHAR   VALUE[DMS_SCALE_VALUE];
	CHAR   VALDESC[DMS_SCALE_DESC];
}  DMS_SCALEDIS;

typedef struct {
	LONG   SCALEID;
	CHAR   SQLTEXT[DMS_SCALE_SQL];
	CHAR   MODULE[8];
}  DMS_SCALETAB;


/**** added by WVC *******/
typedef struct {
	LONG STUDYID;
	LONG REPRESNO;
	CHAR FNAME[DMS_FACTOR_NAME];
} DMS_SREPRES;

typedef struct {
	CHAR FNAME[DMS_FACTOR_NAME];
	LONG TRAITID;
	LONG SCALEID;
	LONG TMETHID;
} DMS_UFACTOR;

typedef struct {
	CHAR VNAME[DMS_VARIATE_NAME];
	LONG TRAITID;
	LONG SCALEID;
	LONG TMETHID;
} DMS_UVARIATE;

typedef struct {
	CHAR TRNAME[DMS_TRAIT_NAME];
	CHAR SCNAME[DMS_SCALE_NAME];
	CHAR TMNAME[DMS_METHOD_NAME];
} DMS_UTRAIT;


/**** a revised SQLTAB with parameter to the SQL statement***/
typedef struct {
	LONG   SCALEID;
	CHAR   SQLTEXT[DMS_SCALE_SQL];
	CHAR   MODULE[8];
	CHAR   PARAMETER[DMS_SCALETAB_PARAM];
	CHAR   TYPE[DMS_SCALETAB_TYPE];
}  DMS_SCALETAB2;


typedef struct {
   LONG            TMETHID;
	LONG            TRAITID;
   UCHAR           TMNAME[DMS_METHOD_NAME];
   UCHAR           TMABBR[DMS_METHOD_ABBR];
    //UCHAR            tmdesc;
}         DMS_TMETHOD;

typedef struct {
	LONG       DMSATID;
	LONG       DMSATYPE;
	CHAR       DMSATAB[DMS_ATTR_TABLE];
	LONG       DMSATREC;
}         DMS_DMSATTR;

typedef struct {
	LONG         ounitid;
	LONG         traitid;
	LONG         scaleid;
	LONG         tmethid;
	double       nvalue;
}         DMS_NumVALUE;

typedef struct {
	LONG         ounitid;
	LONG         traitid;
	LONG         scaleid;
	LONG         tmethid;
	char        cvalue[DMS_DATA_VALUE];
}         DMS_CharVALUE;

struct source {
	long facid;
	long levelno;
}   ;



typedef struct {
	LONG            TID;
    LONG            TRAITID;
    UCHAR           TRNAME[DMS_TRAIT_NAME];
    UCHAR           TRABBR[DMS_TRAIT_ABBR];
	LONG            TRNSTAT;
	LONG            SCALEID;
	LONG            TMETHID;
}         DMS_TRAIT4;



//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// SUPPORTING DATA
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

typedef struct {
	LONG       PERSONID;
    CHAR       FNAME [24];
	CHAR       LNAME[52];
	CHAR       IONAME[16];
	LONG       INSTITID;
	CHAR       PTITLE[28];
	CHAR       PONAME[52];
	LONG       PLANGU;
	CHAR       PPHONE[24];
	CHAR       PEXTENT[24];
	CHAR       PFAX[24];
	CHAR       PEMAIL[44];
	LONG       PROLE;
	LONG       SPERSON;
	LONG       EPERSON;
	LONG       PSTATUS;
	CHAR       CONTACT[256];
}         SUP_PERSONS;


typedef struct {
	LONG       INSTITID;
	LONG       PINSID;
	CHAR       INSNAME[152];
	CHAR       INSACR[24];
	LONG       INSTYPE;
	CHAR       STREET[128];
	CHAR       POSTBOX[28];
	CHAR       CITY[36];
	LONG       STATEID;
	CHAR       CPOSTAL[12];
	LONG       CNTRYID;
    CHAR       APHONE[28];
	CHAR       AFAX[28];
	CHAR       AEMAIL[44];
	CHAR       WEBURL[64];
	LONG       SINS;
	LONG       EINS;
	LONG       ICHANGE;
}   SUP_INSTITUT;


#define   UI_CLASSNAME        52


typedef struct {
	LONG UITABID;
	CHAR UICLASS[UI_CLASSNAME];
}  UI_TABLE;

/** Original
typedef struct {
	LONG UIID;
	LONG UIFID;
	LONG UITABID;
	LONG UILWIDTH;
	LONG UIWIDTH;
	LONG UI_Y;
	LONG UI_X;
	CHAR FLDTYPE[4];
	LONG STUDYID;
	LONG FLDDEF;
	CHAR UIACCESS[4];
}  UI_INTERFACE;
**/


typedef struct {
	LONG UIID;
	LONG UIFID;
	LONG UITABID;
	LONG UILWIDTH;
	LONG UIWIDTH;
	LONG UI_Y;
	LONG UI_X;
	CHAR FLDTYPE[4];
	CHAR FLDDEF[DMS_VARIATE_NAME];
	CHAR UIACCESS[4];
}  UI_INTERFACE;



typedef struct {
	CHAR UICLASS[UI_CLASSNAME];
	LONG TABID;
	LONG NO_ITEMS;
}  UI_CLASS;




//////////////////////////////////////////////////////////////////////////////
// Database interface
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_openDatabase(LPCSTR szIniFile);
void DLL_INTERFACE DMS_closeDatabase(void);
void DLL_INTERFACE DMS_rollbackData(void);
void DLL_INTERFACE DMS_commitData(void);
void DLL_INTERFACE DMS_autoCommit(BOOL autoCommit);


//////////////////////////////////////////////////////////////////////////////
// Study interface
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_findStudy(CHAR *szSearchName, DMS_STUDY *data, LONG fSearchOption);
LONG DLL_INTERFACE DMS_addStudy(DMS_STUDY  *study, CHAR *szDesc);
LONG DLL_INTERFACE DMS_deleteStudy(long styid);
LONG DLL_INTERFACE DMS_getStudy(DMS_STUDY* Study, UCHAR* szMDesc, LONG cMDesc, int fOpt);
LONG DLL_INTERFACE DMS_setStudy(DMS_STUDY  *study, CHAR *szDesc); 
LONG DLL_INTERFACE DMS_setStudyStatParent(long studyid, long status, long hierarchy);
LONG DLL_INTERFACE DMS_executeSQLText(CHAR *strSQL, CHAR *fname, LONG *noCol );
LONG DLL_INTERFACE DMS_getStudy3(DMS_STUDY* Study, UCHAR* szMDesc, LONG cMDesc, int fOpt);
LONG DLL_INTERFACE DMS_findStudyOnly(CHAR *szSearchName, DMS_STUDY *data, LONG fSearchOption);
LONG DLL_INTERFACE DMS_executeSQLValue(CHAR *strSQL, CHAR *value,  LONG lnVal);
LONG DLL_INTERFACE DMS_executeBindSQLValue(CHAR *strSQL, CHAR *value,  LONG lnVal, long param1, LONG fopt);
LONG DLL_INTERFACE GMS_executeSQLValue(CHAR *strSQL, CHAR *value,  LONG lnVal);


//////////////////////////////////////////////////////////////////////////////
// Factor interface
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_addFactor(DMS_FACTOR* Fac);
LONG DLL_INTERFACE DMS_findFactor(CHAR *szSearchName, DMS_FACTOR *data, LONG fSearchOption);
LONG DLL_INTERFACE DMS_findFactorDistinct(CHAR *szSearchName, DMS_FACTOR *data, LONG fSearchOption);
LONG DLL_INTERFACE DMS_getFactor(DMS_FACTOR *data,  int fOpt);
LONG DLL_INTERFACE DMS_getFactorLabel(DMS_FACTOR *data,  int fOpt);
LONG DLL_INTERFACE DMS_findFactorTMS(DMS_FACTOR *data, LONG fSearchOption);


//////////////////////////////////////////////////////////////////////////////
// EFFECT and Representation (Dataset) interface
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getFactorEffect(DMS_EFFECT *data, BOOL fSearchOption);
LONG DLL_INTERFACE DMS_addEffect(LONG *lstfactor, LONG nfactor, LONG *represno, LONG *effectid);
LONG DLL_INTERFACE DMS_addVEffect(DMS_VEFFECT *data);
LONG DLL_INTERFACE DMS_existEffect(LONG *lstfactor, LONG nfactor, LONG *represno, LONG *effectid);
LONG DLL_INTERFACE DMS_getEffectCount(LONG nfactor,LONG *represno, LONG *effectid,  BOOL fSearchOption);
LONG DLL_INTERFACE DMS_getEffect(LONG studyid,LONG* effectid, LONG* repres, BOOL fSearchOption);
LONG DLL_INTERFACE DMS_getEffectFactor(DMS_EFFECT *data, BOOL fSearchOption);
LONG DLL_INTERFACE DMS_getEffectRecord(LONG studyid, DMS_EFFECT *data, BOOL fSearchOption);
LONG DLL_INTERFACE DMS_findStEffect(long studyid,long effectid);
LONG DLL_INTERFACE DMS_getEffectRecordV(LONG studyid, DMS_EFFECT *data, BOOL fSearchOption);
LONG DLL_INTERFACE DMS_getEffectOunit(LONG effectid, LONG *ounitid, BOOL fSearchOption);
LONG DLL_INTERFACE DMS_describeEffectOunit(LONG ounitid, LONG effectid, struct source *dsource, LONG *nsrc,LONG fopt );
LONG DLL_INTERFACE DMS_getVariateOfRepres(DMS_EFFECT *data, BOOL fSearchOption);
LONG DLL_INTERFACE DMS_getVEffectRecord(DMS_VEFFECT *data, BOOL fSearchOption);
LONG DLL_INTERFACE DMS_getSRFactor(DMS_SRFACTOR *data, int fOpt);
LONG DLL_INTERFACE DMS_deleteDataset(long respresno);
LONG DLL_INTERFACE DMS_getRepresName(LONG id, char *name);
LONG DLL_INTERFACE DMS_setRepresName(LONG id, char *name);
LONG DLL_INTERFACE DMS_setEffectName(LONG id, char *name);


//////////////////////////////////////////////////////////////////////////////
// Variate interface
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_addVariate(DMS_VARIATE* data);
LONG DLL_INTERFACE DMS_findVariate(CHAR *szSearchName, DMS_VARIATE *data, LONG fSearchOption);
LONG DLL_INTERFACE DMS_getVariate(DMS_VARIATE *data,  int fOpt);
LONG DLL_INTERFACE DMS_addAttr( DMS_DMSATTR* Dtr, CHAR* szDesc);
LONG DLL_INTERFACE DMS_getDMSAttr(DMS_DMSATTR *data, CHAR* szDesc, LONG nszDesc, int fOpt);
LONG DLL_INTERFACE DMS_findVariateTMS(DMS_VARIATE *data, LONG fSearchOption);
LONG DLL_INTERFACE DMS_getSRVariate(DMS_SRVARIATE *data, int fOpt);
LONG DLL_INTERFACE DMS_deleteDMSATTR(LONG atype,LONG atrec);


//////////////////////////////////////////////////////////////////////////////
// Data/OUNIT interface
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_getOunitEq(long *ounitid, long studyid, long nsource, long fopt);
//LONG DLL_INTERFACE DMS_getObsunit(LONG studyid, DMS_OBSUNIT *dat, LONG fSearchOption);
LONG DLL_INTERFACE DMS_getDataN(DMS_DATAN * dat);
LONG DLL_INTERFACE DMS_getDataC(DMS_DATAC * dat);
LONG DLL_INTERFACE DMS_addDataC(DMS_DATAC * dat);
LONG DLL_INTERFACE DMS_addDataN(DMS_DATAN * dat);
LONG DLL_INTERFACE DMS_addOindex(DMS_OINDEX * dat);
LONG DLL_INTERFACE DMS_minOunit(LONG *ounitid);
LONG DLL_INTERFACE DMS_updateDataC(DMS_DATAC * dat);
LONG DLL_INTERFACE DMS_updateDataN(DMS_DATAN * dat);
LONG DLL_INTERFACE DMS_deleteDataC(LONG ounit, LONG variatid);
LONG DLL_INTERFACE DMS_deleteDataN(LONG ounit, LONG variatid);
LONG DLL_INTERFACE DMS_existCombination(long studyid, struct source *dsource, long *ounitid, LONG nsource);
LONG DLL_INTERFACE DMS_describeOunit(LONG ounitid, struct source *dsource, LONG *nsrc,LONG fopt );
LONG DLL_INTERFACE DMS_existCombinationEffect(long represno, struct source *dsource, long *ounitid, LONG nsource);
LONG DLL_INTERFACE DMS_getOunitDataC(DMS_DATAC * dat , LONG fSearchOption);
LONG DLL_INTERFACE DMS_getOIndex(LONG styid, DMS_OINDEX *dat, LONG fSearchOption);

LONG DLL_INTERFACE DMS_addDataC2(DMS_DATAC * dat);
LONG DLL_INTERFACE DMS_addDataN2(DMS_DATAN * dat);
LONG DLL_INTERFACE DMS_addObsUnit(DMS_OBSUNIT* dat);

LONG DLL_INTERFACE DMS_getDataNOfFactorN(LONG labelid, LONG variateid, double lvalue,  double *datan);
LONG DLL_INTERFACE DMS_getDataCOfFactorN(LONG labelid, LONG variateid, double lvalue,  CHAR *datac);

LONG DLL_INTERFACE DMS_getOunitData(DMS_OUNITDATA *data, int fOpt);
LONG DLL_INTERFACE DMS_getConsData(DMS_CONSDATA *data, int fOpt);


//////////////////////////////////////////////////////////////////////////////
// Level interface
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_addLevelC(DMS_LEVELC* Level);
LONG DLL_INTERFACE DMS_addLevelN(DMS_LEVELN* Level);
LONG DLL_INTERFACE DMS_findLevelN(DMS_LEVELN* Level);
LONG DLL_INTERFACE DMS_findLevelC(DMS_LEVELC* Level);
LONG DLL_INTERFACE DMS_findLFLevel(DMS_LEVELC *data);  
LONG DLL_INTERFACE DMS_findPFLevel(DMS_LEVELC *data);
LONG DLL_INTERFACE DMS_findLabelN(DMS_LEVELN* Level);
LONG DLL_INTERFACE DMS_findLabelC(DMS_LEVELC* Level);
LONG DLL_INTERFACE DMS_getLevelC(DMS_LEVELC* Level);
LONG DLL_INTERFACE DMS_getLevelN(DMS_LEVELN* Level);
LONG DLL_INTERFACE DMS_getLabelC(DMS_LEVELC* Level, LONG fopt);
LONG DLL_INTERFACE DMS_getLabelN(DMS_LEVELN* Level, LONG fopt);
LONG DLL_INTERFACE DMS_getAllLevelN(DMS_LEVELN* Level, LONG fopt);
LONG DLL_INTERFACE DMS_getAllLevelC(DMS_LEVELC* Level, LONG fopt);
LONG DLL_INTERFACE DMS_extractOunitByLNVal(DMS_OINDEX *Ounit, long labelid, double *lvalue,  int oprtr, int fopt);
LONG DLL_INTERFACE DMS_extractOunitByLCVal(DMS_OINDEX *Ounit, long labelid, char *lvalue,  int oprtr, int fopt);
LONG DLL_INTERFACE DMS_getAllLabelN(DMS_LEVELN* Level, LONG fopt);
LONG DLL_INTERFACE DMS_getAllLabelC(DMS_LEVELC* Level, LONG fopt);
LONG DLL_INTERFACE DMS_getAllLabel(DMS_LEVELC* Level, LONG fopt);

LONG DLL_INTERFACE DMS_addLevelN2(DMS_LEVELN* Level);
LONG DLL_INTERFACE DMS_addLevelC2(DMS_LEVELC* Level);


LONG DLL_INTERFACE DMS_getCondLevel(DMS_CONDLEVEL *data, int fOpt);
LONG DLL_INTERFACE DMS_getOunitLevel(DMS_OUNITLEVEL *data, int fOpt);

//////////////////////////////////////////////////////////////////////////////
// Search interface
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_searchLevelN(long Trait, long Scale, long Method, long *Ounit, double *FlevelN, long fopt);
LONG DLL_INTERFACE DMS_searchLevelC(long Trait, long Scale, long Method, long *Ounit, char *FlevelC, long cLevelLen, long fopt);
LONG DLL_INTERFACE DMS_searchDataN(long Trait, long Scale, long Method, long *Ounit, double *DValueN, long fopt);
LONG DLL_INTERFACE DMS_searchDataC(long Trait, long Scale, long Method, long *Ounit, char *DValueC, long cDataLen, long fopt);
LONG DLL_INTERFACE DMS_extractOunitLN(long studyid,double dblValue, DMS_NumVALUE *NumValue,int oprtr,int fopt);
LONG DLL_INTERFACE DMS_extractOunitLC(long studyid, char* chrVal, DMS_CharVALUE *CharValue,int fopt);
LONG DLL_INTERFACE DMS_extractOunitDC(long studyid,char* chrVal,DMS_CharVALUE *CharValue,int fopt);
LONG DLL_INTERFACE DMS_extractOunitDN(long studyid,double dblVal,DMS_NumVALUE *NumValue,int oprtr,int fopt);
LONG DLL_INTERFACE DMS_findStOunit(DMS_CharVALUE *CharValue,int fopt);
LONG DLL_INTERFACE DMS_getOunitLevelN(DMS_LEVELN *LevN, long *ounitid, long fopt);
LONG DLL_INTERFACE DMS_getOunitLevelC(DMS_LEVELC *LevC, long *ounitid, long fopt);

LONG DLL_INTERFACE DMS_searchDataNOfOunit(DMS_NumVALUE *data);
LONG DLL_INTERFACE DMS_searchDataCOfOunit(DMS_CharVALUE *data);
LONG DLL_INTERFACE DMS_searchLevelNOfOunit(DMS_NumVALUE *data);
LONG DLL_INTERFACE DMS_searchLevelCOfOunit(DMS_CharVALUE *data);


LONG DLL_INTERFACE DMS_defineOunitDC(DMS_CharVALUE *VariateC, int fopt);
LONG DLL_INTERFACE DMS_defineOunitDN(DMS_NumVALUE *VariateN,int fopt);
LONG DLL_INTERFACE DMS_defineOunitLC(DMS_CharVALUE *FactorC, int fopt);
LONG DLL_INTERFACE DMS_defineOunitLN(DMS_NumVALUE *FactorN,int fopt);

LONG DLL_INTERFACE DMS_extractOunitByLNVal(DMS_OINDEX *Ounit, long labelid, double *lvalue,  int oprtr, int fopt);
LONG DLL_INTERFACE DMS_extractOunitByLNVal(DMS_OINDEX *Ounit, long labelid, double *lvalue,  int oprtr, int fopt);
LONG DLL_INTERFACE DMS_getAllLabelC(DMS_LEVELC* Level, LONG fopt);
LONG DLL_INTERFACE DMS_getAllLabelN(DMS_LEVELN* Level, LONG fopt);
LONG DLL_INTERFACE DMS_getScaleCon(DMS_SCALECON* Scales, long fOpt);
LONG DLL_INTERFACE DMS_getScaleDis(DMS_SCALEDIS* Scales, long fOpt);
LONG DLL_INTERFACE DMS_getScaleDis2(DMS_SCALEDIS* Scales, long fOpt);
LONG DLL_INTERFACE DMS_getScaleTab(LONG scaleid, CHAR *fname, LONG *noCol);









//////////////////////////////////////////////////////////////////////////////
// Trait interface
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE DMS_addTrait( DMS_TRAIT* Tra, CHAR* szPTra);
LONG DLL_INTERFACE DMS_addScale( DMS_SCALE* Scal);
LONG DLL_INTERFACE DMS_addTmethod( DMS_TMETHOD* Tme, CHAR* szPTme);
LONG DLL_INTERFACE DMS_getTrait(DMS_TRAIT* Traits, UCHAR* szMDesc, LONG cMDesc, int fOpt);
LONG DLL_INTERFACE DMS_getScale(DMS_SCALE* Scales, long fOpt);
LONG DLL_INTERFACE DMS_getTmethod(DMS_TMETHOD* Tmethods, UCHAR* szMDesc, LONG cMDesc, int fOpt);
LONG DLL_INTERFACE DMS_getScale2(DMS_SCALE* Scales, long fOpt);
LONG DLL_INTERFACE DMS_getTmethod2(DMS_TMETHOD* Tmethods, UCHAR* szMDesc, LONG cMDesc, long fopt);
LONG DLL_INTERFACE DMS_findTrait( CHAR *szName, DMS_TRAIT *Trait, LONG fopt);
LONG DLL_INTERFACE DMS_findScale( CHAR *szName,DMS_SCALE *SCALE, LONG fopt);
LONG DLL_INTERFACE DMS_findTmethod( CHAR *szName, DMS_TMETHOD *Tmeth, LONG fopt);
LONG DLL_INTERFACE DMS_getPrefTrait(DMS_TRAIT* Traits, UCHAR* szMDesc, LONG cMDesc, int fOpt);
LONG DLL_INTERFACE DMS_getScaleCon(DMS_SCALECON* Scales, long fOpt);
LONG DLL_INTERFACE DMS_getScaleDis(DMS_SCALEDIS* Scales, long fOpt);
LONG DLL_INTERFACE DMS_getScaleCon(DMS_SCALECON* Scales, long fOpt);
LONG DLL_INTERFACE DMS_getScaleDis2(DMS_SCALEDIS* Scales, long fOpt);
LONG DLL_INTERFACE DMS_getScaleTab(LONG scaleid, CHAR *fname, LONG *noCol);
LONG DLL_INTERFACE DMS_getScaleTab2(LONG scaleid, CHAR *value, LONG nVal);


/********* Added by WVC *********************/

LONG DLL_INTERFACE DMS_getSRepres(DMS_SREPRES *data, int fOpt);
LONG DLL_INTERFACE DMS_getUFactor(DMS_UFACTOR *data, int fOpt);
LONG DLL_INTERFACE DMS_getUVariate(DMS_UVARIATE *data, int fOpt);
LONG DLL_INTERFACE DMS_getUTrait(DMS_UTRAIT *data, int fOpt);
LONG DLL_INTERFACE DMS_getSFactor(DMS_SRFACTOR *data, int fOpt);
LONG DLL_INTERFACE DMS_getSVariate(DMS_SRVARIATE *data, int fOpt);




////////////////////////////////////////////////////////////////////////////////////////
/////// Workbook DLL function
///////////////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE long RetrieveDes(long lStdID, long lRepNo, char *zColTyp, LPSAFEARRAY FAR *zColNam, long *lVarID, \
							 long *lCndCnt, long *lFacCnt, long *lCnsCnt, long *lVarCnt);
LONG DLL_INTERFACE long RetrieveCol(long lStdID, long lRepNo, LPSAFEARRAY FAR *zColNam, long lFacCnt, \
							 long lVarCnt, long *lVarID, long *lObsCnt, long lWtHis);



////////////////////////////////////////////////////////////////////////////////////////
/////// SUPPORTING TABLE
///////////////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE SUP_addPerson( SUP_PERSONS* person);
LONG DLL_INTERFACE SUP_addInstitute( SUP_INSTITUT* data);
LONG DLL_INTERFACE SUP_getPerson(SUP_PERSONS* data,long fOpt);
LONG DLL_INTERFACE SUP_getPersonInst(long institid, char fname[24], char lname[52], char insacr[24],char title[28], long* personid, long fOpt);
LONG DLL_INTERFACE SUP_getInstitutePrnt(SUP_INSTITUT* instit, long fOpt);


//////////////////////////////////////////////////////////////////////////////////////
//////  USER INTERFACE
/////   Date Created: November 4, 2003
//////////////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE UI_getUIField( UI_INTERFACE *data , LONG fOpt);
LONG DLL_INTERFACE UI_getUIHeader( UI_INTERFACE *data,  LONG fOpt);
LONG DLL_INTERFACE UI_getUserInterface(UI_CLASS *uiclass, LONG uiid, LONG FOpt);





#define IMS_SUCCESS                   (1)
#define IMS_NO_DATA                   (0)
#define IMS_ERROR                    (-1)
#define IMS_EXIST                     (-5)
#define IMS_NOT_FOUND                 (-6)
#define IMS_INVALID_ID                (-9)
#define IMS_SOURCETYPE               12
#define IMS_ETYPE                    16
#define IMS_GROUP_PREFIX             52  //50 is used in the table
#define IMS_NAME_LENGTH               52  //50 is used in the table


typedef struct {
	LONG LOTID;
	LONG USERID;
	CHAR ETYPE[IMS_ETYPE];
	LONG EID;
	LONG LOCID;
	LONG SCALEID;
	LONG STATUS;
	LONG  SOURCE;
	CHAR COMMENTS[256];
}   IMS_LOT;


/*typedef struct {
	LONG TRNID;
	LONG TRNUSER;
	LONG LOTID;
	LONG TRNDATE;
	LONG TRNSTAT;
	LONG CMTDATA;
	CHAR COMMENTS[256];
	double TRNQTY;
}   IMS_TRANSACTION;
*/

typedef struct {
	LONG TRNID;
	LONG TRNUSER;
	LONG LOTID;
	LONG TRNDATE;
	LONG TRNSTAT;
	LONG CMTDATA;
	CHAR COMMENTS[256];
	double TRNQTY;
	double PREVAMOUNT;
	CHAR SOURCETYPE[IMS_SOURCETYPE];    // STUDY, LIST
	LONG SOURCEID;                      // STUDYID, LISTID  
	LONG RECORDID;        // INSTANCE ID
	LONG PERSONID;
}   IMS_TRANSACTION;


typedef struct {
    LONG          locid;
    LONG          ltype;
    LONG          nllp;
    CHAR          lname[GMS_LNAME_SIZE];
    CHAR          labbr[GMS_LABBR_SIZE];
    LONG	         snl3id;
    LONG          snl2id;
    LONG          snl1id;
    LONG          cntryid;
    LONG          lrplce;
}           IMS_Location;


typedef struct {
	LONG ID; 
	LONG LABELINFO_ID; 
	CHAR GROUP_PREFIX[IMS_GROUP_PREFIX]; 
	LONG LABELITEMCOUNT;
} IMS_LABELINFO;


typedef struct {
	LONG ID; 
	LONG OTHERINFO_ID; 
	LONG LABELINFO_ID;
	CHAR GROUP_PREFIX[IMS_GROUP_PREFIX];
	CHAR TABLENAME[IMS_NAME_LENGTH];
	CHAR FIELDNAME[IMS_NAME_LENGTH];
	CHAR FOREIGN_FIELDNAME[IMS_NAME_LENGTH];
}  IMS_LABEL_OTHERINFO;






/////////////////////////////////////////////////////////////////////////////////////////////////
//////  IMS INTERFACE
/////   Date Created: January 16, 2004
//////////////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE IMS_openDatabase(LPCSTR szIniFile);
void DLL_INTERFACE IMS_closeDatabase(void);
void DLL_INTERFACE IMS_autoCommit(BOOL autoCommit);
void DLL_INTERFACE IMS_commitData(void);
void DLL_INTERFACE IMS_rollbackData(void);

LONG DLL_INTERFACE IMS_addLot(IMS_LOT *lot);
LONG DLL_INTERFACE IMS_addTransaction(IMS_TRANSACTION* trans);

LONG DLL_INTERFACE IMS_countLot(LONG EID,CHAR ETYPE[16], LONG* cntLot);

LONG DLL_INTERFACE IMS_findTransactionRecord(LONG lotid,IMS_TRANSACTION *data,LONG fSearchOption);
LONG DLL_INTERFACE IMS_findLotRecord(IMS_LOT *data,LONG fSearchOption);

LONG DLL_INTERFACE IMS_getLocation3(IMS_Location *data, LONG fSearchOption);
LONG DLL_INTERFACE IMS_getTransactionReserve(LONG flagAll,IMS_TRANSACTION *data,IMS_LOT *lot,LONG fSearchOption);
LONG DLL_INTERFACE IMS_getTransactionDeposit(LONG flagAll,IMS_TRANSACTION *data,IMS_LOT *lot,LONG fSearchOption);
LONG DLL_INTERFACE IMS_getReserveByPerson(IMS_TRANSACTION *data,IMS_LOT *lot,LONG fSearchOption);
LONG DLL_INTERFACE IMS_getDepositByPerson(IMS_TRANSACTION *data,IMS_LOT *lot,LONG fSearchOption);


LONG DLL_INTERFACE IMS_setLot(IMS_LOT* lot);
LONG DLL_INTERFACE IMS_setTransaction(IMS_TRANSACTION* trans);

LONG DLL_INTERFACE IMS_closeLot(LONG lotid);
LONG DLL_INTERFACE IMS_calculateBalance(LONG lotid, DOUBLE *trnqty1, DOUBLE *trnqyt2);

LONG DLL_INTERFACE IMS_reportLotByEntity( CHAR *szETYPE, LONG *lotid, LONG *eid, CHAR *szLocname, CHAR *szScale,CHAR *szComment, long fSearchOption );
LONG DLL_INTERFACE IMS_reportAllTransaction(LONG *lotid, LONG *cmtdate, double *quantity, CHAR *szLocname, CHAR *szScale,CHAR *szComment, long fSearchOption );
LONG DLL_INTERFACE IMS_reportBalance( LONG *lotid, double *sumTrans, CHAR *szLocname, CHAR *szScale, long fSearchOption );
LONG DLL_INTERFACE IMS_getBalanceOfLot(LONG lot, double *sumTrans);

LONG DLL_INTERFACE IMS_reportLotByEntityGID( CHAR *szETYPE, LONG *lotid, LONG *eid, CHAR *szLocname, CHAR *szScale,CHAR *szComment, long fSearchOption );
LONG DLL_INTERFACE IMS_reportLotByEntityGIDFrom( CHAR *szETYPE, LONG *lotid, LONG *eid, CHAR *szLocname, CHAR *szScale,CHAR *szComment, long fSearchOption );
LONG DLL_INTERFACE IMS_reportLotByEntityGIDRange( CHAR *szETYPE, LONG eidFrom, LONG eidTo, LONG *lotid,  LONG *eid, CHAR *szLocname, CHAR *szScale,CHAR *szComment, long fSearchOption );
LONG DLL_INTERFACE IMS_reportEmptyShelves( LONG *lotid,  LONG *eid, CHAR *szLocname, CHAR *szScale, long fSearchOption );
LONG DLL_INTERFACE IMS_reportMaterialsDist( LONG *lotid, LONG *eid, double *quantity, CHAR *szLocname, CHAR *szScale, CHAR *szComment, LONG *cmtdate, long fSearchOption );
LONG DLL_INTERFACE IMS_reportDormantEntries(LONG minYear, LONG *lotid, LONG *eid, double *quantity, CHAR *szLocname, CHAR *szScale, LONG *cmtdate, long fSearchOption );
LONG DLL_INTERFACE IMS_reportMinimumAmount( double minQty, LONG *lotid, LONG *eid, double *quantity, CHAR *szLocname, CHAR *szScale, long fSearchOption );
LONG DLL_INTERFACE IMS_reportByRequestor( LONG *lotid, LONG *eid, double *quantity, CHAR *szLocname, CHAR *szScale, CHAR *szComment, LONG *cmtdate, LONG *lngStat, LONG *lngPerson, long fSearchOption );

LONG DLL_INTERFACE IMS_geLabelOtherInfo(IMS_LABEL_OTHERINFO *data, long fSearchOption );
LONG DLL_INTERFACE IMS_geLabelInfo(IMS_LABELINFO *data, long fSearchOption );

/////////////////////////////////////////////////////////////////////////////////////////////////
//////  GEMS INTERFACE
/////   Date Created: July 27, 2006
//////////////////////////////////////////////////////////////////////////////////////
#define GEMS_SUCCESS  1
#define GEMS_ERROR  0

#define   GEMS_MARKER_NAME         256
#define   GEMS_TABLE_NAME          256



#define  MARKER_NAME_TYPE  2
#define MARKER_MV_TYPE  "SSR"
#define PROTOCOL_NAME_TYPE  "SSR"
#define MV_NAME_TYPE  3
#define LEN_MATYPE  32
#define LEN_PRIMER  32
#define LEN_MVTYPE  32
#define STANDARD_METHOD  0
#define STANDARD_SCALE  0

typedef struct {
        LONG GOBJID;
        CHAR GNVAL[GEMS_MARKER_NAME];
        CHAR GOBJTYPE[GEMS_TABLE_NAME];
        LONG MARKERID;
}	GEMS_INFO;

typedef struct { 
    LONG mdid;
    CHAR matype[LEN_MATYPE];
    CHAR fprimer[LEN_PRIMER];
    CHAR rprimer[LEN_PRIMER];
    LONG lmdid ;
    LONG mauid;
    LONG maref;
    LONG minallele;
    LONG maxallele;
} GEMS_MARKER_DETECTOR;


typedef struct { 
    LONG mvid ;
    LONG markerid;
    LONG mvtype ;
    DOUBLE mwt;
    LONG mdid;
    LONG lmvid;
    LONG mvuid ;
    LONG mvref ;
}  GEMS_MV;

typedef struct { 
    LONG gnid;
    LONG GOBJID;
    CHAR GOBJTYPE[256] ;
    LONG gntype ;
    LONG gnstat ;
    LONG gnuid ;
    CHAR GNVAL[256];
    LONG gnlocn;
    LONG gndate ;
    LONG gnref ;
} GEMS_NAME;

typedef struct 
{  
    LONG cid;
    LONG condid ;
    LONG comid;
    CHAR comval[256];
    LONG pid ;
    CHAR comtype[256];
    CHAR comgrp[256];
    LONG comuid ;
    LONG comref ;
} GEMS_COMPONENT;

typedef struct 
{  
    LONG pd_comp ;
    LONG pdid ;
    LONG cid ;
} GEMS_PD_COMP;

typedef struct 
{ 
    LONG pid;
    LONG propid ;
    CHAR PropName[256];
    LONG scaleid ;
    LONG methid;
    CHAR Propgrp[256];
} GEMS_PROP;

typedef struct 
{ 
    LONG pdid;
    LONG condid;
    LONG mdid;
}  GEMS_PD;

typedef struct 
{ 
    LONG condid;
    CHAR condname[256];
}  GEMS_CONDITION;

typedef struct 
{ 
    LONG cid;
    LONG condition;
    LONG pdid;
} PD_COMP_INFO;

typedef struct 
{ 
    LONG scaleid;
    CHAR scname[256];
    LONG propid;
    CHAR sctype[256];
} GEMS_SCALE;

typedef struct 
{ 
    LONG methid;
    CHAR mname[256];
    CHAR mabbr[256];
    CHAR mdesc[256];
} GEMS_METHOD;

typedef struct 
{ 
    LONG pid;
    CHAR mname[256];
    CHAR sname[256];
    CHAR pname[256];
    CHAR pgrpname[256];
}  PROP_INFO;

typedef struct 
{ 
    LONG locusid;
    CHAR chr[256];
    CHAR pos[256];
}  GEMS_LOCUS;


LONG DLL_INTERFACE GEMS_openDatabase(LPCSTR szIniFile);
void DLL_INTERFACE GEMS_autoCommit(BOOL autoCommit);
void DLL_INTERFACE GEMS_commitData(void);
void DLL_INTERFACE GEMS_closeDatabase(void);

LONG DLL_INTERFACE GEMS_getGemsMID(GEMS_INFO *data, int fOpt);
LONG DLL_INTERFACE GEMS_getGemsMVID(GEMS_INFO *data, int fOpt);
LONG DLL_INTERFACE GEMS_getMaxID(CHAR *szTblName, CHAR *szFieldName);
LONG DLL_INTERFACE GEMS_addGEMSName(GEMS_NAME *recName);
LONG DLL_INTERFACE GEMS_addMD(GEMS_MARKER_DETECTOR *recTblMD ); 
LONG DLL_INTERFACE  GEMS_addMV(GEMS_MV *recTblMV);
LONG DLL_INTERFACE GEMS_addPD(GEMS_PD *recTblPd );
LONG DLL_INTERFACE  GEMS_addComponent(GEMS_COMPONENT *recTblComp);
LONG DLL_INTERFACE GEMS_getPdComp(PD_COMP_INFO *pdCompInfo, LONG fopt);
LONG DLL_INTERFACE GEMS_addPdComp(GEMS_PD_COMP *recTblPdComp ); 
LONG DLL_INTERFACE  GEMS_addProp(GEMS_PROP *data);
LONG DLL_INTERFACE GEMS_getProtocolID(GEMS_INFO *recProcInfo , LONG fopt);
LONG DLL_INTERFACE GEMS_findPID(PROP_INFO *PropInfo) ;
LONG DLL_INTERFACE GEMS_findPropid(GEMS_PROP *recTblProp);
LONG DLL_INTERFACE GEMS_findMethod( GEMS_METHOD *recTblMethod);
LONG DLL_INTERFACE GEMS_addMethod(GEMS_METHOD *tblMethod) ;
LONG DLL_INTERFACE GEMS_findScale(GEMS_SCALE *tblScale);
LONG DLL_INTERFACE GEMS_addScale(GEMS_SCALE *tblScale) ;
LONG DLL_INTERFACE GEMS_findPdid(GEMS_PD *tblPd);
LONG DLL_INTERFACE GEMS_addLocus(GEMS_LOCUS *tblLocus);
LONG DLL_INTERFACE GEMS_getNameID(GEMS_INFO *data);
LONG DLL_INTERFACE GEMS_getAlleleID( GEMS_INFO *recNameInfo, LONG fopt) ;


#if defined(WEB)
	void CheckWebService(LPCSTR szIniFile);
	_declspec(dllimport) BOOL __stdcall BufferStruct(NetBean *InStr,int sz);
	_declspec(dllimport) BOOL __stdcall BufferMethod(MethodBean *InStr,int sz);
    _declspec(dllimport) BOOL _stdcall BufferLocation(LocationBean *InStruct, int codeFuction, int sze);
#endif

}




#endif //_ICIS32_H_