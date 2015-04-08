//////////////////////////////////////////////////////////////////////////////
// .NET Specific Header
//////////////////////////////////////////////////////////////////////////////
#pragma warning (disable: 4278)
#include <string.h>
#import <mscorlib.tlb> raw_interfaces_only
//#import "mscorlib.tlb"

//#import "C:\ICIS32\Library\bin\Debug\Library.tlb" no_namespace named_guids

// Ignoring the server namespace and using named guids:
#if defined (USINGPROJECTSYSTEM)
#import "..\Library\Library.tlb" no_namespace named_guids
#else  // Compiling from the command line, all files in the same directory
#import "Library.tlb" no_namespace named_guids
#endif  

using namespace std;

//////////////////////////////////////////////////////////////////////////////
// End .NET Specific Header
//////////////////////////////////////////////////////////////////////////////
