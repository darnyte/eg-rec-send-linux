#include <stdlib.h>
#include <stdio.h>

#include <string.h>

#include <assert.h>

#include "../globals.h"
#include "getoptext.h"


#define STR_HELP_LINE_A "\t-%c, --%s"
#define STR_HELP_LINE_B "\t%s\n"

/*

*/
void Fill_getopt_long_options (const struct option_ext optsext[], struct option opts[], size_t optcount)
{
	size_t i;

	for (i = 0; i < optcount; i++)
	{
		opts[i].name =		optsext[i].name;
		opts[i].has_arg =	optsext[i].has_arg;
		opts[i].flag =		optsext[i].flag;
		opts[i].val =		optsext[i].val;
	}
}


/*

*/
void Print_help (FILE * outputfile, const struct option_ext optsext[], size_t optcount)
{
	size_t i, j;
	size_t max_name_len, len = 0;

	fprintf (outputfile, "\n");

	for (i = 0; i < optcount; i++)
	{
		if ( (len = strlen (optsext[i].name)) > max_name_len)
			max_name_len = len;
	}

	for (i = 0; i < optcount; i++)
	{
		fprintf (outputfile, STR_HELP_LINE_A, optsext[i].val, optsext[i].name);

		for (j = strlen (optsext[i].name); j < max_name_len; j++)
			fprintf (outputfile, " ");

		fprintf (outputfile, STR_HELP_LINE_B, optsext[i].descript);
	}

	fprintf (outputfile, "\n");
}


/*

*/
bool Generate_short_options_string (char* opts, size_t opts_buf_len,
									const struct option_ext optsext[], size_t optcount, bool noerrors)
{
	size_t i, inc;
	char* pos = opts;
	char* fmt = NULL;


	/* Check pointer parameters.*/
	assert (opts);
	assert (opts_buf_len >= 2);
	/* ------------------------ */


	if (opts_buf_len < 2)
		return false;

	if (noerrors)
	{
		*opts = ':';
		pos++;
		opts_buf_len--;
	}

	for (i = 0; i < optcount; i++)
	{
		if (opts_buf_len == 0)
			return false;

		if (optsext[i].has_arg == no_argument)
		{
			fmt = "%c";
			inc = 1;
		}
		else if (optsext[i].has_arg == required_argument)
		{
			fmt = "%c:";
			inc = 2;
		}
		else if (optsext[i].has_arg == optional_argument)
		{
			fmt = "%c::";
			inc = 3;
		}
		else
			return false;

		if (snprintf (pos, opts_buf_len, fmt, (char) optsext[i].val) != (int) inc)
			return false;

		pos += inc;

		if (opts_buf_len >= inc)
			opts_buf_len -= inc;
		else
			opts_buf_len = 0;
	}

	if (opts_buf_len < 1)
		return false;

	*pos = '\0';

	return true;
}

