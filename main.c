#include<reg51.h>
#include "predefined.h"


uchar datetabal;
//uchar d1,d2;//数据缓冲

//功能：毫秒延时
void delay(uint xms)
{
    uchar ad,bd;
    for(ad=xms; ad>0; ad--)
        for(bd=110; bd>0; bd--);
}

//功能：短延时
void delay_short()
{
	;;;;;;
}

//功能：LCD忙检测，忙时返回1  
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

//功能：写数据
void write_data(uchar dat)
{ 
   	while(lcd_busy());	  //忙检测
	RS = 1;
   	RW = 0;
   	EN = 0;

   	P0 = dat; 
	delay_short();
	EN = 1;
	delay_short();
	EN = 0;
}

//功能：写指令
void write_cmd(uchar com)
{
   	while(lcd_busy());	  //忙检测
	RS = 0;
   	RW = 0;
   	EN = 0;

   	P0 = com; 
	delay_short();
	EN = 1;
	delay_short();
	EN = 0;
}

//功能：写字符串
void write_string(uchar *str)  
{  
 	while(*str!='\0')  //字符串未结束  
 	{  
		write_data(*str++);	//依次写数据  
		delay(2);  
 	}  
} 

/**
//读数据函数
uchar read_data(void)
{
	uchar temp;
	while(lcd_busy());
	EN = 0;
	P0 = 0xff;  //IO口置高电平，读引脚
	RS = 1;
	RW = 1;
	EN = 1;
	delay(5);    //使能延时!!!注意这里，如果是较快的CPU应该延时久一些
	temp = P0;

	return temp; 
}	
**/

//显示时间变化
void time_change(uint ttime)
{
	uchar data time[8] = {"      秒"};	  //这里定义到xdata里会出错
	time[0]=' ';
	time[1]=ttime/10000 + '0';	   //转换成字符串
	time[2]=ttime/1000%10 + '0';
	time[3]=ttime/100%10 + '0';
	time[4]=ttime/10%10 + '0';
	time[5]=ttime%10 + '0';

	write_cmd(0x98+4);		//写入地址
	write_string(time);	  //写入时间
}

//初始化51，开中断
void int51_timec()
{
	EA=1;
	TMOD=0x1;		//定时器1和定时器0都工作在模式1
	ET0=1;		   //打开定时器0中断
	ET1=1;
	//TR0=1;		 //启动定时器0
	TH0=(65536-50000)/256;	  //求模给高8位
	TL0=(65536-50000)%256;		//求余给低8位
	tcount=0;
}

//功能：定时器0中断
void t0_tt() interrupt 1
{
	TH0=(65536-50000)/256;		 //进入中断重新赋值
	TL0=(65536-50000)%256;
	tcount++;
	if(tcount==20)		//50000*20us = 1s
	{
		tcount=0;
		Timtcount++;
		time_change(Timtcount);	
	}			
}


//功能：12864液晶屏初始化  
void lcd_init()  
{  
 	LCD_PSB = 1;  //并行方式  
 	LCD_RST = 1;  //不复位  
 	write_cmd(0x30);  //设置为基本指令集动作
 	delay(2); 
	write_cmd(0x34);
 	delay(2);

	write_cmd(0x34);   //扩展指令模式 0x30为基本指令模式  	  
 	delay(2);  
 	write_cmd(0x36);   //打开图片显示 
	delay(2);
	write_cmd(0x0c);  //开显示，不显示光标   	  
 	delay(2); 
	write_cmd(0x01);   //清屏指令  	
} 

//功能：清屏函数  清全屏clc(32,8,0)  清右半屏clc(32,4,4)
void clc(uchar x, uchar y, uchar type)
{
	uchar i,j,n;
	write_cmd(0x36);//扩充指令 绘图显示
	for(n=0; n<9; n+=8)	  //上半屏n=0,下半屏n=8
	{
		for(j=0; j<x; j++)
		{
			for(i=0; i<y; i++)
			{
				write_cmd(0x80+j);	  //列地址
				write_cmd(0x80+i+n+type);	//行地址,n=0上半屏，n=8下半屏
				write_data(0x00);
				write_data(0x00);
			}
		}
	}	
	write_cmd(0x30);
}

