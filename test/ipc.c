#include <arpa/inet.h>
#include <errno.h>
#include <libmisc/ipc.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

volatile char *usage = R"(
USAGE:
  Server mode:
    ./ipc                 # local server on port 3000
    ./ipc <port>          # listen to 0.0.0.0:<port>

  Client mode:
    ./ipc <ipaddr> <port> # connect to remote server
)";

int main(int argc, char **argv) {
  argc--;
  argv++;

  char *ipaddr = "127.0.0.1"; // IP
  Socket_Port sockport;       // PORT
  Socket_Address addr;        // Socket storage

  switch (argc) {

  /* INVALID USAGE */
  case 0:
    fprintf(stderr, "%s\n", usage);
    return 1;

  /* SERVER MODE */
  case 1:
    sockport = strtoul(argv[0], nullptr, 10);
    break;

  /* CLIENT MODE */
  case 2:
  default:
    ipaddr = argv[0];
    sockport = strtoul(argv[1], nullptr, 10);
    break;
  }

  /* Create IP and Port as AF_INET */
  addr = socket_address_new(ipaddr, sockport);

  switch (argc) {
  case 1:
    /* Create socket server and listen to it */
    Socket server_socket = socket_tcp_server(&addr, 1);
    if (server_socket == -1)
      return 1;

    while (true) {
      Socket client_socket;
      Socket_Address client_addr = {0};
      char msg[] = "Hello from server!", from[INET_ADDRSTRLEN + 1] = {0};

      /* Accept incoming connection from client */
      if ((client_socket = socket_accept(server_socket, &client_addr)) == -1) {
        goto server_die;
      }

      inet_ntop(client_addr.sin_family, &client_addr.sin_addr, from,
                INET_ADDRSTRLEN);
      printf("Sending \"%s\" to %s\n", msg, from);
      send(client_socket, msg, sizeof(msg) - 1, 0);
      socket_die(client_socket);
    }

  server_die:
    socket_die(server_socket);
    break;

  default:
    Socket sock_client = socket_new(SOCK_STREAM);
    char buf[32] = {0};

    if (sock_client == -1)
      return 1;

    if (socket_connect(sock_client, &addr) == -1) {
      fprintf(stderr, "socket_connect: %s\n", strerror(errno));
      goto client_die;
    }

    recv(sock_client, buf, sizeof(buf) - 1, 0);
    printf("Response recieved: %s\n", buf);

  client_die:
    socket_die(sock_client);
  }
}
