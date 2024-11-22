using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using System.Diagnostics.Eventing.Reader;

namespace TestOpaDLL
{
    // BT100COM.DLL

    enum BT_SERIAL_E
    {
        BT_SERIAL_OK = 0,
        BT_SERIAL_ERROR = 1
    }

    public partial class FormMain : Form
    {
        /*
        #pragma pack(push, 8)  
                struct Location
                {
                    int x;
                    int y;
                };
        #pragma pack(pop) 

        [StructLayout(LayoutKind::Sequential, Pack=8)]  
        value struct MLocation {  
           int x;  
           int y;  
        };   
      */
        // BT100COM.DLL

        // DLL_API int __stdcall   BT100_SetMacSlaveOnBT100Master(int iSeriale, const char* pszNewMAcAddress);    //tuttounito
        [DllImport(@"BT100Com.dll", CharSet = CharSet.Ansi)]
        public static extern Int32 BT100_SetMacSlaveOnBT100Master(int iSeriale, StringBuilder pszNewMAcAddress);

        // AGSCOM.DLL

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct AGS_STATUS_S
        {
            public byte byte_status1;
            public byte byte_status2;
            public byte byte_error1;
            public byte byte_error2;

            public byte warming_up;
            public byte zero_perf;
            public byte calibration_perf;
            public byte new_o2sensor_perf;
            public byte solenoid_calibration;
            public byte pump_active;
            public byte fans_active;
            public byte solenoid_zero;

            public byte zero_request;
            public byte hctest_perf;
            public byte leaktest_perf;
            public byte o2sensor_discharging;
            public byte negatives_gasvalues;
            public byte leaktest_critical;
            public byte internal_scu_presence;
            public byte calibration_suspended;

            public byte error_eeprom_notworking;
            public byte error_eeprom_corrupt;
            public byte warning_highpressure;
            public byte warning_lowflow;
            public byte error_cylinder_values;
            public byte error_high_temperature;
            public byte error_lowpower;
            public byte error_highpower;

            public byte error_zero;
            public byte error_calibration_gas_o2;
            public byte error_hctest;
            public byte error_leaktest;
            public byte new_o2sensor_needed;
            public byte new_noxsensor_needed;
            public byte error_gasvalues;
            public byte error_critical;
        }
        
        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct AGS_GASVALUE_S
        {
            public Int32 iRPM;
            public double dLAMBDA;
            public double dCO;
            public double dCOcor;
            public double dCO2;
            public Int32 iHC;
            public double dO2;
            public Int32 iNOx;
        };

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct AGS_ANALOGVALUE_S
        {
            public Int32 iOilTemp;        //  C
            public double dAmbientTemp;    //  C
            public Int32 iPressure;       // mbar
            public double dPowerBattery;   // mV
        };


