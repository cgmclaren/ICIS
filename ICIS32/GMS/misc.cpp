/* misc.cpp : Implements the utility functions used by other functions of the ICIS32.DLL
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

#include "misc.h"

extern LONG UACCESS,USERID;
extern CODBCdbc *_central,*_local;  //Central and Local Database Connection
extern stack<char*> _routines_stack;

CGermplasmTree *pTreeNode=NULL,*pTreeWalker=NULL;

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG traverseTree(CGermplasmTree *pNode,LONG level,CHAR* szLevel,
                  stringarray &tree,LONGarray &gids)
{
#define VBAR "| "
#define NBAR "  "
#define NODE "+-"
static first_time=TRUE;
string line;
int i,len;
GMS_Germplasm g;
CGermplasmTree *pGroup,*pTemp;

   if (level<0 ||!pNode)
      return TRUE;
   line="";
   len = strlen(szLevel);

   for (i=0;i<len-1;i++)
      line += (szLevel[i]=='1')?VBAR:NBAR;
   if (len)
      line += NODE;

   g = pNode->Data();
   line +=pNode->Name();
   //tree.Add(line);
   //gids.Add(g.gid);
   tree.push_back(line);
   gids.push_back(g.gid);

   if (pNode->Derivative())
   {
      // More expansion ahead
      if (level || (pNode->Group() && pNode->Group()->Landrace()))
      {
         line="";
         pGroup=pNode->Group();
         if (!pGroup) return TRUE;
         g = pGroup->Data();
         for (i=0;i<len-1;i++) 
            line += (szLevel[i]=='1')?VBAR:NBAR;
         pTemp = pNode->Child();
         if (pTemp && pTemp->Parent(pTemp->Data().gnpgs)==pNode)
            line += NBAR;
         else if (len)
            line +=VBAR;
         line +='>';
         line +=pGroup->Name();
         //tree.Add(line);
         //gids.Add(g.gid);
         tree.push_back(line);
         gids.push_back(g.gid);
         pNode=pGroup;
         if (!g.gpid1)
            return TRUE;

      }
      // About to terminate
      else
      {
         //tree[tree.GetSize()-1]+= " <";
         tree[tree.size()-1]+=" <";
         return TRUE;
      }
   }
   strcat(szLevel,"1");
   g=pNode->Data();
   for (i=1;i<=g.gnpgs;i++)
   {
      if (i==g.gnpgs)
         szLevel[strlen(szLevel)-1]=' ';
      traverseTree(pNode->Parent(i),level-1,szLevel,tree,gids);

   }
   szLevel[strlen(szLevel)-1]='\0';
   return TRUE;
}


LONG DLL_INTERFACE GMS_generateTree(LONG gid, LONG level,CHAR *szBuffer, LONG nszBuffer
                           ,LONG *idList, LONG nidList)
{
CGermplasmTree *pNode;
stringarray tree;
string line;
LONGarray gids;
CHAR *p;

   PUSH(GMS_generateTree);

   ret=GMS_treeBuild(gid,level);

   if (ret==GMS_SUCCESS)
   {
      pNode =pTreeNode;
      CHAR szLevel[1024];
      ZeroMemory(szLevel,sizeof(szLevel));
      ret = traverseTree(pNode,level,szLevel,tree,gids);
      ZeroMemory(szBuffer,nszBuffer);
      p =szBuffer;
      for (unsigned int i=0; i< tree.size();i++)
      {
         line = tree[i];
         if ((p-szBuffer)+line.length()+1 < (UINT)nszBuffer)
         {
            strcpy(p,(LPCSTR)line.c_str());
            p += strlen(p)+1;
            if (i <(UINT)nidList)
               idList[i] = gids[i];
         }
         else
            break;
      }
   }
   POP();
}

LONG DLL_INTERFACE GMS_treeBuild(LONG gid,LONG level)
{
   PUSH(GMS_treeBuild);

   if (!pTreeNode)
      pTreeNode = new CGermplasmTree(NULL,gid);
   else if (pTreeNode->Data().gid!=gid)
   {
      delete pTreeNode;
      pTreeNode = new CGermplasmTree(NULL,gid);
   }
   ret = pTreeNode->Expand(pTreeNode->Data().gid,level);
   pTreeWalker=pTreeNode;

   POP();
}

LONG DLL_INTERFACE GMS_treeDescendant(GMS_Germplasm *germ,LONG fWalk)
{
CGermplasmTree *pNode;
   if (!(pNode=pTreeWalker->Child()))
   {
      if (fWalk)
         pTreeWalker = pNode;
      *germ=pTreeWalker->Data();
      return GMS_SUCCESS;
   }
   return GMS_NO_DATA;
}

LONG DLL_INTERFACE GMS_treeProgenitor(LONG index,GMS_Germplasm *germ,LONG fWalk)
{
CGermplasmTree *pNode;
   if (!(pNode=pTreeWalker->Parent(index)))
   {
      if (fWalk)
         pTreeWalker = pNode;
      *germ=pTreeWalker->Data();
      return GMS_SUCCESS;
   }
   return GMS_NO_DATA;
}

//////////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////////
CGermplasmTree::CGermplasmTree(CGermplasmTree*child,LONG gid)
{
   GMS_Name n;
   CHAR szPref[128];
   LONG ret;
   //map<LONG,CGermplasmTree *>::iterator iFUniqueGerm;

   FChild=child;
   FLandrace = FALSE;
   if (!FChild)
   {
      FLevel=1;
      //FUniqueGerm.clear();
   }
   else if (FChild->Derivative())
      FLevel=child->Level();
   else
      FLevel=child->Level()+1;
   ret = GMS_getGermplasm2(gid, &FGerm,&n,szPref,128);
   FName=szPref;
   //FUniqueGerm[gid]=this;
   if (FGerm.gnpgs < 0)
   {
      if (FGerm.gpid1)
      {
         //if ((iFUniqueGerm=FUniqueGerm.find(FGerm.gpid1))!=FUniqueGerm.end())
         //   FParent.push_back(iFUniqueGerm->second);
         //else
            FParent.push_back(new CGermplasmTree(this,FGerm.gpid1));
      }
      else if (!FGerm.gpid2)
         FLandrace = TRUE;
   }
   else
      for (int i=1;i<=FGerm.gnpgs;i++)
         FParent.push_back(NULL);
}

CGermplasmTree::~CGermplasmTree()
{
   for (unsigned int i=0;i<FParent.size();i++)
      if (FParent[i])// && (iFUniqueGerm=FUniqueGerm.find(FParent[i]->Data().gid))!=FUniqueGerm.end())
      {
         delete FParent[i];
         //FUniqueGerm.erase(iFUniqueGerm);
      }
   FParent.clear();
}


LONG CGermplasmTree::Expand(LONG gid,int level)
{
   LONG ret,pid,i;
   BOOL parentExists=FALSE;
   //map<LONG,CGermplasmTree *>::iterator iFUniqueGerm;

   ret= GMS_SUCCESS;

   if (FGerm.gnpgs < 0 )
   {
      // a landrace no more expansion
      if (!FGerm.gpid1 && !FGerm.gpid2)
      {
         return GMS_SUCCESS;
      }
      else if (FGerm.gpid1)
      {
         ret = FParent[0]->Expand(FGerm.gpid1,level);RETURN_ON_ERROR(ret);
      }
      else
         return GMS_ERROR;
   }
   else 
   {
      if (!level)
         return GMS_SUCCESS;
      for (i=1;i<=FGerm.gnpgs;i++)
      {  
         // Get the ith parent
         if (i==1)
            pid = FGerm.gpid1;
         else if (i==2)
            pid = FGerm.gpid2;
         else
            ret = GMS_getProgenitorID( FGerm.gid, i, &pid);
         if (pid)
         {
            // already found;
            parentExists=TRUE;
            //if ((iFUniqueGerm=FUniqueGerm.find(pid))!=FUniqueGerm.end())
            //   FParent[i-1] = iFUniqueGerm->second;
            //else
               FParent[i-1] = new CGermplasmTree(this,pid);
            ret = FParent[i-1]->Expand(pid,level-1);RETURN_ON_ERROR(ret);
         }
      }
   }
   return ret;
}


CGermplasmTree *CGermplasmTree::Parent(int index)
{
   if (index>0 && (UINT)index <=FParent.size())
      return FParent[index-1];
   else
      return NULL;
}

CGermplasmTree *CGermplasmTree::Group()
{
   if (FGerm.gnpgs < 0 && FGerm.gpid1)
      return FParent[0]->Group();
   else if (!FGerm.gpid1 || (FChild && FChild->Derivative()))
      return this;
   else
      return NULL;
}



//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
void sepStr(string &crossStr,LONG level)
{
   if (level==0)
      crossStr= "";
   else if(level==1)
      crossStr= "/";
   else if(level==2)
      crossStr= "//";
   else if(level==3)
      crossStr= "///";
   else
   {
      char szTemp[10];
      sprintf(szTemp,"/%d/",level);
      crossStr = szTemp;
   }
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG maxSepStr(const string &str)
{
   int i=0,j,n,imax=0;

   i=0;
   imax = 0;
   n = str.length();//str.GetLength();
   while (i<n)
   {
      while (i<n && str[i]!='/') i++;
      if (i==n) break;
      if (!str.compare(i,3,"///"))
      {
         imax = max(imax,3);
         i +=3;
      }
      else if (!str.compare(i,2,"//"))
      {
         imax = max(imax,2);
         i +=2;
      }
      else if (str[i]=='/')
      {
         imax=max(imax,1);
         if (i+1<n && isdigit(str[i+1]))
         {
            j = i+2;
            while (j<n && isdigit(str[j])) j++;
            if (j==n) break;
            if (str[j]=='/')
               imax = max(imax,atoi(str.substr(i+1,j-i-1).c_str()));
            i = j+1;
         }
         else
            i +=1;
      }
   }
   return imax;
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
void genCrossStr(string &crossStr,string str1, string str2,LONG level)
{
   LONG s1,s2,s3,s4;
   string sep;

   s1 = maxSepStr(str1);
   s2 = maxSepStr(str2);
   s3 = max(s1,s2);
   s4 = s3+1;
   //if (strlen(str1)==0)
   if (str1.length()==0)
      crossStr= str2;
   else if (str2.length()==0)
      crossStr = str1;
   else
   {
      sepStr(sep,s4);
      crossStr = str1+sep+str2;
   }
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG crossExp(LONG gid1,LONG gid2, LONG methn, LONG level, LONG ntype,string &crossStr,LONG *idList,LONG idLstSz,BOOL fSearch)
{
LONG dosage,ret;
static i=0;
long recp;
GMS_Germplasm g,g1,g2,g3;  
GMS_Name n,n2;
long ret2;
CHAR szPref[129],szPref2[129],szPref3[129],szDosage[19];
string str1="",str2="";

#define RET_ON_ERROR(ret,str) if ((ret)!=GMS_SUCCESS) {crossStr+=(str);return (ret);}
                  
   if (fSearch==FIND_FIRST){
      i=0;
      if (ntype==0 && level==0){
         crossStr = "Set either ntype or level to non-zero value." ;
         return GMS_ERROR;
      }
   }
   if (gid2==0)
   {
      ret = GMS_getGermplasm2(gid1,&g,&n,szPref,128);
      if (ntype!=0 && n.ntype!=ntype){
         n2.gid=n.gid;
         n2.ntype=ntype;
         n2.nstat=0;
         ret2 = GMS_getName(&n2,szPref3,129,FIND_FIRST);
         if (ret2==GMS_SUCCESS){
            n=n2;
            strcpy(szPref,szPref3);
         }
      }
	  
   }
   else{
      ret=GMS_SUCCESS;
      g.gnpgs=2;
      g.gpid1=gid1;
      g.gpid2=gid2;
      g.methn=methn;
      level=1;
      n.ntype=0;
      methn=0;
      gid2=0;
   }
   if (ret==GMS_SUCCESS){
    if (level==0 ||  (ntype!=0 && n.ntype==ntype)){  
//	  if ((level==0 && ntype==0) || (ntype!=0 && n.ntype==ntype)) {    //AMP: 2009/01/19
         if (isdigit(szPref[0]))
            crossStr =' ';
         crossStr = crossStr +szPref;
         if (idList!=NULL  && idLstSz!=0) {
            if (i <idLstSz)
               idList[i++]= g.gid;
            else
               return GMS_MORE_DATA;
         }
         return GMS_SUCCESS;
      }
      // Special Handling of gnpgs
      else if (g.gnpgs < 0)
      {
         // Derivative
		 if (ntype!=0) {
            //AMP: 2009/01/19 if ntype<>0, loop around the derivative sources for the specified name type else go to the cross
			 if (g.gpid1 == g.gpid2) {  //F2 germplasm
 				return crossExp(g.gpid1,gid2,methn,level,ntype,crossStr,idList,idLstSz,FIND_NEXT);
			 }
			 else {
				 GMS_Germplasm g4;
				 GMS_Name n4;
				 CHAR szPref4[129];
				 LONG gid4;
				 ret = GMS_SUCCESS;
			     ret = GMS_getGermplasm2(g.gpid2,&g4,&n4,szPref4,128);
				 while ( ret == GMS_SUCCESS) {
					if (n4.ntype != ntype ) {
						n4.gid=g4.gid;
						n4.ntype=ntype;
						n4.nstat=0;
						ret2 = GMS_getName(&n4,szPref4,129,FIND_FIRST);
					}
					else ret2 = GMS_SUCCESS;
					if (ret2 == GMS_SUCCESS) {
						if (isdigit(szPref4[0]))
							crossStr =' ';
						crossStr = crossStr + szPref4;
						ret = GMS_SUCCESS;
						break;
					}
					else {
						if ((g4.gpid2 != 0 && g4.gpid2 != g4.gpid1)) {
                            gid4 = g4.gpid2;
 			                ret = GMS_getGermplasm2(gid4,&g4,&n4,szPref4,128);
						}
						else ret = GMS_NO_DATA;
					}
				 } //while
			 }
		 }
		 if ((ret!=GMS_SUCCESS) || (ntype==0)) {
			if (g.gpid1!=0)
			{
				//Check if the Groupd ID is a Cross Germplasm ID
				ret = GMS_getGermplasm2(g.gpid1,&g2,&n,szPref2,128);
				if (ret!=GMS_SUCCESS)
				return ret;
				// It is a Cross then Expand it
				if (g2.gnpgs >=0 )
				return crossExp(g.gpid1,gid2,methn,level,ntype,crossStr,idList,idLstSz,FIND_NEXT);
				// It has Unknown Derivative Method
				else if (g2.methn==GMS_UNKDERIVMETH)
				{
				if (isdigit(szPref2[0]))
					crossStr =' ';
					crossStr = crossStr + szPref2;
				}
				// Otherwise it is a Landrace
				else
				{
				if (isdigit(szPref[0]))
					crossStr =' ';
				//crossStr = crossStr +szPref; //ICIS 5.5.1 and deleted on 08/20/2008
					crossStr = crossStr +szPref2;
				}
			}
			// A Landrace
			else{
				if (isdigit(szPref[0]))
				crossStr =' ';
				crossStr = crossStr +szPref;
				if (idList!=NULL  && idLstSz!=0) {
				if (i <idLstSz)
					idList[i++]= g.gid;
				else
					return GMS_MORE_DATA;
				}
			}
		 } // if (ret!=GMS_SUCCESS)
      }  //else if (g.gnpgs==-1)
      // Mutation
      else if (g.gnpgs==1 && g.gpid1!=0 && g.gpid2==0){
         ret = crossExp(g.gpid1,gid2,methn,level-1,ntype,str1,idList,idLstSz,FIND_NEXT);RETURN_ON_ERROR(ret);
         genCrossStr(crossStr,str1,".",level);
      }
      // More than two parents
      else if (g.gnpgs>2){
         LONG k;
         str2="";
         for (LONG j=2;j<=g.gnpgs;j++)
         {
            ret = GMS_getProgenitorID(g.gid,j,&k);RETURN_ON_ERROR(ret);
            ret = crossExp(k,gid2,methn,0,ntype,str1,idList,idLstSz,FIND_NEXT);RETURN_ON_ERROR(ret);
            str2 += str1+",";
            str1="";
         }
         str2.erase(str2.size()-1,1);
         ret = crossExp(g.gpid1,gid2,methn,level-1,ntype,str1,idList,idLstSz,FIND_NEXT);RETURN_ON_ERROR(ret);
         // No Known Female and Male Parents
         if (g.methn==112 || g.methn==421)  
         {
            crossStr = "["+str1+","+str2+"]";
         }
         else
         {
            str2 ="["+str2+"]";
            genCrossStr(crossStr,str1,str2,level);
         }
      }
      //Unknown Parents of a Cross
      else if (g.gpid1==0 && g.gpid2==0)
         crossStr="?/?";
      //At least one parent is unknown
      else if (g.gpid1==0 || g.gpid2==0 ){
         BOOL g1is0 = (g.gpid1==0);
         g1is0?str1="?":str2="?";
         ret = crossExp(g1is0?g.gpid2:g.gpid1,gid2,methn,level-1,ntype,g1is0?str2:str1,idList,idLstSz,FIND_NEXT);RETURN_ON_ERROR(ret);
         genCrossStr(crossStr,str1,str2,level);
      }
      //BackCross
      else if (g.methn==4 || g.methn==GMS_BACKCROSS){
         //Determine the Dosage
         g3=g;
         dosage=1;
         recp=0;
         //traverse the tree until none of the parents are backcross
         while (g3.methn==4 || g3.methn==GMS_BACKCROSS){
            dosage++;
			if (dosage > 100) {
                sprintf(szPref2,"**Backcross dosage exceeds 100 or there is loop in the data");
                crossStr +=szPref2;
                return GMS_ERROR;
			}
            ret = GMS_getGermplasm(g3.gpid1,&g1);RETURN_ON_ERROR(ret);
            ret = GMS_getGermplasm(g3.gpid2,&g2);RETURN_ON_ERROR(ret);
            // Parent 2 is the recurrent, because g1 is a derivative
            if (g1.gnpgs>0 && g2.gnpgs < 0){
               g3 = g1;
               if (!recp) recp=g2.gid;
            }
            // Parent 1 is the recurrent
            else if (g2.gnpgs>0 && g1.gnpgs < 0){
               g3= g2;
               if (!recp) recp=g1.gid;
            }
            // No derivative
            else if(g1.gnpgs>0 && g2.gnpgs>0){
               //Check Parents of Parent2
               if (g2.gpid1 == g1.gid || g2.gpid2 == g1.gid){
                  // Parent 1 is the Recurrent
                  g3=g2;
                  if (!recp) recp=g1.gid;
               }
               else if (g1.gpid1 == g2.gid || g1.gpid2 == g2.gid){
                  // Parent 2 is the Recurrent
                  g3=g1;
                  if (!recp) recp=g2.gid;
               }
               else{
                  sprintf(szPref2,"**ERROR: %d does not have recurrent parent (Code:0010)",g3.gid);
                  crossStr +=szPref2;
                  return GMS_ERROR;
               }
            }
			// if both parents are derivative or management
			//else if ((g1.gnpgs<0) && (g2.gnpgs <0)) {
			//	if ((g1.methn is management) {
			//		get the F1 of g1;
			//      g3 = g1.F1;
			//	    if (!recp) recp = g2.gid;
			//	}
			//	else if ((g2.methn is management) {
			//		get the F1 of g2;
			//		g3 = g2.F1;
			//	    if (!recp) recp = g1.gid;
			//	}
			//}
				
			else
			{
				sprintf(szPref2,"**ERROR: %d does not have recurrent parent (Code:0011)",g3.gid);
				crossStr +=szPref2;
				return GMS_ERROR;
			}
         }
         itoa(dosage,szDosage,10);
         if (recp==g1.gid)
            g3 = g2;
         else if (recp==g2.gid)
            g3 = g1;
         else{
            sprintf(szPref2,"**ERROR: %d does not have recurrent parent (Code:0012)",g3.gid);
            crossStr +=szPref2;
            return GMS_ERROR;
         }
         ret = GMS_getGermplasm(g3.gpid1,&g1);RETURN_ON_ERROR(ret);
         ret =crossExp(g1.gid,gid2,methn,level-1,ntype,str1,idList,idLstSz,FIND_NEXT);RET_ON_ERROR(ret,str1);

         ret = GMS_getGermplasm(g3.gpid2,&g2);RETURN_ON_ERROR(ret);
         ret =crossExp(g2.gid,gid2,methn,level-1,ntype,str2,idList,idLstSz,FIND_NEXT);RET_ON_ERROR(ret,str2);

         //Female Backcross
         if (recp==g1.gid){
            crossStr = str1; crossStr += '*';crossStr += szDosage;
            str1 =crossStr;
         }
         //Male Backcross
         else if (recp==g2.gid){
            crossStr = szDosage; crossStr +='*';crossStr += str2;
            str2 = crossStr;
         }
         else{
            sprintf(szPref2,"**ERROR: Cannot determine the backcross gender (%d)",g3.gid);
            crossStr +=szPref2;
            return GMS_ERROR;
         }
         dosage++;
         genCrossStr(crossStr,str1,str2,level);
      }
      //Cross 
      else{
         // Expand Further
         if (level==1){
            //Three-way cross (102)
            if (g.methn==102){
               ret = GMS_getGermplasm2(g.gpid1,&g1,&n,szPref2,128);RETURN_ON_ERROR(ret);
               if (g1.gnpgs>=0){
                  ret =crossExp(g.gpid1,gid2,methn,1,ntype,str1,idList,idLstSz,FIND_NEXT);RET_ON_ERROR(ret,str1);
                  ret =crossExp(g.gpid2,gid2,methn,0,ntype,str2,idList,idLstSz,FIND_NEXT);RET_ON_ERROR(ret,str2);
               }
               else{
                  ret =crossExp(g.gpid1,gid2,methn,0,ntype,str1,idList,idLstSz,FIND_NEXT);RET_ON_ERROR(ret,str1);
                  ret =crossExp(g.gpid2,gid2,methn,1,ntype,str2,idList,idLstSz,FIND_NEXT);RET_ON_ERROR(ret,str2);
               }
               genCrossStr(crossStr,str1,str2,1);
               return ret;
            }
            // Double way cross (103) or More Complex
            else if (g.methn==103 ||g.methn==106){
               ret =crossExp(g.gpid1,gid2,methn,1,ntype,str1,idList,idLstSz,FIND_NEXT);RET_ON_ERROR(ret,str1);
               ret =crossExp(g.gpid2,gid2,methn,1,ntype,str2,idList,idLstSz,FIND_NEXT);RET_ON_ERROR(ret,str2);
               genCrossStr(crossStr,str1,str2,1);
               return ret;
            }
         }
         ret = crossExp(g.gpid1,gid2,methn,level-1,ntype,str1 ,idList,idLstSz,FIND_NEXT);RET_ON_ERROR(ret,str1);
         ret = crossExp(g.gpid2,gid2,methn,level-1,ntype,str2 ,idList,idLstSz,FIND_NEXT);RET_ON_ERROR(ret,str2);
		 RETURN_ON_ERROR(ret);
         genCrossStr(crossStr,str1,str2,level);
      }
   }
   else
      return ret;

   return GMS_SUCCESS;
#undef RET_ON_ERROR
}


//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_crossExpansion(LONG gid, LONG level,LONG ntype,CHAR *szBuffer, LONG len,LONG *idList, LONG nidList)
{
   LONG ret= crossExpansion(gid,level,ntype,szBuffer,len,idList,nidList);
   return ret;
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG crossExpansion(LONG gid, LONG level,LONG ntype,CHAR* szBuffer,LONG bufsize,LONG *idList,LONG idLstSz)
{
LONG ret;
string crossStr;
CHAR *p;
                              
  ret = crossExp(gid,0,0,level,ntype,crossStr,idList,idLstSz,FIND_FIRST);
  strncpy(szBuffer,(LPCTSTR)crossStr.c_str(),bufsize-1);
  if ((UINT)bufsize<=crossStr.length())
  {
     szBuffer[bufsize-1]='\0';
     if (ret!= GMS_ERROR)
        ret = GMS_MORE_DATA;
  }
  p=szBuffer;
  while (*p!='\0')
  {
     if (*p=='\\' && *(p+1)==' ')
        MoveMemory(p+1,p,strlen(p));
     p+=1;
  }
  return ret;
}

LONG DLL_INTERFACE GMS_generateCrossName(LONG gid1, LONG gid2, LONG methn,CHAR* szBuffer, LONG nszBuffer)
{ 

LONG ret;
string crossStr;
CHAR *p;
   
  ret = crossExp(gid1,gid2,methn,1,0,crossStr,NULL,0,FIND_FIRST);

  strncpy(szBuffer,crossStr.c_str(),nszBuffer-1);
  if ((UINT)nszBuffer<=crossStr.length())
  {
     szBuffer[nszBuffer-1]='\0';
     if (ret!= GMS_ERROR)
        ret = GMS_MORE_DATA;
  }
  p=szBuffer;
  while (*p!='\0')
  {
     if (*p=='\\' && *(p+1)==' ')
        MoveMemory(p+1,p,strlen(p));
     p+=1;
  }

  return ret;
}



//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_validDate(LONG year,LONG month,LONG day)
{
	// Check Year
	if (year<1800) return GMS_ERROR;
	// Check Month
	switch (month){
		// Missing Month;
	   case 0:
		   if (day!=0) return GMS_ERROR;
		   break;
		//Month with 31th
	   case 1:
	   case 3:
	   case 5:
	   case 7:
	   case 8:
	   case 10:
	   case 12:
		   if (day!=0){
			   if (day>31) return GMS_ERROR;
		   }
		   break;
		//Special Month
	   case 2:
		   //Leap Year
		   if (year % 4==0){
			   if (day>29) return GMS_ERROR;
		   }
		   else
			   if (day>28) return GMS_ERROR;
            break;
		//Month only with 30 daysv
	   case 4:
	   case 6:
	   case 9:
	   case 11:
		   if (day!=0){
			   if (day>30) return GMS_ERROR;
		   }
		   break;
       default: 
		   return GMS_ERROR;
	}
	return GMS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_expandDate(LONG date,LONG *year,LONG *month,LONG *day)
{
	LONG temp;

	*year = date / 10000;
	temp = date % 10000;
	*month = temp / 100;
	*day = temp % 100;
	return GMS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_strToDate(CHAR* szDate)
{
	LONG year,month,day;
   CHAR *p,buffer[5];

	if (strlen(szDate)<10) return 0;
	p = szDate;
    ZeroFill(buffer,5);
    strncpy(buffer,p,4);
	p +=5;
	year = atoi(buffer);
    ZeroFill(buffer,5);
	strncpy(buffer,p,2);
	p +=3;
	month = atoi(buffer);
    ZeroFill(buffer,5);
	strncpy(buffer,p,2);
	day = atoi(buffer);
	return GMS_date(year,month,day);
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_date(LONG year,LONG month,LONG day)
{
    return year*10000+month*100+day;
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_dateToStr(LONG date,CHAR* szDate)
{
LONG year,month,day;
CHAR szYear[5],szMonth[3],szDay[3];

   GMS_expandDate(date,&year,&month,&day);
   sprintf(szYear,"%4d",year);
   sprintf(szMonth,"%2d",month);
   sprintf(szDay,"%2d",day);
   if (sprintf(szDate,"%s-%s-%s",!year?"    ":szYear,!month?"  ":szMonth,!day?"  ":szDay)==10) return GMS_SUCCESS;
   return GMS_ERROR;
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_getDate(void)
{
   struct tm *nt;
   long ltime;
   LONG ldate;
   time( &ltime );
   /* Obtain coordinated universal time: */
   nt = localtime( &ltime );
   ldate = (nt->tm_year+1900)*10000+(nt->tm_mon+1)*100+nt->tm_mday;
   return ldate;
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
LONG DLL_INTERFACE GMS_getTime(void)
{
   struct _timeb timebuffer;
   struct tm *nt;
   LONG ltime;

   _ftime( &timebuffer );
   nt = localtime(&timebuffer.time);
   ltime = nt->tm_hour*1000000+nt->tm_min*10000+nt->tm_sec*100+(int)timebuffer.millitm/10;
   return ltime;
}


