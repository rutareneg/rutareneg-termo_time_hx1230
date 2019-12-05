

#include "main.h"
#include <stm32f10x_adc.h>
#include <stm32f10x_flash.h>
#include <stm32f10x_iwdg.h>
#include <stm32f10x_rtc.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
#include <core_cm3.h>
#include "stm32f10x_conf.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_exti.h"

#include "HX_1230_96_68.h"
#include "chip_clock.h"
#include "adc.h"
#include "delay.h"
#include "ds18b20.h"
#include "flash.h"
#include "bat.h"
#include  "strl_vnis_6x8.h"
#include  "strl_vvirh_6x8.h"

#include "celsiy_8x8.h"
#include "key.h"

#include "prtf.h"
#include "time.h"


	void RTC_IRQHandler(void);//����������� 1�+ 1/2� ������
	void TIM4_IRQHandler (void);//���������� ��� ��������� �����
	void  EXTI15_10_IRQHandler(void);//���������� ��� ������
//	void  EXTI9_5_IRQHandler(void);//���������� ��� ������
	u8  men = 0;  //������ ��������
	float TCONTR;  //������ �����������
	//float w = 0;
	u8 L;//������ ����������
    //calib = 0.11;
   // #define adc_calinc  0.01;
	u8 rt=0;//�������� ����������
    u8  contr;

    volatile FLASH_Status FLASHStatus = FLASH_COMPLETE;
    uint32_t timer = 0;//1384850400+14400;
    uint32_t settime;
    void conv_dir (u8 w,char c );
    _Bool S; //������ �������
    u8 S_dat;//��������� ����
    u8 list=0;//��������� ���������
    u8 list_c=0;//��� ��������� ����
    u16 adc_calib;


int main(void)
{

	sysclock = 8;//�������� ������������ � ��������� �������� � ��������
	RCC_SYSCLKConfig( RCC_SYSCLKSource_HSE);
	RCC_HCLKConfig( RCC_SYSCLK_Div2);
	RCC_PLLCmd(DISABLE);
	delay_init(sysclock);//��������� �������� ��� �������� ������ 8�

	iwdt_init (0x06,65000);//������

	IWDG_ReloadCounter();

	key_ini();// ��������� ������ ������ � �� ���
	gpio_spi_Init();
	init_lcd ();

	Init_Timer4 ();//�������������
	blinc_init();
	adc_init ();//�������� ��� 1 �����  � ����� � 1
	RTC_Configuration(); //��������� �����
	Init_USART_GPIO_DS18B20();
	IWDG_ReloadCounter();
		ILL_init();
		//
		read_seatings(); //������ ���������

		if(level_ill>20){level_ill=3;}
		level (level_ill);// ������������� �������

		//if(contr < contr_LCD ||contr > contr_LCD ){contr = contr_LCD/2;} write_seatings();//�������� �� ���������
		//Set_contr(contr);

//**********************************************************************//

		ILL_ON();
	 	lcd_set_rect(2,0,96*4,0);
	 	lcd_set_strs(3,12,16,"ROTARENEG",1);
		delay_s(1);
		ILL_OFF();

	 	IWDG_ReloadCounter();




//****************************************************************************************************************************************//
	 //��������� ��������� �������,���� ����� ����������� ����������� ��������
	 	timer = RTC_GetCounter();
	 	if(timer < tm_def ){RTC_SetCounter_(tm_def);}//���� ������ �� �����
	 	//if(adc_calib < 2||adc_calib >60000){adc_calib = 60; write_seatings(); }

//*******************************************************************************************************************************************//
	 	lcd_clear();


    while(1)
    {  IWDG_ReloadCounter();//���������� �������

    	if(unix_time.hour==0 & unix_time.min==0 & unix_time.sec==0){init_lcd ();Set_contr(contr);}//� 00;00 ����������� �����


     d =  readADC1_W(ADC_Channel_1,adc_calib);
    if(d>600){ILL_ON();}//�������� 9V ������������
    else {ILL_OFF();} //�������� �� �������������!!!
    timer_to_cal (timer, &unix_time);//������������ ��� � ���������� ������� ���� ������ ����

    //	������  ����� � ���������

  	  clock();

//********************��������*******************************//

    	   key_st(10);

    	       if	(meny) { S_dat++; }//������� ��������� ����� ���������, ����������� �������
    	       if	(meny_l) {list=1; S_dat=0; ILL_ON(); lcd_clear();}  //������� ��������� ��������� ��������� ���������� �������


//*******************���� ������***********************************************************//
    	       if(list==1)
    	       {
    	    	   list=2;
    	    while(1){

    	    	if(list==2)  {lcd_set_strs(0,0,8,">�����<",0); if(meny){list=0; IWDG_ReloadCounter();rt=0; S_dat=0; BipStop(); list=0;lcd_clear();ILL_OFF;break;}}
    	    			else  lcd_set_strs(0,0,8," ����� ",0);

    	    	if(list==3)  {lcd_set_strs(1,0,8,">��������<",0);if(meny)list_contrast();}
    	    			else  lcd_set_strs(1,0,8," �������� ",0);

    	    	if(list==4)  {lcd_set_strs(2,0,8,">���������<",0);if(meny)list_ill();}
    	    			else  lcd_set_strs(2,0,8," ��������� ",0);

	    	    if(list==5)  {lcd_set_strs(3,0,8,">���������<",0);if(meny)list_volt();}
	    	   			else  lcd_set_strs(3,0,8," ��������� ",0);

	    	    if(list==6) { lcd_set_strs(4,0,8,">DS18-���������<",0);if(meny)list_eng();}
	    	    		else  lcd_set_strs(4,0,8," DS18-��������� ",0);

	    	    if(list==7)  {lcd_set_strs(5,0,8,">DS18-�����<",0);if(meny)list_out();}
	    	    		else  lcd_set_strs(5,0,8," DS18-����� ",0);

	    	    if(list==8)  {lcd_set_strs(6,0,8,">DS18-�����<",0);if(meny)list_in();}
	    	    		else  lcd_set_strs(6,0,8," DS18-����� ",0);

	    	    if(list==9)  {lcd_set_strs(7,0,8,">���������<",0); if(meny){list=0;rt=0;IWDG_ReloadCounter();S_dat=0; BipStop();
	    	    															lcd_clear();lcd_set_strs(2,21,8,"���������",0);
	    	    															write_seatings(); delay_ms(200); lcd_clear();ILL_OFF;break;}//��������� � �������}
	    	    }
	    	    		else  lcd_set_strs(7,0,8," ��������� ",0);



	    	    key_st(10);
	    	    if	(keys==1){list++; if(list>9){list=2;} IWDG_ReloadCounter();}
	    	    if	(keys==3){list--; if(list<2){list=9;} IWDG_ReloadCounter();}


    	    }
    	       }


    }
}
//*******************��������**************************************************************//

 void list_contrast()

    	       	  {			lcd_clear();
    	           	        lcd_set_strs(1,24,8,"��������",0);
    	           	       	lcd_set_strs(7,0,8,"+",0);
    	           	       	lcd_set_strs(7,36,8,"����",0);
    	           	       	lcd_set_strs(7,90,8,"-",0);

       	         while(1)   {
    	           	       	if	(keys==1){contr++; if(contr>contr_LCD){contr=0;} Set_contr(contr);IWDG_ReloadCounter();}
    	           	       	if	(keys==3){contr--; if(contr>contr_LCD){contr=contr_LCD;} Set_contr(contr);IWDG_ReloadCounter();}



    	           	       	     	_sprtffd(0,buf,contr);
    	           	       	         conv_dir (3,'0');
    	           	       	     	lcd_set_strs(3,24,24,buf,0);

    	           	       	     	delay_ms(50);
    	           	       	     	key_st(10);

    	           	       	  if (meny){rt=0;S_dat=0; lcd_clear(); return;}
    	           	       	     	//meny_driv ();//������� ������� ��� �������
    	           	     	 }

    	       	  }

