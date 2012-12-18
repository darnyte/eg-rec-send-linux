#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>

#include <strings.h>
#include <string.h>

#include <errno.h>

#include <stdint.h>
#include <limits.h>

#include <signal.h>
#include <sys/wait.h>
#include <sys/time.h>


#include <assert.h>

#include "./../Common/globals.h"
#include "processclient.h"
#include "Server.h"
#include "exiterror.h"
#include "./../Common/popen_ext/popenext.h"



#define HASH_LEN					16*2

#define MAGIC_WORD					"quintessence"

#define ACCEPT_MSG					"accept"

#define CLOSE_MSG					"close"

#define PAYLOAD_MSG					"payload"

#define WITHOUT_RELEASE_MSG			"withoutRelease"

#define BUTTON_RELEASED_MSG			"ButtonReleased"


#define MAX_PAYLOAD_LEN_STR_LEN		10
#define MAX_EVENT_TYPE_STR_LEN		1
#define MAX_EVENT_LINE_LEN			PIPE_BUF - 1

#define HELLO_MY_MSG				"HELLO"
#define PAYLOAD_MY_MSG				"PAYLOAD"
#define CLOSE_MY_MSG				"CLOSE"

typedef enum TEventType {tet_unknown = 0, tet_event = 1, tet_enduring_event = 2 , tet_end_event = 3} TEventType;


static bool Execute_and_report_output (const char* executable, const char* client_host_name, const char* client_IP,
									   const uint16_t client_port, const char* event, TEventType event_type, const size_t payload_len,
									   const char** payload, const int *fd_to_close_in_child, const int fd_to_close_in_child_count);

static bool Generate_cookie (char* cookie, size_t cookie_len);

static bool Set_alarm_timer (double time_to_wait);

static void Ignore_or_restore_SIGPIPE_signal (bool ignore);


