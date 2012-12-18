#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>

#include <sys/stat.h>

#include <strings.h>
#include <string.h>

#include <locale.h>

#include <errno.h>

#include <stdint.h>

#include <signal.h>
#include <sys/wait.h>

#include <libconfig.h>


#include "./../Common/globals.h"
#include "./../Common/getopt_ext/getoptext.h"
#include "Server.h"
#include "exiterror.h"
#include "dropprivileges.h"
#include "./../Common/network/network.h"
#include "processclient.h"


/* GDB command for fork debuggung: set follow-fork-mode child */


/*/////////////////////// Constants - main - BEGIN - ////////////////////////////////////*/
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


/*///////////////////////////////////////////////////////////////////////////////////////*/
/*///////////////////////// Constants - main - END - ////////////////////////////////////*/


/*/////////////////////// Global types - main - BEGIN - /////////////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////////////////*/

typedef void (*t_signal_handler) (int);

/*///////////////////////////////////////////////////////////////////////////////////////*/
/*///////////////////////// Global types - main - END - /////////////////////////////////*/


/*/////////////////// Global external variables - BEGIN - ///////////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////////////////*/

bool 						g_daemonized =			FALSE;

bool 						g_processor =			FALSE;

/*/////////////////// Global external variables - END - /////////////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////////////////*/


/*/////////////////////// Global variables - main - BEGIN - /////////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////////////////*/

static struct option_ext optsextarr[OPTIONS_COUNT] =
{
	{STR_VERSION, no_argument, NULL, STR_VERSION_SHORT, STR_VERSION_DESC},					/* Program version. */
	{STR_HELP, no_argument, NULL, STR_HELP_SHORT, STR_HELP_DESC},							/* Help. */
	{STR_DAEMON, no_argument, NULL, STR_DAEMON_SHORT, STR_DAEMON_DESC},		    			/* opt_daemon the server. */
	{STR_IP, required_argument, NULL, STR_IP_SHORT, STR_IP_DESC},							/* Host\IP to bind to. */
	{STR_PORT, required_argument, NULL, STR_PORT_SHORT, STR_PORT_DESC},						/* Port to listen. */
	{STR_PASSWORD, required_argument, NULL, STR_PASSWORD_SHORT, STR_PASSWORD_DESC},			/* opt_password. */
	{STR_SINGLE, no_argument, NULL, STR_SINGLE_SHORT, STR_SINGLE_DESC},   					/* do not accept simultaneous conexions. */
	{STR_USER, required_argument, NULL, STR_USER_SHORT, STR_USER_DESC},     				/* User:Group to drop to. */
	{STR_COMMAND, required_argument, NULL, STR_COMMAND_SHORT, STR_COMMAND_DESC},     		/* Command that process events. */
	{STR_CONFIG_FILE, required_argument, NULL, STR_CONFIG_FILE_SHORT, STR_CONFIG_FILE_DESC},/* Config. file. */
	{STR_LOG_LEVEL, required_argument, NULL, STR_LOG_LEVEL_SHORT, STR_LOG_LEVEL_DESC},		/* Log level. */
};

static bool 				opt_daemon =			FALSE;
static const char*			opt_IP =				NULL;
static uint16_t 			opt_port =				0;
static const char*			opt_password =			NULL;
static bool 				opt_single =			FALSE;
static const char*			opt_user =				NULL;
static const char*			opt_command =			NULL;
static const char*			opt_cfg_file_name =		NULL;
static TLogLevel			opt_log_level =			LL_DEBUG;

static bool					b_opt_daemon =			FALSE;
static bool					b_opt_IP =				FALSE;
static bool					b_opt_port =			FALSE;
static bool					b_opt_password =		FALSE;
static bool					b_opt_single =			FALSE;
static bool					b_opt_user =			FALSE;
static bool					b_opt_command =			FALSE;
static bool					b_opt_cfg_file_name =	FALSE;
static bool					b_opt_log_level =		FALSE;


static pid_t				parent_pid = 			0;
static pid_t				daemon_pid = 			0;
static pid_t				processor_pid = 		0;


static gid_t				user_gid =				0;
static uid_t				user_uid =				0;
static bool 				user_is_primary_group =	FALSE;

static const char*			actual_listen_IP =		NULL;

static struct t_IP_addresses_info
		IP_addresses =								{FALSE, NULL};

static int 					listening_socket_fd =	-1;
static int 					new_client_socket_fd =	-1;

static char*				client_host_name =		NULL;

static struct config_t		cfg_file_info;
static bool					config_info_loaded = 	FALSE;

static char*				host_from_cfg =			NULL;
static char*				password_from_cfg =		NULL;
static char*				user_from_cfg =			NULL;
static char*				command_from_cfg =		NULL;

/*///////////////////////////////////////////////////////////////////////////////////////*/
/*/////////////////////// Global variables - main - END - ///////////////////////////////*/


/*/////////////////////////////// Functions - main - BEGIN - ////////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////////////////*/

static void Clean_exit (int istatus, const int syserror,
						const bool network_error, const bool bshowerrormessage);     /* Termina el programa mostrando el error. */

static void Parse_cmdline (int argc, char * const argv[]);		                   /* Parsea la línea de comando. */

static void Load_config_file (const char* config_file_name);

static void Load_default_config();


static void Validate_str_parameter (const char *pcargs);

static bool Validate_int_parameter (const char *pcargs, long int* par);

static void Do_usage (const int istatus);											/* Procesa la opción "-h". */
static void Do_version();															/* Procesa la opción "-V". */

static void Do_daemon (bool daemon);					                    		/* Procesa la opción "-d". */
static void Do_host (const char *host);												/* Procesa la opción "-b". */
static void Do_port (const long int port);											/* Procesa la opción "-r". */
static void Do_password (const char *password);										/* Procesa la opción "-p". */
static void Do_single (bool single);												/* Procesa la opción "-m". */
static void Do_user_group (const char *user);										/* Procesa la opción "-u". */
static void Do_command (const char *command);										/* Procesa la opción "-c". */
static void Do_config_file (const char* config_file);								/* Procesa la opción "-f". */
static void Do_log_level (const char* loglevel);									/* Procesa la opción "-l". */
static void Do_int_log_level (const long int loglevel);

static inline bool Can_drop_root_privileges (bool root_privileges_needed);
static bool try_to_drop_root_privileges (bool root_privileges_droped, bool root_privileges_needed);

static void Catch_signals (t_signal_handler handler, bool catch_SIGCHLD);
static void Signal_handler (int sig);

/*/////////////////////////////// Functions - main - END - //////////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////////////////*/