//*******************��������� �������********************************//

 void list_ill()
	  {
	 	 	 lcd_clear();
    	       lcd_set_strs(0,27,8,"�������",0);
    	       lcd_set_strs(1,21,8,"���������",0);
     	       		     lcd_set_strs(7,0,8,"+",0);
    	       	    	 lcd_set_strs(7,36,8,"����",0);
    	       	    	 lcd_set_strs(7,90,8,"-",0);

    	   while(1)   {
    	       	if	(keys==1){level_ill++;if(level_ill>20)level_ill=0;    level ( level_ill ); IWDG_ReloadCounter(); }
    	       	if	(keys==3){level_ill--;if(level_ill>20)level_ill=20;   level ( level_ill ); IWDG_ReloadCounter(); }



    	       	     	_sprtffd(0,buf,level_ill);
    	       	         conv_dir (2,'0');
    	       	     	lcd_set_strs(3,32,24,buf,0);

    	       	     	delay_ms(50);
    	       	     	key_st(10);
    	       	     if (meny){rt=0;S_dat=0; lcd_clear(); return;}
    	       	     	//meny_driv ();//������� ������� ��� �������
    	   }
	  }

//*******************��������� ����������********************************//
 void list_volt()
{
	 lcd_clear();
	   	   	   	   	   	   	   	lcd_set_strs(7,0,8,"+",0);
	       	           	       	lcd_set_strs(7,36,8,"����",0);
	       	           	       	lcd_set_strs(7,90,8,"-",0);

	   lcd_set_strs(0,15,8,"�����������",0);
	   lcd_set_strs(1,18,8,"����������",0);
	    while(1)   {
	 if (keys==1){adc_calib +=(adc_calib/50); IWDG_ReloadCounter();}
	 if (keys==3){adc_calib -=(adc_calib/50); IWDG_ReloadCounter();}

	 d =  readADC1_W(ADC_Channel_2,adc_calib);
	     	_sprtffd(2,buf,d);
	     	chek_str(buf,4);
	     	//������������ �� 4 ��������
	     	lcd_set_strs(3,12,24,buf,0);
	     	lcd_set_strs(4,80,16,"V",0);

	     	delay_ms(100);
	     	key_st(10);
	     	if (meny){rt=0;S_dat=0; lcd_clear(); return;}
	     	//meny_driv ();//������� ������� ��� �������
	   }
}
//************************��������� ������� 1 **********************************************************//
 void list_out()
   {IWDG_ReloadCounter();
   lcd_clear();
	   set_d_ds18b20 ("  DS18 �������");

	while(1){

	   key_st(10);

	  for(u8 i=0;i<3;i++)
	       					{  _sprtf16(data,temp_out.code[i]);
	       						buf[i*2]= data[0];
	       						buf[i*2+1]= data[1];
	       						buf[i*2+2]= 0;
	       					}

	    lcd_set_strs(1,5,8,buf,0);

	    //if(temp_out.code[0] == 0x28){ //���� ��� 0 �� ���������� ��������
	        GET_RAM_USART_DS18B20(temp_out.code,data);  //�������� ��� ������
	        	       	 //  R = (crc_check(data));
	        	   d = CONV_TEMP_DS18B20 (0.625,TH_,TL_);// ������������� �� ����������
	        	       	   _sprtffd(1,buf,d);
	        	   lcd_set_strs(1,64,8,buf,0);
	        	   lcd_set_sector (1,88,8,8,celsiy_8x8,0);


	        	   if(S_dat == 0 ){for(u8 i=0;i<8;i++) {temp_out.code[i] = 0;}}//������ �� ������� ������� ���������
	        	   /******************************/
	        	   	        	   //��������� ������ �� ������� ��������� ���������

	        	   	   for(u8 i=0;i<8;i++)
	        	   	   	   {buf[i]=skan[i];}

	        	   	   	   	   GET_RAM_USART_DS18B20(buf,data);  //�������� ��� ������
    		  	  	  	  	  d = CONV_TEMP_DS18B20 (0.625,TH_,TL_);// ������������� �� ����������
    		  	  	  	  	  	 _sprtffd(1,buf,d);

	  	  	  	  	 if(S_dat==1)  {lcd_set_strs(3,64,8,buf,1);lcd_set_sector (3,88,8,8,celsiy_8x8,1); for(u8 i=0;i<8;i++) {temp_out.code[i]=skan[i];}
	  	  	  	  	 	 	 }
	    							else
	    							{lcd_set_strs(3,64,8,buf,0);lcd_set_sector (3,88,8,8,celsiy_8x8,0);}
	  	  	  	  	 /***************************************/
	  	  	  	  	 	 	 	 	 for(u8 i=0;i<8;i++)
	  	  	  		        	   	   	   {buf[i]=skan[i+8];}

	  	  	  		        	   	   	   	   GET_RAM_USART_DS18B20(buf,data);  //�������� ��� ������
	  	  	  	    		  	  	  	  	  d = CONV_TEMP_DS18B20 (0.625,TH_,TL_);// ������������� �� ����������
	  	  	  	    		  	  	  	  	  	 _sprtffd(1,buf,d);

	  	  	  		 if(S_dat==2)  {lcd_set_strs(4,64,8,buf,1);lcd_set_sector (4,88,8,8,celsiy_8x8,1); for(u8 i=0;i<8;i++) {temp_out.code[i]=skan[i+8];}}
	  	  	  		    			else
	  	  	  		    			{lcd_set_strs(4,64,8,buf,0);lcd_set_sector (4,88,8,8,celsiy_8x8,0);}

	  	  	  	  	 /************************************************/
	  	  	  		 	 	 	 	 	 for(u8 i=0;i<8;i++)
	  	  	  		  	  	  		        	   	 {buf[i]=skan[i+16];}

	  	  	  		  	  	  		        	  GET_RAM_USART_DS18B20(buf,data);  //�������� ��� ������
	  	  	  		  	  	  	    		  d = CONV_TEMP_DS18B20 (0.625,TH_,TL_);// ������������� �� ����������
	  	  	  		  	  	  	    		  	_sprtffd(1,buf,d);

	  	  	  		  if(S_dat==3)  {lcd_set_strs(5,64,8,buf,1);lcd_set_sector (5,88,8,8,celsiy_8x8,1); for(u8 i=0;i<8;i++) {temp_out.code[i]=skan[i+16];}}
	  	  	  		  	  	  		  else
	  	  	  		  	  	  	    {lcd_set_strs(5,64,8,buf,0);lcd_set_sector (5,88,8,8,celsiy_8x8,0);}

	  	  	  		  	  	  	  	 /************************************************/

	  	  	  		  if(up){S_dat++;if(S_dat>4)S_dat=0;IWDG_ReloadCounter();}
	  	  	  		  if(dn){S_dat--;if(S_dat>4)S_dat=0;IWDG_ReloadCounter();}

	  	  	  	if (meny){rt=0;S_dat=0; lcd_clear(); return;}
	  	  	  	//meny_driv ();//������� ������� ��� �������
	  	  	  		 // if(meny){S_dat=0; save_ok(); }


		}
   }

