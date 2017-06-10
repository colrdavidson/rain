#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8;
typedef float f32;
typedef double f64;

typedef u32 bool;
#define true 1
#define false 0
#define BOOL_FMT(x) x ? "true" : "false"

#ifdef DEBUG
#define debug(fmt, ...) printf(fmt, __VA_ARGS__);
#else
#define debug(fmt, ...)
#endif

#define get_lock(lock) debug("%s:%s:%d aquiring lock: %s\n", __FILE__, __func__, __LINE__, #lock); pthread_mutex_lock((lock)); debug("%s:%s:%d aquired lock: %s\n", __FILE__, __func__, __LINE__, #lock);
#define release_lock(lock) debug("%s:%s:%d releasing lock: %s\n", __FILE__, __func__, __LINE__, #lock); pthread_mutex_unlock((lock)); debug("%s:%s:%d released lock: %s\n", __FILE__, __func__, __LINE__, #lock);
#define wait_for_lock(cond, lock) debug("%s:%s:%d waiting for signal: %s, %s\n", __FILE__, __func__, __LINE__, #cond, #lock); pthread_cond_wait((cond), (lock)); debug("%s:%s:%d got signal: %s, %s\n", __FILE__, __func__, __LINE__, #cond, #lock);
#define signal_to_locks(cond) debug("%s:%s:%d signaling: %s\n", __FILE__, __func__, __LINE__, #cond); pthread_cond_signal((cond)); debug("%s:%s:%d sent signal: %s\n", __FILE__, __func__, __LINE__, #cond);
#define broadcast_to_locks(cond) debug("%s:%s:%d signaling: %s\n", __FILE__, __func__, __LINE__, #cond); pthread_cond_broadcast((cond)); debug("%s:%s:%d broadcasted signal: %s\n", __FILE__, __func__, __LINE__, #cond);

#endif
