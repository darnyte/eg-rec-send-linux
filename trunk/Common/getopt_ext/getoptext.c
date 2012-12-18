#include <stdlib.h>
#include <stdio.h>

#include "../globals.h"
#include "getoptext.h"


#define STR_HELP_LINE "\t-%c, --%s\t%s\n"


/*

*/
void Fill_getopt_long_options(const struct option_ext optsext[], struct option opts[], size_t optcount)
{
	size_t i;

	for(i = 0; i < optcount; i++)
	{
		opts[i].name =		optsext[i].name;
		opts[i].has_arg =	optsext[i].has_arg;
		opts[i].flag =		optsext[i].flag;
		opts[i].val =		optsext[i].val;
	}
}


/*

*/
void Print_help(FILE * outputfile, const struct option_ext optsext[], size_t optcount)
{
	size_t i;

	fprintf(outputfile, "\n");

	for(i = 0; i < optcount; i++)
	{
		fprintf(outputfile, STR_HELP_LINE, optsext[i].val, optsext[i].name, optsext[i].descript);
	}

	fprintf(outputfile, "\n");
}


/*

*/
void Generate_short_options_string(char* opts, const struct option_ext optsext[], size_t optcount, bool noerrors)
{
	size_t i, inc;
	char* pos = opts;
	char* fmt = NULL;

	if (noerrors)
	{
		*opts = ':';
		pos++;
	}

	for(i = 0; i < optcount; i++)
	{
		if (optsext[i].has_arg == no_argument)
		{
			fmt = "%c";
			inc = 1;
		}
		else
		{
			fmt = "%c:";
			inc = 2;
		}

		sprintf(pos, fmt, (char) optsext[i].val);

		pos+= inc;
	}

	*pos = '\0';
}

