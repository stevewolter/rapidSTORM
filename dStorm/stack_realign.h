#ifndef DSTORM_REALIGN_STACK
#if __x86_64 == 1 && __linux == 1
#define DSTORM_REALIGN_STACK
#else
/** The REALIGN_STACK macro is necessary on Windows platforms and under valgrind's helgrind, both of which have a 4 byte stack alignment. 
 *  GCC will only emit code that is 16 byte aligned and assumes such alignment, thus needs to do a 
 *  realignment on assuming control, e.g. when entering a thread after pthread_create. */
#define DSTORM_REALIGN_STACK __attribute__ (( force_align_arg_pointer))
#endif
#endif
