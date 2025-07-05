#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/sha.h>


#define HASH_SIZE SHA256_DIGEST_LENGTH
#define ELEM_TYPE char

typedef struct Stack {
    void* buffer;
    size_t elem_size;
    size_t capacity;
    size_t count;
    unsigned char hash[HASH_SIZE];
} Stack;

static void stack_poison(Stack* stack) {
    // Calculate unused space after elements (excluding header and footer)
    size_t data_end = 8 + stack->elem_size * stack->count;
    size_t poison_size = stack->capacity - data_end - 8;
    memset((char*)stack->buffer + data_end, 0x00, poison_size);
}

static void stack_hash(Stack* stack) {
    SHA256((const unsigned char*)stack->buffer, stack->capacity, stack->hash);
}

static void stack_dump(Stack* stack) {
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
    printf("\n");
}

static void stack_realloc(Stack* stack) {
    size_t new_capacity = stack->capacity * 1.5;
    stack->buffer = realloc(stack->buffer, new_capacity);
    
    // Update end canary
    void* end_of_buffer = (char*)stack->buffer + new_capacity;
    *(void**)((char*)stack->buffer + new_capacity - 8) = end_of_buffer;
    
    stack->capacity = new_capacity;
}

static void stack_canary_set(Stack* stack){
    *(void**)stack->buffer = stack->buffer;

    void* end_of_buffer = (char*)stack->buffer + stack->capacity;
    *(void**)((char*)stack->buffer + stack->capacity - 8) = end_of_buffer;
}

Stack* stack_init(size_t el_num, size_t el_size) {
    Stack* stack = (Stack*)calloc(1, sizeof(Stack));  // FIX 1: Allocate on heap
    stack->elem_size = el_size;
    stack->capacity = el_num * el_size + 16;
    stack->count = 0;
    stack->buffer = calloc(1, stack->capacity);

    stack_canary_set(stack);

    stack_poison(stack);
    stack_hash(stack);
    return stack;
}


void stack_free(Stack* stack) {
    free(stack->buffer);
    free(stack);
}

Stack* stack_copy(Stack* stack){
    //TODO: Make stack_init doesn't hash stack before buffer will be copied.
    Stack* new_stack = stack_init((stack->capacity - 16) / stack->elem_size, stack->elem_size);
    memcpy((char*)new_stack->buffer + 8,(char*)stack->buffer + 8, stack->capacity - 16);

    stack_canary_set(new_stack);
    stack_hash(new_stack);

    return new_stack;
}

void stack_pop(Stack* stack){
    assert(stack->count > 0 && "Nothing to pop on empty stack");
    stack->count --;
    stack_poison(stack);
    stack_hash(stack);
}

void stack_push(Stack* stack, void* elem) {
    if(stack->capacity - stack->count * stack->elem_size - 16 <= stack->elem_size)
        stack_realloc(stack);

    char* dest_addr = (char*)stack->buffer + stack->count * stack->elem_size + 8;
    memcpy((void*)dest_addr, elem, stack->elem_size);
    stack->count ++;

    stack_hash(stack);
}

int main() {
    Stack* stack = stack_init(10, 1);
    stack_dump(stack);

    char A = 'A';
    stack_push(stack, &A);
    stack_dump(stack);

    Stack* new_stack = stack_copy(stack);
    stack_dump(new_stack);

    stack_free(stack);
    stack_free(new_stack);
    return 0;
}
