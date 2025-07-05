#ifndef STACK_H
#define STACK_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>

#define HASH_SIZE SHA256_DIGEST_LENGTH

typedef struct Stack {
    void* buffer;
    size_t elem_size;
    size_t capacity;
    size_t count;
    unsigned char hash[HASH_SIZE];
} Stack;

// --- Core Functions ---
Stack* stack_init(size_t el_num, size_t el_size);
void stack_free(Stack* stack);
Stack* stack_copy(Stack* stack);

// --- Stack Operations ---
void stack_push(Stack* stack, void* elem);
void stack_pop(Stack* stack);

// --- Debug/Utility Functions ---
void stack_dump(Stack* stack);  // For debugging (optional)

#endif // STACK_H
