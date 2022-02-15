/*
 * updateTimerB.c
 *
 *  Created on: Nov. 8, 2021
 *      Author: Rinz
 */
#include <msp430.h>
#include <ucsiUart.h>
#include <updateTimerB.h>
#include <quadEncDec.h>
#include <ucsiUart.h>
#include <mdd_driver.h>

#include <string.h>
#include <stdio.h>
#include <math.h>

/**************************************
 * Function: void timerB0Init()
 *
 *purpose: Initialize timerA0 to the correct settings
 *purpose: also configure the port settings
 *
 *returns nothing
 **************************************/
void updateTimerBInit(){

    TB0CCR0 = 50000; //0xFFFE, 0.05sec*2(ID_1 divides by 2) = 0.1sec  = 10Hz update rate
    TB0CCR1 = 0;  // CCR1
    TB0CCTL1 |= (OUTMOD_7) // reset set mode
             | CM_0 // no capture
             | CCIE    // ie
             & ~CCIFG  // clr flags
             & ~COV;

    TB0EX0 &= 0x004;     // bits 000  divide by 5

    TB0CTL = (TBSSEL_2 | ID_3 | MC_1 | TBCLR); // Timer_B0 control register, SMCLK, ID_1 SMCLK/ , up mode

}
/**************************************
 * Function: void updateTimer()
 *
 *purpose: provide a 10Hz or 100000 clk cycle update function
 *         that will be used for updating pwm dutyCycles and updating the PID loop
 *
 *Created Nov 8 2021
 *Created by: Matt W
 *returns nothing
 **************************************/
void updateTimer(){

    _BIS_SR(GIE);
    volatile unsigned int currentPul;
    volatile double error;
    volatile double angJ1Current;
    volatile double voutM1;
    volatile signed int sendPWM;


    if (enterLoop == 1){

        // need to change posCount to long signed int
        angJ1Current = (posCount) * DEG_PER_PUL;

        error = angJ1Desired - angJ1Current;
        if (error < 1  && error > -1) // uncertainty.
            error = 0;

        voutM1 = SLOPE*error;

        sendPWM = round(TRANS_FUNC_V_TO_PWM * voutM1);

        if (error == 0) // stop here
            mddCW(0);

        if (sendPWM > 90) // constrain limits
            sendPWM =90;
        else if (sendPWM < -90)
            sendPWM = -90;

        if (sendPWM < 0 && sendPWM >= -90){
            if (sendPWM > -5 && sendPWM < 0 && error != 0)
                sendPWM  = -5;
            else if (sendPWM <= -55)
                sendPWM = -55;
            sendPWM = -1*sendPWM;
            mddCCW(sendPWM);
        }
        else if (sendPWM > 0 && sendPWM <= 90){
            if (sendPWM < 5 && sendPWM > 0 && error != 0)
                sendPWM  = 5;
            else if (sendPWM > 55)
                sendPWM = 55;
            mddCW(sendPWM);
        }

    }
}

#pragma vector = TIMER0_B1_VECTOR
interrupt void timer0_B1_ISR(void){
// CCIFG is still set here and TB0IV = 0x02
    switch(__even_in_range(TB0IV,2)){ // reading TB0IV clears CCIFG, TB0R is counting up from zero now.
    //case 0: break; // nothing
    case 2:// TB0CCTL1 CCIFG
        updateTimer();

    break; // TA0CCR1
   // case 4: break; // CCR2
   // case 6: break;
    default: break;
    }

//    TB0CCTL1 &= ~CCIFG; // CCR0 auto reset

}
