# Microsoft Developer Studio Project File - Name="ICIS32" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=ICIS32 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ICIS32.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ICIS32.mak" CFG="ICIS32 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ICIS32 - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ICIS32 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "ICIS32"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ICIS32 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Release"
# PROP Intermediate_Dir "..\Release\ICIS32"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE F90 /compile_only /dll /nologo /warn:nofileopt
# ADD F90 /compile_only /dll /nologo /warn:nofileopt
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ICIS32_EXPORTS" /YX /FD /c
# ADD CPP /nologo /Gz /Zp4 /W3 /vmg /GX /O2 /Ob0 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "MAKE_DLL" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib odbc32.lib odbccp32.lib /nologo /dll /map /machine:I386 /force
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=lib /machine:ix86 /def:icis32.def /OUT:..\Release\ICIS32.lib	copy ..\Release\ICIS32.dll "P:\Develop\Release"	copy ..\Release\ICIS32.lib "P:\Develop\Release"	copy ..\Release\ICIS32.exp "P:\Develop\Release"	"C:\Program Files\WiredRed\EPop\EPop.exe" /message="New ICIS32.DLL" /subject="ICIS Development" /object="McLaren, Graham.users"
# End Special Build Tool

!ELSEIF  "$(CFG)" == "ICIS32 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\debug\ICIS32_v4"
# PROP Intermediate_Dir "..\Debug\ICIS32_v4\ICIS32"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE F90 /check:bounds /compile_only /debug:full /dll /nologo /traceback /warn:argument_checking /warn:nofileopt
# ADD F90 /alignment:rec4byte /browser /check:bounds /compile_only /debug:full /dll /nologo /traceback /warn:argument_checking /warn:nofileopt
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ICIS32_EXPORTS" /YX /FD /GZ /c
# ADD CPP /Zp4 /MTd /W3 /Gm /vmg /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "MAKE_DLL" /FAs /FR /YX /FD /GZ /c
# SUBTRACT CPP /nologo
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib odbc32.lib odbccp32.lib /nologo /dll /profile /map:"..\Debug/ICIS32.map" /debug /machine:I386
# SUBTRACT LINK32 /verbose /nodefaultlib /force

!ENDIF 

# Begin Target

# Name "ICIS32 - Win32 Release"
# Name "ICIS32 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat;f90;for;f;fpp"
# Begin Group "GMS_SOURCE"

# PROP Default_Filter ".cpp"
# Begin Source File

SOURCE=.\GMS\atributs.cpp
# End Source File
# Begin Source File

SOURCE=.\GMS\Bibrefs.cpp
# End Source File
# Begin Source File

SOURCE=.\GMS\Changes.cpp
# End Source File
# Begin Source File

SOURCE=.\GMS\germplsm.cpp
# End Source File
# Begin Source File

SOURCE=.\GMS\instln.cpp
# End Source File
# Begin Source File

SOURCE=.\GMS\lists.cpp
# End Source File
# Begin Source File

SOURCE=.\GMS\location.cpp
# End Source File
# Begin Source File

SOURCE=.\GMS\methods.cpp
# End Source File
# Begin Source File

SOURCE=.\GMS\misc.cpp
# End Source File
# Begin Source File

SOURCE=.\GMS\names.cpp
# End Source File
# Begin Source File

SOURCE=.\GMS\udfield.cpp
# End Source File
# Begin Source File

SOURCE=.\GMS\user.cpp
# End Source File
# End Group
# Begin Group "TOOLS_SOURCE"

# PROP Default_Filter "*.c;*.cpp"
# Begin Source File

SOURCE=.\Tools\wildcards.cpp
# End Source File
# End Group
# Begin Group "DMS_SOURCE"

# PROP Default_Filter ".cpp"
# Begin Source File

SOURCE=.\DMS\Data.cpp
# End Source File
# Begin Source File

SOURCE=.\DMS\Factor.cpp
# End Source File
# Begin Source File

