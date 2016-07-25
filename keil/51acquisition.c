#include<intrins.h>
#include <reg51.h>
#include <string.h> 
							   //p3^0 p3^1  txd   rxd	    P3�ڿ��� 
sbit AD7606_CONVSTA=P3^2;	  //��CONVSTABֱ������һ��	
sbit AD7606_CONVSTB=P3^3;      //sbit AD7606_CONVSTA=P2^1;  
sbit AD7606_REST=P3^4;
sbit AD7606_RD=P3^5;
sbit AD7606_BUSY=P3^6;

sbit CS_CONTROL_A=P1^6;
sbit CS_CONTROL_B=P1^7;
sbit CS_CONTROL_C=P3^7;			//��ԭ��2�����õ�FRST  VIO�˿� �޸�Ϊ   CS0 CS1 CS2  ����3-8������������6��ģ���CS��  


	

sbit SN_A=P1^0;				 //P1.0-P1.5    6���˿�������ɨ���·��48������
sbit SN_B=P1^1;
sbit SN_C=P1^2;
sbit SN_D=P1^3;
sbit SN_E=P1^4;
sbit SN_F=P1^5;

/*
sbit AD7606_DB0=P0^0;
sbit AD7606_DB1=P0^1;
sbit AD7606_DB2=P0^2;
sbit AD7606_DB3=P0^3;
sbit AD7606_DB4=P0^4;
sbit AD7606_DB5=P0^5;
sbit AD7606_DB6=P0^6;
sbit AD7606_DB7=P0^7;

sbit AD7606_DB8=P2^0;
sbit AD7606_DB9=P2^1;
sbit AD7606_DB10=P2^2;
sbit AD7606_DB11=P2^3;
sbit AD7606_DB12=P2^4;
sbit AD7606_DB13=P2^5;
sbit AD7606_DB14=P2^6;
sbit AD7606_DB15=P2^7;


				  */

void int_serialcom( void ) //����ͨ�ų�ʼ�趨
      {
	   SCON = 0x50 ;   //UARTΪģʽ1��8λ���ݣ��������
	   TMOD = 0x20 ; //��ʱ��1Ϊģʽ2,8λ�Զ���װ
	   PCON = 0x00 ; //SMOD=1; SMOD�����⹦�ܼĴ���PCON�ĵ�7λ,���Կ��Ʋ����ʵģ�SMOD=1ʱ����ͨѶ�Ĳ����ʱ�����Ϊ0ʱ�����ʲ�������
	   TL1 = 0xFD ;
	   TH1 = 0xFD ;   //Baud:19200 fosc="11".0592MHz
	  
	   IE = 0x90 ;     //Enable Serial Interrupt 
	   TR1 = 1 ;       // timer 1 run 	
	   TI=1; 
	   }
/*	   MOV TMOD,#20H
	 MOV TL1,#0FDH
	 MOV TH1,#0FDH
	 MOV PCON,#00H
	 MOV SCON,#50H
	 MOV IP, #10H
	 ;SETB EA				
	 ;SETB ES
;SETB EX0
SETB IT0
SETB TR1  */
void delay(int i)		//��Сʱ���һ����ʱ
              { 
               int n;
               for(n=0;n<i;n++);
              }	  
                   
void send_char_com( char ch) 	  //�򴮿ڷ���һ���ַ� 
	   {		 
	    SBUF=ch;
		while (TI== 0); 
		TI= 0 ;
		} 

void AD7606_Int()			//AD4066��ʼ�� 
{		
        AD7606_RD=1;
       CS_CONTROL_C=0; P1=0x00;    // AD7606_CS0=1;	  000 3-8������ѡ�е�һ����LED����
        AD7606_CONVSTA=1;    
		 AD7606_CONVSTB=1;                         
		 		delay(10000);
        AD7606_REST=1;
       // delay(8);
        AD7606_REST=0;
		  
}
char Date[95];      //ȫ�ֱ�������

