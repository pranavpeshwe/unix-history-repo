/*-
 * Copyright (c) 1991 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)modes.c	5.2 (Berkeley) %G%";
#endif /* not lint */

#include <sys/types.h>
#include <stddef.h>
#include "stty.h"

/*
 * The code in optlist() depends on minus options following regular
 * options, i.e. "foo" must immediately precede "-foo".
 */
struct modes cmodes[] = {
	"cs5",		CS5, CSIZE,
	"cs6",		CS6, CSIZE,
	"cs7",		CS7, CSIZE,
	"cs8",		CS8, CSIZE,
	"cstopb",	CSTOPB, 0,
	"-cstopb",	0, CSTOPB,
	"cread",	CREAD, 0,
	"-cread",	0, CREAD,
	"parenb",	PARENB, 0,
	"-parenb",	0, PARENB,
	"parodd",	PARODD, 0,
	"-parodd",	0, PARODD,
	"parity",	PARENB | CS7, PARODD | CSIZE,
	"-parity",	CS8, PARODD | PARENB | CSIZE,
	"evenp",	PARENB | CS7, PARODD | CSIZE,
	"-evenp",	CS8, PARODD | PARENB | CSIZE,
	"oddp",		PARENB | CS7 | PARODD, CSIZE,
	"-oddp",	CS8, PARODD | PARENB | CSIZE,
	"pass8",	CS8, PARODD | PARENB | CSIZE,
	"hupcl",	HUPCL, 0,
	"-hupcl",	0, HUPCL,
	"hup",		HUPCL, 0,
	"-hup",		0, HUPCL,
	"clocal",	CLOCAL, 0,
	"-clocal",	0, CLOCAL,
	"crtscts",	CRTSCTS, 0,
	"-crtscts",	0, CRTSCTS,
	NULL
};

struct modes imodes[] = {
	"ignbrk",	IGNBRK, 0,
	"-ignbrk",	0, IGNBRK,
	"brkint",	BRKINT, 0,
	"-brkint",	0, BRKINT,
	"ignpar",	IGNPAR, 0,
	"-ignpar",	0, IGNPAR,
	"parmrk",	PARMRK, 0,
	"-parmrk",	0, PARMRK,
	"inpck",	INPCK, 0,
	"-inpck",	0, INPCK,
	"istrip",	ISTRIP, 0,
	"-istrip",	0, ISTRIP,
	"inlcr",	INLCR, 0,
	"-inlcr",	0, INLCR,
	"igncr",	IGNCR, 0,
	"-igncr",	0, IGNCR,
	"icrnl",	ICRNL, 0,
	"-icrnl",	0, ICRNL,
	"ixon",		IXON, 0,
	"-ixon",	0, IXON,
	"flow",		IXON, 0,
	"-flow",	0, IXON,
	"ixoff",	IXOFF, 0,
	"-ixoff",	0, IXOFF,
	"tandem",	IXOFF, 0,
	"-tandem",	0, IXOFF,
	"ixany",	IXANY, 0,
	"-ixany",	0, IXANY,
	"decctlq",	0, IXANY,
	"-decctlq",	IXANY, 0,
	"imaxbel",	IMAXBEL, 0,
	"-imaxbel",	0, IMAXBEL,
	NULL
};

struct modes lmodes[] = {
	"echo",		ECHO, 0,
	"-echo",	0, ECHO,
	"echoe",	ECHOE, 0,
	"-echoe",	0, ECHOE,
	"crterase",	ECHOE, 0,
	"-crterase",	0, ECHOE,
	"crtbs",	ECHOE, 0,   /* crtbs not supported, close enough */
	"-crtbs",	0, ECHOE,
	"echok",	ECHOK, 0,
	"-echok",	0, ECHOK,
	"echoke",	ECHOKE, 0,
	"-echoke",	0, ECHOKE,
	"crtkill",	ECHOKE, 0,
	"-crtkill",	0, ECHOKE,
	"altwerase",	ALTWERASE, 0,
	"-altwerase",	0, ALTWERASE,
	"iexten",	IEXTEN, 0,
	"-iexten",	0, IEXTEN,
	"echonl",	ECHONL, 0,
	"-echonl",	0, ECHONL,
	"echoctl",	ECHOCTL, 0,
	"-echoctl",	0, ECHOCTL,
	"ctlecho",	ECHOCTL, 0,
	"-ctlecho",	0, ECHOCTL,
	"echoprt",	ECHOPRT, 0,
	"-echoprt",	0, ECHOPRT,
	"prterase",	ECHOPRT, 0,
	"-prterase",	0, ECHOPRT,
	"isig",		ISIG, 0,
	"-isig",	0, ISIG,
	"icanon",	ICANON, 0,
	"-icanon",	0, ICANON,
	"noflsh",	NOFLSH, 0,
	"-noflsh",	0, NOFLSH,
	"tostop",	TOSTOP, 0,
	"-tostop",	0, TOSTOP,
	"mdmbuf",	MDMBUF, 0,
	"-mdmbuf",	0, MDMBUF,
	"flusho",	FLUSHO, 0,
	"-flusho",	0, FLUSHO,
	"pendin",	PENDIN, 0,
	"-pendin",	0, PENDIN,
	"crt",		ECHOE|ECHOKE|ECHOCTL, ECHOK|ECHOPRT,
	"-crt",		ECHOK, ECHOE|ECHOKE|ECHOCTL,
	"newcrt",	ECHOE|ECHOKE|ECHOCTL, ECHOK|ECHOPRT,
	"-newcrt",	ECHOK, ECHOE|ECHOKE|ECHOCTL, 
	"nokerninfo",	NOKERNINFO, 0,
	"-nokerninfo",	0, NOKERNINFO,
	"kerninfo",	0, NOKERNINFO,
	"-kerninfo",	NOKERNINFO, 0,
	NULL
};

struct modes omodes[] = {
	"opost",	OPOST, 0,
	"-opost",	0, OPOST,
	"litout",	0, OPOST,
	"-litout",	OPOST, 0,
	"onlcr",	ONLCR, 0,
	"-onlcr",	0, ONLCR,
	"tabs",		0, OXTABS,	/* "preserve" tabs */
	"-tabs",	OXTABS, 0,
	"xtabs",	OXTABS, 0,
	"-xtabs",	0, OXTABS,
	"oxtabs",	OXTABS, 0,
	"-oxtabs",	0, OXTABS,
	NULL
};