/*

*/
int Process_new_client_connection (const int socket_fd, const char* client_host_name, const char* client_IP, const uint16_t client_port,
								   const char* password, const char* executable)
{
	bool auxres;
	char* line_buffer = NULL;
	size_t line_buffer_len = 0;
	size_t line_len = 0;
	FILE *input_lines, *output_lines;
	char cookie[5];
	char* password_latin;
	char hash[HASH_LEN + 1];
	bool eof_found;

	bool close_requested;
	TEventType event_type;

	char* event;
	char** payload;
	size_t payload_len, payload_buffer_len;


	errno = 0;


	/* ************************************************************************************** */
	Vlogit (tll_debug,
			LOG_MESSAGE_PREAMBLE,
			LOGLEVELNAME (tll_debug),
			PROCESS_P_C (g_daemonized, g_processor),
			getpid(), "Promoting socket to FILE...");

	errno = 0;

	/* Promote socket. */
	if ( ( (input_lines = fdopen (socket_fd, "r") ) == NULL) ||
			( (output_lines = fdopen (socket_fd, "w") ) == NULL) )
		return ERROR_PROMOTING_SOCKET_TO_FILE;

	/* Set the buffering to lines for the output file. */
	setlinebuf (output_lines);

	Vlogit (tll_debug,
			LOG_MESSAGE_PREAMBLE,
			LOGLEVELNAME (tll_debug),
			PROCESS_P_C (g_daemonized, g_processor),
			getpid(), "Success promoting socket to FILE.");
	/* ************************************************************************************** */


	/* ************************************************************************************** */
	/* If the program is signaled to terminate. */
	if (Is_signaled_to_terminate() )
		return ERROR_INTERRUPTED;
	/* ************************************************************************************** */


	/* ************************************************************************************** */
	Vlogit (tll_debug,
			LOG_MESSAGE_PREAMBLE,
			LOGLEVELNAME (tll_debug),
			PROCESS_P_C (g_daemonized, g_processor),
			getpid(), "Trying to read the \"Magic word\".");

	errno = 0;

	if (!Read_stripped_line (input_lines, &line_buffer, &line_buffer_len,
							 &line_len, &eof_found, TRUE, FALSE) )
		return ERROR_READING_MAGIC_WORD;

	errno = 0;

	auxres =
		( (line_len == strlen (MAGIC_WORD) ) &&
		  (strncmp (line_buffer, MAGIC_WORD, line_len) == 0) );

	Free_string (&line_buffer);

	if (!auxres)
		return ERROR_BAD_MAGIC_WORD;

	Vlogit (tll_debug,
			LOG_MESSAGE_PREAMBLE,
			LOGLEVELNAME (tll_debug),
			PROCESS_P_C (g_daemonized, g_processor),
			getpid(), "Success reading the \"Magic word\".");
	/* ************************************************************************************** */


	/* ************************************************************************************** */
	/* If the program is signaled to terminate. */
	if (Is_signaled_to_terminate() )
		return ERROR_INTERRUPTED;
	/* ************************************************************************************** */


	/* ************************************************************************************** */
	Vlogit (tll_debug,
			LOG_MESSAGE_PREAMBLE,
			LOGLEVELNAME (tll_debug),
			PROCESS_P_C (g_daemonized, g_processor),
			getpid(), "Generating and sending cookie for password...");

	errno = 0;

	if (!Generate_cookie (cookie, sizeof (cookie) ) )
		return ERROR_GENERATING_PASSWORD_COOKIE;

	if (!Write_line (output_lines,  cookie) )
		return ERROR_WRITING_COOKIE;

	Vlogit (tll_debug,
			LOG_MESSAGE_PREAMBLE,
			LOGLEVELNAME (tll_debug),
			PROCESS_P_C (g_daemonized, g_processor),
			getpid(), "Success generating and sending cookie for password.");
	/* ************************************************************************************** */


	/* ************************************************************************************** */
	/* If the program is signaled to terminate. */
	if (Is_signaled_to_terminate() )
		return ERROR_INTERRUPTED;
	/* ************************************************************************************** */


	/* ************************************************************************************** */
	Vlogit (tll_debug,
			LOG_MESSAGE_PREAMBLE,
			LOGLEVELNAME (tll_debug),
			PROCESS_P_C (g_daemonized, g_processor),
			getpid(), "Calculating hash...");

	errno = 0;

	if (!Convert_between_latin15_and_current_locale (password, &password_latin, FALSE) )
		return ERROR_CALCULATING_HASH;

	errno = 0;

	auxres = Hash_cookied_password (password_latin, cookie, hash, sizeof (hash) );

	Free_string (&password_latin);

	if (!auxres)
		return ERROR_CALCULATING_HASH;

	Vlogit (tll_debug,
			LOG_MESSAGE_PREAMBLE,
			LOGLEVELNAME (tll_debug),
			PROCESS_P_C (g_daemonized, g_processor),
			getpid(), "Success calculating hash.");
	/* ************************************************************************************** */


	/* ************************************************************************************** */
	/* If the program is signaled to terminate. */
	if (Is_signaled_to_terminate() )
		return ERROR_INTERRUPTED;
	/* ************************************************************************************** */


	/* ************************************************************************************** */
	Vlogit (tll_debug,
			LOG_MESSAGE_PREAMBLE,
			LOGLEVELNAME (tll_debug),
			PROCESS_P_C (g_daemonized, g_processor),
			getpid(), "Trying to read and match hash from client...");

	if (!Read_stripped_line (input_lines, &line_buffer, &line_buffer_len,
							 &line_len, &eof_found, FALSE, FALSE) )
		return ERROR_READING_HASH;

	errno = 0;

	auxres = ( (line_len == HASH_LEN) && (strncasecmp (line_buffer, hash, line_len) == 0) );

	Free_string (&line_buffer);

	if (!auxres)
		return ERROR_HASH_DOES_NOT_MATCH;

	Vlogit (tll_debug,
			LOG_MESSAGE_PREAMBLE,
			LOGLEVELNAME (tll_debug),
			PROCESS_P_C (g_daemonized, g_processor),
			getpid(), "Success reading and matching hash from client.");
	/* ************************************************************************************** */


	/* ************************************************************************************** */
	/* If the program is signaled to terminate. */
	if (Is_signaled_to_terminate() )
		return ERROR_INTERRUPTED;
	/* ************************************************************************************** */


	/* ************************************************************************************** */
	Vlogit (tll_debug,
			LOG_MESSAGE_PREAMBLE,
			LOGLEVELNAME (tll_debug),
			PROCESS_P_C (g_daemonized, g_processor),
			getpid(), "Sending \"accept\" message...");

	errno = 0;

	if (!Write_line (output_lines, ACCEPT_MSG) )
		return ERROR_WRITING_ACCEPT_MSG;

	Vlogit (tll_debug,
			LOG_MESSAGE_PREAMBLE,
			LOGLEVELNAME (tll_debug),
			PROCESS_P_C (g_daemonized, g_processor),
			getpid(), "Success sending \"accept\" message.");
	/* ************************************************************************************** */


	/* ************************************************************************************** */
	/* If the program is signaled to terminate. */
	if (Is_signaled_to_terminate() )
		return ERROR_INTERRUPTED;
	/* ************************************************************************************** */


	/* ************************************************************************************** */
	Vlogit (tll_debug,
			LOG_MESSAGE_PREAMBLE,
			LOGLEVELNAME (tll_debug),
			PROCESS_P_C (g_daemonized, g_processor),
			getpid(), "Processing events...");


	close_requested = FALSE;
	event_type = tet_unknown;

	event = NULL;
	payload = NULL;
	payload_len = 0;
	payload_buffer_len = 0;

	line_buffer = NULL;

	do
	{
		if ( (!Read_stripped_line (input_lines, &line_buffer, &line_buffer_len,
								   &line_len, &eof_found, FALSE, TRUE) ) && (!eof_found) )
		{
			Free_string_array (&payload, &payload_len, &payload_buffer_len);
			Free_string (&line_buffer);

			return ERROR_READING_EVENT;
		}
		else if (eof_found)
		{

			Free_string_array (&payload, &payload_len, &payload_buffer_len);
			Free_string (&line_buffer);

			break;
		}
		else if (line_buffer_len > MAX_EVENT_LINE_LEN)
		{
			Free_string_array (&payload, &payload_len, &payload_buffer_len);
			Free_string (&line_buffer);

			return ERROR_EVENT_LINE_TOO_LONG;
		}

		/* If the program is signaled to terminate. */
		if (Is_signaled_to_terminate() )
			break;

		if (strncasecmp (line_buffer, CLOSE_MSG,
						 MINVAL (line_len, strlen (CLOSE_MSG) ) ) == 0)
		{
			Vlogit (tll_debug,
					LOG_MESSAGE_PREAMBLE,
					LOGLEVELNAME (tll_debug),
					PROCESS_P_C (g_daemonized, g_processor),
					getpid(), "Received: <Close>");

			/* Close requested. */

			Free_string_array (&payload, &payload_len, &payload_buffer_len);
			Free_string (&line_buffer);

			close_requested = TRUE;
		}
		else if (strncasecmp (line_buffer, PAYLOAD_MSG,
							  MINVAL (line_len, strlen (PAYLOAD_MSG) ) ) == 0)
		{
			Vlogit (tll_debug,
					LOG_MESSAGE_PREAMBLE,
					LOGLEVELNAME (tll_debug),
					PROCESS_P_C (g_daemonized, g_processor),
					getpid(), "Received: <Payload>");

			/* Add to payload. */

			if (!Add_string_to_string_array (&payload, &payload_len, &payload_buffer_len,
											 line_buffer + strlen (PAYLOAD_MSG) + 1) )
			{
				Free_string_array (&payload, &payload_len, &payload_buffer_len);
				Free_string (&line_buffer);

				return ERROR_ALLOCATING_MEMORY;
			}
		}
		else if ( (payload_len > 0) && (strncasecmp (payload[payload_len - 1], BUTTON_RELEASED_MSG,
										MINVAL (strlen (payload[payload_len - 1]), strlen (BUTTON_RELEASED_MSG) ) ) == 0) )
		{
			Vlogit (tll_debug,
					LOG_MESSAGE_PREAMBLE,
					LOGLEVELNAME (tll_debug),
					PROCESS_P_C (g_daemonized, g_processor),
					getpid(), "Received: <Enduring Event - Stop>");

			/* End last event. */

			event = line_buffer;
			event_type = tet_end_event;
		}
		else if ( (payload_len > 0) && (strncasecmp (payload[payload_len - 1], WITHOUT_RELEASE_MSG,
										MINVAL (strlen (payload[payload_len - 1]), strlen (WITHOUT_RELEASE_MSG) ) ) == 0) )
		{
			Vlogit (tll_debug,
					LOG_MESSAGE_PREAMBLE,
					LOGLEVELNAME (tll_debug),
					PROCESS_P_C (g_daemonized, g_processor),
					getpid(), "Received: <Enduring Event - Start>");

			/* Trigger enduring event. */

			event = line_buffer;
			event_type = tet_enduring_event;
		}
		else
		{
			Vlogit (tll_debug,
					LOG_MESSAGE_PREAMBLE,
					LOGLEVELNAME (tll_debug),
					PROCESS_P_C (g_daemonized, g_processor),
					getpid(), "Received: <Event>");

			/* Trigger event. */

			event = line_buffer;
			event_type = tet_event;
		}

		errno = 0;

		if ( (!Is_signaled_to_terminate() ) && (!close_requested) && (event_type != tet_unknown) &&
				(executable != NULL) && (strlen (executable) > 0) )
		{
			Vlogit (tll_info,
					LOG_MESSAGE_PREAMBLE,
					LOGLEVELNAME (tll_info),
					PROCESS_P_C (g_daemonized, g_processor),
					getpid(),
					"Executing command...");

			Ignore_or_restore_SIGPIPE_signal (TRUE);

			auxres = Execute_and_report_output (executable, client_host_name, client_IP, client_port,
												(event != NULL) ? event : EMPTY_STRING,
												event_type, payload_len, (const char**) payload,
												&socket_fd, 1);

			Ignore_or_restore_SIGPIPE_signal (FALSE);

			if (!auxres)

			{
				Free_string_array (&payload, &payload_len, &payload_buffer_len);
				Free_string (&line_buffer);

				return ERROR_EXECUTING_COMMAND;
			}

			Vlogit (tll_info,
					LOG_MESSAGE_PREAMBLE,
					LOGLEVELNAME (tll_info),
					PROCESS_P_C (g_daemonized, g_processor),
					getpid(), "Success executing command.");
		}
		else if ( (!close_requested) && (event_type != tet_unknown) )
		{
			Vlogit (tll_info,
					LOG_EVENT_NO_COMMAND,
					LOGLEVELNAME (tll_info),
					PROCESS_P_C (g_daemonized, g_processor),
					getpid(), event);
		}

		Free_string (&line_buffer);

		if (event_type != tet_unknown)
		{
			Free_string_array (&payload, &payload_len, &payload_buffer_len);
			close_requested = FALSE;
			event_type = tet_unknown;

			event = NULL;
		}
	}
	while (!Is_signaled_to_terminate() && !close_requested);

	Free_string_array (&payload, &payload_len, &payload_buffer_len);
	Free_string (&line_buffer);
	/* ************************************************************************************** */


	/* ************************************************************************************** */
	/* If the program is signaled to terminate. */
	if (Is_signaled_to_terminate() )
		return ERROR_INTERRUPTED;
	else if (close_requested)
	{
		Vlogit (tll_info,
				LOG_MESSAGE_PREAMBLE,
				LOGLEVELNAME (tll_info),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), "Client requested close.");
	}
	else
	{
		Vlogit (tll_info,
				LOG_MESSAGE_PREAMBLE,
				LOGLEVELNAME (tll_info),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), "Client closed the conection.");
	}
	/* ************************************************************************************** */

	errno = 0;

	return ERROR_SUCESS;
}

