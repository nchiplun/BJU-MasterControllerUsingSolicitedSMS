/*
 * File name            : variableDefinitions.h
 * Compiler             : MPLAB XC8/ MPLAB C18 compiler
 * IDE                  : Microchip  MPLAB X IDE v5.25
 * Processor            : PIC18F66K40
 * Author               : Bhoomi Jalasandharan
 * Created on           : July 15, 2020, 05:23 PM
 * Description          : Definitions of variable
 */

#ifndef Variable_DEFINITIONS_H // Variable definitions
#define	Variable_DEFINITIONS_H

/*
#ifndef DEBUG_MODE_ON_H // Debug Mode On
#define	DEBUG_MODE_ON_H
#endif
*/

#ifndef Encryption_ON_H // Data Encryption On
#define	Encryption_ON_H
#endif

/*
#ifndef SMS_DELIVERY_REPORT_ON_H // SMS_DELIVERY_REPORT On
#define	SMS_DELIVERY_REPORT_ON_H
#endif
*/

/*
#ifndef DOL_DEFINITIONS_H // for direct online motor start
#define DOL_DEFINITIONS_H
#endif
*/

//*
#ifndef STAR_DELTA_DEFINITIONS_H // for star delta motor start
#define	STAR_DELTA_DEFINITIONS_H
#endif
//*/

// include processor files - each processor file is guarded.  
#include "congfigBits.h"

#define Major "0"
#define Minor "7"

#define _XTAL_FREQ 64000000
/** Macro definition for library functions that triggers "suspicious pointer conversion" warning#start **/
#define strncpy(a,b,c)     strncpy((char*)(a),(char*)(b),(c)) 
#define strncmp(a,b,c)     strncmp((char*)(a),(char*)(b),(c))
#define strlwr(a)      strlwr((char*)(a))
/*** Macro definition for library functions that triggers "suspicious pointer conversion" warning#end ***/

/***************************** Port line definition#start ****************************/
#define field1ValveControl PORTAbits.RA0            // To control field1 valve
#define field2ValveControl PORTAbits.RA1            // To control field2 valve
#define field3ValveControl PORTAbits.RA2            // To control field3 valve
#define field4ValveControl PORTAbits.RA3            // To control field4 valve
#define field5ValveControl PORTAbits.RA4            // To control field5 valve
#define field6ValveControl PORTAbits.RA5            // To control field6 valve
#define field7ValveControl PORTFbits.RF0            // To control field7 valve
#define field8ValveControl PORTFbits.RF1            // To control field8 valve
#define field9ValveControl PORTFbits.RF2            // To control field9 valve
#define field10ValveControl PORTFbits.RF3           // To control field10 valve
#define field11ValveControl PORTFbits.RF4           // To control field11 valve
#define field12ValveControl PORTFbits.RF5           // To control field12 valve
#define fertigationValveControl PORTFbits.RF6       // To control fertigation valve
#define MotorControl PORTFbits.RF7                  // To control Motor Pump  -- DOL Red (Stop) Button (NO Relay connection)) // Deactivate DOL Manual Red button 
#define motorPowerTorque PORTGbits.RG6              // To control Capacitor Relay -- motorPowerTorque -- DOL (Start) Green Button (NO Relay connection))
#define filtration1ValveControl PORTGbits.RG7       // To control filtration1 valve
#define filtration2ValveControl PORTEbits.RE2       // To control filtration2 valve
#define filtration3ValveControl PORTEbits.RE3       // To control filtration3 valve
#define injector1Control PORTFbits.RF2              // To control injector1 valve
#define injector2Control PORTFbits.RF3              // To control injector2 valve
#define injector3Control PORTFbits.RF4              // To control injector3 valve
#define injector4Control PORTFbits.RF5              // To control injector4 valve

