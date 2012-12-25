#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include <math.h>
#include <errno.h>

#include <iconv.h>

#include <assert.h>

#include "globals.h"

#include "./md5/md5.h"



#define COOKIE_SEPARATOR ":"
#define EXPAND_STR_LIST_SIZE	5



bool program_terminated = false;



static inline bool Insert_string_to_string_array (char*** list, size_t* array_len, size_t* buffer_len, char* new_str);


/*

*/
inline bool Is_signaled_to_terminate()
{
	return program_terminated;
}


/*

*/
inline void Signal_to_terminate (bool terminate)
{
	program_terminated = terminate;
}


/*

*/
inline void Free_string (char** str)
{
	if (*str != NULL)
	{
		free (*str);
		*str = NULL;
	}
}


/*

*/
bool Copy_string (char** dest, const char* src)
{
	size_t len;

	/* Check parameters.*/
	assert (dest);
	assert (src);
	/* ------------------------ */


	len = strlen (src);

	errno = 0;

	*dest = malloc (len + 1);

	if (*dest == NULL)
		return false;

	strncpy (*dest, src, len + 1);

	return true;
}


/*

*/
bool Luint_to_string (char** dest, const unsigned long int src)
{
	size_t len;

	/* Check parameters.*/
	assert (dest);
	/* ------------------------ */


	len = (size_t) log10 (src) + 1;

	errno = 0;

	*dest = malloc (len + 1);

	if (*dest == NULL)
		return false;

	bzero (*dest, len + 1);

	sprintf (*dest, "%lu", (unsigned long int) src);

	return true;
}


/*

*/
int Rand_lim (int limit)
{
	int divisor = RAND_MAX / limit;
	int retval;

	do
	{
		retval = rand() / divisor;
	}
	while (retval > (limit - 1));

	return retval;
}


/*

*/
bool Hash_cookied_password (const char* password, const char* cookie, char* hash, size_t hash_len)
{
	md5_state_t state;
	md5_byte_t digest[16];
	int di;

	md5_init (&state);
	md5_append (&state, (const md5_byte_t *) cookie, strlen (cookie));
	md5_append (&state, (const md5_byte_t *) COOKIE_SEPARATOR, strlen (COOKIE_SEPARATOR));
	md5_append (&state, (const md5_byte_t *) password, strlen (password));
	md5_finish (&state, digest);


	/* Check parameters.*/
	assert (password);
	assert (cookie);
	assert (hash);
	assert (hash_len);
	/* ------------------------ */


	bzero (hash, hash_len);

	for (di = 0; di < 16; ++di)
		snprintf (hash + di * 2, 3, "%02x", digest[di]);

	return true;
}

/*

*/
bool Convert_between_latin15_and_current_locale (const char* input, char** output, bool to_utf8)
{
#define CURRENT_LOCALE	""
#define ISO_8859_15		"ISO_8859-15"

	char *in_buf = (char *) input;
	size_t in_left = strlen (input);

	size_t output_size = 2 * in_left + 1;

	char *out_buf;
	size_t out_left;

	iconv_t cd ;


	/* Check parameters.*/
	assert (input);
	assert (output);
	/* ------------------------ */


	errno = 0;

	cd = iconv_open (to_utf8 ? CURRENT_LOCALE : ISO_8859_15, to_utf8 ? ISO_8859_15 : CURRENT_LOCALE); /*ISO_8859-15 OR CP1252*/

	if (cd == (iconv_t) - 1)
		return false;

	out_buf = (*output = malloc (output_size));

	if (*output == NULL)
		return false;

	out_left = output_size;

	do
	{
		errno = 0;

		if ( (iconv (cd, &in_buf, &in_left, &out_buf, &out_left) == (size_t) - 1) && (errno != E2BIG))
		{
			iconv_close (cd);

			free (*output);
			*output = NULL;

			return false;
		}
		else if (errno == E2BIG)
		{
			output_size *= 2;

			out_buf = realloc (*output, output_size);

			if (out_buf == NULL)
			{
				iconv_close (cd);

				free (*output);

				return false;
			}

			*output = out_buf;
		}
	}
	while ( (in_left > 0) && (out_left > 0));

	iconv_close (cd);

	*out_buf = '\0';

	return true;
}


/*

*/
bool Read_stripped_line (FILE *input_lines, char** line_buffer, size_t* buffer_len, size_t* line_len,
						 bool* eof_found, bool delim_CR, bool convert_to_utf8)
{
	ssize_t readed;

	char* utf8_line = NULL;

	*line_buffer = NULL;

	*eof_found = false;


	/* Check parameters.*/
	assert (input_lines);
	assert (line_buffer);
	assert (buffer_len);
	assert (line_len);
	assert (eof_found);
	/* ------------------------ */

	errno = 0;

	*buffer_len = LINE_BUFFER_SIZE;
	*line_buffer = malloc (LINE_BUFFER_SIZE);

	if (*line_buffer == NULL)
	{
		*line_buffer = NULL;
		*line_len = 0;
		return false;
	}

	errno = 0;

	if (delim_CR)
		readed = getdelim (line_buffer, buffer_len, '\r', input_lines);
	else
		readed = getline (line_buffer, buffer_len, input_lines);

	(*eof_found = feof (input_lines));

	if ( (readed == -1) || (*eof_found) || ferror (input_lines) ||
			( (*line_len = strlen (*line_buffer)) != (size_t) readed))
	{
		free (*line_buffer);
		*line_buffer = NULL;
		*line_len = 0;
		return false;
	}

	if ( (*line_buffer) [ (*line_len) - 1] != (delim_CR ? '\r' : '\n'))
	{
		free (*line_buffer);
		*line_buffer = NULL;
		*line_len = 0;
		return false;
	}

	(*line_len)--;
	(*line_buffer) [*line_len] = '\0';

	if (delim_CR)
	{
		if ( (*line_buffer) [ (*line_len) - 1] != '\n')
		{
			free (*line_buffer);
			*line_buffer = NULL;
			*line_len = 0;
			return false;
		}

		(*line_len)--;
		(*line_buffer) [*line_len] = '\0';
	}

	if (convert_to_utf8)
	{
		if (!Convert_between_latin15_and_current_locale (*line_buffer, &utf8_line, true))
		{
			free (*line_buffer);
			*line_buffer = NULL;
			*line_len = 0;
			return false;
		}

		free (*line_buffer);

		*line_buffer = utf8_line;
	}

	return true;
}


