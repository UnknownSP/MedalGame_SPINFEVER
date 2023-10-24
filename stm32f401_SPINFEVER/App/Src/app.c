#include "app.h"
#include "DD_Gene.h"
#include "DD_RCDefinition.h"
#include "SystemTaskManager.h"
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "MW_GPIO.h"
#include "MW_IWDG.h"
#include "message.h"
#include "MW_flash.h"
#include "constManager.h"

static
int bumper_setting(int num, bool oneshot, bool enable);
static
int bumper_medal_get(bool recet);
static
int outblock_setting(int side, int num, bool enable, bool stand, bool sit);
static 
int center_motor_setting(Motor_direction_t direction, uint8_t duty);
static
int outblock_get_status(int side, int num);
static
int JPrift_get_status(int side, int num);
static
int outball_get_status(int side, int num);
static
int outball_get_num(int side);
static 
BallLaunch_t ball_launch(int side, bool reset);
static
int launchball_set(int side, bool reset);
static
bool JPC_get_status(int side, int *num);
static 
int JPlottery_motor_setting(Motor_direction_t direction, uint8_t duty, int side);
static 
int JPrift_motor_setting(Motor_direction_t direction, uint8_t duty, int side);
static
bool JPrift_set(int side, int target_posi, bool reset);

static int win_medal_coef = 2;

int appInit(void){

	ad_init();

	/*GPIO の設定などでMW,GPIOではHALを叩く*/
	return EXIT_SUCCESS;
}

/*application tasks*/
int appTask(void){
	
	int i;
	static unsigned int win_medal = 0;

	static bool test_flag = false;
	static bool button_flag = true;
	static unsigned int test_duty = 0;
	static unsigned int rift_target = 0;
	static unsigned int JPC_in_num = 0;
	static bool JPC_flag = true;
	static int test_JPC_duty = 0;

	//ユーザーボタンの状態を管理
	if(MW_GPIORead(GPIOCID,GPIO_PIN_13) == 0 & button_flag){
		button_flag = false;
		if(!test_flag){
			test_flag = true;
		}else{
			test_flag = false;
		}
	}
	if(MW_GPIORead(GPIOCID,GPIO_PIN_13) == 1){
		button_flag = true;
	}

	if(test_flag){
		//全てのドライバをゲーム中にセット
		for(i=0;i<DD_NUM_OF_MD;i++){
			g_md_h[i].mode = D_MMOD_IN_GAME;
		}
		//バンパーを有効化
		bumper_setting(4,false,true);
		//バンパーのヒット枚数をセット
		win_medal += bumper_medal_get(false);
		//アウトブロックを有効化
		outblock_setting(0,4,true,false,false);
		outblock_setting(1,4,true,false,false);
		test_duty++;
		if(test_duty >= 30000){
			test_duty = 30000;
		}
		//センターモータを回転
		center_motor_setting(M_BACKWARD,test_duty/3);
		//リフターをセット
		if(JPrift_get_status(0,0) == 1 && JPrift_get_status(0,1) == 1){
			rift_target = 4;
		}
		if(JPrift_set(0,rift_target,false)){
			if(rift_target == 4){
				rift_target = 0;
			}
		}
		//JPCポケットに入賞した場合
		if(JPC_flag){
			if((g_SY_system_counter % 2000) >= 0 && (g_SY_system_counter % 2000 ) < 10){
				srand(g_SY_system_counter);
				//test_JPC_duty = rand()%100;
				//if(rand()%2 == 0){
				//	JPlottery_motor_setting(M_FORWARD,100,0);
				//}else{
				//	JPlottery_motor_setting(M_BACKWARD,100,0);
				//}

				//JPC抽選機を回転
				JPlottery_motor_setting(M_FORWARD,100,0);
			}
			if(JPC_get_status(0, &JPC_in_num)){
				JPC_flag = false;
			}
		}else{
			JPlottery_motor_setting(M_BRAKE,0,0);
		}
		//ボール発射
		ball_launch(0,false);
	}else{
		//全てのドライバをゲーム中から解除
		for(i=0;i<DD_NUM_OF_MD;i++){
			g_md_h[i].mode = D_MMOD_STANDBY;
		}
		//全ての関数及び駆動を初期化
		bumper_setting(4,false,false);
		bumper_medal_get(true);
		outblock_setting(0,4,false,false,false);
		outblock_setting(1,4,false,false,false);
		test_duty = 0;
		center_motor_setting(M_FREE,0);
		win_medal = 0;
		JPrift_set(0,0,true);
		JPrift_motor_setting(M_FREE,0,0);
		JPlottery_motor_setting(M_FREE,0,0);
		JPC_flag = true;
		ball_launch(0,true);
	}

	if( g_SY_system_counter % _MESSAGE_INTERVAL_MS < _INTERVAL_MS ){
		MW_printf("%d",win_medal);
	}
	return EXIT_SUCCESS;
}

