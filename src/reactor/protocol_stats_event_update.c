#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "reactor/protocol_stats_event_update.h"

#include "data/protocol_stats_data.h"

#define PROTOCOL_STATS_LINE_LEN 32768
#define PROTOCOL_STATS_MAX_SECTIONS 16
#define PROTOCOL_STATS_MAX_SECTION_FIELDS 256
#define PROTOCOL_STATS_MAX_SNMP6_FIELDS 256
#define PROTOCOL_STATS_KEY_LEN 64
#define PROTOCOL_STATS_SECTION_LEN 32

typedef struct protocol_proc_metric
{
    char key[PROTOCOL_STATS_KEY_LEN];
    unsigned long long value;
} ProcMetric;

typedef struct protocol_proc_section
{
    char name[PROTOCOL_STATS_SECTION_LEN];
    ProcMetric fields[PROTOCOL_STATS_MAX_SECTION_FIELDS];
    int count;
} ProcSection;

typedef struct protocol_proc_section_table
{
    ProcSection sections[PROTOCOL_STATS_MAX_SECTIONS];
    int count;
} ProcSectionTable;

typedef struct protocol_proc_snmp6_table
{
    ProcMetric fields[PROTOCOL_STATS_MAX_SNMP6_FIELDS];
    int count;
} ProcSnmp6Table;

typedef struct protocol_totals
{
    unsigned long long tcp_insegs_total;
    unsigned long long tcp_outsegs_total;
    unsigned long long tcp_retranssegs_total;
    unsigned long long tcp_estabresets_total;
    unsigned long long tcp_listenoverflows_total;

    unsigned long long ip_indelivers_total;
    unsigned long long ip_indiscards_total;
    unsigned long long ip_outdiscards_total;

    unsigned long long icmp_inerrors_total;
    unsigned long long icmp_outerrors_total;
    unsigned long long icmp_unreach_total;
    unsigned long long icmp_timeexcd_total;

    unsigned long long udp_indatagrams_total;
    unsigned long long udp_noports_total;

    unsigned long long ip6_indelivers_total;
    unsigned long long ip6_indiscards_total;
    unsigned long long ip6_outdiscards_total;

    unsigned long long icmp6_inerrors_total;
    unsigned long long icmp6_outerrors_total;
    unsigned long long icmp6_unreach_total;
    unsigned long long icmp6_timeexcd_total;

    unsigned long long udp6_indatagrams_total;
    unsigned long long udp6_noports_total;
} ProtocolTotals;

typedef struct protocol_rates
{
    float tcp_insegs_rate;
    float tcp_outsegs_rate;
    float tcp_retranssegs_rate;
    float tcp_estabresets_rate;
    float tcp_listenoverflows_rate;

    float ip_indelivers_rate;
    float ip_indiscards_rate;
    float ip_outdiscards_rate;

    float icmp_inerrors_rate;
    float icmp_outerrors_rate;
    float icmp_unreach_rate;
    float icmp_timeexcd_rate;

    float udp_indatagrams_rate;
    float udp_noports_rate;

    float ip6_indelivers_rate;
    float ip6_indiscards_rate;
    float ip6_outdiscards_rate;

    float icmp6_inerrors_rate;
    float icmp6_outerrors_rate;
    float icmp6_unreach_rate;
    float icmp6_timeexcd_rate;

    float udp6_indatagrams_rate;
    float udp6_noports_rate;
} ProtocolRates;

static void trim_line(char *line)
{
    if (line == NULL) return;
    line[strcspn(line, "\r\n")] = '\0';
}

static void strip_section_suffix(char *name)
{
    size_t len;

    if (name == NULL) return;

    len = strlen(name);
    if (len > 0 && name[len - 1] == ':')
    {
        name[len - 1] = '\0';
    }
}

static const ProcSection *find_section(const ProcSectionTable *table, const char *name)
{
    if (table == NULL || name == NULL) return NULL;

    for (int i = 0; i < table->count; ++i)
    {
        if (strcmp(table->sections[i].name, name) == 0)
        {
            return &table->sections[i];
        }
    }

    return NULL;
}

