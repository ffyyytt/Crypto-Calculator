#define _CRT_SECURE_NO_WARNINGS
#include "md5.h"
#include "aes.h"
#include "crc.h"
#include "sha1.h"
#include "sha2.h"
#include "base64.h"
#include "md5collgen.h"
#include "LengthExtensionAttack.h"
#include <iostream>

int main()
{
    static char msg[] = "123456:myname=ffyytt&uid=1001&lstcmd=1";
    static char msg1[] = "&download=secret.txt";
    static char key[] = "ABCDEFGHIJKLMNOP";
    char newmsg[100];
    
    md5 md5_ctx = md5();
    uint8 md5sum[md5::DIGEST_SIZE];
    md5_ctx.init();
    md5_ctx.update((uint8*)msg, strlen(msg));
    md5_ctx.final(md5sum);
    std::cout << "MD5: " << Hash::bytesToHex(md5sum, md5::DIGEST_SIZE) << std::endl;

    sha1 sha1_ctx;
    uint8 sha1sum[sha1::DIGEST_SIZE];
    sha1_ctx.init();
    sha1_ctx.update((uint8*)msg, strlen(msg));
    sha1_ctx.final(sha1sum);
    std::cout << "SHA1: " << Hash::bytesToHex(sha1sum, sha1::DIGEST_SIZE) << std::endl;

    sha256 h, hcopy; //sha512 h;
    uint8 hsum[h.DIGEST_SIZE];
    h.init();
    h.update((uint8*)msg, strlen(msg));
    h.final(hsum);
    std::cout << Hash::bytesToHex(hsum, h.getDigestSize()) << std::endl;
    //c7ac037d5e6cd43ab7ef47b7fb48f1e7e735f5b7f668178fb8b74c127ea0949f

    // 123456 is password (unknown to the attacker)
    // myname=ffyytt&uid=1001&lstcmd=1&mac=c7ac037d5e6cd43ab7ef47b7fb48f1e7e735f5b7f668178fb8b74c127ea0949f is query 
    // where myname=ffyytt&uid=1001&lstcmd=1 is realquery from user
    // mac = sha256(key:realquery) (server provided to user) = c7ac037d5e6cd43ab7ef47b7fb48f1e7e735f5b7f668178fb8b74c127ea0949f
    // myname=ffyytt&uid=1001&lstcmd=1%80%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%00%01%30&download=secret.txt&mac=085b64794c6f2746a9660f1c5e9457699c38e5d78c43ee71d0d74eebe318e1c4 is new_query
    // where download=secret.txt is in query
    // new_query = realquery + pad + extra_message
    // mac = sha256(new_query) = hashLengthExtensionAttack (without key)

    uint8 outputLEA[h.DIGEST_SIZE];
    LengthExtensionAttack::hashLengthExtensionAttack(hcopy, hsum, (unsigned char*)msg1, strlen(msg1), outputLEA);
    std::cout << Hash::bytesToHex(outputLEA, h.getDigestSize()) << std::endl;

    // Check
    strcpy(newmsg, msg);
    uint64 len = LengthExtensionAttack::padding((uint8*)newmsg, strlen(msg), h.getBlockSize());
    strcpy(&newmsg[len], msg1);
    len += strlen(msg1);
    h.init();
    h.update((uint8*)newmsg, len);
    //h.update((uint8*)msg1, strlen(msg1));

    h.final(hsum);
    std::cout << Hash::bytesToHex(hsum, h.DIGEST_SIZE) << std::endl;

    AES aes(AESKeyLength::AES_128);
    uint8* msgpad = AES::pkcs7_padding((uint8*)msg, strlen(msg));
    uint8* aesoutput = aes.EncryptECB((uint8*)msgpad, AES::pkcs7_padding_length(strlen(msg)), (uint8*)key);
    std::cout << base64().base64_encode(aesoutput, AES::pkcs7_padding_length(strlen(msg))) << std::endl;
    std::cout << base64().base64_encode((uint8*)msg, strlen(msg)) << std::endl;

    
    AES squareAES(AESKeyLength::SQUARE);
    squareAES.generatePlainTextForSquareAttack("plaintexts.txt");
    squareAES.EncryptListInFile("plaintexts.txt", "ciphertexts.txt", 16 * 256, (uint8*)key);
    unsigned char squarekey[] = "AAAAAAAAAAAAAAAA";
    AES::squareAttack("ciphertexts.txt", squarekey);
    std::cout << squarekey << std::endl;

    std::string prefixfn("prefix.txt");
    std::string outfn1("out1.bin");
    std::string outfn2("out2.bin");
    std::string defaultIV("0123456789abcdeffedcba9876543210");
    md5collgen(prefixfn, outfn1, outfn2, defaultIV);

    std::cout << crc::crc32buf(msg, strlen(msg));

    return 0;
}