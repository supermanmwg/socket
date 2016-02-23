#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "md5.h"


void Hex2Str(const char *sSrc, char *sDest, int nSrcLen )  
{  
    int  i;  
    char szTmp[3];  
  
    for( i = 0; i < nSrcLen; i++ )  
    {  
        sprintf( szTmp, "%02x", (unsigned char) sSrc[i] );  
        memcpy( &sDest[i * 2], szTmp, 2 );  
    }  
    return ;  
}  

char* encode(char key[32], char str[12]) {
	int i,j;
	unsigned char decrypt[16];    
	char salt[8];
	
	memset(salt, 0, sizeof(salt));
	GetSalt(salt, key, 0, 0);

	for(j = 7; j >= 0; j--) {
		printf("salt[%d]:%c\n", j, salt[j]);
	}

	char temp[20];
	memcpy(temp, str, 12);
	memcpy(temp+12, salt, 8);

	MD5_CTX md5;
	MD5Init(&md5);         		
	MD5Update(&md5,temp,sizeof(temp));
	MD5Final(&md5,decrypt);        
	printf("加密前:%s\n加密后:",temp);
	for(i=0;i<16;i++)
	{
		printf("%02x",decrypt[i]);
	}
	printf("\n");
	
	char temp1[32];
	Hex2Str(decrypt, temp1, 16);
	printf("加密后:");
	for(i = 0; i < 32; i++) {
		printf("%c", temp1[i]);
	}
	printf("\n");
	
	static char encrypt[48];
	unsigned short sign = htons(1);
	unsigned short size = htons(44);
	
	memcpy(encrypt, &sign, 2);
	memcpy(encrypt + 2, &size, 2);
	memcpy(encrypt + 4,temp1, 32);
	memcpy(encrypt + 36, str, 12);

	return encrypt;
}

int main(int argc, char *argv[])
{
	char decryt[48];
	
	char key[32] = "srlxmsnrcwrlqhbblbymlfphuplmryxn";
	char str[12] = "123456789012";
	memcpy(decryt, encode(key, str), 48);

	printf("2加密后:");
	int i;
	for(i = 0; i < 4; i++) {
		printf("%02x ", decryt[i]);
	}
	printf("\n");

	for(i = 0; i < 48; i++) {
		printf("%c", decryt[i]);
	}
	printf("\n");
/*
	int i,j;
	//unsigned char encrypt[] = "12345smhcxufn";//21232f297a57a5a743894a0e4a801fc3
	unsigned char decrypt[16];    

	char key[32] = "srlxmsnrcwrlqhbblbymlfphuplmryxn";
	char salt[8];

	char testArray[14];
	char *test;
	test = testArray;
	printf("test size is %d\n", sizeof(test));
	memset(salt, 0, sizeof(salt));
	GetSalt(salt, key, 0, 0);

	for(j = 7; j >= 0; j--) {
		printf("salt[%d]:%c\n", j, salt[j]);
	}
	unsigned char myencrypt[] = "123456789012";
	char temp[20];
	memcpy(temp, myencrypt, 12);
	memcpy(temp+12, salt, 8);

	MD5_CTX md5;
	MD5Init(&md5);         		
	MD5Update(&md5,temp,sizeof(temp));
	MD5Final(&md5,decrypt);        
	printf("加密前:%s\n加密后:",temp);
	for(i=0;i<16;i++)
	{
		printf("%02x",decrypt[i]);
	}
	printf("\n");
	
	char temp1[32];
	Hex2Str(decrypt, temp1, 16);
	printf("加密后:");
	for(i = 0; i < 32; i++) {
		printf("%c", temp1[i]);
	}
	printf("\n");
*/
	
	return 0;

}
