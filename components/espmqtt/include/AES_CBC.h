/** @file AES_CBC.h
 *  \brief AES block cipher, ESP32 hardware accelerated version Based on mbedTLS FIPS-197 compliant version.
 *	CBC has an IV and thus needs randomness every time a message is encrypted,
 *  changing a part of the message requires re-encrypting everything after the change,
 *  transmission errors in one ciphertext block completely destroy the plaintext and change
 *  the decryption of the next block, decryption can be parallelized / encryption
 *  can't, the plaintext is malleable to a certain degree.
 *
 *  @author Duraisamy Pandurangan (V-Guard Industrial Ltd,R&D)
 */

#ifndef AES_CBC_H_
#define AES_CBC_H_

#include "mbedtls/aes.h"

/**
 * @brief Get Random key 16 byte value. Random value assigned in arguments.
 */
void GetRandomKey(char *PlainText)
{
	char alphabet[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g',
			'h', 'i', 'j', 'k', 'l', 'm', 'n',
			'o', 'p', 'q', 'r', 's', 't', 'u',
			'v', 'w', 'x', 'y', 'z' };

	for (int i = 0; i < 16; i++){
		PlainText[i] = alphabet[esp_random() % 25];
	}
	return;//  (char *)PlainText;;
}

/**
 * Remove trailing white Ascii  characters value  from less than 32
 */
int  trimTrailing(char * str)
{
    int index, i ;

    /* Set default index */
    index = -1;

    /* Find last index of non-white space character */
    i = 0;
    while(str[i] != '\0')
    {
        if(str[index] > 31) //Ascii value compare
        {
            index= i;
        }

        i++;
    }

    /* Mark next character to last non-white space character as NULL */
    str[index + 1] = '\0';
    return index;


}
/**
 * @brief Decrypt the 128 bit CBC mode,
 * @param The PlainText to be Decrypt.
 * @param length Decode message length
 * @param key ,decrypt key length 128(16 byte)Bits.
 * @param IV decrypt initialize vector 128(16byte)Bits.
 *
 * @return Plain Text message
 */
char *Decrypt_CBC128( char *PlainText,int length,unsigned char key[],unsigned char iv[]){
	/*create the Aes context*/
	mbedtls_aes_context aes;

	/*init the Aes context*/
	mbedtls_aes_init( &aes );

	/*create Local IV variable*/
	unsigned char iv2[16] ="1234567890123456" ;

	strcpy((char *)iv2,(const char *)iv);
	/*set 128bits key into AES lib*/
	mbedtls_aes_setkey_dec( &aes, key, 16*8 ); //128 Bit Encryption Key

	unsigned char Decrypt[length];
	/*Decrypt the Message*/
	mbedtls_aes_crypt_cbc( &aes, MBEDTLS_AES_DECRYPT,length, iv2,(const unsigned char *)PlainText, (unsigned char *)Decrypt );
	Decrypt[length] = '\0';
	strcpy(PlainText,(const char*)Decrypt);
	/*Deinit The AES context*/
	mbedtls_aes_free( &aes );

	/*Return the Plain Text*/
	return (char *)PlainText;

}

/**
 * @brief Encrypt_CBC128 the 128 bit CBC mode,
 * @param The PlainText to be encrypt. The encrypted value push back to the plain text buffer
 * @param key ,encrypt key length 128(16 byte)Bits.
 * @param IV encrypt initialize vector 128(16byte)Bits.
 *
 * @return Decrypt data length
 */
int Encrypt_CBC128(const unsigned char *PlainText,unsigned char key[],unsigned char iv[]){

	int length = strlen((const char*)PlainText);
	int DataValid_length = 16;
	/*Encrypt message should not less than zero*/
	if(length<=0){
		return -1;// (char*)"Unable to process data";
	}else{
		/*create the Aes context*/
		mbedtls_aes_context aes;

		/*Init the Aes context*/
		mbedtls_aes_init( &aes );
		unsigned char iv2[16] ="1234567890123456" ;//{0xb2, 0x4b, 0xf2, 0xf7, 0x7a, 0xc5, 0xec, 0x0c, 0x5e, 0x1f, 0x4d, 0xc1, 0xae, 0x46, 0x5e, 0x75};

		strcpy((char *)iv2,(const char *)iv);

		/*set 128bits key into AES lib*/
		mbedtls_aes_setkey_enc( &aes, key, 16*8 ); //128 Bit Encryption Key

		/*Finding append length*/
		int space = length%16;

		if(space == 0){
			DataValid_length = 0;
		}

		int datasize  = length+(DataValid_length - space);
		const unsigned char *Spacedata= (const unsigned char *)malloc(datasize+1);
		//= PlainText;
		strcpy((char *)Spacedata,(const char*)PlainText);
		memset((char *)PlainText,0,sizeof(PlainText));

		if(space !=0)
			while(space <=15){
				strcat((char *)Spacedata," ");//Data padding Characters
				space++;
			}
		/*Encrypt the Message*/
		mbedtls_aes_crypt_cbc( &aes, MBEDTLS_AES_ENCRYPT,datasize, iv2,(const unsigned char*)Spacedata, (unsigned char *)PlainText);

		/*Deinit The AES context*/
		mbedtls_aes_free( &aes );

		free(Spacedata);
		/*Return the encrypted message length*/
		return (datasize);
	}
	return 0;
}


#endif
