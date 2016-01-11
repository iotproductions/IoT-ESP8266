#include "algorithm.h"
#include "uart.h"
#include <msp430.h>

bool      sleep_status;             //0 sleep, 1 active
uint32_t T_cycle_level[16] = {0, T_cycle_level_1,  T_cycle_level_2,  T_cycle_level_3,  T_cycle_level_4,  T_cycle_level_5,
                            T_cycle_level_6,  T_cycle_level_7,  T_cycle_level_8,  T_cycle_level_9,  T_cycle_level_10,
                            T_cycle_level_11, T_cycle_level_12, T_cycle_level_13, T_cycle_level_14, T_cycle_level_15};

uint32_t K_level[15]       = {0, K_level_1,  K_level_2,  K_level_3,  K_level_4,  K_level_5,
                            K_level_6,  K_level_7,  K_level_8,  K_level_9,  K_level_10,
                            K_level_11, K_level_12, K_level_13, K_level_14};
uint32_t  T_end;
uint32_t  T_cycle;
uint32_t  calculator_T_cycle;
uint32_t  percent_T_cycle;
uint32_t  constant_k;
uint32_t  fulseCounter;
uint32_t  water_value_m3;           // Water volume(m3)
uint16_t  water_value_lt;	    // Water volume(Lit)
uint16_t  pin_percent;		    // Battery level(%)

