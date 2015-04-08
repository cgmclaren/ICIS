#include "Workbook.h"


class ChaLog{
public:
	ChaLog(char*);
	void logChange(long, long, char*);
	~ChaLog();
private:
	ofstream outChaDat;
	char zWrkDir[255];
	char zRowNo[6];
	char zColNo[4];
	char zValue[255];
};

ChaLog::ChaLog(char* zpWrkDir){
	//open the changes registry file
	strcpy(zWrkDir, zpWrkDir);
	outChaDat.open(strcat(zWrkDir, CHADAT), ios::out);
	if (!outChaDat){
		cerr << "File could not be opened." << endl;
		exit(1);
	}
}

void ChaLog::logChange(long lRowNo, long lColNo, char* zpValue){
	sprintf(zRowNo, "%d", lRowNo);
	sprintf(zColNo, "%d", lColNo);
	outChaDat.write(zRowNo, sizeof(zRowNo));
	outChaDat.write(zColNo, sizeof(zColNo));
	outChaDat.write(strcpy(zValue, zpValue), sizeof(zValue));
}

ChaLog::~ChaLog(){
	//close the changes registry file
	outChaDat.close();
}


void *getfld(char *field, const char *record, size_t fldsz){
	int i = fldsz - 1;                     //start looking at last char pos relative to field size
	while(record[i] == ' ' && i > -1) i--; //move up while current char pos holds a space char
	field[i + 1] = '\0';                   //put terminator first after the last non-space char
	while(i > -1){                         //copy the remaining chars from last to first
		field[i] = record[i];
		i--;
	}
	return field;
}


