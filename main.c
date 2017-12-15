/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h"
#include <stdio.h>

struct ms_temp{
	int master_step;	//マスターエンコーダカウンタ
	int slave_step;	    //スレイブモータエンコーダカウンタ
	int error_step;	    //マスタスレイブカウンタずれ
    int master_step_1;
    int slave_step_1;
    int error_step_1;
    float duty;         //pwm duty比
    float duty_1;
    float duty_P;
}ms={0,0,0,0,0,0,0,0,0};

float Tx=(1.0/1.0); //制御入力の周期決定
float sin_input=0.0;
int count=0,flag=0;

void setmotorpwm(float duty)
{
	if(duty>1.0) duty = 1.0;
    else if(duty<-1.0) duty = -1.0;
    else;
	
    if(duty > 0){
        PWM_1_WriteCompare1((uint16)(duty*(float)PWM_1_ReadPeriod()));
    }
    else{
        PWM_1_WriteCompare2((uint16)(-duty*(float)PWM_1_ReadPeriod()));
    }
}

CY_ISR(monitor)//モニタリング用割り込み関数
{
	char8 uartbuf[256];
    
    sprintf(uartbuf,"%d , %d\n",ms.error_step,ms.slave_step);
    TeraTerm_PutString(uartbuf);
}

CY_ISR(control)//制御用割り込み関数
{
    static int16 e0=0,e1=0,e2=0,count=0,speed=1000;
    double y = 0;
    count++;
    
	ms.slave_step += QuadDec_1_GetCounter();  //スレイブモータエンコーダ取得
	QuadDec_1_SetCounter(0);				  //スレイブモータエンコーダカウントクリア
    
    if(speed != y){
        y = count/2;
    }
    else{
        y = speed;
        count = 0;
    }
    
	ms.error_step = y - ms.slave_step;	 //ズレ計算
	
    e2=e1;
	e1=e0;
	e0=ms.error_step;
    
    ms.duty += 0.005f*(float)(e0-e1)+0.000f*e0+0.0f*(e0-e1*2+e2);
	setmotorpwm(ms.duty);                //duty比設定
}

void init_psoc(void)
{
	CyDelay(500);		   //コンポーネント起動
	
    PWM_1_Start();
    PWM_1_Init();
    QuadDec_1_Start();
    TeraTerm_Start();
    isr_1_StartEx(monitor); //シリアル通信用割り込み関数
	isr_2_StartEx(control); //制御用割り込み関数
	
    CyGlobalIntEnable;
}

int main(void)
{
    init_psoc();
    for(;;);
}
/* [] END OF FILE */