/*

*/
int main (int argc, char *argv[])
{
	bool root_privileges_needed = TRUE;
	bool root_privileges_droped = FALSE;
	char str_IP[INET4_OR_6_ADDRSTRLEN + 1] = "";
	u_int16_t port = 0;
	bool is_IPV6;
	char* locale;


	errno = 0;

	/* Termination flag to FALSE. */
	Signal_to_terminate (FALSE);

	/* Set signal handlers. */
	Catch_signals (&Signal_handler, TRUE);

	/* Store the parent PID. */
	parent_pid = getpid();

	/* Set locale to the user locale. */
	locale = setlocale (LC_ALL, "");

	/* Set initial log level (can be changed from command line). */
	Set_actual_log_level (DEFAULT_LOG_LEVEL, FALSE);

#ifdef NDEBUG
	/* Not visible if initial log level is less than tll_debug. */
	Vlogit (tll_debug,
			LOG_MESSAGE_PREAMBLE,
			LOGLEVELNAME (tll_debug),
			PROCESS_P_C (g_daemonized, g_processor),
			getpid(), "Running RELEASE executable...");
#else
	/* Not visible if initial log level is less than tll_debug. */
	Vlogit (tll_debug,
			LOG_MESSAGE_PREAMBLE,
			LOGLEVELNAME (tll_debug),
			PROCESS_P_C (g_daemonized, g_processor),
			getpid(), "Running DEBUG executable...");
#endif


	/* ************************************************************************************** */
	if (locale == NULL)
		Messagewarning (ERROR_SETTING_LOCALE, 0, FALSE);
	else
	{
		locale = setlocale (LC_ALL, NULL);

		if (locale != NULL)
			Vlogit (tll_debug,
					LOG_MESSAGE_LOCALE,
					LOGLEVELNAME (tll_debug),
					PROCESS_P_C (g_daemonized, g_processor),
					getpid(), locale);
	}
	/* ************************************************************************************** */


	/* ************************************************************************************** */
	/*
	Process command line (execution ends here in case of any fatal error).
	*/
	Vlogit (tll_debug,
			LOG_MESSAGE_PREAMBLE,
			LOGLEVELNAME (tll_debug),
			PROCESS_P_C (g_daemonized, g_processor),
			getpid(), "Parsing command line...");

	Parse_cmdline (argc, argv);

	/* ************************************************** */
	/* If the program is signaled to terminate. */
	if (Is_signaled_to_terminate())
		Clean_exit (ERROR_INTERRUPTED, 0, FALSE, TRUE);
	/* ************************************************** */

	Vlogit (tll_debug,
			LOG_MESSAGE_PREAMBLE,
			LOGLEVELNAME (tll_debug),
			PROCESS_P_C (g_daemonized, g_processor),
			getpid(), "Success parsing command line.");


	Load_config_file (opt_cfg_file_name);

	/* ************************************************** */
	/* If the program is signaled to terminate. */
	if (Is_signaled_to_terminate())
		Clean_exit (ERROR_INTERRUPTED, 0, FALSE, TRUE);
	/* ************************************************** */

	Load_default_config();

	/* ************************************************** */
	/* If the program is signaled to terminate. */
	if (Is_signaled_to_terminate())
		Clean_exit (ERROR_INTERRUPTED, 0, FALSE, TRUE);
	/* ************************************************** */

	Vlogit (tll_debug,
			LOG_PARAMETERS_PREAMBLE,
			LOGLEVELNAME (tll_debug),
			PROCESS_P_C (g_daemonized, g_processor),
			getpid(), BOOL_TO_STRING (opt_daemon), opt_IP,
			opt_port,
			SHOW_DEBUG_VAL (STR_ASTERISKS_PASS, opt_password),
			BOOL_TO_STRING (opt_single),
			(opt_user != NULL) ? opt_user : EMPTY_STRING,
			(opt_user != NULL) ? (int) user_uid : -1,
			(opt_user != NULL) ? (int) user_gid : -1,
			(opt_command != NULL) ? opt_command : EMPTY_STRING,
			(opt_cfg_file_name != NULL) ? opt_cfg_file_name : EMPTY_STRING,
			LOGLEVELNAME (opt_log_level));

	Vlogit (tll_debug,
			LOG_PROCESS_IDS,
			LOGLEVELNAME (tll_debug),
			PROCESS_P_C (g_daemonized, g_processor),
			getpid(),
			getuid(), geteuid(), getgid(), getegid());
	/* ************************************************************************************** */


	/* ************************************************** */
	/* If the program is signaled to terminate. */
	if (Is_signaled_to_terminate())
		Clean_exit (ERROR_INTERRUPTED, 0, FALSE, TRUE);
	/* ************************************************** */


	/* ************************************************************************************** */
	/* Set signal handlers. */
	Vlogit (tll_debug,
			LOG_MESSAGE_PREAMBLE,
			LOGLEVELNAME (tll_debug),
			PROCESS_P_C (g_daemonized, g_processor),
			getpid(), "Setting up signal handlers...");

	/* Set signal handlers. */
	Catch_signals (&Signal_handler, (opt_daemon || !opt_single));

	Vlogit (tll_debug,
			LOG_MESSAGE_PREAMBLE,
			LOGLEVELNAME (tll_debug),
			PROCESS_P_C (g_daemonized, g_processor),
			getpid(), "Success setting up signal handlers.");
	/* ************************************************************************************** */


	/* ************************************************** */
	/* If the program is signaled to terminate. */
	if (Is_signaled_to_terminate())
		Clean_exit (ERROR_INTERRUPTED, 0, FALSE, TRUE);
	/* ************************************************** */


	/* ************************************************************************************** */
	root_privileges_droped =
		try_to_drop_root_privileges (root_privileges_droped, root_privileges_needed);

	if (!root_privileges_droped && Do_i_have_root_privileges())
	{
		Messagewarning (ERROR_DROPPING_ROOT_PRIVILEGES, 0, FALSE);
	}
	/* ************************************************************************************** */


	/* ************************************************** */
	/* If the program is signaled to terminate. */
	if (Is_signaled_to_terminate())
		Clean_exit (ERROR_INTERRUPTED, 0, FALSE, TRUE);
	/* ************************************************** */


	/* ************************************************************************************** */
	/* Change the current file mask */
	Vlogit (tll_debug,
			LOG_MESSAGE_PREAMBLE,
			LOGLEVELNAME (tll_debug),
			PROCESS_P_C (g_daemonized, g_processor),
			getpid(), "Changing file mask...");

	umask (FILE_MASK);

	Vlogit (tll_debug,
			LOG_MESSAGE_PREAMBLE,
			LOGLEVELNAME (tll_debug),
			PROCESS_P_C (g_daemonized, g_processor),
			getpid(), "Success changing file mask.");
	/* ************************************************************************************** */


	/* ************************************************** */
	/* If the program is signaled to terminate. */
	if (Is_signaled_to_terminate())
		Clean_exit (ERROR_INTERRUPTED, 0, FALSE, TRUE);
	/* ************************************************** */


	/* ************************************************************************************** */
	/* Change the current working directory */
	Vlogit (tll_debug,
			LOG_MESSAGE_PREAMBLE,
			LOGLEVELNAME (tll_debug),
			PROCESS_P_C (g_daemonized, g_processor),
			getpid(), "Changing directory...");

	errno = 0;

	if ( (chdir (WORKING_DIR)) < 0)
	{
		Clean_exit (ERROR_SETTING_WORKING_DIRECTORY, errno, FALSE, TRUE);
	}

	Vlogit (tll_debug,
			LOG_MESSAGE_PREAMBLE,
			LOGLEVELNAME (tll_debug),
			PROCESS_P_C (g_daemonized, g_processor),
			getpid(), "Success changing directory.");
	/* ************************************************************************************** */


	/* ************************************************** */
	/* If the program is signaled to terminate. */
	if (Is_signaled_to_terminate())
		Clean_exit (ERROR_INTERRUPTED, 0, FALSE, TRUE);
	/* ************************************************** */


	/* opt_daemon if requested. */
	if (opt_daemon)
	{
		/* Our process ID and Session ID */
		pid_t sid;

		/* ************************************************************************************** */
		Vlogit (tll_debug,
				LOG_MESSAGE_PREAMBLE,
				LOGLEVELNAME (tll_debug),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), "Forking...");

		errno = 0;

		/* Fork off the parent process */
		daemon_pid = fork();

		if (daemon_pid < 0)
		{
			Clean_exit (ERROR_FORKING, errno, FALSE, TRUE);
		}
		/* If we got a good PID, then
		   we can exit the parent process. */
		if (daemon_pid > 0)
		{
			Vlogit (tll_info,
					LOG_MESSAGE_PREAMBLE,
					LOGLEVELNAME (tll_info),
					PROCESS_P_C (g_daemonized, g_processor),
					getpid(), "Success forking, nothing more to do...");

			Clean_exit (ERROR_SUCESS, 0, FALSE, FALSE);
		}

		/* Child running daemonized */
		g_daemonized = TRUE;

		/* ************************************************************************************** */
		/* Set signal handlers. */
		Vlogit (tll_debug,
				LOG_MESSAGE_PREAMBLE,
				LOGLEVELNAME (tll_debug),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), "Setting up signal handlers...");

		/* Set signal handlers. */
		Catch_signals (&Signal_handler, !opt_single);

		Vlogit (tll_debug,
				LOG_MESSAGE_PREAMBLE,
				LOGLEVELNAME (tll_debug),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), "Success setting up signal handlers.");
		/* ************************************************************************************** */

		/* ************************************************** */
		/* If the program is signaled to terminate. */
		if (Is_signaled_to_terminate())
			Clean_exit (ERROR_INTERRUPTED, 0, FALSE, TRUE);
		/* ************************************************** */


		/* ************************************************************************************** */
		/* Set signal handlers. */
		Vlogit (tll_debug,
				LOG_MESSAGE_PREAMBLE,
				LOGLEVELNAME (tll_debug),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), "Connecting to syslog...");

		Start_log();
		Set_actual_log_level (opt_log_level, TRUE);

		Vlogit (tll_debug,
				LOG_MESSAGE_PREAMBLE,
				LOGLEVELNAME (tll_debug),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), "Success connecting to syslog.");
		/* ************************************************************************************** */


		/* ************************************************************************************** */
		Vlogit (tll_debug,
				LOG_MESSAGE_PREAMBLE,
				LOGLEVELNAME (tll_debug),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), "Success forking, continue.");
		/* ************************************************************************************** */


		/* ************************************************************************************** */
		/* Create a new SID for the child process */
		Vlogit (tll_debug,
				LOG_MESSAGE_PREAMBLE,
				LOGLEVELNAME (tll_debug),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), "Creating new session...");

		errno = 0;

		sid = setsid();
		if (sid < 0)
		{
			/* Log the failure */
			Clean_exit (ERROR_CREATING_NEW_SID, errno, FALSE, TRUE);
		}

		Vlogit (tll_debug,
				LOG_MESSAGE_PREAMBLE,
				LOGLEVELNAME (tll_debug),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), "Success creating new session.");
		/* ************************************************************************************** */

		/* ************************************************** */
		/* If the program is signaled to terminate. */
		if (Is_signaled_to_terminate())
			Clean_exit (ERROR_INTERRUPTED, 0, FALSE, TRUE);
		/* ************************************************** */

		/* ************************************************************************************** */
		/* Close out the standard file descriptors */
		Vlogit (tll_debug,
				LOG_MESSAGE_PREAMBLE,
				LOGLEVELNAME (tll_debug),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), "Closing standard file descriptors...");

		errno = 0;

		if ( (close (STDIN_FILENO) != 0) ||
				(close (STDOUT_FILENO) != 0) ||
				(close (STDERR_FILENO) != 0))
		{
			Clean_exit (ERROR_CLOSING_STANDARD_FILE_DESCRIPTORS, errno, FALSE, TRUE);
		}

		Vlogit (tll_debug,
				LOG_MESSAGE_PREAMBLE,
				LOGLEVELNAME (tll_debug),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), "Success closing standard file descriptors.");
		/* ************************************************************************************** */
	}


	/* ************************************************** */
	/* If the program is signaled to terminate. */
	if (Is_signaled_to_terminate())
		Clean_exit (ERROR_INTERRUPTED, 0, FALSE, TRUE);
	/* ************************************************** */


	/* ************************************************************************************** */
	/* Bind to Host\IP. */
	Vlogit (tll_debug,
			LOG_MESSAGE_PREAMBLE,
			LOGLEVELNAME (tll_debug),
			PROCESS_P_C (g_daemonized, g_processor),
			getpid(), "Creating socket and binding to IP...");

	errno = 0;

	if (!Init_Socket_And_Bind (&IP_addresses, &listening_socket_fd, &is_IPV6, str_IP, INET4_OR_6_ADDRSTRLEN, &port))
		Clean_exit (ERROR_BINDING_TO_IP_ADDRESS, errno, FALSE, TRUE);

	Free_IP_addresses_info (&IP_addresses);

	root_privileges_needed = FALSE;

	if (strlen (str_IP) > 0)
	{
		Vlogit (tll_debug,
				LOG_MESSAGE_PREAMBLE_STR_IP,
				LOGLEVELNAME (tll_debug),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), is_IPV6 ? 6 : 4, str_IP, port);
	}
	else
	{
		Vlogit (tll_info,
				LOG_MESSAGE_PREAMBLE,
				LOGLEVELNAME (tll_info),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), "Success creating socket and binding to IP.");
	}
	/* ************************************************************************************** */


	/* ************************************************************************************** */
	/* Set socket timeouts. */
	Vlogit (tll_debug,
			LOG_MESSAGE_PREAMBLE,
			LOGLEVELNAME (tll_debug),
			PROCESS_P_C (g_daemonized, g_processor),
			getpid(), "Setting socket options...");

	if (!Set_socket_options (listening_socket_fd))
	{
		/* failed to set socket timeouts. */
		Clean_exit (ERROR_SETTING_SOCKET_OPTIONS, errno, FALSE, TRUE);
	}

	Vlogit (tll_debug,
			LOG_MESSAGE_PREAMBLE,
			LOGLEVELNAME (tll_debug),
			PROCESS_P_C (g_daemonized, g_processor),
			getpid(), "Success setting options.");
	/* ************************************************************************************** */


	/* ************************************************** */
	/* If the program is signaled to terminate. */
	if (Is_signaled_to_terminate())
		Clean_exit (ERROR_INTERRUPTED, 0, FALSE, TRUE);
	/* ************************************************** */

	/* ************************************************************************************** */
	root_privileges_droped =
		try_to_drop_root_privileges (root_privileges_droped, root_privileges_needed);
	/* ************************************************************************************** */


	/* ************************************************** */
	/* If the program is signaled to terminate. */
	if (Is_signaled_to_terminate())
		Clean_exit (ERROR_INTERRUPTED, 0, FALSE, TRUE);
	/* ************************************************** */


	/* ************************************************************************************** */
	/* Listen for connections. */
	Vlogit (tll_info,
			LOG_MESSAGE_PREAMBLE,
			LOGLEVELNAME (tll_info),
			PROCESS_P_C (g_daemonized, g_processor),
			getpid(), "Starting to listen for connections...");

	if (!Start_listenning (listening_socket_fd, opt_single ? 1 : MAX_CLIENT_CONNECTIONS))
		Clean_exit (ERROR_LISTENING, errno, FALSE, TRUE);

	Vlogit (tll_debug,
			LOG_MESSAGE_PREAMBLE,
			LOGLEVELNAME (tll_debug),
			PROCESS_P_C (g_daemonized, g_processor),
			getpid(), "Success starting to listen for connections.");

	/* ************************************************************************************** */


	/* ************************************************** */
	/* If the program is signaled to terminate. */
	if (Is_signaled_to_terminate())
		Clean_exit (ERROR_INTERRUPTED, 0, FALSE, TRUE);
	/* ************************************************** */


	/* While the program is not signaled to terminate. */
	while (!Is_signaled_to_terminate())
	{
		int res;
		bool success_accept, retry_accept;


		/* ************************************************************************************** */
		/* Accept new connection. */
		do
		{
			Vlogit (tll_info,
					LOG_MESSAGE_PREAMBLE,
					LOGLEVELNAME (tll_info),
					PROCESS_P_C (g_daemonized, g_processor),
					getpid(), "Waiting for client connection...");

			success_accept = Accept_new_client (listening_socket_fd, &new_client_socket_fd, &is_IPV6,
												str_IP, INET4_OR_6_ADDRSTRLEN, &client_host_name, &port);

			/* accept() call was break by a signal handler. */
			retry_accept = 	!success_accept && (errno == EINTR) && !Is_signaled_to_terminate();
		}
		while (retry_accept);

		/* ************************************************** */
		/* If the program is signaled to terminate. */
		if (Is_signaled_to_terminate())
			Clean_exit (ERROR_INTERRUPTED, 0, FALSE, TRUE);
		/* ************************************************** */

		if (!success_accept)
			Clean_exit (ERROR_ACCEPTING_NEW_CONNECTION, errno, FALSE, TRUE);

		if (strlen (str_IP) > 0)
		{
			Vlogit (tll_info,
					LOG_MESSAGE_PREAMBLE_STR_CLIENT_IP,
					LOGLEVELNAME (tll_info),
					PROCESS_P_C (g_daemonized, g_processor),
					getpid(), is_IPV6 ? 6 : 4, str_IP, port);
		}
		else
		{
			Vlogit (tll_info,
					LOG_MESSAGE_PREAMBLE,
					LOGLEVELNAME (tll_info),
					PROCESS_P_C (g_daemonized, g_processor),
					getpid(), "New client connected.");
		}
		/* ************************************************************************************** */


		/* fork if we accept multiple simultaneous conections. */
		if (!opt_single)
		{
			pid_t proc_sid;

			/* ************************************************************************************** */
			Vlogit (tll_debug,
					LOG_MESSAGE_PREAMBLE,
					LOGLEVELNAME (tll_debug),
					PROCESS_P_C (g_daemonized, g_processor),
					getpid(), "Forking for new client...");

			/* Create child process */
			processor_pid = fork();

			if (processor_pid < 0)
			{
				/*Fork failed. */

				errno = 0;

				if (close (new_client_socket_fd) != 0)
				{
					/* Failed to close the socket. */
					Messagewarning (ERROR_CLOSING_CLIENT_SOCKET, errno, FALSE);
				}
				else
					new_client_socket_fd = -1;

				/* Fail to fork for a new client. */
				Clean_exit (ERROR_FORKING_FOR_NEW_CLIENT, errno, FALSE, TRUE);
			}

			if (processor_pid > 0)
			{
				/* This is the parent process */

				Vlogit (tll_debug,
						LOG_MESSAGE_PREAMBLE,
						LOGLEVELNAME (tll_debug),
						PROCESS_P_C (g_daemonized, g_processor),
						getpid(), "Success forking for new client, continue listening...");

				errno = 0;

				if (close (new_client_socket_fd) != 0)
				{
					/* Fail to close the socket does not terminate the program. */
					Messagewarning (ERROR_CLOSING_CLIENT_SOCKET, errno, FALSE);
				}
				else
					new_client_socket_fd = -1;

				continue;
			}

			/* We got a new process for the new connection. */
			g_processor = TRUE;

			/* ************************************************************************************** */
			/* Set signal handlers. */
			Vlogit (tll_debug,
					LOG_MESSAGE_PREAMBLE,
					LOGLEVELNAME (tll_debug),
					PROCESS_P_C (g_daemonized, g_processor),
					getpid(), "Setting up signal handlers...");

			/* Set signal handlers. */
			Catch_signals (&Signal_handler, FALSE);

			Vlogit (tll_debug,
					LOG_MESSAGE_PREAMBLE,
					LOGLEVELNAME (tll_debug),
					PROCESS_P_C (g_daemonized, g_processor),
					getpid(), "Success setting up signal handlers.");
			/* ************************************************************************************** */

			/* ************************************************** */
			/* If the program is signaled to terminate. */
			if (Is_signaled_to_terminate())
				Clean_exit (ERROR_INTERRUPTED, 0, FALSE, TRUE);
			/* ************************************************** */

			Vlogit (tll_debug,
					LOG_MESSAGE_PREAMBLE,
					LOGLEVELNAME (tll_debug),
					PROCESS_P_C (g_daemonized, g_processor),
					getpid(), "Success forking for new client, processing client.");
			/* ************************************************************************************** */

			/* ************************************************************************************** */
			/* Create a new SID for the processing child process */
			Vlogit (tll_debug,
					LOG_MESSAGE_PREAMBLE,
					LOGLEVELNAME (tll_debug),
					PROCESS_P_C (g_daemonized, g_processor),
					getpid(), "Creating new session...");

			errno = 0;

			proc_sid = setsid();

			if (proc_sid < 0)
			{
				/* Log the failure */
				Clean_exit (ERROR_CREATING_NEW_SID, errno, FALSE, TRUE);
			}

			Vlogit (tll_debug,
					LOG_MESSAGE_PREAMBLE,
					LOGLEVELNAME (tll_debug),
					PROCESS_P_C (g_daemonized, g_processor),
					getpid(), "Success creating new session.");
			/* ************************************************************************************** */

			/* If the parent was not demonized, close the standard descriptors. */
			if (!g_daemonized)
			{
				/* ************************************************************************************** */
				/* Close out the standard file descriptors */
				Vlogit (tll_debug,
						LOG_MESSAGE_PREAMBLE,
						LOGLEVELNAME (tll_debug),
						PROCESS_P_C (g_daemonized, g_processor),
						getpid(), "Closing standard file descriptors...");

				errno = 0;

				if ( (close (STDIN_FILENO) != 0) ||
						(close (STDOUT_FILENO) != 0) ||
						(close (STDERR_FILENO) != 0))
				{
					Clean_exit (ERROR_CLOSING_STANDARD_FILE_DESCRIPTORS, errno, FALSE, TRUE);
				}

				Vlogit (tll_debug,
						LOG_MESSAGE_PREAMBLE,
						LOGLEVELNAME (tll_debug),
						PROCESS_P_C (g_daemonized, g_processor),
						getpid(), "Success closing standard file descriptors.");
				/* ************************************************************************************** */
			}
		}

		/* ************************************************** */
		/* If the program is signaled to terminate. */
		if (Is_signaled_to_terminate())
			Clean_exit (ERROR_INTERRUPTED, 0, FALSE, TRUE);
		/* ************************************************** */

		/* ************************************************************************************** */
		if (g_processor)
		{
			Vlogit (tll_debug,
					LOG_MESSAGE_PREAMBLE,
					LOGLEVELNAME (tll_debug),
					PROCESS_P_C (g_daemonized, g_processor),
					getpid(), "Closing the listening socket...");

			errno = 0;

			if (close (listening_socket_fd) != 0)
			{
				/* Failed to close the socket. */
				Messagewarning (ERROR_CLOSING_LISTENING_SOCKET, errno, FALSE);

			}
			else
			{
				listening_socket_fd = -1;

				Vlogit (tll_debug,
						LOG_MESSAGE_PREAMBLE,
						LOGLEVELNAME (tll_debug),
						PROCESS_P_C (g_daemonized, g_processor),
						getpid(), "Success closing the listening socket.");
			}

		}
		/* ************************************************************************************** */


		/* ************************************************** */
		/* If the program is signaled to terminate. */
		if (Is_signaled_to_terminate())
			Clean_exit (ERROR_INTERRUPTED, 0, FALSE, TRUE);
		/* ************************************************** */


		/* ************************************************************************************** */
		/* Set socket timeouts. */
		Vlogit (tll_debug,
				LOG_MESSAGE_PREAMBLE,
				LOGLEVELNAME (tll_debug),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), "Setting socket time-outs...");

		if (!Set_socket_timeouts (new_client_socket_fd, DEFAULT_TIME_OUT, DEFAULT_TIME_OUT))
		{
			/* failed to set socket timeouts. */
			Clean_exit (ERROR_SETTING_SOCKET_TIME_OUTS, errno, FALSE, TRUE);
		}

		Vlogit (tll_debug,
				LOG_MESSAGE_PREAMBLE,
				LOGLEVELNAME (tll_debug),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), "Success setting socket time-outs.");
		/* ************************************************************************************** */


		/* ************************************************** */
		/* If the program is signaled to terminate. */
		if (Is_signaled_to_terminate())
			Clean_exit (ERROR_INTERRUPTED, 0, FALSE, TRUE);
		/* ************************************************** */


		/* ************************************************************************************** */
		Vlogit (tll_info,
				LOG_MESSAGE_PREAMBLE,
				LOGLEVELNAME (tll_info),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), "Processing client requests...");

		res = Process_new_client_connection (new_client_socket_fd, client_host_name, str_IP, port,
											 (opt_password != NULL) ? opt_password : EMPTY_STRING, opt_command);

		Free_string (&client_host_name);

		if (close (new_client_socket_fd) != 0)
		{
			/* Failed to close the socket. */
			Messagewarning (ERROR_CLOSING_CLIENT_SOCKET, errno, FALSE);
		}
		else
			new_client_socket_fd = -1;

		if (res != ERROR_SUCESS)
		{
			if (g_processor)
				Clean_exit (res, errno, FALSE, TRUE);
			else
			{
				if (res == ERROR_INTERRUPTED)
					Clean_exit (ERROR_INTERRUPTED, 0, FALSE, TRUE);
				else
				{
					Messageerror (res, errno, FALSE);
					continue;
				}
			}
		}
		else
		{
			Vlogit (tll_info,
					LOG_MESSAGE_PREAMBLE,
					LOGLEVELNAME (tll_info),
					PROCESS_P_C (g_daemonized, g_processor),
					getpid(), "Success processing client.");

			if (g_processor)
			{
				break;
			}
			else
				continue;
		}
		/* ************************************************************************************** */

	} /* end of while */


	/* ************************************************** */
	/* If the program is signaled to terminate. */
	if (Is_signaled_to_terminate())
		Clean_exit (ERROR_INTERRUPTED, 0, FALSE, TRUE);
	/* ************************************************** */


	Clean_exit (ERROR_SUCESS, 0, FALSE, FALSE);

	return ERROR_SUCESS;
}


