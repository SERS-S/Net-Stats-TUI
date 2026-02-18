#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "reactor.h"

#include "data/overall_data.h"
#include "data/interfaces_data.h"
#include "data/addr_dns_data.h"
#include "data/arp_route_data.h"
#include "data/connections_sockets_data.h"
#include "data/protocol_stats_data.h"
#include "data/wifi_data.h"
#include "data/network_profiles_data.h"

static void log_thread_error(const char *section, int rc)
{
    fprintf(stderr, "%s section thread error: %s\n", section, strerror(rc));
}

static void* overall_event_update(void *arg)
{
    OVRLL *ovrll = (OVRLL *) arg;

    while (1)
    {
        
    }

    return NULL;
}

static void* interfaces_event_update(void *arg)
{
    INTRF *intrf = (INTRF *) arg;

    while (1)
    {
        
    }

    return NULL;
}

static void* addr_dns_event_update(void *arg)
{
    ADDRDNS *addrdns = (ADDRDNS *) arg;

    while (1)
    {
        
    }

    return NULL;
}

static void* arp_route_event_update(void *arg)
{
    ARPRT *arprt = (ARPRT *) arg;

    while (1)
    {
        
    }

    return NULL;
}

static void* connections_sockets_event_update(void *arg)
{
    CONSOCK *consock = (CONSOCK *) arg;

    while (1)
    {
        
    }

    return NULL;
}

static void* protocol_stats_event_update(void *arg)
{
    PROTST *protst = (PROTST *) arg;

    while (1)
    {
        
    }

    return NULL;
}

static void* wifi_event_update(void *arg)
{
    WIFI *wifi = (WIFI *) arg;

    while (1)
    {
        
    }

    return NULL;
}

static void* network_profiles_event_update(void *arg)
{
    NETPROF *netprof = (NETPROF *) arg;

    while (1)
    {
        
    }

    return NULL;
}

void event_loop(
    OVRLL *ovrll,
    INTRF *intrf,
    ADDRDNS *addrdns,
    ARPRT *arprt,
    CONSOCK *consock,
    PROTST *protst,
    WIFI *wifi,
    NETPROF *netprof
)
{
    pthread_t ovrll_t;
    pthread_t intrf_t;
    pthread_t addrdns_t;
    pthread_t arprt_t;
    pthread_t consock_t;
    pthread_t protst_t;
    pthread_t wifi_t;
    pthread_t netprof_t;

    int ovrll_rc = pthread_create(&ovrll_t, NULL, overall_event_update, ovrll);
    if (ovrll_rc != 0) log_thread_error("overall", ovrll_rc);
    else pthread_detach(ovrll_t);

    int intrf_rc = pthread_create(&intrf_t, NULL, interfaces_event_update, intrf);
    if (intrf_rc != 0) log_thread_error("interfaces", intrf_rc);
    else pthread_detach(intrf_t);

    int addrdns_rc = pthread_create(&addrdns_t, NULL, addr_dns_event_update, addrdns);
    if (addrdns_rc != 0) log_thread_error("addr_dns", addrdns_rc);
    else pthread_detach(addrdns_t);

    int arprt_rc = pthread_create(&arprt_t, NULL, arp_route_event_update, arprt);
    if (arprt_rc != 0) log_thread_error("arp_route", arprt_rc);
    else pthread_detach(arprt_t);

    int consock_rc = pthread_create(&consock_t, NULL, connections_sockets_event_update, consock);
    if (consock_rc != 0) log_thread_error("connections_sockets", consock_rc);
    else pthread_detach(consock_t);

    int protst_rc = pthread_create(&protst_t, NULL, protocol_stats_event_update, protst);
    if (protst_rc != 0) log_thread_error("protocol_stats", protst_rc);
    else pthread_detach(protst_t);

    int wifi_rc = pthread_create(&wifi_t, NULL, wifi_event_update, wifi);
    if (wifi_rc != 0) log_thread_error("wifi", wifi_rc);
    else pthread_detach(wifi_t);

    int netprof_rc = pthread_create(&netprof_t, NULL, network_profiles_event_update, netprof);
    if (netprof_rc != 0) log_thread_error("network_profiles", netprof_rc);
    else pthread_detach(netprof_t);

    return;
}