LONG DLL_INTERFACE RetrieveDes(long lStdID, long lRepNo, char *zColTyp, LPSAFEARRAY FAR *zColNam, long *lVarID, \
							 long *lCndCnt, long *lFacCnt, long *lCnsCnt, long *lVarCnt){
	
	char tmpDir[255], wrkDir[255];
	GetEnvironmentVariable("TEMP", tmpDir, 255);
	strcat(tmpDir, "\\");
	
	//create the DESCRIPTION SHEET file
	strcpy(wrkDir, tmpDir);
	fstream ofDesSht(strcat(wrkDir, DESSHT), ios::out);
	if (!ofDesSht){
		cerr << "DesSht File could not be opened." << endl;
		exit(1);
	}
	
	//get the study info
	DMS_STUDY uStudy;
	ZeroMemory(&uStudy,sizeof(uStudy));
	char *zStdObj = new char[MAX_STR];
	//char zStdObj[MAX_STR];
	long ret;
	uStudy.STUDYID = lStdID;
	uStudy.SHIERARCHY = 0;
	ret = getStudy(LOCAL, &uStudy, reinterpret_cast<UCHAR*>(zStdObj), MAX_STR, FIND_FIRST);
	if (ret != DMS_SUCCESS) ret = getStudy(CENTRAL, &uStudy, reinterpret_cast<UCHAR*>(zStdObj), MAX_STR, FIND_NEXT);
	
	//write the STUDY section entries
	char *sname = new char[DMS_STUDY_NAME], *title = new char[DMS_STUDY_TITLE], \
		 *pmkey = new char[8], *objec = new char[MAX_STR], \
		 *sdate = new char[8], *edate = new char[8], \
		 *stype = new char[DMS_STUDY_STYPE];
	
	ofDesSht.write("STUDY", 5); ofDesSht.put('\t');
	getfld(sname, uStudy.SNAME, DMS_STUDY_NAME);
	ofDesSht.write(sname, strlen(sname)); ofDesSht.put('\n'); ofDesSht.flush();
	
	ofDesSht.write("TITLE", 5); ofDesSht.put('\t');
	getfld(title, uStudy.TITLE, DMS_STUDY_TITLE);
	ofDesSht.write(title, strlen(title)); ofDesSht.put('\n'); ofDesSht.flush();
	
	ofDesSht.write("PMKEY", 5); ofDesSht.put('\t');
	memset(pmkey, '\0', sizeof(pmkey));
	sprintf(pmkey, "%d", uStudy.PMKEY);
	ofDesSht.write(pmkey, strlen(pmkey)); ofDesSht.put('\n'); ofDesSht.flush();
	
	ofDesSht.write("OBJECTIVE", 9); ofDesSht.put('\t');
	getfld(objec, zStdObj, MAX_STR);
	ofDesSht.write(objec, strlen(objec)); ofDesSht.put('\n'); ofDesSht.flush();
	
	ofDesSht.write("START DATE", 10); ofDesSht.put('\t');
	memset(sdate, '\0', sizeof(sdate));
	sprintf(sdate, "%d", uStudy.SDATE);
	ofDesSht.write(sdate, strlen(sdate)); ofDesSht.put('\n'); ofDesSht.flush();
	
	ofDesSht.write("END DATE", 8); ofDesSht.put('\t');
	memset(edate, '\0', sizeof(edate));
	sprintf(edate, "%d", uStudy.EDATE);
	ofDesSht.write(edate, strlen(edate)); ofDesSht.put('\n'); ofDesSht.flush();
	
	ofDesSht.write("STUDY TYPE", 10); ofDesSht.put('\t');
	getfld(stype, uStudy.STYPE, DMS_STUDY_STYPE);
	ofDesSht.write(stype, strlen(stype)); ofDesSht.put('\n'); ofDesSht.flush();
	
	//free up allocated memory space
	//delete [] zStdObj; delete [] sname; delete [] title; delete [] pmkey;
	//delete [] objec; delete [] sdate; delete [] edate; delete [] stype;
	
	//write the blank row separating the STUDY & CONDITION sections
	ofDesSht.put('\n'); ofDesSht.flush();
	
	//get the study factor ID
	DMS_FACTOR uFactor;
	long lStdFID;
	uFactor.STUDYID = lStdID;
	strcpy(uFactor.FNAME, "STUDY");
	ret = findFactorEq(LOCAL, "STUDY", uFactor, FIND_FIRST);
	if (ret != DMS_SUCCESS) ret = findFactorEq(CENTRAL, "STUDY", uFactor, FIND_NEXT);
	lStdFID = uFactor.FACTORID;
	
	//get the selected factor IDs
	long lFacTtl, lVarTtl, lvar, lColID[255];
	lFacTtl = *lFacCnt;
	for (lvar = 0; lvar < lFacTtl; lvar++){
		lColID[lvar] = lVarID[lvar];
	}
	//get the selected variate IDs
	lVarTtl = *lVarCnt;
	for (lvar = lFacTtl; lvar < lFacTtl + lVarTtl; lvar++){
		lColID[lvar] = lVarID[lvar];
	}
	
	//get all factor IDs for the specified dataset (represno)
	DMS_SRFACTOR* uSRFactor; DMS_DMSATTR uDMSAttr; DMS_TRAIT uProperty; DMS_SCALE uScale; DMS_TMETHOD uMethod;
	long lFacID[255], lCndID[255], lcnd = 0, lidx;
	uVarSect = NULL; //needed to make realloc() work like malloc()
	uCndSect = NULL; //needed to make realloc() work like malloc()
	char *zVarDesc = new char[MAX_STR], *zDtype = new char[255];
	zDtype[0] = '\0';
	char *pzBufNam[255];
	for ( int i = 0; i < 255; ++i ) pzBufNam[i] = (char*) malloc(DMS_FACTOR_NAME * sizeof(char));
	BSTR *pzColNam;
	HRESULT lResult;
	lResult = SafeArrayLock(*zColNam);
	if( lResult) return (lResult);
	pzColNam = (BSTR*) (*zColNam)->pvData;
	
	bool bFound;
	lvar = 0;
	uSRFactor->REPRESNO = lRepNo;
	uSRFactor->STUDYID = lStdID;
	ret = getSRFactor(LOCAL, uSRFactor, FIND_FIRST);
	if (ret != DMS_SUCCESS) ret = getSRFactor(CENTRAL, uSRFactor, FIND_NEXT);
	while (ret == DMS_SUCCESS){
		if (uSRFactor->FACTORID != lStdFID){
			//check if the factor is one of the selected ones
			bFound = false;
			for (lidx = 0; lidx < lFacTtl; lidx++){
				if (uSRFactor->LABELID == lColID[lidx]) {
					bFound = true; break;
				}
			}
			if (bFound == true) {
				//get the label ID, factor ID, factor name, & data type
				lVarID[lvar] = uSRFactor->LABELID;
				lFacID[lvar] = uSRFactor->FACTORID;
				uVarSect = (VAR_Sectn*) realloc(uVarSect, (lvar + 1) * sizeof(VAR_Sectn));
				strcpy(uVarSect[lvar].vname, uSRFactor->FNAME);
				strcpy(uVarSect[lvar].dtype, uSRFactor->LTYPE);
				strcpy(pzBufNam[lvar], uSRFactor->FNAME);
				pzColNam[lvar] = (BSTR) pzBufNam[lvar];
				strcat(zDtype, uVarSect[lvar].dtype);
				//get the factor description
				uDMSAttr.DMSATYPE = 801;
				strcpy(uDMSAttr.DMSATAB, "FACTOR");
				uDMSAttr.DMSATREC = uSRFactor->LABELID;
				memset(zVarDesc, '\0', MAX_STR);
				ret = getDMSATTR(LOCAL, &uDMSAttr, zVarDesc, MAX_STR, FIND_FIRST);
				if (ret != DMS_SUCCESS) ret = getDMSATTR(CENTRAL, &uDMSAttr, zVarDesc, MAX_STR, FIND_NEXT);
				strcpy(uVarSect[lvar].vdesc, zVarDesc);
				//get the property name
				uProperty.TRAITID = uSRFactor->TRAITID;
				memset(zVarDesc, '\0', MAX_STR);
				ret = getTrait_v4(LOCAL, &uProperty, reinterpret_cast<UCHAR*>(zVarDesc), MAX_STR, FIND_FIRST);
				if (ret != DMS_SUCCESS) ret = getTrait_v4(CENTRAL, &uProperty, reinterpret_cast<UCHAR*>(zVarDesc), MAX_STR, FIND_NEXT);
				strcpy(uVarSect[lvar].pname, reinterpret_cast<char*>(uProperty.TRNAME));
				//get the scale name
				uScale.SCALEID = uSRFactor->SCALEID;
				ret = getScale(LOCAL, &uScale, FIND_FIRST);
				if (ret != DMS_SUCCESS) ret = getScale(CENTRAL, &uScale, FIND_NEXT);
				strcpy(uVarSect[lvar].sname, reinterpret_cast<char*>(uScale.SCNAME));
				//get the method name
				uMethod.TMETHID = uSRFactor->TMETHID;
				memset(zVarDesc, '\0', MAX_STR);
				ret = getTmethod(LOCAL, &uMethod, reinterpret_cast<UCHAR*>(zVarDesc), MAX_STR, FIND_FIRST);
				if (ret != DMS_SUCCESS) ret = getTmethod(CENTRAL, &uMethod, reinterpret_cast<UCHAR*>(zVarDesc), MAX_STR, FIND_NEXT);
				strcpy(uVarSect[lvar].mname, reinterpret_cast<char*>(uMethod.TMNAME));
				lvar++;
			}
		}else{
			//get the study condition variables
			if (strcmp(uSRFactor->FNAME, "STUDY") != 0){
				//get the condition ID, name, data type, & label
				lCndID[lcnd] = uSRFactor->LABELID;
				uCndSect = (CND_Sectn*) realloc(uCndSect, (lcnd + 1) * sizeof(CND_Sectn));
				strcpy(uCndSect[lcnd].vname, uSRFactor->FNAME);
				strcpy(uCndSect[lcnd].dtype, uSRFactor->LTYPE);
				strcpy(uCndSect[lcnd].label, "STUDY");
				//get the condition description
				uDMSAttr.DMSATYPE = 801;
				strcpy(uDMSAttr.DMSATAB, "FACTOR");
				uDMSAttr.DMSATREC = uSRFactor->LABELID;
				memset(zVarDesc, '\0', MAX_STR);
				ret = getDMSATTR(LOCAL, &uDMSAttr, zVarDesc, MAX_STR, FIND_FIRST);
				if (ret != DMS_SUCCESS) ret = getDMSATTR(CENTRAL, &uDMSAttr, zVarDesc, MAX_STR, FIND_NEXT);
				strcpy(uCndSect[lcnd].vdesc, zVarDesc);
				//get the property name
				uProperty.TRAITID = uSRFactor->TRAITID;
				memset(zVarDesc, '\0', MAX_STR);
				ret = getTrait_v4(LOCAL, &uProperty, reinterpret_cast<UCHAR*>(zVarDesc), MAX_STR, FIND_FIRST);
				if (ret != DMS_SUCCESS) ret = getTrait_v4(CENTRAL, &uProperty, reinterpret_cast<UCHAR*>(zVarDesc), MAX_STR, FIND_NEXT);
				strcpy(uCndSect[lcnd].pname, reinterpret_cast<char*>(uProperty.TRNAME));
				//get the scale name
				uScale.SCALEID = uSRFactor->SCALEID;
				ret = getScale(LOCAL, &uScale, FIND_FIRST);
				if (ret != DMS_SUCCESS) ret = getScale(CENTRAL, &uScale, FIND_NEXT);
				strcpy(uCndSect[lcnd].sname, reinterpret_cast<char*>(uScale.SCNAME));
				//get the method name
				uMethod.TMETHID = uSRFactor->TMETHID;
				memset(zVarDesc, '\0', MAX_STR);
				ret = getTmethod(LOCAL, &uMethod, reinterpret_cast<UCHAR*>(zVarDesc), MAX_STR, FIND_FIRST);
				if (ret != DMS_SUCCESS) ret = getTmethod(CENTRAL, &uMethod, reinterpret_cast<UCHAR*>(zVarDesc), MAX_STR, FIND_NEXT);
				strcpy(uCndSect[lcnd].mname, reinterpret_cast<char*>(uMethod.TMNAME));
				lcnd++;
			}
		}
		ret = getSRFactor(LOCAL, uSRFactor, FIND_NEXT);
		if (ret != DMS_SUCCESS) ret = getSRFactor(CENTRAL, uSRFactor, FIND_NEXT);
	}
	//get the variable counts
	*lFacCnt = lvar;
	*lCndCnt = lcnd;
	
	//get the factor label
	for (lvar = 0; lvar < *lFacCnt; lvar++){
		for (lidx = 0; lidx < *lFacCnt; lidx++){
			if (lFacID[lvar] == lVarID[lidx]){
				strcpy(uVarSect[lvar].label, uVarSect[lidx].vname);
				break;
			}
		}
	}
	
	//get the condition values
	DMS_CONDLEVEL uCondLevel; 
	lcnd = 0;
	uCondLevel.REPRESNO = lRepNo;
	uCondLevel.FACTORID = lStdFID;
	ret = getCondLevel(LOCAL, &uCondLevel, FIND_FIRST);
	if (ret != DMS_SUCCESS) ret = getCondLevel(CENTRAL, &uCondLevel, FIND_NEXT);
	while (ret == DMS_SUCCESS){
		if (uCondLevel.LABELID != uCondLevel.FACTORID){
			while (uCondLevel.LABELID != lCndID[lcnd]) lcnd++;
			strcpy(uCndSect[lcnd].value, uCondLevel.LVALUE);
		}
		ret = getCondLevel(LOCAL, &uCondLevel, FIND_NEXT);
		if (ret != DMS_SUCCESS) ret = getCondLevel(CENTRAL, &uCondLevel, FIND_NEXT);
	}
	
	//write the CONDITION section headers
	ofDesSht.write("CONDITION", 9); ofDesSht.put('\t');
	ofDesSht.write("DESCRIPTION", 11); ofDesSht.put('\t');
	ofDesSht.write("PROPERTY", 8); ofDesSht.put('\t');
	ofDesSht.write("SCALE", 5); ofDesSht.put('\t');
	ofDesSht.write("METHOD", 6); ofDesSht.put('\t');
	ofDesSht.write("DATA TYPE", 9); ofDesSht.put('\t');
	ofDesSht.write("VALUE", 5); ofDesSht.put('\t');
	ofDesSht.write("LABEL", 5); ofDesSht.put('\n'); ofDesSht.flush();
	
	//write the CONDITION section entries
	char *vname = new char[DMS_FACTOR_NAME], *vdesc = new char[MAX_STR], *trait = new char[DMS_TRAIT_NAME], \
		 *scale = new char[DMS_SCALE_NAME], *methd = new char[DMS_METHOD_NAME], *dtype = new char[DMS_DATA_TYPE], \
		 *value = new char[DMS_DATA_VALUE], *label = new char[DMS_FACTOR_NAME];
	for (lcnd = 0; lcnd < *lCndCnt; lcnd++){
		getfld(vname, uCndSect[lcnd].vname, DMS_FACTOR_NAME);
		ofDesSht.write(vname, strlen(vname)); ofDesSht.put('\t');
		getfld(vdesc, uCndSect[lcnd].vdesc, MAX_STR);
		ofDesSht.write(vdesc, strlen(vdesc)); ofDesSht.put('\t');
		getfld(trait, uCndSect[lcnd].pname, DMS_TRAIT_NAME);
		ofDesSht.write(trait, strlen(trait)); ofDesSht.put('\t');
		getfld(scale, uCndSect[lcnd].sname, DMS_SCALE_NAME);
		ofDesSht.write(scale, strlen(scale)); ofDesSht.put('\t');
		getfld(methd, uCndSect[lcnd].mname, DMS_METHOD_NAME);
		ofDesSht.write(methd, strlen(methd)); ofDesSht.put('\t');
		getfld(dtype, uCndSect[lcnd].dtype, DMS_DATA_TYPE);
		ofDesSht.write(dtype, strlen(dtype)); ofDesSht.put('\t');
		getfld(value, uCndSect[lcnd].value, DMS_DATA_VALUE);
		ofDesSht.write(value, strlen(value)); ofDesSht.put('\t');
		getfld(label, uCndSect[lcnd].label, DMS_FACTOR_NAME);
		ofDesSht.write(label, strlen(label)); ofDesSht.put('\n'); ofDesSht.flush();
	}
	
	//write the blank row separating the CONDITION & FACTOR sections
	ofDesSht.put('\n'); ofDesSht.flush();
	
	//write the FACTOR section headers
	ofDesSht.write("FACTOR", 6); ofDesSht.put('\t');
	ofDesSht.write("DESCRIPTION", 11); ofDesSht.put('\t');
	ofDesSht.write("PROPERTY", 8); ofDesSht.put('\t');
	ofDesSht.write("SCALE", 5); ofDesSht.put('\t');
	ofDesSht.write("METHOD", 6); ofDesSht.put('\t');
	ofDesSht.write("DATA TYPE", 9); ofDesSht.put('\t');
	ofDesSht.write("VALUE", 5); ofDesSht.put('\t');
	ofDesSht.write("LABEL", 5); ofDesSht.put('\n'); ofDesSht.flush();
	
	//write the FACTOR section entries
	for (lvar = 0; lvar < *lFacCnt; lvar++){
		getfld(vname, uVarSect[lvar].vname, DMS_FACTOR_NAME);
		ofDesSht.write(vname, strlen(vname)); ofDesSht.put('\t');
		getfld(vdesc, uVarSect[lvar].vdesc, MAX_STR);
		ofDesSht.write(vdesc, strlen(vdesc)); ofDesSht.put('\t');
		getfld(trait, uVarSect[lvar].pname, DMS_TRAIT_NAME);
		ofDesSht.write(trait, strlen(trait)); ofDesSht.put('\t');
		getfld(scale, uVarSect[lvar].sname, DMS_SCALE_NAME);
		ofDesSht.write(scale, strlen(scale)); ofDesSht.put('\t');
		getfld(methd, uVarSect[lvar].mname, DMS_METHOD_NAME);
		ofDesSht.write(methd, strlen(methd)); ofDesSht.put('\t');
		getfld(dtype, uVarSect[lvar].dtype, DMS_DATA_TYPE);
		ofDesSht.write(dtype, strlen(dtype)); ofDesSht.put('\t');
		ofDesSht.put('\t');
		getfld(label, uVarSect[lvar].label, DMS_FACTOR_NAME);
		ofDesSht.write(label, strlen(label)); ofDesSht.put('\n'); ofDesSht.flush();
	}
	
	//write the blank row separating the FACTOR & CONSTANT sections
	ofDesSht.put('\n'); ofDesSht.flush();
	
	//get the study represno
	long *lStdRN = new long;
	ret = findStEffect(LOCAL, lStdID, lStdRN);
	if (ret != DMS_SUCCESS) ret = findStEffect(CENTRAL, lStdID, lStdRN);
	
	//get the study constant variables
	DMS_SRVARIATE uSRVariate;
	long lCnsID[255], lcns = 0;
	uCnsSect = NULL; //needed to make realloc() work like malloc()
	uSRVariate.REPRESNO = *lStdRN;
	uSRVariate.STUDYID = lStdID;
	ret = getSRVariate(LOCAL, &uSRVariate, FIND_FIRST);
	if (ret != DMS_SUCCESS) ret = getSRVariate(CENTRAL, &uSRVariate, FIND_NEXT);
	while (ret == DMS_SUCCESS){
		//get the constant ID, name, & data type
		lCnsID[lcns] = uSRVariate.VARIATID;
		uCnsSect = (CNS_Sectn*) realloc(uCnsSect, (lcns + 1) * sizeof(CNS_Sectn));
		strcpy(uCnsSect[lcns].vname, uSRVariate.VNAME);
		strcpy(uCnsSect[lcns].dtype, uSRVariate.DTYPE);
		//get the constant description
		uDMSAttr.DMSATYPE = 802;
		strcpy(uDMSAttr.DMSATAB, "VARIATE");
		uDMSAttr.DMSATREC = uSRVariate.VARIATID;
		memset(zVarDesc, '\0', MAX_STR);
		ret = getDMSATTR(LOCAL, &uDMSAttr, zVarDesc, MAX_STR, FIND_FIRST);
		if (ret != DMS_SUCCESS) ret = getDMSATTR(CENTRAL, &uDMSAttr, zVarDesc, MAX_STR, FIND_NEXT);
		strcpy(uCnsSect[lcns].vdesc, zVarDesc);
		//get the property name
		uProperty.TRAITID = uSRVariate.TRAITID;
		memset(zVarDesc, '\0', MAX_STR);
		ret = getTrait_v4(LOCAL, &uProperty, reinterpret_cast<UCHAR*>(zVarDesc), MAX_STR, FIND_FIRST);
		if (ret != DMS_SUCCESS) ret = getTrait_v4(CENTRAL, &uProperty, reinterpret_cast<UCHAR*>(zVarDesc), MAX_STR, FIND_NEXT);
		strcpy(uCnsSect[lcns].pname, reinterpret_cast<char*>(uProperty.TRNAME));
		//get the scale name
		uScale.SCALEID = uSRVariate.SCALEID;
		ret = getScale(LOCAL, &uScale, FIND_FIRST);
		if (ret != DMS_SUCCESS) ret = getScale(CENTRAL, &uScale, FIND_NEXT);
		strcpy(uCnsSect[lcns].sname, reinterpret_cast<char*>(uScale.SCNAME));
		//get the method name
		uMethod.TMETHID = uSRVariate.TMETHID;
		memset(zVarDesc, '\0', MAX_STR);
		ret = getTmethod(LOCAL, &uMethod, reinterpret_cast<UCHAR*>(zVarDesc), MAX_STR, FIND_FIRST);
		if (ret != DMS_SUCCESS) ret = getTmethod(CENTRAL, &uMethod, reinterpret_cast<UCHAR*>(zVarDesc), MAX_STR, FIND_NEXT);
		strcpy(uCnsSect[lcns].mname, reinterpret_cast<char*>(uMethod.TMNAME));
		lcns++;
		ret = getSRVariate(LOCAL, &uSRVariate, FIND_NEXT);
		if (ret != DMS_SUCCESS) ret = getSRVariate(CENTRAL, &uSRVariate, FIND_NEXT);
	}
	//get the variable count
	*lCnsCnt = lcns;
	
	//get the constant values
	DMS_CONSDATA uConsData; 
	lcns = 0;
	uConsData.REPRESNO = *lStdRN;
	ret = getConsData(LOCAL, &uConsData, FIND_FIRST);
	if (ret != DMS_SUCCESS) ret = getConsData(CENTRAL, &uConsData, FIND_NEXT);
	while (ret == DMS_SUCCESS){
		while (uConsData.VARIATID != lCnsID[lcns]) lcns++;
		strcpy(uCnsSect[lcns].value, uConsData.DVALUE);
		ret = getConsData(LOCAL, &uConsData, FIND_NEXT);
		if (ret != DMS_SUCCESS) ret = getConsData(CENTRAL, &uConsData, FIND_NEXT);
	}
	
	//write the CONSTANT section headers
	ofDesSht.write("CONSTANT", 8); ofDesSht.put('\t');
	ofDesSht.write("DESCRIPTION", 11); ofDesSht.put('\t');
	ofDesSht.write("PROPERTY", 8); ofDesSht.put('\t');
	ofDesSht.write("SCALE", 5); ofDesSht.put('\t');
	ofDesSht.write("METHOD", 6); ofDesSht.put('\t');
	ofDesSht.write("DATA TYPE", 9); ofDesSht.put('\t');
	ofDesSht.write("VALUE", 5); ofDesSht.put('\n'); ofDesSht.flush();
	
	//write the CONSTANT section entries
	for (lcns = 0; lcns < *lCnsCnt; lcns++){
		getfld(vname, uCnsSect[lcns].vname, DMS_VARIATE_NAME);
		ofDesSht.write(vname, strlen(vname)); ofDesSht.put('\t');
		getfld(vdesc, uCnsSect[lcns].vdesc, MAX_STR);
		ofDesSht.write(vdesc, strlen(vdesc)); ofDesSht.put('\t');
		getfld(trait, uCnsSect[lcns].pname, DMS_TRAIT_NAME);
		ofDesSht.write(trait, strlen(trait)); ofDesSht.put('\t');
		getfld(scale, uCnsSect[lcns].sname, DMS_SCALE_NAME);
		ofDesSht.write(scale, strlen(scale)); ofDesSht.put('\t');
		getfld(methd, uCnsSect[lcns].mname, DMS_METHOD_NAME);
		ofDesSht.write(methd, strlen(methd)); ofDesSht.put('\t');
		getfld(dtype, uCnsSect[lcns].dtype, DMS_DATA_TYPE);
		ofDesSht.write(dtype, strlen(dtype)); ofDesSht.put('\t');
		getfld(value, uCnsSect[lcns].value, DMS_DATA_VALUE);
		ofDesSht.write(value, strlen(value)); ofDesSht.put('\n'); ofDesSht.flush();
	}
	
	//write the blank row separating the CONSTANT & VARIATE sections
	ofDesSht.put('\n'); ofDesSht.flush();
	
	//get all variate IDs for the specified dataset (represno)
	long lFVTtl;
	lvar = *lFacCnt;
	uSRVariate.REPRESNO = lRepNo;
	uSRVariate.STUDYID = lStdID;
	ret = getSRVariate(LOCAL, &uSRVariate, FIND_FIRST);
	if (ret != DMS_SUCCESS) ret = getSRVariate(CENTRAL, &uSRVariate, FIND_NEXT);
	while (ret == DMS_SUCCESS){
		//check if the variate is one of the selected ones
		bFound = false;
		for (lidx = lFacTtl; lidx < lFacTtl + lVarTtl; lidx++){
			if (uSRVariate.VARIATID == lColID[lidx]) {
				bFound = true; break;
			}
		}
		if (bFound == true) {
			//get the variate ID, name, & data type
			lVarID[lvar] = uSRVariate.VARIATID + 1000; //plus 1000 to avoid collision w/ the label IDs
			uVarSect = (VAR_Sectn*) realloc(uVarSect, (lvar + 1) * sizeof(VAR_Sectn));
			strcpy(uVarSect[lvar].vname, uSRVariate.VNAME);
			strcpy(uVarSect[lvar].dtype, uSRVariate.DTYPE);
			strcpy(pzBufNam[lvar], uSRVariate.VNAME);
			pzColNam[lvar] = (BSTR) pzBufNam[lvar];
			strcat(zDtype, uVarSect[lvar].dtype);
			//get the variate description
			uDMSAttr.DMSATYPE = 802;
			strcpy(uDMSAttr.DMSATAB, "VARIATE");
			uDMSAttr.DMSATREC = uSRVariate.VARIATID;
			memset(zVarDesc, '\0', MAX_STR);
			ret = getDMSATTR(LOCAL, &uDMSAttr, zVarDesc, MAX_STR, FIND_FIRST);
			if (ret != DMS_SUCCESS) ret = getDMSATTR(CENTRAL, &uDMSAttr, zVarDesc, MAX_STR, FIND_NEXT);
			strcpy(uVarSect[lvar].vdesc, zVarDesc);
			//get the property name
			uProperty.TRAITID = uSRVariate.TRAITID;
			memset(zVarDesc, '\0', MAX_STR);
			ret = getTrait_v4(LOCAL, &uProperty, reinterpret_cast<UCHAR*>(zVarDesc), MAX_STR, FIND_FIRST);
			if (ret != DMS_SUCCESS) ret = getTrait_v4(CENTRAL, &uProperty, reinterpret_cast<UCHAR*>(zVarDesc), MAX_STR, FIND_NEXT);
			strcpy(uVarSect[lvar].pname, reinterpret_cast<char*>(uProperty.TRNAME));
			//get the scale name
			uScale.SCALEID = uSRVariate.SCALEID;
			ret = getScale(LOCAL, &uScale, FIND_FIRST);
			if (ret != DMS_SUCCESS) ret = getScale(CENTRAL, &uScale, FIND_NEXT);
			strcpy(uVarSect[lvar].sname, reinterpret_cast<char*>(uScale.SCNAME));
			//get the method name
			uMethod.TMETHID = uSRVariate.TMETHID;
			memset(zVarDesc, '\0', MAX_STR);
			ret = getTmethod(LOCAL, &uMethod, reinterpret_cast<UCHAR*>(zVarDesc), MAX_STR, FIND_FIRST);
			if (ret != DMS_SUCCESS) ret = getTmethod(CENTRAL, &uMethod, reinterpret_cast<UCHAR*>(zVarDesc), MAX_STR, FIND_NEXT);
			strcpy(uVarSect[lvar].mname, reinterpret_cast<char*>(uMethod.TMNAME));
			lvar++;
		}
		ret = getSRVariate(LOCAL, &uSRVariate, FIND_NEXT);
		if (ret != DMS_SUCCESS) ret = getSRVariate(CENTRAL, &uSRVariate, FIND_NEXT);
	}
	//get the variable counts
	lFVTtl = lvar;
	*lVarCnt = lFVTtl - *lFacCnt;
	//unlock the safearray
	lResult = SafeArrayUnlock(*zColNam);
	if (lResult) return (lResult);
	
	//return the variable column data types
	strcpy(zColTyp, zDtype);
	
	//write the VARIATE section headers
	ofDesSht.write("VARIATE", 7); ofDesSht.put('\t');
	ofDesSht.write("DESCRIPTION", 11); ofDesSht.put('\t');
	ofDesSht.write("PROPERTY", 8); ofDesSht.put('\t');
	ofDesSht.write("SCALE", 5); ofDesSht.put('\t');
	ofDesSht.write("METHOD", 6); ofDesSht.put('\t');
	ofDesSht.write("DATA TYPE", 9); ofDesSht.put('\t');
	ofDesSht.write("VALUE", 5); ofDesSht.put('\n'); ofDesSht.flush();
	
	//write the VARIATE section entries
	for (lvar = *lFacCnt; lvar < lFVTtl; lvar++){
		getfld(vname, uVarSect[lvar].vname, DMS_VARIATE_NAME);
		ofDesSht.write(vname, strlen(vname)); ofDesSht.put('\t');
		getfld(vdesc, uVarSect[lvar].vdesc, MAX_STR);
		ofDesSht.write(vdesc, strlen(vdesc)); ofDesSht.put('\t');
		getfld(trait, uVarSect[lvar].pname, DMS_TRAIT_NAME);
		ofDesSht.write(trait, strlen(trait)); ofDesSht.put('\t');
		getfld(scale, uVarSect[lvar].sname, DMS_SCALE_NAME);
		ofDesSht.write(scale, strlen(scale)); ofDesSht.put('\t');
		getfld(methd, uVarSect[lvar].mname, DMS_METHOD_NAME);
		ofDesSht.write(methd, strlen(methd)); ofDesSht.put('\t');
		getfld(dtype, uVarSect[lvar].dtype, DMS_DATA_TYPE);
		ofDesSht.write(dtype, strlen(dtype)); ofDesSht.put('\n'); ofDesSht.flush();
	}
	
	//free up allocated memory space
	//delete [] vname; delete [] vdesc; delete [] trait; delete [] scale;
	//delete [] methd; delete [] dtype; delete [] value; delete [] label;
	
	//close the description sheet file
	ofDesSht.close();
	
	//free up allocated memory space
	free(uVarSect);
	free(uCndSect);
	free(uCnsSect);
	
	return WBK_SUCCESS;
}


