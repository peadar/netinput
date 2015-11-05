#include <iostream>
#include <functional>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <vector>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "util.h"

static void
usage(const char *name)
{
   std::clog <<
      "usage: " << name
      << std::endl << "\t-h <host> -p <port>\t: address and port to connect to"
      << std::endl;
}

int
main(int argc, char *argv[])
{
   int c;
   const char *host = 0;
   const char *port = 0;
   while ((c = getopt(argc, argv, "h:p:")) != -1) {
      switch (c) {
         case 'h': host = optarg; break;
         case 'p': port = optarg; break;
         default: usage(argv[0]); break;
      }
   }
   if (host == 0 || port == 0)
      usage(argv[0]);
   addrinfo hints;
   addrinfo *multicast_addrs, *listen_addrs;

   memset(&hints, 0, sizeof hints);
   hints.ai_socktype = SOCK_DGRAM;
   hints.ai_family = PF_UNSPEC;
   hints.ai_flags = AI_PASSIVE;
   hints.ai_addr = 0;
   auto rc = getaddrinfo(host, port, &hints, &multicast_addrs);
   if (rc != 0) {
      Exception e;
      e << "getaddrinfo failed: " << gai_strerror(rc);
      throw e;
   }

   std::vector<int> sockets;
   // We've got our multicast addresses. For each one, bind a local socket of the right protocol, and then join the group.
   for (addrinfo *mai = multicast_addrs; mai; mai = mai->ai_next) {

      memset(&hints, 0, sizeof hints);
      hints.ai_socktype = SOCK_DGRAM;
      hints.ai_family = mai->ai_family;
      hints.ai_flags = AI_PASSIVE;
      hints.ai_addr = 0;

      int rc = getaddrinfo(0, port, &hints, &listen_addrs);
      if (rc != 0) {
         std::clog << "failed to get address info: " << gai_strerror(rc);
         continue;
      }

      // Now listen on each address we can for this multicast
      for (addrinfo *lai = listen_addrs; lai; lai = lai->ai_next) {
         Closer<int, std::function<int(int)>> fd(socket(lai->ai_socktype, lai->ai_family, lai->ai_protocol), close);
         if (fd == -1) {
            std::clog << "failed to open socket\n";
            continue;
         }
         rc = bind(int(fd), lai->ai_addr, lai->ai_addrlen);
         if (rc == -1) {
            Errno en;
            std::clog << "error: " << en;
            continue;
         }

         switch (mai->ai_family) {
            case PF_INET: {
               ip_mreqn req;
               req.imr_multiaddr = ((struct sockaddr_in *)mai->ai_addr)->sin_addr;
               req.imr_address.s_addr = htonl(INADDR_ANY);
               req.imr_ifindex = 0;
               rc = setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &req, sizeof req);
               if (rc != 0) {
                  std::clog << "error joining group: " << Errno() << "\n";
                  continue;
               }
               break;
            }
            case PF_INET6: {
               abort();
               break;
            }
         }
         sockets.push_back(fd.release());

      }
   }
   while (true) {
      char buf[1024];
      rc = read(sockets[0], buf, sizeof buf);
      switch (rc) {
         case 0:
            return 0;
         case -1:
            std::clog << "error: " << Errno() << "\n";
            return 0;
         default:
            std::cout << "received " << std::string(buf, rc) << "\n";
            break;
      }
   }
   return 0;
}