static int read_section_value(
    const ProcSectionTable *table,
    const char *section_name,
    const char *field_name,
    unsigned long long *out
)
{
    if (table == NULL || section_name == NULL || field_name == NULL || out == NULL) return -1;

    const ProcSection *section = find_section(table, section_name);
    if (section == NULL) return -1;

    for (int i = 0; i < section->count; ++i)
    {
        if (strcmp(section->fields[i].key, field_name) == 0)
        {
            *out = section->fields[i].value;
            return 0;
        }
    }

    return -1;
}

static int read_snmp6_value(
    const ProcSnmp6Table *table,
    const char *field_name,
    unsigned long long *out
)
{
    if (table == NULL || field_name == NULL || out == NULL) return -1;

    for (int i = 0; i < table->count; ++i)
    {
        if (strcmp(table->fields[i].key, field_name) == 0)
        {
            *out = table->fields[i].value;
            return 0;
        }
    }

    return -1;
}

static int parse_named_section_file(const char *path, ProcSectionTable *table)
{
    if (path == NULL || table == NULL) return -1;

    FILE *f = fopen(path, "r");
    if (!f)
    {
        perror(path);
        return -1;
    }

    table->count = 0;

    char header_line[PROTOCOL_STATS_LINE_LEN];
    char value_line[PROTOCOL_STATS_LINE_LEN];

    while (fgets(header_line, sizeof(header_line), f))
    {
        if (!fgets(value_line, sizeof(value_line), f))
        {
            fclose(f);
            return -1;
        }

        trim_line(header_line);
        trim_line(value_line);

        char *header_ctx = NULL;
        char *value_ctx = NULL;
        char *header_tok = strtok_r(header_line, " \t", &header_ctx);
        char *value_tok = strtok_r(value_line, " \t", &value_ctx);

        if (header_tok == NULL || value_tok == NULL) continue;

        strip_section_suffix(header_tok);
        strip_section_suffix(value_tok);

        if (strcmp(header_tok, value_tok) != 0) continue;

        if (table->count >= PROTOCOL_STATS_MAX_SECTIONS)
        {
            fprintf(stderr, "parse_named_section_file(%s): too many sections\n", path);
            fclose(f);
            return -1;
        }

        ProcSection *section = &table->sections[table->count];
        memset(section, 0, sizeof(*section));
        snprintf(section->name, sizeof(section->name), "%s", header_tok);

        while (1)
        {
            header_tok = strtok_r(NULL, " \t", &header_ctx);
            value_tok = strtok_r(NULL, " \t", &value_ctx);

            if (header_tok == NULL || value_tok == NULL) break;

            if (section->count >= PROTOCOL_STATS_MAX_SECTION_FIELDS)
            {
                fprintf(stderr, "parse_named_section_file(%s): too many fields in %s\n", path, section->name);
                fclose(f);
                return -1;
            }

            char *end = NULL;
            unsigned long long value = strtoull(value_tok, &end, 10);
            if (end == value_tok || *end != '\0') continue;

            snprintf(
                section->fields[section->count].key,
                sizeof(section->fields[section->count].key),
                "%s",
                header_tok
            );
            section->fields[section->count].value = value;
            ++section->count;
        }

        ++table->count;
    }

    fclose(f);
    return 0;
}

static int parse_snmp6_file(const char *path, ProcSnmp6Table *table)
{
    if (path == NULL || table == NULL) return -1;

    FILE *f = fopen(path, "r");
    if (!f)
    {
        perror(path);
        return -1;
    }

    table->count = 0;

    char line[PROTOCOL_STATS_LINE_LEN];
    while (fgets(line, sizeof(line), f))
    {
        char key[PROTOCOL_STATS_KEY_LEN];
        unsigned long long value = 0;

        if (sscanf(line, " %63s %llu", key, &value) != 2) continue;

        if (table->count >= PROTOCOL_STATS_MAX_SNMP6_FIELDS)
        {
            fprintf(stderr, "parse_snmp6_file(%s): too many fields\n", path);
            fclose(f);
            return -1;
        }

        snprintf(table->fields[table->count].key, sizeof(table->fields[table->count].key), "%s", key);
        table->fields[table->count].value = value;
        ++table->count;
    }

    fclose(f);
    return 0;
}

