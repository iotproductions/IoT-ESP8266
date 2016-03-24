#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

int main()
{
	uint8_t i,j=0;
	char postion[40]; 
	char status[4]; 
	memset(postion,0,40);
    char * string = "{device:[\"OFF\",\"ON\",\"OFF\",\"ON\"]}"; //,"OFF"]}
    printf("Do dai chuoi: %d\n",strlen(string));
	for(i = 0; i < strlen(string);i++)
	{
		if(string[i] == ',')
		{
			postion[j++] = i;
		}	
	}
	printf("%s\n",string);
	for(i = 0; i < 3;i++)
	{
		printf("Vi tri dau phay: %d\n",postion[i]);
	}
	if(postion[0] == 13)//ON
		status[0] = 1;
	else 
		status[0] = 0;
	if((postion[1] - postion[0])== 5 )//ON
		status[1] = 1;
	else 
		status[1] = 0;
	if((postion[2] - postion[1])== 5 )//ON
		status[2] = 1;
	else 
		status[2] = 0;	
	if((strlen(string) - postion[2]) == 7 )//ON
		status[3] = 1;
	else 
		status[3] = 0;	
	for(i = 0; i < 4;i++)
	{
		printf("%d,",status[i]);
	}			
    return 0;
}
