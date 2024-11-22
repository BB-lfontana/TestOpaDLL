/*****************************************************************************
* Funzioni                  : Funzioni per la gestione delle porte seriali AGS
* Versione                  : 2.000
*****************************************************************************/
#include <vcl.h>
#pragma hdrstop

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static int Demo;

#define DLL_EXPORTS
#include "OpaSeriale.h"

#include "Matem.h"
#ifdef IDE_ENABLEIPERCONNESSIONE
    #include "nnf.h"
#endif

//---------------------------------------------------------------------------
#pragma package(smart_init)

ULONG               ulDataInDLL;

#define UTAC        ( 1 )

#if UTAC
    #define             OPA_DLLVERSION         "1.006"
#else
    #define             OPA_DLLVERSION         "1.008"
#endif

//static TCHAR      szPathDLL[_MAX_PATH + 1];
static AnsiString   aPathDLL;

// Property
OPA_CONFIG_E        RpmConfig;

const unsigned long COM_TIMEOUT     =  1000UL;  //  1 secondo
const int           WIRE_TIMEOUT    = 25000;    // 25 secondi
const int           NUM_TRIES       =     5;    // numero tentativi di comunicazione

// Comandi ASCII di controllo comunicazione seriale
const BYTE          STX             = 0x02;
const BYTE          ETX             = 0x03;
const BYTE          EOT             = 0x04;
const BYTE          ENQ             = 0x05;
const BYTE          ACK             = 0x06;
const BYTE          CR              = 0x0D;
const BYTE          DLE             = 0x10;
const BYTE          NAK             = 0x15;
const BYTE          ETB             = 0x17;
const BYTE          ESC             = 0x1B;
const BYTE          BUSY            = 0x7F;

#ifdef IDE_ENABLEIPERCONNESSIONE

#define INPUT_ANN   (   128 )
#define DIMARRAY    ( 32768 )

static float            m_rawData[INPUT_ANN];

static int              m_iNdx;

static AnsiString       m_aTime[DIMARRAY];
static double           m_dInPOpaANN[DIMARRAY];
static double           m_vDataFilteredHP[DIMARRAY];
static double           m_vDataFilteredHPLP[DIMARRAY];
                        
static double           m_HPF_Alfa;      // costanti
static double           m_LPF_Beta;      // costanti
                        
static double           m_DataRawOld;
                        
static double           m_PiccoFiltratoHP;
static double           m_PiccoFiltratoHPLP;
static double           m_DataFilteredHP;
static double           m_DataFilteredHPLP;
                        
static int              m_RiseStart;
static int              m_RiseEnd;
static int              m_DescEnd;
static int              m_HPLPMax;
                        
// Paramnetri rete neurale
static double           m_dPeakKHP;
static double           m_dSomma;
static int              m_itSalita;
static int              m_itDurata;
static double           m_dPeakKHPLP;
                        
static unsigned char    m_ucHistoryANNPerf;
static bool             m_bANNOpacityPeakChoice;
#endif

// Parametri seriale
enum TBBParity
{
    tbbPAR_NONE         = 0x01,
    tbbPAR_ODD          = 0x02,     // odd parity
    tbbPAR_EVEN         = 0x04,     // even parity
    tbbSTOPBIT_1        = 0x10,     // 1 bit stop
    tbbSTOPBIT_2        = 0x20      // 2 bit stop
};

//---------------------------------------------------------------------------
// Classe specifica per gestire protocollo omniBUS per opacimetri
//---------------------------------------------------------------------------

class TOpaSeriale : public TComponent
{
private:
    // Lunghezza dei buffer di ricezione e trasmissione
    #define BUFFER_OPA 256

    AnsiString      COM_SETADD;
    AnsiString      COM_RESET;
    AnsiString      COM_STATUS;
    AnsiString      COM_VALIST;
    AnsiString      COM_VALPIC;
    AnsiString      COM_VALST;
    AnsiString      COM_VALANA;
    AnsiString      COM_RESVALP;
    AnsiString      COM_ATT_STO;
    AnsiString      COM_SEL_RC;
    AnsiString      COM_BKPTAR;
    AnsiString      COM_RSTTAR;
    AnsiString      COM_RESTAR;
    AnsiString      COM_AUTOZERO;
    AnsiString      COM_CAL_OPA;
    AnsiString      COM_CAL_ALIM;
    AnsiString      COM_CAL_PRES;
    AnsiString      COM_CAL_FUMI;
    AnsiString      COM_CAL_OLIO;
    AnsiString      COM_VALP_OPA;
    AnsiString      COM_VALP_ANA;
    AnsiString      COM_VALIN_DIGI;
    AnsiString      COM_VALOUT_DIGI;
    AnsiString      COM_SETOUT_DIGI;
    AnsiString      COM_RD_FILTRO;
    AnsiString      COM_WR_FILTRO;
    AnsiString      COM_RD_SVBAT;
    AnsiString      COM_WR_SVBAT;
    AnsiString      COM_RD_TCAMERA;
    AnsiString      COM_WR_TCAMERA;
    AnsiString      COM_RD_RC;
    AnsiString      COM_WR_RC;
    AnsiString      COM_RD_LCAMERA;
    AnsiString      COM_WR_LCAMERA;
    AnsiString      COM_RD_FATTRPM;
    AnsiString      COM_WR_FATTRPM;
    AnsiString      COM_RD_NRSERIE;
    AnsiString      COM_WR_NRSERIE;
    AnsiString      COM_RD_DATACAL;
    AnsiString      COM_WR_DATACAL;
    AnsiString      COM_RD_VERSW;
    AnsiString      COM_RD_USERDATA;
    AnsiString      COM_WR_USERDATA;
    AnsiString      COM_RD_COMP;
    AnsiString      COM_WR_COMP;
    AnsiString      COM_RD_GRAF_P;
    AnsiString      COM_RD_GRAF_K;
    AnsiString      COM_RD_AUTOZERO;
    AnsiString      COM_WR_AUTOZERO;
    AnsiString      COM_BLK_VALST;
    AnsiString      COM_RW_ALLDATA;  //("174"),         // int opa_RwAllData(const char *, const char *);

    // AnsiString   COM_RW_CALAUTH  ("175"),            // int opa_RWDataCalibrazioneAuth(bool, char *);
    // AnsiString   COM_RW_VERAUTH  ("176"),            // int opa_RWDataVerificaAuth(bool, char *);

    AnsiString      COM_RW_TRG_PRES; // ("177"),        // int opa_RWTriggerPressione(bool, char *);
#ifdef IDE_ENABLEIPERCONNESSIONE
    AnsiString      COM_RW_ALLDATA2; //  ("178"),       // int opa_RwAllData2(const char *);
#endif
    AnsiString      COM_RD_REVISION; //  ("179"),       // int ReadRevision(char *, char *, char *);

    AnsiString      COM_RD_IDENT;
    AnsiString      COM_WR_IDENT;
    AnsiString      COM_WR_RPM;
    AnsiString      COM_WR_RPM_TEMP;
    // comandi orologio SCU interno
    AnsiString      COM_RD_SCU;
    AnsiString      COM_WR_SCU;

    // comandi comuni omniBUS
    AnsiString      COM_RUN_IN;
    AnsiString      COM_RD_PARAMETRI;

    // comandi orologio SCU-900
    AnsiString      COM_RD_DATA_ORA;
    AnsiString      COM_WR_DATA_ORA;
    AnsiString      COM_RD_DATI_OFF;
    AnsiString      COM_WR_DATI_OFF;
    AnsiString      COM_RD_ESAMINATORE;
    AnsiString      COM_WR_ESAMINATORE;

    // Property
    bool            FLog;

    int             Demo;

    TComPort       *m_oCom;
    TTimer         *m_tmrWire;
    TBaudRate       m_brBaud;
    TFormatSettings m_FormatSettings;

    int             m_iCom;
    int             m_iBufLen;
    int             m_iTrial;
    bool            m_bCOMTimeout;
    bool            m_bWireTimeout;
    bool            m_bOk;
    BYTE            m_ucTipo;

    unsigned int    m_DigitIn;
    unsigned char   m_DigitOut;

    unsigned int    m_iCount;
    unsigned int    m_iRxEnd;
    char            m_szAdrsOPA[4];
    char            m_szAdrsSCU[4];
    BYTE            m_BufTX[BUFFER_OPA];
    BYTE            m_BufRX[BUFFER_OPA];

    AnsiString      m_aMACAddress;

    void __fastcall PutCom(BYTE dt);
    void __fastcall PutCom(BYTE *data, int len);
    void __fastcall Received(TObject *Sender, int Count);
    void __fastcall ReadReceived(void);
    void __fastcall InitSeriale(void);
    void __fastcall ZCanc(char *str);

    // void __fastcall LostCOM(TObject *Sender);
    void __fastcall LostWireless(TObject *Sender);
    void __fastcall CTSChange(TObject *Sender,  bool OnOff);

    int  __fastcall PutOmniBUS(AnsiString cmd, const char *gpv, const char *prot);
    void __fastcall PutCommand(AnsiString cmd, const char *prot);
    int  __fastcall ChkAnswer(AnsiString cmd, const char *prot);

    void __fastcall CalcBCC(BYTE *pFrame, int iLen);
    bool __fastcall ChkBCCOmniBUS(const BYTE *pFrame, int iLen);
    BYTE __fastcall HexASCII2Bin(const BYTE *pFrame);
    void __fastcall Bin2HexASCII(BYTE num, char *pszASCII);

    static unsigned short __fastcall crc16_add(unsigned char b, unsigned short acc);

    // Property Accessor
    void         __fastcall SetRpmConfig(OPA_CONFIG_E value);
    OPA_CONFIG_E __fastcall GetRpmConfig();

    bool __fastcall WaitWireless(bool bEnableModule);

    // OPACIMETRO
    int  __fastcall Get_Demo(void);

    void __fastcall WriteLogTX(const BYTE *pFrame, int iLen);
    void __fastcall WriteLogRX(int iLen);

protected:

public:
    // Logging
    TStringList    *m_logOPA;

    bool            m_bCTS;

         __fastcall  TOpaSeriale(TComponent* Owner, HWND AppHandle, int Seriale);
         __fastcall ~TOpaSeriale(void);

    bool __fastcall opa_openport(void);
    void __fastcall opa_closeport(void);
    int  __fastcall opa_checkanalizer(void);

    int  __fastcall opa_reset(void);
    int  __fastcall opa_status(OPA_STATUS_S *opaStatus);
    int  __fastcall opa_valoriistantanei(OPA_SMOKEVALUE_S *fumVal);
    int  __fastcall opa_valoripicco(OPA_SMOKEVALUE_S *fumValPeak);
    int  __fastcall opa_opacitavalorepiccoANN(OPA_PEAKVALUE_S *opaPeak, OPA_STATUS_S *opaStatus, bool bCamionSelection);
    int  __fastcall opa_valoristorico(const char *num_sample, char *opa_p, char *opa_k, char *giri);

    int  __fastcall opa_ValoriAnalogiciAusiliari(OPA_ANALOGVALUE_S *ValAux);
    int  __fastcall opa_ResetValoriPicco(const char *opa_p, const char *opa_k, const char *giri);
    int  __fastcall opa_AttivazioneStorico(const char *opa_p, const char *opa_k, const char *giri, const char *num);
    int  __fastcall opa_SelezioneRC(const char *costante);

    int  __fastcall opa_BackupTarature(void);
    int  __fastcall opa_RestoreTarature(void);
    int  __fastcall opa_ResetTarature(void);
    int  __fastcall opa_Autozero(void);

    int  __fastcall opa_CalibrazioneOpacita(void);
    int  __fastcall opa_CalAlimentazione(const char *value);
    int  __fastcall opa_CalibrazionePressione(const char *cal_point, const char *value);
    int  __fastcall opa_CalibrazioneFumi(const char *cal_point, const char *value);
    int  __fastcall opa_CalibrazioneOlio(const char *cal_point, const char *value);

    int  __fastcall opa_ValoreOpacitaPrimario(OPA_PRIMARYVALUE_S *valPrim);
    int  __fastcall opa_ValoriAnalogiciPrimari(OPA_PRIMARYVALUE_S *valPrim);
    int  __fastcall opa_IngressiDigitali(void);
    int  __fastcall opa_UsciteDigitali(void);

    int  __fastcall opa_SetUsciteDigitali(const unsigned char mask, const unsigned char outstat);
    int  __fastcall opa_RdFiltro(char *buff);
    int  __fastcall opa_WrFiltro(const char *value);
    int  __fastcall opa_RdLimEl(char *vmin, char *vmax);
    int  __fastcall opa_WrLimEl(const char *vmin, const char *vmax);
    int  __fastcall opa_RdTempCamera(char *temperatura);
    int  __fastcall opa_WrTempCamera(const char *temperatura);
    int  __fastcall RdRC(char *k1, char *k2);
    int  __fastcall WrRC(const char *value1, const char *value2);
    int  __fastcall opa_RdLunghezzaCamera(char *lunghezza);
    int  __fastcall opa_WrLunghezzaCamera(const char *lunghezza);
    int  __fastcall opa_RdFatContagiri(char *contagiri);
    int  __fastcall opa_WrFatContagiri(const char *contagiri);
    int  __fastcall RdNumeroSerie(char *buff);
    int  __fastcall opa_WrNumeroSerie(const char *buff);
    int  __fastcall RdDataCalibrazione(char *buff);
    int  __fastcall opa_WrDataCalibrazione(const char *buff);
    int  __fastcall opa_RdVerSoftware(char *buff);
    int  __fastcall ReadVersion(char *veropa, char *checkopa);
    int  __fastcall ReadRevision(char *pszRevision, char *pszZone, char *pszDateRevision);
    int  __fastcall opa_RdDatiUtente(char *buff);
    int  __fastcall opa_WrDatiUtente(const char *data);
    int  __fastcall RdComp(char *k1, char *k2);
    int  __fastcall WrComp(const char *k1, const char *k2);
    int  __fastcall opa_RdGrafico_p(char *opa_p[], char *giri[]);
    int  __fastcall opa_RdGrafico_k(char *opa_k[], char *giri[]);

    int  __fastcall opa_RdAutozeroTime(char *time);
    int  __fastcall opa_WrAutozeroTime(const char *time);

    int  __fastcall RwAllData(OPA_SMOKEVALUE_S *fumVal, OPA_SMOKEVALUE_S *fumValPeak, OPA_ANALOGVALUE_S *valAux, OPA_STATUS_S *opaStatus, const char *giri, const char *temp);
    int  __fastcall RwAllDataANN(OPA_SMOKEVALUE_S *fumVal, OPA_ANALOGVALUE_S *valAux, OPA_STATUS_S *opaStatus, char *pszRevision);

    int  __fastcall BlkValoriStorico(const char *num_sample, char *opa_p[], char *opa_k[], char *giri[]);
    int  __fastcall BlkValoriStoricoANN(const char * const pszNrSample,
                                        char * const pszOpaPreFiltro[],
                                        char * const pszOpaK[],
                                        char * const pszGiri[],
                                        char * const pszPressione[],
                                        char * const pszOpaAD[]);


    int  __fastcall opa_RWTriggerPressione(bool bEnableRead, char * const pszSoglia);

    int  __fastcall RdIdentCamera(char *marca, char *modello, char *nromol);
    int  __fastcall opa_WrIdentCamera(const char *marca, const char *modello, const char *nromol);

    int  __fastcall WrGiri(const char *giri);
    int  __fastcall WrGiriTemp(const char *giri, const char *temp);
    int  __fastcall opa_RdScu(char *pszOraData);
    int  __fastcall opa_WrScu(const char *pszOraData);

    // Comandi comuni
    int  __fastcall opa_RunIn(void);
    int  __fastcall opa_RdParametri(char *nn, char *buff);

    // SCU-900
    int  __fastcall scu_WrOraData(const char *OraData);
    int  __fastcall scu_RdOraData(char *OraData);

    // Funzioni Statiche
    static int              __fastcall CalcoloTrigger(double MediaMisure);
    static unsigned short   __fastcall Checksum(AnsiString FileName);

    bool    __fastcall CalcolaValorePiccoANN(OPA_PEAKVALUE_S *opaPeak, bool bCamionSelection);
    double  __fastcall CalculateOpaP43(double dOpaLm, double dLengthOPA);
    double  __fastcall CalculateOpaK(double dOpaPerc);
    double  __fastcall CalculateOpaPerc(double dOpaK);
    double  __fastcall CalculateOpaPercCamera(double dOpaK, double dLengthOPA);
    void    __fastcall DemoValues(OPA_SMOKEVALUE_S *fumVal);

    void            __fastcall SaveLogOPA(void);

    // Property
    __property bool         Log         = { read = FLog,             write = FLog,           default = false};

__published:

};

// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the DLL_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// DLL_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.

static TOpaSeriale *opaSeriale = NULL;

//---------------------------------------------------------------------------
//   Important note about DLL memory management when your DLL uses the
//   static version of the RunTime Library:
//
//   If your DLL exports any functions that pass String objects (or structs/
//   classes containing nested Strings) as parameter or function results,
//   you will need to add the library MEMMGR.LIB to both the DLL project and
//   any other projects that use the DLL.  You will also need to use MEMMGR.LIB
//   if any other projects which use the DLL will be performing new or delete
//   operations on any non-TObject-derived classes which are exported from the
//   DLL. Adding MEMMGR.LIB to your project will change the DLL and its calling
//   EXE's to use the BORLNDMM.DLL as their memory manager.  In these cases,
//   the file BORLNDMM.DLL should be deployed along with your DLL.
//   To avoid using BORLNDMM.DLL, pass string information using "char *" or
//   ShortString parameters.
//   If your DLL uses the dynamic version of the RTL, you do not need to
//   explicitly add MEMMGR.LIB as this will be done implicitly for you
//---------------------------------------------------------------------------

#pragma argsused
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void* lpReserved)
{
/*
    switch (reason)
    {
        case DLL_PROCESS_ATTACH:
            if (lpvReserved)
                MessageBox(NULL,"Process has attached to DLL OPA(Static Load)", "DLLMain",MB_OK);
            else
                MessageBox(NULL,"Process has attached to DLL OPA(Dynamic Load)", "DLLMain",MB_OK);
            break;

        case DLL_THREAD_ATTACH:
            // MessageBox(NULL,"Thread has attached to DLL OPA","DLLMain",MB_OK);
            break;

        case DLL_THREAD_DETACH:
            MessageBox(NULL,"Thread has detached to DLL OPA","DLLMain",MB_OK);
            break;

        case DLL_PROCESS_DETACH:
            MessageBox(NULL,"Process has detached to DLL OPA","DLLMain",MB_OK);
            break;
    }
*/
    switch (reason)
    {
        case DLL_PROCESS_ATTACH:
            TCHAR s[_MAX_PATH+1];
            GetModuleFileName(hinst, s, sizeof(s)/sizeof(s[0]) );

            // TCHAR path[_MAX_PATH+1];
            // GetModuleFileName(GetModuleHandle("OpaCom.dll"), path, sizeof(path)/sizeof(path[0]));

            aPathDLL = ExtractFilePath(s);
            break;
    }

    return 1;
}