//************************���� ������� 2 **********************************************************//
 void list_in()

      {IWDG_ReloadCounter();
      lcd_clear();
   	   set_d_ds18b20 ("  DS18  ������");

   	while(1){

   	   key_st(10);

   	  for(u8 i=0;i<4;i++)
   	       					{  _sprtf16(data,temp_in.code[i]);
   	       						buf[i*2]= data[0];
   	       						buf[i*2+1]= data[1];
   	       						buf[i*2+2]= 0;
   	       					}

   	    lcd_set_strs(1,5,8,buf,0);

   	    //if(temp_out.code[0] == 0x28){ //���� ��� 0 �� ���������� ��������
   	        GET_RAM_USART_DS18B20(temp_in.code,data);  //�������� ��� ������
   	        	       	 //  R = (crc_check(data));
   	        	   d = CONV_TEMP_DS18B20 (0.625,TH_,TL_);// ������������� �� ����������
   	        	       	   _sprtffd(1,buf,d);
   	        	   lcd_set_strs(1,64,8,buf,0);
   	        	   lcd_set_sector (1,88,8,8,celsiy_8x8,0);


   	        	   if(S_dat == 0 ){for(u8 i=0;i<8;i++) {temp_in.code[i] = 0;}}//������ �� ������� ������� ���������
   	        	   /******************************/
   	        	   	        	   //��������� ������ �� ������� ��������� ���������

   	        	   	   for(u8 i=0;i<8;i++)
   	        	   	   	   {buf[i]=skan[i];}

   	        	   	   	   	   GET_RAM_USART_DS18B20(buf,data);  //�������� ��� ������
       		  	  	  	  	  d = CONV_TEMP_DS18B20 (0.625,TH_,TL_);// ������������� �� ����������
       		  	  	  	  	  	 _sprtffd(1,buf,d);

   	  	  	  	  	 if(S_dat==1)  {lcd_set_strs(3,64,8,buf,1);lcd_set_sector (3,88,8,8,celsiy_8x8,1); for(u8 i=0;i<8;i++) {temp_in.code[i]=skan[i];}
   	  	  	  	  	 	 	 }

   	    							else
   	    							{lcd_set_strs(3,64,8,buf,0);lcd_set_sector (3,88,8,8,celsiy_8x8,0);}
   	  	  	  	  	 /***************************************/
   	  	  	  	  	 	 	 	 	 for(u8 i=0;i<8;i++)
   	  	  	  		        	   	   	   {buf[i]=skan[i+8];}

   	  	  	  		        	   	   	   	   GET_RAM_USART_DS18B20(buf,data);  //�������� ��� ������
   	  	  	  	    		  	  	  	  	  d = CONV_TEMP_DS18B20 (0.625,TH_,TL_);// ������������� �� ����������
   	  	  	  	    		  	  	  	  	  	 _sprtffd(1,buf,d);

   	  	  	  		 if(S_dat==2)  {lcd_set_strs(4,64,8,buf,1);lcd_set_sector (4,88,8,8,celsiy_8x8,1); for(u8 i=0;i<8;i++) {temp_in.code[i]=skan[i+8];}}
   	  	  	  		    			else
   	  	  	  		    			{lcd_set_strs(4,64,8,buf,0);lcd_set_sector (4,88,8,8,celsiy_8x8,0);}

   	  	  	  	  	 /************************************************/
   	  	  	  		 	 	 	 	 	 for(u8 i=0;i<8;i++)
   	  	  	  		  	  	  		        	   	 {buf[i]=skan[i+16];}

   	  	  	  		  	  	  		        	  GET_RAM_USART_DS18B20(buf,data);  //�������� ��� ������
   	  	  	  		  	  	  	    		  d = CONV_TEMP_DS18B20 (0.625,TH_,TL_);// ������������� �� ����������
   	  	  	  		  	  	  	    		  	_sprtffd(1,buf,d);

   	  	  	  		  if(S_dat==3)  {lcd_set_strs(5,64,8,buf,1);lcd_set_sector (5,88,8,8,celsiy_8x8,1); for(u8 i=0;i<8;i++) {temp_in.code[i]=skan[i+16];}}
   	  	  	  		  	  	  		  else
   	  	  	  		  	  	  	    {lcd_set_strs(5,64,8,buf,0);lcd_set_sector (5,88,8,8,celsiy_8x8,0);}

   	  	  	  		  	  	  	  	 /************************************************/

   	  	  	  		  if(up){S_dat++;if(S_dat>4)S_dat=0;IWDG_ReloadCounter();}
   	  	  	  		  if(dn){S_dat--;if(S_dat>4)S_dat=0;IWDG_ReloadCounter();}
   	  	  	  		  	  	  	  	  /*********************************************/

   	  	  	  	if (meny){rt=0;S_dat=0; lcd_clear(); return;}
   	  	  	  	//meny_driv ();

   		}
      }