#define MoistureSensor1 PORTBbits.RB0           // Field1 moisture sensor measurement
#define MoistureSensor2 PORTBbits.RB1           // Field2 moisture sensor measurement
#define MoistureSensor3 PORTBbits.RB2           // Field3 moisture sensor measurement
#define MoistureSensor4 PORTBbits.RB3           // Field4 moisture sensor measurement
#define MoistureSensor5 PORTBbits.RB4           // Field5 moisture sensor measurement
#define MoistureSensor6 PORTBbits.RB5           // Field6 moisture sensor measurement
#define MoistureSensor7 PORTEbits.RE4           // Field7 moisture sensor measurement
#define MoistureSensor8 PORTDbits.RD4           // Field8 moisture sensor measurement
#define MoistureSensor9 PORTCbits.RC2           // Field9 moisture sensor measurement
#define MoistureSensor10 PORTCbits.RC3          // Field10 moisture sensor measurement
#define MoistureSensor11 PORTCbits.RC4          // Field11 moisture sensor measurement
#define MoistureSensor12 PORTCbits.RC5          // Water tank level sensor measurement

#define SIM_led PORTGbits.RG0            // System in Sleep indication
#define Run_led PORTGbits.RG3            // System running indication
#define decimalPoint PORTDbits.RD7              // BCD decimal point indication

#define RTC_Trigger PORTGbits.RG4                 // RTC Battery measurement control liine

#define phaseR PORTEbits.RE7                    // R Phase input
#define phaseY PORTEbits.RE6                    // Y Phase input
#define phaseB PORTEbits.RE5                    // B Phase input

#define bcd PORTH                             // 7 Segment BCD digit
#define dp PORTDbits.RD7
/***************************** Port line definition#end ******************************/

/***************************** Analog pin definitions#start **************/
#define CT 0b00011000
#define WindSpeed 0b00011001
#define Temperature 0b00011010
#define RTCBattery 0b00011011

#define CTchannel 0
#define WSchannel 1
#define TPchannel 2
#define RTCchannel 3
/***************************** Analog pin definition#end ****************/

/***************************** Device Reset Type definitions#start **************/
#define PowerOnReset 1
#define LowPowerReset 2
#define HardReset 3
#define SoftResest 4
#define WDTReset 5
#define StackReset 6
/***************************** Device Reset Type definition#end ****************/

/***************************** Motor Load Type definitions#start **************/
#define FullLoad 1
#define NoLoad 2
/***************************** Motor Load Type definition#end ****************/

/***************************** Boolean definitions#start **************/
#define ON 1       
#define OFF 0 
#define SET 1
#define CLEAR 0      
#define HIGH 1
#define LOW 0
#define FULL 1
#define EMPTY 0       
#define INPUT 1
#define OUTPUT 0 
#define ENABLED 1
#define DISABLED 0
#define DARK 1
#define GLOW 0
/***************************** Boolean definition#end ****************/