static
bool JPrift_set(int side, int target_posi, bool reset){
	//target_posi = 0  lower
	int target_pic = side + 3;
	static now_position[2] = {0,0};
	static rift_count[2] = {0,0};
	int target = 0;
	const time_rift_posi1 = 100;

	if(reset){
		now_position[0] = 0;
		now_position[1] = 0;
		rift_count[0] = 0;
		rift_count[1] = 0;
		g_md_h[target_pic].snd_data[0] &= 0b11111100;
		g_md_h[target_pic].snd_data[1] = 0;
		return true; 
	}

	target = now_position[side] - target_posi;
	switch(target_posi){
	case 0:
		if(JPrift_get_status(side,1) == 0){
			JPrift_motor_setting(M_BACKWARD,100,side);
			return false;
		}else{
			now_position[side] = 0;
			JPrift_motor_setting(M_FREE,0,side);
			return true;
		}
		break;
	case 1:
	case 2:
	case 3:
		if(rift_count[side] >= abs(target)*time_rift_posi1){
			now_position[side] = target_posi;
			rift_count[side] = 0;
			JPrift_motor_setting(M_FREE,0,side);
			return true;
		}else{
			rift_count[side]++;
			if(target < 0){
				JPrift_motor_setting(M_FORWARD,100,side);
			}else{
				JPrift_motor_setting(M_BACKWARD,100,side);
			}
			return false;
		}
		break;
	case 4:
		if(JPrift_get_status(side,2) == 0){
			JPrift_motor_setting(M_FORWARD,100,side);
			return false;
		}else{
			now_position[side] = 4;
			JPrift_motor_setting(M_FREE,0,side);
			return true;
		}
		break;
	}

}

static
int JPrift_get_status(int side, int num){
	// num = 0  in
	// num = 1  rift_lower
	// num = 2  rift_upper
	int target_pic = side + 3;
	int return_data = 0;

	return_data = (g_md_h[target_pic].rcv_data[1] >> num) & 0b00000001;

	return return_data;
}

static
int outball_get_status(int side, int num){
	// num = 0  rail
	// num = 1  near pall
	// num = 2  bound
	int target_pic = side + 3;
	int return_data = 0;

	return_data = (g_md_h[target_pic].rcv_data[1] >> num+3) & 0b00000001;

	return return_data;
}

static
int outball_get_num(int side){
	int i;
	int return_num = 0;

	for(i=0; i<3; i++){
		if(outball_get_status(side,i) == 1){
			return_num++;
		}
	}
	return return_num;
}

static
int launchball_set(int side, bool reset){
	int target_pic = side + 3;
	static bool setting[2] = {false};

	if(reset){
		setting[side] = false;
		g_md_h[target_pic].snd_data[0] &= 0b11110111;
		return 0;
	}

	if(outball_get_status(side, 0) == 1){
		setting[side] = false;
		g_md_h[target_pic].snd_data[0] &= 0b11110111;
		return 0;
	}else if(outball_get_status(side, 1) == 1 && !setting[side]){
		setting[side] = true;
	}else if(outball_get_status(side, 1) == 0 && !setting[side]){
		g_md_h[target_pic].snd_data[0] &= 0b11110111;
		return -1;
	}
	if(setting[side]){
		g_md_h[target_pic].snd_data[0] |= 0b00001000;
		return 1;
	}
}