CHAR* Trim(CHAR* str)
{
   CHAR* h;
   h = str;
   while (*h==' ')
      h++;
   while (h[strlen(h)-1]==' ')
      h[strlen(h)-1]='\0';
   str=h;
   return str;
}

/********
   Pad spaces to the right until length(s)+1=size   

********/
CHAR* PadRight(CHAR *str, int size) 
{
   int i=strlen(str);
	size--;
   for (;i<size;i++) 
      str[i]=' ';
	str[size]='\0'; 
   return str;
}
void PadNull(CHAR *str, int size) 
{
   int i=strlen(str);
	size--;
   for (i=size;i>0;i--) 
      if ((str[i]=='\0') || (str[i]==' '))
	      str[i]='\0';
      else
	     break;
	str[size]='\0'; 
}


BOOL Changed(BOOL &ol,BOOL ne)
{
   BOOL fret=ol^ne;
   ol=ne;
   return fret;
}

void UpperCase(CHAR *str, int size) 
{
char *p;
int i=0;
    size--;
   /* Reverse case of message. */
   for( p = str; p < str + strlen( str ); p++ )
   {
	   if (i < size) {
		  if (isalpha(*p))   str[i]= toupper( *p ) ;
          i = i + 1;
	   }
	   
   }
   str[size]='\0'; 
}



