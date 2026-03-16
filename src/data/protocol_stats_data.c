#include "data/protocol_stats_data.h"

#include <pthread.h>

void PROTST_init(PROTST *e)
{
    pthread_mutex_init(&e->mtx, NULL);

    e->tcp_insegs_total = 0;
    e->tcp_insegs_rate = 0;
    e->tcp_outsegs_total = 0;
    e->tcp_outsegs_rate = 0;
    e->tcp_retranssegs_total = 0;
    e->tcp_retranssegs_rate = 0;
    e->tcp_estabresets_total = 0;
    e->tcp_estabresets_rate = 0;
    e->tcp_listenoverflows_total = 0;
    e->tcp_listenoverflows_rate = 0;

    e->ip_indelivers_total = 0;
    e->ip_indelivers_rate = 0;
    e->ip_indiscards_total = 0;
    e->ip_indiscards_rate = 0;
    e->ip_outdiscards_total = 0;
    e->ip_outdiscards_rate = 0;

    e->icmp_inerrors_total = 0;
    e->icmp_inerrors_rate = 0;
    e->icmp_outerrors_total = 0;
    e->icmp_outerrors_rate = 0;
    e->icmp_unreach_total = 0;
    e->icmp_unreach_rate = 0;
    e->icmp_timeexcd_total = 0;
    e->icmp_timeexcd_rate = 0;

    e->udp_indatagrams_total = 0;
    e->udp_indatagrams_rate = 0;
    e->udp_noports_total = 0;
    e->udp_noports_rate = 0;

    e->ip6_indelivers_total = 0;
    e->ip6_indelivers_rate = 0;
    e->ip6_indiscards_total = 0;
    e->ip6_indiscards_rate = 0;
    e->ip6_outdiscards_total = 0;
    e->ip6_outdiscards_rate = 0;

    e->icmp6_inerrors_total = 0;
    e->icmp6_inerrors_rate = 0;
    e->icmp6_outerrors_total = 0;
    e->icmp6_outerrors_rate = 0;
    e->icmp6_unreach_total = 0;
    e->icmp6_unreach_rate = 0;
    e->icmp6_timeexcd_total = 0;
    e->icmp6_timeexcd_rate = 0;

    e->udp6_indatagrams_total = 0;
    e->udp6_indatagrams_rate = 0;
    e->udp6_noports_total = 0;
    e->udp6_noports_rate = 0;
}

void PROTST_destroy(PROTST *e)
{
    pthread_mutex_destroy(&e->mtx);
}

