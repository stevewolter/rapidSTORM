#ifndef DSTORM_REALIGN_STACK
#if ( defined(_WIN32) || defined(_WIN64) ) 
/** The REALIGN_STACK macro is necessary on Windows platforms, which have a 4 byte stack alignment. 
 *  GCC will only emit code that is 16 byte aligned and assumes such alignment, thus needs to do a 
 *  realignment on assuming control, e.g. when entering a thread after pthread_create. */
#define DSTORM_REALIGN_STACK __attribute__ (( force_align_arg_pointer))
#else
#define DSTORM_REALIGN_STACK
#endif
#endif
