/*****************************************************************************
* Funzioni                  : Funzioni per la gestione AGS
* Data prima Implementazione: 24.04.2002 16:00
*****************************************************************************/
#ifndef BTSerialeH
#define BTSerialeH

#include "CPort.hpp"

// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the DLL_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// DLL_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.

#ifdef DLL_EXPORTS
    #define DLL_API __declspec(dllexport)
#else
    #define DLL_API __declspec(dllimport)
#endif

DLL_API enum BT_SERIAL_E
{
    BT_SERIAL_OK    = 0,
    BT_SERIAL_ERROR = 1
};


//---------------------------------------------------------------------------
#ifdef __cplusplus
    extern "C" {
#endif
//---------------------------------------------------------------------------

DLL_API int __stdcall   BT100_SetMacSlaveOnBT100Master(int iSeriale, const char *pszNewMAcAddress);    //tuttounito

//---------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//---------------------------------------------------------------------------

#endif