        // DLL_API bool __stdcall AGS_LoadDLL(int iSeriale);
        [DllImport(@"AgsCom.dll", CharSet = CharSet.Ansi)]
        public static extern bool AGS_LoadDLL(int iSeriale);
        // DLL_API void __stdcall AGS_UnloadDLL(void);
        [DllImport(@"AgsCom.dll", CharSet = CharSet.Ansi)]
        public static extern void AGS_UnloadDLL();
        // DLL_API bool __stdcall AGS_Open(void);
        [DllImport(@"AgsCom.dll", CharSet = CharSet.Ansi)]
        public static extern bool AGS_Open();
        // DLL_API void __stdcall AGS_Close(void);
        [DllImport(@"AgsCom.dll", CharSet = CharSet.Ansi)]
        public static extern void AGS_Close();
        // DLL_API int  __stdcall AGS_Version(char *pszVerAGS, char *pszChkAGS, char *pszVerBench, char *pszNrBench);
        [DllImport("AgsCom.dll", CharSet = CharSet.Ansi)]
        public static extern Int32 AGS_Version(StringBuilder VerAGS, StringBuilder ChkAGS, StringBuilder VerBench, StringBuilder NrBench);
        /*        DLL_API int __stdcall AGS_ReadVersionMID( // IDENT MID certidficato
            char* veragsMID,
            char* chkagsMID,
            char* verexeMID,
            char* chkexeMID,
            char* verdllMID,
            char* chkdllMID);
        */
        [DllImport("AgsCom.dll", CharSet = CharSet.Ansi)]
        public static extern Int32 AGS_ReadVersionMID(
                                StringBuilder verAgsMID, 
                                StringBuilder chkAgsMID, 
                                StringBuilder verExeMID, 
                                StringBuilder chkExeMID,
                                StringBuilder verDllMID,
                                StringBuilder chkDllMID
        );
        // DLL_API int __stdcall AGS_Values(AGS_GASVALUE_S* agsVal);
        [DllImport(@"AgsCom.dll", CallingConvention = CallingConvention.StdCall, CharSet = CharSet.Ansi)]
        public static extern Int32 AGS_Values(ref AGS_GASVALUE_S agsVal);
        // DLL_API int __stdcall AGS_AnalogValues(AGS_ANALOGVALUE_S* agsAux);
        [DllImport(@"AgsCom.dll", CallingConvention = CallingConvention.StdCall, CharSet = CharSet.Ansi)]
        public static extern Int32 AGS_AnalogValues(ref AGS_ANALOGVALUE_S agsAux);
        // DLL_API int __stdcall AGS_Status(AGS_STATUS_S* agsStatus);
        [DllImport(@"AgsCom.dll", CallingConvention = CallingConvention.StdCall, CharSet = CharSet.Ansi)]
        public static extern Int32 AGS_Status(ref AGS_STATUS_S agsStatus);
        // DLL_API int __stdcall AGS_RdSCU(char* pszDateTime);
        [DllImport(@"AgsCom.dll", CallingConvention = CallingConvention.StdCall, CharSet = CharSet.Ansi)]
        public static extern Int32 AGS_RdSCU(StringBuilder pszDateTime);

        // OPACOM.DLL
        
        [StructLayout(LayoutKind.Sequential, Pack = 8)]
        public struct OPA_SMOKEVALUE_S
        {
            public Int32 iRpm; // valeur RPM
            public Int32 iReserved1; // valeur RPM

            public double dOpacity_k;
            public double dOpacity_p;

            [MarshalAs(UnmanagedType.ByValArray, ArraySubType = UnmanagedType.U1, SizeConst = 8)]
            public byte[] szRpm;
            [MarshalAs(UnmanagedType.ByValArray, ArraySubType = UnmanagedType.U1, SizeConst = 8)]
            public byte[] szOpacity_k;
            [MarshalAs(UnmanagedType.ByValArray, ArraySubType = UnmanagedType.U1, SizeConst = 8)]
            public byte[] szOpacity_P;

            [MarshalAs(UnmanagedType.ByValArray, ArraySubType = UnmanagedType.U1, SizeConst = 12)]
            public double[] dValue_k;
            [MarshalAs(UnmanagedType.ByValArray, ArraySubType = UnmanagedType.U1, SizeConst = 12)]
            public double[] dValue_p;
            
            public Int32 iValues;
            public Int32 iReserved2; // valeur RPM

            [MarshalAs(UnmanagedType.ByValArray, ArraySubType = UnmanagedType.U1, SizeConst = 12)]
            public double[] dPressure;
            [MarshalAs(UnmanagedType.ByValArray, ArraySubType = UnmanagedType.U1, SizeConst = 12)]
            public double[] dOpacityPreFilter;
            [MarshalAs(UnmanagedType.ByValArray, ArraySubType = UnmanagedType.U1, SizeConst = 12)]
            public Int32[] igRpm;
        }

        [StructLayout(LayoutKind.Sequential, Pack = 8)]
        public struct OPA_ANALOGVALUE_S
        {
            [MarshalAs(UnmanagedType.ByValArray, ArraySubType = UnmanagedType.I1, SizeConst = 8)]
            public byte[] szOilTemp;
            [MarshalAs(UnmanagedType.ByValArray, ArraySubType = UnmanagedType.I1, SizeConst = 8)]
            public byte[] szSmokeTemp;
            [MarshalAs(UnmanagedType.ByValArray, ArraySubType = UnmanagedType.I1, SizeConst = 8)]
            public byte[] szChamberTemp;
            [MarshalAs(UnmanagedType.ByValArray, ArraySubType = UnmanagedType.I1, SizeConst = 8)]
            public byte[] szAmbientTemp;
            [MarshalAs(UnmanagedType.ByValArray, ArraySubType = UnmanagedType.I1, SizeConst = 8)]
            public byte[] szChamberPressure;
            [MarshalAs(UnmanagedType.ByValArray, ArraySubType = UnmanagedType.I1, SizeConst = 8)]
            public byte[] szPowerBattery;
        };


