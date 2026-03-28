#ifndef BLOCKER_H
#define BLOCKER_H

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define HOSTS_FILE "/etc/hosts"
#define HOSTS_BACKUP "/etc/hosts.backup"
#define BLOCK_IP "127.0.0.1"
#define MAX_SITES 1000
#define MAX_DOMAIN_LEN 256

typedef struct
{
  char blocked_sites[MAX_SITES][MAX_DOMAIN_LEN];
  int site_count;
  time_t last_modify;
} BlockerConfig;

BlockerConfig *init_blocker_config (void);
int add_site_to_block (const char *domain);
int remove_site_from_block (const char *domain);
int get_blocked_sites (char sites[MAX_SITES][MAX_DOMAIN_LEN], int max_sites);
int is_site_blocked (const char *domain);
int backup_hosts_file (void);
int restore_hosts_backup (void);
void free_blocker_config (BlockerConfig *config);

#endif
