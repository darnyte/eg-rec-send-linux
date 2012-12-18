#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <unistd.h>

#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <math.h>

#include <stdarg.h>
#include <syslog.h>

#include "./../Common/globals.h"
#include "Server.h"
#include "exiterror.h"
#include "./../Common/network/network.h"


/*/////////////////////// Constantes de exiterror - BEGIN - /////////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////////////////*/

#define ERROR_STR_SUCESS                        		"Process completed."
#define ERROR_STR_INVALID_OPTION_AUX                	"Invalid option."
#define ERROR_STR_INVALID_OPTION                		"Invalid option \"%s\"."

#define ERROR_STR_MISSING_REQUIRED_PARAMETER_AUX    	"Missing required parameter."
#define ERROR_STR_MISSING_REQUIRED_PARAMETER    		"Missing required parameter in option \"%s\"."



#define ERROR_STR_INVALID_COMMAND_LINE					"Invalid command line."


#define ERROR_STR_BAD_PORT_NUMBER               		"Port number not valid. Must be in range [%u, %u]."
#define ERROR_STR_BAD_PORT_NUMBER_AUX           		"Port number not valid."
#define ERROR_STR_ONLY_ONE_DAEMON               		"Option \"daemon\" is duplicated."
#define ERROR_STR_ONLY_ONE_PORT                 		"Option \"port\" is duplicated."
#define ERROR_STR_ONLY_ONE_IP                 			"Option \"host\" is duplicated."
#define ERROR_STR_ONLY_ONE_PASSWORD             		"Option \"password\" duplicated."
#define ERROR_STR_ONLY_ONE_CONFIG_FILE					"Option \"config-file\" duplicated."
#define ERROR_STR_NOT_VALID_IP                      	"Not valid Host\\IP."
#define ERROR_STR_NOT_VALID_DEFAULT_IP_PORT				"Not valid default Host\\IP or default port."
#define ERROR_STR_BAD_LOG_LEVEL_NUMBER          		"Log level not valid. Must be in range [%u, %u] or referenced by name."
#define ERROR_STR_BAD_LOG_LEVEL_NUMBER_AUX      		"Log level not valid."
#define ERROR_STR_NON_OPTION_ARGUMENT_FOUND				"Non option argument(s) found."
#define ERROR_STR_OPTION_VERSION_NOT_ALONE				"option \"--version\"\\\"-V\" should be used alone."
#define ERROR_STR_OPTION_HELP_NOT_ALONE					"option \"--help\"\\\"-h\" should be used alone."

#define ERROR_STR_ALLOCATING_MEMORY             		"Cannot allocate memory."
#define ERROR_STR_PROCESSING_INPUT              		"Unexpected error processing input."
#define ERROR_STR_UNEXPECTED                    		"Unexpected error."


#define ERROR_STR_FORKING                       		"Cannot fork."
#define ERROR_STR_CREATING_NEW_SID              		"Cannot create new SID."
#define ERROR_STR_SETTING_WORKING_DIRECTORY     		"Cannot set working directory."



#define ERROR_STR_FATAL                         		"ERROR:\t%s PID [%d] - Fatal error (code %d): %s\n"
#define ERROR_STR_SUCCESS                       		"INFO:\t%s PID [%d] - Information (code %d): %s\n"

#define ERROR_STR_FATAL_SYS                     		"ERROR:\t%s PID [%d] - Fatal error (code %d): %s\n\tSystem error info: %s.\n"
#define ERROR_STR_SUCCESS_SYS                   		"INFO:\t%s PID [%d] - Unformation (code %d): %s\n\tSystem error info: %s.\n"

#define ERROR_STR_WARNING                       		"INFO:\t%s PID [%d] - Warning (code %d): %s\n"
#define ERROR_STR_WARNING_SYS                   		"INFO:\t%s PID [%d] - Warning (code %d): %s\n\tSystem error info: %s.\n"


#define ERROR_STR_DIRECT                        		"ERROR:\t%s PID [%d] - %s\n"



#define ERROR_STR_INVALID_NUM_VALUE						"Error, invalid numeric value."
#define ERROR_STR_ONLY_ONE_LOG_LEVEL                	"Error, option \"log-level\" is duplicated."
#define ERROR_STR_ONLY_ONE_SINGLE	                	"Error, option \"single\" is duplicated."
#define ERROR_STR_ONLY_ONE_USER							"Error, option \"user\" is duplicated."
#define ERROR_STR_ONLY_ONE_COMMAND						"Error, option \"command\" is duplicated."
#define ERROR_STR_INVALID_USER_GROUP                	"Error, invalid user and\\or group."
#define ERROR_STR_CLOSING_STANDARD_FILE_DESCRIPTORS 	"Error closing standard file descriptors."
#define ERROR_STR_SETTING_LOCALE						"Error setting locale."


