/***************************************************************************
*                                                                          *
* INSERT COPYRIGHT HERE!                                                   *
*                                                                          *
****************************************************************************
Count sizes of trace arguments for IAR
*/

#include <stdio.h>
#include <string.h>

int
main(
  int argc,
  char **argv)
{
  char s[256];
  (void)argv;
  (void)argc;
  while (gets(s))
  {
    int len = 0;
    char *p = strstr(s, "FMT__");
    if (p)
    {
      p += 5;
      while (*p != ' ' && *p != '\t')
      {
        switch(*p)
        {
          case 'H':
          case 'D':
          case 'C':
            len += 2;
            break;
          case 'P':
            len += 2;           /* maybe, 3? */
            break;
          case 'A':
            len += 8;
            break;
          case 'L':
            len += 4;
            break;
        }
        p += 2;
      }
      p = strrchr(s, ',');
      p[1] = 0;
      printf("%s %d\n", s, len);
    }
    else
    {
      puts(s);
    }
  }
  return 0;
}

