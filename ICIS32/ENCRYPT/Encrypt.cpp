#include "encrypt.h"
#include "md5.h"


LONG encryptPWD(LPCSTR szPwd, LPCSTR szEncrypt)
{
	char test[10];

    int status = 1;

	strcpy(test,szPwd);
	md5_state_t state;
	md5_byte_t digest[16];
    char hex_output[5*2 + 1];
//	char hex_output[16*2 + 1];
	int di,i;
    ZeroMemory(digest, sizeof(digest));

	md5_init(&state);
	md5_append(&state, (const md5_byte_t *)szPwd, strlen(szPwd));
	md5_finish(&state, digest);
	i = 0;
	for (di = 0; di < 16; ++di) {
//      sprintf(hex_output + di * 2, "%02x", digest[di]);
		if ((di % 4)==0) {
           sprintf(hex_output + i * 2, "%02x", digest[di]);
           i++;
		}
	}
    sprintf(hex_output + i * 2, "%02x", digest[15]);

    strcpy((char *)szEncrypt,(char *)hex_output);
    
    return status;

}