#define ERROR_STR_DROPPING_ROOT_PRIVILEGES          	"Error droping root privileges."
#define ERROR_STR_CLOSING_CLIENT_SOCKET             	"Error closing client socket."
#define ERROR_STR_CLOSING_LISTENING_SOCKET          	"Error closing listening socket."
#define ERROR_STR_GETTING_NET_BYTE_ORDER_IP_ADDRESS 	"Error getting net byte order IP address."
#define ERROR_STR_BINDING_TO_IP_ADDRESS             	"Error binding to IP address."
#define ERROR_STR_LISTENING                         	"Error listening for connections."
#define ERROR_STR_ACCEPTING_NEW_CONNECTION          	"Error accepting new connection."
#define ERROR_STR_SETTING_SOCKET_OPTIONS				"Error setting socket options."
#define ERROR_STR_SETTING_SOCKET_TIME_OUTS				"Error setting socket time-outs."
#define ERROR_STR_FORKING_FOR_NEW_CLIENT            	"Error forking for new client."

#define ERROR_STR_PROMOTING_SOCKET_TO_FILE				"Error promoting socket to file descriptor."
#define ERROR_STR_READING_MAGIC_WORD					"Error reading \"Magic word\"."
#define ERROR_STR_BAD_MAGIC_WORD						"Error, bad \"Magic word\"."
#define ERROR_STR_WRITING_COOKIE                 		"Error sending cookie."
#define ERROR_STR_READING_HASH							"Error receiving hash."
#define ERROR_STR_WRITING_ACCEPT_MSG					"Error sending \"accept\" message."
#define ERROR_STR_READING_EVENT							"Error reading event."
#define ERROR_STR_EVENT_LINE_TOO_LONG					"Error, event line too long."

#define ERROR_STR_GENERATING_PASSWORD_COOKIE			"Error generating password cookie."
#define ERROR_STR_CALCULATING_HASH						"Error calculating hash."
#define ERROR_STR_HASH_DOES_NOT_MATCH					"Error, hash doesn't match."

#define ERROR_STR_EXECUTING_COMMAND						"Error executing command."


#define ERROR_STR_INTERRUPTED							"Program interrupted."
#define ERROR_STR_SEGMENTATION_FAULT					"Error, segmentation fault."



#define ERROR_STR_CONFIG_FILE_UNKNOWN					"Error in configuration file."
#define ERROR_STR_CONFIG_FILE_READING					"Error reading configuration file."
#define ERROR_STR_CONFIG_FILE_PARSING					"Error in configuration file in line %d."



/*///////////////////////// Constantes de exiterror - END - /////////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////////////////*/

int g_loglevel = LOG_DEBUG;
const char* option_with_error = NULL;
int config_file_line = -1;


static int Set_log_level_mask (int levelmask, bool setsyslog);

static void Message (const int status, const int syserror, const bool network_error, TLogLevel errorlevel,
					 const char* msg_error, const char* msg_succ, const char* msg_error_sys, const char* msg_succ_sys);

bool Start_log()
{
	openlog (STR_PROGRAM_NAME, LOG_PID, LOG_DAEMON);

	return TRUE;
}

bool End_log()
{
	closelog();

	return TRUE;
}

void Logit (TLogLevel level, const char* fmt_str, const int status, const char* pcmessage, const char* str_syserror)
{
	if (g_daemonized || g_processor)
	{
		syslog (TLOGLEVELTOACTUALLOGLEVEL (level), fmt_str, PROCESS_P_C (g_daemonized, g_processor), getpid(), status, pcmessage, str_syserror);
	}
	else
	{
		if ( (g_loglevelmask & LOG_MASK (TLOGLEVELTOACTUALLOGLEVEL (level))) != 0)
			fprintf (stderr, fmt_str, PROCESS_P_C (g_daemonized, g_processor), getpid(), status, pcmessage, str_syserror);
	}
}

void Vlogit (const TLogLevel level, const char* fmt_str, ...)
{
	va_list listPointer;

	va_start (listPointer, fmt_str);

	if (g_daemonized || g_processor)
	{
		vsyslog (TLOGLEVELTOACTUALLOGLEVEL (level), fmt_str, listPointer);
	}
	else
	{
		if ( (g_loglevelmask & LOG_MASK (TLOGLEVELTOACTUALLOGLEVEL (level))) != 0)
			vfprintf (stderr, fmt_str, listPointer);
	}

	va_end (listPointer);
}