static bool Execute_and_report_output (const char* executable, const char* client_host_name, const char* client_IP,
									   const uint16_t client_port, const char* event, TEventType event_type, const size_t payload_len,
									   const char** payload, const int *fd_to_close_in_child, const int fd_to_close_in_child_count)
{

	/* command <client IP> <client port> <event type> <event> <payload length> <payload 1> ...<payload n> */

	size_t len = 0;
	size_t buffer_len = 8 /*+ payload_len*/;

	size_t line_buffer_len = 0;
	char* line_buffer = NULL;
	size_t line_len = 0;
	int linenr;
	bool eof_found;
	FILE *pipe_read, *pipe_write;

	char** argv = NULL;
	pid_t childpid = 0;

	int cmdres;
	char* typeexec = "p";
	bool res = FALSE;

	bool close_received = FALSE;
	bool error_executing = FALSE;
	bool timer_failed = FALSE;
	bool message_line = FALSE;

	/* command <client host name> <client IP> <client port> <event type> <event> <payload length> <payload 1> ...<payload n> */

	if (!Add_string_to_string_array (&argv, &len, &buffer_len, executable) )
	{
		Free_string_array (&argv, &len, &buffer_len);
		return FALSE;
	}

	if (!Add_string_to_string_array (&argv, &len, &buffer_len, client_host_name) )
	{
		Free_string_array (&argv, &len, &buffer_len);
		return FALSE;
	}

	if (!Add_string_to_string_array (&argv, &len, &buffer_len, client_IP) )
	{
		Free_string_array (&argv, &len, &buffer_len);
		return FALSE;
	}

	if (!Add_luint_to_string_array (&argv, &len, &buffer_len, client_port) )
	{
		Free_string_array (&argv, &len, &buffer_len);
		return FALSE;
	}

	if (!Add_luint_to_string_array (&argv, &len, &buffer_len, event_type) )
	{
		Free_string_array (&argv, &len, &buffer_len);
		return FALSE;
	}

	if (!Add_string_to_string_array (&argv, &len, &buffer_len, event) )
	{
		Free_string_array (&argv, &len, &buffer_len);
		return FALSE;
	}

	if (!Add_luint_to_string_array (&argv, &len, &buffer_len, payload_len) )
	{
		Free_string_array (&argv, &len, &buffer_len);
		return FALSE;
	}

	/*for (i = 0; i < payload_len; i++)
	{
		if (!Add_string_to_string_array (&argv, &len, &buffer_len, payload[i]) )
		{
			Free_string_array (&argv, &len, &buffer_len);
			return FALSE;
		}
	}*/

	errno = 0;

	line_buffer_len = LINE_BUFFER_SIZE;

	line_buffer = malloc (LINE_BUFFER_SIZE);

	if (line_buffer == NULL)
	{
		Free_string_array (&argv, &len, &buffer_len);
		return FALSE;
	}

	if (!popen_ext (argv[0], argv, typeexec, &pipe_read, &pipe_write, &childpid,
					fd_to_close_in_child, fd_to_close_in_child_count) )
	{
		Free_string_array (&argv, &len, &buffer_len);
		Free_string (&line_buffer);
		return FALSE;
	}

	/* Set the buffering to lines for the write pipe end. */
	setlinebuf (pipe_write);

	linenr = 1;

	while (!close_received)
	{
		errno = 0;

		if (!Set_alarm_timer (EXECUTE_READ_WRITE_LINE_TIME_OUT) )
		{
			timer_failed = TRUE;
			break;
		}

		error_executing = (!Read_stripped_line (
							   pipe_read, &line_buffer, &line_buffer_len,
							   &line_len, &eof_found, FALSE, FALSE) );

		if (!Set_alarm_timer (0.0) )
		{
			timer_failed = TRUE;
			break;
		}

		if (error_executing)
			break;

		if ( ( (line_len == strlen (CLOSE_MY_MSG) ) &&
				(strncmp (line_buffer, CLOSE_MY_MSG, line_len) == 0) ) )
		{
			/* Child indicates termination. */
			close_received = TRUE;
		}


		if (!close_received)
		{
			if (linenr == 1)
			{
				/* HELLO message expected from child. */

				if ( (line_len != strlen (HELLO_MY_MSG) ) ||
						(strncmp (line_buffer, HELLO_MY_MSG, line_len) != 0) )
				{
					/* Invalid HELLO message. */

					error_executing = TRUE;
					break;
				}
			}
			else
			{
				/* PAYLOAD message expected from child, requesting a payload line. */

				if ( (line_len == strlen (PAYLOAD_MY_MSG) ) &&
						(strncmp (line_buffer, PAYLOAD_MY_MSG, line_len) == 0) &&
						( (size_t) (linenr - 2) < payload_len) )
				{
					if (!Set_alarm_timer (EXECUTE_READ_WRITE_LINE_TIME_OUT) )
					{
						timer_failed = TRUE;
						break;
					}

					if (!Write_line (pipe_write, payload[linenr - 2]) )
					{
						error_executing = TRUE;
						break;
					}

					if (!Set_alarm_timer (0.0) )
					{
						timer_failed = TRUE;
						break;
					}
				}
				else if (!message_line)
				{
					/* Child process sended an extra info line. */

					message_line = TRUE;
				}
				else
				{
					/* Invalid message or no more payload lines. */

					error_executing = TRUE;
					break;
				}
			}
		}

		Vlogit (tll_debug,
				LOG_COMMAND_OUTPUT,
				LOGLEVELNAME (tll_debug),
				PROCESS_P_C (g_daemonized, g_processor),
				getpid(), linenr, line_buffer);

		++linenr;

		if (!Set_alarm_timer (EXECUTE_READ_WRITE_LINE_TIME_OUT) )
		{
			timer_failed = TRUE;
			break;
		}
	}

	if (!Set_alarm_timer (0.0) )
		timer_failed = TRUE;

	Free_string (&line_buffer);

	if (error_executing || timer_failed)
	{
		/* The child process waas executed but we failed to set a timer or
		to read from or to write to the process. */
		error_executing = TRUE;

		/* Ask the child process to terminate. */
		kill (childpid, SIGTERM);
	}

	timer_failed = (!Set_alarm_timer (EXECUTE_WAIT_TIME_OUT) );

	res = ( (!timer_failed) && pclose_ext (&pipe_read, &pipe_write, childpid, &cmdres) );

	Set_alarm_timer (0.0);

	if (!res)
	{
		kill (childpid, SIGKILL);

		usleep (100000);

		waitpid (childpid, NULL, WNOHANG);

		if (pipe_read != NULL)
			fclose (pipe_read);

		if (pipe_write != NULL)
			fclose (pipe_write);
	}

	Free_string_array (&argv, &len, &buffer_len);

	return (!error_executing && res && (cmdres == 0) );
}


