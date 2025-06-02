#pragma once

#if defined(_WIN32) || defined(_WIN64)
#error Windows is not supported
#endif

#include <libmisc/versioning.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#ifdef __cplusplus
MISC_CXX_EXTERN
#endif

/*============ AF_INET ONLY ============*/
/*
 * Domain specific POSIX Socket API.
 * Currently, only AF_INET or IPv4 is supported.
 *
 * */
typedef int __Misc_Socket;
typedef unsigned short __Misc_Port;

typedef __Misc_Socket Socket, Socket_Type, Socket_Status;
typedef __Misc_Port Socket_Port;

typedef struct sockaddr Socket_Generic;
typedef struct sockaddr_in Socket_Address;

enum : socklen_t {
  SOCKADDR_V4LEN = sizeof(Socket_Address),
};

enum : Socket_Status {
  SOCKSTAT_OK = 0,
  SOCKSTAT_ERR = -1,
};

Socket_Address socket_address_new(const char *ipv4_addr, Socket_Port port);
Socket socket_new(Socket_Type type);
Socket socket_tcp_server(Socket_Address *addr, int queue);
Socket socket_udp_server(Socket_Address *addr, int queue);
Socket socket_bind(Socket sockfd, Socket_Address *addr);
Socket socket_accept(Socket sockfd, Socket_Address *addr);
Socket_Status socket_connect(Socket sockfd, Socket_Address *addr);
Socket_Status socket_die(Socket sockfd);
/*============ AF_INET ONLY ============*/

#ifdef __cplusplus
MISC_CXX_ENDEXTERN
#endif
