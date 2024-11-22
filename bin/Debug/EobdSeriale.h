/*****************************************************************************
* Progetto                  : Libreria funzioni comuni SW800
* Funzioni                  : Funzioni per la gestione protocollo FAST
*
* File                      : $RCSfile: AgsEOBD.h,v $
* Autore modifica           : $Author: fontanaleonardo $


*****************************************************************************/
#ifndef EobdSerialeH
#define EobdSerialeH

#include "CPort.hpp"

#include <stdio.h>

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

//---------------------------------------------------------------------------
#ifdef __cplusplus
    extern "C" {
#endif

//------------------------------    ENUM    --------------------------------

// Fast code supportati
typedef enum {
    EOBD_PID_RPMOLD,                            //  0

    EOBD_PID_MIL_STATUS_01,                     //  1
    EOBD_PID_TEMP_05,                           //  2
    EOBD_PID_RPM_0C,                            //  3
    EOBD_PID_OXYGEN_13,                         //  4

    EOBD_PID_SPROBE_14,                         //  5
    EOBD_PID_SPROBE_16,                         //  6
    EOBD_PID_SPROBE_18,                         //  7
    EOBD_PID_SPROBE_1A,                         //  8

    EOBD_PID_EOBD_STATUS_1C,                    //  9
    EOBD_PID_OXYGEN_1D,                         // 10
    EOBD_PID_KM_MILON_21,                       // 11

    EOBD_PID_BPROBE_24,                         // 12
    EOBD_PID_BPROBE_26,                         // 13
    EOBD_PID_BPROBE_28,                         // 14
    EOBD_PID_BPROBE_2A,                         // 15

    EOBD_PID_BPROBE_34,                         // 16
    EOBD_PID_BPROBE_36,                         // 17
    EOBD_PID_BPROBE_38,                         // 18
    EOBD_PID_BPROBE_3A,                         // 19

    EOBD_PID_PED_ACCEL_49,                      // 20
    EOBD_PID_TIME_MILON_4D,                     // 21
    EOBD_PID_TEMP_67,                           // 22
    EOBD_PID_TEMP_6701,                         // 23
    EOBD_PID_AVG_AGENT_85,                      // 24

    EOBD_PID_RDNSUPP_FE,                        // 25
    EOBD_PID_RDNCMPL_FF,                        // 26

    EOBD_PID_DIAGNOSI_INFO,                     // 27

    EOBD_PID_CVN,                               // 28
    EOBD_PID_CALID,                             // 29
    EOBD_PID_VIN,                               // 30

    EOBD_PID_ECUADDRESS,                        // 31

    EOBD_PID_TOTAL_DISTANCE_TRAVELED_1701,      // 32
    EOBD_PID_TOTAL_FUEL_CONSUMED_1702,          // 33

    EOBD_PID_TOTAL_DISTANCE_ENGINE_OFF_1A01,    // 34
    EOBD_PID_TOTAL_DISTANCE_ENGINE_RUN_1A02,    // 35
    EOBD_PID_TOTAL_DISTANCE_SELECTABLE_1A03,    // 36

    EOBD_PID_TOTAL_FUEL_CHARGE_DEPLETING_1B01,  // 37
    EOBD_PID_TOTAL_FUEL_SELECTABLE_CHARGE_1B02, // 38
    EOBD_PID_TOTAL_GRID_ENERGY_BATTERY_1C01,    // 39

    EOBD_TOTAL_PID                              // 40 TOTAL
} EOBD_PID_E;

typedef enum {
    EOBD_SERIAL_VOID                = -1,
    EOBD_SERIAL_OK                  = 0,

    EOBD_SERIAL_ERROR               = 1,
    EOBD_SERIAL_BAD_PARAMETER       = 2,
    EOBD_SERIAL_MISSING_PAYMENT     = 3,
    EOBD_SERIAL_MISSING_ENGINEFILE  = 4
} EOBD_SERIAL_E;

typedef enum {
    EOBD_DEVICE_NONE            = 0,

    EOBD_DEVICE_FASTBOX,
    EOBD_DEVICE_ST6000,
    EOBD_DEVICE_BTOUCH,
    EOBD_DEVICE_FTOUCH,
    EOBD_DEVICE_BTP1000,
    EOBD_DEVICE_CONNEX,
    EOBD_DEVICE_MGT300EVO

} EOBD_DEVICE_E;

typedef enum {
    EOBD_MODULE_NONE            = 0x00,

    EOBD_MODULE_ASIA            = 0x01,
    EOBD_MODULE_MULTIPLEXER1    = 0x02,
    EOBD_MODULE_MULTIPLEXER2    = 0x03

} EOBD_MODULE_E;

// Test

// VL
//  EOBD_PROTOCOL_CANISO15765,      // 3-4   VL
//  EOBD_PROTOCOL_ISO9141KW2000,    // 5-6-7 VL
//  EOBD_PROTOCOL_J1850PWM,         // 8     VL
//  EOBD_PROTOCOL_J1850VPW,         // 9     VL

// PL non EURO VI
//  EOBD_PROTOCOL_J1939,            // 2     PL lors VI
//  EOBD_PROTOCOL_CANISO15765,      // 3-4   PL lors VI
//  EOBD_PROTOCOL_ISO9141KW2000,    // 5-6-7 PL lors VI
//  EOBD_PROTOCOL_J1850PWM,         // 8     PL lors VI
//  EOBD_PROTOCOL_J1850VPW,         // 9     PL lors VI

// PL EURO VI
//  EOBD_PROTOCOL_ISO27145,         // 1     PL Euro VI
//  EOBD_PROTOCOL_J1939,            // 2     PL Euro VI
//  EOBD_PROTOCOL_CANISO15765,      // 3-4   PL Euro VI


typedef enum {
    EOBD_PROTOCOL_ISO27145,         // 1
    EOBD_PROTOCOL_J1939,            // 2

    EOBD_PROTOCOL_CANISO15765,      // 3-4
    EOBD_PROTOCOL_ISO9141,          // 5-6-7
    EOBD_PROTOCOL_KW2000,           // 5-6-7

    EOBD_PROTOCOL_J1850PWM,         // 8
    EOBD_PROTOCOL_J1850VPW,         // 9

    EOBD_PROTOCOL_ALLCAR,                   // Only for connex
    EOBD_PROTOCOL_ELECTRICAL_CONTINUITY     // Test di continuita
} EOBD_PROTOCOL_E;


// Tipo collegamento SCANTOOL
typedef enum {
    EOBD_TEMPPID_AUTODETECT,

    EOBD_TEMPPID_0500,
    EOBD_TEMPPID_6700,
    EOBD_TEMPPID_6701,

    EOBD_TEMPPID_UNDEF
} EOBD_TEMPPID_E;

//-------------------------    STRUTTURE    --------------------------------
// Flash info
typedef struct {
    char            szVersionSoftware[32];
    char            szSerialNumber[32];
    char            szDevice[32];
    char            szDateTime[32];
    unsigned char   DeviceType;
} EOBD_FLASHINFO_S;

typedef struct {
    char            szDate[32];
    char            szHour[32];
    char            szVersion[32];
} EOBD_WHOINFO_S;


//** DLL_API ULONG ulDataInDLL;

DLL_API bool __stdcall EOBD_LoadDLL(int iSeriale);                          // 1 OK
DLL_API void __stdcall EOBD_VersionDLL(char *pszVersion);
DLL_API bool __stdcall EOBD_EngineVersion(char *pszVersion, int iEngine);   // 1 OK
DLL_API void __stdcall EOBD_UnloadDLL(void);

DLL_API bool __stdcall EOBD_Open(void);                                     // 1 OK
DLL_API void __stdcall EOBD_Close(void);
DLL_API int  __stdcall EOBD_FlashInfo(EOBD_FLASHINFO_S *flashInfo);

DLL_API int  __stdcall EOBD_FlashResetNoWait(void);

// TODO input diesel o benzina
DLL_API int  __stdcall EOBD_TestProtocol(int eobdProtocol, int eobdDevice);
DLL_API void __stdcall EOBD_GetProtocol(char *pszProtocol);

DLL_API int  __stdcall EOBD_StatusMIL(int *piMIL);
DLL_API void __stdcall EOBD_ActiveIncDelay(bool bEnable, unsigned long dwDelay);
DLL_API int  __stdcall EOBD_ReadParameter(char *pszParametro, int iIndex);
DLL_API void __stdcall EOBD_ReadVIN(char *pszParametro);
DLL_API void __stdcall EOBD_ReadCALID(char *pszParametro);

DLL_API int  __stdcall EOBD_GetErrors(int *iNrErrori, char *buff, EOBD_PROTOCOL_E eobdProtocol);

DLL_API int  __stdcall EOBD_FastRpm(int *pRpm, bool bEnablePID0C);      // 0 OK
DLL_API int  __stdcall EOBD_FastTemp(int *iTemp, int eOBDTempPID);      // 0 OK
DLL_API int  __stdcall EOBD_FastSupportFast(int *pPID);

DLL_API int  __stdcall EOBD_StopPeriodicRpmTemp(void);                  // D0
DLL_API int  __stdcall EOBD_RestartPeriodicRpmTemp(void);               // D1

DLL_API void __stdcall EOBD_EnableLog(bool bEnable);
DLL_API void __stdcall EOBD_EnableLogECU(bool bEnable);
//---------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//---------------------------------------------------------------------------

#endif

