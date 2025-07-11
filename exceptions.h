// exceptions.h
#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdlib.h>
#include <setjmp.h>
#include <stdio.h>

typedef struct {
    int code;
    const char* message;
} Exception;

typedef struct ExceptionContext {
    jmp_buf env;
    Exception exception;
    struct ExceptionContext* prev;
} ExceptionContext;

static __thread ExceptionContext* exception_stack = NULL;

#define TRY do { \
    ExceptionContext _ctx; \
    _ctx.prev = exception_stack; \
    exception_stack = &_ctx; \
    int _ex = setjmp(_ctx.env); \
    if (_ex == 0)

#define CATCH(code) \
    else if (_ex == (code))

#define CATCH_IF(code) \
    else if (code)

#define CATCH_ALL \
    else

#define THROW(error_code, msg) do { \
    if (exception_stack) { \
        exception_stack->exception.code = (error_code); \
        exception_stack->exception.message = (msg); \
        longjmp(exception_stack->env, (error_code)); \
    } else { \
        fprintf(stderr, "Unhandled exception: %d - %s\n", (error_code), (msg)); \
        abort(); \
    } \
} while (0)

#define END_TRY \
    exception_stack = _ctx.prev; \
    } while (0)

#endif // EXCEPTIONS_H