/*
Muestra por stderr un mensaje de status (error) y termina el programa devolviendo
un valor correspondiente al status de salida.

1er parámetro: Status de salida.

2do parámetro: Indica si se debe o no mostrar el texto del error.

devuelve: -.
*/
static void Clean_exit (int istatus, const int syserror,
						const bool network_error, const bool bshowerrormessage)
{
	if (bshowerrormessage)
		Messageerror (istatus, syserror,
					  network_error);

	if (listening_socket_fd != -1)
	{
		close (listening_socket_fd);

		listening_socket_fd = -1;
	}

	if (new_client_socket_fd != -1)
	{
		close (new_client_socket_fd);

		new_client_socket_fd = -1;
	}

	Free_string (&client_host_name);

	Free_string (&host_from_cfg);

	Free_string (&password_from_cfg);

	Free_string (&user_from_cfg);

	Free_string (&command_from_cfg);

	if (config_info_loaded)
	{
		config_destroy (&cfg_file_info);

		config_info_loaded = FALSE;
	}

	Free_IP_addresses_info (&IP_addresses); /* Free IP_addresses. */

	Vlogit (tll_info,
			LOG_MESSAGE_PREAMBLE,
			LOGLEVELNAME (tll_info),
			PROCESS_P_C (g_daemonized, g_processor),
			getpid(), "Exiting process...");

	End_log();                              /* Close syslog conection. */

	if (istatus == ERROR_INTERRUPTED)
		istatus = ERROR_SUCESS;

	if (getpid() != parent_pid)
		_exit (istatus);
	else
		exit (istatus);

}