        [DllImport(@"OpaCom.dll", CharSet = CharSet.Ansi)]
        public static extern bool OPA_LoadDLL(int iSeriale);
        [DllImport(@"OpaCom.dll", CharSet = CharSet.Ansi)]
        public static extern bool OPA_Open();
        [DllImport(@"OpaCom.dll", CharSet = CharSet.Ansi)]
        public static extern void OPA_Close();
        [DllImport(@"OpaCom.dll", CharSet = CharSet.Ansi)]
        public static extern void OPA_UnloadDLL();
        [DllImport("OpaCom.dll", CharSet = CharSet.Ansi)]
        public static extern void OPA_EnableLog(bool bEnable);
        [DllImport("OpaCom.dll", CharSet = CharSet.Ansi)]
        public static extern Int32 OPA_Version(StringBuilder veropa, StringBuilder checkopa);
        [DllImport("OpaCom.dll", CharSet = CharSet.Ansi)]
        public static extern Int32 OPA_Revision(StringBuilder glb_szOpaRevision, StringBuilder szOpaZone, StringBuilder szOpaDateRevision);
        [DllImport("OpaCom.dll", CharSet = CharSet.Ansi)]
        public static extern Int32 OPA_RdIdentification(StringBuilder szOpaMake, StringBuilder szOpaModel, StringBuilder szOpaOmolNr);
        [DllImport("OpaCom.dll", CharSet = CharSet.Ansi)]
        public static extern Int32 OPA_RdSerialNr(StringBuilder szOpaSerialNr);
        [DllImport("OpaCom.dll", CharSet = CharSet.Ansi)]
        public static extern Int32 OPA_RdCalibrationDate(StringBuilder szOpaCalibrationDate);
        [DllImport(@"OpaCom.dll", CharSet = CharSet.Ansi)]
        public static extern Int32 OPA_VersionDLL(StringBuilder szVerDLLOpa, StringBuilder szChkDLLOpa);
        [DllImport("OpaCom.dll", CharSet = CharSet.Ansi)]
        public static extern Int32 OPA_RevisionDLL(StringBuilder szOpaRevisionDLL, StringBuilder szOpaDateRevisionDLL);
        [DllImport(@"OpaCom.dll", CallingConvention = CallingConvention.StdCall, CharSet = CharSet.Ansi)]
        public static extern Int32 OPA_AnalogValues(ref OPA_ANALOGVALUE_S opaAux);
        
        // DLL_API int __stdcall OPA_Values(OPA_SMOKEVALUE_S* opaVal);
        [DllImport(@"OpaCom.dll", CallingConvention = CallingConvention.StdCall, CharSet = CharSet.Ansi)]
        public static extern Int32 OPA_Values(ref OPA_SMOKEVALUE_S opaVal);
        
        // DLL_API int __stdcall OPA_RdChamberLength(char* lunghezza);
        [DllImport(@"OpaCom.dll", CallingConvention = CallingConvention.StdCall, CharSet = CharSet.Ansi)]
        public static extern Int32 OPA_RdChamberLength(StringBuilder szChamberLength);

        public FormMain()
        {
            InitializeComponent();
        }