// Inizializzazione DLL seriale
#ifdef DLL_EXPORTS
//--------------------------------------------------------------------------
DLL_API bool __stdcall OPA_LoadDLL(int iSeriale)
{
    opaSeriale = new TOpaSeriale(NULL, NULL, iSeriale);
    if (opaSeriale)
    {
        return true;
    }

    return false;
}
#endif

// Restituisce vesrione DLL
//--------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API void __stdcall OPA_VersionDLL(char *pszVersion, char *pszChksum)
{
    AnsiString      aChksum;
    AnsiString      aFile;
    unsigned short  usChksum;

    strcpy(pszVersion, OPA_DLLVERSION);

    aFile       = aPathDLL + "OpaCom.dll";
#if UTAC
    usChksum    = 0x0569;
#else
    usChksum    = TOpaSeriale::Checksum(aFile);
#endif
    aFile       = aPathDLL + "NetPL.txt";
    usChksum   += TOpaSeriale::Checksum(aFile);

    aFile       = aPathDLL + "NetVL.txt";
    usChksum   += TOpaSeriale::Checksum(aFile);

    aChksum     = IntToHex(usChksum, 4);
    strcpy(pszChksum, aChksum.c_str());
}
#endif

// Distruzione oggetto seriale
//--------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API void __stdcall OPA_UnloadDLL(void)
{
    if (opaSeriale != NULL)
    {
        opaSeriale->opa_closeport();

        delete opaSeriale;
        opaSeriale = NULL;
    }
}
#endif

//---------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API void __stdcall OPA_EnableLog(bool bEnable)
{
    if (opaSeriale != NULL)
    {
        opaSeriale->Log = bEnable;
    }
};
#endif


//---------------------------------------------------------------------------
__fastcall TOpaSeriale::TOpaSeriale(TComponent* Owner, HWND AppHandle, int Seriale)
  : TComponent(Owner),
    m_iCom(Seriale),
    m_brBaud(Cport::br9600),
    m_iBufLen(BUFFER_OPA),
    m_iTrial(NUM_TRIES),
    m_bCOMTimeout(false),
    m_bWireTimeout(false),

    COM_SETADD      ("100"), // not implemented
    COM_RESET       ("101"), // int opa_Reset();
    COM_STATUS      ("102"), // int opa_Status();
    COM_VALIST      ("103"), // int opa_ValoriIstantanei();
    COM_VALPIC      ("104"), // int opa_ValoriPicco();
    COM_VALST       ("105"), // int opa_ValoriStorico(const char *, char *, char *, char *);

    COM_VALANA      ("108"), // int opa_ValoriAnalogiciAusiliari();
    COM_RESVALP     ("109"), // int opa_ResetValoriPicco(const char *, const char *, const char *);
    COM_ATT_STO     ("110"), // int opa_AttivazioneStorico(const char *, const char *, const char *, const char *);

    COM_SEL_RC      ("112"), // int opa_Selezione_RC(const char *);

    COM_BKPTAR      ("118"), // int opa_BackupTarature();
    COM_RSTTAR      ("119"), // int opa_RestoreTarature();
    COM_RESTAR      ("120"), // int opa_ResetTarature();
    COM_AUTOZERO    ("121"), // int opa_Autozero();
    COM_CAL_OPA     ("122"), // int opa_CalibrazioneOpacita();
    COM_CAL_ALIM    ("123"), // int opa_CalAlimentazione(const char *);
    COM_CAL_PRES    ("124"), // int opa_CalibrazionePressione(const char *, const char *);
    COM_CAL_FUMI    ("125"), // int opa_CalibrazioneFumi(const char *, const char *);
    COM_CAL_OLIO    ("126"), // int opa_CalibrazioneOlio(const char *, const char *);

    COM_VALP_OPA    ("130"), // int opa_ValoreOpacitaPrimario();
    COM_VALP_ANA    ("131"), // int opa_ValoriAnalogiciPrimari();
    COM_VALIN_DIGI  ("132"), // int opa_IngressiDigitali();
    COM_VALOUT_DIGI ("133"), // int opa_UsciteDigitali();

    COM_SETOUT_DIGI ("135"), // int opa_SetUsciteDigitali(const unsigned char, const unsigned char);

    COM_RD_FILTRO   ("140"), // int opa_RdFiltro(char *);
    COM_WR_FILTRO   ("141"), // int opa_WrFiltro(const char *);
    COM_RD_SVBAT    ("142"), // int opa_RdLimEl(char *, char *);
    COM_WR_SVBAT    ("143"), // int opa_WrLimEl(const char *, const char *);
    COM_RD_TCAMERA  ("144"), // int opa_RdTempCamera(char *);
    COM_WR_TCAMERA  ("145"), // int opa_WrTempCamera(const char *);
    COM_RD_RC       ("146"), // int opa_RdRC(char *, char *);
    COM_WR_RC       ("147"), // int opa_WrRC(const char *, const char *);
    COM_RD_LCAMERA  ("148"), // int opa_RdLunghezzaCamera(char *);
    COM_WR_LCAMERA  ("149"), // int opa_WrLunghezzaCamera(const char *);
    COM_RD_FATTRPM  ("150"), // int opa_RdFatContagiri(char *);
    COM_WR_FATTRPM  ("151"), // int opa_WrFatContagiri(const char *);
    COM_RD_NRSERIE  ("152"), // int opa_RdNumeroSerie(char *);
    COM_WR_NRSERIE  ("153"), // int opa_WrNumeroSerie(const char *);
    COM_RD_DATACAL  ("154"), // int opa_RdDataCalibrazione(char *);
    COM_WR_DATACAL  ("155"), // int opa_WrDataCalibrazione(const char *);
    COM_RD_VERSW    ("156"), // int opa_RdVerSoftware(char *);
    COM_RD_USERDATA ("157"), // int opa_RdDatiUtente(char *);
    COM_WR_USERDATA ("158"), // int opa_WrDatiUtente(const char *);
    COM_RD_COMP     ("159"), // int opa_RdComp(char *, char *);
    COM_WR_COMP     ("160"), // int opa_WrComp(const char *, const char *);
    COM_RD_GRAF_P   ("161"), // int opa_RdGrafico_p(char * [], char * []);
    COM_RD_GRAF_K   ("162"), // int opa_RdGrafico_k(char * [], char * []);

    COM_RD_AUTOZERO ("170"), // int opa_RdAutozeroTime(char *);
    COM_WR_AUTOZERO ("171"), // int opa_WrAutozeroTime(const char *);
    COM_BLK_VALST   ("173"), // int opa_BlkValoriStorico(const char *, char *[], char *[], char *[]);
    COM_RW_ALLDATA  ("174"), // int opa_RwAllData(const char *, const char *);

    // COM_RW_CALAUTH  ("175"),         // int opa_RWDataCalibrazioneAuth(bool, char *);
    // COM_RW_VERAUTH  ("176"),         // int opa_RWDataVerificaAuth(bool, char *);
    COM_RW_TRG_PRES ("177"),            // int opa_RWTriggerPressione(bool, char *);
#ifdef IDE_ENABLEIPERCONNESSIONE
    COM_RW_ALLDATA2 ("178"),            // int opa_RwAllData2(const char *);
#endif
    COM_RD_REVISION ("179"),            // int ReadRevision(char *, char *, char *);

    COM_RD_IDENT    ("271"), // int opa_RdIdentCamera(char *, char *, char *);
    COM_WR_IDENT    ("272"), // int opa_WrIdentCamera(const char *, const char *, const char *);

    COM_WR_RPM      ("280"), // int opa_WrGiri(const char *);
    COM_WR_RPM_TEMP ("281"), // int opa_WrGiriTemp(const char *, const char *);

    // comandi orologio SCU interno
    COM_RD_SCU      ("282"), // int opa_RdScu(char *);
    COM_WR_SCU      ("283"), // int opa_WrScu(const char *);

    // comandi comuni omniBUS
    COM_RUN_IN        ("024"), // int opa_Run_In();
    COM_RD_PARAMETRI  ("043"), // int opa_RdParametri(char *, char *);

    // comandi orologio SCU-900
    COM_RD_DATA_ORA    ("020"), // int scu_WrOraData(const char *);
    COM_WR_DATA_ORA    ("021"), // int scu_RdOraData(char *);
    COM_RD_DATI_OFF    ("900"), // not implemented
    COM_WR_DATI_OFF    ("901"), // not implemented
    COM_RD_ESAMINATORE ("902"), // not implemented
    COM_WR_ESAMINATORE ("903")  // not implemented
{
    Application->Handle     = AppHandle;

    // Default property values
    FLog                = false;

    // Impostazione address omniBUS
    strcpy(m_szAdrsOPA, "10");   //OPA-100
    strcpy(m_szAdrsSCU, "90");   //SCU-900

    m_bCTS                  = false;

    // SCOPO -> Numeri decimali con punto
    GetLocaleFormatSettings(0x0809, m_FormatSettings);  // English (United Kingdom)
    m_FormatSettings.DecimalSeparator   = '.';

    m_FormatSettings.DateSeparator      = '.';          // Data Richiede Punto
    m_FormatSettings.TimeSeparator      = ':';
    m_FormatSettings.ShortDateFormat    = "dd/mm/yyyy"; // / ->DateSeparator Formato Internazionale
    m_FormatSettings.ShortTimeFormat    = "hh:nn";      // : ->TimeSeparator Formato Internazionale

    // APERTURA LOG
    m_logOPA                = new TStringList();

    // Impostazioni Timeout
    m_tmrWire               = new TTimer(this);
    m_tmrWire->Enabled      = false;
    m_tmrWire->Interval     = WIRE_TIMEOUT;  // 25 sec
    m_tmrWire->OnTimer      = LostWireless;
}
//---------------------------------------------------------------------------
void __fastcall TOpaSeriale::InitSeriale(void)
{
    // Creazione componente Seriale
    m_oCom                                  = new TComPort(NULL);
    m_oCom->Port                            = AnsiString("COM") + IntToStr(m_iCom);
    m_oCom->BaudRate                        = m_brBaud;

    // Linee di stato
    m_oCom->FlowControl->ControlDTR         = dtrDisable;
    m_oCom->FlowControl->ControlRTS         = rtsDisable;
    m_oCom->FlowControl->DSRSensitivity     = false;
    m_oCom->FlowControl->FlowControl        = fcNone;
    m_oCom->FlowControl->OutCTSFlow         = false;
    m_oCom->FlowControl->OutDSRFlow         = false;

    // PARITY
    if (m_ucTipo & tbbPAR_NONE)
        m_oCom->Parity->Bits = prNone;        // PARITY NONE
    else if (m_ucTipo & tbbPAR_ODD)
        m_oCom->Parity->Bits = prOdd;         // PARITY ODD
    else if (m_ucTipo & tbbPAR_EVEN)
        m_oCom->Parity->Bits = prEven;        // PARITY EVEN
    // STOP BIT
    if (m_ucTipo & tbbSTOPBIT_1)
        m_oCom->StopBits = sbOneStopBit;
    else if (m_ucTipo & tbbSTOPBIT_2)
        m_oCom->StopBits = sbTwoStopBits;

    // Eventi
    m_oCom->Events.Clear();
    m_oCom->Events = m_oCom->Events << evRxChar << evCTS << evBreak << evError;
    //** m_oCom->Events = m_oCom->Events << evCTS;

    // Buffer e TimeOuts
    m_oCom->Buffer->InputSize               = m_iBufLen;
    m_oCom->Buffer->OutputSize              = m_iBufLen;

    // Impostazione Timeouts scrittura
#ifdef USE_INTERRUPT
    m_oCom->Timeouts->ReadInterval          = -1;
    m_oCom->Timeouts->ReadTotalConstant     = 0;
    m_oCom->Timeouts->ReadTotalMultiplier   = 0;
#else
    m_oCom->Timeouts->ReadInterval          = 5;
    m_oCom->Timeouts->ReadTotalMultiplier   = 0;
    m_oCom->Timeouts->ReadTotalConstant     = 5;
#endif

    m_oCom->Timeouts->WriteTotalConstant    = 100;
    m_oCom->Timeouts->WriteTotalMultiplier  = 0;

    // Impostazione evento di ricezione
    m_oCom->OnRxChar                        = Received;
    m_oCom->OnCTSChange                     = CTSChange;
    m_oCom->SyncMethod                      = smNone;

    // Inizializzazioni variabili
    m_iCount                                = 0;
    Demo                                    = 0;
}

//---------------------------------------------------------------------------
void __fastcall TOpaSeriale::ZCanc(char *str)
{
    unsigned int    ii = 0;

    while(str[ii]=='0' && str[ii+1]!='.' && str[ii+1]!=0)
        str[ii++] = ' ';
}
//---------------------------------------------------------------------------
void __fastcall TOpaSeriale::SetRpmConfig(OPA_CONFIG_E value)
{
    RpmConfig = value;
}
//---------------------------------------------------------------------------
OPA_CONFIG_E __fastcall TOpaSeriale::GetRpmConfig()
{
    return RpmConfig;
}
//---------------------------------------------------------------------------
void __fastcall TOpaSeriale::CTSChange(TObject *Sender, bool OnOff)
{
    // Segnali a logica negata
    // CtsOn = m_bCTS = false -> CTS = 1 SPENTO      LED MODULO ON  OR NOT PRESENT
    // CtsOn = m_bCTS = true  -> CTS = 0 SETTATO(*)  LED MODULO OFF
    m_bCTS = OnOff;
}
//---------------------------------------------------------------------------
bool __fastcall TOpaSeriale::WaitWireless(bool bEnableModule)
{
    bool            bEsito;

    if (Demo == 3)
        return true;

    if (bEnableModule) // Abilitato BlueTooth
    {
        m_bWireTimeout      = false;

        m_tmrWire->Enabled  = true;
        SleepEx(300, false);
        Application->ProcessMessages();
        while (m_bCTS && !m_bWireTimeout)      // MODULO OFF -> ON
        {
            SleepEx(1, false);
            Application->ProcessMessages();
        }
        m_tmrWire->Enabled  = false;

        if (! m_bWireTimeout)
            bEsito = true;
        else
            bEsito = false;
    }
    else             // Abilitato NULL MODEM
    {
        SleepEx(1000, false);   // Simula attesa CTS per chiavette Bluetooth
        Application->ProcessMessages();

        bEsito = true;
    }

    return bEsito;
}
/*
//---------------------------------------------------------------------------
void __fastcall TOpaSeriale::LostCOM(TObject *Sender)
{
    m_tmrCOM->Enabled   = false;
    m_bCOMTimeout       = true;
}
*/
//---------------------------------------------------------------------------
void __fastcall TOpaSeriale::LostWireless(TObject *Sender)
{
    m_tmrWire->Enabled  = false;
    m_bWireTimeout      = true;
}

//---------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API bool __stdcall OPA_Open(void)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_openport();
    }

    return false;
};
#endif

//---------------------------------------------------------------------------
bool __fastcall TOpaSeriale::opa_openport(void)
{
    // PULIZIA LOG
    m_logOPA->Clear();
    m_logOPA->Add(AnsiString("LOG VERSION (") + OPA_DLLVERSION + ")");

    try
    {
        if (m_oCom == NULL)
        {
            InitSeriale();
        }

        m_oCom->Open();
        m_oCom->SetDTR(false);
        m_oCom->ClearBuffer(true, true);
    }
    catch (EComPort &e)
    {
        Demo = 3;

        return false;
    }

//    m_tmrCOM->Enabled   = false;
    m_tmrWire->Enabled  = false;

    return true;
}
//---------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API void __stdcall OPA_Close(void)
{
    if (opaSeriale != NULL)
    {
        opaSeriale->opa_closeport();
    }
};
#endif

//---------------------------------------------------------------------------
void __fastcall TOpaSeriale::opa_closeport(void)
{
    //m_tmrCOM->Enabled  = false;
    m_tmrWire->Enabled = false;

    if (Demo == 3)
    {
        return;
    }

    if (m_oCom != NULL)
    {
        if (m_oCom->Connected == true)
        {
            m_oCom->Close();
        }
    }

    // Salvataggio Log
    if (opaSeriale->Log == true)
    {
        opaSeriale->SaveLogOPA();
    }
}
//---------------------------------------------------------------------------
void __fastcall TOpaSeriale::SaveLogOPA(void)
{
    if (m_logOPA->Count > 1)
    {
        // SALVATAGGIO LOG
        AnsiString aBuf = aPathDLL + "LogOPA.txt";

        if (FileExists(aBuf) == true)
        {
            DeleteFile(aBuf);
        }

        m_logOPA->SaveToFile(aBuf);

        // PULIZIA LOG
        m_logOPA->Clear();
        m_logOPA->Add(AnsiString("LOG VERSION (") + OPA_DLLVERSION + ")");
    }
}
//---------------------------------------------------------------------------
__fastcall TOpaSeriale::~TOpaSeriale(void)
{
    //m_tmrCOM->Enabled  = false;
    m_tmrWire->Enabled = false;

    if (m_oCom)
    {
        m_oCom->OnRxChar     = NULL;
        m_oCom->OnCTSChange  = NULL;

        if (m_oCom->Connected == true)
        {
            m_oCom->Close();
        }

        delete m_oCom;
        m_oCom = NULL;
    }

    // LOG
    delete m_logOPA;
}
//---------------------------------------------------------------------------
void __fastcall TOpaSeriale::Received(TObject *Sender, int iCount)
{
    AnsiString      buffer;
    int             numRead;

    if (m_oCom)
    {
        numRead = m_oCom->ReadStr(buffer, iCount);

        for (int xx = 0; xx < numRead; ++xx, ++m_iCount)
        {
            m_BufRX[m_iCount] = buffer[xx+1];
            if (m_BufRX[m_iCount] == EOT)
            {
               m_iRxEnd = m_iCount;
               m_bOk    = true;
            }
        }
    }
}

