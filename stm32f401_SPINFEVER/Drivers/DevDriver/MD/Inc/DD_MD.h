/* ===Kisarazu RBKN Library===
 *
 * autor          : Oishi
 * version        : v0.10
 * last update    : 20160703
 *
 * **overview***
 * MDの通信プロトコルを定める。
 *
 * ・I2Cのみのサポート
 */

#ifndef __MD_H
#define __MD_H
#include <stdint.h>

#define DD_MD_MAX_DUTY 10000

typedef enum{
  D_MMOD_STANDBY=0,
  D_MMOD_IN_GAME=1,
} DD_MDMode_t;

typedef struct{
  uint8_t add;
  uint8_t snd_data[2];
  uint8_t rcv_data[2];
  DD_MDMode_t mode;
}DD_MDHand_t;

int DD_send2MD(DD_MDHand_t *dmd);
int DD_receive2MD(DD_MDHand_t *dmd);

/*
 *MD handlerを表示。
 *
 *MD(Add:hex):[Fr,Fw,Bw,Br],[duty:dec]
 */
void DD_MDHandPrint(DD_MDHand_t *dmd);

#endif
