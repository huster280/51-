#include<reg51.h>
#include "predefined.h"


uchar datetabal;
//uchar d1,d2;//���ݻ���

//���ܣ�������ʱ
void delay(uint xms)
{
    uchar ad,bd;
    for(ad=xms; ad>0; ad--)
        for(bd=110; bd>0; bd--);
}

//���ܣ�����ʱ
void delay_short()
{
	;;;;;;
}

//���ܣ�LCDæ��⣬æʱ����1  
bit lcd_busy()  
{  
  	 bit result;  
  	 RS = 0;    
  	 RW = 1;  
 	 EN = 1;  
 	 result = (bit)(P0 & 0x80);  
   	 EN = 0;  
     return result;  
} 

//���ܣ�д����
void write_data(uchar dat)
{ 
   	while(lcd_busy());	  //æ���
	RS = 1;
   	RW = 0;
   	EN = 0;

   	P0 = dat; 
	delay_short();
	EN = 1;
	delay_short();
	EN = 0;
}

//���ܣ�дָ��
void write_cmd(uchar com)
{
   	while(lcd_busy());	  //æ���
	RS = 0;
   	RW = 0;
   	EN = 0;

   	P0 = com; 
	delay_short();
	EN = 1;
	delay_short();
	EN = 0;
}

//���ܣ�д�ַ���
void write_string(uchar *str)  
{  
 	while(*str!='\0')  //�ַ���δ����  
 	{  
		write_data(*str++);	//����д����  
		delay(2);  
 	}  
} 

/**
//�����ݺ���
uchar read_data(void)
{
	uchar temp;
	while(lcd_busy());
	EN = 0;
	P0 = 0xff;  //IO���øߵ�ƽ��������
	RS = 1;
	RW = 1;
	EN = 1;
	delay(5);    //ʹ����ʱ!!!ע���������ǽϿ��CPUӦ����ʱ��һЩ
	temp = P0;

	return temp; 
}	
**/

//��ʾʱ��仯
void time_change(uint ttime)
{
	uchar data time[8] = {"      ��"};	  //���ﶨ�嵽xdata������
	time[0]=' ';
	time[1]=ttime/10000 + '0';	   //ת�����ַ���
	time[2]=ttime/1000%10 + '0';
	time[3]=ttime/100%10 + '0';
	time[4]=ttime/10%10 + '0';
	time[5]=ttime%10 + '0';

	write_cmd(0x98+4);		//д���ַ
	write_string(time);	  //д��ʱ��
}

//��ʼ��51�����ж�
void int51_timec()
{
	EA=1;
	TMOD=0x1;		//��ʱ��1�Ͷ�ʱ��0��������ģʽ1
	ET0=1;		   //�򿪶�ʱ��0�ж�
	ET1=1;
	//TR0=1;		 //������ʱ��0
	TH0=(65536-50000)/256;	  //��ģ����8λ
	TL0=(65536-50000)%256;		//�������8λ
	tcount=0;
}

//���ܣ���ʱ��0�ж�
void t0_tt() interrupt 1
{
	TH0=(65536-50000)/256;		 //�����ж����¸�ֵ
	TL0=(65536-50000)%256;
	tcount++;
	if(tcount==20)		//50000*20us = 1s
	{
		tcount=0;
		Timtcount++;
		time_change(Timtcount);	
	}			
}


//���ܣ�12864Һ������ʼ��  
void lcd_init()  
{  
 	LCD_PSB = 1;  //���з�ʽ  
 	LCD_RST = 1;  //����λ  
 	write_cmd(0x30);  //����Ϊ����ָ�����
 	delay(2); 
	write_cmd(0x34);
 	delay(2);

	write_cmd(0x34);   //��չָ��ģʽ 0x30Ϊ����ָ��ģʽ  	  
 	delay(2);  
 	write_cmd(0x36);   //��ͼƬ��ʾ 
	delay(2);
	write_cmd(0x0c);  //����ʾ������ʾ���   	  
 	delay(2); 
	write_cmd(0x01);   //����ָ��  	
} 