static int read_tcp_insegs(const ProcSectionTable *snmp, unsigned long long *out)
{
    return read_section_value(snmp, "Tcp", "InSegs", out);
}

static int read_tcp_outsegs(const ProcSectionTable *snmp, unsigned long long *out)
{
    return read_section_value(snmp, "Tcp", "OutSegs", out);
}

static int read_tcp_retranssegs(const ProcSectionTable *snmp, unsigned long long *out)
{
    return read_section_value(snmp, "Tcp", "RetransSegs", out);
}

static int read_tcp_estabresets(const ProcSectionTable *snmp, unsigned long long *out)
{
    return read_section_value(snmp, "Tcp", "EstabResets", out);
}

static int read_tcp_listenoverflows(const ProcSectionTable *netstat, unsigned long long *out)
{
    return read_section_value(netstat, "TcpExt", "ListenOverflows", out);
}

static int read_ip_indelivers(const ProcSectionTable *snmp, unsigned long long *out)
{
    return read_section_value(snmp, "Ip", "InDelivers", out);
}

static int read_ip_indiscards(const ProcSectionTable *snmp, unsigned long long *out)
{
    return read_section_value(snmp, "Ip", "InDiscards", out);
}

static int read_ip_outdiscards(const ProcSectionTable *snmp, unsigned long long *out)
{
    return read_section_value(snmp, "Ip", "OutDiscards", out);
}

static int read_icmp_inerrors(const ProcSectionTable *snmp, unsigned long long *out)
{
    return read_section_value(snmp, "Icmp", "InErrors", out);
}

static int read_icmp_outerrors(const ProcSectionTable *snmp, unsigned long long *out)
{
    return read_section_value(snmp, "Icmp", "OutErrors", out);
}

static int read_icmp_unreach(const ProcSectionTable *snmp, unsigned long long *out)
{
    unsigned long long in_unreach = 0;
    unsigned long long out_unreach = 0;

    if (read_section_value(snmp, "Icmp", "InDestUnreachs", &in_unreach) != 0) return -1;
    if (read_section_value(snmp, "Icmp", "OutDestUnreachs", &out_unreach) != 0) return -1;

    *out = in_unreach + out_unreach;
    return 0;
}

static int read_icmp_timeexcd(const ProcSectionTable *snmp, unsigned long long *out)
{
    unsigned long long in_timeexcd = 0;
    unsigned long long out_timeexcd = 0;

    if (read_section_value(snmp, "Icmp", "InTimeExcds", &in_timeexcd) != 0) return -1;
    if (read_section_value(snmp, "Icmp", "OutTimeExcds", &out_timeexcd) != 0) return -1;

    *out = in_timeexcd + out_timeexcd;
    return 0;
}

static int read_udp_indatagrams(const ProcSectionTable *snmp, unsigned long long *out)
{
    return read_section_value(snmp, "Udp", "InDatagrams", out);
}

static int read_udp_noports(const ProcSectionTable *snmp, unsigned long long *out)
{
    return read_section_value(snmp, "Udp", "NoPorts", out);
}

static int read_ip6_indelivers(const ProcSnmp6Table *snmp6, unsigned long long *out)
{
    return read_snmp6_value(snmp6, "Ip6InDelivers", out);
}

static int read_ip6_indiscards(const ProcSnmp6Table *snmp6, unsigned long long *out)
{
    return read_snmp6_value(snmp6, "Ip6InDiscards", out);
}

static int read_ip6_outdiscards(const ProcSnmp6Table *snmp6, unsigned long long *out)
{
    return read_snmp6_value(snmp6, "Ip6OutDiscards", out);
}

static int read_icmp6_inerrors(const ProcSnmp6Table *snmp6, unsigned long long *out)
{
    return read_snmp6_value(snmp6, "Icmp6InErrors", out);
}

