#ifndef _SERVER_H_INCLUDED_
#define _SERVER_H_INCLUDED_


/*/////////////////////// Constants globales- main - BEGIN - ////////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////////////////*/


/* ******************************************************** */
/* NON LOCALIZABLE!!!!!!!!! */

#define WORKING_DIR							"/"
#define FILE_MASK							0

#define DEFAULT_DAEMON						FALSE
#define DEFAULT_PORT						1024
#define DEFAULT_IP							"0.0.0.0"
#define DEFAULT_PASSWORD					""
#define DEFAULT_COMMAND						NULL
#define DEFAULT_CONFIG_FILE_NAME			NULL
#define DEFAULT_SINGLE						FALSE
#define DEFAULT_USER						NULL
#ifdef NDEBUG
#define DEFAULT_LOG_LEVEL					tll_error
#else
#define DEFAULT_LOG_LEVEL					tll_debug
#endif


#define DEFAULT_TIME_OUT					3.5
#define EXECUTE_READ_WRITE_LINE_TIME_OUT	2.0
#define EXECUTE_WAIT_TIME_OUT				2.5


#define STR_ASTERISKS_PASS					"***********"
#define WILDCARD_STRING						"*"

#define MAX_CLIENT_CONNECTIONS				25


#ifndef STR_PROGRAM_NAME
#define STR_PROGRAM_NAME					"EGReceiver"
#endif

#define STR_PROGRAM_INTERNAL_NAME			"Eventghost Receiver"

#ifndef VERSION_MAJOR
#define VERSION_MAJOR						0
#endif
#ifndef VERSION_MINOR
#define VERSION_MINOR						1
#endif

#if VERSION_MAJOR < 1
#if VERSION_MINOR <= 8
#define VERSION_TYPE						" (Alpha)"
#else
#define VERSION_TYPE						" (Bchar** incompatible with const char **eta)"
#endif
#else
#define VERSION_TYPE						""
#endif


#define OPTIONS_COUNT						11


#define STR_HELP							"help"
#define STR_VERSION							"version"
#define STR_DAEMON							"daemon"
#define STR_IP								"host"
#define STR_PORT							"port"
#define STR_PASSWORD						"password"
#define STR_SINGLE							"single"
#define STR_USER							"user"
#define STR_COMMAND							"command"

#define STR_CONFIG_FILE						"config-file"

#define STR_LOG_LEVEL						"log-level"




#define STR_CONFIG_DAEMON					"DAEMON"
#define STR_CONFIG_IP						"HOST"
#define STR_CONFIG_PORT						"PORT"
#define STR_CONFIG_PASSWORD					"PASSWORD"
#define STR_CONFIG_SINGLE					"SINGLE"
#define STR_CONFIG_USER						"USER"
#define STR_CONFIG_COMMAND					"COMMAND"
#define STR_CONFIG_LOG_LEVEL				"LOGLEVEL"




#define STR_HELP_SHORT						'h'
#define STR_VERSION_SHORT					'V'
#define STR_DAEMON_SHORT					'd'
#define STR_IP_SHORT						'i'
#define STR_PORT_SHORT						'r'
#define STR_PASSWORD_SHORT					'p'
#define STR_SINGLE_SHORT					's'
#define STR_USER_SHORT						'u'
#define STR_COMMAND_SHORT					'c'
#define STR_CONFIG_FILE_SHORT				'f'
#define STR_LOG_LEVEL_SHORT					'l'


/* ******************************************************** */



#define STR_VERSION_DESCRIPTION				"\n%s daemon version: \"%u.%u.%u\"%s\n\n"
#define STR_COMPILED						"Compiled with GCC version \"%s\".\n\n"
#define STR_COMPILED_DATE					"Build date:\t\"%s\".\n\n"

#define STR_COMPILED_ON						"Compilation platform:\n\nOS name:\t\"%s\".\nKernel:\t\t\"%s\".\n\
Kernel release:\t\"%s\".\nKernel version:\t\"%s\".\nMachine:\t\"%s\".\nProcessor:\t\"%s\".\nHardware:\t\"%s\".\n\n"



#define STR_HELP_DESC						"Prints usage information."
#define STR_VERSION_DESC					"Prints version information."
#define STR_DAEMON_DESC						"Run as daemon."
#define STR_IP_DESC							"Host\\IP to bind to."
#define STR_PORT_DESC						"Port to listen."
#define STR_PASSWORD_DESC					"Receiver password."
#define STR_SINGLE_DESC						"Don't accept multiple simultaneous conexions."
#define STR_USER_DESC						"User:Group to drop to when running as root."
#define STR_COMMAND_DESC					"Command (program or script) that process the payload."
#define STR_CONFIG_FILE_DESC				"Configuration file."
#define STR_LOG_LEVEL_DESC					"Log level [ERROR=1, INFO=2, DEBUG=3]."

