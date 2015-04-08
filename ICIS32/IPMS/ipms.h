#ifndef _IPMS_H_
#define _IPMS_H_

#include "icis32.h"
#include "codbc.h"

#define IPMS_ERROR      -1
#define IPMS_SUCCESS    1
#define IPMS_NO_DATA    0

#define IPMS_REMARK      256          //255 is used   
#define IPMS_ACCID       12           //10 is used
#define PROT_TYPE        28           //25 is used
#define PRIM_DOC   4            //3 is used 

typedef struct {
	LONG IPMS_ID;
	LONG EFFECTIVE_DATA;
	LONG EXPIRY_DATE;
	LONG VERSION_ID;
	CHAR REMARK[255];
}  IPMS_IPMS;


typedef struct {
	LONG PROTID;
	LONG IPMS_ID;
	LONG SUPERSEDED_BY;
	CHAR ACCESSION_ID[IPMS_ACCID];
	CHAR TYPE[PROT_TYPE];
	LONG OWNER;
	CHAR CHECK_PRIM_DOC[PRIM_DOC];
}  IPMS_PROT;


typedef struct {
	CHAR MTA_ID[24];
	CHAR OWNER[52];
	LONG EFFECTIVE_DATE;
	LONG EXPIRY_DATE;
    CHAR REMARK[256];
	CHAR SUPERSEDED_BY[24];
	CHAR MTA_TYPE[4];
	CHAR SENDER[52];
	CHAR RECIPIENT[52];
} IPMS_MTA;



#endif