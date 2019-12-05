

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


	void RTC_IRQHandler(void);//перерывание 1с+ 1/2с таймер
	void TIM4_IRQHandler (void);//прерывание для секундных точек
	void  EXTI15_10_IRQHandler(void);//прерывания для кнопок
//	void  EXTI9_5_IRQHandler(void);//прерывания для кнопок
	u8  men = 0;  //номера страници
	float TCONTR;  //придел температуры
	//float w = 0;
	u8 L;//длинна переменной
    //calib = 0.11;
   // #define adc_calinc  0.01;
	u8 rt=0;//контроль зачернения
    u8  contr;

    volatile FLASH_Status FLASHStatus = FLASH_COMPLETE;
    uint32_t timer = 0;//1384850400+14400;
    uint32_t settime;
    void conv_dir (u8 w,char c );
    _Bool S; //тикаем точками
    u8 S_dat;//установка даты
    u8 list=0;//системные настройки
    u8 list_c=0;//вид основного окна
    u16 adc_calib;


int main(void)
{

	sysclock = 8;//значение используется в настройке таймеров и задержек
	RCC_SYSCLKConfig( RCC_SYSCLKSource_HSE);
	RCC_HCLKConfig( RCC_SYSCLK_Div2);
	RCC_PLLCmd(DISABLE);
	delay_init(sysclock);//уменьшаем скорость для экономии энерги 8м

	iwdt_init (0x06,65000);//ВАЧДОГ

	IWDG_ReloadCounter();

	key_ini();// настройка портов кнопок и зв сиг
	gpio_spi_Init();
	init_lcd ();

	Init_Timer4 ();//мигаемточками
	blinc_init();
	adc_init ();//астройка ацп 1 канал  и порта ј 1
	RTC_Configuration(); //настройка часов
	Init_USART_GPIO_DS18B20();
	IWDG_ReloadCounter();
		ILL_init();
		//
		read_seatings(); //читаем настройки

		if(level_ill>20){level_ill=3;}
		level (level_ill);// устанавливаем яркость

		//if(contr < contr_LCD ||contr > contr_LCD ){contr = contr_LCD/2;} write_seatings();//контраст по умолчанию
		//Set_contr(contr);

//**********************************************************************//

		ILL_ON();
	 	lcd_set_rect(2,0,96*4,0);
	 	lcd_set_strs(3,12,16,"ROTARENEG",1);
		delay_s(1);
		ILL_OFF();

	 	IWDG_ReloadCounter();




//****************************************************************************************************************************************//
	 //проверяем установку времени,если пусто подкидываем контрольное значение
	 	timer = RTC_GetCounter();
	 	if(timer < tm_def ){RTC_SetCounter_(tm_def);}//пока вачдог не нужен
	 	//if(adc_calib < 2||adc_calib >60000){adc_calib = 60; write_seatings(); }

//*******************************************************************************************************************************************//
	 	lcd_clear();


    while(1)
    {  IWDG_ReloadCounter();//перезапуск вачдога

    	if(unix_time.hour==0 & unix_time.min==0 & unix_time.sec==0){init_lcd ();Set_contr(contr);}//в 00;00 переиниваем экран


     d =  readADC1_W(ADC_Channel_1,adc_calib);
    if(d>600){ILL_ON();}//примерно 9V отрабатываетё
    else {ILL_OFF();} //включать по необходимости!!!
    timer_to_cal (timer, &unix_time);//переобразуеи код в переменные времени часы минуты годы

    //	рисуем  время и календарь

  	  clock();

//********************кнопочки*******************************//

    	   key_st(10);

    	       if	(meny) { S_dat++; }//менюшка установки время календарь, однократное нажатие
    	       if	(meny_l) {list=1; S_dat=0; ILL_ON(); lcd_clear();}  //менюшка установки системные настройки длительное нажатие


//*******************МЕНЮ СПИСОК***********************************************************//
    	       if(list==1)
    	       {
    	    	   list=2;
    	    while(1){

    	    	if(list==2)  {lcd_set_strs(0,0,8,">ВЫХОД<",0); if(meny){list=0; IWDG_ReloadCounter();rt=0; S_dat=0; BipStop(); list=0;lcd_clear();ILL_OFF;break;}}
    	    			else  lcd_set_strs(0,0,8," ВЫХОД ",0);

    	    	if(list==3)  {lcd_set_strs(1,0,8,">КОНТРАСТ<",0);if(meny)list_contrast();}
    	    			else  lcd_set_strs(1,0,8," КОНТРАСТ ",0);

    	    	if(list==4)  {lcd_set_strs(2,0,8,">ПОДСВЕТКА<",0);if(meny)list_ill();}
    	    			else  lcd_set_strs(2,0,8," ПОДСВЕТКА ",0);

	    	    if(list==5)  {lcd_set_strs(3,0,8,">ВОЛЬТМЕТР<",0);if(meny)list_volt();}
	    	   			else  lcd_set_strs(3,0,8," ВОЛЬТМЕТР ",0);

	    	    if(list==6) { lcd_set_strs(4,0,8,">DS18-ДВИГАТЕЛЬ<",0);if(meny)list_eng();}
	    	    		else  lcd_set_strs(4,0,8," DS18-ДВИГАТЕЛЬ ",0);

	    	    if(list==7)  {lcd_set_strs(5,0,8,">DS18-УЛИЦА<",0);if(meny)list_out();}
	    	    		else  lcd_set_strs(5,0,8," DS18-УЛИЦА ",0);

	    	    if(list==8)  {lcd_set_strs(6,0,8,">DS18-САЛОН<",0);if(meny)list_in();}
	    	    		else  lcd_set_strs(6,0,8," DS18-САЛОН ",0);

	    	    if(list==9)  {lcd_set_strs(7,0,8,">СОХРАНИТЬ<",0); if(meny){list=0;rt=0;IWDG_ReloadCounter();S_dat=0; BipStop();
	    	    															lcd_clear();lcd_set_strs(2,21,8,"СОХРАНЯЕМ",0);
	    	    															write_seatings(); delay_ms(200); lcd_clear();ILL_OFF;break;}//сохраняем и выходим}
	    	    }
	    	    		else  lcd_set_strs(7,0,8," СОХРАНИТЬ ",0);



	    	    key_st(10);
	    	    if	(keys==1){list++; if(list>9){list=2;} IWDG_ReloadCounter();}
	    	    if	(keys==3){list--; if(list<2){list=9;} IWDG_ReloadCounter();}


    	    }
    	       }


    }
}
//*******************контраст**************************************************************//

 void list_contrast()

    	       	  {			lcd_clear();
    	           	        lcd_set_strs(1,24,8,"КОНТРАСТ",0);
    	           	       	lcd_set_strs(7,0,8,"+",0);
    	           	       	lcd_set_strs(7,36,8,"МЕНЮ",0);
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
    	           	       	     	//meny_driv ();//двигаем менюшку или выходим
    	           	     	 }

    	       	  }

