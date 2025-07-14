#include <stdlib.h>
#include <stdlib.h>
#include "../stack.h"
#include "../exceptions.h"
#include "../errors.h"

#define NUM_OF_OPERATIONS 2

int main(){
int count = 0;    

    Stack* stack = stack_init(100, 1);
    for (int i = 0; i < 10000; i++){
        int operation = rand() % NUM_OF_OPERATIONS;
        if (operation == 1){
            TRY{
                stack_push(stack, (void*)(const char[]){"A"});
                count ++;
                }
            CATCH(ERR_INVALID_PARAM){
                printf("Invalid parameters while stack initialization");
            }
            CATCH(ERR_STACK_ALLOC){
                printf("Trouble with stack structure allocation");
            }
            CATCH(ERR_STACK_BUFF_ALLOC){
                printf("Trouble with stack buff allocation");
            }
            CATCH_ALL{
                perror("Stack damaged while pushing new element.");
                abort();
                }
            END_TRY;
}

        if (operation == 2){
            TRY{
                stack_pop(stack);
                count --;
            }
            CATCH(ERR_POP_EMPTY_STACK){
                continue;
                }
            CATCH_ALL{
                perror("Stack damaged while poping element.");
                printf("Iteration: %d", i);
                stack_dump(stack);
                abort();
                }
            END_TRY;
        }
    }
    stack_dump(stack);
    stack_free(stack);

    printf("Counted number of elements: %d\n", count);

    return 0;
}