//���ܣ���������  ��ȫ��clc(32,8,0)  ���Ұ���clc(32,4,4)
void clc(uchar x, uchar y, uchar type)
{
	uchar i,j,n;
	write_cmd(0x36);//����ָ�� ��ͼ��ʾ
	for(n=0; n<9; n+=8)	  //�ϰ���n=0,�°���n=8
	{
		for(j=0; j<x; j++)
		{
			for(i=0; i<y; i++)
			{
				write_cmd(0x80+j);	  //�е�ַ
				write_cmd(0x80+i+n+type);	//�е�ַ,n=0�ϰ�����n=8�°���
				write_data(0x00);
				write_data(0x00);
			}
		}
	}	
	write_cmd(0x30);
}

//���ܣ����ȫ���ĺ���
void clear_hanzi()
{
	uchar i;
	uchar blank[16] = {"                "};
	for(i=0;i<4;i++)
	{
		write_cmd(0x80+ (8*i));
		write_string(blank);		
	}
}	

//��ʾ�ؿ���
void display_level(uchar n)
{
	uchar str_num[8] = {"��0 ��"};	  //��������4���ռ������ĳ�6�����ԣ���Ϊһ������ռ�����ֽ�
	str_num[3] = n+1 + '0';	   //��һ��ʾ�ؿ���������'0'��Ϊ�˽�����ת�����ַ���
	write_cmd(0x80+5);	 	 //��һ�У�5*16��
	//write_string("��  ��  ");	  //д�뺺�֣�ע�⺺�ֱ�����ַ���ż��λ��ʼд
	write_string(str_num);	  //write_string("1");
	
	write_cmd(0x88+5);
	write_string("��ʱ:");
}

//���ܣ���������,���ڻ�ȫ��ͼ��
void draw_dianzhen(uchar *addr)
{	
	uchar x=0,y=0;		//��ʼ��λ��
	write_cmd(0x3f);//������ָ���������λѡ��8λ
   	delay(2);
   	write_cmd(0x3e);//дָ������ָ�������8λ���ݣ���ͼ���ع� 
  	delay(2);

	for(y=0;y<32;y++)	//����32��
	{
     	
   		for(x=0;x<8;x++)	//8*8*2��
   		{
   			write_cmd((0x80+y));  //дָ������ ��ֱ �е�ַ
   			delay(1);
    		write_cmd((0x80+x));  //дָ������ ˮƽ �е�ַ
       		delay(1);  
  			write_data(*addr++);  //*addrָ����øú���ʱ��������飬��дһ�����ݺ��Լ�1��ָ���һλ����        	
  			write_data(*addr++);  //��д��8λ����д��8λ
    	}
	} 
//***********�������ϰ�����Y=0-1F,X=0-07���������°���Y=8-0F X=0-1F******************************** 
	x=0;y=0;//����XY��������Ϊ�°����Ǵ�88H��ʼ�ģ����ϰ����Ǵ�80H��ʼ�ģ���ʵ��������õ����ϱߵģ�ֻ�Ǹ��˸�ˮƽ����ֵ����ֱ���껹�Ǵ�0-31 
	for(y=0;y<32;y++)
	{
     	x=0;
   		for (x=0;x<8;x++)		//�Լ����Լ�ռ�ô���ռ��С
   		{
			write_cmd((0x80+y));	//дָ������ ��ֱ �е�ַ
   			delay(1);
    		write_cmd((0x88+x));	//дָ������ ˮƽ �е�ַ
       		delay(1); 
   			write_data(*addr++);	//ͬ��
   			write_data(*addr++);
		}
	}
	write_cmd(0x30);  //ת����ָͨ��
}


