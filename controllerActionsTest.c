/*
 * File name            : controllerActions.c
 * Compiler             : MPLAB XC8/ MPLAB C18 compiler
 * IDE                  : Microchip  MPLAB X IDE v5.25
 * Processor            : PIC18F66K40
 * Author               : Bhoomi Jalasandharan
 * Created on           : July 15, 2020, 05:23 PM
 * Description          : Controller general functions
 */

#include "congfigBits.h"
#include "variableDefinitions.h"
#include "ADC.h"
#include "controllerActions.h"
#include "eeprom.h"
#include "gsm.h"
#include "RTC_DS1307.h"
#ifdef Encryption_ON_H
#include "dataEncryption.h"
#endif
#ifdef DEBUG_MODE_ON_H
#include "serialMonitor.h"
#endif
/************************general purpose functions_start*******************************/
/*************************************************************************************************************************

This function is called to copy string from source to destination
The purpose of this function is to copy string till it finds first \n character in source.

 **************************************************************************************************************************/

char *strcpyCustom(char *restrict dest, const char *restrict src) {
	const char *s = src;
	char *d = dest;
	while ((*d++ = *s++))
        if (*s == '\n' || *s == '\r' || *s == '\0')
        break;
	return dest;
}
//****************** 1 millisecond delay function_Start******************//

/*************************************************************************************************************************

This function is called to generate delay of milli seconds
The purpose of this function is to remain in loop for given milliseconds.

 **************************************************************************************************************************/
void myMsDelay(unsigned int mSec) {
    unsigned int itr1, itr2 , calibrated=419;
    for (itr1 = 0; itr1 < mSec; itr1++)
        for (itr2 = 0; itr2 < calibrated; itr2++) {
            Nop();Nop();Nop();Nop();Nop();
            Nop();Nop();Nop();Nop();Nop();
            Nop();Nop();Nop();Nop();Nop();
            Nop();Nop();Nop();Nop();Nop();
            Nop();Nop();Nop();Nop();Nop();
        }
    /*Calibrated for a 1 ms delay in MPLAB FOR 64MHz*/
}
//****************** 1 millisecond delay function_End******************//

/*************************************************************************************************************************

This function is called to check if character is Numeric.
The purpose of this function is to check if the given character is in range of numeric ascii code

 **************************************************************************************************************************/
_Bool isNumber(unsigned char character) {
    if (character >= 48 && character <= 57) {
        return true;
    } 
    else
        return false;
}

#ifdef Encryption_ON_H
/*************************************************************************************************************************

This function is called to check if string is Base64 encoded
The purpose of this function is to check if string has space or = or multiple of 4.

 **************************************************************************************************************************/
_Bool isBase64String(unsigned char * string) {
    //unsigned int stringLength;
    unsigned char * s = string;
	while (*s++ != '\0') {
        if (*s == space) {
            return false;
        }
    }
    return true;
}
//****************** set BCD digit function_Start******************//
#endif

/*************************************************************************************************************************

This function is called to set BCD digit
The purpose of this function is to set BCD DIGIT along with DP point

 **************************************************************************************************************************/
void setBCDdigit(unsigned char digit, _Bool dp_1) {
    bcd = digit;
    dp = dp_1;    
}
//****************** set BCD digit function_End******************//

/************************Get Current Clock Time From GSM#Start************************************/

/*************************************************************************************************************************

This function is called to get current local time
The purpose of this function is to receive local time stamp from GSM module

 **************************************************************************************************************************/
void getDateFromGSM(void) {
    unsigned char index = 0;
    timer3Count = 30;  // 30 sec window
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("getDateFromGSM_IN\r\n");
    //********Debug log#end**************//
#endif
    controllerCommandExecuted = false;
    msgIndex = CLEAR;
    T3CONbits.TMR3ON = ON; // Start timer thread to unlock system if GSM fails to respond within 15 sec
    setBCDdigit(0x0B,1);  // (]) BCD indication for getDateFromGSM action
    while (!controllerCommandExecuted) {
        transmitStringToGSM("AT+CCLK?\r\n"); // To get local time stamp  +CCLK: "18/05/26,12:00:06+22"   ok
        myMsDelay(1000);
        if (!controllerCommandExecuted) {
            myMsDelay(5000);
        }
    }
    setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
    PIR5bits.TMR3IF = SET; //Stop timer thread
    // ADD indication if infinite
    tensDigit = CLEAR;
    unitsDigit = CLEAR;
    currentYY = CLEAR;
    currentMM = CLEAR;
    currentDD = CLEAR;
    currentHour = CLEAR;
    currentMinutes = CLEAR;
    currentSeconds = CLEAR;

    
    // To check no garbage value received for date time command
    for (index = 8;index<22;index+=2)
    {
        if (isNumber( gsmResponse[index]) && isNumber( gsmResponse[index+1]))
        {
           controllerCommandExecuted = true; 
        }
        else
        {
           controllerCommandExecuted = false; 
        }
    }
    
    if (!controllerCommandExecuted) {
        //RESET();
        // set indication of reset due to incorrect time stamp
    }
    else {

        tensDigit = gsmResponse[8] - 48;
        tensDigit = tensDigit * 10;
        unitsDigit = gsmResponse[9] - 48;
        currentYY = tensDigit + unitsDigit;   // Store year in decimal

        tensDigit = gsmResponse[11] - 48;
        tensDigit = tensDigit * 10;
        unitsDigit = gsmResponse[12] - 48;
        currentMM = tensDigit + unitsDigit;     // Store month in decimal

        tensDigit = gsmResponse[14] - 48;
        tensDigit = tensDigit * 10;
        unitsDigit = gsmResponse[15] - 48;
        currentDD = tensDigit + unitsDigit;     // Store day in decimal

        tensDigit = gsmResponse[17] - 48;
        tensDigit = tensDigit * 10;
        unitsDigit = gsmResponse[18] - 48;
        currentHour = tensDigit + unitsDigit;   // Store hour in decimal

        tensDigit = gsmResponse[20] - 48;
        tensDigit = tensDigit * 10;
        unitsDigit = gsmResponse[21] - 48;
        currentMinutes = tensDigit + unitsDigit; // Store minutes in decimal

        tensDigit = gsmResponse[23] - 48;
        tensDigit = tensDigit * 10;
        unitsDigit = gsmResponse[24] - 48;
        currentSeconds = tensDigit + unitsDigit; // Store minutes in decimal
    }
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("getDateFromGSM_OUT\r\n");
    //********Debug log#end**************//
#endif
}
/************************Get Current Clock Time From GSM#End************************************/


/************************Calculate Next Due Dates for Valve Action#Start************************************/

/*************************************************************************************************************************

This function is called to get due date
The purpose of this function is to calculate the future date with given days from current date

 **************************************************************************************************************************/