extern CODBCdbc *_centralDMS,*_localDMS;  //Central and Local Database Connection


LONG getEffectOunitWrbk(LONG represno, LONG *ounitid, LONG fOpt) {
	#define SQL_getEffectOunitWrbk " \
		select distinct OINDEX.OUNITID \
		from OINDEX \
		where (OINDEX.REPRESNO = ?) \
		order by OINDEX.OUNITID asc"
	#define SQL_getEffectOunitWrbkL " \
		select distinct OINDEX.OUNITID \
		from OINDEX \
		where (OINDEX.REPRESNO = ?) \
		order by OINDEX.OUNITID desc"
	
	LONG ret = DMS_SUCCESS;
	static CODBCbindedStmt &central = _centralDMS->BindedStmt(SQL_getEffectOunitWrbk);
	static CODBCbindedStmt &local = _localDMS->BindedStmt(SQL_getEffectOunitWrbkL);
	static LONG trepresno, tounitid;
	static BOOL first_time = TRUE;
	static BOOL fLocal = TRUE;
	
	if (fOpt == FIND_FIRST) {
		if (first_time) {
			BIND(1, tounitid);
			BINDPARAM(1, trepresno);  
			first_time = FALSE;
		}
		trepresno = represno;
		local.Execute();
		central.Execute();
	}
	
	if (fLocal = TRUE) {
		if (local.Fetch()) *ounitid = tounitid;
		else if (local.NoData()) ret = DMS_NO_DATA;
		else ret = DMS_ERROR;
		if (ret == DMS_NO_DATA || ret == DMS_ERROR){
			ret = DMS_SUCCESS;
			if (central.Fetch()) *ounitid = tounitid;
			else if (central.NoData()) ret = DMS_NO_DATA;
			else ret = DMS_ERROR;
			fLocal = FALSE;
		}
	} else {
		if (central.Fetch()) *ounitid = tounitid;
		else if (central.NoData()) ret = DMS_NO_DATA;
		else ret = DMS_ERROR;
	}
	
	return ret;
	#undef SQL_getEffectOunitWrbk
	#undef SQL_getEffectOunitWrbkL
}