/*
Procesa la línea de comandos.

1er parámetro: Cantidad de argumentos del programa.

2do parámetro: Puntero a los argumentos.


devuelve: -. (La función puede terminar el programa)
*/
static void Parse_cmdline (int argc, char * const argv[])
{
	int ch;                                             /* Valor devuelto por la función getopt_long. */
	int array_opt_index = 0;
	struct option optionarr[OPTIONS_COUNT + 1];
	char short_opt_str[OPTIONS_COUNT*2 + 2];

	long int par_int;


	memset (&optionarr, 0x00, sizeof (optionarr));

	bzero (short_opt_str, sizeof (short_opt_str));


	Fill_getopt_long_options (optsextarr, optionarr, OPTIONS_COUNT);

	Generate_short_options_string (short_opt_str, optsextarr, OPTIONS_COUNT, TRUE);

	/*
	Llama repetidamente a la función getopt_long (devuelve el caracter que
	representa a la opción o EOF) hasta procesar todos
	los argumentos.
	*/
	opterr = 0;							/* getopt_long doesn't print errors. */
	optind = 0;
	array_opt_index = 0;				/* Inicializo array_opt_index (indice de opcion). */

	while ( ( (ch = getopt_long (argc, argv, short_opt_str,
								 optionarr, &array_opt_index)) != EOF) &&
			(!Is_signaled_to_terminate()))
	{
		option_with_error = argv[optind - 1 - ( (optarg != NULL) ? 1 : 0) ];

		Vlogit (tll_debug,
				LOG_PARAMETER_PARSING,
				LOGLEVELNAME (tll_debug),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), option_with_error);

		switch (ch)
		{
		case STR_VERSION_SHORT:

			if ( (optind > 1) || (argc > 2))
				Messagewarning (ERROR_OPTION_VERSION_NOT_ALONE, 0, TRUE);

			Do_version (argv[0]);
			break;
		case STR_HELP_SHORT:

			if ( (optind > 1) || (argc > 2))
				Messagewarning (ERROR_OPTION_HELP_NOT_ALONE, 0, TRUE);

			Do_usage (ERROR_SUCESS);
			break;
		case STR_DAEMON_SHORT:

			Do_daemon (TRUE);
			break;
		case STR_IP_SHORT:

			Validate_str_parameter (optarg);

			Do_host (optarg);
			break;
		case STR_PORT_SHORT:

			if (!Validate_int_parameter (optarg, &par_int))
				Clean_exit (ERROR_BAD_PORT_NUMBER, errno, FALSE, TRUE);

			Do_port (par_int);
			break;
		case STR_PASSWORD_SHORT:

			Validate_str_parameter (optarg);

			Do_password (optarg);
			break;
		case STR_SINGLE_SHORT:

			Do_single (TRUE);
			break;
		case STR_USER_SHORT:

			Validate_str_parameter (optarg);

			Do_user_group (optarg);
			break;
		case STR_COMMAND_SHORT:

			Validate_str_parameter (optarg);

			Do_command (optarg);
			break;
		case STR_CONFIG_FILE_SHORT:

			Validate_str_parameter (optarg);

			Do_config_file (optarg);
			break;
		case STR_LOG_LEVEL_SHORT:

			Validate_str_parameter (optarg);

			Do_log_level (optarg);
			break;
		case '?':

			Do_usage (ERROR_INVALID_OPTION);
		case ':':

			Do_usage (ERROR_MISSING_REQUIRED_PARAMETER);
		default:
			Do_usage (ERROR_INVALID_COMMAND_LINE);
		}

		Vlogit (tll_debug,
				LOG_PARAMETER_SUCCESS_PARSING,
				LOGLEVELNAME (tll_debug),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), option_with_error);
	}

	/* ************************************************** */
	/* If the program is signaled to terminate. */
	if (Is_signaled_to_terminate())
		Clean_exit (ERROR_INTERRUPTED, 0, FALSE, TRUE);
	/* ************************************************** */

	if (optind < argc)
	{
		/* Process non option arguments. */

		while (optind < argc)
		{
			Vlogit (tll_debug,
					LOG_NON_OPTION_ARGUMENT,
					LOGLEVELNAME (tll_debug),
					PROCESS_P_C (g_daemonized, g_processor),
					getpid(), argv[optind]);

			optind++;
		}

		Clean_exit (ERROR_NON_OPTION_ARGUMENT_FOUND, 0, FALSE, TRUE);
	}
}