unsigned int scan;  
void AD7606_Read()		     //AD4066 ���ݲɼ� �������ӳ��� ���̷��ͳ�ȥ
{
        int i;int j;int byte_num=0; unsigned int H_Date;unsigned int L_Date; unsigned int m=0x40;   
      	  //�ŵ�  
	
	  //	delay();	   
	   	AD7606_CONVSTA=0;	
     	AD7606_CONVSTA=1;
	  //  delay();
			  AD7606_CONVSTB=0;
			   AD7606_CONVSTB=1;
     
     while(AD7606_BUSY)					  //���� 6��Ƭ�ֵ� BUSY ���ܲ���һֱ���ָߵ�ѹ�� 
	{
		scan=P1;	  CS_CONTROL_C=0; 
       for(j=0;j<6;j++) 
      {   
	        if(j==3)
	           {  CS_CONTROL_C=1; scan=scan+m;
			   }  
			  
         scan=scan+m; P1=scan;   //delay(100000);               // AD7606_CS0=0;
       //  send_char_com(0x99);
        for(i=0;i<8;i++) 
         {        	
				AD7606_RD=0;
            //   delay();
			    H_Date=P2; 	   
                L_Date=P0;
			// 	  delay() ;
				AD7606_RD=1;
	    	/*	 if (i==0)
			     {	 send_char_com(H_Date);	 
				 }	 */
             Date[byte_num++] =H_Date;	   //����DATE[]�����������  ��������ֱ�Ӱѵõ��ĸ��ֽڷ��ͳ�ȥ�� DATE[]Ҳ��û���õ�
			//send_char_com(H_Date);
             Date[byte_num++] =L_Date;
		   	//send_char_com(L_Date);
         }
		  // send_char_com(0x99);		//send_char_com(c);  delay();	send_char_com(c);
	                                        
	  }
	  } 
	  //	AD7606_CONVSTA=0;                                                        
	 //   AD7606_CONVSTB=0;     
}
void AD7606_Send()	  
		 {	  	 int i=48;char Data;int k; 
		     send_char_com(0x99);	
			  for(k=1;k<49;k++)							 //1-48������K��ʾ    1��k 2��i
		     {    
				   send_char_com(k);
				   if(k%2==1)
                     {	 
					     Data=Date[i++];   send_char_com(Data);
					     Data=Date[i];  i=i-49;	   send_char_com(Data); 
					 }
				   else
                     {	 
					     Data=Date[i++]; 	send_char_com(Data);
					     Data=Date[i];  i=i+49;		send_char_com(Data);
					 }
				  
			  }
			  send_char_com(0x99);	send_char_com(0x99);
		 }
