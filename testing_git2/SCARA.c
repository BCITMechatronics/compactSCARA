/*
 * SCARA.c
 *
 *  Created on: May 4, 2022
 *      Author:  Matthew Wonneberg and Jamie Boyd
 */
#include "SCARA.h"

unsigned int gArraySize;
unsigned int gArray_i;
unsigned char arrayNum;
unsigned char byteNum;

// index =0. inputSize = 1. no results
unsigned char binInterp_zeroCount (unsigned char * inputData, unsigned char * outputResults){
    gPosCountL1 = 0;
    gPosCountL2 = 0;
    return 0;
}

// index = 1. inputSize = 1. Results [0] numBytes = 6 [1]errCode , [2,3]encoder L1 counts, [4,5] encoder L2 counts
unsigned char binInterp_getCount (unsigned char * inputData, unsigned char * outputResults){
    outputResults [0] = 6;   // the number of bytes in results, including this byte which is not sent
    outputResults [1] = 0;   // no errors here, code for no errors
    signed int * encPtr = ((signed int *)&outputResults[2]);
    *encPtr++ =  gPosCountL1;
    *encPtr =  gPosCountL2;
    return 1;
}

// index =2. inputSize = 6 [0] = index, [1] = pad byte, [2,3] = encoder L1 count, [4,5] = encoder L2 count. no results
unsigned char binInterp_setMtrs (unsigned char * inputData, unsigned char * outputResults){
    signed int * mtrPtr = (signed int *) & (inputData [2]);
//    signed int L1 = *mtrPtr++;
//    signed int L2 = *mtrPtr;
    // motorsPWMset (L1, L2);
    return 0;
}

// index =3. input size = 1. no results
unsigned char binInterp_eStop (unsigned char * inputData, unsigned char * outputResults){
    eStopSoftware ();
    return 0;
}

// index = 4. input size = 1. no results
unsigned char binInterp_eStopReset  (unsigned char * inputData, unsigned char * outputResults){
    eStopReset ();
    return 0;
}


// index = 5. input size =1. Results [0] = numBytes =4, [1] = errCode = 0, [2,3] = zAxisPos
unsigned char binInterp_zAxisGetPos (unsigned char * inputData, unsigned char * outputResults){
    outputResults [0] = 4;   // the number of bytes when sending results, including this one which is not sent
    outputResults [1] = 0;   // no errors here, code for no errors
    signed int * zPtr = ((signed int *)&outputResults[2]);
    *zPtr =  gZPos;
    return 1;
}

// index = 6. input size =1. no results
unsigned char binInterp_zAxisZero (unsigned char * inputData, unsigned char * outputResults){
    zAxisZero();
    return 0;
}

// index = 7. input size = 4 [0] = index, [1] = pad byte, [2,3] = set z Axis position. No results
unsigned char binInterp_zAxisSetUpper (unsigned char * inputData, unsigned char * outputResults){
    signed int * zPtr = (signed int *) & (inputData [2]);
    zAxisSetUpper (*zPtr);
    return 0;
}

// index = 8 inputSize = 1. No result
unsigned char binInterp_zAxisSetUpperHere(unsigned char * inputData, unsigned char * outputResults){
    zAxisSetUpperHere ();
    return 0;
}

// index = 9. input size = 4 [0] = index, [1] = pad byte, [2,3] = set z Axis position. No results
unsigned char binInterp_zAxisSetLower (unsigned char * inputData, unsigned char * outputResults){
    signed int * zPtr = (signed int *) & (inputData [2]);
    zAxisSetLower (*zPtr);
    return 0;
}

// index = 10 inputSize = 1. No result
unsigned char binInterp_zAxisSetLowerHere(unsigned char * inputData, unsigned char * outputResults){
    zAxisSetLowerHere ();
    return 0;
}


// index = 11. input size = 4 [0] = index, [1] = pad byte, [2,3] = set speed.  Results [0] = numBytes =4, [1] = errCode = 0, [2,3] = actual speed
unsigned char binInterp_zAxisSetSpeed (unsigned char * inputData, unsigned char * outputResults){
    unsigned int * zPtr = (unsigned int *) & (inputData [2]);
    unsigned int setSpeed = zAxisSetSpeed (* zPtr);
    outputResults [0] = 4;   // the number of bytes when sending results, including this one which is not sent
    outputResults [1] = 0;   // no errors here, code for no errors
    zPtr = ((unsigned int *)&outputResults[2]);
    *zPtr = setSpeed;
    return 1;
}

// index = 12. input size = 4 [0] = index, [1] = confirmation request [2,3] = position. No result
unsigned char binInterp_zAxisGoToPos (unsigned char * inputData, unsigned char * outputResults){
   unsigned char rVal = 0;
    unsigned char zPosCode;
    unsigned char doConfirm = inputData [1];
    signed int * zPtr = (signed int *) & (inputData [2]);
    zPosCode = zAxisGoToPos (*zPtr);
    if (doConfirm){
        outputResults [0] = 4;   // the number of bytes when sending results, including this one which is not sent.
        outputResults [1] = zPosCode;   // error code, 0, or 1 for too low, 2 for too high
        rVal = 1;
    }
    return rVal;
}

