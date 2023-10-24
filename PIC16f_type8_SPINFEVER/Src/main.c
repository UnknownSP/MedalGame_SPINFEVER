#include "main.h"
#include <xc.h>
#include <stdbool.h>

void init(void);
void SendToMotor(uint16_t speed,uint8_t stat);

void main(void) {
    uint16_t speed = 0;
    int8_t receive_mode = 0;
    uint8_t com_flg = 0;

    uint8_t recent_mode = 0;
    uint8_t now_mode = 0;
    uint32_t mode_change_count = 0;
    bool mode_change_flag = false;
    bool test_flag = true;

    Game_mode game_mode = 0;

    init();

    while(true){

        //I2Cを受信していた場合
        if(I2C_ReceiveCheck()){
            if (com_flg == 0) com_flg = 1;
            //受信データの中身によってゲーム中かどうか判定
            if((rcv_data[0] & 0b10000000) >> 7 == 1){
                game_mode = IN_GAME;
            }else{
                game_mode = STANDBY;
            }
            CLRWDT();
        }
        else if (com_flg == 0){
            CLRWDT();
        }

        //ゲーム中の場合
        if(game_mode == IN_GAME){
            //センターモータの駆動状況をセット
            receive_mode = rcv_data[0] & 0b00000011;
            speed = ((rcv_data[0] & 0b01111100) >> 2) * 32;
        }else{
            //ゲームモードでない場合は全ての動作を停止
            speed = 0;
            now_mode = 0;
            recent_mode = 0;
            mode_change_flag = false;
            mode_change_count = 0;
        }

        //モータの速度をセット
        PWMSet(speed,now_mode);

        if(RC0 == 1){
            TRISA = 0x00;
            PORTA = 0xff;
        }else{
            //TRISA = 0xff;
            PORTA = 0x00;
        }
    }
}

void init(void){
  uint8_t addr = 0x18;

  // Set oscilation
  OSCCON = 0xF0; //PLL　Enable

  // Set pin mode
  ANSELA = 0x00;
  ANSELB = 0x00;

  TRISA = 0x00;
  TRISCbits.TRISC0 = 1;
  
  // Set watch dog
  WDTCON = 0x13;

  I2C_init(addr);//アドレス
  PWMInit();
}

void interrupt  HAND(void){
    Slave_Interrupt();
}
