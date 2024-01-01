#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>

#define SERVER_ADDRESS "192.168.31.185" /* server IP */
#define PORT 8083
#define MAX_INPUT_LENGTH 1024
#define EXIT_COMMAND "exit"

char buf_rx[MAX_INPUT_LENGTH];

int main()
{
    int sockfd;
    struct sockaddr_in servaddr;

    /* Socket creation */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("CLIENT: socket creation failed...\n");
        return -1;
    }
    else
    {
        printf("CLIENT: Socket successfully created..\n");
    }

    memset(&servaddr, 0, sizeof(servaddr));

    /* assign IP, PORT */
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    servaddr.sin_port = htons(PORT);

    /* try to connect the client socket to server socket */
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0)
    {
        printf("connection with the server failed...\n");
        return -1;
    }

    printf("connected to the server..\n");

    while (true)
    {
        /* Get user input */
        printf("Enter data to send to the server (or type 'exit' to quit): ");
        char user_input[MAX_INPUT_LENGTH];
        fgets(user_input, sizeof(user_input), stdin);

        /* Remove trailing newline character from user input */
        user_input[strcspn(user_input, "\n")] = '\0';
	printf("Write: Received: %s \n", user_input);
	if (strcmp(user_input, EXIT_COMMAND) == 0)
        {
            printf("Exiting the client...\n");
            break; // Exit the loop
        }
        /* Send user input to the server */
        write(sockfd, user_input, strlen(user_input));

        /* Check if the user wants to exit */
        

        /* Receive and print server response */
        read(sockfd, buf_rx, sizeof(buf_rx));
        printf("CLIENT: Received: %s \n", buf_rx);
    }

    /* Close the socket */
    close(sockfd);

    return 0;
}