//---------------------------------------------------------------------------
void __fastcall TOpaSeriale::ReadReceived(void)
{
    AnsiString      buffer;
    int             numRead;

    if (m_oCom)
    {
        numRead = m_oCom->ReadStr(buffer, 100);

        for (int xx = 0; xx < numRead; ++xx, ++m_iCount)
        {
            m_BufRX[m_iCount] = buffer[xx+1];
            if (m_BufRX[m_iCount] == EOT)
            {
               m_iRxEnd = m_iCount;
               m_bOk    = true;
            }
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TOpaSeriale::PutCom(BYTE data)
{
    if (m_oCom)
    {
        m_oCom->Write(&data, 1);
    }
}
//---------------------------------------------------------------------------
void __fastcall TOpaSeriale::PutCom(BYTE *pData, int iLen)
{
    if (m_oCom)
    {
        int nByteWrite = 0;
        do
        {
            int iWrite = m_oCom->Write(&pData[nByteWrite], iLen - nByteWrite);
            nByteWrite += iWrite;

            if (iWrite == 0)
            {
                return;
            }
        } while (nByteWrite < iLen);
    }
}

/*
******************************************************************************
******************************************************************************
**                                                                          **
**         FUNZIONI PER LA COMUNICAZIONE CON L'ANALIZZATORE OPA-100         **
**                                                                          **
******************************************************************************
******************************************************************************
*/
//---------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_CheckAnalizer(void)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_checkanalizer();
    }

    Demo = 1;

    return Demo;
};
#endif

int __fastcall TOpaSeriale::opa_checkanalizer(void)
{
    int             vret = OPA_SERIAL_VOID;

    if (Demo == 3)
        return 3;

    for (int ii = 0; ii < m_iTrial; ++ii)
    {
        m_bOk           = false;
        m_bCOMTimeout   = false;

        PutCommand(COM_STATUS, m_szAdrsOPA);

        // m_TmrCOM->Enabled   = true;
        unsigned long T0 = GetTickCount();
        while ((GetTickCount()-T0) <= COM_TIMEOUT)
        {
            Application->ProcessMessages();
            SleepEx(1, false);

/*
            if (m_Link != elnkBLUETOOTH)
                ReadReceived();
*/
            if (m_bOk)
                break;
        }
        //m_tmrCOM->Enabled   = false;

        if (m_bOk)  // Frame omniBUS arrivato
        {
            vret = ChkAnswer(COM_STATUS, m_szAdrsOPA); // funzione non bloccante
            if ( (vret == OPA_SERIAL_OK) || vret == OPA_SERIAL_NAK)  // OK - NAK
                break;
            else if (vret == OPA_SERIAL_BUSY)            // BUSY
            {
                SleepEx(1000, false);
                Application->ProcessMessages();
            }
            // else Rx con FRAME errato  > ripeti fino a NUM_TRIES prove
        }
        // else TIMEOUT IN RX           -> ripeti fino a NUM_TRIES prove
    }

    if (vret == OPA_SERIAL_OK) // OPA-100 COLLEGATO
        Demo = 0;
    else             // OPA-100 SCOLLEGATO o MALFUNZIONANTE
        Demo = 1;

    return vret;
}

//------------------------------------------------------------------------------
//---------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_Reset(void)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_reset();
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::opa_reset(void)
{
    char            gpv[20];

    if (Demo)
        return OPA_SERIAL_OK;

    strcpy(gpv, COM_RESET.c_str()); // "101"

    return PutOmniBUS(COM_RESET, gpv, m_szAdrsOPA);
}

//---------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_Status(OPA_STATUS_S *opaStatus)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_status(opaStatus);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::opa_status(OPA_STATUS_S *opaStatus)
{
    char            gpv[20];
    int             vret;

    if (Demo)
    {
        opaStatus->byte_status1 = 0x00;
        opaStatus->byte_status2 = 0x00;
        opaStatus->byte_error1  = 0x00;
        opaStatus->byte_error2  = 0x00;

        vret                    = OPA_SERIAL_OK;
    }
    else
    {
        strcpy(gpv, COM_STATUS.c_str()); // "102"
        vret = PutOmniBUS(COM_STATUS, gpv, m_szAdrsOPA);

        if (vret == OPA_SERIAL_OK)
        {
            opaStatus->byte_status1 = HexASCII2Bin(&m_BufRX[7]);
            opaStatus->byte_status2 = HexASCII2Bin(&m_BufRX[9]);
            opaStatus->byte_error1  = HexASCII2Bin(&m_BufRX[11]);
            opaStatus->byte_error2  = HexASCII2Bin(&m_BufRX[13]);
        }
    }

    // entrambi
    if (vret == OPA_SERIAL_OK)
    {
        // S1
        opaStatus->warming_up               = (BYTE)((opaStatus->byte_status1 & 0x01) >> 0);
        opaStatus->zero_perf                = (BYTE)((opaStatus->byte_status1 & 0x02) >> 1);
        // BYTE bit2 ---
        opaStatus->heaters                  = (BYTE)((opaStatus->byte_status1 & 0x08) >> 3);
        opaStatus->cooling_fans             = (BYTE)((opaStatus->byte_status1 & 0x10) >> 4);
        opaStatus->led_on                   = (BYTE)((opaStatus->byte_status1 & 0x20) >> 5);
        opaStatus->cleaning_fans            = (BYTE)((opaStatus->byte_status1 & 0x40) >> 6);
        opaStatus->solenoid_zero            = (BYTE)((opaStatus->byte_status1 & 0x80) >> 7);

        // S2
        opaStatus->zero_request             = (BYTE)((opaStatus->byte_status2 & 0x01) >> 0);
        opaStatus->history_perf             = (BYTE)((opaStatus->byte_status2 & 0x02) >> 1);
        opaStatus->autocal_current_led      = (BYTE)((opaStatus->byte_status2 & 0x04) >> 2);
        opaStatus->history_start            = (BYTE)((opaStatus->byte_status2 & 0x08) >> 3);
        opaStatus->history_ann_perf         = m_ucHistoryANNPerf;
        // unsigned char  bit4 ---
        // unsigned char  bit5 ---
        // unsigned char  bit6 ---
        // unsigned char  bit7 ---

        // E1
        opaStatus->error_eeprom_notworking  = (BYTE)((opaStatus->byte_error1 & 0x01) >> 0);
        opaStatus->error_eeprom_corrupt     = (BYTE)((opaStatus->byte_error1 & 0x02) >> 1);
        opaStatus->fan_emitter_blocked      = (BYTE)((opaStatus->byte_error1 & 0x04) >> 2);
        opaStatus->fan_receiver_blocked     = (BYTE)((opaStatus->byte_error1 & 0x08) >> 3);
        opaStatus->heaters_notworking       = (BYTE)((opaStatus->byte_error1 & 0x10) >> 4);
        opaStatus->error_high_temperature   = (BYTE)((opaStatus->byte_error1 & 0x20) >> 5);
        opaStatus->error_lowpower           = (BYTE)((opaStatus->byte_error1 & 0x40) >> 6);
        opaStatus->error_highpower          = (BYTE)((opaStatus->byte_error1 & 0x80) >> 7);

        // E2
        opaStatus->error_zero               = (BYTE)((opaStatus->byte_error2 & 0x01) >> 0);
        opaStatus->error_lummax_low         = (BYTE)((opaStatus->byte_error2 & 0x02) >> 1);
        opaStatus->error_lumzero_high       = (BYTE)((opaStatus->byte_error2 & 0x04) >> 2);
        opaStatus->error_lummax_void        = (BYTE)((opaStatus->byte_error2 & 0x08) >> 3);
        opaStatus->error_calibration        = (BYTE)((opaStatus->byte_error2 & 0x10) >> 4);
        opaStatus->error_filter             = (BYTE)((opaStatus->byte_error2 & 0x20) >> 5);
        opaStatus->error_thermostat         = (BYTE)((opaStatus->byte_error2 & 0x40) >> 6);
        opaStatus->error_autocal_led        = (BYTE)((opaStatus->byte_error2 & 0x80) >> 7);
    }

    return vret;
}

//------------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_Values(OPA_SMOKEVALUE_S *fumVal)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_valoriistantanei(fumVal);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::opa_valoriistantanei(OPA_SMOKEVALUE_S *fumVal)
{
    char            gpv[20];
    int             vret;

    if (Demo)
    {
        DemoValues(fumVal);
        SleepEx(200, false);
        vret = OPA_SERIAL_OK;
    }
    else
    {
        // Solo nuovi OPA
        switch (RpmConfig)
        {
            case rcINTEGRATA:
                break;

            case rcEOBD:
            case rcMANUALE:
            case rcMCTCNET:
            case rcSERIALE:
                WrGiri(fumVal->szRpm);
                break;
        }

        strcpy(gpv, COM_VALIST.c_str()); // "103"
        vret = PutOmniBUS(COM_VALIST, gpv, m_szAdrsOPA);
        if (vret == OPA_SERIAL_OK) // OK
        {
            memcpy(fumVal->szOpacity_p, (char*)&m_BufRX[ 7], 4);    // PP.P
            fumVal->szOpacity_p[4]  = '\0';

            memcpy(fumVal->szOpacity_k, (char*)&m_BufRX[11], 4);   // M.MM
            fumVal->szOpacity_k[4]  = '\0';

            memcpy(fumVal->szRpm,       (char*)&m_BufRX[15], 4);   // GGGG
            fumVal->szRpm[4]        = '\0';


            if (fumVal->szOpacity_p[0]  != '-')  fumVal->dOpacity_p = atof(fumVal->szOpacity_p);
            if (fumVal->szOpacity_k[0]  != '-')  fumVal->dOpacity_k = atof(fumVal->szOpacity_k);
            if (fumVal->szRpm[0]        != '-')  fumVal->iRpm       = atoi(fumVal->szRpm);

            ZCanc(fumVal->szOpacity_p);
            ZCanc(fumVal->szOpacity_k);
            ZCanc(fumVal->szRpm);
        }
    }

    return vret;
}

//------------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_PeakValues(OPA_SMOKEVALUE_S *FumValPeak)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_valoripicco(FumValPeak);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::opa_valoripicco(OPA_SMOKEVALUE_S *FumValPeak)
{
    char            gpv[20];
    int             vret;

    if (Demo)
    {
        strcpy(FumValPeak->szOpacity_p, "0.0");
        strcpy(FumValPeak->szOpacity_k, "0.00");
        strcpy(FumValPeak->szRpm,       "0");
        FumValPeak->dOpacity_p  = 0.0;
        FumValPeak->dOpacity_k  = 0.0;
        FumValPeak->iRpm        = 0;

        vret                    = OPA_SERIAL_OK;
    }
    else
    {
        strcpy(gpv, COM_VALPIC.c_str()); // "104"
        vret = PutOmniBUS(COM_VALPIC, gpv, m_szAdrsOPA);
        if(vret == OPA_SERIAL_OK) // OK
        {
            memcpy(FumValPeak->szOpacity_p, (char*)&m_BufRX[ 7], 4); // PP.P
            FumValPeak->szOpacity_p[4] = '\0';

            memcpy(FumValPeak->szOpacity_k, (char*)&m_BufRX[11], 4); // M.MM
            FumValPeak->szOpacity_k[4] = '\0';

            memcpy(FumValPeak->szRpm,       (char*)&m_BufRX[15], 4); // GGGG
            FumValPeak->szRpm[4]       = '\0';

            if (FumValPeak->szOpacity_p[0]  != '-')  FumValPeak->dOpacity_p = atof(FumValPeak->szOpacity_p);
            if (FumValPeak->szOpacity_k[0]  != '-')  FumValPeak->dOpacity_k = atof(FumValPeak->szOpacity_k);
            if (FumValPeak->szRpm[0]        != '-')  FumValPeak->iRpm       = atoi(FumValPeak->szRpm);

            ZCanc(FumValPeak->szOpacity_p);
            ZCanc(FumValPeak->szOpacity_k);
            ZCanc(FumValPeak->szRpm);
        }
    }

    return vret;
}

#ifdef IDE_ENABLEIPERCONNESSIONE

//------------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_OpacityPeakValueANN(OPA_PEAKVALUE_S *opaPeak, OPA_STATUS_S *opaStatus, bool bCamionSelection)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_opacitavalorepiccoANN(opaPeak, opaStatus, bCamionSelection);
    }

    return OPA_SERIAL_VOID;
};
#endif
//                                                           out                         in                      in 
int __fastcall TOpaSeriale::opa_opacitavalorepiccoANN(OPA_PEAKVALUE_S *opaPeak, OPA_STATUS_S *opaStatus, bool bCamionSelection)
{
    char            gpv[32];
    int             vret;

    if (Demo)
    {
        vret = OPA_SERIAL_VOID;
    }
    else
    {
        if (opaStatus->history_ann_perf == FALSE) // OK accelerata compiuta
        {
            if (m_bANNOpacityPeakChoice == true)    // Picco calcolato con rete ANN
            {
                // Prendo i campioni
                bool bEsito = this->CalcolaValorePiccoANN(opaPeak, bCamionSelection);

                if (bEsito == true)
                    vret = OPA_SERIAL_OK;
                else
                    vret = OPA_SERIAL_ANN_ERROR;    // mancanza file rete neurale
            }
            else                                    // Picco misurato dall'opacimetro
            {
                OPA_SMOKEVALUE_S fumValPeak;

                vret = opaSeriale->opa_valoripicco(&fumValPeak);
                if (vret == OPA_SERIAL_OK)
                {
                    opaPeak->dOpacity_k = fumValPeak.dOpacity_k;
                    strcpy(opaPeak->szOpacity_k, fumValPeak.szOpacity_k);
                }

                return vret;
            }
        }
        else
        {
            vret = OPA_SERIAL_NAK;
        }
    }

    return vret;
}
#endif

#ifdef IDE_ENABLEIPERCONNESSIONE
//---------------------------------------------------------------------------
bool __fastcall TOpaSeriale::CalcolaValorePiccoANN(OPA_PEAKVALUE_S *opaPeak, bool bCamionSelection)
{
    // FASE DI CALCOLO
    m_dPeakKHP   = CalculateOpaK(m_PiccoFiltratoHP);     // camera 20 cm
    m_dPeakKHPLP = CalculateOpaK(m_PiccoFiltratoHPLP);   // camera 20 cm

    m_itSalita = (m_RiseEnd - m_RiseStart) * 25;  // t durata[ms]
    m_itDurata = (m_DescEnd - m_RiseStart) * 25;  // t durata[ms]

    float   *output;
    float   desiredOutput[1];
    FILE    *f;
    char    fName[256];
    char    fFullName[256];

    NNF_NeuralNet net;

    // POIS_LORD
    if (bCamionSelection == true)
    {
        // Rete neurale PL:
        #define kNumberOfRawDataPerSample 5

        AnsiString aFileName = aPathDLL + "NetPL.txt";
        strcpy(fFullName, aFileName.c_str());

        if (FileExists(aFileName) == false)
        {
            if (Log == true)
            {
                ShowMessage("MISSING FILE NetPL.txt");
            }

            return false;
        }
        f = fopen(fFullName, "r");
        if (f == NULL)
        {
            if (Log == true)
            {
                ShowMessage("MISSING FILE NetPL.txt");
            }

            return false;
        }

        int perceptrons[3] = { kNumberOfRawDataPerSample, 10, 1 };
        //float CondizionamentoOpacita = 1.0/6.0;  // 0.166667

        NNF_Init(&net, 3, perceptrons, 1);
        net.epsilon = (float) 0.025;
        net.activationFunction = NNF_SIGMA_LOGISTIC;

        NNF_Load(&net, f);
        fclose(f);

        // CONDIZIONAMENTO PL
        // condizionamento nostro picco in K dopo filtro passa alto 0.99
        m_rawData[0] = (float)( (m_dPeakKHP / 0.8) - 4.5);
        //condizionamento nostra area %
        m_rawData[1] = (float)( (m_dSomma / 350.0) - 4.5);
        //condizionamento nostro tempo salita
        m_rawData[2] = (float)( (double(m_itSalita) / 270.0) - 4.5);
        //condizionamento nostro durata accelerata
        m_rawData[3] = (float)( (double(m_itDurata) / 280.0) - 6);
        //condizionamento picco filtrato 0.036
        m_rawData[4] = (float)( (m_dPeakKHPLP / 0.4) - 4.5);

        #undef kNumberOfRawDataPerSample
    }
    else
    {
        // Rete neurale VL:
        #define kNumberOfRawDataPerSample 4

        AnsiString aFileName = aPathDLL + "NetVL.txt";
        strcpy(fFullName, aFileName.c_str());

        if (FileExists(aFileName) == false)
        {
            if (Log == true)
            {
                ShowMessage("MISSING FILE NetVL.txt");
            }

            return false;
        }
        f = fopen(fFullName, "r");
        if (f == NULL)
        {
            if (Log == true)
            {
                ShowMessage("MISSING FILE NetVL.txt");
            }

            return false;
        }

        int perceptrons[3] = { kNumberOfRawDataPerSample, 6, 1 };
        //float CondizionamentoOpacita = 0.142857;  // 1/7

        NNF_Init(&net, 3, perceptrons, 1);
        net.epsilon = (float) 0.01;
        net.activationFunction = NNF_SIGMA_LOGISTIC;

        NNF_Load(&net, f);
        fclose(f);

        // condizionamento nostro picco
        m_rawData[0] = (float)( (m_dPeakKHP / 1.5) - 4.5);
        //condizionamento nost ra area %
        m_rawData[1] = (float)( (m_dSomma / 300.0) - 4.5);
        //condizionamento nostro tempo salita
        m_rawData[2] = (float)( (double(m_itSalita) / 200.0) - 4.5);
        //condizionamento nostro durata accelerata
        m_rawData[3] = (float)( (double(m_itDurata) / 500.0) - 4.5);

        #undef kNumberOfRawDataPerSample
    }

    output = NNF_Execute(&net, m_rawData);

    double dANN;
    if (bCamionSelection == true)
        dANN = output[0] * 6.0;
    else
        dANN = output[0] * 7.0;

    m_FormatSettings.DecimalSeparator   = '.';
    if (opaPeak)
    {
        AnsiString aPeak = FormatFloat("0.00", (dANN + 0.004999),  m_FormatSettings);

        strcpy(opaPeak->szOpacity_k, aPeak.c_str());
        opaPeak->dOpacity_k = atof(opaPeak->szOpacity_k);
    }
    if (Log == true)
    {
        AnsiString aRow;

        // Scrittura parametri rete neurale
        if (bCamionSelection == true)
            aRow = "PL NEURAL NETWORK\r\n";
        else
            aRow = "VL NEURAL NETWORK\r\n";
        m_logOPA->Add(aRow);

        aRow.sprintf("%12.5f", m_PiccoFiltratoHP);
        aRow = AnsiString("HIGH FILTER PEAK % 20cm          = ") + aRow;
        m_logOPA->Add(aRow);

        aRow.sprintf("%12.5f", m_dPeakKHP);
        aRow = AnsiString("HIGH FILTER PEAK K               = ") + aRow;
        m_logOPA->Add(aRow);

        aRow.sprintf("%12.5f", m_dSomma);
        aRow = AnsiString("SUM AREA                         = ") + aRow;
        m_logOPA->Add(aRow);

        aRow.sprintf("%12d", m_itSalita);
        aRow = AnsiString("RISE TIME [ms]                   = ") + aRow;
        m_logOPA->Add(aRow);

        aRow.sprintf("%12d", m_itDurata);
        aRow = AnsiString("INTERVAL  [ms]                   = ") + aRow;
        m_logOPA->Add(aRow);

        if (bCamionSelection == true)
        {
            aRow.sprintf("%12.5f", m_PiccoFiltratoHPLP);
            aRow = AnsiString("HIGH/LOW FILTER PEAK % 20 cm     = ") + aRow;
            m_logOPA->Add(aRow);

            aRow.sprintf("%12.5f", m_dPeakKHPLP);
            aRow = AnsiString("HIGH/LOW FILTER PEAK K           = ") + aRow;
            m_logOPA->Add(aRow);
        }
        m_logOPA->Add("");

        aRow.sprintf("%12.5f", (dANN + 0.000005));
        aRow = AnsiString("ANN                              = ") + aRow;
        m_logOPA->Add(aRow);
        m_logOPA->Add("");

        // DATA
        aRow = AnsiString(    "INDEX RISESTART                  = ") + Format("%12d", OPENARRAY(TVarRec, (m_RiseStart)));
        m_logOPA->Add(aRow);

        aRow = AnsiString(    "INDEX RISEEND                    = ") + Format("%12d", OPENARRAY(TVarRec, (m_RiseEnd)));
        m_logOPA->Add(aRow);

        aRow = AnsiString(    "INDEX DESCENDEND                 = ") + Format("%12d", OPENARRAY(TVarRec, (m_DescEnd)));
        m_logOPA->Add(aRow);

        aRow = AnsiString("TIME             #    PRE     FILTERHP   FILTERHPLP");
        m_logOPA->Add(aRow);

        AnsiString aSuf;
        AnsiString aSufPeakHP;
        AnsiString aSufPeakHPLP;

        double dSum = 0.0;
        aRow = "";

        for (int ii = 0; ii < m_iNdx; ++ii)
        {
            if (ii >= m_RiseStart)
            {
                if (ii < m_DescEnd)
                {
                    dSum += m_vDataFilteredHP[ii];
                    aRow.sprintf("%12.5f", dSum);

                    if (ii > m_RiseEnd)
                        aSuf = "#";
                    else
                        aSuf = "*";
                }
                else
                {
                    aSuf = " ";
                    aRow = "";
                }
            }
            else
            {
                aSuf = " ";
            }

            if (ii != m_RiseEnd)
                aSufPeakHP = " ";
            else
                aSufPeakHP = "*";

            if (ii != m_HPLPMax)
                aSufPeakHPLP = " ";
            else
                aSufPeakHPLP = "*";


            AnsiString aLinea  = m_aTime[ii]
                               + Format("%5d", OPENARRAY(TVarRec, (ii)))                                       + aSuf          + ": "
                               + FormatFloat("+00.0;-00.0",         m_dInPOpaANN[ii],        m_FormatSettings) +                " : "
                               + FormatFloat("+00.00000;-00.00000", m_vDataFilteredHP[ii],   m_FormatSettings) + aSufPeakHP    + ": "
                               + FormatFloat("+00.00000;-00.00000", m_vDataFilteredHPLP[ii], m_FormatSettings) + aSufPeakHPLP  + ": "
                               + aRow;
            m_logOPA->Add(aLinea);
        }
    }

    NNF_Delete(&net);

    return true;
}
#endif


