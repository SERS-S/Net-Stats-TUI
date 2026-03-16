#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "reactor/network_profiles_event_update.h"

#include "data/network_profiles_data.h"

typedef struct network_profile_row
{
    char *name;
    char *uuid;
    char *type;
    char *device;
    char *state;
} NetworkProfileRow;

static void trim_line(char *line)
{
    if (line == NULL) return;
    line[strcspn(line, "\r\n")] = '\0';
}

static char *trim_whitespace(char *s)
{
    if (s == NULL) return NULL;

    while (*s == ' ' || *s == '\t') ++s;

    char *end = s + strlen(s);
    while (end > s && (end[-1] == ' ' || end[-1] == '\t'))
    {
        --end;
    }
    *end = '\0';

    return s;
}

static int parse_autoconnect_value(const char *value)
{
    if (value == NULL) return -1;

    if (
        strcmp(value, "true") == 0 ||
        strcmp(value, "yes") == 0 ||
        strcmp(value, "1") == 0
    )
    {
        return 1;
    }

    if (
        strcmp(value, "false") == 0 ||
        strcmp(value, "no") == 0 ||
        strcmp(value, "0") == 0
    )
    {
        return 0;
    }

    return -1;
}

static void free_network_profile_row(NetworkProfileRow *row)
{
    if (row == NULL) return;

    free(row->name);
    free(row->uuid);
    free(row->type);
    free(row->device);
    free(row->state);

    row->name = NULL;
    row->uuid = NULL;
    row->type = NULL;
    row->device = NULL;
    row->state = NULL;
}

static void free_network_profile_rows(NetworkProfileRow *rows, size_t count)
{
    if (rows == NULL) return;

    for (size_t i = 0; i < count; ++i)
    {
        free_network_profile_row(&rows[i]);
    }
    free(rows);
}

static int compare_network_profile_rows(const void *a, const void *b)
{
    const NetworkProfileRow *ra = (const NetworkProfileRow *) a;
    const NetworkProfileRow *rb = (const NetworkProfileRow *) b;

    int name_cmp = strcmp(ra->name, rb->name);
    if (name_cmp != 0) return name_cmp;

    return strcmp(ra->uuid, rb->uuid);
}

static int find_profile_by_uuid(const NetworkProfileRow *rows, size_t count, const char *uuid)
{
    if (rows == NULL || uuid == NULL) return -1;

    for (size_t i = 0; i < count; ++i)
    {
        if (rows[i].uuid != NULL && strcmp(rows[i].uuid, uuid) == 0)
        {
            return (int) i;
        }
    }

    return -1;
}

static int parse_network_profile_file(const char *path, NetworkProfileRow *row_out)
{
    if (path == NULL || row_out == NULL) return -1;

    FILE *f = fopen(path, "r");
    if (!f) return 1;

    NetworkProfileRow row = {0};
    char section[64] = "";
    int autoconnect = -1;

    char line[4096];
    while (fgets(line, sizeof(line), f))
    {
        trim_line(line);

        char *p = trim_whitespace(line);
        if (*p == '\0' || *p == '#' || *p == ';') continue;

        if (*p == '[')
        {
            char *end = strchr(p, ']');
            if (end == NULL) continue;
            *end = '\0';
            snprintf(section, sizeof(section), "%s", trim_whitespace(p + 1));
            continue;
        }

        if (strcmp(section, "connection") != 0) continue;

        char *eq = strchr(p, '=');
        if (eq == NULL) continue;

        *eq = '\0';
        char *key = trim_whitespace(p);
        char *value = trim_whitespace(eq + 1);

        if (strcmp(key, "id") == 0)
        {
            free(row.name);
            row.name = strdup(value);
            if (row.name == NULL)
            {
                fclose(f);
                free_network_profile_row(&row);
                return -1;
            }
        }
        else if (strcmp(key, "uuid") == 0)
        {
            free(row.uuid);
            row.uuid = strdup(value);
            if (row.uuid == NULL)
            {
                fclose(f);
                free_network_profile_row(&row);
                return -1;
            }
        }
        else if (strcmp(key, "type") == 0)
        {
            free(row.type);
            row.type = strdup(value);
            if (row.type == NULL)
            {
                fclose(f);
                free_network_profile_row(&row);
                return -1;
            }
        }
        else if (strcmp(key, "interface-name") == 0)
        {
            free(row.device);
            row.device = strdup(value);
            if (row.device == NULL)
            {
                fclose(f);
                free_network_profile_row(&row);
                return -1;
            }
        }
        else if (strcmp(key, "autoconnect") == 0)
        {
            autoconnect = parse_autoconnect_value(value);
        }
    }

    fclose(f);

    if (row.name == NULL || row.uuid == NULL || row.type == NULL)
    {
        free_network_profile_row(&row);
        return 1;
    }

    if (row.device == NULL)
    {
        row.device = strdup("--");
        if (row.device == NULL)
        {
            free_network_profile_row(&row);
            return -1;
        }
    }

    if (autoconnect == 1) row.state = strdup("auto");
    else if (autoconnect == 0) row.state = strdup("manual");
    else row.state = strdup("unknown");

    if (row.state == NULL)
    {
        free_network_profile_row(&row);
        return -1;
    }

    *row_out = row;
    return 0;
}

