#ifndef _POPENEXT_H_INCLUDED_
#define _POPENEXT_H_INCLUDED_

bool popen_ext (char *, char **, char *type, FILE **, FILE **, pid_t *, const int*, const int);

bool pclose_ext	(FILE**, FILE**, pid_t , int*);

#endif /* _POPENEXT_H_INCLUDED_ */