// index = 13. input size = 4 [0] = index, 1 = pad byte, [2,3] = jog speed/direction. No result
unsigned char binInterp_zAxisJog (unsigned char * inputData, unsigned char * outputResults){
    signed int * zPtr = (signed int *) & (inputData [2]);
    zAxisJog (*zPtr);
    return 0;
}

// index = 14. input size = 1. No result.
unsigned char binInterp_zAxisJogStop  (unsigned char * inputData, unsigned char * outputResults){
    zAxisJogStop ();
    return 0;
}

// index = 15. input size = 6 [0] = index, 1 = pad byte, [2,3] =endAngle1, [4,5] = endAngle2. No result
unsigned char binInterp_moveJ  (unsigned char * inputData, unsigned char * outputResults){
    signed int * mtrPtr = (signed int *) & (inputData [2]);
    signed int endAng1 = *mtrPtr++;
    signed int endAng2 = *mtrPtr;
    scaraStateEnd.scaraPos.theta1 = endAng1;
    scaraStateEnd.scaraPos.theta2 = endAng2;
    sendMoveJ(scaraStateEnd);

    return 0;
}


// index = 16. input size = 10 [0] = index, 1 =armSol, [2,3,4,5] = xPos (float), [6,7,8,9] = yPos. No result
unsigned char binInterp_moveJ_Coord  (unsigned char * inputData, unsigned char * outputResults){
    float j1HoldAng =0;
    float j2HoldAng =0;
    int armSol = (int)inputData[1];
    float * coordPtr = (float *) &inputData[2];
    float xCoord = *coordPtr++;
    float yCoord = *coordPtr;

    // store the desired arm solution (1 for left arm, 0 for right arm
    scaraStateEnd.scaraPos.armSol = armSol;

    // find the cooresponding arm angles based on the desired (x, y) coordinate
    if (!(scaraIkFloat(&j1HoldAng, &j2HoldAng, xCoord, yCoord, &scaraStateEnd))){

        // store the joint angles in pulses
        scaraStateEnd.scaraPos.theta1 = j1HoldAng*PUL_PER_DEG_N70;
        scaraStateEnd.scaraPos.theta2 = j2HoldAng*PUL_PER_DEG_N70;

        // send move command
        sendMoveJ(scaraStateEnd);
    }
    return 0;
}


// index = 17. input size = 10 [0] = index, 1 =armSol, [2,3,4,5] = end xPos (float), [6,7,8,9] = end yPos. No result
unsigned char binInterp_moveL  (unsigned char * inputData, unsigned char * outputResults){

    int armSol = (int)inputData[1];
    float * coordPtr = (float *) &inputData[2];
    float xCoord = *coordPtr++;
    float yCoord = *coordPtr;
    holdLine = initLine(xCoord, yCoord, 0, 0, 0); //xb yb
    SCARA_ROBOT testRobot = scaraInitState(0, 0, armSol,0 ); // x y armSol penPos
    sendMoveL(&testRobot, holdLine);
    return 0;
}

// index = 18. input size = 4 [0] = index,1 =padByte, [2,3] = array Size
unsigned char binInterp_moveCustom  (unsigned char * inputData, unsigned char * outputResults){
    signed int * arraySizePtr = (signed int *) & (inputData [2]);
    gArraySize = *arraySizePtr;
    gArray_i = 0;
//    gArrayNum =0;
    usciA1UartEnableRxInt (0);
 //   usciA1UartInstallRxInt (&array_RxInterrupt);
    usciA1UartEnableRxInt (1);
    __low_power_mode_0();  // when we wake up, data has been received


}

/*
unsigned char array_RxInterrupt (char data){
    unsigned char rVal = 0;
    static char value[2];
    static signed int * valPtr = &value;
    value[byteNum] = data;
    if (byteNum == 1){
        if (gArrayNum ==0){
            posArray1 [gArray_i] = *valPtr;
        } else{
            posArray2 [gArray_i] = *valPtr;
        }
        gArray_i +=1;
        byteNum =0;
        if (gArray_i == gArraySize){
            if (gArrayNum ==0){
                gArray_i =0;
                gArrayNum =1;
            } else{


            }
        }

    }





}


typedef unsigned char (*rxIntFunc)(char);
typedef char (*txIntFunc)(unsigned char*);

usciA1UartInstallTxInt (&DATA_TxInterrupt);
usciA1UartEnableTxInt (1);
__low_power_mode_0();  // when we wake up, data has been sent
usciA1UartInstallTxInt (&binInterp_TxInterrupt); // re-enable usual tx functions
usciA1UartEnableTxInt (1);

*/
