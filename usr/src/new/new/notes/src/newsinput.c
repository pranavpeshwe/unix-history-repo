static char *sccsid = "@(#)newsinput.c	1.3 2/2/83";

#include "parms.h"
#include "structs.h"
#include "globs.h"
#include "newsgate.h"
/*
 *	newsinput.c
 *
 *	This program will accept an article according the the
 *	'A' protocol for news(I).  The article will then be inserted in
 *	the notefile which matches the newsgroup.  
 *	The program will not insert an article into multiple newsgroups! 
 *
 *	Modified April 1982:
 *	The newer version is somewhat more streamlined. It still uses the
 *	A protocol for intersystem transfer, but since the B news release
 *	still accepts this stuff, I don't mind. I also am too lazy to bother
 *	rewriting the header parser to grab the B format. ( someone could
 *	probably figure a way to simply steal the code out of B news and
 *	deposit it here, but I have other things I would rather do)
 *
 *	So, the other major difference in the new version is that if the
 *	title is of the form:
 *		some title - (nf)
 *	(or if the suffix is "- (nf)"
 *	we decide that it has been generated by notesfiles. This means that
 *	we are allowed to skip text up to a # and then we have a special 
 *	ALMOST generic notesfile format. The header information is 
 *	slightly different.
 *	see canon.h for the new format examples
 *
 *	Original Coding:	Ray Essick		Feb 1982
 *	Modified extensively:	Ray Essick		April 1982
 *	Modified to add some neat things
 *				Malcolm Slaney 		July 1982
 *	added code to catch misformed articles
 *				Ray Essick		July 27, 1982
 *	Modified to add even more neat things
 *				Rick L Spickelmier, UCB 1982
 *	Adding the B news parser
 *				Rick L Spickelmier
 *
 */

extern char longtitle[WDLEN];   /* storage for titles longer the TITLEN */

