#include "mydns.h"

int init_mydns(const char *dns_ip, unsigned int dns_port){
  printf("Dns ip: %s\nDns port: %d\n", dns_ip, dns_port);
  return 0;
}

int resolve(const char *node, const char *service,
            const struct addrinfo *hints, struct addrinfo **res){
  return 0;
}
