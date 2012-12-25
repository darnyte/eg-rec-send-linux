#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>

#include <string.h>

#include <errno.h>

#include <fcntl.h>
#include <sys/wait.h>

#include <assert.h>

#include "./../globals.h"

#include "popenext.h"

/*


*/
bool popen_ext (char *cmd,           					/* duplicate this in argv[0] */
				char *argv[],         					/* last argv[] must be NULL */
				const bool use_execvp, 					/* use use_execvp() instead of use_execv */
				FILE **pfp_read_stdout,      			/* read file handle to stdout returned */
				FILE **pfp_write_stdin,     			/* write file handle to stdin returned */
				FILE **pfp_read_stderr,      			/* read file handle to stderr returned */
				bool inherit_stdin,						/* in case pipe to stdin is not required, inherit parent stdin redirect to null device otherwise */
				bool inherit_stdout,					/* in case pipe to stdout is not required, inherit parent stdout redirect to null device otherwise */
				bool inherit_stderr,					/* in case pipe to stderr is not required, inherit parent stderr redirect to null device otherwise */
				pid_t *ppid,            				/* process id returned */
				const int *fd_to_close_in_child,        /* [in], array of fd */
				const int fd_to_close_in_child_count)   /* number of valid fd */
{
	int j;
	int fd_pipe_stdout[2] = { -1, -1};
	int fd_pipe_stdin[2] = { -1, -1};
	int fd_pipe_stderr[2] = { -1, -1};
	int null_file_des;


	/* Check parameters.*/
	assert (pfp_read_stdout || pfp_write_stdin || pfp_read_stderr);
	assert (cmd);
	assert (argv);
	assert (ppid);
	assert (!fd_to_close_in_child_count || fd_to_close_in_child);
	/* ------------------------ */


	/* ********************************************************************************* */
	/* Check parameters.*/

	if ( (pfp_read_stdout == NULL) && (pfp_write_stdin == NULL)  && (pfp_read_stderr == NULL))
		return false;
	/* ********************************************************************************* */


	/* ********************************************************************************* */
	/* Initialize file pointers. */

	if (pfp_read_stdout != NULL)
		*pfp_read_stdout = NULL;

	if (pfp_write_stdin != NULL)
		*pfp_write_stdin = NULL;

	if (pfp_read_stderr != NULL)
		*pfp_read_stderr = NULL;
	/* ********************************************************************************* */


	/* ********************************************************************************* */
	/* Check parameters.*/

	if ( (cmd == NULL) || (argv == NULL) || (ppid == NULL))
		return false;
	/* ********************************************************************************* */


	/* ********************************************************************************* */
	/* Create stdout pipe. */

	if (pfp_read_stdout != NULL)
	{
		errno = 0;

		if (pipe (fd_pipe_stdout) < 0)
			return false;
	}
	/* ********************************************************************************* */


	/* ********************************************************************************* */
	/* Create stdin pipe. */

	if (pfp_write_stdin != NULL)
	{
		errno = 0;

		if (pipe (fd_pipe_stdin) < 0)
		{
			if (pfp_read_stdout != NULL)
			{
				close (fd_pipe_stdout[PIPE_READ_END]);
				close (fd_pipe_stdout[PIPE_WRITE_END]);
			}

			return false;
		}
	}
	/* ********************************************************************************* */


	/* ********************************************************************************* */
	/* Create stderr pipe. */

	if (pfp_read_stderr != NULL)
	{
		errno = 0;

		if (pipe (fd_pipe_stderr) < 0)
		{
			if (pfp_read_stdout != NULL)
			{
				close (fd_pipe_stdout[PIPE_READ_END]);
				close (fd_pipe_stdout[PIPE_WRITE_END]);
			}

			if (fd_pipe_stdin != NULL)
			{
				close (fd_pipe_stdin[PIPE_READ_END]);
				close (fd_pipe_stdin[PIPE_WRITE_END]);
			}

			return false;
		}
	}
	/* ********************************************************************************* */


	errno = 0;

	*ppid = fork();

	if (*ppid < 0)
	{
		/* Error forking. */

		if (pfp_read_stdout != NULL)
		{
			close (fd_pipe_stdout[PIPE_READ_END]);
			close (fd_pipe_stdout[PIPE_WRITE_END]);
		}

		if (fd_pipe_stdin != NULL)
		{
			close (fd_pipe_stdin[PIPE_READ_END]);
			close (fd_pipe_stdin[PIPE_WRITE_END]);
		}

		if (fd_pipe_stderr != NULL)
		{
			close (fd_pipe_stderr[PIPE_READ_END]);
			close (fd_pipe_stderr[PIPE_WRITE_END]);
		}

		return false;
	}
	else if (*ppid == 0)
	{
		/* child continues*/

		/* ********************************************************************************* */
		/* Redirect stdout. */

		if (pfp_read_stdout != NULL)
		{
			if (fd_pipe_stdout[PIPE_WRITE_END] != STDOUT_FILENO)
			{
				errno = 0;

				if (dup2 (fd_pipe_stdout[PIPE_WRITE_END], STDOUT_FILENO) == -1)
				{
					if (pfp_read_stdout != NULL)
					{
						close (fd_pipe_stdout[PIPE_READ_END]);
						close (fd_pipe_stdout[PIPE_WRITE_END]);
					}

					if (fd_pipe_stdin != NULL)
					{
						close (fd_pipe_stdin[PIPE_READ_END]);
						close (fd_pipe_stdin[PIPE_WRITE_END]);
					}

					if (fd_pipe_stderr != NULL)
					{
						close (fd_pipe_stderr[PIPE_READ_END]);
						close (fd_pipe_stderr[PIPE_WRITE_END]);
					}

					_exit (127);
				}

				errno = 0;

				if (close (fd_pipe_stdout[PIPE_WRITE_END]) != 0)
				{
					if (pfp_read_stdout != NULL)
					{
						close (fd_pipe_stdout[PIPE_READ_END]);
						close (fd_pipe_stdout[PIPE_WRITE_END]);
					}

					if (fd_pipe_stdin != NULL)
					{
						close (fd_pipe_stdin[PIPE_READ_END]);
						close (fd_pipe_stdin[PIPE_WRITE_END]);
					}

					if (fd_pipe_stderr != NULL)
					{
						close (fd_pipe_stderr[PIPE_READ_END]);
						close (fd_pipe_stderr[PIPE_WRITE_END]);
					}

					_exit (127);
				}
			}

			errno = 0;

			if (close (fd_pipe_stdout[PIPE_READ_END]) != 0)
			{
				if (pfp_read_stdout != NULL)
				{
					close (fd_pipe_stdout[PIPE_READ_END]);
					close (fd_pipe_stdout[PIPE_WRITE_END]);
				}

				if (fd_pipe_stdin != NULL)
				{
					close (fd_pipe_stdin[PIPE_READ_END]);
					close (fd_pipe_stdin[PIPE_WRITE_END]);
				}

				if (fd_pipe_stderr != NULL)
				{
					close (fd_pipe_stderr[PIPE_READ_END]);
					close (fd_pipe_stderr[PIPE_WRITE_END]);
				}

				_exit (127);
			}
		}
		else if (!inherit_stdout)
		{
			errno = 0;

			if ( (null_file_des = open (NULL_FILE_DEVICE, O_WRONLY, 0)) == -1)
			{
				if (pfp_read_stdout != NULL)
				{
					close (fd_pipe_stdout[PIPE_READ_END]);
					close (fd_pipe_stdout[PIPE_WRITE_END]);
				}

				if (fd_pipe_stdin != NULL)
				{
					close (fd_pipe_stdin[PIPE_READ_END]);
					close (fd_pipe_stdin[PIPE_WRITE_END]);
				}

				if (fd_pipe_stderr != NULL)
				{
					close (fd_pipe_stderr[PIPE_READ_END]);
					close (fd_pipe_stderr[PIPE_WRITE_END]);
				}

				_exit (127);
			}

			errno = 0;

			if (dup2 (null_file_des, STDOUT_FILENO) == -1)
			{
				close (null_file_des);

				if (pfp_read_stdout != NULL)
				{
					close (fd_pipe_stdout[PIPE_READ_END]);
					close (fd_pipe_stdout[PIPE_WRITE_END]);
				}

				if (fd_pipe_stdin != NULL)
				{
					close (fd_pipe_stdin[PIPE_READ_END]);
					close (fd_pipe_stdin[PIPE_WRITE_END]);
				}

				if (fd_pipe_stderr != NULL)
				{
					close (fd_pipe_stderr[PIPE_READ_END]);
					close (fd_pipe_stderr[PIPE_WRITE_END]);
				}

				_exit (127);
			}
		}
		/* ********************************************************************************* */


		/* ********************************************************************************* */
		/* Redirect stdin. */

		if (pfp_write_stdin != NULL)
		{
			if (fd_pipe_stdin[PIPE_READ_END] != STDIN_FILENO)
			{
				errno = 0;

				if (dup2 (fd_pipe_stdin[PIPE_READ_END], STDIN_FILENO) == -1)
				{
					if (pfp_read_stdout != NULL)
					{
						close (fd_pipe_stdout[PIPE_READ_END]);
						close (fd_pipe_stdout[PIPE_WRITE_END]);
					}

					if (fd_pipe_stdin != NULL)
					{
						close (fd_pipe_stdin[PIPE_READ_END]);
						close (fd_pipe_stdin[PIPE_WRITE_END]);
					}

					if (fd_pipe_stderr != NULL)
					{
						close (fd_pipe_stderr[PIPE_READ_END]);
						close (fd_pipe_stderr[PIPE_WRITE_END]);
					}

					_exit (127);
				}

				errno = 0;

				if (close (fd_pipe_stdin[PIPE_READ_END]) != 0)
				{
					if (pfp_read_stdout != NULL)
					{
						close (fd_pipe_stdout[PIPE_READ_END]);
						close (fd_pipe_stdout[PIPE_WRITE_END]);
					}

					if (fd_pipe_stdin != NULL)
					{
						close (fd_pipe_stdin[PIPE_READ_END]);
						close (fd_pipe_stdin[PIPE_WRITE_END]);
					}

					if (fd_pipe_stderr != NULL)
					{
						close (fd_pipe_stderr[PIPE_READ_END]);
						close (fd_pipe_stderr[PIPE_WRITE_END]);
					}

					_exit (127);
				}
			}

			errno = 0;

			if (close (fd_pipe_stdin[PIPE_WRITE_END]) != 0)
			{
				if (pfp_read_stdout != NULL)
				{
					close (fd_pipe_stdout[PIPE_READ_END]);
					close (fd_pipe_stdout[PIPE_WRITE_END]);
				}

				if (fd_pipe_stdin != NULL)
				{
					close (fd_pipe_stdin[PIPE_READ_END]);
					close (fd_pipe_stdin[PIPE_WRITE_END]);
				}

				if (fd_pipe_stderr != NULL)
				{
					close (fd_pipe_stderr[PIPE_READ_END]);
					close (fd_pipe_stderr[PIPE_WRITE_END]);
				}

				_exit (127);
			}
		}
		else if (!inherit_stdin)
		{
			errno = 0;

			if ( (null_file_des = open (NULL_FILE_DEVICE, O_RDONLY, 0)) == -1)
			{
				if (pfp_read_stdout != NULL)
				{
					close (fd_pipe_stdout[PIPE_READ_END]);
					close (fd_pipe_stdout[PIPE_WRITE_END]);
				}

				if (fd_pipe_stdin != NULL)
				{
					close (fd_pipe_stdin[PIPE_READ_END]);
					close (fd_pipe_stdin[PIPE_WRITE_END]);
				}

				if (fd_pipe_stderr != NULL)
				{
					close (fd_pipe_stderr[PIPE_READ_END]);
					close (fd_pipe_stderr[PIPE_WRITE_END]);
				}

				_exit (127);
			}

			errno = 0;

			if (dup2 (null_file_des, STDIN_FILENO) == -1)
			{
				close (null_file_des);

				if (pfp_read_stdout != NULL)
				{
					close (fd_pipe_stdout[PIPE_READ_END]);
					close (fd_pipe_stdout[PIPE_WRITE_END]);
				}

				if (fd_pipe_stdin != NULL)
				{
					close (fd_pipe_stdin[PIPE_READ_END]);
					close (fd_pipe_stdin[PIPE_WRITE_END]);
				}

				if (fd_pipe_stderr != NULL)
				{
					close (fd_pipe_stderr[PIPE_READ_END]);
					close (fd_pipe_stderr[PIPE_WRITE_END]);
				}

				_exit (127);
			}
		}
		/* ********************************************************************************* */


		/* ********************************************************************************* */
		/* Redirect stderr. */

		if (pfp_read_stderr != NULL)
		{
			if (fd_pipe_stderr[PIPE_WRITE_END] != STDERR_FILENO)
			{
				errno = 0;

				if (dup2 (fd_pipe_stdout[PIPE_WRITE_END], STDERR_FILENO) == -1)
				{
					if (pfp_read_stdout != NULL)
					{
						close (fd_pipe_stdout[PIPE_READ_END]);
						close (fd_pipe_stdout[PIPE_WRITE_END]);
					}

					if (fd_pipe_stdin != NULL)
					{
						close (fd_pipe_stdin[PIPE_READ_END]);
						close (fd_pipe_stdin[PIPE_WRITE_END]);
					}

					if (fd_pipe_stderr != NULL)
					{
						close (fd_pipe_stderr[PIPE_READ_END]);
						close (fd_pipe_stderr[PIPE_WRITE_END]);
					}

					_exit (127);
				}

				errno = 0;

				if (close (fd_pipe_stderr[PIPE_WRITE_END]) != 0)
				{
					if (pfp_read_stdout != NULL)
					{
						close (fd_pipe_stdout[PIPE_READ_END]);
						close (fd_pipe_stdout[PIPE_WRITE_END]);
					}

					if (fd_pipe_stdin != NULL)
					{
						close (fd_pipe_stdin[PIPE_READ_END]);
						close (fd_pipe_stdin[PIPE_WRITE_END]);
					}

					if (fd_pipe_stderr != NULL)
					{
						close (fd_pipe_stderr[PIPE_READ_END]);
						close (fd_pipe_stderr[PIPE_WRITE_END]);
					}

					_exit (127);
				}
			}

			errno = 0;

			if (close (fd_pipe_stdout[PIPE_READ_END]) != 0)
			{
				if (pfp_read_stdout != NULL)
				{
					close (fd_pipe_stdout[PIPE_READ_END]);
					close (fd_pipe_stdout[PIPE_WRITE_END]);
				}

				if (fd_pipe_stdin != NULL)
				{
					close (fd_pipe_stdin[PIPE_READ_END]);
					close (fd_pipe_stdin[PIPE_WRITE_END]);
				}

				if (fd_pipe_stderr != NULL)
				{
					close (fd_pipe_stderr[PIPE_READ_END]);
					close (fd_pipe_stderr[PIPE_WRITE_END]);
				}

				_exit (127);
			}
		}
		else if (!inherit_stderr)
		{
			errno = 0;

			if ( (null_file_des = open (NULL_FILE_DEVICE, O_WRONLY, 0)) == -1)
			{
				if (pfp_read_stdout != NULL)
				{
					close (fd_pipe_stdout[PIPE_READ_END]);
					close (fd_pipe_stdout[PIPE_WRITE_END]);
				}

				if (fd_pipe_stdin != NULL)
				{
					close (fd_pipe_stdin[PIPE_READ_END]);
					close (fd_pipe_stdin[PIPE_WRITE_END]);
				}

				if (fd_pipe_stderr != NULL)
				{
					close (fd_pipe_stderr[PIPE_READ_END]);
					close (fd_pipe_stderr[PIPE_WRITE_END]);
				}

				_exit (127);
			}

			errno = 0;

			if (dup2 (null_file_des, STDERR_FILENO) == -1)
			{
				close (null_file_des);

				if (pfp_read_stdout != NULL)
				{
					close (fd_pipe_stdout[PIPE_READ_END]);
					close (fd_pipe_stdout[PIPE_WRITE_END]);
				}

				if (fd_pipe_stdin != NULL)
				{
					close (fd_pipe_stdin[PIPE_READ_END]);
					close (fd_pipe_stdin[PIPE_WRITE_END]);
				}

				if (fd_pipe_stderr != NULL)
				{
					close (fd_pipe_stderr[PIPE_READ_END]);
					close (fd_pipe_stderr[PIPE_WRITE_END]);
				}

				_exit (127);
			}
		}
		/* ********************************************************************************* */


		/* ********************************************************************************* */
		/* Close all indicated file descriptors. */

		for (j = 0; j < fd_to_close_in_child_count; j++)
		{
			errno = 0;

			if (close (fd_to_close_in_child[j]))
			{
				if (pfp_read_stdout != NULL)
				{
					close (fd_pipe_stdout[PIPE_READ_END]);
					close (fd_pipe_stdout[PIPE_WRITE_END]);
				}

				if (fd_pipe_stdin != NULL)
				{
					close (fd_pipe_stdin[PIPE_READ_END]);
					close (fd_pipe_stdin[PIPE_WRITE_END]);
				}

				if (fd_pipe_stderr != NULL)
				{
					close (fd_pipe_stderr[PIPE_READ_END]);
					close (fd_pipe_stderr[PIPE_WRITE_END]);
				}

				_exit (127);
			}
		}
		/* ********************************************************************************* */


		/* ********************************************************************************* */
		/* Replace executable image. */
		errno = 0;

		if (use_execvp)
			execvp (cmd, argv);
		else
			execv (cmd, argv);
		/* ********************************************************************************* */

		/* execv failed */
		_exit (127);
	}

	/* parent continues*/

	/* ********************************************************************************* */
	/* Close stdout pipe write end. */

	if (pfp_read_stdout != NULL)
	{
		errno = 0;

		if (close (fd_pipe_stdout[PIPE_WRITE_END]) != 0)
		{
			if (pfp_read_stdout != NULL)
			{
				close (fd_pipe_stdout[PIPE_READ_END]);
				close (fd_pipe_stdout[PIPE_WRITE_END]);
			}

			if (fd_pipe_stdin != NULL)
			{
				close (fd_pipe_stdin[PIPE_READ_END]);
				close (fd_pipe_stdin[PIPE_WRITE_END]);
			}

			if (fd_pipe_stderr != NULL)
			{
				close (fd_pipe_stderr[PIPE_READ_END]);
				close (fd_pipe_stderr[PIPE_WRITE_END]);
			}

			return false;
		}
	}
	/* ********************************************************************************* */


	/* ********************************************************************************* */
	/* Close stdin pipe read end. */

	if (pfp_write_stdin != NULL)
	{
		errno = 0;

		if (close (fd_pipe_stdin[PIPE_READ_END]) != 0)
		{
			if (pfp_read_stdout != NULL)
			{
				close (fd_pipe_stdout[PIPE_READ_END]);
				close (fd_pipe_stdout[PIPE_WRITE_END]);
			}

			if (fd_pipe_stdin != NULL)
			{
				close (fd_pipe_stdin[PIPE_READ_END]);
				close (fd_pipe_stdin[PIPE_WRITE_END]);
			}

			if (fd_pipe_stderr != NULL)
			{
				close (fd_pipe_stderr[PIPE_READ_END]);
				close (fd_pipe_stderr[PIPE_WRITE_END]);
			}

			return false;
		}
	}
	/* ********************************************************************************* */


	/* ********************************************************************************* */
	/* Close stderr pipe write end. */

	if (pfp_read_stderr != NULL)
	{
		errno = 0;

		if (close (fd_pipe_stderr[PIPE_WRITE_END]) != 0)
		{
			if (pfp_read_stdout != NULL)
			{
				close (fd_pipe_stdout[PIPE_READ_END]);
				close (fd_pipe_stdout[PIPE_WRITE_END]);
			}

			if (fd_pipe_stdin != NULL)
			{
				close (fd_pipe_stdin[PIPE_READ_END]);
				close (fd_pipe_stdin[PIPE_WRITE_END]);
			}

			if (fd_pipe_stderr != NULL)
			{
				close (fd_pipe_stderr[PIPE_READ_END]);
				close (fd_pipe_stderr[PIPE_WRITE_END]);
			}

			return false;
		}
	}
	/* ********************************************************************************* */


	/* ********************************************************************************* */
	/* Open file pointer to stdout. */

	if (pfp_read_stdout != NULL)
	{
		errno = 0;

		if (! (*pfp_read_stdout = fdopen (fd_pipe_stdout[PIPE_READ_END], "r")))
		{
			close (fd_pipe_stdout[PIPE_READ_END]);

			*pfp_read_stdout = NULL;

			if (pfp_write_stdin != NULL)
			{
				close (fd_pipe_stdin[PIPE_WRITE_END]);
				*pfp_write_stdin = NULL;
			}

			if (pfp_read_stderr != NULL)
			{
				close (fd_pipe_stdout[PIPE_READ_END]);
				*pfp_read_stderr = NULL;
			}

			*pfp_read_stdout = NULL;

			return false;
		}
	}
	/* ********************************************************************************* */


	/* ********************************************************************************* */
	/* Open file pointer to stdin. */

	if (pfp_write_stdin != NULL)
	{
		if (! (*pfp_write_stdin = fdopen (fd_pipe_stdin[PIPE_WRITE_END], "w")))
		{
			close (fd_pipe_stdin[PIPE_WRITE_END]);

			*pfp_write_stdin = NULL;

			if (pfp_read_stdout != NULL)
			{
				fclose (*pfp_read_stdout);   /* closing this also closes fd_pipe_stdout[PIPE_READ_END] */
				*pfp_read_stdout = NULL;
			}

			if (pfp_read_stderr != NULL)
			{
				close (fd_pipe_stdout[PIPE_READ_END]);
				*pfp_read_stderr = NULL;
			}

			*pfp_write_stdin = NULL;

			return false;
		}
	}
	/* ********************************************************************************* */


	/* ********************************************************************************* */
	/* Open file pointer to stderr. */

	if (pfp_read_stderr != NULL)
	{
		errno = 0;

		if (! (*pfp_read_stderr = fdopen (fd_pipe_stdout[PIPE_READ_END], "r")))
		{
			close (fd_pipe_stdout[PIPE_READ_END]);

			*pfp_read_stderr = NULL;

			if (pfp_read_stdout != NULL)
			{
				fclose (*pfp_read_stdout);   /* closing this also closes fd_pipe_stdout[PIPE_READ_END] */
				*pfp_read_stdout = NULL;
			}

			if (pfp_write_stdin != NULL)
			{
				fclose (*pfp_write_stdin);   /* closing this also closes fd_pipe_stdin[PIPE_WRITE_END] */
				*pfp_write_stdin = NULL;
			}

			return false;
		}
	}
	/* ********************************************************************************* */

	return true;
}    /*pipe_open_2()*/


