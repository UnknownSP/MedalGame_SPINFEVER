#ifndef __APP_H
#define __APP_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

/*NO Device mode*/
#define _NO_DEVICE 0

int appTask(void);
int appInit(void);

#define DD_NUM_OF_MD 6
#define DD_NUM_OF_AB 0

#define DD_NUM_OF_LD 0
#define DD_NUM_OF_SS 0
#define DD_USE_ENCODER1 0
#define DD_USE_ENCODER2 0
#define DD_NUM_OF_SV 0

#define USE_PC_CONTROL 1
#define DD_USE_RC 0

#define PIC_TYPE1 0
#define PIC_TYPE2 1
#define PIC_TYPE3 2
#define PIC_TYPE4 3
#define PIC_TYPE6 4
#define PIC_TYPE8 5

#define RIFT_IN 0
#define RIFT_LOWER 1
#define RIFT_UPPER 2
#define LOUNCH_RAIL 0
#define OUT_1 1 //near pall
#define OUT_2 2

typedef enum{
    M_FREE = 0,
    M_FORWARD = 1,
    M_BACKWARD = 2,
    M_BRAKE = 3,
}Motor_direction_t;

typedef enum{
    BL_LAUNCHED = 0,
    BL_SETTING = 1,
    BL_NOBALL = 2,
}BallLaunch_t;

#include "DD_RC.h"
#include "DD_LD.h"
#include "DD_MD.h"
#include "DD_SV.h"
#include "DD_SS.h"

#define I2C_ODMETRY 0

#define CENTRAL_THRESHOLD 4

#define MD_GAIN ( DD_MD_MAX_DUTY / DD_RC_ANALOG_MAX )

#endif