static 
BallLaunch_t ball_launch(int side, bool reset){
	BallLaunch_t return_state;
	int target_pic = side + 3;
	static bool launch[2] = {false};
	static int launch_count[2] = {0};

	if(reset){
		launchball_set(side,true);
		launch[side] = false;
		launch_count[side] = 0;
		g_md_h[target_pic].snd_data[0] &= 0b11111011;

		return BL_SETTING;
	}

	return_state = BL_SETTING;
	if(!launch[side]){
		if(launchball_set(side,false) == 0){
			launch[side] = true;
			return_state = BL_SETTING;
		}
		if(launchball_set(side,false) == -1){
			return_state = BL_NOBALL;
		}
	}else{
		launch_count[side]++;
		if(launch_count[side] >= 100){
			g_md_h[target_pic].snd_data[0] |= 0b00000100;
		}
		if(launch_count[side] >= 200){
			launch_count[side] = 200;
			g_md_h[target_pic].snd_data[0] &= 0b11111011;
			return_state = BL_LAUNCHED;
		}
	}

	return return_state;
}

static
bool JPC_get_status(int side, int *num){
	// num = 0  nothing
	// num = 1  JPJP
	int target_pic = side + 4;
	int i;

	for(i=1;i<=5;i++){
		if(((g_md_h[target_pic].rcv_data[1] >> i) & 0b00000001) == 1){
			*num = i;
			break;
		}
		*num = 0;
	}

	if((g_md_h[target_pic].rcv_data[1] & 0b00000001) == 1){
		return true;
	}else{
		return false;
	}
}

static 
int JPrift_motor_setting(Motor_direction_t direction, uint8_t duty, int side){
	int target_pic = side + 3;

	g_md_h[target_pic].snd_data[0] &= 0b11111100;
	g_md_h[target_pic].snd_data[0] |= direction;
	if(duty >= 100){
		g_md_h[target_pic].snd_data[1] = (uint8_t)(100.0 * 2.55);
	}else{
		g_md_h[target_pic].snd_data[1] = (uint8_t)((double)duty * 2.55);
	}
	return 0;
}

static 
int JPlottery_motor_setting(Motor_direction_t direction, uint8_t duty, int side){
	int target_pic = side + 4;

	g_md_h[target_pic].snd_data[0] &= 0b11111100;
	g_md_h[target_pic].snd_data[0] |= direction;
	if(duty >= 100){
		g_md_h[target_pic].snd_data[1] = (uint8_t)(100.0 * 2.55);
	}else{
		g_md_h[target_pic].snd_data[1] = (uint8_t)((double)duty * 2.55);
	}
	return 0;
}

static 
int center_motor_setting(Motor_direction_t direction, uint8_t duty){

	g_md_h[PIC_TYPE8].snd_data[0] &= 0b11111100;
	g_md_h[PIC_TYPE8].snd_data[0] |= direction;
	if(duty >= 100){
		g_md_h[PIC_TYPE8].snd_data[1] = (uint8_t)(100.0 * 2.55);
	}else{
		g_md_h[PIC_TYPE8].snd_data[1] = (uint8_t)((double)duty * 2.55);
	}
	return 0;
}