//*******************настройка яркости********************************//

 void list_ill()
	  {
	 	 	 lcd_clear();
    	       lcd_set_strs(0,27,8,"ЯРКОСТЬ",0);
    	       lcd_set_strs(1,21,8,"ПОДСВЕТКИ",0);
     	       		     lcd_set_strs(7,0,8,"+",0);
    	       	    	 lcd_set_strs(7,36,8,"МЕНЮ",0);
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
    	       	     	//meny_driv ();//двигаем менюшку или выходим
    	   }
	  }

//*******************настройка вольтметра********************************//
 void list_volt()
{
	 lcd_clear();
	   	   	   	   	   	   	   	lcd_set_strs(7,0,8,"+",0);
	       	           	       	lcd_set_strs(7,36,8,"МЕНЮ",0);
	       	           	       	lcd_set_strs(7,90,8,"-",0);

	   lcd_set_strs(0,15,8,"КАЛЛИБРОВКА",0);
	   lcd_set_strs(1,18,8,"ВОЛЬТМЕТРА",0);
	    while(1)   {
	 if (keys==1){adc_calib +=(adc_calib/50); IWDG_ReloadCounter();}
	 if (keys==3){adc_calib -=(adc_calib/50); IWDG_ReloadCounter();}

	 d =  readADC1_W(ADC_Channel_2,adc_calib);
	     	_sprtffd(2,buf,d);
	     	chek_str(buf,4);
	     	//ограничиваеи до 4 символов
	     	lcd_set_strs(3,12,24,buf,0);
	     	lcd_set_strs(4,80,16,"V",0);

	     	delay_ms(100);
	     	key_st(10);
	     	if (meny){rt=0;S_dat=0; lcd_clear(); return;}
	     	//meny_driv ();//двигаем менюшку или выходим
	   }
}
//************************установка датчика 1 **********************************************************//
 void list_out()
   {IWDG_ReloadCounter();
   lcd_clear();
	   set_d_ds18b20 ("  DS18 ВНЕШНИЙ");

	while(1){

	   key_st(10);

	  for(u8 i=0;i<3;i++)
	       					{  _sprtf16(data,temp_out.code[i]);
	       						buf[i*2]= data[0];
	       						buf[i*2+1]= data[1];
	       						buf[i*2+2]= 0;
	       					}

	    lcd_set_strs(1,5,8,buf,0);

	    //if(temp_out.code[0] == 0x28){ //если код 0 то пропускаем действае
	        GET_RAM_USART_DS18B20(temp_out.code,data);  //забираем всю память
	        	       	 //  R = (crc_check(data));
	        	   d = CONV_TEMP_DS18B20 (0.625,TH_,TL_);// пересчитавеем по приращению
	        	       	   _sprtffd(1,buf,d);
	        	   lcd_set_strs(1,64,8,buf,0);
	        	   lcd_set_sector (1,88,8,8,celsiy_8x8,0);


	        	   if(S_dat == 0 ){for(u8 i=0;i<8;i++) {temp_out.code[i] = 0;}}//ничего не выбрано удаляем значениия
	        	   /******************************/
	        	   	        	   //извлекаем данные из массива найденных устройств

	        	   	   for(u8 i=0;i<8;i++)
	        	   	   	   {buf[i]=skan[i];}

	        	   	   	   	   GET_RAM_USART_DS18B20(buf,data);  //забираем всю память
    		  	  	  	  	  d = CONV_TEMP_DS18B20 (0.625,TH_,TL_);// пересчитавеем по приращению
    		  	  	  	  	  	 _sprtffd(1,buf,d);

	  	  	  	  	 if(S_dat==1)  {lcd_set_strs(3,64,8,buf,1);lcd_set_sector (3,88,8,8,celsiy_8x8,1); for(u8 i=0;i<8;i++) {temp_out.code[i]=skan[i];}
	  	  	  	  	 	 	 }
	    							else
	    							{lcd_set_strs(3,64,8,buf,0);lcd_set_sector (3,88,8,8,celsiy_8x8,0);}
	  	  	  	  	 /***************************************/
	  	  	  	  	 	 	 	 	 for(u8 i=0;i<8;i++)
	  	  	  		        	   	   	   {buf[i]=skan[i+8];}

	  	  	  		        	   	   	   	   GET_RAM_USART_DS18B20(buf,data);  //забираем всю память
	  	  	  	    		  	  	  	  	  d = CONV_TEMP_DS18B20 (0.625,TH_,TL_);// пересчитавеем по приращению
	  	  	  	    		  	  	  	  	  	 _sprtffd(1,buf,d);

	  	  	  		 if(S_dat==2)  {lcd_set_strs(4,64,8,buf,1);lcd_set_sector (4,88,8,8,celsiy_8x8,1); for(u8 i=0;i<8;i++) {temp_out.code[i]=skan[i+8];}}
	  	  	  		    			else
	  	  	  		    			{lcd_set_strs(4,64,8,buf,0);lcd_set_sector (4,88,8,8,celsiy_8x8,0);}

	  	  	  	  	 /************************************************/
	  	  	  		 	 	 	 	 	 for(u8 i=0;i<8;i++)
	  	  	  		  	  	  		        	   	 {buf[i]=skan[i+16];}

	  	  	  		  	  	  		        	  GET_RAM_USART_DS18B20(buf,data);  //забираем всю память
	  	  	  		  	  	  	    		  d = CONV_TEMP_DS18B20 (0.625,TH_,TL_);// пересчитавеем по приращению
	  	  	  		  	  	  	    		  	_sprtffd(1,buf,d);

	  	  	  		  if(S_dat==3)  {lcd_set_strs(5,64,8,buf,1);lcd_set_sector (5,88,8,8,celsiy_8x8,1); for(u8 i=0;i<8;i++) {temp_out.code[i]=skan[i+16];}}
	  	  	  		  	  	  		  else
	  	  	  		  	  	  	    {lcd_set_strs(5,64,8,buf,0);lcd_set_sector (5,88,8,8,celsiy_8x8,0);}

	  	  	  		  	  	  	  	 /************************************************/

	  	  	  		  if(up){S_dat++;if(S_dat>4)S_dat=0;IWDG_ReloadCounter();}
	  	  	  		  if(dn){S_dat--;if(S_dat>4)S_dat=0;IWDG_ReloadCounter();}

	  	  	  	if (meny){rt=0;S_dat=0; lcd_clear(); return;}
	  	  	  	//meny_driv ();//двигаем менюшку или выходим
	  	  	  		 // if(meny){S_dat=0; save_ok(); }


		}
   }