SOURCE=.\DMS\Interface.cpp
# End Source File
# Begin Source File

SOURCE=.\DMS\Level.cpp
# End Source File
# Begin Source File

SOURCE=.\DMS\Persons.cpp
# End Source File
# Begin Source File

SOURCE=.\DMS\Search.cpp
# End Source File
# Begin Source File

SOURCE=.\DMS\Study.cpp
# End Source File
# Begin Source File

SOURCE=.\DMS\Trait.cpp
# End Source File
# Begin Source File

SOURCE=.\DMS\Variate.cpp
# End Source File
# End Group
# Begin Group "IMS_SOURCE"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\INV\Lot.cpp
# End Source File
# End Group
# Begin Group "ENCRYPT"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ENCRYPT\Encrypt.cpp
# End Source File
# Begin Source File

SOURCE=.\ENCRYPT\md5.cpp
# End Source File
# End Group
# Begin Group "IPMS_SOURCE"

# PROP Default_Filter ".cpp"
# Begin Source File

SOURCE=.\IPMS\ipms.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=..\Codbc\Codbc.cpp
# End Source File
# Begin Source File

SOURCE=.\ICIS32.cpp
# End Source File
# Begin Source File

SOURCE=.\icis32.def
# End Source File
# Begin Source File

SOURCE=..\Codbc\ODBCTools.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Group "GMS_HEADER"

# PROP Default_Filter ".h"
# Begin Source File

SOURCE=.\GMS\atributs.h
# End Source File
# Begin Source File

SOURCE=.\GMS\bibrefs.h
# End Source File
# Begin Source File

SOURCE=.\GMS\changes.h
# End Source File
# Begin Source File

SOURCE=.\GMS\germplsm.h
# End Source File
# Begin Source File

SOURCE=.\GMS\instln.h
# End Source File
# Begin Source File

SOURCE=.\GMS\lists.h
# End Source File
# Begin Source File

SOURCE=.\GMS\location.h
# End Source File
# Begin Source File

SOURCE=.\GMS\methods.h
# End Source File
# Begin Source File

SOURCE=.\GMS\misc.h
# End Source File
# Begin Source File

SOURCE=.\GMS\names.h
# End Source File
# Begin Source File

SOURCE=.\GMS\udfield.h
# End Source File
# Begin Source File

SOURCE=.\GMS\user.h
# End Source File
# End Group
# Begin Group "TOOLS_HEADER"

# PROP Default_Filter "*.h"
# Begin Source File

SOURCE=.\Tools\wildcards.h
# End Source File
# End Group
# Begin Group "DMS_HEADER"

# PROP Default_Filter ".h"
# Begin Source File

SOURCE=.\DMS\Data.h
# End Source File
# Begin Source File

SOURCE=.\DMS\Factor.h
# End Source File
# Begin Source File

SOURCE=.\DMS\Interface.h
# End Source File
# Begin Source File

SOURCE=.\DMS\Level.h
# End Source File
# Begin Source File

SOURCE=.\DMS\Persons.h
# End Source File
# Begin Source File

SOURCE=.\DMS\Search.h
# End Source File
# Begin Source File

SOURCE=.\DMS\Study.h
# End Source File
# Begin Source File

SOURCE=.\DMS\Trait.h
# End Source File
# Begin Source File

SOURCE=.\DMS\Variate.h
# End Source File
# End Group
# Begin Group "IMS_HEADER"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\INV\Lot.h
# End Source File
# End Group
# Begin Group "ENCRYPT_HEADER"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ENCRYPT\Encrypt.h
# End Source File
# Begin Source File

SOURCE=.\ENCRYPT\md5.h
# End Source File
# End Group
# Begin Group "IPMS"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\IPMS\ipms.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\Codbc\CODBC.h
# End Source File
# Begin Source File

SOURCE=.\ICIS32.h
# End Source File
# Begin Source File

SOURCE=..\Codbc\ODBCTools.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\ICIS32.rc
# End Source File
# End Group
# End Target
# End Project