//功能：清除全屏的汉字
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

//显示关卡数
void display_level(uchar n)
{
	uchar str_num[8] = {"第0 关"};	  //这里声明4个空间会出错，改成6个可以，因为一个汉字占两个字节
	str_num[3] = n+1 + '0';	   //加一表示关卡数，加上'0'是为了将数字转换成字符串
	write_cmd(0x80+5);	 	 //第一行，5*16列
	//write_string("第  关  ");	  //写入汉字，注意汉字必须从字符的偶数位开始写
	write_string(str_num);	  //write_string("1");
	
	write_cmd(0x88+5);
	write_string("用时:");
}

//功能：画点阵函数,用于画全屏图像
void draw_dianzhen(uchar *addr)
{	
	uchar x=0,y=0;		//初始化位置
	write_cmd(0x3f);//开扩充指令操作数据位选择8位
   	delay(2);
   	write_cmd(0x3e);//写指令扩充指令操作，8位数据，绘图开关关 
  	delay(2);

	for(y=0;y<32;y++)	//上面32行
	{
     	
   		for(x=0;x<8;x++)	//8*8*2列
   		{
   			write_cmd((0x80+y));  //写指令设置 垂直 列地址
   			delay(1);
    		write_cmd((0x80+x));  //写指令设置 水平 行地址
       		delay(1);  
  			write_data(*addr++);  //*addr指向调用该函数时传入的数组，且写一次数据后自加1，指向后一位数据        	
  			write_data(*addr++);  //先写高8位，再写低8位
    	}
	} 
//***********以上是上半屏（Y=0-1F,X=0-07）以下是下半屏Y=8-0F X=0-1F******************************** 
	x=0;y=0;//这里XY清零是因为下半屏是从88H开始的，而上半屏是从80H开始的，其实这里就是用到了上边的，只是改了个水平坐标值，垂直坐标还是从0-31 
	for(y=0;y<32;y++)
	{
     	x=0;
   		for (x=0;x<8;x++)		//自减比自加占用代码空间更小
   		{
			write_cmd((0x80+y));	//写指令设置 垂直 列地址
   			delay(1);
    		write_cmd((0x88+x));	//写指令设置 水平 行地址
       		delay(1); 
   			write_data(*addr++);	//同上
   			write_data(*addr++);
		}
	}
	write_cmd(0x30);  //转回普通指令
}


//功能：画小图标函数,x(0~7)、y(0~7)表示绘图位置，num范围0~5，表示地图信息
void display(uchar x, uchar y, uchar num_p, uchar num_q)	  
{
  	uchar j,*p,*q;
  	p = &elements[num_p];
	q = &elements[num_q];
  	write_cmd(0x34);  //开扩充指令操作数据位选择8位
   	delay(2);
   	write_cmd(0x3e);  //写指令扩充指令操作，8位数据，绘图开关关
	
	if(x<4)		//小于4上半屏，大于等于4下半屏，加8
	{
		for(j=0;j<8;j++)  //i只需1次循环，因此删掉了
    	{
	  		write_cmd(0x80+j + 8*x);	//先往下数，再往右数
	  		write_cmd(0x80 + y);
			write_data(*p++);
			write_data(*q++);
		}	
	}
	else
	{  
		for(j=0;j<8;j++)  //i只需1次循环，因此删掉了
    	{
	  		write_cmd(0x80+j + 8*(x-4));	//先往下数，再往右数
	  		write_cmd(0x88 + y);
			write_data(*p++);
			write_data(*q++);
		}		
	}
  	write_cmd(0x30);  //转回普通指令
}


//功能：显示游戏本关卡游戏界面，n为关卡代号
void game_level(uchar n)
{
	uchar i,j;
	//新的一关，CR=0,步数清零，CR=1，允许计数
	step=0;
	count_CR = 0;
	delay(1);  
	count_CR = 1;

	for(i=0;i<8;i++)                                      //行
	{
		for(j=0;j<8;j++)                                  //列
		{
			if(level[n][i][j]==1) 	  //找到人物
			{
				man_x=j;
				man_y=i;
			}
			level_temp[i][j]=level[n][i][j];       //当前关卡存入数组
		}
	}
	for(i=0;i<8;i++)
		for(j=0;j<8;j+=2)
			display(i,j/2,level_temp[i][j],level_temp[i][j+1]);          //显示当前关卡，一次写入两个图标
																										  	
	clc(32,4,4);   //清空右半屏以显示关卡数和时间
	display_level(n);
	time_change(Timtcount);			
}


