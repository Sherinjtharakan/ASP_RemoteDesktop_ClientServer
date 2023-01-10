// Client side C/C++ program to demonstrate Socket
// programming
// usage gcc client.c -o client && ./client L 8080
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define size 1024
char serverIP[15] = {0};
int port;

int commandConnection(int port)
{
  port--;
  int sock = 0, valread, client_fd, cont = 1, flag = 1;
  struct sockaddr_in serv_addr;
  char buffer[size] = {0};
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("\n Socket creation error \n");
    return -1;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);

  // Convert IPv4 and IPv6 addresses from text to binary
  // form
  if (inet_pton(AF_INET, serverIP, &serv_addr.sin_addr) <= 0)
  {
    printf("\nInvalid address/ Address not supported \n");
    return -1;
  }

retry:
  if ((client_fd = connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0)
  {
    printf("\nConnection failed...reconnecting \n");
    // printf("%d\n", port);
    goto retry;
  }
  else
    printf("connected\n");

  while (cont)
  {
    if(flag){
      printf("\n$");
      flag=0;
    }
    fgets(buffer, sizeof(buffer), stdin);
    // printf("%s\n", buffer);
    if (strcmp(buffer, "quit\n") == 0)
      cont = 0;
    send(sock, buffer, strlen(buffer), 0);
    memset(buffer, 0x00, strlen(buffer));
    valread = read(sock, buffer, size);
    printf("%s", buffer);
    memset(buffer, 0x00, strlen(buffer));
    // perror("checl");
  }

  // closing the connected socket
  close(client_fd);
  return 0;
}

int negotiatePort()
{
  int sock = 0, valread, client_fd;
  struct sockaddr_in serv_addr;
  char *init = "init";
  char buffer[size] = {0};
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("\n Socket creation error \n");
    return -1;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);

  // Convert IPv4 and IPv6 addresses from text to binary
  // form
  if (inet_pton(AF_INET, serverIP, &serv_addr.sin_addr) <= 0)
  {
    printf("\nInvalid address/ Address not supported \n");
    return -1;
  }

retry:
  if ((client_fd = connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0)
  {
    printf("\nConnection failed...reconnecting \n");
    // printf("%d\n", port);
    goto retry;
  }
  else
    printf("connected\n");

  send(sock, init, strlen(init), 0);
  // printf("init message sent\n");
  valread = read(sock, buffer, size);
  // printf("%s\n", buffer);
  // closing the connected socket
  // close(client_fd);
  if (strcmp(buffer, "server_fork_error") != 0 || strcmp(buffer, "invalid_command") != 0)
  {
    if (strchr(buffer, '.') != NULL)
    {
      memset(serverIP, 0x00, strlen(serverIP));
      strcpy(serverIP, buffer);
      if (port == 8081)
        port = 8080;
      else
        port = 8081;
      printf("redirected to %s and %d\n", serverIP, port);
      negotiatePort();
    }
    else{
      printf("%s\n",buffer);
      commandConnection(atoi(buffer));
    }
    return 0;
  }
  return 1;
}

int main(int argc, char const *argv[])
{
  if (argc < 3)
  {
    fprintf(stderr, "3 args required [progname] [IP of other server/L [localhost]] [PORT of server -[8080]/[8081]]\n");
    return 1;
  }
  else
  {

    if (strcmp(argv[1], "l") == 0 || strcmp(argv[1], "L") == 0)
      strcpy(serverIP, "127.0.0.1");
    else
      strcpy(serverIP, argv[1]);

    if (strcmp(argv[2], "8080") == 0)
      port = 8080;
    else
      port = 8081;

    negotiatePort();
  }
}