//************************поис датчика 2 **********************************************************//
 void list_in()

      {IWDG_ReloadCounter();
      lcd_clear();
   	   set_d_ds18b20 ("  DS18  ВНУТРИ");

   	while(1){

   	   key_st(10);

   	  for(u8 i=0;i<4;i++)
   	       					{  _sprtf16(data,temp_in.code[i]);
   	       						buf[i*2]= data[0];
   	       						buf[i*2+1]= data[1];
   	       						buf[i*2+2]= 0;
   	       					}

   	    lcd_set_strs(1,5,8,buf,0);

   	    //if(temp_out.code[0] == 0x28){ //если код 0 то пропускаем действае
   	        GET_RAM_USART_DS18B20(temp_in.code,data);  //забираем всю память
   	        	       	 //  R = (crc_check(data));
   	        	   d = CONV_TEMP_DS18B20 (0.625,TH_,TL_);// пересчитавеем по приращению
   	        	       	   _sprtffd(1,buf,d);
   	        	   lcd_set_strs(1,64,8,buf,0);
   	        	   lcd_set_sector (1,88,8,8,celsiy_8x8,0);


   	        	   if(S_dat == 0 ){for(u8 i=0;i<8;i++) {temp_in.code[i] = 0;}}//ничего не выбрано удаляем значениия
   	        	   /******************************/
   	        	   	        	   //извлекаем данные из массива найденных устройств

   	        	   	   for(u8 i=0;i<8;i++)
   	        	   	   	   {buf[i]=skan[i];}

   	        	   	   	   	   GET_RAM_USART_DS18B20(buf,data);  //забираем всю память
       		  	  	  	  	  d = CONV_TEMP_DS18B20 (0.625,TH_,TL_);// пересчитавеем по приращению
       		  	  	  	  	  	 _sprtffd(1,buf,d);

   	  	  	  	  	 if(S_dat==1)  {lcd_set_strs(3,64,8,buf,1);lcd_set_sector (3,88,8,8,celsiy_8x8,1); for(u8 i=0;i<8;i++) {temp_in.code[i]=skan[i];}
   	  	  	  	  	 	 	 }

   	    							else
   	    							{lcd_set_strs(3,64,8,buf,0);lcd_set_sector (3,88,8,8,celsiy_8x8,0);}
   	  	  	  	  	 /***************************************/
   	  	  	  	  	 	 	 	 	 for(u8 i=0;i<8;i++)
   	  	  	  		        	   	   	   {buf[i]=skan[i+8];}

   	  	  	  		        	   	   	   	   GET_RAM_USART_DS18B20(buf,data);  //забираем всю память
   	  	  	  	    		  	  	  	  	  d = CONV_TEMP_DS18B20 (0.625,TH_,TL_);// пересчитавеем по приращению
   	  	  	  	    		  	  	  	  	  	 _sprtffd(1,buf,d);

   	  	  	  		 if(S_dat==2)  {lcd_set_strs(4,64,8,buf,1);lcd_set_sector (4,88,8,8,celsiy_8x8,1); for(u8 i=0;i<8;i++) {temp_in.code[i]=skan[i+8];}}
   	  	  	  		    			else
   	  	  	  		    			{lcd_set_strs(4,64,8,buf,0);lcd_set_sector (4,88,8,8,celsiy_8x8,0);}

   	  	  	  	  	 /************************************************/
   	  	  	  		 	 	 	 	 	 for(u8 i=0;i<8;i++)
   	  	  	  		  	  	  		        	   	 {buf[i]=skan[i+16];}

   	  	  	  		  	  	  		        	  GET_RAM_USART_DS18B20(buf,data);  //забираем всю память
   	  	  	  		  	  	  	    		  d = CONV_TEMP_DS18B20 (0.625,TH_,TL_);// пересчитавеем по приращению
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


//*********************поисk датчика 3************************************************************************//
 void list_eng()
      {IWDG_ReloadCounter();
      lcd_clear();
   	   set_d_ds18b20 ("  DS18 ВИГАТЕЛЬ");

   	while(1){

   	   key_st(10);

   	  for(u8 i=0;i<4;i++)
   	       					{  _sprtf16(data,temp_eng.code[i]);
   	       						buf[i*2]= data[0];
   	       						buf[i*2+1]= data[1];
   	       						buf[i*2+2]= 0;
   	       					}

   	    lcd_set_strs(1,5,8,buf,0);

   	    //if(temp_out.code[0] == 0x28){ //если код 0 то пропускаем действае
   	        GET_RAM_USART_DS18B20(temp_eng.code,data);  //забираем всю память
   	        	       	 //  R = (crc_check(data));
   	        	   d = CONV_TEMP_DS18B20 (0.625,TH_,TL_);// пересчитавеем по приращению
   	        	       	   _sprtffd(1,buf,d);
   	        	   lcd_set_strs(1,64,8,buf,0);
   	        	   lcd_set_sector (1,88,8,8,celsiy_8x8,0);


   	        	   if(S_dat == 0 ){for(u8 i=0;i<8;i++) {temp_eng.code[i] = 0;}}//ничего не выбрано удаляем значениия
   	        	   /******************************/
   	        	   	        	   //извлекаем данные из массива найденных устройств

   	        	   	   for(u8 i=0;i<8;i++)
   	        	   	   	   {buf[i]=skan[i];}

   	        	   	   	   	   GET_RAM_USART_DS18B20(buf,data);  //забираем всю память
       		  	  	  	  	  d = CONV_TEMP_DS18B20 (0.625,TH_,TL_);// пересчитавеем по приращению
       		  	  	  	  	  	 _sprtffd(1,buf,d);

   	  	  	  	  	 if(S_dat==1)  {lcd_set_strs(3,64,8,buf,1);lcd_set_sector (3,88,8,8,celsiy_8x8,1); for(u8 i=0;i<8;i++) {temp_eng.code[i]=skan[i];}
   	  	  	  	  	 	 	 }

   	    							else
   	    							{lcd_set_strs(3,64,8,buf,0);lcd_set_sector (3,88,8,8,celsiy_8x8,0);}
   	  	  	  	  	 /***************************************/
   	  	  	  	  	 	 	 	 	 for(u8 i=0;i<8;i++)
   	  	  	  		        	   	   	   {buf[i]=skan[i+8];}

   	  	  	  		        	   	   	   	   GET_RAM_USART_DS18B20(buf,data);  //забираем всю память
   	  	  	  	    		  	  	  	  	  d = CONV_TEMP_DS18B20 (0.625,TH_,TL_);// пересчитавеем по приращению
   	  	  	  	    		  	  	  	  	  	 _sprtffd(1,buf,d);

   	  	  	  		 if(S_dat==2)  {lcd_set_strs(4,64,8,buf,1);lcd_set_sector (4,88,8,8,celsiy_8x8,1); for(u8 i=0;i<8;i++) {temp_eng.code[i]=skan[i+8];}}
   	  	  	  		    			else
   	  	  	  		    			{lcd_set_strs(4,64,8,buf,0);lcd_set_sector (4,88,8,8,celsiy_8x8,0);}

   	  	  	  	  	 /************************************************/
   	  	  	  		 	 	 	 	 	 for(u8 i=0;i<8;i++)
   	  	  	  		  	  	  		        	   	 {buf[i]=skan[i+16];}

   	  	  	  		  	  	  		        	  GET_RAM_USART_DS18B20(buf,data);  //забираем всю память
   	  	  	  		  	  	  	    		  d = CONV_TEMP_DS18B20 (0.625,TH_,TL_);// пересчитавеем по приращению
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
	/*lcd_set_strs(2,21,8,"СЛЕДУЮЩИЙ",0);delay_ms(200);lcd_clear();*///следующее меню//   L=(84-9*6)/2;
   	       	    // if(meny_l){rt=0;IWDG_ReloadCounter();S_dat=0; BipStop(); lcd_clear();lcd_set_strs(2,21,8,"СОХРАНЯЕМ",0);
   	      	     //   write_seatings(); delay_ms(200); list=0;lcd_clear();}//сохраняем и выходим
   	       	 }


//******************************************************************************************************//
void conv_dir (u8 w, char c ){//длинна знакоместа, заполнение пустого места 0 или пробел
		   	   	   	   u8 i=0; //********длинна слова
	    	 			//u8 cl[10];//**временный массивoo
	    	 			while(buf[i]!=0)// определяем длинну строки
	    	 			{i++;}

	    	 		if(i < w){buf[w+1]=0;
	    	 			while(i>0)// конвертируем направление
	    	 			{w--;i--; buf[w]= buf[i];}
	    	 			while(w>0){w--; buf[w] = c;}
	    	 				}
	  					}
//*******************************************************************************************************//
void RTC_IRQHandler(void)//часики вывод по прерыванию 1с
 {
     if (RTC_GetITStatus(RTC_IT_SEC) != RESET)
     {
         //* Clear the RTC Second interrupt
         RTC_ClearITPendingBit(RTC_IT_SEC);
         timer = RTC_GetCounter();           //Получаем значение счётчика
         RTC_WaitForLastTask();
        //if(S)S=0;
      //	else S=1;
         S=1;
         TIM_Cmd (TIM4, ENABLE);
     }

    // IWDG_ReloadCounter();//перезапуск вачдога
 }

//*****************мигаем точками***********************************************************************************//
void TIM4_IRQHandler (void)
{     if(TIM_GetITStatus(TIM4, TIM_IT_Update)== SET)

      	{
      		TIM_Cmd (TIM4, DISABLE);
      		TIM_ClearITPendingBit (TIM4, TIM_IT_Update );
      		//
      		S=0;

      	} //IWDG_ReloadCounter();//перезапуск вачдога

}

//************************выгружаем  время*******************************************************************************//

//********************************************************************************************************************//
 void clock()
 {
	 IWDG_ReloadCounter();
	 if(rt==0) {lcd_set_rect(2,0,96*3,0);rt=1;}//зачерняет место часиков

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
	 		    	   				case 0 : lcd_set_strs(pos_dn,"ПОН",0);break;
	 		    	   				case 1 : lcd_set_strs(pos_dn,"ВТР",0);break;
	 		    	   				case 2 : lcd_set_strs(pos_dn,"СРД",0);break;
	 		    	   				case 3 : lcd_set_strs(pos_dn,"ЧТВ",0);break;
	 		    	   				case 4 : lcd_set_strs(pos_dn,"ПТН",0);break;
	 		    	   				case 5 : lcd_set_strs(pos_dn,"СБТ",0);break;
	 		    	   				case 6 : lcd_set_strs(pos_dn,"ВСК",0);break;
	 		    	   				default : return;
	 		    	   					}
	 	lcd_set_strs(0,0,8,"*",0);
	 	lcd_set_strs(0,90,8,"*",0);
//***************************************************//
 if(timer < tm_def+50000) //время не установлено
	   {lcd_set_strs(1,0,8,"УСТАНОВИТЕ ВРЕМЯ",1);}
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

	    	   //**спаль до прерывания по таймеру сек или  индикации
	    	  	    	     	 if(S){lcd_set_strs(2,40,24,":", 1);if(!S_dat){__WFI();}}//рисуем точки и спать

	    	  	    	     	 	 if(!S)
	    	  	    	     	       	   {
	    	  	    	     	 		 	lcd_set_rect(2,46,4,0);//убираем точки и спать
	    	  	    	 				    lcd_set_rect(3,46,4,0);
	    	  	    	 				    lcd_set_rect(4,46,5,0);
	    	  	    	 				   if(!S_dat){__WFI();}//если не в меню то спать
	    	  	    	 				    }

	    	  	    	     	 	 if(S_dat>5){ S_dat = 0;BipStop();}

 }
//************************************************************************************************************************//

 //************************выгружаем  время*******************************************************************************//


  //*******батарея**********//
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
      	//ограничиваеи до 4 символов
      //	lcd_write_cmd(0x40);
      	lcd_set_strs(y,x,h,buf,c);
      	lcd_set_strs(y,l,h,"V",0);


}


//******************температура с наружи******************************//
void T_out(u8 y,u8 x, u8 h,u8 c)//D код датчика
{u8 l;IWDG_ReloadCounter();
		if(h==8)l=6;
    	if(h==16)l=8;
    	if(h==24)l=16;
    	if(h==32)l=24;
    	l=x+l*4;

	if(temp_out.code[0] == 0x28){ //если код 0 то пропускаем действае
    GET_RAM_USART_DS18B20(temp_out.code,data);  //забираем всю память
    	       	 //  R = (crc_check(data));
    	   temp_out.d = CONV_TEMP_DS18B20 (0.625,TH_,TL_);// пересчитавеем по приращению
    	       	   _sprtffd(1,buf,temp_out.d);
    	       	   chek_str(buf,4);
    	   lcd_set_strs(y,x,h,buf,c);
    	   lcd_set_sector (y,l,8,8,celsiy_8x8,0);
    	   lcd_set_rect(y+1,l,8,1);

   } else {lcd_set_strs(y,x,h,"----",0);}//датчик не установлен
}
//*********************************************************************************************************************//
//******************температура в******************************//
void T_in(u8 y,u8 x, u8 h,u8 c)
  {u8 l;IWDG_ReloadCounter();
  	  	  	  	if(h==8)l=6;
  	  	  	  	if(h==16)l=8;
    	       	if(h==24)l=16;
    	       	if(h==32)l=24;
    	       	l=x+l*4;
	if(temp_in.code[0] == 0x28){ //если код 0 то пропускаем действае
    GET_RAM_USART_DS18B20(temp_in.code,data);  //забираем всю память
    	 //  R = (crc_check(data));
    	   temp_in.d = CONV_TEMP_DS18B20 (0.625,TH_,TL_);// пересчитавеем по приращению
    	   _sprtffd(1,buf,temp_in.d);
    	   chek_str(buf,4);
    	   lcd_set_strs(y,x,h,buf,c);
    	   lcd_set_sector (y,l,8,8,celsiy_8x8,c);
    	   lcd_set_rect(y+1,l,8,1);
  } else {lcd_set_strs(y,x,h,"----",c);}//датчик не установлен
  }
//*********************************************************************************************************************//
//*****************температура двиг***************************//


 void T_eng(u8 y,u8 x, u8 h,u8 c)
	    {u8 l;IWDG_ReloadCounter();
	    	  	  	if(h==8)l=6;
	    	  	  	if(h==16)l=8;
	      	       	if(h==24)l=16;
	      	       	if(h==32)l=24;
	      	       	l=x+l*4;
	  	if(temp_eng.code[0] == 0x28){ //если код 0 то пропускаем действае
	      GET_RAM_USART_DS18B20(temp_eng.code,data);  //забираем всю память
	      	 //  R = (crc_check(data));
	      	   temp_eng.d = CONV_TEMP_DS18B20 (0.625,TH_,TL_);// пересчитавеем по приращению
	      	   _sprtffd(1,buf,temp_eng.d);
	      	   chek_str(buf,4);
	      	   lcd_set_strs(y,x,h,buf,c);
	      	   lcd_set_sector (y,l,8,8,celsiy_8x8,c);
	      	   lcd_set_rect(y+1,l,8,1);
	    } else {lcd_set_strs(y,x,h,"----",c);}//датчик не установлен
	    }


//**********************подключение датчиков**************************************************************************//

 void set_d_ds18b20 (unsigned char set_d[16])//set_d ПИШЕМ ЗАГАЛОВОК
  {	IWDG_ReloadCounter();
  u8 N ;//колличество найденых устройств
  	  	  	lcd_set_strs(0,0,8,set_d,0);//собщение
  	  	  	//lcd_set_strs(2,42,8,"ID",0);//

     		lcd_set_strs(7,32,8,"УСТНОВ",0);
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




//************************сохранил*************************************************************************//


 void save_ok ()  {  	 IWDG_ReloadCounter();
	 lcd_clear();//   L=(84-6*6)/2;
   	lcd_set_strs(3,33,8,"ГОТОВ!",0);
      delay_ms(1000); list++;lcd_clear();

  }

 //********************************************************************************************************//
 void no_save ()
 {	IWDG_ReloadCounter();
	 lcd_clear();//   L=(84-14*6)/2;
	 lcd_set_strs(3,6,8,"НЕ УСТАНОВЛЕН!",0);
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
 		         FLASH_ClearFlag (FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);//С‡РёСЃС‚РёРј С„Р»РіРё
 		           FLASHStatus = FLASH_ErasePage (adr_start);
 		           adr = adr_start;//назначаем адресс начала данных


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
 		adr = adr_start;//назначаем адресс начала данных

 	FLASH_read(temp_in.code, sizeof(temp_in.code));
 	FLASH_read(temp_out.code, sizeof(temp_out.code));
 	FLASH_read(temp_eng.code, sizeof(temp_eng.code));
  	level_ill=*((uint16_t*)adr);adr+=2;
 	adc_calib=*((uint16_t*)adr);adr+=2;
 	contr=*((uint16_t*)adr);

 	if(temp_eng.code[0] == 0x28){
 		//Reset_USART_DS18B20();//контроль темпер воды
 		Get_CFG_USART_DS18B20(temp_eng.code,data);
 		convert_atl_ath ();
 	    temp_eng.TCONTR = CONV_TEMP_DS18B20 (0.625,TH_,TL_);	}// контроль тепмпер пересчитавеем по приращению

 	if(temp_in.code[0] == 0x28){ //если код 0 то пропускаем действае
 		//Reset_USART_DS18B20();//контроль темпер воды
 		Get_CFG_USART_DS18B20(temp_in.code,data);
 		convert_atl_ath ();
 	    temp_in.TCONTR = CONV_TEMP_DS18B20 (0.625,TH_,TL_);}	// контроль тепмпер пересчитавеем по приращению


 	}
 //*****************************************************************************************//
 	void FLASH_read(uint16_t *p, u8 R)
 	{
 		 while(R--)
 			    {IWDG_ReloadCounter();//перезапуск вачдога
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
 			lcd_set_strs(pos_lout,"У",0);
 			lcd_set_strs(pos_leng,"D",0);

 	}
 	 //***********************************************************************************************//
 	void set_2t()					//рисуем секундные точки
 	{	IWDG_ReloadCounter();
 		if(S){lcd_set_strs(1,34,24,":", 0);}
 				else
 				{ 	lcd_set_rect(1,40,4,1);//убираем точки
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
