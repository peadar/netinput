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
   addrinfo *addrs;

   memset(&hints, 0, sizeof hints);
   hints.ai_socktype = SOCK_DGRAM;
   hints.ai_family = PF_UNSPEC;
   hints.ai_addr = 0;
   auto rc = getaddrinfo(host, port, &hints, &addrs);
   if (rc != 0) {
      Exception e;
      e << "getaddrinfo failed: " << gai_strerror(rc);
      throw e;
   }

   for (auto ai = addrs; ai; ai = ai->ai_next) {
      Closer<int, std::function<int(int)>> fd(socket(ai->ai_socktype, ai->ai_family, ai->ai_protocol), close);

      if (fd == -1) {
         std::clog << "failed to open socket\n";
         continue;
      }
      const char msg[] = "hello world\n";
      sendto(fd, msg, sizeof msg - 1, 0, ai->ai_addr, ai->ai_addrlen);
      break;
   }
   return 0;
}

