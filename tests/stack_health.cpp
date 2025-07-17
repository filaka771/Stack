#include <stdlib.h>
#include <time.h>
#include "../stack.h"
#include "../exceptions.h"
#include "../errors.h"

void stack_health_check_test(){
    srand(time(NULL) ^ getpt());
    
    Stack* stack_null_buf = stack_init(100, 1);

    // Null ptr on stack buffer error.
    void* real_addr = stack_null_buf->buffer;
    stack_null_buf->buffer = NULL;
    TRY{stack_health_check(stack_null_buf);
    }
    CATCH(ERR_STACK_BUFF_IS_NULL){
        printf("NULL ptr on stack buffer successfully handled!\n");
    }
    CATCH_ALL{
        printf("NULL ptr on stack buffer unhandled!\n");
    }
    END_TRY;
    free(real_addr);
    free(stack_null_buf);

    //Verifying hash --------------------------------------------------
/*
    Stack* stack_hash_test = stack_init(100,1);
    void* test_buffer = calloc(100, 1);

    memset(test_buffer, 0x00,100);
    unsigned char test_buffer_hash [HASH_SIZE];
    SHA256((const unsigned char*)test_buffer, 100, test_buffer_hash);

    stack_dump(stack_hash_test);
    for(int i = 0; i < HASH_SIZE; i++){
       printf("%02x", test_buffer_hash[i]);
    }
    free(test_buffer);
    free(stack_hash_test->buffer);
    free(stack_hash_test);
*/
    // Random bytes changing ------------------------------------------
    Stack* stack_damaged_buf = stack_init(100, 1);
    int random_el = rand() % (stack_damaged_buf->capacity - 16);
    int range     = rand() % (stack_damaged_buf->capacity - random_el - 16) + 1 ;

    memset((char*)stack_damaged_buf->buffer + random_el, 'A', range);

    TRY{stack_health_check(stack_damaged_buf);
    }
    CATCH(ERR_STACK_HASH){
        printf("Random stack buffer changes handled successfully!\n");
    }
    CATCH_ALL{
        printf("Random stack buffer changes unhandled!\n");
    }
    END_TRY;
    free(stack_damaged_buf->buffer);
    free(stack_damaged_buf);

    //Left canary corruption
    Stack* stack_l_canary_corrupt = stack_init(100, 1);
    for(int i = 0; i < 7; i++){
        *((char*)stack_l_canary_corrupt->buffer + i) =
            *((char*)stack_l_canary_corrupt->buffer + i) ^ (char)(rand() % 256);
    }

    TRY{stack_health_check(stack_l_canary_corrupt);
    }
    CATCH(ERR_STACK_LEFT_CANARY_CORRUPTION){
        printf("Random changes of left canary successfully handled!\n");
    }
    CATCH_ALL{
        printf("Random changes of left canary unhandled!\n");
    }
    END_TRY;
    free(stack_l_canary_corrupt->buffer);
    free(stack_l_canary_corrupt);

    //Right canary corruption
    Stack* stack_r_canary_corrupt = stack_init(2, 1);

    for(int i = 1; i < 9; i++){
        *((char*)stack_r_canary_corrupt->buffer + stack_r_canary_corrupt->capacity - i) =
            *((char*)stack_r_canary_corrupt->buffer + stack_r_canary_corrupt->capacity - i) ^ (char)(rand() % 256);
    }

    TRY{stack_health_check(stack_r_canary_corrupt);
    }
    CATCH(ERR_STACK_HASH){
        printf("Hash damaged");
    }
    CATCH(ERR_STACK_RIGHT_CANARY_CORRUPTION){
        printf("Random changes of right canary successfully handled!\n");
    }
    CATCH_ALL{
        printf("Random changes of right canary unhandled!\n");
    }
    END_TRY;
    free(stack_r_canary_corrupt->buffer);
    free(stack_r_canary_corrupt);
}

int main(){
    stack_health_check_test();
return 0;
}