static
int outblock_setting(int side, int num, bool enable, bool stand, bool sit){
	int target_pic = side + 1;

	if(enable){
		if(num == 4){
			g_md_h[target_pic].snd_data[0] |= 0b00001111;
			g_md_h[target_pic].snd_data[1] &= 0b00000000;
		}else{
			g_md_h[target_pic].snd_data[0] |= 0b00000001 << num;
			g_md_h[target_pic].snd_data[1] &= 0b11111111 ^ (0b00000001 << num);
			g_md_h[target_pic].snd_data[1] &= 0b11111111 ^ (0b00000001 << (num+4));
		}
	}else{
		if(num == 4){
			g_md_h[target_pic].snd_data[0] &= 0b11110000;
		}else{
			g_md_h[target_pic].snd_data[0] &= 0b11111111 ^ (0b00000001 << num);
		}
	}
	if(stand){
		if(num == 4){
			g_md_h[target_pic].snd_data[1] |= 0b11110000;
			g_md_h[target_pic].snd_data[1] &= 0b11110000;
		}else{
			g_md_h[target_pic].snd_data[1] |= 0b00000001 << (num+4);
			g_md_h[target_pic].snd_data[1] &= 0b11111111 ^ (0b00000001 << num);
		}
	}else if(sit){
		if(num == 4){
			g_md_h[target_pic].snd_data[1] |= 0b00001111;
			g_md_h[target_pic].snd_data[1] &= 0b00001111;
		}else{
			g_md_h[target_pic].snd_data[1] |= 0b00000001 << num;
			g_md_h[target_pic].snd_data[1] &= 0b11111111 ^ (0b00000001 << (num+4));
		}
	}else{
		if(num == 4){
			g_md_h[target_pic].snd_data[1] &= 0b00000000;
		}else{
			g_md_h[target_pic].snd_data[1] &= 0b11111111 ^ (0b00000001 << num);
			g_md_h[target_pic].snd_data[1] &= 0b11111111 ^ (0b00000001 << (num+4));
		}
	}
}

static
int outblock_get_status(int side, int num){
	int target_pic = side + 1;
	int return_data = 0;

	return_data = (g_md_h[target_pic].rcv_data[1] >> num) & 0b00000001;

	return return_data;
}

static
int bumper_setting(int num, bool oneshot, bool enable){
	if(!oneshot){
		if(num == 4){
			if(enable){
				g_md_h[PIC_TYPE1].snd_data[1] |= 0b11110000;
			}else{
				g_md_h[PIC_TYPE1].snd_data[1] &= 0b00001111;
			}
		}else{
			if(enable){
				g_md_h[PIC_TYPE1].snd_data[1] |= (0x01 << (num+4));
			}else{
				g_md_h[PIC_TYPE1].snd_data[1] &= (0b11111111 ^ (0x01 << (num+4)));
			}
		}
	}else{
		if(num == 4){
			if(enable){
				g_md_h[PIC_TYPE1].snd_data[1] &= 0b00001111;
				g_md_h[PIC_TYPE1].snd_data[1] |= 0b00001111;
			}else{
				g_md_h[PIC_TYPE1].snd_data[1] &= 0b11110000;
			}
		}else{
			if(enable){
				g_md_h[PIC_TYPE1].snd_data[1] &= (0b11111111 ^ (0x01 << (num+4)));
				g_md_h[PIC_TYPE1].snd_data[1] |= (0x01 << num);
			}else{
				g_md_h[PIC_TYPE1].snd_data[1] &= (0b11111111 ^ (0x01 << num));
			}
		}
	}
}

static
int bumper_medal_get(bool recet){
	int recet_bit = 0;
	static int recent_read_bit = 0;
	static bool read_enable = true;
	int i;
	int count = 0;

	if(recet){
		read_enable = true;
		recent_read_bit = 0;
		return 0;
	}

	recet_bit = (g_md_h[PIC_TYPE1].rcv_data[1] >> 4) & 0b00000001;
	if(recet_bit == 0){
		g_md_h[PIC_TYPE1].snd_data[0] |= 0b00000001;
		g_md_h[PIC_TYPE1].snd_data[0] &= 0b11111101;
	}else if(recet_bit == 1){
		g_md_h[PIC_TYPE1].snd_data[0] |= 0b00000010;
		g_md_h[PIC_TYPE1].snd_data[0] &= 0b11111110;
	}
	if(recet_bit != recent_read_bit){
		read_enable = true;
	}
	if(read_enable){
		for(i=0;i<4;i++){
			if(((g_md_h[PIC_TYPE1].rcv_data[1] >> i) & 0b00000001) == 1) count++;
		}
		read_enable = false;
	}else{
		count = 0;
	}
	recent_read_bit = recet_bit;
	return count;
}