/*
Setea el valor de logeo.

1er parámetro: Nivel de log a setear.


devuelve: -.
*/
void Set_actual_log_level (const TLogLevel level, const bool setsyslog)
{
	switch (level)
	{
	case tll_error:
		Set_log_level_mask (LOG_UPTO (LOG_ERR), setsyslog);
		break;

	case tll_info:
		Set_log_level_mask (LOG_UPTO (LOG_INFO), setsyslog);
		break;

	case tll_debug:
		Set_log_level_mask (LOG_UPTO (LOG_DEBUG), setsyslog);
		break;

	default:
		Set_log_level_mask (LOG_UPTO (LOG_ERR), setsyslog);
	}
}


/*
Convierte un código de error pasado punsigned long long x=(unsigned long long)a*b;or parámetro en un mensaje de texto
descriptivo y lo imprime por stderr.

1er parámetro: Código de error.


devuelve: -
*/
void Messagewarning (const int status, const int syserror, const bool network_error)
{
	Message (status, syserror, network_error, tll_info,
			 ERROR_STR_WARNING, ERROR_STR_SUCCESS, ERROR_STR_WARNING_SYS, ERROR_STR_SUCCESS_SYS);
}


/*
Convierte un código de error pasado punsigned long long x=(unsigned long long)a*b;or parámetro en un mensaje de texto
descriptivo y lo imprime por stderr.

1er parámetro: Código de error.


devuelve: -
*/
void Messageerror (const int status, const int syserror, const bool network_error)
{
	Message (status, syserror, network_error, tll_error,
			 ERROR_STR_FATAL, ERROR_STR_SUCCESS, ERROR_STR_FATAL_SYS, ERROR_STR_SUCCESS_SYS);
}


/*

*/
void Messagedirect (const TLogLevel loglevel, const char* msg)
{
	Vlogit (loglevel, ERROR_STR_DIRECT, PROCESS_P_C (g_daemonized, g_processor), getpid(), msg);
}