void c_charge()
{             send_char_com(0x01);  				                  //��	 ������� ����ȱ��
        /*     for(i=0;i<47;i++)
			{        int can=0x01;
			        char linenumber;	
			         linenumber=can;
			        send_char_com(linenumber);
				    P1=can; 
			   //     delay(1000000);
					AD7606_Read();
					AD7606_Send();
					can=can+n;	
					if(i==6)
					{can=0x10;
					}  
					else if(i==22)
					{can=0x20;
					}
					else if(i==30)
					{can=0x30;
					}
			}
		*/
      //P1=0x00;	delay(1000000); //ÿɨ��һ������һ�����嵽��ʱ  ����6��ADоƬ�����ݲɼ�	ǰ���Ѿ���P1����0 �ټ�һ�䵼������ʧ��
		P1=0x20; 	delay(0);	 send_char_com(0x01);     AD7606_Read();   AD7606_Send();
		P1=0x01;	delay(0);	 send_char_com(0x02);	  AD7606_Read();   AD7606_Send();
		P1=0x21;	delay(0);	 send_char_com(0x03);	  AD7606_Read();   AD7606_Send();
		P1=0x02;	delay(0);	 send_char_com(0x04);	  AD7606_Read();   AD7606_Send();  //  ����Ҫ��Ծ
		P1=0x22;	delay(0);	 send_char_com(0x05);	  AD7606_Read();   AD7606_Send();
		P1=0x03;	delay(0);	 send_char_com(0x06);	  AD7606_Read();   AD7606_Send();
		P1=0x23;	delay(0);	 send_char_com(0x07);	  AD7606_Read();   AD7606_Send();
		P1=0x04;	delay(0);	 send_char_com(0x08);	  AD7606_Read();   AD7606_Send();
		P1=0x24;	delay(0);	 send_char_com(0x09);	  AD7606_Read();   AD7606_Send();
		P1=0x05;	delay(0);	 send_char_com(0x10);	  AD7606_Read();   AD7606_Send();
		P1=0x25;	delay(0);	 send_char_com(0x11);	  AD7606_Read();   AD7606_Send();
		P1=0x06;	delay(0);	 send_char_com(0x12);	  AD7606_Read();   AD7606_Send();
		P1=0x26;	delay(0);	 send_char_com(0x13);	  AD7606_Read();   AD7606_Send();
		P1=0x07;	delay(0);	 send_char_com(0x14);	  AD7606_Read();   AD7606_Send();
		P1=0x27;	delay(0);	 send_char_com(0x15);	  AD7606_Read();   AD7606_Send();

		P1=0x10; 	delay(0);	 send_char_com(0x16);     AD7606_Read();   AD7606_Send();
		P1=0x30; 	delay(0);	 send_char_com(0x17);     AD7606_Read();   AD7606_Send();
		P1=0x11;	delay(0);	 send_char_com(0x18);	  AD7606_Read();   AD7606_Send();
		P1=0x31;	delay(0);	 send_char_com(0x19);	  AD7606_Read();   AD7606_Send();
		P1=0x12;	delay(0);	 send_char_com(0x20);	  AD7606_Read();   AD7606_Send();  //  ����Ҫ��Ծ
		P1=0x32;	delay(0);	 send_char_com(0x21);	  AD7606_Read();   AD7606_Send();
		P1=0x13;	delay(0);	 send_char_com(0x22);	  AD7606_Read();   AD7606_Send();
		P1=0x33;	delay(0);	 send_char_com(0x23);	  AD7606_Read();   AD7606_Send();
		P1=0x14;	delay(0);	 send_char_com(0x24);	  AD7606_Read();   AD7606_Send();
		P1=0x34;	delay(0);	 send_char_com(0x25);	  AD7606_Read();   AD7606_Send();
		P1=0x15;	delay(0);	 send_char_com(0x26);	  AD7606_Read();   AD7606_Send();
		P1=0x35;	delay(0);	 send_char_com(0x27);	  AD7606_Read();   AD7606_Send();
		P1=0x16;	delay(0);	 send_char_com(0x28);	  AD7606_Read();   AD7606_Send();
		P1=0x36;	delay(0);	 send_char_com(0x29);	  AD7606_Read();   AD7606_Send();
		P1=0x17;	delay(0);	 send_char_com(0x30);	  AD7606_Read();   AD7606_Send();
		P1=0x37;	delay(0);	 send_char_com(0x31);	  AD7606_Read();   AD7606_Send();
		P1=0x18; 	delay(0);	 send_char_com(0x32);     AD7606_Read();   AD7606_Send();
		P1=0x38; 	delay(0);	 send_char_com(0x33);     AD7606_Read();   AD7606_Send();
		P1=0x19;	delay(0);	 send_char_com(0x34);	  AD7606_Read();   AD7606_Send();
		P1=0x39;	delay(0);	 send_char_com(0x35);	  AD7606_Read();   AD7606_Send();
		P1=0x1A;	delay(0);	 send_char_com(0x36);	  AD7606_Read();   AD7606_Send();  //  ����Ҫ��Ծ
		P1=0x3A;	delay(0);	 send_char_com(0x37);	  AD7606_Read();   AD7606_Send();
		P1=0x1B;	delay(0);	 send_char_com(0x38);	  AD7606_Read();   AD7606_Send();
		P1=0x3B;	delay(0);	 send_char_com(0x39);	  AD7606_Read();   AD7606_Send();
		P1=0x1C;	delay(0);	 send_char_com(0x40);	  AD7606_Read();   AD7606_Send();
		P1=0x3C;	delay(0);	 send_char_com(0x41);	  AD7606_Read();   AD7606_Send();
		P1=0x1D;	delay(0);	 send_char_com(0x42);	  AD7606_Read();   AD7606_Send();
		P1=0x3D;	delay(0);	 send_char_com(0x43);	  AD7606_Read();   AD7606_Send();
		P1=0x1E;	delay(0);	 send_char_com(0x44);	  AD7606_Read();   AD7606_Send();
		P1=0x3E;	delay(0);	 send_char_com(0x45);	  AD7606_Read();   AD7606_Send();
		P1=0x1F;	delay(0);	 send_char_com(0x46);	  AD7606_Read();   AD7606_Send();
		P1=0x3F;	delay(0);	 send_char_com(0x47);	  AD7606_Read();   AD7606_Send();
	

	
	/*
		P1=0x10;	delay(1000000);	      AD7606_Read();   AD7606_Send(); //	   ��16��  0x40    0001 0000
		P1=0x11;	delay(1000000);	//	  AD7606_Read();   AD7606_Send();
		P1=0x12;	delay(1000000);	//	  AD7606_Read();   AD7606_Send();
		P1=0x13;	delay(1000000);	//	  AD7606_Read();   AD7606_Send();
		P1=0x14;	delay(1000000);	//	  AD7606_Read();   AD7606_Send();
		P1=0x15;	delay(1000000);	//	  AD7606_Read();   AD7606_Send();
		P1=0x16;	delay(1000000);	//	  AD7606_Read();   AD7606_Send();
		P1=0x17;	delay(1000000);	//	  AD7606_Read();   AD7606_Send();				 
		P1=0x18;	delay(1000000);	//	  AD7606_Read();   AD7606_Send();
		P1=0x19;	delay(1000000);	//	  AD7606_Read();   AD7606_Send();
		P1=0x1A;	delay(1000000);	//	  AD7606_Read();   AD7606_Send();
		P1=0x1B;	delay(1000000);	//	  AD7606_Read();   AD7606_Send();
		P1=0x1C;	delay(1000000);	//	  AD7606_Read();   AD7606_Send();
		P1=0x1D;	delay(1000000);	//	  AD7606_Read();   AD7606_Send();
		P1=0x1E;	delay(1000000);	//	  AD7606_Read();   AD7606_Send();
		P1=0x1F;	delay(1000000);  	//	  AD7606_Read();   AD7606_Send();
	*/
}

void main()							  //������
{	  while(1)
     { 	
	 int_serialcom();
	  AD7606_Int();
	   c_charge();	 
	  
	 // delay(100) ;
	 }
 }


end;