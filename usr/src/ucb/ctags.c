static char *sccsid = "@(#)ctags.c	4.7 (Berkeley) 8/18/83";

#include <stdio.h>
#include <ctype.h>

/*
 * ctags: create a tags file
 */

#define	reg	register
#define	logical	char

#define	TRUE	(1)
#define	FALSE	(0)

#define	iswhite(arg)	(_wht[arg])	/* T if char is white		*/
#define	begtoken(arg)	(_btk[arg])	/* T if char can start token	*/
#define	intoken(arg)	(_itk[arg])	/* T if char can be in token	*/
#define	endtoken(arg)	(_etk[arg])	/* T if char ends tokens	*/
#define	isgood(arg)	(_gd[arg])	/* T if char can be after ')'	*/

#define	max(I1,I2)	(I1 > I2 ? I1 : I2)

struct	nd_st {			/* sorting structure			*/
	char	*entry;			/* function or type name	*/
	char	*file;			/* file name			*/
	logical f;			/* use pattern or line no	*/
	int	lno;			/* for -x option		*/
	char	*pat;			/* search pattern		*/
	logical	been_warned;		/* set if noticed dup		*/
	struct	nd_st	*left,*right;	/* left and right sons		*/
};

long	ftell();
typedef	struct	nd_st	NODE;

logical	number,				/* T if on line starting with #	*/
	term	= FALSE,		/* T if print on terminal	*/
	makefile= TRUE,			/* T if to creat "tags" file	*/
	gotone,				/* found a func already on line	*/
					/* boolean "func" (see init)	*/
	_wht[0177],_etk[0177],_itk[0177],_btk[0177],_gd[0177];

	/* typedefs are recognized using a simple finite automata,
	 * tydef is its state variable.
	 */
typedef enum {none, begin, middle, end } TYST;

TYST tydef = none;

char	searchar = '/';			/* use /.../ searches 		*/

int	lineno;				/* line number of current line */
char	line[4*BUFSIZ],		/* current input line			*/
	*curfile,		/* current input file name		*/
	*outfile= "tags",	/* output file				*/
	*white	= " \f\t\n",	/* white chars				*/
	*endtk	= " \t\n\"'#()[]{}=-+%*/&|^~!<>;,.:?",
				/* token ending chars			*/
	*begtk	= "ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz",
				/* token starting chars			*/
	*intk	= "ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz0123456789",
				/* valid in-token chars			*/
	*notgd	= ",;";		/* non-valid after-function chars	*/

int	file_num;		/* current file number			*/
int	aflag;			/* -a: append to tags */
int	tflag;			/* -t: create tags for typedefs */
int	uflag;			/* -u: update tags */
int	wflag;			/* -w: suppress warnings */
int	vflag;			/* -v: create vgrind style index output */
int	xflag;			/* -x: create cxref style output */

char	lbuf[BUFSIZ];

FILE	*inf,			/* ioptr for current input file		*/
	*outf;			/* ioptr for tags file			*/

long	lineftell;		/* ftell after getc( inf ) == '\n' 	*/

NODE	*head;			/* the head of the sorted binary tree	*/

char	*savestr();
char	*rindex();
main(ac,av)
int	ac;
char	*av[];
{
	char cmd[100];
	int i;

	while (ac > 1 && av[1][0] == '-') {
		for (i=1; av[1][i]; i++) {
			switch(av[1][i]) {
				case 'B':
					searchar='?';
					break;
				case 'F':
					searchar='/';
					break;
				case 'a':
					aflag++;
					break;
				case 't':
					tflag++;
					break;
				case 'u':
					uflag++;
					break;
				case 'w':
					wflag++;
					break;
				case 'v':
					vflag++;
					xflag++;
					break;
				case 'x':
					xflag++;
					break;
				default:
					goto usage;
			}
		}
		ac--; av++;
	}

	if (ac <= 1) {
usage:		printf("Usage: ctags [-BFatuwvx] file ...\n");
		exit(1);
	}

	init();			/* set up boolean "functions"		*/
	/*
	 * loop through files finding functions
	 */
	for (file_num = 1; file_num < ac; file_num++)
		find_entries(av[file_num]);

	if (xflag) {
		put_entries(head);
		exit(0);
	}
	if (uflag) {
		for (i=1; i<ac; i++) {
			sprintf(cmd,
				"mv %s OTAGS;fgrep -v '\t%s\t' OTAGS >%s;rm OTAGS",
				outfile, av[i], outfile);
			system(cmd);
		}
		aflag++;
	}
	outf = fopen(outfile, aflag ? "a" : "w");
	if (outf == NULL) {
		perror(outfile);
		exit(1);
	}
	put_entries(head);
	fclose(outf);
	if (uflag) {
		sprintf(cmd, "sort %s -o %s", outfile, outfile);
		system(cmd);
	}
	exit(0);
}