/*

*/
static bool Generate_cookie (char* cookie, size_t cookie_len)
{
	int cookie_num;

	cookie_num = Rand_lim (0xFFFF);

	bzero (cookie, cookie_len);

	return (snprintf (cookie, cookie_len, "%4X", cookie_num) == 4);
}


/*


*/
static bool Set_alarm_timer (double time_to_wait)
{
	struct itimerval time;

	/* Check parameters.*/
	assert (time_to_wait >= 0.0);
	/* ------------------------ */

	memset (&time, 0x00, sizeof (time) );

	if (time_to_wait > 0.0)
	{
		time.it_value.tv_sec = time_to_wait;
		time.it_value.tv_usec = ( (time_to_wait - (double) time.it_value.tv_sec) * 1000000);
	}

	errno = 0;

	return (setitimer (ITIMER_REAL, &time, NULL) == 0);
}


/*


*/
static void Ignore_or_restore_SIGPIPE_signal (bool ignore)
{
	struct sigaction hand;

	memset (&hand, 0x00, sizeof (hand) );

	sigemptyset (&hand.sa_mask);
	hand.sa_flags = 0;

	if (ignore)
		hand.sa_handler = SIG_IGN;
	else
		hand.sa_handler = SIG_DFL;

	sigaction (SIGPIPE, &hand, NULL);
}


