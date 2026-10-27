#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/select.h>
#include <stdio.h>

fd_set	all_fds, read_fds;
int		sockfd, next_id, max_fd;
int		client_ids[1024];
char	*client_bufs[1024];

void handle_accept();
void handle_close(int fd);
void init_server();
void exit_error();
void handle_read(int fd, char *buf);
void broadcast(int current_fd, char *msg);
int extract_message(char **buf, char **msg);
char *str_join(char *buf, char *add);

int main (int argc, char **argv)
{
	if (argc != 2)
	{
		write(STDERR_FILENO, "Wrong number of arguments\n", strlen("Wrong number of arguments\n"));
		exit(1);
	}
	struct sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1)
		exit_error();
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(2130706433);
	servaddr.sin_port = htons(atoi(argv[1]));
	if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
		exit_error();
	if (listen(sockfd, 10) != 0)
		exit_error();
	init_server();
	while(1)
	{
		read_fds = all_fds;
		if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0)
			continue;
		for (int fd = 3; fd <= max_fd; fd++)
		{
			if (FD_ISSET(fd, &read_fds) != 0)
			{
				if (fd == sockfd)
				{
					handle_accept();
					continue;
				}
				else
				{
					char buf[1024];
					int n = recv(fd, buf, sizeof(buf) - 1, 0);
					if (n <= 0)
						handle_close(fd);
					else
					{
						buf[n] = '\0';
						handle_read(fd, buf);
					}
				}
			}
		}
	}
	return (0);
}

void exit_error()
{
	write(STDERR_FILENO, "Fatal error\n", strlen("Fatal error\n"));
	exit(1);
}

void init_server()
{
	FD_ZERO(&all_fds);
	FD_SET(sockfd, &all_fds);
	next_id = 0;
	max_fd = sockfd;
	for (int i = 0; i < 1024; i++)
	{
		client_ids[i] = -1;
		client_bufs[i] = NULL;
	}
}

void handle_accept()
{
	int client_fd = accept(sockfd, NULL, NULL);
	if (client_fd < 0)
		return;
	FD_SET(client_fd, &all_fds);
	client_ids[client_fd] = next_id++;
	if (client_fd > max_fd)
		max_fd = client_fd;
	char msg[64];
	sprintf(msg, "server: client %d just arrived\n", client_ids[client_fd]);
	broadcast(client_fd, msg);
}

void handle_close(int fd)
{
	char msg[64];
	sprintf(msg, "server: client %d just left\n", client_ids[fd]);
	broadcast(fd, msg);
	FD_CLR(fd, &all_fds);
	close(fd);
	client_ids[fd] = -1;
	free(client_bufs[fd]);
	client_bufs[fd] = NULL;
	if (fd == max_fd)
	{
		while(max_fd > 0 && !FD_ISSET(max_fd, &all_fds))
			max_fd--;
	}
}
void broadcast(int current_fd, char *msg)
{
	for (int fd = 3; fd <= max_fd; fd++)
	{
		if (FD_ISSET(fd, &all_fds) && fd != current_fd && fd != sockfd)
			send(fd, msg, strlen(msg), MSG_NOSIGNAL);
	}
}

void handle_read(int fd, char *buf)
{
	client_bufs[fd] = str_join(client_bufs[fd], buf);
	if(client_bufs[fd] == NULL)
		exit_error();
	char *msg;
	int ret;
	while ((ret = extract_message(&client_bufs[fd], &msg)) == 1)
	{
		char *full_msg = malloc(strlen(msg) + 64);
		if(!full_msg)
			exit_error();
		sprintf(full_msg, "client %d: %s", client_ids[fd], msg);
		broadcast(fd, full_msg);
		free(full_msg);
		free(msg);
		msg = NULL;
	}
	if (ret < 0)
		exit_error();
	return;
}

int extract_message(char **buf, char **msg)
{
	char	*newbuf;
	int	i;

	*msg = 0;
	if (*buf == 0)
		return (0);
	i = 0;
	while ((*buf)[i])
	{
		if ((*buf)[i] == '\n')
		{
			newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
			if (newbuf == 0)
				return (-1);
			strcpy(newbuf, *buf + i + 1);
			*msg = *buf;
			(*msg)[i + 1] = 0;
			*buf = newbuf;
			return (1);
		}
		i++;
	}
	return (0);
}

char *str_join(char *buf, char *add)
{
	char	*newbuf;
	int		len;

	if (buf == 0)
		len = 0;
	else
		len = strlen(buf);
	newbuf = malloc(sizeof(*newbuf) * (len + strlen(add) + 1));
	if (newbuf == 0)
		return (0);
	newbuf[0] = 0;
	if (buf != 0)
		strcat(newbuf, buf);
	free(buf);
	strcat(newbuf, add);
	return (newbuf);
}