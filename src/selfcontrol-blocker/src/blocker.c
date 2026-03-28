#include "blocker.h"
#include <sys/stat.h>
#include <unistd.h>

static void
extract_domain (const char *input, char *output, size_t output_size)
{
  if (!input || !output || output_size == 0)
    return;

  char buffer[MAX_DOMAIN_LEN];
  strncpy (buffer, input, MAX_DOMAIN_LEN - 1);
  buffer[MAX_DOMAIN_LEN - 1] = '\0';

  char *start = buffer;
  while (*start == ' ')
    start++;

  if (strstr (start, "://"))
    {
      start = strstr (start, "://") + 3;
    }

  char *slash = strchr (start, '/');
  if (slash)
    {
      *slash = '\0';
    }

  char *colon = strchr (start, ':');
  if (colon)
    {
      *colon = '\0';
    }

  char *end = start + strlen (start) - 1;
  while (end > start && (*end == ' ' || *end == '\n' || *end == '\r'))
    {
      *end = '\0';
      end--;
    }

  for (char *p = start; *p; p++)
    {
      *p = tolower (*p);
    }

  strncpy (output, start, output_size - 1);
  output[output_size - 1] = '\0';
}

int
add_site_to_block (const char *domain)
{
  if (!domain || strlen (domain) == 0)
    return 0;

  char clean_domain[MAX_DOMAIN_LEN];
  extract_domain (domain, clean_domain, MAX_DOMAIN_LEN);

  if (strlen (clean_domain) == 0)
    return 0;

  if (is_site_blocked (clean_domain))
    return 1;

  backup_hosts_file ();

  FILE *hosts = fopen (HOSTS_FILE, "a");
  if (!hosts)
    return 0;

  fprintf (hosts, "%s %s\n", BLOCK_IP, clean_domain);
  fprintf (hosts, "%s www.%s\n", BLOCK_IP, clean_domain);

  fclose (hosts);

  system ("sudo systemd-resolve --flush-caches 2>/dev/null || true");
  system ("sudo killall -HUP dnsmasq 2>/dev/null || true");

  return 1;
}

int
remove_site_from_block (const char *domain)
{
  if (!domain)
    return 0;

  char clean_domain[MAX_DOMAIN_LEN];
  extract_domain (domain, clean_domain, MAX_DOMAIN_LEN);

  if (strlen (clean_domain) == 0)
    return 0;

  backup_hosts_file ();

  FILE *hosts = fopen (HOSTS_FILE, "r");
  if (!hosts)
    return 0;

  FILE *temp = fopen ("/tmp/hosts.tmp", "w");
  if (!temp)
    {
      fclose (hosts);
      return 0;
    }

  char line[512];
  int removed = 0;

  while (fgets (line, sizeof (line), hosts))
    {
      if (strstr (line, clean_domain) && strstr (line, BLOCK_IP))
        {
          removed++;
          continue;
        }
      fputs (line, temp);
    }

  fclose (hosts);
  fclose (temp);

  if (remove (HOSTS_FILE) != 0)
    return 0;
  if (rename ("/tmp/hosts.tmp", HOSTS_FILE) != 0)
    return 0;

  return removed > 0;
}

int
get_blocked_sites (char sites[MAX_SITES][MAX_DOMAIN_LEN], int max_sites)
{
  FILE *hosts = fopen (HOSTS_FILE, "r");
  if (!hosts)
    return 0;

  char line[512];
  int count = 0;

  while (fgets (line, sizeof (line), hosts) && count < max_sites)
    {
      if (strstr (line, BLOCK_IP))
        {
          char domain[MAX_DOMAIN_LEN];
          if (sscanf (line, "%*s %255s", domain) == 1)
            {
              char *clean_domain = domain;
              if (strncmp (domain, "www.", 4) == 0)
                {
                  clean_domain = domain + 4;
                }

              int duplicate = 0;
              for (int i = 0; i < count; i++)
                {
                  if (strcmp (sites[i], clean_domain) == 0)
                    {
                      duplicate = 1;
                      break;
                    }
                }

              if (!duplicate)
                {
                  size_t len = strlen (clean_domain);
                  if (len >= MAX_DOMAIN_LEN)
                    len = MAX_DOMAIN_LEN - 1;
                  memcpy (sites[count], clean_domain, len);
                  sites[count][len] = '\0';
                  count++;
                }
            }
        }
    }

  fclose (hosts);
  return count;
}

int
is_site_blocked (const char *domain)
{
  char sites[MAX_SITES][MAX_DOMAIN_LEN];
  int count = get_blocked_sites (sites, MAX_SITES);

  for (int i = 0; i < count; i++)
    {
      if (strcmp (sites[i], domain) == 0)
        {
          return 1;
        }
    }
  return 0;
}

int
backup_hosts_file (void)
{
  FILE *src = fopen (HOSTS_FILE, "r");
  if (!src)
    return 0;

  FILE *dst = fopen (HOSTS_BACKUP, "w");
  if (!dst)
    {
      fclose (src);
      return 0;
    }

  char buffer[1024];
  size_t bytes;
  while ((bytes = fread (buffer, 1, sizeof (buffer), src)) > 0)
    {
      fwrite (buffer, 1, bytes, dst);
    }

  fclose (src);
  fclose (dst);
  return 1;
}

int
restore_hosts_backup (void)
{
  FILE *src = fopen (HOSTS_BACKUP, "r");
  if (!src)
    return 0;

  FILE *dst = fopen (HOSTS_FILE, "w");
  if (!dst)
    {
      fclose (src);
      return 0;
    }

  char buffer[1024];
  size_t bytes;
  while ((bytes = fread (buffer, 1, sizeof (buffer), src)) > 0)
    {
      fwrite (buffer, 1, bytes, dst);
    }

  fclose (src);
  fclose (dst);
  return 1;
}

BlockerConfig *
init_blocker_config (void)
{
  BlockerConfig *config = malloc (sizeof (BlockerConfig));
  if (config)
    {
      config->site_count = 0;
      config->last_modify = 0;
      memset (config->blocked_sites, 0, sizeof (config->blocked_sites));
      get_blocked_sites (config->blocked_sites, MAX_SITES);
    }
  return config;
}

void
free_blocker_config (BlockerConfig *config)
{
  if (config)
    free (config);
}