static int read_icmp6_outerrors(const ProcSnmp6Table *snmp6, unsigned long long *out)
{
    return read_snmp6_value(snmp6, "Icmp6OutErrors", out);
}

static int read_icmp6_unreach(const ProcSnmp6Table *snmp6, unsigned long long *out)
{
    unsigned long long in_unreach = 0;
    unsigned long long out_unreach = 0;

    if (read_snmp6_value(snmp6, "Icmp6InDestUnreachs", &in_unreach) != 0) return -1;
    if (read_snmp6_value(snmp6, "Icmp6OutDestUnreachs", &out_unreach) != 0) return -1;

    *out = in_unreach + out_unreach;
    return 0;
}

static int read_icmp6_timeexcd(const ProcSnmp6Table *snmp6, unsigned long long *out)
{
    unsigned long long in_timeexcd = 0;
    unsigned long long out_timeexcd = 0;

    if (read_snmp6_value(snmp6, "Icmp6InTimeExcds", &in_timeexcd) != 0) return -1;
    if (read_snmp6_value(snmp6, "Icmp6OutTimeExcds", &out_timeexcd) != 0) return -1;

    *out = in_timeexcd + out_timeexcd;
    return 0;
}

static int read_udp6_indatagrams(const ProcSnmp6Table *snmp6, unsigned long long *out)
{
    return read_snmp6_value(snmp6, "Udp6InDatagrams", out);
}

static int read_udp6_noports(const ProcSnmp6Table *snmp6, unsigned long long *out)
{
    return read_snmp6_value(snmp6, "Udp6NoPorts", out);
}

static int read_protocol_totals(
    const ProcSectionTable *snmp,
    const ProcSectionTable *netstat,
    const ProcSnmp6Table *snmp6,
    ProtocolTotals *totals
)
{
    if (snmp == NULL || netstat == NULL || snmp6 == NULL || totals == NULL) return -1;

    memset(totals, 0, sizeof(*totals));

    if (read_tcp_insegs(snmp, &totals->tcp_insegs_total) != 0) return -1;
    if (read_tcp_outsegs(snmp, &totals->tcp_outsegs_total) != 0) return -1;
    if (read_tcp_retranssegs(snmp, &totals->tcp_retranssegs_total) != 0) return -1;
    if (read_tcp_estabresets(snmp, &totals->tcp_estabresets_total) != 0) return -1;
    if (read_tcp_listenoverflows(netstat, &totals->tcp_listenoverflows_total) != 0) return -1;

    if (read_ip_indelivers(snmp, &totals->ip_indelivers_total) != 0) return -1;
    if (read_ip_indiscards(snmp, &totals->ip_indiscards_total) != 0) return -1;
    if (read_ip_outdiscards(snmp, &totals->ip_outdiscards_total) != 0) return -1;

    if (read_icmp_inerrors(snmp, &totals->icmp_inerrors_total) != 0) return -1;
    if (read_icmp_outerrors(snmp, &totals->icmp_outerrors_total) != 0) return -1;
    if (read_icmp_unreach(snmp, &totals->icmp_unreach_total) != 0) return -1;
    if (read_icmp_timeexcd(snmp, &totals->icmp_timeexcd_total) != 0) return -1;

    if (read_udp_indatagrams(snmp, &totals->udp_indatagrams_total) != 0) return -1;
    if (read_udp_noports(snmp, &totals->udp_noports_total) != 0) return -1;

    if (read_ip6_indelivers(snmp6, &totals->ip6_indelivers_total) != 0) return -1;
    if (read_ip6_indiscards(snmp6, &totals->ip6_indiscards_total) != 0) return -1;
    if (read_ip6_outdiscards(snmp6, &totals->ip6_outdiscards_total) != 0) return -1;

    if (read_icmp6_inerrors(snmp6, &totals->icmp6_inerrors_total) != 0) return -1;
    if (read_icmp6_outerrors(snmp6, &totals->icmp6_outerrors_total) != 0) return -1;
    if (read_icmp6_unreach(snmp6, &totals->icmp6_unreach_total) != 0) return -1;
    if (read_icmp6_timeexcd(snmp6, &totals->icmp6_timeexcd_total) != 0) return -1;

    if (read_udp6_indatagrams(snmp6, &totals->udp6_indatagrams_total) != 0) return -1;
    if (read_udp6_noports(snmp6, &totals->udp6_noports_total) != 0) return -1;

    return 0;
}

