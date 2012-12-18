#ifndef _DROPPRIVILEGES_H_INCLUDED_
#define _DROPPRIVILEGES_H_INCLUDED_

inline bool Do_i_have_root_privileges();

bool Get_uid_and_gid_from_string(const char*, uid_t*, gid_t*, bool*);

bool Drop_privileges(uid_t uid, gid_t gid, bool);

#endif /* _DROPPRIVILEGES_H_INCLUDED_ */
