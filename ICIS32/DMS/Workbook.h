#ifndef _WORKBOOK_H_
#define _WORKBOOK_H_

#include "icis32.h"
#include "codbc.h"

#include "study.h"
#include "factor.h"
#include "variate.h"
#include "trait.h"
#include "search.h"
#include "level.h"
#include "data.h"

#include <windows.h>
#include <iostream>
using std::ios;
using std::endl;
using std::cerr;
using std::cout;
#include <iomanip>
using std::setw;
#include <fstream>
using std::ifstream;
using std::ofstream;
using std::fstream;
#include <cstring>
#include <string>
using std::string;
#include <sstream>
std::string ltos( long aInt )
{
     std::ostringstream buf;
     buf << aInt;
     return buf.str();
}

#define DESSHT "Description.txt"
#define OBSCOL "ObsCol"
#define CHADAT "Cha.dat"
#define WBK_SUCCESS (1)

#define DMS_STUDY_NAME       52
#define DMS_STUDY_TITLE     256
#define DMS_STUDY_STYPE       4
#define DMS_FACTOR_NAME      52
#define DMS_LEVEL_TYPE        4
#define DMS_ATTR_TABLE       12
#define DMS_TRAIT_NAME       52
//#define DMS_TRAIT_ABBR        8
#define DMS_SCALE_NAME       52
//#define DMS_SCALE_TYPE        4
#define DMS_METHOD_NAME      52
//#define DMS_METHOD_ABBR       8
#define DMS_DATA_VALUE      256
#define DMS_VARIATE_NAME     52
#define DMS_VARIATE_TYPE      4
#define DMS_DATA_TYPE         4
#define GMS_MAX_TABLE_NAME   16
#define GMS_MAX_FIELD_NAME   16
#define GMS_MAX_GROUP_NAME   20 //16 is used

typedef struct{
	char vname[DMS_FACTOR_NAME];
    char vdesc[1000];
    char pname[DMS_TRAIT_NAME];
    char sname[DMS_SCALE_NAME];
    char mname[DMS_METHOD_NAME];
    char dtype[DMS_DATA_TYPE];
    char label[DMS_FACTOR_NAME];
} VAR_Sectn;

typedef struct{
    char vname[DMS_FACTOR_NAME];
    char vdesc[1000];
    char pname[DMS_TRAIT_NAME];
    char sname[DMS_SCALE_NAME];
    char mname[DMS_METHOD_NAME];
    char dtype[DMS_DATA_TYPE];
	char value[DMS_DATA_VALUE];
    char label[DMS_FACTOR_NAME];
} CND_Sectn;

typedef struct{
    char vname[DMS_VARIATE_NAME];
    char vdesc[1000];
    char pname[DMS_TRAIT_NAME];
    char sname[DMS_SCALE_NAME];
    char mname[DMS_METHOD_NAME];
    char dtype[DMS_DATA_TYPE];
	char value[DMS_DATA_VALUE];
} CNS_Sectn;

VAR_Sectn *uVarSect;
CND_Sectn *uCndSect;
CNS_Sectn *uCnsSect;

#endif