static void Load_config_file (const char* config_file_name)
{
	const char* par_str;
	int par_int;
	size_t len;

	if (!b_opt_cfg_file_name ||
			(b_opt_daemon && b_opt_IP && b_opt_port && b_opt_password &&
			 b_opt_single && b_opt_user && b_opt_command && b_opt_log_level))
		return;

	Vlogit (tll_debug,
			LOG_MESSAGE_PREAMBLE,
			LOGLEVELNAME (tll_debug),
			PROCESS_P_C (g_daemonized, g_processor),
			getpid(), "Loading config file...");

	memset (&cfg_file_info, 0x00, sizeof (cfg_file_info));

	/*Initialization */
	config_init (&cfg_file_info);

	config_info_loaded = TRUE;

	/* Read the file. If there is an error, report it and exit. */
	if (config_read_file (&cfg_file_info, config_file_name) != CONFIG_TRUE)
	{
		config_error_t err;

		err = config_error_type (&cfg_file_info);

		config_file_line = config_error_line (&cfg_file_info);

		config_destroy (&cfg_file_info);

		config_info_loaded = FALSE;

		switch (err)
		{
		case CONFIG_ERR_FILE_IO:
			Clean_exit (ERROR_CONFIG_FILE_READING, 0, FALSE, TRUE);
			break;
		case CONFIG_ERR_PARSE:
			Clean_exit (ERROR_CONFIG_FILE_PARSING, 0, FALSE, TRUE);
			break;
		case CONFIG_ERR_NONE:
		default:
			Clean_exit (ERROR_CONFIG_FILE_UNKNOWN, 0, FALSE, TRUE);
			break;
		}
	}

	/* ************************************************** */
	/* If the program is signaled to terminate. */
	if (Is_signaled_to_terminate())
		Clean_exit (ERROR_INTERRUPTED, 0, FALSE, TRUE);
	/* ************************************************** */

	if (!b_opt_daemon)
	{
		Vlogit (tll_debug,
				LOG_PARAMETER_PARSING_CFG,
				LOGLEVELNAME (tll_debug),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), STR_DAEMON);

		if (config_lookup_bool (&cfg_file_info, STR_CONFIG_DAEMON, &par_int) == CONFIG_TRUE)
		{
			Do_daemon (par_int);

			Vlogit (tll_debug,
					LOG_PARAMETER_SUCCESS_PARSING_CFG,
					LOGLEVELNAME (tll_debug),
					PROCESS_P_C (g_daemonized, g_processor),
					getpid(), STR_DAEMON);
		}
		else
		{
			Vlogit (tll_debug,
					LOG_PARAMETER_FAILED_PARSING_CFG,
					LOGLEVELNAME (tll_debug),
					PROCESS_P_C (g_daemonized, g_processor),
					getpid(), STR_DAEMON);
		}
	}

	/* ************************************************** */
	/* If the program is signaled to terminate. */
	if (Is_signaled_to_terminate())
		Clean_exit (ERROR_INTERRUPTED, 0, FALSE, TRUE);
	/* ************************************************** */

	if (!b_opt_IP)
	{
		Vlogit (tll_debug,
				LOG_PARAMETER_PARSING_CFG,
				LOGLEVELNAME (tll_debug),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), STR_IP);

		if (config_lookup_string (&cfg_file_info, STR_CONFIG_IP, &par_str) == CONFIG_TRUE)
		{
			len = strlen (par_str);

			errno = 0;

			host_from_cfg = malloc (len + 1);

			if (host_from_cfg == NULL)
			{
				config_destroy (&cfg_file_info);

				config_info_loaded = FALSE;

				Clean_exit (ERROR_ALLOCATING_MEMORY, errno, FALSE, TRUE);
			}

			bzero (host_from_cfg, len + 1);

			strncpy (host_from_cfg, par_str, len);

			Do_host (host_from_cfg);

			Vlogit (tll_debug,
					LOG_PARAMETER_SUCCESS_PARSING_CFG,
					LOGLEVELNAME (tll_debug),
					PROCESS_P_C (g_daemonized, g_processor),
					getpid(), STR_IP);
		}
		else
		{
			Vlogit (tll_debug,
					LOG_PARAMETER_FAILED_PARSING_CFG,
					LOGLEVELNAME (tll_debug),
					PROCESS_P_C (g_daemonized, g_processor),
					getpid(), STR_IP);
		}
	}

	/* ************************************************** */
	/* If the program is signaled to terminate. */
	if (Is_signaled_to_terminate())
		Clean_exit (ERROR_INTERRUPTED, 0, FALSE, TRUE);
	/* ************************************************** */

	if (!b_opt_port)
	{
		Vlogit (tll_debug,
				LOG_PARAMETER_PARSING_CFG,
				LOGLEVELNAME (tll_debug),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), STR_PORT);

		if (config_lookup_int (&cfg_file_info, STR_CONFIG_PORT, &par_int) == CONFIG_TRUE)
		{
			Do_port (par_int);

			Vlogit (tll_debug,
					LOG_PARAMETER_SUCCESS_PARSING_CFG,
					LOGLEVELNAME (tll_debug),
					PROCESS_P_C (g_daemonized, g_processor),
					getpid(), STR_PORT);
		}
		else
		{
			Vlogit (tll_debug,
					LOG_PARAMETER_FAILED_PARSING_CFG,
					LOGLEVELNAME (tll_debug),
					PROCESS_P_C (g_daemonized, g_processor),
					getpid(), STR_PORT);
		}
	}

	/* ************************************************** */
	/* If the program is signaled to terminate. */
	if (Is_signaled_to_terminate())
		Clean_exit (ERROR_INTERRUPTED, 0, FALSE, TRUE);
	/* ************************************************** */

	if (!b_opt_password)
	{
		Vlogit (tll_debug,
				LOG_PARAMETER_PARSING_CFG,
				LOGLEVELNAME (tll_debug),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), STR_PASSWORD);

		if (config_lookup_string (&cfg_file_info, STR_CONFIG_PASSWORD, &par_str) == CONFIG_TRUE)
		{
			len = strlen (par_str);

			errno = 0;

			password_from_cfg = malloc (len + 1);

			if (password_from_cfg == NULL)
			{
				config_destroy (&cfg_file_info);

				config_info_loaded = FALSE;

				Clean_exit (ERROR_ALLOCATING_MEMORY, errno, FALSE, TRUE);
			}

			bzero (password_from_cfg, len + 1);

			strncpy (password_from_cfg, par_str, len);

			Do_password (password_from_cfg);

			Vlogit (tll_debug,
					LOG_PARAMETER_SUCCESS_PARSING_CFG,
					LOGLEVELNAME (tll_debug),
					PROCESS_P_C (g_daemonized, g_processor),
					getpid(), STR_PASSWORD);
		}
		else
		{
			Vlogit (tll_debug,
					LOG_PARAMETER_FAILED_PARSING_CFG,
					LOGLEVELNAME (tll_debug),
					PROCESS_P_C (g_daemonized, g_processor),
					getpid(), STR_PASSWORD);
		}
	}

	/* ************************************************** */
	/* If the program is signaled to terminate. */
	if (Is_signaled_to_terminate())
		Clean_exit (ERROR_INTERRUPTED, 0, FALSE, TRUE);
	/* ************************************************** */

	if (!b_opt_single)
	{
		Vlogit (tll_debug,
				LOG_PARAMETER_PARSING_CFG,
				LOGLEVELNAME (tll_debug),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), STR_SINGLE);

		if (config_lookup_bool (&cfg_file_info, STR_CONFIG_SINGLE, &par_int) == CONFIG_TRUE)
		{
			Do_single (par_int);

			Vlogit (tll_debug,
					LOG_PARAMETER_SUCCESS_PARSING_CFG,
					LOGLEVELNAME (tll_debug),
					PROCESS_P_C (g_daemonized, g_processor),
					getpid(), STR_SINGLE);
		}
		else
		{
			Vlogit (tll_debug,
					LOG_PARAMETER_FAILED_PARSING_CFG,
					LOGLEVELNAME (tll_debug),
					PROCESS_P_C (g_daemonized, g_processor),
					getpid(), STR_SINGLE);
		}
	}

	/* ************************************************** */
	/* If the program is signaled to terminate. */
	if (Is_signaled_to_terminate())
		Clean_exit (ERROR_INTERRUPTED, 0, FALSE, TRUE);
	/* ************************************************** */

	if (!b_opt_user)
	{
		Vlogit (tll_debug,
				LOG_PARAMETER_PARSING_CFG,
				LOGLEVELNAME (tll_debug),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), STR_USER);

		if (config_lookup_string (&cfg_file_info, STR_CONFIG_USER, &par_str) == CONFIG_TRUE)
		{
			len = strlen (par_str);

			errno = 0;

			user_from_cfg = malloc (len + 1);

			if (user_from_cfg == NULL)
			{
				config_destroy (&cfg_file_info);

				config_info_loaded = FALSE;

				Clean_exit (ERROR_ALLOCATING_MEMORY, errno, FALSE, TRUE);
			}

			bzero (user_from_cfg, len + 1);

			strncpy (user_from_cfg, par_str, len);

			Do_user_group (user_from_cfg);

			Vlogit (tll_debug,
					LOG_PARAMETER_SUCCESS_PARSING_CFG,
					LOGLEVELNAME (tll_debug),
					PROCESS_P_C (g_daemonized, g_processor),
					getpid(), STR_USER);
		}
		else
		{
			Vlogit (tll_debug,
					LOG_PARAMETER_FAILED_PARSING_CFG,
					LOGLEVELNAME (tll_debug),
					PROCESS_P_C (g_daemonized, g_processor),
					getpid(), STR_USER);
		}
	}

	/* ************************************************** */
	/* If the program is signaled to terminate. */
	if (Is_signaled_to_terminate())
		Clean_exit (ERROR_INTERRUPTED, 0, FALSE, TRUE);
	/* ************************************************** */

	if (!b_opt_command)
	{
		Vlogit (tll_debug,
				LOG_PARAMETER_PARSING_CFG,
				LOGLEVELNAME (tll_debug),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), STR_COMMAND);

		if (config_lookup_string (&cfg_file_info, STR_CONFIG_COMMAND, &par_str) == CONFIG_TRUE)
		{
			len = strlen (par_str);

			errno = 0;

			command_from_cfg = malloc (len + 1);

			if (command_from_cfg == NULL)
			{
				config_destroy (&cfg_file_info);

				config_info_loaded = FALSE;

				Clean_exit (ERROR_ALLOCATING_MEMORY, errno, FALSE, TRUE);
			}

			bzero (command_from_cfg, len + 1);

			strncpy (command_from_cfg, par_str, len);

			Do_command (command_from_cfg);

			Vlogit (tll_debug,
					LOG_PARAMETER_SUCCESS_PARSING_CFG,
					LOGLEVELNAME (tll_debug),
					PROCESS_P_C (g_daemonized, g_processor),
					getpid(), STR_COMMAND);
		}
		else
		{
			Vlogit (tll_debug,
					LOG_PARAMETER_FAILED_PARSING_CFG,
					LOGLEVELNAME (tll_debug),
					PROCESS_P_C (g_daemonized, g_processor),
					getpid(), STR_COMMAND);
		}
	}

	/* ************************************************** */
	/* If the program is signaled to terminate. */
	if (Is_signaled_to_terminate())
		Clean_exit (ERROR_INTERRUPTED, 0, FALSE, TRUE);
	/* ************************************************** */

	if (!b_opt_log_level)
	{
		Vlogit (tll_debug,
				LOG_PARAMETER_PARSING_CFG,
				LOGLEVELNAME (tll_debug),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), STR_LOG_LEVEL);

		if (config_lookup_string (&cfg_file_info, STR_CONFIG_LOG_LEVEL, &par_str) == CONFIG_TRUE)
		{
			Do_log_level (par_str);

			Vlogit (tll_debug,
					LOG_PARAMETER_SUCCESS_PARSING_CFG,
					LOGLEVELNAME (tll_debug),
					PROCESS_P_C (g_daemonized, g_processor),
					getpid(), STR_LOG_LEVEL);
		}
		else
		{
			Vlogit (tll_debug,
					LOG_PARAMETER_FAILED_PARSING_CFG,
					LOGLEVELNAME (tll_debug),
					PROCESS_P_C (g_daemonized, g_processor),
					getpid(), STR_LOG_LEVEL);
		}
	}

	/* ************************************************** */
	/* If the program is signaled to terminate. */
	if (Is_signaled_to_terminate())
		Clean_exit (ERROR_INTERRUPTED, 0, FALSE, TRUE);
	/* ************************************************** */

	if (!b_opt_log_level)
	{
		Vlogit (tll_debug,
				LOG_PARAMETER_PARSING_CFG,
				LOGLEVELNAME (tll_debug),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), STR_LOG_LEVEL);

		if (config_lookup_int (&cfg_file_info, STR_CONFIG_LOG_LEVEL, &par_int) == CONFIG_TRUE)
		{
			Do_int_log_level (par_int);

			Vlogit (tll_debug,
					LOG_PARAMETER_SUCCESS_PARSING_CFG,
					LOGLEVELNAME (tll_debug),
					PROCESS_P_C (g_daemonized, g_processor),
					getpid(), STR_LOG_LEVEL);
		}
		else
		{
			Vlogit (tll_debug,
					LOG_PARAMETER_FAILED_PARSING_CFG,
					LOGLEVELNAME (tll_debug),
					PROCESS_P_C (g_daemonized, g_processor),
					getpid(), STR_LOG_LEVEL);
		}
	}

	config_destroy (&cfg_file_info);

	config_info_loaded = FALSE;

	Vlogit (tll_debug,
			LOG_MESSAGE_PREAMBLE,
			LOGLEVELNAME (tll_debug),
			PROCESS_P_C (g_daemonized, g_processor),
			getpid(), "Success loading config file.");
}


