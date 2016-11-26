/*
 * @���ǌ^�ΐ�ID�`�B�����N������ ���M�r�b�g6bit (�X�^�[�g�r�b�g�@�f�[�^4bit �����p���e�B�r�b�g(�X�^�[�g�r�b�g���܂܂Ȃ�)�j�@at 30hz
 *
 * Created: 2015/11/07 10:39:49
 *  Author: Yamamoto Yohei
 
	Pin Assignment
	PD5 Red
	PD6 Green�@�@
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

#define DATA_LONG 6	//���r�b�g���M���邩
#define DATA_WAIT 10  //���M�㉽�r�b�g��wait
#define DATA_SECTION DATA_LONG+DATA_WAIT	//���M�̂P�Z�N�V�����͉��r�b�g��

int dat=0;			//���荞�݂ɓn���V���A���f�[�^
int inx=0;			//ISR�֐����̏�����dat�̉��Ԗڂ̃r�b�g��]��������
int portd_tmp=0;	//PORTD�̈ꎞ�ޔ�

ISR(TIMER1_COMPA_vect){		//OCR1A��Timer1�ƈ�v���ɌĂяo���@33ms = 30hz
	
	if(inx<DATA_LONG){	//dat�𑗐M���Ȃ�
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
	}else{				//dat�𑗐M�I����Ă���Ȃ�
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
	TCCR1B=_BV(WGM12) | _BV(CS12);	//CTC,������256
	TIMSK1=_BV(OCIE1A);				//OCR1A���r�ΏۂƂ��Đݒ�
	OCR1A=640;						//about 33.3ms at 4.9152Mhz(ExtOsc)
	//OCR1A=1041;						//for 8MHz internal Osc
	sei();
	
	if(ID==0){
		PORTB=0;						//ID=0:��LEDOFF
	}else{
		PORTB=_BV(BLUE);			//ID=1,2:��LEDON
	}
	while(1){

		
		ADMUX=0B01100000;
		//AVCC����d��Vref=2.56V Left Adjust ADC0(PC0) ;
		ADCSRA |= 1<<ADSC; //AD start
		while(!(ADCSRA & (1<<ADIF))){}
		//ADCSR�̃r�b�g4(ADIF)��1�ɂȂ�܂ő҂�
		ad1=ADCH;
		ADMUX=0B01100001;
		//AVCC����d��Vref=2.56V Left Adjust ADC0(PC0)
		ADCSRA |= 1<<ADSC; //AD start
		while(!(ADCSRA & (1<<ADIF))){}
		//ADCSR�̃r�b�g4(ADIF)��1�ɂȂ�܂ő҂�
		ad2=ADCH;
		
		
		
		//���[�U1�i01--�j
		
		
		if((ad1<=255) && (ad1>200)){		//����� col=2
			
			if((ID==0)||(ID==3)){
				dat=(1<<5)+(ID<<3)+(2<<1)+1;	//parity=1
			}else{
				dat=(1<<5)+(ID<<3)+(2<<1)+0;	//parity=0
			}

			
		}
		else if ((ad1<55) && (ad1>=0)){		//���� col=0
			
			if((ID==0)||(ID==3)){
				dat=(1<<5)+(ID<<3)+(0<<1)+0;	//parity=0
			}else{
				dat=(1<<5)+(ID<<3)+(0<<1)+1;	//parity=1
			}

		}
		else if((ad2<=255) && (ad2>200)){		//�E��� col=1
			
			if((ID==0)||(ID==3)){
				dat=(1<<5)+(ID<<3)+(1<<1)+1;	//parity=1
			}else{
				dat=(1<<5)+(ID<<3)+(1<<1)+0;	//parity=0
			}

		}
		
		else if((ad2<55) && (ad2>=0)){		//����� col=3
			
			if((ID==0)||(ID==3)){
				dat=(1<<5)+(ID<<3)+(3<<1)+0;	//parity=0
			}else{
				dat=(1<<5)+(ID<<3)+(3<<1)+1;	//parity=1
			}

			
		}
		else{		//�m��ԁ@����	col=0
			
			if((ID==0)||(ID==3)){
				dat=(1<<5)+(ID<<3)+(0<<1)+0;	//parity=0
				}else{
				dat=(1<<5)+(ID<<3)+(0<<1)+1;	//parity=1
			}

		}
		
	}
}
