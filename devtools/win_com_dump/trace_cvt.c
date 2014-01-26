/***************************************************************************
*                                                                          *
* INSERT COPYRIGHT HERE!                                                   *
*                                                                          *
****************************************************************************
Filter that updates trace lines from 

  TRACE_MSG(TRACE_NWK3, "nlde_data h 0x%x pkt %p dst %hd rad %hd seq %d a_mode %hd disc_r %hd", (FMT__D_P_H_H_D_H_H,
                         nsdu->u.hdr.handle, nsdu, nwhdr->dst_addr, nwhdr->radius,
                         nwhdr->seq_num, nldereq->addr_mode, nldereq->discovery_route));

to

  TRACE_MSG(TRACE_NWK3, "nlde_data h 0x%x pkt %p dst %hd rad %hd seq %d a_mode %hd disc_r %hd", (FMT__D_P_H_H_D_H_H, (FMT__D_P_H_H_D_H_H,
                         nsdu->u.hdr.handle, nsdu, nwhdr->dst_addr, nwhdr->radius,
                         nwhdr->seq_num, nldereq->addr_mode, nldereq->discovery_route));
*/

#include <stdio.h>
#include <string.h>

int
main(
  int argc,
  char **argv)
{
  char s[1024];
  char fmt[20];
  char *p;
  int inside_trace = 0;
  FILE *f = fopen(argv[1], "r");
  int n;

  while(fgets(s, sizeof(s), f))
  {
    if (!inside_trace
        && strstr(s, "TRACE_MSG"))
    {
      inside_trace = 1;
    }
    /* remove FL, */
    if (inside_trace &&
        (p = strstr(s, "(FL, ")))
    {
      memmove(p, p+5, strlen(p+5)+1);
    }

    if (inside_trace && (p = strchr(s,'\"')))
    {
      n = 0;
      p++;
      strcpy(fmt, "FMT_");
      while (*p !='\"')
      {
        if (*p == '%')
        {
          p++;
          switch (*p)
          {
            case 'h':
              strcat(fmt, "_H");
              p++;
              n++;
              break;
            case 'l':
              strcat(fmt, "_L");
              p++;
              n+= 4;
              break;
            case 'u':
            case 'd':
            case 'i':
            case 'x':
              strcat(fmt, "_D");
              n += 2;
              break;
            case 'p':
              strcat(fmt, "_P");
              n += 3;
              break;
            case 'c':
              strcat(fmt, "_C");
              n++;
              break;
          }
        }
        p++;
        if (!strncmp(p, "\" TRACE_FORMAT_64,", 18))
        {
          strcat(fmt, "_A");
          p += 16;
          n += 8;
          break;
        }
        else if (!strncmp(p, "\" TRACE_FORMAT_64 \"", 19))
        {
          strcat(fmt, "_A");
          p += 19;
          n += 8;
        }
      } /* while */

      if (!strcmp(fmt, "FMT_"))
      {
        strcat(fmt, "_0");
      }

      fprintf(stderr, "#define %s\t\t%d\n", fmt, n);

      p++;
      memmove(p + strlen(fmt)+3, p, strlen(p)+1);
      *p++ = ',';
      *p++ = ' ';
      *p++ = '(';
      memcpy(p, fmt, strlen(fmt));
    }

    if (inside_trace
        && strchr(s, ';'))
    {
      inside_trace = 0;
    }

    fputs(s, stdout);
    *s = 0;
  }
  return 0;
}