//���ܣ���Сͼ�꺯��,x(0~7)��y(0~7)��ʾ��ͼλ�ã�num��Χ0~5����ʾ��ͼ��Ϣ
void display(uchar x, uchar y, uchar num_p, uchar num_q)	  
{
  	uchar j,*p,*q;
  	p = &elements[num_p];
	q = &elements[num_q];
  	write_cmd(0x34);  //������ָ���������λѡ��8λ
   	delay(2);
   	write_cmd(0x3e);  //дָ������ָ�������8λ���ݣ���ͼ���ع�
	
	if(x<4)		//С��4�ϰ��������ڵ���4�°�������8
	{
		for(j=0;j<8;j++)  //iֻ��1��ѭ�������ɾ����
    	{
	  		write_cmd(0x80+j + 8*x);	//������������������
	  		write_cmd(0x80 + y);
			write_data(*p++);
			write_data(*q++);
		}	
	}
	else
	{  
		for(j=0;j<8;j++)  //iֻ��1��ѭ�������ɾ����
    	{
	  		write_cmd(0x80+j + 8*(x-4));	//������������������
	  		write_cmd(0x88 + y);
			write_data(*p++);
			write_data(*q++);
		}		
	}
  	write_cmd(0x30);  //ת����ָͨ��
}


//���ܣ���ʾ��Ϸ���ؿ���Ϸ���棬nΪ�ؿ�����
void game_level(uchar n)
{
	uchar i,j;
	//�µ�һ�أ�CR=0,�������㣬CR=1���������
	step=0;
	count_CR = 0;
	delay(1);  
	count_CR = 1;

	for(i=0;i<8;i++)                                      //��
	{
		for(j=0;j<8;j++)                                  //��
		{
			if(level[n][i][j]==1) 	  //�ҵ�����
			{
				man_x=j;
				man_y=i;
			}
			level_temp[i][j]=level[n][i][j];       //��ǰ�ؿ���������
		}
	}
	for(i=0;i<8;i++)
		for(j=0;j<8;j+=2)
			display(i,j/2,level_temp[i][j],level_temp[i][j+1]);          //��ʾ��ǰ�ؿ���һ��д������ͼ��
																										  	
	clc(32,4,4);   //����Ұ�������ʾ�ؿ�����ʱ��
	display_level(n);
	time_change(Timtcount);			
}


//��Ϸ�������溯��
void start_interface()
{
	uchar j;
	draw_dianzhen(start);	//��������
	for(j=0;j<8;j++)	//��ʼ����
	{
		display(5,j,1,0);	   //0,1�ֱ����հ׺�С��
		delay(100);
		display(5,j,0,1);	 //5��ʾ��5��
		delay(100);
		if(j!=0)	  //��һ�β��ò���
			display(5,j-1,0,0);	//	����ǰ���С�˺ۼ�
		delay(100);
	}
}


void t1_tt() interrupt 3	   //��ʱ��3
{
	speaker=~speaker;
	TH1=timer_h;
	TL1=timer_l;

}

void sing(uchar length,uchar type,uchar ttime2)
{
	uchar i,k;
	TR1 = 1;
	
	for(i=0;i<length;i+=3)
	{
		if(type)
			k=s1[i]+7*s1[i+1]-1;	
		else
			k=s2[i]+7*s2[i+1]-1;
		timer_h=FREQH[k];
		timer_l=FREQL[k];
		tt=s2[i+2];
	
		TH1=timer_h;
		TL1=timer_l;
		delay(ttime2*tt);
	}
	TR1=0;
}