//------------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_HistoryValues(const char *num_sample,
                                              char *opa_p,
                                              char *opa_k,
                                              char *giri)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_valoristorico(num_sample, opa_p, opa_k, giri);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::opa_valoristorico(const char *num_sample,
                                              char *opa_p,
                                              char *opa_k,
                                              char *giri)
{
    char          gpv[32];
    int           vret;

    if (Demo)
    {
        strcpy(opa_p, "00.0");
        strcpy(opa_k, "0.00");
        strcpy(giri,  "0000");
        vret = OPA_SERIAL_OK;
    }
    else
    {
        strcpy(gpv, COM_VALST.c_str()); // "105"
        strcat(gpv, num_sample);        // Nr (01-50)
        vret = PutOmniBUS(COM_VALST, gpv, m_szAdrsOPA);
        if(vret == OPA_SERIAL_OK) // OK
        {
            strncpy(opa_p, (char *)&m_BufRX[7],  4); // PP.P
            opa_p[4] = 0;
            strncpy(opa_k, (char *)&m_BufRX[11], 4); // M.MM
            opa_k[4] = 0;
            strncpy(giri,  (char *)&m_BufRX[15], 4); // GGGG
            giri[4]  = 0;
        }
    }

    return vret;
}

//---------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_AnalogValues(OPA_ANALOGVALUE_S *valAux)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_ValoriAnalogiciAusiliari(valAux);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::opa_ValoriAnalogiciAusiliari(OPA_ANALOGVALUE_S *valAux)
{
    char            gpv[20];
    int             vret;

    if (Demo)
    {
        strcpy(valAux->szOilTemp,           "101");
        strcpy(valAux->szSmokeTemp,         "75");
        strcpy(valAux->szChamberTemp,       "3.4");
        strcpy(valAux->szAmbientTemp,       "12.3");
        strcpy(valAux->szChamberPressure,   "32.5");
        strcpy(valAux->szPowerBattery,      "3.9");

        vret = OPA_SERIAL_OK;
    }
    else
    {
/*
        // Solo Nuovi OPA
        switch (RpmConfig)
        {
            case rcINTEGRATA:
                break;

            case rcMCTCNET:
                WrGiri(giri);
                break;

            case rcEOBD:
            case rcMANUALE:
            case rcSERIALE:
                WrGiriTemp(giri, ValAux->szOilTemp);
                break;
        }
*/
        strcpy(gpv, COM_VALANA.c_str());                                        // "108"
        vret = PutOmniBUS(COM_VALANA, gpv, m_szAdrsOPA);
        if(vret == OPA_SERIAL_OK) // OK
        {
            memcpy(valAux->szOilTemp,     (char*)&m_BufRX[7], 3); // OOO
            valAux->szOilTemp[3]        = 0;

            memcpy(valAux->szSmokeTemp,     (char*)&m_BufRX[10], 3); // FFF
            valAux->szSmokeTemp[3]      = 0;

            memcpy(valAux->szChamberTemp,   (char*)&m_BufRX[13], 4); // CC.C
            valAux->szChamberTemp[4]    = 0;

            memcpy(valAux->szAmbientTemp, (char*)&m_BufRX[17], 4); // AA.A
            valAux->szAmbientTemp[4]    = 0;

            memcpy(valAux->szChamberPressure,  (char*)&m_BufRX[21], 5); // +PP.P
            valAux->szChamberPressure[5] = 0;

            memcpy(valAux->szPowerBattery, (char*)&m_BufRX[26], 4); // VV.V
            valAux->szPowerBattery[4]   = 0;

            ZCanc(valAux->szOilTemp);
            ZCanc(valAux->szSmokeTemp);
            //ZCanc(valAux->szChamberTemp);
            //ZCanc(valAux->szAmbientTemp);
            //ZCanc(valAux->szChamberPressure);
            //ZCanc(valAux->szPowerBattery);
        }
    }

    return vret;
}

//------------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_ResetPeakValues(const char *opa_p, const char *opa_k, const char *giri)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_ResetValoriPicco(opa_p, opa_k, giri);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::opa_ResetValoriPicco(const char *opa_p, const char *opa_k, const char *giri)
{
    char            gpv[32];
    int             vret;

    if (Demo)
        return OPA_SERIAL_OK;

    strcpy(gpv, COM_RESVALP.c_str()); // "109"
    strcat(gpv, opa_p);
    strcat(gpv, opa_k);
    strcat(gpv, giri);

    vret = PutOmniBUS(COM_RESVALP, gpv, m_szAdrsOPA);

    return vret;
}

//------------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_HistoryActivation(const char *opa_p, const char *opa_k, const char *giri, const char *num)
{
    if (opaSeriale != NULL)
    {
        #ifdef IDE_ENABLEIPERCONNESSIONE
        m_HPF_Alfa          = 0.99;
        m_LPF_Beta          = 0.036;

        m_iNdx              = 0;
        m_DataRawOld        = 0.0;

        m_PiccoFiltratoHP   = 0.0;
        m_PiccoFiltratoHPLP = 0.0;

        m_DataFilteredHP    = 0.0;
        m_DataFilteredHPLP  = 0.0;

        m_RiseStart         = -1;
        m_RiseEnd           = -1;
        m_DescEnd           = -1;
        m_HPLPMax           = -1;

        // features ANN
        m_dPeakKHP          = 0.0;
        m_dSomma            = 0.0;
        m_itSalita          = 0;
        m_itDurata          = 0;
        m_dPeakKHPLP        = 0.0;

        m_ucHistoryANNPerf      = TRUE;
        m_bANNOpacityPeakChoice = false;
        #endif

        return opaSeriale->opa_AttivazioneStorico(opa_p, opa_k, giri, num);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::opa_AttivazioneStorico(const char *opa_p, const char *opa_k, const char *giri, const char *num)
{
    char            gpv[32];
    int             vret;

    if (Demo)
        return OPA_SERIAL_OK;

    strcpy(gpv, COM_ATT_STO.c_str()); // "110"
    strcat(gpv, opa_p);
    strcat(gpv, opa_k);
    strcat(gpv, giri);
    strcat(gpv, num);

    vret = PutOmniBUS(COM_ATT_STO, gpv, m_szAdrsOPA);

    return vret;
}

//------------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_RCSelection(const char *costante)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_SelezioneRC(costante);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::opa_SelezioneRC(const char *costante)
{
    char            gpv[32];
    int             vret;

    if (Demo)
        return OPA_SERIAL_OK;

    strcpy(gpv, COM_SEL_RC.c_str()); // "112"
    strcat(gpv, costante);

    vret = PutOmniBUS(COM_SEL_RC, gpv, m_szAdrsOPA);

    return vret;
}

//------------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_BackupCalibrations(void)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_BackupTarature();
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::opa_BackupTarature(void)
{
    char            gpv[20];
    int             vret;

    if (Demo)
        return OPA_SERIAL_OK;

    strcpy(gpv, COM_BKPTAR.c_str()); // "118"
    vret = PutOmniBUS(COM_BKPTAR, gpv, m_szAdrsOPA);

    return vret;
}

//------------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_RestoreCalibrations(void)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_RestoreTarature();
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::opa_RestoreTarature(void)
{
    char            gpv[20];
    int             vret;

    if (Demo)
        return OPA_SERIAL_OK;

    strcpy(gpv, COM_RSTTAR.c_str()); // "119"
    vret = PutOmniBUS(COM_RSTTAR, gpv, m_szAdrsOPA);

    return vret;
}

//------------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_ResetCalibrations(void)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_ResetTarature();
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::opa_ResetTarature(void)
{
    char            gpv[20];
    int             vret;

    if (Demo)
        return OPA_SERIAL_OK;

    strcpy(gpv, COM_RESTAR.c_str()); // "120"
    vret = PutOmniBUS(COM_RESTAR, gpv, m_szAdrsOPA);

    return vret;
}

//------------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_Autozero(void)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_Autozero();
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::opa_Autozero(void)
{
    char            gpv[20];
    int             vret;

    if (Demo)
        return OPA_SERIAL_OK;

    strcpy(gpv, COM_AUTOZERO.c_str()); // "121"
    vret = PutOmniBUS(COM_AUTOZERO, gpv, m_szAdrsOPA);

    return vret;
}

//------------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_OpacityCalibration(void)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_CalibrazioneOpacita();
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::opa_CalibrazioneOpacita(void)
{
    char            gpv[20];
    int             vret;

    if (Demo)
        return OPA_SERIAL_OK;

    strcpy(gpv, COM_CAL_OPA.c_str()); // "122"
    vret = PutOmniBUS(COM_CAL_OPA, gpv, m_szAdrsOPA);

    return vret;
}

//------------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_PowerCalibration(const char *value)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_CalAlimentazione(value);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::opa_CalAlimentazione(const char *value)
{
    char            gpv[32];
    int             vret;

    if (Demo)
        return OPA_SERIAL_OK;

    strcpy(gpv, COM_CAL_ALIM.c_str()); // "123"
    strcat(gpv, value);

    vret = PutOmniBUS(COM_CAL_ALIM, gpv, m_szAdrsOPA);

    return vret;
}

//------------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_PressureCalibration(const char *cal_point, const char *value)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_CalibrazionePressione(cal_point, value);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::opa_CalibrazionePressione(const char *cal_point, const char *value)
{
    char            gpv[32];
    int             vret;

    if (Demo)
        return OPA_SERIAL_OK;

    strcpy(gpv, COM_CAL_PRES.c_str()); // "124"
    strcat(gpv, cal_point);
    strcat(gpv, value);

    vret = PutOmniBUS(COM_CAL_PRES, gpv, m_szAdrsOPA);

    return vret;
}

//------------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_SmokeCalibration(const char *cal_point, const char *value)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_CalibrazioneFumi(cal_point, value);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::opa_CalibrazioneFumi(const char *cal_point, const char *value)
{
    char            gpv[32];
    int             vret;

    if (Demo)
        return OPA_SERIAL_OK;

    strcpy(gpv, COM_CAL_FUMI.c_str()); // "125"
    strcat(gpv, cal_point);
    strcat(gpv, value);

    vret = PutOmniBUS(COM_CAL_FUMI, gpv, m_szAdrsOPA);

    return vret;
}

//------------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_OilCalibration(const char *cal_point, const char *value)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_CalibrazioneOlio(cal_point, value);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::opa_CalibrazioneOlio(const char *cal_point, const char *value)
{
    char            gpv[32];
    int             vret;

    if (Demo)
        return OPA_SERIAL_OK;

    strcpy(gpv, COM_CAL_OLIO.c_str()); // "126"
    strcat(gpv, cal_point);
    strcat(gpv, value);

    vret = PutOmniBUS(COM_CAL_OLIO, gpv, m_szAdrsOPA);

    return vret;
}

//------------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_BitOpacityValue(OPA_PRIMARYVALUE_S *opaPrim)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_ValoreOpacitaPrimario(opaPrim);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::opa_ValoreOpacitaPrimario(OPA_PRIMARYVALUE_S *opaPrim)
{
    char            gpv[20];
    int             vret;

    if (Demo)
    {
        strcpy(opaPrim->szOpacity, "1024");

        vret = OPA_SERIAL_OK;
    }
    else
    {
        strcpy(gpv, COM_VALP_OPA.c_str()); // "130"
        vret = PutOmniBUS(COM_VALP_OPA, gpv, m_szAdrsOPA);
        if (vret == OPA_SERIAL_OK)  // OK
        {
            memcpy(opaPrim->szOpacity, (char*)&m_BufRX[7], 4); // BBBB
            opaPrim->szOpacity[4] = '\0';
        }
    }

    return vret;
}

//------------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_BitAnalogValues(OPA_PRIMARYVALUE_S *valPrim)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_ValoriAnalogiciPrimari(valPrim);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::opa_ValoriAnalogiciPrimari(OPA_PRIMARYVALUE_S *valPrim)
{
    char            gpv[20];
    int             vret;

    if (Demo)
    {
        strcpy(valPrim->szOilTemp     , "512");
        strcpy(valPrim->szSmokeTemp     , "1024");
        strcpy(valPrim->szChamberTemp   , "200");
        strcpy(valPrim->szAmbientTemp , "300");
        strcpy(valPrim->szOffsetTemp       , "400");
        strcpy(valPrim->szChamberPressure  , "500");
        strcpy(valPrim->szPowerBattery , "1");

        vret = OPA_SERIAL_OK;
    }
    else
    {
        strcpy(gpv, COM_VALP_ANA.c_str()); // "131"
        vret = PutOmniBUS(COM_VALP_ANA, gpv, m_szAdrsOPA);
        if(vret == OPA_SERIAL_OK) // OK
        {
            memcpy(valPrim->szOilTemp,      (char*)&m_BufRX[ 7], 4);  // OOOO
            valPrim->szOilTemp[4]       = '\0';

            memcpy(valPrim->szSmokeTemp,    (char*)&m_BufRX[11], 4); // FFFF
            valPrim->szSmokeTemp[4]     = '\0';

            memcpy(valPrim->szChamberTemp,  (char*)&m_BufRX[15], 4); // CCCC
            valPrim->szChamberTemp[4]   = '\0';

            memcpy(valPrim->szAmbientTemp,  (char*)&m_BufRX[19], 4); // AAAA
            valPrim->szAmbientTemp[4]   = '\0';

            memcpy(valPrim->szOffsetTemp,   (char*)&m_BufRX[23], 4); // SSSS
            valPrim->szOffsetTemp[4]    = '\0';

            memcpy(valPrim->szChamberPressure,     (char*)&m_BufRX[27], 4); // PPPP
            valPrim->szChamberPressure[4]='\0';

            memcpy(valPrim->szPowerBattery,    (char*)&m_BufRX[31], 4); // VVVV
            valPrim->szPowerBattery[4]='\0';
        }
    }

    return vret;
}

//------------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_RdDigitalInput(void)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_IngressiDigitali();
    }

    return OPA_SERIAL_VOID;

};
#endif

int __fastcall TOpaSeriale::opa_IngressiDigitali(void)
{
    char            gpv[20];
    int             vret;

    if (Demo)
    {
        m_DigitIn = 0;
        vret    = OPA_SERIAL_OK;
    }
    else
    {
        strcpy(gpv, COM_VALIN_DIGI.c_str()); // "132"
        vret = PutOmniBUS(COM_VALIN_DIGI, gpv, m_szAdrsOPA);
        if(vret == OPA_SERIAL_OK)
        {
            m_DigitIn = HexASCII2Bin(&m_BufRX[7]);
        }
    }

    return vret;
}

//------------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_RdDigitalOutput(void)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_UsciteDigitali();
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::opa_UsciteDigitali(void)
{
    char            gpv[20];
    int             vret;

    if (Demo)
    {
        m_DigitOut = 0;
        vret     = OPA_SERIAL_OK;
    }
    else
    {
        strcpy(gpv, COM_VALOUT_DIGI.c_str()); // "133"
        vret = PutOmniBUS(COM_VALOUT_DIGI, gpv, m_szAdrsOPA);
        if (vret == OPA_SERIAL_OK)  // OK
        {
            m_DigitOut = HexASCII2Bin(&m_BufRX[7]);
        }
    }

    return vret;
}

//------------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_WrDigitalOutput(unsigned char mask, unsigned char outstat)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_SetUsciteDigitali(mask, outstat);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::opa_SetUsciteDigitali(unsigned char mask, unsigned char outstat)
{
    char            gpv[32];
    int             vret;

    if (Demo)
    {
        m_DigitOut = 0;
        vret     = OPA_SERIAL_OK;
    }
    else
    {
        strcpy(gpv, COM_SETOUT_DIGI.c_str()); // "135"
        Bin2HexASCII(mask,    &gpv[3]);
        Bin2HexASCII(outstat, &gpv[5]);
        gpv[7] = 0;

        vret = PutOmniBUS(COM_SETOUT_DIGI, gpv, m_szAdrsOPA);
        if(vret == OPA_SERIAL_OK)
        {
            m_DigitOut = HexASCII2Bin(&m_BufRX[7]);
        }
    }

    return vret;
}

//------------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_RdFilter(char *buff)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_RdFiltro(buff);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::opa_RdFiltro(char *buff)
{
    char            gpv[32];
    int             vret;

    if (Demo)
    {
        strcpy(buff, "1.64");
        vret = OPA_SERIAL_OK;
    }
    else
    {
        strcpy(gpv, COM_RD_FILTRO.c_str()); // "140"
        vret = PutOmniBUS(COM_RD_FILTRO, gpv, m_szAdrsOPA);
        if(vret == OPA_SERIAL_OK) // OK
        {
            strncpy(buff, (char *)&m_BufRX[7], 4); // F.FF
            buff[4] = 0;
        }
    }

    return vret;
}

