#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/sha.h>

#include "exceptions.h"
#include "errors.h"


#define HASH_SIZE SHA256_DIGEST_LENGTH
#define ELEM_TYPE char


typedef struct Stack {
    void* buffer;
    size_t elem_size;
    size_t capacity;
    size_t count;
    unsigned char hash[HASH_SIZE];
    void* left_canary;
    void* right_canary;
} Stack;

//-------------------------------------Stack_health_check-------------------------------------

void stack_health_check(Stack* stack){
    if(stack->buffer == NULL)
        THROW(ERR_STACK_BUFF_IS_NULL, "Stack structure damaged, NULL is invalid value for pointer on stack buffer.");

    unsigned char* curren_hash = SHA256((const unsigned char*)stack->buffer + 8, stack->capacity - 8, stack->hash);
    if(stack->hash != curren_hash)
        THROW(ERR_STACK_HASH, "Wrong hash. Stack damaged or unauthorized modification happens.");

    if((char*)stack->left_canary != (char*)stack->buffer || (char*)stack->right_canary != (char*)stack->buffer + stack->capacity)
        THROW(ERR_STACK_CANARY_CORRUPTION, "Stack overflowed or unauthorized copying happens.");

    return;
}

//-------------------------------------HMMMMMM....-------------------------------------
static void stack_poison(Stack* stack) {
    // Calculate unused space after elements (excluding header and footer)
    size_t data_end = 8 + stack->elem_size * stack->count;
    size_t poison_size = stack->capacity - data_end - 8;
    memset((char*)stack->buffer + data_end, 0x00, poison_size);
}

static void stack_hash(Stack* stack) {
    SHA256((const unsigned char*)stack->buffer + 8, stack->capacity - 8, stack->hash);
}

static void stack_dump(Stack* stack) {
    printf("\nSTACK DUMP: \n");
    printf("\nBuffer size: %zu\nElement size: %zu\nNumber of elements: %zu\n", 
           stack->capacity, stack->elem_size, stack->count);

    printf("Stack buffer: \n");
    for(size_t i = 0; i < stack->capacity; i++) {
        printf("%02x", ((unsigned char*)stack->buffer)[i]);
    }
    printf("\nHash: ");
    for(size_t i = 0; i < HASH_SIZE; i++) {
        printf("%02x", stack->hash[i]);
    }
    printf("\nLeft canary: %p\n", stack->left_canary);
    printf("Right canary: %p\n", stack->right_canary);
    ptrdiff_t distance = (char*)stack->right_canary - (char*)stack->left_canary;
    printf("Distance: %zd\n", distance);  
}

static void stack_canary_set(Stack* stack){
    *(void**)stack->buffer = stack->buffer;
    stack->left_canary = *(void**)stack->buffer;

    void* end_of_buffer = (char*)stack->buffer + stack->capacity;
    *(void**)((char*)stack->buffer + stack->capacity - 8) = end_of_buffer;
    stack->right_canary = end_of_buffer;
}

static void stack_realloc(Stack* stack) {
    size_t new_capacity = stack->capacity * 1.5;
    stack->buffer = realloc(stack->buffer, new_capacity);
    stack->capacity = new_capacity;

    stack_canary_set(stack);

}
//-------------------------------------Stack-------------------------------------

Stack* stack_init(size_t el_num, size_t el_size) {
    // Check initial parameters
    if (el_num == 0 || el_size == 0){
        THROW(1, "Invalid stack parameters");
        return NULL;
    }

    Stack* stack = (Stack*)calloc(1, sizeof(Stack));  // FIX 1: Allocate on heap
    // Stack allocation failure
    if (!stack){
        THROW(2, "Failed to allocate Stack buffer");
        return NULL;
    }

    stack->elem_size = el_size;
    stack->capacity = el_num * el_size + 16;
    stack->count = 0;

    stack->buffer = calloc(1, stack->capacity);
    // Stack buffer allocation failure
    if (!stack->buffer){
        free(stack);
        THROW(3, "Failed to allocate Stack buffer");
        return NULL;
    }

    stack_canary_set(stack);

    stack_poison(stack);
    stack_hash(stack);
    return stack;
}

void stack_free(Stack* stack) {
    stack_health_check(stack);
    free(stack->buffer);
    free(stack);
}

Stack* stack_copy(Stack* stack){
    stack_health_check(stack);
    //TODO: Make stack_init doesn't hash stack before buffer will be copied.
    Stack* new_stack = stack_init((stack->capacity - 16) / stack->elem_size, stack->elem_size);
    memcpy((char*)new_stack->buffer + 8,(char*)stack->buffer + 8, stack->capacity - 16);

    stack_canary_set(new_stack);
    stack_hash(new_stack);

    return new_stack;
}

void stack_pop(Stack* stack){
    stack_health_check(stack);

    if(stack->count == 0)
        THROW(ERR_POP_EMPTY_STACK, "Nothing to pop on empty stack.");

    stack->count --;
    stack_poison(stack);
    stack_hash(stack);
}

void stack_push(Stack* stack, void* elem){
    stack_health_check(stack);
    if(stack->capacity - stack->count * stack->elem_size - 16 <= stack->elem_size)
        stack_realloc(stack);

    char* dest_addr = (char*)stack->buffer + stack->count * stack->elem_size + 8;
    memcpy((void*)dest_addr, elem, stack->elem_size);
    stack->count ++;

    stack_hash(stack);
}

void* stack_get_element(Stack* stack, size_t position){
    if(position > stack->count || position < 0)
        THROW(ERR_ELEM_INDEX_OUT_OF_RANGE, "Element index out of stack range.");

    void* elem_ptr = (char*)stack->buffer + position * stack->elem_size;
    return elem_ptr;
}