//���ܣ��ж��Ƿ�ͨ�����ؿ�(������ĿΪ0Ϊ�ж�����)
void level_suc()
{
	uchar i,j;
	uchar wait=3;
	uchar suc_flag=1;
	for(i=0; i<8; i++)	
	{
		for(j=0; j<8; j++)
		{/**	                                                     
			if((level[n][i][j]==4) || (level[n][i][j]==5))	//ѭ��ɨ��Ŀ��λ��,����һ������û������,�Ͳ�����
			{
				if(level_temp[i][j]!=5)
					suc_flag=0;	
			}  **/
			if(level_temp[i][j]==3)	  	//ѭ��ɨ���ͼ�������Ӷ����5����û������3���˹ؿ���ͨ��
				suc_flag = 0;
		}
	}

	if(step>50)		//��������99ʱ����սʧ�ܣ�
	{  
		sing(21,0,250);

		TR0=0;	//�رն�ʱ��
		write_cmd(0x84);
		write_string("        ");
		while(wait--)	
			delay(8);
		write_cmd(0x84);
		write_string("��սʧ��");
		while(wait--)	
			delay(8);
		write_cmd(0x84);
		write_string("        ");
		while(wait--)	
			delay(8);
		write_cmd(0x84);
		write_string("������  ");  //�ո�Ϊ�˲���ǰ��ġ��ܡ���
		while(wait--)	
			delay(8);
		write_cmd(0x84);
		write_string("        ");
		while(wait--)	
			delay(8);
		write_cmd(0x84);
		write_string("��������\xfd");	//keil��һ��bug�����ֺ������ͻ����������ʾ�����ں����\xfd
		while(wait--)	
			delay(24);

		playing=0;
		Timtcount=0;
		clear_hanzi();
		clc(32,8,0);					
		start_interface();	//�ص�������	   
	}	

	if(suc_flag==1)		 //����Ŀ��λ�ö�������
	{	
		sing(18,1,250);
		
		TR0=0; 	//�رն�ʱ��
		write_cmd(0x84);
		write_string("        ");
		while(wait--)	
			delay(8);
		write_cmd(0x84);
		write_string("��ս�ɹ�");
		while(wait--)	
			delay(8);
		write_cmd(0x84);
		write_string("        ");
		while(wait--)	
			delay(8);		 

		if(n<8)
		{
			write_cmd(0x84);
			write_string("��������");
			while(wait--)	
				delay(8);
			write_cmd(0x84);
			write_string("        ");
			while(wait--)	
				delay(8);
			write_cmd(0x84);
			write_string("��һ��  ");   //�ո�Ϊ�˲���ǰ��ġ��롱��
			while(wait--)	
				delay(8);
			write_cmd(0x84);
			write_string("        ");
			while(wait--)	
				delay(8);
			TR0=1;
			
			n+=1;  	//��һ��
			Timtcount=0;	//��ʱ����
			TH0=(65536-50000)/256;
			TL0=(65536-50000)%256;
			tcount=0;
			game_level(n);
		}
		else 
		{
			write_cmd(0x84);
			write_string("�����ص�");
			while(wait--)	
				delay(8);
			write_cmd(0x84);
			write_string("        ");
			while(wait--)	
				delay(8);
			write_cmd(0x84);
			write_string("������  ");
			while(wait--)	
				delay(8);
			write_cmd(0x84);
			write_string("        ");
			while(wait--)	
				delay(8);
			
			playing=0;
			Timtcount=0;
			//TR0=0;
			clear_hanzi();
			clc(32,8,0);					
			start_interface();	//ͨ�غ�ص�������
		}			
	}
}

//����������			
/***�������ּ�⹦�ܣ�(successed!)
1.����,loose=1 -> return ��9
2.����,loose=0 -> return 9
3.�ɿ�,loose=1 -> return 9���ҽ������
4.�ɿ�,loose=0 -> return 9
***/
uchar key_scan()	 
{	  	
	static uchar key_loose=1;	 //���ּ���־
	uchar tmp; 
	uchar keys_state = P1 & 0x3f;	//ֻ����6λ�˿�,����λ��0��������Ӱ��
	uchar keyno=0;
	if( key_loose && (keys_state != 0x3f) )	 //�а���������δ������
	{
		key_loose=0;
		delay(200); 		//ȥ����,������keyno��ȻΪ0
		if(keys_state != (P1 & 0x3f))	//���ΰ�����һ�£�������Ӧ
			keyno = 0;
		else	  
		{
			//������00111111�����00���� ����������������һ��Ϊ0,����Ϊ1
			//��������������5��1���0��Ψһ��0���1
			tmp = keys_state ^ 0x3f;
			switch(tmp)		//�ж��ĸ�����������
			{
				case 1: keyno = 1; break;
				case 2: keyno = 2; break;
				case 4: keyno = 3; break;
				case 8: keyno = 4; break;
				case 16: keyno = 5;break;
				case 32: keyno = 6;break;
				default:keyno = 0;		//�޼�����	
			}  
		}
	}
	else if(keys_state == 0x3f)	  //������ȫ���ָ�
		key_loose=1;	//�������
	return keyno; 
			  
/***ԭ�������̫�����ӣ��Ż���һ��
	static uchar key_loose=1;		//�����ɿ���־	
	if(key_loose&&(key_up==0||key_down==0||key_left==0||key_right==0))
	{
		delay(10);	//ȥ��
		if(key_up==0||key_down==0||key_left==0||key_right==0)
		{	
			key_loose=0;			
			if (key_up==0)
				return 1;
			else if(key_down==0)
				return 2;
			else if(key_left==0)
				return 3;
			else if(key_right==0)
				return 4;
		} 
	}				 
	else if(key_up==1&&key_down==1&&key_left==1&&key_right==1)  //������ȫ���ɿ�
		key_loose=1; 
	return 0;		// �ް�������	   
***/	
}					 

