/*
 *  Project      : 
 * 
 *  file         : app_remote.c
 *  Description  : 本文件用来处理遥控器线程
 *  LastEditors  : Lee
 *  Date         : 2024年8月25日22:14:49
 *  LastEditTime : 
 */

#include "main.h"
#include "app_remote.h"
#include "drv_remote.h"
#include "module_chassis.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "tim.h"

#define REMOTE_TASK_PERIOD  1

Remote_RemoteControlTypeDef Remote_remoteControlData;
Remote_RemoteDataTypeDef *testdata;



#define MIN_BUZZER_PWM ((uint32_t)10000)
#define MAX_BUZZER_PWM ((uint32_t)20000)
#define MAX_PSC ((uint32_t)1000)
#define C3 131
#define C3s 139
#define D3 147
#define D3s 156
#define E3 165
#define F3 175
#define F3s 185
#define G3 196
#define G3s 208
#define A3 220
#define A3s 233
#define B3 247
#define C4 262
#define C4s 277
#define D4 294
#define D4s 311
#define E4 330
#define F4 349
#define F4s 370
#define G4 392
#define G4s 415
#define A4 440
#define A4s 466
#define B4 494
#define C5 523
#define C5s 554
#define D5 587
#define D5s 622
#define E5 659
#define F5 698
#define F5s 740
#define G5 784
#define G5s 831
#define A5 880
#define A5s 932
#define B5 988
#define C6 1047


unsigned int notes[] = {
                        G3,C4,D4s,D4,C4,D4s,C4,D4,C4,G3s,A3s,G3,
                        G3,C4,D4s,D4,C4,D4s,C4,D4,C4,G3,F3s,F3,
                        F3,G3s,B3,D4,  F3,G3s,B3,C4,
                        C4,D4s,A4s,G4s,G4,A4s,G4s,G4s,G4,G4,B3,C4,
                        C5,C5,B4,A4s,  A4s,D5,C5,G4s,G4,
                        G4,A4s,G4,F4,  F4,G4s,G4,F4,G4
                        };
unsigned int duration[] = {
                          1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5,
                          1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 
                          1, 1, 1, 5,    1, 1, 1, 4,
                          1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5,
                          1, 1, 1, 3,  1, 2, 1, 1, 4,
                          1, 1, 1, 3,  1, 2, 1, 1, 4
                          };

						  
uint32_t Frequency = 0; 
uint32_t CLOCK = 10500000;
int i;

int pwm = MIN_BUZZER_PWM;
int psc = 0;

void buzzer_on(uint16_t note) {
    Frequency = CLOCK / note; // 计算定时器的重载值

    htim4.Init.Prescaler = 0; // 预分频器设置为0
    htim4.Init.Period = Frequency; // 设置定时器的重载值

    // 设置PWM的占空比为50%
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, Frequency / 2);
 
    // 启动PWM
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
}

void noTone(){
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, 0);
    HAL_Delay(30);
}


void playSong(){
    for (i = 0; i < sizeof(notes)/sizeof(notes[0]); i++) {
        Frequency = CLOCK / notes[i]; // 计算定时器的重载值
        htim4.Instance->ARR = Frequency; // 设置定时器的重载值
        __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, Frequency / 2);// 设置PWM的占空比为50%
        HAL_Delay(400*duration[i]);
        noTone();
    }
}						  
						  

/**
  * @brief          Remote task
  * @param          NULL
  * @retval         NULL
  */
void Remote_Task(void const * argument) {

    for(;;) {
        Remote_ControlCom();
				osDelay(REMOTE_TASK_PERIOD);
    }
}


/**
* @brief      遥控器控制切换
* @param      NULL
* @retval     NULL
*/

void Remote_ControlCom() {
    Remote_RemoteDataTypeDef *data = Remote_GetRemoteDataPtr();
		testdata = data;

    switch (data->remote.s[0]) {
		/*	右侧拨杆，选择模式		*/
			
        case Remote_SWITCH_UP: {
            /* 拨杆向上，选择底盘控制模式 */
			HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LED_B_GPIO_Port, LED_B_Pin, GPIO_PIN_SET);
//			playSong();
            Remote_ChissisProcess();
            break;
        }
        case Remote_SWITCH_MIDDLE: {
			HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LED_B_GPIO_Port, LED_B_Pin, GPIO_PIN_SET);
						break;
        }
        case Remote_SWITCH_DOWN: {
			HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LED_B_GPIO_Port, LED_B_Pin, GPIO_PIN_RESET);
            break;
        }
        default:
            break;
    }
}


/**
* @brief      遥控器控制底盘
* @param      本函数智能控制底盘，用于例程演示，不会在实际中使用到，可删除
* @retval     NULL
*/
void Remote_ChissisProcess() {

    Remote_RemoteDataTypeDef *data = Remote_GetRemoteDataPtr();

		
    switch (data->remote.s[1]) {
    /*	选择左侧推杆状态	*/
			
        case Remote_SWITCH_UP: {
						/*	左侧推杆向上，底盘下电	*/
						Chassis_SetChassisMode(Chassis_NULL);
						Chassis_SetChassisRef(NULL,NULL,NULL);
            break;
        }
				
        case Remote_SWITCH_MIDDLE: {
						/*	左侧推杆居中,	控制底盘运动*/
						Chassis_SetChassisMode(Chassis_SEP);
						Chassis_SetChassisRef((float)data->remote.ch[1]  , (float)data->remote.ch[0] ,(float)data->remote.ch[2]);
            break;
        }
				
        case Remote_SWITCH_DOWN: {
            /* 左侧推杆向下,电机下电   */
			
            Chassis_SetChassisMode(Chassis_XTL);
			Chassis_SetChassisRef((float)data->remote.ch[1]  , (float)data->remote.ch[0] , 100);
            break;
        }
        default:
            break;
    }

}