/***************************** Field Valve structure declaration#start ***************/
struct FIELDVALVE {
    unsigned int dryValue;              // 16 BIT  --2 LOCATION 0,1
    unsigned int wetValue;              // 16 BIT  --2 LOCATION 2,3
    unsigned int onPeriod;             	// 16 BIT  --2 LOCATION 4,5
    unsigned int fertigationDelay;      // 16 BIT  --2 LOCATION 6,7
    unsigned int fertigationONperiod;   // 16 BIT  --2 LOCATION 8,9
    unsigned int injector1OnPeriod;     // 16 BIT  --2 LOCATION 10,11
    unsigned int injector2OnPeriod;     // 16 BIT  --2 LOCATION 12,13
    unsigned int injector3OnPeriod;     // 16 BIT  --2 LOCATION 14,15
    unsigned int injector4OnPeriod;     // 16 BIT  --2 LOCATION 16,17
    unsigned int injector1OffPeriod;    // 16 BIT  --2 LOCATION 18,19  
    unsigned int injector2OffPeriod;    // 16 BIT  --2 LOCATION 20,21
    unsigned int injector3OffPeriod;    // 16 BIT  --2 LOCATION 22,23
    unsigned int injector4OffPeriod;    // 16 BIT  --2 LOCATION 24,25
    unsigned char injector1Cycle;       //  8 BIT  --1 LOCATION 26
    unsigned char injector2Cycle;       //  8 BIT  --1 LOCATION 27
    unsigned char injector3Cycle;       //  8 BIT  --1 LOCATION 28
    unsigned char injector4Cycle;       //  8 BIT  --1 LOCATION 29
    unsigned char offPeriod;            //  8 BIT  --1 LOCATION 30
    unsigned char motorOnTimeHour;      //  8 BIT  --1 LOCATION 31
    unsigned char motorOnTimeMinute;    //  8 BIT  --1 LOCATION 32
    unsigned char nextDueDD;            //  8 BIT  --1 LOCATION 33
    unsigned char nextDueMM;            //  8 BIT  --1 LOCATION 34
    unsigned char nextDueYY;            //  8 BIT  --1 LOCATION 35
    unsigned char fertigationInstance;  //  8 BIT  --1 LOCATION 36
	unsigned char priority;             //  8 BIT  --1 LOCATION 37
    unsigned char fertigationStage;     //  8 BIT  --1 LOCATION 38
    unsigned char cycles;               //  8 BIT  --1 LOCATION 39
    unsigned char cyclesExecuted;       //  8 BIT  --1 LOCATION 40
    _Bool status;                       //  8 BIT  --1 LOCATION 41
    _Bool isConfigured;                 //  8 BIT  --1 LOCATION 42
    _Bool isFertigationEnabled;         //  8 BIT  --1 LOCATION 43
    _Bool fertigationValveInterrupted;  //  8 BIT  --1 LOCATION 44
};
/***************************** Field Valve structure declaration#end *****************/

/************* statically allocated initialized user variables#start *****/
#pragma idata fieldValve
struct FIELDVALVE fieldValve[12] = {0}; //Initialize field valve structure to zero
/************* statically allocated initialized user variables#end *******/

/***************************** EEPROM Address definition#start ***********************/
/** statically allocated initialized user variables#start **/
#pragma idata eepromAddress
const unsigned int eepromAddress[16] = {0x0000, 0x0030, 0x0060, 0x0090, 0x00C0, 0x00F0, 0x0120, 0x0150, 0x0180, 0x01B0, 0x01E0, 0x0210, 0x0240, 0x0270, 0x02A0, 0x2D0}; //EEPROM Address locations from 0x00 t0 0x3FF ~1024KB
/*** statically allocated initialized user variables#end ***/
/***************************** EEPROM Address definition#end *************************/

/***************************** Macros for EEPROM Address location#start **************/
#define field1 0       // To store Field1 Valve data
#define field2 1       // To store Field2 Valve data
#define field3 2       // To store Field1 Valve data
#define field4 3       // To store Field2 Valve data
#define field5 4       // To store Field1 Valve data
#define field6 5       // To store Field2 Valve data
#define field7 6       // To store Field1 Valve data
#define field8 7       // To store Field2 Valve data
#define field9 8       // To store Field1 Valve data
#define field10 9       // To store Field2 Valve data
#define field11 10       // To store Field1 Valve data
#define field12 11       // To store Field2 Valve data
#define forFiltration 12    // To store filtration Valve data 
#define forMobileNo 13  // To store 10 byte user mobile no.
#define forPassword 14  // To store 6 byte user password
#define forSystem 15    // To store other system values
/***************************** Macros for EEPROM Address location#end ****************/

/***************************** Macros for Fertigation stages #start ****************/
#define wetPeriod 1   
#define injectPeriod 2
#define flushPeriod 3
/***************************** Macros for Fertigation stages #end ****************/

