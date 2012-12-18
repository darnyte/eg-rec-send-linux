#ifndef _NETWORK_H_INCLUDED_
#define _NETWORK_H_INCLUDED_

#include <arpa/inet.h>


#define INET4_OR_6_ADDRSTRLEN	MAXVAL(INET_ADDRSTRLEN, INET6_ADDRSTRLEN)

typedef struct t_IP_addresses_info
{
	bool filled;
	void* addressses;
} t_addresses_info;


bool Validate_And_Parse_Host_Or_IP(const char*, const uint16_t, struct t_IP_addresses_info*, int*);

bool Init_Socket_And_Bind(const struct t_IP_addresses_info*, int*, bool*, char*, const size_t, u_int16_t*);

bool Set_socket_timeouts(const int, const double, const double);

bool Set_socket_options(const int);

bool Start_listenning(const int, const int);

bool Accept_new_client(const int, int*, bool*, char*, const size_t, char**, u_int16_t*);

void Free_IP_addresses_info(struct t_IP_addresses_info*);

bool Dns_look_up_address (struct sockaddr*, char**);

inline const char* Network_strerror(const int);

#endif /* _NETWORK_H_INCLUDED_ */
