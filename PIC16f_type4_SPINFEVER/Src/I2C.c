///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//I2C通信用関数
//Use device microchip PIC16F1938
//HI-TECH C Compiler for PIC10/12/16 MCUs Version 9.80 in Lite mode
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include<htc.h>
#include"I2C.h"
uint8_t rcv_data[RCV_DATA_LEN]; // 受信データバッファ
uint8_t snd_data[SND_DATA_LEN]; // 送信データバッファ


int8_t rcv_flg; // データ受信回数を保存するフラグ
uint8_t *Sdtp;
uint8_t *Rdtp; 

int8_t AckCheck;
int8_t success;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//I2Cの初期化を行う関数
//  Master  void I2C_init(unsigned char speed); spped = 通信速度(default 0x4f)(Fosc = 32MHz)
//  Slave   void I2C_init(unsigned char add);   add = スレーブ側のアドレス, サイズは1Byte
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef  Master

void I2C_init(uint8_t speed)
#endif
#ifdef  Slave
void I2C_init(uint8_t add)
#endif
{
#ifdef Master
    SSPSTAT = 0x80;
    SSPCON1 = 0x28;
    SSPADD = speed;
    SSPIE = 1; // SSP(I2C)
    BCLIE = 1; // MSSP(I2C)
    PEIE = 1;
    GIE = 1;
    SSPIF = 0;
    BCLIF = 0;
    pinModeSCK = 1;
    pinModeSDA = 1;

#endif
#ifdef Slave
    SSPSTAT = 0x00;
    SSPCON1 = 0x26;
    SEN = 1;
    SSPADD = add << 1;
    SSPMSK = 0xfe;
    SSPIE = 1; // SSP(I2C)
    BCLIE = 1; // MSSP(I2C)
    PEIE = 1; 
    GIE = 1; 
    SSPIF = 0;
    BCLIF = 0;
    pinModeSCK = 1;
    pinModeSDA = 1;
#endif
}
/**************************************Master_Mode********************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Masterの割り込み関数
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef Master

void Master_Interrupt(void) {
    if (SSPIF == 1) { // SSP(I2C)
        if (AckCheck == 1) AckCheck = 0;
        SSPIF = 0; 
    } else if (BCLIF == 1) { // MSSP(I2C)
        BCLIF = 0;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//ans = I2C_Send(adrs,len,*buf) : I2C送信関数
//
//  adrs = 送信先アドレス
//  len  = 送信データの長さ
//  buf  = 送信データのバッファポインタ
//  ans  = 0:正常終了, 1:応答無し, 2:受信拒否
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t I2C_Send(uint8_t adrs, uint8_t len, uint8_t *buf) {
    uint8_t i;
    int8_t ans;

    // (START CONDITION)
    I2C_IdleCheck(0x5);
    SSPCON2bits.SEN = 1;
    I2C_IdleCheck(0x5);
    AckCheck = 1;
    SSPBUF = (uint8_t) (adrs << 1);
    while (AckCheck);
    ans = SSPCON2bits.ACKSTAT;
    if (ans == 0) {
        for (i = 0; i < len; i++) {
            I2C_IdleCheck(0x5);
            AckCheck = 1;
            SSPBUF = (unsigned char) *buf; 
            buf++;
            while (AckCheck);
            ans = SSPCON2bits.ACKSTAT;
            if (ans != 0) {
                ans = 2; 
                success = 1;
                break;
            }
        }
    }
    // (STOP CONDITION)
    I2C_IdleCheck(0x5);
    SSPCON2bits.PEN = 1;
    return ans;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//int I2C_Receive(adrs,len,*buf) : I2C受信関数
//  adrs = 受信先アドレス
//  len  = 受信データの長さ
//  ans  = 0:正常終了, 1:応答無し, 2:送信拒否
//  *buf = 受信データを入れるバッファポインタ
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t I2C_Receive(uint8_t adrs, uint8_t len, uint8_t *buf) {
    //	 uint8_t data = 0 ;

    uint8_t i, ans;

    //(START CONDITION)
    I2C_IdleCheck(0x5);
    SSPCON2bits.SEN = 1;
    I2C_IdleCheck(0x5);
    AckCheck = 1;
    SSPBUF = (char) ((adrs << 1) + 1); // R/W=1
    while (AckCheck); 
    ans = SSPCON2bits.ACKSTAT;
    if (ans == 0) {
        for (i = 0; i < len; i++) {
            I2C_IdleCheck(0x5);
            SSPCON2bits.RCEN = 1;
            I2C_IdleCheck(0x4);
            *(buf + i) = SSPBUF;
            //data = SSPBUF;
            //buf++ ;
            I2C_IdleCheck(0x5);
            if (i = len) SSPCON2bits.ACKDT = 1; // ACK
            else SSPCON2bits.ACKDT = 0; // ACK
            SSPCON2bits.ACKEN = 1; // ACK
        }
    }
    //(STOP CONDITION)
    I2C_IdleCheck(0x5);
    SSPCON2bits.PEN = 1;
    return ans;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//I2C_IdleCheck(char mask) : I2Cの待機関数
//  ACKEN RCEN PEN RSEN SEN R/W BF j
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void I2C_IdleCheck(uint8_t mask) {
    while ((SSPCON2 & 0x1F) | (SSPSTAT & mask));
}
#endif
/***************************************Slave_Mode*********************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Slaveの割り込み関数
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef Slave

void Slave_Interrupt(void) {
    char x;

    /*** SSP(I2C)***/
    if (SSPIF == 1) { //
        if (SSPSTATbits.R_nW == 0) {
            if (SSPSTATbits.D_nA == 0) {
                Rdtp = (char *) rcv_data;
                x = SSPBUF;
                rcv_flg = 0;
            } else {
                *Rdtp = SSPBUF;
                Rdtp++;
                rcv_flg++;
            }
            SSPIF = 0;
            SSPCON1bits.CKP = 1; // SCL
        } else {
            if (SSPSTATbits.BF == 1) {
                Sdtp = (char *) snd_data;
                x = SSPBUF;
                while ((SSPCON1bits.CKP) | (SSPSTATbits.BF));
                SSPBUF = *Sdtp;
                Sdtp++;
                SSPIF = 0;
                SSPCON1bits.CKP = 1; // SCL
            } else {
                if (SSPCON2bits.ACKSTAT == 0) {
                    while ((SSPCON1bits.CKP) | (SSPSTATbits.BF));
                    SSPBUF = *Sdtp; 
                    Sdtp++;
                    SSPIF = 0;
                    SSPCON1bits.CKP = 1; // SCL
                } else {
                    SSPIF = 0;
                }
            }
        }
    }

    /* MSSP(I2C)*/
    if (BCLIF == 1) {
        BCLIF = 0;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//int I2C_ReceiveCheck() : データ受信回数をチェックしてrcv_flgをクリアする関数
//  ans = データ受信回数
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t I2C_ReceiveCheck() {
    int ans;

    ans = 0;
    if (rcv_flg != 0) { // 受信したデータがあったら
        if ((SSPSTATbits.S == 0)&&(SSPSTATbits.P == 1)) {
            ans = rcv_flg;
            rcv_flg = 0;
        }
    }
    return (ans);
}
#endif