/***************************** Macros for Additional Info in SMS #start ****************/
#define noInfo 0                // No additional info required
#define newAdmin 1              // To check if password is successfully reset by another user.
#define fieldNoRequired 2       // TO mention field no in SMS
#define timeRequired 3          // To indicate time in SMS
#define secretCodeRequired 4    // To give secret factory code
#define motorLoadRequired 5     // To give motor load values
#define frequencyRequired 6     // To give motor load values
#define IrrigationData 7        // To give Irrigation values
#define filtrationData 8        // To give Filtration values
#define commonActiveFieldNoRequired 9// To give common field list to activate
#define commonInActiveFieldNoRequired 10// To give common field list to deactivate
#define fieldListRequired 11     // To give field list
#define secretCode1Required 12    // To give current password code
/***************************** Macros for Additional Info in SMS #end ****************/

/***************************** Global variables definition#start *********************/
/***** System Config definition#start *************************/
unsigned int filtrationSeperationTime = CLEAR; // To store filtration SeperationTime1 in minutes
unsigned int dueDD = CLEAR; // To store calculated due day in DD format
unsigned int sleepCount = CLEAR; // To store sleep count -- max 65550
unsigned int remainingFertigationOnPeriod = CLEAR; //To store remaining sleep count for interrupted fertigation valve
unsigned int lower8bits = CLEAR; // To store lower 8 bits for 16 bit operations
unsigned int higher8bits = CLEAR; // To store higher 8 bits for 16 bit operations
unsigned int moistureLevel = CLEAR; // To store moisture level in Hz
unsigned int injector1OnPeriod = CLEAR; // to store injector 1 on period
unsigned int injector2OnPeriod = CLEAR; // to store injector 2 on period
unsigned int injector3OnPeriod = CLEAR; // to store injector 3 on period
unsigned int injector4OnPeriod = CLEAR; // to store injector 4 on period
unsigned int injector1OnPeriodCnt = CLEAR; // to store injector 1 on period count
unsigned int injector2OnPeriodCnt = CLEAR; // to store injector 2 on period count
unsigned int injector3OnPeriodCnt = CLEAR; // to store injector 3 on period count
unsigned int injector4OnPeriodCnt = CLEAR; // to store injector 4 on period count
unsigned int injector1OffPeriod = CLEAR; // to store injector 1 on period
unsigned int injector2OffPeriod = CLEAR; // to store injector 2 on period
unsigned int injector3OffPeriod = CLEAR; // to store injector 3 on period
unsigned int injector4OffPeriod = CLEAR; // to store injector 4 on period
unsigned int injector1OffPeriodCnt = CLEAR; // to store injector 1 on period count
unsigned int injector2OffPeriodCnt = CLEAR; // to store injector 2 on period count
unsigned int injector3OffPeriodCnt = CLEAR; // to store injector 3 on period count
unsigned int injector4OffPeriodCnt = CLEAR; // to store injector 4 on period count
unsigned int noLoadCutOff = CLEAR;
unsigned int fullLoadCutOff = CLEAR;
unsigned char fieldList[12] = {'\0'}; // To store valve list to be executed simultaneous
unsigned char lastFieldList[12] = {'\0'}; // To store valve list to be switched off after execution simultaneous
unsigned char userMobileNo[11] = ""; // To store 10 byte user mobile no.
unsigned char temporaryBytesArray[20] = ""; // To store 20 byte buffer.
unsigned char null[11] = {'\0'}; // Null.
unsigned char pwd[7] = ""; // To store 6 byte user set password.
unsigned char factryPswrd[7] = ""; // To store 6 byte factory password until authentication
unsigned char injector1Cycle = CLEAR;
unsigned char injector2Cycle = CLEAR;
unsigned char injector3Cycle = CLEAR;
unsigned char injector4Cycle = CLEAR;
unsigned char injector1CycleCnt = CLEAR;
unsigned char injector2CycleCnt = CLEAR;
unsigned char injector3CycleCnt = CLEAR;
unsigned char injector4CycleCnt = CLEAR;
unsigned char timer3Count = CLEAR; // To store timer 0 overflow count
unsigned char rxCharacter = CLEAR; // To store received 1 byte character from GSM through RX pin
unsigned char msgIndex = CLEAR; // To point received character position in Message
unsigned char temp = CLEAR; // Temporary buffer
unsigned char iterator = CLEAR; // To navigate through iteration in for loop
unsigned char fieldCount = 12;   // To Store no. of fields to configure
unsigned char resetCount = CLEAR; // To store count of reset occurred by MCLR Reset for menu option
//unsigned char startFieldNo = 0;  // To indicate starting field irrigation valve no. for scanning
unsigned char space = 0x20; // Represents space Ascii
unsigned char terminateSms = 0x1A; // Represents Ctrl+z to indicate end of SMS
unsigned char enter = 0x0D; // Represents Enter Key ASCII
unsigned char newLine = 0x0A; // Represents New Line Key ASCII
unsigned char hundredsDigit = CLEAR; // To store hundred's digit no. "1XX"
unsigned char tensDigit = CLEAR; // To store ten's digit no. "X1X"
unsigned char unitsDigit = CLEAR; // To store unit's digit no. "XX1"
unsigned char Timer0Overflow = CLEAR; // To store timer0 overflow count
unsigned char Timer1Overflow = CLEAR; // To store timer1 overflow count
unsigned char Timer3Overflow = CLEAR; // To store timer1 overflow count
unsigned char filtrationCycleSequence = CLEAR; // To store filtration  cycle sequence
unsigned char currentYY = CLEAR; // To store extracted year from received date from GSM in YYYY format
unsigned char currentMM = CLEAR; // To store extracted month from received date from GSM in MM format
unsigned char currentDD = CLEAR; // To store extracted day from received date from GSM in DD format
unsigned char currentHour = CLEAR; // To store extracted hour time from received date from GSM in hr format
unsigned char currentMinutes = CLEAR; // To store extracted minutes time from received date from GSM in mm format
unsigned char currentSeconds = CLEAR; // To store extracted seconds time from received date from GSM in mm format
unsigned char dueYY = CLEAR; // To store calculated due year in YYYY format
unsigned char dueMM = CLEAR; // To store calculated due month in MM format
unsigned char filtrationDelay1 = CLEAR; // To store filtration Delay1 in minutes
unsigned char filtrationDelay2 = CLEAR; // To store filtration Delay2 in minutes
unsigned char filtrationDelay3 = CLEAR; // To store filtration Delay3 in minutes
unsigned char filtrationOnTime = CLEAR; // To store filtration OnTime in minutes
unsigned char dryRunCheckCount = CLEAR; // To store dry run check count
unsigned char nxtPriority = 1; // To store next priority for due valve by default set to first priority
/***** System Config definition#end *************************/