LONG getFactorLevel(DMS_OUNITLEVEL &data, LONG fOpt) {
	#define SQL_GETFACTORLEVEL1 \
		"select O.REPRESNO, O.OUNITID, L.LABELID, L.FACTORID, L.LEVELNO,  L.LVALUE \
		from OINDEX O, LEVEL_N L\
		where ((O.LEVELNO = L.LEVELNO) and (O.FACTORID = L.FACTORID) \
		and (O.REPRESNO = ?) and (L.LABELID = ?)) \
		union \
		(select O.REPRESNO, O.OUNITID, L.LABELID, L.FACTORID, L.LEVELNO, L.LVALUE \
		from OINDEX O, LEVEL_C L \
		where ((O.LEVELNO = L.LEVELNO) and (O.FACTORID = L.FACTORID) \
		and (O.REPRESNO = ?) and (L.LABELID = ?))) \
		order by O.OUNITID asc"
	
	#define SQL_GETFACTORLEVEL1_Oracle \
		"select O.REPRESNO, O.OUNITID, L.LABELID, L.FACTORID, L.LEVELNO, {fn convert(L.LVALUE, SQL_VARCHAR)} \
		from OINDEX O, LEVEL_N L\
		where ((O.LEVELNO = L.LEVELNO) and (O.FACTORID = L.FACTORID) \
		and (O.REPRESNO = ?) and (L.LABELID = ?)) \
		union \
		(select O.REPRESNO, O.OUNITID, L.LABELID, L.FACTORID, L.LEVELNO, L.LVALUE \
		from OINDEX O, LEVEL_C L \
		where ((O.LEVELNO = L.LEVELNO) and (O.FACTORID = L.FACTORID) \
		and (O.REPRESNO = ?) and (L.LABELID = ?))) \
		order by O.OUNITID asc"
	
	#define SQL_GETFACTORLEVEL1_MySQL \
		"select REPRESNO, OUNITID, LABELID, LEVEL_N.FACTORID, LEVEL_N.LEVELNO, cast(LVALUE as char) \
		from OINDEX, LEVEL_N \
		where ((OINDEX.LEVELNO = LEVEL_N.LEVELNO) and (OINDEX.FACTORID = LEVEL_N.FACTORID) \
		and (OINDEX.REPRESNO = ?) and (LEVEL_N.LABELID = ?)) \
		union \
		(select REPRESNO, OUNITID, LABELID, LEVEL_C.FACTORID, LEVEL_C.LEVELNO, LVALUE \
		from OINDEX, LEVEL_C \
		where ((OINDEX.LEVELNO = LEVEL_C.LEVELNO) and (OINDEX.FACTORID = LEVEL_C.FACTORID) \
        and (OINDEX.REPRESNO = ?) and (LEVEL_C.LABELID = ?))) \
		order by OINDEX.OUNITID asc"
		
	#define SQL_GETFACTORLEVEL2 \
		"select O.REPRESNO, O.OUNITID, L.LABELID, L.FACTORID, L.LEVELNO, {fn convert(L.LVALUE, SQL_VARCHAR)} \
		from OINDEX O, LEVEL_N L \
		where ((O.LEVELNO = L.LEVELNO) and (O.FACTORID = L.FACTORID) \
		and (O.REPRESNO = ?) and (L.LABELID = ?)) \
		union \
		(select O.REPRESNO, O.OUNITID, L.LABELID, L.FACTORID, L.LEVELNO, L.LVALUE \
		from OINDEX O, LEVEL_C L \
		where ((O.LEVELNO = L.LEVELNO) and (O.FACTORID = L.FACTORID) \
		and (O.REPRESNO = ?) and (L.LABELID = ?))) \
		order by O.OUNITID desc"
	
	#define SQL_GETFACTORLEVEL2_Oracle \
		"select O.REPRESNO, O.OUNITID, L.LABELID, L.FACTORID, L.LEVELNO, {fn convert(L.LVALUE, SQL_VARCHAR)} \
		from OINDEX O, LEVEL_N L \
		where ((O.LEVELNO = L.LEVELNO) and (O.FACTORID = L.FACTORID) \
		and (O.REPRESNO = ?) and (L.LABELID = ?)) \
		union \
		(select O.REPRESNO, O.OUNITID, L.LABELID, L.FACTORID, L.LEVELNO, L.LVALUE \
		from OINDEX O, LEVEL_C L \
		where ((O.LEVELNO = L.LEVELNO) and (O.FACTORID = L.FACTORID) \
		and (O.REPRESNO = ?) and (L.LABELID = ?))) \
		order by O.OUNITID desc"
	
	#define SQL_GETFACTORLEVEL2_MySQL \
		"select REPRESNO, OUNITID, LABELID, LEVEL_N.FACTORID, LEVEL_N.LEVELNO, cast(LVALUE as char) \
		from OINDEX, LEVEL_N \
		where ((OINDEX.LEVELNO = LEVEL_N.LEVELNO) and (OINDEX.FACTORID = LEVEL_N.FACTORID) \
		and (OINDEX.REPRESNO = ?) and (LEVEL_N.LABELID = ?)) \
		union \
		select REPRESNO, OUNITID, LABELID, LEVEL_C.FACTORID, LEVEL_C.LEVELNO, LVALUE \
		from OINDEX, LEVEL_C \
		where ((OINDEX.LEVELNO = LEVEL_C.LEVELNO) and (OINDEX.FACTORID = LEVEL_C.FACTORID) \
		and (OINDEX.REPRESNO = ?) and (LEVEL_C.LABELID = ?)) \
		order by OINDEX.OUNITID desc"
	
	LONG ret = DMS_SUCCESS;
	static CODBCbindedStmt &central = _centralDMS->BindedStmt();
	static CODBCbindedStmt &local = _localDMS->BindedStmt();
	LPCSTR szDB;
    static string central_sql, local_sql;
	static DMS_OUNITLEVEL _data;
	static BOOL first_time = TRUE;
	static BOOL fLocal = TRUE;
	
	if (fOpt == FIND_FIRST) {
		if (first_time) {
			
			szDB = _localDMS->DatabaseName();
			if ((strcmp(szDB, "MySQL") == 0) || (strcmp(szDB, "PostgreSQL") == 0))
				local_sql = SQL_GETFACTORLEVEL2_MySQL;
			else if (strncmp(szDB, "Oracle", 6) == 0)
				local_sql = SQL_GETFACTORLEVEL2_Oracle;
			else
				local_sql = SQL_GETFACTORLEVEL2;
			
			szDB = _centralDMS->DatabaseName();
			if ((strcmp(szDB, "MySQL") == 0) || (strcmp(szDB, "PostgreSQL") == 0))
				central_sql = SQL_GETFACTORLEVEL1_MySQL;
			else if (strncmp(szDB, "Oracle", 6) == 0)
				central_sql = SQL_GETFACTORLEVEL1_Oracle;
			else
				central_sql = SQL_GETFACTORLEVEL1;
			
			central.SetSQLstr(central_sql.c_str());
			local.SetSQLstr(local_sql.c_str());
			
			BINDPARAM(1, _data.REPRESNO);
			BINDPARAM(2, _data.LABELID);
			BINDPARAM(3, _data.REPRESNO);
			BINDPARAM(4, _data.LABELID);
			
			BIND(1, _data.REPRESNO);
			BIND(2, _data.OUNITID);
			BIND(3, _data.LABELID);
			BIND(4, _data.FACTORID);
			BIND(5, _data.LEVELNO);
			BINDS(6, _data.LVALUE, DMS_DATA_VALUE);
			
			first_time = FALSE;
		}
		_data.REPRESNO = data.REPRESNO;
		_data.LABELID = data.LABELID;
		local.Execute();
		central.Execute();
	}
	
	if (fLocal = TRUE) {
		if (local.Fetch()) data = _data;
		else if (local.NoData()) ret = DMS_NO_DATA;
		else ret = DMS_ERROR;
		if (ret == DMS_NO_DATA || ret == DMS_ERROR){
			ret = DMS_SUCCESS;
			if (central.Fetch()) data = _data;
			else if (central.NoData()) ret = DMS_NO_DATA;
			else ret = DMS_ERROR;
			fLocal = FALSE;
		}
	} else {
		if (central.Fetch()) data = _data;
		else if (central.NoData()) ret = DMS_NO_DATA;
		else ret = DMS_ERROR;
	}
	
	return ret;
	#undef SQL_GETFACTORLEVEL1
	#undef SQL_GETFACTORLEVEL2
	#undef SQL_GETFACTORLEVEL1_MySQL
	#undef SQL_GETFACTORLEVEL2_MySQL
	#undef SQL_GETFACTORLEVEL1_Oracle
	#undef SQL_GETFACTORLEVEL2_Oracle
}