static float calc_rate(unsigned long long curr, unsigned long long prev, float dt)
{
    if (dt <= 0 || curr < prev) return 0.0f;
    return (float) (curr - prev) / dt;
}

static void compute_protocol_rates(
    const ProtocolTotals *curr,
    const ProtocolTotals *prev,
    int has_prev,
    float dt,
    ProtocolRates *rates
)
{
    if (curr == NULL || prev == NULL || rates == NULL) return;

    memset(rates, 0, sizeof(*rates));
    if (!has_prev) return;

    rates->tcp_insegs_rate = calc_rate(curr->tcp_insegs_total, prev->tcp_insegs_total, dt);
    rates->tcp_outsegs_rate = calc_rate(curr->tcp_outsegs_total, prev->tcp_outsegs_total, dt);
    rates->tcp_retranssegs_rate = calc_rate(curr->tcp_retranssegs_total, prev->tcp_retranssegs_total, dt);
    rates->tcp_estabresets_rate = calc_rate(curr->tcp_estabresets_total, prev->tcp_estabresets_total, dt);
    rates->tcp_listenoverflows_rate = calc_rate(
        curr->tcp_listenoverflows_total,
        prev->tcp_listenoverflows_total,
        dt
    );

    rates->ip_indelivers_rate = calc_rate(curr->ip_indelivers_total, prev->ip_indelivers_total, dt);
    rates->ip_indiscards_rate = calc_rate(curr->ip_indiscards_total, prev->ip_indiscards_total, dt);
    rates->ip_outdiscards_rate = calc_rate(curr->ip_outdiscards_total, prev->ip_outdiscards_total, dt);

    rates->icmp_inerrors_rate = calc_rate(curr->icmp_inerrors_total, prev->icmp_inerrors_total, dt);
    rates->icmp_outerrors_rate = calc_rate(curr->icmp_outerrors_total, prev->icmp_outerrors_total, dt);
    rates->icmp_unreach_rate = calc_rate(curr->icmp_unreach_total, prev->icmp_unreach_total, dt);
    rates->icmp_timeexcd_rate = calc_rate(curr->icmp_timeexcd_total, prev->icmp_timeexcd_total, dt);

    rates->udp_indatagrams_rate = calc_rate(curr->udp_indatagrams_total, prev->udp_indatagrams_total, dt);
    rates->udp_noports_rate = calc_rate(curr->udp_noports_total, prev->udp_noports_total, dt);

    rates->ip6_indelivers_rate = calc_rate(curr->ip6_indelivers_total, prev->ip6_indelivers_total, dt);
    rates->ip6_indiscards_rate = calc_rate(curr->ip6_indiscards_total, prev->ip6_indiscards_total, dt);
    rates->ip6_outdiscards_rate = calc_rate(curr->ip6_outdiscards_total, prev->ip6_outdiscards_total, dt);

    rates->icmp6_inerrors_rate = calc_rate(curr->icmp6_inerrors_total, prev->icmp6_inerrors_total, dt);
    rates->icmp6_outerrors_rate = calc_rate(curr->icmp6_outerrors_total, prev->icmp6_outerrors_total, dt);
    rates->icmp6_unreach_rate = calc_rate(curr->icmp6_unreach_total, prev->icmp6_unreach_total, dt);
    rates->icmp6_timeexcd_rate = calc_rate(curr->icmp6_timeexcd_total, prev->icmp6_timeexcd_total, dt);

    rates->udp6_indatagrams_rate = calc_rate(curr->udp6_indatagrams_total, prev->udp6_indatagrams_total, dt);
    rates->udp6_noports_rate = calc_rate(curr->udp6_noports_total, prev->udp6_noports_total, dt);
}