/******Data Encryption and Decryption#start *****************/
#ifdef Encryption_ON_H
#pragma idata encodedString 
unsigned char encodedString[100] = {'\0'};
#pragma idata stringToEncode
unsigned char stringToEncode[100] = {'\0'};
#pragma idata stringToDecode
unsigned char stringToDecode[220] = {'\0'};
#endif
#pragma idata decodedString
unsigned char decodedString[220] = {'\0'};
/***** Data Encryption and Decryption#end *********************/

/***** SMS prototype definition#start *************************/
unsigned static char admin[3] = "AU"; // To Administer device
unsigned static char hook[5] = "HOOK"; // To connect with device
unsigned static char set[4] = "SET"; // To Configure valve settings
unsigned static char enable[7] = "ENABLE"; // To enable fertigation valve
unsigned static char disable[8] = "DISABLE"; // To disable fertigation valve
unsigned static char active[7] = "ACTIVE"; // To ACTIVATE filtration valve
unsigned static char dactive[8] = "DACTIVE"; // To De-Activate filtration valve
unsigned static char hold[5] = "HOLD"; // To hold irrigation valve settings
unsigned static char extract[8] = "EXTRACT"; // To extract diagnostic data
unsigned static char ok[3] = "ok"; // Acknowledge from GSM	
unsigned static char time[5] = "TIME"; // To get current time from RTC
unsigned static char feed[5] = "FEED"; // To Set current time into RTC
unsigned static char fdata[6] = "FDATA"; // To get filtration cycle data
unsigned static char inject[7] = "INJECT"; // To Inject Test Data
unsigned static char ct[3] = "CT"; // To set motor load readings
unsigned static char setct[4] = "SCT"; // To set motor load condition thrpugh diagnostic
unsigned static char secret[12] = "12345678912"; //Secret code to fetch unique factory password
unsigned static char secret1[12] = "12345678913"; //Secret code to fetch current password
unsigned static char getct[6] = "GETCT"; // get ct values
unsigned static char getfreq[8] = "GETFREQ"; // get ct values
unsigned static char countryCode[4] = "+91"; //Country code for GSM
/***** SMS prototype definition#end ***************************/

