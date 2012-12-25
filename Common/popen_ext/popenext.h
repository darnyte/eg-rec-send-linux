#ifndef _POPENEXT_H_INCLUDED_
#define _POPENEXT_H_INCLUDED_

#define PIPE_READ_END	0
#define PIPE_WRITE_END	1


bool popen_ext (char *, char **, const bool, FILE **, FILE **, FILE **,
				bool, bool, bool, pid_t *, const int *, const int);

bool pclose_ext	(FILE**, FILE**, FILE **, pid_t , bool, int*);

#endif /* _POPENEXT_H_INCLUDED_ */