/*

*/
static void Load_default_config()
{
	int network_syserror = 0;

	/* Set default values for missing arguments. */

	if (!b_opt_daemon)
	{
		opt_daemon = DEFAULT_DAEMON;

		Vlogit (tll_info,
				LOG_DEFAULT_PARAMETERS_B,
				LOGLEVELNAME (tll_info),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), STR_DAEMON, BOOL_TO_STRING (DEFAULT_DAEMON));
	}

	if (!b_opt_IP)
	{
		opt_IP = DEFAULT_IP;
		actual_listen_IP = DEFAULT_IP;

		Vlogit (tll_info,
				LOG_DEFAULT_PARAMETERS_S,
				LOGLEVELNAME (tll_info),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), STR_IP, DEFAULT_IP);
	}

	if (!b_opt_port)
	{
		opt_port = DEFAULT_PORT;

		Vlogit (tll_info,
				LOG_DEFAULT_PARAMETERS_D,
				LOGLEVELNAME (tll_info),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), STR_PORT, DEFAULT_PORT);
	}

	if (!b_opt_password)
	{
		opt_password = DEFAULT_PASSWORD;

		Vlogit (tll_info,
				LOG_DEFAULT_PARAMETERS_S,
				LOGLEVELNAME (tll_info),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), STR_PASSWORD, DEFAULT_PASSWORD);
	}

	if (!b_opt_single)
	{
		opt_single = DEFAULT_SINGLE;

		Vlogit (tll_info,
				LOG_DEFAULT_PARAMETERS_B,
				LOGLEVELNAME (tll_info),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), STR_SINGLE, BOOL_TO_STRING (DEFAULT_SINGLE));
	}

	if (!b_opt_user)
	{
		opt_user = DEFAULT_USER;
		Set_actual_log_level (opt_log_level, FALSE);

		Vlogit (tll_info,
				LOG_DEFAULT_PARAMETERS_S,
				LOGLEVELNAME (tll_info),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), STR_USER, DEFAULT_USER ? DEFAULT_USER : EMPTY_STRING);
	}

	if (!b_opt_command)
	{
		opt_command = DEFAULT_COMMAND;

		Vlogit (tll_info,
				LOG_DEFAULT_PARAMETERS_S,
				LOGLEVELNAME (tll_info),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), STR_COMMAND, DEFAULT_COMMAND ? DEFAULT_COMMAND : EMPTY_STRING);
	}

	if (!b_opt_cfg_file_name)
	{
		opt_cfg_file_name = DEFAULT_CONFIG_FILE_NAME;

		Vlogit (tll_info,
				LOG_DEFAULT_PARAMETERS_S,
				LOGLEVELNAME (tll_info),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), STR_CONFIG_FILE,
				DEFAULT_CONFIG_FILE_NAME ? DEFAULT_CONFIG_FILE_NAME : EMPTY_STRING);
	}

	if (!b_opt_log_level)
	{
		opt_log_level = DEFAULT_LOG_LEVEL;
		Set_actual_log_level (opt_log_level, FALSE);

		Vlogit (tll_info,
				LOG_DEFAULT_PARAMETERS_S,
				LOGLEVELNAME (tll_info),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), STR_LOG_LEVEL, LOGLEVELNAME (DEFAULT_LOG_LEVEL));
	}

	/* Validate default Host\IP and port number (when both parameters were missing). */

	if (!b_opt_IP && !b_opt_IP)
	{
		Vlogit (tll_debug,
				LOG_MESSAGE_PREAMBLE,
				LOGLEVELNAME (tll_debug),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), "Validating default IP address and port...");


		if (!Validate_And_Parse_Host_Or_IP (actual_listen_IP, opt_port,
											&IP_addresses, &network_syserror))
			Clean_exit (ERROR_NOT_VALID_DEFAULT_IP_PORT, network_syserror, TRUE, TRUE);

		Vlogit (tll_debug,
				LOG_MESSAGE_PREAMBLE,
				LOGLEVELNAME (tll_debug),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), "Success validating default IP address and port.");
	}
}


