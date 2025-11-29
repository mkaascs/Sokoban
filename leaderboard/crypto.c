#include "crypto.h"

#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/sha.h>

static const unsigned char aes_private_key[] = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
};

void hash_password(const char *password, char *hashed) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char *)password, strlen(password), hash);
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        snprintf(hashed + i * 2, 3, "%02x", hash[i]);

    hashed[SHA256_DIGEST_LENGTH * 2] = '\0';
}

int encrypt_data(const unsigned char *plaintext, const int plaintext_len, unsigned char *ciphertext) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return -1;

    const unsigned char iv[16] = {0};
    int len;

    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, aes_private_key, iv)) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len)) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    int ciphertext_len = len;

    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);
    return ciphertext_len;
}

int decrypt_data(const unsigned char *ciphertext, const int ciphertext_len, unsigned char *plaintext) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return -1;

    const unsigned char iv[16] = {0};
    int len;

    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, aes_private_key, iv)) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    if (1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len)) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    int plaintext_len = len;

    if (1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len)) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);
    return plaintext_len;
}