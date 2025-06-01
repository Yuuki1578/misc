#pragma once

#include <libmisc/versioning.h>

#if defined(_WIN32) || defined(_WIN64)
#error Windows is not supported
#endif

#ifdef __cplusplus
MISC_CXX_EXTERN
#endif

#include <netinet/in.h>
#include <sys/socket.h>

typedef int __misc_socket_t;
typedef unsigned short __misc_port_t;

typedef __misc_socket_t Socket, Socket_Type, Socket_Status;
typedef __misc_port_t Socket_Port;

typedef struct sockaddr Socket_Generic;
typedef struct sockaddr_in Socket_Address;
typedef struct sockaddr_in6 Socket_Address_V6;

enum : socklen_t {
  SOCKADDR_V4LEN = sizeof(Socket_Address),
  SOCKADDR_V6LEN = sizeof(Socket_Address_V6),
};

enum : Socket_Status {
  SOCKSTAT_OK = 0,
  SOCKSTAT_ERR = -1,
};

Socket_Address sockaddr_new(const char *ipv4_addr, Socket_Port port);
Socket socket_server(Socket_Type type, Socket_Address *addr_v4, int queue);
Socket socket_new(Socket_Type type);
Socket_Status socket_connect(Socket sockfd, Socket_Address *addr_v4);

Socket_Address_V6 sockaddr_new_v6(const char *ipv6_addr, Socket_Port port);
Socket socket_server_v6(Socket_Type type, Socket_Address_V6 *addr_v6,
                        int queue);
Socket_Status socket_connect_v6(Socket sockfd, Socket_Address_V6 *addr_v6);

Socket_Status socket_die(Socket sockfd);

#ifdef __cplusplus
MISC_CXX_ENDEXTERN
#endif