/*

*/
static void Validate_str_parameter (const char *pcargs)
{
	if (!pcargs || !strlen (pcargs))
		Clean_exit (ERROR_MISSING_REQUIRED_PARAMETER, 0, FALSE, TRUE);
}


/*

*/
static bool Validate_int_parameter (const char *pcargs, long int* par)
{
	if (!pcargs || !strlen (pcargs))
		Clean_exit (ERROR_MISSING_REQUIRED_PARAMETER, 0, FALSE, TRUE);

	return Convert_num_val (pcargs, par);
}


/*
Imprime la ayuda de la linea de comandos
para el programa.

1er parámetro: Puntero al nombre del ejecutable.

2do parámetro: Puntero al parámetro de la opcion.


devuelve: -.
*/
static void Do_usage (const int istatus)
{
	if (istatus != ERROR_SUCESS)
		Messageerror (istatus, 0, FALSE);

	fprintf (stderr, STR_USAGE_TEXT, STR_PROGRAM_NAME, STR_PROGRAM_NAME, STR_PROGRAM_NAME);

	Print_help (stderr, optsextarr, OPTIONS_COUNT);

	fprintf (stderr, STR_EXAMP_TEXT, STR_PROGRAM_NAME, STR_PROGRAM_NAME, STR_PROGRAM_NAME,
			 STR_PROGRAM_NAME, STR_PROGRAM_NAME);

	Clean_exit (istatus, 0, FALSE, FALSE);
}


/*
Imprime la version del programa.

1er parámetro: Puntero al nombre del ejecutable.


devuelve: -.
*/
static void Do_version()
{
	Vlogit (tll_debug,
			LOG_MESSAGE_PREAMBLE,
			LOGLEVELNAME (tll_debug),
			PROCESS_P_C (g_daemonized, g_processor),
			getpid(), "Parsing \"-V\" option...");

	fprintf (stderr, STR_VERSION_DESCRIPTION, STR_PROGRAM_INTERNAL_NAME,
			 VERSION_MAJOR, VERSION_MINOR, BUILD_NUMBER, VERSION_TYPE);

#ifdef __GNUC__
	fprintf (stderr, STR_COMPILED, GCC_STR_VER);
#endif

#ifdef BUILD_DATE_TIME
	fprintf (stderr, STR_COMPILED_DATE, BUILD_DATE_TIME);
#endif

#ifdef UNAME_PLATFORM
#if UNAME_PLATFORM > 0
	fprintf (stderr, STR_COMPILED_ON,
			 OS_UNAME,
			 KERNEL_UNAME, KERNEL_RELEASE_UNAME, KERNEL_VERSION_UNAME,
			 MACHINE_UNAME, PROCESSOR_UNAME, HARDWARE_UNAME);
#endif
#endif

	Clean_exit (ERROR_SUCESS, 0, FALSE, FALSE);
}


/*
Determina si el programa se demoniza.

1er parámetro: Puntero al nombre del ejecutable.


devuelve: -.
*/
static void Do_daemon (bool daemon)
{
	if (b_opt_daemon)                                /* Opción repetida. */
		Clean_exit (ERROR_ONLY_ONE_DAEMON, 0, FALSE, TRUE);

	b_opt_daemon = TRUE;

	opt_daemon = daemon;
}


/*
Determina el host\IP.

1er parámetro: Puntero al nombre del ejecutable.

2do parámetro: Puntero al parámetro de la opcion.


devuelve: -.
*/
static void Do_host (const char *host)
{
	int network_syserror;

	if (b_opt_IP)
		Clean_exit (ERROR_ONLY_ONE_IP, 0, FALSE, TRUE);

	b_opt_IP = TRUE;

	opt_IP = host;

	if (strcmp (host, WILDCARD_STRING) == 0)
		actual_listen_IP = NULL;
	else
		actual_listen_IP = host;

	if (!Validate_And_Parse_Host_Or_IP (actual_listen_IP, b_opt_port ? opt_port : DEFAULT_PORT,
										&IP_addresses, &network_syserror))
		Clean_exit (ERROR_NOT_VALID_IP, network_syserror, TRUE, TRUE);
}


/*
Determina el port.

1er parámetro: Puntero al nombre del ejecutable.

2do parámetro: Puntero al parámetro de la opcion.


devuelve: -.
*/
static void Do_port (const long int  port)
{
	int network_syserror;

	if (b_opt_port)
		Clean_exit (ERROR_ONLY_ONE_PORT, 0, FALSE, TRUE);

	b_opt_port = TRUE;

	/* Range check [MIN_VALID_PORT; MAX_VALID_PORT]. */
	if ( (port < MIN_VALID_PORT) || (port > MAX_VALID_PORT))
		Clean_exit (ERROR_PORT_NUMBER_OUT_OF_RANGE, 0, FALSE, TRUE);

	opt_port = port;

	if (!Validate_And_Parse_Host_Or_IP (b_opt_IP ? actual_listen_IP : DEFAULT_IP, opt_port,
										&IP_addresses, &network_syserror))
		Clean_exit (ERROR_NOT_VALID_IP, network_syserror, TRUE, TRUE);
}


/*
Determina el opt_password.

1er parámetro: Puntero al nombre del ejecutable.

2do parámetro: Puntero al parámetro de la opcion.


devuelve: -.
*/
static void Do_password (const char *password)
{
	if (b_opt_password)
		Clean_exit (ERROR_ONLY_ONE_PASSWORD, 0, FALSE, TRUE);

	b_opt_password = TRUE;

	opt_password = password;
}


/*
Determina el nivel de loggeo.

1er parámetro: Puntero al nombre del ejecutable.

2do parámetro: Puntero al parámetro de la opcion.


devuelve: -.
*/
static void Do_log_level (const char* loglevel)
{
	long int level;

	if (b_opt_log_level)
		Clean_exit (ERROR_ONLY_ONE_LOG_LEVEL, 0, FALSE, TRUE);

	b_opt_log_level = TRUE;

	if ( (strlen (loglevel) == strlen (STR_DEBUG)) && (strncasecmp (loglevel, STR_DEBUG, strlen (STR_DEBUG)) == 0))
	{
		level = LL_DEBUG;
	}
	else if ( (strlen (loglevel) == strlen (STR_INFO)) && (strncasecmp (loglevel, STR_INFO, strlen (STR_INFO)) == 0))
	{
		level = LL_INFO;
	}
	else if ( (strlen (loglevel) == strlen (STR_ERROR)) && (strncasecmp (loglevel, STR_ERROR, strlen (STR_ERROR)) == 0))
	{
		level = LL_ERROR;
	}
	else
	{
		if (!Convert_num_val (loglevel, &level))
			Clean_exit (ERROR_BAD_LOG_LEVEL_NUMBER, errno, FALSE, TRUE);

		/* Verifico que el número pasado esté en el rango válido. */
		if ( (level < LL_MIN) || (level > LL_MAX))
			Clean_exit (ERROR_LOG_LEVEL_OUT_OF_RANGE, 0, FALSE, TRUE);
	}

	opt_log_level = (TLogLevel) level;

	Set_actual_log_level (opt_log_level, FALSE);
}