//*********************����k ������� 3************************************************************************//
 void list_eng()
      {IWDG_ReloadCounter();
      lcd_clear();
   	   set_d_ds18b20 ("  DS18 ��������");

   	while(1){

   	   key_st(10);

   	  for(u8 i=0;i<4;i++)
   	       					{  _sprtf16(data,temp_eng.code[i]);
   	       						buf[i*2]= data[0];
   	       						buf[i*2+1]= data[1];
   	       						buf[i*2+2]= 0;
   	       					}

   	    lcd_set_strs(1,5,8,buf,0);

   	    //if(temp_out.code[0] == 0x28){ //���� ��� 0 �� ���������� ��������
   	        GET_RAM_USART_DS18B20(temp_eng.code,data);  //�������� ��� ������
   	        	       	 //  R = (crc_check(data));
   	        	   d = CONV_TEMP_DS18B20 (0.625,TH_,TL_);// ������������� �� ����������
   	        	       	   _sprtffd(1,buf,d);
   	        	   lcd_set_strs(1,64,8,buf,0);
   	        	   lcd_set_sector (1,88,8,8,celsiy_8x8,0);


   	        	   if(S_dat == 0 ){for(u8 i=0;i<8;i++) {temp_eng.code[i] = 0;}}//������ �� ������� ������� ���������
   	        	   /******************************/
   	        	   	        	   //��������� ������ �� ������� ��������� ���������

   	        	   	   for(u8 i=0;i<8;i++)
   	        	   	   	   {buf[i]=skan[i];}

   	        	   	   	   	   GET_RAM_USART_DS18B20(buf,data);  //�������� ��� ������
       		  	  	  	  	  d = CONV_TEMP_DS18B20 (0.625,TH_,TL_);// ������������� �� ����������
       		  	  	  	  	  	 _sprtffd(1,buf,d);

   	  	  	  	  	 if(S_dat==1)  {lcd_set_strs(3,64,8,buf,1);lcd_set_sector (3,88,8,8,celsiy_8x8,1); for(u8 i=0;i<8;i++) {temp_eng.code[i]=skan[i];}
   	  	  	  	  	 	 	 }

   	    							else
   	    							{lcd_set_strs(3,64,8,buf,0);lcd_set_sector (3,88,8,8,celsiy_8x8,0);}
   	  	  	  	  	 /***************************************/
   	  	  	  	  	 	 	 	 	 for(u8 i=0;i<8;i++)
   	  	  	  		        	   	   	   {buf[i]=skan[i+8];}

   	  	  	  		        	   	   	   	   GET_RAM_USART_DS18B20(buf,data);  //�������� ��� ������
   	  	  	  	    		  	  	  	  	  d = CONV_TEMP_DS18B20 (0.625,TH_,TL_);// ������������� �� ����������
   	  	  	  	    		  	  	  	  	  	 _sprtffd(1,buf,d);

   	  	  	  		 if(S_dat==2)  {lcd_set_strs(4,64,8,buf,1);lcd_set_sector (4,88,8,8,celsiy_8x8,1); for(u8 i=0;i<8;i++) {temp_eng.code[i]=skan[i+8];}}
   	  	  	  		    			else
   	  	  	  		    			{lcd_set_strs(4,64,8,buf,0);lcd_set_sector (4,88,8,8,celsiy_8x8,0);}

   	  	  	  	  	 /************************************************/
   	  	  	  		 	 	 	 	 	 for(u8 i=0;i<8;i++)
   	  	  	  		  	  	  		        	   	 {buf[i]=skan[i+16];}

   	  	  	  		  	  	  		        	  GET_RAM_USART_DS18B20(buf,data);  //�������� ��� ������
   	  	  	  		  	  	  	    		  d = CONV_TEMP_DS18B20 (0.625,TH_,TL_);// ������������� �� ����������
   	  	  	  		  	  	  	    		  	_sprtffd(1,buf,d);

   	  	  	  		  if(S_dat==3)  {lcd_set_strs(5,64,8,buf,1);lcd_set_sector (5,88,8,8,celsiy_8x8,1); for(u8 i=0;i<8;i++) {temp_eng.code[i]=skan[i+16];}}
   	  	  	  		  	  	  		  else
   	  	  	  		  	  	  	    {lcd_set_strs(5,64,8,buf,0);lcd_set_sector (5,88,8,8,celsiy_8x8,0);}

   	  	  	  		  	  	  	  	 /************************************************/

   	  	  	  		  if(up){S_dat++;if(S_dat>4)S_dat=0;IWDG_ReloadCounter();}
   	  	  	  		  if(dn){S_dat--;if(S_dat>4)S_dat=0;IWDG_ReloadCounter();}
   	  	  	  		  	  	  	  	  /*********************************************/
   	  	  	  	if (meny){rt=0;S_dat=0; lcd_clear(); return;}
   	  	  	  	//meny_driv ();


   		}
      }







