/*****************************************************************************
* Progetto                  : Programma analisi dei Fumi su sistema SW-800
* Funzioni                  : Funzioni per la gestione delle porte seriali
* Versione                  : 2.000
* File                      : OpaSeriale.h
* Autore                    : Leonardo Fontana
* Data                      : 21.05.2003  09:30
*****************************************************************************/
#ifndef OpaSerialeH
#define OpaSerialeH

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

//---------------------------------------------------------------------------
#ifdef __cplusplus
    extern "C" {
#endif
//---------------------------------------------------------------------------

struct OPA_STATUS_S
{
    unsigned char   byte_status1;
    unsigned char   byte_status2;
    unsigned char   byte_error1;
    unsigned char   byte_error2;

    // S1
    unsigned char   warming_up;             // bit0
    unsigned char   zero_perf;              // bit1
    // unsigned char bit2 ---
    unsigned char   heaters;                // bit3
    unsigned char   cooling_fans;           // bit4
    unsigned char   led_on;                 // bit5
    unsigned char   cleaning_fans;          // bit6
    unsigned char   solenoid_zero;          // bit7

    // S2
    unsigned char   zero_request;           // bit0
    unsigned char   history_perf;           // bit1
    unsigned char   autocal_current_led;    // bit2
    unsigned char   history_start;          // bit3
    unsigned char   history_ann_perf;       // bit4 virtuel created by DLL
    // unsigned char bit5 ---
    // unsigned char bit6 ---
    // unsigned char bit7 ---

    // E1
    unsigned char   error_eeprom_notworking;
    unsigned char   error_eeprom_corrupt;
    unsigned char   fan_emitter_blocked;
    unsigned char   fan_receiver_blocked;
    unsigned char   heaters_notworking;
    unsigned char   error_high_temperature;
    unsigned char   error_lowpower;
    unsigned char   error_highpower;

    // E2
    unsigned char   error_zero;
    unsigned char   error_lummax_low;
    unsigned char   error_lumzero_high;
    unsigned char   error_lummax_void;
    unsigned char   error_calibration;
    unsigned char   error_filter;
    unsigned char   error_thermostat;
    unsigned char   error_autocal_led;
};

struct OPA_SMOKEVALUE_S
{
    int         iRpm;
    int         iReserved1;

    double      dOpacity_k;
    double      dOpacity_p;

    char        szRpm[8];
    char        szOpacity_k[8];
    char        szOpacity_p[8];

    double      dValue_k[12];        // array
    double      dValue_p[12];        // array

    int         iValues;
    int         iReserved2;

    double      dPressione[12];
    double      dOpacitaPreFiltro[12];
    int         igRpm[12];
};

struct OPA_ANALOGVALUE_S
{
    char        szOilTemp[8];
    char        szSmokeTemp[8];
    char        szChamberTemp[8];
    char        szAmbientTemp[8];
    char        szChamberPressure[8];
    char        szPowerBattery[8];
};

struct  OPA_PRIMARYVALUE_S
{
    char        szOpacity[8];

    char        szOilTemp[8];
    char        szSmokeTemp[8];
    char        szChamberTemp[8];
    char        szAmbientTemp[8];
    char        szOffsetTemp[8]; //LED
    char        szChamberPressure[8];
    char        szPowerBattery[8];
};

struct OPA_PEAKVALUE_S
{
    // int     rpm;
    double      dOpacity_k;
    // double  opacita_p;
    // char    vrpm[8];
    char        szOpacity_k[8];
    // char    vopacita_p[8];
};

enum OPA_SERIAL_E
{
    OPA_SERIAL_OK        = 0,
    OPA_SERIAL_VOID      = 1,
    OPA_SERIAL_NAK       = 2,
    OPA_SERIAL_BUSY      = 3,
    OPA_SERIAL_FRAME     = 4,
    OPA_SERIAL_ANN_ERROR = 5
};

enum OPA_CONFIG_E
{
    rcINTEGRATA,  // 0
    rcMANUALE,    // 1
    rcEOBD,       // 2
    rcMCTCNET,    // 3
    rcSERIALE,    // 4
    rcNUMEROCONF  // TOTALE
};

//---------------------------------------------------------------------------
// Classe specifica per gestire protocollo omniBUS per opacimetri
//---------------------------------------------------------------------------

DLL_API bool __stdcall OPA_LoadDLL(int iSeriale);
DLL_API void __stdcall OPA_VersionDLL(char *pszVersion, char *pszChksum);
DLL_API void __stdcall OPA_UnloadDLL(void);

DLL_API bool __stdcall OPA_Open(void);
DLL_API void __stdcall OPA_Close(void);

DLL_API int  __stdcall OPA_CheckAnalizer(void);
DLL_API int  __stdcall OPA_Reset(void);
DLL_API int  __stdcall OPA_Status(OPA_STATUS_S *opaStatus);
DLL_API int  __stdcall OPA_Values(OPA_SMOKEVALUE_S *opaVal);

DLL_API int  __stdcall OPA_PeakValues(OPA_SMOKEVALUE_S *opaPeak);
#ifdef IDE_ENABLEIPERCONNESSIONE
DLL_API int  __stdcall OPA_OpacityPeakValueANN(OPA_PEAKVALUE_S *opaPeak, OPA_STATUS_S *opaStatus, bool bCamionSelection);
#endif

DLL_API int  __stdcall OPA_HistoryValues(const char *num_sample, char *opa_p, char *opa_k, char *rpm);
DLL_API int  __stdcall OPA_AnalogValues(OPA_ANALOGVALUE_S *opaAux);
DLL_API int  __stdcall OPA_ResetPeakValues(const char *opa_p, const char *opa_k, const char *rpm);

DLL_API int  __stdcall OPA_HistoryActivation(const char *opa_p, const char *opa_k, const char *giri, const char *num);

DLL_API int  __stdcall OPA_RCSelection(const char *costant);
DLL_API int  __stdcall OPA_BackupCalibrations(void);
DLL_API int  __stdcall OPA_RestoreCalibrations(void);
DLL_API int  __stdcall OPA_ResetCalibrations(void);
DLL_API int  __stdcall OPA_Autozero(void);
DLL_API int  __stdcall OPA_OpacityCalibration(void);
DLL_API int  __stdcall OPA_PowerCalibration(const char *value);
DLL_API int  __stdcall OPA_PressureCalibration(const char *cal_point, const char *value);
DLL_API int  __stdcall OPA_SmokeCalibration(const char *cal_point, const char *value);
DLL_API int  __stdcall OPA_OilCalibration(const char *cal_point, const char *value);
DLL_API int  __stdcall OPA_BitOpacityValue(OPA_PRIMARYVALUE_S *valPrim);
DLL_API int  __stdcall OPA_BitAnalogValues(OPA_PRIMARYVALUE_S *valPrim);
DLL_API int  __stdcall OPA_RdDigitalInput(void);
DLL_API int  __stdcall OPA_RdDigitalOutput(void);
DLL_API int  __stdcall OPA_WrDigitalOutput(unsigned char mask, unsigned char outstat);
DLL_API int  __stdcall OPA_RdFilter(char *buff);
DLL_API int  __stdcall OPA_WrFilter(const char *value);
DLL_API int  __stdcall OPA_RdPowerLimits(char *vmin, char *vmax);
DLL_API int  __stdcall OPA_WrPowerLimits(const char *vmin, const char *vmax);
DLL_API int  __stdcall OPA_RdChamberTemperature(char *temperatura);
DLL_API int  __stdcall OPA_WrChamberTemperature(const char *temperatura);
DLL_API int  __stdcall OPA_RdRC(char *k1, char *k2);
DLL_API int  __stdcall OPA_WrRC(const char *value1, const char *value2);
DLL_API int  __stdcall OPA_RdChamberLength(char *lunghezza);
DLL_API int  __stdcall OPA_WrChamberLength(const char *lunghezza);
DLL_API int  __stdcall OPA_RdRpmCounterMode(char *contagiri);
DLL_API int  __stdcall OPA_WrRpmCounterMode(const char *contagiri);
DLL_API int  __stdcall OPA_RdSerialNr(char *buff);
DLL_API int  __stdcall OPA_WrSerialNr(const char *code);
DLL_API int  __stdcall OPA_RdCalibrationDate(char *buff);
DLL_API int  __stdcall OPA_WrCalibrationDate(const char *data);
DLL_API int  __stdcall OPA_RdVerSoftware(char *buff);

DLL_API int  __stdcall OPA_Version(char *veropa, char *checkopa);
DLL_API int  __stdcall OPA_Revision(char *pszRevision, char *pszZone, char *pszDateRevision);
DLL_API int  __stdcall OPA_RevisionDLL(char *pszRevision, char *pszDateRevision);

DLL_API int  __stdcall OPA_RdUserData(char *buff);
DLL_API int  __stdcall OPA_WrUserData(const char *data);
DLL_API int  __stdcall OPA_RdRCCompensation(char *k1, char *k2);
DLL_API int  __stdcall OPA_WrRCCompensation(const char *value1, const char *value2);
DLL_API int  __stdcall OPA_RdGraphPerc(char *opa_p[], char *giri[]);
DLL_API int  __stdcall OPA_RdGraphK(char *opa_k[], char *giri[]);
DLL_API int  __stdcall OPA_RdAutozeroTime(char *time);
DLL_API int  __stdcall OPA_WrAutozeroTime(const char *time);

DLL_API int  __stdcall OPA_RdBlockHistoryValues(const char *num_sample, char *opa_p[], char *opa_k[], char *giri[]);
#ifdef IDE_ENABLEIPERCONNESSIONE
DLL_API int  __stdcall OPA_RdBlockHistoryValuesANN(const char *num_sample, char * const pszOpaPreFiltro[], char * const pszOpaK[], char * const pszGiri[],  char * const pszPressione[],  char * const pszOpaAD[]);
#endif

DLL_API int  __stdcall OPA_RdAllData(OPA_SMOKEVALUE_S *fumVal, OPA_SMOKEVALUE_S *fumValPeak, OPA_ANALOGVALUE_S *valAux, OPA_STATUS_S *opaStatus, const char *giri, const char *temp);
#ifdef IDE_ENABLEIPERCONNESSIONE
DLL_API int  __stdcall OPA_RdAllDataANN(OPA_SMOKEVALUE_S *fumVal, OPA_ANALOGVALUE_S *valAux, OPA_STATUS_S *opaStatus, char *pszRevision);
#endif

#ifdef IDE_ENABLEIPERCONNESSIONE
DLL_API int  __stdcall OPA_RdWrPressureTrigger(bool bEnableRead, char * const pszSoglia);
#endif

DLL_API int  __stdcall OPA_RdIdentification(char *marca, char *modello, char *nromol);
DLL_API int  __stdcall OPA_WrIdentification(const char *marca, const char *modello, const char *nromol);
DLL_API int  __stdcall OPA_WrRpm(const char *giri);
DLL_API int  __stdcall OPA_WrRpmTemp(const char *giri, const char *temp);
DLL_API int  __stdcall OPA_RdSCU(char *pszOraData);
DLL_API int  __stdcall OPA_WrSCU(const char *pszOraData);
DLL_API int  __stdcall OPA_RunIn(void);
DLL_API int  __stdcall OPA_EEpromValues(char *nn, char *buff);
DLL_API int  __stdcall OPA_RdSCUDaTeTime(char *OraData);
DLL_API int  __stdcall OPA_WrSCUDaTeTime(const char *OraData);

DLL_API int  __stdcall OPA_GetDemo(void);
DLL_API void __stdcall OPA_EnableLog(bool bEnable);

//-------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//-------------------------------------------------------------------------

#endif