//������1����
void add_step()
{
	step += 1;
	count_step = 0;
	delay(1);
	count_step = 1;
}

//��ͼ�꺯��,ÿ�λ���������16*8
void printelement(uchar xh, uchar yh, uchar elum)
{
	if(xh%2 == 0)	
		display(yh, xh/2, elum,level_temp[yh][xh+1]);
	else
		display(yh, xh/2, level_temp[yh][xh-1], elum);
}

void state1(uchar plus,uchar direction)	 //plus=1,��������;plus=-1,��������
{
	if(level[n][man_y][man_x]==4||level[n][man_y][man_x]==5)           //if ��λ��ԭ��==Ŀ��or�ɹ�
	{
		level_temp[man_y][man_x]=4;		//���ߺ���Ŀ��
		printelement(man_x,man_y,4); 
	}
	else
	{
		level_temp[man_y][man_x]=0;		//���ߺ��ɿյ�
		printelement(man_x,man_y,0);  
	}
	man_y=man_y + plus;					//��=��//���һ���ƶ�
	add_step();		//������һ
	sing(9,1,30);
	level_temp[man_y][man_x]=1;
	printelement(man_x,man_y,direction);
}

//���ܣ������ϼ��Ĳ���
void move_up()
{
	if(level_temp[man_y-1][man_x]==0||level_temp[man_y-1][man_x]==4)	     //if ����==��orĿ��   
		state1(-1,6);  
	
	else if(level_temp[man_y-1][man_x]==3)		 	//if ����==��
	{				  
		if(level_temp[man_y-2][man_x]==0)	   		//if ����==�հ�
		{
			state1(-1,6);
			level_temp[man_y-1][man_x]=3;		//����=��//���һ���ƶ�
			printelement(man_x,man_y-1,3);
		}
		else if(level_temp[man_y-2][man_x]==4)		//if ����==Ŀ
		{
			state1(-1,6);
			level_temp[man_y-1][man_x]=5;											 //����=��
			printelement(man_x,man_y-1,5); 			
		} 
	}

	else if(level_temp[man_y-1][man_x]==5)	       //����==��
	{		   
		if(level_temp[man_y-2][man_x]==0)		 //���ϣ�=��
		{
			state1(-1,6);
			level_temp[man_y-1][man_x]=3;		//����=��
			printelement(man_x,man_y-1,3);
		}
		else if(level_temp[man_y-2][man_x]==4)	  //����?=Ŀ
		{
			state1(-1,6);
		    level_temp[man_y-1][man_x]=5;		//����=��
		    printelement(man_x,man_y-1,5);	 		    
		}  			
	}
	level_suc();       //�ж��Ƿ���ɱ���				 
}


//���ܣ������¼��Ĳ���
void move_down()
{
	if(level_temp[man_y+1][man_x]==0||level_temp[man_y+1][man_x]==4)
		state1(1,1);
	
	else if(level_temp[man_y+1][man_x]==3)											   
	{
		if(level_temp[man_y+2][man_x]==0)											  
		{
			state1(1,1);
			level_temp[man_y+1][man_x]=3;											   
			printelement(man_x,man_y+1,3);
		}
		else if(level_temp[man_y+2][man_x]==4)											
		{
			state1(1,1);
			level_temp[man_y+1][man_x]=5;
			printelement(man_x,man_y+1,5);			
		}
	}

	else if(level_temp[man_y+1][man_x]==5)
	{
		if(level_temp[man_y+2][man_x]==0)
		{
			state1(1,1);
			level_temp[man_y+1][man_x]=3;
			printelement(man_x,man_y+1,3);
		}
		else if(level_temp[man_y+2][man_x]==4)
		{
			state1(1,1);
			level_temp[man_y+1][man_x]=5;
			printelement(man_x,man_y+1,5); 						  
		} 				  
	}				  				  
	level_suc();
}