//*****************************************************************************************************//
//*******************************************************************************************************//
void meny_driv() {
	if (meny){list=1;rt=0;S_dat=0; lcd_clear(); return;}
	/*lcd_set_strs(2,21,8,"���������",0);delay_ms(200);lcd_clear();*///��������� ����//   L=(84-9*6)/2;
   	       	    // if(meny_l){rt=0;IWDG_ReloadCounter();S_dat=0; BipStop(); lcd_clear();lcd_set_strs(2,21,8,"���������",0);
   	      	     //   write_seatings(); delay_ms(200); list=0;lcd_clear();}//��������� � �������
   	       	 }


//******************************************************************************************************//
void conv_dir (u8 w, char c ){//������ ����������, ���������� ������� ����� 0 ��� ������
		   	   	   	   u8 i=0; //********������ �����
	    	 			//u8 cl[10];//**��������� ������oo
	    	 			while(buf[i]!=0)// ���������� ������ ������
	    	 			{i++;}

	    	 		if(i < w){buf[w+1]=0;
	    	 			while(i>0)// ������������ �����������
	    	 			{w--;i--; buf[w]= buf[i];}
	    	 			while(w>0){w--; buf[w] = c;}
	    	 				}
	  					}
//*******************************************************************************************************//
void RTC_IRQHandler(void)//������ ����� �� ���������� 1�
 {
     if (RTC_GetITStatus(RTC_IT_SEC) != RESET)
     {
         //* Clear the RTC Second interrupt
         RTC_ClearITPendingBit(RTC_IT_SEC);
         timer = RTC_GetCounter();           //�������� �������� ��������
         RTC_WaitForLastTask();
        //if(S)S=0;
      //	else S=1;
         S=1;
         TIM_Cmd (TIM4, ENABLE);
     }

    // IWDG_ReloadCounter();//���������� �������
 }

//*****************������ �������***********************************************************************************//
void TIM4_IRQHandler (void)
{     if(TIM_GetITStatus(TIM4, TIM_IT_Update)== SET)

      	{
      		TIM_Cmd (TIM4, DISABLE);
      		TIM_ClearITPendingBit (TIM4, TIM_IT_Update );
      		//
      		S=0;

      	} //IWDG_ReloadCounter();//���������� �������

}

//************************���������  �����*******************************************************************************//

