/***************************************************************************
*                                                                          *
* INSERT COPYRIGHT HERE!                                                   *
*                                                                          *
****************************************************************************
PURPOSE:
*/

#include <stdio.h>
int
main(
  int argc,
  char **argv)
{
  char s[150];
  char def[80];
  char *p;
  int size;

  while(fgets(s, sizeof(s), stdin))
  {
    sscanf(s, "%*s%s", def);
    p = def+5;
    size = 0;
    while (*p)
    {
      switch(*p)
      {
        case 'P':
          size += 3;
          break;
        case 'A':
          size += 8;
          break;
        case 'H':
        case 'C':
          size++;
          break;
        case 'D':
          size += 2;
          break;
        case 'L':
          size += 4;
          break;
      }
      p++;
    }
    printf("#define %-*s __FILE__,__LINE__, %d\n", 40, def, size);
  }
}

