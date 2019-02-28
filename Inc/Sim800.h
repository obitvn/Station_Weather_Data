#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define SIM800_UART_BUFFER_MAX_SIZE      512
#define SIM800_UART                      huart2

uint8_t Sim800_UartBuffer[SIM800_UART_BUFFER_MAX_SIZE];
uint16_t Sim800_UartRxIndex;
uint32_t Sim800_UartLastTime;
uint8_t  Sim800_DataBuff;



void obit_split_string(uint8_t *str_in, uint8_t size_str_in,uint8_t char_begin, uint8_t frequency_char_begin, uint8_t char_end, uint8_t frequency_char_end, uint8_t *str_out, uint8_t size_str_out)
{
	uint8_t done=0;
	uint8_t i=0, count_begin=0, count_end=0;
	memset((char*)str_out,'\0',size_str_out);
	for(i=0; i<size_str_in; i++)
	{
		if(str_in[i] == char_begin)
		{
			count_begin++;
			if(count_begin==frequency_char_begin)
			{
			for(int j=1; j<size_str_out+1; j++)
			 {
				
				if(str_in[i+j]==char_end)
				{
					count_end++;
					if(count_end==frequency_char_end)
					{
					done=1;
					break;
					}
					else
						str_out[j-1]=str_in[i+j];
				}
				else
				{
					str_out[j-1]=str_in[i+j];
				}
			 }
		  }
		}
		else
		{			
		 if(done==1) break;
	  }
	}
}

int obit_strcmp(uint8_t *str_expected,uint8_t size_str_expected, uint8_t *str_real, uint8_t size_str_real)
{
 int i=0;
 uint8_t next=0;
 while(i<size_str_real)
 {
	 if(str_expected[0]==str_real[i])
	 {
		 for(int j=0; j<size_str_expected; j++)
		 {
			 if(str_expected[j]!=str_real[j+i]) 
			 {
				 i++;
				 next=1;
				 break;
			 }
			 else
				 next = 0;
				 //return -1;
		 }
		 if(next==0) return 1;
	 }
	 else 
	 {
		 i++;
		 next=0;
	 }
 }
 return 0;
}

void Sim800_UartCallBack()
{
		Sim800_UartLastTime = HAL_GetTick();
		if(Sim800_UartRxIndex < SIM800_UART_BUFFER_MAX_SIZE - 1)
		{
			Sim800_UartRxIndex++;
		}
		Sim800_UartBuffer[Sim800_UartRxIndex] = 0; // clear 
		Sim800_UartBuffer[Sim800_UartRxIndex] = Sim800_DataBuff;
		HAL_UART_Receive_IT(&huart2,&Sim800_DataBuff,1);
}

void Sim800_SendRaw(uint8_t *data, uint16_t length)
{
	HAL_UART_Transmit(&SIM800_UART,data,length,100);
}

void Sim800_SendString(char *str)
{
	HAL_UART_Transmit(&SIM800_UART,(uint8_t*)str,strlen(str),100);
}

void Sim800_SendAtNotWaitResponse(char *str)
{
	HAL_UART_Transmit(&SIM800_UART,(uint8_t*)str,strlen(str),100);
}

uint8_t Sim800_SendAtCommand(uint8_t* AtCommand, uint8_t AtLength, uint32_t Time_out, uint8_t* answer, uint8_t AnsLength, uint8_t clear_buffer)
{
	
	// clear_buffer = 1 ==> xoa buffer khi nhan duoc chuoi chinh xac
	uint8_t str1;
	uint16_t SizeUartBuffer;
	uint32_t SendCommandStartTime = HAL_GetTick();
	uint32_t TimeCommandWait;
	//Sim800_SendAtNotWaitResponse(AtCommand);
	while(HAL_GetTick() - SendCommandStartTime < Time_out)
	{
		TimeCommandWait = HAL_GetTick() - Sim800_UartLastTime ;
		if(( TimeCommandWait > 50)&&(Sim800_UartRxIndex> AnsLength-1))
		{
        SizeUartBuffer = Sim800_UartRxIndex;
				str1 = obit_strcmp(answer,AnsLength,Sim800_UartBuffer,SizeUartBuffer);
			  //str1 = obit_strcmp("\r\nOK\r\n",6,"\r\n\r\nss\r\nOK\r\n",12);
				if(str1==1)
				{
					if(clear_buffer)
					{
					Sim800_UartRxIndex=0;
					memset((char*)Sim800_UartBuffer,0,SIM800_UART_BUFFER_MAX_SIZE);
					}
					return 1;
				}
				else
		    {
			  Sim800_SendAtNotWaitResponse(AtCommand);
			  HAL_Delay(200);
		    }
		}
		
		else
		{
			Sim800_SendAtNotWaitResponse(AtCommand);
			HAL_Delay(200);
		}
	}
	return 0;
}
uint8_t WaitForStringInTimeOut( uint8_t *str, uint16_t str_len, uint16_t time_out)
{
	uint32_t  StartTime = HAL_GetTick();
	while(HAL_GetTick() - StartTime < time_out)
	{
		if((HAL_GetTick() - Sim800_UartLastTime>50)&&(Sim800_UartRxIndex>4))
	  {
	   if(obit_strcmp(str,str_len,Sim800_UartBuffer,Sim800_UartRxIndex)) return 1;
		}
	}
	return 0;
}

