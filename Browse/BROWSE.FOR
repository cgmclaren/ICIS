C******************************************************* BROWSE
C  PURPOSE:
C  BROWSE, SEARCH, CHANGE AND ANALYSE GMS DATA
C 
C  Author: Graham McLaren
C
C  Copyright (C) 2005 IRRI
C 
C  This program is free software; you can redistribute it and/or
C  modify it under the terms of the GNU General Public License
C  as published by the Free Software Foundation; either version 2
C  of the License, or (at your option) any later version.
C 
C  This program is distributed in the hope that it will be useful,
C  but WITHOUT ANY WARRANTY; without even the implied warranty of
C  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
C  GNU General Public License for more details.
C  You should have received a copy of the GNU General Public License
C  along with this program; if not, write to the Free Software
C  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA.
C
C  UPDATES:
C 5.5.3.11 ---------------------------------------------------------------
C 16/07/12 Set MAXTREE=30000 and MAXLIST=3000 as a working compromize.
C Reverted change 3 in 5.5.3.10 since it also caused a problem in
C deallocatestack.
C 20/07/12 replaced recursive function COP with two functions COPSF and COPCF
C COPSF is the same as COP but is only called when IBTYPE>0 - for SF crops.
C COPCF is for cross fertilizing crops and is called when IBTYPE=0.
C It implements more boundary assumptions:
C  IF(METHN.EQ.002 Unknown Generative) COPZ=0.0 like an F1
C  IF(METHN.EQ.032 Unknown Deravitive) COPZ=1.0 fully inbred
C  IF(METHN.EQ.421 Random Mating) COPZ=-1.0 unrelated
C  IF(METHN.EQ.422 Open Pollination) COPZ=-1.0 unrelated
C  IF(METHN.EQ.551 Landrace population) COPZ=-1.0 unrelated
C  IF(METHN.EQ.553 Landrace Cultivar) COPZ=-1.0 unrelated
C  IF(METHN.EQ.554 Collection Population) COPZ=-1.0 unrelated
C  IF(METHN.EQ.556 Collection Wild) COPZ=-1.0 unrelated
C  IF(METHN.EQ.558 Collection weedy) COPZ=-1.0 unrelated
C 5.5.3.10 ---------------------------------------------------------------
C 09/09/11 A special BROWSE version for calculation of large COP Matrices.
C Dimensions are set for lists of 6010 entries and 59999 progenitors. 
C With large array addressing enabled 4GB can be used for the temporary
C sparse array of intermediate COP values.
C 1. Changed the size of GADDR and DADDR in GMS_TREE to INTEGER*4
C    to allow pedigree trees with more that 32000 entries. The old tree
C    structure using ITREE which is populated in subroutine GETTREE1 
C    still uses I2 addressing and so the limit is set there to 32000.
C    GETTREE1 is used for memglegrams (.MGM).
C 2. Changed the ALLOCATE statement in function allocateElement of module
C    modSparseMatrix to return the STATE of the allocate command which is
C    positive when memory is unavailable. Otherwise the application exits
C    abruptly. Passed the STATE to the calling subroutines which checks for
C    a non zero state and executes an alternative RETURN if found so that 
C    the application terminates in a controlled fashion.
C 3. Modified stackLength increases in function allocateElement to linearly
C    increase stackLength by 10000 elements on each call instead of doubling
C    stackLength on each call to see if this allows larger sparse matrices
C    before triggering an out of memory error. This led to an error in
C    deallocateElement which was avoided by a work-around documented there.
C 4. Corrected the error return from subroutine COP to print a message and 
C    pause instead of just terminating the application. All calls to this
C    return in COP originate from failed calls to addNewElement for COPVALS.
C 5. Added PARAMETERS MAXTREE and MAXLIST to specify the maximum dimentsions
C    of LSTGID, LSTECD and LLST and DEPNDS respectively.
C 6. Changed setting Project>Properties>Linker>System>Enable Large Addresses
C    to YES so that BROWSE can address more than 2GB of dataspace. The limit
C    with this setting seems to be 4GB and this still was not enough to 
C    compute the COP matrix of a list with 6000 entries and 57000 progenitors.
C    Processing aborted at about row 5600 of the matrix with 'out of memory'
C    error.
C 7. Suppressed printing of COP values to the screen for lists over 100
C    entries and supressed printing of trialgular matrices and storage and 
C    inversion of COPMAT for lists over 2000 entries.
C 5.5.3.9 ----------------------------------------------------------------
C 11/09/11 Reverted to 5.5.3.7 and allowed .SET NDATE=0
C 5.5.3.8 ----------------------------------------------------------------
C 08/09/11 Increased dimensions of LSTGID, LSTECD, LLST and DEPNDS to 
C allow lists of up to 10000 entries. This requires COMMON WRK to be 
C large enough for LLST.
C Increased dimensions of ITREE, GTREE and NTREE and the value of MXTREE
C to 10000 to allow large pedigrees. ITREE automatically increases the size 
C of COMMON WRK so that it easily accomodates LLST of size 10000.
C Restricted screen print of COP values for lists over 100 entries. 
C 5.5.3.7 ----------------------------------------------------------------
C 08/06/11 Increased the field width for GID in MGRAM (.mgm)
C 5.5.3.6 ----------------------------------------------------------------
C 08/04/11 Added command .SET MGID=ngid IN nnnn to subroutine SETGERM
C 5.5.3.5 ----------------------------------------------------------------
C 01/04/11 Added INI key GVIZ to obtain the full path to the DOT program
C          of GraphViz. This is passed as variable GVIZPATH in COMMON
C          OPTIONS. If null, the it is assumed that DOT.exe is in the
C          current path, but this does not seem to be the case on WINDOWS 7
C          Eg: C:\Program Files (x86)\Graphviz2.26.3\bin\
C 5.5.3.4 ----------------------------------------------------------------
C 08/02/11 CORRECTED ADDLIST FUNCTION TO FIND THE HIGHEST LISTID AND GIVE
C          THE NEXT ID TO THE ENTRY TO BE ADDED TO THE LIST
C 10/02/11 CORRECTED .OPT LIST TO CORRECTLY TERMINATE WITH A REQUEST FOR
C          THE ID OF THE LIST TO OPEN.
C 10/02/11 ADDED FUNCTION .CXP <gid>,n TO PRODUCE A CROSS EXPANSION FOR 
C          gid TO n LEVELS USING THE BROWSE SUBROUTINE CROSSEXP.
C 5.5.3.3 ----------------------------------------------------------------
C 01/10/10 Added graphical viewer to .COP ,GID1>,<GID2> WITH COMMON TREE
C          NODES HIGHLIGHTED IN RED. REQUIRES 'VIEW' INI KEY.
C          ADDED ARGUMENT LVW TO DRAWTREE SO THAT IT PRINTS TO THE SCREEN
C          AND GRAPH VIEWER WHEN LVW=.TRUE. AND TO THE VIEWER ONLY IF
C          IT IS FALSE.
C 5.5.3.2 ----------------------------------------------------------------
C 10/09/10 Added graphical viewer to .DNB (derivative neighbourhood) and
C          .GRP (group relatives) commands. The focal GID for each command
C          is shown in red, and the group source of all the lines in the 
C          output set is in green. For the .DNB dispaly the edges connecting
C          members to the group source are not shown so that the group
C          source could apper isolated from the entries (if the root of the
C          neighbourhood is a derivative with unknown source). For the .GRP
C          command, entries are connected by green lines to the group source
C          and by black edges to the sources. These commands call subroutine
C          VIEWLST to draw the graphs provided inin key VIWE is not null.
C 06/09/10 Modified viewer output from DRAWTREE to use GIDs for node names
C          and node labels for carrying information about the nodes such as 
C          germplasm names etc. 
C 5.5.3.1 ----------------------------------------------------------------
C 05/09/10 Changed .TRE to allow argument LST in order to print trees for
c          all entries in a list. The command is .TRE <gid>,n for a single
c          tree as before or .TRE LST,n for the list version. 
c          Added capability to plot trees as directed graphs with the 
c          GraphViz 2.26.3 package if it is installed. This also requires
c          an immage viewer. The full path to the immage viewer must be 
c          specified in a new INI key called VIEW=. The command line
c          argument for this viewer must be the name of the .gif file and
c          this is added by BROWSE when it starts the viewer.
c          I am using IrfanView for windows ver 4.27 and the ini key
c          is VIEW=C:\Program Files\IrfanView\i_view32.exe.
c          IF this ini key is missing, .tre only produces line graphs as before,
c          but the LST version is available for that also.
c          This required many changes to .TRE, and writing a new DRAWTREE subroutine,
c          the old function is available through command .tr2 using the old subroutine
c          now called DRAWTREE2, (the first version is still available through .tr1).
c          The graph view for .TRE LST,n is a single graph rooted on the list name.
c          Each entry, up to 20, is given a distinct color. After 20 they are all 
c          light grey. Nodes which occur in more than one entry are colored red.
c          So to see the intersection of two pedigrees, make a list with two entries,
c          open the list in BROWSE and use .TRE LST,n (for n generations). The red 
c          nodes indicate the intersection points of the trees. The branches above
c          these nodes are also common, but I could not work out how to find them 
c          to color them red.
c          Added argument <locid> to .MGM which stops expansion of the Mendlegram
c          whenever a gid with GLOCN=<locid> is encountered. The total contribution
c          of all terminal points with GLOCN=<locid> is printed. 
C 5.5.3.0 ----------------------------------------------------------------
C 02/09/10 Added command .PTR to plot a pedigree tree as a directed graph
C          using the .DOT format of the GraphViz plotting package. The
C          .dot file is prepared in subroutine PlotTREE.
C 5.5.2.0 ----------------------------------------------------------------
C 09/07/10 Added option NMSTD to supress name standardization (NMSTD=1)
C 5.5.1.6 ----------------------------------------------------------------
C 21/05/10 Added default REFID to all GMS_addChanges Calls
C 5.5.1.5 ----------------------------------------------------------------
C 13/02/10  Increased field width for printing GIDs to 9 characters.
C 12/11/09  INCREASED FIELDWIDTH FOR GID IN FORMAT 116
C 12/11/09  Had to change the file type of .rc files to "Resource Script"
C           before Visual studio would recognize the resource file.
C 5.5.1.3 ------------------------------------------------------------------
C 20/10/08
C Changes: Modified COP to assign 1 for inbreeding coefficient of DH lines
C           ADDED COMPONENTS %ORDER AND %EXORD TO THE GMS_TREE DATA
C           STRUCTURE TO STORE THE CROSS ORDER AND THE EXTRACTION ORDER
C           OF TREE ELEMENTS RESPECIVELY. CHANGED DIMENSION OF GTREE,
C           ITREE AND RTREE TO ACCOMODATE THE NEW STRUCTURE.
C           MODIFIED FUNCTION COP TO CHECK %METHOD FOR DH METHOD
C           WHEN COMPUTING INBREEDING COEFFICIENT OF LINES AND
C           ASSIGN 1 WHEN FOUND.
C           CLEANED UP SUBROUTINE LCOPI TO USE THE NEW TRREE STRUCTURE
C           WHEN OUTPUTING A TREE IN OFFSPRING, SIRE, DAM FORMAT.
C  19/10/08 Update to Intel FORTRAN 10.1
C  5.5.1.2 ----------------------------------------------------------------
C  29/06/08 Increased output field width for LISTID in SUBROUTINE SEARCHLISTS        
C  29/06/08 Increased size of LISTDATA.SOURCE to 256
C  5.5.1.1 ---------------------------------------------------------------- 
C  09/05/08 MODIFIED DEFAULTS FOR INBREEDING IN COP CALCULATIONS
C           CHANGED OPTION FLAG BTYPE FROM REAL TO INTEGER WITH VALUES:
C  BTYPE=1 FOR SF CROPS ASSUMING FULL INBREEDING FOR LINES WITH UNKNOWN SOURCE
C  BTYPE=2 FOR SF CROPS ASSUMING F4 FOR LINES WITH UNKNOWN SOURCE BUT KNOWN PARENTS
C  BTYPE=0 FOR CF CROPS
C  5.4.2.11 ---------------------------------------------------------------
C  09/05/08 Increased dimension of UDFLDS buffer (UDFNO and UDFCD)
C  5.4.2.10 ----------------------------------------------------------------
C  24/04/08 INCREASED LENGTH OF NAME VARIABLES FROM 80 TO 256 TO CATER FOR 
C           MAIZE PEDIGREES. INCREASED VARIABLE SIZE IN NAMLST AND FCROSS1,
C           BUT NOT IN NAMLST1 WHICH IS ONLY CALLED FROM ADDCROSS WHICH IS
C           NOT USED AT ALL AT THE MOMENT. 
C  24/04/08 Added NUID to output of NAMES in WRTGERM.
C  5.4.2.7 ----------------------------------------------------------------
C  18/04/08 A list of initial option settings is now printed to the log file
C           and missing fields from the germplasm details have been added to
C           the log file. Some change error messages have been logged, but those
C           requiring an interactive response are not logged.
C  18/04/08 SETGPID2 CHANGED TO DETECT CREATION OF ONE-STEP CIRCULARITIES
C  5.4.2.6 ----------------------------------------------------------------
C  15/04/08 FIXED SOME SPACE BUGS IN SEARCHLISTS AND ADDED ERROR DIAGNOSTICS
C           REMOVED AUTOMATIC LIST SEARCH FROM WRITEGERM SO USER MUST USE .SLS
C  5.4.2.4 ----------------------------------------------------------------
C  21/02/08 ADDED COMMAND .ICP LST <LVL> TO EXPORT THE TREE FOR A LIST
C           USING OFFSPRING DAM SIRE FORMAT.
C  11/02/08 Added argument LVL to commands .COP LST and .EXT LST so that
C           .EXT LST <lvl> will extract the combined pedigree tree for the
C           open list up to lvl generative generations only and 
C           .COP LST <lvl> computes the COP matrix using pedigrees over 
C           the last lvl generative generations only.
C           This involved setting GADDR to 0 in ancestors with incomplete 
C           pedigree expansion at line 25-2 in SUBROUTINE LCOP and skipping
C           the computation of coefficient of inbreeding for non expanded
C           common group sources at line 5-5 in FUNCTION COP.
C  5.4.2.2 ----------------------------------------------------------------
C  25/10/07 ADDED FUNCTION .DNB TO RETURN A DERIVATIVE NEIGHBORHOOD
C  Older Releases ---------------------------------------------------------
C  21/08/98 ADDED SOURCE TO WRTGERM
C  21/08/98 ADDED .SET GPID1 FUNCTION
C  25/08/98 ADDED .SET GPID2 FUNCTION
C  08/09/98 ADDED .OPT IOPT=x OPTION
C           x=0 THEN USER MUST SELECT MATCH IN ALL CASES
C           x=1 TO SELECT A SINGLE MATCH AUTOMATICALLY
C           x=2 NO SELECTION, HIT LIST LOGED TO BROWSE.LOG
C  10/01/99 ADDED .SET METHN, GDATE, GLOCN OPTIONS
C  12/01/99 ADDED .LST #,CODE,SOURCE,DESIG FUNCTION
C  10/04/99 ADDED .HAS FUNCTION TO CONVERT A GENERATIVE GERMPLASM
C           TO UNKNOWN DERIVATIVE SOURCES OF AN UNMAMED CROSS
C  19/04/99 ADDED .OPT ISRH=x OPTION TO NAMLST
C           x=0 TO LIST ALL MATCHES
C           x=1 TO LIST ONLY GENERATIVE GERMPLASM
C           x=2 TO LIST ONLY DERIVATIVE GERMPLASM
C           x=3 FOR GERMPLASM WITHOUT ACCESSION NAMES
C           x=4 FOR GERMPLASM WITH ACCESSION NAMES
C  26/04/99 ADDED GETTREE1 SUBROUTINE TO RETRIEVE TREES
C  27/04/99 ADDED DRAWTREE1 FUNCTION TO DISTPAY TREES
C  27/04/99 ADDED .MGM FUNCTION TO DISPLAY MENDELGRAM
C  27/04/99 ADDED " SEARCH STRING TO REPEAT LAST SEARCH
C  09/05/99 ADDED .COP FUNCTION
C  09/05/99 CHANGED .TRE TO USE GETTREE1 AND DRAWTREE1
C  01/08/99 ADDED ARGUMENT NTC TO CROSSEXP TO STOP ON NAME TYPE
C  31/08/99 ADDED RICHNESS INDEX TO .MGM
C  01/09/99 ADDED .TCE FUNCTION WITH TRACE SUBROUTINE
C  03/09/99 ADDED .SAV FUNCTION
C  30/09/99 FIXED HASFIX SINCE NAMLST DELETES UNFOUND NAMES
C           FROM THE INPUT STRING
C  21/11/99 ADDED .NOTE #,<value> IF #, IS ABSENT THEN
C           THE CURRENT GID IS USED AND <value> MAY NOT
C           START WITH A NUMBER.
C  21/11/99 ADDED .OPT SETTINGS FOR REFID AND LOCID
C  21/11/99 ADDED .NAME FUNCTION WITH FORMAT
C           .NAME <gid>,<ntype>,<nlocn>,<ndate>,<nref>,<nval>
C           IF GID IS ZERO OR NULL THE CURRENT GID IS USED
C  02/12/99 ADDED .LOC FOR FIND LOCATION NUMBER
C  01/03/00 CORRECTED REPLACE AT LINE LREPLACE 109-2
C  15/03/00 ADDED .FCR FUNCTION TO FIND CROSS COMBINATIONS
C  17/03/00 ADDED .GRP FUNCTION TO FIND GROUP RELATIVES
C  20/12/01 CORRECTED SUBROUTINE LREPLACE BY ADDING LINE 108+12
C  08/01/02 COP MODIFIED TO ACCOMODATE CROSS POLINATED CROPS
C  OPTION BTYPE=1 FOR SELF POLINATED, 0 FOR CROSS POLINATED
C  15/01/02 NEW GETTREE DEVELOPED TO RECORD INTERMEDIATE
C  DERIVATIVE STEPS, ICOP MODIFIED TO USE NEW TREE AND
C  COMPUTE COPS BETWEEN SISTER LINES. DRAWTREE MODIFIED
C  TO USE NEW TREE AND ALLOW DISPLAY OF INTERMEDIATE DERIVATIVES
C  WHEN OPTION LDST=1 (OTHERWISE LDST=0)
C  TAB AND MGM STILL USE THE OLD TREE (GETTREE1) AND THE
C  OLD TREE MAY STILL BE DISPLAYED WITH COMMAND .TR1.
C  25/01/2002 ADDED MORE HELP LINES AND MADE THE .SET COMMAND
C  CASE INSENSITIVE
C  05/02/02 ADDED NEW TABTREE SUBROUTINE TO TABULATE TREE ENTRIES.
C  THE OLD ROUTINE (TABTREE1) IS STILL ACCESSIBLE THROUGH .OTB
C  06/02/02 PRINT OUT OPTION VALUES WITH OPTION HELP (.OPT)
C  20/02/02 FIXED PROBLEM IN COP WITH SISTER LINES FROM A MUTANT
C  05/03/02 CHANGED LREPLACE TO SET THE REPLACEMENT GID AS THE
C  CURRENT GID AT THE END OF THE REPLACEMENT
C  04/02/03 ADDED FLAG IYAL TO ACCEPT ALL CHANGES AUTOMATICALLY
C  28/10/03 ADDED OPTIONS IOPT=2 AND ISRH=4
C  13/11/03 ADDED SET GNPGS FUNCTION
C  14/11/03 ADDED SET NDATE,NLOCN,NREFN,NTYPE,NSTAT FUNCTIONS
C  23/01/04 ADDED LIST NAMES TO THE .OPT LIST OUTPUT
C  16/03/04 ADDED GENERATION CODE FUNCTION .GCD
C  28/04/04 ADDED ENCRYPTION FUNCTION .ENC THIS FUNCTION
C           TAKES STRING INPUT AFTER THE '.ENC ' COMMAND
C           AND RETURNS THE 10-BYTE ENCRIPTION STRING.
C           IT IS ONLY ACCESSIBLE TO ICIS USERS WITH ACCESS
C           PRIVELAGE 150
C  28/04/04 ALLOWED ENCRYPTED USERNAME AND/OR PWD
C  01/05/04 MADE MAKELIST CYCLE FOR UNIQUE LIST NAMES
C           AND SET TRACE TO USE MAKELIST.
C           LOGGED ALL COMMANDS TO BROWSE.LOG
C  05/05/04 Added .ADU function to add users to a local
c           installation. the Syntax is:
c           .ADU Uname,Pwd,<security>,<access>,<type>
c           The last three fields may be blank for defaults
c           Only local and central administrators may set security=2
c           Only central administrators may set security=3
C  07/05/04 ADDED .REF FUNCTION AND OUTPUT REFERENCE IN WRTGERM
C  07/05/04 ADDED A FUNCTION TO GET SECOND COMMAND LINE ARGUMENT 
C           WHICH CAN BE A COMMAND OR NAME TO SEARCH
C           EG BROWSE " " .GID100. FIRST ARGUMENT MAY BE AN INI
C           FILE: BROWSE D:\ICIS4\EXES\ADMIN.INI .GID100
C           IF AN ARGUMENT SHOULD CONTAIN A SPACE, ENCLOSE IT
C           IN DOUBLE QUOTES EG: BROWSE " " ".GID 50533"
c           OR BROWSE D:\ICIS4\EXES\YANCO.INI ".INP D:\ICIS4\LIST14.TXT"
C  07/05/04 ADDED ERROR DESCRIPTIONS FOR UNSUCCESSFUL LOG ON
C  01/06/04 CORRECTED BACKCROSS PROCESSING IN HASFIX
C  04/06/04 CORRECTED DETECTION OF ILLEGAL GROUP IN SET GPID1
C  08/06/04 ADDED IBEL OPTION TO SILENCE WARNING BELL
C  16/08/04 ADDED .CID COMMAND TO FIND CID-SID
C           The DLL checks the GERMPLSM table in the current LOCAL database
C           to see if it has fields called CID and SID. If so, it assumes the
C           central also has these fields and it enbles function GMS_findCIDSID.
C           BROWSE uses this function to:
C           1) Display the CID-SID for any germplasm record that is displayed
C              via subroutine WRTGERM.
C           2) Implement a new command .CID xxx,[yyy] where xxx is any CID value
C              and, optionally, yyy is any SID value. .CID returns a list of
C              germplasm having CID=xxx and SID=yyy. There will be at most one
C              hit if both xxx and yyy are specified, but there may be many hits
C              if only xxx is specified.
C  01/02/05 Corrected sequential update of list names in subroutine MAKELIST
C  05/03/05 Added .DSL <listid> function to display list entries. If <listid>
C           is null entries for the current open list are displayed, otherwise
C           list <listid> is opened and entries displayed.
C  05/03/05 Added confirmation of .LST function
C  05/03/05 Added ZEROLSTNM subroutine to clear LISTNAME structure before 
C           calls getListName with fopt=0. 
C  05/03/05 Updated SUBROUTINE ADDLIST to allow adding of list entries only for
C           lists with LISTSATAUS=1
C  04/03/05 Corrected sequential access to different lists by setting
C           new field Listdata%LRECID=0 before calls to getListData.
C  05/03/05 Added subroutine GIV to compute the inverse or g-inverse of
C           a COP matrix produced with the command .COP LST.
C           If the cop matrix is non singilar the matrix and its inverse
C           are printed to file BROWSE.LOG. If it is singular (eigen 
C           values < 1.0e-9) then the G-inverse is computed and printed.
C  06/03/05 Corrected all GMS_addListData calls which now require LRECID=0 
C           and ENTRYID=0 before the call so that these are set by the DLL
C  18/03/05 Supressed case change of input for USERNAME and PASSWORD.
C  26/04/05 Added field MGID to GERMPLSM structure and command .M
C  20/05/05 Corrected error in findGermplasm by initializing
C  19/10/05 Increased dimension of UDFLDS buffer (UDFNO and UDFCD)
C  27/05/06 CHANGED COP PROCESSING FOR LISTS. ALL PEDIGREES ARE NOW
C           FETCHED BEFORE COP PROCESSING BY SUBROUTINE LCOP.
C           A MAXIMUN OF 20000 DISTINCT LINES MAY BE PRESENT IN THE
C           COMPLETE PEDIGREE TREE FOR ALL GERMPLASM IN THE LIST
C           (DIMENSION OF GTREE, RTREE, ITREE, NTREE and value of MXTREE)
C           SURPRISINGLY THIS DOES NOT SEEM TO SPEED UP PROCESSING.
C  28/05/06 PRINT OUT LEADING EIGEN VECTORS OF THE COP MATRIX LABELED
C           WITH A COLUMN OF GIDS FOR ORDINATION PLOTS.
C  01/06/06 ADDED SPARSE MATRIX COPVALS WHICH STORES INTERMEDIATE
C           COP VALUES FOR RE-USE IN COMPUTING COPS. THIS SPEEDS
C           UP THE COP CALCULATIONS
C  07/06/06 ADDED COMMAND .EXT TO EXPORT A PEDIGREE TREE FOR A LIST
C           BUT IT IS NOT VERY CLEAR HOW USEFUL THIS WILL BE SINCE
C           THE RELATIONSHIPS ARE LOST IN THE EXPORTED LIST
C  10/07/06 INCREASED BUFFER (MXLST) TO 500 FOR DEPENDENCIES IN 
C           SUBROUTINE LREPLACE AND ADDED CHECKS FOR DEPENDENCIES
C           ALREADY ENTERED IN THE BUFFER.
C  24/09/06 INCREASED PRINT FIELDS FOR GIDS
C  05/12/06 MODIFIED THE '.EXT LST' COMMAND TO EXPORT THE BROWSE
C           TREE STRUCTURE FOR A LIST TO THE LOG FILE
C  05/12/06 MODIFIED .OPT LIST TO PRESENT LISTS BY PAGES
C  05/12/06 ADDED COMMAND .MLS TO DISPLAY MLS ANCESTORS OF A GID
C  22/01/07 UPDATED TO FORTRAN 9
C  02/02/07 USED IMSL TO COMPUTE THE EIGEN STRUCTURE OF COP MATRICES
C           IN SUBROUTINE GIV AND ADDED A RANK TWO ORDINATION PLOT
C  16/02/07 USED VARIABLE DIMENSION IN DDPNDS, ALLOW PASSAGE THROUGH
C           THE ILLEGAL GROUP TRAP IN DDPNDS AT STATEMENT 11 -2 AND
C           CORRECTED ERROR IN PORT OF GETDATM
C  16/02/07 SET MAXIMUM DEPENDENCIES IN DDPNDS TO 2000
C  17/02/07 Corrected error in WRTGERM
C  13/03/07 Changed inbreeding assumption for lines with unknown source
C           to F4 (COI=15/16) from homozygosity (COI=1) at lines
C           1-7 and 5-7 in SUBROUTINE COP.
C  17/05/07 Added Change UID to print of changes in WRTGERM
C  30/05/07 Increased Country buffer to 400 (LCN and LCNM)
C  05/06/07 Trapped cases with GNPGS=0 but GPID1<>0 in GETTREE.
C           These are illegal cases but a bug in DLL allows a
C           CHANGE record to set GNPGS for a local variable.
C  05/06/07 Corrected DRAWTREE for cases with unknown parents
C           which were causing illegal addressing in GTREE
C  27/06/07 Changed maximum dependencies in LREPLACE to 2000
C  09/07/07 Corrected a bug in CUTCROSS which did not return
C           IER=0 if no cross was found.
C  10/07/07 Added code in FCROSS to search for backcrosses in
C           either order: A*2/B = A//A/B or A/B//A
C  10/07/07 Found .FCR hanging after repeated use or after 
C           complicated searches. It appears that the recursion
C           is sapping resources and not releasing them.
C           First tried making SUBROUTINE FPARENTS recursive as
C           well as FCROSS since it is called by FCROSS, but this 
C           did not help. Then tried including FPARENTS in FCROSS
C           as SUBROUTINE FCROSS1 but this also does not solve 
C           the problem. BROWSE 5.4.1.11 used FCROSS1 althogh the
C           code for FCROSS and FPARENTS is still in GMSHDR.
C  08/10/07 Changed SETGPID1 to correctly set the source to zero
C           when the new GPID1 has a different group.
C  08/10/07 Changed SETGPID1 to check that the new value is a valid
C           group source and to set the group source of dependencies
C           to the target GID of the new value of GPID1 is zero
C           (making the target a derivative group source).
C  19/10/07 Added .SLS <GID> search list function (SUB SEARCHLISTS)
C  20/10/07 CALL SEARCHLISTS FROM LREPLACE AND ASK USER TO CONFIRM
C           REPLACEMENT IF REPLACED GERMPLASM IS FOUND IN LISTS.
C           IF CONFIRMED, GERMPLASM DETAILS (GID, DESIG AND GROUP) 
C           ARE CHANGED IN LOCAL LISTS TO MISSING IN THE CASE OF 
C           DELETIONS OR TO VALUES FROM THE REPLACEMENT GERMPLASM
C           FOR REPLACEMENTS. CHANGES IN CENTRAL LISTS MUST BE APPLIED
C           FROM GERMPLASM CHANGE RECORDS AT UPDATE AND HENCE WILL ONLY
C           TAKE EFFECT AFTER UPDATE.
C  20/10/07 REFACTORED LREPLACE, SETGERM, SETGPID1, SETGPID2 AND SETNAM
C           TO USE A COMMON CGROUP VALUE AND TO WRITE THE PRIMARY 
C           CHANGE COMMAND TO CDESC.
C------------------------------------------------------- BROWSE
      USE GMS_STRUCTURES
      USE GMS_INTERFACE
      USE GMS_TOOLS
      USE GMS_FUNCTIONS
      USE MSFLIB
      USE DFLIB
      PARAMETER (MAXTREE=30000,MAXLIST=3000)      
      CHARACTER*80 INIFILE,CENDSN,LCLDSN,LASTDIR,INSTLN
      character COLOR*20,COLORS(20)*20
      CHARACTER*256 CLWK,CTMP,CWK,LSTSRCH
      CHARACTER USERNAME*30,PASSWORD*12,RTIME*12,RDATE*12,VIEWER*60
      CHARACTER GVIZPATH*60,ADATE*10,ATIME*10
      CHARACTER GMSERROR(10)*64,ANAME*10000,UDFCD(600)*8,CMD*4,NULL*1
      CHARACTER LCNM(400)*3,MCD(200)*8,BELL*1,ATMP*12,LSTECD(MAXLIST)*48
      DIMENSION RTREE(MAXTREE,5),GTREE(MAXTREE,5),LLST(MAXLIST),IWK(20)
      INTEGER*4 UDFNO(600),FCPS,SECURITY,CID,SID,LCN(400),MIDS(200)
      INTEGER*4 LSTGID(MAXLIST,2)
      INTEGER*2 ITREE(MAXTREE,10),II2,IIS
      REAL*4, ALLOCATABLE :: COPMAT(:)
      TYPE(GMS_GERMPLASM) GERM
      TYPE(GMS_NAMEDATA) NAMDAT
      TYPE(GMS_USER) USER
      TYPE(GMS_INSTALLATION) INST
      TYPE(GMS_LISTNAME) LNAME
      TYPE(GMS_LISTDATA) LDATA
      TYPE(GMS_LOCATION) LOCN
      TYPE(GMS_BIBREFS) BREF
      TYPE(GMS_ATTRIBUTE) ATTB
      TYPE(GMS_TREE) NTREE(MAXTREE)
      COMMON /CODES/ NUDF,UDFNO,UDFCD,NCNTRY,LCN,LCNM,NMTHD,MIDS,
     &MCD
      COMMON /WRK/ NLST,MXLST,ITREE
      COMMON /OPTIONS/ NUID,NLOC,NREF,IOPT,LIST,ISRH,MGAT,NAMP,NTCX
     &,IBTYPE,LDST,IYAL,NMSTD,VIEWER,GVIZPATH
      EQUIVALENCE (IWK(1),CWK),(ITREE(1,1),LLST(1)),(ITREE(1,2),ANAME)
      EQUIVALENCE (ITREE(1,1),RTREE(1,1)),(ITREE(1,1),GTREE(1,1))
      EQUIVALENCE (ITREE(1,1),NTREE(1))
      DATA NULL,BELL /0,7/, NUDF,NCNTRY,NMTHD/0,0,0/
      DATA COLORS /'orange','violet','green','indigo','yellow',
     &'cyan','blue','brown','gold','magenta',
     &'bisque','coral','turquoise','pink','navy',
     &'plum','sienna','tan','tomato','firebrick'/
      DATA GMSERROR /'GMS_ERROR (UNDEFINED)','GMS_NO_ACCESS',
     &'GMS_INVALID_PASSWORD','GMS_INVALID_USER','GMS_NAME_EXISTS',
     &'GMS_NOT_FOUND','GMS_MORE_DATA','GMS_READ_ONLY',
     &'GMS_INVALID_INSTALLATION','GMS_INVALID_DATABASE_STRUCTURE'/
      BELL=G_BELL
      CALL GETDATM(ADATE,ATIME,I,J)
C
C  DISPLAY THE INI FILE
C
      II2=1
      CALL GETARG(II2,CTMP,IIS)
      IF(CTMP.EQ.' ') THEN
      I4=GETENVQQ('temp',CWK)
      CTMP=CWK(1:I4)//'\ICIS.INI'
      ENDIF
      INIFILE=CTMP
      WRITE(0,108) ADATE,ATIME,TRIM(INIFILE)
 108  FORMAT(1X,72(1H*)/
     &' ICIS UTILITY FOR BROWSING AND MODIFYING A GMS DATABASE',
     &' (Version 5,5,3,11)'/1X,52(1H*),1X,A10,':',A10/' INI FILE (',A,')')
      WRITE(0,'(" THE INI FILE CONTAINS THE FOLLOWING RECORDS:")')     
      OPEN(2,FILE=CTMP,STATUS='OLD',ERR=7)
      OPEN(1,FILE='CONIN$')
	OPEN(99,FILE='CONOUT$',CARRIAGECONTROL='FORTRAN')
C
C  SET OPTIONS
C
      IYAL=0
      LSTP=0
      NLOC=0
      NREF=0
      IOPT=0
      LIST=0
      ISRH=0
      MGAT=0
      NAMP=0
      NTCX=0
      FCPS=1
      IBTYPE=1
      LDST=0
      NMSTD=0
      LASTDIR=' '
      USERNAME=' '
      PASSWORD=' '
      VIEWER=' '
      GVIZPATH=' '
C
      LIST=0
 1    READ(2,100,END=2) CWK
 100  FORMAT(A)
      IF(LIST.EQ.1.AND.CWK(1:4).EQ.'DSN=') CENDSN=CWK(1:LEN_TRIM(CWK))
      IF(LIST.EQ.2.AND.CWK(1:4).EQ.'DSN=') LCLDSN=CWK(1:LEN_TRIM(CWK))
      IF(CWK(1:1).EQ."#") GO TO 1
      LSTSRCH=CWK
      CALL CCASE(CWK,1,LEN_TRIM(CWK),1)
      IF(CWK(1:1).EQ.'[') LIST=0
      IF(CWK.EQ.'[CENTRAL GMS]') LIST=1
      IF(CWK.EQ.'[LOCAL GMS]') LIST=2
      IF(CWK.EQ.'[BROWSE]') LIST=3
      IF(LIST.EQ.2.AND.CWK(1:4).EQ.'UID=') THEN
      USERNAME=LSTSRCH(5:)
      ELSE IF(LIST.EQ.2.AND.CWK(1:4).EQ.'PWD=') THEN
      PASSWORD=LSTSRCH(5:)
      ELSE IF(CWK.NE.' '.AND.LIST.GT.0.AND.CWK(1:4).NE.'PWD=')THEN
      WRITE(0,104) CWK(1:LEN_TRIM(CWK))
      ENDIF
 104  FORMAT(1X,A)
      IF(LIST.EQ.3.AND.CWK(1:8).EQ.'LASTDIR=') LASTDIR=CWK(9:)
      L=7
      IF(LIST.EQ.3.AND.CWK(1:6).EQ.'LOCID=') NLOC=INUMBA(IWK,L,80,J,K)
      L=7
      IF(LIST.EQ.3.AND.CWK(1:6).EQ.'REFID=') NREF=INUMBA(IWK,L,80,J,K)
      L=6
      IF(LIST.EQ.3.AND.CWK(1:5).EQ.'IOPT=') IOPT=INUMBA(IWK,L,80,J,K)
      L=6
      IF(LIST.EQ.3.AND.CWK(1:5).EQ.'ISRH=') ISRH=INUMBA(IWK,L,80,J,K)
      L=6
      IF(LIST.EQ.3.AND.CWK(1:5).EQ.'MGAT=') MGAT=INUMBA(IWK,L,80,J,K)
      L=6
      IF(LIST.EQ.3.AND.CWK(1:5).EQ.'NAMP=') NAMP=INUMBA(IWK,L,80,J,K)
      L=6
      IF(LIST.EQ.3.AND.CWK(1:5).EQ.'LDST=') LDST=INUMBA(IWK,L,80,J,K)
      L=6
      IF(LIST.EQ.3.AND.CWK(1:5).EQ.'NTCX=') NTCX=INUMBA(IWK,L,80,J,K)
      L=6
      IF(LIST.EQ.3.AND.CWK(1:5).EQ.'FCPS=') FCPS=INUMBA(IWK,L,80,J,K)
      L=6
      IF(LIST.EQ.3.AND.CWK(1:5).EQ.'IYAL=') IYAL=INUMBA(IWK,L,80,J,K)
      IF(IYAL.NE.1) IYAL=0
      IF(LIST.EQ.3.AND.CWK(1:5).EQ.'VIEW=') VIEWER=CWK(6:)
      IF(LIST.EQ.3.AND.CWK(1:5).EQ.'GVIZ=') GVIZPATH=CWK(6:)
      L=7
C  BTYPE=1 FOR SF CROPS ASSUMING FULL INBREEDING FOR LINES WITH UNKNOWN SOURCE
C  BTYPE=2 FOR SF CROPS ASSUMING F4 FOR LINES WITH UNKNOWN SOURCE BUT KNOWN PARENTS
C  BTYPE=0 FOR CF CROPS
      IF(LIST.EQ.3.AND.CWK(1:6).EQ.'BTYPE=')
     & IBTYPE=INUMBA(IWK,L,80,J,K)
      IF(IBTYPE.NE.0.AND.IBTYPE.NE.1.AND.IBTYPE.NE.2) THEN
      WRITE(0,*) ' *** ERROR *** ILLEGAL BTYPE VALUE 1 ASSUMED'
      IBTYPE=1
      ENDIF
      L=7
      IF(LIST.EQ.3.AND.CWK(1:6).EQ.'NMSTD=') NMSTD=INUMBA(IWK,L,80,J,K)
      IF(NMSTD.NE.1) NMSTD=0
      GOTO 1
 2    CLOSE(2)
      LIST=0
      CWK=USERNAME
      IF(CWK.EQ.' ') THEN
      WRITE(0,*) 'ENTER ICIS USER_NAME (BLANK TO QUIT)'
      READ(1,'(A)') CWK
      IF(CWK.EQ.' ') STOP
      ENDIF
      USERNAME=FTOCSTR(CWK)
      CWK=PASSWORD
      IF(CWK.EQ.' ') THEN
      WRITE(0,*) 'ENTER ICIS PASSWORD'
      READ(1,'(A)') CWK
      ENDIF
      PASSWORD=FTOCSTR(CWK)
C
C  OPEN THE DATABASE
C
      CTMP=FTOCSTR(CTMP)
      I4=GMS_openDatabase2(CTMP,USERNAME,PASSWORD,NUSER)
      IF(I4.NE.1) THEN
      WRITE(0,101) TRIM(CTOFSTR(USERNAME)),NUSER,
     & "******",' COULD NOT BE ',I4,GMSERROR(-I4)
 101  FORMAT(/' USER ',A,'(',I3,'),',2A,'CONNECTED'/
     &' WITH STATUS',I3,': ',A)
      USERNAME=' '
      PASSWORD=' '
      GOTO 2
      ELSE
      WRITE(0,101) TRIM(CTOFSTR(USERNAME)),NUSER,
     & "******",' ',I4,'GMS_SUCESS'
      ENDIF
      INST%INSTALLATION=0
      I4=GMS_getInstallation(INST,CWK,80)
      INSTLN=CWK(1:LEN_TRIM(INSTLN)-1)
      WRITE(0,103) INST%INSTALLATION,INST%UDATE,TRIM(INSTLN)      
 103  FORMAT(' TO INSTALLATION',I3,1X,I12,1X,A)
      USER%USER_ID=0
      USER%NAME=USERNAME
      I4=GMS_getUser(USER,0)
      NUID=USER%USER_ID
      WRITE(0,*) 'LOADING LOCATION BUFFER - PLEASE WAIT'
      CWK=CNTRY(0)      
C
C  OPEN THE LOG FILE
C
 12   J=LEN_TRIM(LASTDIR)
      IF(J.GT.0.AND.LASTDIR(J:J).NE.'\') LASTDIR(J+1:J+1)='\'
      CWK='BROWSE.LOG'
      IF(LASTDIR.NE.' ') CWK=TRIM(LASTDIR)//'BROWSE.LOG'
      IF(J.GT.0) OPEN(3,FILE=CWK,IOSTAT=IER)
      IF(J.LE.0.OR.IER.EQ.29.OR.IER.EQ.43.OR.IER.EQ.30) THEN
      WRITE(0,*) 'ENTER VALID DIRECTORY PATH FOR LOG FILE'
      READ(1,*) LASTDIR
      IF(LASTDIR.EQ.' ') STOP
      GOTO 12
      ENDIF
      IF(IER.NE.0) STOP 'FILERR'
      WRITE(3,108) ADATE,ATIME,TRIM(INIFILE)
      WRITE(3,'(" CENTRAL ",A,", LOCAL ",A)') TRIM(CTOFSTR(CENDSN)),
     &TRIM(CTOFSTR(LCLDSN))
      WRITE(3,'(" USER ",A," CONNECTED TO INSTALLATION ",I3,1X,I12,1X,A)')
     & TRIM(CTOFSTR(USERNAME)),INST%INSTALLATION,INST%UDATE,
     &TRIM(CTOFSTR(INSTLN))
      WRITE(3,134)' OPTIONS ', 
     &IOPT,ISRH,MGAT,NAMP,LDST,NTCX,LIST,NREF,
     &NLOC,FCPS,IBTYPE,IYAL

C  SEE IF THERE IS A COMMAND ON THE RUN LINE
      NLST=0
      LSTP=0
      MXLST=500
      CTMP=' '
      II2=2
      CALL GETARG(II2,CTMP,IIS)
      IF(CTMP.NE.' ') GOTO 11
C
C  READ THE COMMAND OR SEARCH STRING
C
 4    NLST=0
      LSTP=0
      MXLST=500
      WRITE(0,132)
 132  FORMAT(1X,70(1H-)/
     &' BROWSE WILL SEARCH FOR NAME STRINGS OR EXECUTE COMMANDS,'/
     &' A % AT THE END OF A SEARCH STRING MATCHES ALL ENDINGS,'/
     &' A _ IN A SEARCH STRING MATCHES ANY CHARACTER AT THAT POSITION'/
     &' ,NLOC=xxx and ,NTYPE=yyy RESTRICS THE SEARCH BY LOCATION OR TYPE
     &'/
     &' COMMANDS START WITH ".", OTHERWISE A SEARCH STRING IS ASSUMED.'/
     &' THE FOLLOWING COMMANDS ARE AVAILABLE:'/
     &' .n     - DISPLAY ENTRY n FROM THE CURRENT LIST'/
     &' .GID # - DISPLAY GMS DATA FOR GID #, EG .GID 12345'/
     &' .Pn    - DISPLAY GMS DATA FOR PROGENITOR n OF CURRRENT GID'/
     &' .m     - DISPALY GMS DATA FOR THE MANAGEMENT GROUP FOUNDER'/
     &' .FCR <Purdy string> - FINDS CROSSES BETWEEN PARENTS'/     
     &' .TRE #,n - PLOT THE PEDIGREE TREE FOR n GENERATIONS OR'/
     &' .TRE LST,n - PLOT THE TREE FOR ALL ENTRIES IN THE OPEN LIST'/
     &' .INP <file.ext> - SET THE INPUT STREAM TO FILE'/
     &' .MGM #,n,m - DISPLAY A MENDELGRAM FOR GID # UP TO n'/
     &'         GENERATIONS OR TO GERMPLASM FROM LOCATION=m'/
     &' .COP #1,#2 - COMPUTE COP BETWEEN GID1 AND GID2 OR'/
     &' .COP LST FOR THE COP MATRIX FOR ALL GIDS IN A LIST'/
     &' .REP #1,#2,<reason> - REPLACE GID1 WITH GID2',
     &' (#2=0 TO DELETE GID1)'/
     &' .SET fld=## IN # - CHANGE GERMPLASM OR NAME DATA',
     &' (.SET for HELP)'/
     &' .NAME #,<ntype>,<nlocn>,<ndate>,<nref>,<nvalue>',
     &' - ADDS A NAME FOR GID #'/
     &' .OPT  - GIVES HELP ON OPTIONS'/     
     &' .MORE - GIVES HELP ON MORE FUNCTIONS')
 141  FORMAT(1X,70(1H-)/
     &' COMMANDS START WITH ".", OTHERWISE A SEARCH STRING IS ASSUMED.'/
     &' THE FOLLOWING COMMANDS ARE AVAILABLE:'/
     &' .NOTE #,<avalue> - ADDS A NOTE ATTRIBUTE FOR GID #'/
     &' .TCE #,n - TRACE ALL DESCENDANTS FOR n GENERATIONS'/     
     &' .REL # - LIST CLOSE RELATIVES OF GID #'/
     &' .GRP # - DISPLAYS ALL GROUP RELATIVES OF GID #'/     
     &' .SLS # - SEARCHES GERMPLASM LISTS FOR ENTRIES WITH GID=#'/
     &' .GCD # - DISPLAY GENERATION CODE FOR GID # AS:'/
     &'      <cROSScODE>[FGen, SPGen or DHGen, ManGen]<DerivCode>'/
     &' .TAB #,n - TABULATE PROGENITORS FOR GID # UP TO n LEVELS'/    
     &' .DNB #,n - DISPLAY ALL DERIVATIVE NEIGHBORS OF GID #',
     &' WITHIN n STEPS'/
     &' .LST #,<code>,<source>,<desig> - ADD GID # TO A LIST',
     &' (<fields> MAY BE NULL)'/
     &' .SAV - SAVE DISPLAYED GIDS TO A GERMPLASM LIST'/     
     &' .DSL <lid> - DISPLAY ENTRIES OF LIST lid OR',
     &' OPEN LIST IF lid IS NULL'/
     &' .EXT LST - EXPORTS ANCESTORS TO THE',
     &' LOG FILE FOR ALL ENTRIES IN THE OPEN LIST'/
     &' .OPT LIST=<lid> - OPENS LIST lid OR DISPLAYS LISTS IF lid NULL'/
     &' .OPT - GIVES HELP ON OPTIONS')      
      INP=0
      CLOSE(2)
 3    IF(INP.EQ.1) GOTO 10
 9    CTMP="."
      IF(NLST.GT.0) CTMP='.n=DISPLAY ITEM n, .0=NONE, .'
      WRITE(0,102) CTMP(1:LEN_TRIM(CTMP))
 102  FORMAT(' ENTER SEARCH OR COMMAND STRING ',
     &'(',A,'HLP=HELP, ./=END)')
      READ(1,100) CTMP
 19   IF(CTMP.EQ.'"') CTMP=LSTSRCH
      IF(CTMP.EQ.'./') GOTO 6
      IF(CTMP.EQ.' '.AND.NLST.GT.0) CTMP='.1'
      IF(CTMP.EQ.'.0') THEN
      WRITE(3,112) CWK(1:LEN_TRIM(CWK))//' NOT SELECTED'
      GOTO 3
      ENDIF
      IF(CTMP.EQ.' ') GOTO 4
      GOTO 11
 10   READ(2,100,ERR=7,END=4) CTMP
      IF(CTMP.EQ.' ') GOTO 3
 11   WRITE(3,133) CTMP(1:LEN_TRIM(CTMP))
 133  FORMAT(' >',A)
C
      IF(CTMP.NE.'"') CWK=CTMP
      CMD=CWK
      CALL CCASE(CMD,1,4,1)
C  COMMAND .HLP --------------------------------------------------------------------
      IF(CMD.EQ.'.HLP') GOTO 4
C  COMMAND .MORE --------------------------------------------------------------------
      IF(CMD.EQ.'.MOR') THEN
      WRITE(0,141)
      GOTO 3
      ENDIF      
C  COMMAND .INP --------------------------------------------------------------------
      IF(CMD.EQ.'.INP') THEN
      I=VERIFY(CTMP(5:),' ')+4
      CWK=CTMP(I:)
      IF(LASTDIR.NE.' '.AND.INDEX(CTMP,':').EQ.0)
     &CWK=TRIM(LASTDIR)//CTMP(I:)
      OPEN(2,FILE=CWK,ERR=7,ACTION='READ',STATUS='OLD')
      INP=1
      GOTO 3
      ENDIF
C  COMMAND .OPT --------------------------------------------------------------------
      IF(CMD.EQ.'.OPT') THEN
      IF(CWK(5:).EQ.' ')THEN
      WRITE(0,134) ' SET OPTIONS WITH COMMAND: .OPT <option>=x ',
     &IOPT,ISRH,MGAT,NAMP,LDST,NTCX,LIST,NREF,
     &NLOC,FCPS,IBTYPE,IYAL
 134  FORMAT(A,'(Current value in parenthesis)'/
     &' IF <option> IS IOPT, x=1 TO SELECT SINGLE SEARCH HITS,'/
     &4X,'x=2 FOR NO SELECTION BUT LIST IS LOGGED, ELSE 0',
     &' (',I1,')'/
     &' IF <option> IS ISRH, x=1 ONLY GENERATIVE, x=2 ONLY DERIVATIVES'/
     &4X,'x=3 TO EXCLUDE ACCESSIONS, x=4 ONLY WITH ACCESSION NAMES',
     &' ELSE 0',' (',I1,')'/
     &' IF <option> IS MGAT, x=1 TO SHOW ALL PROGENITORS',
     &' IN THE MENDLEGRAM, ELSE 0',' (',I1,')'/
     &' IF <option> IS NAMP, x=0 FOR',
     &' PREFERRED NAME, 1 FOR ALL, 2 FOR ACCESSION NO',' (',I1,')'/
     &' IF <option> IS LDST, x=1 TO SHOW DERIVATIVE STEPS, ELSE 0',
     &' (',I1,')'/
     &' IF <option> IS NTCX, x=n TO STOP CROSS EXPANSION ON',
     &' NAME TYPE n, ELSE 0',' (',I1,')'/
     &' IF <option> IS LIST, x=0 TO CLOSE LIST OR x=n TO OPEN LIST n',
     &' (',I4,')'/
     &' IF <option> IS REFID, SET DEFAULT REFID TO x, ELSE 0',
     &' (',I4,')'/
     &' IF <option> IS LOCID, SET DEFAULT LOCATION TO x, ELSE 0',
     &' (',I4,')'/
     &' IF <option> IS FCPS, x=1 TO FIND CROSSES BY PARENT SEARCH',
     &' (',I1,')'/
     &' IF <option> IS BTYPE, x=1 OR 2 FOR FOR SF, 0 FOR CP CROPS',
     &' (',I1,')'/
     &' IF <option> IS IYAL, x=1 TO ACCEPT CHANGES AUTOMATICALLY,',
     &' ELSE 0 (',I1,')'/
     &' IF <option> IS IBEL, x=1 TO SILENCE WARNING NOTES, ELSE 0'/
     &' IF <option> IS NMSTD, x=1 TO PREVENT NAME STANDARDIZATION,',
     &' ELSE 0')
      GOTO 3
      ENDIF
      K=INDEX(CWK,'IOPT=')
      K=MAX(K,INDEX(CWK,'iopt='))
      IF(K.GT.0) THEN
      L=K+5
      IOPT=INUMBA(IWK,L,80,J,K)
      IF(K.GT.0) IOPT=2
      GOTO 3
      ENDIF
      K=INDEX(CWK,'ISRH=')
      K=MAX(K,INDEX(CWK,'isrh='))
      IF(K.GT.0) THEN
      L=K+5
      ISRH=INUMBA(IWK,L,80,J,K)
      IF(K.GT.0) ISRH=0
      GOTO 3
      ENDIF
      K=INDEX(CWK,'MGAT=')
      K=MAX(K,INDEX(CWK,'mgat='))
      IF(K.GT.0) THEN
      L=K+5
      MGAT=INUMBA(IWK,L,80,J,K)
      IF(K.GT.0) MGAT=0
      GOTO 3
      ENDIF
      K=INDEX(CWK,'NAMP=')
      K=MAX(K,INDEX(CWK,'namp='))
      IF(K.GT.0) THEN
      L=K+5
      NAMP=INUMBA(IWK,L,80,J,K)
      IF(K.GT.0) NAMP=0
      GOTO 3
      ENDIF
      K=INDEX(CWK,'LDST=')
      K=MAX(K,INDEX(CWK,'ldst='))
      IF(K.GT.0) THEN
      L=K+5
      LDST=INUMBA(IWK,L,80,J,K)
      IF(K.GT.0) LDST=0
      GOTO 3
      ENDIF
      K=INDEX(CWK,'NTCX=')
      K=MAX(K,INDEX(CWK,'ntcx='))
      IF(K.GT.0) THEN
      L=K+5
      NTCX=INUMBA(IWK,L,80,J,K)
      IF(K.GT.0) NTCX=0
      GOTO 3
      ENDIF
      K=INDEX(CWK,'FCPS=')
      K=MAX(K,INDEX(CWK,'fcps='))
      IF(K.GT.0) THEN
      L=K+5
      FCPS=INUMBA(IWK,L,80,J,K)
      IF(K.GT.0) FCPS=0
      GOTO 3
      ENDIF
      K=INDEX(CWK,'NMSTD=')
      K=MAX(K,INDEX(CWK,'nmstd='))
      IF(K.GT.0) THEN
      L=K+6
      NMSTD=INUMBA(IWK,L,80,J,K)
      IF(K.GT.0) NMSTD=0
      GOTO 3
      ENDIF      
      K=INDEX(CWK,'LIST')
      K=MAX(K,INDEX(CWK,'list'))
      IF(K.GT.0) THEN
      IF(LNAME%LISTID.NE.0) THEN
      WRITE(0,135) 'LIST',LNAME%LISTID,TRIM(CTOFSTR(LNAME%LISTDESC)),
     &LNAME%LISTSTATUS,'CLOSED'
      LNAME%LISTID=0
      LIST=0
      ENDIF
      L=K+5
      LIST=INUMBA(IWK,L,80,J,K)
      IF(K.GE.0) THEN
        LIST=0
	  L=0
        CALL ZEROLSTNM(LNAME)
        LNAME%LISTID=LIST
        WRITE(0,130)
 130    FORMAT(' LISTID   NAME     STATUS DESCRIPTION')              
        I4=GMS_getListName(LNAME,0)
         DO WHILE (I4.EQ.1)
         CWK=CTOFSTR(LNAME%LISTDESC)
         IF(CWK(50:).NE.' ') CWK(50:)='>'
         CTMP=CTOFSTR(LNAME%LISTNAME)
         WRITE(0,135) ' ',LNAME%LISTID,CTMP(1:11),LNAME%LISTSTATUS,
     &   TRIM(CWK)
	   L=L+1
	   CTMP=' '
	     IF (L.EQ.20) THEN
	      WRITE(0,138) '(BLANK FOR MORE LISTS, ./ FOR NONE)'
	      READ(1,139) CTMP
            IF (CTMP.NE.' ') EXIT
            L=0
	      WRITE(0,130)
	     ENDIF
         I4=GMS_getListName(LNAME,1)
         ENDDO
        LNAME%LISTID=0
	IF (CTMP.EQ.' ') THEN
        WRITE(0,138) '(./ FOR NONE)'
 138    FORMAT(' ENTER ID OF LIST TO OPEN ',A)
        READ(1,139) CTMP
 139    FORMAT(A)
      ENDIF
	  IF(CTMP.EQ.'./') CTMP=' '
        L=1
        CWK=CTMP
        LIST=INUMBA(IWK,L,80,J,K)
        IF(CTMP.EQ.' ') GOTO 3
        IF(LIST.EQ.0.OR.K.GE.0) GOTO 19
      ENDIF
      IF(LIST.EQ.0) GOTO 3
      CALL ZEROLSTNM(LNAME)
      LNAME%LISTID=LIST
      I4=GMS_getListName(LNAME,0)
      IF(I4.EQ.1) THEN
	IF(LNAME%LISTSTATUS.EQ.0) THEN
	WRITE(0,140)
 140  FORMAT(' CANNOT OPEN A FOLDER IN BROWSE.',
     &' USE .OPT LIST TO VIEW LISTS')
	ELSE
      CWK=CTOFSTR(LNAME%LISTDESC)
      IF(CWK(60:).NE.' ') CWK(60:)='>'
      WRITE(0,135) 'LIST',LNAME%LISTID,TRIM(CWK),LNAME%LISTSTATUS,'OPEN'
 135  FORMAT(1X,A,I5,' ',A,I4,' ',A)
      ENDIF
      ELSE
      WRITE(0,136) LIST,I4
 136  FORMAT(' LIST',I4,' COULD NOT BE OPENED - ERROR',I4)
      LIST=0
      ENDIF
      GOTO 3
      ENDIF
      K=INDEX(CWK,'REFID=')
      K=MAX(K,INDEX(CWK,'refid='))
      IF(K.GT.0) THEN
      L=K+6
      NREF=INUMBA(IWK,L,80,J,K)
      IF(K.GT.0) NREF=0
      GOTO 3
      ENDIF
      K=INDEX(CWK,'LOCID=')
      K=MAX(K,INDEX(CWK,'locid='))
      IF(K.GT.0) THEN
      L=K+6
      NLOC=INUMBA(IWK,L,80,J,K)
      IF(K.GT.0) NLOC=0
      GOTO 3
      ENDIF
      K=INDEX(CWK,'BTYPE=')
      K=MAX(K,INDEX(CWK,'btype='))
      IF(K.GT.0) THEN
      L=K+6
      IBTYPE=INUMBA(IWK,L,80,J,K)
      IF(K.GT.0) IBTYPE=1
      IF(IBTYPE.NE.0.AND.IBTYPE.NE.1.AND.IBTYPE.NE.2) THEN
      WRITE(0,*) ' *** ERROR *** ILLEGAL BTYPE VALUE 1 ASSUMED'
      IBTYPE=1
      ENDIF      
      GOTO 3
      ENDIF
      K=INDEX(CWK,'IYAL=')
      K=MAX(K,INDEX(CWK,'iyal='))
      IF(K.GT.0) THEN
      L=K+5
      IYAL=INUMBA(IWK,L,80,J,K)
      IF(IYAL.NE.1) IYAL=0
      GOTO 3
      ENDIF
      K=INDEX(CWK,'IBEL=')
      K=MAX(K,INDEX(CWK,'ibel='))
      IF(K.GT.0) THEN
      L=K+5
      Ik=INUMBA(IWK,L,80,J,K)
      G_BELL=' '
      IF(Ik.NE.1) G_BELL=CHAR(7)
      BELL=G_BELL
      GOTO 3
      ENDIF
      ENDIF
C  COMMAND .LOC --------------------------------------------------------------------
      IF(CMD.EQ.'.LOC') THEN
      LOCN%PREFERRED_NAME=FTOCSTR(CTMP(6:))
      LOCN%TYPE=0
      I4=GMS_findLocation(LOCN,0)
      DO WHILE (I4.EQ.1)
      II=INDEX(LOCN%PREFERRED_NAME,NULL)-1
      WRITE(0,119) LOCN%LOCATION_ID,LOCN%PREFERRED_NAME(1:II),
     & LOCN%COUNTRY,LOCN%TYPE
 119  FORMAT(I5,1X,A,T69,I4,I4)
      I4=GMS_findLocation(LOCN,1)
      END DO
      GO TO 3
      ENDIF
C  COMMAND .NXT --------------------------------------------------------------------
      IF(CMD.EQ.'.NXT') THEN
      IF(CWK(5:).NE.' ') THEN
      L=6
      I=INUMBA(IWK,L,80,J,K)
      IF(K.GT.0) L=7
      IF(I.EQ.0) GOTO 3
      IF(CWK(L:L).EQ.',') L=L+1
      ENDIF
      CLWK=FTOCSTR(TRIM(CWK(L:)))
      I4=GMS_getNextSequence(I,CLWK,L)
      WRITE(0,'(A,I5)') TRIM(CTOFSTR(CLWK)),L
      GO TO 3
      ENDIF      
C  COMMAND .ENC --------------------------------------------------------------------
      IF(CMD.EQ.'.ENC') THEN
      CWK=FTOCSTR(CTMP(6:))
      I4=GMS_encryptPWD(CWK,ATMP)
      IF(I4.NE.1) THEN
      WRITE(0,124)
 124  FORMAT(' *** ERROR *** FUNCTION NOT ACCESSIBLE')
      GO TO 3
      ENDIF
      ATMP=CTOFSTR(ATMP)
      CWK=CTOFSTR(CWK)
      WRITE(0,123) CWK,ATMP
 123  FORMAT(1X,A,T31,A)
      GO TO 3
      ENDIF
C  COMMAND .REF --------------------------------------------------------------------
      IF(CMD.EQ.'.REF') THEN
      IREF=0
      IF(CWK(5:).NE.' ') THEN
      L=5
      IREF=INUMBA(IWK,L,80,J,K)
        IF(K.GT.0) THEN
          IF(IER.NE.1) THEN
            WRITE(0,118) BELL,'*** ERROR *** .REF FUNCTION ABANDONED'
          ENDIF
        ENDIF
      ENDIF
      IF(IREF.EQ.0) IREF=GERM%REFERENCE
      CALL ZEROBIBREFS(BREF)
      BREF%REFID=IREF
      I4=-1
      IF(IREF.NE.0)I4=GMS_getBibrefs(BREF,0)
      IF(I4.NE.1) GOTO 3
      WRITE(0,127) TRIM(CTOFSTR(BREF%AUTHORS)),BREF%PUBDATE,
     &TRIM(CTOFSTR(BREF%ANALYT))
 127  FORMAT(1X,A,1X,I8/1X,A)
      GOTO 3
      ENDIF
C  COMMAND .ADU --------------------------------------------------------------------
      IF(CMD.EQ.'.ADU') THEN
      LINST=INST%INSTALLATION
      CALL ADDUSER(CTMP,LINST,USER,IER)
      IF(IER.NE.1) THEN
      WRITE(0,118) BELL,'*** ERROR *** .ADU FUNCTION ABANDONED:'
     &,TRIM(CTMP)
      WRITE(0,118) ' ',
     &'FORMAT: .ADU <uname>,<pwd>,<security>,<access>,<type>',' '
      WRITE(3,118) ' ','*** ERROR *** .ADU FUNCTION ABANDONED:'
     &,TRIM(CTMP)
      IF(IER.EQ.-1) CALL GMS_rollbackData()
      ELSE
      CALL GMS_commitData()
      ENDIF
      GOTO 3
      ENDIF
C  COMMAND .FIX --------------------------------------------------------------------
      IF(CMD.EQ.'.FIX') THEN
      IF(CWK(5:).NE.' ') THEN
      L=5
      I=INUMBA(IWK,L,80,J,K)
      IF(K.GT.0) L=7
      IF(I.NE.0) IGID=I
      IF(IGID.EQ.0) GOTO 3
      ENDIF
      CALL FIXATT(IGID)
      GO TO 3
      ENDIF
C  COMMAND .CID --------------------------------------------------------------------
      IF(CMD.EQ.'.CID') THEN
      IF(CWK(5:).EQ.' ') GOTO 3
      L=5
      I=INUMBA(IWK,L,80,J,K)
      IF(K.GE.0) GOTO 3
      CID=I
      I=INUMBA(IWK,L,80,J,K)
      IF(K.GT.0) GOTO 3
C  K>0 -> ERROR
C  K<0 -> SID GIVEN
C      CALL GMS_findCIDSID WITH FOPT=2 AND DONT CALL IT AGAIN
C  K=0 -> SID NOT GIVEN
C      CALL GMS_findCIDSID WITH FOPT=0 AND THEN WITH FOPT=1 UNTIL I4=0
      SID=I
      IGID=0
      IF(K.LT.0) K=2
      I4=GMS_findCIDSID(CID,SID,IGID,K)
      IF(I4.NE.1) THEN
      IF(I4.EQ.0) WRITE(0,'("CID=",I8," SID=",I5," NOT FOUND")') CID,SID
      IF(I4.NE.0) WRITE(0,*) '.CID COMMAND NOT AVAILABLE'
      GOTO 3
      ENDIF
      I5=GMS_getGermplasm2(IGID,GERM,NAMDAT,ANAME,256)
      NLST=0
      ANAME=LSTNMS(GERM%GERMPLASM_ID,47)
      NLST=NLST+1
      IF (NLST.EQ.1) THEN
      WRITE(0,144)
      WRITE(3,144)
 144  FORMAT(' CID-SID MATCHES:'/
     &' NO    GID  NAMES',43X,'METHOD  CID   SID')
      ENDIF
      IF(NLST.EQ.MXLST) GOTO 21
      LLST(NLST)=GERM%GERMPLASM_ID
      WRITE(0,143)NLST,GERM%GERMPLASM_ID,ANAME,GERM%METHOD,CID,SID
      WRITE(3,143)NLST,GERM%GERMPLASM_ID,ANAME,GERM%METHOD,CID,SID
 143  FORMAT(I3,I8,1X,A47,I4,I8,I5)
      IF(K.EQ.2) GOTO 3
      DO WHILE (I4.EQ.1)
      IGID=0
      I4=GMS_findCIDSID(CID,SID,IGID,1)
      IF(I4.NE.1) GOTO 3
      I5=GMS_getGermplasm2(IGID,GERM,NAMDAT,ANAME,256)
      ANAME=LSTNMS(GERM%GERMPLASM_ID,47)
      NLST=NLST+1
      IF(NLST.EQ.MXLST) GOTO 21
      LLST(NLST)=GERM%GERMPLASM_ID
      WRITE(0,143)NLST,GERM%GERMPLASM_ID,ANAME,GERM%METHOD,CID,SID
      WRITE(3,143)NLST,GERM%GERMPLASM_ID,ANAME,GERM%METHOD,CID,SID
      ENDDO
      GO TO 3
      ENDIF
C  COMMAND .NAME --------------------------------------------------------------------
      IF(CMD.EQ.'.NAM') THEN
      CALL ADDANAME(CTMP,IGID,IER)
      IF(IER.NE.1) THEN
      WRITE(0,118) BELL,'*** ERROR *** .NAME FUNCTION ABANDONED:'
     &,TRIM(CTMP)
      WRITE(0,118) ' ',
     &'FORMAT: .NAME <gid>,<ntype>,<nlocn>,<ndate>,<nref>,<nval>',' '
      WRITE(3,118) ' ','*** ERROR *** .NAME FUNCTION ABANDONED:'
     &,TRIM(CTMP)
      IF(IER.EQ.-1) CALL GMS_rollbackData()
      ELSE
      CALL GMS_commitData()
      CALL WRTGERM(IGID,GERM,3)
      ENDIF
      GOTO 3
      ENDIF
C  COMMAND .NOTE --------------------------------------------------------------------
      IF(CMD.EQ.'.NOT') THEN
      IF(CWK(5:).NE.' ') THEN
      L=6
      I=INUMBA(IWK,L,80,J,K)
      IF(K.GT.0) L=7
      IF(I.NE.0) IGID=I
      IF(IGID.EQ.0) GOTO 5
      IF(CTMP(L:L).EQ.',') L=L+1
      ENDIF
      IF(I.EQ.IGID) CALL WRTGERM(IGID,GERM,3)
      CALL GETDATM(RDATE,RTIME,IDTE,ITME)
      J=MAX(L,LEN_TRIM(CTMP))
      CALL ADDATT(IGID,103,NLOC,IDTE,NREF,CTMP(L:J),IER)
      IF(IER.NE.1) THEN
      WRITE(0,118)BELL,'*** ERROR *** ERROR .NOTE FUNCTION ABANDONED:',
     ,TRIM(CTMP)
      CALL GMS_RollbackData()
      GOTO 3
      ELSE
      CALL GMS_CommitData()
      CALL WRTGERM(IGID,GERM,3)
      GOTO 3
      ENDIF
      ENDIF
C  COMMAND .DSL - DISPLAY LIST ENTRIES  -------------------------------------
      IF(CMD.EQ.'.DSL') THEN
      L=5
      I4=INUMBA(IWK,L,80,J,K)
      IF(K.GT.0) GOTO 5
      IF(I4.NE.0) LIST=I4
      IF(LIST.EQ.0) THEN
      WRITE(0,118) ' ',
     &'*** WARNING *** NO LIST OPEN. Use .OPT LIST to view lists'
      ELSE
      CALL ZEROLSTNM(LNAME)
      LNAME%LISTID=LIST
      I4=GMS_getListName(LNAME,0)
      IF(I4.NE.1) THEN
      WRITE(0,118) BELL,'*** ERROR *** UNABLE TO OPEN LIST'
      GOTO 3
      ENDIF
      CWK=CTOFSTR(LNAME%LISTDESC)
      IF(CWK(60:).NE.' ') CWK(60:)='>'
      WRITE(0,135) 'LIST',LNAME%LISTID,TRIM(CWK)
      WRITE(0,126)
      WRITE(3,135) 'LIST',LNAME%LISTID,TRIM(CWK)
      WRITE(3,126)
 126  FORMAT(' NO. DESIGNATION',31X,'SOURCE       ENTRYCD      GID')
      NLST=0
      LDATA%LISTID=LIST
      LDATA%GID=0
      LDATA%ENTRYID=0
      LDATA%LRECID=0
      I4=GMS_getListData(LDATA,0)
      DO WHILE (I4.EQ.1)
      NLST=NLST+1
      IF(NLST.GT.MXLST) GOTO 21
      LLST(NLST)=LDATA%GID
      CWK=CTOFSTR(LDATA%DESIG)
      IF(CWK(42:).NE.' ') CWK(42:)='>'
      CTMP=CTOFSTR(LDATA%SOURCE)
      IF(CTMP(12:12).NE.' ') CTMP(12:12)='>'
      WRITE(0,129) NLST,TRIM(CWK),CTMP,CTOFSTR(LDATA%ENTRYCD),LDATA%GID
      WRITE(3,129) NLST,TRIM(CWK),CTMP,CTOFSTR(LDATA%ENTRYCD),LDATA%GID
 129  FORMAT(I4,1X,A,T48,A12,1X,A10,I8)
      I4=GMS_getListData(LDATA,1)
      ENDDO
      ENDIF
      GOTO 3
      ENDIF
C  COMMAND .LST --------------------------------------------------------------------
      IF(CMD.EQ.'.LST') THEN
      IER=1
      IF(LIST.EQ.0) CALL MAKELIST(' ',' ',' ',LIST,IER)
      IF(IER.EQ.1) CALL ADDLIST(LIST,CWK,IGID,NTRY,IER)
      IF(IER.NE.1) THEN
      WRITE(0,118)BELL,'*** ERROR *** LST FUNCTION ABANDONED:',TRIM(CWK)
      WRITE(3,118) ' ','*** ERROR *** LST FUNCTION ABANDONED:',TRIM(CWK)
      IF(IER.EQ.-1) CALL GMS_rollbackData()
      ELSE
      WRITE(0,125) IGID,LIST
 125  FORMAT(' GID',I9,' ADDED TO LIST',I4)
      CALL GMS_commitData()
      ENDIF
      GOTO 3
      ENDIF
C  COMMAND .SAV --------------------------------------------------------------------
      IF(CMD.EQ.'.SAV') THEN
      IF(LSTP.EQ.0) THEN
      IER=1
      IF(LIST.EQ.0) CALL MAKELIST('SAVE LIST','SAVE',' ',LIST,IER)
      DO 14 I=1,NLST
      IF(IER.EQ.1) CALL ADDLIST(LIST,'.LST',LLST(I),NTRY,IER)
 14   CONTINUE
      ENDIF
C DENDROGRAM LIST
      IF(LSTP.EQ.1) THEN
      CALL MAKELIST('TREE LIST','TREE',' ',LIST,IER)
      IF(IER.EQ.1) CALL SAVETREE(1,0,CWK,LVL,0)
      ENDIF
C OLD DENDROGRAM LIST
      IF(LSTP.EQ.3) THEN
      CALL MAKELIST('TREE LIST','TREE',' ',LIST,IER)
      IF(IER.EQ.1) CALL SAVETREE1(1,0,CWK,LVL,0)
      ENDIF
C MENDELGRAM LIST
      IF(LSTP.EQ.2) THEN
      IER=1
      CTMP=' '
      I4=GMS_getGermplasm2(LLST(NLST+1),GERM,NAMDAT,CTMP,80)
      I4=INDEX(CTMP,NULL)
      IF(I4.GT.0.AND.I4.LT.50)
     & WRITE(CTMP(I4:),107) LLST(NLST+1),RTREE(NLST+1,2)
 107  FORMAT(' (GID=',I9,', RICHNESS=',F6.3,')')
      IF(LIST.EQ.0) CALL MAKELIST('MGM','MGM',CTMP,LIST,IER)
      DO 15 I=1,NLST
      GERM%LOCATION=0
      I4=GMS_getGermplasm2(LLST(I),GERM,NAMDAT,CTMP,80)
      WRITE(CTMP,109) RTREE(I,2),ITREE(I,6),CNTRY(GERM%LOCATION)
 109  FORMAT('LST ,',F8.3,I4,',',A3)
      IF(IER.EQ.1) CALL ADDLIST(LIST,CTMP,LLST(I),NTRY,IER)
 15   CONTINUE
      ENDIF
      IF(IER.NE.1) THEN
      WRITE(0,118) BELL,'*** ERROR *** LST FUNCTION ABANDONED',' '
      IF(IER.EQ.-1) CALL GMS_rollbackData()
      ELSE
      CALL GMS_commitData()
      ENDIF
      GOTO 3
      ENDIF
C  COMMAND .n --------------------------------------------------------------------
 8    IF(CWK(1:1).EQ.'.') THEN
      L=2
      I=INUMBA(IWK,L,80,J,K)
      IF(K.LT.0.AND.I.GT.0.AND.I.LE.NLST) THEN
      IF(LSTP.EQ.1) THEN
      WRITE(CWK,111,ERR=7) NTREE(I)%GID
      ELSE
      WRITE(CWK,111,ERR=7) LLST(I)
      ENDIF
 111  FORMAT('.GID ',I12)
      CMD='.GID'
      ENDIF
      ENDIF
C  COMMAND .Pn --------------------------------------------------------------------
      IF(CMD(1:2).EQ.'.P'.AND.IGID.NE.0) THEN
      L=3
      I=INUMBA(IWK,L,80,J,K)
      L=GERM%NO_PROGENITORS
      IF(L.EQ.-1) L=2
      IF(K.LT.0.AND.I.GT.0.AND.I.LE.2) THEN
      IF(I.EQ.1) WRITE(CWK,111) GERM%PROGENITOR_ID1
      IF(I.EQ.2) WRITE(CWK,111) GERM%PROGENITOR_ID2
      CMD='.GID'
      ENDIF
      ENDIF
C  COMMAND .M --------------------------------------------------------------------
      IF(CMD.EQ.'.M'.AND.IGID.NE.0) THEN
      WRITE(CWK,111) GERM%MGID
      CMD='.GID'
      ENDIF
C  COMMAND .SET --------------------------------------------------------------------
      IF(CMD.EQ.'.SET') THEN
        IF(CWK(5:).EQ.' ') THEN
        WRITE(0,128)
 128  FORMAT('.SET fld=xx IN nn <st>'/' WHERE'/' fld IS GPIDn,GLOCN,',
     & 'GDATE,METHN,GREFN,GNPGS,NDATE,NLOCN,NREFN,NTYPE OR NSTAT,'/
     & ' xx  IS THE NEW VALUE FOR THE SPECIFIED FIELD, AND'/
     & ' nn  IS THE GID FOR GERMPLASM FIELDS OR NID FOR NAME FIELDS'/
     & ' <st> IS ANY TEXT STRING GIVING A REASON FOR THE CHANGE')
        GOTO 3
        ENDIF
        ATMP=' '
        IF(CWK(6:6).EQ.'N'.OR.CWK(6:6).EQ.'n') THEN
        CALL SETNAM(CWK,INID,NAMDAT,ATMP,ATMP,IER)
      IGID=NAMDAT%GERMPLASM
        ELSE
      CALL SETGERM(CWK,IGID,GERM,ATMP,ATMP,IER)
        ENDIF
      IF(IER.NE.1) THEN
      WRITE(0,118)BELL,'*** ERROR *** SET FUNCTION ABANDONED:',TRIM(CWK)
      WRITE(3,118) ' ','*** ERROR *** SET FUNCTION ABANDONED:',TRIM(CWK)
 118  FORMAT(1X,A1,A,1X,A)
      IF(IER.EQ.-1) CALL GMS_rollbackData()
      ELSE
      CALL GMS_commitData()
      CALL WRTGERM(IGID,GERM,3)
      ENDIF
      GOTO 3
      ENDIF
C  COMMAND .HAS --------------------------------------------------------------------
      IF(CMD.EQ.'.HAS') THEN
      CALL HASFIX(CWK,IGID,GERM,IER)
      IF(IER.NE.1) THEN
      WRITE(0,118)BELL,'*** ERROR *** HAS FUNCTION ABANDONED:',TRIM(CWK)
      IF(IER.EQ.-1) CALL GMS_rollbackData()
      ELSE
      CALL GMS_commitData()
      CALL WRTGERM(IGID,GERM,3)
      ENDIF
      GOTO 3
      ENDIF
C  COMMANDS .GID, .REL, .TR2, .TR1, .MGM, .TCE, .TAB --------------------------------
      IF (CMD.EQ.'.REL'.OR.CMD.EQ.'.GID'.OR.CMD.EQ.'.TR2'
     &.OR.CMD.EQ.'.TR1'.OR.CMD.EQ.'.MGM'.OR.CMD.EQ.'.TCE'
     &.OR.CMD.EQ.'.GRP'.OR.CMD.EQ.'.TAB'.OR.CMD.EQ.'.OTB'
     &.OR.CMD.EQ.'.GCD'.OR.CMD.EQ.'.MLS'.OR.CMD.EQ.'.SLS'
     &.OR.CMD.EQ.'.DNB'.OR.CMD.EQ.'.PTR'.OR.CMD.EQ.'.CXP') THEN
      LVL=0
      IF(CWK(5:).NE.' ') THEN
      L=5
      IGID=INUMBA(IWK,L,80,J,K)
      IF(K.GT.0) GOTO 5
      IF(L.LT.80) LVL=INUMBA(IWK,L,80,J,K)
      IF(K.GT.0) GOTO 5
C  PICK UP A THIRD NUMERIC PARAMETER IF IT EXISTS (LOCN IN .MGM)      
      M=0
      IF(L.LT.80) M=INUMBA(IWK,L,80,J,K)
      IF(IGID.NE.0) CALL WRTGERM(IGID,GERM,3)
      ENDIF
      IF(IGID.EQ.0) GOTO 5
      IF(LVL.EQ.0) LVL=-1
C  GET DERIVATIVE NEIGHBORHOOD -------------------------------------------------------
      IF(CMD.EQ.'.DNB') THEN
      NLST=500
      MSTEP=3
      IF(LVL.GT.0) MSTEP=LVL
      NSTEP=MSTEP 
      I4=GMS_getDerivativeNbh(IGID,MSTEP,NSTEP,LLST,NLST,0)
      IF(I4.NE.1) PAUSE 'ERROR'
      WRITE(0,142) NLST,MSTEP,NSTEP,IGID
      WRITE(3,142) NLST,MSTEP,NSTEP,IGID      
142   FORMAT(I4,' DERIVATIVE NEIGHBORS WITHIN',I2,
     &' BACKWARD AND',I2,' FORWARD STEPS OF GID',I9/
     &' NO NAMES',48X,'GID METHOD CTY DATE')
      DO I=1,NLST
      I4=GMS_getGermplasm(LLST(I),GERM)
      IF(I4.NE.1) THEN
      WRITE(0,'(" *** ERROR *** UNABLE TO GET GERMPLASM",I9)') LLST(I)
      GOTO 3
      ENDIF
      CLWK=LSTNMS(LLST(I),50)
      WRITE(0,189) I,CLWK,LLST(I),MCODE(GERM%METHOD),
     &CNTRY(GERM%LOCATION),GERM%DATE/10000      
      WRITE(3,189) I,CLWK,LLST(I),MCODE(GERM%METHOD),
     &CNTRY(GERM%LOCATION),GERM%DATE/10000       
      ENDDO
      IF(VIEWER.NE.' ') CALL VIEWLST(IGID,LASTDIR,0)
      ENDIF            
C  SEARCH LISTS -----------------------------------------------------------------------------------------
      IF(CMD.EQ.'.SLS') THEN
      CALL SEARCHLISTS(IGID,LSTGID,NLSTS,IER)
      ENDIF      
C  GENERATION CODE
      IF(CMD.EQ.'.GCD') THEN
      CTMP=GENCDE(IGID)
      WRITE(0,131) TRIM(CTMP),IGID
      WRITE(3,131) TRIM(CTMP),IGID,LSTNMS(IGID,50)
 131  FORMAT(' GENERATION CODE: ',A,T40,' FOR',I8,': ',A)
      ENDIF
C DENDROGRAM TREE --------------------------------------------------------------------
      IF(CMD.EQ.'.TR2') THEN
      IF(LVL.EQ.-1) LVL=3
      NLST=0
      LSTP=1
      CALL GETTREE(IGID,LVL,IER)
      IF(IER.NE.1) PAUSE
      CALL DRAWTREE2(1,0,CWK,LVL,0)
	ENDIF
C ALTERNATIVE TREE COMMAND -----------------------------------------------------------	
      IF(CMD.EQ.'.TR1') THEN
      IF(LVL.EQ.-1) LVL=3
      NLST=0
      LSTP=3
      CALL GETTREE1(IGID,LVL,0,IER)
      IF(IER.NE.1) PAUSE      
      J=0
      DO 13 I=1,NLST
      IF(ITREE(I,5).GT.0.AND.ITREE(I,6).GT.0) J=J+1
 13   CONTINUE
      IF(IER.NE.1) GOTO 3
      CALL DRAWTREE1(1,0,CWK,LVL,0)
 113  FORMAT(I5,' PROGENITORS AND',I5,' GENERATIONS SHOWN')
      WRITE(0,113) NLST,J
      WRITE(3,113) NLST,J
      ENDIF
C MENDLEGRAM -------------------------------------------------------------------------
      IF(CMD.EQ.'.MGM') THEN
      IF(LVL.EQ.-1) LVL=100
      LSTP=2
      CALL MGRAM(IGID,LVL,M)
      ENDIF
C FIND RELATIVES ---------------------------------------------------------------------      
      IF(CMD.EQ.'.REL') THEN
      LSTP=0
      NLST=0
      CALL RELATIVES(GERM)
      ENDIF
C FIND GROUP RELATIVES --------------------------------------------------------------       
      IF(CMD.EQ.'.GRP') THEN
      LSTP=0
      NLST=0
      CALL GROUP(GERM)
      IF(VIEWER.NE.' ') CALL VIEWLST(IGID,LASTDIR,1)      
      ENDIF
C TRACE FORWARD ----------------------------------------------------------------------      
      IF(CMD.EQ.'.TCE') THEN
      IF(LIST.NE.0.AND.LNAME%LISTTYPE(1:5).NE.'TRACE') THEN
      WRITE(0,137) LNAME%LISTTYPE(1:5)
 137  FORMAT(' *** WARNING *** ',A5,' LIST CLOSED')
      LIST=0
      ENDIF
      CALL TRACE(GERM,LVL)
      ENDIF
C COMMAND PTR ---------------------------------------------------------------------------      
      IF(CMD.EQ.'.PTR') THEN
      IF(LVL.EQ.-1) LVL=3
      NLST=0
      LSTP=3
      CALL GETTREE1(IGID,LVL,0,IER)
      IF (IER.NE.1) GOTO 3
      CALL PlotTREE(LASTDIR)
      ENDIF
C COMMAND OTB ---------------------------------------------------------------------------      
      IF(CMD.EQ.'.OTB') THEN
      IF(LVL.EQ.-1) LVL=3
      NLST=0
      LSTP=3
      CALL GETTREE1(IGID,LVL,0,IER)
      IF (IER.NE.1) GOTO 3
      CALL TABTREE1()
      ENDIF
C COMMAND CXP CROSS EXPANSION -------------------------------------------------------------            
      IF(CMD.EQ.'.CXP') THEN
      IF(LVL.EQ.-1) LVL=1
      CALL CROSSEXP(IGID,LVL,0,3)
      ENDIF      
C COMMAND TAB ---------------------------------------------------------------------------            
      IF(CMD.EQ.'.TAB') THEN
      IF(LVL.EQ.-1) LVL=3
      NLST=0
      LSTP=1
      CALL GETTREE(IGID,LVL,IER)
      IF (IER.NE.1) GOTO 3
      I=1
      IER=0
      WRITE(0,187)
      WRITE(3,187)
 187  FORMAT(7X,'GID    FEMALE      MALE STEPS NAME')
      CALL TABTREE(I,IER,LVL)
      ENDIF
C  COMMAND .MLS --------------------------------------------------------------------
      IF(CMD.EQ.'.MLS') THEN
      IF(LVL.EQ.-1) LVL=1000
      NLST=0
      LSTP=1
      CALL GETTREE(IGID,LVL,IER)
      IF(IER.NE.1) PAUSE      
	WRITE(0,188) IGID
	WRITE(3,188) IGID
 188  FORMAT(' MLS ANCESTORS OF GID',I9/
     &' NO NAMES',47X,'GID METHOD')
      J=NLST
	NLST=0
	LSTP=0
      DO 186 I=1,J
C      WRITE(3,'(6I10)') I,NTREE(I)
	CTMP=' '
      ATTB%TYPE=223
      ATTB%GERMPLASM=NTREE(I)%GID
      I4=GMS_getAttribute(ATTB,CTMP,80,0)
	IF(I4.EQ.1) THEN
	DO L=1,NLST
	IF(NTREE(I)%GID.EQ.LLST(L)) GOTO 186
	ENDDO
      CLWK=LSTNMS(NTREE(I)%GID,50)
      NLST=NLST+1
      L=NTREE(I)%METHN 
	WRITE(0,189) NLST,CLWK,NTREE(I)%GID,MCODE(L)
	WRITE(3,189) NLST,CLWK,NTREE(I)%GID,MCODE(L)
 189  FORMAT(I3,1X,A50,I8,1X,2A4,I5)
	LLST(NLST)=NTREE(I)%GID
	ENDIF
 186  CONTINUE
	ENDIF
      GOTO 3
      ENDIF
C  COMMAND .REP --------------------------------------------------------------------
      IF(CMD.EQ.'.REP') THEN
      I4=0
      IF(IWK(1).NE.'.REP'.AND.IWK(1).NE.'.rep') GOTO 20
      L=5
      IGID=INUMBA(IWK,L,80,J,IER)
      IF(IER.GE.0) GOTO 20
      NGID=INUMBA(IWK,L,80,J,IER)
      IF(IER.GT.0) GOTO 20
      I4=LREPLACE(IGID,NGID,CWK)
 20   IF(I4.NE.1) THEN
      WRITE(0,118) BELL,'*** ERROR *** REPLACE CHANGE ABANDONED:',' '
      IF(I4.EQ.-1) CALL GMS_rollbackData()
      ELSE
      IGID=NGID
      CALL GMS_commitData()
      ENDIF
      GOTO 3
      ENDIF
C  COMMAND .COP --------------------------------------------------------------------
      IF(CMD.EQ.'.COP') THEN
      IF(CWK(6:8).EQ.'LST'.OR.CWK(6:8).EQ.'lst') THEN
      L=9
      LVL=INUMBA(IWK,L,80,J,K)
      IF(K.GE.0) LVL=100
      ILST=0
	NLST=0
      LDATA%LISTID=LIST
      LDATA%GID=0
      LDATA%ENTRYID=0
      LDATA%LRECID=0
      I4=GMS_getListData(LDATA,0)
	IF(I4.NE.1) THEN
	WRITE(0,156) BELL
 156  FORMAT(A1,' *** ERROR *** ERROR READING LIST')
      GOTO 3
	ENDIF
	WRITE(99,'(" FETCHING PEDIGREES ...")')
      DO WHILE (I4.EQ.1)
      ILST=ILST+1
      IF(ILST.NE.LDATA%ENTRYID) THEN
      WRITE(0,105) BELL,LIST
 105  FORMAT(A1,' *** ERROR *** ENTRIES IN LIST',I4,
     &' HAVE NON CONSECUTIVE ENTRY IDS')
      GOTO 3
      ENDIF
	IF(ILST.GT.MAXLIST) THEN
	WRITE(0,155) BELL,ILST
 155  FORMAT(A1,' *** ERROR *** TOO MANY LIST ENTRIES (MAX 3000)',I4)
      GOTO 3
	ENDIF
	LSTGID(ILST,1)=LDATA%GID
	LSTGID(ILST,2)=NLST+1
	LSTECD(ILST)=CTOFSTR(LDATA%ENTRYCD)
	CALL GETTREE(LSTGID(ILST,1),LVL,IER)
	WRITE(99,'(A1,I10,1X,A)') '+',NLST,'ENTRIES IN PEDIGREE LIST'
	IF(IER.NE.1) THEN
	WRITE(0,157) BELL,IER
 157  FORMAT(A1,' *** ERROR *** ERROR RETRIEVING TREE',I4)
      GOTO 3
	ENDIF
      I4=GMS_getListData(LDATA,1)
      ENDDO
C  FOR COP LST         
      WRITE(0,106) ILST,LIST,TRIM(CTOFSTR(LNAME%LISTNAME)),NLST,IBTYPE
      WRITE(3,166) ILST,LIST,TRIM(CTOFSTR(LNAME%LISTNAME)),NLST,IBTYPE
 106  FORMAT(' COP VALUES FOR',I4,' ENTRIES FROM LIST NO',I4,' (',A,')'
     &/' WITH APPROXIMATELY',I7,' DISTINCT ANCESTORS. BTYPE=',I2
     &/'(LOWER TRIANGLE OF THE COP MATRIX BY ROWS)')
 166  FORMAT('H COP VALUES FOR',I4,' ENTRIES FROM LIST NO',I4,' (',A,')'
     &/'H WITH APPROXIMATELY',I7,' DISTINCT ANCESTORS. BTYPE=',I2
     &/'H (LOWER TRIANGLE OF THE COP MATRIX BY ROWS)'
     &/'V001 ROWID ROW GID'/'V002 COLID COLUMN GID'
     &/'V003 ROWNO ROW NUMBER'/'V004 COLNO COLUMN NUMBER'
     &/'V005 COP COP VALUE'/'V006 ROWCD$ ROW ENTRY CODE'
     &/'V007 COLCD$ COLUMN ENTRY CODE'/'V008 ROWNM$ NAME OF ROW ENTRY'
     &/'V009 COLNM$ NAME OF COLUMN ENTRY'/'////')
      I4=1
      IF(ILST.LE.2000) I4=ILST*(ILST+1)/2
      ALLOCATE (COPMAT(I4))
	CALL LCOP(ILST,LSTGID,LSTECD,COPMAT)
      IF(ILST.LE.2000) THEN
      CALL GIV(3,ILST,LSTGID,COPMAT)
      ENDIF
      DEALLOCATE (COPMAT)
C FOR COP <gid1> <gid2> ----------------------------------------------     
      ELSE
      NLST=0
      CALL ICOP(CWK,COP,IDY)
      IF(VIEWER.NE.' ') THEN   
      CTMP='BROWSE'
      IF(LASTDIR.NE.' ') CTMP=TRIM(LASTDIR)//'BROWSE'
      OPEN(4,FILE=TRIM(CTMP)//'.dot',IOSTAT=IER)
      WRITE(4,"('digraph tree {'/'node [style=filled]')")
      CTMP=CWK
      WRITE(4,211) NTREE(1)%GID,TRIM(CTMP),COP
 211  FORMAT(I10,' ->"',A,'=',F10.4,'";')
      COLOR=COLORS(1)                      
      CALL DRAWTREE(1,0,CWK,0,COLOR,100,0,.FALSE.)
      WRITE(4,211) NTREE(IDY)%GID,TRIM(CTMP),COP      
      COLOR=COLORS(2)
      CALL DRAWTREE(IDY,0,CWK,0,COLOR,100,0,.FALSE.)      
      WRITE(4,199) 'TREES FOR ',2,' ENTRIES:'//TRIM(CTMP)//
     &' (Red nodes, and the branches above them, contribute to'//
     &' more than one entry)'    
      CLOSE (4)
      CWK='BROWSE'
      IF(LASTDIR.NE.' ') CWK=TRIM(LASTDIR)//'BROWSE'      
      R=RUNQQ(TRIM(GVIZPATH)//'DOT',
     &'-Tgif '//TRIM(CWK)//'.dot -Gcharset=latin1 -o'//TRIM(CWK)//'.gif')
      R=RUNQQ(TRIM(VIEWER),TRIM(CWK)//'.gif')
C     &'C:\Program Files\IrfanView\i_view32.exe',TRIM(CWK)//'.gif')
      ENDIF     
      ENDIF
      GOTO 3
      ENDIF
C  COMMAND .ICP - INVERSE COP MATRIX FOR A LIST -----------------------
      IF(CMD.EQ.'.ICP') THEN
      IF(CWK(6:8).NE.'LST'.AND.CWK(6:8).NE.'lst') GOTO 3
      L=9
      LVL=INUMBA(IWK,L,80,J,K)
      IF(K.GE.0) LVL=100
      ILST=0
	NLST=0
      LDATA%LISTID=LIST
      LDATA%GID=0
      LDATA%ENTRYID=0
      LDATA%LRECID=0
      I4=GMS_getListData(LDATA,0)
	IF(I4.NE.1) THEN
	WRITE(0,156) BELL
      GOTO 3
	ENDIF
	WRITE(99,'(" FETCHING PEDIGREES ...")')
      DO WHILE (I4.EQ.1)
      ILST=ILST+1
      IF(ILST.NE.LDATA%ENTRYID) THEN
      WRITE(0,105) BELL,LIST
      GOTO 3
      ENDIF
	IF(ILST.GT.MAXLIST) THEN
	WRITE(0,155) BELL,ILST
      GOTO 3
	ENDIF
	LSTGID(ILST,1)=LDATA%GID
	LSTGID(ILST,2)=NLST+1
	LSTECD(ILST)=CTOFSTR(LDATA%ENTRYCD)
	CALL GETTREE(LSTGID(ILST,1),LVL,IER)
	WRITE(99,'(A1,I10,1X,A)') '+',NLST,'ENTRIES IN PEDIGREE LIST'
	IF(IER.NE.1) THEN
	WRITE(0,157) BELL,IER
      GOTO 3
	ENDIF
      I4=GMS_getListData(LDATA,1)
      ENDDO
C      ALLOCATE (COPMAT(ILST*(ILST+1)/2))
      WRITE(0,176) ILST,LIST,TRIM(CTOFSTR(LNAME%LISTNAME)),NLST
      WRITE(3,176) ILST,LIST,TRIM(CTOFSTR(LNAME%LISTNAME)),NLST
 176  FORMAT(' PEDIGREE TREE FOR',I4,' ENTRIES FROM LIST NO',I4,
     &' (',A,')'/' WITH APPROXIMATELY',I7,' DISTINCT ANCESTORS.')
	CALL LCOPI(ILST,LSTGID,LSTECD,COPMAT)
C	DEALLOCATE (COPMAT)
      GOTO 3
      ENDIF      
C  COMMAND .FCR --------------------------------------------------------------------
      IF(CMD.EQ.'.FCR') THEN
      NLST=0
      MXLST=6000
      ILST=0
      CALL FCROSS1(CWK(6:),IER)
      WRITE(0,117) TRIM(CWK(6:)),IER
      IF(NLST.GT.0) WRITE(3,117) TRIM(CWK(6:)),IER
 117  FORMAT(' SEARCH FOR CROSS: ',A,' (',I4,' UNITS USED)'/
     &' NO. NAMES',46X,'GID NTYP METH LOC  DATE')
      IF(IER.GE.MXLST.OR.IER.LE.0) THEN
      WRITE(0,120) IER
      WRITE(3,120) IER
 120  FORMAT(' *** WARNING *** ERROR IN FINDCROSS',I5)
      ENDIF
      DO 16 I=1,NLST
      I4=GMS_GetGermplasm2(LLST(I),GERM,NAMDAT,CWK,80)
      CWK=LSTNMS(NAMDAT%GERMPLASM,47)
      WRITE(0,116) I,TRIM(CWK),
     &NAMDAT%GERMPLASM,FCODE(NAMDAT%TYPE),MCODE(GERM%METHOD),
     &CNTRY(GERM%LOCATION),GERM%DATE/10000
      WRITE(3,116) I,TRIM(CWK),
     &NAMDAT%GERMPLASM,FCODE(NAMDAT%TYPE),MCODE(GERM%METHOD),
     &CNTRY(GERM%LOCATION),GERM%DATE/10000
 116  FORMAT(I3,1X,A,T52,I8,1X,A5,1X,2A4,I5)
 16   CONTINUE
      GOTO 3
      ENDIF
C  COMMAND .EXT LST ------------------------------------------------------------
      IF(CMD.EQ.'.EXT') THEN
      IF(CWK(6:8).EQ.'LST'.OR.CWK(6:8).EQ.'lst') THEN
      L=10
      LVL=INUMBA(IWK,L,80,J,K)
      IF(K.GE.0) LVL=100      
	LSTP=1
	NLST=0
      ILST=0
      LDATA%LISTID=LIST
      LDATA%GID=0
      LDATA%ENTRYID=0
      LDATA%LRECID=0
      I4=GMS_getListData(LDATA,0)
	IF(I4.NE.1) THEN
	WRITE(0,156) BELL
      GOTO 3
	ENDIF
	WRITE(99,'("  FETCHING PEDIGREES ...")')
      DO WHILE (I4.EQ.1)
      ILST=ILST+1
      IF(ILST.NE.LDATA%ENTRYID) THEN
      WRITE(0,105) BELL,LIST
      GOTO 3
      ENDIF
	IF(ILST.GT.MAXLIST) THEN
	WRITE(0,155) BELL,ILST
      GOTO 3
	ENDIF
	LSTGID(ILST,1)=LDATA%GID
C  CHECK IF THE ENTRY IS ALREADY IN THE PEDIGREE LIST
	LSTGID(ILST,2)=0
	I=0
	IF(NLST.GT.0) THEN
      DO WHILE (LSTGID(ILST,2).EQ.0.AND.I.LT.NLST)
      I=I+1
	IF(LSTGID(ILST,1).EQ.NTREE(I)%GID) LSTGID(ILST,2)=I
      ENDDO
	ENDIF
      IF(LSTGID(ILST,2).EQ.0) THEN 
	LSTGID(ILST,2)=NLST+1
	CALL GETTREE(LSTGID(ILST,1),LVL,IER)
	WRITE(99,'(A1,I10,1X,A)') '+',NLST,'ENTRIES IN PEDIGREE LIST'
	ENDIF
	IF(IER.NE.1) THEN
	WRITE(0,157) BELL,IER
      GOTO 3
	ENDIF
      I4=GMS_getListData(LDATA,1)
      ENDDO
C
C  THE PEDIGREE TREE FOR THE LIST IS CONTAINED IN NTREE.
C  FOR EACH LINE IN THE PEDIGREE, NTREE(I) CONTAINS THE GID, METHN,
C  GNPGS AND POINTERS TO THE SOURCE (DADDR) AND/OR THE FIRST PARENT (GADDR).
C
C  GNPGS IS -1 OF THE CURRENT ENTRY IS DERIVATIVE OR CONTAINS THE NUMBER
C  OF PROGENITORS IF IT IS GENERATIVE
C
C  DADDR CONTAINS THE LINE NUMBER FOR THE SOURCE OF THE CURRENT ENTRY,  
C  IT IS -1 IF THE LINE IS GENERATIVE OR A DERIVATIVE GROUP SOURCE
C  AND IT IS 0 IF THE LINE HAS KNOWN GROUP BUT UNKNOWN SOURCE.
C
C  GADDR CONTAINS THE LINE NUMBER FOR THE GROUP OF THE CURRENT ENTRY IF
C  IT IS DERIVATIVE OR THE FIRST PARENT IF IT IS GENERATIVE;
C  SUBSEQUENT PARENTS OCCUR ON LINES GADDR+1 TO GADDR+GNPGS-1. 
C  GADDR=0 IF THE LINE IS A DERIVATIVE GROUP SOURCE OR AN UNKNOWN GENERATIVE.
C  IT IS -1 IF EXPANSION WAS TERMINATED DUE TO THE LEVEL PARAMETER.
C
      WRITE(3,193) LIST
 193  FORMAT(' PEDIGREE TREE FOR LIST WITH ID',I5)
      ELSE
	WRITE(0,'(A,"*** ERROR *** .EXT ONLY AVAILABLE AS A LST CMD")')
     & BELL
	GOTO 3
	ENDIF
	WRITE(3,194)
 194  FORMAT(/
     &' GNPGS IS -1 IF THE CURRENT ENTRY IS DERIVATIVE OR CONTAINS'/
     &' THE NUMBER OF PROGENITORS IF IT IS GENERATIVE'//
     &' DADDR CONTAINS THE LINE NO FOR THE SOURCE OF THE',
     &' CURRENT ENTRY',/
     &' IT IS -1 IF THE ENTRY IS GENERATIVE OR A DERIVATIVE',
     &' GROUP SOURCE'/
     &' AND IT IS 0 IF THE LINE HAS UNKNOWN SOURCE.'//
     &' GADDR CONTAINS THE LINE NO FOR THE GROUP OF THE CURRENT ENTRY'/
     &' IF IT IS DERIVATIVE OR THE FIRST PARENT IF IT IS GENERATIVE;'/
     &' SUBSEQUENT PARENTS OCCUR ON LINES GADDR+1 TO GADDR+GNPGS-1.'/
     &' IT IS 0 IF THE ENTRY IS A DERIVATIVE GROUP SOURCE OR AN',
     &' UNKNOWN GENERATIVE'/
     &' AND IT IS -1 IF EXPANSION WAS TERMINATED DUE TO THE LEVEL',
     &' PARAMETER'/
     &' ORDER AND EXORD NOT USED HERE'//3X,
     &' LINE   GID      METHN   GNPGS    DADDR   GADDR   ORDER   EXORD')
      DO 190,I=1,NLST
	DO J=1,ILST
	IF(LSTGID(J,2).EQ.I) WRITE(3,191) J,LSTGID(J,1),I 
 191  FORMAT(60(1H-)/' LIST ENTRY',I5,' GID',I9,' STARTS AT LINE',I6)
      ENDDO
 190  WRITE(3,'(I6,I10,6I8)') I,NTREE(I)
      IER=1
C      CALL MAKELIST('EXPORT PEDIGREES','SAVE',' ',LIST,IER)
C	WRITE(99,'(1X,"SAVING PROGENITORS IN LIST",I4/)') LIST
C      DO I=1,NLST
C	WRITE(99,'(A1,I10,1X,A)') '+',I,'ENTRIES SAVED'
C      IF(IER.EQ.1) CALL ADDLIST(LIST,'.LST',NTREE(I)%GID,NTRY,IER)
C      ENDDO
C      IF(IER.NE.1) THEN
C      WRITE(0,118) BELL,'*** ERROR *** LST FUNCTION ABANDONED',' '
C      IF(IER.EQ.-1) CALL GMS_rollbackData()
C      ELSE
C      CALL GMS_commitData()
C      ENDIF
      GOTO 3
      ENDIF  
C  COMMAND .TRE LST Display Pedigree Trees -----------------------------------
      IF(CMD.EQ.'.TRE') THEN
      IF(.NOT.(CWK(6:8).EQ.'LST'.OR.CWK(6:8).EQ.'lst')) THEN
C  TREE FOR A SINGLE GID -----------------------------------------------------
      LVL=0
      IF(CWK(5:).NE.' ') THEN
      L=5
      IGID=INUMBA(IWK,L,80,J,K)
      IF(K.GT.0) GOTO 5
      IF(L.LT.80) LVL=INUMBA(IWK,L,80,J,K)
      IF(K.GT.0) GOTO 5
      IF(IGID.NE.0) CALL WRTGERM(IGID,GERM,3)
      ENDIF
      IF(IGID.EQ.0) GOTO 5
      IF(LVL.EQ.0) LVL=-1      
      IF(LVL.EQ.-1) LVL=3
      NLST=0
      LSTP=1
      CALL GETTREE(IGID,LVL,IER)
      IF(IER.NE.1) PAUSE
      IF(VIEWER.NE.' ') THEN
      CLWK=' '
      IF(LASTDIR.NE.' ') CWK=TRIM(LASTDIR)//'BROWSE'
      OPEN(4,FILE=TRIM(CWK)//'.DOT',IOSTAT=IER)
      WRITE(4,"('digraph tree {')")
      ENDIF
      COLOR='black'    
      CALL DRAWTREE(1,0,CWK,0,COLOR,LVL,0,.TRUE.)
      IF(VIEWER.NE.' ') THEN
      NAMDAT%GERMPLASM=NTREE(1)%GID
      NAMDAT%TYPE=0
      NAMDAT%STATUS=1
      CWK='<Unknown>'C
      I4=GMS_getName(NAMDAT,CWK,256,0)
      WRITE(4,199) 'GID:',NTREE(1)%GID,TRIM(CTOFSTR(CWK))
 199  FORMAT('overlap=false'/'label="'A,I8,' NAME: ',A,'"'/
     &'fontsize=20;'/'}')      
      CLOSE (4)
      CWK='BROWSE'
      IF(LASTDIR.NE.' ') CWK=TRIM(LASTDIR)//'BROWSE'      
      R=RUNQQ(TRIM(GVIZPATH)//'DOT',
     &'-Tgif '//TRIM(CWK)//'.dot -Gcharset=latin1 -o'//TRIM(CWK)//'.gif')
      R=RUNQQ(TRIM(VIEWER),TRIM(CWK)//'.gif')
C     &'C:\Program Files\IrfanView\i_view32.exe',TRIM(CWK)//'.gif')
      GOTO 3
      ENDIF
C  TREES FOR ALL ENTRIES IN A LIST -------------------------------------------      
      ELSE
      L=10
      LVL=INUMBA(IWK,L,80,J,K)
      IF(K.GE.0) LVL=100      
	LSTP=1
	NLST=0
      ILST=0
      WRITE(CLWK,'("LISTID:",I5)') LIST
      LDATA%LISTID=LIST
      LDATA%GID=0
      LDATA%ENTRYID=0
      LDATA%LRECID=0
      I4=GMS_getListData(LDATA,0)
	IF(I4.NE.1) THEN
	WRITE(0,156) BELL
      GOTO 3
	ENDIF
	WRITE(99,'("  FETCHING PEDIGREES ...")')
      DO WHILE (I4.EQ.1)
      ILST=ILST+1
      IF(ILST.NE.LDATA%ENTRYID) THEN
      WRITE(0,105) BELL,LIST
      GOTO 3
      ENDIF
	IF(ILST.GT.MAXLIST) THEN
	WRITE(0,155) BELL,ILST
      GOTO 3
	ENDIF
	LSTGID(ILST,1)=LDATA%GID
C  CHECK IF THE ENTRY IS ALREADY IN THE PEDIGREE LIST
	I=0
	IF(NLST.GT.0) THEN
      DO WHILE (LSTGID(ILST,2).EQ.0.AND.I.LT.NLST)
      I=I+1
	IF(LSTGID(ILST,1).EQ.NTREE(I)%GID) NTREE(I)%ORDER=NTREE(I)%ORDER+1
      ENDDO
	ENDIF
	LSTGID(ILST,2)=NLST+1
	CALL GETTREE(LSTGID(ILST,1),LVL,IER)
	DO I=LSTGID(ILST,2),NLST
	NTREE(I)%ORDER=1
	ENDDO
	WRITE(99,'(A1,I10,1X,A)') '+',NLST,'ENTRIES IN PEDIGREE LIST'
	IF(IER.NE.1) THEN
	WRITE(0,157) BELL,IER
      GOTO 3
	ENDIF
      I4=GMS_getListData(LDATA,1)
      ENDDO
      LSTGID(ILST+1,2)=NLST+1
C  LSTGID(ILIST,1) CONTAINS THE GID OF THE ILIST ENTRY IN THE LIST
C  LSTGID(ILIST,2) CONTAINS THE START POSITION IN NTREE FOR THE ILSIT
C  ENTRY IN THE LIST
C
C  NLST ENTRIES FOR THE COMBINED PEDIGREE TREE FOR THE LIST ARE CONTAINED IN NTREE.
C  FOR EACH LINE IN THE PEDIGREE, NTREE(I) CONTAINS THE GID, METHN,
C  GNPGS AND POINTERS TO THE SOURCE (DADDR) AND/OR THE FIRST PARENT (GADDR).
C
C  GNPGS IS -1 OF THE CURRENT ENTRY IS DERIVATIVE OR CONTAINS THE NUMBER
C  OF PROGENITORS IF IT IS GENERATIVE
C
C  DADDR CONTAINS THE LINE NUMBER FOR THE SOURCE OF THE CURRENT ENTRY,  
C  IT IS -1 IF THE LINE IS GENERATIVE OR A DERIVATIVE GROUP SOURCE
C  AND IT IS 0 IF THE LINE HAS KNOWN GROUP BUT UNKNOWN SOURCE.
C
C  GADDR CONTAINS THE LINE NUMBER FOR THE GROUP OF THE CURRENT ENTRY IF
C  IT IS DERIVATIVE OR THE FIRST PARENT IF IT IS GENERATIVE;
C  SUBSEQUENT PARENTS OCCUR ON LINES GADDR+1 TO GADDR+GNPGS-1. 
C  GADDR=0 IF THE LINE IS A DERIVATIVE GROUP SOURCE OR AN UNKNOWN GENERATIVE.
C  IT IS -1 IF EXPANSION WAS TERMINATED DUE TO THE LEVEL PARAMETER.
C
C      WRITE(3,193) LIST
C 193  FORMAT(' PEDIGREE TREE FOR LIST WITH ID',I5)
	ENDIF
C write out NTREE        	
C	WRITE(3,194)
C 194  FORMAT(/
C     &' GNPGS IS -1 IF THE CURRENT ENTRY IS DERIVATIVE OR CONTAINS'/
C     &' THE NUMBER OF PROGENITORS IF IT IS GENERATIVE'//
C     &' DADDR CONTAINS THE LINE NO FOR THE SOURCE OF THE',
C     &' CURRENT ENTRY',/
C     &' IT IS -1 IF THE ENTRY IS GENERATIVE OR A DERIVATIVE',
C     &' GROUP SOURCE'/
C     &' AND IT IS 0 IF THE LINE HAS UNKNOWN SOURCE.'//
C     &' GADDR CONTAINS THE LINE NO FOR THE GROUP OF THE CURRENT ENTRY'/
C     &' IF IT IS DERIVATIVE OR THE FIRST PARENT IF IT IS GENERATIVE;'/
C     &' SUBSEQUENT PARENTS OCCUR ON LINES GADDR+1 TO GADDR+GNPGS-1.'/
C     &' IT IS 0 IF THE ENTRY IS A DERIVATIVE GROUP SOURCE OR AN',
C     &' UNKNOWN GENERATIVE'/
C     &' AND IT IS -1 IF EXPANSION WAS TERMINATED DUE TO THE LEVEL',
C     &' PARAMETER'/
C     &' ORDER AND EXORD NOT USED HERE'//3X,
C     &' LINE   GID      METHN   GNPGS    DADDR   GADDR   ORDER   EXORD')
C      DO I=1,NLST
C	DO J=1,ILST
C	IF(LSTGID(J,2).EQ.I) WRITE(3,191) J,LSTGID(J,1),I 
C 191  FORMAT(' LIST ENTRY',I4,' GID',I9,' STARTS AT LINE',I4)
C      ENDDO
C      WRITE(3,'(I6,I10,6I8)') I,NTREE(I)
C      ENDDO
      IER=1
C draw the trees for each entry using the old DRAWTREE2      
C      DO J=1,ILST
C      WRITE(0,191) J,LSTGID(J,1),LSTGID(J,2) 
C      WRITE(3,191) J,LSTGID(J,1),LSTGID(J,2)                 
C      CALL DRAWTREE2(LSTGID(J,2),0,CWK,LVL,0)
C      ENDDO
C draw the trees for each entry using the new DRAWTREE with plotting
      IF(VIEWER.NE.' ') THEN   
      CWK='BROWSE'
      IF(LASTDIR.NE.' ') CWK=TRIM(LASTDIR)//'BROWSE'
      OPEN(4,FILE=TRIM(CWK)//'.dot',IOSTAT=IER)
      WRITE(4,"('digraph tree {'/'node [style=filled]')")
      ENDIF     
      DO J=1,ILST
      COLOR='lightgrey'
      IF(J.LE.20) COLOR=COLORS(J)
      WRITE(0,191) J,LSTGID(J,1),LSTGID(J,2) 
      WRITE(3,191) J,LSTGID(J,1),LSTGID(J,2)
      IF(VIEWER.NE.' ') THEN
      WRITE(4,210) NTREE(LSTGID(J,2))%GID,LIST
 210  FORMAT(I10,' ->"LISTID:',I5,'";')
      ENDIF                      
      CALL DRAWTREE(LSTGID(J,2),0,CWK,0,COLOR,LVL,0,.TRUE.)
      ENDDO
      IF(VIEWER.NE.' ') THEN
      CALL ZEROLSTNM(LNAME)
      LNAME%LISTID=LIST
      I4=GMS_getListName(LNAME,0)   
      WRITE(4,199) 'LISTID:',LIST,TRIM(CTOFSTR(LNAME%LISTNAME))//
     &' (Red nodes, and the branches above them, contribute to'//
     &' more than one entry)'    
      CLOSE (4)
      CWK='BROWSE'
      IF(LASTDIR.NE.' ') CWK=TRIM(LASTDIR)//'BROWSE'      
      R=RUNQQ(TRIM(GVIZPATH)//'DOT',
     &'-Tgif '//TRIM(CWK)//'.dot -Gcharset=latin1 -o'//TRIM(CWK)//'.gif')
      R=RUNQQ(TRIM(VIEWER),TRIM(CWK)//'.gif')
C     &'C:\Program Files\IrfanView\i_view32.exe',TRIM(CWK)//'.gif')
      ENDIF
      GOTO 3
      ENDIF
C------------------------------------------------------------------------
C  SEARCH FOR THE NAME, LIST ALL OCCURANCES (UP TO A MAXIMUM
C  OF MXLST), LIST ALL NAMES ASSOCIATED WITH THOSE OCCURANCES.
C
 5    MXLST=500
      LSTP=0
      LSTSRCH=CWK
      II=0
      IF(IOPT.EQ.2) II=-1
      CALL NAMLST(II,CWK,IGID,ISRH,FCPS)
      CTMP=CWK
      IF(CTMP.NE.' ') GOTO 19
      IF(IOPT.EQ.1.AND.NLST.EQ.1) THEN
      CWK='.1'
      GOTO 8
      ENDIF
      IF(IOPT.EQ.2) GOTO 3
      IF(INP.EQ.1.AND.NLST.GT.0) GOTO 9
      IF(NLST.EQ.0) WRITE(3,112) CWK(1:LEN_TRIM(CWK))//' NOT FOUND'
 112  FORMAT(1X,A,T68,'GID:       0')
      GOTO 3
 6    CALL GMS_closeDatabase()
      STOP
 7    WRITE(0,118) BELL,' *** ERROR *** INPUT ERROR:',TRIM(CTMP)
      GOTO 4
 21   WRITE(0,118) BELL,' *** ERROR *** TOO MANY ENTRIES'
      GOTO 3
      END
C******************************************************* LCOP
C  PURPOSE:
C  CALCULATE COEFFICIENTS OF PARENTAGE FOR ALL PAIRS
C  OF ENTRIES IN A LIST
C  ARGUMENTS:
C  ILST   - NUMBER OF ENTRIES EXTRACTED FROM THE LIST
C  LSTGID - CONTAINS THE GIDs AND THEIR ENTRY POINTS IN
C           THE COMBINED PEDIGREE TREE CONTAINED IN GTREE
C  COPMAT - RETURNS WITH THE LT MATRIX OF COP VALUES IF
C           ILIST<=2000, ELSE NOT USED
C
C  USES RECURSIVE FUNCTION COP
C---- 27/05/06 ----------------------------------------- LCOP
      SUBROUTINE LCOP(ILST,LSTGID,LSTECD,COPMAT)
      USE GMS_STRUCTURES
      USE GMS_TOOLS
      USE GMS_INTERFACE
      use modSparseMatrix
      PARAMETER (MAXTREE=30000,MAXLIST=3000) 
      type(cMatrix) :: COPVALS
	DIMENSION COPMAT(*),LSTGID(MAXLIST,2)
      CHARACTER XNAME*80,YNAME*80,LSTECD(MAXLIST)*48,VIEWER*60
      CHARACTER GVIZPATH*60
      TYPE(GMS_TREE) GTREE(MAXTREE)
      TYPE(GMS_NAMEDATA) NAME
      REAL*4 COPIJ
      COMMON /WRK/ NCR,MXTREE,GTREE
      COMMON /OPTIONS/ NUID,NLOC,NREF,IOPT,LIST,ISRH,MGAT,NAMP,NTCX
     &,IBTYPE,LDST,IYAL,NMSTD,VIEWER,GVIZPATH
C FIND THE ORDER OF EACH PROGENITOR IN THE COMBINED TREE
C 1. Mark the terminal ancestors
      DO I=1,NCR
      GTREE(I)%ORDER=0
      IF(GTREE(I)%GADDR.LE.0) GTREE(I)%ORDER=1
      IF(GTREE(I)%GADDR.LT.0) GTREE(I)%GADDR=0      
      ENDDO
C 2. Find the longest generative path from each ancestor to any terminal ancestor      
  25  L=-1
      DO 26,I=1,NCR
      IF(GTREE(I)%ORDER.GT.0) GOTO 26
      L=0
      DO J=GTREE(I)%GADDR,GTREE(I)%GADDR+IABS(GTREE(I)%GNPGS)-1
      K=GTREE(J)%ORDER
      IF(K.EQ.0) GOTO 26
      L=MAX(L,K)
      ENDDO
      GTREE(I)%ORDER=L+1
 26   CONTINUE
      IF(L.GE.0) GOTO 25
C      DO 10,I=1,NCR
C 10   WRITE(3,'(7I10)') I,GTREE(I)
      call initializeStack(10)
      call setMatrix(COPVALS,NCR,NCR)
      IJ=0
      DO 18 I=1,ILST
      NAME%GERMPLASM=LSTGID(I,1)
      NAME%TYPE=0
      NAME%STATUS=1
      XNAME='<Unknown>'C
      I4=GMS_getName(NAME,XNAME,80,0)
      XNAME=CTOFSTR(XNAME)
      IF(ILST.LE.100) THEN
	WRITE(0,'(4H ROW,I4,36(1H-),1X,A)') I,TRIM(XNAME)//' WITH'
	ELSE
      WRITE(99,'(A1,3HROW,I10,3H OF,I10)') '+',I,ILST
      ENDIF
      DO 17 J=1,I
      IJ=IJ+1
      IF(IBTYPE.GT.0) THEN
      COPIJ=COPSF(LSTGID(I,2),LSTGID(J,2),COPVALS)
      ELSE
      COPIJ=COPCF(LSTGID(I,2),LSTGID(J,2),COPVALS)
      ENDIF
      IF(ILST.LE.2000) COPMAT(IJ)=COPIJ
      NAME%GERMPLASM=LSTGID(J,1)
      NAME%TYPE=0
      NAME%STATUS=1
      YNAME='<Unknown>'C
      I4=GMS_getName(NAME,YNAME,80,0)
      YNAME=CTOFSTR(YNAME)
      IF(ILST.LE.100) WRITE(0,100) LSTGID(I,1),LSTGID(J,1),COPIJ,
     & TRIM(YNAME)//')'
      WRITE(3,101) LSTGID(I,1),LSTGID(J,1),I,J,COPIJ,
     & TRIM(LSTECD(I)),TRIM(LSTECD(J)),TRIM(XNAME),TRIM(YNAME)
 100  FORMAT(' COP FOR GIDS',I9,' AND',I9,' IS',F9.4,
     &' (',A,':',A,'), BTYPE=',I2)
 101  FORMAT(2I10,2I5,1X,F9.4,4(' "',A,'"'))
 17   CONTINUE
 18   CONTINUE
      WRITE(0,'(" NO. OF ELEMENTS IN COPVALS",I8)') getCount(COPVALS)
      CALL destroySparseMatrix(COPVALS,*99)
	call deallocateStack()
      RETURN
 99   WRITE(0,'(" ERROR DELETING COPVALS MATRIX")')
      RETURN
      END
C******************************************************* LCOPI
C  PURPOSE:
C  TO EXPORT THE TREE FOR A LIST USING OFFSPRING DAM SIRE FORMAT
C  ONLY BIPARENTAL CROSSES CAN BE CONSIDERED AT THE MOMENT
C  THE TREE IS CONTAINED IN COMMON /WRK/ (gtree) AND MAY HAVE BEEN 
C  EXTRACTED TO TERMINATE AT A GIVEN NUMBER OF LEVELS
C  THE OBJECTIVE WAS TO CALCULATE INVERSE COP MATRIX FOR ALL
C  PAIRS OF ENTRIES IN THE LIST DIRECTLY BUT THIS IS NOT DONE YET.
C  ARGUMENTS:
C  ILST   - NUMBER OF ENTRIES EXTRACTED FROM THE LIST
C  LSTGID - CONTAINS THE GIDs AND THEIR ENTRY POINTS IN
C           THE COMBINED PEDIGREE TREE CONTAINED IN GTREE
C  COPMAT - RETURNS WITH THE LT MATRIX OF COP VALUES
C
C---- 14/02/08 ----------------------------------------- LCOPI
      SUBROUTINE LCOPI(ILST,LSTGID,LSTECD,COPMAT)
      USE GMS_STRUCTURES
      USE GMS_TOOLS
      USE GMS_INTERFACE
      use modSparseMatrix
      PARAMETER (MAXTREE=30000,MAXLIST=3000)
      type(cMatrix) :: COPVALS
	DIMENSION COPMAT(*),LSTGID(MAXLIST,2)
      CHARACTER XNAME*80,YNAME*80,LSTECD(MAXLIST)*48,VIEWER*60
      CHARACTER GVIZPATH*60
      TYPE(GMS_TREE) GTREE(MAXTREE)
      TYPE(GMS_NAMEDATA) NAME
      COMMON /WRK/ NCR,MXTREE,GTREE
      COMMON /OPTIONS/ NUID,NLOC,NREF,IOPT,LIST,ISRH,MGAT,NAMP,NTCX
     &,IBTYPE,LDST,IYAL,NMSTD,VIEWER,GVIZPATH
C FIND THE ORDER OF EACH PROGENITOR IN THE COMBINED TREE
C 1. Mark the terminal ancestors
      DO I=1,NCR
      GTREE(I)%ORDER=0
      IF(GTREE(I)%GADDR.LE.0) GTREE(I)%ORDER=1
      IF(GTREE(I)%GADDR.LT.0) GTREE(I)%GADDR=0      
      ENDDO
C 2. Find the longest generative path from each ancestor to any terminal ancestor
C    AND PUT THE LEVEL ALONG THIS PATH (PARENTS->OFFSPRING) INTO GTREE(I)%ORDER
C    M IS THE HIGHEST LEVEL (LONGEST PATH)
      M=0      
  25  L=-1
      DO 26,I=1,NCR
      IF(GTREE(I)%ORDER.GT.0) GOTO 26
      L=0
      DO J=GTREE(I)%GADDR,GTREE(I)%GADDR+IABS(GTREE(I)%GNPGS)-1
      K=GTREE(J)%ORDER
      IF(K.EQ.0) GOTO 26
      L=MAX(L,K)
      ENDDO
      GTREE(I)%ORDER=L+1
      M=MAX(M,GTREE(I)%ORDER)
 26   CONTINUE
      IF(L.GE.0) GOTO 25
C  PUT THE SORT ORDER OF THE TREE (PARENTS->OFFSPRING) 
C  INTO GTREE(I)%EXORD KEEPING ORDER WITHIN LEVELS 
      K=0
      DO 1 L=1,M
      DO 2 I=1,NCR
      IF(GTREE(I)%ORDER.NE.L) GOTO 2
      K=K+1
      GTREE(K)%EXORD=I
 2    CONTINUE     
 1    CONTINUE
C SET GTREE()%ORDER TO THE ORIGINAL POSITION OF THE ITH
C STRAIN IN THE SORTED PEDIGREE
C WRITE OUT THE FULL SERIAL PEDIGREE IN SORTED ORDER 
      DO 10 I=1,NCR
      IDX=GTREE(I)%EXORD
      GTREE(IDX)%ORDER=I
C      WRITE(3,'(8I8)') I,GTREE(I),GTREE(I)%ORDER
c      WRITE(3,'(8I8)') IDX,GTREE(IDX),GTREE(IDX)%ORDER
 10   CONTINUE
      DO 3 I=1,NCR
      IF(GTREE(I)%GNPGS.EQ.-1) THEN
      IF(GTREE(I)%GADDR.GT.0) GTREE(GTREE(GTREE(I)%GADDR)%ORDER)%EXORD=0
C      ITORD(ITORD(GTREE(I)%GADDR,1),2)=0
      ENDIF
      IF(GTREE(I)%GNPGS.GT.2) THEN
      WRITE(0,100) GTREE(I)%GID,GTREE(I)%GNPGS
      WRITE(3,100) GTREE(I)%GID,GTREE(I)%GNPGS      
 100  FORMAT(' *** WARNING *** GID',I9,' HAS',I8,' PROGENITORS',
     &' ICP OUTPUT ONLY CONSIDERS BIPARENTAL CROSSES')
      ENDIF
      DO 5 J=I+1,NCR
      IF(GTREE(I)%GID.EQ.GTREE(J)%GID) GTREE(GTREE(J)%ORDER)%EXORD=0
C      ITORD(ITORD(J,1),2)=0
 5    CONTINUE 
 3    CONTINUE     
C WRITE OUT THE TREE FOR BIPARENTAL CROSSES IN PARALLEL
C COUNT DERIVATIVE STEPS BUT DO NOT PRINT THEM OUT
      WRITE(3,101)
 101  FORMAT(' OFFSPRING FEMALE-PARENT MALE-PARENT SELFING METHOD NAME '
     &     /'    GID         GID         GID    GENERATIONS'/
     &25X,' FOR SELFING GENERATIONS -1 IS UNKNOWN')
      DO 4 I=1,NCR
      IF(GTREE(I)%EXORD.GT.0) THEN
      IDX=GTREE(I)%EXORD
      IO=GTREE(IDX)%GID
      IF(IO.EQ.2) THEN
      IM=0
      ENDIF
      IM=0
      IP=0
      IG=IDX
      NSTP=0      
      IF(GTREE(IDX)%GNPGS.EQ.-1) THEN
C  COUNT DDERIVATIVE STEPS AND FLAG THEM WITH ZERO IN GTREE(*)%EXORD
C  SO THAT THEY WILL NOT BE PRINTED
      NSTP=-1
      IF(GTREE(IDX)%DADDR.GT.0) THEN
        NSTP=0
        II=GTREE(IDX)%DADDR
        DO WHILE (GTREE(II)%DADDR.GT.0)
          GTREE(GTREE(II)%ORDER)%EXORD=0        
          II=II+1
        ENDDO
        NSTP=II-GTREE(IDX)%DADDR+1
        IF(GTREE(II)%DADDR.GE.0) NSTP=-1
        IF(GTREE(II)%GNPGS.LT.0) NSTP=-1        
      ENDIF
      IG=GTREE(IDX)%GADDR
      ENDIF    
      IF(IG.GT.0) THEN
      IF(GTREE(IG)%GNPGS.GT.0.AND.GTREE(IG)%GADDR.GT.0) THEN      
      IM=GTREE(GTREE(IG)%GADDR)%GID
      IP=GTREE(GTREE(IG)%GADDR+1)%GID
      ENDIF
      ENDIF
      NAME%GERMPLASM=IO
      NAME%TYPE=0
      NAME%STATUS=1
      XNAME='<Unknown>'C
      I4=GMS_getName(NAME,XNAME,80,0)
      XNAME=CTOFSTR(XNAME)     
      WRITE(3,'(5I10,1X,A)') IO,IM,IP,NSTP,GTREE(IDX)%METHN,TRIM(XNAME)
      ENDIF 
 4    CONTINUE
C CALCULATE A-INVERSE
c      DO 9 I=1,NCR
c      FI=1.0
c      
c      DO 8 J=I,NCR
c      
c 8    CONTINUE
c 9    CONTINUE           
      RETURN
      END      
C******************************************************* TRACE
C  PURPOSE:
C  TRACE DESCENDANTS OF GERMPLASM GERM
C
C  ARGUMENTS:
C  GERM - GERMPLASM RECORD FOR GEN TO BE TRACED
C  NGEN - NUMBER OF GENERATIONS TO TRACE (-1 FOR ALL)
C  COMMON ARGUMENTS:
C  LIST - INPUT LISTID: 0 FOR NEW LIST OR LISTID OF
C         A PARTIAL TRACE LIST TO CONTINUE A TRACE
C---- LAST UPDATE 01/05/04 ----------------------------- TRACE
      SUBROUTINE TRACE(GERM,NGEN)
      USE GMS_STRUCTURES
      USE GMS_INTERFACE
      USE GMS_TOOLS
      USE GMS_FUNCTIONS
      PARAMETER (MAXTREE=30000,MAXLIST=3000)
      DIMENSION NTAB(7)
      CHARACTER NULL*1,CWK*80,NAME*80,CTMP*10,CTMP1*5,VIEWER*60
      CHARACTER GVIZPATH*60
      TYPE(GMS_GERMPLASM) GERM
      TYPE(GMS_NAMEDATA) NAMDAT
      TYPE(GMS_LISTDATA) LDATA,MDATA
      TYPE(GMS_LISTNAME) LSTN
      COMMON /OPTIONS/ NUID,NLOC,NREF,IOPT,LIST,ISRH,MGAT,NAMP,NTCX
     &,IBTYPE,LDST,IYAL,NMSTD,VIEWER,GVIZPATH
      COMMON /WRK/ MLST,MXLST,LLST(MAXLIST)
      DATA NULL /0/
      MXLST=3000
      MLST=0
      IF(LIST.EQ.0) THEN
        IGID=GERM%GERMPLASM_ID
        I4=GMS_getGermplasm2(IGID,GERM,NAMDAT,NAME,80)
        WRITE(CTMP,'(I10)') IGID
        J=VERIFY(CTMP,' ')
        NAME=CTOFSTR(NAME)
        I=MIN(40,LEN_TRIM(NAME))
        WRITE(CWK,101) '000',CTMP(J:10),NAME(1:I)
 101    FORMAT('TRACE OF ',A3,' GENERATIONS FOR GID ',A,' (',A,')')
        CALL MAKELIST('TRACE','TRACE',CWK,LIST,IER)
        CALL ZEROLSTNM(LSTN)
        LSTN%LISTID=LIST
        I4=GMS_getListName(LSTN,0)
        IF(IER.NE.1.AND .I4.NE.1) THEN
          WRITE(0,111)
 111      FORMAT(' *** ERROR *** CANNOT OPEN TRACE LIST')
          RETURN
        ENDIF
C      LIST=LSTN%LISTID
        NLST=1
        KLST=1
        JLST=1
        IGEN=0
        INBR=NDSTEPS(GERM)+1
        IF(INBR.EQ.0) INBR=9
        MDATA%GID=GERM%GERMPLASM_ID
        WRITE(CTMP1,'(I3)') INBR
        MDATA%ENTRYCD=FTOCSTR(CTMP1)
        MDATA%SOURCE=NULL
        MDATA%DESIG=NULL
        MDATA%GRPNAME=NULL
        MDATA%LISTID=LIST
        MDATA%ENTRYID=0
        MDATA%LRECID=0
        I4=GMS_addListData(MDATA)
        IF(I4.NE.1) THEN
        WRITE(0,112)
 112    FORMAT(' *** ERROR *** CANNOT WRITE TO TRACE LIST')
        RETURN
        ENDIF
      ELSE
C USE PARTIAL TRACE IN OPEN LIST
        CALL ZEROLSTNM(LSTN)
        LSTN%LISTID=LIST
        I4=GMS_getListName(LSTN,0)
        IF(I4.NE.1) THEN
        WRITE(0,111)
        RETURN
        ENDIF
        IGEN=-1
        NLST=0
        JLST=0
        LDATA%LISTID=LIST
        LDATA%GID=0
        LDATA%LRECID=0
        LDATA%ENTRYID=0
        CTMP='         0'
 10     I4=GMS_getListData(LDATA,0)
        DO WHILE (I4.EQ.1)
        NLST=NLST+1
        IF(IGEN.LT.0.AND.LDATA%ENTRYCD(5:5).EQ.'?') THEN
        JLST=NLST
        READ(CTMP,'(5X,I5)') IGEN
        ENDIF
        CTMP=LDATA%ENTRYCD
        I4=GMS_getListData(LDATA,1)
        ENDDO
        IF(IGEN.LT.0) JLST=NLST+1
        KLST=NLST
      ENDIF
      WRITE(0,105)
      WRITE(3,105)
 105  FORMAT(' GEN.   NUMBER OF DESCENDANTS'/8X,'TOTAL',4X,
     &'GENERATIVE DERIVATIVE ACCESSIONS TESTS   RELEASES TERMINALS')
 3    IGEN=IGEN+1
      DO 9 I=1,7
 9    NTAB(I)=0
      DO 5 ILST=JLST,KLST
      LDATA%LISTID=LIST
      LDATA%GID=0
      LDATA%LRECID=0
      LDATA%ENTRYID=ILST
      I4=GMS_getListData(LDATA,0)
      IF(I4.NE.1) STOP 'TRACE4'
      IGID=LDATA%GID
      INBR=10
      IF(LDATA%ENTRYCD(1:1).NE.'*') READ(LDATA%ENTRYCD,'(I1)') INBR
      I4=GMS_getGermplasm2(IGID,GERM,NAMDAT,NAME,80)
      LDATA%SOURCE=CNTRY(GERM%LOCATION)//NULL
      WRITE(CTMP,'(I1,4X,I5)') INBR,IGEN
C      WRITE(0,111) IGEN,JLST,KLST,ILST,IGID,NLST
C 111  FORMAT(6I10)
      IF(NAMDAT%TYPE.EQ.4) CTMP(4:4)='R'
      NTAB(1)=NTAB(1)+1
      IF(INBR.EQ.1) NTAB(2)=NTAB(2)+1
      IF(INBR.GT.1) NTAB(3)=NTAB(3)+1
      KGID=GROUPID(GERM)
C A=ACCESSION
      NAMDAT%GERMPLASM=IGID
      NAMDAT%TYPE=1
      NAMDAT%STATUS=0
      I4=GMS_getName(NAMDAT,CWK,80,0)
      IF(I4.EQ.1) THEN
      NTAB(4)=NTAB(4)+1
      CTMP(2:2)='A'
      I=INDEX(NAME,NULL)
      NAME(I:)=', '//CWK
      ENDIF
C I=INTERNATIONAL TEST ENTRY
      NAMDAT%GERMPLASM=IGID
      NAMDAT%TYPE=11
      NAMDAT%STATUS=0
      I4=GMS_getName(NAMDAT,CWK,80,0)
      IF(I4.EQ.1) THEN
      NTAB(5)=NTAB(5)+1
      CTMP(3:3)='I'
      I=INDEX(NAME,NULL)
      NAME(I:)=', '//CWK
      ENDIF
C R=RELEASE
      IF(CTMP(4:4).EQ.' ') THEN
      NAMDAT%GERMPLASM=IGID
      NAMDAT%TYPE=4
      NAMDAT%STATUS=0
      I4=GMS_getName(NAMDAT,CWK,80,0)
      IF(I4.EQ.1) THEN
      CTMP(4:4)='R'
      I=INDEX(NAME,NULL)
      NAME(I:)=', '//CWK
      ENDIF
      ENDIF
      IF(CTMP(4:4).EQ.'R') NTAB(6)=NTAB(6)+1
      CTMP(5:5)='?'
      IF(IGEN.NE.NGEN+1) THEN
C  FIND DERIVATIVES AND OFFSPRING OF IGID
      I4=GMS_findDescendant(IGID,GERM,NPRG,0)
      JNBR=-1
      DO WHILE (I4.EQ.1)
      IF(GERM%REPLACE.NE.0) GOTO 7
      JNBR=INBR+1
      IF(GERM%NO_PROGENITORS.EQ.-1.AND.GERM%PROGENITOR_ID2.EQ.0) JNBR=10
      IF(GERM%NO_PROGENITORS.GT.0) JNBR=1
      IF(KGID.EQ.IGID.AND.GERM%NO_PROGENITORS.EQ.-1.AND.NPRG.EQ.1
     &.AND.GERM%PROGENITOR_ID2.NE.0) GOTO 7
C S=SOURCE, P=PARENT, B=BOTH, T=TERMINAL
      IF(GERM%NO_PROGENITORS.EQ.-1) THEN
      IF(CTMP(5:5).EQ.'?') THEN
      CTMP(5:5)='S'
      ELSEIF(CTMP(5:5).EQ.'P') THEN
      CTMP(5:5)='B'
      ENDIF
      ELSE
      IF(CTMP(5:5).EQ.'?') THEN
      CTMP(5:5)='P'
      ELSEIF(CTMP(5:5).EQ.'S') THEN
      CTMP(5:5)='B'
      ENDIF
      MDATA%LISTID=LIST
      MDATA%LRECID=0
      MDATA%ENTRYID=0
      MDATA%GID=GERM%GERMPLASM_ID
      IF(GMS_getListData(MDATA,0).NE.0) GOTO 7
      ENDIF
      NLST=NLST+1
      MDATA%GID=GERM%GERMPLASM_ID
      WRITE(CTMP1,100) JNBR
 100  FORMAT(I1,3X,'?')
      MDATA%ENTRYCD=FTOCSTR(CTMP1)
      MDATA%SOURCE=NULL
      MDATA%DESIG=NULL
      MDATA%GRPNAME=NULL
      MDATA%LISTID=LIST
      MDATA%LRECID=0
      MDATA%ENTRYID=0
      I4=GMS_addListData(MDATA)
      IF(I4.NE.1) STOP 'TRACE3'
 7    I4=GMS_findDescendant(IGID,GERM,NPRG,1)
      END DO
      IF(CTMP(5:5).EQ.'?') CTMP(5:5)='T'
      IF(CTMP(5:5).EQ.'T') NTAB(7)=NTAB(7)+1
      ENDIF
      LDATA%DESIG=NAME
      LDATA%ENTRYCD=FTOCSTR(CTMP)
      LDATA%DESIG=NAME
      I4=GMS_setListData(LDATA)
      IF(CTMP(2:4).NE.'   '.AND.MLST.LE.MXLST) THEN
      MLST=MLST+1
      LLST(MLST)=LDATA%ENTRYID
      ENDIF
C      CALL GMS_COMMITDATA()
 5    CONTINUE
      IF(IGEN.GT.0.AND.IGEN.NE.NGEN+1) THEN
      WRITE(0,'(I3,7I10)') IGEN,(NTAB(J),J=1,7)
      WRITE(3,'(I3,7I10)') IGEN,(NTAB(J),J=1,7)
      ENDIF
      WRITE(CTMP1,'(I3)') IGEN
      IF(JNBR.LT.0) CTMP1='ALL'
      LSTN%LISTDESC(10:12)=CTMP1(1:3)
      I4=GMS_setListName(LSTN)
      IF(I4.NE.1) STOP 'TRACE5'
      CALL GMS_COMMITDATA()
      JLST=ILST
      KLST=NLST
      IF (KLST.GE.JLST) GOTO 3
C 6    CALL GMS_COMMITDATA()
      CWK='COMPLETE'
      IF(MLST.GE.MXLST) CWK='PARTIAL'
      WRITE(0,106) TRIM(CWK)
      WRITE(3,106) TRIM(CWK)
 106  FORMAT(A,' LIST OF DESCENDANTS WHICH ARE ACCESSIONS,',
     &' TEST ENTRIES OR RELEASES'/
     &' NO. NAMES',47X,'GID    TYPE    SOURCE')
      DO 8 I=1,MLST
      LDATA%ENTRYID=LLST(I)
      LDATA%LRECID=0
      LDATA%GID=0
      I4=GMS_getListData(LDATA,0)
      IF(I4.NE.1) STOP 'TRACE6'
      LLST(I)=LDATA%GID
      I4=INDEX(LDATA%DESIG,NULL)-1
      WRITE(0,104) I,LFT(LDATA%DESIG(1:I4)),LDATA%GID,
     &LDATA%ENTRYCD,LDATA%SOURCE
      WRITE(3,104) I,LFT(LDATA%DESIG(1:I4)),LDATA%GID,
     &LDATA%ENTRYCD,LDATA%SOURCE     
 104  FORMAT(I4,1X,A50,I8,1X,A10,1X,A3)
 8    CONTINUE
      LIST=0
      RETURN
      END
C******************************************************* ICOP
C  PURPOSE:
C  CALCULATE COEFFICIENTS OF PARENTAGE FOR TWO GIDs
C  ARGUMENTS:
C  LINE - CONTAINS THE .COP GIDx, GIDy COMMAND
C
C  EXTRACT COMBINED TREE, FIND DEVELOPMENT LEVELS FOR
C  EACH PROGENITOR AND USE RECURSIVE FUNCTION COP
C---- 08/05/99 ----------------------------------------- ICOP
      SUBROUTINE ICOP(LINE,RCOP,IDY)
      USE GMS_STRUCTURES
      USE GMS_TOOLS
      USE GMS_INTERFACE
      use modSparseMatrix
      PARAMETER (MAXTREE=30000)
      type(cMatrix) :: COPVALS
      CHARACTER LINE*80
      INTEGER*4 GIDX,GIDY,IWK(20)
      CHARACTER XNAME*80,YNAME*80,VIEWER*60
      CHARACTER GVIZPATH*60      
      TYPE(GMS_TREE) GTREE(MAXTREE)
      TYPE(GMS_NAMEDATA) NAME
      COMMON /WRK/ NCR,MXTREE,GTREE
      COMMON /OPTIONS/ NUID,NLOC,NREF,IOPT,LIST,ISRH,MGAT,NAMP,NTCX
     &,IBTYPE,LDST,IYAL,NMSTD,VIEWER,GVIZPATH
C  PARSE GIDx AND GIDy FROM THE COMMAND LINE
      GIDY=0
      READ(LINE,'(20A4)') IWK
      IF(IWK(1).NE.'.COP'.AND.IWK(1).NE.'.cop') RETURN
      L=5
      GIDX=INUMBA(IWK,L,80,J,IER)
      IF(IER.GE.0) RETURN
      GIDY=INUMBA(IWK,L,80,J,IER)
      IF(IER.GE.0) RETURN
C EXTRACT THE COMBINED TREE FOR GIDx AND GIDy
      IDX=1
      IF(NCR.EQ.0) THEN
      CALL GETTREE(GIDX,100,IER)
      IF(IER.NE.1) RETURN
      ENDIF
      IDY=NCR+1
      CALL GETTREE(GIDY,100,IER)
      IF(IER.NE.1) RETURN
C FIND THE LEVEL OF EACH PROGENITOR IN THE COMBINED TREE
      DO 1,I=1,NCR
      GTREE(I)%ORDER=0
      IF(GTREE(I)%GADDR.LE.0) GTREE(I)%ORDER=1
 1    CONTINUE
 2    L=-1
      DO 4 I=1,NCR
      IF(GTREE(I)%ORDER.GT.0) GOTO 4
      L=0
      DO 3 J=GTREE(I)%GADDR,GTREE(I)%GADDR+IABS(GTREE(I)%GNPGS)-1
      K=GTREE(J)%ORDER
      IF(K.EQ.0) GOTO 4
      L=MAX(L,K)
 3    CONTINUE
      GTREE(I)%ORDER=L+1
 4    CONTINUE
      IF(L.GE.0) GOTO 2
C      DO 10,I=1,NCR
C 10   WRITE(0,'(6I10)') I,GTREE(I)
      call initializeStack(10000)
      call setMatrix(COPVALS,NCR,NCR)
      IF(IBTYPE.GT.0) THEN
      RCOP=COPSF(IDX,IDY,COPVALS)
      ELSE
      RCOP=COPCF(IDX,IDY,COPVALS)
      ENDIF
      NAME%GERMPLASM=GIDX
      NAME%TYPE=0
      NAME%STATUS=1
      XNAME='<Unknown>'C
      I4=GMS_getName(NAME,XNAME,80,0)
      XNAME=CTOFSTR(XNAME)
      NAME%GERMPLASM=GIDY
      NAME%TYPE=0
      NAME%STATUS=1
      YNAME='<Unknown>'C
      I4=GMS_getName(NAME,YNAME,80,0)
      YNAME=CTOFSTR(YNAME)
      WRITE(0,100) GIDX,GIDY,RCOP,TRIM(XNAME),TRIM(YNAME),IBTYPE
      WRITE(3,100) GIDX,GIDY,RCOP,TRIM(XNAME),TRIM(YNAME),IBTYPE
 100  FORMAT(' COP FOR GIDS',I9,' AND',I9,' IS',F9.4,
     &' (',A,':',A,'), BTYPE=',I2)
      NCR=IDY-1
      CALL destroySparseMatrix(COPVALS,*99)
	call deallocateStack()
      RETURN
 99   WRITE(0,'(" ERROR DELETING COPVALS MATRIX")')
      RETURN
      END
C******************************************************* COPSF
C  PURPOSE:
C  RECURSIVE CALCULATION OF COEFFICIENTS OF PARENTAGE
C  FOR SELF FERTILIZING CROPS
C
C  ARGUMENTS:
C  IDX is the row in the pedigree tree (GTREE) containing
C  information on the first strain to be compared.
C  IDY is the row in GTREE for the second strain.
C
C  COP(GIDx,GIDy)=SUM(COP(GIDx,GIDi),i=1,GNPGSy)/GNPGSy
C  WHERE GIDy CANNOT BE A PROGENITOR OF GIDx (IE GIDy IS AT
C  THE SAME OR HIGHER LEVEL THAN GIDx, ELSE REVERSE x AND y)
C  AND WHERE GIDi ARE THE PARENTS FOR THE CROSS OF GIDy
C  PARENTS OF UNKNOWN ORIGIN ARE ASSUMED UNRELATED
C  FOR SELF POLINATING CROPS THE INBREEDING COEFFICIENT
C  OF LINES IS COMPUTED FROM THE NUMBER OF SELFING GENERATIONS
C  IF THIS IS KNOWN, OTHERWISE THEY ARE ASSUMED HOMOZYGOUS.
C  FOR CROSS POLINATING CROPS LINES ARE ASSUMED TO BE F1S
C---- 20/02/02 ----------------------------------------- COPSF
      RECURSIVE FUNCTION COPSF(IDX,IDY,COPVALS)
      USE GMS_STRUCTURES
      use modSparseMatrix
      PARAMETER (MAXTREE=30000)
      type(cMatrix) :: COPVALS
      type(cElement), pointer :: ptr
      INTEGER*4 GIDX,GIDY,IWK(20)
      CHARACTER VIEWER*60,GVIZPATH*60
      TYPE (GMS_TREE) GTREE(MAXTREE)
      COMMON /WRK/ NCR,MXTREE,GTREE
      COMMON /OPTIONS/ NUID,NLOC,NREF,IOPT,LIST,ISRH,MGAT,NAMP,NTCX
     &,IBTYPE,LDST,IYAL,NMSTD,VIEWER,GVIZPATH
C CHECK IF THIS COP HAS BEEN CALCULATED BEFORE
      ICV=IDX
	JCV=IDY
	IF(IDX.LT.IDY) THEN
	ICV=IDY
	JCV=IDX
	ENDIF
      ptr=>findElement(COPVALS,ICV,JCV,IER)
	if(IER.EQ.0) THEN
	COPSF=getDataVal(ptr)
	RETURN
	ENDIF
C IF EITHER GERMPLASM IS UNKNOWN COP=0
      IF(GTREE(IDX)%GID.EQ.0.OR.GTREE(IDY)%GID.EQ.0) THEN
      COPSF=0.0
      call addNewElement(COPVALS, ICV, JCV, DBLE(COPSF),*99)
      RETURN
      ENDIF
      IGX=IDX
C Get the group of IDX      
      IF(GTREE(IDX)%DADDR.GE.0) IGX=GTREE(IDX)%GADDR
C  IF IDX HAS A DERIVATIVE GROUP SOURCE USE IDX NOT IGX
      IF(IGX.NE.IDX.AND.GTREE(IGX)%GNPGS.EQ.-1) IGX=IDX
      IGY=IDY
      IF(GTREE(IDY)%DADDR.GE.0) IGY=GTREE(IDY)%GADDR
      IF(IGY.NE.IDY.AND.GTREE(IGY)%GNPGS.EQ.-1) IGY=IDY
C---------------------------------------------------------
C  COP OF A LINE WITH ITSELF
      IF(GTREE(IDX)%GID.EQ.GTREE(IDY)%GID) THEN
C  CALCULATE COEF OF INBREEDING FZ IN COPZ
      IF(GTREE(IDX)%METHN.EQ.202) THEN
C  If the line is a DH line then FZ is 1      
      COPZ=1.0
      ELSE IF(GTREE(IGX)%GADDR.LE.0) THEN
C  GADDR <=0 if the group is unknown or not expanded      
C  IF THERE ARE NO PARENTS FOR IDX USE
C  BTYPE=1 FOR SF CROPS ASSUMING FULL INBREEDING FOR LINES WITH UNKNOWN SOURCE
C  BTYPE=2 FOR SF CROPS ASSUMING F4 FOR LINES WITH UNKNOWN SOURCE BUT KNOWN PARENTS
C  BTYPE=0 FOR CF CROPS ASSUMING F1 STATUS
      COPZ=0.0
      IF(IBTYPE.GT.0) COPZ=1.0
      ELSE IF(GTREE(IGX)%GNPGS.GT.1) THEN
C  For derivatives from known group ...       
C  IF THERE ARE 2 or more PARENTS ...
C  COUNT INBREEDING GENERATIONS
      NSTP=0
      IF(GTREE(IDX)%GNPGS.EQ.-1.AND.GTREE(IDX)%DADDR.LE.0) NSTP=-1
      IF(GTREE(IDX)%DADDR.GT.0) THEN
      I=GTREE(IDX)%DADDR
      DO WHILE (GTREE(I)%DADDR.GT.0)
      I=I+1
      ENDDO
      NSTP=I-GTREE(IDX)%DADDR+1
      IF(GTREE(I)%DADDR.GE.0) NSTP=-1
      ENDIF

      IF(NSTP.LT.0) THEN
C  Assume no inbreeding for CF crops and       
C  ...  F4 derived for breeding line with unknown source in SP crops
C  BTYPE=1 FOR SF CROPS ASSUMING FULL INBREEDING FOR LINES WITH UNKNOWN SOURCE
C  BTYPE=2 FOR SF CROPS ASSUMING F4 FOR LINES WITH UNKNOWN SOURCE BUT KNOWN PARENTS
C  BTYPE=0 FOR CF CROPS
      COPZ=0.0
      IF(IBTYPE.EQ.1) COPZ=1.0
      IF(IBTYPE.EQ.2) COPZ=15.0/16.0
      ELSE
C  ... BUT WITH N GENERATIONS OF INBREEDING
C  ... FZ=1-(1-COP(PARENTS OF IDX))(1/2)**NSTP
      IDW=GTREE(IGX)%GADDR
      COPZ=0.0
      NPG=IABS(GTREE(IGX)%GNPGS)-1
C  FOR POLYCROSSES WITH MORE THAN 2 PARENTS
C  USE THE MEAN COP WITH FEMALE
      DO 1 I=IDW+1,IDW+NPG
      COPZ=COPZ+COPSF(IDW,I,COPVALS)
 1    CONTINUE
      COPZ=COPZ/FLOAT(NPG)
      COPZ=1.0-(1.0-COPZ)/(2.0**NSTP)
      ENDIF
      ELSE
C  FOR DERIVATIVES from a DERIVATIVE GROUP SOURCE OR for MUTANTS (one parent) ...
C  BTYPE=1 FOR SF CROPS ASSUMING FULL INBREEDING FOR LINES WITH UNKNOWN SOURCE
C  BTYPE=2 FOR SF CROPS ASSUMING F4 FOR LINES WITH UNKNOWN SOURCE BUT KNOWN PARENTS
C  BTYPE=0 FOR CF CROPS
      COPZ=0.0
      IF(IBTYPE.GT.0) COPZ=1.0
C      COPZ=IBTYPE
      ENDIF
      COPSF=(1.0+COPZ)/2.0
      call addNewElement(COPVALS, ICV, JCV, DBLE(COPSF),*99)
C      WRITE(3,100) GTREE(IDX)%GID,GTREE(IDY)%GID,COP,1
      RETURN
      ENDIF
C----------------------------------------------------------
C  COP BETWEEN SISTER LINES
      IF(GTREE(IDX)%GNPGS.EQ.-1.OR.GTREE(IDY)%GNPGS.EQ.-1) THEN
      IF(GTREE(IGX)%GID.EQ.GTREE(IGY)%GID) THEN
C  IF THE COMMON GROUP IS A DERIVATIVE GROUP SOURCE OR A SINGLE PROGENITOR
C  ASSUME HOMOZYGOUS FOR SF CROPS AND HETEROZYGOUS FOR CP CROPS
      NPG=IABS(GTREE(IGX)%GNPGS)
      IF(NPG.EQ.1) THEN
C  BTYPE=1 FOR SF CROPS ASSUMING FULL INBREEDING FOR LINES WITH UNKNOWN SOURCE
C  BTYPE=2 FOR SF CROPS ASSUMING F4 FOR LINES WITH UNKNOWN SOURCE BUT KNOWN PARENTS
C  BTYPE=0 FOR CF CROPS
      COPZ=0.0
      IF(IBTYPE.GT.0) COPZ=1.0      
C      COPZ=IBTYPE
C  ELSE FIND OUT IF THE LINES HAVE A COMMON DERIVATIVE ANCESTOR
      ELSE
      IDW=IDX
      DO WHILE (IDW.GT.0)
      IDZ=IDY
      DO WHILE (IDZ.GT.0)
      IF(GTREE(IDW)%GID.EQ.GTREE(IDZ)%GID) GOTO 3
      IDZ=GTREE(IDZ)%DADDR
      ENDDO
      IDW=GTREE(IDW)%DADDR
      ENDDO
C  LINES NOT CONNECTED ASSUME DIVERGENCE AT F1
      GOTO 4
C  LINES CONNECTED AT IDW, COUNT DERIVATIVE STEPS TO GROUP
 3    NSTP=0
      IF(GTREE(IDW)%GNPGS.EQ.-1.AND.GTREE(IDW)%DADDR.LE.0) NSTP=-1
      IF(GTREE(IDW)%DADDR.GT.0) THEN
      I=GTREE(IDW)%DADDR
      DO WHILE (GTREE(I)%DADDR.GT.0)
      I=I+1
      ENDDO
      NSTP=I-GTREE(IDW)%DADDR+1
      IF(GTREE(I)%DADDR.EQ.0) NSTP=-1
      ENDIF
      IF(NSTP.LT.0) THEN
C IF NO PATH FROM THE COMMON LINE TO THE GROUP OR IF THERE IS ONLY ONE  
C PROGENITOR ASSUME F4 FOR SF AND HETEROZYGOUS FOR CP CROPS
C  BTYPE=1 FOR SF CROPS ASSUMING FULL INBREEDING FOR LINES WITH UNKNOWN SOURCE
C  BTYPE=2 FOR SF CROPS ASSUMING F4 FOR LINES WITH UNKNOWN SOURCE BUT KNOWN PARENTS
C  BTYPE=0 FOR CF CROPS
      COPZ=0.0
      IF(IBTYPE.EQ.1) COPZ=1.0
      IF(IBTYPE.EQ.2) COPZ=15.0/16.0
      ELSE
C IF NSTP STEPS TO GROUP CALCULATE COEF OF INBREEDING AS
C  ... FZ=1-(1-COP(PARENTS))(1/2)**NSTP
      IDW=GTREE(IGX)%GADDR
      COPZ=0.0
C IF THE PEDIGREE IS NOT EXPANDED AT IGX, IDW WILL BE ZERO      
      IF(IDW.GT.0) THEN      
C  FOR POLYCROSSES WITH MORE THAN 2 PARENTS
C  USE THE MEAN COP WITH FEMALE
      DO 5 I=IDW+1,IDW+NPG-1
      COPZ=COPZ+COPSF(IDW,I,COPVALS)
 5    CONTINUE
      COPZ=COPZ/FLOAT(NPG-1)
      COPZ=1.0-(1.0-COPZ)/(2.0**NSTP)
      ENDIF
      ENDIF
      ENDIF
C  NOW THE COP IS (1+COEF INBREEDING)/2
      COPSF=(1.0+COPZ)/2.0
      call addNewElement(COPVALS, ICV, JCV, DBLE(COPSF),*99)
C      WRITE(3,100) GTREE(IDX)%GID,GTREE(IDY)%GID,COP,2
      RETURN
      ENDIF
      ENDIF
C  -------------------------------------------------------------
C  COP BETWEEN LINES FROM DIFFERENT GROUPS
C  FIND ARGUMENT WITH LOWEST LEVEL
 4    COPZ=0.0
      IDW=IDY
      IDZ=GTREE(IGX)%GADDR
      NPG=IABS(GTREE(IGX)%GNPGS)
      IF(GTREE(IDX)%ORDER.LT.GTREE(IDY)%ORDER) THEN
      IDW=IDX
      IDZ=GTREE(IGY)%GADDR
      NPG=IABS(GTREE(IGY)%GNPGS)
      ENDIF
C IDW IS THE STRAIN WITH LOWEST ORDER. 
C Expand the strain at the higher level      
C  CALCULATE COP FROM COPS WITH PROGENITORS
      IF(IDZ.GT.0) THEN
      DO 2,I=IDZ,IDZ+NPG-1
      COPZ=COPZ+COPSF(I,IDW,COPVALS)
 2    CONTINUE
      COPZ=COPZ/FLOAT(NPG)
      ENDIF
      COPSF=COPZ
      call addNewElement(COPVALS, ICV, JCV, DBLE(COPSF),*99)
C      WRITE(3,100) GTREE(IDX)%GID,GTREE(IDY)%GID,COPSF,3
C 100  FORMAT(' COP FOR GIDS',I9,' AND',I9,' IS',F9.4,I2)
      RETURN
 99   WRITE(0,'(" ERROR ADDING COPVAL(",I9,",",I9,")")') ICV,JCV
      PAUSE
      RETURN
      END
C******************************************************* COPCF
C  PURPOSE:
C  RECURSIVE CALCULATION OF COEFFICIENTS OF PARENTAGE FOR 
C  CROSS FERTALIZING CROPS
C
C  ARGUMENTS:
C  IDX is the row in the pedigree tree (GTREE) containing
C  information on the first strain to be compared.
C  IDY is the row in GTREE for the second strain.
C
C  COPCF(GIDx,GIDy)=SUM(COPCF(GIDx,GIDi),i=1,GNPGSy)/GNPGSy
C  WHERE GIDy CANNOT BE A PROGENITOR OF GIDx (IE GIDy IS AT
C  THE SAME OR HIGHER LEVEL THAN GIDx, ELSE REVERSE x AND y)
C  AND WHERE GIDi ARE THE PARENTS FOR THE CROSS OF GIDy
C  PARENTS OF UNKNOWN ORIGIN ARE ASSUMED UNRELATED
C  THE INBREEDING COEFFICIENT OF A LINE IS COMPUTED FROM 
C  THE NUMBER OF SELFING GENERATIONS IF THIS IS KNOWN, 
C  OTHERWISE THEY ARE ASSUMED TO BE F1S
C---- 19/07/12 ----------------------------------------- COPCF
      RECURSIVE FUNCTION COPCF(IDX,IDY,COPVALS)
      USE GMS_STRUCTURES
      use modSparseMatrix
      PARAMETER (MAXTREE=30000)
      type(cMatrix) :: COPVALS
      type(cElement), pointer :: ptr
      INTEGER*4 GIDX,GIDY,IWK(20)
      CHARACTER VIEWER*60,GVIZPATH*60
      TYPE (GMS_TREE) GTREE(MAXTREE)
      COMMON /WRK/ NCR,MXTREE,GTREE
      COMMON /OPTIONS/ NUID,NLOC,NREF,IOPT,LIST,ISRH,MGAT,NAMP,NTCX
     &,IBTYPE,LDST,IYAL,NMSTD,VIEWER,GVIZPATH
C CHECK IF THIS COP HAS BEEN CALCULATED BEFORE
      ICV=IDX
	JCV=IDY
	IF(IDX.LT.IDY) THEN
	ICV=IDY
	JCV=IDX
	ENDIF
      ptr=>findElement(COPVALS,ICV,JCV,IER)
	if(IER.EQ.0) THEN
	COPCF=getDataVal(ptr)
	RETURN
	ENDIF
C IF EITHER GERMPLASM IS UNKNOWN COP=0
      IF(GTREE(IDX)%GID.EQ.0.OR.GTREE(IDY)%GID.EQ.0) THEN
      COPCF=0.0
      call addNewElement(COPVALS, ICV, JCV, DBLE(COPCF),*99)
      RETURN
      ENDIF
      IGX=IDX
C Get the group of IDX      
      IF(GTREE(IDX)%DADDR.GE.0) IGX=GTREE(IDX)%GADDR
C  IF IDX HAS A DERIVATIVE GROUP SOURCE USE IDX NOT IGX
      IF(IGX.NE.IDX.AND.GTREE(IGX)%GNPGS.EQ.-1) IGX=IDX
      IGY=IDY
      IF(GTREE(IDY)%DADDR.GE.0) IGY=GTREE(IDY)%GADDR
      IF(IGY.NE.IDY.AND.GTREE(IGY)%GNPGS.EQ.-1) IGY=IDY
C---------------------------------------------------------
C  COP OF A LINE WITH ITSELF
      IF(GTREE(IDX)%GID.EQ.GTREE(IDY)%GID) THEN
C  CALCULATE COEF OF INBREEDING FZ IN COPZ
      IF(GTREE(IDX)%METHN.EQ.502) THEN
C  If the line is a DH line then FZ is 1      
      COPZ=1.0
      ELSE IF(GTREE(IGX)%GADDR.LE.0) THEN
C  GADDR <=0 if the group is unknown or not expanded      
C  IF THERE ARE NO PARENTS FOR IDX USE
        COPZ=0.0
        IF(GTREE(IDX)%METHN.EQ.2) COPZ=0.0
        IF(GTREE(IDX)%METHN.EQ.32) COPZ=1.0
        IF(GTREE(IDX)%METHN.EQ.421) COPZ=-1.0
        IF(GTREE(IDX)%METHN.EQ.422) COPZ=-1.0
        IF(GTREE(IDX)%METHN.EQ.551) COPZ=-1.0
        IF(GTREE(IDX)%METHN.EQ.553) COPZ=-1.0
        IF(GTREE(IDX)%METHN.EQ.554) COPZ=-1.0
        IF(GTREE(IDX)%METHN.EQ.556) COPZ=-1.0
        IF(GTREE(IDX)%METHN.EQ.558) COPZ=-1.0
      ELSE IF(GTREE(IGX)%GNPGS.GT.0) THEN
C  For derivatives from known group ...       
C  COUNT INBREEDING GENERATIONS
      NSTP=0
      IF(GTREE(IDX)%GNPGS.EQ.-1.AND.GTREE(IDX)%DADDR.LE.0) NSTP=-1
      IF(GTREE(IDX)%DADDR.GT.0) THEN
      I=GTREE(IDX)%DADDR
      DO WHILE (GTREE(I)%DADDR.GT.0)
      I=I+1
      ENDDO
      NSTP=I-GTREE(IDX)%DADDR+1
      IF(GTREE(I)%DADDR.GE.0) NSTP=-1
      ENDIF
C
      IF(NSTP.LT.0) THEN
C  Assume no inbreeding for CF crops and       
      COPZ=0.0
        IF(GTREE(IDX)%METHN.EQ.2) COPZ=0.0
        IF(GTREE(IDX)%METHN.EQ.32) COPZ=1.0
        IF(GTREE(IDX)%METHN.EQ.421) COPZ=-1.0
        IF(GTREE(IDX)%METHN.EQ.422) COPZ=-1.0
        IF(GTREE(IDX)%METHN.EQ.551) COPZ=-1.0
        IF(GTREE(IDX)%METHN.EQ.553) COPZ=-1.0
        IF(GTREE(IDX)%METHN.EQ.554) COPZ=-1.0
        IF(GTREE(IDX)%METHN.EQ.556) COPZ=-1.0
        IF(GTREE(IDX)%METHN.EQ.558) COPZ=-1.0
      ELSE
C  ... BUT WITH N GENERATIONS OF INBREEDING
      IF(GTREE(IGX)%GNPGS.GT.1) THEN
C  ... FZ=1-(1-COP(PARENTS OF IDX))(1/2)**NSTP
      IDW=GTREE(IGX)%GADDR
      COPZ=0.0
      NPG=IABS(GTREE(IGX)%GNPGS)-1
C  FOR POLYCROSSES WITH MORE THAN 2 PARENTS
C  USE THE MEAN COP WITH FEMALE
      DO 1 I=IDW+1,IDW+NPG
      COPZ=COPZ+COPCF(IDW,I,COPVALS)
 1    CONTINUE
      COPZ=COPZ/FLOAT(NPG)  
      ELSE
C WITH ONLY ONE PROGENITOR (MUTANTS WILL GET COPZ=0.0)
        COPZ=0.0
        IF(GTREE(IGX)%METHN.EQ.2) COPZ=0.0
        IF(GTREE(IGX)%METHN.EQ.32) COPZ=1.0
        IF(GTREE(IGX)%METHN.EQ.421) COPZ=-1.0
        IF(GTREE(IGX)%METHN.EQ.422) COPZ=-1.0
        IF(GTREE(IGX)%METHN.EQ.551) COPZ=-1.0
        IF(GTREE(IGX)%METHN.EQ.553) COPZ=-1.0
        IF(GTREE(IGX)%METHN.EQ.554) COPZ=-1.0
        IF(GTREE(IGX)%METHN.EQ.556) COPZ=-1.0
        IF(GTREE(IGX)%METHN.EQ.558) COPZ=-1.0
      ENDIF
      COPZ=1.0-(1.0-COPZ)/(2.0**NSTP)     
      ENDIF
      ELSE
C  FOR DERIVATIVES from a DERIVATIVE GROUP SOURCE 
      COPZ=1.0
      ENDIF
      COPCF=(1.0+COPZ)/2.0
      call addNewElement(COPVALS, ICV, JCV, DBLE(COPCF),*99)
C      WRITE(3,100) GTREE(IDX)%GID,GTREE(IDY)%GID,COP,1
      RETURN
      ENDIF
C----------------------------------------------------------
C  COP BETWEEN SISTER LINES
      IF(GTREE(IDX)%GNPGS.EQ.-1.OR.GTREE(IDY)%GNPGS.EQ.-1) THEN
      IF(GTREE(IGX)%GID.EQ.GTREE(IGY)%GID) THEN
      NPG=IABS(GTREE(IGX)%GNPGS)
C  IF THE COMMON GROUP IS A DERIVATIVE GROUP SOURCE
C  ASSUME HOMOZYGOUS
C      IF(GTREE(IGX)%GNPGS.EQ.-1) THEN
C      COPZ=0.0
C      IF(GTREE(IGX)%METHN.EQ.2) COPZ=0.0
C      IF(GTREE(IGX)%METHN.EQ.32) COPZ=1.0
C      IF(GTREE(IGX)%METHN.EQ.421) COPZ=-1.0
C      IF(GTREE(IGX)%METHN.EQ.422) COPZ=-1.0
C      IF(GTREE(IGX)%METHN.EQ.511) COPZ=-1.0
C      IF(GTREE(IGX)%METHN.EQ.554) COPZ=-1.0            
C  ELSE FIND OUT IF THE LINES HAVE A COMMON DERIVATIVE ANCESTOR
C      ELSE
      IDW=IDX
      DO WHILE (IDW.GT.0)
      IDZ=IDY
      DO WHILE (IDZ.GT.0)
      IF(GTREE(IDW)%GID.EQ.GTREE(IDZ)%GID) GOTO 3
      IDZ=GTREE(IDZ)%DADDR
      ENDDO
      IDW=GTREE(IDW)%DADDR
      ENDDO
C  LINES NOT CONNECTED ASSUME DIVERGENCE AT F1
      GOTO 4
C  LINES CONNECTED AT IDW, COUNT DERIVATIVE STEPS TO GROUP
 3    NSTP=0
C SET NSTEP=-1 IF THE COMMON ANCESTOR IS THE DERIVATIVE GROUP SOURCE
      IF(GTREE(IDW)%GNPGS.EQ.-1.AND.GTREE(IDW)%DADDR.LE.0) NSTP=-1
      IF(GTREE(IDW)%DADDR.GT.0) THEN
      I=GTREE(IDW)%DADDR
      DO WHILE (GTREE(I)%DADDR.GT.0)
      I=I+1
      ENDDO
      NSTP=I-GTREE(IDW)%DADDR+1
C SET NSTEP=-1 IF THE DERIVATIVE HISTORY IS BROKEN AT I
C (DADDR=0 IF LINE I HAS KNOWN GROUP BUT UNKNOWN SOURCE)     
      IF(GTREE(I)%DADDR.EQ.0) NSTP=-1
      ENDIF
      IF(NSTP.LT.0) THEN
C IF NO PATH FROM THE COMMON LINE TO THE GROUP OR IF THERE IS ONLY ONE PROGENITOR 
        COPZ=0.0
        IF(GTREE(IGX)%METHN.EQ.2) COPZ=0.0
        IF(GTREE(IGX)%METHN.EQ.32) COPZ=1.0
        IF(GTREE(IGX)%METHN.EQ.421) COPZ=-1.0
        IF(GTREE(IGX)%METHN.EQ.422) COPZ=-1.0
        IF(GTREE(IGX)%METHN.EQ.551) COPZ=-1.0
        IF(GTREE(IGX)%METHN.EQ.553) COPZ=-1.0
        IF(GTREE(IGX)%METHN.EQ.554) COPZ=-1.0
        IF(GTREE(IGX)%METHN.EQ.556) COPZ=-1.0
        IF(GTREE(IGX)%METHN.EQ.558) COPZ=-1.0
      ELSE
C IF NSTP STEPS TO GROUP CALCULATE COEF OF INBREEDING AS
C  ... FZ=1-(1-COP(PARENTS))(1/2)**NSTP
      IDW=GTREE(IGX)%GADDR
      COPZ=0.0
C IF THE PEDIGREE IS NOT EXPANDED AT IGX, IDW WILL BE ZERO      
      IF(IDW.GT.0) THEN      
C  FOR POLYCROSSES WITH MORE THAN 2 PARENTS
C  USE THE MEAN COP WITH FEMALE
      IF(NPG.GT.1) THEN
      DO 5 I=IDW+1,IDW+NPG-1
      COPZ=COPZ+COPCF(IDW,I,COPVALS)
 5    CONTINUE
      COPZ=COPZ/FLOAT(NPG-1)
      ELSE
        COPZ=0.0
        IF(GTREE(IGX)%METHN.EQ.2) COPZ=0.0
        IF(GTREE(IGX)%METHN.EQ.32) COPZ=1.0
        IF(GTREE(IGX)%METHN.EQ.421) COPZ=-1.0
        IF(GTREE(IGX)%METHN.EQ.422) COPZ=-1.0
        IF(GTREE(IGX)%METHN.EQ.551) COPZ=-1.0
        IF(GTREE(IGX)%METHN.EQ.553) COPZ=-1.0
        IF(GTREE(IGX)%METHN.EQ.554) COPZ=-1.0
        IF(GTREE(IGX)%METHN.EQ.556) COPZ=-1.0
        IF(GTREE(IGX)%METHN.EQ.558) COPZ=-1.0
      ENDIF
      COPZ=1.0-(1.0-COPZ)/(2.0**NSTP)      
      ENDIF
      ENDIF
C      ENDIF
C  NOW THE COP IS (1+COEF INBREEDING)/2
      COPCF=(1.0+COPZ)/2.0
      call addNewElement(COPVALS, ICV, JCV, DBLE(COPCF),*99)
C      WRITE(3,100) GTREE(IDX)%GID,GTREE(IDY)%GID,COP,2
      RETURN
      ENDIF
      ENDIF
C  -------------------------------------------------------------
C  COP BETWEEN LINES FROM DIFFERENT GROUPS
C  FIND ARGUMENT WITH LOWEST LEVEL
 4    COPZ=0.0
      IDW=IDY
      IDZ=GTREE(IGX)%GADDR
      NPG=IABS(GTREE(IGX)%GNPGS)
      IF(GTREE(IDX)%ORDER.LT.GTREE(IDY)%ORDER) THEN
      IDW=IDX
      IDZ=GTREE(IGY)%GADDR
      NPG=IABS(GTREE(IGY)%GNPGS)
      ENDIF
C IDW IS THE STRAIN WITH LOWEST ORDER. 
C Expand the strain at the higher level      
C  CALCULATE COP FROM COPS WITH PROGENITORS
      IF(IDZ.GT.0) THEN
      DO 2,I=IDZ,IDZ+NPG-1
      COPZ=COPZ+COPCF(I,IDW,COPVALS)
 2    CONTINUE
      COPZ=COPZ/FLOAT(NPG)
      ENDIF
      COPCF=COPZ
      call addNewElement(COPVALS, ICV, JCV, DBLE(COPCF),*99)
C      WRITE(3,100) GTREE(IDX)%GID,GTREE(IDY)%GID,COPCF,3
C 100  FORMAT(' COP FOR GIDS',I9,' AND',I9,' IS',F9.4,I2)
      RETURN
 99   WRITE(0,'(" ERROR ADDING COPVAL(",I9,",",I9,")")') ICV,JCV
      PAUSE
      RETURN
      END      
C******************************************************* DRAWTREE2
C  PURPOSE:
C  DISPLAY THE PEDIGREE TREE STORED IN COMMON /WRK/
C
C  ARGUMENTS:
C  LINE  - CURRENT LINE IN GTREE TO WRITE
C  LEVEL - LEVEL OF LINE IN THE PEDIGREE
C  LINEP - LINE NUMBER OF THE PREVIOUS LEVEL
C  LSTLVL- LAST LEVEL TO DRAW
C  LST   - .TRUE. IF CURRENT LINE IS LAST PROGENITOR IN A SET
C  IN /OPTIONS/
C  NAMP  - 0 LIST PARENTS ONLY AND PREFERRED NAMES ONLY,
C          1 PARENTS ONLY WITH ALL NAMES,
C          2 FOR PARENTS ONLY WITH PREFERRED NAMES + ACCESSION NOS
C  LDST  - 0 SUPRESS DISPLAY OF DERIVATIVE STEPS
C          1 DISPLAY DERIVATIVE STEPS AND GROUP SOURCES
C
C---- 09/01/02 ----------------------------------------- DRAWTREE2
      RECURSIVE SUBROUTINE DRAWTREE2(LINE,LEVEL,CWK,LSTLVL,LST)
      USE GMS_STRUCTURES
      USE GMS_INTERFACE
      USE GMS_TOOLS
      PARAMETER (MAXTREE=30000)
      TYPE(GMS_GERMPLASM) GERM
      TYPE(GMS_TREE) GTREE(MAXTREE)
      TYPE(GMS_NAMEDATA) NAME
      CHARACTER CWK*(*),MRK*1,ANAME*256,STEP*3,VIEWER*60
      CHARACTER GVIZPATH*60      
      LOGICAL LST
      COMMON /WRK/ NCR,MXTREE,GTREE
      COMMON /OPTIONS/ NUID,NLOC,NREF,IOPT,LIST,ISRH,MGAT,NAMP,NTCX
     &,IBTYPE,LDST,IYAL,NMSTD,VIEWER,GVIZPATH
      L=2*LEVEL-1
      IF(LEVEL.EQ.0) THEN
      CWK=' '
      L=1
      ELSE IF (CWK(L+2:L+2).NE.'>') THEN
      CWK(L:)=' +-'
      DO 1 I=2,L,2
      IF(CWK(I:I).NE.' ') CWK(I:I)='|'
 1    CONTINUE
      ENDIF
      LGID=GTREE(LINE)%GID
      I=LEN_TRIM(CWK)
      IF(NAMP.EQ.1) THEN
      CWK=CWK(1:I)//LSTNMS(LGID,61-I)
      ELSE
      NAME%GERMPLASM=LGID
      NAME%TYPE=0
      NAME%STATUS=1
      ANAME='<Unknown>'C
      I4=GMS_getName(NAME,ANAME,256,0)
      CWK=CWK(1:I)//CTOFSTR(ANAME)
      ENDIF
      IF(NAMP.EQ.2) THEN
      NAME%GERMPLASM=LGID
      NAME%TYPE=1
      NAME%STATUS=0
      I4=GMS_getName(NAME,ANAME,256,0)
      I=LEN_TRIM(CWK)
      IF(I4.EQ.1) CWK=CWK(1:I)//'('//TRIM(CTOFSTR(ANAME))//')'
      ENDIF
C LOCATE THE GROUP LINE FOR THE CURRENT PROGENITOR AND
C COUNT DERIVATIVE STEPS
      LN=LINE
      NSTP=0
      IF(GTREE(LINE)%GNPGS.EQ.-1.AND.GTREE(LINE)%DADDR.LE.0) NSTP=-1
      IF(GTREE(LINE)%DADDR.GE.0) THEN
      LN=GTREE(LINE)%GADDR
      I=GTREE(LINE)%DADDR
      IF(I.GT.0) THEN
      DO WHILE (GTREE(I)%DADDR.GT.0)
      I=I+1
      ENDDO
      NSTP=I-GTREE(LINE)%DADDR+1
      IF(GTREE(I)%DADDR.GE.0) NSTP=-1
      ENDIF
      ENDIF
C
      MRK=' '
      IF(LN.GT.0) THEN
      IF(GTREE(LN)%GADDR.EQ.0) MRK='#'
      IF(GTREE(LN)%GADDR.EQ.-1.AND.(LDST.EQ.0.OR.LN.EQ.LINE)) MRK='<'
      IF(GTREE(LN)%GADDR.GT.0.AND.GTREE(LN)%GADDR.LT.LINE) MRK='*'
      IF(GTREE(LN)%GNPGS.EQ.-1.AND.LN.NE.LINE) MRK=' '
      ENDIF
      STEP='  ?'
      IF(NSTP.GE.0) WRITE(STEP,'(I3)') NSTP
      WRITE(0,101) LINE,TRIM(CWK),STEP,LGID,MRK
      WRITE(3,101) LINE,TRIM(CWK),STEP,LGID,MRK
 101  FORMAT(I4,A,T66,A3,I9,A1)
C  IF LDST=1 THEN LIST ALL INTERMEDIATE DERIVATIVES
      IF(LDST.EQ.1.AND.MRK.NE.'*') THEN
      IF(GTREE(LINE)%DADDR.GT.0) THEN
      ILN=GTREE(LINE)%DADDR
 4    IF(ILN.EQ.-1) GOTO 3
      CWK(L:)=' |>'
      IF(LST) CWK(L:)='  >'
      NSTP=NSTP-1
      STEP='  ?'
      IF(NSTP.GE.0) WRITE(STEP,'(I3)') NSTP
      LGID=GTREE(ILN)%GID
      I=LEN_TRIM(CWK)
      IF(NAMP.EQ.1) THEN
      CWK=CWK(1:I)//LSTNMS(LGID,61-I)
      ELSE
      NAME%GERMPLASM=LGID
      NAME%TYPE=0
      NAME%STATUS=1
      ANAME='<Unknown>'C
      I4=GMS_getName(NAME,ANAME,256,0)
      CWK=CWK(1:I)//CTOFSTR(ANAME)
      ENDIF
      IF(NAMP.EQ.2) THEN
      NAME%GERMPLASM=LGID
      NAME%TYPE=1
      NAME%STATUS=0
      I4=GMS_getName(NAME,ANAME,256,0)
      I=LEN_TRIM(CWK)
      IF(I4.EQ.1) CWK=CWK(1:I)//'('//TRIM(CTOFSTR(ANAME))//')'
      ENDIF
      MRK=' '
      IF(GTREE(ILN)%GADDR.EQ.0) MRK='#'
      IF(GTREE(ILN)%GADDR.EQ.-1) MRK='<'
      IF(GTREE(ILN)%GADDR.GT.0.AND.GTREE(ILN)%GADDR.LT.LINE) MRK='*'
      WRITE(0,101) ILN,TRIM(CWK),STEP,LGID,MRK
      WRITE(3,101) ILN,TRIM(CWK),STEP,LGID,MRK
      I=ILN
      ILN=GTREE(ILN)%DADDR
      IF(ILN.EQ.0) ILN=I+1
      GOTO 4
 3    CWK(L+2:)=' '
      ENDIF
      IF(LST) CWK(L:)=' '
      ELSE
      IF(LST.OR.CWK(L+2:L+2).EQ.'>') CWK(L:)=' '
C  IF LDST.EQ.0 AND PROGENITOR HAS A DERIVATIVE GROUP SOURCE SHOW IT
      IF(LN.GT.0) THEN
      IF(GTREE(LN)%GNPGS.EQ.-1.AND.LN.NE.LINE) THEN
      IF(L.LE.0) THEN
      CWK=''
      ELSE
      CWK(L:)=' |>'
      IF(LST) CWK(L:)='  >'
      ENDIF
      NLINE=LN
      NLEVEL=LEVEL
      CALL DRAWTREE2(NLINE,NLEVEL,CWK,LSTLVL,.TRUE.)
      ENDIF
      ENDIF
      ENDIF
C
      NLEVEL=LEVEL+1
      IF(LN.EQ.0.OR.NLEVEL.GT.LSTLVL.OR.MRK.EQ.'*') RETURN
      IF(GTREE(LN)%GADDR.LE.0) RETURN
      K=IABS(GTREE(LN)%GNPGS)
      DO 2,I=1,K
      NLINE=GTREE(LN)%GADDR+I-1
      CALL DRAWTREE2(NLINE,NLEVEL,CWK,LSTLVL,(I.EQ.K))
 2    CONTINUE
      RETURN
      END SUBROUTINE DRAWTREE2
C******************************************************* TABTREE1
C  PURPOSE:
C  TABULATE THE PEDIGREE TREE STORED IN COMMON /WRK/
C
C  ARGUMENTS:
C  LINE  - CURRENT LINE IN GTREE TO WRITE
C  LEVEL - LEVEL OF LINE IN THE PEDIGREE
C  LINEP - LINE NUMBER OF THE PREVIOUS LEVEL
C  LSTLVL- LAST LEVEL TO DRAW
C  LST   - .TRUE. IF CURRENT LINE IS LAST PROGENITOR IN A SET
C  IN /OPTIONS/
C  NAMP  - ONE TO LIST ALL NAMES, ZERO FOR PREFERRED NAMES
C
C---- 27/04/99 ----------------------------------------- TABTREE1
      SUBROUTINE TABTREE1()
      USE GMS_STRUCTURES
      USE GMS_INTERFACE
      USE GMS_TOOLS
      PARAMETER (MAXTREE=30000)
      TYPE(GMS_GERMPLASM) GERM
      TYPE(GMS_NAMEDATA) NAME
      CHARACTER CWK*80,ANAME*80,BNAME*80,CNAME*80,VIEWER*60
      CHARACTER GVIZPATH*60      
      INTEGER*2 ITREE(MAXTREE,6)
      INTEGER*4 GTREE(MAXTREE,3)
      COMMON /WRK/ NCR,MXTREE,ITREE
      COMMON /OPTIONS/ NUID,NLOC,NREF,IOPT,LIST,ISRH,MGAT,NAMP,NTCX
     &,IBTYPE,LDST,IYAL,NMSTD,VIEWER,GVIZPATH
      EQUIVALENCE (ITREE(1,1),GTREE(1,1))
      WRITE(0,194)
      WRITE(3,196)
 194  FORMAT(' CHILD',12X,'CHILD GID PARENT',10X,
     &'PARENT GID  GEN METH ACCN NO.')
 196  FORMAT(' CHILD',23X,'CHILD GID PARENT',20X,
     &'PARENT GID  GEN METH ACCN NO.')
      IP=GTREE(1,1)
      NAME%GERMPLASM=IP
      NAME%TYPE=0
      NAME%STATUS=1
      I4=GMS_getName(NAME,ANAME,80,0)
      CWK=CTOFSTR(ANAME)
      NG=ITREE(1,3)+1
      MG=ITREE(1,4)
      NAME%GERMPLASM=IP
      NAME%TYPE=1
      NAME%STATUS=0
      ANAME='<None>'C
      I4=GMS_getName(NAME,ANAME,80,0)
      ANAME=CTOFSTR(ANAME)
      WRITE(0,193) ' ',0,CWK,IP,NG,MG,ANAME
      WRITE(3,195) ' ',0,CWK,IP,NG,MG,ANAME
      CNAME=CWK
C
      DO 191 I=1,NCR
      NAME%GERMPLASM=GTREE(I,1)
      NAME%TYPE=0
      NAME%STATUS=1
      I4=GMS_getName(NAME,ANAME,80,0)
      BNAME=CTOFSTR(ANAME)
      DO 192 J=1,IABS(ITREE(I,5))
      IP=0
      ANAME='<Unknown>'C
      CWK='<Unknown>'
      NG=0
      MG=0
      IF(ITREE(I,6).GT.0) THEN
      IP=GTREE(ITREE(I,6)+J-1,1)
      NAME%GERMPLASM=IP
      NAME%TYPE=0
      NAME%STATUS=1
      I4=GMS_getName(NAME,ANAME,80,0)
      CWK=CTOFSTR(ANAME)
      NG=ITREE(ITREE(I,6)+J-1,3)+1
      MG=ITREE(ITREE(I,6)+J-1,4)
      NAME%GERMPLASM=IP
      NAME%TYPE=1
      NAME%STATUS=0
      ANAME='<None>'C
      I4=GMS_getName(NAME,ANAME,80,0)
      ANAME=CTOFSTR(ANAME)
      WRITE(0,193) BNAME,GTREE(I,1),CWK,IP,NG,MG,ANAME
      WRITE(3,195) BNAME,GTREE(I,1),CWK,IP,NG,MG,ANAME
      ENDIF
 192  CONTINUE
 191  CONTINUE
 193  FORMAT(A20,I9,1X,A20,I9,2I4,1X,A14)
 195  FORMAT(A30,I9,1X,A30,I9,2I4,1X,A20)
      RETURN
      END SUBROUTINE TABTREE1            
C******************************************************* PlotTREE
C  PURPOSE:
C  PRODUCE A .DOT COMMAND FILE AND CALL THE DOT PROGRAM OF 
C  GRAPHVIZ FOLLOWED BY THE IrfanView GRAPH VIEWER FOT 
C  THE PEDIGREE TREE STORED IN COMMON /WRK/
C
C  ARGUMENTS:
C  LINE  - CURRENT LINE IN GTREE TO WRITE
C  LEVEL - LEVEL OF LINE IN THE PEDIGREE
C  LINEP - LINE NUMBER OF THE PREVIOUS LEVEL
C  LSTLVL- LAST LEVEL TO DRAW
C  LST   - .TRUE. IF CURRENT LINE IS LAST PROGENITOR IN A SET
C  IN /OPTIONS/
C  NAMP  - ONE TO LIST ALL NAMES, ZERO FOR PREFERRED NAMES
C
C---- 27/04/99 ----------------------------------------- PlotTREE
      SUBROUTINE PlotTREE(LASTDIR)
      USE IFPORT
      USE GMS_STRUCTURES
      USE GMS_INTERFACE
      USE GMS_TOOLS
      PARAMETER (MAXTREE=30000)      
      TYPE(GMS_GERMPLASM) GERM
      TYPE(GMS_NAMEDATA) NAME
      CHARACTER*80 CWK,ANAME,BNAME,CNAME,LASTDIR,DNAME,FNAME
      CHARACTER VIEWER*60,GVIZPATH*60
      INTEGER*2 ITREE(MAXTREE,6),R
      INTEGER*4 GTREE(MAXTREE,3)
      COMMON /WRK/ NCR,MXTREE,ITREE
      COMMON /OPTIONS/ NUID,NLOC,NREF,IOPT,LIST,ISRH,MGAT,NAMP,NTCX
     &,IBTYPE,LDST,IYAL,NMSTD,VIEWER,GVIZPATH
      EQUIVALENCE (ITREE(1,1),GTREE(1,1))
C      WRITE(0,194)
C      WRITE(3,196)
C 194  FORMAT(' CHILD',12X,'CHILD GID PARENT',10X,
C     &'PARENT GID  GEN METH ACCN NO.')
C 196  FORMAT(' CHILD',23X,'CHILD GID PARENT',20X,
C     &'PARENT GID  GEN METH ACCN NO.')
      FNAME='BROWSE'
      IF(LASTDIR.NE.' ') FNAME=TRIM(LASTDIR)//'BROWSE'
      OPEN(4,FILE=TRIM(FNAME)//'.dot',IOSTAT=IER)     
      IP=GTREE(1,1)
      NAME%GERMPLASM=IP
      NAME%TYPE=0
      NAME%STATUS=1
      I4=GMS_getName(NAME,ANAME,80,0)
      CWK=CTOFSTR(ANAME)
      NG=ITREE(1,3)+1
      MG=ITREE(1,4)
      NAME%GERMPLASM=IP
      NAME%TYPE=1
      NAME%STATUS=0
      ANAME='<None>'C
      I4=GMS_getName(NAME,ANAME,80,0)
      ANAME=CTOFSTR(ANAME)
C      WRITE(0,193) ' ',0,CWK,IP,NG,MG,ANAME
C      WRITE(3,195) ' ',0,CWK,IP,NG,MG,ANAME
      CNAME=CWK
      WRITE(4,198)
 198  FORMAT('digraph tree {')         
C
      DO 191 I=1,NCR
      NAME%GERMPLASM=GTREE(I,1)
      NAME%TYPE=0
      NAME%STATUS=1
      I4=GMS_getName(NAME,ANAME,80,0)
      BNAME=CTOFSTR(ANAME)
      NAME%TYPE=1
      NAME%STATUS=0
      ANAME='<None>'C
      I4=GMS_getName(NAME,ANAME,80,0)
      DNAME=CTOFSTR(ANAME)
      
      DO 192 J=1,IABS(ITREE(I,5))
      IP=0
      ANAME='<Unknown>'C
      CWK='<Unknown>'
      NG=0
      MG=0
      IF(ITREE(I,6).GT.0) THEN
      IP=GTREE(ITREE(I,6)+J-1,1)
      NAME%GERMPLASM=IP
      NAME%TYPE=0
      NAME%STATUS=1
      I4=GMS_getName(NAME,ANAME,80,0)
      CWK=CTOFSTR(ANAME)
      NG=ITREE(ITREE(I,6)+J-1,3)+1
      MG=ITREE(ITREE(I,6)+J-1,4)
      NAME%GERMPLASM=IP
      NAME%TYPE=1
      NAME%STATUS=0
      ANAME='<None>'C
      I4=GMS_getName(NAME,ANAME,80,0)
      ANAME=CTOFSTR(ANAME)
C      WRITE(0,193) BNAME,GTREE(I,1),CWK,IP,NG,MG,ANAME
C      WRITE(3,195) BNAME,GTREE(I,1),CWK,IP,NG,MG,ANAME
C Check through edges already printed to unit 4 and skip printing for repeats      
      DO Ix=1,I-1
      DO Jx=1,IABS(ITREE(Ix,5))
      JP=0
      IF(ITREE(Ix,6).GT.0) JP=GTREE(ITREE(Ix,6)+Jx-1,1)
      IF(GTREE(Ix,1).eq.GTREE(I,1).AND.IP.eq.JP) GOTO 192
      ENDDO
      ENDDO
c      WRITE(4,197) trim(CWK),TRIM(BNAME)
c 197  FORMAT('"',A,'" -> "',A,'";')
      WRITE(4,197) trim(CWK),TRIM(ANAME),TRIM(BNAME),TRIM(DNAME)
 197  FORMAT('"',A,'\n',A'" -> "',A,'\n',A'";')          
      ENDIF
 192  CONTINUE
 191  CONTINUE
      WRITE(4,199) GTREE(1,1), TRIM(CNAME)
 199  FORMAT('overlap=false'/'label=" Gid:',I8,1x,A,'"'/
     &'fontsize=12;'/'}')
C 193  FORMAT(A20,I9,1X,A20,I9,2I4,1X,A14)
C 195  FORMAT(A30,I9,1X,A30,I9,2I4,1X,A20)
      close (4)
      R=RUNQQ(TRIM(GVIZPATH)//'DOT',
     &'-Tgif '//TRIM(FNAME)//'.dot -Gcharset=latin1 -o'//TRIM(FNAME)//'.gif')
      R=RUNQQ(TRIM(VIEWER),TRIM(FNAME)//'.gif')    
C      R=RUNQQ('C:\Program Files\IrfanView\i_view32.exe',TRIM(FNAME)//'.gif')
c      R=SYSTEMQQ('"C:\Program Files\IrfanView\i_view32.exe" b.gif')
c      R=SYSTEMQQ('rundll32.exe '//
c     &'C:\Windows\system32\shimgvw.dll,ImageView_Fullscreen B.gif')
      RETURN
      END SUBROUTINE PlotTREE
C******************************************************* SAVETREE
C  PURPOSE:
C  SAVE THE PEDIGREE TREE STORED IN COMMON /WRK/ TO A LIST
C
C  ARGUMENTS:
C  LINE  - CURRENT LINE IN GTREE TO WRITE
C  LEVEL - LEVEL OF LINE IN THE PEDIGREE
C  LINEP - LINE NUMBER OF THE PREVIOUS LEVEL
C  LSTLVL- LAST LEVEL TO DRAW
C  LST   - .TRUE. IF CURRENT LINE IS LAST PROGENITOR IN A SET
C  IN /OPTIONS/
C  LDST  - 0 SAVE PARENTS ONLY AND ALL NAMES,
C          1 ALL STEPS AND ALL NAMES
C
C---- 09/01/02 ----------------------------------------- SAVETREE
      RECURSIVE SUBROUTINE SAVETREE(LINE,LEVEL,CWK,LSTLVL,LST)
      USE GMS_STRUCTURES
      USE GMS_INTERFACE
      USE GMS_TOOLS
      PARAMETER (MAXTREE=30000)      
      TYPE(GMS_GERMPLASM) GERM
      TYPE(GMS_TREE) GTREE(MAXTREE)
      TYPE(GMS_NAMEDATA) NAME
      TYPE(GMS_LISTDATA) LSTDTA
      CHARACTER CWK*164,MRK*1,ANAME*164,STEP*3,NULL*1,VIEWER*60
      CHARACTER GVIZPATH*60
      LOGICAL LST
      COMMON /WRK/ NCR,MXTREE,GTREE
      COMMON /OPTIONS/ NUID,NLOC,NREF,IOPT,LIST,ISRH,MGAT,NAMP,NTC
     &,IBTYPE,LDST,IYAL,NMSTD,VIEWER,GVIZPATH
      DATA NULL/0/
      LSTDTA%LISTID=LIST
      L=2*LEVEL-1
      IF(LEVEL.EQ.0) THEN
      CWK=' '
      ELSE IF (CWK(L+2:L+2).NE.'>') THEN
      CWK(L:)=' +-'
      DO 1 I=2,L,2
      IF(CWK(I:I).NE.' ') CWK(I:I)='|'
 1    CONTINUE
      ENDIF
      LGID=GTREE(LINE)%GID
      I=LEN_TRIM(CWK)
      I4=GMS_getGermplasm2(LGID,GERM,NAME,ANAME,80)
      CWK=CWK(1:I)//CTOFSTR(ANAME)
      ANAME=LSTNMS(LGID,163)
C LOCATE THE GROUP LINE FOR THE CURRENT PROGENITOR AND
C COUNT DERIVATIVE STEPS
      LN=LINE
      NSTP=0
      IF(GTREE(LINE)%GNPGS.EQ.-1.AND.GTREE(LINE)%DADDR.LE.0) NSTP=-1
      IF(GTREE(LINE)%DADDR.GE.0) THEN
      LN=GTREE(LINE)%GADDR
      I=GTREE(LINE)%DADDR
      IF(I.GT.0) THEN
      DO WHILE (GTREE(I)%DADDR.GT.0)
      I=I+1
      ENDDO
      NSTP=I-GTREE(LINE)%DADDR+1
      IF(GTREE(I)%DADDR.GE.0) NSTP=-1
      ENDIF
      ENDIF
C
      MRK=' '
      IF(GTREE(LN)%GADDR.EQ.0) MRK='#'
      IF(GTREE(LN)%GADDR.EQ.-1.AND.(LDST.EQ.0.OR.LN.EQ.LINE)) MRK='<'
      IF(GTREE(LN)%GADDR.GT.0.AND.GTREE(LN)%GADDR.LT.LINE) MRK='*'
      IF(GTREE(LN)%GNPGS.EQ.-1.AND.LN.NE.LINE) MRK=' '
      STEP='  ?'
      IF(NSTP.GE.0) WRITE(STEP,'(I3)') NSTP
      LSTDTA%GID=LGID
      WRITE(LSTDTA%ENTRYCD,'(I4,1X,A3,2A1)') LINE,STEP,MRK,NULL
      LSTDTA%SOURCE=CNTRY(GERM%LOCATION)//NULL
      LSTDTA%DESIG=TRIM(ANAME)//NULL
      LSTDTA%GRPNAME=TRIM(CWK)//NULL
      LSTDTA%ENTRYID=0
	LSTDTA%LRECID=0
      I4=GMS_addListData(LSTDTA)
      IF(I4.NE.1) STOP 'SAVTREE'
C  IF LDST.EQ.1 THEN LIST ALL INTERMEDIATE DERIVATIVES
      IF(LDST.EQ.1.AND.MRK.NE.'*') THEN
      IF(GTREE(LINE)%DADDR.GT.0) THEN
      ILN=GTREE(LINE)%DADDR
 4    IF(ILN.EQ.-1) GOTO 3
      IF(L.LT.1) THEN
      CWK='>'
      ELSE
      CWK(L:)=' |>'
      IF(LST) CWK(L:)='  >'
      ENDIF
      NSTP=NSTP-1
      STEP='  ?'
      IF(NSTP.GE.0) WRITE(STEP,'(I3)') NSTP
      LGID=GTREE(ILN)%GID
      I=LEN_TRIM(CWK)
      I4=GMS_getGermplasm2(LGID,GERM,NAME,ANAME,80)
      CWK=CWK(1:I)//CTOFSTR(ANAME)
      ANAME=LSTNMS(LGID,163)
      MRK=' '
      IF(GTREE(ILN)%GADDR.EQ.0) MRK='#'
      IF(GTREE(ILN)%GADDR.EQ.-1) MRK='<'
      IF(GTREE(ILN)%GADDR.GT.0.AND.GTREE(ILN)%GADDR.LT.LINE) MRK='*'
      LSTDTA%GID=LGID
      WRITE(LSTDTA%ENTRYCD,'(I4,1X,A3,2A1)') LINE,STEP,MRK,NULL
      LSTDTA%SOURCE=CNTRY(GERM%LOCATION)//NULL
      LSTDTA%DESIG=TRIM(ANAME)//NULL
      LSTDTA%GRPNAME=TRIM(CWK)//NULL
      LSTDTA%ENTRYID=0
	LSTDTA%LRECID=0
      I4=GMS_addListData(LSTDTA)
      IF(I4.NE.1) STOP 'SAVTREE'
      I=ILN
      ILN=GTREE(ILN)%DADDR
      IF(ILN.EQ.0) ILN=I+1
      GOTO 4
 3    CWK(L+2:)=' '
      ENDIF
      IF(LST) CWK(L:)=' '
      ELSE
      IF(LST.OR.CWK(L+2:L+2).EQ.'>') CWK(L:)=' '
C  IF LDST.EQ.0 AND PROGENITOR HAS A DERIVATIVE GROUP SOURCE SHOW IT
      IF(GTREE(LN)%GNPGS.EQ.-1.AND.LN.NE.LINE) THEN
      CWK(L:)=' |>'
      IF(LST) CWK(L:)='  >'
      NLINE=LN
      NLEVEL=LEVEL
      CALL SAVETREE(NLINE,NLEVEL,CWK,LSTLVL,.TRUE.)
      ENDIF
      ENDIF
C
      NLEVEL=LEVEL+1
      IF(NLEVEL.GT.LSTLVL.OR.GTREE(LN)%GADDR.LE.0.OR.MRK.EQ.'*') RETURN
      K=IABS(GTREE(LN)%GNPGS)
      DO 2,I=1,K
      NLINE=GTREE(LN)%GADDR+I-1
      CALL SAVETREE(NLINE,NLEVEL,CWK,LSTLVL,(I.EQ.K))
 2    CONTINUE
      RETURN
      END SUBROUTINE SAVETREE
C******************************************************* TABTREE
C  PURPOSE:
C  SAVE THE PEDIGREE TREE STORED IN COMMON /WRK/ TO A LIST
C
C  ARGUMENTS:
C  LINE  - CURRENT LINE IN GTREE TO WRITE
C  LEVEL - LEVEL OF LINE IN THE PEDIGREE
C  LINEP - LINE NUMBER OF THE PREVIOUS LEVEL
C  LSTLVL- LAST LEVEL TO DRAW
C  LST   - .TRUE. IF CURRENT LINE IS LAST PROGENITOR IN A SET
C  IN /OPTIONS/
C  LDST  - 0 SAVE PARENTS ONLY AND ALL NAMES,
C          1 ALL STEPS AND ALL NAMES
C
C---- 09/01/02 ----------------------------------------- TABTREE
      RECURSIVE SUBROUTINE TABTREE(LINE,LEVEL,LSTLVL)
      USE GMS_STRUCTURES
      USE GMS_INTERFACE
      USE GMS_TOOLS
      PARAMETER (MAXTREE=30000)      
      TYPE(GMS_GERMPLASM) GERM
      TYPE(GMS_TREE) GTREE(MAXTREE)
      TYPE(GMS_NAMEDATA) NAME
      TYPE(GMS_LISTDATA) LSTDTA
      CHARACTER ANAME*80,STEP*3,VIEWER*60,GVIZPATH*60
      LOGICAL LST
      COMMON /WRK/ NCR,MXTREE,GTREE
      COMMON /OPTIONS/ NUID,NLOC,NREF,IOPT,LIST,ISRH,MGAT,NAMP,NTC
     &,IBTYPE,LDST,IYAL,NMSTD,VIEWER,GVIZPATH
      LGID=GTREE(LINE)%GID
      I4=GMS_getGermplasm2(LGID,GERM,NAME,ANAME,80)
      ANAME=CTOFSTR(ANAME)
C LOCATE THE GROUP LINE FOR THE CURRENT PROGENITOR AND
C COUNT DERIVATIVE STEPS
      LN=LINE
      NSTP=0
      IF(GTREE(LINE)%GNPGS.EQ.-1.AND.GTREE(LINE)%DADDR.LE.0) NSTP=-1
      IF(GTREE(LINE)%DADDR.GE.0) THEN
      LN=GTREE(LINE)%GADDR
      I=GTREE(LINE)%DADDR
      IF(I.GT.0) THEN
      DO WHILE (GTREE(I)%DADDR.GT.0)
      I=I+1
      ENDDO
      NSTP=I-GTREE(LINE)%DADDR+1
      IF(GTREE(I)%DADDR.GE.0) NSTP=-1
      ENDIF
      ENDIF
      STEP='  ?'
      IF(NSTP.GE.0) WRITE(STEP,'(I3)') NSTP
C
      NLEVEL=LEVEL+1
      IF(NLEVEL.GT.LSTLVL) RETURN
      DO 3 I=1,NCR
      IF(LGID.EQ.GTREE(I)%GID.AND.GTREE(I)%METHN.EQ.0) RETURN
 3    CONTINUE
      K=IABS(GTREE(LN)%GNPGS)
      MGID=0
      NGID=0
      IF(GTREE(LN)%GADDR.GT.0) THEN
      DO 2,I=1,K
      NLINE=GTREE(LN)%GADDR+I-1
      IF(I.EQ.1) MGID=GTREE(NLINE)%GID
      IF(I.EQ.2) NGID=GTREE(NLINE)%GID
      CALL TABTREE(NLINE,NLEVEL,LSTLVL)
 2    CONTINUE
      ENDIF
      WRITE(3,100) LGID,MGID,NGID,STEP,TRIM(ANAME)
      WRITE(0,100) LGID,MGID,NGID,STEP,TRIM(ANAME)
      GTREE(LINE)%METHN=0
 100  FORMAT(3I10,A4,1X,'"',A,'"')
      RETURN
      END SUBROUTINE TABTREE
C******************************************************* MGRAM
C  PURPOSE:
C  DISPLAY THE MENDELGRAM FOR A PEDIGREE
C
C  ARGUMENTS:
C  IGID - TARGET GID
C  LVL  - NUMBER OF GENERATIVE LEVELS AT WHICH TO TERMINATE THE TREE
C  LOCN - IF LOCN=K TERMINATE RETRIEVAL WHEN GERMPLASM
C         WITH GLOCN=K IS ENCOUNTERED, ELSE 0
C  IN /OPTIONS/
C  MGAT - FLAG FOR SCOPE OF MENDELGRAM
C         MGAT=0 FOR CONTRIBUTIONS FROM TERMINAL ANCESTORS ONLY
C         MGAT=1 FOR CONTRIBUTIONS FROM ALL ANCESTORS
C
C---- LAST UPDATE 02/09/10 ----------------------------- MGRAM
      RECURSIVE SUBROUTINE MGRAM(IGID,LVL,LOCN)
      USE GMS_STRUCTURES
      USE GMS_INTERFACE
      USE GMS_TOOLS
      PARAMETER (MAXTREE=30000)      
      TYPE(GMS_GERMPLASM) GERM
      TYPE(GMS_NAMEDATA) NAME
      CHARACTER CWK*80,MRK*2,NULL*1,CTMP*3,VIEWER*60
      CHARACTER GVIZPATH*60      
      LOGICAL TRMNL
      INTEGER*2 ITREE(MAXTREE,6)
      INTEGER*4 GTREE(MAXTREE,3),LOCN
      DIMENSION RTREE(MAXTREE,3)
      COMMON /WRK/ NCR,MXTREE,ITREE
      COMMON /OPTIONS/ NUID,NLOC,NREF,IOPT,LIST,ISRH,MGAT,NAMP,NTCX
     &,IBTYPE,LDST,IYAL,NMSTD,VIEWER,GVIZPATH
      EQUIVALENCE (ITREE(1,1),RTREE(1,1)),(ITREE(1,1),GTREE(1,1))
      DATA NULL /0/
      NCR=0
      CALL GETTREE1(IGID,LVL,LOCN,IER)
      IF(IER.NE.1) PAUSE 'BUFFER SPACE IN MGRAM'
      TRMNL=.TRUE.
      IF(MGAT.EQ.1) TRMNL=.FALSE.
      MTM=0
      DO 1 I=1,NCR
      IF(TRMNL.AND.ITREE(I,6).GT.0) GOTO 1
      DO 2 J=1,MTM
      IF(GTREE(NCR+J,1).EQ.GTREE(I,1)) GOTO 1
 2    CONTINUE
      IF(NCR+MTM.EQ.MXTREE) THEN
	PAUSE 'TOO MANY ENTRIES IN PEDIGREE TREE'
	RETURN
	ENDIF
      MTM=MTM+1
      GTREE(NCR+MTM,1)=GTREE(I,1)
      RTREE(NCR+MTM,2)=0.0
      ITREE(NCR+MTM,5)=ITREE(I,5)
      ITREE(NCR+MTM,6)=0
 1    CONTINUE
      MXTREE=NCR+MTM
      CALL MGRM(1,1.0)
C      DO 9 I=NCR+1,MXTREE
C 9    WRITE(0,'(I10,F10.3,2I10)') GTREE(I,1),
C     &RTREE(I,2),ITREE(I,5),ITREE(I,6)
      L=0
      DO 3 I=1,MTM
      K=0
      CMX=-1.0
      DO 4 J=1,MTM
      IF(RTREE(NCR+J,2).LE.CMX) GOTO 4
      K=NCR+J
      CMX=RTREE(K,2)
 4    CONTINUE
      L=L+1
      GTREE(L,1)=GTREE(K,1)
      RTREE(L,2)=RTREE(K,2)
      ITREE(L,5)=ITREE(K,5)
      ITREE(L,6)=ITREE(K,6)
      RTREE(K,2)=-1*RTREE(K,2)
 3    CONTINUE
      NCR=MTM
      I4=GMS_getGermplasm2(IGID,GERM,NAME,CWK,80)
      IF(I4.NE.1) STOP 'MRGM'
      J=INDEX(CWK,NULL)
      CWK(J:)=' '
      WRITE(3,101) IGID,TRIM(CWK)
      WRITE(0,101) IGID,TRIM(CWK)
 101  FORMAT(' MENDELGRAM FOR GID',I9,' - ',A/' CONT IS THE PROBAB',
     &'ILITY THAT AN UNSELECTED ALLELE COMES FROM THAT PROGENITOR'/
     &' TYPE IS THE TYPE OF PROGENITOR: T - TERMINAL,'/
     &' IG - INTERMEDIATE GENERATIVE, ID - INTERMEDIATE DERIVATIVE'/
     &/'  NO    GID  NAME OF PROGENITOR',28X,'TYPE  CONT COUNT CTY')
      K=0
      RICH=1.0
      CLOC=0.0
      DO 5 I=1,NCR
      GERM%LOCATION=0
      I4=GMS_getGermplasm2(GTREE(I,1),GERM,NAME,CWK,80)
      IF(I4.NE.1) CWK='<Unknown>'C
      J=INDEX(CWK,NULL)
      CWK(J:)=' '
      MRK='IG'
      IF(ITREE(I,5).EQ.-1) MRK='ID'
      IF(GERM%PROGENITOR_ID1.EQ.0) MRK='T'
      CTMP=CNTRY(GERM%LOCATION)
      IF(LOCN.NE.0.AND.GERM%LOCATION.EQ.LOCN) CLOC=CLOC+RTREE(I,2)
      WRITE(0,100) I,GTREE(I,1),CWK,MRK,RTREE(I,2),ITREE(I,6),CTMP
      WRITE(3,100) I,GTREE(I,1),CWK,MRK,RTREE(I,2),ITREE(I,6),CTMP
 100  FORMAT(I3,1X,I9,1X,A47,A2,F8.5,I4,1X,A3)
      IF(MRK.EQ.'IG'.AND.ITREE(I,5).GT.1) K=K+ITREE(I,6)
      IF(MRK.EQ.'T') RICH=RICH-RTREE(I,2)*RTREE(I,2)
 5    CONTINUE
      WRITE(0,103) RICH
 103  FORMAT(' RICHNESS INDEX=',F6.4,' (PROBABILITY THAT TWO RANDOMLY',
     &' CHOSEN,'/' UNSELECTED LOCI HAVE ALLELS FROM DIFFERENT',
     &' TERMINAL SOURCES)')
      IF(NCR.LT.MXTREE) THEN
      GTREE(NCR+1,1)=IGID
      RTREE(NCR+1,2)=RICH
      ENDIF
      IF(.NOT.TRMNL) WRITE(0,102) K
      IF(.NOT.TRMNL) WRITE(3,102) K
 102  FORMAT(' TOTAL NUMBER OF CROSSES IN PEDIGREE',I6)
      IF(LOCN.NE.0) THEN 
      WRITE(0,104) LOCN, CLOC
      WRITE(3,104) LOCN, CLOC
      ENDIF
 104  FORMAT(' TOTAL CONTRIBUTION FROM LOCATION',I5,' IS ', F8.5)     
      RETURN
      END SUBROUTINE MGRAM
C******************************************************* MGRM
C  PURPOSE:
C  DISPLAY THE MENDELGRAM FOR A PEDIGREE
C
C  ARGUMENTS:
C  LINE - CURRENT LINE IN GTREE TO WRITE
C  CONT - COTRIBUTION AT CURRENT LEVEL
C
C---- LAST UPDATE 30/04/99 ----------------------------- MGRM
      RECURSIVE SUBROUTINE MGRM(LINE,CONT)
      PARAMETER (MAXTREE=30000)      
      INTEGER*2 ITREE(MAXTREE,6)
      DIMENSION RTREE(MAXTREE,3)
      INTEGER*4 GTREE(MAXTREE,3)
      COMMON /WRK/ NCR,MXTREE,ITREE
      EQUIVALENCE (ITREE(1,1),RTREE(1,1)),(ITREE(1,1),GTREE(1,1))
      CONTL=CONT*1.0/ABS(ITREE(LINE,5))
      DO 1 I=NCR+1,MXTREE
      IF(GTREE(LINE,1).NE.GTREE(I,1)) GOTO 1
      RTREE(I,2)=RTREE(I,2)+CONT
      ITREE(I,6)=ITREE(I,6)+1
      EXIT
 1    CONTINUE
      IF(ITREE(LINE,6).LE.0) RETURN
      K=IABS(ITREE(LINE,5))
      DO 2,I=1,K
      NLINE=ITREE(LINE,6)+I-1
      CALL MGRM(NLINE,CONTL)
 2    CONTINUE
      RETURN
      END SUBROUTINE MGRM
C******************************************************* GETTREE
C  PURPOSE:
C  RETRIEVE A PEDIGREE TREE FROM GMS
C
C  ARGUMENTS:
C  IGID - TARGET GID
C  LEVEL- NUMBER OF GENERATIVE GENERATIONS TO FETCH
C  IER  - 0 IF RETRIEVAL WAS TERMINATED DUE TO BUFFER OR DATA ERROR
C         1 IF ALL REQUESTED RECORDS WERE RETRIEVED
C  COMMON ARGUMENTS:
C  COMMON /WRK/ RETURNS WITH THE NUMBER OF GIDs FETCHED IN NCR,
C         MXTREE CONTAINS THE NUMBER ALLOWED (BUFFER SIZE)
C         AND FOR EACH LINE GTREE(I) CONTAINS THE GID, METHN,
C         GNPGS AND POINTERS TO THE SOURCE (DADDR) AND/OR
C         THE FIRST PARENT (GADDR) IN THE ARRAY GTREE.
C         GTREE(I)%DADDR=-1 IF THE LINE IS GENERATIVE OR
C         A DERIVATIVE GROUP SOURCE, IT IS 0 IF THE LINE
C         HAS KNOWN GROUP BUT UNKNOWN SOURCE.
C         GTREE(I)%GADDR=0 IF THE LINE IS A DERIVATIVE GROUP
C         SOURCE OR IS AN UNKNOWN GENERATIVE. IT IS -1 IF
C         EXPANSION WAS TERMINATED DUE TO THE LEVEL PARAMETER
C
C  NOTES:
C  FOR A NEW TREE NCR MUST BE SET TO ZERO IN THE CALLING PROGRAM
C  BUT THE GTREE MAY ALREADY HOLD SOME ENTRIES IF NCR>0 IN THIS
C  CASE THE NEW ROOT IS ADDED TO GTREE AND THE PEDIGREE EXTRACTED
C  LINKING TO PREVIOUS RECORDS WHERE APPROPRIATE.
C  GTREE CAN HOLD DUPLICATE ENTRIES BECAUSE ALL GNPGS PARENTS OF
C  GENERATIVE GERMPLASM MUST FOLLOW IN CONSECUTIVE RECORDS 
C  FROM RECORD GADDR. THIS ALLOWS MULTIPLE PARENTS.
C
C---- LAST UPDATE 09/01/02 ----------------------------- GETTREE
      SUBROUTINE GETTREE(IGID,LEVEL,IER)
      USE GMS_STRUCTURES
      USE GMS_INTERFACE
      PARAMETER (MAXTREE=30000)
      TYPE(GMS_GERMPLASM) GERM
      TYPE(GMS_TREE) GTREE(MAXTREE)
      INTEGER*4 FPR
      COMMON /WRK/ NCR,MXTREE,GTREE
C      DIMENSION LVLS(1000)
      MXTREE=MAXTREE
      IER=0
      LVL=0
      NCR=NCR+1
	IF(NCR.GT.MXTREE) GOTO 3
      MCR=NCR
      GTREE(MCR)%GID=IGID
      GTREE(MCR)%METHN=0
      GTREE(MCR)%GNPGS=0
      GTREE(MCR)%DADDR=0
      GTREE(MCR)%GADDR=-2
C      LVLS(MCR)=LVL
C  PROCESS THE NEXT RECORD IN THE TREE
C  UNPROCESSED RECORDS HAVE GADDR<0
C  ... RETRIEVE LEVEL OF THE CURRENT UNPROCESSED RECORD FROM DADDR
 1    IF(GTREE(MCR)%GADDR.GE.-1) GOTO 8
      LVL=GTREE(MCR)%DADDR
C      IF(LVL.GT.LEVEL) GOTO 1
      IF(GTREE(MCR)%GID.EQ.0) THEN
      GTREE(MCR)%METHN=0
      GTREE(MCR)%GNPGS=0
      GTREE(MCR)%DADDR=0
      GTREE(MCR)%GADDR=0
      ELSE
C  CHECK IF PROGENITOR MCR HAS ALREADY BEEN PROCESSED
      DO 5 I=1,MCR-1
      IF(GTREE(MCR)%GID.NE.GTREE(I)%GID) GOTO 5
      GTREE(MCR)%METHN=GTREE(I)%METHN
      GTREE(MCR)%GNPGS=GTREE(I)%GNPGS
      GTREE(MCR)%DADDR=GTREE(I)%DADDR
      GTREE(MCR)%GADDR=GTREE(I)%GADDR
      GOTO 8
 5    CONTINUE
C  LOAD GERMPLASM CHARACTERISTICS FOR CURRENT RECORD
      K=GMS_getGermplasm(GTREE(MCR)%GID,GERM)
      IF(GERM%NO_PROGENITORS.EQ.0.AND.GERM%PROGENITOR_ID1.NE.0) THEN
      WRITE(0,104) GERM%GERMPLASM_ID
 104  FORMAT(' *** ERROR *** ILLEGAL NO OF PROGNITORS AT GID',I9)
      GOTO 3
      ENDIF
      GTREE(MCR)%METHN=GERM%METHOD
      GTREE(MCR)%GNPGS=GERM%NO_PROGENITORS
      GTREE(MCR)%DADDR=-1
      FPR=GERM%PROGENITOR_ID1
      IF(FPR.EQ.0) GTREE(MCR)%GADDR=0
      IF(FPR.NE.0.OR.GERM%NO_PROGENITORS.GT.0) THEN
C
C  FOR DERIVATIVES WITH KNOWN GROUP
      IF(GERM%NO_PROGENITORS.EQ.-1) THEN
C  ... ADD ALL DERIVATIVE STEPS
      NSTEPS=0
      IF(NCR.LT.MXTREE) GTREE(MCR)%DADDR=NCR+1
 6    MPR=GERM%PROGENITOR_ID2
      IF(MPR.EQ.0) THEN
      IF(NSTEPS.EQ.0) GTREE(MCR)%DADDR=0
      IF(NSTEPS.GE.1) GTREE(NCR)%DADDR=0
      GOTO 7
      ENDIF
      IF(NSTEPS.GE.1) GTREE(NCR)%DADDR=NCR+1
      IF(MPR.EQ.FPR) GOTO 7
      NSTEPS=NSTEPS+1
      K=GMS_getGermplasm(MPR,GERM)
      IF(NSTEPS.GE.100) THEN
      WRITE(0,101) MPR
 101  FORMAT(' *** ERROR *** SUSPECTED CIRCULARITY AT GID',I9)
      GOTO 3
      ENDIF
      IF(GERM%PROGENITOR_ID1.NE.FPR) THEN
      WRITE(0,102) GTREE(MCR)%GID,GERM%GERMPLASM_ID
 102  FORMAT(' *** ERROR *** DERIVATIVE CHANGES GROUP BETWEEN GID'
     %,I9,' and',I9)
      GOTO 3
      ENDIF
      IF(NCR.EQ.MXTREE) GOTO 3
      NCR=NCR+1
      GTREE(NCR)%GID=GERM%GERMPLASM_ID
      GTREE(NCR)%METHN=GERM%METHOD
      GTREE(NCR)%GNPGS=GERM%NO_PROGENITORS
      GTREE(NCR)%DADDR=0
      GTREE(NCR)%GADDR=0
      GOTO 6
C
 7    K=GMS_getGermplasm(FPR,GERM)
      IF(GERM%NO_PROGENITORS.EQ.0.AND.GERM%PROGENITOR_ID1.NE.0) THEN
      WRITE(0,104) GERM%GERMPLASM_ID
      GOTO 3
      ENDIF
      IF(NCR.EQ.MXTREE) GOTO 3
      NCR=NCR+1
      GTREE(NCR)%GID=GERM%GERMPLASM_ID
      GTREE(NCR)%METHN=GERM%METHOD
      GTREE(NCR)%GNPGS=GERM%NO_PROGENITORS
      GTREE(NCR)%DADDR=-1
      GTREE(NCR)%GADDR=-1
C
      GTREE(MCR)%GADDR=NCR
      DO 4 I=1,NSTEPS
      GTREE(NCR-I)%GADDR=NCR
 4    CONTINUE
C
C      IF(GERM%PROGENITOR_ID1.NE.0.OR.GERM%NO_PROGENITORS.GT.0)
C     & FPR=GERM%PROGENITOR_ID1
      IF(GTREE(NCR)%GNPGS.GE.0) LVL=LVL+1
      GTREE(NCR)%GADDR=-1
C      IF(LVL.GT.LEVEL) GOTO 8      
      IF(LVL.LE.LEVEL) GTREE(NCR)%GADDR=0
      IF(LVL.LE.LEVEL.AND.
     &(GERM%PROGENITOR_ID1.NE.0.OR.GERM%NO_PROGENITORS.GT.0))
     &GTREE(NCR)%GADDR=NCR+1
      ELSE
      IF(GTREE(MCR)%GNPGS.GE.0) LVL=LVL+1
      GTREE(MCR)%GADDR=-1
C      IF(LVL.GT.LEVEL) GOTO 8
      IF(LVL.LE.LEVEL) GTREE(MCR)%GADDR=0
      IF(LVL.LE.LEVEL.AND.
     &(GERM%PROGENITOR_ID1.NE.0.OR.GERM%NO_PROGENITORS.GT.0))
     &GTREE(MCR)%GADDR=NCR+1
      ENDIF
C      IF(LVL.GT.LEVEL) GOTO 8      
      IF(LVL.LE.LEVEL.AND.
     &(GERM%PROGENITOR_ID1.NE.0.OR.GERM%NO_PROGENITORS.GT.0)) THEN
      IF(NCR.EQ.MXTREE) GOTO 3
      NCR=NCR+1
      FPR=GERM%PROGENITOR_ID1
      GTREE(NCR)%GID=FPR
      GTREE(NCR)%DADDR=LVL
C      LVLS(NCR)=LVL
      GTREE(NCR)%METHN=0
      GTREE(NCR)%GNPGS=0
      GTREE(NCR)%GADDR=-2
      IF(GERM%NO_PROGENITORS.GT.1) THEN
      MPR=GERM%PROGENITOR_ID2
      IF(NCR.EQ.MXTREE) GOTO 3
      NCR=NCR+1
      GTREE(NCR)%GID=MPR
      GTREE(NCR)%DADDR=LVL
C      LVLS(NCR)=LVL      
      GTREE(NCR)%METHN=0
      GTREE(NCR)%GNPGS=0
      GTREE(NCR)%GADDR=-2
      DO 2 I=3,GERM%NO_PROGENITORS
      K=GMS_getProgenitorID(GERM%GERMPLASM_ID,I,NPR)
      IF(NCR.EQ.MXTREE) GOTO 3
      NCR=NCR+1
      GTREE(NCR)%GID=NPR
      GTREE(NCR)%DADDR=LVL
C      LVLS(NCR)=LVL      
      GTREE(NCR)%METHN=0
      GTREE(NCR)%GNPGS=0
      GTREE(NCR)%GADDR=-2
 2    CONTINUE
      ENDIF
      ENDIF
      ENDIF
      ENDIF
 8    MCR=MCR+1
      IF(MCR.LE.NCR) GOTO 1
      IER=1
c      WRITE(3,193)
c 193  FORMAT(5X,
c     &' LINE   GID         METHN     GNPGS      DADDR     GADDR')
c      DO 190,I=1,NCR
c 190  WRITE(3,'(7I10)') I,GTREE(I)
      RETURN
 3    IER=0
      WRITE(0,103) NCR
 103  FORMAT(' *** ERROR *** RETRIEVAL OF GENEALOGY FAILED',I5)
C      WRITE(0,193)
C 9    DO 10,I=1,NCR
C 10   WRITE(0,'(6I10)') I,GTREE(I)
      RETURN
      END SUBROUTINE GETTREE
C******************************************************* GETTREE1
C  PURPOSE:
C  RETRIEVE A PEDIGREE TREE FROM GMS
C
C  ARGUMENTS:
C  IGID - TARGET GID
C  LEVEL- NUMBER OF GENERATIVE GENERATIONS TO FETCH
C  LOCN - IF LOCN=K TERMINATE RETRIEVAL WHEN GERMPLASM
C         WITH GLOCN=K IS ENCOUNTERED, ELSE 0
C  IER  - 0 IF RETRIEVAL WAS TERMINATED DUE TO BUFFER SIZE
C         1 IF ALL REQUESTED RECORDS WERE RETRIEVED
C  COMMON ARGUMENTS:
C  COMMON /WRK/ RETURNS WITH THE NUMBER OF GIDs FETCHED IN NCR,
C             MXTREE CONTAINS THE NUMBER ALLOWED (BUFFER SIZE)
C             AND FOR EACH GID:
C  GTREE(I,1) CONTAINS THE GID FOR PROGENITOR I OR ZERO
C             IF PROGENITOR I IS REFERENCED BUT UNKNOWN
C             (GNPGS>x AND GPIDx=0 IN DESCENDANT OF I)
C  ITREE(I,3) CONTAINS THE NUMBER OF DERIVATIVE STEPS FROM
C             GTREE(I,1) TO ITS GROUP SOURCE OR -1 IF UNKNOWN
C             PROVIDED RECORD I HAS BEEN FULLY PROCESSED.
C             IF IT HAS NOT THEN (ITREE(I,6)<-1) THEN ITREE(I,3) CONTAINS
C             THE GENERATIVE LEVEL OF PROGENITOR I WHICH IS
C             THE NUMBER OF CROSSES FROM THE TARGET GENOTYPE
C  ITREE(I,4) CONTAINS THE METHOD OF THE GROUP GERMPLASM OF
C             PROGEBNITOR I
C  ITREE(I,5) CONTAINS THE NUMBER OF PROGENITORS FOR THE GROUP
C             GERMPLASM OF PROGENITOR I IF IT HAS BEEN
C             FULLY PROCESSED (ITREE(I,6)<-1), OTHERWISE ZERO.
C  ITREE(I,6) IF GTREE(I,3)>0 THEN THIS IS THE LINE NUMBER
C             IN GTREE WHICH CONTAINS INFORMATION ON THE
C             FIRST PROGENITOR OF THE CURRENT GID.
C             IF ITREE(I,5)=+1 OR -1 THIS IS THE ONLY PROGENITOR
C             OTHERWISE SUBSEQUENT LINES: ITREE(I,6)+1...
C             ITREE(I,6)+ITREE(I,5)-1, CONTAIN INFORMATION
C             FOR THE OTHER PROGENITORS.
C             ITREE(I,6)=0 IF PROGENITOR I IS A TERMINAL
C             PROGENITOR (DUE TO LEVEL LIMIT) WHICH COULD BE
C             EXPANDED.
C             ITREE(I,6) IS -1 IF PROGENITOR I IS A TERMINAL
C             PROGENITOR WHICH COULD NOT BE EXPANDED.
C             ITREE(I,6) <-1 IF LINE I WAS NOT FULLY PROCESSED
C             DUE TO LACK OF BUFFER SPACE (MXTREE). IT IS NOT
C             KNOWN WHETHER PROGENITOR I IS TERMINAL OR
C             EXPANDABLE.
C
C---- LAST UPDATE 25/04/99 ----------------------------- GETTREE1
      SUBROUTINE GETTREE1(IGID,LEVEL,LOCN,IER)
      USE GMS_STRUCTURES
      USE GMS_INTERFACE
      PARAMETER (MAXTREE=30000)      
      TYPE(GMS_GERMPLASM) GERM
      INTEGER*2 ITREE(MAXTREE,6)
      INTEGER*4 GTREE(MAXTREE,3),FPR,LOCN
      COMMON /WRK/ NCR,MXTREE,ITREE
      EQUIVALENCE (ITREE(1,1),GTREE(1,1))
C Limit MXTREE to Integer*2 maximum for GETTREE1      
      MXTREE=MIN(MAXTREE,32000)
      IER=0
      NCR=NCR+1
      MCR=NCR
      GTREE(MCR,1)=IGID
      ITREE(MCR,3)=0
      ITREE(MCR,4)=0
      ITREE(MCR,5)=0
      ITREE(MCR,6)=-1
 1    LVL=ITREE(MCR,3)
C  IF A TERMINATION LOCATION IS SPECIFIED IN LOCN GET THE GLOCN OF THE CURRENT ENTRY 
      IG=1
      IF(LOCN.NE.0) THEN
      K=GMS_getGermplasm(GTREE(MCR,1),GERM)
      IG=GERM%LOCATION
      ENDIF
      IF(LVL.LE.LEVEL.and.IG.ne.LOCN) THEN
      ITREE(MCR,5)=0
      ITREE(MCR,6)=0
      IF(GTREE(MCR,1).EQ.0) THEN
      ITREE(MCR,3)=-1
      ITREE(MCR,5)=-1
      ITREE(MCR,6)=-1
      ELSE
C  CHECK IF PROGENITOR MCR HAS ALREADY BEEN PROCESSED
      DO 5 I=1,MCR-1
      IF(GTREE(MCR,1).NE.GTREE(I,1)) GOTO 5
      ITREE(MCR,3)=ITREE(I,3)
      ITREE(MCR,4)=ITREE(I,4)
      ITREE(MCR,5)=ITREE(I,5)
      ITREE(MCR,6)=ITREE(I,6)
      GOTO 8
 5    CONTINUE
C
 55   K=GMS_getGermplasm(GTREE(MCR,1),GERM)
      ITREE(MCR,3)=-1
      ITREE(MCR,4)=GERM%METHOD
      ITREE(MCR,5)=GERM%NO_PROGENITORS
      FPR=GERM%PROGENITOR_ID1
      IF(FPR.EQ.0) ITREE(MCR,6)=-1
      IF(FPR.NE.0.OR.GERM%NO_PROGENITORS.GT.0) THEN
      ITREE(MCR,3)=0
      IF(GERM%NO_PROGENITORS.EQ.-1) THEN
C  COUNT NUMBER OF DERIVATIVE STEPS (-1 IF UNKNOWN)
 6    ITREE(MCR,3)=ITREE(MCR,3)+1
      MPR=GERM%PROGENITOR_ID2
      IF(MPR.EQ.0) THEN
      ITREE(MCR,3)=-1
      GOTO 7
      ENDIF
      IF(MPR.EQ.FPR) GOTO 7
 66   K=GMS_getGermplasm(MPR,GERM)
      IF(ITREE(MCR,3).GE.100) THEN
      WRITE(0,101) MPR
 101  FORMAT(' *** ERROR *** SUSPECTED CIRCULARITY AT GID',I9)
      RETURN
      ENDIF
      IF(GERM%PROGENITOR_ID1.NE.FPR) THEN
      WRITE(0,102) GERM%GERMPLASM_ID
 102  FORMAT(' *** ERROR *** DERIVATIVE CHANGES GROUP AT GID',I9)
      RETURN
      ENDIF
      GOTO 6
C
 7    K=GMS_getGermplasm(FPR,GERM)
      ITREE(MCR,4)=GERM%METHOD
      ITREE(MCR,5)=GERM%NO_PROGENITORS
      IF(GERM%PROGENITOR_ID1.NE.0.OR.GERM%NO_PROGENITORS.GT.0)
     & FPR=GERM%PROGENITOR_ID1
      ENDIF
      IF(ITREE(MCR,5).GE.0) LVL=LVL+1
      IF(LVL.LE.LEVEL) THEN
      ITREE(MCR,6)=NCR+1
      IF(NCR.EQ.MXTREE) GOTO 3
      NCR=NCR+1
      GTREE(NCR,1)=FPR
      ITREE(NCR,3)=LVL
      ITREE(NCR,4)=0
      ITREE(NCR,5)=0
      ITREE(NCR,6)=-1*MCR
      IF(GERM%NO_PROGENITORS.GT.1) THEN
      MPR=GERM%PROGENITOR_ID2
      IF(NCR.EQ.MXTREE) GOTO 3
      NCR=NCR+1
      GTREE(NCR,1)=MPR
      ITREE(NCR,3)=LVL
      ITREE(NCR,4)=0
      ITREE(NCR,5)=0
      ITREE(NCR,6)=-1*MCR
      DO 2 I=3,ITREE(MCR,5)
      K=GMS_getProgenitorID(GERM%GERMPLASM_ID,I,NPR)
      IF(NCR.EQ.MXTREE) GOTO 3
      NCR=NCR+1
      GTREE(NCR,1)=NPR
      ITREE(NCR,3)=LVL
      ITREE(NCR,4)=0
      ITREE(NCR,5)=0
      ITREE(NCR,6)=-1*MCR
 2    CONTINUE
      ENDIF
      ENDIF
      ENDIF
      ENDIF
      ENDIF
 8    MCR=MCR+1
      IF(MCR.LE.NCR) GOTO 1
      IER=1
C      GOTO 9
c      WRITE(3,193)
c 193  FORMAT(5X,
c     &' LINE   GID         NSTEPS     METHOD     NPROJ     START'/
c     &8X,'I    GTREE(I,1) ITREE(I,3) ITREE(I,4) ITREE(I,5) ITREE(I,6)')
c      DO 190,I=1,NCR
c 190  WRITE(3,'(6I10)') I,GTREE(I,1),(ITREE(I,J),J=3,6)
      RETURN
 3    IER=0
      ITREE(MCR,6)=0
      WRITE(0,103) NCR
 103  FORMAT(' *** ERROR *** RETRIEVAL OF GENEALOGY FAILED',I5)
C      WRITE(3,193)
C 193  FORMAT(5X,
C     &' LINE   GID         NSTEPS     METHOD     NPROJ     START'/
C     &8X,'I    GTREE(I,1) ITREE(I,3) ITREE(I,4) ITREE(I,5) ITREE(I,6)')
C 9    DO 10,I=1,NCR
C 10   WRITE(3,'(6I10)') I,GTREE(I,1),(ITREE(I,J),J=3,6)
      RETURN
      END SUBROUTINE GETTREE1
C******************************************************* DRAWTREE1
C  PURPOSE:
C  DISPLAY THE PEDIGREE TREE STORED IN COMMON /WRK/
C
C  ARGUMENTS:
C  LINE  - CURRENT LINE IN GTREE TO WRITE
C  LEVEL - LEVEL OF LINE IN THE PEDIGREE
C  LINEP - LINE NUMBER OF THE PREVIOUS LEVEL
C  LSTLVL- LAST LEVEL TO DRAW
C  LST   - .TRUE. IF CURRENT LINE IS LAST PROGENITOR IN A SET
C  IN /OPTIONS/
C  NAMP  - ONE TO LIST ALL NAMES, ZERO FOR PREFERRED NAMES
C
C---- 27/04/99 ----------------------------------------- DRAWTREE1
      RECURSIVE SUBROUTINE DRAWTREE1(LINE,LEVEL,CWK,LSTLVL,LST)
      USE GMS_STRUCTURES
      USE GMS_INTERFACE
      USE GMS_TOOLS
      PARAMETER (MAXTREE=30000)      
      TYPE(GMS_GERMPLASM) GERM
      TYPE(GMS_NAMEDATA) NAME
      CHARACTER CWK*80,MRK*1,ANAME*80,STEP*3,VIEWER*60
      CHARACTER GVIZPATH*60      
      INTEGER*2 ITREE(MAXTREE,6)
      INTEGER*4 GTREE(MAXTREE,3)
      LOGICAL LST
      COMMON /WRK/ NCR,MXTREE,ITREE
      COMMON /OPTIONS/ NUID,NLOC,NREF,IOPT,LIST,ISRH,MGAT,NAMP,NTCX
     &,IBTYPE,LDST,IYAL,NMSTD,VIEWER,GVIZPATH
      EQUIVALENCE (ITREE(1,1),GTREE(1,1))
      L=2*LEVEL-1
      IF(LEVEL.EQ.0) THEN
      CWK=' '
      ELSE IF (CWK(L+2:L+2).NE.'>') THEN
      CWK(L:)=' +'
      DO 1 I=2,L,2
      IF(CWK(I:I).NE.' ') CWK(I:I)='|'
 1    CONTINUE
      ENDIF
      I=LEN_TRIM(CWK)
      IF(NAMP.EQ.1) THEN
      CWK=CWK(1:I)//LSTNMS(GTREE(LINE,1),61-I)
      ELSE
      NAME%GERMPLASM=GTREE(LINE,1)
      NAME%TYPE=0
      NAME%STATUS=1
      ANAME='<Unknown>'C
      I4=GMS_getName(NAME,ANAME,80,0)
      CWK=CWK(1:I)//CTOFSTR(ANAME)
      ENDIF
      IF(NAMP.EQ.2) THEN
      NAME%GERMPLASM=GTREE(LINE,1)
      NAME%TYPE=1
      NAME%STATUS=0
      I4=GMS_getName(NAME,ANAME,80,0)
      I=LEN_TRIM(CWK)
      IF(I4.EQ.1) CWK=CWK(1:I)//'('//TRIM(CTOFSTR(ANAME))//')'
      ENDIF
      MRK=' '
      IF(ITREE(LINE,6).EQ.0) MRK='<'
      IF(ITREE(LINE,6).EQ.-1) MRK='#'
      IF(ITREE(LINE,6).LT.-1) MRK='?'
      IF(ITREE(LINE,6).GT.0.AND.ITREE(LINE,6).LT.LINE) MRK='*'
      STEP='  ?'
      IF(ITREE(LINE,3).GE.0) WRITE(STEP,'(I3)') ITREE(LINE,3)
      WRITE(0,101) LINE,TRIM(CWK),STEP,GTREE(LINE,1),MRK
      WRITE(3,101) LINE,TRIM(CWK),STEP,GTREE(LINE,1),MRK
 101  FORMAT(I4,A,T67,A3,I8,A1)
      IF(LST.OR.CWK(L+2:L+2).EQ.'>') CWK(L:)=' '
      NLEVEL=LEVEL+1
      IF(ITREE(LINE,5).EQ.-1) NLEVEL=LEVEL
      IF(NLEVEL.GT.LSTLVL.OR.ITREE(LINE,6).LE.0.OR.MRK.EQ.'*') RETURN
      K=IABS(ITREE(LINE,5))
      IF(ITREE(LINE,5).EQ.-1.AND.ITREE(LINE,6).GT.0) THEN
      CWK(L:)=' |>'
      IF(LST) CWK(L:)='  >'
      ENDIF
      DO 2,I=1,K
      NLINE=ITREE(LINE,6)+I-1
      CALL DRAWTREE1(NLINE,NLEVEL,CWK,LSTLVL,(I.EQ.K))
 2    CONTINUE
      RETURN
      END SUBROUTINE DRAWTREE1
C******************************************************* SAVETREE1
C  PURPOSE:
C  SAVE THE PEDIGREE TREE STORED IN COMMON /WRK/
C
C  ARGUMENTS:
C  LINE  - CURRENT LINE IN GTREE TO WRITE
C  LEVEL - LEVEL OF LINE IN THE PEDIGREE
C  LINEP - LINE NUMBER OF THE PREVIOUS LEVEL
C  LSTLVL- LAST LEVEL TO DRAW
C  LST   - .TRUE. IF CURRENT LINE IS LAST PROGENITOR IN A SET
C  IN /OPTIONS/
C  NAMP  - ONE TO LIST ALL NAMES, ZERO FOR PREFERRED NAMES
C
C---- 27/04/99 ----------------------------------------- SAVETREE1
      RECURSIVE SUBROUTINE SAVETREE1(LINE,LEVEL,CWK,LSTLVL,LST)
      USE GMS_STRUCTURES
      USE GMS_INTERFACE
      USE GMS_TOOLS
      PARAMETER (MAXTREE=30000)      
      TYPE(GMS_GERMPLASM) GERM
      TYPE(GMS_NAMEDATA) NAME
      TYPE(GMS_LISTDATA) LSTDTA
      CHARACTER CWK*164,MRK*1,ANAME*164,STEP*3,NULL*1,VIEWER*60
      CHARACTER GVIZPATH*60
      INTEGER*2 ITREE(MAXTREE,6)
      INTEGER*4 GTREE(MAXTREE,3)
      LOGICAL LST
      COMMON /WRK/ NCR,MXTREE,ITREE
      COMMON /OPTIONS/ NUID,NLOC,NREF,IOPT,LIST,ISRH,MGAT,NAMP,NTCX
     &,IBTYPE,LDST,IYAL,NMSTD,VIEWER,GVIZPATH
      EQUIVALENCE (ITREE(1,1),GTREE(1,1))
      DATA NULL/0/
      LSTDTA%LISTID=LIST
      L=2*LEVEL-1
      IF(LEVEL.EQ.0) THEN
      CWK=' '
      ELSE IF (CWK(L+2:L+2).NE.'>') THEN
      CWK(L:)=' +'
      DO 1 I=2,L,2
      IF(CWK(I:I).NE.' ') CWK(I:I)='|'
 1    CONTINUE
      ENDIF
      I=LEN_TRIM(CWK)
      I4=GMS_getGermplasm2(GTREE(LINE,1),GERM,NAME,ANAME,80)
      CWK=CWK(1:I)//CTOFSTR(ANAME)
      ANAME=LSTNMS(GTREE(LINE,1),163)
      MRK=' '
      IF(ITREE(LINE,6).EQ.0) MRK='<'
      IF(ITREE(LINE,6).EQ.-1) MRK='#'
      IF(ITREE(LINE,6).LT.-1) MRK='?'
      IF(ITREE(LINE,6).GT.0.AND.ITREE(LINE,6).LT.LINE) MRK='*'
      STEP='  ?'
      IF(ITREE(LINE,3).GE.0) WRITE(STEP,'(I3)') ITREE(LINE,3)
      LSTDTA%GID=GTREE(LINE,1)
      WRITE(LSTDTA%ENTRYCD,'(I4,1X,A3,2A1)') LINE,STEP,MRK,NULL
      LSTDTA%SOURCE=CNTRY(GERM%LOCATION)//NULL
      LSTDTA%DESIG=TRIM(ANAME)//NULL
      LSTDTA%GRPNAME=TRIM(CWK)//NULL
      LSTDTA%ENTRYID=0
	LSTDTA%LRECID=0
      I4=GMS_addListData(LSTDTA)
      IF(I4.NE.1) STOP 'SAVTREE'
      IF(LST.OR.CWK(L+2:L+2).EQ.'>') CWK(L:)=' '
      NLEVEL=LEVEL+1
      IF(ITREE(LINE,5).EQ.-1) NLEVEL=LEVEL
      IF(NLEVEL.GT.LSTLVL.OR.ITREE(LINE,6).LE.0.OR.MRK.EQ.'*') RETURN
      K=IABS(ITREE(LINE,5))
      IF(ITREE(LINE,5).EQ.-1.AND.ITREE(LINE,6).GT.0) THEN
      CWK(L:)=' |>'
      IF(LST) CWK(L:)='  >'
      ENDIF
      DO 2,I=1,K
      NLINE=ITREE(LINE,6)+I-1
      CALL SAVETREE1(NLINE,NLEVEL,CWK,LSTLVL,(I.EQ.K))
 2    CONTINUE
      RETURN
      END SUBROUTINE SAVETREE1
C******************************************************* HASFIX
C  PURPOSE:
C  CHANGE GENERATIVE RECORDS TO UNKNOWN DERIVATIVES
C  WITH GROUP SOURCES FORMED BY LEVEL ONE CROSS
C  EXPANSION OF THE GENERATIVE RECORD.
C  THE GROUP SOURCES ARE ADDED IF THEY DO NOT EXIST.
C---- LAST UPDATE 01/06/04 ----------------------------- HASFIX
      SUBROUTINE HASFIX(LINE,IGID,GERM,IER)
      USE GMS_STRUCTURES
      USE GMS_INTERFACE
      USE GMS_TOOLS
      USE GMS_FUNCTIONS
      PARAMETER(MAXTREE=30000,MAXLIST=3000)
      CHARACTER AVAL*80,LCNM(400)*3,MCD(200)*8,LINE*(*),VIEWER*60
      CHARACTER GVIZPATH*60      
      CHARACTER CWK*80,ANAME*100,UDFCD(600)*8,NULL*1,BNAME*80
      CHARACTER NUMS*10,CWK2*80,RDATE*10,RTIME*10,UWRK*9900
      INTEGER*4 I4,UDFNO(600),LCN(400),MIDS(200),IWK(20)
      TYPE(GMS_GERMPLASM) GERM,GERMN,GERMA,GERMB,GERMX
      TYPE(GMS_NAMEDATA) NAMDAT,NAMDATA,NAMDATB,NAMDATX
      TYPE(GMS_UDFIELD) UDFLD
      COMMON /CODES/ NUDF,UDFNO,UDFCD,NCNTRY,LCN,LCNM,NMTHD,MIDS,MCD
      COMMON /WRK/ NLST,MXLST,LLST(MAXLIST),ANAME
      COMMON /OPTIONS/ NUID,NLOC,NREF,IOPT,LIST,ISRH,MGAT,NAMP,NTCX
     &,IBTYPE,LDST,IYAL,NMSTD,VIEWER,GVIZPATH
      EQUIVALENCE (IWK(1),CWK)
      DATA NULL /0/, NUMS /'1234567890'/
C
      IER=0
      CWK=LINE
      WRITE(0,111) CWK(1:LEN_TRIM(CWK))
 111  FORMAT(1X,70(1H-)/1X,A)
c      IF(LINE(1:5).NE.'.HAS '.and.LINE(1:5).NE.'.has ') RETURN
      L=6
      KGID=INUMBA(IWK,L,80,J,K)
      IF(K.GT.0) RETURN
      IF(KGID.NE.0) IGID=KGID
      IF(IGID.EQ.0) RETURN
      CALL WRTGERM(IGID,GERM,3)
      IF(IGID.EQ.0) RETURN
      IF(IGID.NE.GERM%GERMPLASM_ID) THEN
      WRITE(0,101)
      WRITE(3,101)
 101  FORMAT(' *** ERROR *** REPLACED GERMPLASM')
      RETURN
      ENDIF
      IDTE=0
      ITME=0
      IF(GERM%NO_PROGENITORS.NE.2) THEN
      WRITE(0,100)
      WRITE(3,100)
 100  FORMAT(' *** ERROR *** NON GENERATIVE GERMPLASM')
      RETURN
      ENDIF
      CWK=' '
      IGID=GERM%GERMPLASM_ID
      NAMDAT%GERMPLASM=IGID
      NAMDAT%STATUS=1
      I4=GMS_getName(NAMDAT,CWK,80,0)
      I=INDEX(CWK,NULL)
      IF(I.GT.0) CWK(I:)=' '
      NTYPE=3
      NDATE=0
      NLOC=0
      IF(INDEX(CWK,'/').EQ.0) THEN
          I4=
     &    GMS_getGermplasm2(GERM%PROGENITOR_ID1,GERMA,NAMDATA,ANAME,80)
          I4=
     &    GMS_getGermplasm2(GERM%PROGENITOR_ID2,GERMB,NAMDATB,BNAME,80)
          METHOD=106
          I=INDEX(ANAME,NULL)
          IF(I.GT.0) ANAME(I:)=' '
          I=INDEX(BNAME,NULL)
          IF(I.GT.0) BNAME(I:)=' '
          IF(GERMA%NO_PROGENITORS.GT.0.AND.INDEX(ANAME,'/').GT.0)THEN
            CALL CUTCROSS(ANAME,CWK,AVAL,I4,IER)
            IF (GERMA%PROGENITOR_ID1.EQ.GERMB%GERMPLASM_ID) THEN
              K4=LEN_TRIM(CWK)
              IF(ANAME(K4+1:K4+1).EQ.'*') THEN
                I=INDEX(NUMS,ANAME(K4+2:K4+2))
                CWK=ANAME(1:K4+1)//NUMS(I+1:I+1)//ANAME(K4+3:)
              ELSE
                CWK=ANAME(1:K4)//'*2'//ANAME(K4+1:)
              ENDIF
              METHOD=4
              NTYPE=3
            ELSE IF (GERMA%PROGENITOR_ID2.EQ.GERMB%GERMPLASM_ID) THEN
              K4=LEN_TRIM(ANAME)-LEN_TRIM(AVAL)
              IF(ANAME(K4:K4).EQ.'*') THEN
                I=INDEX(NUMS,ANAME(K4-1:K4-1))
                CWK=ANAME(1:K4-2)//NUMS(I+1:I+1)//ANAME(K4:)
              ELSE
                CWK=ANAME(1:K4)//'2*'//ANAME(K4+1:)
              ENDIF
              METHOD=4
              NTYPE=3
            ENDIF
            ELSE IF
     &      (GERMB%NO_PROGENITORS.GT.0.AND.INDEX(BNAME,'/').GT.0)THEN
              CALL CUTCROSS(BNAME,CWK,AVAL,I4,IER)
              IF (GERMB%PROGENITOR_ID1.EQ.GERMA%GERMPLASM_ID) THEN
                K4=LEN_TRIM(CWK)
                IF(BNAME(K4+1:K4+1).EQ.'*') THEN
                  I=INDEX(NUMS,BNAME(K4+2:K4+2))
                  CWK=BNAME(1:K4+1)//NUMS(I+1:I+1)//BNAME(K4+3:)
                ELSE
                  CWK=BNAME(1:K4)//'*2'//BNAME(K4+1:)
                ENDIF
              METHOD=4
              NTYPE=3
            ELSE IF (GERMB%PROGENITOR_ID2.EQ.GERMA%GERMPLASM_ID) THEN
              K4=LEN_TRIM(BNAME)-LEN_TRIM(AVAL)
              IF(BNAME(K4:K4).EQ.'*') THEN
                I=INDEX(NUMS,BNAME(K4-1:K4-1))
                CWK=BNAME(1:K4-2)//NUMS(I+1:I+1)//BNAME(K4:)
              ELSE
                CWK=BNAME(1:K4)//'2*'//BNAME(K4+1:)
              ENDIF
              METHOD=4
              NTYPE=3
            ENDIF
        ENDIF
        IF(METHOD.EQ.106) THEN
          CALL CUTCROSS(ANAME,CWK,AVAL,I4,IER)
          CALL CUTCROSS(BNAME,CWK,AVAL,K4,IER)
          IF(I4.EQ.0.AND.K4.EQ.0) METHOD=101
          IF(I4.EQ.0.AND.K4.EQ.1) METHOD=102
          IF(I4.EQ.1.AND.K4.EQ.0) METHOD=102
          IF(I4.EQ.1.AND.K4.EQ.1) METHOD=103
          I4=MAX(I4,K4)+1
          AVAL='/'
          IF(I4.EQ.2) AVAL='//'
          IF(I4.GT.2) AVAL='/'//NUMS(I4:I4)//'/'
          IF(I4.GT.9) STOP 6
          I4=LEN_TRIM(BNAME)
          CWK=ANAME(1:LEN_TRIM(ANAME))//
     &    AVAL(1:LEN_TRIM(AVAL))//BNAME(1:I4)
          NTYPE=3
          NDATE=0
          NLOC=0
        ENDIF
      ELSE
        CALL CUTCROSS(CWK,BNAME,ANAME,I4,IER)
        METHOD=106
        IF(IER.EQ.4) THEN
          METHOD=107
          I4=0
        ENDIF
        IF(I4.EQ.1) METHOD=101
        IF(I4.GT.2) THEN
          K4=INDEX(BNAME,'/')
          J4=INDEX(ANAME,'/')
          IF(J4.EQ.0.AND.K4.GT.0) METHOD=102
          IF(J4.GT.0.AND.K4.EQ.0) METHOD=102
          IF(J4.GT.0.AND.K4.GT.0) METHOD=103
        ENDIF
        NTYPE=NAMDAT%TYPE
        NMLOC=NAMDAT%LOCATION
        NDATE=NAMDAT%DATE
        NREF=NAMDAT%REFERENCE
      ENDIF
      ANAME=CWK
      NGPID1=0
      CALL NAMLST(2,CWK,NGPID1,1,0)
      IF(NGPID1.EQ.IGID) NGPID1=0
      IF(NGPID1.EQ.0) THEN
        CALL ADDGERM(METHOD,2,GERM%PROGENITOR_ID1,GERM%PROGENITOR_ID2,
     &    0,GERM%LOCATION,GERM%DATE,GERM%REFERENCE,
     &    NTYPE,NMLOC,NDATE,NREF,ANAME,GERMN,IWK,IER)
          NGPID1=GERMN%GERMPLASM_ID
      ENDIF
      CALL GETDATM(RDATE,RTIME,KDATE,KTIME)
      CWK='.SET GPID2=0'
      CALL SETGERM(CWK,IGID,GERM,RDATE,RTIME,IER)
      IF(IER.NE.1) RETURN
      WRITE(CWK,'(".SET GPID1=",I12)') NGPID1
      CALL SETGERM(CWK,IGID,GERM,RDATE,RTIME,IER)
      IF(IER.NE.1) RETURN
      CWK='.SET METHN=31'
      CALL SETGERM(CWK,IGID,GERM,RDATE,RTIME,IER)
      IF(IER.NE.1) RETURN
      RETURN
      END
C******************************************************* SETGERM
C  PURPOSE:
C  SET GERMPLASM PARAMETERS
C
C  ARGUMENTS:
C  LINE - ENTERS WITH THE COMMAND STRING:
C            .SET GPID1=NNNNN IN NNNNN
C         OR .SET GPID2=NNNNN IN NNNNN
C         OR .SET GDATE=YYYYMMDD IN NNNNN
C                 GLOCN
C                 GREFN
C                 METHN
C                 MGID
C         THE LAST PART 'IN NNNNN' IS OPTIONAL
C  IGID - ENTERS WITH THE TARGET GID IF 'IN NNNNN' IS MISSING
C         ON THE COMMAND LINE
C  GERM - ENTERS WITH THE GERMPLASM DATA FOR THE TARGET GERMPLASM
C         IF 'IN NNNNN' IS MISSING ON THE COMMAND LINE
C  RDATE- ENTERS WITH THE PRIMARY DATE IF SEVERAL CALLS TO
C         SETGERM ARE TO BE GROUPED ELSE BLANK
C  RTIME- ENTERS WITH THE PRIMARY TIME IF SEVERAL CALLS TO
C         SETGERM ARE TO BE GROUPED ELSE BLANK
C  IER  - RETURNS AS 0 IF THERE WAS AN ERROR BUT NO DATA
C         WAS WRITTEN, -1 IF THERE WAS AN ERROR AND DATA
C         WAS WRITTEN AND 1 OTHERWISE
C---- 10/04/99 ----------------------------------------- SETGERM
      SUBROUTINE SETGERM(LINE,IGID,GERM,RDATE,RTIME,IER)
      USE GMS_STRUCTURES
      USE GMS_INTERFACE
      USE GMS_FUNCTIONS
      USE GMS_TOOLS
      PARAMETER (MAXTREE=30000,MAXLIST=3000)
      CHARACTER RDATE*10,RTIME*10,LINE*80,CWK*80,AVAL*8000,CTMP*12
      CHARACTER LDATE*10,LTIME*10,VIEWER*60,GVIZPATH*60
      CHARACTER LCNM(400)*3,MCD(200)*8,UDFCD(600)*8,NULL*1,BELL*1
      INTEGER*4 IWK(20),DEPNDS(MAXLIST,2),UDFNO(600),LCN(400),MIDS(200)
      TYPE (GMS_GERMPLASM) GERM,GERMD
      TYPE (GMS_NAMEDATA) NAMDAT
      TYPE (GMS_METHOD) METHOD
      TYPE (GMS_LOCATION) LOCN
      TYPE (GMS_CHANGES) CHANGE
      EQUIVALENCE(IWK(1),CWK)
      COMMON /CODES/ NUDF,UDFNO,UDFCD,NCNTRY,LCN,LCNM,NMTHD,MIDS,MCD
      COMMON /WRK/ NDPNDS,MXLST,DEPNDS,AVAL
      COMMON /OPTIONS/ NUID,NLOC,NREF,IOPT,LIST,ISRH,MGAT,NAMP,NTCX
     &,IBTYPE,LDST,IYAL,NMSTD,VIEWER,GVIZPATH
      DATA NULL,BELL/0,7/
      BELL=G_BELL
      LDATE=RDATE
      LTIME=RTIME
      IF(LDATE.EQ.' ') CALL GETDATM(LDATE,LTIME,I,L)
      IER=0
      CWK=LINE
      WRITE(0,111) CWK(1:LEN_TRIM(CWK))
 111  FORMAT(1X,70(1H-)/1X,A)
      CALL CCASE(CWK,1,LEN_TRIM(CWK),1)
      IF(CWK(1:5).NE.'.SET ') RETURN
      L=INDEX(CWK,'=')+1
      KGID=INUMBA(IWK,L,80,J,K)
C IF THE COMMAND LINE DOES NOT HAVE A NUMBER AFTER POSITION 12
C THEN CHECK IF IT IS A LOCATION CODE OTHERWISE RETURN
      IF(K.GT.0) THEN
      L=16
      IF(CWK(1:11).NE.'.SET GLOCN=') RETURN
C LOCATION GIVEN AS TEXT-----------------------------------------
      IF(CWK(15:15).EQ.' ') THEN
      DO 14 K=1,NCNTRY
      KGID=LCN(K)
      IF(CWK(12:14).EQ.LCNM(K)) GOTO 15
 14   CONTINUE
      ENDIF
      I=MAX(11,INDEX(CWK,' IN ')-1)
      L=I+2
      IF(I.EQ.11) I=LEN_TRIM(CWK)
      LOCN%LOCATION_ID=0
      LOCN%TYPE=0
      LOCN%NLLP=0
      LOCN%PREFERRED_NAME=LINE(12:I)//NULL
      LOCN%ABBREVIATION=NULL
      I4=GMS_findLocation(LOCN,0)
      IF(I4.NE.1) RETURN
      KGID=LOCN%LOCATION_ID
      I4=GMS_findLocation(LOCN,1)
      IF(I4.EQ.1) RETURN
      ENDIF
 15   L=L+2
      K=0
      IF(CWK(L-3:L).EQ.' IN ') THEN
      IGID=INUMBA(IWK,L,80,J,K)
      IF(K.GT.0.OR.IGID.EQ.0) RETURN
      CALL WRTGERM(IGID,GERM,3)
      ENDIF
      IF(IGID.EQ.0) RETURN
C GPID2 ----------------------------------------------------------
      IF(CWK(1:11).EQ.'.SET GPID2=') THEN
      CALL SETGPID2(KGID,IGID,GERM,IER,LDATE,LTIME,LINE)
      IF(IER.NE.1) RETURN
      ENDIF
C GPID1 ----------------------------------------------------------
      IF(CWK(1:11).EQ.'.SET GPID1=') THEN
      CALL SETGPID1(KGID,IGID,GERM,IER,LDATE,LTIME,LINE)
      IF(IER.NE.1) RETURN
      ENDIF
C MGID ----------------------------------------------------------
      IF(CWK(1:10).EQ.'.SET MGID=') THEN
      CALL SETMGID(KGID,IGID,GERM,IER,LDATE,LTIME,LINE)
      IF(IER.NE.1) RETURN
      ENDIF      
C DATE -----------------------------------------------------------
      IF(CWK(1:11).EQ.'.SET GDATE=') THEN
      IF(KGID.LT.9999) KGID=KGID*10000
      I4=GMS_expandDate(KGID,IY,IM,ID)
      IF(KGID.NE.0) I4=GMS_validDate(IY,IM,ID)
      IF(I4.NE.1) RETURN
      IER=-1
      CALL ZEROCHANGE(CHANGE)
      CHANGE%ctable='GERMPLSM'C
      CHANGE%cfield='GDATE'C
      CHANGE%cgroup=LDATE//':'//LTIME(1:8)//NULL
      CHANGE%crecord=IGID
      CHANGE%cfrom=GERM%DATE
      CHANGE%cto=KGID
      CHANGE%cref=NREF
      I4=GMS_addChanges(CHANGE,FTOCSTR(TRIM(LINE)//' SETGERM'))
      IF(I4.NE.1) RETURN
C IF THE GERMPLASM IS LOCAL MAKE CHANGE NOW
      IF(IGID.LT.0) THEN
      GERM%DATE=KGID
      I4=GMS_setGermplasm(GERM)
      IF(I4.NE.1) RETURN
      ENDIF
      ENDIF
C  LOCATION GIVEN AS NUMBER ----------------------------------------------
      IF(CWK(1:11).EQ.'.SET GLOCN=') THEN
      IER=-1
      CALL ZEROCHANGE(CHANGE)
      CHANGE%ctable='GERMPLSM'C
      CHANGE%cfield='GLOCN'C
      CHANGE%cgroup=LDATE//':'//LTIME(1:8)//NULL
      CHANGE%crecord=IGID
      CHANGE%cfrom=GERM%LOCATION
      CHANGE%cto=KGID
      CHANGE%cref=NREF
      I4=GMS_addChanges(CHANGE,FTOCSTR(TRIM(LINE)//' SETGERM'))
      IF(I4.NE.1) RETURN
C IF THE GERMPLASM IS LOCAL MAKE CHANGE NOW
      IF(IGID.LT.0) THEN
      GERM%LOCATION=KGID
      I4=GMS_setGermplasm(GERM)
      IF(I4.NE.1) RETURN
      ENDIF
      ENDIF
C  REFERENCE -------------------------------------------------
      IF(CWK(1:11).EQ.'.SET GREFN=') THEN
      IER=-1
      CALL ZEROCHANGE(CHANGE)
      CHANGE%ctable='GERMPLSM'C
      CHANGE%cfield='GREF'C
      CHANGE%cgroup=LDATE//':'//LTIME(1:8)//NULL
      CHANGE%crecord=IGID
      CHANGE%cfrom=GERM%REFERENCE
      CHANGE%cto=KGID
      CHANGE%cref=NREF
      I4=GMS_addChanges(CHANGE,FTOCSTR(TRIM(LINE)//' SETGERM'))
      IF(I4.NE.1) RETURN
C IF THE GERMPLASM IS LOCAL MAKE CHANGE NOW
      IF(IGID.LT.0) THEN
      GERM%REFERENCE=KGID
      I4=GMS_setGermplasm(GERM)
      IF(I4.NE.1) RETURN
      ENDIF
      ENDIF
C  GNPGS -------------------------------------------------
      IF(CWK(1:11).EQ.'.SET GNPGS=') THEN
      METHOD%METHOD_ID=GERM%METHOD
      I4=GMS_getMethod(METHOD,AVAL,3296,0)
      IF(I4.NE.1) RETURN
      IF(KGID.LT.0.AND.METHOD%NO_PROGENITORS.GT.0) RETURN
      IF(KGID.GE.0.AND.METHOD%NO_PROGENITORS.EQ.-1) RETURN
      IF(KGID.GE.0.AND.KGID.LT.GERM%NO_PROGENITORS) THEN
        DO 5 I=KGID+1,GERM%NO_PROGENITORS
        I4=GMS_getProgenitorID(IGID,I,K)
        IF(K.NE.0) THEN
        WRITE(0,132) BELL,I,K
 132    FORMAT(A,
     &' DO YOU WANT TO CHANGE PROGENITOR',I3,' FROM',I9,' TO 0. (Y/N)?')
        READ(1,121) CTMP
        IF(CTMP.NE.' '.AND.CTMP.NE.'Y'.AND.CTMP.NE.'y') RETURN
        IER=-1
         WRITE(CTMP,'("GPID",I1)') I
         IF(I.GE.10) WRITE(CTMP,'("GPID",I2)') I
         IF(I.GE.100) WRITE(CTMP,'("GPID",I3)') I
         CALL ZEROCHANGE(CHANGE)
         CHANGE%ctable='GERMPLSM'C
         CHANGE%cfield=FTOCSTR(CTMP)
         CHANGE%cgroup=LDATE//':'//LTIME(1:8)//NULL
         CHANGE%crecord=IGID
         CHANGE%cfrom=K
         CHANGE%cto=0
         CHANGE%cref=NREF         
         I4=GMS_addChanges(CHANGE,FTOCSTR(TRIM(LINE)//' SETGERM'))
         IF(I4.NE.1) RETURN
         IF(I.EQ.1) GERM%PROGENITOR_ID1=0
         IF(I.EQ.2) GERM%PROGENITOR_ID2=0
         IF(IGID.LT.0) THEN
         I4=GMS_setGermplasm(GERM)
         IF(I4.NE.1) RETURN
         ENDIF
         ENDIF
 5     CONTINUE
       ENDIF
       IER=-1
      CALL ZEROCHANGE(CHANGE)
      CHANGE%ctable='GERMPLSM'C
      CHANGE%cfield='GNPGS'C
      CHANGE%cgroup=LDATE//':'//LTIME(1:8)//NULL
      CHANGE%crecord=IGID
      CHANGE%cfrom=GERM%NO_PROGENITORS
      CHANGE%cto=KGID
      CHANGE%cref=NREF      
      I4=GMS_addChanges(CHANGE,FTOCSTR(TRIM(LINE)//' SETGERM'))
      IF(I4.NE.1) RETURN
C IF THE GERMPLASM IS LOCAL MAKE CHANGE NOW
      IF(IGID.LT.0) THEN
      GERM%NO_PROGENITORS=KGID
      I4=GMS_setGermplasm(GERM)
      IF(I4.NE.1) RETURN
      ENDIF
      ENDIF
C METHOD---------------------------------------------------------
      IF(CWK(1:11).EQ.'.SET METHN=') THEN
      METHOD%METHOD_ID=KGID
      I4=GMS_getMethod(METHOD,AVAL,3296,0)
      IF(I4.NE.1) RETURN
      IER=-1
      CALL ZEROCHANGE(CHANGE)
      CHANGE%ctable='GERMPLSM'C
      CHANGE%cfield='METHN'C
      CHANGE%cgroup=LDATE//':'//LTIME(1:8)//NULL
      CHANGE%crecord=IGID
      CHANGE%cfrom=GERM%METHOD
      CHANGE%cto=KGID
      CHANGE%cref=NREF
      I4=GMS_addChanges(CHANGE,FTOCSTR(TRIM(LINE)//' SETGERM'))
      IF(I4.NE.1) RETURN
C
      NGNPGS=METHOD%NO_PROGENITORS
C FOR GEN/DER METHOD CHANGES CHANGE GNPGS AND GROUP OF
C ALL DERIVATIVES OF IGID
      IF(NGNPGS*GERM%NO_PROGENITORS.LT.0) THEN
      IER=-1
      CALL ZEROCHANGE(CHANGE)
      CHANGE%ctable='GERMPLSM'C
      CHANGE%cfield='GNPGS'C
      CHANGE%cgroup=LDATE//':'//LTIME(1:8)//NULL
      CHANGE%crecord=IGID
      CHANGE%cfrom=GERM%NO_PROGENITORS
      CHANGE%cto=NGNPGS
      CHANGE%cref=NREF      
      I4=GMS_addChanges(CHANGE,FTOCSTR(TRIM(LINE)//' SETGERM'))
      IF(I4.NE.1) RETURN
      NGPID1=IGID
      IF(NGNPGS.EQ.-1) NGPID1=GERM%PROGENITOR_ID1
      IF(NGPID1.NE.0) THEN
C  DISPLAY DEPENDENTS NDPNDS=NO OF DEPENDENTS
      CALL DDPNDS(IGID,GERM,-1,2000,NDPNDS,DEPNDS(1,1),DEPNDS(1,2),JER)
      IF(JER.NE.1) RETURN
      IF(NDPNDS.GT.0) THEN
      WRITE(0,102) BELL,NGPID1
 102  FORMAT(' DO YOU WANT TO CHANGE THE GROUP IN THESE ',
     &'DEPENDANTS TO',A1,I9,' (Y/N)?')
      READ(1,121) CTMP
 121  FORMAT(A)
      IF(CTMP.NE.' '.AND.CTMP.NE.'Y'.AND.CTMP.NE.'y') RETURN
      DO 4 K=1,NDPNDS
      IER=-1
C      CALL GETDATM(RDATE,RTIME,IDTE,ITME)
      I4=GMS_getGermplasm2(DEPNDS(K,1),GERMD,NAMDAT,AVAL,256)
      IF(I4.NE.1) RETURN
      IF(NGPID1.EQ.DEPNDS(K,1)) THEN
      WRITE(0,108) BELL,DEPNDS(K,1),1,GERMD%PROGENITOR_ID1,NGPID1
 108  FORMAT(' *** ERROR ***',A1,
     &' THE FOLLOWING CHANGE WILL CREATE A CIRCULARITY:'/
     &' IN',I12,' CHANGE PROGENITOR_ID',I1,' FROM',I12,' TO',I12)
      RETURN
      ENDIF
      IF(GERMD%PROGENITOR_ID1.NE.NGPID1) THEN
C      WRITE(AVAL,113) RTIME,GERMD%PROGENITOR_ID1,NGPID1,
C     &'PROGENITOR_ID1     ',JDATE,JTIME
      CALL ZEROCHANGE(CHANGE)
      CHANGE%ctable='GERMPLSM'C
      CHANGE%cfield='GPID1'C
      CHANGE%cgroup=LDATE//':'//LTIME(1:8)//NULL
      CHANGE%crecord=GERMD%GERMPLASM_ID
      CHANGE%cfrom=GERMD%PROGENITOR_ID1
      CHANGE%cto=NGPID1
      CHANGE%cref=NREF
      I4=GMS_addChanges(CHANGE,FTOCSTR(TRIM(LINE)//' SETGERM'))
      IF(I4.NE.1) RETURN
C IF THE GERMPLASM IS LOCAL MAKE CHANGE NOW
      IF(GERMD%GERMPLASM_ID.LT.0) THEN
      GERMD%PROGENITOR_ID1=NGPID1
      I4=GMS_setGermplasm(GERMD)
      IF(I4.NE.1) RETURN
      ENDIF
      ENDIF
 4    CONTINUE
      ENDIF
      ENDIF
      ENDIF
C IF THE GERMPLASM IS LOCAL MAKE CHANGE NOW
      IF(IGID.LT.0) THEN
      GERM%METHOD=KGID
      GERM%NO_PROGENITORS=NGNPGS
      I4=GMS_setGermplasm(GERM)
      IF(I4.NE.1) RETURN
      ENDIF
      ENDIF
      I4=GMS_getGermplasm2(IGID,GERM,NAMDAT,AVAL,256)
      IF(I4.NE.1) RETURN
      IER=1
      RETURN
      END
C******************************************************* SETGPID1
C  PURPOSE:
C  SET PROGENITOR_ID1=NGID IN GERMPLASM IGID
C
C  NOTES:
C  IF IGID IS DERIVATIVE WE NEED TO CHANGE THE GROUP OF ITS
C  SOURCE AND ITS DEPENDANTS (DIRECT DERIVATIVES AND DERIVATIVES
C  IN THE SAME GROUP WITH DEVELOPMENT PATHS TRACING THROUGH IGID)
C
C  ARGUMENTS:
C  NGID - NEW GPID1 VALUE
C  IGID - GID OF TARGET GERMPLASM
C  GERM - ENTERS WITH THE GERMPLASM DATA FOR THE TARGET GERMPLASM
C  RDATE- DATE OF PRIMARY CHANGE IF THIS IS A SECONDARY CHANGE ELSE 0
C  RTIME- TIME OF PRIMARY CHANGE IF THIS IS A SECONDARY CHANGE ELSE 0
C  IER  - RETURNS AS 0 IF THERE WAS AN ERROR AND 1 OTHERWISE
C---- 22/10/98 ----------------------------------------- SETGPID1
      SUBROUTINE SETGPID1(NGID,IGID,GERM,IER,RDATE,RTIME,REASON)
      USE GMS_STRUCTURES
      USE GMS_INTERFACE
      USE GMS_FUNCTIONS
      USE GMS_TOOLS
      PARAMETER (MAXTREE=30000,MAXLIST=3000)
      CHARACTER RDATE*10,RTIME*10,LDATE*10,LTIME*10,CWK*80
      CHARACTER AVAL*8000,CTMP*1,REASON*(*),BELL*1,NULL*1,VIEWER*60
      CHARACTER GVIZPATH*60      
      INTEGER*4 IWK(20),DGID,DEPNDS(MAXLIST,2)
      TYPE (GMS_GERMPLASM) GERM,GERMD
      TYPE (GMS_NAMEDATA) NAMDAT
      TYPE (GMS_CHANGES) CHANGE
      EQUIVALENCE(IWK(1),CWK)
      COMMON /WRK/ NDPNDS,MXLST,DEPNDS,AVAL
      COMMON /OPTIONS/ NUID,NLOC,NREF,IOPT,LIST,ISRH,MGAT,NAMP,NTCX
     &,IBTYPE,LDST,IYAL,NMSTD,VIEWER,GVIZPATH
      DATA BELL/7/,NULL/0/
      BELL=G_BELL
      IER=0
      IF(IGID.EQ.0) RETURN
      LDATE=RDATE
      LTIME=RTIME
      IF(LDATE.EQ.' ') CALL GETDATM(LDATE,LTIME,I,I4)      
      IF(NGID.EQ.IGID) THEN
      WRITE(0,*) ' *** ERROR *** ILLEGAL SELF REFERENCE'
      WRITE(3,*) ' *** ERROR *** ILLEGAL SELF REFERENCE'      
      RETURN
      ENDIF
      IER=-1
      CALL ZEROCHANGE(CHANGE)
      CHANGE%ctable='GERMPLSM'C
      CHANGE%cfield='GPID1'C
      CHANGE%cgroup=LDATE//':'//LTIME(1:8)//NULL
      CHANGE%crecord=IGID
      CHANGE%cfrom=GERM%PROGENITOR_ID1
      CHANGE%cto=NGID
      CHANGE%cref=NREF
      I4=GMS_addChanges(CHANGE,FTOCSTR(TRIM(REASON)//' SETGPID1'))
      IF(I4.NE.1) RETURN
C
      NSOURCE=GERM%PROGENITOR_ID2
C  IF THE TARGET GERMPLASM IS GENERATIVE MAKE THE CHANGE
      IF(GERM%NO_PROGENITORS.GE.0) THEN
      GERM%PROGENITOR_ID1=NGID
      IF(IGID.LT.0) THEN
      I4=GMS_setGermplasm(GERM)
      IF(I4.NE.1) RETURN
      ENDIF
C  ELSE IF TARGET GERMPLASM IS DERIVATIVE CHECK ITS SOURCE AND DERIVATIVES
      ELSE
C  CHECK GROUP
      NGROUP=IGID
      IF(NGID.NE.0) THEN
      I4=GMS_getGermplasm2(NGID,GERMD,NAMDAT,AVAL,256)
      IF(I4.NE.1) RETURN
      NGROUP=GROUPID(GERMD)
      IF(NGROUP.NE.NGID) THEN 
      WRITE(0,100) BELL
      WRITE(3,100) ' '
 100  FORMAT(' *** ERROR *** CHANGE WILL FORM AN ILLEGAL GROUP',A1)
      RETURN      
      ENDIF
      ENDIF
C  CHECK SOURCE
      IF(NSOURCE.NE.0) THEN
      I4=GMS_getGermplasm2(NSOURCE,GERMD,NAMDAT,AVAL,256)
      IF(I4.NE.1) RETURN
      I4=GROUPID(GERMD)
      IF(I4.NE.NGID) THEN
      WRITE(0,103) BELL,GERMD%PROGENITOR_ID1
 103  FORMAT(' SOURCE HAS DIFFERENT GROUP (',A1,I9,')'/
     & ' DO YOU WANT TO SET THE SOURCE TO MISSING (Y/N)?')
      READ(1,121) CTMP
 121  FORMAT(A)
      IF(CTMP.NE.' '.AND.CTMP.NE.'Y'.AND.CTMP.NE.'y') RETURN      
C WRITE THE CHANGE RECORD
      IER=-1
      NSOURCE=0
      CALL ZEROCHANGE(CHANGE)
      CHANGE%ctable='GERMPLSM'C
      CHANGE%cfield='GPID2'C
      CHANGE%cgroup=LDATE//':'//LTIME(1:8)//NULL
      CHANGE%crecord=IGID
      CHANGE%cfrom=GERM%PROGENITOR_ID2
      CHANGE%cto=NSOURCE
      CHANGE%cref=NREF      
      I4=GMS_addChanges(CHANGE,FTOCSTR(TRIM(REASON)//' SETGPID1'))
      IF(I4.NE.1) RETURN
      ENDIF
      ENDIF
C  CHECK DERIVATIVES
C  DISPLAY DEPENDENTS NDPNDS=NO OF DEPENDENTS
      CALL DDPNDS(IGID,GERM,0,2000,NDPNDS,DEPNDS(1,1),DEPNDS(1,2),JER)
      IF(JER.NE.1) RETURN
C  MAKE THE CHANGE IN THE TARGET
      GERM%PROGENITOR_ID1=NGID
      GERM%PROGENITOR_ID2=NSOURCE           
      IF(IGID.LT.0) THEN
      I4=GMS_setGermplasm(GERM)
      IF(I4.NE.1) RETURN
      ENDIF
C  CHECK FOR CHANGES IN THE DEPENDANTS
      IF(NDPNDS.GT.0) THEN
      WRITE(0,102) BELL,NGROUP
 102  FORMAT(' DO YOU WANT TO CHANGE THE GROUP IN THESE ',
     &'DEPENDANTS TO',A1,I9,' (Y/N)?')
      READ(1,121) CTMP
      IF(CTMP.NE.' '.AND.CTMP.NE.'Y'.AND.CTMP.NE.'y') RETURN
      DO 4 K=1,NDPNDS
C      CALL GETDATM(DATE,TIME,IDTE,ITME)
      L=NGROUP
      I4=GMS_getGermplasm2(DEPNDS(K,1),GERMD,NAMDAT,AVAL,256)
      IF(I4.NE.1) RETURN
      IF(NGROUP.EQ.DEPNDS(K,1)) THEN
      WRITE(0,108) BELL,DEPNDS(K,1),1,GERMD%PROGENITOR_ID1,NGROUP
 108  FORMAT(' *** WARNING ***',A1,
     &' THE FOLLOWING CHANGE WILL CREATE A CIRCULARITY:'/
     &' IN',I12,' CHANGE PROGENITOR_ID',I1,' FROM',I12,' TO',I12/
     &' SET LINK TO ZERO? (Y/N)')
      READ(1,121) CTMP
      IF(CTMP.NE.'Y'.AND.CTMP.NE.'y') RETURN
      L=0
      ENDIF
      IF(GERMD%PROGENITOR_ID1.NE.NGROUP) THEN
      IER=-1
      CALL ZEROCHANGE(CHANGE)
      CHANGE%ctable='GERMPLSM'C
      CHANGE%cfield='GPID1'C
      CHANGE%cgroup=LDATE//':'//LTIME(1:8)//NULL
      CHANGE%crecord=DEPNDS(K,1)
      CHANGE%cfrom=GERMD%PROGENITOR_ID1
      CHANGE%cto=L
      CHANGE%cref=NREF
      I4=GMS_addChanges(CHANGE,FTOCSTR(TRIM(REASON)//' SETGPID1'))
      IF(I4.NE.1) RETURN
C IF THE GERMPLASM IS LOCAL MAKE CHANGE NOW
      IF(DEPNDS(K,1).LT.0) THEN
      GERMD%PROGENITOR_ID1=NGROUP
      I4=GMS_setGermplasm(GERMD)
      IF(I4.NE.1) RETURN
      ENDIF
      ENDIF
 4    CONTINUE
      ENDIF
      ENDIF
      IER=1
      RETURN
      END
C******************************************************* SETMGID
C  PURPOSE:
C  SET MGID=NGID IN GERMPLASM IGID
C
C  NOTES:
C
C  ARGUMENTS:
C  NGID - NEW MGID VALUE
C  IGID - GID OF TARGET GERMPLASM
C  GERM - ENTERS WITH THE GERMPLASM DATA FOR THE TARGET GERMPLASM
C  RDATE- DATE OF PRIMARY CHANGE IF THIS IS A SECONDARY CHANGE ELSE 0
C  RTIME- TIME OF PRIMARY CHANGE IF THIS IS A SECONDARY CHANGE ELSE 0
C  IER  - RETURNS AS 0 IF THERE WAS AN ERROR AND 1 OTHERWISE
C---- 08/04/11 ----------------------------------------- SETMGID
      SUBROUTINE SETMGID(NGID,IGID,GERM,IER,RDATE,RTIME,REASON)
      USE GMS_STRUCTURES
      USE GMS_INTERFACE
      USE GMS_FUNCTIONS
      USE GMS_TOOLS
      PARAMETER (MAXTREE=30000,MAXLIST=3000)
      CHARACTER RDATE*10,RTIME*10,LDATE*10,LTIME*10,CWK*80
      CHARACTER AVAL*8000,CTMP*1,REASON*(*),BELL*1,NULL*1,VIEWER*60
      CHARACTER GVIZPATH*60      
      INTEGER*4 IWK(20),DGID,DEPNDS(MAXLIST,2)
      TYPE (GMS_GERMPLASM) GERM,GERMD
      TYPE (GMS_NAMEDATA) NAMDAT
      TYPE (GMS_CHANGES) CHANGE
      EQUIVALENCE(IWK(1),CWK)
      COMMON /WRK/ NDPNDS,MXLST,DEPNDS,AVAL
      COMMON /OPTIONS/ NUID,NLOC,NREF,IOPT,LIST,ISRH,MGAT,NAMP,NTCX
     &,IBTYPE,LDST,IYAL,NMSTD,VIEWER,GVIZPATH
      DATA BELL/7/,NULL/0/
      BELL=G_BELL
      IER=0
      IF(IGID.EQ.0) RETURN
      LDATE=RDATE
      LTIME=RTIME
      IF(LDATE.EQ.' ') CALL GETDATM(LDATE,LTIME,I,I4)      
      IER=-1
      CALL ZEROCHANGE(CHANGE)
      CHANGE%ctable='GERMPLSM'C
      CHANGE%cfield='MGID'C
      CHANGE%cgroup=LDATE//':'//LTIME(1:8)//NULL
      CHANGE%crecord=IGID
      CHANGE%cfrom=GERM%MGID
      CHANGE%cto=NGID
      CHANGE%cref=NREF
      I4=GMS_addChanges(CHANGE,FTOCSTR(TRIM(REASON)//' SETMGID'))
      IF(I4.NE.1) RETURN
C
c      NSOURCE=GERM%PROGENITOR_ID2
C  IF THE TARGET GERMPLASM IS GENERATIVE MAKE THE CHANGE
c      IF(GERM%NO_PROGENITORS.GE.0) THEN
      GERM%MGID=NGID
      IF(IGID.LT.0) THEN
      I4=GMS_setGermplasm(GERM)
      IF(I4.NE.1) RETURN
      ENDIF
      IER=1
      RETURN
      END      
C******************************************************* SETGPID2
C  PURPOSE:
C  SET PROGENITOR_ID2=NSOURCE IN GERMPLASM IGID
C
C  NOTES:
C  IF IGID IS DERIVATIVE AND NSOURCE IS IN A DIFFERENT GROUP
C  THEN CHANGE THE GROUP OF IGID AS WELL AS THE GROUP OF ANY
C  DEPENDANTS (DIRECT DERIVATIVES AND DERIVATIVES IN THE SAME
C  GROUP WITH DEVELOPMENT PATHS TRACING THROUGH IGID)
C
C  ARGUMENTS:
C  NSOURCE = NEW VALUE OF GPID2
C  IGID - GID OF TARGET GERMPLASM
C  GERM - ENTERS WITH THE GERMPLASM DATA FOR THE TARGET GERMPLASM
C  IER  - RETURNS AS 0 IF THERE WAS AN ERROR AND 1 OTHERWISE
C  RDATE- DATE OF PRIMARY CHANGE IF THIS IS A SECONDARY CHANGE ELSE 0
C  RTIME- TIME OF PRIMARY CHANGE IF THIS IS A SECONDARY CHANGE ELSE 0
C---- 20/10/07 ----------------------------------------- SETGPID2
      SUBROUTINE SETGPID2(NSOURCE,IGID,GERM,IER,RDATE,RTIME,REASON)
      USE GMS_STRUCTURES
      USE GMS_INTERFACE
      USE GMS_FUNCTIONS
      USE GMS_TOOLS
      PARAMETER (MAXTREE=30000,MAXLIST=3000)
      CHARACTER RDATE*10,RTIME*10,LDATE*10,LTIME*10,CWK*80
      CHARACTER AVAL*8000,CTMP*1,REASON*(*),BELL*1,NULL*1,VIEWER*60
      CHARACTER GVIZPATH*60      
      INTEGER*4 IWK(20),DGID,DEPNDS(MAXLIST,2)
      TYPE (GMS_GERMPLASM) GERM,GERMD
      TYPE (GMS_NAMEDATA) NAMDAT
      TYPE (GMS_CHANGES) CHANGE
      EQUIVALENCE(IWK(1),CWK)
      COMMON /WRK/ NDPNDS,MXLST,DEPNDS,AVAL
      COMMON /OPTIONS/ NUID,NLOC,NREF,IOPT,LIST,ISRH,MGAT,NAMP,NTCX
     &,IBTYPE,LDST,IYAL,NMSTD,VIEWER,GVIZPATH
      DATA BELL /7/,NULL/0/
      BELL=G_BELL
      IER=0
      IF(IGID.EQ.0) RETURN
      LDATE=RDATE
      LTIME=RTIME
      IF(LDATE.EQ.' ') CALL GETDATM(LDATE,LTIME,I,I4)
      IF(NSOURCE.EQ.IGID) THEN
      WRITE(0,*) ' *** ERROR *** ILLEGAL SELF REFERENCE'
      WRITE(3,*) ' *** ERROR *** ILLEGAL SELF REFERENCE'      
      RETURN
      ENDIF
C GET GERMPLASM record for the new source
      NGROUP=0
      IF(NSOURCE.NE.0) THEN
      I4=GMS_getGermplasm2(NSOURCE,GERMD,NAMDAT,AVAL,256)
      IF(I4.NE.1) RETURN
C CHECK FOR CIRCULARITY
      IF(GERMD%PROGENITOR_ID2.EQ.IGID) THEN
      WRITE(0,*) ' *** ERROR *** CHANGE WILL CREATE A CIRCULARITY'
      WRITE(3,*) ' *** ERROR *** CHANGE WILL CREATE A CIRCULARITY'      
      RETURN
      ENDIF      
C GET THE GROUP OF THE NEW SOURCE GERMPLASM      
      NGROUP=GROUPID(GERMD)
      ENDIF
C WRITE THE CHANGE ATTRIBUTE
      IER=-1
      CALL ZEROCHANGE(CHANGE)
      CHANGE%ctable='GERMPLSM'C
      CHANGE%cfield='GPID2'C
      CHANGE%cgroup=LDATE//':'//LTIME(1:8)//NULL
      CHANGE%crecord=IGID
      CHANGE%cfrom=GERM%PROGENITOR_ID2
      CHANGE%cto=NSOURCE
      CHANGE%cref=NREF
      I4=GMS_addChanges(CHANGE,FTOCSTR(TRIM(REASON)//' SETGPID2'))
      IF(I4.NE.1) RETURN
C  IF THE TARGET GERMPLASM IS DERIVATIVE CHECK GROUPS
      IF(GERM%NO_PROGENITORS.EQ.-1.AND.
     &   NGROUP.NE.GERM%PROGENITOR_ID1) THEN
      WRITE(0,100) BELL,IGID,GERM%PROGENITOR_ID1,NGROUP
 100  FORMAT(' GERMPLASM',A1,I9,' HAS GROUP',I9,' CHANGE TO',I9,
     & ' (Y/N, X TO ABORT)?')
      READ(1,121) CTMP
 121  FORMAT(A)
      IF(CTMP.EQ.'X'.OR.CTMP.EQ.'x') RETURN
      IF(CTMP.NE.' '.AND.CTMP.NE.'Y'.AND.CTMP.NE.'y') THEN
      IF(NSOURCE.EQ.0) GOTO 5
      WRITE(0,*) " *** ERROR *** CANNOT CREATE ILLEGAL GROUP"
      WRITE(3,*) " *** ERROR *** CANNOT CREATE ILLEGAL GROUP"      
      RETURN
      ENDIF
      NNGROUP=NGROUP
      IF(NNGROUP.EQ.0.AND.NSOURCE.EQ.0) NGROUP=GERM%GERMPLASM_ID
C  DISPLAY DEPENDENTS NDPNDS=NO OF DEPENDENTS
      CALL DDPNDS(IGID,GERM,0,2000,NDPNDS,DEPNDS(1,1),DEPNDS(1,2),JER)
      IF(JER.NE.1) RETURN
      IF(NDPNDS.GT.0) THEN
      WRITE(0,102) BELL,NGROUP
 102  FORMAT(' DO YOU WANT TO CHANGE THE GROUP IN THESE ',
     &'DEPENDANTS TO',A1,I9,' (Y/N)?')
      READ(1,121) CTMP
      IF(CTMP.NE.' '.AND.CTMP.NE.'Y'.AND.CTMP.NE.'y') RETURN
      DO 4 K=1,NDPNDS
      L=NGROUP
      I4=GMS_getGermplasm2(DEPNDS(K,1),GERMD,NAMDAT,AVAL,256)
      IF(I4.NE.1) RETURN
      IF(NGROUP.EQ.DEPNDS(K,1)) THEN
      WRITE(0,108) BELL,DEPNDS(K,1),1,GERMD%PROGENITOR_ID1,NGROUP
 108  FORMAT(' *** WARNING ***',A1,
     &' THE FOLLOWING CHANGE WILL CREATE A CIRCULARITY:'/
     &' IN',I12,' CHANGE PROGENITOR_ID',I1,' FROM',I12,' TO',I12/
     &' SET LINK TO ZERO? (Y/N)')
      READ(1,121) CTMP
      IF(CTMP.NE.'Y'.AND.CTMP.NE.'y') RETURN
      L=0
      ENDIF
      IF(GERMD%PROGENITOR_ID1.NE.L) THEN
      CALL ZEROCHANGE(CHANGE)
      CHANGE%ctable='GERMPLSM'C
      CHANGE%cfield='GPID1'C
      CHANGE%cgroup=LDATE//':'//LTIME(1:8)//NULL
      CHANGE%crecord=DEPNDS(K,1)
      CHANGE%cfrom=GERMD%PROGENITOR_ID1
      CHANGE%cto=L
      CHANGE%cref=NREF
      I4=GMS_addChanges(CHANGE,FTOCSTR(TRIM(REASON)//' SETGPID2'))
      IF(I4.NE.1) RETURN
C IF THE GERMPLASM IS LOCAL MAKE CHANGE NOW
      IF(DEPNDS(K,1).LT.0) THEN
      GERMD%PROGENITOR_ID1=L
      I4=GMS_setGermplasm(GERMD)
      IF(I4.NE.1) RETURN
      ENDIF
      ENDIF
 4    CONTINUE
      ENDIF
C CHANGE THE GROUP IN IGID ITSELF
      IER=-1
      CALL ZEROCHANGE(CHANGE)
      CHANGE%ctable='GERMPLSM'C
      CHANGE%cfield='GPID1'C
      CHANGE%cgroup=LDATE//':'//LTIME(1:8)//NULL
      CHANGE%crecord=IGID
      CHANGE%cfrom=GERM%PROGENITOR_ID1
      CHANGE%cto=NNGROUP
      CHANGE%cref=NREF
      I4=GMS_addChanges(CHANGE,FTOCSTR(TRIM(REASON)//' SETGPID2'))
      IF(I4.NE.1) RETURN
      GERM%PROGENITOR_ID1=NNGROUP
      ENDIF
 5    GERM%PROGENITOR_ID2=NSOURCE
C IF THE GERMPLASM IS LOCAL MAKE CHANGE NOW
      IF(IGID.LT.0) THEN
      I4=GMS_setGermplasm(GERM)
      IF(I4.NE.1) RETURN
      ENDIF
      IER=1
      RETURN
      END
C******************************************************* SETNAM
C  PURPOSE:
C  SET GERMPLASM NMAE PARAMETERS
C
C  ARGUMENTS:
C  LINE - ENTERS WITH THE COMMAND STRING:
C            .SET NTYPE=NNNNN IN NID
C         OR .SET NSTAT=NNNNN IN NID
C         OR .SET NDATE=YYYYMMDD IN NID
C                 NLOCN
C                 NREFN
C         'IN NID' IS OPTIONAL 
C  INID - ENTERS WITH THE TARGET NID IF 'IN NID' IS MISSING
C         ON THE COMMAND LINE
C  NAMDAT- ENTERS WITH THE NAME DATA FOR THE TARGET NID
C         IF 'IN NID' IS MISSING ON THE COMMAND LINE
C  RDATE- ENTERS WITH THE PRIMARY DATE IF SEVERAL CALLS TO
C         SETGERM ARE TO BE GROUPED ELSE BLANK
C  RTIME- ENTERS WITH THE PRIMARY TIME IF SEVERAL CALLS TO
C         SETGERM ARE TO BE GROUPED ELSE BLANK
C  IER  - RETURNS AS 0 IF THERE WAS AN ERROR BUT NO DATA
C         WAS WRITTEN, -1 IF THERE WAS AN ERROR AND DATA
C         WAS WRITTEN AND 1 OTHERWISE
C---- 14/11/03 ----------------------------------------- SETNAM
      SUBROUTINE SETNAM(LINE,INID,NAMDAT,RDATE,RTIME,IER)
      USE GMS_STRUCTURES
      USE GMS_INTERFACE
      USE GMS_FUNCTIONS
      USE GMS_TOOLS
      PARAMETER (MAXTREE=30000,MAXLIST=3000)
      CHARACTER RDATE*10,RTIME*10,LINE*80,CWK*80,AVAL*8000,CTMP*12
      CHARACTER LCNM(400)*3,MCD(200)*8,UDFCD(600)*8,NULL*1,BELL*1
      CHARACTER ANAME*256,LDATE*10,LTIME*10,CGID*12,VIEWER*60
      CHARACTER GVIZPATH*60      
      INTEGER*4 IWK(20),DEPNDS(MAXLIST,2),UDFNO(600),LCN(400),MIDS(200)
      TYPE (GMS_GERMPLASM) GERM,GERMD
      TYPE (GMS_NAMEDATA) NAMDAT,NAMD
      TYPE (GMS_METHOD) METHOD
      TYPE (GMS_LOCATION) LOCN
      TYPE (GMS_CHANGES) CHANGE
      EQUIVALENCE(IWK(1),CWK)
      COMMON /CODES/ NUDF,UDFNO,UDFCD,NCNTRY,LCN,LCNM,NMTHD,MIDS,MCD
      COMMON /WRK/ NDPNDS,MXLST,DEPNDS,AVAL
      COMMON /OPTIONS/ NUID,NLOC,NREF,IOPT,LIST,ISRH,MGAT,NAMP,NTCX
     &,IBTYPE,LDST,IYAL,NMSTD,VIEWER,GVIZPATH
      DATA NULL,BELL/0,7/
      BELL=G_BELL
      IER=0
      LDATE=RDATE
      LTIME=RTIME
      IF(LDATE.EQ.' ') CALL GETDATM(LDATE,LTIME,I,L)      
      CWK=LINE
      WRITE(0,111) CWK(1:LEN_TRIM(CWK))
 111  FORMAT(1X,70(1H-)/1X,A)
      CALL CCASE(CWK,1,LEN_TRIM(CWK),1)
      IF(CWK(1:5).NE.'.SET ') RETURN
      L=12
      KVAL=INUMBA(IWK,L,80,J,K)
C IF THE COMMAND LINE DOES NOT HAVE A NUMBER AFTER POSITION 12 RETURN
      IF(K.GT.0) RETURN
      L=MAX(11,INDEX(CWK,' IN ')-1)+4
      K=0
      IF(CWK(L-3:L).EQ.' IN ') THEN
      INID=INUMBA(IWK,L,80,J,K)
      IF(K.GT.0.OR.INID.EQ.0) RETURN
      ENDIF
      IF(INID.EQ.0) RETURN
        CALL ZERONAMDAT(NAMD)
        NAMD%GERMPLASM=0
        NAMD%NID=INID
        I4=GMS_getNameRecord(NAMD,ANAME,256)
        WRITE(CGID,'("GID=",I8)') NAMD%GERMPLASM
C DATE -----------------------------------------------------------
      IF(CWK(1:11).EQ.'.SET NDATE=') THEN
      IF(KVAL.LT.9999) KVAL=KVAL*10000
      I4=GMS_expandDate(KVAL,IY,IM,ID)
      I4=GMS_validDate(IY,IM,ID)
      IF(I4.NE.1.AND.KVAL.NE.0) RETURN
      IER=-1
      CALL ZEROCHANGE(CHANGE)
      CHANGE%ctable='NAMES'C
      CHANGE%cfield='NDATE'C
      CHANGE%cgroup=LDATE//':'//LTIME(1:8)//NULL
      CHANGE%crecord=INID
      CHANGE%cfrom=NAMD%DATE
      CHANGE%cto=KVAL
      CHANGE%cref=NREF
      I4=GMS_addChanges(CHANGE,FTOCSTR(TRIM(LINE)//' SETNAM '//CGID))
      IF(I4.NE.1) RETURN
C IF THE NAME RECORD IS LOCAL MAKE CHANGE NOW
      IF(INID.LT.0) THEN
      NAMD%DATE=KVAL
      I4=GMS_setName(NAMD,ANAME,256)
      IF(I4.NE.1) RETURN
      ENDIF
      ENDIF
C  LOCATION GIVEN AS NUMBER ----------------------------------------------
      IF(CWK(1:11).EQ.'.SET NLOCN=') THEN
      IER=-1
      CALL ZEROCHANGE(CHANGE)
      CHANGE%ctable='NAMES'C
      CHANGE%cfield='NLOCN'C
      CHANGE%cgroup=LDATE//':'//LTIME(1:8)//NULL
      CHANGE%crecord=INID
      CHANGE%cfrom=NAMD%LOCATION
      CHANGE%cto=KVAL
      CHANGE%cref=NREF
      I4=GMS_addChanges(CHANGE,FTOCSTR(TRIM(LINE)//' SETNAM '//CGID))
      IF(I4.NE.1) RETURN
C IF THE NAME RECORD IS LOCAL MAKE CHANGE NOW
      IF(INID.LT.0) THEN
      NAMD%LOCATION=KVAL
      I4=GMS_setName(NAMD,ANAME,256)
      IF(I4.NE.1) RETURN
      ENDIF
      ENDIF
C  REFERENCE -------------------------------------------------
      IF(CWK(1:11).EQ.'.SET NREFN=') THEN
      IER=-1
      CALL ZEROCHANGE(CHANGE)
      CHANGE%ctable='NAMES'C
      CHANGE%cfield='NREF'C
      CHANGE%cgroup=LDATE//':'//LTIME(1:8)//NULL
      CHANGE%crecord=INID
      CHANGE%cfrom=NAMD%REFERENCE
      CHANGE%cto=KVAL
      CHANGE%cref=NREF
      I4=GMS_addChanges(CHANGE,FTOCSTR(TRIM(LINE)//' SETNAM '//CGID))
      IF(I4.NE.1) RETURN
C IF THE NAME RECORD IS LOCAL MAKE CHANGE NOW
      IF(INID.LT.0) THEN
      NAMD%REFERENCE=KVAL
      I4=GMS_setName(NAMD,ANAME,256)
      IF(I4.NE.1) RETURN
      ENDIF
      ENDIF
C  NAME STATUS -------------------------------------------------
      IF(CWK(1:11).EQ.'.SET NSTAT=') THEN
      IER=-1
      CALL ZEROCHANGE(CHANGE)
      CHANGE%ctable='NAMES'C
      CHANGE%cfield='NSTAT'C
      CHANGE%cgroup=LDATE//':'//LTIME(1:8)//NULL
      CHANGE%crecord=INID
      CHANGE%cfrom=NAMD%STATUS
      CHANGE%cto=KVAL
      CHANGE%cref=NREF
      I4=GMS_addChanges(CHANGE,FTOCSTR(TRIM(LINE)//' SETNAM '//CGID))
      IF(I4.NE.1) RETURN
C IF THE NAME RECORD IS LOCAL MAKE CHANGE NOW
      IF(INID.LT.0) THEN
      NAMD%STATUS=KVAL
      I4=GMS_setName(NAMD,ANAME,256)
      IF(I4.NE.1) RETURN
      ENDIF
      ENDIF
C  NAME TYPE -------------------------------------------------
      IF(CWK(1:11).EQ.'.SET NTYPE=') THEN
      IER=-1
      CALL ZEROCHANGE(CHANGE)
      CHANGE%ctable='NAMES'C
      CHANGE%cfield='NTYPE'C
      CHANGE%cgroup=LDATE//':'//LTIME(1:8)//NULL
      CHANGE%crecord=INID
      CHANGE%cfrom=NAMD%TYPE
      CHANGE%cto=KVAL
      CHANGE%cref=NREF
      I4=GMS_addChanges(CHANGE,FTOCSTR(TRIM(LINE)//' SETNAM '//CGID))
      IF(I4.NE.1) RETURN
C IF THE NAME RECORD IS LOCAL MAKE CHANGE NOW
      IF(INID.LT.0) THEN
      NAMD%TYPE=KVAL
      I4=GMS_setName(NAMD,ANAME,256)
      IF(I4.NE.1) RETURN
      ENDIF
      ENDIF
      IER=1
      NAMDAT=NAMD
      RETURN
      END
C******************************************************* DDPNDS
C  PURPOSE:
C  DISPLAY DEPENDANTS OF GERMPLASM IGID
C
C  NOTES:
C  DEPENDANTS OF GERMPLSM IGID INCLUDE OFFSPRING, DIRECT
C  DERIVATIVES AND GROUP DEPENDANTS WHICH ARE DERIVATIVES
C  IN THE SAME GROUP AS IGID WITH DEVELOPMENT PATHS TRACING
C  THROUGH IGID
C
C  ARGUMENTS:
C  IGID - GID OF TARGET GERMPLASM
C  GERM - GERMPLASM DATA OF TARGET GERMPLASM
C  CASE - CASE=-1 LIST GRPUP DEPENDANTS ONLY
C         CASE=0 TO LIST DIRECT DERIVATIVES AND GROUP DEPENDANTS
C         CASE=1,2,3 TO LIST OFFSPRING AND DIRECT DERIVATIVES
C         CASE=4 FOR OFFSPRING, DERIVATIVES AND GROUP DEPENDANTS
C  MXDP - MAXIMUM DEPENDANTS ALLOWED (DIMENTION OF DGID AND DNPR)
C  NDPNDS NUMBER OF DEPENDANTS RETURNED
C  DGIDS- ARRAY OF DEPENDANT GIDS
C  DNPRS- ARRAY OF DEPENDANT LINKS (PROGENITOR NUO FOR OFFSPRING,
C         2 FOR DIRECT DERIVATIVES AND 1 FOR GROUP DEPENDANTS)
C  IER  - ERROR RETURN (O FOR ERROR, 1 FOR SUCCESS)
C---- 22/02/99 ----------------------------------------- DDPNDS
      SUBROUTINE DDPNDS(IGID,GERM,CASE,MXDP,NDPNDS,DGIDS,DNPRS,IER)
      USE GMS_STRUCTURES
      USE GMS_INTERFACE
      USE GMS_FUNCTIONS
      USE GMS_TOOLS
      CHARACTER AVAL*256
      INTEGER*4 CASE,DGID,DGIDS(*),DNPRS(*)
      TYPE (GMS_GERMPLASM) GERM,GERMD
      TYPE (GMS_NAMEDATA) NAMDAT
C  GET THE GROUP OF IGID
      NGROUP=GROUPID(GERM)
C  DISPLAY DEPENDENTS NDPNDS=NO OF DEPENDENTS
      IER=0
      NDPNDS=0
      I4=GMS_findDescendant(IGID,GERMD,NPRG,0)
      DO WHILE (I4.EQ.1)
      IF(GERMD%REPLACE.NE.0.OR.
     & (GERMD%NO_PROGENITORS.GE.0.AND.CASE.LE.0)) GOTO 3
      IF(NPRG.NE.1.AND.CASE.EQ.-1) GOTO 3
      DGID=GERMD%GERMPLASM_ID
      IF(NDPNDS.EQ.0) WRITE(0,103) IGID
 103  FORMAT(' GID',I9,' HAS THE FOLLOWING DEPENDENCIES'/
     & ' NO NAMES',40X,'GID  METHOD  GPID1  GPID2 NPRG')
	DO I=1,NDPNDS
	IF(DGIDS(I).EQ.DGID.AND.DNPRS(I).EQ.-1) GOTO 3
	ENDDO
      NDPNDS=NDPNDS+1
      IF(NDPNDS.GT.MXDP) THEN
      PAUSE 'TOO MANY DEPENDENCIES'
      RETURN
      ENDIF
      DGIDS(NDPNDS)=DGID
      DNPRS(NDPNDS)=NPRG
C FLAG DIRECT DEPENDANTS TO FOLLOW PATHS (SET DNPRS(NDPNDS)=0)
      IF(CASE.EQ.0.OR.CASE.EQ.4.AND.
     &(GERMD%NO_PROGENITORS.EQ.-1.AND.NPRG.EQ.2)) DNPRS(NDPNDS)=0
      WRITE(0,101) NDPNDS,LSTNMS(DGID,41),DGID,GERMD%METHOD,
     & GERMD%PROGENITOR_ID1,GERMD%PROGENITOR_ID2,NPRG
 101  FORMAT(1X,I3,1X,A40,I9,I4,2I9,I3)
 3    I4=GMS_findDescendant(IGID,GERMD,NPRG,1)
      END DO
C  DISPLAY DEPENDANTS WITH DERIVATIVE HISTORIES PASSING THROUGH IGID
      K=1
      DO WHILE (K.LE.NDPNDS)
      IF(DNPRS(K).GT.0) GOTO 10
C RE-SET DEPENDANT LOCATION AFTER FOLLOWING PATHS
      DNPRS(K)=DNPRS(K)+2
      I4=GMS_findDescendant(DGIDS(K),GERMD,NPRG,0)
      DO WHILE (I4.EQ.1)
      IF(GERMD%REPLACE.NE.0.OR.GERMD%NO_PROGENITORS.GT.0) GOTO 11
      IF(GERMD%GERMPLASM_ID.EQ.IGID) GOTO 11
      DGID=GERMD%GERMPLASM_ID
	DO I=1,NDPNDS
	IF(DGIDS(I).EQ.DGID.AND.DNPRS(I).EQ.-1) GOTO 11
	ENDDO
      NDPNDS=NDPNDS+1
      IF(NDPNDS.GT.MXDP) THEN
      PAUSE 'TOO MANY DEPENDENCIES'
      RETURN
      ENDIF
      DGIDS(NDPNDS)=DGID
C  FLAG INDIRECT DEPENDANTS TO FOLLOW PATHS
      DNPRS(NDPNDS)=-1
      WRITE(0,101) NDPNDS,LSTNMS(DGID,43),DGID,GERMD%METHOD,
     & GERMD%PROGENITOR_ID1,GERMD%PROGENITOR_ID2,NPRG
      IF(NPRG.NE.2.OR.GERMD%PROGENITOR_ID1.NE.NGROUP) THEN
      write(0,'(A)') 
     &'** WARNING ** THIS DEPENDENT MAY HAVE AN ILLEGAL GROUP'
C THE DEPENDENCY IS AN ILLEGAL DERIVATIVE BECAUSE EITHER
C IGID IS OCCURING AS ITS GROUP ID (AND IGID IS NOT GENERATIVE
C OR A SOURCE GROUP) OR IT IS NOT IN THE SAME GROUP AS IGID
C (ALL DERIVATIVATIVES MUST STAY IN THE SAME GROUP)
c      RETURN
      ENDIF
 11   I4=GMS_findDescendant(DGIDS(K),GERMD,NPRG,1)
      END DO
 10   K=K+1
      END DO
      IER=1
      RETURN
      END
C******************************************************* LREPLACE
C  PURPOSE:
C  REPLACE OR DELETE GERMPLASM
C
C  ARGUMENTS:
C  GIDX - GID OF GERMPLASM TO BE REPLACED OR DELETED
C  GIDY - REPLACEMENT GERMPLASM OR ZERO FOR DELETION
C
C  LREPLACE RETURNS WITH VALUE 1 IF GIDX WAS SUCESSFULLY
C  REPLACED OR DELETED
C  LREPLACE RETURNS WITH ZERO IF NO CHANGES WERE MADE
C  LREPLACE RETURNS WITH -1 IF ONLY PARTIAL REPLACEMENT
C  WAS ACHIEVED
C---- 20/10/07 ----------------------------------------- LREPLACE
      INTEGER*4 FUNCTION LREPLACE(GIDX,GIDY,REASON)
      USE GMS_STRUCTURES
      USE GMS_INTERFACE
      USE GMS_FUNCTIONS
      USE GMS_TOOLS
      PARAMETER (MAXTREE=30000,MAXLIST=3000)
      CHARACTER REASON*80,NBRS*50,NULL*1,AVAL*8000
      CHARACTER RDATE*10,RTIME*10,CTMP*1,BELL*1,VIEWER*60
      CHARACTER GVIZPATH*60      
      INTEGER*4 GIDX,GIDY,DGID,DEPNDS(MAXLIST,2),VAL,CASE,GY,GX,GD
      INTEGER*4 IWK(100),NODATE,KDATE,KTIME
      TYPE (GMS_GERMPLASM) GERMX,GERMY,GERMD
      TYPE (GMS_ATTRIBUTE) ATTB
      TYPE (GMS_NAMEDATA) NAMDAT,NAMDAT1
      TYPE (GMS_CHANGES) CHANGE
      TYPE (GMS_LISTDATA) LSTDTA
      COMMON /WRK/ NDPNDS,MXLST,DEPNDS,AVAL
      COMMON /OPTIONS/ NUID,NLOC,NREF,IOPT,LIST,ISRH,MGAT,NAMP,NTCX
     &,IBTYPE,LDST,IYAL,NMSTD,VIEWER,GVIZPATH
      DATA NULL /0/,NODATE/0/,BELL/7/
      BELL=G_BELL
      MXLST=500
      LREPLACE=0
      CALL GETDATM(RDATE,RTIME,KDATE,KTIME)      
      CASE=1
      GY=0
      WRITE(0,100) GIDX,GIDY,REASON(1:LEN_TRIM(REASON))
      WRITE(3,100) GIDX,GIDY,REASON(1:LEN_TRIM(REASON))      
 100  FORMAT(' REPLACE GID',I9,' WITH ',I9,' BECAUSE',A)
      IF(GIDX.EQ.0) RETURN
C WRITE OUT REPLACED RECORD AND CHECK FOR CHANGE ATTRIBUTES
      CALL WRTGERM(GIDX,GERMX,3)
      IF(GIDX.NE.GERMX%GERMPLASM_ID) THEN
      WRITE(0,115)
 115  FORMAT(/' *** ERROR *** RETRIEVAL ERROR OR REPLACED GERMPLASM')
      RETURN
      ENDIF
      VAL=GIDX
      GX=GROUPID(GERMX)
      IF(GX.EQ.GIDX) CASE=4
      IF(GIDY.NE.0) THEN
      VAL=GIDY
C WRITE OUT REPLACEMENT RECORD
      CALL WRTGERM(GIDY,GERMY,3)
      IF(GIDY.NE.GERMY%GERMPLASM_ID) THEN
      WRITE(0,115)
      RETURN
      ENDIF
      GY=GROUPID(GERMY)
      IF(GERMX%NO_PROGENITORS.GE.0.AND.GERMY%NO_PROGENITORS.GE.0) THEN
C  GIDX AND GIDY BOTH GENERATIVE
      CASE=1
      ELSE
     &IF(GERMX%NO_PROGENITORS.EQ.-1.AND.GERMY%NO_PROGENITORS.EQ.-1
     &   .AND.GX.EQ.GY) THEN
C  BOTH DERIVATIVE IN THE SAME GERMPLASM GROUP
      CASE=2
      IF(GY.EQ.GIDX) GY=GIDY
      ELSE
     &IF(GERMX%NO_PROGENITORS.GE.0.AND.GERMY%NO_PROGENITORS.EQ.-1) THEN
C  GIDX GENERATIVE AND GIDY DERIVATIVE
      CASE=3
C DEAL WITH THE SPECIAL CASES OF CASE THREE WHERE GIDX IS THE GROUP
C SOURCE OF GIDY SO THAT GIDY MUST BECOME A DERIVATIVE GROUP SOURCE
      IF(GERMY%PROGENITOR_ID2.NE.0.AND.GERMY%PROGENITOR_ID2.NE.GIDX)THEN
      PAUSE 'DERIVATIVE WITH KNOWN SOURCE CANNOT BE GROUP SOURCE'
      RETURN
      ENDIF
      GY=GIDY
      ELSE
C  GIDX IS DERIVATIVE AND GIDY IS GENERATIVE OR DERIVATIVE
C  BUT IN A DIFFERENT GROUP TO GIDX
      CASE=4
      ENDIF
C  COPY NAMES
      AVAL=' '
      NAMDAT%GERMPLASM=GIDX
      NAMDAT%TYPE=0
      NAMDAT%STATUS=0
      I4=GMS_getName(NAMDAT,AVAL,256,0)
      DO WHILE (I4.EQ.1)
      II=INDEX(AVAL,NULL)
      AVAL(II:)=' '
      II=LEN_TRIM(AVAL)
 26   WRITE(0,118) FCODE(NAMDAT%TYPE),AVAL(1:LEN_TRIM(AVAL))
 118  FORMAT(' TYPE='A8,A)
      WRITE(0,120) 'NAME'
      CTMP='Y'
      IF(IYAL.EQ.0) READ(1,121) CTMP
 120  FORMAT(' COPY THIS ',A,'? (Y/N, X TO ABBORT)')
 121  FORMAT(A1)
      IF(CTMP.EQ.'X'.OR.CTMP.EQ.'x') RETURN
      IF(CTMP.EQ.' '.OR.CTMP.EQ.'Y'.OR.CTMP.EQ.'y') THEN
      CALL ADDNAME(GIDY,NAMDAT%TYPE,NAMDAT%LOCATION,NAMDAT%DATE,
     &NAMDAT%REFERENCE,AVAL,IER)
      LREPLACE=-1
      ENDIF
      AVAL=' '
      NAMDAT%TYPE=0
      NAMDAT%STATUS=0
      I4=GMS_getName(NAMDAT,AVAL,256,1)
      END DO
C CHECK GERMPLASM FIELDS
      GERMD=GERMY
C -- LOCATION
      IF(GERMX%LOCATION.NE.0.AND.GERMX.LOCATION.NE.GERMY%LOCATION) THEN
      WRITE(0,111) GERMX.LOCATION,GERMY.LOCATION
 111  FORMAT(' REPLACED GERMPLASM HAS LOCATION',I5,' REPLACEMENT',I5/
     &' DO YOU WANT TO ASSIGN REPLACED LOCATION TO REPLACEMENT?',
     &' (Y/N, X TO ABORT)')
      CTMP='Y'
      IF(IYAL.EQ.0) READ(1,121) CTMP
      IF(CTMP.EQ.'X'.OR.CTMP.EQ.'x') RETURN
      IF(CTMP.EQ.' '.OR.CTMP.EQ.'Y'.OR.CTMP.EQ.'y') THEN
      CALL ZEROCHANGE(CHANGE)
      CHANGE%ctable='GERMPLSM'C
      CHANGE%cfield='GLOCN'C
      CHANGE%cgroup=RDATE//':'//RTIME(1:8)//NULL
      CHANGE%crecord=GIDY
      CHANGE%cfrom=GERMY%LOCATION
      CHANGE%cto=GERMX%LOCATION
      CHANGE%cref=NREF
      I4=GMS_addChanges(CHANGE,FTOCSTR(TRIM(REASON)//' LREPLACE'))
      IF(I4.NE.1) RETURN
      GERMY%LOCATION=GERMX%LOCATION
      LREPLACE=-1
      ENDIF
      AVAL=' '
      ENDIF
C -- DATE
      IF(GERMX%DATE.NE.0.AND.GERMX%DATE.NE.GERMY%DATE) THEN
      WRITE(0,114) GERMX.DATE,GERMY.DATE
 114  FORMAT(' REPLACED GERMPLASM HAS DATE ',I10,' REPLACEMENT ',I10/
     &' DO YOU WANT TO ASSIGN REPLACED DATE TO REPLACEMENT?',
     &' (Y/N, X TO ABORT)')
      CTMP='Y'
      IF(IYAL.EQ.0) READ(1,121) CTMP
      IF(CTMP.EQ.'X'.OR.CTMP.EQ.'x') RETURN
      IF(CTMP.EQ.' '.OR.CTMP.EQ.'Y'.OR.CTMP.EQ.'y') THEN
      CALL ZEROCHANGE(CHANGE)
      CHANGE%ctable='GERMPLSM'C
      CHANGE%cfield='GDATE'C
      CHANGE%cgroup=RDATE//':'//RTIME(1:8)//NULL
      CHANGE%crecord=GIDY
      CHANGE%cfrom=GERMY%DATE
      CHANGE%cto=GERMX%DATE
      CHANGE%cref=NREF
      I4=GMS_addChanges(CHANGE,FTOCSTR(TRIM(REASON)//' LREPLACE'))
      IF(I4.NE.1) RETURN
      GERMY%DATE=GERMX%DATE
      LREPLACE=-1
      ENDIF
      AVAL=' '
      ENDIF
C -- IF THE GERMPLASM IS LOCAL MAKE CHANGE NOW
      IF(GIDY.LT.0.AND..NOT.COMPARE_GERMPLASM(GERMD,GERMY)) THEN
      I4=GMS_setGermplasm(GERMY)
      IF(I4.NE.1) PAUSE 'SETGERM'
      ENDIF
C  COPY ATTRIBUTES
      AVAL=' '
      ATTB%TYPE=0
      ATTB%GERMPLASM=GIDX
      I4=GMS_getAttribute(ATTB,AVAL,256,0)
      DO WHILE (I4.EQ.1)
      IF(ATTB%TYPE.NE.102) THEN
      II=INDEX(AVAL,NULL)
      AVAL(II:II)=' '
 28   WRITE(0,118) FCODE(ATTB%TYPE),AVAL(1:LEN_TRIM(AVAL))
      WRITE(0,120) 'ATTRUBUTE'
      CTMP='Y'
      IF(IYAL.EQ.0) READ(1,121) CTMP
      IF(CTMP.EQ.'X'.OR.CTMP.EQ.'x') RETURN
      IF(CTMP.EQ.' '.OR.CTMP.EQ.'Y'.OR.CTMP.EQ.'y') THEN
      CALL ADDATT(GIDY,ATTB%TYPE,ATTB%LOCATION,ATTB%DATE,
     &ATTB%REFERENCE,AVAL,IER)
      LREPLACE=-1
      ENDIF
      AVAL=' '
      ATTB%TYPE=0
      ENDIF
      I4=GMS_getAttribute(ATTB,AVAL,256,1)
      END DO
      ENDIF ! GIDY.NE.0
C  DISPLAY DEPENDENTS NDPNDS=TOTAL NO OF DEPENDENTS
      NDPNDS=0
      CALL DDPNDS(GIDX,GERMX,CASE,2000,
     & NDPNDS,DEPNDS(1,1),DEPNDS(1,2),JER)
      IF(JER.NE.1) RETURN
C  CHANGE DEPENDENCIES FOR DELETION?
      IF(GIDY.EQ.0.AND.NDPNDS.GT.0) THEN
      WRITE(0,122)
      WRITE(3,122)
 122  FORMAT(' *** WARNING *** THESE DEPENDENCIES WILL BE SET TO ',
     & 'MISSING(0). CONTINUE? (Y/N)')
      CTMP='Y'
      IF(IYAL.EQ.0) READ(1,121) CTMP
      IF(CTMP.NE.'Y'.AND.CTMP.NE.'y') RETURN
      ENDIF
C  REPLACE DEPENDENCIES
      LREPLACE=-1
      NDPS=0
      DO 4 K=1,NDPNDS
      I4=GMS_getGermplasm2(DEPNDS(K,1),GERMD,NAMDAT,AVAL,256)
      IF(I4.NE.1) RETURN
      LFM=GIDX
      LTO=GIDY
      IF(GERMD%NO_PROGENITORS.EQ.-1) THEN
      IF(DEPNDS(K,2).EQ.1) THEN
      LFM=GX
      LTO=GY
      ELSE !  DEPNDS(K,2).EQ.2
      GD=GERMD%PROGENITOR_ID1
      IF(GX.NE.GY.AND.GD.NE.GY) THEN
      WRITE(0,109) BELL,K
      WRITE(3,109) ' ',K
 109  FORMAT(' *** WARNING ***',A1,
     &' DEPENDENCY NO',I3,' HAS A DIFFERENT GROUP TO NEW SOURCE'/
     &' (CHANGE GROUP? (Y/N, X TO ABORT)')
      CTMP='Y'
      IF(IYAL.EQ.0) READ(1,121) CTMP
      IF(CTMP.NE.'Y'.AND.CTMP.NE.'y') RETURN
      CALL ZEROCHANGE(CHANGE)
      CHANGE%ctable='GERMPLSM'C
      CHANGE%cfield='GPID1'C
      CHANGE%cgroup=RDATE//':'//RTIME(1:8)//NULL
      CHANGE%crecord=DEPNDS(K,1)
      CHANGE%cfrom=GX
      CHANGE%cto=GY
      CHANGE%cref=NREF
      I4=GMS_addChanges(CHANGE,FTOCSTR(TRIM(REASON)//' LREPLACE'))
      IF(I4.NE.1) RETURN
      IF(DEPNDS(K,1).LT.0) GERMD%PROGENITOR_ID1=GY
      ENDIF
      ENDIF
      ENDIF
      IF(LTO.EQ.DEPNDS(K,1)) THEN
      WRITE(3,108) ' ',DEPNDS(K,1),DEPNDS(K,2),LFM,LTO
      WRITE(0,108) BELL,DEPNDS(K,1),DEPNDS(K,2),LFM,LTO
 108  FORMAT(' *** WARNING ***',A1,
     &' THE FOLLOWING CHANGE WILL CREATE A CIRCULARITY:'/
     &' IN',I12,' CHANGE PROGENITOR_ID',I1,' FROM',I12,' TO',I12/
     &' SET LINK TO ZERO? (Y/N)')
      CTMP='Y'
      IF(IYAL.EQ.0) READ(1,121) CTMP
      IF(CTMP.NE.'Y'.AND.CTMP.NE.'y') RETURN
      LTO=0
      ENDIF
      IF(LTO.EQ.0.AND.
     &DEPNDS(K,2).EQ.1.AND.GERMD%PROGENITOR_ID2.NE.0) THEN
       IF(GERMD%PROGENITOR_ID2.EQ.GERMD%PROGENITOR_ID1
     & .OR.GERMD%NO_PROGENITORS.GE.0) THEN
      CALL ZEROCHANGE(CHANGE)
      CHANGE%ctable='GERMPLSM'C
      CHANGE%cfield='GPID2'C
      CHANGE%cgroup=RDATE//':'//RTIME(1:8)//NULL
      CHANGE%crecord=DEPNDS(K,1)
      CHANGE%cfrom=LFM
      CHANGE%cto=LTO
      CHANGE%cref=NREF
      I4=GMS_addChanges(CHANGE,FTOCSTR(TRIM(REASON)//' LREPLACE'))
      IF(I4.NE.1) RETURN
      GERMD%PROGENITOR_ID2=LTO
       ELSE
      WRITE(0,119) BELL,DEPNDS(K,1),DEPNDS(K,2),LFM,LTO
      WRITE(3,119) BELL,DEPNDS(K,1),DEPNDS(K,2),LFM,LTO      
 119  FORMAT(' *** WARNING ***',A1,
     &' THE FOLLOWING CHANGE WILL CREATE AN ILLEGAL LINK:'/
     &' IN',I12,' CHANGE PROGENITOR_ID',I1,' FROM',I12,' TO',I12)
      RETURN
      ENDIF
      ENDIF
      IF(DEPNDS(K,2).EQ.1.AND.GERMD%PROGENITOR_ID1.EQ.LTO) GOTO 4
      IF(DEPNDS(K,2).EQ.2.AND.GERMD%PROGENITOR_ID2.EQ.LTO) GOTO 4
      CALL ZEROCHANGE(CHANGE)
      CHANGE%ctable='GERMPLSM'C
      CHANGE%cfield='GPID1'C
      if (DEPNDS(K,2).eq.2) CHANGE%cfield='GPID2'C
      CHANGE%cgroup=RDATE//':'//RTIME(1:8)//NULL
      CHANGE%crecord=DEPNDS(K,1)
      CHANGE%cfrom=LFM
      CHANGE%cto=LTO
      CHANGE%cref=NREF
      I4=GMS_addChanges(CHANGE,FTOCSTR(TRIM(REASON)//' LREPLACE'))
      IF(I4.NE.1) RETURN
      NDPS=NDPS+1
C IF THE GERMPLASM IS LOCAL MAKE CHANGE NOW
      IF(DEPNDS(K,1).LT.0) THEN
      IF(DEPNDS(K,2).EQ.1) THEN
      GERMD%PROGENITOR_ID1=LTO
      I4=GMS_setGermplasm(GERMD)
      ELSE IF(DEPNDS(K,2).EQ.2) THEN
      GERMD%PROGENITOR_ID2=LTO
      I4=GMS_setGermplasm(GERMD)
      ELSE
     &IF(DEPNDS(K,2).GT.2.AND.DEPNDS(K,2).LE.GERMD%NO_PROGENITORS)THEN
      I4=GMS_setProgenitorID(DEPNDS(K,1),DEPNDS(K,2),LTO)
      ELSE
      RETURN
      ENDIF
      IF(I4.NE.1) RETURN
      ENDIF
 4    CONTINUE
C  CHECK FOR OCCURANCE OF GIDX IN LISTS
      CALL SEARCHLISTS(GIDX,DEPNDS,NDPNDS,IER)
      IF(NDPNDS.GT.0) THEN
      WRITE(0,123) GIDX
      WRITE(3,123) GIDX
 123  FORMAT(' *** WARNING *** GID',I9,' OCCURS IN THE ABOVE LISTS'/
     &' GERMPLASM DATA WILL BE CHANGED IN LOCAL LISTS ONLY.'/
     &' CONTINUE REPLACEMENT? (Y/N)')
      CTMP='Y'
      IF(IYAL.EQ.0) READ(1,121) CTMP
      IF(CTMP.NE.'Y'.AND.CTMP.NE.'y') RETURN
      DO I=1,NDPNDS
      IF(DEPNDS(I,1).LT.0) THEN
      CALL ZEROLSTDAT(LSTDTA)
      LSTDTA%LISTID=DEPNDS(I,1)
      LSTDTA%LRECID=DEPNDS(I,2)
      I4=GMS_getListData(LSTDTA,0)
      IF(I4.NE.1.OR.LSTDTA%GID.NE.GIDX) RETURN
      LSTDTA%GID=GIDY
      IF(GIDY.EQ.0)THEN
      LSTDTA%DESIG=' '//NULL
      LSTDTA%GRPNAME=' '//NULL
      ELSE      
      I4=GMS_getGermplasm2(GIDY,GERMD,NAMDAT,AVAL,256)
      IF(I4.NE.1) RETURN      
      LSTDTA%DESIG=AVAL
C GET CROSS EXPANSION      
      IF(GERMD%NO_PROGENITORS.GT.0) THEN !GERMPLASM IS GENERATIVE
      I4=GMS_crossExpansion(GERMD%GERMPLASM_ID,1,NTCX,AVAL,4096,IWK,100)
      ELSE IF(GERMD%PROGENITOR_ID1.NE.0) THEN 
      JER=GERMD%PROGENITOR_ID1 !GERMPLASM IS DERIVATIVE WITH KNOWN GROUP
      I4=GMS_getGermplasm2(JER,GERMD,NAMDAT,AVAL,256)
      IF(I4.NE.1) RETURN
        IF(GERMD%NO_PROGENITORS.GT.0) THEN !THE GROUP IS GENERATIVE
          I4=GMS_crossExpansion(JER,1,NTCX,AVAL,4096,IWK,100)
        ENDIF
      ENDIF
      LSTDTA%GRPNAME=AVAL
      ENDIF      
      I4=GMS_setListData(LSTDTA)
      IF(I4.NE.1) RETURN
      ENDIF
      ENDDO
      ENDIF
      CALL ZEROCHANGE(CHANGE)
      CHANGE%ctable='GERMPLSM'C
      CHANGE%cfield='GID'C
      CHANGE%cgroup=RDATE//':'//RTIME(1:8)//NULL
      CHANGE%crecord=GIDX
      CHANGE%cfrom=GIDX
      CHANGE%cto=GIDY
      IF(GIDY.EQ.0) CHANGE%cto=GIDX
      CHANGE%cref=NREF
      I4=GMS_addChanges(CHANGE,FTOCSTR(TRIM(REASON)//' LREPLACE'))
      IF(I4.NE.1) RETURN
C IF GIDX IS LOCAL MAKE THE CHANGE NOW      
      IF(GIDX.LT.0) THEN
      GERMX%REPLACE=VAL
      I4=GMS_setGermplasm(GERMX)
      IF(I4.NE.1) RETURN
      ENDIF
      LREPLACE=1
      WRITE(0,104)
 104  FORMAT(1X,60(1H-))
      RETURN
      END
C******************************************************* RELATIVES
C  PURPOSE:
C  DISPLAY RELATIVES OF GERMPLASM GERM
C---- LAST UPDATE 12/03/98 ----------------------------- RELATIVES
      SUBROUTINE RELATIVES(GERM)
      USE GMS_STRUCTURES
      USE GMS_INTERFACE
      USE GMS_TOOLS
      PARAMETER (MAXTREE=30000,MAXLIST=3000)
      CHARACTER ANAME*10000,NULL*1,TYPE*10
      INTEGER*4 FIND_FIRST,FIND_NEXT
      TYPE(GMS_GERMPLASM) GERM,GERM2
      TYPE(GMS_NAMEDATA) NAMDAT
      COMMON /WRK/ NLST,MXLST,LLST(MAXLIST),ANAME
      DATA FIND_FIRST,FIND_NEXT,NULL /0,1,0/
      NLST=0
      WRITE(0,101)
      WRITE(3,101)
 101  FORMAT(' CLOSE RELATIVES:'/
     &' NO NAMES',47X,'GID METHOD RELATION PNO')
 100  FORMAT(I3,1X,A48,I9,I4,1X,A,I2)
      IGID=GERM%GERMPLASM_ID
C  GET SOURCES
      KGID=GERM%PROGENITOR_ID1
      TYPE='GROUP SRC'
      II=1
 40   IF(GERM%NO_PROGENITORS.EQ.-1.AND.KGID.NE.0) THEN
      I5=GMS_getGermplasm2(KGID,GERM2,NAMDAT,ANAME,256)
      IF(I5.NE.1) STOP 12
      ANAME=LSTNMS(GERM2%GERMPLASM_ID,50)
      IF(NLST.EQ.MXLST) GOTO 4
      NLST=NLST+1
      LLST(NLST)=GERM2%GERMPLASM_ID
      WRITE(0,100) NLST,ANAME,LLST(NLST),GERM2%METHOD,TYPE,II
      WRITE(3,100) NLST,ANAME,LLST(NLST),GERM2%METHOD,TYPE,II
      TYPE='SOURCE'
      II=II+1
      KGID=GERM%PROGENITOR_ID2
      IF(II.EQ.2) GOTO 40
C  AND SISTERS (LINES WITH THE SAME SOURCE)
      JGID=GERM2%GERMPLASM_ID
      II=FIND_FIRST
      TYPE='SISTER LINE'
 1    I4=GMS_findDescendant(JGID,GERM2,NPRG,II)
      II=FIND_NEXT
      IF(I4.NE.1) GOTO 2
      IF(GERM2%REPLACE.NE.0) GOTO 1
      IF(GERM2%PROGENITOR_ID2.EQ.JGID.AND.GERM2%NO_PROGENITORS.EQ.-1
     & .AND.GERM2%GERMPLASM_ID.NE.IGID) THEN
      IF(NLST.EQ.MXLST) GOTO 4
      NLST=NLST+1
      LLST(NLST)=GERM2%GERMPLASM_ID
      ANAME=LSTNMS(GERM2%GERMPLASM_ID,50)
      WRITE(0,100) NLST,ANAME,LLST(NLST),GERM2%METHOD,TYPE,NPRG
      WRITE(3,100) NLST,ANAME,LLST(NLST),GERM2%METHOD,TYPE,NPRG
      ENDIF
      GOTO 1
      ENDIF
C  FIND DERIVATIVES AND OFFSPRING OF GERM
 2    II=FIND_FIRST
 3    I4=GMS_findDescendant(IGID,GERM2,NPRG,II)
      II=FIND_NEXT
      IF(I4.NE.1) RETURN
      IF(GERM2%REPLACE.NE.0) GOTO 3
      ANAME=LSTNMS(GERM2%GERMPLASM_ID,50)
      TYPE='OFFSPRING'
      IF(GERM2%NO_PROGENITORS.EQ.-1) TYPE='DERIVATIVE'
      IF(NLST.EQ.MXLST) GOTO 4
      NLST=NLST+1
      LLST(NLST)=GERM2%GERMPLASM_ID
      WRITE(0,100) NLST,ANAME,LLST(NLST),GERM2%METHOD,TYPE,NPRG
      WRITE(3,100) NLST,ANAME,LLST(NLST),GERM2%METHOD,TYPE,NPRG
      GOTO 3
      RETURN
 4    WRITE(0,102)
      WRITE(3,102)
 102  FORMAT(' *** PARTIAL LIST OF RELATIVES')
      RETURN
      END
C******************************************************* ADDLIST
C  PURPOSE:
C  ADD A GERMPLASM TO A LIST
C
C  ARGUMENTS:
C  LSTID- ID OF LIST WHERE ENTRY IS TO BE ADDED
C  LINE- .LST COMMAND: .LST <gid>,<code>,<source>,<desig>
C        <fields> MAY BE NULL, IF <gid>=0 IGID SPECIFIES GID
C  IGID- GID TO ADD TO LIST UNLESS SPECIFIED IN LINE
C  NTRY- RETURNS WITH THE ENTRY ID IF SUCESSFUL
C  IER - IER = 0 IF PROCESS FAILED WITHOUT ATTEMPT
C             -1 IF PROCESS FAILED AFTER WRITING
C              1 IF PROCESS WAS SUCESSFUL
C---- LAST UPDATE 09/05/99 ----------------------------- ADDLIST
      SUBROUTINE ADDLIST(LSTID,LINE,IGID,NTRY,IER)
      USE GMS_STRUCTURES
      USE GMS_INTERFACE
      USE GMS_TOOLS
      CHARACTER NULL*1,CWK*256,LINE*(*),RDATE*10,RTIME*10,LFT*12
      CHARACTER VIEWER*60,GVIZPATH*60
      INTEGER*4 IWK(20)
      TYPE(GMS_GERMPLASM) GERM
      TYPE(GMS_NAMEDATA) NAMDAT
      TYPE(GMS_LISTDATA) LSTD
      TYPE(GMS_LISTNAME) LNAME
      COMMON /OPTIONS/ NUID,NLOC,NREF,IOPT,LIST,ISRH,MGAT,NAMP,NTCX
     &,IBTYPE,LDST,IYAL,NMSTD,VIEWER,GVIZPATH
      DATA NULL /0/
      EQUIVALENCE (IWK(1),CWK)
      CWK=LINE
      IER=0
      IF(LSTID.EQ.0) RETURN
      CALL ZEROLSTNM(LNAME)
      LNAME%LISTID=LSTID
      I4=GMS_getListName(LNAME,0)
      IF(I4.NE.1.OR.LNAME%LISTSTATUS.NE.1) RETURN
C  FIND THE HIGHEST ENTRYID
      NTRY=0
      LSTD%LISTID=LSTID
      LSTD%GID=0
      LSTD%ENTRYID=0
      LSTD%LRECID=0
      I4=GMS_getListData(LSTD,0)
      DO WHILE (I4.EQ.1)
      NTRY=MAX(NTRY,LSTD%ENTRYID)
      I4=GMS_getListData(LSTD,1)
      ENDDO
      NTRY=NTRY+1
C SET UP THE NEW LISTDATA RECORD         
      LSTD%ENTRYCD=NULL
      LSTD%SOURCE=NULL
      LSTD%DESIG=NULL
      LSTD%GRPNAME=NULL
      IF(CWK(5:).NE.' ') THEN
      L=5
      M=INUMBA(IWK,L,80,J,K)
      IF(K.GT.0.AND.K.NE.44) THEN
      WRITE(0,104) 'ILLEGAL LST COMMAND'
 104  FORMAT(1X,A)
      RETURN
      ENDIF
      IF(M.EQ.0) M=IGID
      IGID=M
C CHECK FOR <code>
      IF(L.LT.80) THEN
      IF(CWK(L:).NE.' ') THEN
      K=INDEX(CWK(L:),',')+L-2
      IF(K.EQ.L-2) K=80
      IF(K.GE.L) THEN
      LSTD%ENTRYCD=LFT(CWK(L:K))
      L=LEN_TRIM(LSTD%ENTRYCD)+1
      LSTD%ENTRYCD(L:L)=NULL
      ENDIF
      L=K+2
      ENDIF
C CHECK FOR <source>
      IF(L.LT.80) THEN
      IF(CWK(L:).NE.' ') THEN
      K=INDEX(CWK(L:),',')+L-2
      IF(K.EQ.L-2) K=80
      IF(K.GE.L) LSTD%SOURCE=FTOCSTR(LFT(CWK(L:K)))
      L=K+2
      ENDIF
C CHECK FOR <desig>
      IF(L.LT.80) THEN
      K=INDEX(CWK(L:),',')+L-2
      IF(K.EQ.L-2) K=80
      IF(K.GE.L) LSTD%DESIG=FTOCSTR(LFT(CWK(L:K)))
      L=K+2
      ENDIF
C CHECK FOR <gpname>
      IF(L.LT.80) THEN
      IF(CWK(L:).NE.' ') LSTD%GRPNAME=FTOCSTR(LFT(CWK(L:)))
      ENDIF
      ENDIF
      ENDIF
      ENDIF
      LSTD%GID=IGID
      IF(IGID.NE.0) THEN
      I4=GMS_getGermplasm2(IGID,GERM,NAMDAT,CWK,256)
      IF(I4.NE.1) THEN
      WRITE(0,104) 'ILLEGAL GID'
      RETURN
      ENDIF
      CWK=CTOFSTR(CWK)
      IGID=GERM%GERMPLASM_ID
      LSTD%GID=IGID
      IF(LSTD%DESIG.EQ.NULL) THEN
      IF(NAMP.EQ.1) CWK=LSTNMS(IGID,255)
      I4=MIN(LEN_TRIM(CWK)+1,256)
      CWK(I4:I4)=NULL
      LSTD%DESIG=CWK(1:I4)
      ENDIF
      IF(LSTD%SOURCE.EQ.NULL) THEN
      L=0
      IF(GERM%NO_PROGENITORS.EQ.-1) L=GERM%PROGENITOR_ID2
      IF(L.NE.0) THEN
      I4=GMS_getGermplasm2(L,GERM,NAMDAT,CWK,256)
      L=MIN(48,INDEX(CWK,NULL))
      CWK(L:L)=NULL
      IF(I4.EQ.1) LSTD%SOURCE=CWK(1:L)
      ENDIF
      ENDIF
      IF(LSTD%GRPNAME.EQ.NULL) THEN
      I=1
      IF(NTCX.NE.0) I=5
      I4=GMS_crossExpansion(IGID,I,NTCX,LSTD%GRPNAME,164,IWK,20)
      IF(INDEX(LSTD%GRPNAME,'/').EQ.0.AND.NTCX.EQ.0) I4=0
      IF(I4.NE.1) LSTD%GRPNAME=NULL
      ENDIF
      ENDIF
      LSTD%LISTID=LSTID
      LSTD%LRECID=0
      LSTD%ENTRYID=NTRY
      IER=-1
      I4=GMS_addListData(LSTD)
      IF(I4.NE.1) THEN
      WRITE(0,104) 'UNABLE TO ADD ENTRY'
      RETURN
      ENDIF
      NTRY=LSTD%ENTRYID
      IER=1
C      ENDIFCWK
      RETURN
      END
C******************************************************* MAKELIST
C  PURPOSE:
C  MAKE A NEW GERMPLASM LIST
C
C  ARGUMENTS:
C  NAME - NAME FOR NEW LIST (<12)
C  TYPE - TYPE OF LIST (<8)
C  DESC - DESCRIPTION OF LIST (<80)
C  LSTID- RETURNS WITH LIST ID IF SUCESSFUL
C  IER  - IER = -1 IF PROCESS FAILED
C                1 IF PROCESS WAS SUCESSFUL
c               -5 IF LIST NAME IS TOO LONG
C---- LAST UPDATE 30/04/04 ----------------------------- MAKELIST
      SUBROUTINE MAKELIST(NAME,TYPE,DESC,LSTID,IER)
      USE GMS_STRUCTURES
      USE GMS_INTERFACE
      USE GMS_TOOLS
      CHARACTER RDATE*10,RTIME*10,NAME*(*),TYPE*(*),DESC*(*),NULL*1
      CHARACTER CTMP*12,CNAME*12,LFT*12,VIEWER*60,GVIZPATH*60
      TYPE(GMS_LISTNAME) LSTN
      COMMON /OPTIONS/ NUID,NLOC,NREF,IOPT,LIST,ISRH,MGAT,NAMP,NTCX
     &,IBTYPE,LDST,IYAL,NMSTD,VIEWER,GVIZPATH
      DATA NULL/0/
      LSTID=0
      IER=-5
      CALL GETDATM(RDATE,RTIME,IDTE,ITME)
      CNAME=NAME
      IF(NAME.EQ.' ') CNAME='BROWSE'
      LSTN%LISTNAME=FTOCSTR(CNAME)
      IF(LEN_TRIM(LSTN%LISTNAME).GE.47) RETURN
      LSTN%LISTDATE=IDTE
      LSTN%LISTTYPE=FTOCSTR(TYPE)
      LSTN%LISTUID=NUID
      LSTN%LISTDESC=FTOCSTR(DESC)
      LSTN%LISTSTATUS=1
      LSTN%LISTHIER=0
      IF(TYPE.EQ.' ') LSTN%LISTTYPE='BROWSE'C
      IF(DESC.EQ.' ')
     &LSTN%LISTDESC='BROWSE'//RDATE//':'//RTIME(1:8)//NULL
      II=0
 1    IER=GMS_addListName(LSTN)
      IF(IER.EQ.-5) THEN
      II=II+1
      WRITE(CTMP,'(I12)') II
!      I=SCAN(CTMP,'1234567890')
      LSTN%LISTNAME=FTOCSTR(TRIM(CNAME)//LFT(CTMP))
      IF(LEN_TRIM(LSTN%LISTNAME).LT.47) GO TO 1
      ENDIF
      IF(IER.EQ.1) LSTID=LSTN%LISTID
      RETURN
      END
C******************************************************* ADDANAME
C  PURPOSE:
C  ADD A GERMPLASM NAME
C
C  ARGUMENTS:
C  LINE- .NAME COMMAND: .NAME <gid>,<ntype>,<ndate>,<nlocn>,<nvalue>
C        <fields> MAY BE NULL, IF <gid>=0 OR NULL, IGID SPECIFIES GID
C  IGID- GID TO ADD NAME
C  IER - IER = 0 IF PROCESS FAILED WITHOUT ATTEMPT
C             -1 IF PROCESS FAILED AFTER WRITING
C              1 IF PROCESS WAS SUCESSFUL
C---- LAST UPDATE 09/05/99 ----------------------------- ADDANAME
      SUBROUTINE ADDANAME(LINE,IGID,IER)
      USE GMS_STRUCTURES
      USE GMS_INTERFACE
      USE GMS_FUNCTIONS
      USE GMS_TOOLS
      PARAMETER (MAXTREE=30000,MAXLIST=3000)
      CHARACTER ANAME*10000,NULL*1,CWK*256,LINE*(*),RDATE*10,RTIME*10
      CHARACTER VIEWER*60,GVIZPATH*60
      INTEGER*4 IWK(64)
      TYPE(GMS_GERMPLASM) GERM
      COMMON /OPTIONS/ NUID,NLOC,NREF,IOPT,LIST,ISRH,MGAT,NAMP,NTCX
     &,IBTYPE,LDST,IYAL,NMSTD,VIEWER,GVIZPATH
      COMMON /WRK/ NLST,MXLST,LLST(MAXLIST),ANAME
      DATA NULL /0/
      EQUIVALENCE (IWK(1),CWK)
      CWK=LINE
      IER=0
      IF(CWK(6:).EQ.' ') RETURN
C  CHECK FOR GID
      L=6
      M=INUMBA(IWK,L,80,J,K)
      IF(K.GT.0.AND.K.NE.44) THEN
      WRITE(0,104) 'ILLEGAL NAME COMMAND'
 104  FORMAT(1X,A)
      RETURN
      ENDIF
      IF(M.EQ.0) M=IGID
      IF(M.NE.0) CALL WRTGERM(M,GERM,0)
      IGID=M
C CHECK FOR <ntype>
      IF(L.GE.80.OR.CWK(L:).EQ.' ') RETURN
      M=INUMBA(IWK,L,80,J,K)
      IF(M.EQ.0.OR.(K.GT.0.AND.K.NE.44)) THEN
      WRITE(0,104) 'ILLEGAL NAME COMMAND'
      RETURN
      ENDIF
      NTYPE=M
C CHECK FOR <locid>
      IF(L.GE.80.OR.CWK(L:).EQ.' ') RETURN
      MLOC=INUMBA(IWK,L,80,J,K)
      IF(K.GT.0.AND.K.NE.44) THEN
      WRITE(0,104) 'ILLEGAL NAME COMMAND'
      RETURN
      ENDIF
      IF(MLOC.EQ.0) MLOC=NLOC
C CHECK FOR <date>
      IF(L.GE.80.OR.CWK(L:).EQ.' ') RETURN
      M=INUMBA(IWK,L,80,J,K)
      IF(M.GT.1900.AND.M.LT.10000) M=M*10000
      IF((M.NE.0.AND.M.LT.1900).OR.(K.GT.0.AND.K.NE.44)) THEN
      WRITE(0,104) 'ILLEGAL NAME COMMAND'
      RETURN
      ENDIF
      NDATE=M
C CHECK FOR <refid>
      IF(L.GE.80.OR.CWK(L:).EQ.' ') RETURN
      MREF=INUMBA(IWK,L,80,J,K)
      IF(K.GT.0.AND.K.NE.44) THEN
      WRITE(0,104) 'ILLEGAL NAME COMMAND'
      RETURN
      ENDIF
      IF(MREF.EQ.0) MREF=NREF
C
      ANAME=LINE(L:)
      CWK=LINE(L:)
      IF(NMSTD.EQ.0) THEN
      I=GMS_standardName(ANAME,CWK,255)
      CWK=CTOFSTR(CWK)
      ENDIF
      CALL ADDNAME(IGID,NTYPE,MLOC,NDATE,MREF,CWK,IER)
      RETURN
      END
C******************************************************* ADDUSER
C  PURPOSE:
C  ADD A USER
C
C  ARGUMENTS:
C  LINE - .ADU COMMAND: .ADU <uname>,<pwd>,<security>,<access>,<type>
C        <fields> MAY BE NULL
C  LINST- LOCAL INSTALLATION ID FOR WHICH USERS WILL BE ADDED
C  USER - USER STRUCTURE WITH PARAMETERS OF TEH CURRENT USER
C  IER  - IER = 0 IF PROCESS FAILED WITHOUT ATTEMPT
C             -1 IF PROCESS FAILED AFTER WRITING
C              1 IF PROCESS WAS SUCESSFUL
C---- LAST UPDATE 05/05/04 ----------------------------- ADDUSER
      SUBROUTINE ADDUSER(LINE,LINST,USER,IER)
      USE GMS_STRUCTURES
      USE GMS_INTERFACE
      USE GMS_FUNCTIONS
      USE GMS_TOOLS
      PARAMETER (MAXTREE=30000,MAXLIST=3000)
      CHARACTER ANAME*10000,NULL*1,CWK*80,LINE*(*)
      CHARACTER ATMP*12,RTIME*12,RDATE*12,VIEWER*60,GVIZPATH*60
      INTEGER*4 IWK(20),SECURITY
      TYPE(GMS_USER) USER,AUSER
      COMMON /OPTIONS/ NUID,NLOC,NREF,IOPT,LIST,ISRH,MGAT,NAMP,NTCX
     &,IBTYPE,LDST,IYAL,NMSTD,VIEWER,GVIZPATH
      COMMON /WRK/ NLST,MXLST,LLST(MAXLIST),ANAME
      DATA NULL /0/
      EQUIVALENCE (IWK(1),CWK)
      CWK=LINE
      IER=0
      IF(CWK(6:).EQ.' ') RETURN
      ANAME=LINE(6:)
      K=INDEX(ANAME,',')
      IF(K.EQ.0.OR.K.GT.31) THEN
      WRITE(0,104) 'ILLEGAL ADU COMMAND: USERNAME'
      RETURN
      ENDIF
      L=INDEX(ANAME(K+1:),',')+K
      IF (L.EQ.K) L=LEN_TRIM(ANAME)+1
      IF (L.EQ.K+1.OR.L-K-1.GT.10) THEN
      WRITE(0,104) 'ILLEGAL ADU COMMAND: PASSWORD'
      RETURN
      ENDIF
      ATMP=ANAME(K+1:L-1)
      ANAME(K:)=' '
      L=L+6
C  CHECK FOR INSTALLID
c      L=6
c      M=INUMBA(IWK,L,80,J,K)
c      IF(K.GT.0.AND.K.NE.44) THEN
c      WRITE(0,104) 'ILLEGAL ADU COMMAND'
 104  FORMAT(1X,A)
c      RETURN
c      ENDIF
c      IF(M.EQ.0) M=LINST
c      LINST=M
C CHECK FOR <security>
      M=0
      IF(L.LT.80.AND.CWK(L:).NE.' ') THEN
      M=INUMBA(IWK,L,80,J,K)
      IF(K.GT.0.AND.K.NE.44) THEN
      WRITE(0,104) 'ILLEGAL ADU COMMAND: SECURITY'
      RETURN
      ENDIF
      ENDIF
      IF (M.EQ.0) M=1
      SECURITY=M
C CHECK FOR <access>
      M=0
      IF(L.LT.80.AND.CWK(L:).NE.' ') THEN
      M=INUMBA(IWK,L,80,J,K)
      IF(K.GT.0.AND.K.NE.44) THEN
      WRITE(0,104) 'ILLEGAL ADU COMMAND: ACCESS'
      RETURN
      ENDIF
      ENDIF
      IF(USER%ACCESS.EQ.150.AND.M.EQ.0) M=100
      IF(USER%ACCESS.GE. 80.AND.M.EQ.0) M=70
      MACC=M
C CHECK FOR <type>
      M=0
      IF(L.LT.80.AND.CWK(L:).NE.' ') THEN
      M=INUMBA(IWK,L,80,J,K)
      IF(K.GT.0.AND.K.NE.44) THEN
      WRITE(0,104) 'ILLEGAL ADU COMMAND: TYPE'
      RETURN
      ENDIF
      ENDIF
      IF(M.EQ.0) M=422
      MTYPE=M
C CHECK FOR <date>
C      IF(L.GE.80.OR.CWK(L:).EQ.' ') RETURN
C      M=INUMBA(IWK,L,80,J,K)
C      IF(M.GT.1900.AND.M.LT.10000) M=M*10000
C      IF((M.NE.0.AND.M.LT.1900).OR.(K.GT.0.AND.K.NE.44)) THEN
C      WRITE(0,104) 'ILLEGAL ADU COMMAND'
C      RETURN
C      ENDIFC
C      IF(M.EQ.0) THEN
      CALL GETDATM(RDATE,RTIME,MDATE,ITME)      
C      M=IDTE
C      ENDIF
C      MDATE=M
      AUSER%USER_ID=0
      AUSER%INSTALLATION=LINST
      AUSER%STATUS=1
      IF(SECURITY.GT.1) AUSER%STATUS=2
      AUSER%ACCESS=MACC
      AUSER%TYPE=MTYPE
      AUSER%NAME=FTOCSTR(ANAME)
      AUSER%PASSWORD=FTOCSTR(ATMP)
      AUSER%PERSON_ID=0
      AUSER%ASSIGN_DATE=MDATE
      AUSER%CLOSE_DATE=0
      I4=GMS_addUser(AUSER,ATMP,SECURITY)
      IF(I4.NE.1) THEN
      WRITE(0,125)
 125  FORMAT(' *** ERROR *** ADDUSER FUNCTION ABANDONED')
      IER=-1
      RETURN
      ENDIF
      IER=1
      IF(SECURITY.EQ.3) WRITE(0,'(1X,A)') ATMP
      RETURN
      END
C******************************************************* GROUP
C  PURPOSE:
C  DISPLAY GROUP RELATIVES OF GERMPLASM GERM
C---- LAST UPDATE 17/03/00 ----------------------------- GROUP
      SUBROUTINE GROUP(GERM)
      USE GMS_STRUCTURES
      USE GMS_INTERFACE
      USE GMS_FUNCTIONS
      USE GMS_TOOLS
      PARAMETER (MAXTREE=30000,MAXLIST=3000)
      CHARACTER CTMP*80
      INTEGER*4 FIND_FIRST,FIND_NEXT
      TYPE(GMS_GERMPLASM) GERM,GERM2
      TYPE(GMS_NAMEDATA) NAMDAT
      COMMON /WRK/ NLST,MXLST,LLST(MAXLIST)
      DATA FIND_FIRST,FIND_NEXT /0,1/
      NGROUP=GROUPID(GERM)
      I4=GMS_getGermplasm2(NGROUP,GERM2,NAMDAT,CTMP,80)
      IF(I4.NE.1) STOP 22
      CTMP=CTOFSTR(CTMP)
      WRITE(0,101) TRIM(CTMP),GERM2%GERMPLASM_ID
      WRITE(3,101) TRIM(CTMP),GERM2%GERMPLASM_ID
 101  FORMAT(' GROUP RELATIVES OF ',A,' (GID',I9,')')
      CALL ZEROGERM(GERM2)
      GERM2%NO_PROGENITORS=-1
      GERM2%PROGENITOR_ID1=NGROUP
      I4=GMS_FindGermplasm(GERM2,FIND_FIRST)
      DO WHILE (I4.EQ.1)
      IF(NLST.EQ.MXLST) GOTO 4
      NLST=NLST+1
      LLST(NLST)=GERM2%GERMPLASM_ID
      I4=GMS_GetGermplasm2(LLST(NLST),GERM2,NAMDAT,CTMP,80)
      CTMP=LSTNMS(NAMDAT%GERMPLASM,47)
      WRITE(0,100) NLST,TRIM(CTMP),
     &NAMDAT%GERMPLASM,FCODE(NAMDAT%TYPE),MCODE(GERM2%METHOD),
     &CNTRY(GERM2%LOCATION),GERM2%DATE/10000
      WRITE(3,100) NLST,TRIM(CTMP),
     &NAMDAT%GERMPLASM,FCODE(NAMDAT%TYPE),MCODE(GERM2%METHOD),
     &CNTRY(GERM2%LOCATION),GERM2%DATE/10000
 100  FORMAT(I3,1X,A,T50,I9,1X,A5,1X,2A4,I5)
      I4=GMS_FindGermplasm(GERM2,FIND_NEXT)
      END DO
      RETURN
 4    WRITE(0,102)
      WRITE(3,102)
 102  FORMAT(' *** PARTIAL LIST OF GROUP RELATIVES')
      RETURN
      END    
C******************************************************* FIXATT
C  PURPOSE:
C  FIX OLD REPLACE ATTRIBUTES
C------------------------------------------------------- FIXATT
      SUBROUTINE FIXATT(IGID)
      USE GMS_STRUCTURES
      USE GMS_INTERFACE
      USE GMS_TOOLS
      USE GMS_FUNCTIONS
      PARAMETER (MAXTREE=30000,MAXLIST=3000)
      CHARACTER ANAME*10000,NULL*1,CWK*12,AVAL*256,ATIME*8
      CHARACTER KTIME*6,VIEWER*60,GVIZPATH*60
      TYPE(GMS_ATTRIBUTE) ATTB
      COMMON /OPTIONS/ NUID,NLOC,NREF,IOPT,LIST,ISRH,MGAT,NAMP,NTCX
     &,IBTYPE,LDST,IYAL,NMSTD,VIEWER,GVIZPATH
      COMMON /WRK/ NLST,MXLST,LLST(MAXLIST),IPROG(1000),ANAME
      DATA NULL /0/
C
C  SEARCH FOR ALL ATTRIBUTES
C
      NREF=1000
  2   READ(2,102,END=999) ANAME
 102  FORMAT(A)
      READ(ANAME,*) IGID,KDATE
      I=INDEX(ANAME,' ')
      I=I+9
      ATIME=ANAME(I+1:I+8)
      KTIME=ANAME(I+1:I+2)//ANAME(I+4:I+5)//ANAME(I+7:I+8)
      AVAL=ANAME(I+9:I+26)
      II=LEN_TRIM(ANAME)+1
      READ(AVAL,101) KGID,NDPNDS
      IF(NDPNDS.GT.110) THEN
      READ(2,102) ANAME(II:)
      AVAL=ANAME(2000:2100)
      ENDIF
 101  FORMAT(I12,I6)
      II=I+28+18*NDPNDS
      WRITE(AVAL,114) ATIME,KGID,NDPNDS,KDATE,KTIME,
     & ANAME(II:LEN_TRIM(ANAME))
 114  FORMAT(A8,I12,I6,I9,A6,1X,A)
C      WRITE(0,103) IGID,AVAL(1:LEN_TRIM(AVAL))
      CALL ADDATT(IGID,101,NLOC,KDATE,NREF,AVAL,IER)
      IF (IER.NE.1) STOP 1
      WRITE(AVAL,100) I+26,NDPNDS,NDPNDS
 100  FORMAT('(',I3,'X,',I3,'I12,',I3,'I6)')
      READ(ANAME,AVAL) (LLST(I),I=1,NDPNDS),(IPROG(I),I=1,NDPNDS)
      DO 1 I=1,NDPNDS
      WRITE(AVAL,113) ATIME,IGID,KGID,IPROG(I),KDATE,KTIME
 113  FORMAT(A8,2I12,1X,'PROGENITOR_ID',I1,I14,A6)
C      WRITE(0,103) LLST(I),AVAL(1:LEN_TRIM(AVAL))
 103  FORMAT(I8,1X,A)
      CALL ADDATT(LLST(I),102,NLOC,KDATE,NREF,AVAL,I4)
      IF(I4.NE.1) STOP 2
 1    CONTINUE
      GOTO 2
 999  CALL GMS_COMMITDATA()
      RETURN
      END SUBROUTINE FIXATT
C******************************************************* GIV
C  PURPOSE:
C  INVERT COP MATRIX
C  ARGUMENTS:
C  LU     - LOGICAL UNIT FOR TEST OUTPUT
C  N      - NUMBER OF ENTRIES IN THE LIST
C  LGID   - INTEGER ARRAY OF N GIDS IN LIST ORDER
C  COPMAT - REAL ARRAY 
C           ENTERS WITH LOWER TRIANGULAR PART OF COP MATRIX IN ROW ORDER
C           RETURNS WITH LOWER TRIANGULAR PART OF (G-)INVERSE COP MATRIX
C  NOTES:
C  EIGEN VALUES LESS THAN ABSTOL=1.0D-9 ARE DECLARED ZERO
C  IF ZERO EIGEN VALUES ARE DETECTED A G-INVERSE IS COMPUTED
C-- LAST UPDATE 05/03/05 ------------------------------- GIV
      SUBROUTINE GIV(LU,N,LGID,COPMAT)
C SET UP THE IMSL ENVIRONMENT
      include 'link_fnl_static.h'
!DEC$ OBJCOMMENT LIB:"libguide.lib"
      USE GMS_TOOLS
      DIMENSION LGID(1000,2),COPMAT(*)
      CHARACTER CTMP*2,LNMS*80,FMT1*20
      DOUBLE PRECISION VL,VU,ABSTOL,VT
      CHARACTER, ALLOCATABLE :: LNAME(:)*12
C      INTEGER, ALLOCATABLE :: ISUPPZ(:),LWORK(:)
      REAL*8, ALLOCATABLE :: COP(:,:),W(:),Z(:,:)
C      REAL*8, ALLOCATABLE :: WORK(:)
      REAL*4, ALLOCATABLE :: X(:),Y(:)
      ABSTOL=0.0D0
      ALLOCATE (COP(N,N),W(N),Z(N,N))
C      ALLOCATE (ISUPPZ(2*N),WORK(26*N),LWORK(10*N))
      ALLOCATE (LNAME(N))
      IJ=0
      DO 2 I=1,N
      WRITE(LNAME(I),'(I12)') LGID(I,1)
      DO 1 J=1,I
      IJ=IJ+1
      COP(I,J)=DBLE(COPMAT(IJ))
      COP(J,I)=COP(I,J)
 1    CONTINUE
 2    CONTINUE
      CALL SSMPR(COPMAT,N,3,' COP MATRIX LABELLED BY GID',LNAME,1,0)
      CALL DEVCSF (N, COP, N, W, Z, N)
      VL = DEPISF (N, N, COP, N, W, Z, N)
CC SECTION REPLACED BY CALL TO IMSL DEVCSF ABOVE -------           
C      I1=26*N
C      I2=10*N
C      CALL DSYEVR('V','A','L',N,COP,N,VL,VU,IL,IU,
C     $ABSTOL,M,W,Z,N,ISUPPZ,WORK,I1,LWORK,I2,INFO)
C      ABSTOL=1.0D-9
CC REORDER W AND Z SINCE DSYEVER RETURNS THEM IN ASCENDING
CC ORDER OF EIGEN VALUES
C      DO J=1,N/2
C        VL=W(J)
C        W(J)=W(N-J+1)
C        W(N-J+1)=VL
C        DO I=1,N
C          VL=Z(I,J)
C          Z(I,J)=Z(I,N-J+1)
C          Z(I,N-J+1)=VL
C        ENDDO
C      ENDDO
CC END OF SECTION REPLACED ---------------------------
      CTMP=' '
      M=0
      VT=0.0D0
      DO I=1,N
      IF(W(I).LT.ABSTOL) M=M+1
      VT=VT+W(I)
      END DO
      IF(M.GT.0) CTMP='G-'
      WRITE(LU,101) 'EIGEN VALUES OF COP MATRIX '//
     &'(TRACE,  NUMBER ZERO,  INFO (<1=GOOD, <100=OK, >100=NOT GOOD))',
     &VT,M,VL
 101  FORMAT(/1X,A/27X,G12.4,I5,5X,G12.4)
      WRITE(LU,104) (W(I),I=1,N)
	WRITE(LU,101) 'GIDS AND LEADING EIGEN VECTORS OF COP MATRIX'
	WRITE(FMT1,100) MIN(4,N-M)
 100  FORMAT('(A12,',I1,'G12.4,1X,A)')	
	DO I=1,N
	LNMS=LSTNMS(LGID(I,1),40)
	WRITE(LU,FMT1) LNAME(I),(Z(I,J),J=1,MIN(4,N-M)),TRIM(LNMS)
	ENDDO
      IJ=0
      DO I=1,N
        DO  J=1,I
          VL=0.0D0
            DO K=1,N
              IF(W(K).GT.ABSTOL) VL=VL+Z(I,K)*Z(J,K)/W(K)
            END DO
          IJ=IJ+1
          COPMAT(IJ)=VL
        END DO
      END DO
 104  FORMAT(6G10.3)
      CALL SSMPR(COPMAT,N,3,CTMP//'INVERSE COP MATRIX LABELLED BY GID',
     &LNAME,1,0)
      WRITE(LU,101) 'COP- IN GIV FORMAT'
      IJ=0
      DO I = 1,N
      DO J = 1,I
      IJ=IJ+1
      WRITE(LU,103) I,J,COPMAT(IJ),LGID(I,1),LGID(J,1)
      END DO
      END DO
 103  FORMAT(2I5,G16.8,2I12)
      DEALLOCATE (COP,LNAME)
C      DEALLOCATE (ISUPPZ,WORK,LWORK)
      ALLOCATE (X(N),Y(N))
      VL=DSQRT(W(1))
      VU=DSQRT(W(2))
      DO I=1,N
      X(I)=SNGL(Z(I,1)*VL)
      Y(I)=SNGL(Z(I,2)*VU)
      ENDDO
      WRITE(LU,105) 100.0D0*(W(1)+W(2))/VT
 105  FORMAT(/' PLOT OF AXIS 1 vs AXIS 2 FOR SPECTRAL DECOMPOSITION OF',
     1' COP MATRIX'/' CONTAINS',G8.2,
     2' % OF TRACE, AXIS VALUES SCALED BY ROOT EIGENVALUE' )       
      CALL SCATPL(X,N,1,I,0,Y,LU,50,80,0.0,0.0,0.0,0.0,1,IER)
      DEALLOCATE (W,X,Y,Z)  
      RETURN
      END      
C******************************************************* DRAWTREE
C  PURPOSE:
C  DRAW AND PLOT THE PEDIGREE TREE STORED IN COMMON STRUCTURE GTREE
C
C  ARGUMENTS:
C  LINE  - CURRENT LINE IN GTREE TO WRITE
C  LEVEL - LEVEL OF LINE IN THE PEDIGREE
C  CWK   - WORK ARRAY
C  LINEP - LINE NUMBER OF THE PREVIOUS LEVEL
C  COLOR - COLOR FOR THE CURRENT NODE
C  LSTLVL- LAST LEVEL TO DRAW
C  LST   - .TRUE. IF CURRENT LINE IS LAST PROGENITOR IN A SET
C  LVW   - .TRUE. FOR LINE AND GRAPH VIEWER, .false. FOR VIEWER ONLY
C  IN /OPTIONS/
C  NAMP  - 0 LIST PARENTS ONLY AND PREFERRED NAMES ONLY,
C          1 PARENTS ONLY WITH ALL NAMES,
C          2 FOR PARENTS ONLY WITH PREFERRED NAMES + ACCESSION NOS
C  LDST  - 0 SUPRESS DISPLAY OF DERIVATIVE STEPS
C          1 DISPLAY DERIVATIVE STEPS AND GROUP SOURCES
C
C---- 06/09/10 ----------------------------------------- DRAWTREE
      RECURSIVE SUBROUTINE DRAWTREE(LINE,LEVEL,CWK,LINEP,
     &COLOR,LSTLVL,LST,LVW)
      USE GMS_STRUCTURES
      USE GMS_INTERFACE
      USE GMS_TOOLS
      PARAMETER (MAXTREE=30000)      
      TYPE(GMS_GERMPLASM) GERM
      TYPE(GMS_TREE) GTREE(MAXTREE)
      TYPE(GMS_NAMEDATA) NAME
      CHARACTER CWK*(*),MRK*1,ANAME*256,STEP*3,NODE*256
      CHARACTER COLOR*20,CTMP*256,ITOC*12,VIEWER*60,LNTMPO*256
      CHARACTER GVIZPATH*60      
      LOGICAL LST,LVW
      COMMON /WRK/ NCR,MXTREE,GTREE
      COMMON /OPTIONS/ NUID,NLOC,NREF,IOPT,LIST,ISRH,MGAT,NAMP,NTCX
     &,IBTYPE,LDST,IYAL,NMSTD,VIEWER,GVIZPATH
      L=2*LEVEL-1
      IF(LEVEL.EQ.0) THEN
      CWK=' '
      L=1
      ELSE IF (CWK(L+2:L+2).NE.'>') THEN
      CWK(L:)=' +-'
      DO 1 I=2,L,2
      IF(CWK(I:I).NE.' ') CWK(I:I)='|'
 1    CONTINUE
      ENDIF
      LGID=GTREE(LINE)%GID
      I=LEN_TRIM(CWK)
      IF(NAMP.EQ.1) THEN
      ANAME=LSTNMS(LGID,61-I)
      CWK=CWK(1:I)//TRIM(ANAME)
      ELSE
      NAME%GERMPLASM=LGID
      NAME%TYPE=0
      NAME%STATUS=1
      ANAME='<Unknown>'C
      I4=GMS_getName(NAME,ANAME,256,0)
      CWK=CWK(1:I)//CTOFSTR(ANAME)
      ENDIF
      CTMP=TRIM(CTOFSTR(ANAME))      
      IF(NAMP.EQ.2) THEN
      NAME%GERMPLASM=LGID
      NAME%TYPE=1
      NAME%STATUS=0
      I4=GMS_getName(NAME,ANAME,256,0)
      I=LEN_TRIM(CWK)
      IF(I4.EQ.1) THEN
      CWK=CWK(1:I)//'('//TRIM(CTOFSTR(ANAME))//')'
      I=LEN_TRIM(CTMP)
      CTMP=CTMP(1:I)//'\nACCNO: '//TRIM(CTOFSTR(ANAME))
      ENDIF
      ENDIF
C LOCATE THE GROUP LINE FOR THE CURRENT PROGENITOR AND
C COUNT DERIVATIVE STEPS
      LN=LINE
      NSTP=0
      IF(GTREE(LINE)%GNPGS.EQ.-1.AND.GTREE(LINE)%DADDR.LE.0) NSTP=-1
      IF(GTREE(LINE)%DADDR.GE.0) THEN
      LN=GTREE(LINE)%GADDR
      I=GTREE(LINE)%DADDR
      IF(I.GT.0) THEN
      DO WHILE (GTREE(I)%DADDR.GT.0)
      I=I+1
      ENDDO
      NSTP=I-GTREE(LINE)%DADDR+1
      IF(GTREE(I)%DADDR.GE.0) NSTP=-1
      ENDIF
      ENDIF
C
      MRK=' '
      IF(LN.GT.0) THEN
      IF(GTREE(LN)%GADDR.EQ.0) MRK='#'
      IF(GTREE(LN)%GADDR.EQ.-1.AND.(LDST.EQ.0.OR.LN.EQ.LINE)) MRK='<'
      IF(GTREE(LN)%GADDR.GT.0.AND.GTREE(LN)%GADDR.LT.LINE) MRK='*'
      IF(GTREE(LN)%GNPGS.EQ.-1.AND.LN.NE.LINE) MRK=' '
      ENDIF
      STEP='  ?'
      IF(NSTP.GE.0) WRITE(STEP,'(I3)') NSTP
      IF(LVW)WRITE(0,101) LINE,TRIM(CWK),STEP,LGID,MRK
      IF(LVW)WRITE(3,101) LINE,TRIM(CWK),STEP,LGID,MRK
 101  FORMAT(I4,A,T66,A3,I9,A1)
C  IF THERE IS A PLOT VIEWER WRITE THE .DOT FILE 
      IF(VIEWER.NE.' ') THEN
      LGIDP=0
      IF(LINEP.EQ.0) THEN
      REWIND(4)
      J=0 
      K=0 
 10   READ(4,'(A)',END=11) LNTMPO
      IF(INDEX(LNTMPO,'[color=').GT.0) THEN
      READ(LNTMPO,'(I10)') NO1 
      IF(NO1.EQ.LGID) THEN
      NO2=INDEX(LNTMPO,',')-1       
      J=1
      IF(LNTMPO(19:NO2).NE.COLOR) K=1 
      IF(LNTMPO(19:NO2).EQ.'red') K=0 
      ENDIF
      ENDIF
      GO TO 10
 11   IF(J.EQ.0) 
     &WRITE(4,104) LGID,TRIM(COLOR),TRIM(CTMP),TRIM(ITOC(LGID,K))
      IF(J.EQ.1.AND.K.EQ.1)
     &WRITE(4,104) LGID,'red',TRIM(CTMP),TRIM(ITOC(LGID,K))
      ELSE 
      LGIDP=GTREE(LINEP)%GID
C  ONLY WRITE THE NEW LINE IF IT IS NOT ALREADY ON THE FILE       
      REWIND(4)
      I=0 
      J=0 
      K=0 
 5    READ(4,'(A)',END=6) LNTMPO
      IF(INDEX(LNTMPO,' -> ').GT.0) THEN 
      READ(LNTMPO,'(I10,4X,I10)') NO1,NO2
      IF(NO1.EQ.LGID.AND.NO2.EQ.LGIDP) I=1 
      ENDIF 
      IF(INDEX(LNTMPO,'[color=').GT.0) THEN 
      NO2=INDEX(LNTMPO,',')-1
      IF(NO1.EQ.LGID) THEN 
      J=1
      IF(LNTMPO(19:NO2).NE.COLOR) K=1 
      IF(LNTMPO(19:NO2).EQ.'red') K=0 
      ENDIF
      ENDIF
      GO TO 5
 6    IF(J.EQ.0) 
     &WRITE(4,104) LGID,TRIM(COLOR),TRIM(CTMP),TRIM(ITOC(LGID,K))
      IF(J.EQ.1.AND.K.EQ.1)
     &WRITE(4,104) LGID,'red',TRIM(CTMP),TRIM(ITOC(LGID,K))
 104  FORMAT(I10,' [color=',A',label="',A'\nGID:',A,'"];')
      IF(I.EQ.0) WRITE(4,102) LGID,LGIDP
 102  FORMAT(I10,' -> ',I10,';')
      ENDIF
      ENDIF
      LINEX=LINE
C  IF LDST=1 THEN LIST ALL INTERMEDIATE DERIVATIVES
      IF(LDST.EQ.1.AND.MRK.NE.'*') THEN
      IF(GTREE(LINE)%DADDR.GT.0) THEN
      ILN=GTREE(LINE)%DADDR
 4    IF(ILN.EQ.-1) GOTO 3
      CWK(L:)=' |>'
      IF(LST) CWK(L:)='  >'
      NSTP=NSTP-1
      STEP='  ?'
      IF(NSTP.GE.0) WRITE(STEP,'(I3)') NSTP
      LGIDP=LGID
      LGID=GTREE(ILN)%GID
      I=LEN_TRIM(CWK)
      IF(NAMP.EQ.1) THEN
      ANAME=LSTNMS(LGID,61-I)
      CWK=CWK(1:I)//TRIM(ANAME)
      ELSE
      NAME%GERMPLASM=LGID
      NAME%TYPE=0
      NAME%STATUS=1
      ANAME='<Unknown>'C
      I4=GMS_getName(NAME,ANAME,256,0)
      CWK=CWK(1:I)//CTOFSTR(ANAME)
      ENDIF
      CTMP=TRIM(CTOFSTR(ANAME))      
      IF(NAMP.EQ.2) THEN
      NAME%GERMPLASM=LGID
      NAME%TYPE=1
      NAME%STATUS=0
      I4=GMS_getName(NAME,ANAME,256,0)
      I=LEN_TRIM(CWK)
      IF(I4.EQ.1) CWK=CWK(1:I)//'('//TRIM(CTOFSTR(ANAME))//')'
      I=LEN_TRIM(CTMP)
      CTMP=CTMP(1:I)//'\nACCNO: '//TRIM(CTOFSTR(ANAME))      
      ENDIF
      MRK=' '
      IF(GTREE(ILN)%GADDR.EQ.0) MRK='#'
      IF(GTREE(ILN)%GADDR.EQ.-1) MRK='<'
      IF(GTREE(ILN)%GADDR.GT.0.AND.GTREE(ILN)%GADDR.LT.LINE) MRK='*'
      IF(LVW)WRITE(0,101) ILN,TRIM(CWK),STEP,LGID,MRK
      IF(LVW)WRITE(3,101) ILN,TRIM(CWK),STEP,LGID,MRK
      IF(VIEWER.NE.' ') THEN
      REWIND(4)
      I=0 
      J=0 
      K=0 
 7    READ(4,'(A)',END=8) LNTMPO
      IF(INDEX(LNTMPO,' -> ').GT.0) THEN 
      READ(LNTMPO,'(I10,4X,I10)') NO1,NO2
      IF(NO1.EQ.LGID.AND.NO2.EQ.LGIDP) I=1 
      ENDIF 
      IF(INDEX(LNTMPO,'[color=').GT.0) THEN 
      NO2=INDEX(LNTMPO,',')-1
      IF(NO1.EQ.LGID) THEN 
      J=1
      IF(LNTMPO(19:NO2).NE.COLOR) K=1 
      IF(LNTMPO(19:NO2).EQ.'red') K=0 
      ENDIF
      ENDIF
      GO TO 7
 8    IF(J.EQ.0) 
     &WRITE(4,104) LGID,TRIM(COLOR),TRIM(CTMP),TRIM(ITOC(LGID,K))
      IF(J.EQ.1.AND.K.EQ.1)
     &WRITE(4,104) LGID,'red',TRIM(CTMP),TRIM(ITOC(LGID,K))
      IF(I.EQ.0) WRITE(4,102) LGID,LGIDP
      ENDIF
      I=ILN
      ILN=GTREE(ILN)%DADDR
      IF(ILN.EQ.0) ILN=I+1
      GOTO 4
 3    CWK(L+2:)=' '
      LINEX=I    
C
      ENDIF
      IF(LST) CWK(L:)=' '
      ELSE
      IF(LST.OR.CWK(L+2:L+2).EQ.'>') CWK(L:)=' '
C  IF LDST.EQ.0 AND PROGENITOR HAS A DERIVATIVE GROUP SOURCE SHOW IT
      IF(LN.GT.0) THEN
      IF(GTREE(LN)%GNPGS.EQ.-1.AND.LN.NE.LINE) THEN
      IF(L.LE.0) THEN
      CWK=''
      ELSE
      CWK(L:)=' |>'
      IF(LST) CWK(L:)='  >'
      ENDIF
      NLINE=LN
      NLEVEL=LEVEL
      CALL DRAWTREE(NLINE,NLEVEL,CWK,LINEX,COLOR,LSTLVL,.TRUE.,LVW)
      ENDIF
      ENDIF
      ENDIF
C
      NLEVEL=LEVEL+1
      IF(LN.EQ.0.OR.NLEVEL.GT.LSTLVL.OR.MRK.EQ.'*') RETURN
      IF(GTREE(LN)%GADDR.LE.0) RETURN
      K=IABS(GTREE(LN)%GNPGS)
      DO 2,I=1,K
      NLINE=GTREE(LN)%GADDR+I-1
      CALL DRAWTREE(NLINE,NLEVEL,CWK,LINEX,COLOR,LSTLVL,(I.EQ.K),LVW)
 2    CONTINUE
      RETURN
      END SUBROUTINE DRAWTREE      
C******************************************************* VIEWLST
C  PURPOSE:
C  DRAW THE DERIVATIVE NEIGHBORHOOD STORED IN LIST
C
C  ARGUMENTS:
C  IGID - GID OF THE CENER OF THE NEIGHBORHOOD
C  LASTDIR - OUTPUT DIRECTIRY
C
C---- 12/09/10 ----------------------------------------- VIEWLST
      SUBROUTINE VIEWLST(IGID,LASTDIR,GRP)
      USE GMS_STRUCTURES
      USE GMS_INTERFACE
      USE GMS_TOOLS
      PARAMETER (MAXTREE=30000,MAXLIST=3000)
      CHARACTER CWK*256,LASTDIR*(*),VIEWER*60,ITOC*12,COLOR*20
      CHARACTER GVIZPATH*60      
      CHARACTER CTMP*256, ANAME*256
      INTEGER*4 OGID,PGID,PLOC,OLOC,GGID
      LOGICAL*4 GRP    
      TYPE(GMS_GERMPLASM) GERM
      TYPE(GMS_NAMEDATA) NAMDAT
      TYPE(GMS_LOCATION) LOCN
      COMMON /WRK/ NLST,MXLST,LLST(MAXLIST)     
      COMMON /OPTIONS/ NUID,NLOC,NREF,IOPT,LIST,ISRH,MGAT,NAMP,NTCX
     &,IBTYPE,LDST,IYAL,NMSTD,VIEWER,GVIZPATH     
      CWK='BROWSE'
      IF(LASTDIR.NE.' ') CWK=TRIM(LASTDIR)//'BROWSE'
      OPEN(4,FILE=TRIM(CWK)//'.dot',IOSTAT=IER)
      WRITE(4,"('digraph NBH {')")
C  WRITE THE GROUP NODE
      I4=GMS_getGermplasm(IGID,GERM)
      IF(I4.NE.1) GOTO 9   
      GGID=GERM%PROGENITOR_ID1      
      IF(GGID.NE.0) THEN
       I4=GMS_getGermplasm(GGID,GERM)
       IF(I4.NE.1) GOTO 9
       PLOC=GERM%LOCATION
       IF(PLOC.NE.0) THEN
        LOCN%LOCATION_ID=PLOC
        I4=GMS_getLocation(LOCN,0)
        ANAME=CTOFSTR(LOCN%PREFERRED_NAME)
        CWK=CTOFSTR(LOCN%ABBREVIATION)
        IF(CWK.NE.' '.AND.LEN_TRIM(ANAME).GT.20) ANAME=CWK
        IF(LOCN%COUNTRY.NE.0) ANAME=TRIM(ANAME)//','//CNTRY(LOCN%COUNTRY)
       ENDIF                  
       CWK=LSTNMS(GGID,50)
       WRITE(4,104) GGID,'green',TRIM(CWK),TRIM(ITOC(PGID,K)),
     & TRIM(ANAME)     
      ENDIF
C  PROCESS EACH ENTRY IN THE NEIGHBORHOOD      
      DO I=1,NLST
      I4=GMS_getGermplasm(LLST(I),GERM)
      IF(I4.NE.1) GOTO 9
C WRITE THE OFSPRING NODE      
      OGID=GERM%GERMPLASM_ID
      OLOC=GERM%LOCATION
      IF(OLOC.NE.0) THEN
      LOCN%LOCATION_ID=GERM%LOCATION
      I4=GMS_getLocation(LOCN,0)
      ANAME=CTOFSTR(LOCN%PREFERRED_NAME)
      CWK=CTOFSTR(LOCN%ABBREVIATION)
      IF(CWK.NE.' '.AND.LEN_TRIM(ANAME).GT.20) ANAME=CWK
      IF(LOCN%COUNTRY.NE.0) ANAME=TRIM(ANAME)//','//CNTRY(LOCN%COUNTRY)
      ENDIF      
      CWK=LSTNMS(OGID,50)
      COLOR='black'
      IF(OGID.EQ.IGID) COLOR='red'
      K=0
      REWIND(4)
 1    READ(4,'(A)',END=2) CTMP
      IF(CTMP(12:18).EQ.'[color=')THEN
      READ(CTMP,'(I10)') J 
      IF(J.EQ.OGID) K=1
      ENDIF
      GOTO 1      
 2    IF(K.EQ.0)
     &WRITE(4,104) OGID,TRIM(COLOR),TRIM(CWK),TRIM(ITOC(OGID,K)),
     &TRIM(ANAME)
 104  FORMAT(I10,' [color=',A',label="',A,'\nGID:',A,'\nLOC:',A,'"];')
C  WRITE THE GROUP-OFSPRING EDGE          
      IF(GRP)WRITE(4,102) GGID,OGID,'GRP','green' 
C  WRITE THE PARENT (SOURCE) NODE 
      PLOC=0
      PGID=GERM%PROGENITOR_ID2      
      IF(PGID.NE.0) THEN
      I4=GMS_getGermplasm(PGID,GERM)
      IF(I4.NE.1) GOTO 9
      PLOC=GERM%LOCATION
      IF(PLOC.NE.0) THEN
      LOCN%LOCATION_ID=PLOC
      I4=GMS_getLocation(LOCN,0)
      ANAME=CTOFSTR(LOCN%PREFERRED_NAME)
      CWK=CTOFSTR(LOCN%ABBREVIATION)
      IF(CWK.NE.' '.AND.LEN_TRIM(ANAME).GT.20) ANAME=CWK
      IF(LOCN%COUNTRY.NE.0) ANAME=TRIM(ANAME)//','//CNTRY(LOCN%COUNTRY)
      ENDIF                  
      CWK=LSTNMS(PGID,50)
      K=0
      REWIND(4)      
 3    READ(4,'(A)',END=4) CTMP
      IF(CTMP(12:18).EQ.'[color=')THEN
      READ(CTMP,'(I10)') J 
      IF(J.EQ.PGID) K=1
      ENDIF
      GOTO 3      
 4    IF(K.EQ.0)
     &WRITE(4,104) PGID,'black',TRIM(CWK),TRIM(ITOC(PGID,K)),
     &TRIM(ANAME)
C  WRITE THE PARENT-OFSPRING EDGE          
      WRITE(4,102) PGID,OGID,MCODE(GERM%METHOD),'black'
 102  FORMAT(I10,' -> ',I10,' [label="',A,'",color="',A,'"];')
      ENDIF      
      ENDDO
      CALL ZERONAMDAT(NAMDAT)
      NAMDAT%GERMPLASM=IGID
      NAMDAT%STATUS=1
      I4=GMS_getName(NAMDAT,CWK,256,0)
      IF(I4.NE.1) GOTO 9
      WRITE(4,199) 'GID:',IGID,TRIM(CTOFSTR(CWK))
 199  FORMAT('overlap=false'/'label="'A,I8,' NAME: ',A,'"'/
     &'fontsize=20;'/'}')      
      CLOSE (4)
      CWK='BROWSE'
      IF(LASTDIR.NE.' ') CWK=TRIM(LASTDIR)//'BROWSE'      
      R=RUNQQ(TRIM(GVIZPATH)//'DOT',
     &'-Tgif '//TRIM(CWK)//'.dot -Gcharset=latin1 -o'//TRIM(CWK)//'.gif')
      R=RUNQQ(TRIM(VIEWER),TRIM(CWK)//'.gif')
      RETURN
 9    WRITE(0,'("*** ERROR FROM GMS CALL IN VIEWGRP ***")')
      RETURN     
      END SUBROUTINE VIEWLST

