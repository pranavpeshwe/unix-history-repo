#ifndef lint
static char sccsid[] = "@(#)uuclean.c	5.2 (Berkeley) 7/2/83";
#endif

#include "uucp.h"
#include <signal.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef	NDIR
#include "ndir.h"
#else
#include <sys/dir.h>
#endif

extern time_t time();


/*******
 *
 *	uuclean  -  this program will search through the spool
 *	directory (Spool) and delete all files with a requested
 *	prefix which are older than (nomtime) seconds.
 *	If the -m option is set, the program will try to
 *	send mail to the usid of the file.
 *
 *	options:
 *		-m  -  send mail for deleted file
 *		-d  -  directory to clean
 *		-n  -  time to age files before delete (in hours)
 *		-p  -  prefix for search
 *		-x  -  turn on debug outputs
 *	exit status:
 *		0  -  normal return
 *		1  -  can not read directory
 */

#define DPREFIX "U"
#define NOMTIME 72	/* hours to age files before deletion */

main(argc, argv)
char *argv[];
{
	DIR *dirp;
	char file[NAMESIZE];
	time_t nomtime, ptime;
	struct stat stbuf;
	int mflg=0;
	int orig_uid = getuid();

	strcpy(Progname, "uuclean");
	uucpname(Myname);
	nomtime = NOMTIME * (time_t)3600;

	while (argc>1 && argv[1][0] == '-') {
		switch (argv[1][1]) {
		case 'd':
			Spool = &argv[1][2];
			break;
		case 'm':
			mflg = 1;
			break;
		case 'n':
			nomtime = atoi(&argv[1][2]) * (time_t)3600;
			break;
		case 'p':
			if (&argv[1][2] != '\0')
				stpre(&argv[1][2]);
			break;
		case 'x':
			chkdebug(orig_uid);
			Debug = atoi(&argv[1][2]);
			if (Debug <= 0)
				Debug = 1;
			break;
		default:
			printf("unknown flag %s\n", argv[1]); break;
		}
		--argc;  argv++;
	}

	DEBUG(4, "DEBUG# %s\n", "START");
	chdir(Spool);	/* NO subdirs in uuclean!  rti!trt */

	if ((dirp = opendir(Spool)) == NULL) {
		printf("%s directory unreadable\n", Spool);
		exit(1);
	}

	time(&ptime);
	while (gnamef(dirp, file)) {
		if (!chkpre(file))
			continue;

		if (stat(file, &stbuf) == -1) {	/* NO subdirs in uuclean! */
		DEBUG(4, "stat on %s failed\n", file);
			continue;
		}


		if ((stbuf.st_mode & S_IFMT) == S_IFDIR)
			continue;
/*
 * teklabs.1518, Terry Laskodi, +2s/ctime/mtime/
 * so mv-ing files about does not defeat uuclean
 */
		if ((ptime - stbuf.st_mtime) < nomtime)
			continue;
		if (file[0] == CMDPRE)
			notfyuser(file);
		DEBUG(4, "unlink file %s\n", file);
		unlink(file);
		if (mflg) sdmail(file, stbuf.st_uid);
	}

	closedir(dirp);
	exit(0);
}


#define MAXPRE 10
char Pre[MAXPRE][NAMESIZE];
int Npre = 0;
/***
 *	chkpre(file)	check for prefix
 *	char *file;
 *
 *	return codes:
 *		0  -  not prefix
 *		1  -  is prefix
 */

chkpre(file)
char *file;
{
	int i;

	for (i = 0; i < Npre; i++) {
		if (prefix(Pre[i], file))
			return(1);
		}
	return(0);
}

/***
 *	stpre(p)	store prefix
 *	char *p;
 *
 *	return codes:  none
 */

stpre(p)
char *p;
{
	if (Npre < MAXPRE - 2)
		strcpy(Pre[Npre++], p);
	return;
}

/***
 *	notfyuser(file)	- notfiy requestor of deleted requres
 *
 *	return code - none
 */

notfyuser(file)
char *file;
{
	FILE *fp;
	int numrq;
	char frqst[100], lrqst[100];
	char msg[BUFSIZ];
	char *args[10];

	if ((fp = fopen(file, "r")) == NULL)
		return;
	if (fgets(frqst, 100, fp) == NULL) {
		fclose(fp);
		return;
	}
	numrq = 1;
	while (fgets(lrqst, 100, fp))
		numrq++;
	fclose(fp);
	sprintf(msg,
	  "File %s delete. \nCould not contact remote. \n%d requests deleted.\n", file, numrq);
	if (numrq == 1) {
		strcat(msg, "REQUEST: ");
		strcat(msg, frqst);
	}
	else {
		strcat(msg, "FIRST REQUEST: ");
		strcat(msg, frqst);
		strcat(msg, "\nLAST REQUEST: ");
		strcat(msg, lrqst);
	}
	getargs(frqst, args);
	mailst(args[3], msg, "");
	return;
}


/***
 *	sdmail(file, uid)
 *
 *	sdmail  -  this routine will determine the owner
 *	of the file (file), create a message string and
 *	call "mailst" to send the cleanup message.
 *	This is only implemented for local system
 *	mail at this time.
 */

sdmail(file, uid)
char *file;
{
	static struct passwd *pwd;
	struct passwd *getpwuid();
	char mstr[40];

	sprintf(mstr, "uuclean deleted file %s\n", file);
	if (pwd->pw_uid == uid) {
		mailst(pwd->pw_name, mstr, "");
	return(0);
	}

	setpwent();
	if ((pwd = getpwuid(uid)) != NULL) {
		mailst(pwd->pw_name, mstr, "");
	}
	return(0);
}

cleanup(code)
int code;
{
	exit(code);
}