//********************************************************************************************************************//
 void clock()
 {
	 IWDG_ReloadCounter();
	 if(rt==0) {lcd_set_rect(2,0,96*3,0);rt=1;}//��������� ����� �������

	 			_sprtffd(0,buf,unix_time.mday);
	 	    	conv_dir (2,'0');
	 	    	lcd_set_strs(0,20,8,"/",0);
	 	 if(S_dat==3) {lcd_set_strs(pos_mday,buf,1);if(keys==1){timer+= 86400;}
	 	 	 	 	 	 	 	 	 	 	 	 	 if(keys==3){timer-= 86400;}
	 	 	 	 	 	 	 	 	 	 	 	 	 RTC_SetCounter_(timer);}
	 		else lcd_set_strs(pos_mday,buf,0);

	 	 	 	 lcd_set_strs(0,38,8,"/",0);
	 	    	_sprtffd(0,buf,unix_time.mon);
	 	    	conv_dir (2,'0');
	 	 if(S_dat==4) {lcd_set_strs(pos_mon,buf,1); if(keys==1){unix_time.mon++; timer = cal_to_timer (&unix_time); }
	 	 	 	 	 	 	 	 	 	 	 	 	if(keys==3){unix_time.mon--; timer = cal_to_timer (&unix_time); }
	 	 	 	 	 	 	 	 	 	 	 	 	RTC_SetCounter_(timer); }
	 	 	 else lcd_set_strs(pos_mon,buf,0);

	 	 	 	 _sprtffd(0,buf,unix_time.year);
	 	 	    	conv_dir (2,'0');
	 	if(S_dat==5) {lcd_set_strs(pos_year,buf,1); if(keys==1){unix_time.year++; timer = cal_to_timer (&unix_time); }
	 	 	    		 	 	 	 	 	 	 	if(keys==3){unix_time.year--; timer = cal_to_timer (&unix_time); }
	 	 	    		 	 	 	 	 	 	 	RTC_SetCounter_(timer); }
	 	 	 	 else lcd_set_strs(pos_year,buf,0);

	 	 switch(unix_time.wday)
	 		    	   					{
	 		    	   				case 0 : lcd_set_strs(pos_dn,"���",0);break;
	 		    	   				case 1 : lcd_set_strs(pos_dn,"���",0);break;
	 		    	   				case 2 : lcd_set_strs(pos_dn,"���",0);break;
	 		    	   				case 3 : lcd_set_strs(pos_dn,"���",0);break;
	 		    	   				case 4 : lcd_set_strs(pos_dn,"���",0);break;
	 		    	   				case 5 : lcd_set_strs(pos_dn,"���",0);break;
	 		    	   				case 6 : lcd_set_strs(pos_dn,"���",0);break;
	 		    	   				default : return;
	 		    	   					}
	 	lcd_set_strs(0,0,8,"*",0);
	 	lcd_set_strs(0,90,8,"*",0);
//***************************************************//
 if(timer < tm_def+50000) //����� �� �����������
	   {lcd_set_strs(1,0,8,"���������� �����",1);}
 else 	lcd_set_strs(1,0,8,"*--------------*",0);
//**************************************************//

	// 	lcd_set_strs(2,0,8,"*",0);
	// 	lcd_set_strs(3,0,8,"*",0);
	//	lcd_set_strs(4,0,8,"*",0);
	// 	lcd_set_strs(2,90,8,"*",0);
	// 	lcd_set_strs(3,90,8,"*",0);
	// 	lcd_set_strs(4,90,8,"*",0);

	    	 _sprtffd(0,buf,unix_time.hour);
	    	 conv_dir (2,'0');

	    	 lcd_set_strs(2,40,24,":",1);

	 if(S_dat==2) {lcd_set_strs(2,8,24,buf,0);if(keys==1) {timer+=3600;}
	  	  	  	  	  	  	  	  	  	  	  	 if(keys==3) {timer-=3600;}
	  	  	  	  	  	  	  	  	  	  	  	  	RTC_SetCounter_(timer);}
	 	 	 else  lcd_set_strs(2,8,24,buf,1);

	    	 _sprtffd(0,buf,unix_time.min);
	    	 conv_dir (2,'0');

	  if(S_dat==1) {lcd_set_strs(2,56,24,buf,0);if(keys==1) {timer+=60; }
	    	 	  	  	  	  	  	  	  	  	 if(keys==3) {timer-=60; }
	    	 	  	  	  	  	  	  	  	  	  	 RTC_SetCounter_(timer);}
	  	  	  else  lcd_set_strs(2,56,24,buf,1);


	  	  	 // lcd_set_strs(3,0,16,"OUT",0);
	  	  	   T_out(5,0,16,0);
	  	  	   lcd_set_strs(5,45,16,"S",0);
	  	  	   T_in(5,55,16,0);
	    	   lcd_set_strs(7,45,16,"D",0);
	    	   T_eng(7,55,16,0);
	    	   Bat(7,0,16,0);

	    	   //**����� �� ���������� �� ������� ��� ���  ���������
	    	  	    	     	 if(S){lcd_set_strs(2,40,24,":", 1);if(!S_dat){__WFI();}}//������ ����� � �����

	    	  	    	     	 	 if(!S)
	    	  	    	     	       	   {
	    	  	    	     	 		 	lcd_set_rect(2,46,4,0);//������� ����� � �����
	    	  	    	 				    lcd_set_rect(3,46,4,0);
	    	  	    	 				    lcd_set_rect(4,46,5,0);
	    	  	    	 				   if(!S_dat){__WFI();}//���� �� � ���� �� �����
	    	  	    	 				    }

	    	  	    	     	 	 if(S_dat>5){ S_dat = 0;BipStop();}

 }
