/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)local.h	8.3 (Berkeley) 7/3/94
 *
 * $FreeBSD$
 */

#include <sys/types.h>	/* for off_t */
#include <pthread.h>

/*
 * Information local to this implementation of stdio,
 * in particular, macros and private variables.
 */

extern int	_ftello __P((FILE *, fpos_t *));
extern int	_fseeko __P((FILE *, off_t, int, int));
extern int	__fflush __P((FILE *fp));
extern int	__sflush __P((FILE *));
extern FILE	*__sfp __P((void));
extern int	__srefill __P((FILE *));
extern int	__sread __P((void *, char *, int));
extern int	__swrite __P((void *, char const *, int));
extern fpos_t	__sseek __P((void *, fpos_t, int));
extern int	__sclose __P((void *));
extern void	__sinit __P((void));
extern void	_cleanup __P((void));
extern void	(*__cleanup) __P((void));
extern void	__smakebuf __P((FILE *));
extern int	__swhatbuf __P((FILE *, size_t *, int *));
extern int	_fwalk __P((int (*)(FILE *)));
extern int	__swsetup __P((FILE *));
extern int	__sflags __P((const char *, int *));
extern int	__ungetc __P((int, FILE *));
extern int	__vfprintf __P((FILE *, const char *, _BSD_VA_LIST_));

extern int	__sdidinit;


/* hold a buncha junk that would grow the ABI */
struct __sFILEX {
	unsigned char	*_up;	/* saved _p when _p is doing ungetc data */
	pthread_mutex_t	fl_mutex;	/* used for MT-safety */
	pthread_t	fl_owner;	/* current owner */
	int		fl_count;	/* recursive lock count */
};

/*
 * Return true iff the given FILE cannot be written now.
 */
#define	cantwrite(fp) \
 	((((fp)->_flags & __SWR) == 0 || \
 	    ((fp)->_bf._base == NULL && ((fp)->_flags & __SSTR) == 0)) && \
	 __swsetup(fp))

/*
 * Test whether the given stdio file has an active ungetc buffer;
 * release such a buffer, without restoring ordinary unread data.
 */
#define	HASUB(fp) ((fp)->_ub._base != NULL)
#define	FREEUB(fp) { \
	if ((fp)->_ub._base != (fp)->_ubuf) \
		free((char *)(fp)->_ub._base); \
	(fp)->_ub._base = NULL; \
}

/*
 * test for an fgetln() buffer.
 */
#define	HASLB(fp) ((fp)->_lb._base != NULL)
#define	FREELB(fp) { \
	free((char *)(fp)->_lb._base); \
	(fp)->_lb._base = NULL; \
}

#define	INITEXTRA(fp) { \
	(fp)->_extra->_up = NULL; \
	(fp)->_extra->fl_mutex = PTHREAD_MUTEX_INITIALIZER; \
	(fp)->_extra->fl_owner = NULL; \
	(fp)->_extra->fl_count = 0; \
}