/*
 * This routine sets up the boolean psuedo-functions which work
 * by seting boolean flags dependent upon the corresponding character
 * Every char which is NOT in that string is not a white char.  Therefore,
 * all of the array "_wht" is set to FALSE, and then the elements
 * subscripted by the chars in "white" are set to TRUE.  Thus "_wht"
 * of a char is TRUE if it is the string "white", else FALSE.
 */
init()
{

	reg	char	*sp;
	reg	int	i;

	for (i = 0; i < 0177; i++) {
		_wht[i] = _etk[i] = _itk[i] = _btk[i] = FALSE;
		_gd[i] = TRUE;
	}
	for (sp = white; *sp; sp++)
		_wht[*sp] = TRUE;
	for (sp = endtk; *sp; sp++)
		_etk[*sp] = TRUE;
	for (sp = intk; *sp; sp++)
		_itk[*sp] = TRUE;
	for (sp = begtk; *sp; sp++)
		_btk[*sp] = TRUE;
	for (sp = notgd; *sp; sp++)
		_gd[*sp] = FALSE;
}

/*
 * This routine opens the specified file and calls the function
 * which finds the function and type definitions.
 */
find_entries(file)
char	*file;
{
	char *cp;

	if ((inf=fopen(file,"r")) == NULL) {
		perror(file);
		return;
	}
	curfile = savestr(file);
	cp = rindex(file, '.');
	/* .l implies lisp source code */
	if (cp && cp[1] == 'l' && cp[2] == '\0') {
		L_funcs(inf);
		fclose(inf);
		return;
	}
	/* if not a .c or .h file, try fortran */
	if (cp && (cp[1] != 'c' && cp[1] != 'h') && cp[2] == '\0') {
		if (PF_funcs(inf) != 0) {
			fclose(inf);
			return;
		}
		rewind(inf);	/* no fortran tags found, try C */
	}
	C_entries();
	fclose(inf);
}

pfnote(name, ln, f)
	char *name;
	logical f;			/* f == TRUE when function */
{
	register char *fp;
	register NODE *np;
	char nbuf[BUFSIZ];

	if ((np = (NODE *) malloc(sizeof (NODE))) == NULL) {
		fprintf(stderr, "ctags: too many entries to sort\n");
		put_entries(head);
		free_tree(head);
		head = np = (NODE *) malloc(sizeof (NODE));
	}
	if (xflag == 0 && !strcmp(name, "main")) {
		fp = rindex(curfile, '/');
		if (fp == 0)
			fp = curfile;
		else
			fp++;
		sprintf(nbuf, "M%s", fp);
		fp = rindex(nbuf, '.');
		if (fp && fp[2] == 0)
			*fp = 0;
		name = nbuf;
	}
	np->entry = savestr(name);
	np->file = curfile;
	np->f = f;
	np->lno = ln;
	np->left = np->right = 0;
	if (xflag == 0) {
		lbuf[50] = 0;
		strcat(lbuf, "$");
		lbuf[50] = 0;
	}
	np->pat = savestr(lbuf);
	if (head == NULL)
		head = np;
	else
		add_node(np, head);
}

/*
 * This routine finds functions and typedefs in C syntax and adds them
 * to the list.
 */