/*

*/
inline bool Write_line (FILE *output_lines, const char* text)
{


	/* Check parameters.*/
	assert (output_lines);
	assert (text);
	/* ------------------------ */


	return (fprintf (output_lines, "%s\n", text) >= 0);
}



/*
Convierte un string al valor numérico que representa.

1er parámetro: Puntero al comienzo del string.

3er parámetro: Devuelve el número.


devuelve: Si hubo o no error.
*/
bool Convert_num_val (const char* pcnumstart, long int* pulinum)
{
	bool bconvertresult;

	char* pfirst_invalid = NULL;


	/* Check parameters.*/
	assert (pcnumstart);
	assert (pulinum);
	/* ------------------------ */

	/*
	*pulinum = atoi(pcnumstart);
	*/

	errno = 0;

	*pulinum = strtol (pcnumstart, &pfirst_invalid, 10);

	bconvertresult = (errno == 0) && (*pfirst_invalid == '\0') ;

	return bconvertresult;
}


/*

*/
bool Add_string_to_string_array (char*** list, size_t* array_len, size_t* buffer_len, const char* str)
{
	char* new_str;
	size_t str_len;


	/* Check parameters.*/
	assert (list);
	assert (array_len);
	assert (buffer_len);
	assert (str);
	/* ------------------------ */
	assert (EXPAND_STR_LIST_SIZE >= 1);


	str_len = strlen (str);

	errno = 0;

	new_str = malloc (str_len + 1);

	if (new_str == NULL)
		return true;

	bzero (new_str, str_len + 1);

	strncpy (new_str, str, str_len);

	if (!Insert_string_to_string_array (list, array_len, buffer_len, new_str))
	{
		free (new_str);
		return false;
	}
	else
		return true;
}


/*

*/
bool Add_luint_to_string_array (char*** list, size_t* array_len, size_t* buffer_len, const unsigned long int luint)
{
	char* new_str;
	size_t str_len;


	/* Check parameters.*/
	assert (list);
	assert (array_len);
	assert (buffer_len);
	/* ------------------------ */
	assert (EXPAND_STR_LIST_SIZE >= 1);


	str_len = (size_t) log10 (luint) + 1;

	errno = 0;

	new_str = malloc (str_len + 1);

	if (new_str == NULL)
		return true;

	bzero (new_str, str_len + 1);

	sprintf (new_str, "%lu", luint);

	if (!Insert_string_to_string_array (list, array_len, buffer_len, new_str))
	{
		free (new_str);
		return false;
	}
	else
		return true;
}


/*

*/
void Free_string_array (char*** list, size_t* array_len, size_t* buffer_len)
{
	char* str_aux;
	size_t i;


	/* Check parameters.*/
	assert (list);
	assert (buffer_len);
	/* ------------------------ */


	if ( ( (*list) == NULL) || (*buffer_len == 0))
		return;

	for (i = 0; i < *array_len; i++)
	{
		str_aux = (*list) [i];

		if (str_aux != NULL)
			free (str_aux);

		(*list) [i] = NULL;
	}

	*buffer_len = 0;
	*array_len = 0;
	free (*list);
	*list = NULL;
}


/*

*/
static inline bool Insert_string_to_string_array (char*** list, size_t* array_len, size_t* buffer_len, char* new_str)
{
	char** new_list;
	size_t i;
	size_t expandsize = EXPAND_STR_LIST_SIZE;


	/* Check parameters.*/
	assert (list);
	assert (array_len);
	assert (buffer_len);
	assert (new_str);
	/* ------------------------ */
	assert (EXPAND_STR_LIST_SIZE >= 1);


	if ( ( (*list) == NULL) && (*buffer_len > 0))
	{
		expandsize = *buffer_len;
		*buffer_len = 0;
	}

	if (*buffer_len < (*array_len + 1))
	{
		errno = 0;

		new_list = realloc ( (*list), (*buffer_len + expandsize) * sizeof (char*));

		if (new_list == NULL)
			return false;

		(*list) = new_list;

		*buffer_len += expandsize;

		for (i = *array_len + 1; i < *buffer_len; i++)
			(*list) [i] = NULL;
	}

	(*array_len) ++;

	(*list) [ (*array_len) - 1] = new_str;

	return true;
}