//游戏开机界面函数
void start_interface()
{
	uchar j;
	draw_dianzhen(start);	//开机画面
	for(j=0;j<8;j++)	//开始动画
	{
		display(5,j,1,0);	   //0,1分别代表空白和小人
		delay(100);
		display(5,j,0,1);	 //5表示第5行
		delay(100);
		if(j!=0)	  //第一次不用擦除
			display(5,j-1,0,0);	//	擦除前面的小人痕迹
		delay(100);
	}
}


void t1_tt() interrupt 3	   //定时器3
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

//功能：判断是否通过本关卡(箱子数目为0为判断条件)
void level_suc()
{
	uchar i,j;
	uchar wait=3;
	uchar suc_flag=1;
	for(i=0; i<8; i++)	
	{
		for(j=0; j<8; j++)
		{/**	                                                     
			if((level[n][i][j]==4) || (level[n][i][j]==5))	//循环扫描目标位置,任意一个上面没有箱子,就不过关
			{
				if(level_temp[i][j]!=5)
					suc_flag=0;	
			}  **/
			if(level_temp[i][j]==3)	  	//循环扫描地图，若箱子都变成5，即没有箱子3，此关卡就通过
				suc_flag = 0;
		}
	}

	if(step>50)		//步数大于99时，挑战失败！
	{  
		sing(21,0,250);

		TR0=0;	//关闭定时器
		write_cmd(0x84);
		write_string("        ");
		while(wait--)	
			delay(8);
		write_cmd(0x84);
		write_string("挑战失败");
		while(wait--)	
			delay(8);
		write_cmd(0x84);
		write_string("        ");
		while(wait--)	
			delay(8);
		write_cmd(0x84);
		write_string("大侠请  ");  //空格为了擦除前面的“败”字
		while(wait--)	
			delay(8);
		write_cmd(0x84);
		write_string("        ");
		while(wait--)	
			delay(8);
		write_cmd(0x84);
		write_string("重新来过\xfd");	//keil的一个bug，部分汉字因冲突不能正常显示，需在后面加\xfd
		while(wait--)	
			delay(24);

		playing=0;
		Timtcount=0;
		clear_hanzi();
		clc(32,8,0);					
		start_interface();	//回到主界面	   
	}	

	if(suc_flag==1)		 //所有目标位置都有箱子
	{	
		sing(18,1,250);
		
		TR0=0; 	//关闭定时器
		write_cmd(0x84);
		write_string("        ");
		while(wait--)	
			delay(8);
		write_cmd(0x84);
		write_string("挑战成功");
		while(wait--)	
			delay(8);
		write_cmd(0x84);
		write_string("        ");
		while(wait--)	
			delay(8);		 

		if(n<8)
		{
			write_cmd(0x84);
			write_string("即将进入");
			while(wait--)	
				delay(8);
			write_cmd(0x84);
			write_string("        ");
			while(wait--)	
				delay(8);
			write_cmd(0x84);
			write_string("下一关  ");   //空格为了擦除前面的“入”字
			while(wait--)	
				delay(8);
			write_cmd(0x84);
			write_string("        ");
			while(wait--)	
				delay(8);
			TR0=1;
			
			n+=1;  	//下一关
			Timtcount=0;	//计时清零
			TH0=(65536-50000)/256;
			TL0=(65536-50000)%256;
			tcount=0;
			game_level(n);
		}
		else 
		{
			write_cmd(0x84);
			write_string("即将回到");
			while(wait--)	
				delay(8);
			write_cmd(0x84);
			write_string("        ");
			while(wait--)	
				delay(8);
			write_cmd(0x84);
			write_string("主界面  ");
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
			start_interface();	//通关后回到主界面
		}			
	}
}