//------------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_WrFilter(const char *value)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_WrFiltro(value);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::opa_WrFiltro(const char *value)
{
    char            gpv[32];
    int             vret;

    if (Demo)
        return OPA_SERIAL_OK;

    strcpy(gpv, COM_WR_FILTRO.c_str()); // "141"
    strcat(gpv, value);

    vret = PutOmniBUS(COM_WR_FILTRO, gpv, m_szAdrsOPA);

    return vret;
}

//------------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_RdPowerLimits(char *vmin, char *vmax)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_RdLimEl(vmin, vmax);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::opa_RdLimEl(char *vmin, char *vmax)
{
    char            gpv[32];
    int             vret;

    if (Demo)
    {
        strcpy(vmin, "11.5");
        strcpy(vmax, "15.7");
        vret = OPA_SERIAL_OK;
    }
    else
    {
        strcpy(gpv, COM_RD_SVBAT.c_str()); // "142"
        vret = PutOmniBUS(COM_RD_SVBAT, gpv, m_szAdrsOPA);
        if(vret == OPA_SERIAL_OK) // OK
        {
            strncpy(vmin, (char *)&m_BufRX[7],  4); // LL.L
            vmin[4] = 0;
            strncpy(vmax, (char *)&m_BufRX[11], 4); // HH.H
            vmax[4] = 0;
        }
    }

    return vret;
}

//------------------------------------------------------------------------------
#ifdef DLL_EXPORTS
int __stdcall OPA_WrPowerLimits(const char *vmin, const char *vmax)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_WrLimEl(vmin, vmax);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::opa_WrLimEl(const char *vmin, const char *vmax)
{
    char          gpv[32];
    int           vret;

    if (Demo)
        return OPA_SERIAL_OK;

    strcpy(gpv, COM_WR_SVBAT.c_str()); // "143"
    strcat(gpv, vmin);
    strcat(gpv, vmax);

    vret = PutOmniBUS(COM_WR_SVBAT, gpv, m_szAdrsOPA);

    return vret;
}

//------------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_RdChamberTemperature(char *temperatura)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_RdTempCamera(temperatura);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::opa_RdTempCamera(char *temperatura)
{
    char          gpv[32];
    int           vret;

    if (Demo)
    {
        strcpy(temperatura, "49.6");
        vret = OPA_SERIAL_OK;
    }
    else
    {
        strcpy(gpv, COM_RD_TCAMERA.c_str()); // "144"
        vret = PutOmniBUS(COM_RD_TCAMERA, gpv, m_szAdrsOPA);
        if(vret == OPA_SERIAL_OK)
        {
            strncpy(temperatura, (char *)&m_BufRX[7], 4); // TT.T
            temperatura[4] = 0;
        }
    }

    return vret;
}

//------------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_WrChamberTemperature(const char *temperatura)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_WrTempCamera(temperatura);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::opa_WrTempCamera(const char *temperatura)
{
    char          gpv[32];
    int           vret;

    if (Demo)
        return OPA_SERIAL_OK;

    strcpy(gpv, COM_WR_TCAMERA.c_str()); // "145"
    strcat(gpv, temperatura);

    vret = PutOmniBUS(COM_WR_TCAMERA, gpv, m_szAdrsOPA);

    return vret;
}

//------------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_RdRC(char *k1, char *k2)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->RdRC(k1, k2);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::RdRC(char *k1, char *k2)
{
    char            gpv[32];
    int             vret;

    if (Demo)
    {
        strcpy(k1, "0.200");
        strcpy(k2, "0.666");
        vret = OPA_SERIAL_OK;
    }
    else
    {
        strcpy(gpv, COM_RD_RC.c_str()); // "146"
        vret = PutOmniBUS(COM_RD_RC, gpv, m_szAdrsOPA);
        if(vret == OPA_SERIAL_OK) // OK
        {
            strncpy(k1, (char *)&m_BufRX[7],  5); // K.KKK
            k1[5] = 0;
            strncpy(k2, (char *)&m_BufRX[12], 5); // K.KKK
            k2[5] = 0;
        }
    }

    return vret;
}

//------------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_WrRC(const char *value1, const char *value2)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->WrRC(value1, value2);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::WrRC(const char *value1, const char *value2)
{
    char            gpv[32];
    int             vret;

    if (Demo)
        return OPA_SERIAL_OK;

    strcpy(gpv, COM_WR_RC.c_str()); // "147"
    strcat(gpv, value1);
    strcat(gpv, value2);

    vret = PutOmniBUS(COM_WR_RC, gpv, m_szAdrsOPA);

    return vret;
}

//------------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_RdChamberLength(char *lunghezza)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_RdLunghezzaCamera(lunghezza);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::opa_RdLunghezzaCamera(char *lunghezza)
{
    char            gpv[32];
    int             vret;

    if (Demo)
    {
        strcpy(lunghezza, "0.20");
        vret = OPA_SERIAL_OK;
    }
    else
    {
        strcpy(gpv, COM_RD_LCAMERA.c_str()); // "148"
        vret = PutOmniBUS(COM_RD_LCAMERA, gpv, m_szAdrsOPA);
        if(vret == OPA_SERIAL_OK)  // OK
        {
            strncpy(lunghezza, (char *)&m_BufRX[7], 4); // L.LL
            lunghezza[4] = 0;
        }
    }

    return vret;
}

//------------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_WrChamberLength(const char *lunghezza)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_WrLunghezzaCamera(lunghezza);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::opa_WrLunghezzaCamera(const char *lunghezza)
{
    char            gpv[32];
    int             vret;

    if (Demo)
        return OPA_SERIAL_OK;

    strcpy(gpv, COM_WR_LCAMERA.c_str()); // "149"
    strcat(gpv, lunghezza);

    vret = PutOmniBUS(COM_WR_LCAMERA, gpv, m_szAdrsOPA);

    return vret;
}

//------------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_RdRpmCounterMode(char *contagiri)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_RdFatContagiri(contagiri);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::opa_RdFatContagiri(char *contagiri)
{
    char            gpv[32];
    int             vret;

    if (Demo)
    {
        strcpy(contagiri, "0");
        vret = OPA_SERIAL_OK;
    }
    else
    {
        strcpy(gpv, COM_RD_FATTRPM.c_str()); // "150"
        vret = PutOmniBUS(COM_RD_FATTRPM, gpv, m_szAdrsOPA);
        if(vret == OPA_SERIAL_OK) //OK
        {
            strncpy(contagiri, (char *)&m_BufRX[7], 1); // F
            contagiri[1] = 0; //0 radio 1 opa-ports div/coeff.
        }
    }

    return vret;
}

//------------------------------------------------------------------------------
#ifdef DLL_EXPORTS
int __stdcall OPA_WrRpmCounterMode(const char *contagiri)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_WrFatContagiri(contagiri);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::opa_WrFatContagiri(const char *contagiri)
{
    char            gpv[32];
    int             vret;

    if (Demo)
        return OPA_SERIAL_OK;

    strcpy(gpv, COM_WR_FATTRPM.c_str()); // "151"
    strcat(gpv, contagiri);

    vret = PutOmniBUS(COM_WR_FATTRPM, gpv, m_szAdrsOPA);

    return vret;
}

//------------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_RdSerialNr(char *buff)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->RdNumeroSerie(buff);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::RdNumeroSerie(char *buff)
{
    char            gpv[32];
    int             vret;

    if (Demo)
    {
        strcpy(buff, "123456789012");
        vret = OPA_SERIAL_OK;
    }
    else
    {
        strcpy(gpv, COM_RD_NRSERIE.c_str()); // "152"
        vret = PutOmniBUS(COM_RD_NRSERIE, gpv, m_szAdrsOPA);
        if(vret == OPA_SERIAL_OK)  // OK
        {
            if(m_BufRX[15] == ETX)
            {
                strncpy(buff, (char *)&m_BufRX[7],  8);  // CCCCCCCC
                buff[8] = 0;
            }
            else if(m_BufRX[19] == ETX)
            {
                strncpy(buff, (char *)&m_BufRX[7], 12);  // CCCCCCCCNNNN
                buff[12] = 0;
            }
        }
    }

    return vret;
}

//------------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_WrSerialNr(const char *code)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_WrNumeroSerie(code);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::opa_WrNumeroSerie(const char *code)
{
    char            gpv[32];
    int             vret;

    if (Demo)
        return OPA_SERIAL_OK;

    strcpy(gpv, COM_WR_NRSERIE.c_str()); // "153"
    strcat(gpv, code);

    vret = PutOmniBUS(COM_WR_NRSERIE, gpv, m_szAdrsOPA);

    return vret;
}

//------------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_RdCalibrationDate(char *buff)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->RdDataCalibrazione(buff);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::RdDataCalibrazione(char *buff)
{
    char            gpv[32];
    int             vret;

    if (Demo)
    {
        strcpy(buff, "12:0001.01.2001");
        vret = OPA_SERIAL_OK;
    }
    else
    {
        strcpy(gpv, COM_RD_DATACAL.c_str()); // "154"
        vret = PutOmniBUS(COM_RD_DATACAL, gpv, m_szAdrsOPA);
        if(vret == OPA_SERIAL_OK) //OK
        {
            strncpy(buff, (char *)&m_BufRX[7], 15); // hh:nnGG.MM.AAAA
            buff[15] = 0;
        }
    }

    return vret;
}

//------------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_WrCalibrationDate(const char *data)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_WrDataCalibrazione(data);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::opa_WrDataCalibrazione(const char *data)
{
    char            gpv[32];
    int             vret;

    if (Demo)
        return OPA_SERIAL_OK;

    strcpy(gpv, COM_WR_DATACAL.c_str()); // "155"
    strcat(gpv, data);

    vret = PutOmniBUS(COM_WR_DATACAL, gpv, m_szAdrsOPA);

    return vret;
}

//------------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_RdVerSoftware(char *buff)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_RdVerSoftware(buff);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::opa_RdVerSoftware(char *buff)
{
    char            gpv[32];
    int             vret;

    if (Demo)
    {
        strcpy(buff, "-.---");
        vret = OPA_SERIAL_OK;
    }
    else
    {
        strcpy(gpv, COM_RD_VERSW.c_str()); // "156"
        vret = PutOmniBUS(COM_RD_VERSW, gpv, m_szAdrsOPA);
        if(vret == OPA_SERIAL_OK)  //OK
        {
            if(m_BufRX[16] == ETX)      // CHKSUM
            {
                strncpy(buff, (char *)&m_BufRX[7], 9);  // S.SSSCKSM
                buff[9] = 0;
            }
            else if(m_BufRX[12] == ETX) // NO CHKSUM
            {
                strncpy(buff, (char *)&m_BufRX[7], 5);  // S.SSS
                buff[5] = 0;
            }
        }
    }

    return vret;
}

//---------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_Version(char *veropa, char *checkopa)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->ReadVersion(veropa, checkopa);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::ReadVersion(char *veropa, char *checkopa)
{
    char            gpv[20];
    int             vret;

    if (Demo)
    {
        strcpy(veropa,   "-.---");
        strcpy(checkopa, "----");
        vret = OPA_SERIAL_OK;
    }
    else
    {
        strcpy(gpv, COM_RD_VERSW.c_str()); // "156"
        vret = PutOmniBUS(COM_RD_VERSW, gpv, m_szAdrsOPA);
        if(vret == OPA_SERIAL_OK)
        {
            strncpy(veropa, (char *)&m_BufRX[7], 5);
            veropa[5]   = 0;

            if(m_BufRX[12] == ETX) // NO CHKSUM
            {
                strcpy(checkopa, "");
            }
            else if(m_BufRX[16] == ETX)      // CHKSUM
            {
                strncpy(checkopa, (char *)&m_BufRX[12], 4);  // CKSM
                checkopa[5] = 0;
            }
        }
    }

    return vret;
}

//---------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_Revision(char *pszRevision, char *pszZone, char *pszDateRevision)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->ReadRevision(pszRevision, pszZone, pszDateRevision);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::ReadRevision(char *pszRevision, char *pszZone, char *pszDateRevision)
{
    char            gpv[32];
    int             vret;

    if (Demo)
    {
        strcpy(pszRevision,     "");
        strcpy(pszZone,         "");
        strcpy(pszDateRevision, "");

        vret = OPA_SERIAL_OK;
    }
    else
    {
        strcpy(gpv, COM_RD_REVISION.c_str()); // "179"
        vret = PutOmniBUS(COM_RD_REVISION, gpv, m_szAdrsOPA);
        if (vret == OPA_SERIAL_OK)
        {
            strncpy(pszRevision, (char *)&m_BufRX[7], 4);
            pszRevision[4] = '\0';

            AnsiString aRevision(pszRevision);
            strcpy(pszRevision, aRevision.Trim().c_str());

            strncpy(pszZone, (char *)&m_BufRX[11], 4);
            pszZone[4] = '\0';

            AnsiString aZone(pszZone);
            strcpy(pszZone, aZone.Trim().c_str());

            strncpy(pszDateRevision, (char *)&m_BufRX[15], 11);
            pszDateRevision[11] = '\0';
        }
        else if (vret == OPA_SERIAL_NAK)
        {
            strcpy(pszRevision,     "");
            strcpy(pszZone,         "");
            strcpy(pszDateRevision, "");

            return OPA_SERIAL_OK;
        }
    }

    return vret;
}
//---------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_RevisionDLL(char *pszRevision, char *pszDateRevision)
{
#if UTAC
    strcpy(pszRevision,     "A");
    strcpy(pszDateRevision, __DATE__);
#else
    strcpy(pszRevision,     "");
    strcpy(pszDateRevision, "");
#endif

    return OPA_SERIAL_OK;
};
#endif

//------------------------------------------------------------------------------
#ifdef DLL_EXPORTS
int __stdcall OPA_RdUserData(char *buff)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_RdDatiUtente(buff);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::opa_RdDatiUtente(char *buff)
{
    char            gpv[32];
    int             vret;

    if (Demo)
    {
        strcpy(buff, "12:0001.01.2001000000000");
        vret = OPA_SERIAL_OK;
    }
    else
    {
        strcpy(gpv, COM_RD_USERDATA.c_str()); // "157"
        vret = PutOmniBUS(COM_RD_USERDATA, gpv, m_szAdrsOPA);
        if(vret == OPA_SERIAL_OK) //OK
        {
            strncpy(buff, (char *)&m_BufRX[7], 24); // 24 car.
            buff[24] = 0;
        }
    }

    return vret;
}

//------------------------------------------------------------------------------
//---------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_WrUserData(const char *data)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_WrDatiUtente(data);
    }

    return OPA_SERIAL_VOID;
};
#endif
int __fastcall TOpaSeriale::opa_WrDatiUtente(const char *data)
{
    char            gpv[32];
    int             vret;

    if (Demo)
        return OPA_SERIAL_OK;

    strcpy(gpv, COM_WR_USERDATA.c_str()); // "158"
    strcat(gpv, data);

    vret = PutOmniBUS(COM_WR_USERDATA, gpv, m_szAdrsOPA);

    return vret;
}

//------------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_RdRCCompensation(char *k1, char *k2)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->RdComp(k1, k2);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::RdComp(char *k1, char *k2)
{
    char            gpv[32];
    int             vret;

    if (Demo)
    {
        strcpy(k1, "0.108");
        strcpy(k2, "0.245");
        vret = OPA_SERIAL_OK;
    }
    else
    {
        strcpy(gpv, COM_RD_COMP.c_str());  // "159"
        vret = PutOmniBUS(COM_RD_COMP, gpv, m_szAdrsOPA);
        if(vret == OPA_SERIAL_OK)      // OK
        {
            strncpy(k1, (char *)&m_BufRX[7],  5); // K.KKK
            k1[5] = 0;
            strncpy(k2, (char *)&m_BufRX[12], 5); // K.KKK
            k2[5] = 0;
        }
        else                  // Frame errato, NAK -> Comando non supportato
        {
            strcpy(k1, "-.---");
            strcpy(k2, "-.---");
        }
    }

    return vret;
}

//------------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_WrRCCompensation(const char *value1, const char *value2)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->WrComp(value1, value2);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::WrComp(const char *value1, const char *value2)
{
    char            gpv[32];
    int             vret;

    if (Demo)
        return OPA_SERIAL_OK;

    strcpy(gpv, COM_WR_COMP.c_str()); // "160"
    strcat(gpv, value1);
    strcat(gpv, value2);

    vret = PutOmniBUS(COM_WR_COMP, gpv, m_szAdrsOPA);

    return vret;
}

