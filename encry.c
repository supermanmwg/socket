#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "md5.h"

char *join1(char *a, char *b) {  
	printf("a length is %d, b length is %d\n", strlen(a), strlen(b));
    char *c = (char *) malloc(strlen(a) + 8 + 1); //局部变量，用malloc申请内存  
    if (c == NULL) exit (1);  
    char *tempc = c; //把首地址存下来  
    while (*a != '\0') {  
        *c++ = *a++;  
    }  
    while ((*c++ = *b++) != '\0') {  
        ;  
    }  
    //注意，此时指针c已经指向拼接之后的字符串的结尾'\0' !  
    return tempc;//返回值是局部malloc申请的指针变量，需在函数调用结束后free之  
}  

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

char* encode(char* key, char* encry) {
	char salt[8];
	unsigned char decrypt[16];    

	memset(salt, 0, sizeof(salt));
	GetSalt(salt, key, 0, 0);

	char *temp = join1(encry, salt);
	printf("before encry %s\n", temp);
	MD5_CTX md5;
	MD5Init(&md5);         		
	MD5Update(&md5,temp,17);
	MD5Final(&md5,decrypt);        
	free(temp);
	char *temp1 = (char *)malloc(sizeof(char) * 32);
	Hex2Str(decrypt, temp1, 16);

	return temp1;
}


int main(int argc, char *argv[])
{
/*
	char key[32] = "srlxmsnrcwrlqhbblbymlfphuplmryxn";
	unsigned char myencrypt[] = "12345";

	char* afterEncrypt = encode(key, myencrypt);
	printf("加密后:");
	int i;
	for(i = 0; i < 32; i++) {
		printf("%c", afterEncrypt[i]);
	}
	printf("\n");
*/
	
	int i,j;
	unsigned char encrypt[] = "12345smhcxufn";//21232f297a57a5a743894a0e4a801fc3
	unsigned char decrypt[16];    

	char key[32] = "srlxmsnrcwrlqhbblbymlfphuplmryxn";
	char salt[8];

	memset(salt, 0, sizeof(salt));
	GetSalt(salt, key, 0, 0);

	for(j = 7; j >= 0; j--) {
		printf("salt[%d]:%c\n", j, salt[j]);
	}
	unsigned char myencrypt[] = "12345";
	char *temp = join1(myencrypt, salt);

	MD5_CTX md5;
	MD5Init(&md5);         		
	MD5Update(&md5,temp,strlen((char *)temp));
	MD5Final(&md5,decrypt);        
	printf("加密前:%s\n加密后:",temp);
	for(i=0;i<16;i++)
	{
		printf("%02x",decrypt[i]);
	}
	printf("\n");
	
	char *temp1 = (char *)malloc(sizeof(char) * 32);
	Hex2Str(decrypt, temp1, 16);
	printf("加密后:");
	for(i = 0; i < 32; i++) {
		printf("%c", temp1[i]);
	}
	printf("\n");
	
	return 0;

}