LONG getVariateData(DMS_OUNITDATA &data, LONG fOpt) {
	#define SQL_GETVARIATEDATA1 \
		"select REPRESNO, OUNITID, VEFFECT.VARIATID, DVALUE \
		from VEFFECT, DATA_N \
		where ((VEFFECT.VARIATID = DATA_N.VARIATID) \
		and (VEFFECT.REPRESNO = ?) and (DATA_N.VARIATID = ?)) \
		union \
		(select REPRESNO, OUNITID, VEFFECT.VARIATID, DVALUE \
		from VEFFECT, DATA_C \
		where ((VEFFECT.VARIATID = DATA_C.VARIATID) \
		and (VEFFECT.REPRESNO = ?) and (DATA_C.VARIATID = ?))) \
		order by OUNITID asc"
	
	#define SQL_GETVARIATEDATA1_Oracle \
		"select V.REPRESNO, D.OUNITID, V.VARIATID, {fn convert(D.DVALUE, SQL_VARCHAR)} \
		from VEFFECT V, DATA_N D \
		where ((V.VARIATID = D.VARIATID) \
		and (V.REPRESNO = ?) and (D.VARIATID = ?)) \
		union \
		select V.REPRESNO, D.OUNITID, V.VARIATID, D.DVALUE \
		from VEFFECT V, DATA_C D \
		where ((V.VARIATID = D.VARIATID) \
		and (V.REPRESNO = ?) and (D.VARIATID = ?)) \
		order by OUNITID asc"
	
	#define SQL_GETVARIATEDATA1_Postgres \
		"select V.REPRESNO, D.OUNITID, V.VARIATID, cast(D.DVALUE as char) \
		from VEFFECT V, DATA_N D \
		where ((V.VARIATID = D.VARIATID) \
		and (V.REPRESNO = ?) and (D.VARIATID = ?)) \
		union \
		select V.REPRESNO, D.OUNITID, V.VARIATID, D.DVALUE \
		from VEFFECT V, DATA_C D \
		where ((V.VARIATID = D.VARIATID) \
		and (V.REPRESNO = ?) and (D.VARIATID = ?)) \
		order by OUNITID asc"
	
	#define SQL_GETVARIATEDATA2 \
		"select REPRESNO, OUNITID, VEFFECT.VARIATID, DVALUE \
		from VEFFECT, DATA_N \
		where ((VEFFECT.VARIATID = DATA_N.VARIATID) \
		and (VEFFECT.REPRESNO = ?) and (DATA_N.VARIATID = ?)) \
		union \
		(select REPRESNO, OUNITID, VEFFECT.VARIATID, DVALUE \
		from VEFFECT, DATA_C \
		where ((VEFFECT.VARIATID = DATA_C.VARIATID) \
		and (VEFFECT.REPRESNO = ?) and (DATA_C.VARIATID = ?))) \
		order by OUNITID desc"
	
	#define SQL_GETVARIATEDATA2_Oracle \
	    "select V.REPRESNO, D.OUNITID, V.VARIATID, {fn convert(D.DVALUE, SQL_VARCHAR)} \
		from VEFFECT V, DATA_N D \
		where ((V.VARIATID = D.VARIATID) \
		and (V.REPRESNO = ?) and (D.VARIATID = ?)) \
		union \
		select V.REPRESNO, D.OUNITID, V.VARIATID, D.DVALUE \
		from VEFFECT V, DATA_C D \
		where ((V.VARIATID = D.VARIATID) \
		and (V.REPRESNO = ?) and (D.VARIATID = ?)) \
		order by OUNITID desc"
	
	#define SQL_GETVARIATEDATA2_Postgres \
		"select V.REPRESNO, D.OUNITID, V.VARIATID, cast(D.DVALUE as char) \
		from VEFFECT V, DATA_N D \
		where ((V.VARIATID = D.VARIATID) \
		and (V.REPRESNO = ?) and (D.VARIATID = ?)) \
		union \
		select V.REPRESNO, D.OUNITID, V.VARIATID, D.DVALUE \
		from VEFFECT V, DATA_C D \
		where ((V.VARIATID = D.VARIATID) \
		and (V.REPRESNO = ?) and (D.VARIATID = ?)) \
		order by OUNITID desc"
	
	LONG ret = DMS_SUCCESS;
	static CODBCbindedStmt &central = _centralDMS->BindedStmt();;
	static CODBCbindedStmt &local = _localDMS->BindedStmt();
	LPCSTR szDB;
    static string central_sql, local_sql;
	static DMS_OUNITDATA _data;
	static BOOL first_time = TRUE;
	static BOOL fLocal = TRUE;
	
	if (fOpt == FIND_FIRST) {
		if (first_time) {
			
			szDB = _localDMS->DatabaseName();
			if (strncmp(szDB, "Oracle", 6) == 0)
				local_sql = SQL_GETVARIATEDATA2_Oracle;
			else if ((strcmp(szDB, "MySQL") == 0) || (strcmp(szDB, "PostgreSQL") == 0))
				local_sql = SQL_GETVARIATEDATA2_Postgres;
			else
				local_sql = SQL_GETVARIATEDATA2;
			
			szDB = _centralDMS->DatabaseName();
			if (strncmp(szDB, "Oracle", 6) == 0)
				central_sql = SQL_GETVARIATEDATA1_Oracle;
			else if ((strcmp(szDB, "MySQL") == 0) || (strcmp(szDB, "PostgreSQL") == 0))
				central_sql = SQL_GETVARIATEDATA1_Postgres;
			else
				central_sql = SQL_GETVARIATEDATA1;
			
			central.SetSQLstr(central_sql.c_str());
			local.SetSQLstr(local_sql.c_str());
			
			BINDPARAM(1, _data.REPRESNO);
			BINDPARAM(2, _data.VARIATID);
			BINDPARAM(3, _data.REPRESNO);
			BINDPARAM(4, _data.VARIATID);
			
			BIND(1, _data.REPRESNO);
			BIND(2, _data.OUNITID);
			BIND(3, _data.VARIATID);
			BINDS(4, _data.DVALUE, DMS_DATA_VALUE);
			
			first_time = FALSE;
		}
		_data.REPRESNO = data.REPRESNO;
		_data.VARIATID = data.VARIATID;
		local.Execute();
		central.Execute();
	}
	
	if (fLocal = TRUE) {
		if (local.Fetch()) data = _data;
		else if (local.NoData()) ret = DMS_NO_DATA;
		else ret = DMS_ERROR;
		if (ret == DMS_NO_DATA || ret == DMS_ERROR){
			ret = DMS_SUCCESS;
			if (central.Fetch()) data = _data;
			else if (central.NoData()) ret = DMS_NO_DATA;
			else ret = DMS_ERROR;
			fLocal = FALSE;
		}
	} else {
		if (central.Fetch()) data = _data;
		else if (central.NoData()) ret = DMS_NO_DATA;
		else ret = DMS_ERROR;
	}
	
	return ret;
	#undef SQL_GETVARIATEDATA1
	#undef SQL_GETVARIATEDATA2
	#undef SQL_GETVARIATEDATA1_Oracle
	#undef SQL_GETVARIATEDATA2_Oracle
	#undef SQL_GETVARIATEDATA1_Postgres
	#undef SQL_GETVARIATEDATA2_Postgres
}


