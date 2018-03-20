//
// Created by fanz on 10/20/16.
//

#ifndef POUW_BLOCKCHAIN_H
#define POUW_BLOCKCHAIN_H

#define BLOCK_HASH_LEN 32


typedef struct _pouw_voucher {
    uint8_t header_hash[32];
    double difficulty;
    uint8_t is_win;
} pouw_voucher;

typedef struct _difficulty {
    double difficulty;
} difficulty_t;

typedef struct _block_hash{
    uint8_t h[BLOCK_HASH_LEN];
} block_hash_t;


#endif //POUW_BLOCKCHAIN_H