C_entries()
{
	register int c;
	register char *token, *tp;
	logical incomm, inquote, inchar, midtoken;
	int level;
	char *sp;
	char tok[BUFSIZ];

	lineno = 1;
	number = gotone = midtoken = inquote = inchar = incomm = FALSE;
	level = 0;
	sp = tp = token = line;
	for (;;) {
		*sp=c=getc(inf);
		if (feof(inf))
			break;
		if (c == '\n')
			lineno++;
		if (c == '\\') {
			c = *++sp = getc(inf);
			if (c = '\n')
				c = ' ';
		} else if (incomm) {
			if (c == '*') {
				while ((*++sp=c=getc(inf)) == '*')
					continue;
				if (c == '\n')
					lineno++;
				if (c == '/')
					incomm = FALSE;
			}
		} else if (inquote) {
			/*
			 * Too dumb to know about \" not being magic, but
			 * they usually occur in pairs anyway.
			 */
			if (c == '"')
				inquote = FALSE;
			continue;
		} else if (inchar) {
			if (c == '\'')
				inchar = FALSE;
			continue;
		} else switch (c) {
		case '"':
			inquote = TRUE;
			continue;
		case '\'':
			inchar = TRUE;
			continue;
		case '/':
			if ((*++sp=c=getc(inf)) == '*')
				incomm = TRUE;
			else
				ungetc(*sp, inf);
			continue;
		case '#':
			if (sp == line)
				number = TRUE;
			continue;
		case '{':
			if (tydef == begin) {
				tydef=middle;
			}
			level++;
			continue;
		case '}':
			if (sp == line)
				level = 0;	/* reset */
			else
				level--;
			if (!level && tydef==middle) {
				tydef=end;
			}
			continue;
		}
		if (!level && !inquote && !incomm && gotone == FALSE) {
			if (midtoken) {
				if (endtoken(c)) {
					int f;
					int pfline = lineno;
					if (start_entry(&sp,token,tp,&f)) {
						strncpy(tok,token,tp-token+1);
						tok[tp-token+1] = 0;
						getline();
						pfnote(tok, pfline, f);
						gotone = f;	/* function */
					}
					midtoken = FALSE;
					token = sp;
				} else if (intoken(c))
					tp++;
			} else if (begtoken(c)) {
				token = tp = sp;
				midtoken = TRUE;
			}
		}
		if (c == ';'  &&  tydef==end)	/* clean with typedefs */
			tydef=none;
		sp++;
		if (c == '\n' || sp > &line[sizeof (line) - BUFSIZ]) {
			tp = token = sp = line;
			lineftell = ftell(inf);
			number = gotone = midtoken = inquote = inchar = FALSE;
		}
	}
}

/*
 * This routine  checks to see if the current token is
 * at the start of a function, or corresponds to a typedef
 * It updates the input line * so that the '(' will be
 * in it when it returns.
 */
start_entry(lp,token,tp,f)
char	**lp,*token,*tp;
int	*f;
{

	reg	char	c,*sp,*tsp;
	static	logical	found;
	logical	firsttok;		/* T if have seen first token in ()'s */
	int	bad;

	*f = 1;			/* a function */
	sp = *lp;
	c = *sp;
	bad = FALSE;
	if (!number) {		/* space is not allowed in macro defs	*/
		while (iswhite(c)) {
			*++sp = c = getc(inf);
			if (c == '\n') {
				lineno++;
				if (sp > &line[sizeof (line) - BUFSIZ])
					goto ret;
			}
		}
	/* the following tries to make it so that a #define a b(c)	*/
	/* doesn't count as a define of b.				*/
	} else {
		if (!strncmp(token, "define", 6))
			found = 0;
		else
			found++;
		if (found >= 2) {
			gotone = TRUE;
badone:			bad = TRUE;
			goto ret;
		}
	}
	/* check for the typedef cases		*/
	if (tflag && !strncmp(token, "typedef", 7)) {
		tydef=begin;
		goto badone;
	}
	if (tydef==begin && (!strncmp(token, "struct", 6) ||
	    !strncmp(token, "union", 5) || !strncmp(token, "enum", 4))) {
		goto badone;
	}
	if (tydef==begin) {
		tydef=end;
		goto badone;
	}
	if (tydef==end) {
		*f = 0;
		goto ret;
	}
	if (c != '(')
		goto badone;
	firsttok = FALSE;
	while ((*++sp=c=getc(inf)) != ')') {
		if (c == '\n') {
			lineno++;
			if (sp > &line[sizeof (line) - BUFSIZ])
				goto ret;
		}
		/*
		 * This line used to confuse ctags:
		 *	int	(*oldhup)();
		 * This fixes it. A nonwhite char before the first
		 * token, other than a / (in case of a comment in there)
		 * makes this not a declaration.
		 */
		if (begtoken(c) || c=='/') firsttok++;
		else if (!iswhite(c) && !firsttok) goto badone;
	}
	while (iswhite(*++sp=c=getc(inf)))
		if (c == '\n') {
			lineno++;
			if (sp > &line[sizeof (line) - BUFSIZ])
				break;
		}
ret:
	*lp = --sp;
	if (c == '\n')
		lineno--;
	ungetc(c,inf);
	return !bad && (!*f || isgood(c));
					/* hack for typedefs */
}