void* protocol_stats_event_update(void *arg)
{
    PROTST *protst = (PROTST *) arg;
    if (protst == NULL) return NULL;

    ProcSectionTable *snmp_table = malloc(sizeof(*snmp_table));
    ProcSectionTable *netstat_table = malloc(sizeof(*netstat_table));
    ProcSnmp6Table *snmp6_table = malloc(sizeof(*snmp6_table));
    if (snmp_table == NULL || netstat_table == NULL || snmp6_table == NULL)
    {
        fprintf(stderr, "protocol_stats_event_update malloc failed\n");
        free(snmp_table);
        free(netstat_table);
        free(snmp6_table);
        return NULL;
    }

    ProtocolTotals prev_totals;
    memset(&prev_totals, 0, sizeof(prev_totals));

    int has_prev = 0;
    const float interval_sec = 1.0f;
    const unsigned int sleep_sec = 1;

    while (1)
    {
        ProtocolTotals curr_totals;
        ProtocolRates curr_rates;

        if (parse_named_section_file("/proc/net/snmp", snmp_table) != 0)
        {
            sleep(sleep_sec);
            continue;
        }

        if (parse_named_section_file("/proc/net/netstat", netstat_table) != 0)
        {
            sleep(sleep_sec);
            continue;
        }

        if (parse_snmp6_file("/proc/net/snmp6", snmp6_table) != 0)
        {
            sleep(sleep_sec);
            continue;
        }

        if (read_protocol_totals(snmp_table, netstat_table, snmp6_table, &curr_totals) != 0)
        {
            fprintf(stderr, "read_protocol_totals failed\n");
            sleep(sleep_sec);
            continue;
        }

        compute_protocol_rates(&curr_totals, &prev_totals, has_prev, interval_sec, &curr_rates);

        PROTST_update_data(
            protst,
            curr_totals.tcp_insegs_total,
            curr_rates.tcp_insegs_rate,
            curr_totals.tcp_outsegs_total,
            curr_rates.tcp_outsegs_rate,
            curr_totals.tcp_retranssegs_total,
            curr_rates.tcp_retranssegs_rate,
            curr_totals.tcp_estabresets_total,
            curr_rates.tcp_estabresets_rate,
            curr_totals.tcp_listenoverflows_total,
            curr_rates.tcp_listenoverflows_rate,
            curr_totals.ip_indelivers_total,
            curr_rates.ip_indelivers_rate,
            curr_totals.ip_indiscards_total,
            curr_rates.ip_indiscards_rate,
            curr_totals.ip_outdiscards_total,
            curr_rates.ip_outdiscards_rate,
            curr_totals.icmp_inerrors_total,
            curr_rates.icmp_inerrors_rate,
            curr_totals.icmp_outerrors_total,
            curr_rates.icmp_outerrors_rate,
            curr_totals.icmp_unreach_total,
            curr_rates.icmp_unreach_rate,
            curr_totals.icmp_timeexcd_total,
            curr_rates.icmp_timeexcd_rate,
            curr_totals.udp_indatagrams_total,
            curr_rates.udp_indatagrams_rate,
            curr_totals.udp_noports_total,
            curr_rates.udp_noports_rate,
            curr_totals.ip6_indelivers_total,
            curr_rates.ip6_indelivers_rate,
            curr_totals.ip6_indiscards_total,
            curr_rates.ip6_indiscards_rate,
            curr_totals.ip6_outdiscards_total,
            curr_rates.ip6_outdiscards_rate,
            curr_totals.icmp6_inerrors_total,
            curr_rates.icmp6_inerrors_rate,
            curr_totals.icmp6_outerrors_total,
            curr_rates.icmp6_outerrors_rate,
            curr_totals.icmp6_unreach_total,
            curr_rates.icmp6_unreach_rate,
            curr_totals.icmp6_timeexcd_total,
            curr_rates.icmp6_timeexcd_rate,
            curr_totals.udp6_indatagrams_total,
            curr_rates.udp6_indatagrams_rate,
            curr_totals.udp6_noports_total,
            curr_rates.udp6_noports_rate
        );

        prev_totals = curr_totals;
        has_prev = 1;

        sleep(sleep_sec);
    }

    return NULL;
}