LONG DLL_INTERFACE RetrieveCol(long lStdID, long lRepNo, LPSAFEARRAY FAR *zColNam, long lFacCnt, long lVarCnt, long *lVarID, long *lObsCnt, long lWtHis){
	
	char tmpDir[255], wrkDir[255];
	GetEnvironmentVariable("TEMP", tmpDir, 255);
	strcat(tmpDir, "\\");
	
	//open the changes registry file to start logging changes
	GMS_Changes* uChanges;
	string strdate, strtime;
	char *zVarDesc = new char[255], *vdesc = new char[255];
	long *lOunitID = new long;
	long ret2, l1stOunitID;
	ret2 = getEffectOunitWrbk(lRepNo, lOunitID, FIND_FIRST);
	l1stOunitID = abs(*lOunitID);
	ChaLog chaRep(tmpDir);
	
	//create an OBSERVATION SHEET file for every variate column
	fstream ofObsSht[255];
	string zExt = ".txt";
	char *vname = new char[DMS_FACTOR_NAME], *value = new char[DMS_DATA_VALUE];
	BSTR *pzColNam;
	HRESULT lResult;
	lResult = SafeArrayLock(*zColNam);
	if( lResult) return (lResult);
	pzColNam = (BSTR*) (*zColNam)->pvData;
	DMS_OUNITLEVEL uOunitLevel; DMS_OUNITDATA uOunitData;
	
	//get the study factor ID
	DMS_FACTOR uFactor;
	long ret, lStdFID;
	uFactor.STUDYID = lStdID;
	strcpy(uFactor.FNAME, "STUDY");
	ret = findFactorEq(LOCAL, "STUDY", uFactor, FIND_FIRST);
	if (ret != DMS_SUCCESS) ret = findFactorEq(CENTRAL, "STUDY", uFactor, FIND_NEXT);
	lStdFID = uFactor.FACTORID;
	
	//get the first ounitid
	long lvar, lnul, l1stOUID, lLstOUID, lObsIdx;
	uOunitLevel.REPRESNO = lRepNo;
	uOunitLevel.LABELID = lVarID[0];
	if (getFactorLevel(uOunitLevel, FIND_FIRST) == DMS_SUCCESS) l1stOUID = uOunitLevel.OUNITID;
	
	for(lvar = 0; lvar < lFacCnt + lVarCnt; lvar++) {
		strcpy(wrkDir, tmpDir);
		ofObsSht[lvar].open((strcat(wrkDir, OBSCOL) + ltos(lvar) + zExt.c_str()).c_str(), ios::out);
		if (!(!ofObsSht[lvar])) {
			
			//write the FACTOR/VARIATE column header
			getfld(vname, reinterpret_cast<char*>(pzColNam[lvar]), DMS_FACTOR_NAME);
			ofObsSht[lvar].write(vname, strlen(vname)); ofObsSht[lvar].put('\n'); ofObsSht[lvar].flush();
			
			lLstOUID = l1stOUID - 1;
			if (lvar < lFacCnt) {
				//get all lvalue for the current represno & labelid
				lObsIdx = 0;
				uOunitLevel.REPRESNO = lRepNo;
				uOunitLevel.LABELID = lVarID[lvar];
				ret = getFactorLevel(uOunitLevel, FIND_FIRST);
				while (ret == DMS_SUCCESS){
					if (uOunitLevel.FACTORID != lStdFID){
						for(lnul = lLstOUID; lnul < uOunitLevel.OUNITID - 1; lnul++){
							ofObsSht[lvar].put('\n'); ofObsSht[lvar].flush();
							lObsIdx++;
						}
						getfld(value, uOunitLevel.LVALUE, DMS_DATA_VALUE);
						ofObsSht[lvar].write(value, strlen(value)); ofObsSht[lvar].put('\n'); ofObsSht[lvar].flush();
						lObsIdx++;
						lLstOUID = uOunitLevel.OUNITID;
					}
					ret = getFactorLevel(uOunitLevel, FIND_NEXT);
				}
			} else {
				//get all dvalue for the current represno & variatid
				uOunitData.REPRESNO = lRepNo;
				uOunitData.VARIATID = lVarID[lvar] - 1000;
				ret = getVariateData(uOunitData, FIND_FIRST);
				while (ret == DMS_SUCCESS){
					for(lnul = lLstOUID; lnul < uOunitData.OUNITID - 1; lnul++){
						ofObsSht[lvar].put('\n'); ofObsSht[lvar].flush();
					}
					getfld(value, uOunitData.DVALUE, DMS_DATA_VALUE);
					ofObsSht[lvar].write(value, strlen(value)); ofObsSht[lvar].put('\n'); ofObsSht[lvar].flush();
					//get historical data changes
					if (lWtHis == 1){
						//search the change record in the database
						uChanges->cid = 0;
						uChanges->cuid = 0;
						uChanges->ctable[0] = '\0';
						uChanges->cgroup[0] = '\0';
						strcpy(uChanges->cfield, "DVALUE");
						uChanges->crecord = lStdID;
						uChanges->cfrom = uOunitData.VARIATID;
						uChanges->cto = uOunitData.OUNITID;
						zVarDesc[0] = '\0';
						ret2 = GMS_findChanges2(uChanges, zVarDesc, 255, FIND_FIRST);
						while (ret2 == DMS_SUCCESS) {
							getfld(vdesc, zVarDesc, MAX_STR);
							strdate.assign(ltos(uChanges->cdate));
							strtime.assign(ltos(uChanges->ctime));
							strcpy(value, "[");
							strcat(value, (strdate.substr(4, 2) + \
									"/" + strdate.substr(6, 2) + \
									"/" + strdate.substr(0, 4) + \
									" " + strtime.substr(0, 2) + \
									":" + strtime.substr(2, 2) + \
									"]\n" + vdesc).c_str());
							chaRep.logChange((abs(uOunitData.OUNITID) - l1stOunitID + 2), (lvar + 1), value);							
							zVarDesc[0] = '\0';
							ret2 = GMS_findChanges2(uChanges, zVarDesc, 255, FIND_NEXT);
						}
					}
					lLstOUID = uOunitData.OUNITID;
					ret = getVariateData(uOunitData, FIND_NEXT);
				}
			}
			
		}
		ofObsSht[lvar].close();
	}
	//unlock the safearray
	lResult = SafeArrayUnlock(*zColNam);
	if (lResult) return (lResult);
	
	//reutrn the no. of observations
	*lObsCnt = lObsIdx;
	
	return WBK_SUCCESS;
}
