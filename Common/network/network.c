#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>

#include <limits.h>

#include <errno.h>

#include <string.h>

#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <assert.h>

#include "./../globals.h"
#include "network.h"


#define MAX_DNS_LOOK_TRIES	10
#define MIN_HOST_NAME		16

/*

*/
static void Free_IP_addreses (struct addrinfo * IP_adresses);

static void* Get_in_addr_from_sockaddr (struct sockaddr* addr);

static socklen_t Get_addr_size_from_sockaddr (struct sockaddr* addr);

static u_int16_t Get_family_from_sockaddr (struct sockaddr* addr);

static u_int16_t Get_port_from_sockaddr (struct sockaddr* addr);




/*
Validates a string representing an IP address or a host name and a port number or service name.

1er parámetro: Puntero al comienzo del string.

3er parámetro: Devuelve el número.


devuelve: Si hubo o no error.
*/
bool Validate_And_Parse_Host_Or_IP (const char *ipAddress, const uint16_t tcp_port,
									struct t_IP_addresses_info* IP_adresses, int* net_errorno)
{
	struct addrinfo hints;
	char str_tcp_port[MAX_VALID_PORT_STR_LEN + 1];
	int res;

	/* Check pointer parameters.*/
	/*assert (ipAddress);*/
	assert (IP_adresses);
	assert (net_errorno);
	/* ------------------------ */


	res = snprintf (str_tcp_port, sizeof (str_tcp_port), "%lu", (long unsigned int) tcp_port);

	if ( (res < 1) || (res >= (int) sizeof (str_tcp_port)))
		return false;

	memset (&hints, 0, sizeof (hints));

	hints.ai_family = AF_UNSPEC;                    					/* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_STREAM;                					/* Datagram socket */
	hints.ai_flags =
		AI_ADDRCONFIG | AI_PASSIVE /* | AI_CANONNAME | AI_CANONIDN*/ ;  /* For wildcard IP address and ... */
	hints.ai_protocol = 0;                          					/* Any protocol */
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	Free_IP_addresses_info (IP_adresses);

	*net_errorno = getaddrinfo (ipAddress, str_tcp_port,
								&hints, (struct addrinfo **) &IP_adresses->addressses);

	if (*net_errorno == 0)
	{
		IP_adresses->filled = true;

		return true;
	}
	else
	{
		memset (IP_adresses, 0x00, sizeof (*IP_adresses));

		return false;
	}
}


/*

*/
bool Init_Socket_And_Bind (const struct t_IP_addresses_info* IP_adresses, int* socket_fd, bool* is_IPV6,
						   char* str_IP, const size_t str_IP_buff_len, u_int16_t* tcp_port)
{
	struct addrinfo *addr = NULL;


	/* Check pointer parameters.*/
	assert (IP_adresses);
	assert (socket_fd);
	assert (is_IPV6);
	assert (tcp_port);
	/* ------------------------ */


	*socket_fd = -1;

	for (addr = (struct addrinfo *) IP_adresses->addressses; addr != NULL; addr = addr->ai_next)
	{
		/* create sockect. */
		*socket_fd = socket (addr->ai_family, addr->ai_socktype,
							 addr->ai_protocol);
		if (*socket_fd < 0)
			continue;

		if (bind (*socket_fd, addr->ai_addr, addr->ai_addrlen) == 0)
			break;

		if (close (*socket_fd) != 0)
		{
			/* Failed to close the socket. */
			addr = NULL;
			break;
		}

		*socket_fd = -1;
	}

	if (addr != NULL)
	{
		void* addr_in = NULL;

		*is_IPV6 = (addr->ai_family == AF_INET6);

		*tcp_port = Get_port_from_sockaddr (addr->ai_addr);

		if ( (addr->ai_family == Get_family_from_sockaddr (addr->ai_addr)) &&
				( (addr_in = Get_in_addr_from_sockaddr (addr->ai_addr)) != NULL))
		{
			if (inet_ntop (addr->ai_family, addr_in, str_IP, str_IP_buff_len) == NULL)
			{
				bzero (str_IP, str_IP_buff_len);
			}
		}
	}

	return (addr != NULL);
}


/*

*/
bool Set_socket_timeouts (const int socket_fd, const double read_timeout, const double write_timeout)
{
	struct timeval time;

	if (socket_fd < 0)
		return false;

	time.tv_sec = read_timeout;
	time.tv_usec = ( (read_timeout - (double) time.tv_sec) * 1000000);

	errno = 0;

	if (setsockopt (socket_fd, SOL_SOCKET, SO_RCVTIMEO, &time, sizeof (struct timeval)) != 0)
		return false;


	time.tv_sec = write_timeout;
	time.tv_usec = ( (write_timeout - (double) time.tv_sec) * 1000000);

	errno = 0;

	return (setsockopt (socket_fd, SOL_SOCKET, SO_SNDTIMEO, &time, sizeof (struct timeval)) == 0);
}


/*

*/
bool Set_socket_options (const int socket_fd)
{
#ifdef DEBUG
	int optval = 1;
#endif

	if (socket_fd < 0)
		return false;

#ifdef DEBUG
	return (setsockopt (socket_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof (optval)) == 0);
#else
	return true;
#endif
}


/*

*/
bool Start_listenning (const int socket_fd, const int max_connections)
{
	errno = 0;

	return (listen (socket_fd, max_connections) == 0);
}


