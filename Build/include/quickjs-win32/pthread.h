#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef DWORD pthread_t;
typedef int pthread_attr_t;
#define PTHREAD_CREATE_DETACHED 1
#define pthread_attr_init(a) (*a) = 0
#define pthread_attr_setdetachstate(a, b) (*a) = b
#define pthread_attr_destroy(...)
struct pthread_start_routine_parameter { void* (*__start_routine)(void*); void* __parameter; };
static DWORD WINAPI pthread_start_routine(LPVOID lpThreadParameter)
{
    struct pthread_start_routine_parameter* routine = (struct pthread_start_routine_parameter*)lpThreadParameter;
    void* (*__start_routine)(void*) = routine->__start_routine;
    void* __parameter = routine->__parameter;
    free(routine);
    __start_routine(__parameter);
    return 0;
}
static int pthread_create(pthread_t* __pthread_ptr, pthread_attr_t const* __attr, void* (*__start_routine)(void*), void* __parameter)
{
    struct pthread_start_routine_parameter* routine = malloc(sizeof(struct pthread_start_routine_parameter));
    if (routine == NULL) {
        return -1;
    }
    routine->__start_routine = __start_routine;
    routine->__parameter = __parameter;
    HANDLE handle = CreateThread(NULL, 0, pthread_start_routine, routine, 0, __pthread_ptr);
    if (handle == NULL || handle == INVALID_HANDLE_VALUE) {
        free(routine);
        return -1;
    }
    if (__attr && ((*__attr) & PTHREAD_CREATE_DETACHED)) {
        CloseHandle(handle);
    }
    return 0;
}

typedef CONDITION_VARIABLE pthread_cond_t;
#define pthread_cond_init(a, b) InitializeConditionVariable(a)
#define pthread_cond_destroy(a)
#define pthread_cond_wait(a, b) SleepConditionVariableCS(a, b, INFINITE)
#define pthread_cond_timedwait(a, b, c) SleepConditionVariableCS(a, b, (c)->tv_sec * 1000 + (c)->tv_nsec / 1000000)
#define pthread_cond_signal WakeConditionVariable

typedef CRITICAL_SECTION pthread_mutex_t;
#define PTHREAD_MUTEX_INITIALIZER {(PCRITICAL_SECTION_DEBUG)-1, -1, 0, 0, 0, 0}
#define pthread_mutex_init(a, b) InitializeCriticalSection(a)
#define pthread_mutex_destroy DeleteCriticalSection
#define pthread_mutex_lock EnterCriticalSection
#define pthread_mutex_unlock LeaveCriticalSection
