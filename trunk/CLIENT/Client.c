#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <netdb.h>

#include "./../Common/md5/md5.h"



/*//////////////////// Constantes globales del programa - BEGIN - ///////////////////////*/
/*///////////////////////////////////////////////////////////////////////////////////////*/

#define false 0
#define true 1


/*///////////////////// Constantes globales del programa - END - ////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////////////////*/


/*/////////////////////// Tipos globales del programa - BEGIN - /////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////////////////*/

typedef unsigned char byte;
typedef unsigned short bool;

/*/////////////////////// Tipos globales del programa - END - ///////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////////////////*/


#define LF_CHAR '\n'
#define CR_CHAR '\r'
#define NULL_CHAR '\0'


#define SALT_SEPARATOR ":"

#define MAGIC_WORD "quintessence\n\r"

#define ACCEPT_MSG "accept"

#define CLOSE_MSG "close"

#define BUFFER_SIZE 513


/*

*/
bool Read_line(int socket_fd, char* line, size_t buffer_line_len, bool read_CR, size_t* line_len)
{
	int byte_count;
	char *pnewline_char, *pcrr_char;

	bzero(line, buffer_line_len);

	byte_count = read(socket_fd, line, buffer_line_len - 1);

	if(byte_count < 0)
		return false;

	pcrr_char = strchr(line, LF_CHAR);

	if(pcrr_char == NULL)
		return false;

	*line_len = pcrr_char - line;

	if(read_CR)
	{
		pnewline_char = pcrr_char + 1;

		if(*pnewline_char != CR_CHAR)
			return false;

		*pnewline_char = NULL_CHAR;
	}

	*pcrr_char = NULL_CHAR;

	return true;
}


/*

*/
static bool Write_line(int socket_fd, char* line, bool write_CR)
{
	int byte_count;
	char* buffer = NULL;
	size_t len;


	len = strlen(line);

	buffer = malloc(len + 2 * sizeof(char) + 1);

	bzero(buffer, len);

	strncpy(buffer, line, len);

	buffer[len] = LF_CHAR;

	buffer[len + 1] = NULL_CHAR;

	if(write_CR)
	{
		buffer[len + 1] = CR_CHAR;
		buffer[len + 2] = NULL_CHAR;
	}

	len = strlen(buffer);

	byte_count = write(socket_fd, buffer, len);

	free(buffer);

	if(byte_count != (int) len)
		return false;

	return true;
}


/*

*/
static bool Hash_salted_password(const char* password, const char* salt, char* hash, size_t hash_len)
{
	md5_state_t state;
	md5_byte_t digest[16];
	int di;

	md5_init(&state);
	md5_append(&state, (const md5_byte_t *) salt, strlen(salt));
	md5_append(&state, (const md5_byte_t *) SALT_SEPARATOR, strlen(SALT_SEPARATOR));
	md5_append(&state, (const md5_byte_t *) password, strlen(password));
	md5_finish(&state, digest);

	bzero(hash, hash_len);

	for(di = 0; di < 16; ++di)
		snprintf(hash + di * 2, 3, "%02x", digest[di]);

	return true;
}


int main(int argc, char *argv[])
{
    int sockfd, portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;

	char password[60] = "bakaneko";

    char line[BUFFER_SIZE];
    size_t line_len;

    char hash[16*2 + 1];


    if (argc < 3)
    {
        fprintf(stderr,"usage %s hostname port\n", argv[0]);
        exit(0);
    }
    portno = atoi(argv[2]);
    /* Create a socket point */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("ERROR opening socket");
        exit(1);
    }
    server = gethostbyname(argv[1]);
    if (server == NULL)
    {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;

    bcopy((char *) server->h_addr_list[0],
          (char *) &serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(portno);

    /* Now connect to the server */
    if (connect(sockfd, (const struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("ERROR connecting");
        exit(1);
    }



    /* Send Magic word to the server */
    if (!Write_line(sockfd, MAGIC_WORD, true))
    {
        perror("ERROR writing to socket");
        exit(1);
    }


	bzero(line, sizeof(line));


	if(!Read_line(sockfd, line, BUFFER_SIZE, false, &line_len))
    {
        perror("ERROR reading from socket");
        exit(1);
    }


	if(!Hash_salted_password(password, line, hash, sizeof(hash)))
    {
        perror("ERROR calculating hash");
        exit(1);
    }


    /* Send Hash */
    if (!Write_line(sockfd, hash, false))
    {
        perror("ERROR writing to socket");
        exit(1);
    }

	bzero(line, sizeof(line));

	/* Read accept */
	if(!Read_line(sockfd, line, BUFFER_SIZE, false, &line_len))
    {
        perror("ERROR reading accept confirmation");
        exit(1);
    }

    /* Send Hash */
    if (!Write_line(sockfd, "HOLA", false))
    {
        perror("ERROR writing event");
        exit(1);
    }

    /* Send Hash */
    if (!Write_line(sockfd, "close", false))
    {
        perror("ERROR writing close command");
        exit(1);
    }


    exit(0);
}
