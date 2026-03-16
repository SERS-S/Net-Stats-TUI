#ifndef PROTOCOL_STATS_DATA
#define PROTOCOL_STATS_DATA

#include <pthread.h>

typedef struct protocol_stats_data
{
    pthread_mutex_t mtx;

    unsigned long long tcp_insegs_total;
    float tcp_insegs_rate;
    unsigned long long tcp_outsegs_total;
    float tcp_outsegs_rate;
    unsigned long long tcp_retranssegs_total;
    float tcp_retranssegs_rate;
    unsigned long long tcp_estabresets_total;
    float tcp_estabresets_rate;
    unsigned long long tcp_listenoverflows_total;
    float tcp_listenoverflows_rate;

    unsigned long long ip_indelivers_total;
    float ip_indelivers_rate;
    unsigned long long ip_indiscards_total;
    float ip_indiscards_rate;
    unsigned long long ip_outdiscards_total;
    float ip_outdiscards_rate;

    unsigned long long icmp_inerrors_total;
    float icmp_inerrors_rate;
    unsigned long long icmp_outerrors_total;
    float icmp_outerrors_rate;
    unsigned long long icmp_unreach_total;
    float icmp_unreach_rate;
    unsigned long long icmp_timeexcd_total;
    float icmp_timeexcd_rate;

    unsigned long long udp_indatagrams_total;
    float udp_indatagrams_rate;
    unsigned long long udp_noports_total;
    float udp_noports_rate;

    unsigned long long ip6_indelivers_total;
    float ip6_indelivers_rate;
    unsigned long long ip6_indiscards_total;
    float ip6_indiscards_rate;
    unsigned long long ip6_outdiscards_total;
    float ip6_outdiscards_rate;

    unsigned long long icmp6_inerrors_total;
    float icmp6_inerrors_rate;
    unsigned long long icmp6_outerrors_total;
    float icmp6_outerrors_rate;
    unsigned long long icmp6_unreach_total;
    float icmp6_unreach_rate;
    unsigned long long icmp6_timeexcd_total;
    float icmp6_timeexcd_rate;

    unsigned long long udp6_indatagrams_total;
    float udp6_indatagrams_rate;
    unsigned long long udp6_noports_total;
    float udp6_noports_rate;
} PROTST;

void PROTST_init(PROTST *e);
void PROTST_destroy(PROTST *e);
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
);
PROTST PROTST_get_data(PROTST *e);

#endif