//按键检测程序			
/***加了松手检测功能：(successed!)
1.按下,loose=1 -> return 非9
2.按下,loose=0 -> return 9
3.松开,loose=1 -> return 9，且解除屏蔽
4.松开,loose=0 -> return 9
***/
uchar key_scan()	 
{	  	
	static uchar key_loose=1;	 //松手检测标志
	uchar tmp; 
	uchar keys_state = P1 & 0x3f;	//只检测低6位端口,高两位置0，不受其影响
	uchar keyno=0;
	if( key_loose && (keys_state != 0x3f) )	 //有按键按下且未被屏蔽
	{
		key_loose=0;
		delay(200); 		//去抖动,抖动后keyno仍然为0
		if(keys_state != (P1 & 0x3f))	//两次按键不一致，不会响应
			keyno = 0;
		else	  
		{
			//按键后00111111将变成00×× ××××，×中有一个为0,其余为1
			//下面的异或操作会把5个1变成0，唯一的0变成1
			tmp = keys_state ^ 0x3f;
			switch(tmp)		//判断哪个按键被按下
			{
				case 1: keyno = 1; break;
				case 2: keyno = 2; break;
				case 4: keyno = 3; break;
				case 8: keyno = 4; break;
				case 16: keyno = 5;break;
				case 32: keyno = 6;break;
				default:keyno = 0;		//无键按下	
			}  
		}
	}
	else if(keys_state == 0x3f)	  //若按键全部恢复
		key_loose=1;	//解除屏蔽
	return keyno; 
			  
/***原版检测程序，太过复杂，优化了一下
	static uchar key_loose=1;		//按键松开标志	
	if(key_loose&&(key_up==0||key_down==0||key_left==0||key_right==0))
	{
		delay(10);	//去抖
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
	else if(key_up==1&&key_down==1&&key_left==1&&key_right==1)  //若按键全部松开
		key_loose=1; 
	return 0;		// 无按键按下	   
***/	
}					 

//步数加1函数
void add_step()
{
	step += 1;
	count_step = 0;
	delay(1);
	count_step = 1;
}

//画图标函数,每次画两个，即16*8
void printelement(uchar xh, uchar yh, uchar elum)
{
	if(xh%2 == 0)	
		display(yh, xh/2, elum,level_temp[yh][xh+1]);
	else
		display(yh, xh/2, level_temp[yh][xh-1], elum);
}

void state1(uchar plus,uchar direction)	 //plus=1,代表下移;plus=-1,代表上移
{
	if(level[n][man_y][man_x]==4||level[n][man_y][man_x]==5)           //if 本位置原来==目标or成功
	{
		level_temp[man_y][man_x]=4;		//人走后变成目标
		printelement(man_x,man_y,4); 
	}
	else
	{
		level_temp[man_y][man_x]=0;		//人走后变成空地
		printelement(man_x,man_y,0);  
	}
	man_y=man_y + plus;					//上=人//完成一次移动
	add_step();		//步数加一
	sing(9,1,30);
	level_temp[man_y][man_x]=1;
	printelement(man_x,man_y,direction);
}

//功能：按向上键的操作
void move_up()
{
	if(level_temp[man_y-1][man_x]==0||level_temp[man_y-1][man_x]==4)	     //if 人上==空or目标   
		state1(-1,6);  
	
	else if(level_temp[man_y-1][man_x]==3)		 	//if 人上==箱
	{				  
		if(level_temp[man_y-2][man_x]==0)	   		//if 再上==空白
		{
			state1(-1,6);
			level_temp[man_y-1][man_x]=3;		//再上=箱//完成一次移动
			printelement(man_x,man_y-1,3);
		}
		else if(level_temp[man_y-2][man_x]==4)		//if 再上==目
		{
			state1(-1,6);
			level_temp[man_y-1][man_x]=5;											 //再上=成
			printelement(man_x,man_y-1,5); 			
		} 
	}

	else if(level_temp[man_y-1][man_x]==5)	       //人上==成
	{		   
		if(level_temp[man_y-2][man_x]==0)		 //再上？=空
		{
			state1(-1,6);
			level_temp[man_y-1][man_x]=3;		//再上=箱
			printelement(man_x,man_y-1,3);
		}
		else if(level_temp[man_y-2][man_x]==4)	  //再上?=目
		{
			state1(-1,6);
		    level_temp[man_y-1][man_x]=5;		//再上=成
		    printelement(man_x,man_y-1,5);	 		    
		}  			
	}
	level_suc();       //判断是否完成本关				 
}


