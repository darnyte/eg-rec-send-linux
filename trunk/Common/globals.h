#ifndef _GLOBALS_H_INCLUDED_
#define _GLOBALS_H_INCLUDED_


/*//////////////////// Constantes globales del programa - BEGIN - ///////////////////////*/
/*///////////////////////////////////////////////////////////////////////////////////////*/

/* ******************************************************** */
/* NON LOCALIZABLE!!!!!!!!! */

#define FALSE 0
#define TRUE 1

#define STR_TRUE "TRUE"
#define STR_FALSE "FALSE"

#define LINE_BUFFER_SIZE					512

#define EMPTY_STRING						""


#define STR_DEBUG							"DEBUG"
#define STR_INFO							"INFO"
#define STR_ERROR							"ERROR"

#define MIN_VALID_PORT						1
#define MAX_VALID_PORT						UINT16_MAX
#define MAX_VALID_PORT_STR_LEN				5


/* ******************************************************** */

/*///////////////////// Constantes globales del programa - END - ////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////////////////*/


/*/////////////////////// Tipos globales del programa - BEGIN - /////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////////////////*/

typedef unsigned char byte;
typedef unsigned short bool;

/*/////////////////////// Tipos globales del programa - END - ///////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////////////////*/


/*/////////////////////// Macros globales del programa - BEGIN - ////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////////////////*/

#define QUOTE_MACRO(str) #str
#define EXPAND_AND_QUOTE_MACRO(str) QUOTE_MACRO(str)


#if __GNUC__ >= 3
#define GCC_STR_VER EXPAND_AND_QUOTE_MACRO(__GNUC__.__GNUC_MINOR__.__GNUC_PATCHLEVEL__)
#else
#define GCC_STR_VER EXPAND_AND_QUOTE_MACRO(GCC version: __GNUC__.__GNUC_MINOR__)
#endif


#define NOT_USED(x) ( (void)(x) )

#define strequal(x, y) \
	(strcmp((x), (y)) == 0)

#define MAXVAL(a,b) a > b ? a : b

#define MINVAL(a,b) a < b ? a : b

#define BOOL_TO_STRING(boolval) (boolval) ? STR_TRUE : STR_FALSE

#ifdef NDEBUG
#define SHOW_DEBUG_VAL(val, debug_val) (val)
#else
#define SHOW_DEBUG_VAL(val, debug_val) (debug_val)
#endif

/*/////////////////////// Macros globales del programa - END - //////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////////////////*/


bool program_terminated;



inline bool Is_signaled_to_terminate();

inline void Signal_to_terminate(bool);

inline void Free_string(char**);

bool Copy_string (char**, const char*);

bool Luint_to_string (char**, const unsigned long int);

int Rand_lim(int);

bool Hash_cookied_password(const char*, const char*, char*, size_t);

bool Convert_between_latin15_and_current_locale(const char*, char**, bool);

bool Read_stripped_line(FILE *, char**, size_t*, size_t*, bool*, bool, bool);

inline bool Write_line (FILE *, const char*);

bool Convert_num_val(const char*, long int*);

bool Add_string_to_string_array(char***, size_t*, size_t*, const char*);

bool Add_luint_to_string_array (char***, size_t*, size_t*, const unsigned long int);

void Free_string_array(char***, size_t*, size_t*);


#endif /* _GLOBALS_H_INCLUDED_ */
