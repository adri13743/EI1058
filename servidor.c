#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <signal.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#define PORTTCP 8080 /* Puerto TCP */
#define PORTUDP 9009 /* Puerto UDP */
#define TAM_BUFFER 1024 /* Puerto UDP TAM BUFF */
#define BUF_SIZE   1024 /* Puerto TCP TAM BUFF */
#define BACKLOG 5 /* Max. TCP client pending connections */


int crearsockTCP (int port, struct sockaddr_in servaddr){
    /* socket creation */
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        fprintf(stderr, "[SERVER-error]: socket TCP creation failed. %d: %s \n", errno, strerror( errno ));
        return -1;
    }
    else
    {
        printf("[SERVER]: Socket TCP successfully created..\n");
    }
    /* clear structure */
    memset(&servaddr, 0, sizeof(servaddr));
 
    /* assign IP, SERV_PORT, IPV4 */
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(port);
    /* Bind socket */
    if ((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
    {
        fprintf(stderr, "[SERVER-error]: socket TCP bind failed. %d: %s \n", errno, strerror( errno ));
        return -1;
    }
    else
    {
        printf("[SERVER]: Socket TCP successfully binded \n");
    }
 
    /* Listen */
    if ((listen(sockfd, BACKLOG)) != 0)
    {
        fprintf(stderr, "[SERVER-error]: socket TCP listen failed. %d: %s \n", errno, strerror( errno ));
        return -1;
    }
    else
    {
        printf("[SERVER]: Listening TCP on SERV_PORT %d \n\n", ntohs(servaddr.sin_port) );
    }
    return sockfd;
}


int crearSocketUDP(const char *serverIP, int port, struct sockaddr_in servidorAddr) {
    int udpSocket;
    if ((udpSocket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("Error al crear el socket UDP");
        exit(-1);
    }


    memset(&servidorAddr, 0, sizeof(servidorAddr));
    servidorAddr.sin_family = AF_INET;
    servidorAddr.sin_port = htons(port);
   
    if (inet_pton(AF_INET, serverIP, &(servidorAddr.sin_addr)) <= 0) {
        perror("Error al convertir la dirección IP");
        exit(-1);
    }


    if (connect(udpSocket, (struct sockaddr *)&servidorAddr, sizeof(servidorAddr)) == -1) {
        perror("Error al enlazar el socket UDP");
        fprintf(stderr, "Error en bind %s\n", strerror(errno));
        fprintf(stderr, "IP: %s, Puerto: %d\n", serverIP, PORTUDP);
        exit(-1);
    }
    return udpSocket;
}


void main (int argc, char *argv[]){
    const char *nombreArchivo = "historial.txt";
    // Intentar abrir el archivo en modo lectura
    FILE *archivo = fopen(nombreArchivo, "r");
    if (archivo == NULL) {// Verificar si el archivo existe
        // El archivo no existe, así que intentamos crearlo
        archivo = fopen(nombreArchivo, "w");
        // Verificar si la creación fue exitosa
        if (archivo == NULL) {
            perror("[SERVER-error] Error al crear el archivo .txt");
            return 1; // Código de error
        }
        printf("[SERVER] Archivo 'historial.txt' creado exitosamente.\n");
        // Cerrar el archivo después de crearlo
        fclose(archivo);
    } else {
        // El archivo ya existe, no hacemos nada
        printf("[SERVER] El archivo 'historial.txt' ya existe.\n");
        // Cerrar el archivo si estaba abierto en modo lectura
        fclose(archivo);
    }

    //sem_t sem;
    /* Servidor Internet en el puerto TCP número 4321 */
    int sock_escucha_tcp, sock_service_tcp;
    struct sockaddr_in servaddr, client; /* TCP */
    int  len_rx, len_tx = 0;
    char buff_tx[BUF_SIZE];
    char buff_rx[BUF_SIZE];   /* buffers for reception  */
    int port_tcp = PORTTCP;
    struct sockaddr_in servidorAddrUDP;
    char buffer[TAM_BUFFER];
    /* Servidor Internet en el puerto UDP número 1234 */
    const char *serverIP = "192.168.31.132"; /* Cambiar con la ip del ESP32*/
    int esp32Socket; /* UDP */
    if ((esp32Socket = crearSocketUDP(serverIP, PORTUDP, servidorAddrUDP)) == -1 ) {
    fprintf(stderr, "Fallo en la creación/conexión del socket udp ESP32 \n");
    exit(-1);
    }
    if ((sock_escucha_tcp = crearsockTCP(port_tcp, servaddr)) == -1) {
            fprintf(stderr, "Fallo en la creación del socket tcp de escucha \n");
            exit(-1);
    }
    //if(sem_init(&sem, 0, 1) == -1){
    	//perror("Error al crear el semaforo");
    	//exit(EXIT_FAILURE);
    //}
    int tub[2];
    pipe(tub);
    if (fork() == 0){
        /* hijo gestiona escuchas tcp y redirecciona a nietos*/
        while(1){
            int len = sizeof(client);  
            sock_service_tcp = accept(sock_escucha_tcp, (struct sockaddr *)&client, &len);
            if (sock_service_tcp < 0){
                fprintf(stderr, "[hijo SERVER-error]: connection not accepted. %d: %s \n", errno, strerror( errno ));
                close(sock_service_tcp);
                close(sock_escucha_tcp);
                exit(-1);
            }
            if (fork() == 0){
                /* nieto */
                close(tub[0]);/* nieto no usa tuberia para leer */
                len_rx = 1;
                int sesion = 0;
                while(len_rx != 0){  /* read data from a client socket till it is closed */  
                    /* read client message, copy it into buffer */
                    len_rx = read(sock_service_tcp, buff_rx, sizeof(buff_rx));  
                    if(len_rx == -1)
                    {
                        fprintf(stderr, "[nieto, SERVER-error]: sock_service_tcp cannot be read. %d: %s \n", errno, strerror( errno ));
                        break;
                    }else if (len_rx == 0) {
        		// El socket se cerró, salir del bucle
        		break;
        	    }else{
                    	printf("SERVER:Received: %s \n", buff_rx);
                        if(sesion == 0){ /* primer envio comprobar logg */
                            /* falta if comprobando si loggin es correcto */
                            sesion = 1;
                            strcpy(buff_tx, "Logg correcto");
                            write(sock_service_tcp, buff_tx, sizeof(buff_tx));    
                        }else{
                            /* si logg ok abro puerta ? */
                            const char *message = "quien es el cliente y que puerta abre";
                            // Escribir en la tubería si abro puerta
                            ssize_t bytesWritten = write(tub[1], message, strlen(message));
                            if (bytesWritten == -1) {
                                perror("[nieto] Error en la escritura en la tubería");
                            }else{
                            	strcpy(buff_tx, "puerta abierta");
                            	write(sock_service_tcp, buff_tx, sizeof(buff_tx)); 
                            	//sem_post(&sem);
                            }
                        }
                    }            
                }
                /* termina funciones */
                close(sock_service_tcp);
                close(tub[1]);
                /* termina cliente y nieto */  
                exit(0);
            }
        }
    }else{
        /* padre */
        close(tub[1]);/* padre no usa tuberias para escribir */
        close(sock_escucha_tcp);
        while(1){
            ssize_t bytesRead = read(tub[0], buff_tx, sizeof(buff_tx));
            if (bytesRead == -1) {
                perror("[padre] Error en la lectura de la tubería");
                exit(EXIT_FAILURE);
            }else{
                /* escribo en un txt cliente , puerta abiuerta y fecha , hora, min */
                FILE *archivo = fopen(nombreArchivo, "a");
                if (archivo == NULL) {
                    perror("[padre] Error al abrir el archivo");
                    
                }
                fputs(buff_tx, archivo);
                fclose(archivo);
                printf("[padre] Mensaje recibido del nieto: %s\n", buff_tx);
            }
        }


    }
}