main (argc, argv)
char  **argv;
{
    struct io_f io;
    char    title[TITLEN + 40];			/* title */
    char    nf[NNLEN];				/* the notefile */
    char    fname[WDLEN];			/* hold scratch file name */
    char    ngroup[WDLEN];			/* newsgroup name */
    char    fromsys[SYSSZ],
            origsys[SYSSZ];
    char   *p,
           *q,
           *r;
    char   *suffix;
    int     count,
            atcount,				/* count ARPA hosts */
            status;
    int     c;
    int     i,
            j,
            stat,
            length,				/* of title */
            notenum;
    struct note_f   note;
    struct note_f   note2;
    struct auth_f   auth;
    long newsseq;
    struct id_f respid;
    struct when_f   entered,
                    whentime;
    struct id_f newsid;
    char    line[CMDLEN];		/* scratch */
    struct daddr_f  where;
    FILE * rawnews;			/* raw news read from here */
    FILE * scr;				/* scratch file for holding article */

#include "main.i"			/* common init code and such */

    rawnews = stdin;			/* read from the right file */

/* Parse the header */

    if (getc(rawnews) != 'A') {
	fprintf(stderr, "Article not protocol A\n");
	exit(BAD);			/* wrong protocol */
    }

    fscanf(rawnews, "%[^.].%ld", origsys, &newsseq);
    while (getc(rawnews) != '\n');		/* skip to next line */

/* grab the 'newsgroup' */

    for (i = 0; (i < WDLEN-1) && ((c = getc(rawnews)) != '\n') && (c != ','); i++)
	ngroup[i] = c;

    ngroup[++i] = '\0';				/* null terminate */

    if (c != '\n') {
	while (getc(rawnews) != '\n');		/* skip to eol */
    }

    /* now the author line */

    for (i = 0; (i < CMDLEN-1) && ((c = getc (rawnews)) != '\n'); i++) {
	line[i] = c;				/* grab the whole line */
    }
    line[i] = '\0';				/* terminate it */
    if (c != '\n') {
	while (getc (rawnews) != '\n');		/* make sure is end */
    }
    for (j = 0; j < i; j++)
	if (line[j] == ' ')
	    break;
    while (j && (line[j] != '!') && (line[j] != ':') && (line[j] != '.')) {
	j--;					/* back to sys */
    }
    atcount = 0;				/* count ARPA hosts parsed */

    for (i = 0, j++; (i < NAMESZ); i++) {
	if ((auth.aname[i] = line[j + i]) == '@' && atcount++) {
	    auth.aname[i] = '\0';
	    break;				/* only parse 1 ARPA host */
	} else {
	    if (auth.aname[i] == ' ') {
		auth.aname[i] = '\0';		/* null terminate */
		break;				/* and get out of here */
	    } else {
		if (auth.aname[i] == 0) {
		    break;
		}
	    }
	}
    }
    /* build the from system */
    for (i = 0; (i < SYSSZ - 1) && line[i] != '!' && line[i] != ':' && line[i] != '.'; i++) {
	fromsys[i] = line[i];
    }
    fromsys[i] = '\0';


/* get the date written */

    i = 0;
    while ((line[i++] = getc (rawnews)) != '\n');
    line[i] = '\0';				/* build this line */
    getdate (line, &entered);			/* and grab it */

/* now the title line */

    for (i = 0; (i < TITLEN + 39) && ((c = getc(rawnews)) != '\n'); i++) {
	title[i] = c;
	longtitle[i] = c;
    }
    title[i] = '\0';				/* null terminator there */

    /* long title stuff added by RLS */
    longtitle[i++] = c;
    if (c != '\n') {
	while ((c = getc(rawnews)) != '\n') {	/* skip eol */
	    longtitle[i++] = c;
	}
	longtitle[i++] = '\n';
    }
    longtitle[i++] = '\n';
    longtitle[i] = '\0';

/*
 *		The header is now parsed, all left is text. Check existence
 *	of the notesfile, permission to write and other things like that
 */

    newsgroup (ngroup, nf, NEWSNF);		/* alias the bugger */
    
    if (init(&io, nf) < 0) {			/* well is it there? */
	printf("Non-existant notesfile: %s\n", nf);
#ifdef AUTOCREATE
	/* try to create the notes file - RLS */
	sprintf(line,"%s/%s/mknf -on %s",MSTDIR,UTILITY,nf);
	system(line);
	if (init(&io, nf) < 0) {
	    sprintf(line, "notesfile: %s, newsgroup %s\n", ngroup, nf);
	    nfcomment(NOSUCHWARN, line, "Failure", 0, 0);
	    exit(BAD);
	}
	    
	sprintf(line, "Created: %s\n", nf);
        nfcomment(NOSUCHWARN, line, line, 0, 0);
#else
	sprintf(line, "notesfile: %s, newsgroup %s\n", ngroup, nf);
	nfcomment(NOSUCHWARN, line, "Failure", 0, 0);
	exit(BAD);
#endif
    }

    if ((io.descr.d_stat & NETWRKD) == 0) {		/* networked ? */
	printf("Notesfile %s is not networked\n", nf);
	exit (BAD);
    }

    r = q = title;
    while (*r) {		/* look for last '-' */
	if (*r == '-') {
	    q = r;		/* this is new last */
	}
	r++;			/* on to the next */
    }
    suffix = q;			/* save the pointer for possible later use */
				/* like misformed articles with "- (nf)" */
    length = q - title;		/* non-nulls before '-' */
    if (strcmp(q, NFSUFFIX) != 0 && strcmp(q, OLDSUFFIX) != 0) {
					/* so is not nf generated */
/*
 *	we come back up to here if the article had the notesfile
 *	suffix and was not properly formatted.
 *	This is a stopgap measure due to the fact that I misdesigned
 *	the system so that notes used a right associative rule
 *	to determine its articles, while news programs used a
 *	left associative rule. Oh well. 
 *		- Ray Essick	July 27, 1982
 */

badform: 		/* come here if no "#" in a "- (nf)" article */

	getperms(&io, 1, origsys);
	if (allow(&io, WRITOK) == 0) {
	    printf("System %s not allowed to write notes\n", origsys);
	    exit (BAD);
	}


	strmove(origsys, newsid.sys);			/* build uniq id */
	newsid.uniqid = (-newsseq * 100) - 1;
							/* is not there yet */
	strmove(origsys, note.n_id.sys);
	note.n_id.uniqid = (-100 * newsseq);	/* build the note descriptor */
	copydate(&entered, &note.n_date);
	strmove(fromsys, note.n_from);
	stat = FRMNEWS;				/* came from news system */

	i = 0;					/* fix up the title */
	while (title[i++]);			/* get to the end */
	for (i--; i < TITLEN; i++) {
	    title[i] = ' ';			/* space pad */
	}

	lock(&io, 'n');
	if ((notenum = chknote (&io, &note.n_id, &note2)) == 0) {
	    if (!strncmp (title, "Re:", 3)) {	/* see if a "followup" */
		p = title;
		do {
		    p += 3;
		    while (*p == ' ')
			p++;				/* Skip Spaces */
		} while (!strncmp (p, "Re:", 3));       /* get all re's */
		strcpy(io.xstring, p);
		notenum = findtitle(&io, io.descr.d_nnote);
		if (notenum > 0 && !chkresp(&io, &newsid, &note2, notenum)) {
		    longtitle[0] = '\0';
		    pagein(&io, rawnews, &where);
		    gettime(&whentime);
		    putresp(&io, &where, stat, notenum, &entered, &auth,
			    &note, NOLOCKIT, &newsid, NOADDID, fromsys, ADDTIME,
			    &whentime);
		} else {
		    if (notenum < 1) {
			pagein(&io, rawnews, &where);
			notenum = putnote(&io, &where, title, stat, &note, &auth,
				NOPOLICY, NOLOCKIT, NOADDID, fromsys, ADDTIME);
			io.nnotrcvd++;		/* count as networked in */
		    } else {
			printf("Duplicate Response handed back by news\n");
		    }
		}
	    } else {
		pagein(&io, rawnews, &where);
		notenum = putnote(&io, &where, title, stat, &note, &auth,
			NOPOLICY, NOLOCKIT, NOADDID, fromsys, ADDTIME);
		io.nnotrcvd++;			/* count as networked in */
	    }
	} else if (note2.n_stat & ORPHND) {	/* replace foster parent */
		pagein(&io, rawnews, &note2.n_addr);   /* collect text */
		gettime(&note2.n_rcvd);		/* current tod */
		gettime(&note2.n_lmod);		/* last touched */
		copyauth(&auth, &note2.n_auth);	/* fill in the author */
		note2.n_stat |= FRMNEWS;		/* brand it */
		for (i = 0; i < TITLEN; i++) {
		    note2.ntitle[i] = title[i];		/* move new title */
		}
		copydate(&entered, &note2.n_date);     /* written date */
		strcpy(note2.n_from, fromsys);	/* and who gave it to us */
		putnrec(&io, notenum, &note2);		/* and replace */
	    } else {
		printf("Duplicate news article received\n");
	    }

	    unlock(&io, 'n');

    } else {					/* nf generated article */
	sprintf(fname, "/tmp/nfn%d", getpid ());       /* generate name */
	scr = fopen(fname, "w");			/* open scratch file */
	while ((c = getc(rawnews)) != '#' && c != EOF) { /* find start */
	    if (scr != NULL) {
		putc(c, scr);		/* hold it in the scratch file */
	    }
	}
	if (c == EOF) {
	    fclose(scr);			/* flush what is there */
	    if ((rawnews = fopen(fname, "r")) == NULL) {
		printf("Article lost in file system: %s\n", fname);
		unlink(fname);			/* remove the file */
		closenf(&io);			/* close the notesfile */
		exit(BAD);
	    }
	    *--suffix = '\0';		/* remove " - (nf)" suffix */
	    unlink(fname);		/* only link is the file descriptor */
	    goto badform;		/* reparse as a news article */
	}

	fclose(scr);				/* close and */
	unlink(fname);				/* toss the scratch file */

	switch (getc(rawnews)) {
	    case 'N': 			/* base note coming through news */
		if (fscanf(rawnews, ":%[^:]:%ld:%o:%d", note.n_id.sys,
			    &note.n_id.uniqid, &status, &count) != 4)
		    goto toobad;	/* bad form */
		while (getc(rawnews) != '\n');		/* skip eol */

		for (i = length; i < TITLEN; i++) {
		    title[i] = ' ';			/* space fill */
		}

		/* skip sys name, got elsewhere */
		while ((c = getc(rawnews)) != '!');  

		for (i = 0; (i < NAMESZ - 1) && ((c = getc (rawnews)) != ' '); i++)
		    auth.aname[i] = c;			/* get the author */

		i = 0;
		while ((line[i] = getc(rawnews)) != '\n');
							/* grab date */
		getdate(line, &note.n_date);	/* use secret decoder ring */


		while (getc(rawnews) != '\n');	/* loop in case trash on line */
					/* in theory this line is empty */

		getperms(&io, 1, note.n_id.sys);     /* see if he's allowed */
		if (allow(&io, WRITOK) == 0) {	/* not a chance */
		    closenf(&io);
		    exit(BAD);
		}

		lock(&io, 'n');
		if ((notenum = chknote (&io, &note.n_id, &note2)) == 0) {
					/* is it there? */
		    pagein(&io, rawnews, &where);      /* grab text */
		    status |= FRMNEWS;			/* came through news! */
		    strmove(fromsys, note.n_from);
		    putnote(&io, &where, title, status, &note, &auth, NOPOLICY,
			    NOLOCKIT, NOADDID, fromsys, ADDTIME);
		    io.nnotrcvd++;			/* count it */
		} else {
		    if ((note2.n_stat & ORPHND) && NOT (status & ORPHND)) {				/* extant is orphan, new isnt */
			pagein(&io, rawnews, &note2.n_addr);
						/* grab text */
			gettime(&note2.n_rcvd);
			gettime(&note2.n_lmod);	/* time stamp it */
			copyauth(&auth, &note2.n_auth);
							/* put correct author */
			note2.n_stat = status + FRMNEWS;
							/* correct status */
			for (i = 0; i < TITLEN; i++) {
			    note2.ntitle[i] = title[i];
			}
			copydate(&entered, &note2.n_date);
			strmove(fromsys, note2.n_from);
			putnrec(&io, notenum, &note2); /* and replace */
		    } else {					/* duplicate */
			printf("Duplicate note handed back by news\n");
		    }
		}
		unlock(&io, 'n');
		break;

	    case 'R': 			/* response coming through news */
		if (fscanf(rawnews, ":%[^:]:%ld:%[^:]:%ld:%o:%d", note.n_id.sys,
			    &note.n_id.uniqid, respid.sys, &respid.uniqid, &status, &count) != 6)
		    goto toobad;			/* bad form */
		while (getc(rawnews) != '\n');		/* skip eol */

		getperms(&io, 1, respid.sys);	/* see if he's allowed */
		if (allow(&io, RESPOK) == 0) {		/* not a chance */
		    closenf(&io);
		    exit(BAD);
		}

		while ((c = getc(rawnews)) != '!');    /* skip sys name, got elsewhere */
		for (i = 0; (i < NAMESZ - 1) && ((c = getc(rawnews)) != ' '); i++)
		    auth.aname[i] = c;			/* get the author */

		i = 0;
		while ((line[i] = getc(rawnews)) != '\n');
							/* grab date */
		getdate(line, &entered);	/* use secret decoder ring */

		while (getc(rawnews) != '\n');	/* loop in case trash on line */
					/* in theory this line is empty */

		lock(&io, 'n');
		notenum = chknote(&io, &note.n_id, &note2);
		if (notenum == 0) {		/* build foster parent */
		    printf("Orphaned response handed in by news\n");
		    strmove(fromsys, note.n_from);
		    note.n_nresp = 0;
		    note.n_auth.aid = ANONUID;
		    strcpy(note.n_auth.aname, "Unknown");
		    copydate(&entered, &note.n_date);
		    gettime(&whentime);		/* get current time */
		    status = ORPHND + FRMNEWS;		/* combo there */
		    for (i = 0, p = "Orphaned Response"; (i < TITLEN) && *p; p++, i++)
			note.ntitle[i] = *p;
		    for (; i < TITLEN; i++) {
			note.ntitle[i] = ' ';	 /* pad */
		    }

		    where.addr = 0;			/* no text */
		    notenum = putnote(&io, &where, note.ntitle, status,
			    &note, &note.n_auth, NOPOLICY, NOLOCKIT, NOADDID, fromsys, ADDTIME);
		    io.norphans++;			/* census of orphans */
		    getnrec(&io, notenum, &note2);     /* get good one */
		}
		if (chkresp(&io, &respid, &note2, notenum) == 0) {
		 					/* none, insert it */
		    status |= FRMNEWS;
		    longtitle[0] = '\0';
		    pagein(&io, rawnews, &where);
		    putresp(&io, &where, status, notenum, &entered, &auth,
			    &note, NOLOCKIT, &respid, NOADDID, fromsys, ADDTIME, &whentime);
		    io.nrsprcvd++;			/* count him in */
		} else {
		    printf("Duplicate response handed back by news\n");
		}
		unlock(&io, 'n');
		break;

	    default: 			/* bad news coming through news */
	toobad: 			/* label for bad format jumps */
		printf("Some sort of failure caused jump to toobad\n");
		closenf(&io);
		exit(BAD);
	}
    }					/* end of nf coming back in */

    lock(&io, 'n');
    getdscr(&io, &io.descr);
    io.descr.netwrkins++;		/* count as net in */
    putdscr(&io, &io.descr);
    unlock(&io, 'n');

    finish(&io);
    exit(GOOD);
}
