#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>

#define PORT "12341" 
#define MAXDATASIZE 500 

void print_logo()
{
    printf("                            .-.    .  _   *     _   .                   \n");
    printf("                *          /   \\     ((       _/ \\       *    .         \n");
    printf("              _    .   .--'\\/\\_ \\     `      /    \\  *   ___            \n");
    printf("          *  / \\_    _/ ^      \\/\\'__       /\\/\\  /\\  __/   \\ *         \n");
    printf("            /    \\  /    .'   _/  /  \\  *' /    \\/  \\/ .`'\\_/\\   .      \n");
    printf("       .   /\\/\\  /\\/ :' __  ^/  ^/    `--./.'  ^  `-.\\ _    _:\\ _       \n");
    printf("          /    \\/  \\  _/  \\-' __/.' ^ _   \\_   .'\\   _/ \\ .  __/ \\      \n");
    printf("        /\\  .-   `. \\/     \\ / -.   _/ \\ -. `_/   \\ /    `._/  ^  \\     \n");
    printf("       /  `-.__ ^   / .-'.--'    . /    `--./ .-'  `-.  `-. `.  -  `.   \n");
    printf("      /        `.  / /      `-.   /  .-'   / .   .'   \\    \\  \\  .-  \\  \n");
    printf("\n");
    printf("\t        '||                          '||                .   \n");
    printf("\t ....    ||  ... ...   ....    ....   || ..    ....   .||.  \n");
    printf("\t'' .||   ||   ||'  || ||. '  .|   ''  ||' ||  '' .||   ||   \n");
    printf("\t.|' ||   ||   ||    | . '|.. ||       ||  ||  .|' ||   ||   \n");
    printf("\t'|..'|' .||.  ||...'  |'..|'  '|...' .||. ||. '|..'|'  '|.' \n");
    printf("\t              ||                                            \n");
    printf("\t             ''''                                           \n");
    printf("\n");
}

int main(int argc, char *argv[])
{
    print_logo();
    int sockfd, nbytes, connected = 0;
    char *data = malloc(MAXDATASIZE);
    struct addrinfo hints, *res, *tmp;
    ssize_t read = MAXDATASIZE;

    char s[INET_ADDRSTRLEN];

    if (argc != 2) {
        fprintf(stderr, "usage: client host");
        exit(1);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(argv[1], PORT, &hints, &res) != 0) {
        fprintf(stderr, "ERR: getaddrinfo\n");
        exit(1);
    }

    for(tmp = res; tmp != NULL; tmp = tmp->ai_next) {
        if ((sockfd = socket(tmp->ai_family, tmp->ai_socktype, tmp->ai_protocol))
            == -1) {
                perror("client: socket");
                continue;
            }

        if (connect(sockfd, tmp->ai_addr, tmp->ai_addrlen) == -1) {
            close(sockfd);
            perror("sockfd");
            continue;
        }
        break;
    }

    if (tmp == NULL) {
        fprintf(stderr, "client: unable to connect\n");
        exit(2);
    }

    printf("Connected \n");

    //freeaddrinfo(res);

    if ((nbytes = recv(sockfd, data, 200, 0)) == -1) {
        perror("recv");
        exit(1);
    }

    printf("%s\n", data);

    switch (fork()){
    case -1:
        perror("fork");
        exit(1);
        break;
    case 0:
        while (1) {
            read = getline(&data, &read, stdin);
            if (send(sockfd, data, 200, 0) == -1) {
                perror("send");
                exit(1);
            }
        }
        break;
    default:
        while(1) {
            if ((nbytes = recv(sockfd, data, 200, 0)) == -1) {
                perror("recv");
                exit(1);
            } else if (nbytes == 0) {
                printf("Connection closed.\n");
                exit(0);
            } else {
                data[nbytes] = '\0';
                printf("%s\n", data);
            }
        }
        break;
    }
    return 0;
}