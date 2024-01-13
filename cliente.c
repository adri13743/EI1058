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
#define PORT 8080
#define MAX_INPUT_LENGTH 1024
#define EXIT_COMMAND "salir"

char buf_rx[MAX_INPUT_LENGTH];

int main()
{
    int sockfd;
    struct sockaddr_in servaddr;

    /* Creacion del Socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("CLIENTE: error al crear el socket...\n");
        return -1;
    }
    else
    {
        printf("CLIENTE: Socket creado correctamente..\n");
    }

    memset(&servaddr, 0, sizeof(servaddr));

    /* asignar IP, PORT */
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    servaddr.sin_port = htons(PORT);

    /* Intentando conectar con el servidor */
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0)
    {
        printf("Conexion con el server fallida...\n");
        printf("Saliendo del programa\n");
        return -1;
    }

    printf("Conectado con el servidor..\n");

    while (true)
    {
        printf("Introduce: 'IpPuerta-Usuario-Contraseña' ponlo separado con el - (o introduce 'salir' para terminar): ");
        char user_input[MAX_INPUT_LENGTH];
        fgets(user_input, sizeof(user_input), stdin);
        user_input[strcspn(user_input, "\n")] = '\0';
        printf("Has escrito: %s \n", user_input);

        if (strcmp(user_input, EXIT_COMMAND) == 0)
        {
            printf("Saliendo del programa... \n");
            /* Cerrar el socket */
            close(sockfd);
            return 0;
        }
		
        /* Enviar daros al servidor */
        write(sockfd, user_input, strlen(user_input));

        /* Respuesta del servidor */
        read(sockfd, buf_rx, sizeof(buf_rx));
        //printf("CLiente: Recivo del server: %s \n", buf_rx);

        /* Check if the user provided correct credentials */
        if (strcmp(buf_rx, "Logg y UDP correctos") == 0)
        {
            printf("Bienvenido. Puedes abrir la puerta o cerrar la sesión.\n");
            int x = 0;
            while (x==0)
            {
                printf("Opciones: 'abrir puerta', 'cerrar sesion': ");
                fgets(user_input, sizeof(user_input), stdin);
                user_input[strcspn(user_input, "\n")] = '\0';
                /* Enviar daros al servidor */
                if (strcmp(user_input, "abrir puerta") == 0 || strcmp(user_input, "cerrar sesion") == 0 )
                {
		        write(sockfd, user_input, strlen(user_input));

		        /* Respuesta del servidor */
		        read(sockfd, buf_rx, sizeof(buf_rx));
		        
		        /* Si quieres abrir la puerta*/
		        if (strcmp(buf_rx, "puerta abierta") == 0)
		        {
		            printf("Puerta abierta. \n");
		        }
		        else if (strcmp(buf_rx, "sesion cerrada") == 0)
		        {
		            printf("Sesión cerrada. Volviendo al inicio...\n");
		            x=1;
		        }     
                }else{
                	printf("Error al escribir la opcion.\n");
                }
                
                //printf("CLiente: Recivo del server: %s \n", buf_rx);

            }
        }else{
            printf("Error al escribir 'IpPuerta-Usuario-Contraseña'. \n");
        }
    }

    /* Cerrar el socket */
    close(sockfd);
    return 0;
}