/***** SMS strings definition#start *************************/
const char SmsAU1[23] = "Admin set successfully"; // Acknowledge user about successful Admin Registration
const char SmsAU2[51] = "You are no more Admin now. New Admin is set to\r\n"; //To notify old Admin about new Admin.
const char SmsAU3[22] = "Authentication failed"; // Acknowledge user about Authentication failed
const char SmsAU4[64] = "System Authenticated with Phase failure, suspending all actions"; // Acknowledge user about System Authenticated with Phase failure

const char SmsPwd1[32] = "Login code changed successfully"; // Acknowledge user about successful password update
const char SmsPwd2[23] = "Login code not changed";
const char SmsPwd3[23] = "Wrong login code found"; // Acknowledge user about successful motor off action

const char SmsIrr1[36] = "Irrigation configured for priority "; // Acknowledge user about successful Irrigation configuration
const char SmsIrr2[48] = "Irrigation configuration disabled for priority "; // Acknowledge user about successful Irrigation configuration disable action
const char SmsIrr3[40] = "Irrigation not configured for priority "; // Acknowledge user about  Irrigation not configured
const char SmsIrr4[30] = "Irrigation started for field "; // Acknowledge user about successful Irrigation started action
const char SmsIrr5[30] = "Irrigation stopped for field "; // Acknowledge user about successful Irrigation stopped action
const char SmsIrr6[57] = "Wet field detected.\r\nIrrigation not started for field "; // Acknowledge user about Irrigation not started due to wet field detection
const char SmsIrr7[15] = "Irrigation No:"; // Send diagnostic data for irrigation

const char SmsFert1[64] = "Irrigation is not Active. Fertigation not enabled for priority "; // Acknowledge user about Fertigation not configured due to disabled irrigation
const char SmsFert2[56] = "Incorrect values. Fertigation not enabled for priority "; // Acknowledge user about Fertigation not configured due to incorrect values
const char SmsFert3[34] = "Fertigation enabled for priority "; // Acknowledge user about successful Fertigation enabled action
const char SmsFert4[35] = "Fertigation disabled for priority "; // Acknowledge user about successful Fertigation disabled action
const char SmsFert5[31] = "Fertigation started for field "; // Acknowledge user about successful Fertigation started action
const char SmsFert6[31] = "Fertigation stopped for field "; // Acknowledge user about successful Fertigation stopped action
const char SmsFert7[68] = "Fertigation stopped with fertilizer level sensor failure for field "; // Acknowledge user about Fertigation stopped action with sensor failure
const char SmsFert8[57] = "Fertigation stopped with low fertilizer level for field "; // Acknowledge user about Fertigation stopped action with low fertilizer level 

const char SmsFilt1[27] = "Water filtration activated";
const char SmsFilt2[29] = "Water filtration deactivated"; 
const char SmsFilt3[32] = "Water Filtration is not enabled";
const char SmsFilt4[27] = "Water Filtration Sequence:";