void getDueDate(unsigned char days) {
    unsigned int remDays = CLEAR, offset = CLEAR, leapYearDays = 366, yearDays = 365;
    unsigned char firstMonth = 1, lastMonth =12, month[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("getDueDate_IN\r\n");
    //********Debug log#end**************//
#endif
    dueDD = CLEAR, dueMM = CLEAR, dueYY = CLEAR;
    myMsDelay(100);
    fetchTimefromRTC();
    myMsDelay(100);
    dueDD = currentDD; // get todays day and set as temporary  dueDD
    switch (currentMM - 1) {
    case 11:
        dueDD += 30;
    case 10:
        dueDD += 31;
    case 9:
        dueDD += 30;
    case 8:
        dueDD += 31;
    case 7:
        dueDD += 31;
    case 6:
        dueDD += 30;
    case 5:
        dueDD += 31;
    case 4:
        dueDD += 30;
    case 3:
        dueDD += 31;
    case 2:
        dueDD += 28;
    case 1:
        dueDD += 31;
    }
    // leap year and greater than February
    if ((((2000+ (unsigned int)currentYY) % 100 != 0 && currentYY % 4 == 0) || (2000+ (unsigned int)currentYY) % 400 == 0) && currentMM > 2) {
        dueDD += 1;
    }
    //leap year
    if (((2000+ (unsigned int)currentYY) % 100 != 0 && currentYY % 4 == 0) || (2000+ (unsigned int)currentYY) % 400 == 0) {
        remDays = leapYearDays - dueDD;
    } 
    else {
        remDays = yearDays - dueDD;
    }
    if (days <= remDays) {
        dueYY = currentYY;
        dueDD += days;
    } 
    else {
        days -= remDays;
        dueYY = currentYY + 1;
        //leap year
        if (((2000+ (unsigned int)dueYY) % 100 != 0 && dueYY % 4 == 0) || (2000+ (unsigned int)dueYY) % 400 == 0) {
            offset = leapYearDays;
        } 
        else {
            offset = yearDays;
        }
        while (days >= offset) {
            days -= offset;
            dueYY++;
            //leap year
            if (((2000+ (unsigned int)dueYY) % 100 != 0 && dueYY % 4 == 0) || (2000+ (unsigned int)dueYY) % 400 == 0) {
                offset = leapYearDays;
            } 
            else {
                offset = yearDays;
            }
        }
        dueDD = days;
    }
    //leap year
    if (((2000+ (unsigned int)currentYY) % 100 != 0 && currentYY % 4 == 0) || (2000+ (unsigned int)currentYY) % 400 == 0) {
        month[2] = 29;
    }
    for (dueMM = firstMonth; dueMM <= lastMonth; dueMM++) {
        if (dueDD <= month[dueMM])
            break;
        dueDD = dueDD - month[dueMM];
    }
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("getDueDate_OUT\r\n");
    //********Debug log#end**************//
#endif
}
/************************Calculate Next Due Dates for Valve Action#End************************************/


/************************SleepCount for Next Valve Action#Start************************************/

/*************************************************************************************************************************

This function is called to get sleep count
The purpose of this function is to check if configured valve is due then calculate sleep count for on period else calculate sleep count upto the nearest configured valve setting.

 **************************************************************************************************************************/
void scanValveScheduleAndGetSleepCount(void) {
    unsigned long newCount = CLEAR; // Used to save temporary calculated sleep count
    unsigned int leapYearDays = 366, yearDays = 365;
    unsigned char iLocal = CLEAR;
    unsigned char maxPriority = CLEAR;
    _Bool firstPriorityChecked = false;
    fieldDueForCycles = false;
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("scanValveScheduleAndGetSleepCount_IN\r\n");
    //********Debug log#end**************//
#endif
    sleepCount = 65500; // Set Sleep count to default value until it is calculated
    currentDateCalled = false;
    for (iterator = 0; iterator < fieldCount; iterator++) {  // scan max configured priority
        if (fieldValve[iterator].isConfigured && fieldValve[iterator].priority > maxPriority) {
            maxPriority = fieldValve[iterator].priority;
        }
    }
    if (nxtPriority > maxPriority) { // After last field valve execution
        nxtPriority = 1;
    }
	if (maxPriority == 0) {
        valveDue = false;
        sleepCount = 4095;
        return;
    }					   
    nxtCycle:for (iterator = 0; iterator < fieldCount; iterator++) {
        if (fieldValve[iterator].priority == nxtPriority) {
            // do if Configured valve is not in action
            if (fieldValve[iterator].isConfigured && fieldValve[iterator].status != ON) {
                //get current date only for one iteration
                if (!currentDateCalled) {
                    myMsDelay(100);
                    fetchTimefromRTC(); // Get today's date
                    myMsDelay(100);
                    currentDateCalled = true; // Today's date is known
                    sleepCount = 65500; // Set Sleep count to default value until it is calculated
                }
                /*** Due date is over passed without taking action on valves ***/
                // if year over passes || if month  over passes || if day over passes || if hour over passes ||if minute over passes
                if ((currentYY > fieldValve[iterator].nextDueYY)||(currentMM > fieldValve[iterator].nextDueMM && currentYY == fieldValve[iterator].nextDueYY)||(currentDD > fieldValve[iterator].nextDueDD && currentMM == fieldValve[iterator].nextDueMM && currentYY == fieldValve[iterator].nextDueYY)||(currentHour > fieldValve[iterator].motorOnTimeHour && currentDD == fieldValve[iterator].nextDueDD && currentMM == fieldValve[iterator].nextDueMM && currentYY == fieldValve[iterator].nextDueYY)||(currentMinutes >= fieldValve[iterator].motorOnTimeMinute && currentHour == fieldValve[iterator].motorOnTimeHour && currentDD == fieldValve[iterator].nextDueDD && currentMM == fieldValve[iterator].nextDueMM && currentYY == fieldValve[iterator].nextDueYY))  {
                    valveDue = true; // Set Valve Due
                    fieldDueForCycles = false;
                    //dueCycles = false; // Set Valve due cycle
                    break;
                }
                else if (fieldValve[iterator].cyclesExecuted < fieldValve[iterator].cycles) {
                    valveDue = true; // Set Valve Due
                    fieldDueForCycles = true;
                    //dueCycles = true; // Set Valve due cycle
                    break;
                }
                // Due Date is yet to come find the sleep count to reach the Due date	
                else {
                    valveDue = false; // All due valves are operated
                    newCount = CLEAR; // clear initial temporary calculated sleep count

                    /*** temporary sleep count between today's date and valve's next due date ***/

                    for (iLocal = currentYY; iLocal < fieldValve[iterator].nextDueYY; iLocal++) {
                        if ((2000+ (unsigned int)iLocal) % 100 != 0 && iLocal % 4 == 0 && (2000+ (unsigned int)iLocal) % 400 == 0)
                            newCount += leapYearDays;
                        else
                            newCount += yearDays;
                    }
                    newCount += days(fieldValve[iterator].nextDueMM, fieldValve[iterator].nextDueYY);
                    newCount += fieldValve[iterator].nextDueDD;
                    newCount -= days(currentMM, currentYY);
                    newCount -= currentDD;
                    newCount *= 24; // converting into no. of hours
                    // Consider current hour in calculated sleep count
                    if (fieldValve[iterator].motorOnTimeHour >= currentHour) {
                        newCount += (fieldValve[iterator].motorOnTimeHour - currentHour);
                        /****converting in minutes****/
                        newCount *= 60;
                        if (currentMinutes >= fieldValve[iterator].motorOnTimeMinute) {
                            newCount -= (currentMinutes - fieldValve[iterator].motorOnTimeMinute);
                        } 
                        else {
                            newCount += (fieldValve[iterator].motorOnTimeMinute - currentMinutes);
                        }
                    }
                    // Subtract current hour from calculated sleep count
                    else if (fieldValve[iterator].motorOnTimeHour < currentHour) {
                        newCount -= (currentHour - fieldValve[iterator].motorOnTimeHour);
                        /****converting in minutes****/
                        newCount *= 60;
                        if (currentMinutes >= fieldValve[iterator].motorOnTimeMinute) {
                            newCount -= (currentMinutes - fieldValve[iterator].motorOnTimeMinute);
                        } 
                        else {
                            newCount += (fieldValve[iterator].motorOnTimeMinute - currentMinutes);
                        }
                    }
                    // Valve is due in a minute
                    if (newCount == 0 || newCount == 1) {
                        sleepCount = 1;                             // calculate sleep count for upcoming due valve
                    }
                    // Save sleep count for nearest next valve action  
                    else if (newCount < sleepCount) {
                        sleepCount = (unsigned int)newCount;                      // calculate sleep count for upcoming due valve
                    }
                }
            }
        }
    }
    if (!valveDue) {
        if (nxtPriority == 1) {
            firstPriorityChecked = true;
        }
        nxtPriority = nxtPriority + 1;     // Scan All valves for next priority
        if (nxtPriority <= maxPriority) {
            goto nxtCycle;
        }
        else if (!firstPriorityChecked ) {
            nxtPriority = 1;               // Reset nxtPriority after scanning all irrigation valves from start field no.
            goto nxtCycle;
        }
        if (sleepCount > 1 && sleepCount < 4369) {
            sleepCount = sleepCount*15;
            sleepCount = (sleepCount/17);
        }
        else if (sleepCount >= 4369) {
            sleepCount = 4095;
        }
    #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("scanValveScheduleAndGetSleepCount_W/OValveDue_OUT\r\n");
        //********Debug log#end**************//
    #endif
    }
    else { // valve due
        fieldList[0] = iterator;
        /* check Fertigation status and set sleep count to fertigation wet period*/
        if(fieldValve[iterator].isFertigationEnabled && fieldValve[iterator].fertigationInstance != 0) {
            sleepCount = fieldValve[iterator].fertigationDelay; // calculate sleep count for fertigation delay 
            fieldValve[iterator].fertigationStage = wetPeriod;
            saveFertigationValveStatusIntoEeprom(eepromAddress[iterator], &fieldValve[iterator]);
        #ifdef DEBUG_MODE_ON_H
            //********Debug log#start************//
            transmitStringToDebug("scanValveScheduleAndGetSleepCount_ValveDueWithFertigation_OUT\r\n");
            //********Debug log#end**************//
        #endif
        }/*if Only Irrigation valve set sleep count to ON period*/
        else {
            sleepCount = fieldValve[iterator].onPeriod; // calculate sleep count for Valve on period 
        #ifdef DEBUG_MODE_ON_H
            //********Debug log#start************//
            transmitStringToDebug("scanValveScheduleAndGetSleepCount_ValveDueW/OFertigation_OUT\r\n");
            //********Debug log#end**************//
        #endif   
        }
    }
}
/************************SleepCount for Next Valve Action#End************************************/


/************************Days Between Two Dates#Start************************************/

/*************************************************************************************************************************

This function is called to get no. of days.
The purpose of this function is to calculate no. of days left in the calender year from given month and year
It counts no.of days from 1st January of calender to 1st day of given month
 **************************************************************************************************************************/
unsigned int days(unsigned char mm, unsigned char yy) {
    unsigned char itr = CLEAR, month[12] = {31, 0, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    unsigned int days = CLEAR;
    for (itr = 0; itr < mm - 1; itr++) {
        if (itr == 1) {
            if ((2000+ (unsigned int)yy) % 100 != 0 && yy % 4 == 0 && (2000+ (unsigned int)yy) % 400 == 0)
                days += 29;
            else
                days += 28;
        } else
            days += month[itr];
    }
    return (days);
}
/************************Days Between Two Dates#End************************************/


/************************Extract field no. in msg#Start************************************/

/*************************************************************************************************************************

This function is called to fetch field no mentioned in sms.
The purpose of this function is to navigate through received sms and fetch field no from given position 
It fetches two digit field no. from given position
 **************************************************************************************************************************/
unsigned char fetchFieldNo(unsigned char index) {
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("fetchFieldNo_IN: ");
    //********Debug log#end**************//
#endif
    if (decodedString[index] == '0' && decodedString[index+1] == '1') {
    #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        temporaryBytesArray[0] = 48; // To store field no. of valve in action 
        temporaryBytesArray[1] = 49; // To store field no. of valve in action 
        transmitNumberToDebug(temporaryBytesArray, 2);
        transmitStringToDebug("\r\nfetchFieldNo_OUT\r\n");
        //********Debug log#end**************//
    #endif
        return 0;    
    }
    else if (decodedString[index] == '0' && decodedString[index+1] == '2') {
    #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        temporaryBytesArray[0] = 48; // To store field no. of valve in action 
        temporaryBytesArray[1] = 50; // To store field no. of valve in action 
        transmitNumberToDebug(temporaryBytesArray, 2);
        transmitStringToDebug("\r\nfetchFieldNo_OUT\r\n");
        //********Debug log#end**************//
    #endif
        return 1;            
    }    
    else if (decodedString[index] == '0' && decodedString[index+1] == '3') {
    #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        temporaryBytesArray[0] = 48; // To store field no. of valve in action 
        temporaryBytesArray[1] = 51; // To store field no. of valve in action 
        transmitNumberToDebug(temporaryBytesArray, 2);
        transmitStringToDebug("\r\nfetchFieldNo_OUT\r\n");
        //********Debug log#end**************//
    #endif
        return 2;    
    }
    else if (decodedString[index] == '0' && decodedString[index+1] == '4') {
    #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        temporaryBytesArray[0] = 48; // To store field no. of valve in action 
        temporaryBytesArray[1] = 52; // To store field no. of valve in action 
        transmitNumberToDebug(temporaryBytesArray, 2);
        transmitStringToDebug("\r\nfetchFieldNo_OUT\r\n");
        //********Debug log#end**************//
    #endif
        return 3;    
    }
    else if (decodedString[index] == '0' && decodedString[index+1] == '5') {
    #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        temporaryBytesArray[0] = 48; // To store field no. of valve in action 
        temporaryBytesArray[1] = 53; // To store field no. of valve in action 
        transmitNumberToDebug(temporaryBytesArray, 2);
        transmitStringToDebug("\r\nfetchFieldNo_OUT\r\n");
        //********Debug log#end**************//
    #endif
        return 4;    
    }
    else if (decodedString[index] == '0' && decodedString[index+1] == '6') {
    #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        temporaryBytesArray[0] = 48; // To store field no. of valve in action 
        temporaryBytesArray[1] = 54; // To store field no. of valve in action 
        transmitNumberToDebug(temporaryBytesArray, 2);
        transmitStringToDebug("\r\nfetchFieldNo_OUT\r\n");
        //********Debug log#end**************//
    #endif
        return 5;    
    }
    else if (decodedString[index] == '0' && decodedString[index+1] == '7') {
    #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        temporaryBytesArray[0] = 48; // To store field no. of valve in action 
        temporaryBytesArray[1] = 55; // To store field no. of valve in action 
        transmitNumberToDebug(temporaryBytesArray, 2);
        transmitStringToDebug("\r\nfetchFieldNo_OUT\r\n");
        //********Debug log#end**************//
    #endif
        return 6;    
    }
    else if (decodedString[index] == '0' && decodedString[index+1] == '8') {
    #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        temporaryBytesArray[0] = 48; // To store field no. of valve in action 
        temporaryBytesArray[1] = 56; // To store field no. of valve in action 
        transmitNumberToDebug(temporaryBytesArray, 2);
        transmitStringToDebug("\r\nfetchFieldNo_OUT\r\n");
        //********Debug log#end**************//
    #endif
        return 7;   
    }
    else if (decodedString[index] == '0' && decodedString[index+1] == '9') {
    #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        temporaryBytesArray[0] = 48; // To store field no. of valve in action 
        temporaryBytesArray[1] = 57; // To store field no. of valve in action 
        transmitNumberToDebug(temporaryBytesArray, 2);
        transmitStringToDebug("\r\nfetchFieldNo_OUT\r\n");
        //********Debug log#end**************//
    #endif
        return 8;   
    }
    else if (decodedString[index] == '1' && decodedString[index+1] == '0') {
    #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        temporaryBytesArray[0] = 49; // To store field no. of valve in action 
        temporaryBytesArray[1] = 48; // To store field no. of valve in action 
        transmitNumberToDebug(temporaryBytesArray, 2);
        transmitStringToDebug("\r\nfetchFieldNo_OUT\r\n");
        //********Debug log#end**************//
    #endif
        return 9;    
    }
    else if (decodedString[index] == '1' && decodedString[index+1] == '1') {
    #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        temporaryBytesArray[0] = 49; // To store field no. of valve in action 
        temporaryBytesArray[1] = 49; // To store field no. of valve in action 
        transmitNumberToDebug(temporaryBytesArray, 2);
        transmitStringToDebug("\r\nfetchFieldNo_OUT\r\n");
        //********Debug log#end**************//
    #endif
        return 10;    
    }
    else if (decodedString[index] == '1' && decodedString[index+1] == '2') {
    #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        temporaryBytesArray[0] = 49; // To store field no. of valve in action 
        temporaryBytesArray[1] = 50; // To store field no. of valve in action 
        transmitNumberToDebug(temporaryBytesArray, 2);
        transmitStringToDebug("\r\nfetchFieldNo_OUT\r\n");
        //********Debug log#end**************//
    #endif
        return 11;    
    }
    else {
    #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        temporaryBytesArray[0] = 57; // To store field no. of valve in action 
        temporaryBytesArray[1] = 57; // To store field no. of valve in action 
        transmitNumberToDebug(temporaryBytesArray, 2);
        transmitStringToDebug("\r\nfetchFieldNo_OUT\r\n");
        //********Debug log#end**************//
    #endif
        return 255;
    }
    
}
/************************Extract field no. in msg#End************************************/

/************************Fetch array of  field valves due on same time#Start************************************/

/*************************************************************************************************************************

This function is called to fetch no. of valves due for same time
The purpose of this function is to navigate through configured valve list and fetch valves due on same date and time 
It generate array of field valve no.s to be activated on same time
**************************************************************************************************************************/
void fetchParallelValveList(unsigned char FieldNo) {
    unsigned char localIndex = 1;
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("fetchParallelValveList_IN\r\n");
    //********Debug log#end**************//
#endif
    // Reset Active Valve list
    iterator = 0;
    while(iterator < fieldCount) {
        fieldList[iterator] = 255;
        iterator++;
    }
    fieldList[0] = FieldNo; // assign due valve
    if (fieldDueForCycles) {
        for (iterator = 0; iterator < fieldCount ; iterator++) {
            if (iterator != FieldNo) { // not assigned valve
                if (fieldValve[iterator].isConfigured && fieldValve[iterator].priority == fieldValve[FieldNo].priority) { // Fetch valves with same priority first
                    if (fieldValve[iterator].cyclesExecuted < fieldValve[iterator].cycles){ // Among same priority fetch valves with same due date and cycles remaining
                        fieldList[localIndex] = iterator;
                        localIndex++;
                    }
                }
            }
        }
    }
    else {
        for (iterator = 0; iterator < fieldCount ; iterator++) {
            if (iterator != FieldNo) { // not assigned valve
                if (fieldValve[iterator].isConfigured && fieldValve[iterator].priority == fieldValve[FieldNo].priority) { // Fetch valves with same priority first
                    if (fieldValve[iterator].motorOnTimeHour == fieldValve[FieldNo].motorOnTimeHour && fieldValve[iterator].motorOnTimeMinute == fieldValve[FieldNo].motorOnTimeMinute && fieldValve[iterator].nextDueDD == fieldValve[FieldNo].nextDueDD && fieldValve[iterator].nextDueMM == fieldValve[FieldNo].nextDueMM && fieldValve[iterator].nextDueYY == fieldValve[FieldNo].nextDueYY) { // Among same priority fetch valves with same due date and cycles remaining
                        fieldList[localIndex] = iterator;
                        localIndex++;
                    }
                }
            }
        }
        // Reset cycles for Active Valve list when due for date and not for cycles
        iterator = 0;
        while(iterator < fieldCount) {
            fieldValve[fieldList[iterator]].cyclesExecuted = fieldValve[fieldList[iterator]].cycles; // reset cycles when cycles are incomplete for interrupt valve
            iterator++;
        } 
    }
    
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("fetchParallelValveList_OUT\r\n");
    //********Debug log#end**************//
#endif
}
/************************Fetch array of  field valves due on same time##End************************************/


/************************Actions on Received SMS#Start************************************/

/*************************************************************************************************************************

This function is called to extract received message after new message notification is received.
The purpose of this function is to read sms received at 1st SIM memory location.
The Action is decided upon Type of message received.

 **************************************************************************************************************************/
void extractReceivedSms(void) {
    unsigned char count = CLEAR, onHour = CLEAR, onMinute = CLEAR, fetchedPriority = CLEAR;
    unsigned int digit = CLEAR;
	//timer3Count = 30; // 30 sec window
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("extractReceivedSms_IN\r\n");
    //********Debug log#end**************//
#endif
	//#2>.............Connection message...............//
    // if received connection command is from userMobileNo
    if (strncmp(decodedString, hook, 4) == 0) {
        msgIndex = CLEAR;
        /***************************/                
        sendSms(SmsConnect, userMobileNo, noInfo);
    #ifdef SMS_DELIVERY_REPORT_ON_H
        sleepCount = 2; // Load sleep count for SMS transmission action
        sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
        setBCDdigit(0x05,0);
        deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
        setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
    #endif
        /***************************/
    #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("extractReceivedSms_Connect_OUT\r\n");
        //********Debug log#end**************// 
    #endif
        return;
    }
    //#3>..............Change Password.................// 
    // set Admin format (AU 123456 666666)
    else if (strncmp(decodedString, admin, 2) == 0) {
        if (strncmp(decodedString + 3, pwd, 6) == 0) {
            strncpy(pwd, decodedString + 10, 6);
            savePasswordIntoEeprom(); //store password into eeprom
            msgIndex = CLEAR;
            /***************************/                  
            sendSms(SmsPwd1, userMobileNo, noInfo);
        #ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
            setBCDdigit(0x05,0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
            setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
        #endif
            /***************************/
        #ifdef DEBUG_MODE_ON_H
            //********Debug log#start************//
            transmitStringToDebug("extractReceivedSms_Password updated_OUT\r\n");
            //********Debug log#end**************// 
        #endif
            return;
        } 
        else {
            msgIndex = CLEAR;
            /***************************/
            sendSms(SmsPwd3, userMobileNo, noInfo);
        #ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
            setBCDdigit(0x05,0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
            setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
        #endif
            /***************************/ 
        #ifdef DEBUG_MODE_ON_H
            //********Debug log#start************//
            transmitStringToDebug("extractReceivedSms_Password not changed_OUT\r\n");
            //********Debug log#end**************//
        #endif
            return;
        }
    }			 
    //#4>..............Save New Configured values of Field Valve X .................//
    //Msg Format---------***SET<FieldNo.><Space><OnPeriod><Space><OffPeriod><Space><MotorOnTimeHr><Space><MotorOnTimeMins><Space><SensorDryValue><Space><SensorWetValue><Space>***------------//
    //SMS Location---------63--------<72/73>------<74>-----------<77/78>---------<79/80/81>-------------//

    else if (strncmp(decodedString, set, 3) == 0) {
        digit = CLEAR;
        count = CLEAR; //count to extract onperiod, offperiod,motorOnHour,motorOnMinute,DryValue, wetValue; i.e total 9 attributes
        fetchedPriority = fetchFieldNo(3) + 1;   // fetch priority and add 1 for 1-12 range
        // Erase existing valve details for fetched priority
        for (iterator = 0; iterator < fieldCount; iterator++) {
            if (fieldValve[iterator].priority == fetchedPriority) {
				fieldValve[iterator].isConfigured = false;
                fieldValve[iterator].priority = 0; // reset priority as well
                saveIrrigationValvePriorityIntoEeprom(eepromAddress[iterator], &fieldValve[iterator]);
                myMsDelay(100);
                saveIrrigationValveConfigurationStatusIntoEeprom(eepromAddress[iterator], &fieldValve[iterator]);
                myMsDelay(100);
            }
        }
        for (iterator = 0; iterator < 20; iterator++)
            temporaryBytesArray[iterator] = 255;    // blank list

        // fetch valve list to be configured for fetched priority
        for (count = 0, msgIndex = 5; count < 7; msgIndex++) {
            if (decodedString[msgIndex] == space) {
                count++;
                if (count == 7) {
                    for (iterator = 0 ; decodedString[msgIndex+1]!= space; iterator++) {
                        temporaryBytesArray[iterator] = fetchFieldNo(msgIndex+1);
                        msgIndex = msgIndex+2;
                    }
                }
            }
        }
        for (iterator = 0; temporaryBytesArray[iterator] != 255; iterator++) {
            for (count = 1, msgIndex = 6; count <= 9 ; msgIndex++) {
                //is number
                if (isNumber(decodedString[msgIndex])) {
                    if (count != 7 ) { // skip priority 
                        if (decodedString[msgIndex + 1] == space) {
                            temp = decodedString[msgIndex] - 48;
                            digit = digit + temp;
                        } 
                        else {
                            temp = decodedString[msgIndex] - 48;
                            temp = temp * 10;
                            digit = digit * 10;
                            digit = digit + temp;
                        }
                    }   
                } 
                else {
                    switch (count) {
                    case 1: // code to extract on period;
                        fieldValve[temporaryBytesArray[iterator]].onPeriod = digit;
                        digit = CLEAR;
                        break;
                    case 2: // code to extract off period;
                        fieldValve[temporaryBytesArray[iterator]].offPeriod = (unsigned char)digit;
                        digit = CLEAR;
                        break;
                    case 3: // code to extract motorOnTimeHour;
                        fieldValve[temporaryBytesArray[iterator]].motorOnTimeHour = (unsigned char)digit;
                        digit = CLEAR;
                        break;
                    case 4: // code to extract motorOnTimeMinute;
                        fieldValve[temporaryBytesArray[iterator]].motorOnTimeMinute = (unsigned char)digit;
                        digit = CLEAR;
                        break;
                    case 5: // code to extract dryValue
                        fieldValve[temporaryBytesArray[iterator]].dryValue = digit;
                        digit = CLEAR;
                        break;
                    case 6: // code to extract wetValue
                        fieldValve[temporaryBytesArray[iterator]].wetValue = digit;
                        digit = CLEAR;
                        break;
                    case 7: // code to extract priority
                        fieldValve[temporaryBytesArray[iterator]].priority = fetchedPriority;
                        digit = CLEAR;
                        break;
                    case 8: // code to extract cycles
                        fieldValve[temporaryBytesArray[iterator]].cycles = (unsigned char)digit;
                        fieldValve[temporaryBytesArray[iterator]].cyclesExecuted = (unsigned char)digit;
                        digit = CLEAR;
                        break;    
                    case 9: // code to extract day count;
                        getDueDate((unsigned char)digit); // Get due dates w.r.t triggered from date
                        digit = CLEAR;
                        fieldValve[temporaryBytesArray[iterator]].nextDueDD = (unsigned char)dueDD;
                        fieldValve[temporaryBytesArray[iterator]].nextDueMM = dueMM;
                        fieldValve[temporaryBytesArray[iterator]].nextDueYY = dueYY;
                        fieldValve[temporaryBytesArray[iterator]].status = OFF;
                        fieldValve[temporaryBytesArray[iterator]].isConfigured = true;
                        fieldValve[temporaryBytesArray[iterator]].fertigationDelay = 0;
                        fieldValve[temporaryBytesArray[iterator]].fertigationONperiod = 0;
                        fieldValve[temporaryBytesArray[iterator]].fertigationInstance = 0;
                        fieldValve[temporaryBytesArray[iterator]].isFertigationEnabled = false;
                        fieldValve[temporaryBytesArray[iterator]].fertigationStage = OFF;
                        fieldValve[temporaryBytesArray[iterator]].fertigationValveInterrupted = false;
                        break;
                    }
                    count++;
                }
            }
            myMsDelay(100);
            saveIrrigationValveValuesIntoEeprom(eepromAddress[temporaryBytesArray[iterator]], &fieldValve[temporaryBytesArray[iterator]]);
            myMsDelay(100);
            saveIrrigationValveDueTimeIntoEeprom(eepromAddress[temporaryBytesArray[iterator]], &fieldValve[temporaryBytesArray[iterator]]);
            myMsDelay(100);
            saveIrrigationValveOnOffStatusIntoEeprom(eepromAddress[temporaryBytesArray[iterator]], &fieldValve[temporaryBytesArray[iterator]]);
            myMsDelay(100);
            saveIrrigationValveCycleStatusIntoEeprom(eepromAddress[temporaryBytesArray[iterator]], &fieldValve[temporaryBytesArray[iterator]]);
            myMsDelay(100);
            saveIrrigationValveConfigurationStatusIntoEeprom(eepromAddress[temporaryBytesArray[iterator]], &fieldValve[temporaryBytesArray[iterator]]);
            myMsDelay(100);
            saveFertigationValveValuesIntoEeprom(eepromAddress[temporaryBytesArray[iterator]], &fieldValve[temporaryBytesArray[iterator]]);
            myMsDelay(100);
        }
        /***************************/
        // for field no. 01 to 09
        if (fetchedPriority < 10) {
            temporaryBytesArray[0] = 48; // To store field no. of valve in action 
            temporaryBytesArray[1] = fetchedPriority + 48; // To store field no. of valve in action 
        }// for field no. 10 to 12
        else if (fetchedPriority >=10 && fetchedPriority <= 12) {
            temporaryBytesArray[0] = 49; // To store field no. of valve in action 
            temporaryBytesArray[1] = fetchedPriority + 38; // To store field no. of valve in action 
        }
        /***************************/
        msgIndex = CLEAR;                   
        /***************************/
        sendSms(SmsIrr1, userMobileNo, fieldNoRequired); // Acknowledge user about successful Irrigation configuration
    #ifdef SMS_DELIVERY_REPORT_ON_H
        sleepCount = 2; // Load sleep count for SMS transmission action
        sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
        setBCDdigit(0x05,0);
        deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
        setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
    #endif
        /***************************/ 
    #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("extractReceivedSms_Configure_SmsIrr1_OUT\r\n");
        //********Debug log#end**************//
    #endif
        return;
    }
	//#5>..............Hold Field x Irrigation.................//
    // Hold<x>
    else if (strncmp(decodedString, hold, 4) == 0) {
        temp = 255;
        onHold = false;
		fetchedPriority = fetchFieldNo(4) + 1;   // fetch priority and add 1 for 1-12 range
		// scan priority valve status
		for (iterator = 0; iterator < fieldCount; iterator++) {
            if (fieldValve[iterator].priority == fetchedPriority && fieldValve[iterator].isConfigured == true) {
				temp = iterator; // meaning temp != 255
                // if Active Valve
                if (fieldValve[iterator].status == ON) { // hold active valve
                    onHold = true; // execute onhold condition of sleepcount finish
                    nxtPriority = fieldValve[iterator].priority + 1;
				}
                break;
            }
		}
        if (temp == 255) { // none of the valve have this priority										   
            /***************************/
            // for field no. 01 to 09
            if (fetchedPriority < 10) {
				temporaryBytesArray[0] = 48; // To store field no. of valve in action 
                temporaryBytesArray[1] = fetchedPriority + 48; // To store field no. of valve in action 
            }// for field no. 10 to 12
            else if (fetchedPriority >=10 && fetchedPriority <= 12) {
                temporaryBytesArray[0] = 49; // To store field no. of valve in action 
                temporaryBytesArray[1] = fetchedPriority + 38; // To store field no. of valve in action 
            }
            /***************************/
            msgIndex = CLEAR;                   
            /***************************/
            sendSms(SmsIrr3, userMobileNo, fieldNoRequired); // Acknowledge user about successful Irrigation configuration disable action
        #ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
            setBCDdigit(0x05,0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
            setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
        #endif
            /***************************/ 
        #ifdef DEBUG_MODE_ON_H
            //********Debug log#start************//
            transmitStringToDebug("extractReceivedSms_DisableIrrigation_SmsIrr3_OUT\r\n");
            //********Debug log#end**************//
        #endif
            return;
        }
        else {
            //Hold configuration for valves having same priority
            for (iterator = 0; iterator < fieldCount ; iterator++) {
                if (fieldValve[iterator].priority == fetchedPriority) { // Fetch valves with same priority first
                    fieldValve[iterator].isConfigured = false; // configuration hold
                    fieldValve[iterator].priority = 0; // reset priority as well
                    saveIrrigationValvePriorityIntoEeprom(eepromAddress[iterator], &fieldValve[iterator]);
                    myMsDelay(100);
                    saveIrrigationValveConfigurationStatusIntoEeprom(eepromAddress[iterator], &fieldValve[iterator]);
                    myMsDelay(100);
                    if (fieldValve[iterator].isFertigationEnabled == true) {
                        fieldValve[iterator].isFertigationEnabled = false; // configuration hold
                        saveFertigationValveValuesIntoEeprom(eepromAddress[iterator], &fieldValve[iterator]);
                        myMsDelay(100);
                    }
                    if (fieldValve[iterator].cyclesExecuted == fieldValve[iterator].cycles) {
                        fieldValve[iterator].cyclesExecuted = 1; //Cycles execution begin after valve due for first time
                    }
                    else {
                        fieldValve[iterator].cyclesExecuted++; //Cycles execution record
                    }
                    saveIrrigationValveCycleStatusIntoEeprom(eepromAddress[iterator], &fieldValve[iterator]);
                    myMsDelay(100);
                }
            }             
            /***************************/
            // for field no. 01 to 09
            if (fetchedPriority < 10) {
                temporaryBytesArray[0] = 48; // To store field no. of valve in action 
                temporaryBytesArray[1] = fetchedPriority + 48; // To store field no. of valve in action 
            }// for field no. 10 to 12
            else if (fetchedPriority >=10 && fetchedPriority <= 12) {
                temporaryBytesArray[0] = 49; // To store field no. of valve in action 
                temporaryBytesArray[1] = fetchedPriority + 38; // To store field no. of valve in action 
            }
            /***************************/
            msgIndex = CLEAR;                   
            /***************************/
            sendSms(SmsIrr2, userMobileNo, fieldNoRequired); // Acknowledge user about successful Irrigation configuration disable action
        #ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
            setBCDdigit(0x05,0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
            setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
        #endif
            /***************************/ 
        #ifdef DEBUG_MODE_ON_H
            //********Debug log#start************//
            transmitStringToDebug("extractReceivedSms_DisableIrrigation_SmsIrr2_OUT\r\n");
            //********Debug log#end**************//
        #endif
            return;
        }
    }
    //#6>.............. Enable Fertigation Valve for field.................//
        //Msg Format---------***Enable<FieldNo.><Space><Delay><Space><ONPeriod><Space><Instance><Space>***------------//
        //SMS Location---------63--------<69>------<71>-----------<73/74>---------<78/79/80>-------------//

    else if (strncmp(decodedString, enable, 6) == 0) {
        digit = CLEAR;
        count = CLEAR; //count to extract delayStart, onPeriod, no. of times
        fetchedPriority = fetchFieldNo(6) + 1;   // fetch priority and add 1 for 1-12 range
        for (iterator = 0; iterator < fieldCount; iterator++) {
            if (fieldValve[iterator].priority == fetchedPriority && fieldValve[iterator].isConfigured == true) {			  
                for (msgIndex = 9; count < 15 ; msgIndex++) {
                    if (isNumber(decodedString[msgIndex])) {
                        if (decodedString[msgIndex + 1] == space) {
                            temp = decodedString[msgIndex] - 48;
                            digit = digit + temp;
                        } 
                        else {
                            temp = decodedString[msgIndex] - 48;
                            temp = temp * 10;
                            digit = digit * 10;
                            digit = digit + temp;
                        }
                    } 
                    else {
                        count++;
                        switch (count) {
                        case 1: // code to extract fertigationDelay;
                            fieldValve[iterator].fertigationDelay = digit;
                            digit = CLEAR;
                            break;
                        case 2: // code to extract fertigationONperiod;
                            fieldValve[iterator].fertigationONperiod = digit;
                            digit = CLEAR;
                            break;
                        case 3: // code to extract fertigationInstance;
                            fieldValve[iterator].fertigationInstance = (unsigned char)digit;
                            digit = CLEAR;
                            break;
                        case 4: // code to extract injector1OnPeriod;
                            fieldValve[iterator].injector1OnPeriod = digit;
                            digit = CLEAR;
                            break;
                        case 5: // code to extract injector1OffPeriod;
                            fieldValve[iterator].injector1OffPeriod = digit;
                            digit = CLEAR;
                            break;
                        case 6: // code to extract injector1Cycle;
                            fieldValve[iterator].injector1Cycle = (unsigned char)digit;
                            digit = CLEAR;
                            break;
                        case 7: // code to extract injector2OnPeriod;
                            fieldValve[iterator].injector2OnPeriod = digit;
                            digit = CLEAR;
                            break;
                        case 8: // code to extract injector2OffPeriod;
                            fieldValve[iterator].injector2OffPeriod = digit;
                            digit = CLEAR;
                            break;
                        case 9: // code to extract injector2Cycle;
                            fieldValve[iterator].injector2Cycle = (unsigned char)digit;
                            digit = CLEAR;
                            break;
                        case 10: // code to extract injector3OnPeriod;
                            fieldValve[iterator].injector3OnPeriod = digit;
                            digit = CLEAR;
                            break;
                        case 11: // code to extract injector3OffPeriod;
                            fieldValve[iterator].injector3OffPeriod = digit;
                            digit = CLEAR;
                            break;
                        case 12: // code to extract injector3Cycle;
                            fieldValve[iterator].injector3Cycle = (unsigned char)digit;
                            digit = CLEAR;
                            break;
                        case 13: // code to extract injector4OnPeriod;
                            fieldValve[iterator].injector4OnPeriod = digit;
                            digit = CLEAR;
                            break;
                        case 14: // code to extract injector4OffPeriod;
                            fieldValve[iterator].injector4OffPeriod = digit;
                            digit = CLEAR;
                            break;
                        case 15: // code to extract injector1Cycle;
                            fieldValve[iterator].injector4Cycle = (unsigned char)digit;
                            fieldValve[iterator].fertigationStage = OFF;
                            fieldValve[iterator].fertigationValveInterrupted = false;
                            digit = CLEAR;
                            if ((fieldValve[iterator].fertigationDelay + fieldValve[iterator].fertigationONperiod) >= fieldValve[iterator].onPeriod) {
                                fieldValve[iterator].isFertigationEnabled = false;
                                /***************************/
                                // for field no. 01 to 09
                                if (fetchedPriority < 10) {
                                    temporaryBytesArray[0] = 48; // To store field no. of valve in action 
                                    temporaryBytesArray[1] = fetchedPriority + 48; // To store field no. of valve in action 
                                }// for field no. 10 to 12
                                else if (fetchedPriority >=10 && fetchedPriority <= 12) {
                                    temporaryBytesArray[0] = 49; // To store field no. of valve in action 
                                    temporaryBytesArray[1] = fetchedPriority + 38; // To store field no. of valve in action 
                                }
                                /***************************/
                                msgIndex = CLEAR;
                                /***************************/
                                sendSms(SmsFert2, userMobileNo, fieldNoRequired);   // Acknowledge user about Fertigation not configured due to incorrect values
                            #ifdef SMS_DELIVERY_REPORT_ON_H
                                sleepCount = 2; // Load sleep count for SMS transmission action
                                sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
                                setBCDdigit(0x05,0);
                                deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
                                setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
                            #endif
                                /***************************/
                            #ifdef DEBUG_MODE_ON_H
                                //********Debug log#start************//
                                transmitStringToDebug("extractReceivedSms_EnableFertigation_SmsFert2_OUT\r\n");
                                //********Debug log#end**************//
                            #endif
                                return;
                            }
                            else {
                                fieldValve[iterator].isFertigationEnabled = true;
                                myMsDelay(100);
                                saveFertigationValveValuesIntoEeprom(eepromAddress[iterator], &fieldValve[iterator]);
                                myMsDelay(100);
                                temp = iterator;
                                // Copy Paste fertigation values for valves having same priority
                                for (iterator = 0; iterator < fieldCount ; iterator++) {
                                    if (iterator != temp) { // not assigned valve
                                        if (fieldValve[iterator].isConfigured && fieldValve[iterator].priority == fieldValve[temp].priority) { // Fetch valves with same priority first
                                            fieldValve[iterator].isFertigationEnabled = true; 
                                            fieldValve[iterator].fertigationDelay = fieldValve[temp].fertigationDelay;
                                            fieldValve[iterator].fertigationONperiod = fieldValve[temp].fertigationONperiod;
                                            fieldValve[iterator].fertigationInstance = fieldValve[temp].fertigationInstance;
                                            fieldValve[iterator].injector1OnPeriod = fieldValve[temp].injector1OnPeriod;
                                            fieldValve[iterator].injector1OffPeriod = fieldValve[temp].injector1OffPeriod;
                                            fieldValve[iterator].injector1Cycle = fieldValve[temp].injector1Cycle;
                                            fieldValve[iterator].injector2OnPeriod = fieldValve[temp].injector2OnPeriod;
                                            fieldValve[iterator].injector2OffPeriod = fieldValve[temp].injector2OffPeriod;
                                            fieldValve[iterator].injector2Cycle = fieldValve[temp].injector2Cycle;
                                            fieldValve[iterator].injector3OnPeriod = fieldValve[temp].injector3OnPeriod;
                                            fieldValve[iterator].injector3OffPeriod = fieldValve[temp].injector3OffPeriod;
                                            fieldValve[iterator].injector3Cycle = fieldValve[temp].injector3Cycle;
                                            fieldValve[iterator].injector4OnPeriod = fieldValve[temp].injector4OnPeriod;
                                            fieldValve[iterator].injector4OffPeriod = fieldValve[temp].injector4OffPeriod;
                                            fieldValve[iterator].injector4Cycle = fieldValve[temp].injector4Cycle;
                                            fieldValve[iterator].fertigationStage = fieldValve[temp].fertigationStage;
                                            fieldValve[iterator].fertigationValveInterrupted = fieldValve[temp].fertigationValveInterrupted;
                                            myMsDelay(100);
                                            saveFertigationValveValuesIntoEeprom(eepromAddress[iterator], &fieldValve[iterator]);
                                            myMsDelay(100);
                                        }
                                    }
                                }
                                //iterator = temp;
                                /***************************/
                                // for field no. 01 to 09
                                if (fetchedPriority < 10) {
                                    temporaryBytesArray[0] = 48; // To store field no. of valve in action 
                                    temporaryBytesArray[1] = fetchedPriority + 48; // To store field no. of valve in action 
                                }// for field no. 10 to 12
                                else if (fetchedPriority >=10 && fetchedPriority <= 12) {
                                    temporaryBytesArray[0] = 49; // To store field no. of valve in action 
                                    temporaryBytesArray[1] = fetchedPriority + 38; // To store field no. of valve in action 
                                }
                                /***************************/
                                msgIndex = CLEAR;
                                /***************************/                       
                                sendSms(SmsFert3, userMobileNo, fieldNoRequired);  // Acknowledge user about successful Fertigation enabled action
                            #ifdef SMS_DELIVERY_REPORT_ON_H
                                sleepCount = 2; // Load sleep count for SMS transmission action
                                sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
                                setBCDdigit(0x05,0);
                                deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
                                setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
                            #endif
                                /***************************/
                                /***************************/
                            #ifdef DEBUG_MODE_ON_H
                                //********Debug log#start************//
                                transmitStringToDebug("extractReceivedSms_EnableFertigation_SmsFert3_OUT\r\n");
                                //********Debug log#end**************//
                            #endif
                                return;
                            }
                            break;
                        }
                    }
                }
            }
        }
        /***************************/
        // for field no. 01 to 09
        if (fetchedPriority < 10) {
            temporaryBytesArray[0] = 48; // To store field no. of valve in action 
            temporaryBytesArray[1] = fetchedPriority + 48; // To store field no. of valve in action 
        }// for field no. 10 to 12
        else if (fetchedPriority >=10 && fetchedPriority <= 12) {
            temporaryBytesArray[0] = 49; // To store field no. of valve in action 
            temporaryBytesArray[1] = fetchedPriority + 38; // To store field no. of valve in action 
        }
        /***************************/
        msgIndex = CLEAR;
        /***************************/                       
        sendSms(SmsFert1, userMobileNo, fieldNoRequired);  // Acknowledge user about successful Fertigation enabled action
    #ifdef SMS_DELIVERY_REPORT_ON_H
        sleepCount = 2; // Load sleep count for SMS transmission action
        sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
        setBCDdigit(0x05,0);
        deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
        setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
    #endif
        /***************************/
        /***************************/
    #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("extractReceivedSms_EnableFertigation_SmsFert1_OUT\r\n");
        //********Debug log#end**************//
    #endif
        return;  
    }
    //#7>..............Disable Field x fertigation.................//
    // DISABLE<x>
    else if (strncmp(decodedString, disable, 7) == 0) {
        temp = 255;
        onHold = false;
        fetchedPriority = fetchFieldNo(7) + 1;   // fetch priority and add 1 for 1-12 range
        // scan priority valve status
        for (iterator = 0; iterator < fieldCount; iterator++) {
            if (fieldValve[iterator].priority == fetchedPriority && fieldValve[iterator].isConfigured == true && fieldValve[iterator].isFertigationEnabled == true) {
                temp = iterator; // meaning temp!=255;
                if (fieldValve[iterator].status == ON && (fieldValve[iterator].fertigationStage == wetPeriod || fieldValve[iterator].fertigationStage == injectPeriod)) {
                    onHold = true;
                }
                break;
            }
        }
        if (temp == 255) { // none of the valve matching fetched priority
            /***************************/
            // for field no. 01 to 09
            if (fetchedPriority < 10) {
                temporaryBytesArray[0] = 48; // To store field no. of valve in action 
                temporaryBytesArray[1] = fetchedPriority + 48; // To store field no. of valve in action 
            }// for field no. 10 to 12
            else if (fetchedPriority >=10 && fetchedPriority <= 12) {
                temporaryBytesArray[0] = 49; // To store field no. of valve in action 
                temporaryBytesArray[1] = fetchedPriority + 38; // To store field no. of valve in action 
            }
            /***************************/
            msgIndex = CLEAR;
            /***************************/                       
            sendSms(SmsIrr3, userMobileNo, fieldNoRequired);  // Acknowledge user about successful Fertigation enabled action
        #ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
            setBCDdigit(0x05,0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
            setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
        #endif
            /***************************/
            /***************************/
        #ifdef DEBUG_MODE_ON_H
            //********Debug log#start************//
            transmitStringToDebug("extractReceivedSms_DisableFertigation_SmsIrr3_OUT\r\n");
            //********Debug log#end**************//
        #endif
            return;
        }
        else {
            //Hold configuration for valves having same priority
            for (iterator = 0; iterator < fieldCount ; iterator++) {															 
                if (fieldValve[iterator].priority == fetchedPriority) { // Fetch valves with same priority first
                    fieldValve[iterator].isFertigationEnabled = false; // configuration hold									   
                    saveFertigationValveValuesIntoEeprom(eepromAddress[iterator], &fieldValve[iterator]);
                    myMsDelay(100);
                }
            }							
            /***************************/
            // for field no. 01 to 09
            if (fetchedPriority < 10) {
                temporaryBytesArray[0] = 48; // To store field no. of valve in action 
                temporaryBytesArray[1] = fetchedPriority + 48; // To store field no. of valve in action 
            }// for field no. 10 to 12
            else if (fetchedPriority >=10 && fetchedPriority <= 12) {
                temporaryBytesArray[0] = 49; // To store field no. of valve in action 
                temporaryBytesArray[1] = fetchedPriority + 38; // To store field no. of valve in action 
            }
            /***************************/
            msgIndex = CLEAR;
            /***************************/
            sendSms(SmsFert4, userMobileNo, fieldNoRequired);   // Acknowledge user about successful Fertigation disabled action
        #ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
            setBCDdigit(0x05,0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
            setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
        #endif
            /***************************/ 
        #ifdef DEBUG_MODE_ON_H
            //********Debug log#start************//
            transmitStringToDebug("extractReceivedSms_DisableFertigation_SmsFert4_OUT\r\n");
            //********Debug log#end**************//
        #endif    
            return;
        }
    }																			 
    //#8>..............Activate Filtration.................//
    //Msg Format---------***ACTIVE<Delay1><Space><Delay2><Space><Delay3><Space><OnTime><Space><SeparationTime><Space>***------------//
    else if (strncmp(decodedString, active, 6) == 0) {
        digit = CLEAR;
        count = CLEAR; //count to extract Timestamp (total 6 attributes)                
        for (msgIndex = 6; count < 5 ; msgIndex++) {
            //is number
            if (isNumber(decodedString[msgIndex])) {
                if (decodedString[msgIndex + 1] == space) {
                    temp = decodedString[msgIndex] - 48;
                    digit = digit + temp;
                } 
                else {
                    temp = decodedString[msgIndex] - 48;
                    temp = temp * 10;
                    digit = digit * 10;
                    digit = digit + temp;
                }
            } 
            else {
                count++;
                switch (count) {
                case 1: // code to extract filtrationDelay1;
                    filtrationDelay1 = (unsigned char)digit;
                    digit = CLEAR;
                    break;
                case 2: // code to extract filtrationDelay2;
                    filtrationDelay2 = (unsigned char)digit;
                    digit = CLEAR;
                    break;
                case 3: // code to extract filtrationDelay3;
                    filtrationDelay3 = (unsigned char)digit;
                    digit = CLEAR;
                    break;
                case 4: // code to extract filtrationOnTime;
                    filtrationOnTime = (unsigned char)digit;
                    digit = CLEAR;
                    break;
                case 5: // code to filtrationSeperationTime
                    filtrationSeperationTime = (unsigned char)digit;
                    digit = CLEAR;
                    filtrationEnabled = true;
                    break;
                }
            }
        }
        myMsDelay(100);
        saveFiltrationSequenceData();
        myMsDelay(100);
        msgIndex = CLEAR;
        /***************************/
        sendSms(SmsFilt1, userMobileNo, noInfo);
    #ifdef SMS_DELIVERY_REPORT_ON_H
        sleepCount = 2; // Load sleep count for SMS transmission action
        sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
        setBCDdigit(0x05,0);
        deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
        setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
    #endif
        /***************************/ 
    #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("extractReceivedSms_Configure_OUT\r\n");
        //********Debug log#end**************//
    #endif
        return;
    }
	//#9>..............Disable filtration.................//
    // DACTIVE
    else if (strncmp(decodedString, dactive, 7) == 0) {
        filtrationEnabled = false; // filtration disabled
        myMsDelay(100);
        saveFiltrationSequenceData();
        myMsDelay(100);
        msgIndex = CLEAR;
        /***************************/
        sendSms(SmsFilt2, userMobileNo, noInfo);
    #ifdef SMS_DELIVERY_REPORT_ON_H
        sleepCount = 2; // Load sleep count for SMS transmission action
        sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
        setBCDdigit(0x05,0);
        deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
        setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
    #endif
        /***************************/ 
    #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("extractReceivedSms_Hold_OUT\r\n");
        //********Debug log#end**************//
    #endif
        return;
    }
    //#10>..............get filtration data.................//
    // FDATA
    else if (strncmp(decodedString, fdata, 5) == 0) {
        if (filtrationEnabled) {
            msgIndex = CLEAR;
            /***************************/
            sendSms(SmsFilt4, userMobileNo, filtrationData);
        #ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
            setBCDdigit(0x05,0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
            setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
        #endif
            /***************************/
        }
        else {
            msgIndex = CLEAR;
            /***************************/
            sendSms(SmsFilt3, userMobileNo, noInfo);
        #ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
            setBCDdigit(0x05,0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
            setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
        #endif
            /***************************/                    
        }
        /***************************/
    #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("extractReceivedSms_sendDiagnosticData_OUT\r\n");
        //********Debug log#end**************//
    #endif
        return;
    }					  
    //#11>..............Set RTC Time.................//
    //Msg Format---------***Feed<Space><DD><Space><MM><Space><YY><Space><Hr><Space><Min><Space><Sec>***------------//
    //SMS Location----------63---------<68>-------<71>-------<74>-------<77>-------<80>------<83>//

    else if (strncmp(decodedString, feed, 4) == 0) {
        digit = CLEAR;
        count = CLEAR; //count to extract Timestamp (total 6 attributes)                
        for (msgIndex = 5; count < 6 ; msgIndex+=3) {
            //is number
            if (isNumber(decodedString[msgIndex])) {
                temp = decodedString[msgIndex] - 48;
                digit = temp;
                digit = digit * 10;
                temp = decodedString[msgIndex+1] - 48;
                digit = digit + temp;
                count++;
                switch (count) {
                case 1: // code to extract DD;
                    currentDD = (unsigned char)digit;
                    digit = CLEAR;
                    break;
                case 2: // code to extract mm;
                    currentMM = (unsigned char)digit;
                    digit = CLEAR;
                    break;
                case 3: // code to extract YY;
                    currentYY  = (unsigned char)digit;
                    digit = CLEAR;
                    break;
                case 4: // code to extract Hour;
                    currentHour = (unsigned char)digit;
                    digit = CLEAR;
                    break;
                case 5: // code to extract Minutes;
                    currentMinutes = (unsigned char)digit;
                    digit = CLEAR;
                    break;
                case 6: // code to extract Seconds;
                    currentSeconds = (unsigned char)digit;
                    digit = CLEAR;
                    break;
                }
            }
            else {
                msgIndex = CLEAR;
                /***************************/
                sendSms(SmsT1, userMobileNo, noInfo);
            #ifdef SMS_DELIVERY_REPORT_ON_H
                sleepCount = 2; // Load sleep count for SMS transmission action
                sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
                setBCDdigit(0x05,0);
                deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
                setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
            #endif
                /***************************/ 
            #ifdef DEBUG_MODE_ON_H
                //********Debug log#start************//
                transmitStringToDebug("extractReceivedSms_Configure_OUT\r\n");
                //********Debug log#end**************//
            #endif
                return;
            }				  
        }
        if (count == 6) {
            feedTimeInRTC();
        }				
        msgIndex = CLEAR;           
        /***************************/
        sendSms(SmsRTC2, userMobileNo, noInfo);
    #ifdef SMS_DELIVERY_REPORT_ON_H
        sleepCount = 2; // Load sleep count for SMS transmission action
        sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
        setBCDdigit(0x05,0);
        deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
        setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
    #endif
        /***************************/ 
    #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("extractReceivedSms_Configure_OUT\r\n");
        //********Debug log#end**************//
    #endif
        return;
    }
    //#12>..............Get RTC Time.................//
    // Get current Time
    else if (strncmp(decodedString, time, 4) == 0) {
        fetchTimefromRTC(); // Get today's date
        temporaryBytesArray[0] = (currentDD / 10) + 48;
        temporaryBytesArray[1] = (currentDD % 10) + 48;
        temporaryBytesArray[2] = '/';
        temporaryBytesArray[3] = (currentMM / 10) + 48;
        temporaryBytesArray[4] = (currentMM % 10) + 48;
        temporaryBytesArray[5] = '/';
        temporaryBytesArray[6] = (currentYY / 10) + 48;
        temporaryBytesArray[7] = (currentYY % 10) + 48;
        temporaryBytesArray[8] = ' ';
        temporaryBytesArray[9] = (currentHour / 10) + 48;
        temporaryBytesArray[10] = (currentHour % 10) + 48;
        temporaryBytesArray[11] = ':';
        temporaryBytesArray[12] = (currentMinutes / 10) + 48;
        temporaryBytesArray[13] = (currentMinutes % 10) + 48;
        temporaryBytesArray[14] = ':';
        temporaryBytesArray[15] = (currentSeconds / 10) + 48;
        temporaryBytesArray[16] = (currentSeconds % 10) + 48;
        msgIndex = CLEAR;
        /***************************/
        sendSms(SmsT3, userMobileNo, timeRequired);
    #ifdef SMS_DELIVERY_REPORT_ON_H
        sleepCount = 2; // Load sleep count for SMS transmission action
        sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
        setBCDdigit(0x05,0);
        deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
        setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
    #endif
        /***************************/
    #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("extractReceivedSms_Current Time_OUT\r\n");
        //********Debug log#end**************//
    #endif
        return;
    }
	//#13>.............. Extract configured Data for Self Diagnostic.................//
    // extract<x>
    else if (strncmp(decodedString, extract, 7) == 0) {
        iterator = fetchFieldNo(7);
        /***************************/
        // for field no. 01 to 09
        if (iterator<9){
            temporaryBytesArray[0] = 48; // To store field no. of valve in action 
            temporaryBytesArray[1] = iterator + 49; // To store field no. of valve in action 
        }// for field no. 10 to 12
        else if (iterator > 8 && iterator < 12) {
            temporaryBytesArray[0] = 49; // To store field no. of valve in action 
            temporaryBytesArray[1] = iterator + 39; // To store field no. of valve in action 
        }
        /***************************/
        msgIndex = CLEAR;
        if (fieldValve[iterator].isConfigured) {
            /***************************/ 
            sendSms(SmsIrr7, userMobileNo, IrrigationData);  // Give diagnostic data
        #ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
            setBCDdigit(0x05,0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
            setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
        #endif
            /***************************/
        }
        else {
            /***************************/                        
            sendSms(SmsIrr3, userMobileNo, fieldNoRequired);  // Acknowledge user about  Irrigation not configured
        #ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
            setBCDdigit(0x05,0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
            setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
        #endif
            /***************************/
        }
    #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("extractReceivedSms_sendDiagnosticData_OUT\r\n");
        //********Debug log#end**************//
    #endif
        return;
    }																							   
    //#14>......Set up Motor load condition Manually .......//
    // CT<Space><No Load Cutoff>Space<Full Load Cutoff><Space>
    else if (strncmp(decodedString, ct, 2) == 0) {
        if(MotorControl == ON) {
            msgIndex = CLEAR;
            /***************************/
            sendSms(SmsMotor4, userMobileNo, noInfo);   // Acknowledge user about Irrigation is active, Motor load cut-off procedure not started
        #ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
            setBCDdigit(0x05,0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
            setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
        #endif
            /***************************/
        #ifdef DEBUG_MODE_ON_H
            //********Debug log#start************//
            transmitStringToDebug("Motor load values set successfully_OUT\r\n");
            //********Debug log#end**************//
        #endif
            return;
        }
        digit = CLEAR;
        count = CLEAR; //count to extract motor load condition (total 2 attributes)
        for (count = 1, msgIndex = 3; count <= 2 ; msgIndex++) {
            //is number
            if (isNumber(decodedString[msgIndex])) {
                if (decodedString[msgIndex + 1] == space) {
                    temp = decodedString[msgIndex] - 48;
                    digit = digit + temp;
                } 
                else {
                    temp = decodedString[msgIndex] - 48;
                    temp = temp * 10;
                    digit = digit * 10;
                    digit = digit + temp;
                }
            } 
            else {
                switch (count) {
                case 1: // code to extract on period;
                    noLoadCutOff = digit;
                    digit = CLEAR;
                    break;
                case 2: // code to extract off period;
                    fullLoadCutOff = digit;
                    digit = CLEAR;
                    break;
                }
                count++;
            }
        }
        myMsDelay(100);
        saveMotorLoadValuesIntoEeprom();
        myMsDelay(100);
        msgIndex = CLEAR;
        /***************************/
        sendSms(SmsMotor2, userMobileNo, noInfo);
    #ifdef SMS_DELIVERY_REPORT_ON_H
        sleepCount = 2; // Load sleep count for SMS transmission action
        sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
        setBCDdigit(0x05,0);
        deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
        setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
    #endif
        /***************************/
    #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("Motor load values set successfully_OUT\r\n");
        //********Debug log#end**************//
    #endif
        return;
    }
	//#15>......GetCTValues for fetching Motor load values.......//
    else if (strncmp(decodedString, getct, 5) == 0) {
        msgIndex = CLEAR;
        /***************************/
        sendSms(SmsMotor3, userMobileNo, motorLoadRequired);
    #ifdef SMS_DELIVERY_REPORT_ON_H
        sleepCount = 2; // Load sleep count for SMS transmission action
        sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission	
        setBCDdigit(0x05,0);
        deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
        setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
    #endif
        /***************************/
    #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("extractReceivedSMS_Get CT Values_OUT\r\n");
        //********Debug log#end**************//
    #endif
        return;
    }
    //#16>......Get FREQUENCY Values for field moisture sensors.......//
    else if (strncmp(decodedString, getfreq, 7) == 0) {
        iterator = fetchFieldNo(7);
        myMsDelay(100);
        isFieldMoistureSensorWet(iterator);
        /***************************/
        // for field no. 01 to 09
        if (iterator<9){
            temporaryBytesArray[0] = 48; // To store field no. of valve in action 
            temporaryBytesArray[1] = iterator + 49; // To store field no. of valve in action 
        }// for field no. 10 to 12
        else if (iterator > 8 && iterator < 12) {
            temporaryBytesArray[0] = 49; // To store field no. of valve in action 
            temporaryBytesArray[1] = iterator + 39; // To store field no. of valve in action 
        }
        /***************************/
        if(moistureSensorFailed) {
            moistureSensorFailed = false;
            /***************************/
            sendSms(SmsMS3, userMobileNo, fieldNoRequired);
        #ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
            setBCDdigit(0x05,0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
            setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
        #endif
            /***************************/
        }
        else {
            /***************************/
            msgIndex = CLEAR;
            /***************************/                        
            sendSms(SmsMS2, userMobileNo, frequencyRequired);
        #ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
            setBCDdigit(0x05,0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
            setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
        #endif
            /***************************/

        }

    #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("extractReceivedSms_sendMoistureSensorData_OUT\r\n");
        //********Debug log#end**************//
    #endif
        return;
    }
    //#17>......Set up Motor load condition Automatically.......//
    // SETCT<FieldNno.>
    else if (strncmp(decodedString, setct, 3) == 0) {
        iterator = fetchFieldNo(3);
        myMsDelay(100);
        if(MotorControl == ON) {
            msgIndex = CLEAR;
            /***************************/
            sendSms(SmsMotor4, userMobileNo, noInfo);  // Acknowledge user about Irrigation is active, Motor load cut-off procedure not started
        #ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
            setBCDdigit(0x05,0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
            setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
        #endif
            /***************************/
        #ifdef DEBUG_MODE_ON_H
            //********Debug log#start************//
            transmitStringToDebug("Motor load values set successfully_OUT\r\n");
            //********Debug log#end**************//
        #endif
            return;
        }
        /***************************/
        // for field no. 01 to 09
        if (iterator<9){
            temporaryBytesArray[0] = 48; // To store field no. of valve in action 
            temporaryBytesArray[1] = iterator + 49; // To store field no. of valve in action 
        }// for field no. 10 to 12
        else if (iterator > 8 && iterator < 12) {
            temporaryBytesArray[0] = 49; // To store field no. of valve in action 
            temporaryBytesArray[1] = iterator + 39; // To store field no. of valve in action 
        }
        myMsDelay(5000);
        //calibrateMotorCurrent(NoLoad, iterator);
        //myMsDelay(1000);
        calibrateMotorCurrent(FullLoad, iterator);
        myMsDelay(1000);
        saveMotorLoadValuesIntoEeprom();
        myMsDelay(1000);                    
        msgIndex = CLEAR;
        /***************************/
        sendSms(SmsMotor3, userMobileNo, motorLoadRequired);
    #ifdef SMS_DELIVERY_REPORT_ON_H
        sleepCount = 2; // Load sleep count for SMS transmission action
        sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
        setBCDdigit(0x05,0);
        deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
        setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
    #endif
        /***************************/
    #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("Motor load values set successfully_OUT\r\n");
        //********Debug log#end**************//
    #endif
        return;
    }																		   
	//#xx>......InjectTestData.......//
    // INJECT<Space><HR><Space><MIN><Space><trigger>
    else if (strncmp(decodedString, inject, 6) == 0) {
        digit = CLEAR;
        count = CLEAR; //count to extract Timestamp (total 6 attributes)                
        for (msgIndex = 7; count < 3 ; msgIndex+=3) {
            //is number
            if (isNumber(decodedString[msgIndex])) {
                temp = decodedString[msgIndex] - 48;
                digit = temp;
                digit = digit * 10;
                temp = decodedString[msgIndex+1] - 48;
                digit = digit + temp;
                count++;
                switch (count) {
                case 1: // code to extract motorOnTimeHour;
                    onHour = (unsigned char)digit;
                    digit = CLEAR;
                    break;
                case 2: // code to extract motorOnTimeMinute;
                    onMinute = (unsigned char)digit;
                    digit = CLEAR;
                    break;
                case 3: // code to extract day;
                    getDueDate((unsigned char)digit); // Get due dates w.r.t today's date
                    digit = CLEAR;
                    break;
                }
            }
        }
        for (iterator = 0; iterator < fieldCount; iterator++) {
            fieldValve[iterator].onPeriod = 15;
            fieldValve[iterator].offPeriod = 1;
            fieldValve[iterator].motorOnTimeHour = onHour;
            fieldValve[iterator].motorOnTimeMinute = onMinute;
            fieldValve[iterator].nextDueDD = (unsigned char)dueDD;
            fieldValve[iterator].nextDueMM = dueMM;
            fieldValve[iterator].nextDueYY = dueYY;
            fieldValve[iterator].dryValue = 100;
            fieldValve[iterator].wetValue = 30000;
            fieldValve[iterator].priority = iterator+1;
            fieldValve[iterator].status = OFF;
            fieldValve[iterator].cycles = 2;
            fieldValve[iterator].cyclesExecuted = 2;
            fieldValve[iterator].isConfigured = true;                
            fieldValve[iterator].fertigationDelay = 5;
            fieldValve[iterator].fertigationONperiod = 15;
            fieldValve[iterator].fertigationInstance = iterator+1;
            fieldValve[iterator].fertigationStage = OFF;
            fieldValve[iterator].fertigationValveInterrupted = false;
            fieldValve[iterator].isFertigationEnabled = true;

            saveIrrigationValveValuesIntoEeprom(eepromAddress[iterator], &fieldValve[iterator]);
            myMsDelay(100);
            saveIrrigationValveDueTimeIntoEeprom(eepromAddress[iterator], &fieldValve[iterator]);
            myMsDelay(100);
            saveIrrigationValveOnOffStatusIntoEeprom(eepromAddress[iterator], &fieldValve[iterator]);
            myMsDelay(100);
            saveIrrigationValveCycleStatusIntoEeprom(eepromAddress[iterator], &fieldValve[iterator]);
            myMsDelay(100);
            saveIrrigationValveConfigurationStatusIntoEeprom(eepromAddress[iterator], &fieldValve[iterator]);
            myMsDelay(100);
            saveFertigationValveValuesIntoEeprom(eepromAddress[iterator], &fieldValve[iterator]);
            myMsDelay(100);
        }      
        msgIndex = CLEAR;
        /***************************/
        sendSms(SmsTest, userMobileNo, noInfo);
    #ifdef SMS_DELIVERY_REPORT_ON_H
        sleepCount = 2; // Load sleep count for SMS transmission action
        sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
        setBCDdigit(0x05,0);
        deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
        setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
    #endif
        /***************************/
    #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("InjectTestData_Admin set successfully_OUT\r\n");
        //********Debug log#end**************//
    #endif
        return;
    }											   
    //#1>......Register New Admin.......//
    // set admin format (AU 123456)
    else if (strncmp(decodedString, admin, 2) == 0) {
        if (strncmp(decodedString + 3, pwd, 6) == 0) {
            // if not first time installation
            if (systemAuthenticated) {
                msgIndex = CLEAR;
                /***************************/
                sendSms(SmsAU2, userMobileNo, newAdmin); //To notify old Admin about new Admin.
            #ifdef SMS_DELIVERY_REPORT_ON_H
                sleepCount = 2; // Load sleep count for SMS transmission action
                sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
                setBCDdigit(0x05,0);
                deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
                setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
            #endif
                /***************************/
            }
            strncpy(pwd, decodedString + 10, 6);
            strncpy(userMobileNo, temporaryBytesArray, 10);
            systemAuthenticated = true;
            strncpy(temporaryBytesArray, null, 10);
            myMsDelay(50);
            savePasswordIntoEeprom();
            myMsDelay(50);
            saveMobileNoIntoEeprom();
            myMsDelay(50);
            saveAuthenticationStatus();
            myMsDelay(50);
            msgIndex = CLEAR;
            /***************************/
            sendSms(SmsAU1, userMobileNo, noInfo);  // Acknowledge user about successful Admin Registration
        #ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
            setBCDdigit(0x05,0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
            setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
        #endif
            /***************************/
        #ifdef DEBUG_MODE_ON_H
            //********Debug log#start************//
            transmitStringToDebug("extractReceivedSms_Admin set successfully_OUT\r\n");
            //********Debug log#end**************//
        #endif
            return;
        }
        else {
            msgIndex = CLEAR;
            /***************************/
            sendSms(SmsPwd3, temporaryBytesArray, noInfo);
        #ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
            setBCDdigit(0x05,0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
            setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
        #endif
            /***************************/
        #ifdef DEBUG_MODE_ON_H
            //********Debug log#start************//
            transmitStringToDebug("extractReceivedSms_Wrong Password_OUT\r\n");
            //********Debug log#end**************//
        #endif
            strncpy(temporaryBytesArray, null, 10);
            return;
        }
    }
    //#0>......Connect from non admin.......//
    else if (strncmp(decodedString, hook, 4) == 0) {   
        msgIndex = CLEAR;
        /***************************/
        sendSms(SmsAU3, temporaryBytesArray, noInfo);   // Acknowledge user about Authentication failed
    #ifdef SMS_DELIVERY_REPORT_ON_H
        sleepCount = 2; // Load sleep count for SMS transmission action
        sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission	
        setBCDdigit(0x05,0);
        deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
        setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
    #endif
        /***************************/
    #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("extractReceivedSms_Not Authenticated_OUT\r\n");
        //********Debug log#end**************//
    #endif
        strncpy(temporaryBytesArray, null, 10);
        return;
    }
    //#-1>......Secret code for fetching Factory Password.......//
    else if (strncmp(decodedString, secret, 11) == 0) {
        msgIndex = CLEAR;
        /***************************/
        sendSms(SmsKey1, temporaryBytesArray, secretCodeRequired);
    #ifdef SMS_DELIVERY_REPORT_ON_H
        sleepCount = 2; // Load sleep count for SMS transmission action
        sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission	
        setBCDdigit(0x05,0);
        deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
        setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
    #endif
        /***************************/
    #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("extractReceivedSms_Factory Password_OUT\r\n");
        //********Debug log#end**************//
    #endif
        strncpy(temporaryBytesArray, null, 10);
        return;
    }
    //#-2>......Secret code for fetching currentPassword.......//
    else if (strncmp(decodedString, secret1, 11) == 0) {
        msgIndex = CLEAR;
        /***************************/
        sendSms(SmsKey2, temporaryBytesArray, secretCode1Required);
    #ifdef SMS_DELIVERY_REPORT_ON_H
        sleepCount = 2; // Load sleep count for SMS transmission action
        sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission	
        setBCDdigit(0x05,0);
        deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
        setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
    #endif
        /***************************/
    #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("extractReceivedSms_Current Password_OUT\r\n");
        //********Debug log#end**************//
    #endif
        strncpy(temporaryBytesArray, null, 10);
        return;
    }
    else {
        setBCDdigit(0x06,0);  // (6.) BCD indication for unknown sms from unregistered user
        myMsDelay(1000);
        /***************************/ 
    #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("extractReceivedSms_UnknownSMS_fromRegisteredUser_OUT\r\n");
        //********Debug log#end**************//
    #endif
        return;
    }													
}
/************************Actions on Received SMS#End************************************/


/*********** Moisture sensor measurement#Start********/

/*************************************************************************************************************************

This function is called to measure soil moisture of given field and indicate if wet field found .
The Moisture level is measured in terms of frequency of square wave generated by IC555 based on Senor resistance.
The Sensor resistance is high and low for Dry and Wet condition respectively.
This leads the output of IC555 with high and low pulse width.
For Dry condition pulse width is high and for wet condition pulse width is low.
i.e. for Dry condition pulse occurrence is low and for wet condition pulse occurrence is high
Here Timer1 is used to count frequency of pulses by measuring timer count for 1 pulse width and averaging it for 10 pulses.

 **************************************************************************************************************************/
_Bool isFieldMoistureSensorWet(unsigned char FieldNo) {
    unsigned long moistureLevelAvg = CLEAR;
    unsigned long timer1Value = CLEAR; // To store 16 bit SFR Timer1 Register value
    unsigned long constant = 160000; // Constant to calculate frequency in Hz ~16MHz 16000000/100 to convert freq from 5 digit to 3 digit
    unsigned char itr = CLEAR, avg = 20;
    moistureLevel = CLEAR; // To store moisture level in Hz
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    /***************************/
    // for field no. 01 to 09
    if (FieldNo<9){
        temporaryBytesArray[0] = 48; // To store field no. of valve in action 
        temporaryBytesArray[1] = FieldNo + 49; // To store field no. of valve in action 
    }// for field no. 10 to 12
    else if (FieldNo > 8 && FieldNo < 12) {
        temporaryBytesArray[0] = 49; // To store field no. of valve in action 
        temporaryBytesArray[1] = FieldNo + 39; // To store field no. of valve in action 
    }
    else {
        temporaryBytesArray[0] = 57; // To store field no. of valve in action 
        temporaryBytesArray[1] = 57; // To store field no. of valve in action 
    
    }
    /***************************/
    transmitStringToDebug("isFieldMoistureSensorWet_IN : ");
    transmitNumberToDebug(temporaryBytesArray, 2);
    transmitStringToDebug("\r\n");
    //********Debug log#end**************//
#endif
    setBCDdigit(0x09,0); // (9.) BCD indication for Moisture Sensor Failure Error
    moistureLevel = LOW;
    checkMoistureSensor = true;
    moistureSensorFailed = false;
    timer3Count = 5; // 5 second window
    // Averaging measured pulse width
    for (itr = 1; itr <= avg && !moistureSensorFailed; itr++) {
        T1CONbits.TMR1ON = OFF;
        TMR1H = CLEAR;
        TMR1L = CLEAR;
        Timer1Overflow = CLEAR;
        // check field moisture of valve in action
    #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("MoistureSensor1 == HIGH\r\n");
        //********Debug log#end**************//
    #endif
        T3CONbits.TMR3ON = ON; // Start timer thread to unlock system if Sensor fails to respond within 15 sec   
        controllerCommandExecuted = false;
        /*******Sensor 1) is selected as common sensor for all fields*******/
        while (MoistureSensor1 == HIGH && controllerCommandExecuted == false);
    #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("MoistureSensor1 == LOW\r\n");
        //********Debug log#end**************//
    #endif
        while (MoistureSensor1 == LOW && controllerCommandExecuted == false); // Wait for rising edge
        T1CONbits.TMR1ON = ON; // Start Timer after rising edge detected
        while (MoistureSensor1 == HIGH && controllerCommandExecuted == false); // Wait for falling edge
        if (!controllerCommandExecuted) {
            controllerCommandExecuted = true;
            PIR5bits.TMR3IF = SET; //Stop timer thread 
        }
        T1CONbits.TMR1ON = OFF; // Stop timer after falling edge detected
        timer1Value = TMR1L;    // Store lower byte of 16 bit timer
        timer1Value|=((unsigned int)TMR1H) << 8;    // Get higher and lower byte of  timer1 register
        moistureLevelAvg = (Timer1Overflow * 65536) + timer1Value;
        moistureLevelAvg *= 2; // Entire cycle width
        moistureLevelAvg = (constant / moistureLevelAvg); // Frequency = 16Mhz/ Pulse Width
        if (itr == 1) {
            moistureLevel = (unsigned int)moistureLevelAvg;
        }
        moistureLevel = moistureLevel/2;
        moistureLevelAvg = moistureLevelAvg/2;
        moistureLevel = moistureLevel + (unsigned int)moistureLevelAvg;
        if(moistureSensorFailed) {
            moistureLevel = 0;
        }
    }
    checkMoistureSensor = false;
    setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
    //For Fertigation sensor 12
    //if (FieldNo == 11) {
    //    if (moistureLevel >= 150) {
    //    #ifdef DEBUG_MODE_ON_H
            //********Debug log#start************//
        //    transmitStringToDebug("isFertigationSensorWet_Yes_Out\r\n");
            //********Debug log#end**************//
        //#endif
        //    return true;
        //}
        //else {
        //#ifdef DEBUG_MODE_ON_H
            //********Debug log#start************//
        //    transmitStringToDebug("isFertigationSensorWet_No_Out\r\n");
            //********Debug log#end**************//
        //#endif
        //    return false;            
        //}
    //}
    if (moistureLevel >= fieldValve[FieldNo].wetValue) { //Field is full wet, no need to switch ON valve and motor, estimate new due dates
    #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("isFieldMoistureSensorWet_Yes_Out\r\n");
        //********Debug log#end**************//
    #endif
        return true;
    } 
    else {
    #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("isFieldMoistureSensorWet_No_Out\r\n");
        //********Debug log#end**************//
    #endif
        return false;
    }
}

/*********** Motor Dry run condition#Start********/

/*************************************************************************************************************************

This function is called to measure motor phase current to detect dry run condition.
The Dry run condition is measured in terms of voltage of CT connected to one of the phase of motor.
The motor phase current is high  and low  for Wet (load) and Dry (no load) condition respectively.
This leads the output of CT with high and low voltage.
For Dry condition, the CT voltage is high and for wet condition, the CT voltage is low.
Here ADC module is used to measure high/ low voltage of CT thereby identifying load/no load condition.

 **************************************************************************************************************************/

_Bool isMotorInNoLoad(void) {
    unsigned int ctOutput = 0;
    unsigned int temp = 0;
    lowPhaseCurrentDetected = false;
    dryRunDetected = false;
    temp = (fullLoadCutOff)/10;
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("isMotorInNoLoad_IN\r\n");
    //********Debug log#end**************//
#endif
    // Averaging measured pulse width
    selectChannel(CTchannel);
    ctOutput = getADCResult();
    if (ctOutput > temp && ctOutput <= noLoadCutOff) {
        dryRunDetected = true; //Set Low water level
    #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("isMotorInNoLoad_Dry_Yes_OUT\r\n");
        //********Debug log#end**************//
    #endif
        return true;
    }
    else if (ctOutput == 0 || (ctOutput > 0 && ctOutput <= temp)) {  // no phase current
        lowPhaseCurrentDetected = true; //Set phase current low
    #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("isMotorInNoLoad_LowPhase_Yes_OUT\r\n");
        //********Debug log#end**************//
    #endif
        return true;
    }
    else {
        lowPhaseCurrentDetected = false; 
        dryRunDetected = false; //Set High water level
    #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("isMotorInNoLoad_Dry_LowPhase_No_OUT\r\n");
        //********Debug log#end**************//
    #endif
        return false;
    }
}

/*********** Motor Dry run condition#End********/

/*********** Motor current calibration#Start********/

/*************************************************************************************************************************

This function is called to calibrate motor phase current to set no load, overload condition.
The Dry run condition is measured in terms of voltage of CT connected to one of the phase of motor.
The motor phase current is high  and low  for Wet (load) and Dry (no load) condition respectively.
This leads the output of CT with high and low voltage.
For Dry condition, the CT voltage is high and for wet condition, the CT voltage is low.
Here ADC module is used to measure high/ low voltage of CT thereby identifying load/no load condition.

 **************************************************************************************************************************/

void calibrateMotorCurrent(unsigned char loadType, unsigned char FieldNo) {
    unsigned int ctOutput = 0;
    unsigned char itr = 0, limit = 30;
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("calibrateMotorCurrent_IN\r\n");
    //********Debug log#end**************//
#endif
    if(loadType == FullLoad) {
        switch (FieldNo) {
            myMsDelay(1000);
        case 0:
            field1ValveControl = ON; // switch on valve for field 1
            break;
        case 1:
            field2ValveControl = ON; // switch off valve for field 2
            break;
        case 2:
            field3ValveControl = ON; // switch on valve for field 3
            break;
        case 3:
            field4ValveControl = ON; // switch on valve for field 4
            break;    
        case 4:
            field5ValveControl = ON; // switch off valve for field 5
            break;
        case 5:
            field6ValveControl = ON; // switch off valve for field 6
            break;
        case 6:
            field7ValveControl = ON; // switch on valve for field 7
            break;
        case 7:
            field8ValveControl = ON; // switch on valve for field 8
            break;
        case 8:
            field9ValveControl = ON; // switch on valve for field 9
            break;
        case 9:
            field10ValveControl = ON; // switch on valve for field 10
            break;
        case 10:
            field11ValveControl = ON; // switch on valve for field 11
            break;
        case 11:
            field12ValveControl = ON; // switch on valve for field 12
            break;
        }
    }
    if(MotorControl != ON) {
        myMsDelay(5000);
        MotorControl = ON;
        myMsDelay(100);
    #ifdef STAR_DELTA_DEFINITIONS_H
        myMsDelay(500);
        motorPowerTorque = ON;
        myMsDelay(900);
        motorPowerTorque = OFF;
#endif
    }
    myMsDelay(5000);
    // Averaging measured pulse width
    setBCDdigit(0x0F,1); // BCD Indication for Flash
    myMsDelay(5000);
    selectChannel(CTchannel);
    if (loadType == NoLoad) {
        limit = 11;           //~1.5 min
    }
    for (itr = 0; itr < limit ; itr++) {
        ctOutput = getADCResult();
        myMsDelay(2500);
        lower8bits = ctOutput;
        temporaryBytesArray[0] = (unsigned char) ((lower8bits / 1000) + 48);
        setBCDdigit(temporaryBytesArray[0], 1);
        myMsDelay(1000);
        setBCDdigit(0x0F, 1);
        myMsDelay(500);
        lower8bits = lower8bits % 1000;
        temporaryBytesArray[0] = (unsigned char) ((lower8bits / 100) + 48);
        setBCDdigit(temporaryBytesArray[0], 1);
        myMsDelay(1000);
        setBCDdigit(0x0F, 1);
        myMsDelay(500);
        lower8bits = lower8bits % 100;
        temporaryBytesArray[0] = (unsigned char) ((lower8bits / 10) + 48);
        setBCDdigit(temporaryBytesArray[0], 1);
        myMsDelay(1000);
        setBCDdigit(0x0F, 1);
        myMsDelay(500);
        lower8bits = lower8bits % 10;
        temporaryBytesArray[0] = (unsigned char) (lower8bits + 48);
        setBCDdigit(temporaryBytesArray[0], 1);
        myMsDelay(1000);
        setBCDdigit(0x0F, 1);
        if (loadType == FullLoad) {
            fullLoadCutOff = ctOutput;
            noLoadCutOff = (7*fullLoadCutOff)/10;
        }
        else if (loadType == NoLoad) {
            noLoadCutOff = ctOutput;
        } 
    }
    if(loadType == FullLoad) {
        MotorControl = OFF;
    }
    if(loadType == FullLoad) {
        myMsDelay(1000);
        switch (FieldNo) {
        case 0:
            field1ValveControl = OFF; // switch on valve for field 1
            break;
        case 1:
            field2ValveControl = OFF; // switch off valve for field 2
            break;
        case 2:
            field3ValveControl = OFF; // switch on valve for field 3
            break;
        case 3:
            field4ValveControl = OFF; // switch on valve for field 4
            break;    
        case 4:
            field5ValveControl = OFF; // switch off valve for field 5
            break;
        case 5:
            field6ValveControl = OFF; // switch off valve for field 6
            break;
        case 6:
            field7ValveControl = OFF; // switch on valve for field 7
            break;
        case 7:
            field8ValveControl = OFF; // switch on valve for field 8
            break;
        case 8:
            field9ValveControl = OFF; // switch on valve for field 9
            break;
        case 9:
            field10ValveControl = OFF; // switch on valve for field 10
            break;
        case 10:
            field11ValveControl = OFF; // switch on valve for field 11
            break;
        case 11:
            field12ValveControl = OFF; // switch on valve for field 12
            break;
        }
    }
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("calibrateMotorCurrent_OUT\r\n");
    //********Debug log#end**************//
#endif
}

/*********** Motor current calibration#End********/

/*********** DRY RUN Action#Start********/

/*************************************************************************************************************************

This function is called to perform actions after detecting dry run condition.
After detecting Dry run condition, stop all active valves and set all valves due from today to tomorrow.
Notify user about all actions

**************************************************************************************************************************/

void doDryRunAction(void) {
    unsigned char field_No = CLEAR;
	unsigned int sleepCountVar = CLEAR;								   
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("doDryRunAction_IN\r\n");
    //********Debug log#end**************//
#endif
    myMsDelay(100);
    fetchTimefromRTC(); // Get today's date
    myMsDelay(100);
	getDueDate(1); // calculate next day date										 
    for (field_No = 0; field_No < fieldCount; field_No++) {
        if (fieldValve[field_No].status == ON) {
            myMsDelay(100);
            powerOffMotor();
            myMsDelay(1000);
            deActivateValve(field_No);   // Deactivate Valve upon Dry run condition and reset valve to next due time
            valveDue = false;
            fieldValve[field_No].status = OFF;
            fieldValve[field_No].cyclesExecuted = fieldValve[field_No].cycles;
            myMsDelay(100);
            saveIrrigationValveOnOffStatusIntoEeprom(eepromAddress[field_No], &fieldValve[field_No]);
            myMsDelay(100);
            saveIrrigationValveCycleStatusIntoEeprom(eepromAddress[field_No], &fieldValve[field_No]);
            myMsDelay(100);
			sleepCountVar = readActiveSleepCountFromEeprom();												 
			if (fieldValve[field_No].isFertigationEnabled) {												
				/************Fertigation switch off due to dry run***********/
				if (fieldValve[field_No].fertigationStage == injectPeriod) {
					fertigationValveControl = OFF; // Switch off fertigation valve in case it is ON
                    if (!fieldValve[8].isConfigured && !fieldValve[9].isConfigured && !fieldValve[10].isConfigured && !fieldValve[11].isConfigured) { // 9-12 used as injector
                        //Switch off all Injectors after completing fertigation on Period
                        injector1Control = OFF;
                        injector2Control = OFF;
                        injector3Control = OFF;
                        injector4Control = OFF;
                    }
					fieldValve[field_No].fertigationStage = OFF;
					fieldValve[field_No].fertigationValveInterrupted = true;
					remainingFertigationOnPeriod = readActiveSleepCountFromEeprom();
					myMsDelay(100);
					saveRemainingFertigationOnPeriod();
					myMsDelay(100);
					saveFertigationValveStatusIntoEeprom(eepromAddress[field_No], &fieldValve[field_No]);
					myMsDelay(100);
					/******** Calculate and save Field Valve next date**********/
					fieldValve[field_No].nextDueDD = (unsigned char)dueDD;
					fieldValve[field_No].nextDueMM = dueMM;
					fieldValve[field_No].nextDueYY = dueYY;
					myMsDelay(100);
					saveIrrigationValveDueTimeIntoEeprom(eepromAddress[field_No], &fieldValve[field_No]);
					myMsDelay(100);
					/***********************************************/						 
					/***************************/
					// for field no. 01 to 09
					if (field_No<9){
						temporaryBytesArray[0] = 48; // To store field no. of valve in action 
						temporaryBytesArray[1] = field_No + 49; // To store field no. of valve in action 
					}// for field no. 10 to 12
					else if (field_No > 8 && field_No < 12) {
						temporaryBytesArray[0] = 49; // To store field no. of valve in action 
						temporaryBytesArray[1] = field_No + 39; // To store field no. of valve in action 
					}
					/***************************/

					/***************************/
					sendSms(SmsDR1, userMobileNo, fieldNoRequired); // Acknowledge user about dry run detected and action taken
					#ifdef SMS_DELIVERY_REPORT_ON_H
					sleepCount = 2; // Load sleep count for SMS transmission action
					sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
					setBCDdigit(0x05,0);
					deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
					setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
					#endif
					/***************************/
				}
				else if (fieldValve[field_No].fertigationStage == wetPeriod) {
					/******** Calculate and save Field Valve next date**********/
                    fieldValve[field_No].nextDueDD = (unsigned char)dueDD;
                    fieldValve[field_No].nextDueMM = dueMM;
                    fieldValve[field_No].nextDueYY = dueYY;
                    myMsDelay(100);
                    saveIrrigationValveDueTimeIntoEeprom(eepromAddress[field_No], &fieldValve[field_No]);
                    myMsDelay(100);
                    /***********************************************/
                    /***************************/
                    // for field no. 01 to 09
                    if (field_No<9){
                        temporaryBytesArray[0] = 48; // To store field no. of valve in action 
                        temporaryBytesArray[1] = field_No + 49; // To store field no. of valve in action 
                    }// for field no. 10 to 12
                    else if (field_No > 8 && field_No < 12) {
                        temporaryBytesArray[0] = 49; // To store field no. of valve in action 
                        temporaryBytesArray[1] = field_No + 39; // To store field no. of valve in action 
                    }
                    /***************************/

                    /***************************/
                    sendSms(SmsDR2, userMobileNo, fieldNoRequired); // Acknowledge user about dry run detected and action taken
                #ifdef SMS_DELIVERY_REPORT_ON_H
                    sleepCount = 2; // Load sleep count for SMS transmission action
                    sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
                    setBCDdigit(0x05,0);
                    deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
                    setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
                #endif
                    /***************************/
				}
			}
			else {
				if (sleepCountVar > (fieldValve[field_No].onPeriod/2)) { // major part of valve execution is pending hence shift to next day
                    /******** Calculate and save Field Valve next date**********/
                    fieldValve[field_No].nextDueDD = (unsigned char)dueDD;
                    fieldValve[field_No].nextDueMM = dueMM;
                    fieldValve[field_No].nextDueYY = dueYY;
                    myMsDelay(100);
                    saveIrrigationValveDueTimeIntoEeprom(eepromAddress[field_No], &fieldValve[field_No]);
                    myMsDelay(100);
                    /***********************************************/ 
                    /***************************/
                    // for field no. 01 to 09
                    if (field_No<9){
                        temporaryBytesArray[0] = 48; // To store field no. of valve in action 
                        temporaryBytesArray[1] = field_No + 49; // To store field no. of valve in action 
                    }// for field no. 10 to 12
                    else if (field_No > 8 && field_No < 12) {
                        temporaryBytesArray[0] = 49; // To store field no. of valve in action 
                        temporaryBytesArray[1] = field_No + 39; // To store field no. of valve in action 
                    }
                    /***************************/

                    /***************************/
					sendSms(SmsDR3, userMobileNo, fieldNoRequired); // Acknowledge user about dry run detected and action taken
					#ifdef SMS_DELIVERY_REPORT_ON_H
					sleepCount = 2; // Load sleep count for SMS transmission action
					sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
					setBCDdigit(0x05,0);
					deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
					setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
					#endif
					/***************************/
                }
                else { // next due date
                    /***************************/
                    // for field no. 01 to 09
                    if (field_No<9){
                        temporaryBytesArray[0] = 48; // To store field no. of valve in action 
                        temporaryBytesArray[1] = field_No + 49; // To store field no. of valve in action 
                    }// for field no. 10 to 12
                    else if (field_No > 8 && field_No < 12) {
                        temporaryBytesArray[0] = 49; // To store field no. of valve in action 
                        temporaryBytesArray[1] = field_No + 39; // To store field no. of valve in action 
                    }
                    /***************************/

                    /***************************/
					sendSms(SmsDR4, userMobileNo, fieldNoRequired); // Acknowledge user about dry run detected and action taken
					#ifdef SMS_DELIVERY_REPORT_ON_H
					sleepCount = 2; // Load sleep count for SMS transmission action
					sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
					setBCDdigit(0x05,0);
					deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
					setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
					#endif
					/***************************/
                }
            }
            if (phaseR) {
                /***************************/
                sendSms(SmsPh3, userMobileNo, noInfo); // Acknowledge user about Phase failure detected and action taken
            #ifdef SMS_DELIVERY_REPORT_ON_H
                sleepCount = 2; // Load sleep count for SMS transmission action
                sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
                setBCDdigit(0x05,0);
                deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
                setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
            #endif
                /***************************/
            }
            else if (phaseY) {
                /***************************/
                sendSms(SmsPh4, userMobileNo, noInfo); // Acknowledge user about Phase failure detected and action taken
            #ifdef SMS_DELIVERY_REPORT_ON_H
                sleepCount = 2; // Load sleep count for SMS transmission action
                sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
                setBCDdigit(0x05,0);
                deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
                setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
            #endif
                /***************************/
            }
            else if (phaseB) {
                /***************************/
                sendSms(SmsPh5, userMobileNo, noInfo); // Acknowledge user about Phase failure detected and action taken
            #ifdef SMS_DELIVERY_REPORT_ON_H
                sleepCount = 2; // Load sleep count for SMS transmission action
                sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
                setBCDdigit(0x05,0);
                deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
                setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
            #endif
                /***************************/
            }
            else {
                /***************************/
                sendSms(SmsPh6, userMobileNo, noInfo); // Acknowledge user about Phase failure detected and action taken
            #ifdef SMS_DELIVERY_REPORT_ON_H
                sleepCount = 2; // Load sleep count for SMS transmission action
                sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
                setBCDdigit(0x05,0);
                deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
                setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
            #endif
                /***************************/
            }
            
        }
        else if ((currentDD == fieldValve[field_No].nextDueDD && currentMM == fieldValve[field_No].nextDueMM && currentYY == fieldValve[field_No].nextDueYY)) {
            /******** Calculate and save Field Valve next date**********/
            fieldValve[field_No].nextDueDD = (unsigned char)dueDD;
            fieldValve[field_No].nextDueMM = dueMM;
            fieldValve[field_No].nextDueYY = dueYY;
            fieldValve[field_No].cyclesExecuted = fieldValve[field_No].cycles;
            myMsDelay(100);
            saveIrrigationValveDueTimeIntoEeprom(eepromAddress[field_No], &fieldValve[field_No]);
            myMsDelay(100);
            saveIrrigationValveCycleStatusIntoEeprom(eepromAddress[field_No], &fieldValve[field_No]);
            myMsDelay(100);
            /***********************************************/
        }
    }
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("doDryRunAction_OUT\r\n");
    //********Debug log#end**************//
#endif 
}

/*********** DRY RUN Action#End********/

/*********** Low Phase Action#Start********/

/*************************************************************************************************************************

This function is called to perform actions after detecting low phase condition.
After detecting low phase condition, stop all active valves and set them due when phase current recovers
Notify user about all actions

**************************************************************************************************************************/

void doLowPhaseAction(void) {
    //unsigned char field_No = CLEAR;
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("dolowPhaseAction_IN\r\n");
    //********Debug log#end**************//
#endif
    /***************************/
    sendSms(SmsPh2, userMobileNo, noInfo); // Acknowledge user about low phase current
#ifdef SMS_DELIVERY_REPORT_ON_H
    sleepCount = 2; // Load sleep count for SMS transmission action
    sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
    setBCDdigit(0x05,0);
    deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
    setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
#endif
    /***************************/
    if (valveDue) {
        powerOffMotor();
        iterator = 0;
        while(fieldList[iterator] != 255 && iterator < fieldCount) {
            deActivateValve(fieldList[iterator]); // Activate valve for given field
            myMsDelay(100);
            iterator++;
        }
        /************Fertigation switch off due to low phase detection***********/
        if (fieldValve[fieldList[0]].fertigationStage == injectPeriod) {
            fertigationValveControl = OFF; // Switch off fertigation valve in case it is ON
            if (!fieldValve[8].isConfigured && !fieldValve[9].isConfigured && !fieldValve[10].isConfigured && !fieldValve[11].isConfigured) { // 9-12 used as injector
                //Switch off all Injectors after completing fertigation on Period
                injector1Control = OFF;
                injector2Control = OFF;
                injector3Control = OFF;
                injector4Control = OFF;
            }

            /***************************/
            sendSms(SmsFert6, userMobileNo, fieldListRequired); // Acknowledge user about successful Fertigation stopped action due to low phase detection
        #ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
            setBCDdigit(0x05,0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider           
            setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
        #endif
            /***************************/
        }
    }
    phaseFailureActionTaken = true;
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("dolowPhaseAction_OUT\r\n");
    //********Debug log#end**************//
#endif
}

/*********** Low Phase Action#End********/

/*********** Phase Failure Action#Start********/

/*************************************************************************************************************************

This function is called to perform actions after detecting phase failure condition.
After detecting phase failure condition, stop all active valves and set them due when phase recovers
Notify user about all actions

**************************************************************************************************************************/

void doPhaseFailureAction(void) {
    //unsigned char field_No = CLEAR;
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("doPhaseFailureAction_IN\r\n");
    //********Debug log#end**************//
#endif
    /***************************/
    sendSms(SmsPh1, userMobileNo, noInfo); // Acknowledge user about phase failure
#ifdef SMS_DELIVERY_REPORT_ON_H
    sleepCount = 2; // Load sleep count for SMS transmission action
    sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
    setBCDdigit(0x05,0);
    deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
    setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
#endif
    /***************************/
    if (valveDue) {
        powerOffMotor();
        iterator = 0;
        while(fieldList[iterator] != 255 && iterator < fieldCount) {
            deActivateValve(fieldList[iterator]); // Activate valve for given field
            myMsDelay(100);
            iterator++;
        }
        /************Fertigation switch off due to Phase failure***********/
        if (fieldValve[fieldList[0]].fertigationStage == injectPeriod) {
            fertigationValveControl = OFF; // Switch off fertigation valve in case it is ON
            if (!fieldValve[8].isConfigured && !fieldValve[9].isConfigured && !fieldValve[10].isConfigured && !fieldValve[11].isConfigured) { // 9-12 used as injector
                //Switch off all Injectors after completing fertigation on Period
                injector1Control = OFF;
                injector2Control = OFF;
                injector3Control = OFF;
                injector4Control = OFF;
            }

            /***************************/
            sendSms(SmsFert6, userMobileNo, fieldListRequired); // Acknowledge user about successful Fertigation stopped action due to PhaseFailure
        #ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
            setBCDdigit(0x05,0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider           
            setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
        #endif
            /***************************/
        }
    }
    //if (valveDue) {
    //    for (field_No = 0; field_No < fieldCount; field_No++) {
    //        if (fieldValve[field_No].status == ON) {
    //            powerOffMotor();
    //            myMsDelay(1000);
    //            deActivateValve(field_No);   // Deactivate Valve upon phase failure condition and reset valve to next due time
    //            /************Fertigation switch off due to Phase failure***********/
    //            if (fieldValve[field_No].fertigationStage == injectPeriod) {
    //                fertigationValveControl = OFF; // Switch off fertigation valve in case it is ON
    //                if (!fieldValve[8].isConfigured && !fieldValve[9].isConfigured && !fieldValve[10].isConfigured && !fieldValve[11].isConfigured) { // 9-12 used as injector
    //                    //Switch off all Injectors after completing fertigation on Period
    //                    injector1Control = OFF;
    //                    injector2Control = OFF;
    //                   injector3Control = OFF;
    //                    injector4Control = OFF;
    //                }
    //
    //                /***************************/
    //                // for field no. 01 to 09
    //                if (field_No<9){
    //                    temporaryBytesArray[0] = 48; // To store field no. of valve in action 
    //                    temporaryBytesArray[1] = field_No + 49; // To store field no. of valve in action 
    //                }// for field no. 10 to 12
    //                else if (field_No > 8 && field_No < 12) {
    //                    temporaryBytesArray[0] = 49; // To store field no. of valve in action 
    //                    temporaryBytesArray[1] = field_No + 39; // To store field no. of valve in action 
    //                }
    //                /***************************/
    //
    //                /***************************/
    //                sendSms(SmsFert6, userMobileNo, fieldNoRequired); // Acknowledge user about successful Fertigation stopped action due to PhaseFailure
    //            #ifdef SMS_DELIVERY_REPORT_ON_H
    //                sleepCount = 2; // Load sleep count for SMS transmission action
    //                sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
    //                setBCDdigit(0x05,0);
    //                deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider           
    //                setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
    //            #endif
    //                /***************************/
    //            }
    //        }
    //    }
    //}
    phaseFailureActionTaken = true;
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("doPhaseFailureAction_OUT\r\n");
    //********Debug log#end**************//
#endif
}

/*********** Phase Failure Action#End********/

/*********** RTC battery drained#Start********/

/*************************************************************************************************************************

This function is called to measure motor phase current to detect dry run condition.
The Dry run condition is measured in terms of voltage of CT connected to one of the phase of motor.
The motor phase current is high  and low  for Wet (load) and Dry (no load) condition respectively.
This leads the output of CT with high and low voltage.
For Dry condition, the CT voltage is high and for wet condition, the CT voltage is low.
Here ADC module is used to measure high/ low voltage of CT thereby identifying load/no load condition.

 **************************************************************************************************************************/

_Bool isRTCBatteryDrained(void) {
    unsigned int batteryVoltage = 0;
    unsigned int batteryVoltageCutoff = 555;   //~2.7 v
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("isRTCBatteryDrained_IN\r\n");
    //********Debug log#end**************//
#endif
    selectChannel(RTCchannel);
    RTC_Trigger = ENABLED;
    myMsDelay(50);
    batteryVoltage = getADCResult();
    RTC_Trigger = DISABLED;
    if (batteryVoltage <= batteryVoltageCutoff) {
        lowRTCBatteryDetected = true;
        myMsDelay(100);
        saveRTCBatteryStatus();
        myMsDelay(100);
    #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("isRTCBatteryDrained_Yes_OUT\r\n");
        //********Debug log#end**************//
    #endif
        return true;
    } 
    else {
    #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("isRTCBatteryDrained_No_OUT\r\n");
        //********Debug log#end**************//
    #endif
        return false;
    }
}

/*********** RTC battery drained#End********/


/*********** Check RYB phase Detection#Start********/

/*************************************************************************************************************************

This function is called to detect phase failure condition.
The phase failure condition is measured by scanning three phase input lines.
Each phase line is converted into TTL signal using Digital signal comparator.high  and low for lost (out) and present (In) condition respectively.
The output of Comparator is high  and low for lost (out) and present (In) condition respectively.
For Phase failure condition, at least one comparator output is high and for Phase detected condition all comparator output is low.
Here Rising Edge and Falling Edge is used to detect high/ low Comparator Output.

 **************************************************************************************************************************/

_Bool phaseFailure(void) {
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("phaseFailure_IN\r\n");
    //********Debug log#end**************//
#endif
    if (!phaseB && !phaseY &&  !phaseR) {
        //All 3 phases are ON 
        phaseFailureDetected = false;
    #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("phaseFailure_No_OUT\r\n");
        //********Debug log#end**************//
    #endif
        return false;  //no Phase failure
    }
    // Falling Edge -- Any one Phase lost
    else {
        // one phase is lost
        phaseFailureDetected = false; //true
        phaseFailureActionTaken = false;
    #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("phaseFailure_Yes_OUT\r\n");
        //********Debug log#end**************//
    #endif
        return false; //phase failure // true
    }
}

/*********** Motor Dry run condition#End********/


/*********** Motor Power On #Start********/

/*************************************************************************************************************************

This function is called to power on motor
The purpose of this function is to activate relays to Switch ON Motor
 *
 **************************************************************************************************************************/
void powerOnMotor(void) {
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("powerOnMotor_IN\r\n");
    //********Debug log#end**************//
#endif
    myMsDelay(100);
    MotorControl = ON;
    Timer0Overflow = 0;  
    T0CON0bits.T0EN = ON; // Start timer0 to initiate 1 min cycle
    if(filtrationEnabled) {					  
        filtrationCycleSequence = 1;																		 
    }
    else {
        filtrationCycleSequence = 99;
    }
    dryRunCheckCount = 0;
#ifdef STAR_DELTA_DEFINITIONS_H
    myMsDelay(500);
    motorPowerTorque = ON;
    myMsDelay(900);
    motorPowerTorque = OFF;
#endif
    
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("powerOnMotor_OUT\r\n");
    //********Debug log#end**************//
#endif
}

/*********** Motor Power On#End********/


/*********** Motor Power Off #Start********/

/*************************************************************************************************************************

This function is called to power Off motor
The purpose of this function is de-activate relays to Switch OFF Motor

 **************************************************************************************************************************/

void powerOffMotor(void) {
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("powerOffMotor_IN\r\n");
    //********Debug log#end**************//
#endif
    T0CON0bits.T0EN = OFF; // stop timer0
    myMsDelay(100);
    filtration1ValveControl = OFF; // switch off filtration  valve if it is ON
    myMsDelay(50);
    filtration2ValveControl = OFF; // switch off filtration  valve if it is ON
    myMsDelay(50);
    filtration3ValveControl = OFF; // switch off filtration  valve if it is ON
    myMsDelay(50);
    MotorControl = OFF; // switch off Motor
    myMsDelay(50);
    
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("powerOffMotor_OUT\r\n");
    //********Debug log#end**************//
#endif
}

/*********** Motor Power Off#End********/


/*********** Field Valve Activation#Start********/

/*************************************************************************************************************************

This function is called to activate valve
The purpose of this function is to activate mentioned field valve and notify user about activation through SMS
	
 **************************************************************************************************************************/
void activateValve(unsigned char FieldNo) {
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("activateValve_IN\r\n");
    //********Debug log#end**************//
#endif
    // check field no. of valve in action
    switch (FieldNo) {
    case 0:
        myMsDelay(1000);
        field1ValveControl = ON; // switch on valve for field 1
        break;
    case 1:
        myMsDelay(1000);
        field2ValveControl = ON; // switch off valve for field 2
        break;
    case 2:
        myMsDelay(1000);
        field3ValveControl = ON; // switch on valve for field 3
        break;
    case 3:
        myMsDelay(1000);
        field4ValveControl = ON; // switch on valve for field 4
        break;    
    case 4:
        myMsDelay(1000);
        field5ValveControl = ON; // switch off valve for field 5
        break;
    case 5:
        myMsDelay(1000);
        field6ValveControl = ON; // switch off valve for field 6
        break;
    case 6:
        myMsDelay(1000);
        field7ValveControl = ON; // switch on valve for field 7
        break;
    case 7:
        myMsDelay(1000);
        field8ValveControl = ON; // switch on valve for field 8
        break;
    case 8:
        myMsDelay(1000);
        field9ValveControl = ON; // switch on valve for field 9
        break;
    case 9:
        myMsDelay(1000);
        field10ValveControl = ON; // switch on valve for field 10
        break;
    case 10:
        myMsDelay(1000);
        field11ValveControl = ON; // switch on valve for field 11
        break;
    case 11:
        myMsDelay(1000);
        field12ValveControl = ON; // switch on valve for field 12
        break;
    }
    fieldValve[FieldNo].status = ON; //notify field valve status
    valveDue = true; // Set Valve ON status
    myMsDelay(100);
    saveIrrigationValveOnOffStatusIntoEeprom(eepromAddress[FieldNo], &fieldValve[FieldNo]);
    myMsDelay(100);
#ifdef DEBUG_MODE_ON_H    
        //********Debug log#start************//
        transmitStringToDebug("activateValve_OUT\r\n");
        //********Debug log#end**************//
#endif
}
/*********** Field Valve Activation#End********/


/*********** Field Valve De-Activation#Start********/

/*************************************************************************************************************************

This function is called to de-activate valve
The purpose of this function is to deactivate mentioned field valve and notify user about De-activation through SMS

 **************************************************************************************************************************/
void deActivateValve(unsigned char FieldNo) {
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("deActivateValve_IN for valve: ");
    //********Debug log#end**************//
#endif
    // check field no. of valve in action
    switch (FieldNo) {
    case 0:
        myMsDelay(1000);
        field1ValveControl = OFF; // switch off valve for field 1
        break;
    case 1:
        myMsDelay(1000);
        field2ValveControl = OFF; // switch off valve for field 2
        break;
    case 2:
        myMsDelay(1000);
        field3ValveControl = OFF; // switch off valve for field 3
        break;
    case 3:
        myMsDelay(1000);
        field4ValveControl = OFF; // switch off valve for field 4
        break;    
    case 4:
        myMsDelay(1000);
        field5ValveControl = OFF; // switch off valve for field 5
        break;
    case 5:
        myMsDelay(1000);
        field6ValveControl = OFF; // switch off valve for field 6
        break;
    case 6:
        myMsDelay(1000);
        field7ValveControl = OFF; // switch off valve for field 7
        break;
    case 7:
        myMsDelay(1000);
        field8ValveControl = OFF; // switch off valve for field 8
        break;
    case 8:
        myMsDelay(1000);
        field9ValveControl = OFF; // switch off valve for field 9
        break;
    case 9:
        myMsDelay(1000);
        field10ValveControl = OFF; // switch off valve for field 10
        break;
    case 10:
        myMsDelay(1000);
        field11ValveControl = OFF; // switch off valve for field 11
        break;    
    case 11:
        myMsDelay(1000);
        field12ValveControl = OFF; // switch off valve for field 12
        break;   
    }
#ifdef DEBUG_MODE_ON_H    
        //********Debug log#start************//
        transmitStringToDebug("deActivateValve_OUT\r\n");
        //********Debug log#end**************//
#endif
}
/*********** Field Valve De-Activation#End********/


/********************Deep Sleep function#Start************************/

/*************************************************************************************************************************

This function is called to activate deep sleep mode
The purpose of this function is to go into sleep mode until it is interrupted by GSM or Sleep count is reached to 0

 **************************************************************************************************************************/
void deepSleep(void) {
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("deepSleep_IN\r\n");
    //********Debug log#end**************//
#endif
    // check until sleep timer ends given sleep count
    while (sleepCount > 0 && !newSMSRcvd) {
        if(phaseFailureDetected) {
            if(!phaseFailureActionTaken) {
                doPhaseFailureAction();
            }
            sleepCount = 65500;
            setBCDdigit(0x03,0);  // (3.) BCD Indication for Phase Failure Error
        }
        // Motor is ON without any external/Internal interrupt
        else if (MotorControl == ON ) {
            saveActiveSleepCountIntoEeprom(); // Save current valve on time
            // check Motor Dry run condition after each sleep count
            if (isMotorInNoLoad() && dryRunCheckCount > 2) {
                if (dryRunDetected) {
                    doDryRunAction();
                }
                else if (lowPhaseCurrentDetected) {
                    doLowPhaseAction();
                    sleepCount = 65500;
                }
            }
            else {
                setBCDdigit(0x0C,1);  // (u) BCD Indication for valve in action
            }     
        }
        else if(dryRunDetected) {
            setBCDdigit(0x0C,0);  // (u.) BCD Indication for Dry Run Detected Error
        }
        else if(lowPhaseCurrentDetected) {
            setBCDdigit(0x03,0);  // (3.) BCD Indication for Phase Failure Error
        }
        else if(lowRTCBatteryDetected) {
            setBCDdigit(0x02,0);  // (2.) BCD Indication for RTC Battery Low Error
        }
        else if (systemAuthenticated) {
            setBCDdigit(0x01,1);  // (1) BCD Indication for System Authenticated
        }
        Run_led = DARK; // Led Indication for system in Sleep/ Idle Mode
        inSleepMode = true; // Indicate in Sleep mode
        WDTCON0bits.SWDTEN = ENABLED; // Enable sleep mode timer
        if(sleepCount > 0 && !newSMSRcvd) {
            Sleep(); // CPU sleep. Wakeup when Watchdog overflows, each of 16 Seconds if value of WDTPS is 4096
        }
        /*************Debug*******************/
        if (sleepCount == 9) {
            sendSms(SmsTest, userMobileNo, noInfo); // Acknowledge user about successful Fertigation stopped action
            newSMSRcvd = true;
        }
        /*************Debug*******************/
        if(valveDue) {
            myMsDelay(1500); // compensate for new sms when valve is active
        }
        WDTCON0bits.SWDTEN = DISABLED; //turn off sleep mode timer
        Run_led = GLOW; // Led Indication for system in Operational Mode
        if(!valveDue && !phaseFailureDetected && !lowPhaseCurrentDetected) {
            sleepCount--; // Decrement sleep count after every sleep cycle
        }
    }
    if(sleepCount == 0 && !newSMSRcvd ) {
        myMsDelay(2000); // To compensate incoming SMS if valve is due within 10 minutes
    }
    inSleepMode = false; // Indicate not in sleep mode
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("deepSleep_OUT\r\n");
    //********Debug log#end**************//
#endif   
}
/********************Deep Sleep function#End************************/


/*************Initialize#Start**********/

/*************************************************************************************************************************

This function is called to initialized system
The purpose of this function is to define port lines, interrupt priorities and to configure Timer and UART module.

 **************************************************************************************************************************/
void configureController(void) {
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("configureController_IN\r\n");
    //********Debug log#end**************//
#endif
    
    BSR = 0x0f;     // Set BSR for Banked SFR
    LATA = 0x00;    // Set all output bits to zero for PORTA
    TRISA = 0x00;   // Set all pins of PORTA as outputs
    ANSELA = 0x00;  // All pins of PORTA are digital I/O  -- Relay control lines 1-6
    WPUA = 0x00;    //Weak Pull-up disabled
    ODCONA = 0x00;  //Output drives both high-going and low-going signals (source and sink current)
    SLRCONA = 0xFF; //Port pin slew rate is limited
    INLVLA = 0xFF;  //ST input used for port reads and interrupt-on-change
    
    LATB = 0x00;    // Set all output bits to zero for PORTB
    TRISB = 0xff;   // Set all pins of PORTB as inputs -- Moisture sensor input 1-6
    ANSELB = 0x00;  // All pins of PORTB are digital I/O -- Moisture sensor input lines 1-6
    WPUB = 0x00;    //Weak Pull-up disabled
    ODCONB = 0x00;  //Output drives both high-going and low-going signals (source and sink current)
    SLRCONB = 0xFF; //Port pin slew rate is limited
    INLVLB = 0xFF;  //ST input used for port reads and interrupt-on-change
    
    LATC = 0x00;        // Set all output bits to zero for PORTC
    TRISC = 0b10111100; // Set RC<0:1> unused outputs, RC<2:5> as inputs -- Moisture sensor input 9-12, RC<6> as output -- TX1 (WIFI), RC<7> as input -- RX1 (WIFI)
    WPUC = 0x00;        //Weak Pull-up disabled
    ODCONC = 0x00;      //Output drives both high-going and low-going signals (source and sink current)
    SLRCONC = 0xFF;     //Port pin slew rate is limited
    INLVLC = 0xFF;      //ST input used for port reads and interrupt-on-change
    
    LATD = 0x00;        // Set all output bits to zero for PORTD
    TRISD = 0b01111111; // Set RD<0:3> as input CT,Temperature,wind speed,RTC battery, RD<4> as input -- Moisture sensor input 8 RD<5:6> as inputs -- SDA,SCL (RTC), RD<7> as output -- BCD DP
    ANSELD = 0b00001111;// Set RD<0:3> as Analog , RD<4:7> as Digital I/O 
    WPUD = 0x00;        //Weak Pull-up disabled
    ODCOND = 0x00;      //Output drives both high-going and low-going signals (source and sink current)
    SLRCOND = 0xFF;     //Port pin slew rate is limited
    INLVLD = 0xFF;      //ST input used for port reads and interrupt-on-change
    
    LATE = 0x00;        // Set all output bits to zero for PORTE
    TRISE = 0b11110010; // Set RE<0> as output -- TX3 (GSM), RE<1> as input -- RX3 (GSM), RE<2:3> as output -- Relay control lines 17-18, RE<4> as input -- Moisture sensor input 7, RE<5:7> as input -- Phase input lines
    ANSELE = 0x00;      // All pins of PORTE are digital I/O
    WPUE = 0x00;        //Weak Pull-up disabled //Weak Pull-up disabled
    ODCONE = 0x00;      //Output drives both high-going and low-going signals (source and sink current)
    SLRCONE = 0xFF;     //Port pin slew rate is limited
    INLVLE = 0xFF;      //ST input used for port reads and interrupt-on-change
    IOCEN = 0b11100000; //Interrupt-on-Change Negative Edge Enable bits for RE5,RE6 and RE7  -- RYB phase
    IOCEP = 0b11100000; //Interrupt-on-Change Positive Edge Enable bits for RE5,RE6 and RE7  -- RYB Phase
    IOCEF = 0b00000000; // Clear all initial IOC flags
    PIE0bits.IOCIE = ENABLED; //Peripheral Interrupt-on-Change Enabled
    
    LATF = 0x00;    // Set all output bits to zero for PORTF
    TRISF = 0x00;   // Set all pins of PORTF as outputs
    ANSELF = 0x00;  // All pins of PORTF are digital I/O -- Relay control lines 7-14
    WPUF = 0x00;    //Weak Pull-up disabled
    ODCONF = 0x00;  //Output drives both high-going and low-going signals (source and sink current)
    SLRCONF = 0xFF; //Port pin slew rate is limited
    INLVLF = 0xFF;  //ST input used for port reads and interrupt-on-change
    
    LATG = 0x00;        // Set all output bits to zero for PORTG
    TRISG = 0b00000100; // Set RG<0;4> as outputs -- LED, RG<1> as output -- TX2, RG<2> as inputs -- RX2, RG<3> as output -- GSM Reset, RG<6:7> as output -- Relay control lines 15-16
    ANSELG = 0x00;      // All pins of PORTG are digital I/O -- Relay control lines 15-16
    WPUG = 0x00;        //Weak Pull-up disabled
    ODCONG = 0x00;      //Output drives both high-going and low-going signals (source and sink current)
    SLRCONG = 0xFF;     //Port pin slew rate is limited
    INLVLG = 0xFF;      //ST input used for port reads and interrupt-on-change
    
    LATH = 0x00;        // Set all output bits to zero for PORTH
    TRISH = 0b00000000; // Set RH<0:3> as outputs -- BCD
    WPUH = 0x00;        //Weak Pull-up disabled
    ODCONH = 0x00;      //Output drives both high-going and low-going signals (source and sink current)
    SLRCONH = 0xFF;     //Port pin slew rate is limited
    INLVLH = 0xFF;      //ST input used for port reads and interrupt-on-change
    
    
     //-----------ADC_Config-----------------------//
    
	ADREF = 0b00000000;  // Reference voltage set to VDD and GND
	ADCON0bits.ADFM = 1; // ADC results Format -- ADRES and ADPREV data are right-justified
	ADCON0bits.ADCS = 1; // ADC Clock supplied from FRC dedicated oscillator
    ADCON0bits.ADON = 1; //ADC is enabled
    ADCON0bits.ADCONT = 0; //ADC Continuous Operation is disabled
    ADCON1 = 0X00;
    ADCON2 = 0X00;
    ADCON3 = 0X00;
    ADACQ = 0X00;
    ADCAP = 0X00;
    ADRPT = 0X00;
    ADACT = 0X00;

    //-----------Timer0_Config (60 sec) used for SLEEP Count control during Motor ON period and to control filtration  cycle sequence followup----------------------//
    //-----------Timer will not halt in sleep mode------------------------------------------------------//

    T0CON0 = 0b00010000; // 16 bit Timer 
    T0CON1 = 0b10011000; // Asynchronous with LFINTOSC 31KHZ as clock source with prescalar 1:256
    TMR0H = 0xE3; // Load Timer0 Register Higher Byte 0xE390
    TMR0L = 0xB0; // Load Timer0 Register Lower Byte FFFF-(60*31K)/(256) = 0xE39F)
    PIR0bits.TMR0IF = CLEAR; // Clear Timer0 Overflow Interrupt at start
    PIE0bits.TMR0IE = ENABLED; // Enables the Timer0 Overflow Interrupt
    IPR0bits.TMR0IP = LOW; // Low Timer0 Overflow Interrupt Priority

    //-----------Timer1_Config used for calculation pulse width of moisture sensor output-----------//
    //-----------Timer will halt in sleep mode------------------------------------------------------//

    T1CON = 0b00000010; // 16 bit Timer with Synchronous mode
    TMR1CLK = 0b00000001; //  Clock source as FOSC/4
    TMR1H = CLEAR; // Clear Timer1 Register Higher Byte
    TMR1L = CLEAR; // Clear Timer1 Register Lower Byte
    PIR5bits.TMR1IF = CLEAR; // Clear Timer1 Overflow Interrupt at start
    PIE5bits.TMR1IE = ENABLED; // Enables the Timer1 Overflow Interrupt
    IPR5bits.TMR1IP = LOW; // Low Timer1 Overflow Interrupt Priority
    
    //-----------Timer3_Config (1 sec) used if command fails to respond within timer limit----------------------//
    //-----------Timer will halt in sleep mode------------------------------------------------------//

    T3CON = 0b00110010; // 16 bit Timer with synchronous mode with 1:8 pre scale
    TMR3CLK = 0b00000100; // Clock source as LFINTOSC
    TMR3H = 0xF0; // Load Timer3 Register Higher Byte 
    TMR3L = 0xDC; // Load Timer3 Register lower Byte 
    PIR5bits.TMR3IF = CLEAR; // Clear Timer3 Overflow Interrupt at start
    PIE5bits.TMR3IE = ENABLED; // Enables the Timer3 Overflow Interrupt
    IPR5bits.TMR3IP = LOW; // Low Timer3 Overflow Interrupt Priority
/*
    //-----------UART1_Config PRODUCTION-----------------------//
    TX1STA = 0b00100100; // 8 Bit Transmission Enabled with High Baud Rate
    RC1STA = 0b10000000; // 8 Bit Serial Port Enabled with Reception Disabled
    BAUD1CON = 0b00001000; // 16 Bit Baud Rate Register used
    SP1BRG = 0x0681; // XTAL=16MHz, Fosc=64Mhz for SYNC=0 BRGH=1 BRG16=1 (Asynchronous high 16 bit baud rate)
    RC7PPS = 0x17; //EUSART1 Receive
    RC6PPS = 0x0C; //EUSART1 Transmit
    temp = RC1REG; // Empty buffer
    PIE3bits.RC1IE = DISABLED; // Disables the EUSART Receive Interrupt
    PIE3bits.TX1IE = DISABLED; // Disables the EUSART Transmit Interrupt
    IPR3bits.RC1IP = HIGH; // EUSART Receive Interrupt Priority
*/
#ifdef DEBUG_MODE_ON_H
    //-----------UART2_Config DEBUG-----------------------//
  
    TX2STA = 0b00100100; // 8 Bit Transmission Enabled with High Baud Rate
    RC2STA = 0b10000000; // Serial Port Enabled with Reception Disabled  
    BAUD2CON = 0b00011000; // 16 Bit Baud Rate Register used
    SP2BRG = 0x0681; // XTAL=16MHz, Fosc=64Mhz for SYNC=0 BRGH=1 BRG16=1 (Asynchronous high 16 bit baud rate)
    RG2PPS = 0x32;
    RG1PPS = 0X0E;
    PIE3bits.RC2IE = DISABLED; // Disables the EUSART Receive Interrupt
    PIE3bits.TX2IE = DISABLED; // Disables the EUSART Transmit Interrupt
#endif

    //-----------UART3_Config PRODUCTION GSM-----------------------//
  
    TX3STA = 0b00100100; // 8 Bit Transmission Enabled with High Baud Rate
    RC3STA = 0b10010000; // 8 Bit Reception Enabled with Continuous Reception
    BAUD3CON = 0b00001000; // 16 Bit Baud Rate Register used
    SP3BRG = 0x0681; // XTAL=16MHz, Fosc=64Mhz for SYNC=0 BRGH=1 BRG16=1 (Asynchronous high 16 bit baud rate)
    RE1PPS = 0x21; //EUSART3 Receive
    RE0PPS = 0x10; //EUSART3 Transmit
    temp = RC3REG; // Empty buffer
    PIE4bits.RC3IE = ENABLED; // Enables the EUSART Receive Interrupt
    PIE4bits.TX3IE = DISABLED; // Disables the EUSART Transmit Interrupt
    IPR4bits.RC3IP = HIGH; // EUSART Receive Interrupt Priority
        
    //-----------I2C_Config-----------------------//
    
	SSP2STAT |= 0x80; //Slew Rate Disabled
	SSP2CON1 = 0b00101000; //Master mode
    SSP2DATPPS = 0x1D;   //RD5<-MSSP2:SDA2;    
    RD5PPS = 0x1C;   //RD5->MSSP2:SDA2;    
    RD6PPS = 0x1B;   //RD6->MSSP2:SCL2;    
    SSP2CLKPPS = 0x1E;   //RD6<-MSSP2:SCL2; 
	SSP2ADD = 119;
    
    //-----------Interrupt_Config---------------//
    
    OSCENbits.SOSCEN = DISABLED; //Secondary Oscillator is disabled
    INTCONbits.IPEN = ENABLED; // Enables Priority Levels on Interrupts
    INTCONbits.PEIE = ENABLED; // Enables all unmasked  peripheral interrupts
    INTCONbits.GIE = ENABLED; // Enables all unmasked Global interrupts
    CPUDOZEbits.IDLEN = ENABLED; //Device enters into Idle mode on SLEEP Instruction.
    
    // Initialize Valve list
    iterator = 0;
    while(iterator < fieldCount) {
        fieldList[iterator] = 255;
        lastFieldList[iterator] = 255;
        iterator++;
    }
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("configureController_OUT\r\n");
    //********Debug log#end**************//
#endif
}
/*************Initialize#End**********/

/*************actionsOnSystemReset#Start**********/
/*************************************************************************************************************************

This function is called to do actions on system reset
The purpose of this function is to perform actions on Power on reset or System hard Reset.

***************************************************************************************************************************/
void actionsOnSystemReset(void) {
    unsigned char resetType = CLEAR;
    unsigned char localIndex = CLEAR;
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("actionsOnSystemReset_IN\r\n");
    //********Debug log#end**************//
#endif
    // check if system power reset occurred
    if (!PCON0bits.nPOR || !PCON0bits.nRI || !PCON0bits.nRMCLR || !PCON0bits.nBOR || !PCON0bits.nRWDT || PCON0bits.STKOVF || PCON0bits.STKUNF) {
        if(!PCON0bits.nPOR || !PCON0bits.nBOR) {
            PCON0bits.nRMCLR = SET; // Reset reset on MCLR status
            PCON0bits.nRI = SET;    // Reset reset on instruction status
            PCON0bits.nRWDT = SET;  // Reset WDT status
            PCON0bits.STKOVF = CLEAR;  // Reset Stack Overflow status
            PCON0bits.STKUNF = CLEAR;  // Reset Stack underflow status
        }
        if (!PCON0bits.nPOR) {
            resetType = PowerOnReset;
            PCON0bits.nPOR = SET;   // Reset power status
        }
        else if (!PCON0bits.nBOR) {
            resetType = LowPowerReset;
            PCON0bits.nBOR = SET;   // Reset BOR status
        }
        else if (!PCON0bits.nRMCLR) {
            resetType = HardReset;
            PCON0bits.nRMCLR = SET; // Reset reset on MCLR status
        }
        else if (!PCON0bits.nRI) {
            resetType = SoftResest;
            PCON0bits.nRI = SET;    // Reset reset on instruction status
        }
        else if (!PCON0bits.nRWDT) {
            resetType = WDTReset;
            PCON0bits.nRWDT = SET;  // Reset WDT status
        }
        else if (PCON0bits.STKOVF || PCON0bits.STKUNF) {
            resetType = StackReset;
            PCON0bits.STKOVF = CLEAR;  // Reset Stack Overflow status
            PCON0bits.STKUNF = CLEAR;  // Reset Stack underflow status
        }
        if (resetType != HardReset) {
            myMsDelay(30000); // Warmup 5 seconds
        }
        configureController(); // set Microcontroller ports, ADC, Timer, I2C, UART, Interrupt Config
        myMsDelay(1000);
        loadDataFromEeprom(); // Read configured valve data saved in EEprom
        //myMsDelay(1000);
        //configureGSM(); // Configure GSM in TEXT mode
        //myMsDelay(1000);
        //setGsmToLocalTime();
        //myMsDelay(1000);
        if (DeviceBurnStatus == false) {
            DeviceBurnStatus = true;
            //randomPasswordGeneration();
            //saveFactryPswrdIntoEeprom();
            saveDeviceProgramStatusIntoEeprom();
            //*******************Load Test Data_start*************************//
            systemAuthenticated = true;
            myMsDelay(50);
            saveAuthenticationStatus();
            myMsDelay(50);
            strncpy(decodedString,"CT 10 200 ",10);
            extractReceivedSms();
            strncpy(decodedString,"FEED 10 10 10 10 10 50",22);
            extractReceivedSms();
            strncpy(decodedString,"SET01 10 2 10 10 0 300 0401 3 0 ",31);
            extractReceivedSms();
            //strncpy(decodedString,"SET02 11 1 10 15 0 300 0203 3 0 ",32);
            //extractReceivedSms();
            //strncpy(decodedString,"SET03 12 3 10 15 0 300 050607 3 0 ",34);
            //extractReceivedSms();
            /*
            //strncpy(decodedString,"SET04 8 2 10 10 0 300 1 2 0 ",28);
            //extractReceivedSms();
            //strncpy(decodedString,"SET01 12 3 10 10 0 300 3 3 0 ",29);
            //extractReceivedSms();
            //strncpy(decodedString,"SET05 12 3 10 10 0 300 3 3 0 ",29);
            //extractReceivedSms();
            //strncpy(decodedString,"SET06 12 3 10 10 0 300 3 3 0 ",29);
            //extractReceivedSms();
            //strncpy(decodedString,"SET07 12 3 10 10 0 300 3 3 0 ",29);
            //extractReceivedSms();
            */ 
            //strncpy(decodedString,"ENABLE02 1 9 3 2 1 3 1 2 2 2 1 2 1 2 3 ",39);
            //extractReceivedSms();
            //strncpy(decodedString,"ENABLE03 1 9 3 2 1 3 2 1 2 2 1 1 2 1 3 ",39);
            //extractReceivedSms();
            //strncpy(decodedString,"ENABLE04 1 6 2 1 2 2 1 2 1 2 1 2 2 1 2 ",39);
            //extractReceivedSms();
            //strncpy(decodedString,"ENABLE01 1 8 2 1 1 2 1 1 2 1 1 2 1 1 2 ",39);
            //extractReceivedSms();
            //strncpy(decodedString,"ENABLE07 1 10 3 1 1 3 1 1 3 1 1 3 1 1 1 ",40);
            //extractReceivedSms();
            //strncpy(decodedString,"TIME",4);
            //extractReceivedSms();
            for (iterator = 0; iterator < fieldCount; iterator++) {
                // for field no. 01 to 09
                if (iterator<9){
                    temporaryBytesArray[0] = 48; // To store field no. of valve in action 
                    temporaryBytesArray[1] = iterator + 49; // To store field no. of valve in action 
                }// for field no. 10 to 12
                else if (iterator > 8 && iterator < 12) {
                    temporaryBytesArray[0] = 49; // To store field no. of valve in action 
                    temporaryBytesArray[1] = iterator + 39; // To store field no. of valve in action 
                }
                fieldList[iterator] = iterator;
                sendSms(SmsIrr7, userMobileNo, IrrigationData);  // Give diagnostic data
            }
            //*******************Load Test Data_End*************************//
        }
        if (resetType == HardReset) {
            for(iterator = 0; iterator < 5 && resetCount == 0; iterator++) {
                    setBCDdigit(0x00,1); // BCD Indication for Reset Action#n
                    myMsDelay(500);
                    setBCDdigit(0x0F,1); // BCD Indication for Flash
                    myMsDelay(500);
			}
            if (resetCount == 0) {
                for(iterator = 1; iterator < 10; iterator++) {
                    resetCount++;
                    saveResetCountIntoEeprom();
                    setBCDdigit(iterator,1); // BCD Indication for Reset Action#n
                    myMsDelay(10000);
                }
                resetCount = 0x00;
                saveResetCountIntoEeprom();
            }
            else {
                switch(resetCount) {
                case 1:
                    resetCount = 0x00;
                    saveResetCountIntoEeprom();
                    for(iterator = 0; iterator < 5; iterator++) {
                        setBCDdigit(0x0F,1); // BCD Indication for Flash
                        myMsDelay(500);
                        setBCDdigit(0x01,1); // BCD Indication for Reset Action#n
                        myMsDelay(1000);
                    }
                    calibrateMotorCurrent(NoLoad,0);
                    MotorControl = OFF;             //Manual procedure off
                    msgIndex = CLEAR;
                    /***************************/
                    sendSms(SmsMotor3, userMobileNo, motorLoadRequired);
                #ifdef SMS_DELIVERY_REPORT_ON_H
                    sleepCount = 2; // Load sleep count for SMS transmission action
                    sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
                    setBCDdigit(0x05,0);
                    deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
                    setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
                #endif
                    /***************************/
                #ifdef DEBUG_MODE_ON_H
                    //********Debug log#start************//
                    transmitStringToDebug("actionsOnSystemReset_1_OUT\r\n");
                    //********Debug log#end**************//
                #endif
                    break;
                case 2:
                    resetCount = 0x00;
                    saveResetCountIntoEeprom();
                    for(iterator = 0; iterator < 5; iterator++) {
                        setBCDdigit(0x0F,1); // BCD Indication for Flash
                        myMsDelay(500);
                        setBCDdigit(0x02,1); // BCD Indication for Reset Action#n
                        myMsDelay(1000);
                    }
                    calibrateMotorCurrent(FullLoad,0);
                    msgIndex = CLEAR;
                    /***************************/
                    sendSms(SmsMotor3, userMobileNo, motorLoadRequired);
                #ifdef SMS_DELIVERY_REPORT_ON_H
                    sleepCount = 2; // Load sleep count for SMS transmission action
                    sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
                    setBCDdigit(0x05,0);
                    deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
                    setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
                #endif
                    /***************************/
                #ifdef DEBUG_MODE_ON_H
                    //********Debug log#start************//
                    transmitStringToDebug("actionsOnSystemReset_2_OUT\r\n");
                    //********Debug log#end**************//
                #endif
                    break;
                case 3:
                    resetCount = 0x00;
                    saveResetCountIntoEeprom();
                    for(iterator = 0; iterator < 5; iterator++) {
                        setBCDdigit(0x0F,1); // BCD Indication for Flash
                        myMsDelay(500);
                        setBCDdigit(0x03,1); // BCD Indication for Reset Action#n
                        myMsDelay(1000);
                    }
                    checkSignalStrength();
                    break;
                case 4:
                    resetCount = 0x00;
                    saveResetCountIntoEeprom();
                    for(iterator = 0; iterator < 5; iterator++) {
                        setBCDdigit(0x0F,1); // BCD Indication for Flash
                        myMsDelay(500);
                        setBCDdigit(0x04,1); // BCD Indication for Reset Action#n
                        myMsDelay(1000);
                    }
                    break;
                case 5:
                    resetCount = 0x00;
                    saveResetCountIntoEeprom();
                    for(iterator = 0; iterator < 5; iterator++) {
                        setBCDdigit(0x0F,1); // BCD Indication for Flash
                        myMsDelay(500);
                        setBCDdigit(0x05,1); // BCD Indication for Reset Action#n
                        myMsDelay(1000);
                    }
                    break;
                case 6:
                    resetCount = 0x00;
                    saveResetCountIntoEeprom();
                    for(iterator = 0; iterator < 5; iterator++) {
                        setBCDdigit(0x0F,1); // BCD Indication for Flash
                        myMsDelay(500);
                        setBCDdigit(0x06,1); // BCD Indication for Reset Action#n
                        myMsDelay(1000);
                    }
                    break;
                case 7:
                    resetCount = 0x00;
                    saveResetCountIntoEeprom();
                    if (systemAuthenticated) {
                        for(iterator = 0; iterator < 5; iterator++) {
                            setBCDdigit(0x0F,1); // BCD Indication for Flash
                            myMsDelay(500);
                            setBCDdigit(0x07,1); // BCD Indication for Reset Action#n
                            myMsDelay(1000);
                        }
                        deleteValveData();
                        deleteUserData();
                        myMsDelay(1000);
                        loadDataFromEeprom(); // Read configured valve data saved in EEprom
                    }
                    break;
                case 8:
                    resetCount = 0x00;
                    saveResetCountIntoEeprom();
                    if (systemAuthenticated) {
                        for(iterator = 0; iterator < 5; iterator++) {
                            setBCDdigit(0x0F,1); // BCD Indication for Flash
                            myMsDelay(500);
                            setBCDdigit(0x08,1); // BCD Indication for Reset Action#n
                            myMsDelay(1000);
                        }
                        deleteValveData();
                        myMsDelay(1000);
                        loadDataFromEeprom(); // Read configured valve data saved in EEprom
                    }
                    break;
                case 9:
                    resetCount = 0x00;
                    saveResetCountIntoEeprom();
                    if (systemAuthenticated) {
                        for(iterator = 0; iterator < 5; iterator++) {
                            setBCDdigit(0x0F,1); // BCD Indication for Flash
                            myMsDelay(500);
                            setBCDdigit(0x09,1); // BCD Indication for Reset Action#n
                            myMsDelay(1000);
                        }
                        deleteUserData();
                        myMsDelay(1000);
                        loadDataFromEeprom(); // Read configured valve data saved in EEprom
                    }
                }
                resetCount = 0x00;
                saveResetCountIntoEeprom();
            }
        }     
        //deleteMsgFromSIMStorage(); // Clear GSM storage memory for new Messages
    }
    // check if system is authenticated and valve action is due
    if (systemAuthenticated) {
        // phase failure detected
        if (phaseFailure()) {                
            sleepCount = 65500;
            /***************************/
            sendSms(SmsSR01, userMobileNo, noInfo); // Acknowledge user about System restarted with phase failure
        #ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission 
            setBCDdigit(0x05,0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider 
            setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
        #endif
            phaseFailureActionTaken = true;
        }
        else {
            fetchTimefromRTC();
            // check if System is configured
            for (iterator = 0; iterator < fieldCount; iterator++) {
                // check if any field valve status was true after reset
                if (fieldValve[iterator].status == ON) {
                    nxtPriority = fieldValve[iterator].priority; // start action from interrupted field irrigation valve
                    /*** Check if System Restarted on next day of Due date ***/
                    // if year over passes ||  if month  over passes ||  if day over passes 
                    if ((currentYY > fieldValve[iterator].nextDueYY)||(currentMM > fieldValve[iterator].nextDueMM && currentYY == fieldValve[iterator].nextDueYY)||(currentDD > fieldValve[iterator].nextDueDD && currentMM == fieldValve[iterator].nextDueMM && currentYY == fieldValve[iterator].nextDueYY) || (currentHour > fieldValve[iterator].motorOnTimeHour && currentDD == fieldValve[iterator].nextDueDD && currentMM == fieldValve[iterator].nextDueMM && currentYY == fieldValve[iterator].nextDueYY)) {
                        valveDue = false; // Clear Valve Due
                        fieldValve[iterator].status = OFF;
                        fieldValve[iterator].cyclesExecuted = fieldValve[iterator].cycles;
                        if (fieldValve[iterator].isFertigationEnabled) {
                            if (fieldValve[iterator].fertigationStage == injectPeriod) {
                                fieldValve[iterator].fertigationStage = OFF;
                                fieldValve[iterator].fertigationValveInterrupted = true;
                                remainingFertigationOnPeriod = readActiveSleepCountFromEeprom();
                                saveRemainingFertigationOnPeriod();
                            }
                            else if (fieldValve[iterator].fertigationStage == flushPeriod || fieldValve[iterator].fertigationStage == wetPeriod) {
                                fieldValve[iterator].fertigationStage = OFF;
                            }
                        }
                        myMsDelay(100);
                    #ifdef DEBUG_MODE_ON_H
                        //********Debug log#start************//
                        transmitStringToDebug("System restarted with Due valve on next day\r\n");
                        //********Debug log#end**************//
                    #endif
                        break;
                    }
                    else { // if system restarted on same day with due valve
                        valveDue = true; // Set valve ON status
                    #ifdef DEBUG_MODE_ON_H
                        //********Debug log#start************//
                        transmitStringToDebug("System restarted with Due valve on same day\r\n");
                        //********Debug log#end**************//
                    #endif
                        break;
                    }   
                }
            }
            if (valveDue) { 
                //fetch valve list which were interrupted with status ON
                parallelValveFetched = true;
                fieldList[0] = iterator; // assign due valve
                localIndex = 1;
                for (iterator = 0; iterator < fieldCount ; iterator++) {
                    if (iterator != fieldList[0]) { // not assigned valve
                        if (fieldValve[iterator].status == ON) { // Fetch valves status ON
                            fieldList[localIndex] = iterator;
                            localIndex++;
                        }
                    }
                }
                iterator = fieldList[0];
                dueValveChecked = true;
                switch (resetType) {
                case PowerOnReset:
                    sendSms(SmsSR02, userMobileNo, fieldListRequired); // Acknowledge user about system restarted with Valve action
                    break;
                case LowPowerReset:
                    sendSms(SmsSR03, userMobileNo, fieldListRequired); // Acknowledge user about system restarted with Valve action
                    break;
                case HardReset:
                    sendSms(SmsSR04, userMobileNo, fieldListRequired); // Acknowledge user about system restarted with Valve action
                    break;
                case SoftResest:
                    sendSms(SmsSR05, userMobileNo, fieldListRequired); // Acknowledge user about system restarted with Valve action
                    break;
                case WDTReset:
                    sendSms(SmsSR06, userMobileNo, fieldListRequired); // Acknowledge user about system restarted with Valve action
                    break;
                case StackReset:
                    sendSms(SmsSR07, userMobileNo, fieldListRequired); // Acknowledge user about system restarted with Valve action
                    break;
                }
                resetType = CLEAR;
                /***************************/
            #ifdef SMS_DELIVERY_REPORT_ON_H
                sleepCount = 2; // Load sleep count for SMS transmission action
                sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission 
                setBCDdigit(0x05,0);
                deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
                setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
            #endif
                /***************************/
            #ifdef DEBUG_MODE_ON_H
                //********Debug log#start************//
                transmitStringToDebug("System restarted with Due valve\r\n");
                //********Debug log#end**************//
            #endif
                sleepCount = readActiveSleepCountFromEeprom();
            }
            else { // check if no valve action is due
                /* // disabled SMS for reset on NO valve in action to reduce SMS count
                switch (resetType) {
                case PowerOnReset:
                    sendSms(SmsSR08, userMobileNo, noInfo); // Acknowledge user about system restarted with Valve action
                    break;
                case LowPowerReset:
                    sendSms(SmsSR09, userMobileNo, noInfo); // Acknowledge user about system restarted with Valve action
                    break;
                case HardReset:
                    sendSms(SmsSR10, userMobileNo, noInfo); // Acknowledge user about system restarted with Valve action
                    break;
                case SoftResest:
                    sendSms(SmsSR11, userMobileNo, noInfo); // Acknowledge user about system restarted with Valve action
                    break;
                case WDTReset:
                    sendSms(SmsSR12, userMobileNo, noInfo); // Acknowledge user about system restarted with Valve action
                    break;
                case StackReset:
                    sendSms(SmsSR13, userMobileNo, noInfo); // Acknowledge user about system restarted with Valve action
                    break;
                }
                */
                resetType = CLEAR;
                /***************************/
            #ifdef SMS_DELIVERY_REPORT_ON_H
                sleepCount = 2; // Load sleep count for SMS transmission action
                sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission 
                setBCDdigit(0x05,0);
                deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider 
                setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
            #endif
                /***************************/
            #ifdef DEBUG_MODE_ON_H
                //********Debug log#start************//
                transmitStringToDebug("System restarted W/O Due Valve\r\n");
                //********Debug log#end**************//
            #endif
            } 
        }   
    }
    // check if System not yet configured
    else {
        while (!systemAuthenticated) {
            setBCDdigit(0x01,0); // (1.) BCD Indication for Authentication Error 
            strncpy(pwd, factryPswrd, 6); // consider factory password
            sleepCount = 65500; // Set Default Sleep count until next sleep count is calculated
            deepSleep(); // Sleep with default sleep count until system is configured
            // check if Sleep count executed with interrupt occurred due to new SMS command reception
            if (newSMSRcvd) {
                setBCDdigit(0x02,1); // (2) BCD Indication for New SMS Received
                myMsDelay(500);
                newSMSRcvd = false; // received cmd is processed										
                extractReceivedSms(); // Read received SMS
                deleteMsgFromSIMStorage();
            }
        }
        if (phaseFailure()) {                
            sleepCount = 65500;
            /***************************/
            sendSms(SmsAU4, userMobileNo, noInfo); // Acknowledge user about System Authenticated with Phase failure
        #ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission 
            setBCDdigit(0x05,0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider 
            setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
        #endif
            phaseFailureActionTaken = true;
        }
    }
    if (isRTCBatteryDrained()) {   
        /***************************/
        sendSms(SmsRTC1, userMobileNo, noInfo); // Acknowledge user about Please replace RTC battery
    #ifdef SMS_DELIVERY_REPORT_ON_H
        sleepCount = 2; // Load sleep count for SMS transmission action
        sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
        setBCDdigit(0x05,0);
        deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
        setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
    #endif
        /***************************/
        if(gsmSetToLocalTime) {
            getDateFromGSM(); // Get today's date from Network
            myMsDelay(1000);
            feedTimeInRTC(); // Feed fetched date from network into RTC
            myMsDelay(1000);
        }
    }
    else if (lowRTCBatteryDetected) {
        lowRTCBatteryDetected = false;
        myMsDelay(100);
        saveRTCBatteryStatus();
        myMsDelay(100);
        if(gsmSetToLocalTime) {
            getDateFromGSM(); // Get today's date from Network
            myMsDelay(1000);
            feedTimeInRTC(); // Feed fetched date from network into RTC
            myMsDelay(1000);
            /***************************/
            sendSms(SmsRTC3, userMobileNo, noInfo); // Acknowledge user about New RTC battery found, system time is set to local time
        #ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
            setBCDdigit(0x05,0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
            setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
        #endif
            /***************************/
        }
        else {
            /***************************/
            sendSms(SmsRTC4, userMobileNo, noInfo); // Acknowledge user about New RTC battery found, please set system time manually to local time
        #ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
            setBCDdigit(0x05,0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
            setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
        #endif
            /***************************/  
        }   
    } 
    else if(gsmSetToLocalTime) {
        getDateFromGSM(); // Get today's date from Network
        myMsDelay(1000);
        feedTimeInRTC(); // Feed fetched date from network into RTC
        myMsDelay(1000);
    }
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("actionsOnSystemReset_OUT\r\n");
    //********Debug log#end**************//
#endif
}
/*************actionsOnSystemReset#End**********/

/*************actionsOnSleepCountFinish#Start**********/
/*************************************************************************************************************************

This function is called to do actions after completing sleep count
The purpose of this function is to perform actions after awaking from deep sleep.

***************************************************************************************************************************/
void actionsOnSleepCountFinish(void) {
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("actionsOnSleepCountFinish_IN\r\n");
    //********Debug log#end**************//
#endif
    if (valveDue && sleepCount == 0 && !dryRunDetected && !phaseFailureDetected && !onHold && !lowPhaseCurrentDetected) {
        // upon completing first delay start period sleep , switch ON fertigation valve
        if (fieldValve[fieldList[0]].status == ON && fieldValve[fieldList[0]].isFertigationEnabled && fieldValve[fieldList[0]].fertigationStage == wetPeriod) {
            myMsDelay(100);
            fertigationValveControl = ON; // switch on fertigation valve for given field after start period
            if (!fieldValve[8].isConfigured && !fieldValve[9].isConfigured && !fieldValve[10].isConfigured && !fieldValve[11].isConfigured) { // 9-12 used as injector
                // Injector code
                // Initialize all count to zero
                injector1OnPeriodCnt = CLEAR;
                injector2OnPeriodCnt = CLEAR;
                injector3OnPeriodCnt = CLEAR;
                injector4OnPeriodCnt = CLEAR;

                injector1OffPeriodCnt = CLEAR; 
                injector2OffPeriodCnt = CLEAR;
                injector3OffPeriodCnt = CLEAR;
                injector4OffPeriodCnt = CLEAR;

                injector1CycleCnt = CLEAR;
                injector2CycleCnt = CLEAR;
                injector3CycleCnt = CLEAR;
                injector4CycleCnt = CLEAR;

                // Initialize Injectors values to configured values
                injector1OnPeriod = fieldValve[fieldList[0]].injector1OnPeriod; 
                injector2OnPeriod = fieldValve[fieldList[0]].injector2OnPeriod; 
                injector3OnPeriod = fieldValve[fieldList[0]].injector3OnPeriod; 
                injector4OnPeriod = fieldValve[fieldList[0]].injector4OnPeriod; 

                injector1OffPeriod = fieldValve[fieldList[0]].injector1OffPeriod; 
                injector2OffPeriod = fieldValve[fieldList[0]].injector2OffPeriod; 
                injector3OffPeriod = fieldValve[fieldList[0]].injector3OffPeriod; 
                injector4OffPeriod = fieldValve[fieldList[0]].injector4OffPeriod; 

                injector1Cycle = fieldValve[fieldList[0]].injector1Cycle;
                injector2Cycle = fieldValve[fieldList[0]].injector2Cycle;
                injector3Cycle = fieldValve[fieldList[0]].injector3Cycle;
                injector4Cycle = fieldValve[fieldList[0]].injector4Cycle;

                // Initialize injector cycle
                if(injector1OnPeriod > 0) {
                    injector1Control = ON;
                    injector1OnPeriodCnt++;
                }
                if(injector2OnPeriod > 0) {
                    injector2Control = ON;
                    injector2OnPeriodCnt++;
                }
                if(injector3OnPeriod > 0) {
                    injector3Control = ON;
                    injector3OnPeriodCnt++;
                }
                if(injector4OnPeriod > 0) {
                    injector4Control = ON;
                    injector4OnPeriodCnt++;
                }
            }
            // Do action for all simultaneous valve
            iterator = 0;
            while(fieldList[iterator] != 255 && iterator < fieldCount) {
                fieldValve[fieldList[iterator]].fertigationStage = injectPeriod;
                iterator++;
            }

            if (fieldValve[fieldList[0]].fertigationValveInterrupted) {
                // Do action for all simultaneous valve
                iterator = 0;
                while(fieldList[iterator] != 255 && iterator < fieldCount) { 
                    fieldValve[fieldList[iterator]].fertigationValveInterrupted = false;
                    iterator++;
                }
                remainingFertigationOnPeriod = readRemainingFertigationOnPeriodFromEeprom();
                sleepCount = remainingFertigationOnPeriod; // Calculate SleepCounnt after fertigation interrupt due to power off
            }
            else {
                sleepCount = fieldValve[fieldList[0]].fertigationONperiod; // calculate sleep count for fertigation on period 
            }
            // Do action for all simultaneous valve
            iterator = 0;
            while(fieldList[iterator] != 255 && iterator < fieldCount) {
                myMsDelay(100);
                saveFertigationValveStatusIntoEeprom(eepromAddress[fieldList[iterator]], &fieldValve[fieldList[iterator]]);
                myMsDelay(100);
                iterator++;
            }
            saveActiveSleepCountIntoEeprom(); // Save current valve on time 
            myMsDelay(100);
            /***************************/
            sendSms(SmsFert5, userMobileNo, fieldListRequired); // Acknowledge user about successful Fertigation started action
        #ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
            setBCDdigit(0x05,0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
            setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
        #endif
            /***************************/
        #ifdef DEBUG_MODE_ON_H
            //********Debug log#start************//
            transmitStringToDebug("actionsOnSleepCountFinish_FertStart_OUT\r\n");
            //********Debug log#end**************//
        #endif
        }
        // Upon completing fertigation on period sleep, switch off fertigation valve
        else if (fieldValve[fieldList[0]].status == ON && fieldValve[fieldList[0]].isFertigationEnabled && fieldValve[fieldList[0]].fertigationStage == injectPeriod) {
            myMsDelay(100);
            fertigationValveControl = OFF; // switch off fertigation valve for given field after on period
            if (!fieldValve[8].isConfigured && !fieldValve[9].isConfigured && !fieldValve[10].isConfigured && !fieldValve[11].isConfigured) { // 9-12 used as injector
                //Switch off all Injectors after completing fertigation on Period
                injector1Control = OFF;
                injector2Control = OFF;
                injector3Control = OFF;
                injector4Control = OFF;
            }
            // Do action for all simultaneous valve
            iterator = 0;
            while(fieldList[iterator] != 255 && iterator < fieldCount) {
                fieldValve[fieldList[iterator]].fertigationStage = flushPeriod;
                fieldValve[fieldList[iterator]].fertigationInstance--;
                if(fieldValve[fieldList[iterator]].fertigationInstance == 0) {
                    fieldValve[fieldList[iterator]].isFertigationEnabled = false;
                }
                myMsDelay(100);
                saveFertigationValveValuesIntoEeprom(eepromAddress[fieldList[iterator]], &fieldValve[fieldList[iterator]]);
                myMsDelay(100);
                iterator++;
            }
            sleepCount = fieldValve[fieldList[0]].onPeriod - (fieldValve[fieldList[0]].fertigationDelay + fieldValve[fieldList[0]].fertigationONperiod); // calculate sleep count for on period of Valve 
            myMsDelay(100);
            saveActiveSleepCountIntoEeprom(); // Save current valve on time 
            myMsDelay(100);
            /***************************/
            sendSms(SmsFert6, userMobileNo, fieldListRequired); // Acknowledge user about successful Fertigation stopped action
        #ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
            setBCDdigit(0x05,0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider 
            setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
        #endif
            /***************************/
            /*Send sms*/
        #ifdef DEBUG_MODE_ON_H
            //********Debug log#start************//
            transmitStringToDebug("actionsOnSleepCountFinish_FertSTOP_OUT\r\n");
            //********Debug log#end**************//
        #endif
        }
        // upon completing entire field valve on period switch off field valve
        else if (fieldValve[fieldList[0]].status == ON) {
            /************************Debug************************/
            //sendSms(SmsKey2, userMobileNo, fieldListRequired); // Acknowledge user about successful Fertigation stopped action
            /************************Debug************************/
            // Reset De-active Valve list
            iterator = 0;
            while(iterator < fieldCount) {
                lastFieldList[iterator] = 255;
                iterator++;
            }
            // Do action for all simultaneous valve
            iterator = 0;
            while(fieldList[iterator] != 255 && iterator < fieldCount) {
                fieldValve[fieldList[iterator]].status = OFF;  // make status of all simultaneous valve to off
                lastFieldList[iterator] = fieldList[iterator]; // copy list to switch off all simultaneous valves after switching next simultaneous valves
                if (fieldValve[fieldList[iterator]].cyclesExecuted == fieldValve[fieldList[iterator]].cycles) {
                        fieldValve[fieldList[iterator]].cyclesExecuted = 1; //Cycles execution begin after valve due for first time
                }
                else {
                    fieldValve[fieldList[iterator]].cyclesExecuted++; //Cycles execution record
                }
                myMsDelay(100);
                saveIrrigationValveOnOffStatusIntoEeprom(eepromAddress[fieldList[iterator]], &fieldValve[fieldList[iterator]]);
                myMsDelay(100);
                saveIrrigationValveCycleStatusIntoEeprom(eepromAddress[fieldList[iterator]], &fieldValve[fieldList[iterator]]);
                myMsDelay(100);
                if(fieldValve[fieldList[iterator]].isFertigationEnabled) {
                    fieldValve[fieldList[iterator]].fertigationStage = OFF;
                    myMsDelay(100);
                    saveFertigationValveStatusIntoEeprom(eepromAddress[fieldList[iterator]], &fieldValve[fieldList[iterator]]);
                    myMsDelay(100);
                }
                iterator++;
            }
            valveDue = false;
            valveExecuted = true;                                       // Valve successfully executed
            //startFieldNo = fieldList[0];  //+1;                         // scan from same field no.
            nxtPriority = fieldValve[fieldList[0]].priority + 1;    // scan for next priority.
            myMsDelay(100);
            //saveIrrigationValveNoIntoEeprom(field_No);  ****************************************        
        }
    #ifdef DEBUG_MODE_ON_H
        //********Debug log#start************//
        transmitStringToDebug("actionsOnSleepCountFinish_IRRISTOP_OUT\r\n");
        //********Debug log#end**************//
    #endif
    }
    else if (onHold) {
        myMsDelay(100);
        onHold = false;
        if (fieldValve[fieldList[0]].status == ON) {
            if (!fieldValve[fieldList[0]].isConfigured) {  // Disable Irrigation and (Fertigation if in Inject period)
                iterator = 0;
                while(fieldList[iterator] != 255 && iterator < fieldCount) {
                    fieldValve[fieldList[iterator]].status = OFF;
                    myMsDelay(100);
                    saveIrrigationValveOnOffStatusIntoEeprom(eepromAddress[fieldList[iterator]], &fieldValve[fieldList[iterator]]);
                    myMsDelay(100);
                    iterator++;
                }
                if (fieldValve[fieldList[0]].fertigationStage == injectPeriod) {
                    fertigationValveControl = OFF; // switch off fertigation valve for given field after on period
					if (!fieldValve[8].isConfigured && !fieldValve[9].isConfigured && !fieldValve[10].isConfigured && !fieldValve[11].isConfigured) { // 9-12 used as injector
                        //Switch off all Injectors after completing fertigation on Period
                        injector1Control = OFF;
                        injector2Control = OFF;
                        injector3Control = OFF;
                        injector4Control = OFF;
                    }
                    while(fieldList[iterator] != 255 && iterator < fieldCount) {
                        fieldValve[fieldList[iterator]].fertigationStage = OFF;
                        saveFertigationValveStatusIntoEeprom(eepromAddress[fieldList[iterator]], &fieldValve[fieldList[iterator]]);
                        myMsDelay(100);
                        iterator++;
                    }
                }
                valveDue = false;
                valveExecuted = true;					 // complete valve for hold
                //startFieldNo = field_No+1;               // scan for next field no.																								  
                myMsDelay(100);
                // Reset De-active Valve list
                iterator = 0;
                while(iterator < fieldCount) {
                    lastFieldList[iterator] = 255;
                    iterator++;
                }
                iterator = 0;
                while(fieldList[iterator] != 255 && iterator < fieldCount) {
                    lastFieldList[iterator] = fieldList[iterator]; // copy list to switch off all simultaneous valves after switching next simultaneous valves			   
                    iterator++;
                }
            }// Only Disable Fertigation SMS
            else if (fieldValve[fieldList[0]].fertigationStage == wetPeriod) {
                sleepCount = readActiveSleepCountFromEeprom();
                sleepCount = (sleepCount + (fieldValve[fieldList[0]].onPeriod - fieldValve[fieldList[0]].fertigationDelay)); // Calculate Sleep count after fertigation on hold operation  
                saveActiveSleepCountIntoEeprom();
                myMsDelay(100);
            } // Only Disable Fertigation SMS
            else if (fieldValve[fieldList[0]].fertigationStage == injectPeriod) {
                fertigationValveControl = OFF; // switch off fertigation valve for given field after on period
                if (!fieldValve[8].isConfigured && !fieldValve[9].isConfigured && !fieldValve[10].isConfigured && !fieldValve[11].isConfigured) { // 9-12 used as injector
                    //Switch off all Injectors after completing fertigation on Period
                    injector1Control = OFF;
                    injector2Control = OFF;
                    injector3Control = OFF;
                    injector4Control = OFF;
                }
                // Do action for all simultaneous valve
                iterator = 0;
                while(fieldList[iterator] != 255 && iterator < fieldCount) {
                    fieldValve[fieldList[iterator]].fertigationStage = OFF;
                    myMsDelay(100);
                    saveFertigationValveStatusIntoEeprom(eepromAddress[fieldList[iterator]], &fieldValve[fieldList[iterator]]);
                    myMsDelay(100);
                    iterator++;
                }
                sleepCount = readActiveSleepCountFromEeprom();
                sleepCount = (sleepCount + (fieldValve[fieldList[0]].onPeriod - (fieldValve[fieldList[0]].fertigationDelay + fieldValve[fieldList[0]].fertigationONperiod))); // Calculate Sleep count during fertigation hold operation
                saveActiveSleepCountIntoEeprom();
                myMsDelay(100);
            }
        }
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("actionsOnSleepCountFinish_IRRIHOLD_OUT\r\n");
    //********Debug log#end**************//
#endif
    }
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("actionsOnSleepCountFinish_OUT\r\n");
    //********Debug log#end**************//
#endif
}
/*************actionsOnSleepCountFinish#End**********/

/*************actionsOnDueValve#Start**********/
/*************************************************************************************************************************

This function is called to do actions on due valve
The purpose of this function is to perform actions after valve is due.

***************************************************************************************************************************/
void actionsOnDueValve(unsigned char field_No) {
    unsigned char index = CLEAR;
    _Bool valveMatched = false;
    _Bool valveSwitched = false;
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("actionsOnDueValve_IN\r\n");
    //********Debug log#end**************//
#endif
    wetSensor = false;
    /*********Debug**********/
    strncpy(decodedString,"TIME",4);
    extractReceivedSms();
    /*********Debug**********/
    if (!parallelValveFetched) {
        fetchParallelValveList(field_No);  // find the list of valves which are due/set for same time and on period
    }
    parallelValveFetched = false;
    // Check if Moisture sensor is wet (Sensor 1 (0-11)is selected for all fields)
    if (isFieldMoistureSensorWet(field_No)) {  //Skip current valve execution and go for next
        wetSensor = true;
        valveDue = false;
        // Do action for all simultaneous valve
        iterator = 0;
        while(fieldList[iterator] != 255 && iterator < fieldCount) {
            fieldValve[fieldList[iterator]].status = OFF;
            fieldValve[fieldList[iterator]].cyclesExecuted = fieldValve[fieldList[iterator]].cycles;
            myMsDelay(100);
            getDueDate(fieldValve[fieldList[iterator]].offPeriod); // calculate next due date of valve
            myMsDelay(100);
            fieldValve[fieldList[iterator]].nextDueDD = (unsigned char)dueDD;
            fieldValve[fieldList[iterator]].nextDueMM = dueMM;
            fieldValve[fieldList[iterator]].nextDueYY = dueYY;
            myMsDelay(100);
            saveIrrigationValveOnOffStatusIntoEeprom(eepromAddress[fieldList[iterator]], &fieldValve[fieldList[iterator]]);
            myMsDelay(100);
            saveIrrigationValveCycleStatusIntoEeprom(eepromAddress[fieldList[iterator]], &fieldValve[fieldList[iterator]]);
            myMsDelay(100);
            saveIrrigationValveDueTimeIntoEeprom(eepromAddress[fieldList[iterator]], &fieldValve[fieldList[iterator]]);
            myMsDelay(100);
            iterator++;
        }
        //startFieldNo = field_No;  //+1;                             // scan from same field no.
        nxtPriority = fieldValve[field_No].priority + 1;        // scan for next priority.
        /***************************/
        sendSms(SmsIrr6, userMobileNo, fieldListRequired); // Acknowledge user about Irrigation not started due to wet field detection						
    #ifdef SMS_DELIVERY_REPORT_ON_H
        sleepCount = 2; // Load sleep count for SMS transmission action
        sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
        setBCDdigit(0x05,0);
        deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
        setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
    #endif
        /***************************/
    } 
    // All phase present
    else if (!phaseFailure()){
        myMsDelay(100);
        // Do action for all simultaneous valve
        iterator = 0;
        while(fieldList[iterator] != 255 && iterator < fieldCount) {
            activateValve(fieldList[iterator]); // Activate valve for given field
            myMsDelay(100);
            iterator++;
        }
        iterator = 0;
        while(fieldList[iterator] != 255 && iterator < fieldCount) {
            for (index = 0; lastFieldList[index] != 255; index++) {
                if (fieldList[iterator]==lastFieldList[index]) {
                    valveMatched = true; 
                    break; // skip sms for activating already active valve
                }
            }
            if (valveMatched == false) {
                valveSwitched = true;  // at least one new valve activated send sms for activating new valve
            }
            valveMatched = false;
            iterator++;
        }
        if (valveSwitched) {  // Send Sms for activating new valve
            valveSwitched = false;
            if(moistureSensorFailed) {
                moistureSensorFailed = false;
                /***************************/
                sendSms(SmsMS1, userMobileNo, commonActiveFieldNoRequired);
            #ifdef SMS_DELIVERY_REPORT_ON_H
                sleepCount = 2; // Load sleep count for SMS transmission action
                sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
                setBCDdigit(0x05,0);
                deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
                setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
            #endif
                /***************************/
            }
            else {
                /***************************/
                sendSms(SmsIrr4, userMobileNo, commonActiveFieldNoRequired);   // Acknowledge user about successful Irrigation started action
            #ifdef SMS_DELIVERY_REPORT_ON_H
                sleepCount = 2; // Load sleep count for SMS transmission action
                sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
                setBCDdigit(0x05,0);
                deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
                setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
            #endif
                /***************************/
            }    
        }
        /************************Debug************************/
        // for field no. 01 to 09
        if (field_No<9){
            temporaryBytesArray[0] = 48; // To store field no. of valve in action 
            temporaryBytesArray[1] = field_No + 49; // To store field no. of valve in action 
        }// for field no. 10 to 12
        else if (field_No > 8 && field_No < 12) {
            temporaryBytesArray[0] = 49; // To store field no. of valve in action 
            temporaryBytesArray[1] = field_No + 39; // To store field no. of valve in action 
        }
        sendSms(SmsKey2, userMobileNo, fieldNoRequired); // Acknowledge user about successful Fertigation stopped action
        /************************Debug************************/
        //Switch ON Fertigation valve interrupted due to power on same day
        if (fieldValve[field_No].fertigationStage == injectPeriod) {
            powerOnMotor(); // Power ON Motor
            myMsDelay(1000);
            fertigationValveControl = ON;
            if (!fieldValve[8].isConfigured && !fieldValve[9].isConfigured && !fieldValve[10].isConfigured && !fieldValve[11].isConfigured) { // 9-12 used as injector
                // Injector code
                // Initialize all count to zero
                injector1OnPeriodCnt = CLEAR;
                injector2OnPeriodCnt = CLEAR;
                injector3OnPeriodCnt = CLEAR;
                injector4OnPeriodCnt = CLEAR;

                injector1OffPeriodCnt = CLEAR; 
                injector2OffPeriodCnt = CLEAR;
                injector3OffPeriodCnt = CLEAR;
                injector4OffPeriodCnt = CLEAR;

                injector1CycleCnt = CLEAR;
                injector2CycleCnt = CLEAR;
                injector3CycleCnt = CLEAR;
                injector4CycleCnt = CLEAR;

                // Initialize Injectors values to configured values
                injector1OnPeriod = fieldValve[field_No].injector1OnPeriod; 
                injector2OnPeriod = fieldValve[field_No].injector2OnPeriod; 
                injector3OnPeriod = fieldValve[field_No].injector3OnPeriod; 
                injector4OnPeriod = fieldValve[field_No].injector4OnPeriod; 

                injector1OffPeriod = fieldValve[field_No].injector1OffPeriod; 
                injector2OffPeriod = fieldValve[field_No].injector2OffPeriod; 
                injector3OffPeriod = fieldValve[field_No].injector3OffPeriod; 
                injector4OffPeriod = fieldValve[field_No].injector4OffPeriod; 

                injector1Cycle = fieldValve[field_No].injector1Cycle;
                injector2Cycle = fieldValve[field_No].injector2Cycle;
                injector3Cycle = fieldValve[field_No].injector3Cycle;
                injector4Cycle = fieldValve[field_No].injector4Cycle;

                // Initialize injector cycle
                if(injector1OnPeriod > 0) {
                    injector1Control = ON;
                    injector1OnPeriodCnt++;
                }
                if(injector2OnPeriod > 0) {
                    injector2Control = ON;
                    injector2OnPeriodCnt++;
                }
                if(injector3OnPeriod > 0) {
                    injector3Control = ON;
                    injector3OnPeriodCnt++;
                }
                if(injector4OnPeriod > 0) {
                    injector4Control = ON;
                    injector4OnPeriodCnt++;
                }
            }
            /***************************/
            sendSms(SmsFert5, userMobileNo, fieldListRequired); // Acknowledge user about successful Fertigation started action
        #ifdef SMS_DELIVERY_REPORT_ON_H
            sleepCount = 2; // Load sleep count for SMS transmission action
            sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
            setBCDdigit(0x05,0);
            deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider           
            setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
        #endif
            /***************************/
        }
        else if (valveExecuted) { // DeActivate previous executed field valve
            iterator = 0;
            while(lastFieldList[iterator] != 255 && iterator < fieldCount) {
                for (index = 0; fieldList[index] != 255; index++) {
                    if (lastFieldList[iterator]==fieldList[index]) {
                        valveMatched = true; 
                        break; // skip deactivation for matched valve
                    }
                }
                if (valveMatched == false) {
                    deActivateValve(lastFieldList[iterator]); // Successful Deactivate valve for not matched valve list
                    valveSwitched = true; // at least one new valve deactivated send sms for activating new valve
                    myMsDelay(100);
                }
                valveMatched = false;
                iterator++;
            }
            if (valveSwitched) { // Send Sms for only when deactivating executed
                valveSwitched = false;
                /***************************/
                sendSms(SmsIrr5, userMobileNo, commonInActiveFieldNoRequired); // Acknowledge user about successful Irrigation stopped action
            #ifdef SMS_DELIVERY_REPORT_ON_H
                sleepCount = 2; // Load sleep count for SMS transmission action
                sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission
                setBCDdigit(0x05,0);
                deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider
                setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
            #endif
                /***************************/
            }
            // Reset De-active Valve list
            iterator = 0;
            while(iterator < fieldCount) {
                lastFieldList[iterator] = 255;
                iterator++;
            }
            /*
            last_Field_No = readFieldIrrigationValveNoFromEeprom();
            if(last_Field_No != field_No) { // if not multiple cycles for same valve  ******************************
               deActivateValve(last_Field_No); // Successful Deactivate valve 
            }
            */
            
            valveExecuted = false;            
        } 
        else { // Switch on Motor for First Valve activation
            powerOnMotor(); // Power ON Motor
        }
        // Do action for all simultaneous valve
        iterator = 0;
        while(fieldList[iterator] != 255 && iterator < fieldCount) {
            //Calculate Due date at start of Valve cycle
            if (fieldValve[fieldList[iterator]].cyclesExecuted == fieldValve[fieldList[iterator]].cycles) {
                /******** Calculate and save Field Valve next Due date**********/
                getDueDate(fieldValve[fieldList[iterator]].offPeriod); // calculate next due date of valve
                fieldValve[fieldList[iterator]].nextDueDD = (unsigned char)dueDD;
                fieldValve[fieldList[iterator]].nextDueMM = dueMM;
                fieldValve[fieldList[iterator]].nextDueYY = dueYY;
                myMsDelay(100);
                saveIrrigationValveDueTimeIntoEeprom(eepromAddress[fieldList[iterator]], &fieldValve[fieldList[iterator]]);
                myMsDelay(100);
                /******** Calculate and save Field Valve next Due date**********/
            }
            iterator++;
        }
    }
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("actionsOnDueValve_OUT\r\n");
    //********Debug log#end**************//
#endif
}
/*************actionsOnDueValve#End**********/


/*************deleteUserDataOnRequest#Start**********/
/*************************************************************************************************************************

This function is called to delete user data
The purpose of this function is to delete user data and informed user about deletion

***************************************************************************************************************************/
void deleteUserData(void) {
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("deleteUserData_IN\r\n");
    //********Debug log#end**************//
#endif
    sendSms(SmsSR14, userMobileNo, noInfo);
#ifdef SMS_DELIVERY_REPORT_ON_H
    sleepCount = 2; // Load sleep count for SMS transmission action
    sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission 
    setBCDdigit(0x05,0);
    deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider 
    setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
#endif							   
    systemAuthenticated = false;
    saveAuthenticationStatus();
    for (iterator=0; iterator<10; iterator++) {
        userMobileNo[iterator] = '0';
    }
    saveMobileNoIntoEeprom();
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("deleteUserData_OUT\r\n");
    //********Debug log#end**************//
#endif
}
/*************deleteUserDataOnRequest#End**********/


/*************deleteValveDataOnRequest#Start**********/
/*************************************************************************************************************************

This function is called to delete valve configuration data
The purpose of this function is to delete user configured valve data and informed user about deletion

***************************************************************************************************************************/
void deleteValveData(void) {
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("deleteValveData_IN\r\n");
    //********Debug log#end**************//
#endif
    sendSms(SmsSR14, userMobileNo, noInfo);
#ifdef SMS_DELIVERY_REPORT_ON_H
    sleepCount = 2; // Load sleep count for SMS transmission action
    sleepCountChangedDueToInterrupt = true; // Sleep count needs to read from memory after SMS transmission 
    setBCDdigit(0x05,0);
    deepSleep(); // Sleep until message transmission acknowledge SMS is received from service provider 
    setBCDdigit(0x0F,0); // Blank "." BCD Indication for Normal Condition
#endif							   
    filtrationDelay1 = 0;
    filtrationDelay2 = 0;
    filtrationDelay3 = 0;
    filtrationOnTime = 0;
    filtrationSeperationTime = 0;
    filtrationEnabled = false;
    myMsDelay(100);
    saveFiltrationSequenceData();
    myMsDelay(100);
    for (iterator = 0; iterator < fieldCount; iterator++) {
        msgIndex = CLEAR;
        fieldValve[iterator].status = OFF;
        fieldValve[iterator].isConfigured = false;
        fieldValve[iterator].fertigationDelay = 0;
        fieldValve[iterator].fertigationONperiod = 0;
        fieldValve[iterator].fertigationInstance = 0;
        fieldValve[iterator].fertigationStage = OFF;
        fieldValve[iterator].fertigationValveInterrupted = false;
        fieldValve[iterator].isFertigationEnabled = false;

        saveIrrigationValveOnOffStatusIntoEeprom(eepromAddress[iterator], &fieldValve[iterator]);
        myMsDelay(100);
        saveIrrigationValveConfigurationStatusIntoEeprom(eepromAddress[iterator], &fieldValve[iterator]);
        myMsDelay(100);
        saveFertigationValveValuesIntoEeprom(eepromAddress[iterator], &fieldValve[iterator]);
        myMsDelay(100);
    }
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("deleteValveData_OUT\r\n");
    //********Debug log#end**************//
#endif
}
/*************deleteValveDataOnRequest#End**********/

/*************randomPasswordGeneration#Start**********/
/*************************************************************************************************************************

This function is called to generate 6 digit password
The purpose of this function is to randomly generate password of length 6

***************************************************************************************************************************/
void randomPasswordGeneration(void) {
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("randomPasswordGeneration_IN\r\n");
    //********Debug log#end**************//
#endif
    // Seed the random-number generator
    // with current time so that the
    // numbers will be different every time
    getDateFromGSM();
    srand((unsigned int)(currentDD+currentHour+currentMinutes+currentSeconds));
  
    // Array of numbers
    unsigned char numbers[] = "0123456789";
  
    // Iterate over the range [0, N]
    for (iterator = 0; iterator < 6; iterator++) {
        factryPswrd[iterator] = numbers[rand() % 10]; 
    }
    factryPswrd[6] = '\0';
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("randomPasswordGeneration_OUT\r\n");
    //********Debug log#end**************//
#endif
}

/*************clear gsmResponse string#Start**********/
/*************************************************************************************************************************

This function is called to clear gsm response string
The purpose of this function is to enter null values in gsm response

***************************************************************************************************************************/
void clearGsmResponse(void) {
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("clearGsmResponse_IN\r\n");
    //********Debug log#end**************//
#endif
    // Iterate over the range [0, N]
    for (iterator = 0; iterator < 220; iterator++) {
        gsmResponse[iterator] = '\0'; 
    }
    msgIndex = CLEAR; 
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("clearGsmResponse_OUT\r\n");
    //********Debug log#end**************//
#endif
}

/*************clear StringToDecode string#Start**********/
/*************************************************************************************************************************

This function is called to clear stringToDecode  string
The purpose of this function is to enter null values in stringToDecode

***************************************************************************************************************************/
void clearStringToDecode(void) { 
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("clearStringToDecode_IN\r\n");
    //********Debug log#end**************//
#endif
    // Iterate over the range [0, N]
    for (iterator = 0; iterator < 220; iterator++) {
        stringToDecode[iterator] = '\0'; 
    }
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("clearStringToDecode_OUT\r\n");
    //********Debug log#end**************//
#endif
}

/*************clear Decoded string#Start**********/
/*************************************************************************************************************************

This function is called to clear Decoded string
The purpose of this function is to enter null values in Decoded string

***************************************************************************************************************************/
void clearDecodedString(void) {
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("clearDecodedString_IN\r\n");
    //********Debug log#end**************//
#endif
    // Iterate over the range [0, N]
    for (iterator = 0; iterator < 220; iterator++) {
        decodedString[iterator] = '\0'; 
    }  
#ifdef DEBUG_MODE_ON_H
    //********Debug log#start************//
    transmitStringToDebug("clearDecodedString_OUT\r\n");
    //********Debug log#end**************//
#endif
}