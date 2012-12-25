#include <stdlib.h>
#include <unistd.h>

#include <errno.h>

#include <string.h>
#include <strings.h>

#include <pwd.h>
#include <grp.h>

#include <assert.h>

#include "./../Common/globals.h"
#include "exiterror.h"
#include "dropprivileges.h"



#define MAX_BUF_LEN 10*1024*1024


static bool get_uid_from_username (const char* username, uid_t* uid, gid_t* gid);

static bool get_gid_from_uid (uid_t uid, uid_t* euid, gid_t* gid);

static bool get_gid_from_groupname (const char* groupname, gid_t* gid, uid_t ** uid_list, size_t *uid_count);

static bool from_user_list_to_uid_list (char** names_list, uid_t ** uid_list, size_t *uid_count);


/*

*/
inline bool Do_i_have_root_privileges()
{
	return (getuid() == 0);
}

/*
Get uid and gid from a string in the form "user:group" or "user", in the latter case returns
the primary group for the user, the user must belong to the group or the function will fail.

1st parameter: String in the form "user:group" or "user".

2nd parameter: Pointer to uid_t to receive the uid.

3rd parameter: Pointer to gid_t to receive the gid.

Returns: sucecss\Fail.
*/
bool Get_uid_and_gid_from_string (const char* username, uid_t* uid, gid_t* gid, bool* isprimarygroup)
{
	size_t len = 0;
	uid_t *uid_list = NULL;
	size_t uid_count = 0;
	bool res = true;
	gid_t primary_group;
	const char* search = ":";
	char *user, *groupname;
	char* myusername;
	bool validgroup;


	/* Check pointer parameters.*/
	assert (username);
	assert (uid);
	assert (gid);
	assert (isprimarygroup);
	/* ------------------------ */


	if ( (username == NULL) || ( (len = strlen (username)) == 0))
		return false;

	if ( (myusername = malloc (len + 1)) == NULL)
		return false;

	bzero (myusername, len + 1);

	strncpy (myusername, username, len);

	user = strtok (myusername, search);

	if ( (user == NULL) || (strlen (user) == 0))
	{
		free (myusername);
		return false;
	}

	groupname = strtok (NULL, search);

	validgroup = (groupname != NULL);

	if (strtok (NULL, search) != NULL)
	{
		free (myusername);
		return false;
	}

	if (!get_uid_from_username (user, uid, &primary_group))
	{
		free (myusername);
		return false;
	}

	*gid = primary_group;
	*isprimarygroup = true;

	if (validgroup)
	{
		res = false;

		if (!get_gid_from_groupname (groupname, gid, &uid_list, &uid_count))
		{
			free (myusername);
			return false;
		}

		if (*gid != primary_group)
		{
			size_t i;

			for (i = 0; i < uid_count; i++)
			{
				if ( (*uid) == uid_list[i])
				{
					res = true;
					*isprimarygroup = false;
					break;
				}
			}
		}
		else
			res = true;

		if ( (uid_count > 0) && (uid_list != NULL))
			free (uid_list);
	}

	free (myusername);

	return res;
}


/*

*/
bool Drop_privileges (uid_t uid, gid_t gid, bool isprimarygroup)
{
	/* root user or group not allowed. */
	if ( (uid == 0) || (gid == 0))
		return false;

	/* Check for root. */
	if (Do_i_have_root_privileges())
	{
		/* process is running as root, drop privileges */

		if (isprimarygroup)
		{
			gid_t primary_group;
			uid_t euid;

			if ( (!get_gid_from_uid (uid, &euid, &primary_group)) || (uid != euid))
				return false;

			if (gid != primary_group)
				return false;

			if (setgroups (0, NULL) != 0)
				return false;
		}
		else
		{
			if (setgroups (1, &gid) != 0)
				return false;
		}

		if (setgid (gid) != 0)
			return false;

		setegid (gid);

		if (setuid (uid) != 0)
			return false;

		seteuid (uid);

		if (setuid (0) != -1)
			return false;

		if (getuid() != uid)
			return false;
	}

	return true;
}


/*

*/
static bool get_uid_from_username (const char* username, uid_t* uid, gid_t* gid)
{
	/*char* buf = NULL;*/
	/*struct passwd thepassword;*/
	struct passwd *thepassword_res = NULL;
	/*long int buf_len;*/


	/* Check pointer parameters.*/
	assert (username);
	assert (uid);
	assert (gid);
	/* ------------------------ */


	errno = 0;

	if ( (thepassword_res = getpwnam (username)) == NULL)
		return false;

	*uid = thepassword_res->pw_uid;
	*gid = thepassword_res->pw_gid;

	return true;

	/*

	if ( (buf_len = sysconf (_SC_GETGR_R_SIZE_MAX) * sizeof (char)) <= 0)
		return false;

	errno = 0;

	while ( ( (errno == ERANGE) || (buf == NULL)) && (buf_len <= MAX_BUF_LEN))
	{
		if (buf != NULL)
		{
			free (buf);
			buf = NULL;
		}

		buf = malloc (buf_len);

		if (buf == NULL)
			return false;

		memset (buf, 0x00, buf_len);

		memset (&thepassword, 0x00, sizeof (thepassword));

		errno = 0;

		if (getpwnam_r (username, &thepassword, buf, buf_len, &thepassword_res) == 0)
			break;

		buf_len = buf_len * 2;
	}

	if (thepassword_res == NULL)
	{
		free (buf);
		return false;
	}

	*uid = thepassword.pw_uid;
	*gid = thepassword.pw_gid;

	free (buf);

	return true;

	*/
}