void PROTST_update_data(
    PROTST *e,
    unsigned long long new_tcp_insegs_total,
    float new_tcp_insegs_rate,
    unsigned long long new_tcp_outsegs_total,
    float new_tcp_outsegs_rate,
    unsigned long long new_tcp_retranssegs_total,
    float new_tcp_retranssegs_rate,
    unsigned long long new_tcp_estabresets_total,
    float new_tcp_estabresets_rate,
    unsigned long long new_tcp_listenoverflows_total,
    float new_tcp_listenoverflows_rate,
    unsigned long long new_ip_indelivers_total,
    float new_ip_indelivers_rate,
    unsigned long long new_ip_indiscards_total,
    float new_ip_indiscards_rate,
    unsigned long long new_ip_outdiscards_total,
    float new_ip_outdiscards_rate,
    unsigned long long new_icmp_inerrors_total,
    float new_icmp_inerrors_rate,
    unsigned long long new_icmp_outerrors_total,
    float new_icmp_outerrors_rate,
    unsigned long long new_icmp_unreach_total,
    float new_icmp_unreach_rate,
    unsigned long long new_icmp_timeexcd_total,
    float new_icmp_timeexcd_rate,
    unsigned long long new_udp_indatagrams_total,
    float new_udp_indatagrams_rate,
    unsigned long long new_udp_noports_total,
    float new_udp_noports_rate,
    unsigned long long new_ip6_indelivers_total,
    float new_ip6_indelivers_rate,
    unsigned long long new_ip6_indiscards_total,
    float new_ip6_indiscards_rate,
    unsigned long long new_ip6_outdiscards_total,
    float new_ip6_outdiscards_rate,
    unsigned long long new_icmp6_inerrors_total,
    float new_icmp6_inerrors_rate,
    unsigned long long new_icmp6_outerrors_total,
    float new_icmp6_outerrors_rate,
    unsigned long long new_icmp6_unreach_total,
    float new_icmp6_unreach_rate,
    unsigned long long new_icmp6_timeexcd_total,
    float new_icmp6_timeexcd_rate,
    unsigned long long new_udp6_indatagrams_total,
    float new_udp6_indatagrams_rate,
    unsigned long long new_udp6_noports_total,
    float new_udp6_noports_rate
)
{
    pthread_mutex_lock(&e->mtx);

    e->tcp_insegs_total = new_tcp_insegs_total;
    e->tcp_insegs_rate = new_tcp_insegs_rate;
    e->tcp_outsegs_total = new_tcp_outsegs_total;
    e->tcp_outsegs_rate = new_tcp_outsegs_rate;
    e->tcp_retranssegs_total = new_tcp_retranssegs_total;
    e->tcp_retranssegs_rate = new_tcp_retranssegs_rate;
    e->tcp_estabresets_total = new_tcp_estabresets_total;
    e->tcp_estabresets_rate = new_tcp_estabresets_rate;
    e->tcp_listenoverflows_total = new_tcp_listenoverflows_total;
    e->tcp_listenoverflows_rate = new_tcp_listenoverflows_rate;

    e->ip_indelivers_total = new_ip_indelivers_total;
    e->ip_indelivers_rate = new_ip_indelivers_rate;
    e->ip_indiscards_total = new_ip_indiscards_total;
    e->ip_indiscards_rate = new_ip_indiscards_rate;
    e->ip_outdiscards_total = new_ip_outdiscards_total;
    e->ip_outdiscards_rate = new_ip_outdiscards_rate;

    e->icmp_inerrors_total = new_icmp_inerrors_total;
    e->icmp_inerrors_rate = new_icmp_inerrors_rate;
    e->icmp_outerrors_total = new_icmp_outerrors_total;
    e->icmp_outerrors_rate = new_icmp_outerrors_rate;
    e->icmp_unreach_total = new_icmp_unreach_total;
    e->icmp_unreach_rate = new_icmp_unreach_rate;
    e->icmp_timeexcd_total = new_icmp_timeexcd_total;
    e->icmp_timeexcd_rate = new_icmp_timeexcd_rate;

    e->udp_indatagrams_total = new_udp_indatagrams_total;
    e->udp_indatagrams_rate = new_udp_indatagrams_rate;
    e->udp_noports_total = new_udp_noports_total;
    e->udp_noports_rate = new_udp_noports_rate;

    e->ip6_indelivers_total = new_ip6_indelivers_total;
    e->ip6_indelivers_rate = new_ip6_indelivers_rate;
    e->ip6_indiscards_total = new_ip6_indiscards_total;
    e->ip6_indiscards_rate = new_ip6_indiscards_rate;
    e->ip6_outdiscards_total = new_ip6_outdiscards_total;
    e->ip6_outdiscards_rate = new_ip6_outdiscards_rate;

    e->icmp6_inerrors_total = new_icmp6_inerrors_total;
    e->icmp6_inerrors_rate = new_icmp6_inerrors_rate;
    e->icmp6_outerrors_total = new_icmp6_outerrors_total;
    e->icmp6_outerrors_rate = new_icmp6_outerrors_rate;
    e->icmp6_unreach_total = new_icmp6_unreach_total;
    e->icmp6_unreach_rate = new_icmp6_unreach_rate;
    e->icmp6_timeexcd_total = new_icmp6_timeexcd_total;
    e->icmp6_timeexcd_rate = new_icmp6_timeexcd_rate;

    e->udp6_indatagrams_total = new_udp6_indatagrams_total;
    e->udp6_indatagrams_rate = new_udp6_indatagrams_rate;
    e->udp6_noports_total = new_udp6_noports_total;
    e->udp6_noports_rate = new_udp6_noports_rate;

    pthread_mutex_unlock(&e->mtx);
}

PROTST PROTST_get_data(PROTST *e)
{
    PROTST new_e;
    pthread_mutex_lock(&e->mtx);
    new_e = *e;
    pthread_mutex_unlock(&e->mtx);
    return new_e;
}