//************************************************************************************************************************//

 //************************���������  �����*******************************************************************************//


  //*******�������**********//
void Bat(u8 y,u8 x, u8 h,u8 c)
{u8 l;IWDG_ReloadCounter();
				if(h==8)l=6;
      	    	if(h==16)l=8;
      	    	if(h==24)l=16;
      	    	if(h==32)l=24;
      	    	l=x+l*4;
	d =  readADC1_W(ADC_Channel_2,adc_calib);
      	_sprtffd(2,buf,d);
      	chek_str(buf,4);
      	//������������ �� 4 ��������
      //	lcd_write_cmd(0x40);
      	lcd_set_strs(y,x,h,buf,c);
      	lcd_set_strs(y,l,h,"V",0);


}


//******************����������� � ������******************************//
void T_out(u8 y,u8 x, u8 h,u8 c)//D ��� �������
{u8 l;IWDG_ReloadCounter();
		if(h==8)l=6;
    	if(h==16)l=8;
    	if(h==24)l=16;
    	if(h==32)l=24;
    	l=x+l*4;

	if(temp_out.code[0] == 0x28){ //���� ��� 0 �� ���������� ��������
    GET_RAM_USART_DS18B20(temp_out.code,data);  //�������� ��� ������
    	       	 //  R = (crc_check(data));
    	   temp_out.d = CONV_TEMP_DS18B20 (0.625,TH_,TL_);// ������������� �� ����������
    	       	   _sprtffd(1,buf,temp_out.d);
    	       	   chek_str(buf,4);
    	   lcd_set_strs(y,x,h,buf,c);
    	   lcd_set_sector (y,l,8,8,celsiy_8x8,0);
    	   lcd_set_rect(y+1,l,8,1);

   } else {lcd_set_strs(y,x,h,"----",0);}//������ �� ����������
}
//*********************************************************************************************************************//
//******************����������� �******************************//
void T_in(u8 y,u8 x, u8 h,u8 c)
  {u8 l;IWDG_ReloadCounter();
  	  	  	  	if(h==8)l=6;
  	  	  	  	if(h==16)l=8;
    	       	if(h==24)l=16;
    	       	if(h==32)l=24;
    	       	l=x+l*4;
	if(temp_in.code[0] == 0x28){ //���� ��� 0 �� ���������� ��������
    GET_RAM_USART_DS18B20(temp_in.code,data);  //�������� ��� ������
    	 //  R = (crc_check(data));
    	   temp_in.d = CONV_TEMP_DS18B20 (0.625,TH_,TL_);// ������������� �� ����������
    	   _sprtffd(1,buf,temp_in.d);
    	   chek_str(buf,4);
    	   lcd_set_strs(y,x,h,buf,c);
    	   lcd_set_sector (y,l,8,8,celsiy_8x8,c);
    	   lcd_set_rect(y+1,l,8,1);
  } else {lcd_set_strs(y,x,h,"----",c);}//������ �� ����������
  }
//*********************************************************************************************************************//
//*****************����������� ����***************************//


 void T_eng(u8 y,u8 x, u8 h,u8 c)
	    {u8 l;IWDG_ReloadCounter();
	    	  	  	if(h==8)l=6;
	    	  	  	if(h==16)l=8;
	      	       	if(h==24)l=16;
	      	       	if(h==32)l=24;
	      	       	l=x+l*4;
	  	if(temp_eng.code[0] == 0x28){ //���� ��� 0 �� ���������� ��������
	      GET_RAM_USART_DS18B20(temp_eng.code,data);  //�������� ��� ������
	      	 //  R = (crc_check(data));
	      	   temp_eng.d = CONV_TEMP_DS18B20 (0.625,TH_,TL_);// ������������� �� ����������
	      	   _sprtffd(1,buf,temp_eng.d);
	      	   chek_str(buf,4);
	      	   lcd_set_strs(y,x,h,buf,c);
	      	   lcd_set_sector (y,l,8,8,celsiy_8x8,c);
	      	   lcd_set_rect(y+1,l,8,1);
	    } else {lcd_set_strs(y,x,h,"----",c);}//������ �� ����������
	    }


//**********************����������� ��������**************************************************************************//

 void set_d_ds18b20 (unsigned char set_d[16])//set_d ����� ���������
  {	IWDG_ReloadCounter();
  u8 N ;//����������� �������� ���������
  	  	  	lcd_set_strs(0,0,8,set_d,0);//��������
  	  	  	//lcd_set_strs(2,42,8,"ID",0);//

     		lcd_set_strs(7,32,8,"������",0);
     		lcd_set_sector(7,0,8,8,strl_vnis_6x8,0);
     		lcd_set_sector(7,88,8,8,strl_vvirh_6x8,0);

     		N = Skan_1_wire();

     	while(N) {N--;
     		for(u8 i=0;i<3;i++)
     					{  _sprtf16(data,skan[i+(8*N)]);
     						buf[i*2]= data[0];
     						buf[i*2+1]= data[1];
     						buf[i*2+2]= 0;
     					}

     		lcd_set_strs(3+N,5,8,"<",0);
     		lcd_set_strs(3+N,11,8,buf,0);
     		lcd_set_strs(3+N,41,8,">",0);


     	}


  }