/*

*/
static bool get_gid_from_uid (uid_t uid, uid_t* euid, gid_t* gid)
{
	/*char* buf = NULL;*/
	/*struct passwd thepassword;*/
	struct passwd *thepassword_res = NULL;
	/*long int buf_len;*/


	/* Check pointer parameters.*/
	assert (euid);
	assert (gid);
	/* ------------------------ */


	if ( (thepassword_res = getpwuid (uid)) == NULL)
		return false;

	*euid = thepassword_res->pw_uid;
	*gid = thepassword_res->pw_gid;

	return true;

	/*

	if ((buf_len = sysconf (_SC_GETGR_R_SIZE_MAX) * sizeof (char)) <= 0)
		return false;

	errno = 0;

	while ( ( (errno == ERANGE) || (buf == NULL)) && (buf_len <= MAX_BUF_LEN))
	{
		if (buf != NULL)
		{
			free (buf);
			buf = NULL;
		}

		errno = 0;

		buf = malloc (buf_len);

		if (buf == NULL)
			return false;

		memset (buf, 0x00, buf_len);

		memset (&thepassword, 0x00, sizeof (thepassword));

		errno = 0;

		if (getpwuid_r (uid, &thepassword, buf, buf_len, &thepassword_res) == 0)
			break;

		buf_len = buf_len * 2;
	}

	if (thepassword_res == NULL)
	{
		free (buf);
		return false;
	}

	*euid = thepassword.pw_uid;
	*gid = thepassword.pw_gid;

	free (buf);

	return true;

	*/
}


/*

*/
static bool get_gid_from_groupname (const char* groupname, gid_t* gid, uid_t ** uid_list, size_t *uid_count)
{
	/*char* buf = NULL;*/
	/*struct group thegroup;*/
	struct group *thegroup_res = NULL;
	/*long int buf_len;*/


	/* Check pointer parameters.*/
	assert (groupname);
	assert (gid);
	/* ------------------------ */


	errno = 0;

	if ( (thegroup_res = getgrnam (groupname)) == NULL)
		return false;

	*gid = thegroup_res->gr_gid;

	if ( (uid_list != NULL) && (uid_count != NULL))
	{
		if (!from_user_list_to_uid_list (thegroup_res->gr_mem, uid_list, uid_count))
			return false;

	}

	return true;



	/*

	if ( (buf_len = sysconf (_SC_GETGR_R_SIZE_MAX) * sizeof (char)) <= 0)
		return false;

	errno = 0;

	while ( ( (errno == ERANGE) || (buf == NULL)) && (buf_len <= MAX_BUF_LEN))
	{
		if (buf != NULL)
		{
			free (buf);
			buf = NULL;
		}

		buf = malloc (buf_len);

		if (buf == NULL)
			return false;

		memset (buf, 0x00, buf_len);

		memset (&thegroup, 0x00, sizeof (thegroup));

		errno = 0;

		if (getgrnam_r (groupname, &thegroup, buf, buf_len, &thegroup_res) == 0)
			break;

		buf_len = buf_len * 2;
	}

	if (thegroup_res == NULL)
	{
		free (buf);
		return false;
	}

	*gid = thegroup_res->gr_gid;

	if ( (uid_list != NULL) && (uid_count != NULL))
	{
		if (!from_user_list_to_uid_list (thegroup_res->gr_mem, uid_list, uid_count))
		{
			free (buf);
			return false;
		}
	}

	free (buf);

	return true;

	*/
}

static bool from_user_list_to_uid_list (char** names_list, uid_t ** uid_list, size_t *uid_count)
{
	size_t i;
	gid_t dummy;
	char** my_names_list = names_list;

	/* Check pointer parameters.*/
	assert (names_list);
	assert (uid_list);
	assert (uid_count);
	/* ------------------------ */


	*uid_list = NULL;

	*uid_count = 0;

	while (*my_names_list != NULL)
	{
		my_names_list++;
		(*uid_count) ++;
	}

	if ( (*uid_count) == 0)
		return true;

	*uid_list = calloc ( (*uid_count), sizeof (uid_t));

	if (*uid_list == NULL)
		return false;

	for (i = 0; i < (*uid_count); i++)
	{
		if (!get_uid_from_username (names_list[i], & ( (*uid_list) [i]), &dummy))
		{
			free ( (*uid_list));
			*uid_list = NULL;
			return false;
		}
	}

	return true;
}