/*

*/
bool pclose_ext	(FILE **pfp_read_stdout,		/* returned from pipe_open() */
				 FILE **pfp_write_stdin,		/* returned from pipe_open() */
				 FILE **pfp_read_stderr,		/* returned from pipe_open() */
				 pid_t pid,						/* returned from pipe_open() */
				 bool return_on_signal,			/* don't start the wait after a signal break. */
				 int *result)					/* can be NULL */
{
	int res1, res2, res3, status;


	/* Check parameters.*/
	assert (pfp_read_stdout || pfp_write_stdin || pfp_read_stderr);
	/* ------------------------ */


	if (result != NULL)
		*result = 255;

	if ( (pfp_read_stdout != NULL) && (*pfp_read_stdout != NULL))
	{
		if ( (res1 = fclose (*pfp_read_stdout)) != EOF)
			* pfp_read_stdout = NULL;
	}
	else
		res1 = 0;

	if ( (pfp_write_stdin != NULL) && (*pfp_write_stdin != NULL))
	{
		if ( (res2 = fclose (*pfp_write_stdin)) != EOF)
			* pfp_write_stdin = NULL;
	}
	else
		res2 = 0;

	if ( (pfp_read_stderr != NULL) && (*pfp_read_stderr != NULL))
	{
		if ( (res3 = fclose (*pfp_read_stderr)) != EOF)
			* pfp_read_stderr = NULL;
	}
	else
		res3 = 0;

	if ( (res1 == EOF) || (res2 == EOF) || (res3 == EOF))
		return false;

	if (pid == 0)
		return false;

	while (waitpid (pid, &status, /*WNOHANG*/0) < 0)
	{
		if (errno != EINTR)
			return false;
		else if (return_on_signal)
			return false;
	}

	if ( (result != NULL) && WIFEXITED (status))
		*result = WEXITSTATUS (status);

	return true;
}




