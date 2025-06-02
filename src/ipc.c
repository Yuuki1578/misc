#include <arpa/inet.h>
#include <libmisc/ipc.h>
#include <unistd.h>

Socket_Address socket_address_new(const char *ipv4_addr, Socket_Port port) {
  Socket_Address addr = {
      .sin_family = AF_INET,
      .sin_port = htons(port),
      .sin_addr = {0},
  };

  if (inet_pton(AF_INET, ipv4_addr, &addr.sin_addr) != 1)
    return (Socket_Address){0};

  return addr;
}

Socket socket_new(Socket_Type type) { return socket(AF_INET, type, 0); }

static Socket socket_server(Socket_Type type, Socket_Address *addr, int queue) {
  Socket sockfd = socket_new(type);

  if (sockfd == -1)
    return SOCKSTAT_ERR;

  if (addr == nullptr)
    return sockfd;

  if (socket_bind(sockfd, addr) == -1) {
    close(sockfd);
    return SOCKSTAT_ERR;
  }

  if (listen(sockfd, queue) == -1) {
    close(sockfd);
    return SOCKSTAT_ERR;
  }

  return sockfd;
}

Socket socket_tcp_server(Socket_Address *addr, int queue) {
  return socket_server(SOCK_STREAM, addr, queue);
}

Socket socket_udp_server(Socket_Address *addr, int queue) {
  return socket_server(SOCK_DGRAM, addr, queue);
}

Socket socket_bind(Socket sockfd, Socket_Address *addr) {
  return bind(sockfd, (Socket_Generic *)addr, SOCKADDR_V4LEN);
}

Socket socket_accept(Socket sockfd, Socket_Address *addr) {
  return accept(sockfd, (Socket_Generic *)addr, &(socklen_t){SOCKADDR_V4LEN});
}

Socket_Status socket_connect(Socket sockfd, Socket_Address *addr) {
  return connect(sockfd, (Socket_Generic *)addr, SOCKADDR_V4LEN);
}

Socket_Status socket_die(Socket sockfd) {
  return (Socket_Status)shutdown(sockfd, SHUT_RDWR);
}