void replaceString(string szInput, string* szOutput,string szfind, string szReplace, long nszLength)
{
unsigned int i;
string input=szInput,output;
i=input.find(szfind,0);
//i=input.find_first_of(szfind);
while (( i >= 0) &&  (i!=string::npos)){
   input.replace(i,nszLength,szReplace);
   i=input.find(szfind,0);
//   i=input.find_first_of(szfind);
}
*szOutput = input;
}


void DellBlank(string* szInput) {
	string input = *szInput, output;
    long nszInput  = input.length(),i;
    for (i=1;i<=nszInput;i++) {
	   if (input.at(i) != ' ') 
        output.append(input.substr(0,1));

	}
    
}

void spellAbbr(string* szInput, string szAbbr, string szLongName, long nszLength)
{
string input=*szInput,output,szfind=szAbbr,szReplace=szLongName;
input+="~";
szfind=szAbbr+"~";
szReplace=szLongName+"~";
replaceString(input,&output,szfind,szReplace,nszLength+1);
input = output;
szfind=szAbbr+"*";
szReplace=szLongName+"*";
replaceString(input,&output,szfind,szReplace,nszLength+1);
input = output;
szfind=szAbbr+"/";
szReplace=szLongName+"/";
replaceString(input,&output,szfind,szReplace,nszLength+1);
szfind="~";
szReplace="\0";
replaceString(input,&output,szfind,szReplace,1);
*szInput = output;
}