        private void BtnOpenPort_Click(object sender, EventArgs e)
        {
            if (!OPA_LoadDLL(18))
            {
                MessageBox.Show("DLL OpaCom.dll failure!!!");
                return;
            }

            if (! OPA_Open())
            {
                MessageBox.Show("Open serial port failure!!!");
                return;
            }

            bool mio = true;
            //Boolean tuo = true;
            OPA_EnableLog(mio);

            // LETTURA VERSIONE DLL
            StringBuilder pszVersion = new StringBuilder(32);
            StringBuilder pszChksum = new StringBuilder(32);
            OPA_VersionDLL(pszVersion, pszChksum);
            label7.Text = pszVersion.ToString();
            label8.Text = pszChksum.ToString();

            // LETTURA VERSIONE FIRMWARE OPACIMETRO
            StringBuilder szVerOpa = new StringBuilder(32);
            StringBuilder szChkOpa = new StringBuilder(32);
            OPA_Version(szVerOpa, szChkOpa);

            // LETTURA REVISIONE FIRMWARE OPACIMETRO
            StringBuilder glb_szOpaRevision = new StringBuilder(32);
            StringBuilder szOpaZone = new StringBuilder(32);
            StringBuilder szOpaDateRevision = new StringBuilder(32);
            OPA_Revision(glb_szOpaRevision, szOpaZone, szOpaDateRevision);

            // LETTURA IDENTIFICAZIONE OPACIMETRO
            StringBuilder szOpaMake = new StringBuilder(32);
            StringBuilder szOpaModel = new StringBuilder(32);
            StringBuilder szOpaOmolNr = new StringBuilder(32);
            OPA_RdIdentification(szOpaMake, szOpaModel, szOpaOmolNr);

            // LETTURA NUMERO DI SERIE OPACIMETRO
            StringBuilder szOpaSerialNr = new StringBuilder(32);
            OPA_RdSerialNr(szOpaSerialNr);

            // LETTURA DATA ULTIMA CALIBRAZIONE OPACIMETRO
            StringBuilder szOpaCalibrationDate = new StringBuilder(32);
            OPA_RdCalibrationDate(szOpaCalibrationDate);

            // LETTURA VERSIONE DLL E CHKSUM DLL
            StringBuilder szVerDLLOpa = new StringBuilder(32);
            StringBuilder szChkDLLOpa = new StringBuilder(32);
            OPA_VersionDLL(szVerDLLOpa, szChkDLLOpa);

            // LETTURA REVISIONE DLL
            StringBuilder szOpaRevisionDLL = new StringBuilder(32);
            StringBuilder szOpaDateRevisionDLL = new StringBuilder(32);
            OPA_RevisionDLL(szOpaRevisionDLL, szOpaDateRevisionDLL);

            OPA_SMOKEVALUE_S sOpaVal = new OPA_SMOKEVALUE_S();
            // OPA_SMOKEVALUE_S sOpaPeakVal  = newv OPA_SMOKEVALUE_S();

            OPA_Values(ref sOpaVal);

            StringBuilder szChamberLength = new StringBuilder(8);
            OPA_RdChamberLength(szChamberLength);

            // LETTURA VALORI ANALOGICI
            OPA_ANALOGVALUE_S opaAux = new OPA_ANALOGVALUE_S();
            OPA_AnalogValues(ref opaAux);
            label1.Text = System.Text.Encoding.Default.GetString(opaAux.szAmbientTemp);  // Marshal.PtrToStringAnsi(opaAux.szAmbientTemp);
            label2.Text = System.Text.Encoding.Default.GetString(opaAux.szChamberPressure);
            label3.Text = System.Text.Encoding.Default.GetString(opaAux.szChamberTemp);
            label4.Text = System.Text.Encoding.Default.GetString(opaAux.szOilTemp);
            label5.Text = System.Text.Encoding.Default.GetString(opaAux.szPowerBattery);
            label6.Text = System.Text.Encoding.Default.GetString(opaAux.szSmokeTemp);


            lblValues.Text = "Make:" + szOpaMake + " Model:" + szOpaModel + " Omologation:" + szOpaOmolNr + "\n" +
                "Version:" + szVerOpa + " ChkSum:" + szChkOpa + " Revision:" + glb_szOpaRevision + "\n" +
                "Serial Nr.:" + szOpaSerialNr + " Calibration Date:" + szOpaCalibrationDate + "\n" +
                "OpaCom.dll Version:" + szVerDLLOpa + " Chksum:" + szChkDLLOpa + " Revision:" + szOpaRevisionDLL + "\n" +
                "Chamber Length:" + szChamberLength;

            /*
                        string convert = "here is my superscript: \u00B9\u3045\u3145";
                        UTF8Encoding utf8 = new UTF8Encoding();
                        byte[] encodedBytes = utf8.GetBytes(convert);
                        //byte[] buffer = { 0x48, 0x31, 0x9c, 0x31, 0x31, 0x31, 0x31 };
                        string s = System.Text.Encoding.UTF8.GetString(encodedBytes, 0, encodedBytes.Length);
                        label2.Text = s;
            */
        }

