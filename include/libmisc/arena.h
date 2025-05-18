/*
 * April 2025, [https://github.com/Yuuki1578/misc.git]
 * This is a part of the libmisc library.
 * Any damage caused by this software is not my responsibility at all.

 * @file arena.h
 * @brief linear allocator with segmented region (arena)
 *
 * */

/*
 * I don't know if bits/size.h exist on all UNIX like system so yeah
 * This device is using __ANDROID_API__ 24
 *
 * */
#ifdef __ANDROID__
#include <bits/page_size.h>
#else

/*
 * constexpr (constant expression): determined at compile time
 * present as read-only data on binary file (maybe platform dependent)
 * 
 * auto: different from auto for storage class specifier, this is more like
 * in C++, where the type is expanded at compile time through the right hand side expression
 *
 * */
constexpr auto PAGE_SIZE = 4096;
#endif

#include <stddef.h>

/*
 * Default size for arena, work for non-POSIX system
 * 
 * */
constexpr auto ARENA_ALLOC_STEP = PAGE_SIZE;

/* 
 * Magic values
 *
 * */
constexpr auto ARENA_READY = 0;
constexpr auto ARENA_NOAVAIL = -1;
constexpr auto ARENA_BUSY = -2;

/*
 * Arena data types
 * The arena is responsible for managing its inner buffer
 * 
 * Task like segmenting pointer, allocating it to another, growing its size
 * things like that
 *
 * You can initialize the arena once, use it everywhere, and have to free it
 * only once
 *
 * */
typedef struct {
    void    *rawptr;   // pointer to allocated memory
    size_t  capacity;  // total memory that arena can hold
    size_t  offset;    // an offset from the left of the pointer
    size_t  step;      // how much bytes per allocation
} Arena;

/*
 * Create a new Arena, can be allocated early if should_allocated is true
 *
 * */
extern int arena_new(Arena *arena, size_t step, bool should_allocate);

/*
 * Create a new arena from rawptr[count]
 *
 * */
extern int arena_from(Arena *arena, void *rawptr, size_t count);

/*
 * Return an allocated chunk of memory from arena to the caller
 *
 * */
extern void *arena_alloc(Arena *arena, size_t size);

/* 
 * Change the size of the allocated memory
 * 
 * */
void *arena_realloc(Arena   *arena,
                    void    *dst,
                    size_t  old_size,
                    size_t  new_size);

/*
 * Freeing the memory hold by arena
 *
 * */
extern void arena_dealloc(Arena *arena);

/*
 * Return the arena capacity (in bytes)
 *
 * */
extern size_t arena_capacity(Arena *arena);

/*
 * Return the remaining arena capacity
 *
 * */
extern size_t arena_remaining(Arena *arena);

/*
 * Return offset from the left of arena
 *
 * */
extern size_t arena_offset(Arena *arena);

/*
 * Return the first memory address from arena
 *
 * */
extern void *arena_first_addr(Arena *arena);

/*
 * Return the last memory address used from arena
 * 
 * */
extern void *arena_last_addr(Arena *arena);

/*
 * Return the last memory address from arena
 * 
 * */
extern void *arena_brk_addr(Arena *arena);

/*
 * Check whether the arena offset is equal to arena capacity - 1
 *
 * */
extern bool arena_on_limit(Arena *arena);

/*
 * Return the inner buffer as new allocated pointer and freeing the arena.
 *
 * */
extern void *arena_popout(Arena *arena);