const char SmsSR01[60] = "System restarted with phase failure, suspending all actions"; // Acknowledge user about system restarted with Valve action
const char SmsSR02[75] = "System restarted for Power Interrupt with incomplete Irrigation for field "; // Acknowledge user about system restarted with Valve action
const char SmsSR03[72] = "System restarted for Low Power In with incomplete Irrigation for field "; // Acknowledge user about system restarted with Valve action
const char SmsSR04[74] = "System restarted in Diagnostic Mode with incomplete Irrigation for field "; // Acknowledge user about system restarted with Valve action
const char SmsSR05[79] = "System restarted for All Phase Detection with incomplete Irrigation for field "; // Acknowledge user about system restarted with Valve action
const char SmsSR06[71] = "System restarted for WDT Timeout with incomplete Irrigation for field "; // Acknowledge user about system restarted with Valve action
const char SmsSR07[71] = "System restarted for Stack Error with incomplete Irrigation for field "; // Acknowledge user about system restarted with Valve action
/* //disabled SMS for reset on NO valve in action to reduce SMS count
const char SmsSR08[37] = "System restarted for Power Interrupt"; // Acknowledge user about system restarted with No Valve action
const char SmsSR09[31] = "System restarted for Low Power"; // Acknowledge user about system restarted with No Valve action
const char SmsSR10[36] = "System restarted in Diagnostic mode"; // Acknowledge user about system restarted with No Valve action
const char SmsSR11[41] = "System restarted for All Phase Detection"; // Acknowledge user about system restarted with No Valve action
const char SmsSR12[33] = "System restarted for WDT timeout"; // Acknowledge user about system restarted with No Valve action
const char SmsSR13[33] = "System restarted for stack error"; // Acknowledge user about system restarted with No Valve action
*/
const char SmsSR14[59] = "System reset occurred, login code reset to Factory setting"; // Acknowledge user about successful motor off action
const char SmsSR15[50] = "System reset occurred, Irrigation setting deleted"; // Acknowledge user about successful motor off action

const char SmsRTC1[50] = "RTC Battery found low, please replace RTC battery"; // Acknowledge user about replace RTC battery
const char SmsRTC2[41] = "System time synced to current local time"; 
const char SmsRTC3[56] = "New RTC battery found, system time is set to local time";
const char SmsRTC4[56] = "New RTC battery found, please sync system time manually"; // Acknowledge user about setting RTC through Mobile App

const char SmsDR1[130] = "Dry Run detected, Motor, Irrigation and Fertigation switched off.\r\nIrrigation scheduled to next day with fertigation for field "; // Acknowledge user about successful motor off action
const char SmsDR2[117] = "Dry Run detected, Motor and Irrigation switched off.\r\nIrrigation scheduled to next day with fertigation for field "; // Acknowledge user about successful motor off action
const char SmsDR3[100] = "Dry Run detected, Motor and Irrigation switched off.\r\nIrrigation scheduled to next day for field "; // Acknowledge user about successful motor off action
const char SmsDR4[105] = "Dry Run detected, Motor and Irrigation switched off.\r\nIrrigation scheduled to next due date for field "; // Acknowledge user about successful motor off action

const char SmsT1[27] = "Incorrect local time found"; 
const char SmsT2[15] = "Current Time: ";
/***********Debug**********/
const char SmsT3[2] = "-";
/***********Debug**********/

const char SmsMotor1[58] = "Irrigation completed for due fields\r\nMotor switched off"; // Acknowledge user about successful motor off action
const char SmsMotor2[35] = "Motorload cut-off set successfully"; // Acknowledge user about successful motor off action
const char SmsMotor3[47] = "NoLoad and FullLoad Motor thresholds set to : "; // Acknowledge user about motor load values
const char SmsMotor4[49] = "Irrigation is active, Motor load cut-off not set"; // Acknowledge user about Irrigation is active, Motor load cut-off procedure not started

const char SmsConnect[17] = "System Connected"; // Acknowledge user about successful connection

const char SmsTest[19] = "Test Data Injected"; 

const char SmsKey1[15] = "Factory Key : "; // Factory key details
const char SmsKey2[15] = "Current Key : "; // Current key details