        private void BtnClosePort_Click(object sender, EventArgs e)
        {
            OPA_Close();
            OPA_UnloadDLL();
        }

        private void btnGasComPort_Click(object sender, EventArgs e)
        {
            if (!AGS_LoadDLL(15))
            {
                MessageBox.Show("DLL AgsCom.dll failure!!!");
                return;
            }

            if (!AGS_Open())
            {
                MessageBox.Show("Open GAS serial port failure!!!");
                return;
            }
            // LECTURE VERSIONE DLL
            StringBuilder pszVerAGS = new StringBuilder(32);
            StringBuilder pszChkAGS = new StringBuilder(32);
            StringBuilder pszVerBench = new StringBuilder(32);
            StringBuilder pszNrBench = new StringBuilder(32);

            AGS_Version(pszVerAGS, pszChkAGS, pszVerBench, pszNrBench);
            label7.Text = pszVerAGS.ToString();
            label8.Text = pszChkAGS.ToString();
            label9.Text = pszVerBench.ToString();
            label10.Text = pszNrBench.ToString();
            StringBuilder verAgsMID = new StringBuilder(32);
            StringBuilder chkAgsMID = new StringBuilder(32);
            StringBuilder verExeMID = new StringBuilder(32);
            StringBuilder chkExeMID= new StringBuilder(32);
            StringBuilder verDllMID = new StringBuilder(32);
            StringBuilder chkDllMID = new StringBuilder(32);

            AGS_ReadVersionMID(verAgsMID, chkAgsMID, verExeMID, chkExeMID, verDllMID, chkDllMID);
            label1.Text = verAgsMID.ToString();
            label2.Text = chkAgsMID.ToString();
            label3.Text = verExeMID.ToString();
            label4.Text = chkExeMID.ToString();
            label5.Text = verDllMID.ToString();
            label6.Text = chkDllMID.ToString();

            AGS_GASVALUE_S agsVal = new AGS_GASVALUE_S();
            AGS_Values(ref agsVal);

            lblValues.Text = "Rpm:" + agsVal.iRPM.ToString() + " Lambda:" + agsVal.dLAMBDA.ToString() + " CO:" + agsVal.dCO.ToString() + "\n" +
                "CO2:" + agsVal.dCO2.ToString() + " HC:" + agsVal.iHC.ToString() + " O2:" + agsVal.dO2.ToString() + "\n" +
                "NOx:" + agsVal.iNOx.ToString();

            AGS_ANALOGVALUE_S agsAux = new AGS_ANALOGVALUE_S();
            AGS_AnalogValues(ref agsAux);

            lblAUX.Text = "Temperature:" + agsAux.iOilTemp.ToString() + " Ambient Temp.:" + agsAux.dAmbientTemp.ToString() + "\n" +
                "Pressure:" + agsAux.iPressure.ToString() + " PowerBattery:" + agsAux.dPowerBattery.ToString();

            AGS_STATUS_S agsStatus = new AGS_STATUS_S();
            AGS_Status(ref agsStatus);

            StringBuilder pszDateTime = new StringBuilder(16);
            AGS_RdSCU(pszDateTime);
            ldbDateTime.Text = pszDateTime.ToString().Substring(5,10);
        }

        private void btnGASClosePort_Click(object sender, EventArgs e)
        {
            AGS_Close();
            AGS_UnloadDLL();

        }

