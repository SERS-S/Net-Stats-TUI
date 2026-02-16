#ifndef reactor
#define reactor

#include "data/overall_data.h"
#include "data/interfaces_data.h"
#include "data/addr_dns_data.h"
#include "data/arp_route_data.h"
#include "data/connections_sockets_data.h"
#include "data/protocol_stats_data.h"
#include "data/wifi_data.h"
#include "data/network_profiles_data.h"

void event_loop(
    OVRLL ovrll, 
    INTRF intrf, 
    ADDRDNS addrdns,
    ARPRT arprt,
    CONSOCK consock,
    PROTST protst,
    WIFI wifi,
    NETPROF netprof
);


#endif