#define STR_USAGE_TEXT	\
	"\nUsage:\n\n"\
	"%s -h\n"\
	"%s -V\n"\
	"%s [options]\n\n"\
	"Options:\n"

#define STR_EXAMP_TEXT	\
	"Examples:\n\n"\
	"%s\n"\
	"%s -d -p mypassword\n"\
	"%s -d -i 192.168.1.2 -r 1024 -p mypassword\n"\
	"%s -r 1024 -p mypassword\n"\
	"%s -i \"*\" -r 1024 -p mypassword -l DEBUG -d -s -u myuser:mygroup -c \"/path/to/command/command.sh\"\n\n"


#define LOG_MESSAGE_LOCALE					"%s:\t%s PID [%d] - Locale: %s\n"


#define LOG_MESSAGE_PREAMBLE				"%s:\t%s PID [%d] - %s\n"
#define LOG_MESSAGE_PREAMBLE_STR_IP			"%s:\t%s PID [%d] - Success creating socket and binding to IPV%u: \"%s:%u\".\n"
#define LOG_MESSAGE_PREAMBLE_STR_CLIENT_IP 	"%s:\t%s PID [%d] - New client connected with IPV%u: \"%s:%u\".\n"
#define LOG_PARAMETERS_PREAMBLE				"%s:\t%s PID [%d] - \
Parameters:\n\tDaemon:\t\t%s\n\tHost:\t\t\"%s\"\n\tPort:\t\t%d\n\tPass:\t\t\"%s\"\n\
\tSingle:\t\t%s\n\tUser:\t\t\"%s\" \
=> (%d:%d)\n\tCommand:\t\"%s\"\n\tConfig:\t\t\"%s\"\n\tLog:\t\t%s\n"


#define LOG_PARAMETER_PARSING				"%s:\t%s PID [%d] - Parsing \"%s\" option...\n"
#define LOG_PARAMETER_SUCCESS_PARSING		"%s:\t%s PID [%d] - Success parsing \"%s\" option.\n"

#define LOG_PARAMETER_PARSING_CFG			"%s:\t%s PID [%d] - Loading \"%s\" option from configuration file...\n"
#define LOG_PARAMETER_SUCCESS_PARSING_CFG	"%s:\t%s PID [%d] - Success loading \"%s\" option from configuration file.\n"
#define LOG_PARAMETER_FAILED_PARSING_CFG	"%s:\t%s PID [%d] - Failure loading \"%s\" option (not found or wrong type) from configuration file.\n"


#define LOG_DEFAULT_PARAMETERS_S			"%s:\t%s PID [%d] - Defaulting %s to \"%s\".\n"
#define LOG_DEFAULT_PARAMETERS_D			"%s:\t%s PID [%d] - Defaulting %s to %d.\n"
#define LOG_DEFAULT_PARAMETERS_B			"%s:\t%s PID [%d] - Defaulting %s to %s.\n"
#define LOG_CHILD_DIED						"%s:\t%s PID [%d] - Child process PID [%d] died.\n"
#define LOG_PROCESS_IDS						"%s:\t%s PID [%d] - Process IDS:\n\tUID:\t%d\n\tEUID:\t%d\n\tGID:\t%d\n\tEGID:\t%d\n"
#define LOG_EVENT_NO_COMMAND				"%s:\t%s PID [%d] - Event (No command defined):\n\t%s\n"
#define LOG_COMMAND_OUTPUT					"%s:\t%s PID [%d] - Command, line %d output:\n\t%s\n"

#define LOG_NON_OPTION_ARGUMENT				"%s:\t%s PID [%d] - Non option argument found: \"%s\".\n"

#define STR_PARENT							"parent"
#define STR_DAEMON							"daemon"
#define STR_PROCESSING_CHILD_FROM_DAEMON	"processing child from daemon"
#define STR_PROCESSING_CHILD_FROM_PARENT	"processing child from parent"



/*/////////////////////// Constants globales- main - END - //////////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////////////////*/



/*/////////////////////// Macros globales del programa - BEGIN - ////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////////////////*/


#define PROCESS_P_C(daemon, processor) \
(daemon) ? ((processor) ? STR_PROCESSING_CHILD_FROM_DAEMON : STR_DAEMON) : \
((processor) ? STR_PROCESSING_CHILD_FROM_PARENT : STR_PARENT)


/*/////////////////////// Macros globales del programa - END - //////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////////////////*/


bool g_daemonized;
bool g_processor;
int g_loglevelmask;


#endif /* _SERVER_H_INCLUDED_ */