/*

*/
bool Accept_new_client (const int socket_fd, int* socket_new_fd, bool* is_IPV6, char* str_IP,
						const size_t str_IP_buff_len, char** str_host_name, u_int16_t* tcp_port)
{
	struct sockaddr_storage cli_addr_buffer;

	socklen_t cli_len =  sizeof (cli_addr_buffer);

	struct sockaddr* paddr = NULL;

	bool failed = true;


	/* Check parameters.*/
	assert (socket_new_fd);
	assert (socket_fd != -1);
	assert (is_IPV6);
	assert (tcp_port);
	/* ------------------------ */


	memset (&cli_addr_buffer, 0x00, cli_len);

	errno = 0;

	*socket_new_fd = accept (socket_fd, (struct sockaddr *) &cli_addr_buffer, &cli_len);

	failed = (*socket_new_fd < 0);

	if (!failed)
	{
		void* addr_in = NULL;

		paddr = (struct sockaddr *) &cli_addr_buffer;

		*is_IPV6 = (paddr->sa_family == AF_INET6);

		*tcp_port = Get_port_from_sockaddr (paddr);

		if ( (addr_in = Get_in_addr_from_sockaddr (paddr)) != NULL)
		{
			if (inet_ntop (paddr->sa_family, addr_in, str_IP, str_IP_buff_len) == NULL)
			{
				bzero (str_IP, str_IP_buff_len);
			}
		}


		if (! Dns_look_up_address (paddr, str_host_name))
			Free_string (str_host_name);
	}

	return (!failed);
}


/*

*/
void Free_IP_addresses_info (struct t_IP_addresses_info* IP_adresses)
{


	/* Check pointer parameters.*/
	assert (IP_adresses);
	/* ------------------------ */


	if (IP_adresses->filled)
		Free_IP_addreses ( (struct addrinfo *) IP_adresses->addressses);

	IP_adresses->addressses = NULL;
	IP_adresses->filled = false;
}


/*

*/
bool Dns_look_up_address (struct sockaddr* addr, char** str_name)
{
	socklen_t len;
	size_t name_buffer_len =  MINVAL ( (size_t) (NI_MAXHOST / 64), MIN_HOST_NAME);
	char* new_buff;
	int errnum;
	bool res, retry;
	size_t retry_count = MAX_DNS_LOOK_TRIES;


	/* Check parameters.*/
	assert (addr);
	assert (str_name);
	/* ------------------------ */
	assert (name_buffer_len);


	len = Get_addr_size_from_sockaddr (addr);

	if (len == 0)
		return false;

	errno = 0;

	*str_name = malloc (name_buffer_len + 1);

	if (*str_name == NULL)
		return false;

	do
	{
		bzero (*str_name, name_buffer_len + 1);

		errno = 0;

		res = ( (errnum = getnameinfo (addr, len,
									   *str_name, name_buffer_len, NULL, 0,
									   NI_NAMEREQD | NI_IDN | NI_IDN_USE_STD3_ASCII_RULES)) == 0) ;

		retry_count--;

		retry = (!res && ( (errnum == EAI_OVERFLOW) || (errnum == EAI_AGAIN)) && (retry_count > 0));

		if (retry && (errnum == EAI_OVERFLOW))
		{
			name_buffer_len *= 2;

			new_buff = realloc (*str_name, name_buffer_len + 1);

			if (new_buff == NULL)
			{
				Free_string (str_name);
				return false;
			}

			*str_name = new_buff;
		}
		else if (!res)
			Free_string (str_name);
	}
	while (retry);

	return res;
}


/*

*/
inline const char* Network_strerror (const int network_errorno)
{
	return gai_strerror (network_errorno);
}


/*

*/
static void Free_IP_addreses (struct addrinfo * IP_adresses)
{
	if (IP_adresses != NULL)
	{
		freeaddrinfo (IP_adresses);
	}
}


/*

*/
static void* Get_in_addr_from_sockaddr (struct sockaddr* addr)
{
	/* Check parameters.*/
	assert (addr);
	/* ------------------------ */

	if (addr->sa_family == AF_INET)
	{
		struct sockaddr_in * addrv4 = (struct sockaddr_in *) addr;
		return & (addrv4->sin_addr);
	}
	else if (addr->sa_family == AF_INET6)
	{
		struct sockaddr_in6 * addrv6 = (struct sockaddr_in6 *) addr;
		return & (addrv6->sin6_addr);
	}
	else
		return NULL;
}


/*

*/
static socklen_t Get_addr_size_from_sockaddr (struct sockaddr* addr)
{


	/* Check parameters.*/
	assert (addr);
	/* ------------------------ */


	if (addr->sa_family == AF_INET)
		return sizeof (struct sockaddr_in);
	else if (addr->sa_family == AF_INET6)
		return sizeof (struct sockaddr_in6);
	else
		return 0;
}


/*

*/
static u_int16_t Get_family_from_sockaddr (struct sockaddr* addr)
{
	/* Check parameters.*/
	assert (addr);
	/* ------------------------ */

	return addr->sa_family;
}


/*

*/
static in_port_t Get_port_from_sockaddr (struct sockaddr* addr)
{
	/* Check parameters.*/
	assert (addr);
	/* ------------------------ */

	if (addr->sa_family == AF_INET)
	{
		struct sockaddr_in * addrv4 = (struct sockaddr_in *) addr;
		return ntohs (addrv4->sin_port);
	}
	else if (addr->sa_family == AF_INET6)
	{
		struct sockaddr_in6 * addrv6 = (struct sockaddr_in6 *) addr;
		return ntohs (addrv6->sin6_port);
	}
	else
		return 0;
}