void water_counter_algorithm(void)
{
	if(T_cycle < T_cycle_level[1])   // he so k la lon nhat n
	{
		constant_k = K_level[1];
	}
	else if(T_cycle < T_cycle_level[2]) // he so k tu n->m
	{
		calculator_T_cycle = T_cycle;
		calculator_T_cycle -= T_cycle_level[1];
		percent_T_cycle = (calculator_T_cycle * 100) / (T_cycle_level[2] - T_cycle_level[1]);
		constant_k = K_level[1] - ((percent_T_cycle * (K_level[1] - K_level[2]))/100);
	}
	else if(T_cycle < T_cycle_level[3]) // he so k tu n->m
	{
		calculator_T_cycle = T_cycle;
		calculator_T_cycle -= T_cycle_level[2];
		percent_T_cycle = (calculator_T_cycle * 100) / (T_cycle_level[3] - T_cycle_level[2]);
		constant_k = K_level[2] - ((percent_T_cycle * (K_level[2] - K_level[3]))/100);
	}
	else if(T_cycle < T_cycle_level[4]) // he so k tu n->m
	{
		calculator_T_cycle = T_cycle;
		calculator_T_cycle -= T_cycle_level[3];
		percent_T_cycle = (calculator_T_cycle * 100) / (T_cycle_level[4] - T_cycle_level[3]);
		constant_k = K_level[3] - ((percent_T_cycle * (K_level[3] - K_level[4]))/100);
	}
	else if(T_cycle < T_cycle_level[5]) // he so k tu n->m
	{
		calculator_T_cycle = T_cycle;
		calculator_T_cycle -= T_cycle_level[4];
		percent_T_cycle = (calculator_T_cycle * 100) / (T_cycle_level[5] - T_cycle_level[4]);
		constant_k = K_level[4] - ((percent_T_cycle * (K_level[4] - K_level[5]))/100);
	}
	else if(T_cycle < T_cycle_level[6]) // he so k tu n->m
	{
		calculator_T_cycle = T_cycle;
		calculator_T_cycle -= T_cycle_level[5];
		percent_T_cycle = (calculator_T_cycle * 100) / (T_cycle_level[6] - T_cycle_level[5]);
		constant_k = K_level[5] - ((percent_T_cycle * (K_level[5] - K_level[6]))/100);
	}
	else if(T_cycle < T_cycle_level[7]) // he so k tu n->m
	{
		calculator_T_cycle = T_cycle;
		calculator_T_cycle -= T_cycle_level[6];
		percent_T_cycle = (calculator_T_cycle * 100) / (T_cycle_level[7] - T_cycle_level[6]);
		constant_k = K_level[6] - ((percent_T_cycle * (K_level[6] - K_level[7]))/100);
	}
	else if(T_cycle < T_cycle_level[8]) // he so k tu n->m
	{
		calculator_T_cycle = T_cycle;
		calculator_T_cycle -= T_cycle_level[7];
		percent_T_cycle = (calculator_T_cycle * 100) / (T_cycle_level[8] - T_cycle_level[7]);
		constant_k = K_level[7] - ((percent_T_cycle * (K_level[7] - K_level[8]))/100);
	}
	else if(T_cycle < T_cycle_level[9]) // he so k tu n->m
	{
		calculator_T_cycle = T_cycle;
		calculator_T_cycle -= T_cycle_level[8];
		percent_T_cycle = (calculator_T_cycle * 100) / (T_cycle_level[9] - T_cycle_level[8]);
		constant_k = K_level[8] - ((percent_T_cycle * (K_level[8] - K_level[9]))/100);
	}
	else if(T_cycle < T_cycle_level[10]) // he so k tu n->m
	{
		calculator_T_cycle = T_cycle;
		calculator_T_cycle -= T_cycle_level[9];
		percent_T_cycle = (calculator_T_cycle * 100) / (T_cycle_level[10] - T_cycle_level[9]);
		constant_k = K_level[9] - ((percent_T_cycle * (K_level[9] - K_level[10]))/100);
	}
	else if(T_cycle < T_cycle_level[11]) // he so k tu n->m
	{
		calculator_T_cycle = T_cycle;
		calculator_T_cycle -= T_cycle_level[10];
		percent_T_cycle = (calculator_T_cycle * 100) / (T_cycle_level[11] - T_cycle_level[10]);
		constant_k = K_level[10] - ((percent_T_cycle * (K_level[10] - K_level[11]))/100);
	}
	else if(T_cycle < T_cycle_level[12]) // he so k tu n->m
	{
		calculator_T_cycle = T_cycle;
		calculator_T_cycle -= T_cycle_level[11];
		percent_T_cycle = (calculator_T_cycle * 100) / (T_cycle_level[12] - T_cycle_level[11]);
		constant_k = K_level[11] - ((percent_T_cycle * (K_level[11] - K_level[12]))/100);
	}
	else if(T_cycle < T_cycle_level[13]) // he so k tu n->m
	{
		calculator_T_cycle = T_cycle;
		calculator_T_cycle -= T_cycle_level[12];
		percent_T_cycle = (calculator_T_cycle * 100) / (T_cycle_level[13] - T_cycle_level[12]);
		constant_k = K_level[12] - ((percent_T_cycle * (K_level[12] - K_level[13]))/100);
	}
	else if(T_cycle < T_cycle_level[14]) // he so k tu n->m
	{
		calculator_T_cycle = T_cycle;
		calculator_T_cycle -= T_cycle_level[13];
		percent_T_cycle = (calculator_T_cycle * 100) / (T_cycle_level[14] - T_cycle_level[13]);
		constant_k = K_level[13] - ((percent_T_cycle * (K_level[13] - K_level[14]))/100);
	}
	else if(T_cycle < T_cycle_level[15]) // he so k tu n->m
		constant_k = K_level[14];
	else
	{
		T_cycle = T_cycle_level[1];
		constant_k = K_level[1];
	}
	fulseCounter += T_cycle * constant_k;
	
	//comment_test
	if(fulseCounter >= NUMBER_PULSE_PER_LIT)
	{
		fulseCounter = fulseCounter - NUMBER_PULSE_PER_LIT; // tich luy gia tri cho lan toi
		water_value_lt++;
		P1OUT |= BIT0;
		_NOP();
		uart_puts("1 lit");
		_NOP();
		P1OUT &= ~BIT0;
		__delay_cycles(1000000);
		uart_puts("AT+SLEEP");
		P1OUT |= BIT0;
		
		if(water_value_lt>MQTT_MAX_WATER_INDEX_LIT)
		{
			water_value_m3 ++;
			water_value_lt = 0;
			if(water_value_m3 > MQTT_MAX_WATER_INDEX_CUBIC)
			{
				water_value_m3 = 0;
				water_value_lt = 0;
			
//				TxArray[0] = (water_value_m3>>24)&0xff;
//				TxArray[1] = (water_value_m3>>16)&0xff;
//				TxArray[2] = (water_value_m3>>8)&0xff;
//				TxArray[3] = (water_value_m3&0xff);
//				Write_Buffer_To_EEPROM_AT24C512(EEPROM_WATER_INDEX_M3_OFFSET,4,TxArray, Address_AT24C512C_1);
				
			}
		}
		if((water_value_lt % 100) == 0)
		{
//			TxArray[0] = (water_value_lt>>8)&0xff;  //Tach bien dem chi so nuoc: lit
//			TxArray[1] = (water_value_lt&0xff);
		}
	}
}
