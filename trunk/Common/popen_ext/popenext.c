#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>

#include <string.h>

#include <errno.h>

#include <sys/wait.h>

#include <assert.h>

#include "./../globals.h"

/*
Unix version
returns 1 on success, 0 on failure
use this one to open both an input and output pipe to the executable
*/
bool popen_ext ( char *cmd,           				/* duplicate this in argv[0] */
				 char *argv[],         				/* last argv[] must be NULL */
				 char *type,     					/* "p" for execvp() */
				 FILE **pfp_read,      				/* read file handle returned */
				 FILE **pfp_write,     				/* write file handle returned */
				 pid_t *ppid,            			/* process id returned */
				 const int *fd_to_close_in_child,        	/* [in], array of fd */
				 const int fd_to_close_in_child_count )   	/* number of valid fd */
{
	int j, pfd_read[2], pfd_write[2];

	/* Check parameters.*/
	assert ( pfp_read );
	assert ( pfp_write );
	assert ( cmd );
	assert ( argv );
	assert ( type );
	assert ( ppid );
	assert ( !fd_to_close_in_child_count || fd_to_close_in_child );
	/* ------------------------ */


	if ( !pfp_read || !pfp_write )
		return FALSE;

	*pfp_read = NULL;
	*pfp_write = NULL;

	if ( !cmd || !argv || !type || !ppid )
		return FALSE;


	errno = 0;

	if ( pipe ( pfd_read ) < 0 )
		return FALSE;

	errno = 0;

	if ( pipe ( pfd_write ) < 0 )
	{
		close ( pfd_read[0] );
		close ( pfd_read[1] );
		return FALSE;
	}

	errno = 0;

	if ( ( *ppid = fork() ) < 0 )
	{
		close ( pfd_read[0] );
		close ( pfd_read[1] );
		close ( pfd_write[0] );
		close ( pfd_write[1] );
		return FALSE;
	}

	if ( !*ppid )
	{
		/* child continues*/
		if ( STDOUT_FILENO != pfd_read[1] )
		{
			errno = 0;

			if ( dup2 ( pfd_read[1], STDOUT_FILENO ) == -1 )
			{
				close ( pfd_read[0] );
				close ( pfd_read[1] );
				close ( pfd_write[0] );
				close ( pfd_write[1] );
				return FALSE;
			}

			close ( pfd_read[1] );
		}

		close ( pfd_read[0] );

		if ( STDIN_FILENO != pfd_write[0] )
		{
			errno = 0;

			if ( dup2 ( pfd_write[0], STDIN_FILENO ) == -1 )
			{
				close ( pfd_read[0] );
				close ( pfd_read[1] );
				close ( pfd_write[0] );
				close ( pfd_write[1] );
				return FALSE;
			}

			close ( pfd_write[0] );
		}

		close ( pfd_write[1] );

		for ( j = 0; j < fd_to_close_in_child_count; j++ )
			close ( fd_to_close_in_child[j] );

		if ( strstr ( type, "p" ) )
			execvp ( cmd, argv );
		else
			execv ( cmd, argv );

		_exit ( 127 ); /* execv() failed */
	}

	/* parent continues*/
	close ( pfd_read[1] );
	close ( pfd_write[0] );

	if ( ! ( *pfp_read = fdopen ( pfd_read[0], "r" ) ) )
	{
		close ( pfd_read[0] );
		close ( pfd_write[1] );
		return FALSE;
	}

	if ( ! ( *pfp_write = fdopen ( pfd_write[1], "w" ) ) )
	{
		fclose ( *pfp_read ); /* closing this also closes pfd_read[0] */
		*pfp_read = NULL;
		close ( pfd_write[1] );
		return FALSE;
	}

	return TRUE;
}    /*pipe_open_2()*/



/*
Unix version
returns 1 on success, 0 on failure
*/
bool pclose_ext	( FILE **fp_read,		/* returned from pipe_open() */
				  FILE **fp_write,		/* returned from pipe_open() */
				  pid_t pid,			/* returned from pipe_open() */
				  int *result )			/* can be NULL */
{
	int res1, res2, status;


	/* Check parameters.*/
	assert ( fp_read );
	assert ( fp_write );
	/* ------------------------ */


	if ( result != NULL )
		*result = 255;

	if ( *fp_read != NULL )
	{
		if ( ( res1 = fclose (*fp_read ) ) != EOF )
			*fp_read = NULL;
	}
	else
		res1 = 0;

	if ( *fp_write != NULL )
	{
		if ( ( res2 = fclose ( *fp_write ) ) != EOF )
			*fp_write = NULL;
	}
	else
		res2 = 0;

	if ( res1 == EOF || res2 == EOF )
		return FALSE;

	if ( !pid )
		return FALSE;

	while ( waitpid ( pid, &status, /*WNOHANG*/0 ) < 0 )
	{
		/*if (errno != EINTR)*/
		return FALSE;
	}

	if ( ( result != NULL ) && WIFEXITED ( status ) )
		*result = WEXITSTATUS ( status );

	return TRUE;
}

