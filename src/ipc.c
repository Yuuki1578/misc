#include <arpa/inet.h>
#include <libmisc/ipc.h>
#include <unistd.h>

Socket_Address sockaddr_new(const char *ipv4_addr, Socket_Port port) {
  Socket_Address addr = {
      .sin_family = AF_INET,
      .sin_port = htons(port),
      .sin_addr = {0},
  };

  if (inet_pton(AF_INET, ipv4_addr, &addr.sin_addr) != 1)
    return (Socket_Address){0};

  return addr;
}

Socket socket_server(Socket_Type type, Socket_Address *addr_v4, int queue) {
  Socket sockfd = socket(AF_INET, type, 0);

  if (sockfd == -1)
    return SOCKSTAT_ERR;

  if (addr_v4 == nullptr)
    return sockfd;

  if (bind(sockfd, (Socket_Generic *)addr_v4, sizeof *addr_v4) == -1) {
    close(sockfd);
    return SOCKSTAT_ERR;
  }

  if (listen(sockfd, queue) == -1) {
    close(sockfd);
    return SOCKSTAT_ERR;
  }

  return sockfd;
}

Socket socket_new(Socket_Type type) {
  // default socket
  return socket(AF_INET, type, 0);
}

Socket_Status socket_connect(Socket sockfd, Socket_Address *addr_v4) {
  if (sockfd == SOCKSTAT_ERR || addr_v4 == nullptr)
    return SOCKSTAT_ERR;

  return connect(sockfd, (Socket_Generic *)addr_v4, sizeof *addr_v4);
}

Socket_Status socket_die(Socket sockfd) {
  return (Socket_Status)shutdown(sockfd, SHUT_RDWR);
}