//************************��������*************************************************************************//


 void save_ok ()  {  	 IWDG_ReloadCounter();
	 lcd_clear();//   L=(84-6*6)/2;
   	lcd_set_strs(3,33,8,"�����!",0);
      delay_ms(1000); list++;lcd_clear();

  }

 //********************************************************************************************************//
 void no_save ()
 {	IWDG_ReloadCounter();
	 lcd_clear();//   L=(84-14*6)/2;
	 lcd_set_strs(3,6,8,"�� ����������!",0);
       delay_ms(1000);list++;lcd_clear();
 }
 //**********************************************************************************************************//

 void messag (unsigned char mess[16])
  {	IWDG_ReloadCounter();
	 lcd_clear();//   L=(84-9*6)/2;
 	 lcd_set_strs(2,0,8,mess,0);
        delay_ms(1000);list++;lcd_clear();
  }

 /****************************************************************************************/
 	void write_seatings()
 	{
 		IWDG_ReloadCounter();
 		FLASH_Unlock();
 		         FLASH_ClearFlag (FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);//чистим флги
 		           FLASHStatus = FLASH_ErasePage (adr_start);
 		           adr = adr_start;//��������� ������ ������ ������


 		           FLASH_Write (temp_in.code,sizeof(temp_in.code));
 		           FLASH_Write (temp_out.code,sizeof(temp_out.code));
 		           FLASH_Write(temp_eng.code,sizeof(temp_eng.code));
 		          FLASHStatus = FLASH_ProgramHalfWord(adr, level_ill);adr+=2;
 		           FLASHStatus = FLASH_ProgramHalfWord(adr, adc_calib);adr+=2;
 		          FLASHStatus = FLASH_ProgramHalfWord(adr, contr );

 		        FLASH_Lock();
 		    }

 //*****************************************************************************************//
 	void FLASH_Write (uint16_t *p, u8 R)
 	{
 		while  (R--)
 				        { IWDG_ReloadCounter();
 				        FLASHStatus = FLASH_ProgramHalfWord(adr, *p++); adr +=2;}
 	}

 /*****************************************************************************************/
 	void read_seatings()
 	{
 		IWDG_ReloadCounter();
 		adr = adr_start;//��������� ������ ������ ������

 	FLASH_read(temp_in.code, sizeof(temp_in.code));
 	FLASH_read(temp_out.code, sizeof(temp_out.code));
 	FLASH_read(temp_eng.code, sizeof(temp_eng.code));
  	level_ill=*((uint16_t*)adr);adr+=2;
 	adc_calib=*((uint16_t*)adr);adr+=2;
 	contr=*((uint16_t*)adr);

 	if(temp_eng.code[0] == 0x28){
 		//Reset_USART_DS18B20();//�������� ������ ����
 		Get_CFG_USART_DS18B20(temp_eng.code,data);
 		convert_atl_ath ();
 	    temp_eng.TCONTR = CONV_TEMP_DS18B20 (0.625,TH_,TL_);	}// �������� ������� ������������� �� ����������

 	if(temp_in.code[0] == 0x28){ //���� ��� 0 �� ���������� ��������
 		//Reset_USART_DS18B20();//�������� ������ ����
 		Get_CFG_USART_DS18B20(temp_in.code,data);
 		convert_atl_ath ();
 	    temp_in.TCONTR = CONV_TEMP_DS18B20 (0.625,TH_,TL_);}	// �������� ������� ������������� �� ����������


 	}
 //*****************************************************************************************//
 	void FLASH_read(uint16_t *p, u8 R)
 	{
 		 while(R--)
 			    {IWDG_ReloadCounter();//���������� �������
 			     *(p++)=*((uint16_t*)adr); adr+=2;
 			   }
 	}

 	//*************************************************************************************************//
 	 void convert_atl_ath ()
 	 {
 		 	 	 TL_ = ATH_;
 		      	  TL_ <<= 4;
 		 		  TH_= ATH_;
 		 		  TH_ >>= 4 ;
 		 	 if(ATH_ > 0x7D){TH_ |= 0xF8;}
 	 }

 	//************************************************************************************************//
 	void set_def_list()
 	{
 		IWDG_ReloadCounter();
 		lcd_set_strs(pos_hour,"00:00", 0);
 			lcd_set_strs(pos_mday,"00/00/0000",0);
 			lcd_set_strs(pos_lin,"M",0);
 			lcd_set_strs(pos_lout,"�",0);
 			lcd_set_strs(pos_leng,"D",0);

 	}
 	 //***********************************************************************************************//
 	void set_2t()					//������ ��������� �����
 	{	IWDG_ReloadCounter();
 		if(S){lcd_set_strs(1,34,24,":", 0);}
 				else
 				{ 	lcd_set_rect(1,40,4,1);//������� �����
 				    lcd_set_rect(2,40,4,1);
 				    lcd_set_rect(3,40,5,1);
 				}
 	}

 //************************************************************************************************************//




//***********************************************************************************************************//
 	void  EXTI15_10_IRQHandler(void)
 	 {
 	 	 	 EXTI_ClearITPendingBit(EXTI_Line10|EXTI_Line11|EXTI_Line12);

 	 }

 /*	void  EXTI9_5_IRQHandler(void)
 	 	 {
 	 	 	 	 EXTI_ClearITPendingBit(EXTI_Line9);

 	 	 }*/
//***********************************************************************************************************//



//************************************************************************************************************//