getline()
{
	long saveftell = ftell( inf );
	register char *cp;

	fseek( inf , lineftell , 0 );
	fgets(lbuf, sizeof lbuf, inf);
	cp = rindex(lbuf, '\n');
	if (cp)
		*cp = 0;
	fseek(inf, saveftell, 0);
}

free_tree(node)
NODE	*node;
{

	while (node) {
		free_tree(node->right);
		cfree(node);
		node = node->left;
	}
}

add_node(node, cur_node)
	NODE *node,*cur_node;
{
	register int dif;

	dif = strcmp(node->entry, cur_node->entry);
	if (dif == 0) {
		if (node->file == cur_node->file) {
			if (!wflag) {
fprintf(stderr,"Duplicate entry in file %s, line %d: %s\n",
    node->file,lineno,node->entry);
fprintf(stderr,"Second entry ignored\n");
			}
			return;
		}
		if (!cur_node->been_warned)
			if (!wflag)
fprintf(stderr,"Duplicate entry in files %s and %s: %s (Warning only)\n",
    node->file, cur_node->file, node->entry);
		cur_node->been_warned = TRUE;
		return;
	} 
	if (dif < 0) {
		if (cur_node->left != NULL)
			add_node(node,cur_node->left);
		else
			cur_node->left = node;
		return;
	}
	if (cur_node->right != NULL)
		add_node(node,cur_node->right);
	else
		cur_node->right = node;
}

put_entries(node)
reg NODE	*node;
{
	reg char	*sp;

	if (node == NULL)
		return;
	put_entries(node->left);
	if (xflag == 0)
		if (node->f) {		/* a function */
			fprintf(outf, "%s\t%s\t%c^",
				node->entry, node->file, searchar);
			for (sp = node->pat; *sp; sp++)
				if (*sp == '\\')
					fprintf(outf, "\\\\");
				else if (*sp == searchar)
					fprintf(outf, "\\%c", searchar);
				else
					putc(*sp, outf);
			fprintf(outf, "%c\n", searchar);
		} else {		/* a typedef; text pattern inadequate */
			fprintf(outf, "%s\t%s\t%d\n",
				node->entry, node->file, node->lno);
		}
	else if (vflag)
		fprintf(stdout, "%s %s %d\n",
				node->entry, node->file, (node->lno+63)/64);
	else
		fprintf(stdout, "%-16s%4d %-16s %s\n",
			node->entry, node->lno, node->file, node->pat);
	put_entries(node->right);
}

char	*dbp = lbuf;
int	pfcnt;

