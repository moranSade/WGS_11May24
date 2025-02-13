/* @(#) $Header: /software/software/vocallo/pktapi/porting/transport/win32/pcap/include/Gnuc.h 1     5/16/08 7:57a Wiada $ (LBL) */

/* Define __P() macro, if necessary */

#ifndef __P
#if __STDC__
#define __P(protos) protos
#else
#define __P(protos) ()
#endif
#endif

/* inline foo */
#ifndef __cplusplus
#ifdef __GNUC__
#define inline __inline
#else
#define inline
#endif
#endif

/*
 * Handle new and old "dead" routine prototypes
 *
 * For example:
 *
 *	__dead void foo(void) __attribute__((volatile));
 *
 */
#ifdef __GNUC__
#ifndef __dead
#define __dead volatile
#endif
#if __GNUC__ < 2  || (__GNUC__ == 2 && __GNUC_MINOR__ < 5)
#ifndef __attribute__
#define __attribute__(args)
#endif
#endif
#else
#ifndef __dead
#define __dead
#endif
#ifndef __attribute__
#define __attribute__(args)
#endif
#endif