// Deprecated
//----------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_RdGraphPerc(char *opa_p[], char *giri[])
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_RdGrafico_p(opa_p, giri);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::opa_RdGrafico_p(char *opa_p[], char *giri[])
{
    char            gpv[32];
    int             vret;

    if (Demo)
    {
        for (int i=0; i<8; ++i)
        {
            strcpy(opa_p[i], "10.0");
            strcpy(giri[i],  "1000");
        }
        vret = OPA_SERIAL_OK;
    }
    else
    {
        //TODO: Gestire
        /*switch(RpmConfig)
        {
            case rcINTEGRATA:
                break;

            case rcEOBD:
            case rcMANUALE:
            case rcMCTCNET:
            case rcSERIALE:
                opa_WrGiri(giri);
                break;
        }*/

        strcpy(gpv, COM_RD_GRAF_P.c_str());  // "161"
        vret = PutOmniBUS(COM_RD_GRAF_P, gpv, m_szAdrsOPA);
        if(vret == OPA_SERIAL_OK) // OK
        {
            for (int i=0; i<8; ++i)
            {
                strncpy(opa_p[i], (char *)&m_BufRX[7+8*i],  4); // PP.P
                opa_p[i][4] = 0;
                strncpy(giri[i],  (char *)&m_BufRX[11+8*i], 4); // GGGG
                giri[i][4]  = 0;
            }
        }
        else             // Frame errato, NAK -> Comando non supportato
        {
            for (int i=0; i<8; ++i)
            {
                strcpy(opa_p[i], "00.0");
                strcpy(giri[i],  "0000");
            }
        }
    }

    return vret;
}
// Deprecated
//----------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_RdGraphK(char *opa_k[], char *giri[])
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_RdGrafico_k(opa_k, giri);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::opa_RdGrafico_k(char *opa_k[], char *giri[])
{
    char            gpv[32];
    int             vret;

    if (Demo)
    {
        for (int i=0;i<8; ++i)
        {
            strcpy(opa_k[i], "5.00");
            strcpy(giri[i],  "1000");
        }
        vret = OPA_SERIAL_OK;
    }
    else
    {
        //TODO: Gestire
        /*
        switch(RpmConfig)
        {
            case rcINTEGRATA:
                break;

            case rcEOBD:
            case rcMANUALE:
            case rcMCTCNET:
            case rcSERIALE:
                opa_WrGiri(p_RPMTemp[RpmConfig]->giri);
                break;
        }
        */

        strcpy(gpv, COM_RD_GRAF_K.c_str());  // "162"
        vret = PutOmniBUS(COM_RD_GRAF_K, gpv, m_szAdrsOPA);
        if(vret == OPA_SERIAL_OK) // OK
        {
            for (int i=0; i<8; ++i)
            {
                strncpy(opa_k[i], (char *)&m_BufRX[7+8*i], 4); // K.KK
                opa_k[i][4] = 0;
                strncpy(giri[i], (char *)&m_BufRX[11+8*i], 4); // GGGG
                giri[i][4] = 0;
            }
        }
        else             // Frame errato, NAK -> Comando non supportato
        {
            for (int i=0; i<8; ++i)
            {
                strcpy(opa_k[i], "0.00");
                strcpy(giri[i],  "0000");
            }
        }
    }

    return vret;
}
//----------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_RdAutozeroTime(char *time)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_RdAutozeroTime(time);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::opa_RdAutozeroTime(char *time)
{
    char            gpv[32];
    int             vret;

    if (Demo)
    {
        strcpy(time, "20");
        vret = OPA_SERIAL_OK;
    }
    else
    {
        strcpy(gpv, COM_RD_AUTOZERO.c_str()); // "170"
        vret = PutOmniBUS(COM_RD_AUTOZERO, gpv, m_szAdrsOPA);
        if(vret == OPA_SERIAL_OK) //OK
        {
            strncpy(time, (char *)&m_BufRX[7], 2); // TT
            time[2] = 0;
        }
    }

    return vret;
}
//----------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_WrAutozeroTime(const char *time)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_WrAutozeroTime(time);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::opa_WrAutozeroTime(const char *time)
{
    char            gpv[32];
    int             vret;

    if (Demo)
        return OPA_SERIAL_OK;

    strcpy(gpv, COM_WR_AUTOZERO.c_str());                                       // "171"
    strcat(gpv, time);

    vret = PutOmniBUS(COM_WR_AUTOZERO, gpv, m_szAdrsOPA);

    return vret;
}
//---------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_RdBlockHistoryValues(const char *num_sample,
                                                 char *opa_p[],
                                                 char *opa_k[],
                                                 char *giri[])
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->BlkValoriStorico(num_sample, opa_p, opa_k, giri);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::BlkValoriStorico(const char *num_sample,
                                                 char *opa_p[],
                                                 char *opa_k[],
                                                 char *giri[])
{
    char          gpv[32];
    int           vret;

    if (Demo)
    {
        for(int i=0; i<5; ++i)
        {
            strcpy(opa_p[i], "00.0");
            strcpy(opa_k[i], "0.00");
            strcpy(giri[i],  "0000");
        }
        vret = OPA_SERIAL_OK;
    }
    else
    {
        strcpy(gpv, COM_BLK_VALST.c_str());                                     // "173"
        strcat(gpv, num_sample);        // Nr (01-50)
        vret = PutOmniBUS(COM_BLK_VALST, gpv, m_szAdrsOPA);
        if(vret == OPA_SERIAL_OK) // OK
        {
            for(int i=0; i<5; ++i)
            {
                strncpy(opa_p[i], (char *)&m_BufRX[ 7+12*i],  4); // PP.P
                opa_p[i][4] = 0;
                strncpy(opa_k[i], (char *)&m_BufRX[11+12*i],  4); // M.MM
                opa_k[i][4] = 0;
                strncpy(giri[i],  (char *)&m_BufRX[15+12*i], 4);  // GGGG
                giri[i][4]  = 0;
            }
        }
        else             // Frame errato, NAK -> Comando non supportato
        {
            for(int i=0; i<5; ++i)
            {
                strcpy(opa_p[i], "00.0");
                strcpy(opa_k[i], "0.00");
                strcpy(giri[i],  "0000");
            }
        }
    }

    return vret;
}

//---------------------------------------------------------------------------
#ifdef IDE_ENABLEIPERCONNESSIONE

#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_RdBlockHistoryValuesANN(const char *num_sample,
                                          char * const pszOpaPreFiltro[],
                                          char * const pszOpaK[],
                                          char * const pszGiri[],
                                          char * const pszPressione[],
                                          char * const pszOpaAD[]
                                          )
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->BlkValoriStoricoANN(num_sample, pszOpaPreFiltro, pszOpaK, pszGiri, pszPressione, pszOpaAD);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::BlkValoriStoricoANN(const char * const pszNrSample,
                                                  char * const pszOpaPreFiltro[],
                                                  char * const pszOpaK[],
                                                  char * const pszGiri[],
                                                  char * const pszPressione[],
                                                  char * const pszOpaAD[]
                                                  )
{
    char          gpv[32];
    int           vret;

    if (Demo)
    {
        for(int ii = 0; ii < 5; ++ii)
        {
            strcpy(pszOpaPreFiltro[ii], "+00.0");
            strcpy(pszOpaK[ii],         "0.00");
            strcpy(pszGiri[ii],         "0000");
            strcpy(pszPressione[ii],    "+0.00");
            strcpy(pszOpaAD[ii],        "00000");
        }
        vret = OPA_SERIAL_OK;
    }
    else
    {
#define LENGTH_TRAMA  ( 23 )

        strcpy(gpv, COM_BLK_VALST.c_str());                                     // "173"
        strcat(gpv, pszNrSample);                                               // Nr (001-160)

        vret = PutOmniBUS(COM_BLK_VALST, gpv, m_szAdrsOPA);
        if (vret == OPA_SERIAL_OK) // OK
        {
            for (int ii = 0; ii < 5; ++ii)
            {
                strncpy(pszOpaPreFiltro[ii],    (char *)&m_BufRX[ 7 + LENGTH_TRAMA*ii], 5); // +PP.P
                pszOpaPreFiltro[ii][5] = '\0';

                strncpy(pszOpaK[ii],            (char *)&m_BufRX[12 + LENGTH_TRAMA*ii], 4); // M.MM
                pszOpaK[ii][4]      = '\0';

                strncpy(pszGiri[ii],            (char *)&m_BufRX[16 + LENGTH_TRAMA*ii], 4); // GGGG
                pszGiri[ii][4]      = '\0';

                strncpy(pszPressione[ii],       (char *)&m_BufRX[20 + LENGTH_TRAMA*ii], 5); // +PP.P
                pszPressione[ii][5] = '\0';

                strncpy(pszOpaAD[ii],           (char *)&m_BufRX[25 + LENGTH_TRAMA*ii], 5); // OOOOO
                pszOpaAD[ii][5]     = '\0';
            }
        }
        else             // Frame errato, NAK -> Comando non supportato
        {
            for (int ii = 0; ii < 5; ++ii)
            {
                strcpy(pszOpaPreFiltro[ii], "+00.0");
                strcpy(pszOpaK[ii],         "0.00");
                strcpy(pszGiri[ii],         "0000");
                strcpy(pszPressione[ii],    "+0.00");
                strcpy(pszOpaAD[ii],        "00000");
            }
        }
#undef LENGTH_TRAMA
    }

    return vret;
}

#endif

//----------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_RdAllData(OPA_SMOKEVALUE_S *fumVal, OPA_SMOKEVALUE_S *fumValPeak, OPA_ANALOGVALUE_S *valAux, OPA_STATUS_S *opaStatus, const char *giri, const char *temp)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->RwAllData(fumVal, fumValPeak, valAux, opaStatus, giri, temp);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::RwAllData(OPA_SMOKEVALUE_S *fumVal, OPA_SMOKEVALUE_S *fumValPeak, OPA_ANALOGVALUE_S *valAux, OPA_STATUS_S *opaStatus, const char *giri, const char *temp)
{
    char            gpv[128];
    char            buf[8];
    int             vret;
    int             val;

    if (Demo)
    {
        fumVal->dOpacity_k = 4.10+((float)(rand()%100)/100.0);
        fumVal->dOpacity_p = 100.0*(1.0-(exp(-fumVal->dOpacity_k*0.43)));
        fumVal->iRpm        =1870+(rand()%100);

        sprintf(fumVal->szOpacity_k, "%4.2f", fumVal->dOpacity_k);
        sprintf(fumVal->szRpm, "%4d", fumVal->iRpm);

        fumVal->iValues = 1;
        fumVal->dValue_p[0] = fumVal->dOpacity_p;
        fumVal->dValue_k[0] = fumVal->dOpacity_k;

        SleepEx(200, false);
        return OPA_SERIAL_OK;
    }

    strcpy(gpv, COM_RW_ALLDATA.c_str());                                        // "174"
    if (giri)
    {
        if ( (! strcmp(giri, "XXXX")) || (atoi(giri) == 0) )
        {
            strcat(gpv, "XXXX");
        }
        else
        {
            val = atoi(giri); // ----- XXXX
            if(val > 9990)
                val = 9990;
            sprintf(buf, "%04d", val);
            strcat(gpv, buf);
        }
    }
    else
        strcat(gpv, "XXXX");

    if (temp)
    {
        if ( (! strcmp(temp, "XXX")) || (atoi(temp) == 0) )
        {
            strcat(gpv, "XXX");
        }
        else
        {
            val = atoi(temp); // --- XXX
            if (val > 1000)
            {
                val = 999;
            }
            sprintf(buf, "%03d", val);
            strcat(gpv, buf);
        }
    }
    else
        strcat(gpv, "XXX");    

    vret = PutOmniBUS(COM_RW_ALLDATA, gpv, m_szAdrsOPA);
    if (vret == OPA_SERIAL_OK) // OK
    {
/*
#ifdef _DEBUG
        strcpy(fumVal->szRpm,  "1000");
        strcpy(valAux->szSmokeTemp,  "128");
        strcpy(valAux->szOilTemp,  "068");
#else
*/
        // VALORI ISTANTANEI GIRI, TEMPERATURA OLIO, TEMPERATURA FUMI
        if (fumVal)
        {
            memcpy(fumVal->szRpm,        (char*)&m_BufRX[ 7], 4 );  // GGGG
            fumVal->szRpm[4] = '\0';
        }
        if (valAux)
        {
            memcpy(valAux->szSmokeTemp,  (char*)&m_BufRX[11], 3) ; // FFF
            valAux->szSmokeTemp[3] = '\0';
            memcpy(valAux->szOilTemp,    (char*)&m_BufRX[14], 3) ; // OOO
            valAux->szOilTemp[3] = '\0';
        }
// #endif

        // STATO
        if (opaStatus)
        {
            opaStatus->byte_status1         = HexASCII2Bin(&m_BufRX[ 7+10]);
            opaStatus->byte_status2         = HexASCII2Bin(&m_BufRX[ 9+10]);
            opaStatus->byte_error1          = HexASCII2Bin(&m_BufRX[11+10]);
            opaStatus->byte_error2          = HexASCII2Bin(&m_BufRX[13+10]);

            // S1
            opaStatus->warming_up               = (BYTE)((opaStatus->byte_status1 & 0x01) >> 0);
            opaStatus->zero_perf                = (BYTE)((opaStatus->byte_status1 & 0x02) >> 1);
            // BYTE bit2 ---
            opaStatus->heaters                  = (BYTE)((opaStatus->byte_status1 & 0x08) >> 3);
            opaStatus->cooling_fans             = (BYTE)((opaStatus->byte_status1 & 0x10) >> 4);
            opaStatus->led_on                   = (BYTE)((opaStatus->byte_status1 & 0x20) >> 5);
            opaStatus->cleaning_fans            = (BYTE)((opaStatus->byte_status1 & 0x40) >> 6);
            opaStatus->solenoid_zero            = (BYTE)((opaStatus->byte_status1 & 0x80) >> 7);

            // S2
            opaStatus->zero_request             = (BYTE)((opaStatus->byte_status2 & 0x01) >> 0);
            opaStatus->history_perf             = (BYTE)((opaStatus->byte_status2 & 0x02) >> 1);
            opaStatus->autocal_current_led      = (BYTE)((opaStatus->byte_status2 & 0x04) >> 2);
            opaStatus->history_start            = (BYTE)((opaStatus->byte_status2 & 0x08) >> 3);
            opaStatus->history_ann_perf         = m_ucHistoryANNPerf;
            // unsigned char  bit4 ---
            // unsigned char  bit5 ---
            // unsigned char  bit6 ---
            // unsigned char  bit7 ---

            // E1
            opaStatus->error_eeprom_notworking  = (BYTE)((opaStatus->byte_error1 & 0x01) >> 0);
            opaStatus->error_eeprom_corrupt     = (BYTE)((opaStatus->byte_error1 & 0x02) >> 1);
            opaStatus->fan_emitter_blocked      = (BYTE)((opaStatus->byte_error1 & 0x04) >> 2);
            opaStatus->fan_receiver_blocked     = (BYTE)((opaStatus->byte_error1 & 0x08) >> 3);
            opaStatus->heaters_notworking       = (BYTE)((opaStatus->byte_error1 & 0x10) >> 4);
            opaStatus->error_high_temperature   = (BYTE)((opaStatus->byte_error1 & 0x20) >> 5);
            opaStatus->error_lowpower           = (BYTE)((opaStatus->byte_error1 & 0x40) >> 6);
            opaStatus->error_highpower          = (BYTE)((opaStatus->byte_error1 & 0x80) >> 7);

            // E2
            opaStatus->error_zero               = (BYTE)((opaStatus->byte_error2 & 0x01) >> 0);
            opaStatus->error_lummax_low         = (BYTE)((opaStatus->byte_error2 & 0x02) >> 1);
            opaStatus->error_lumzero_high       = (BYTE)((opaStatus->byte_error2 & 0x04) >> 2);
            opaStatus->error_lummax_void        = (BYTE)((opaStatus->byte_error2 & 0x08) >> 3);
            opaStatus->error_calibration        = (BYTE)((opaStatus->byte_error2 & 0x10) >> 4);
            opaStatus->error_filter             = (BYTE)((opaStatus->byte_error2 & 0x20) >> 5);
            opaStatus->error_thermostat         = (BYTE)((opaStatus->byte_error2 & 0x40) >> 6);
            opaStatus->error_autocal_led        = (BYTE)((opaStatus->byte_error2 & 0x80) >> 7);
        }

        // VALORI ISTANTANEI OPACITA
        char        buffer[8];
        int         tramaIst = 0;

        if (fumVal)
        {
            fumVal->iValues = 0;
            for (int xx = 0; xx < 8; ++xx)
            {
                strncpy(buffer, (char *)&m_BufRX[25+xx*8],  4);
                buffer[4]             = '\0';
                fumVal->dValue_p[xx]  = atof(buffer);

                if (buffer[0] != '-')
                {
                    ++fumVal->iValues;

                    tramaIst = xx;
                }

                strncpy(buffer, (char *)&m_BufRX[29+xx*8],  4);
                buffer[4]             = '\0';
                fumVal->dValue_k[xx]  = atof(buffer);
            }
        }
/*
#ifdef _DEBUG
        strcpy(fumVal->szOpacity_p, "12.3");
        strcpy(fumVal->szOpacity_k, "0.47");

        // VALORI DI PICCO
        strcpy(fumValPeak->szOpacity_p, "20.0");
        strcpy(fumValPeak->szOpacity_k, "13.7");
        strcpy(fumValPeak->szRpm,       "0930");

        // PRESSIONE
        strcpy(valAux->szChamberPressure,  "-12.5");
#else
*/
        // ULTIMO CAMPIONE VALIDO FA DA VALORE ISTANTANEO
        if (fumVal)
        {
            memcpy(fumVal->szOpacity_p, (char*)&m_BufRX[25+tramaIst*8], 4); // PP.P
            fumVal->szOpacity_p[4]      = '\0';
            memcpy(fumVal->szOpacity_k, (char*)&m_BufRX[29+tramaIst*8], 4); // M.MM
            fumVal->szOpacity_k[4]      = '\0';
        }
        // VALORI DI PICCO
        if (fumValPeak)
        {
            memcpy(fumValPeak->szOpacity_p, (char*)&m_BufRX[89], 4); // PP.P
            fumValPeak->szOpacity_p[4]  = '\0';
            memcpy(fumValPeak->szOpacity_k, (char*)&m_BufRX[93], 4); // M.MM
            fumValPeak->szOpacity_k[4]  = '\0';
            memcpy(fumValPeak->szRpm,       (char*)&m_BufRX[97], 4); // GGGG
            fumValPeak->szRpm[4]        = '\0';
        }

        // PRESSIONE
        if (valAux)
        {
            memcpy(valAux->szChamberPressure,     (char*)&m_BufRX[101], 5); // +PP.P
            valAux->szChamberPressure[5] = '\0';
        }
// #endif

        if (fumVal)
        {
            fumVal->iRpm             = atoi(fumVal->szRpm);
            fumVal->dOpacity_p       = atof(fumVal->szOpacity_p);
            fumVal->dOpacity_k       = atof(fumVal->szOpacity_k);

            ZCanc(fumVal->szRpm);
            ZCanc(fumVal->szOpacity_p);
            ZCanc(fumVal->szOpacity_k);
        }

        if (fumValPeak)
        {
            fumValPeak->dOpacity_p   = atof(fumValPeak->szOpacity_p);
            fumValPeak->dOpacity_k   = atof(fumValPeak->szOpacity_k);
            fumValPeak->iRpm         = atoi(fumValPeak->szRpm);

            ZCanc(fumValPeak->szOpacity_p);
            ZCanc(fumValPeak->szOpacity_k);
            ZCanc(fumValPeak->szRpm);
        }

        if (valAux)
        {
            ZCanc(valAux->szSmokeTemp);
            ZCanc(valAux->szOilTemp);
        }
    }

    return vret;
}