void WaitForString( uint8_t *str, uint16_t str_len)
{
	while(obit_strcmp(str,str_len,Sim800_UartBuffer,Sim800_UartRxIndex) == 0);
}


uint16_t Sim800_CheckNewSMS()
{
	uint8_t string_index[3];
	uint16_t index_sms=0;
	uint8_t value;
	if((HAL_GetTick() - Sim800_UartLastTime>50)&&(Sim800_UartRxIndex>4))
	{
	value = obit_strcmp("+CMTI:",6,Sim800_UartBuffer,Sim800_UartRxIndex);
	if(value==1)
		{
			obit_split_string(Sim800_UartBuffer,Sim800_UartRxIndex,',',1,'\r',1,string_index,sizeof(string_index));
			       
			if(string_index[1]!='\0') 
			{
				if(string_index[2]!='\0')
				{
					 if(string_index[0]!='\0')
					 {
						  index_sms = (string_index[0]-48)*100+(string_index[1]-48)*10+(string_index[2]-48);
					 }
					 else
					 {
						 index_sms = '\0';
					 }
				}
				else
				{
				  index_sms =(string_index[0]-48)*10 + (string_index[1]-48);
				}
			}
			else index_sms = string_index[0]-48;
				
			Sim800_UartRxIndex=0;
			memset((char*)Sim800_UartBuffer,0,SIM800_UART_BUFFER_MAX_SIZE);
			return index_sms;
		}
	}
	return 0;
	
}

void Sim800_ReadSMS(uint16_t index, uint8_t *SMS_content, uint16_t size_content, uint8_t *SMS_header, uint16_t size_header)
{
	Sim800_UartRxIndex=0;
	memset((char*)Sim800_UartBuffer,0,SIM800_UART_BUFFER_MAX_SIZE);
	uint8_t string_send[13] = "AT+CMGR=";
	uint8_t len_string_send=0;
	if(index <10)
	{
		string_send[8] = (char) (index + 48);
		string_send[9] = '\r';
		string_send[10] = '\n';
		len_string_send =11;
	}
	else if((index>9)&&(index <100))
	{
		string_send[8] = (char) (index/10 + 48);
		string_send[9] = (char) (index%10 + 48);
		string_send[10] = '\r';
		string_send[11] = '\n';
		len_string_send =12;
	}
	else if((index>99)&&(index < 1000))
	{
		string_send[8] = (char) (index/100 + 48);
		string_send[9] = (char) (index%100/10 + 48);
		string_send[10] = (char) (index%10 + 48);
		string_send[11] = '\r';
		string_send[12] = '\n';
		len_string_send =13;
	}
	if(Sim800_SendAtCommand(string_send,len_string_send,2000,(uint8_t*)"+CMGR:",6,0)==1)
	{
		obit_split_string(Sim800_UartBuffer,Sim800_UartRxIndex,'"',1,'"',7,SMS_header,size_header);
		obit_split_string(Sim800_UartBuffer,Sim800_UartRxIndex,'\r',2,'\r',1,SMS_content,size_content);
	}
	
}

void Sim800L_Init()
{
	Sim800_SendAtCommand((uint8_t*)"ATE0\r\n",6,2000,(uint8_t*)"\r\nOK\r\n",6,1);
	Sim800_SendAtCommand((uint8_t*)"AT\r\n",4,2000,(uint8_t*)"\r\nOK\r\n",6,1);
	while(Sim800_SendAtCommand((uint8_t*)"AT+CPIN?\r\n",10,500,(uint8_t*)"+CPIN: READY",11,1)!=1)
	{
		HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);
	  HAL_Delay(100);
	}
	Sim800_SendAtCommand((uint8_t*)"AT+CMGF=1\r\n",11,2000,(uint8_t*)"\r\nOK\r\n",6,1);
}

void Sim800L_SendSMS(uint8_t *SMS, uint8_t SMS_len, uint8_t *PhoneNumber, uint8_t PhoneNumLen)
{
	char AT_SMS[60];
	uint8_t ctrl_z =26;
	sprintf(AT_SMS,"AT+CMGS=\"%s\"\r\n",PhoneNumber);
	Sim800_SendAtNotWaitResponse(AT_SMS);
	HAL_Delay(200);
	Sim800_SendAtNotWaitResponse(SMS);
	HAL_UART_Transmit(&SIM800_UART,&ctrl_z,1,100);// CTRL_Z
 
}