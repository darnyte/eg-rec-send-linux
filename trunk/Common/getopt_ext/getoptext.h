#ifndef _GETOPTEXT_H_INCLUDED_
#define _GETOPTEXT_H_INCLUDED_

#include <getopt.h>


typedef struct option_ext
{
  const char *name;
  /* has_arg can't be an enum because some compilers complain about
     type mismatches in all the code that assumes it is an int.  */
  int has_arg;
  int *flag;
  int val;
  /* ******************** */

  const char* descript;
} option_ext;


void Fill_getopt_long_options(const struct option_ext optsext[], struct option opts[], size_t optcount);

void Print_help(FILE * outputfile, const struct option_ext optsext[], size_t optcount);

bool Generate_short_options_string(char* opts, size_t, const struct option_ext optsext[], size_t optcount, bool noerrors);


#endif /* _GETOPTEXT_H_INCLUDED_ */