//功能：按向下键的操作
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

void state2(uchar plus,uchar direction)	 //plus=1,代表右移;plus=-1,代表左移
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
	add_step();		//步数加一
	sing(9,1,30);
	level_temp[man_y][man_x]=direction;	  //本来是1，表示更新人物位置
	printelement(man_x,man_y,direction);
}

//功能：按向左键的操作
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

//功能：按向右键的操作
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


//选关界面
void chooose_level()
{ 
	uchar keyupdate,i;
	uchar down=1;
	uchar position[9] = {0x80,0x90,0x88,0x98,0x83,0x93,0x8b,0x9b,0x86};		  //显示的地址索引
	uchar level_name[6] = {" Lv "}; 
	clear_hanzi();	//字库写入的汉字要写入空格清除
	clc(32,8,0);	//先清屏

	for(i=0;i<9;i++) 	  //显示关卡名
	{
		write_cmd(position[i]); 
		level_name[3] = i+1 + '0';
		write_string(level_name);
	}
	
	while(down)			 //等待确定键按下
	{
		write_cmd(position[n]);    //指示标识“>”
		write_string(">");
		delay(20);
		write_cmd(position[n]);    //画空白,擦除旧的标识
		write_string(" ");

		keyupdate=key_scan();
		switch(keyupdate)
		{
			case 1:
				if(n>3)		//第二列或n=9,按左键才有效
					n -= 4;
				break;
			case 2:
				if(n!=0)	//第1关按上键无效
					n -= 1;
				break;
			case 3:
				if(n!=8)	//第9关按下键无效
					n += 1;
				break;
			case 4:
				if(n<5)		//第一列或n=4,按右键才有效
					n += 4;
				break;
			case 6:
				down = 0;	 //退出循环
				break;
		}
	}
}

//功能：推箱子游戏规则
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
				game_level(n);		  //显示下一关
				Timtcount=0;
				TH0=(65536-50000)/256;
				TL0=(65536-50000)%256;
				tcount=0;
				break;		**/
				TR0=0;	 //关闭定时器0
				chooose_level();
				clear_hanzi();	//字库写入的汉字要写入空格清除
				Timtcount=0;
				TH0=(65536-50000)/256;
				TL0=(65536-50000)%256;
				tcount=0;
				TR0=1;	 //重新启动定时器0
				game_level(n);	//进入到选择的关卡
				break;
		case 6:		  //重新开始按钮
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
	count_CR=1;	   //允许计数
	start_interface();
	//write_cmd(0x0c); 
	while(1)
	{
		while(key_scan()!=6);			 //等待确定键按下

		playing=1;
		EA=1; 
		n=0;
		chooose_level();	//进入选关界面
		clear_hanzi();	//字库写入的汉字要写入空格清除		
		game_level(n);		  //显示第一关 
		Timtcount=0;
		TR0=1;
		while(playing)		  //游戏中...
		{
		 	put_box();	 
		}
	}		
	  
	  //*******************************************************
      //把12864分成两部分，上半部分12832，下半部分12832
      //然后用扫描的办法从第一行开始扫描，直到32行，再执行下半部分扫描程序，第33行开始，扫描32行到64行
      //这里还要说一个事：关于12864画图。所以要想在12864屏幕上显示一幅图像，你就要
      //从一个点开始，第一步我让第一行的第一列显示一个点，怎么实现呢？我开始很是不明白，看了一天的手册，一下午的实验，终于总结出
      //1：向7920的写入3F和3E指令，使液晶工作在扩展指令状态，8位数据，绘图开关开
	  //2：设定绘图RAM地址，先写入列（Y轴）再写入行（X轴）
      //然后再一口气写入16位的数据，也可以写8位，但要写2次，我是采取后者，然后这十六是什么呢？---即这样的排列D15-D0，让第一个点就写入8000H
      //那么，现在就是在第一行第一列显示一个点！会写一个点，你就一定会写一幅图像了，为什么呢，你只要让单片机这样重复的写512个字就是有一幅
      //12864的图像了，这其实说白了就是点阵啊
}