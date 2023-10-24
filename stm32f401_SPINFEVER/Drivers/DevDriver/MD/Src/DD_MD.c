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
#include <stdint.h>
#include "message.h"
#include "DD_MD.h"
#include "DD_Gene.h"

/*
 * ２バイト送信。
 * [0(7:4)|dir(3:2)|duty[9:8](1:0)][duty[7:0](7:0)]
 *
 * dirは0...free,1...forward,2...back,3...brake
 */

int DD_send2MD(DD_MDHand_t *dmd){
  uint8_t send_data[2];
  const uint8_t sizeof_data = 2;
  
  /* Merge Data merge */
  send_data[0] = dmd->snd_data[0] | (uint8_t)dmd->mode << 7;
  send_data[1] = dmd->snd_data[1];

  /* Send data */
  return DD_I2C1Send(dmd->add, send_data, sizeof_data);
}

int DD_receive2MD(DD_MDHand_t *dmd){
  const uint8_t sizeof_data = 2;

  /* Receive data */
  return DD_I2C1Receive(dmd->add, dmd->rcv_data, sizeof_data);
}

/*
 * *MD handlerを表示。
 *
 * MD(Add:hex):[Fr,Fw,Bw,Br],[duty:dec]
 */

void DD_MDHandPrint(DD_MDHand_t *dmd){
  uint8_t send_data;
  MW_printf("MD(%02x):[", dmd->add);
  switch( dmd->mode ){
  case D_MMOD_STANDBY:
    MW_printf("STANDBY");
    break;
  case D_MMOD_IN_GAME:
    MW_printf("IN_GAME");
    break;
  }
  send_data = dmd->snd_data[0] | (uint8_t)dmd->mode << 7;
  MW_printf("],[%08b][%08b],[%08b][%08b]\n", send_data,dmd->snd_data[1],dmd->rcv_data[0],dmd->rcv_data[1]);
}