//----------------------------------------------------------------------
#ifdef IDE_ENABLEIPERCONNESSIONE
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_RdAllDataANN(OPA_SMOKEVALUE_S *fumVal, OPA_ANALOGVALUE_S *valAux, OPA_STATUS_S *opaStatus, char *pszRevision)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->RwAllDataANN(fumVal, valAux, opaStatus, pszRevision);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::RwAllDataANN(OPA_SMOKEVALUE_S *fumVal, OPA_ANALOGVALUE_S *valAux, OPA_STATUS_S *opaStatus, char *pszRevision)
{
    char            szGpv[128];
    int             vret;
    int             val;
    bool            bULTRON;

    if (Demo)
    {
        fumVal->dOpacity_k = 4.10+((float)(rand()%100)/100.0);
        fumVal->dOpacity_p = 100.0*(1.0-(exp(-fumVal->dOpacity_k*0.43)));
        fumVal->iRpm        =1870+(rand()%100);

        sprintf(fumVal->szOpacity_k, "%4.2f", fumVal->dOpacity_k);
        sprintf(fumVal->szRpm, "%4d", fumVal->iRpm);

        fumVal->iValues = 1;
        fumVal->dValue_p[0] = fumVal->dOpacity_p;
        fumVal->dValue_k[0] = fumVal->dOpacity_k;

        opaStatus->byte_status1  = 0x00;
        opaStatus->byte_status2  = 0x00;
        opaStatus->byte_error1   = 0x00;
        opaStatus->byte_error2   = 0x00;

        SleepEx(200, false);

        return OPA_SERIAL_OK;
    }

    bULTRON = (strlen(pszRevision) == 1);

    m_aTime[m_iNdx] = FormatDateTime("hh:nn:ss.zzz", Now()) + "|";

    strcpy(szGpv, COM_RW_ALLDATA2.c_str());                                       // "178"
    strcat(szGpv, pszRevision);

    vret = PutOmniBUS(COM_RW_ALLDATA2, szGpv, m_szAdrsOPA);
    if (vret == OPA_SERIAL_OK) // OK
    {
        // VALORI ISTANTANEI TEMPERATURA OLIO
        memcpy(valAux->szOilTemp,    (char*)&m_BufRX[7], 3) ; // OOO
        valAux->szOilTemp[3]                = '\0';
        ZCanc(valAux->szOilTemp);

        if (opaStatus)
        {
            // STATO
            opaStatus->byte_status1             = HexASCII2Bin(&m_BufRX[10]);
            opaStatus->byte_status2             = HexASCII2Bin(&m_BufRX[12]);
            opaStatus->byte_error1              = HexASCII2Bin(&m_BufRX[14]);
            opaStatus->byte_error2              = HexASCII2Bin(&m_BufRX[16]);

            // S1
            opaStatus->warming_up               = (BYTE)((opaStatus->byte_status1 & 0x01) >> 0);
            opaStatus->zero_perf                = (BYTE)((opaStatus->byte_status1 & 0x02) >> 1);
            // BYTE bit2 ---
            opaStatus->heaters                  = (BYTE)((opaStatus->byte_status1 & 0x08) >> 3);
            opaStatus->cooling_fans             = (BYTE)((opaStatus->byte_status1 & 0x10) >> 4);
            opaStatus->led_on                   = (BYTE)((opaStatus->byte_status1 & 0x20) >> 5);
            opaStatus->cleaning_fans            = (BYTE)((opaStatus->byte_status1 & 0x40) >> 6);
            opaStatus->solenoid_zero            = (BYTE)((opaStatus->byte_status1 & 0x80) >> 7);

            // S2
            opaStatus->zero_request             = (BYTE)((opaStatus->byte_status2 & 0x01) >> 0);
            opaStatus->history_perf             = (BYTE)((opaStatus->byte_status2 & 0x02) >> 1);
            opaStatus->autocal_current_led      = (BYTE)((opaStatus->byte_status2 & 0x04) >> 2);
            opaStatus->history_start            = (BYTE)((opaStatus->byte_status2 & 0x08) >> 3);
            opaStatus->history_ann_perf         = m_ucHistoryANNPerf;
            // unsigned char  bit4 ---
            // unsigned char  bit5 ---
            // unsigned char  bit6 ---
            // unsigned char  bit7 ---

            // E1
            opaStatus->error_eeprom_notworking  = (BYTE)((opaStatus->byte_error1 & 0x01) >> 0);
            opaStatus->error_eeprom_corrupt     = (BYTE)((opaStatus->byte_error1 & 0x02) >> 1);
            opaStatus->fan_emitter_blocked      = (BYTE)((opaStatus->byte_error1 & 0x04) >> 2);
            opaStatus->fan_receiver_blocked     = (BYTE)((opaStatus->byte_error1 & 0x08) >> 3);
            opaStatus->heaters_notworking       = (BYTE)((opaStatus->byte_error1 & 0x10) >> 4);
            opaStatus->error_high_temperature   = (BYTE)((opaStatus->byte_error1 & 0x20) >> 5);
            opaStatus->error_lowpower           = (BYTE)((opaStatus->byte_error1 & 0x40) >> 6);
            opaStatus->error_highpower          = (BYTE)((opaStatus->byte_error1 & 0x80) >> 7);

            // E2
            opaStatus->error_zero               = (BYTE)((opaStatus->byte_error2 & 0x01) >> 0);
            opaStatus->error_lummax_low         = (BYTE)((opaStatus->byte_error2 & 0x02) >> 1);
            opaStatus->error_lumzero_high       = (BYTE)((opaStatus->byte_error2 & 0x04) >> 2);
            opaStatus->error_lummax_void        = (BYTE)((opaStatus->byte_error2 & 0x08) >> 3);
            opaStatus->error_calibration        = (BYTE)((opaStatus->byte_error2 & 0x10) >> 4);
            opaStatus->error_filter             = (BYTE)((opaStatus->byte_error2 & 0x20) >> 5);
            opaStatus->error_thermostat         = (BYTE)((opaStatus->byte_error2 & 0x40) >> 6);
            opaStatus->error_autocal_led        = (BYTE)((opaStatus->byte_error2 & 0x80) >> 7);
        }

        // GIRI ISTANTANEI
        memcpy(fumVal->szRpm,               (char*)&m_BufRX[18], 4);  // GGGG
        fumVal->szRpm[4]                    = '\0';
        fumVal->iRpm                        = atoi(fumVal->szRpm);
        ZCanc(fumVal->szRpm);

        // VALORI ISTANTANEI OPACITA
        #define START_TRAMA         ( 22 )
        int LENGTH_TRAMA;
        int INTRAOFFSET_TRAMA;

        char        szBuffer[16];
        int         iTramaIst = 0;

        if (bULTRON)
        {
            LENGTH_TRAMA        =  5;
            INTRAOFFSET_TRAMA   =  0;
        }
        else
        {
            LENGTH_TRAMA        =  14;
            INTRAOFFSET_TRAMA   =  5;
        }

        // VALORI ISTANTANEI PRESSIONE, OPACITA GREZZA, OPACITA(k)
        fumVal->iValues = 0;
        for (int ii = 0; ii < 12; ++ii)
        {
            if (! bULTRON)
            {
                // 1) PRESSIONE PER SAMPLE CONTIENE SEGNO - !!!
                strncpy(szBuffer, (char *)&m_BufRX[START_TRAMA + (ii*LENGTH_TRAMA)],  5);
                szBuffer[5]                     = '\0';
                fumVal->dPressione[ii]          = atof(szBuffer);
            }

            // 2) OPACITAPREFILTRO PER SAMPLE CONTIENE SEGNO - !!!!
            strncpy(szBuffer, (char *)&m_BufRX[START_TRAMA + INTRAOFFSET_TRAMA + (ii*LENGTH_TRAMA)],  5);
            szBuffer[5]                     = '\0';
            fumVal->dOpacitaPreFiltro[ii]    = atof(szBuffer);

            if (! bULTRON)
            {
                // 3) OPACITA K (POSTFILTRO)
                strncpy(szBuffer, (char *)&m_BufRX[START_TRAMA + 10 + (ii*LENGTH_TRAMA)],  4);
                szBuffer[4]                     = '\0';
                fumVal->dValue_k[ii]            = atof(szBuffer);
            }

            // Conta campioni
            if (szBuffer[1] != '-')   // secondo e sempre un numero !!!!
            {
                ++fumVal->iValues;

                if (! bULTRON)
                {
                    iTramaIst = ii;
                }
            }
        }

        if (opaStatus)
        {
            if (opaStatus->history_ann_perf == TRUE)  // Scattara fase di attivazione storico -> ACCELERARE !!!
            {
                for (int ll = 0; ll < fumVal->iValues; ++ll)
                {
                    // ANALISI FILTRA ALTO
                    // Opacita grezza o prefiltro -> m_dInPOpaANN[jj] o DataRow
                    assert(m_iNdx < DIMARRAY);

                    double DataRaw = m_dInPOpaANN[m_iNdx] = fumVal->dOpacitaPreFiltro[ll];

                    if (m_DataRawOld == 0.0)
                    {
                        m_DataRawOld = DataRaw;
                    }

                    // prima faccio filtro passa Alto                   ingresso
                    m_DataFilteredHP = m_HPF_Alfa * (m_DataFilteredHP + DataRaw - m_DataRawOld);      //  y[i] := α * y[i-1] + α * (x[i] - x[i-1])
                    m_vDataFilteredHP[m_iNdx] = m_DataFilteredHP;

                    // update
                    m_DataRawOld = DataRaw;

                    // poi faccio filtro passa Basso                                 ingresso alto
                    m_DataFilteredHPLP = m_DataFilteredHPLP * (1.0 - m_LPF_Beta) + m_DataFilteredHP * m_LPF_Beta;  //  y[i] := α * x[i] + (1-α) * y[i-1]
                    m_vDataFilteredHPLP[m_iNdx] = m_DataFilteredHPLP;

                    // trigger sul 2.0 di opacita grezza filtrata alta
                    if ( (m_RiseStart == -1) && (m_DataFilteredHP > 2.0) )  // in %
                    {
                        m_RiseStart = m_iNdx;   // inizio salita trapezio
                    }

                    if ( (m_DescEnd == -1) && (m_RiseStart != -1) )   // Triggerata ANN
                    {
                        if (
                             ( (m_DataFilteredHP < 2.0) && (m_iNdx > (m_RiseStart + 20)) ) // Chiusura del trapezio
                                  ||
                             (m_iNdx >= (m_RiseStart + 160))                               // Raggiunto limite dei 4 sec di accelererata
                           )
                        {
                            m_DescEnd = m_iNdx;    // fine discesa trapezio
                        }
                    }

                    // double val = DataFiltered;
                    if ( (m_RiseStart != -1) && (m_DescEnd == -1) )
                    {
                        m_dSomma += m_DataFilteredHP;   //somma se all'interno del trapezio
                    }

                    if ( (m_PiccoFiltratoHP < m_DataFilteredHP) && (m_DataFilteredHP > 2.0) )        // filtro passa alto
                    {
                        // Aggiornamento picco HIGH filter
                        m_PiccoFiltratoHP   = m_DataFilteredHP;
                        m_RiseEnd           = m_iNdx;           // memorizzo fine salita trapezio
                    }

                    if (m_PiccoFiltratoHPLP < m_DataFilteredHPLP)   // filtro passa alto/basso
                    {
                        // Aggiornamento picco HIGH/LOW filter
                        m_PiccoFiltratoHPLP = m_DataFilteredHPLP;
                        m_HPLPMax           = m_iNdx;           // memorizzo fine salita trapezio
                    }

                    // Aggiornamento nuovo campione
                    ++m_iNdx;

                    m_aTime[m_iNdx]  = AnsiString("            |");
                }

                // USCITA DAL CICLO
                if (m_RiseStart != -1)  // se trigerata ANN
                {
                    if (m_DescEnd != -1) // concluso trapezio
                    {
                        m_ucHistoryANNPerf              = FALSE;
                        opaStatus->history_ann_perf     = FALSE;

                        m_bANNOpacityPeakChoice         = true;

                        if (Log == true)
                        {
                            AnsiString aMsg = "ANN ACCELERATION COMPLETED";
                            m_logOPA->Add(aMsg);
                        }
                    }
                }
                // Controllo eventuale accelerata rilevata dall'opacimetro se (m_RiseStart == -1)
                else if (opaStatus->history_perf == FALSE)
                {
                    //
                    m_ucHistoryANNPerf              = FALSE;
                    opaStatus->history_ann_perf     = FALSE;

                    m_bANNOpacityPeakChoice         = false;

                    if (Log == true)
                    {
                        AnsiString aMsg = "OPACIMETER ACCELERATION COMPLETED";
                        m_logOPA->Add(aMsg);
                    }
                }
            }
        }

        if (bULTRON)
        {
            // TEMPERATURA FUMI
            strncpy(valAux->szSmokeTemp,    (char *)&m_BufRX[START_TRAMA + (LENGTH_TRAMA * 12) + 0], 3); // FFF
            valAux->szSmokeTemp[3]          = '\0';
            ZCanc(valAux->szSmokeTemp);

            // OPACITA ISTANTANEA K
            strncpy(fumVal->szOpacity_k,    (char *)&m_BufRX[START_TRAMA + (LENGTH_TRAMA * 12) + 3], 4); // M.MM
            fumVal->szOpacity_k[4]          = '\0';
            fumVal->dOpacity_k              = atof(fumVal->szOpacity_k);
            ZCanc(fumVal->szOpacity_k);

            // PRESSIONE ISTANTANEA
            strncpy(valAux->szChamberPressure,  (char *)&m_BufRX[START_TRAMA + (LENGTH_TRAMA * 12) + 7], 5); // +PP.P
            valAux->szChamberPressure[5]    = '\0';
        }
        else
        {
            // ULTIMO CAMPIONE VALIDO FA DA PRESSIONE ISTANTANEA
            strncpy(valAux->szChamberPressure,  (char *)&m_BufRX[START_TRAMA + 0 + (iTramaIst*LENGTH_TRAMA)], 5); // +PP.P
            valAux->szChamberPressure[5]    = '\0';

            // ULTIMO CAMPIONE VALIDO FA DA OPACITA ISTANTANEA
            strncpy(fumVal->szOpacity_k,    (char *)&m_BufRX[START_TRAMA + 10 + (iTramaIst*LENGTH_TRAMA)],  4); // M.MM
            fumVal->szOpacity_k[4]          = '\0';
            fumVal->dOpacity_k              = atof(fumVal->szOpacity_k);
            ZCanc(fumVal->szOpacity_k);
        }
    }

    return vret;
}
#endif

/*
// IMPOSTAZIONE SOGLIA DI PRESSIONE
//  opaSeriale->opa_RWTriggerPressione(false, "----");    // scrittura soglia 99.9

int iRet = opaSeriale->opa_RWTriggerPressione(true, m_szPressione);  // 177 lettura
if (iRet == TOpaSeriale::V_OK)
{
    if (m_dlgSimulazione != NULL)
    {                                 // attuale
        strcpy(m_szPressione, m_dlgSimulazione->TriggerPressure.c_str());
    }
    m_dPressione = atof(m_szPressione);

    opaSeriale->opa_RWTriggerPressione(false, m_szPressione);        // 177 scrittura
}
*/

#ifdef IDE_ENABLEIPERCONNESSIONE
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_RdWrPressureTrigger(bool bEnableRead, char * const pszSoglia)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_RWTriggerPressione(bEnableRead, pszSoglia);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::opa_RWTriggerPressione(bool bEnableRead, char * const pszSoglia)
{
    char            szGpv[20];
    int             iRet = OPA_SERIAL_OK;

    if (Demo)
    {
        if (bEnableRead == true)
        {
            strcpy(pszSoglia, "00.2");
        }
    }
    else
    {
        strcpy(szGpv, COM_RW_TRG_PRES.c_str());                                 // 177
        if (bEnableRead == true)
        {
            strcat(szGpv, "1");
        }
        else
        {
            strcat(szGpv, "0");
            strcat(szGpv, pszSoglia);
        }

        iRet = PutOmniBUS(COM_RW_TRG_PRES, szGpv, m_szAdrsOPA);
        if (iRet == OPA_SERIAL_OK)
        {
            if (bEnableRead == true)
            {
                strncpy(pszSoglia, (char *)&m_BufRX[7], 4);
                pszSoglia[4] = '\0';
            }
        }
    }

    return iRet;
}
#endif


//---------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_RdIdentification(char *marca, char *modello, char *nromol)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->RdIdentCamera(marca, modello, nromol);
    }

    return OPA_SERIAL_VOID;

};
#endif

int __fastcall TOpaSeriale::RdIdentCamera(char *marca, char *modello, char *nromol)
{
    char            gpv[32];
    int             vret;
    unsigned int    cont;
    unsigned int    ii;

    if (Demo)
        return OPA_SERIAL_OK;

    strcpy(gpv, COM_RD_IDENT.c_str());                                          // "271"
    vret = PutOmniBUS(COM_RD_IDENT, gpv, m_szAdrsOPA);
    if(vret == OPA_SERIAL_OK) // OK
    {
        cont = 7;
        ii   = 7;
        for (unsigned int n=1; n<=3; ++n)
        {
            while (m_BufRX[ii++] != DLE);
            m_BufRX[--ii]=0;
            switch(n)
            {
                case 1:
                    strncpy(marca,   (char *)&m_BufRX[cont], 15); // MARCA
                    marca[15]   = '\x00';
                    break;
                case 2:
                    strncpy(modello, (char *)&m_BufRX[cont], 15); // MODELLO
                    modello[15] = '\x00';
                    break;
                case 3:
                    strncpy(nromol,  (char *)&m_BufRX[cont], 23); // OMOLOGAZIONE
                    nromol[23]  = '\x00';
                    break;
            }
            cont = ++ii;
        }
    }
    else             // Frame errato, NAK -> Comando non supportato
    {
        strcpy(marca,   "");
        strcpy(modello, "");
        strcpy(nromol,  "");
    }

    return vret;
}
//---------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_WrIdentification(const char *marca, const char *modello, const char *nromol)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_WrIdentCamera(marca, modello, nromol);
    }

    return OPA_SERIAL_VOID;

};
#endif

int __fastcall TOpaSeriale::opa_WrIdentCamera(const char *marca, const char *modello, const char *nromol)
{
    char          gpv[80];
    int           vret;

    if (Demo)
        return OPA_SERIAL_OK;

    strcpy(gpv, COM_WR_IDENT.c_str()); // "272"
    strcat(gpv, marca);
    strcat(gpv, "\x10");
    strcat(gpv, modello);
    strcat(gpv, "\x10");
    strcat(gpv, nromol);
    strcat(gpv, "\x10");

    vret = PutOmniBUS(COM_WR_IDENT, gpv, m_szAdrsOPA);

    return vret;
}
//---------------------------------------------------------------------------
// Comando scrittura giri su OPA-100
// Abilitato dalla versione 1.105
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_WrRpm(const char *giri)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->WrGiri(giri);
    }

    return OPA_SERIAL_VOID;

};
#endif

int __fastcall TOpaSeriale::WrGiri(const char *giri)
{
    char            gpv[16];
    char            rpm[5+1];
    int             val;
    int             vret;

    if(Demo) return OPA_SERIAL_OK;

    val = atoi(giri); // ---- XXXX
    if(val > 9990)
        val = 9990;
    sprintf(rpm, "%04d", val);
    strcpy(gpv, COM_WR_RPM.c_str()); // "280"
    strcat(gpv, rpm);

    vret = PutOmniBUS(COM_WR_RPM, gpv, m_szAdrsOPA);

    return vret;
}
//---------------------------------------------------------------------------
// Comando scrittura giri e temperatura su OPA-100
// Abilitato dalla versione 1.105
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_WrRpmTemp(const char *giri, const char *temp)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->WrGiriTemp(giri, temp);
    }

    return OPA_SERIAL_VOID;

};
#endif

int __fastcall TOpaSeriale::WrGiriTemp(const char *giri, const char *temp)
{
    char            gpv[16];
    char            buf[5+1];
    int             val;
    int             vret;

    if(Demo) return OPA_SERIAL_OK;

    // RPM
    val = atoi(giri); // ---- XXXX
    if(val > 9990)
        val = 9990;
    sprintf(buf, "%04d", val);
    strcpy(gpv, COM_WR_RPM_TEMP.c_str()); // "281"
    strcat(gpv, buf);
    // TEMP
    val = atoi(temp); // --- XXX
    if(val > 1000)
        val = 999;
    sprintf(buf, "%03d", val);
    strcat(gpv, buf);
    vret = PutOmniBUS(COM_WR_RPM_TEMP, gpv, m_szAdrsOPA);

    return vret;
}
// SCU INTERNO
//---------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_RdSCU(char *pszOraData)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_RdScu(pszOraData);
    }

    return OPA_SERIAL_VOID;

};
#endif