static int collect_network_profiles(NetworkProfileRow **rows_out, size_t *count_out)
{
    if (rows_out == NULL || count_out == NULL) return -1;

    static const char *dirs[] = {
        "/etc/NetworkManager/system-connections",
        "/run/NetworkManager/system-connections",
        "/usr/lib/NetworkManager/system-connections"
    };

    NetworkProfileRow *rows = NULL;
    size_t count = 0;
    size_t cap = 0;
    int failed = 0;

    for (size_t dir_i = 0; dir_i < sizeof(dirs) / sizeof(dirs[0]); ++dir_i)
    {
        DIR *dir = opendir(dirs[dir_i]);
        if (dir == NULL) continue;

        struct dirent *ent = NULL;
        while ((ent = readdir(dir)) != NULL)
        {
            if (
                strcmp(ent->d_name, ".") == 0 ||
                strcmp(ent->d_name, "..") == 0
            )
            {
                continue;
            }

            char path[PATH_MAX];
            int path_len = snprintf(path, sizeof(path), "%s/%s", dirs[dir_i], ent->d_name);
            if (path_len < 0 || path_len >= (int) sizeof(path)) continue;

            struct stat st;
            if (stat(path, &st) != 0 || !S_ISREG(st.st_mode)) continue;

            NetworkProfileRow row = {0};
            int rc = parse_network_profile_file(path, &row);
            if (rc > 0) continue;
            if (rc < 0)
            {
                failed = 1;
                break;
            }

            if (find_profile_by_uuid(rows, count, row.uuid) >= 0)
            {
                free_network_profile_row(&row);
                continue;
            }

            if (count == cap)
            {
                size_t new_cap = cap ? cap * 2 : 8;
                NetworkProfileRow *tmp = realloc(rows, new_cap * sizeof(*rows));
                if (tmp == NULL)
                {
                    free_network_profile_row(&row);
                    failed = 1;
                    break;
                }
                rows = tmp;
                cap = new_cap;
            }

            rows[count++] = row;
        }

        closedir(dir);
        if (failed) break;
    }

    if (failed)
    {
        free_network_profile_rows(rows, count);
        *rows_out = NULL;
        *count_out = 0;
        return -1;
    }

    if (count > 1)
    {
        qsort(rows, count, sizeof(*rows), compare_network_profile_rows);
    }

    *rows_out = rows;
    *count_out = count;
    return 0;
}

static int build_network_profiles_snapshot(
    NetworkProfileRow *rows,
    size_t count,
    char ***name_out,
    char ***uuid_out,
    char ***type_out,
    char ***device_out,
    char ***state_out
)
{
    if (
        name_out == NULL || uuid_out == NULL || type_out == NULL ||
        device_out == NULL || state_out == NULL
    )
    {
        return -1;
    }

    if (count == 0)
    {
        *name_out = NULL;
        *uuid_out = NULL;
        *type_out = NULL;
        *device_out = NULL;
        *state_out = NULL;
        return 0;
    }

    char **name = calloc(count, sizeof(*name));
    char **uuid = calloc(count, sizeof(*uuid));
    char **type = calloc(count, sizeof(*type));
    char **device = calloc(count, sizeof(*device));
    char **state = calloc(count, sizeof(*state));
    if (!name || !uuid || !type || !device || !state)
    {
        free(name);
        free(uuid);
        free(type);
        free(device);
        free(state);
        return -1;
    }

    for (size_t i = 0; i < count; ++i)
    {
        name[i] = rows[i].name;
        uuid[i] = rows[i].uuid;
        type[i] = rows[i].type;
        device[i] = rows[i].device;
        state[i] = rows[i].state;

        rows[i].name = NULL;
        rows[i].uuid = NULL;
        rows[i].type = NULL;
        rows[i].device = NULL;
        rows[i].state = NULL;
    }

    *name_out = name;
    *uuid_out = uuid;
    *type_out = type;
    *device_out = device;
    *state_out = state;
    return 0;
}

void* network_profiles_event_update(void *arg)
{
    NETPROF *netprof = (NETPROF *) arg;
    if (netprof == NULL) return NULL;

    while (1)
    {
        NetworkProfileRow *rows = NULL;
        size_t count = 0;

        if (collect_network_profiles(&rows, &count) != 0)
        {
            sleep(1);
            continue;
        }

        char **name = NULL;
        char **uuid = NULL;
        char **type = NULL;
        char **device = NULL;
        char **state = NULL;

        if (
            build_network_profiles_snapshot(
                rows,
                count,
                &name,
                &uuid,
                &type,
                &device,
                &state
            ) != 0
        )
        {
            free_network_profile_rows(rows, count);
            sleep(1);
            continue;
        }

        free(rows);

        NETPROF_update_data(
            netprof,
            (int) count,
            name,
            uuid,
            type,
            device,
            state
        );

        sleep(1);
    }

    return NULL;
}