void state2(uchar plus,uchar direction)	 //plus=1,��������;plus=-1,��������
{
	if(level[n][man_y][man_x]==4||level[n][man_y][man_x]==5)
	{
		level_temp[man_y][man_x]=4;
		printelement(man_x,man_y,4);
	}
	else
	{
		level_temp[man_y][man_x]=0;
		printelement(man_x,man_y,0);
	}
	man_x=man_x + plus;
	add_step();		//������һ
	sing(9,1,30);
	level_temp[man_y][man_x]=direction;	  //������1����ʾ��������λ��
	printelement(man_x,man_y,direction);
}

//���ܣ���������Ĳ���
void move_left()
{
	if(level_temp[man_y][man_x-1]==0||level_temp[man_y][man_x-1]==4)
		state2(-1,7);

	else if(level_temp[man_y][man_x-1]==3)
	{
		if(level_temp[man_y][man_x-2]==0)
		{
			state2(-1,7);
			level_temp[man_y][man_x-1]=3;
			printelement(man_x-1,man_y,3);
		} 
		else if(level_temp[man_y][man_x-2]==4)
		{ 
			state2(-1,7);
			level_temp[man_y][man_x-1]=5;
			printelement(man_x-1,man_y,5);
		}
	}  
	else if(level_temp[man_y][man_x-1]==5)
	{
		if(level_temp[man_y][man_x-2]==0)
		{	
			state2(-1,7);
			level_temp[man_y][man_x-1]=3;
			printelement(man_x-1,man_y,3); 						      
		}
		else if(level_temp[man_y][man_x-2]==4)
		{	  
			state2(-1,7);
			level_temp[man_y][man_x-1]=5;
			printelement(man_x-1,man_y,5);
		}						 
	}			
	level_suc();			
}

//���ܣ������Ҽ��Ĳ���
void move_right()
{
	if(level_temp[man_y][man_x+1]==0||level_temp[man_y][man_x+1]==4)
		state2(1,8);
	
	else if(level_temp[man_y][man_x+1]==3)
	{	 
		if(level_temp[man_y][man_x+2]==0)
		{	  
			state2(1,8);
			level_temp[man_y][man_x+1]=3;
			printelement(man_x+1,man_y,3);
		}
		else if(level_temp[man_y][man_x+2]==4)
		{	  
			state2(1,8);
			level_temp[man_y][man_x+1]=5;
			printelement(man_x+1,man_y,5);						  
		}	
	}	 
	else if(level_temp[man_y][man_x+1]==5)
	{	  
		if(level_temp[man_y][man_x+2]==0)
		{
			state2(1,8);
			level_temp[man_y][man_x+1]=3;
			printelement(man_x+1,man_y,3); 						      
		}
		else if(level_temp[man_y][man_x+2]==4)
		{	  
			state2(1,8);
			level_temp[man_y][man_x+1]=5;
			printelement(man_x+1,man_y,5); 						  
		}						 
	}					 
	level_suc();	
}


//ѡ�ؽ���
void chooose_level()
{ 
	uchar keyupdate,i;
	uchar down=1;
	uchar position[9] = {0x80,0x90,0x88,0x98,0x83,0x93,0x8b,0x9b,0x86};		  //��ʾ�ĵ�ַ����
	uchar level_name[6] = {" Lv "}; 
	clear_hanzi();	//�ֿ�д��ĺ���Ҫд��ո����
	clc(32,8,0);	//������

	for(i=0;i<9;i++) 	  //��ʾ�ؿ���
	{
		write_cmd(position[i]); 
		level_name[3] = i+1 + '0';
		write_string(level_name);
	}
	
	while(down)			 //�ȴ�ȷ��������
	{
		write_cmd(position[n]);    //ָʾ��ʶ��>��
		write_string(">");
		delay(20);
		write_cmd(position[n]);    //���հ�,�����ɵı�ʶ
		write_string(" ");

		keyupdate=key_scan();
		switch(keyupdate)
		{
			case 1:
				if(n>3)		//�ڶ��л�n=9,���������Ч
					n -= 4;
				break;
			case 2:
				if(n!=0)	//��1�ذ��ϼ���Ч
					n -= 1;
				break;
			case 3:
				if(n!=8)	//��9�ذ��¼���Ч
					n += 1;
				break;
			case 4:
				if(n<5)		//��һ�л�n=4,���Ҽ�����Ч
					n += 4;
				break;
			case 6:
				down = 0;	 //�˳�ѭ��
				break;
		}
	}
}