int __fastcall TOpaSeriale::opa_RdScu(char *pszOraData)
{
    char          szGpv[32];
    int           iRet;

    strcpy(szGpv, COM_RD_SCU.c_str()); // "282"

    iRet = PutOmniBUS(COM_RD_SCU, szGpv, "20"); // Ags_address
    if (iRet == OPA_SERIAL_OK)
    {
        strncpy(pszOraData, (char *)&m_BufRX[7], 15); // hh:nnGG.MM.AAAA
        pszOraData[15] = 0;
    }

    return iRet;
}
//---------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_WrSCU(const char *pszOraData)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_WrScu(pszOraData);
    }

    return OPA_SERIAL_VOID;

};
#endif

int __fastcall TOpaSeriale::opa_WrScu(const char *pszOraData)
{
    char          szGpv[32];

    strcpy(szGpv, COM_WR_SCU.c_str()); // "283"
    strcat(szGpv, pszOraData);

    return PutOmniBUS(COM_WR_SCU, szGpv, "20");
}


/*
******************************************************************************
**                                                                          **
**                    COMANDI COMUNI SISTEMA omniBUS                        **
**                                                                          **
******************************************************************************
*/
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_RunIn(void)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_RunIn();
    }

    return OPA_SERIAL_VOID;

};
#endif

int __fastcall TOpaSeriale::opa_RunIn(void)
{
    char            gpv[20];
    int             vret;

    if (Demo)
        return OPA_SERIAL_OK;

    strcpy(gpv, COM_RUN_IN.c_str()); // "024"
    vret = PutOmniBUS(COM_RUN_IN, gpv, m_szAdrsOPA);

    return vret;
}

//------------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_EEpromValues(char *nn, char *buff)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->opa_RdParametri(nn, buff);
    }

    return OPA_SERIAL_VOID;

};
#endif

int __fastcall TOpaSeriale::opa_RdParametri(char *nn, char *buff)
{
    char            gpv[32];
    int             vret;

    if (Demo)
    {
        strcpy(buff, "123456789012345678901234");
        vret = OPA_SERIAL_OK;
    }
    else
    {
        strcpy(gpv, COM_RD_PARAMETRI.c_str()); // "043"
        strcat(gpv, nn);
        vret = PutOmniBUS(COM_RD_PARAMETRI, gpv, m_szAdrsOPA);
        if(vret == OPA_SERIAL_OK) //OK
        {
            strncpy(buff, (char *)&m_BufRX[7], 24);
            buff[24] = 0;
        }
    }

    return vret;
}

/*
******************************************************************************
******************************************************************************
**                                                                          **
**         FUNZIONI PER LA COMUNICAZIONE CON L'OROLOGIO SCU-900             **
**                                                                          **
******************************************************************************
******************************************************************************
*/
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_RdSCUDaTeTime(char *OraData)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->scu_RdOraData(OraData);
    }

    return OPA_SERIAL_VOID;
};
#endif

int __fastcall TOpaSeriale::scu_RdOraData(char *OraData)
{
    char          gpv[32];
    int           vret;

    strcpy(gpv, COM_RD_DATA_ORA.c_str()); // "020"
    strcat(gpv, "0");

    vret = PutOmniBUS(COM_RD_DATA_ORA, gpv, m_szAdrsSCU);

    if (vret == OPA_SERIAL_OK) // OK
    {
        strncpy(OraData, (char *)&m_BufRX[7], 15);
        OraData[15] = 0;
    }
    else          // Comando non abilitato
        strcpy(OraData, "12:0001.01.2001");

    return vret;
}

//------------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_WrSCUDaTeTime(const char *OraData)
{
    if (opaSeriale != NULL)
    {
        return opaSeriale->scu_WrOraData(OraData);
    }

    return OPA_SERIAL_VOID;

};
#endif

int __fastcall TOpaSeriale::scu_WrOraData(const char *OraData)
{
    char          gpv[32];
    int           vret;

    strcpy(gpv, COM_WR_DATA_ORA.c_str()); // "021"
    strcat(gpv, "0");
    strcat(gpv, OraData);

    vret = PutOmniBUS(COM_WR_DATA_ORA, gpv, m_szAdrsSCU);

    return vret;
}

/*
******************************************************************************
******************************************************************************
**                                                                          **
**    FUNZIONI DI SUPPORTO ALLA COMUNCAZIONE CON L'ANALIZZATORE OPA-100     **
**                                                                          **
******************************************************************************
******************************************************************************
*/
int __fastcall TOpaSeriale::PutOmniBUS(AnsiString cmd, const char *gpv, const char *prot)
{
    int             vret;

    vret = OPA_SERIAL_VOID; // DEFAULT -> RX non pervenuta (AGS-200 scollegato)

    for (int ii = 0; ii < m_iTrial; ++ii)
    {
        m_bOk               = false;
        m_bCOMTimeout       = false;

        PutCommand(gpv, prot);

        //m_tmrCOM->Enabled   = true;
        unsigned long T0 = GetTickCount();
        while((GetTickCount()-T0) <= COM_TIMEOUT)
        {
            SleepEx(1, false);
            Application->ProcessMessages();

/*
            if (m_Link != elnkBLUETOOTH)
                ReadReceived();
*/
            if (m_bOk)
                break;
        }
        //m_tmrCOM->Enabled   = false;

        if (m_bOk)  // Frame omniBUS arrivato
        {
            vret = ChkAnswer(cmd, prot); // funzione non bloccante
            if (vret == OPA_SERIAL_OK || vret == OPA_SERIAL_NAK) // OK - NAK
                break;
            else if (vret == OPA_SERIAL_BUSY)           // BUSY
            {
                SleepEx(1000, false);
                Application->ProcessMessages();
            }
            // else RX con FRAME errato   -> ripeti fino a NUM_TRIES prove
        }
        // else TIMEOUT in RX             -> ripeti fino a NUM_TRIES prove
    }

    if (vret==OPA_SERIAL_VOID && !strcmp(prot, m_szAdrsOPA)) // se AGS scollegato (non vale per SCU-900)
        Demo = 1;

    return vret;
}

//------------------------------------------------------------------------------
void __fastcall TOpaSeriale::PutCommand(AnsiString cmd, const char *prot)
{
    m_iRxEnd = 0;
    m_iCount = 0;

    // Azzeramento buffer di ricezione
    for (int ii = 32; ii >= 0; --ii)
    {
        m_BufRX[ii] = '\0';
    }

    m_BufTX[0] = ENQ;
    strcpy((char *)&m_BufTX[1], prot);
    m_BufTX[3] = STX;
    strcpy((char *)&m_BufTX[4], cmd.c_str());
    int ii = strlen((char *)m_BufTX);
    m_BufTX[ii] = ETX;
    CalcBCC(m_BufTX, ii+1);
    m_BufTX[ii+3]= EOT;

    SleepEx(20, false);  // Risolve blocco seriale tas RS-232 con adattori USB e alimentatori PSI-50

    PutCom(m_BufTX, ii+4);

    // Scrittura LOG
    if (Log == true)
    {
        WriteLogTX(m_BufTX, ii + 4);
    }
}

//------------------------------------------------------------------------------
int __fastcall TOpaSeriale::ChkAnswer(AnsiString cmd, const char *prot)
{
    char            gpv[32];
    int             vret = OPA_SERIAL_VOID;  // -> RX non pervenuta

    if (Demo)
    {
        return OPA_SERIAL_OK;
    }
    else
    {
        // controllo indirizzo apparecchiatura che ha risposto
        gpv[0] = m_BufRX[1];
        gpv[1] = m_BufRX[2];
        gpv[2] = 0;

        if (! strcmp(gpv, prot))  // RX da ADDRESS interrogato
        {
            if (m_BufRX[0] == ACK)
            {
                if (ChkBCCOmniBUS(m_BufRX, m_iRxEnd))  // CHKSUM CORRETTO
                {
                    // cmd +1 -> ANSISTRING!!!!!
                    if (m_BufRX[4] == cmd[1] &&  // RX a COMANDO CORRETTO
                        m_BufRX[5] == cmd[2] &&
                        m_BufRX[6] == cmd[3])
                    {
                        if (m_BufRX[7] == BUSY)
                            vret = OPA_SERIAL_BUSY; // OPA-100 in BUSY
                        else
                            vret = OPA_SERIAL_OK;   // risposta corretta
                    }
                    else                      // RX a COMANDO DIVERSO
                        vret = OPA_SERIAL_FRAME;    // risposta non corretta
                }
                else                                  // CHKSUM NON CORRETTO
                    vret = OPA_SERIAL_FRAME;        // risposta non corretta
            }
            else if (m_BufRX[0] == NAK)
            {
                if (ChkBCCOmniBUS(m_BufRX, m_iRxEnd)) // CHKSUM CORRETTO
                    vret = OPA_SERIAL_NAK;   // risposta NAK
                else                                 // CHKSUM NON CORRETTO
                    vret = OPA_SERIAL_FRAME; // risposta non corretta
            }
            else                 // Trama non omniBUS
                vret = OPA_SERIAL_FRAME;     // risposta non corretta
        }
        else if (gpv[0])            // RX da ADDRESS DIVERSO
            vret = OPA_SERIAL_FRAME;         // risposta non corretta
    }

    if (Log == true)    // Log trama RX
    {
        if (vret == OPA_SERIAL_OK)  // Frame MCTCNET arrivato
        {
            WriteLogRX(m_iCount);
        }
    }

    return vret;
}

//------------------------------------------------------------------------------
void __fastcall TOpaSeriale::CalcBCC(BYTE *pFrame, int iLen)
{
    BYTE            sum = 0x00;
    DWORD           a;

    if (iLen < 3)
        return; // Rx Non Corretto

    // Calcolo checksum
    for (int ii = iLen; ii != 0; --ii)
        sum += *pFrame++;

    // Conversione in ASCII cifra più significativa della somma
    a = (sum & 0xF0) >> 4;
    if (a > 9)      a = (a-9) | 0x40;
    else            a |= 0x30;
    *pFrame++   = (BYTE)a;

    // Conversione in ASCII cifra meno significativa della somma
    a = sum & 0x0F;
    if (a > 9)      a = (a-9) | 0x40;
    else            a |= 0x30;
    *pFrame     = (BYTE)a;
}
//------------------------------------------------------------------------------
bool __fastcall TOpaSeriale::ChkBCCOmniBUS(const BYTE *pFrame, int iLen)
{
    BYTE            sum = 0x00;
    DWORD           a, b;

    if (iLen < 3)
        return false;   // Rx Non Corretto

    // Calcolo checksum
    for (int ii = (iLen-2); ii != 0; --ii)
        sum += *pFrame++;

    // Conversione in ASCII cifra più significativa della somma
    a = (sum & 0xF0) >> 4;
    if (a > 9)  a = (a-9) | 0x40;
    else        a |= 0x30;

    // Conversione in ASCII cifra meno significativa della somma
    b = sum & 0x0F;
    if (b > 9)  b = (b-9) | 0x40;
    else        b |= 0x30;

    if (*pFrame == (BYTE)a && *(pFrame+1) == (BYTE)b)
        return true;    // Rx Corretto

    return false;       // Rx Non Corretto
}
//---------------------------------------------------------------------------
BYTE __fastcall TOpaSeriale::HexASCII2Bin(const BYTE *pFrame)
{
    DWORD           a, b;

    if (pFrame == NULL)
        return 0x00;

    if (pFrame[0] > '@') a = (pFrame[0] & 0x0F)+9;
    else                 a =  pFrame[0] & 0x0F;
    if (pFrame[1] > '@') b = (pFrame[1] & 0x0F)+9;
    else                 b =  pFrame[1] & 0x0F;

    return ((BYTE)((a << 4) | b));
}
//---------------------------------------------------------------------------
void __fastcall TOpaSeriale::Bin2HexASCII(BYTE num, char *pszASCII)
{
    DWORD           a, b;

    // conversione in ASCII cifra più significativa della somma
    a = (num & 0xF0) >> 4;
    if (a > 9)  a = (a-9) | 0x40;
    else        a |= 0x30;
    // conversione in ASCII cifra meno significativa della somma
    b = num & 0x0F;
    if (b > 9)  b = (b-9) | 0x40;
    else        b |= 0x30;

    pszASCII[0] = (BYTE)a;
    pszASCII[1] = (BYTE)b;
}
//---------------------------------------------------------------------------
int __fastcall TOpaSeriale::Get_Demo(void)
{
    return Demo;
}
//---------------------------------------------------------------------------
int __fastcall TOpaSeriale::CalcoloTrigger(double MediaMisure)
{
    double  trigger;
    int     valore;

    trigger = MediaMisure + MediaMisure/4.0 + 2.0;
    if (trigger < 99.9)
        valore = static_cast<int>(trigger);
    else
        valore = 99;

    return valore;
}
//---------------------------------------------------------------------------
void __fastcall TOpaSeriale::DemoValues(OPA_SMOKEVALUE_S *fumVal)
{
    int             rpm, rrpm;
    double          opa_k, ropa_k;

    opa_k = 1.10;
    rpm = 1870;

    ropa_k = (double)(rand()%100)/100.0;
    rrpm = rand()%100;

    fumVal->dOpacity_k  = opa_k + ropa_k;
    fumVal->dOpacity_p  = 100.0*(1.0-(exp(-fumVal->dOpacity_k*0.43)));
    fumVal->iRpm        = rpm+rrpm;

    sprintf(fumVal->szOpacity_k,"%04.2f", fumVal->dOpacity_k);
    sprintf(fumVal->szRpm,"%4d", fumVal->iRpm);
}

//---------------------------------------------------------------------------
#ifdef DLL_EXPORTS
DLL_API int __stdcall OPA_GetDemo(void)
{
    return Demo;
};
#endif

//---------------------------------------------------------------------------
/* CITT CRC16 polynomial ^16 + ^12 + ^5 + 1 */
unsigned short __fastcall TOpaSeriale::crc16_add(unsigned char b, unsigned short acc)
{
  acc ^= b;
  acc  = (unsigned short)((acc >> 8) | (acc << 8));
  acc ^= (unsigned short)((acc & 0xff00) << 4);
  acc ^= (unsigned short)((acc >> 8) >> 4);
  acc ^= (unsigned short)((acc & 0xff00) >> 5);

  return acc;
}
//---------------------------------------------------------------------------
unsigned short __fastcall TOpaSeriale::Checksum(AnsiString FileName)
{
    int             iFileHandle;
    int             iFileLength;
    int             iBytesRead;
    char           *pszBuffer;
    unsigned short  crc16 = 0x000;

    try
    {
        if (FileExists(FileName))
        {
            iFileHandle = FileOpen(FileName, fmShareDenyWrite);
            iFileLength = FileSeek(iFileHandle,0,2);
            FileSeek(iFileHandle,0,0);
            pszBuffer = new char[iFileLength+1];
            iBytesRead = FileRead(iFileHandle, pszBuffer, iFileLength);
            FileClose(iFileHandle);

            crc16 = 0x0000;
            for(int ii = 0; ii < iBytesRead; ii++)
                crc16 = crc16_add(pszBuffer[ii], crc16);

            delete [] pszBuffer;
        }
    }
    catch (...)
    {
        return 0x0000;
    }

    return crc16;
}

//---------------------------------------------------------------------------
// out operc 43                                  in operc 20
double __fastcall TOpaSeriale::CalculateOpaP43(double dOpaLm, double dLengthOPA)
{
    double dOpaLs = 100.0 * (1.0 - std::pow((1.0 - dOpaLm / 100.0), (0.43 / dLengthOPA)));
    if (dOpaLs <= 0.0)
        dOpaLs = 0.0;
    else if (dOpaLs > 99.9)
        dOpaLs = 99.9;

    return  dOpaLs;
    // sprintf(FumVal.vopacita_p, "%.1f", dOpaLs);
    // FumVal.opacita_p = atof(FumVal.vopacita_p);
}

//---------------------------------------------------------------------------
double __fastcall TOpaSeriale::CalculateOpaK(double dOpaPerc)
{
    double          k;

    //    lunghezza camera in [m]
    k = (-1.0 / 0.20) * log_o(1.0 - dOpaPerc / 100.0);
    if (k < 0.0)  k = 0.0001;
#ifdef IDE_ENABLEIPERCONNESSIONE
    //
#else
    if (k > 9.98) k = 9.99;
#endif

    return k;
}

//---------------------------------------------------------------------------
double __fastcall TOpaSeriale::CalculateOpaPerc(double dOpaK)
{
    double          p;

    //                      lunghezza camera in [m]
    p = 100.0 * (1.0 - exp_o(-dOpaK * 0.20));
    if (p < 0.0)  p = 0.0001;
    if (p > 99.8) p = 99.9;

    return p;          //(00.0-99.9)
}

/**
*=============================================================================
* \brief Calcolo opacità percentuale
*
*=============================================================================
*/
double __fastcall TOpaSeriale::CalculateOpaPercCamera(double dOpaK, double dLengthOPA)
{
    double opacita_p = 100.0 * (1.0 - exp_o(-dOpaK * dLengthOPA));

    if (opacita_p > 99.8)
        opacita_p = 99.9;
    else if (opacita_p < 0.0)
        opacita_p = 0.0001;

    return opacita_p;
}

//---------------------------------------------------------------------------
void __fastcall TOpaSeriale::WriteLogTX(const BYTE *pFrame, int iLen)
{
    // COSTRUZIONE FRAME DI TRASMISSIONE
    AnsiString aHex = "";
    for (int ii = iLen; ii != 0; --ii)
    {
        if ( (unsigned)(*pFrame - ' ') < ('~' - ' '))               // Carattere stampabile
        {
            aHex = aHex + AnsiString((char)(*pFrame));
        }
        else                                                        // Carattere NON stampabile
        {
            aHex = aHex + AnsiString(".");
        }

        ++pFrame;
    }


    // WRITE LOG
    AnsiString aPrefix = "TX [OK] " + FormatDateTime("hh:nn:ss.zzz", Now()) + " -> ";
    m_logOPA->Add(aPrefix + aHex);
    //  OutputDebugString(AnsiString(aPrefix + aHex).c_str());
}

//---------------------------------------------------------------------------
void __fastcall TOpaSeriale::WriteLogRX(int iLen)
{
    // COSTRUZIONE FRAME DI TRASMISSIONE
    AnsiString aHex = "";

    BYTE *pFrame = m_BufRX;

    for (int ii = iLen; ii != 0; --ii)
    {
        if ( (unsigned int)(*pFrame - ' ') < ('~' - ' '))   // Carattere stampabile
        {
            aHex = aHex + AnsiString((char)(*pFrame));
        }
        else                                                // Carattere NON stampabile
        {
            aHex = aHex + AnsiString(".");
        }

        ++pFrame;
    }

    // WRITE LOG
    AnsiString aPrefix = "RX [OK] " + FormatDateTime("hh:nn:ss.zzz", Now()) + " -> ";
    m_logOPA->Add(aPrefix + aHex);
    //  OutputDebugString(AnsiString(aPrefix + aHex).c_str());
}

