#ifndef CRYPTO_H
#define CRYPTO_H

void hash_password(const char *password, char *hashed);

int encrypt_data(const unsigned char *plaintext, int plaintext_len, unsigned char *ciphertext);
int decrypt_data(const unsigned char *ciphertext, int ciphertext_len, unsigned char *plaintext);

#endif //CRYPTO_H