//���ܣ���������Ϸ����
void put_box()
{
	uchar keyupdate;	
	keyupdate=key_scan();
	switch(keyupdate)
	{
		case 1:move_left();break;				                                 
		case 2:  move_up();break;
		case 3:move_down();break;	
		case 4:move_right();break;
				
		case 5:
	    /**		if(n!=8)
					n++;
				game_level(n);		  //��ʾ��һ��
				Timtcount=0;
				TH0=(65536-50000)/256;
				TL0=(65536-50000)%256;
				tcount=0;
				break;		**/
				TR0=0;	 //�رն�ʱ��0
				chooose_level();
				clear_hanzi();	//�ֿ�д��ĺ���Ҫд��ո����
				Timtcount=0;
				TH0=(65536-50000)/256;
				TL0=(65536-50000)%256;
				tcount=0;
				TR0=1;	 //����������ʱ��0
				game_level(n);	//���뵽ѡ��Ĺؿ�
				break;
		case 6:		  //���¿�ʼ��ť
				Timtcount=0;
				TH0=(65536-50000)/256;
				TL0=(65536-50000)%256;
				tcount=0;
				game_level(n);
				break;	
	}
}

void main()
{
	int51_timec();
	lcd_init();
	//Timtcount=0;
	playing=0;
	count_CR=1;	   //�������
	start_interface();
	//write_cmd(0x0c); 
	while(1)
	{
		while(key_scan()!=6);			 //�ȴ�ȷ��������

		playing=1;
		EA=1; 
		n=0;
		chooose_level();	//����ѡ�ؽ���
		clear_hanzi();	//�ֿ�д��ĺ���Ҫд��ո����		
		game_level(n);		  //��ʾ��һ�� 
		Timtcount=0;
		TR0=1;
		while(playing)		  //��Ϸ��...
		{
		 	put_box();	 
		}
	}		
	  
	  //*******************************************************
      //��12864�ֳ������֣��ϰ벿��12832���°벿��12832
      //Ȼ����ɨ��İ취�ӵ�һ�п�ʼɨ�裬ֱ��32�У���ִ���°벿��ɨ����򣬵�33�п�ʼ��ɨ��32�е�64��
      //���ﻹҪ˵һ���£�����12864��ͼ������Ҫ����12864��Ļ����ʾһ��ͼ�����Ҫ
      //��һ���㿪ʼ����һ�����õ�һ�еĵ�һ����ʾһ���㣬��ôʵ���أ��ҿ�ʼ���ǲ����ף�����һ����ֲᣬһ�����ʵ�飬�����ܽ��
      //1����7920��д��3F��3Eָ�ʹҺ����������չָ��״̬��8λ���ݣ���ͼ���ؿ�
	  //2���趨��ͼRAM��ַ����д���У�Y�ᣩ��д���У�X�ᣩ
      //Ȼ����һ����д��16λ�����ݣ�Ҳ����д8λ����Ҫд2�Σ����ǲ�ȡ���ߣ�Ȼ����ʮ����ʲô�أ�---������������D15-D0���õ�һ�����д��8000H
      //��ô�����ھ����ڵ�һ�е�һ����ʾһ���㣡��дһ���㣬���һ����дһ��ͼ���ˣ�Ϊʲô�أ���ֻҪ�õ�Ƭ�������ظ���д512���־�����һ��
      //12864��ͼ���ˣ�����ʵ˵���˾��ǵ���
}