#include "main.h"
#include <xc.h>
#include <stdbool.h>
#include <stdint.h>

void init(void);

void main(void) {
    uint8_t com_flg = 0;
    bool ob_enable_flag[4] = {false,false,false,false};
    bool ob_stand_flag[4] = {false,false,false,false};
    bool ob_sit_flag[4] = {false,false,false,false};
    bool ob_status[4] = {false,false,false,false};
    bool ob_recent_status[4] = {false,false,false,false};
    uint32_t ob_error_count[4] = {0,0,0,0};
    bool ob_receive_enable[4] = {true,true,true,true};

    Game_mode game_mode = 0;

    int i;

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
            //マスタによるアウトブロック駆動命令があった場合はフラグをセット
            for(i=0;i<4;i++){
                if((((rcv_data[0] >> i) & 0b00000001) == 1) && ob_receive_enable[i]){
                    ob_enable_flag[i] = true;
                    ob_receive_enable[i] = false;
                }
                if(((rcv_data[0] >> i) & 0b00000001) == 0){
                    ob_receive_enable[i] = true;
                }
                if(((rcv_data[1] >> i) & 0b00000001) == 1){
                    ob_sit_flag[i] = true;
                    ob_stand_flag[i] = false;
                }else if(((rcv_data[1] >> (4+i)) & 0b00000001) == 1){
                    ob_stand_flag[i] = true;
                    ob_sit_flag[i] = false;
                }else{
                    ob_stand_flag[i] = false;
                    ob_sit_flag[i] = false;
                }
            }
            CLRWDT();
        }
        else if (com_flg == 0){
            CLRWDT();
        }

        //ゲーム中の場合
        if(game_mode == IN_GAME){
            //各アウトブロックの動作処理を行う
            for(i=0;i<4;i++){
                if(ob_enable_flag[i] || ob_stand_flag[i]){
                    output(LED,i,1);
                    output(OUT,i,1);
                    ob_enable_flag[i] = false;
                    ob_status[i] = true;
                }
                if((!ob_enable_flag[i] && (input(S_1,i) == 0)) || ob_sit_flag[i]){
                    output(LED,i,0);
                    output(OUT,i,0);
                    output(LED,i,0);
                    output(OUT,i,0);
                    ob_status[i] = false;
                }
                if(ob_recent_status[i] != ob_status[i]){
                    ob_error_count[i] = 0;
                }
                if(ob_status[i]){
                    if(input(S_3,i) == 1){
                        ob_error_count[i] += 1;
                    }else{
                        ob_error_count[i] = 0;
                    }
                    snd_data[1] |= 0b00000001 << i;
                }else{
                    if(input(S_2,i) == 1){
                        ob_error_count[i] += 1;
                    }else{
                        ob_error_count[i] = 0;
                    }
                    snd_data[1] &= 0b11111111 ^ (0b00000001 << i);
                }
                //ソレノイド動作のタイムアウト処理
                if(ob_error_count[i] >= ERROR_TIME_COUNT){
                    snd_data[0] |= 0b00000001 << i;
                }else{
                    snd_data[0] &= 0b11111111 ^ (0b00000001 << i);
                }
                ob_recent_status[i] = ob_status[i];
            }
        }else{
            //ゲームモードでない場合は全ての動作を停止
            output(LED,4,0);
            output(OUT,4,0);
            output(LED,4,0);
            output(OUT,4,0);
            for(i=0;i<4;i++){
                ob_enable_flag[i] = false;
                ob_stand_flag[i] = false;
                ob_sit_flag[i] = false;
                ob_status[i] = false;
                ob_recent_status[i] = false;
                ob_error_count[i] = 0;
                ob_receive_enable[i] = true;
            }
        }

        //センサ情報をマスタに送信
        for(i=0;i<4;i++){
            if(input(S_1,i) == 0){
                snd_data[0] |= 0b00000001 << (i+4);
            }else{
                snd_data[0] &= (0b11111111 ^ (0b00000001 << (i+4)));
            }
            if(input(S_2,i) == 1){
                snd_data[1] |= 0b00000001 << i;
            }else{
                snd_data[1] &= (0b11111111 ^ (0b00000001 << i));
            }
            if(input(S_3,i) == 1){
                snd_data[1] |= 0b00000001 << (i+4);
            }else{
                snd_data[1] &= (0b11111111 ^ (0b00000001 << (i+4)));
            }
        }
    }
}

void init(void){
    uint8_t addr = 0x12;

    // Set oscilation
    OSCCON = 0xF0; //PLL　Enable

    // Set pin mode
    ANSELA = 0x00;
    ANSELB = 0x00;
  
    TRISA = 0xFF;
    TRISB = 0x00;
    WPUB = 0x00;
    TRISCbits.TRISC0 = 1;
    TRISCbits.TRISC1 = 1;
    TRISCbits.TRISC2 = 1;
    TRISCbits.TRISC5 = 1;
    TRISCbits.TRISC6 = 0;
    TRISCbits.TRISC7 = 0;

    output(LED,4,0);
    output(OUT,4,0);

    // Set watch dog
    WDTCON = 0x13;

    I2C_init(addr);//アドレス
}

void output(Output_t type, int num, int output){
    if(type == LED){
        switch (num)
        {
        case 0:
            O1_LED = output;
            break;
        case 1:
            O2_LED = output;
            break;
        case 2:
            O3_LED = output;
            break;
        case 3:
            O4_LED = output;
            break;
        case 4:
            O1_LED = output;
            O2_LED = output;
            O3_LED = output;
            O4_LED = output;
            break;
        }
    }
    if(type == OUT){
        switch (num)
        {
        case 0:
            O1_OUT = output;
            break;
        case 1:
            O2_OUT = output;
            break;
        case 2:
            O3_OUT = output;
            break;
        case 3:
            O4_OUT = output;
            break;
        case 4:
            O1_OUT = output;
            O2_OUT = output;
            O3_OUT = output;
            O4_OUT = output;
            break;
        }
    }
}

int input(Input_t type, int num){
    if(type == S_1){
        switch (num)
        {
        case 0:
            return O1_S_1;
            break;
        case 1:
            return O2_S_1;
            break;
        case 2:
            return O3_S_1;
            break;
        case 3:
            return O4_S_1;
            break;
        }
    }
    if(type == S_2){
        switch (num)
        {
        case 0:
            return O1_S_2;
            break;
        case 1:
            return O2_S_2;
            break;
        case 2:
            return O3_S_2;
            break;
        case 3:
            return O4_S_2;
            break;
        }
    }
    if(type == S_3){
        switch (num)
        {
        case 0:
            return O1_S_3;
            break;
        case 1:
            return O2_S_3;
            break;
        case 2:
            return O3_S_3;
            break;
        case 3:
            return O4_S_3;
            break;
        }
    }
}

void interrupt  HAND(void){
    Slave_Interrupt();
}