PF_funcs(fi)
	FILE *fi;
{

	lineno = 0;
	pfcnt = 0;
	while (fgets(lbuf, sizeof(lbuf), fi)) {
		lineno++;
		dbp = lbuf;
		if ( *dbp == '%' ) dbp++ ;	/* Ratfor escape to fortran */
		while (isspace(*dbp))
			dbp++;
		if (*dbp == 0)
			continue;
		switch (*dbp |' ') {

		case 'i':
			if (tail("integer"))
				takeprec();
			break;
		case 'r':
			if (tail("real"))
				takeprec();
			break;
		case 'l':
			if (tail("logical"))
				takeprec();
			break;
		case 'c':
			if (tail("complex") || tail("character"))
				takeprec();
			break;
		case 'd':
			if (tail("double")) {
				while (isspace(*dbp))
					dbp++;
				if (*dbp == 0)
					continue;
				if (tail("precision"))
					break;
				continue;
			}
			break;
		}
		while (isspace(*dbp))
			dbp++;
		if (*dbp == 0)
			continue;
		switch (*dbp|' ') {

		case 'f':
			if (tail("function"))
				getit();
			continue;
		case 's':
			if (tail("subroutine"))
				getit();
			continue;
		case 'p':
			if (tail("program")) {
				getit();
				continue;
			}
			if (tail("procedure"))
				getit();
			continue;
		}
	}
	return (pfcnt);
}

tail(cp)
	char *cp;
{
	register int len = 0;

	while (*cp && (*cp&~' ') == ((*(dbp+len))&~' '))
		cp++, len++;
	if (*cp == 0) {
		dbp += len;
		return (1);
	}
	return (0);
}

takeprec()
{

	while (isspace(*dbp))
		dbp++;
	if (*dbp != '*')
		return;
	dbp++;
	while (isspace(*dbp))
		dbp++;
	if (!isdigit(*dbp)) {
		--dbp;		/* force failure */
		return;
	}
	do
		dbp++;
	while (isdigit(*dbp));
}

getit()
{
	register char *cp;
	char c;
	char nambuf[BUFSIZ];

	for (cp = lbuf; *cp; cp++)
		;
	*--cp = 0;	/* zap newline */
	while (isspace(*dbp))
		dbp++;
	if (*dbp == 0 || !isalpha(*dbp))
		return;
	for (cp = dbp+1; *cp && (isalpha(*cp) || isdigit(*cp)); cp++)
		continue;
	c = cp[0];
	cp[0] = 0;
	strcpy(nambuf, dbp);
	cp[0] = c;
	pfnote(nambuf, lineno);
	pfcnt++;
}

char *
savestr(cp)
	char *cp;
{
	register int len;
	register char *dp;

	len = strlen(cp);
	dp = (char *)malloc(len+1);
	strcpy(dp, cp);
	return (dp);
}

/*
 * Return the ptr in sp at which the character c last
 * appears; NULL if not found
 *
 * Identical to v7 rindex, included for portability.
 */

char *
rindex(sp, c)
register char *sp, c;
{
	register char *r;

	r = NULL;
	do {
		if (*sp == c)
			r = sp;
	} while (*sp++);
	return(r);
}

/*
 * lisp tag functions
 * just look for (def or (DEF
 */

L_funcs (fi)
FILE *fi;
{
	lineno = 0;
	pfcnt = 0;
	while (fgets(lbuf, sizeof(lbuf), fi)) {
		lineno++;
		dbp = lbuf;
		if (dbp[0] == '(' && 
		     (dbp[1] == 'D' || dbp[1] == 'd') &&
		     (dbp[2] == 'E' || dbp[2] == 'e') &&
		     (dbp[3] == 'F' || dbp[3] == 'f')) {
		   while (!isspace(*dbp)) dbp++;
		   while (isspace(*dbp)) dbp++;
		   L_getit();
		   }
		}
	}

L_getit()
{
	register char *cp;
	char c;
	char nambuf[BUFSIZ];

	for (cp = lbuf; *cp; cp++) ;
	*--cp = 0;		/* zap newline */
	if (*dbp == 0) return;
	for (cp = dbp+1; *cp && *cp != '(' && *cp != ' '; cp++)
	        continue;
	c = cp[0];
	cp[0] = 0;
	strcpy(nambuf, dbp);
	cp[0] = c;
	pfnote(nambuf, lineno,TRUE);
	pfcnt++;
}