/*
Convierte un código de error pasado punsigned long long x=(unsigned long long)a*b;or parámetro en un mensaje de texto
descriptivo y lo imprime por stderr.

1er parámetro: Código de error.


devuelve: -
*/
static void Message (const int status, const int syserror, const bool network_error, TLogLevel errorlevel,
					 const char* msg_error, const char* msg_succ, const char* msg_error_sys, const char* msg_succ_sys)
{
	char* pcmessage;
	char* pcauxmsg = NULL;
	const char* str_syserror = NULL;

	switch (status)
	{
	case ERROR_SUCESS:
		pcmessage = ERROR_STR_SUCESS;
		break;
	case ERROR_INVALID_OPTION:


		if (option_with_error != NULL)
		{
			pcauxmsg = (char*) malloc (strlen (ERROR_STR_INVALID_OPTION) +
									   strlen (option_with_error));
		}
		else
			pcauxmsg = NULL;


		if (pcauxmsg)
		{
			sprintf (pcauxmsg, ERROR_STR_INVALID_OPTION, option_with_error);

			pcmessage = pcauxmsg;
		}
		else
			pcmessage = ERROR_STR_INVALID_OPTION_AUX;


		break;
	case ERROR_MISSING_REQUIRED_PARAMETER:


		if (option_with_error != NULL)
		{
			pcauxmsg = (char*) malloc (strlen (ERROR_STR_MISSING_REQUIRED_PARAMETER) +
									   strlen (option_with_error));
		}
		else
			pcauxmsg = NULL;




		if (pcauxmsg)
		{
			sprintf (pcauxmsg, ERROR_STR_MISSING_REQUIRED_PARAMETER,
					 option_with_error);

			pcmessage = pcauxmsg;
		}
		else
			pcmessage = ERROR_STR_MISSING_REQUIRED_PARAMETER_AUX;


		break;
	case ERROR_INVALID_COMMAND_LINE:
		pcmessage = ERROR_STR_INVALID_COMMAND_LINE;
		break;
	case ERROR_ONLY_ONE_DAEMON:
		pcmessage = ERROR_STR_ONLY_ONE_DAEMON;
		break;
	case ERROR_ONLY_ONE_PORT:
		pcmessage = ERROR_STR_ONLY_ONE_PORT;
		break;
	case ERROR_ONLY_ONE_IP:
		pcmessage = ERROR_STR_ONLY_ONE_IP;
		break;
	case ERROR_ONLY_ONE_PASSWORD:
		pcmessage = ERROR_STR_ONLY_ONE_PASSWORD;
		break;
	case ERROR_ONLY_ONE_CONFIG_FILE:
		pcmessage = ERROR_STR_ONLY_ONE_CONFIG_FILE;
		break;
	case ERROR_BAD_PORT_NUMBER:
	case ERROR_PORT_NUMBER_OUT_OF_RANGE:

		pcauxmsg = (char*) malloc (strlen (ERROR_STR_BAD_PORT_NUMBER) +
								   log10 (MIN_VALID_PORT) + log10 (MAX_VALID_PORT) + 3);

		if (pcauxmsg)
		{
			sprintf (pcauxmsg, ERROR_STR_BAD_PORT_NUMBER, MIN_VALID_PORT, MAX_VALID_PORT);
			pcmessage = pcauxmsg;
		}
		else
			pcmessage = ERROR_STR_BAD_PORT_NUMBER_AUX;

		break;
	case ERROR_BAD_LOG_LEVEL_NUMBER:
	case ERROR_LOG_LEVEL_OUT_OF_RANGE:

		pcauxmsg = (char*) malloc (strlen (ERROR_STR_BAD_LOG_LEVEL_NUMBER) +
								   log10 (LL_MIN) + log10 (LL_MAX) + 3);

		if (pcauxmsg)
		{
			sprintf (pcauxmsg, ERROR_STR_BAD_LOG_LEVEL_NUMBER, LL_MIN, LL_MAX);
			pcmessage = pcauxmsg;
		}
		else
			pcmessage = ERROR_STR_BAD_LOG_LEVEL_NUMBER_AUX;

		break;

	case ERROR_CONFIG_FILE_UNKNOWN:
		pcmessage = ERROR_STR_CONFIG_FILE_UNKNOWN;
		break;
	case ERROR_CONFIG_FILE_READING:
		pcmessage = ERROR_STR_CONFIG_FILE_READING;
		break;
	case ERROR_CONFIG_FILE_PARSING:

		pcauxmsg = (char*) malloc (strlen (ERROR_STR_CONFIG_FILE_PARSING) +
								   log10 (config_file_line) + 3);

		if (pcauxmsg)
		{
			sprintf (pcauxmsg, ERROR_STR_CONFIG_FILE_PARSING, config_file_line);
			pcmessage = pcauxmsg;
		}
		else
			pcmessage = ERROR_STR_CONFIG_FILE_UNKNOWN;

		break;

	case ERROR_ALLOCATING_MEMORY:
		pcmessage = ERROR_STR_ALLOCATING_MEMORY;
		break;
	case ERROR_FORKING:
		pcmessage = ERROR_STR_FORKING;
		break;
	case ERROR_CREATING_NEW_SID:
		pcmessage = ERROR_STR_CREATING_NEW_SID;
		break;
	case ERROR_SETTING_WORKING_DIRECTORY:
		pcmessage = ERROR_STR_SETTING_WORKING_DIRECTORY;
		break;
	case ERROR_INVALID_NUM_VALUE:
		pcmessage = ERROR_STR_INVALID_NUM_VALUE;
		break;
	case ERROR_ONLY_ONE_LOG_LEVEL:
		pcmessage = ERROR_STR_ONLY_ONE_LOG_LEVEL;
		break;
	case ERROR_ONLY_ONE_SINGLE:
		pcmessage = ERROR_STR_ONLY_ONE_SINGLE;
		break;
	case ERROR_ONLY_ONE_USER:
		pcmessage = ERROR_STR_ONLY_ONE_USER;
		break;
	case ERROR_ONLY_ONE_COMMAND:
		pcmessage = ERROR_STR_ONLY_ONE_COMMAND;
		break;
	case ERROR_INVALID_USER_GROUP:
		pcmessage = ERROR_STR_INVALID_USER_GROUP;
		break;
	case ERROR_NOT_VALID_IP:
		pcmessage = ERROR_STR_NOT_VALID_IP;
		break;
	case ERROR_OPTION_VERSION_NOT_ALONE:
		pcmessage = ERROR_STR_OPTION_VERSION_NOT_ALONE;
		break;
	case ERROR_OPTION_HELP_NOT_ALONE:
		pcmessage = ERROR_STR_OPTION_HELP_NOT_ALONE;
		break;
	case ERROR_NOT_VALID_DEFAULT_IP_PORT:
		pcmessage = ERROR_STR_NOT_VALID_DEFAULT_IP_PORT;
		break;
	case ERROR_NON_OPTION_ARGUMENT_FOUND:
		pcmessage = ERROR_STR_NON_OPTION_ARGUMENT_FOUND;
		break;
	case ERROR_CLOSING_STANDARD_FILE_DESCRIPTORS:
		pcmessage = ERROR_STR_CLOSING_STANDARD_FILE_DESCRIPTORS;
		break;
	case ERROR_SETTING_LOCALE:
		pcmessage = ERROR_STR_SETTING_LOCALE;
		break;
	case ERROR_DROPPING_ROOT_PRIVILEGES:
		pcmessage = ERROR_STR_DROPPING_ROOT_PRIVILEGES;
		break;
	case ERROR_CLOSING_CLIENT_SOCKET:
		pcmessage = ERROR_STR_CLOSING_CLIENT_SOCKET;
		break;
	case ERROR_CLOSING_LISTENING_SOCKET:
		pcmessage = ERROR_STR_CLOSING_LISTENING_SOCKET;
		break;
	case ERROR_GETTING_NET_BYTE_ORDER_IP_ADDRESS:
		pcmessage = ERROR_STR_GETTING_NET_BYTE_ORDER_IP_ADDRESS;
		break;
	case ERROR_BINDING_TO_IP_ADDRESS:
		pcmessage = ERROR_STR_BINDING_TO_IP_ADDRESS;
		break;
	case ERROR_LISTENING:
		pcmessage = ERROR_STR_LISTENING;
		break;
	case ERROR_ACCEPTING_NEW_CONNECTION:
		pcmessage = ERROR_STR_ACCEPTING_NEW_CONNECTION;
		break;
	case ERROR_SETTING_SOCKET_OPTIONS:
		pcmessage = ERROR_STR_SETTING_SOCKET_OPTIONS;
		break;
	case ERROR_SETTING_SOCKET_TIME_OUTS:
		pcmessage = ERROR_STR_SETTING_SOCKET_TIME_OUTS;
		break;
	case ERROR_FORKING_FOR_NEW_CLIENT:
		pcmessage = ERROR_STR_FORKING_FOR_NEW_CLIENT;
		break;
	case ERROR_PROMOTING_SOCKET_TO_FILE:
		pcmessage = ERROR_STR_PROMOTING_SOCKET_TO_FILE;
		break;
	case ERROR_READING_MAGIC_WORD:
		pcmessage = ERROR_STR_READING_MAGIC_WORD;
		break;
	case ERROR_BAD_MAGIC_WORD:
		pcmessage = ERROR_STR_BAD_MAGIC_WORD;
		break;
	case ERROR_WRITING_COOKIE:
		pcmessage = ERROR_STR_WRITING_COOKIE;
		break;
	case ERROR_READING_HASH:
		pcmessage = ERROR_STR_READING_HASH;
		break;
	case ERROR_WRITING_ACCEPT_MSG:
		pcmessage = ERROR_STR_WRITING_ACCEPT_MSG;
		break;
	case ERROR_READING_EVENT:
		pcmessage = ERROR_STR_READING_EVENT;
		break;
	case ERROR_EVENT_LINE_TOO_LONG:
		pcmessage = ERROR_STR_EVENT_LINE_TOO_LONG;
		break;
	case ERROR_GENERATING_PASSWORD_COOKIE:
		pcmessage = ERROR_STR_GENERATING_PASSWORD_COOKIE;
		break;
	case ERROR_CALCULATING_HASH:
		pcmessage = ERROR_STR_CALCULATING_HASH;
		break;
	case ERROR_HASH_DOES_NOT_MATCH:
		pcmessage = ERROR_STR_HASH_DOES_NOT_MATCH;
		break;
	case ERROR_EXECUTING_COMMAND:
		pcmessage = ERROR_STR_EXECUTING_COMMAND;
		break;
	case ERROR_INTERRUPTED:
		pcmessage = ERROR_STR_INTERRUPTED;
		break;
	case ERROR_SEGMENTATION_FAULT:
		pcmessage = ERROR_STR_SEGMENTATION_FAULT;
		break;

	default:
		pcmessage = ERROR_STR_UNEXPECTED;
	}

	if (syserror)
		str_syserror = network_error ? Network_strerror (syserror) : strerror (syserror);

	Logit ( ( (status != ERROR_SUCESS) && (status != ERROR_INTERRUPTED)) ? errorlevel : tll_info,
			( (status != ERROR_SUCESS) && (status != ERROR_INTERRUPTED)) ?
			( (syserror && str_syserror) ? msg_error_sys : msg_error) :
				( (syserror && str_syserror) ? msg_succ_sys : msg_succ),
				status, pcmessage, str_syserror);

	if (pcauxmsg)
		free (pcauxmsg);
}

static int Set_log_level_mask (int levelmask, bool setsyslog)
{
	g_loglevelmask = levelmask;

	if (setsyslog)
		return setlogmask (levelmask);
	else
		return TRUE;
}


