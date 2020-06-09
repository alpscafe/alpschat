#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>

#define PORT "12341"
#define CMAX 2

int create_listener() 
{
	int li, yes = 1, r, bound = 0;
	struct addrinfo hints, *res, *f_res;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((r = getaddrinfo(NULL, PORT, &hints, &f_res)) != 0) {
		fprintf(stderr, "Server: %s\n", gai_strerror(r));
		exit(1);
	}

	for(res = f_res; res != NULL && !bound; res = res->ai_next) {
		li = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (li != -1) {
			if (setsockopt(li, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
				perror("setsockopt");
				exit(2);
			}
			if (bind(li, res->ai_addr, res->ai_addrlen) == -1) {
				close(li);
			} else {
				bound = 1;
			}

		}
	}

	if (res == NULL) 
		return -1;

	freeaddrinfo(f_res);

	if (listen(li, CMAX) == -1)
		return -1;

	return li;
	
}

int main()
{
	printf("ALPSCHAT SERVER v0.1\n");

	int listener_fd, nb, poll_c;

	int client_fd;
	struct sockaddr_storage client_addr;
	socklen_t addrlen;
	char data[512], client_IP[INET_ADDRSTRLEN];

	struct pollfd pfds[CMAX+1];
	int current_n = 1;

	if ((listener_fd = create_listener()) == -1) {
		fprintf(stderr, "ERR: creating a listener failed.\n");
		exit(1);
	}

	pfds[0].fd = listener_fd;
	pfds[0].events = POLLIN;

	while(current_n != CMAX+1) {
		
		if(poll(pfds, current_n, -1) == -1) {
			perror("poll");
			exit(1);
		}

		if (pfds[0].revents & POLLIN) {
			addrlen = sizeof(client_addr);
			if ((client_fd = accept(listener_fd, (struct sockaddr *)&client_addr, 
				&addrlen)) == -1)
				perror("accept");
			else {
				printf("New client joined.\n");
				pfds[current_n].fd = client_fd;
				pfds[current_n].events = POLLIN;
				current_n++;
			}
		}
	}

	printf("Everyone joined!\n");

	char greetings[] = "Welcome to the room!";
	for (int i = 1; i < CMAX+1; ++i) {
		if ((nb = send(pfds[i].fd, greetings, sizeof(greetings)+1, 0)) == -1)
			perror("send");
	}

	while(1) {
		if ((poll_c = poll(pfds, current_n, -1)) == -1) {
			perror("poll");
			exit(1);
		}
		for (int i = 1; i < CMAX+1; ++i) {
			if (pfds[i].revents & POLLIN) {
				nb = recv(pfds[i].fd, data, 500, 0);
				printf("nb: %d\n", nb);
				if (nb == 0) {
					printf("Conneciton closed.");
					exit(0);
				} else if (nb < 0) {
					perror("recv");
					exit(1);
				}
				for (int j = 1; j < CMAX+1; ++j) {
					if (pfds[j].fd != pfds[i].fd) {
						if (send(pfds[j].fd, data, nb, 0) == -1) 
							perror("send");
					}
				}
			}	
		}
	}

	return 0;
}
