#include "main.h"
#include <xc.h>
#include <stdbool.h>

void init(void);

void main(void) {
    uint8_t com_flg = 0;
    bool bumper_flag[4] = {false,false,false,false};
    bool bumper_on_flag[4] = {false,false,false,false};

    bool bumper_enable[4][3] = {
        {false,false,false},
        {false,false,false},
        {false,false,false},
        {true,true,true},
    };

    Game_mode game_mode = 0;
    uint8_t recet_read_bit = 0;
    uint8_t bumper_hit = 0;

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
            //マスタによるバンパー駆動命令があった場合はフラグをセット
            for(i=0;i<4;i++){
                if(((rcv_data[1] >> (4+i)) & 0b00000001) == 1){
                    bumper_enable[i][0] = true;
                    bumper_enable[i][1] = false;
                    bumper_enable[i][2] = true;
                }else{
                    bumper_enable[i][0] = false;
                    if((((rcv_data[1] >> i) & 0b00000001) == 1) && bumper_enable[i][2]){
                        bumper_enable[i][1] = true;
                        bumper_enable[i][2] = false;
                    }
                    if(((rcv_data[1] >> i) & 0b00000001) == 0){
                        bumper_enable[i][1] = false;
                        bumper_enable[i][2] = true;
                    }
                }
            }
            //バンパーのヒット枚数をマスタに送信
            if(((rcv_data[0] >> recet_read_bit) & 0b00000001) == 1){
                snd_data[1] &= 0b11110000;
                snd_data[1] |= bumper_hit;
                bumper_hit = 0;
                if(recet_read_bit == 0){
                    recet_read_bit = 1;
                }else if(recet_read_bit == 1){
                    recet_read_bit = 0;
                }
            }
            snd_data[1] &= 0b11101111;
            snd_data[1] |= recet_read_bit << 4;
            CLRWDT();
        }
        else if (com_flg == 0){
            game_mode = STANDBY;
            CLRWDT();
        }

        //ゲーム中の場合
        if(game_mode == IN_GAME){
            //各バンパーの動作処理を行う
            for(i=0;i<4;i++){
                if((input(S_1,i) == 0 && !bumper_flag[i] && bumper_enable[i][0]) || bumper_enable[i][1]){
                    output(LED,i,1);
                    output(OUT,i,1);
                    bumper_flag[i] = true;
                    bumper_enable[i][1] = false;
                    bumper_on_flag[i] = false;
                    if(!bumper_enable[i][1]){
                        bumper_hit |= 0b00000001 << i;
                    }
                }else if(input(S_2,i) == 0 && bumper_flag[i] && input(S_1,i) == 1){
                    output(LED,i,0);
                    output(OUT,i,0);
                    output(LED,i,0);
                    output(OUT,i,0);
                    bumper_on_flag[i] = true;
                }
                if(input(S_2,i) == 1 && bumper_on_flag[i]){
                    bumper_on_flag[i] = false;
                    bumper_flag[i] = false;
                }
            }
        }else{
            //ゲームモードでない場合は全ての動作を停止
            output(LED,4,0);
            output(OUT,4,0);
            output(LED,4,0);
            output(OUT,4,0);
            for(i=0;i<4;i++){
                bumper_enable[i][0] = false;
                bumper_enable[i][1] = false;
                bumper_enable[i][2] = true;
                bumper_on_flag[i] = false;
                bumper_flag[i] = false;
            }
            bumper_hit = 0;
            recet_read_bit = 0;
        }

        //センサ情報をマスタに送信
        for(i=0;i<4;i++){
            if(input(S_1,i) == 0){
                snd_data[0] |= 0b00000001 << i;
            }else{
                snd_data[0] &= (0b11111111 ^ (0b00000001 << i));
            }
            if(input(S_2,i) == 0){
                snd_data[0] |= 0b00000001 << (i+4);
            }else{
                snd_data[0] &= (0b11111111 ^ (0b00000001 << (i+4)));
            }
        }
    }
}

void init(void){
    uint8_t addr = 0x11;

    // Set oscilation
    OSCCON = 0xF0; //PLL　Enable

    // Set pin mode
    ANSELA = 0x00;
    ANSELB = 0x00;
  
    TRISA = 0xFF;
    TRISB = 0x00;
    WPUB = 0x00;
    TRISCbits.TRISC0 = 0;
    TRISCbits.TRISC1 = 0;
    TRISCbits.TRISC2 = 0;
    TRISCbits.TRISC5 = 0;
    TRISCbits.TRISC6 = 0;
    TRISCbits.TRISC7 = 0;

    output(LED,4,0);
    output(OUT,4,0);
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
            B1_LED = output;
            break;
        case 1:
            B2_LED = output;
            break;
        case 2:
            B3_LED = output;
            break;
        case 3:
            B4_LED = output;
            break;
        case 4:
            B1_LED = output;
            B2_LED = output;
            B3_LED = output;
            B4_LED = output;
            break;
        }
    }
    if(type == OUT){
        switch (num)
        {
        case 0:
            B1_OUT = output;
            break;
        case 1:
            B2_OUT = output;
            break;
        case 2:
            B3_OUT = output;
            break;
        case 3:
            B4_OUT = output;
            break;
        case 4:
            B1_OUT = output;
            B2_OUT = output;
            B3_OUT = output;
            B4_OUT = output;
            break;
        }
    }
}

int input(Input_t type, int num){
    if(type == S_1){
        switch (num)
        {
        case 0:
            return B1_S_1;
            break;
        case 1:
            return B2_S_1;
            break;
        case 2:
            return B3_S_1;
            break;
        case 3:
            return B4_S_1;
            break;
        }
    }
    if(type == S_2){
        switch (num)
        {
        case 0:
            return B1_S_2;
            break;
        case 1:
            return B2_S_2;
            break;
        case 2:
            return B3_S_2;
            break;
        case 3:
            return B4_S_2;
            break;
        }
    }
}

void interrupt  HAND(void){
    Slave_Interrupt();
}