        private void button1_Click(object sender, EventArgs e)
        {
            const int BT100_SERIALPORT = 11;
            const string GASANALYZER_MACADDRESS = "008098F76E78";
            const string SMOKEMETER_MACADDRESS = "000BCE002DC4";

            btnSwitch.Enabled = false;

            // SWITCH MAC ADRRESS of the CLIENT DEVICE
            StringBuilder pszAGSMACaddress = new StringBuilder(32);
            pszAGSMACaddress.Append(GASANALYZER_MACADDRESS.ToString());
            Int32 iRetValue = BT100_SetMacSlaveOnBT100Master(BT100_SERIALPORT, pszAGSMACaddress);
            if (iRetValue == (int)BT_SERIAL_E.BT_SERIAL_ERROR) {
                MessageBox.Show("Serial port of the module BT100 not available !!!");
                btnSwitch.Enabled = true;
                return;
            }

            if (! AGS_LoadDLL(BT100_SERIALPORT)) {
                MessageBox.Show("AgsCom.dll not present !!!");
                btnSwitch.Enabled = true;
                return;
            }
            if (! AGS_Open()) {
                MessageBox.Show("Gas Analyzer serial port not available !!!");
                btnSwitch.Enabled = true;
                return;
            }
            // LECTURE VERSIONE DLL
            StringBuilder pszVerAGS = new StringBuilder(32);
            StringBuilder pszChkAGS = new StringBuilder(32);
            StringBuilder pszVerBench = new StringBuilder(32);
            StringBuilder pszNrBench = new StringBuilder(32);

            AGS_Version(pszVerAGS, pszChkAGS, pszVerBench, pszNrBench);
            label7.Text = pszVerAGS.ToString();
            label8.Text = pszChkAGS.ToString();
            label9.Text = pszVerBench.ToString();
            label10.Text = pszNrBench.ToString();
            StringBuilder verAgsMID = new StringBuilder(32);
            StringBuilder chkAgsMID = new StringBuilder(32);
            StringBuilder verExeMID = new StringBuilder(32);
            StringBuilder chkExeMID = new StringBuilder(32);
            StringBuilder verDllMID = new StringBuilder(32);
            StringBuilder chkDllMID = new StringBuilder(32);

            AGS_ReadVersionMID(verAgsMID, chkAgsMID, verExeMID, chkExeMID, verDllMID, chkDllMID);
            label1.Text = verAgsMID.ToString();
            label2.Text = chkAgsMID.ToString();
            label3.Text = verExeMID.ToString();
            label4.Text = chkExeMID.ToString();
            label5.Text = verDllMID.ToString();
            label6.Text = chkDllMID.ToString();

            AGS_GASVALUE_S agsVal = new AGS_GASVALUE_S();
            AGS_Values(ref agsVal);

            lblValues.Text = "Rpm:" + agsVal.iRPM.ToString() + " Lambda:" + agsVal.dLAMBDA.ToString() + " CO:" + agsVal.dCO.ToString() + "\n" +
                "CO2:" + agsVal.dCO2.ToString() + " HC:" + agsVal.iHC.ToString() + " O2:" + agsVal.dO2.ToString() + "\n" +
                "NOx:" + agsVal.iNOx.ToString();

            AGS_ANALOGVALUE_S agsAux = new AGS_ANALOGVALUE_S();
            AGS_AnalogValues(ref agsAux);

            lblAUX.Text = "Temperature:" + agsAux.iOilTemp.ToString() + " Ambient Temp.:" + agsAux.dAmbientTemp.ToString() + "\n" +
                "Pressure:" + agsAux.iPressure.ToString() + " PowerBattery:" + agsAux.dPowerBattery.ToString();

            AGS_STATUS_S agsStatus = new AGS_STATUS_S();
            AGS_Status(ref agsStatus);

            StringBuilder pszDateTime = new StringBuilder(16);
            AGS_RdSCU(pszDateTime);
            ldbDateTime.Text = pszDateTime.ToString().Substring(5, 10);

            AGS_Close();
            AGS_UnloadDLL();

            // SWITCH MAC ADRRESS of the CLIENT DEVICE
            StringBuilder pszOPAMACaddress = new StringBuilder(32);
            pszOPAMACaddress.Append(SMOKEMETER_MACADDRESS.ToString());
            iRetValue = BT100_SetMacSlaveOnBT100Master(BT100_SERIALPORT, pszOPAMACaddress);
            if (iRetValue == (int)BT_SERIAL_E.BT_SERIAL_ERROR) {
                MessageBox.Show("Serial port of the module BT100 not available !!!");
                btnSwitch.Enabled = true;
                return;
            }

            if (!OPA_LoadDLL(BT100_SERIALPORT)) {
                MessageBox.Show("OpaCom.dll not present !!!");
                btnSwitch.Enabled = true;
                return;
            }
            if (!OPA_Open()) {
                MessageBox.Show("Smokemeter serial port not available !!!");
                btnSwitch.Enabled = true;
                return;
            }

            OPA_EnableLog(true);

            // LETTURA VERSIONE DLL
            StringBuilder pszVersion = new StringBuilder(32);
            StringBuilder pszChksum = new StringBuilder(32);
            OPA_VersionDLL(pszVersion, pszChksum);
            label7.Text = pszVersion.ToString();
            label8.Text = pszChksum.ToString();

            // LETTURA VERSIONE FIRMWARE OPACIMETRO
            StringBuilder szVerOpa = new StringBuilder(32);
            StringBuilder szChkOpa = new StringBuilder(32);
            OPA_Version(szVerOpa, szChkOpa);

            // LETTURA REVISIONE FIRMWARE OPACIMETRO
            StringBuilder glb_szOpaRevision = new StringBuilder(32);
            StringBuilder szOpaZone = new StringBuilder(32);
            StringBuilder szOpaDateRevision = new StringBuilder(32);
            OPA_Revision(glb_szOpaRevision, szOpaZone, szOpaDateRevision);

            // LETTURA IDENTIFICAZIONE OPACIMETRO
            StringBuilder szOpaMake = new StringBuilder(32);
            StringBuilder szOpaModel = new StringBuilder(32);
            StringBuilder szOpaOmolNr = new StringBuilder(32);
            OPA_RdIdentification(szOpaMake, szOpaModel, szOpaOmolNr);

            // LETTURA NUMERO DI SERIE OPACIMETRO
            StringBuilder szOpaSerialNr = new StringBuilder(32);
            OPA_RdSerialNr(szOpaSerialNr);

            // LETTURA DATA ULTIMA CALIBRAZIONE OPACIMETRO
            StringBuilder szOpaCalibrationDate = new StringBuilder(32);
            OPA_RdCalibrationDate(szOpaCalibrationDate);

            // LETTURA VERSIONE DLL E CHKSUM DLL
            StringBuilder szVerDLLOpa = new StringBuilder(32);
            StringBuilder szChkDLLOpa = new StringBuilder(32);
            OPA_VersionDLL(szVerDLLOpa, szChkDLLOpa);

            // LETTURA REVISIONE DLL
            StringBuilder szOpaRevisionDLL = new StringBuilder(32);
            StringBuilder szOpaDateRevisionDLL = new StringBuilder(32);
            OPA_RevisionDLL(szOpaRevisionDLL, szOpaDateRevisionDLL);

            // LETTURA VALORI ANALOGICI
            OPA_ANALOGVALUE_S opaAux = new OPA_ANALOGVALUE_S();
            OPA_AnalogValues(ref opaAux);
            label1.Text = System.Text.Encoding.Default.GetString(opaAux.szAmbientTemp);  // Marshal.PtrToStringAnsi(opaAux.szAmbientTemp);
            label2.Text = System.Text.Encoding.Default.GetString(opaAux.szChamberPressure);
            label3.Text = System.Text.Encoding.Default.GetString(opaAux.szChamberTemp);
            label4.Text = System.Text.Encoding.Default.GetString(opaAux.szOilTemp);
            label5.Text = System.Text.Encoding.Default.GetString(opaAux.szPowerBattery);
            label6.Text = System.Text.Encoding.Default.GetString(opaAux.szSmokeTemp);

            lblValues.Text = "Make:" + szOpaMake + " Model:" + szOpaModel + " Omologation:" + szOpaOmolNr + "\n" +
                "Version:" + szVerOpa + " ChkSum:" + szChkOpa + " Revision:" + glb_szOpaRevision + "\n" +
                "Serial Nr.:" + szOpaSerialNr + " Calibration Date:" + szOpaCalibrationDate + "\n" +
                "OpaCom.dll Version:" + szVerDLLOpa + " Chksum:" + szChkDLLOpa + " Revision:" + szOpaRevisionDLL;

            OPA_Close();
            OPA_UnloadDLL();

            btnSwitch.Enabled = true;
        }
    }
}