const char SmsPh1[47] = "Phase failure detected, suspending all actions"; // Acknowledge user about Phase failure status
const char SmsPh2[69] = "Low Phase current detected, actions suspended, please restart system"; // Acknowledge user about successful motor off action
const char SmsPh3[25] = "Phase R failure detected"; // Acknowledge user about phase failure status
const char SmsPh4[25] = "Phase Y failure detected"; // Acknowledge user about phase failure status
const char SmsPh5[25] = "Phase B failure detected"; // Acknowledge user about phase failure status
const char SmsPh6[19] = "All Phase detected"; // Acknowledge user about phase status

const char SmsMS1[57] = "Moisture sensor is failed, Irrigation started for field "; // Acknowledge user about failure in moisture sensor
const char SmsMS2[43] = "Moisture sensor frequency value for field "; // Acknowledge user measured moisture sensor value
const char SmsMS3[37] = "Moisture sensor is failed for field "; // Acknowledge user about failure in moisture sensor
/***** SMS string definition#end ***************************/

/***** statically allocated initialized user variables#start **/
#pragma idata gsmResponse 
unsigned char gsmResponse[220] = "HELLO"; // To store the received message from GSM -- message can be command response or SMS
/***** statically allocated initialized user variables#end ****/

/***** statically allocated initialized user variables#start **/
#pragma idata cmti
unsigned char static cmti[14] = "+CMTI: \"SM\",x"; // This cmd is received from GSM to Notify New SMS at 1st SIM memory
/***** statically allocated initialized user variables#end ****/
/************* Strings definition#end ************************************/

/************* Booleans definition#start *********************************/ 
_Bool systemAuthenticated = false;              // To check if system is initialized by user for first time.
_Bool newSMSRcvd = false;                       // To check if communication is first initialized by GSM. 
_Bool checkMoistureSensor = false;              // To check status of Moisture sensor
_Bool moistureSensorFailed = false;             // status of Moisture sensor
_Bool controllerCommandExecuted = false;        // To check response to system cmd.
_Bool currentDateCalled = false;                // To avoid repetitive fetching of date through GSM
_Bool sleepCountChangedDueToInterrupt = false;  // TO check if sleep count need to calculate again if change due to GSM interrupt
_Bool inSleepMode = false;                      // To check if system is in sleep mode
_Bool dryRunDetected = false;                   // To state water level of tank --true for Low and --false for Full
_Bool lowPhaseCurrentDetected = false;          // To state motor not started due to low phase current
_Bool valveDue = false;                         // To indicate if any valve is due for operation after checking
_Bool valveExecuted = false;                    // To indicate if valve executed
_Bool onHold = false;                           // To indicate Field valve hold status
_Bool dueValveChecked = false;					// To indicate valve due is checked at latest 
_Bool correctDate = false;          			// To indicate received date is correct
_Bool phaseFailureDetected = false;             // To indicate phase failure
_Bool lowRTCBatteryDetected = false;            // To store RTC Battery level Status
_Bool rtcBatteryLevelChecked = false;           // To indicate condition to check RTC battery level
_Bool phaseFailureActionTaken = false;          // To indicate action taken after phase failure detection
_Bool filtrationEnabled = false;                // To store tank filtration operation status    
_Bool cmtiCmd = false;                          // Set to indicate cmti command received
_Bool DeviceBurnStatus = false;                 // To store Device program status
_Bool gsmSetToLocalTime = false;                // To indicate if gsm set to local timezone
_Bool wetSensor = false;                        // To indicate if sensor is wet
//_Bool fertigationDry = false;                   // To indicate fertigation level
_Bool fieldDueForCycles = false;                // To indicate field valve due for remaining cycles
_Bool parallelValveFetched = false;              // To indicate if parallel valves fetched
/************* BOOLeans definition#end ***********************************/
/***************************** Global variables definition#end ***********************/

#endif
/* DOL_DEFINITIONS_H */