/*
Determina el nivel de loggeo.

1er parámetro: Puntero al nombre del ejecutable.

2do parámetro: Puntero al parámetro de la opcion.


devuelve: -.
*/
static void Do_int_log_level (const long int loglevel)
{
	if (b_opt_log_level)
		Clean_exit (ERROR_ONLY_ONE_LOG_LEVEL, 0, FALSE, TRUE);

	b_opt_log_level = TRUE;

	/* Verifico que el número pasado esté en el rango válido. */
	if ( (loglevel < LL_MIN) || (loglevel > LL_MAX))
		Clean_exit (ERROR_LOG_LEVEL_OUT_OF_RANGE, 0, FALSE, TRUE);

	opt_log_level = (TLogLevel) loglevel;

	Set_actual_log_level (opt_log_level, FALSE);
}


/*
Determina si se aceptan conexiones múltiples simultaneamente.

1er parámetro: Puntero al nombre del ejecutable.

2do parámetro: Puntero al parámetro de la opcion.


devuelve: -.
*/
static void Do_single (bool single)
{
	if (b_opt_single)
		Clean_exit (ERROR_ONLY_ONE_SINGLE, 0, FALSE, TRUE);

	b_opt_single = TRUE;

	opt_single = single;
}


/*
Determina el usuario\grupo al que se dropea.

1er parámetro: Puntero al nombre del ejecutable.

2do parámetro: Puntero al parámetro de la opcion.


devuelve: -.
*/
static void Do_user_group (const char *user)
{
	if (b_opt_user)
		Clean_exit (ERROR_ONLY_ONE_USER, 0, FALSE, TRUE);

	b_opt_user = TRUE;

	opt_user = user;

	if (!Get_uid_and_gid_from_string (opt_user, &user_uid, &user_gid, &user_is_primary_group))
		Clean_exit (ERROR_INVALID_USER_GROUP, 0, FALSE, TRUE);
}


/*
Process the command option.

1er parámetro: Puntero al nombre del ejecutable.

2do parámetro: Puntero al parámetro de la opcion.


devuelve: -.
*/
static void Do_command (const char *command)
{
	if (b_opt_command)
		Clean_exit (ERROR_ONLY_ONE_COMMAND, 0, FALSE, TRUE);

	b_opt_command = TRUE;

	opt_command = command;
}


/*

*/
static void Do_config_file (const char *config_file)
{
	if (b_opt_cfg_file_name)
		Clean_exit (ERROR_ONLY_ONE_CONFIG_FILE, 0, FALSE, TRUE);

	b_opt_cfg_file_name = TRUE;

	opt_cfg_file_name = config_file;
}


/*

*/
static inline bool Can_drop_root_privileges (bool root_privileges_needed)
{
	return (!root_privileges_needed || (opt_port > 1023));
}


/*

*/
static bool try_to_drop_root_privileges (bool root_privileges_droped, bool root_privileges_needed)
{
	if ( (!root_privileges_droped) && Do_i_have_root_privileges() &&
			Can_drop_root_privileges (root_privileges_needed))
	{
		/* ************************************************************************************** */
		/* Drop root privileges. */

		if (opt_user != NULL)
		{
			Vlogit (tll_debug,
					LOG_MESSAGE_PREAMBLE,
					LOGLEVELNAME (tll_debug),
					PROCESS_P_C (g_daemonized, g_processor),
					getpid(), "Dropping root privileges...");

			if (!Drop_privileges (user_uid, user_gid, user_is_primary_group))
			{
				Clean_exit (ERROR_DROPPING_ROOT_PRIVILEGES, errno, FALSE, TRUE);
			}

			Vlogit (tll_debug,
					LOG_MESSAGE_PREAMBLE,
					LOGLEVELNAME (tll_debug),
					PROCESS_P_C (g_daemonized, g_processor),
					getpid(), "Success droping root privileges.");

			Vlogit (tll_debug,
					LOG_PROCESS_IDS,
					LOGLEVELNAME (tll_debug),
					PROCESS_P_C (g_daemonized, g_processor),
					getpid(),
					getuid(), geteuid(), getgid(), getegid());


			return TRUE;
		}
		else
			return FALSE;
		/* ************************************************************************************** */
	}
	else
		return FALSE;
}


/*

*/
static void Signal_handler (int sig)
{
	pid_t child_pid;

	/*pid_t child_pid, pid;*/

	switch (sig)
	{
	case SIGCHLD:
		Vlogit (tll_debug,
				LOG_MESSAGE_PREAMBLE,
				LOGLEVELNAME (tll_debug),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), "\"SIGCHLD\" signal catched.");

		/*pid = getpid();*/

		/*if ( ( (daemon_pid != 0) && (pid == daemon_pid)) || ( (parent_pid != 0) && (pid == parent_pid)))*/

		while ( (child_pid = waitpid (-1, NULL, WNOHANG)) > 0)
		{
			Vlogit (tll_debug,
					LOG_CHILD_DIED,
					LOGLEVELNAME (tll_debug),
					PROCESS_P_C (g_daemonized, g_processor),
					getpid(), child_pid);
		}

		break;
	case SIGHUP:
		Vlogit (tll_debug,
				LOG_MESSAGE_PREAMBLE,
				LOGLEVELNAME (tll_debug),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), "\"SIGHUP\" signal catched.");
		break;
	case SIGTERM:
		Vlogit (tll_debug,
				LOG_MESSAGE_PREAMBLE,
				LOGLEVELNAME (tll_debug),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), "\"SIGTERM\" signal catched.");

		Signal_to_terminate (TRUE);
		break;
	case SIGINT:
		Vlogit (tll_info,
				LOG_MESSAGE_PREAMBLE,
				LOGLEVELNAME (tll_info),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), "\"SIGINT\" signal catched.");

		Signal_to_terminate (TRUE);
		break;
	case SIGQUIT:
		Vlogit (tll_info,
				LOG_MESSAGE_PREAMBLE,
				LOGLEVELNAME (tll_info),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), "\"SIGQUIT\" signal catched.");

		Signal_to_terminate (TRUE);
		break;
	case SIGALRM:
		Vlogit (tll_info,
				LOG_MESSAGE_PREAMBLE,
				LOGLEVELNAME (tll_info),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), "\"SIGALRM\" signal catched,.");
		break;
	case SIGSEGV:
		Vlogit (tll_info,
				LOG_MESSAGE_PREAMBLE,
				LOGLEVELNAME (tll_info),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), "\"SIGSEGV\" signal catched, oops!!!.");

		Clean_exit (ERROR_SEGMENTATION_FAULT, 0, FALSE, TRUE);
		break;
	default:
		Vlogit (tll_info,
				LOG_MESSAGE_PREAMBLE,
				LOGLEVELNAME (tll_info),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), "\"<UNKNOWN>\" signal catched.");
	}
}


/*

*/
static void Catch_signals (t_signal_handler handler, bool catch_SIGCHLD)
{
	struct sigaction hand;

	memset (&hand, 0x00, sizeof (hand));

	hand.sa_handler = handler;
	sigemptyset (&hand.sa_mask);
	hand.sa_flags = 0;

	sigaction (SIGHUP, &hand, NULL);    	/* Catch "hangup" signal. */
	sigaction (SIGTERM, &hand, NULL);   	/* Catch "terminate" signal. */
	sigaction (SIGINT, &hand, NULL);    	/* Catch "interrupt" signal. */
	sigaction (SIGQUIT, &hand, NULL);    	/* Catch "quit" signal. */
	sigaction (SIGSEGV, &hand, NULL);   	/* Catch "segmentation fault" signal. */
	sigaction (SIGALRM, &hand, NULL);   	/* Catch "alarm" signal. */


	if (catch_SIGCHLD)
		sigaction (SIGCHLD, &hand, NULL);	/* Catch "child died" signal. */

	hand.sa_handler = SIG_IGN;

	sigaction (SIGTSTP, &hand, NULL);   	/* Ignore "stop" signals. */
	sigaction (SIGTTIN, &hand, NULL);   	/* Ignore "tried to read from console" signals. */
	sigaction (SIGTTOU, &hand, NULL);   	/* Ignore "tried to write to console" signals. */

	hand.sa_handler = SIG_DFL;

	if (!catch_SIGCHLD)
		sigaction (SIGCHLD, &hand, NULL);	/* Default "child died" signal. */
}

















