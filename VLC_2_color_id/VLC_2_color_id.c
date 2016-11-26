/*
 * @改良型緑青光ID伝達式光クレヨン 送信ビット6bit (スタートビット　データ4bit 偶数パリティビット(スタートビットを含まない)）　at 30hz
 *
 * Created: 2015/11/07 10:39:49
 *  Author: Yamamoto Yohei
 
	Pin Assignment
	PD5 Red
	PD6 Green　　
	PB1 Blue 
	     
	-----+----------------	
	Blue |OFF OFF ON  ON
	-----+----------------
	Green|OFF ON  OFF ON
	-----+----------------   
	ID   |-1  0   1   2
  
 */ 

#define F_CPU 8000000
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define SLEEP 33
#define ID 0 //0~2

#define RED PD6		//PD5> PD6
#define GREEN PD5	//PD6 >PB1
#define BLUE PB1	//PB1>PD5

#define DATA_LONG 6	//何ビット送信するか
#define DATA_WAIT 10  //送信後何ビット分wait
#define DATA_SECTION DATA_LONG+DATA_WAIT	//送信の１セクションは何ビット分

int dat=0;			//割り込みに渡すシリアルデータ
int inx=0;			//ISR関数内の処理でdatの何番目のビットを転送したか
int portd_tmp=0;	//PORTDの一時退避

ISR(TIMER1_COMPA_vect){		//OCR1AがTimer1と一致時に呼び出す　33ms = 30hz
	
	if(inx<DATA_LONG){	//datを送信中なら
		switch(ID){	
			case 0:
			case 2:
			//ID=0: GREEN=ON,BLUE=OFF
				PORTD = (((dat>>(DATA_LONG-inx-1))&1)<<RED)+(1<<GREEN);	
				break;
			case 1:
				//ID=1:GREEN=OFF,BLUE=ON
				PORTD = (((dat>>(DATA_LONG-inx-1))&1)<<RED);
				break;
			default:
				break;
		}
	}else{				//datを送信終わっているなら
		switch(ID){
			case 0:
			case 2:
				//ID=0: GREEN=ON,BLUE=OFF
				PORTD=_BV(GREEN);
				break;
			case 1:
				//ID=1:GREEN=OFF,BLUE=ON
				PORTD =0;
				break;
			default:
				break;
		}
	}
	inx++;
	if(inx>DATA_SECTION) inx=0;		
	
	
}
int main(void)
{
	int ad1,ad2;
	DDRB=_BV(PD1);
	DDRC=0;
	DDRD=_BV(PD5) | _BV(PD6);
	ADCSRA=0B10000110; //CK/64
	TCCR1A=0;
	TCCR1B=_BV(WGM12) | _BV(CS12);	//CTC,分周比256
	TIMSK1=_BV(OCIE1A);				//OCR1Aを比較対象として設定
	OCR1A=640;						//about 33.3ms at 4.9152Mhz(ExtOsc)
	//OCR1A=1041;						//for 8MHz internal Osc
	sei();
	
	if(ID==0){
		PORTB=0;						//ID=0:青LEDOFF
	}else{
		PORTB=_BV(BLUE);			//ID=1,2:青LEDON
	}
	while(1){

		
		ADMUX=0B01100000;
		//AVCCが基準電圧Vref=2.56V Left Adjust ADC0(PC0) ;
		ADCSRA |= 1<<ADSC; //AD start
		while(!(ADCSRA & (1<<ADIF))){}
		//ADCSRのビット4(ADIF)が1になるまで待つ
		ad1=ADCH;
		ADMUX=0B01100001;
		//AVCCが基準電圧Vref=2.56V Left Adjust ADC0(PC0)
		ADCSRA |= 1<<ADSC; //AD start
		while(!(ADCSRA & (1<<ADIF))){}
		//ADCSRのビット4(ADIF)が1になるまで待つ
		ad2=ADCH;
		
		
		
		//ユーザ1（01--）
		
		
		if((ad1<=255) && (ad1>200)){		//下状態 col=2
			
			if((ID==0)||(ID==3)){
				dat=(1<<5)+(ID<<3)+(2<<1)+1;	//parity=1
			}else{
				dat=(1<<5)+(ID<<3)+(2<<1)+0;	//parity=0
			}

			
		}
		else if ((ad1<55) && (ad1>=0)){		//上状態 col=0
			
			if((ID==0)||(ID==3)){
				dat=(1<<5)+(ID<<3)+(0<<1)+0;	//parity=0
			}else{
				dat=(1<<5)+(ID<<3)+(0<<1)+1;	//parity=1
			}

		}
		else if((ad2<=255) && (ad2>200)){		//右状態 col=1
			
			if((ID==0)||(ID==3)){
				dat=(1<<5)+(ID<<3)+(1<<1)+1;	//parity=1
			}else{
				dat=(1<<5)+(ID<<3)+(1<<1)+0;	//parity=0
			}

		}
		
		else if((ad2<55) && (ad2>=0)){		//左状態 col=3
			
			if((ID==0)||(ID==3)){
				dat=(1<<5)+(ID<<3)+(3<<1)+0;	//parity=0
			}else{
				dat=(1<<5)+(ID<<3)+(3<<1)+1;	//parity=1
			}

			
		}
		else{		//Ｎ状態　未完	col=0
			
			if((ID==0)||(ID==3)){
				dat=(1<<5)+(ID<<3)+(0<<1)+0;	//parity=0
				}else{
				dat=(1<<5)+(ID<<3)+(0<<1)+1;	//parity=1
			}

		}
		
	}
}
