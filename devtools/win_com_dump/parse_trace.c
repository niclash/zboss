/***************************************************************************
*                                                                          *
* INSERT COPYRIGHT HERE!                                                   *
*                                                                          *
****************************************************************************
PURPOSE: Parse trace file
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>

#define FILE_NAME_LEN 150
#define N_FILES 200
#define N_TRACE_LINES_PER_FILE 500

typedef struct cache_line_s
{
  int line1;
  int line2;
  char *fmt;
} cache_line_t;

typedef struct file_cache_s
{
  char file_name[FILE_NAME_LEN];
  int n_lines;
  cache_line_t lines[N_TRACE_LINES_PER_FILE];
} file_cache_t;

extern int is_iar;

int n_files;
file_cache_t files[N_FILES];

static int get_file_cache(char *name, file_cache_t **fc);
static int get_format(file_cache_t *fc, int line, char **fmt);
static void parse_dump(char *format, char *s, FILE *f);
static int get_bytes(char *s, unsigned char *buf, int n_bytes);
static int get_bytes_be(char *s, unsigned char *buf, int n_bytes);
static int load_file(char *name, file_cache_t *fc);
static FILE *open_source_file(char *name);
static FILE *open_source_recursive(char *name, char *root);

void parse_trace_line(char *str, FILE *outf)
{
  char file[FILE_NAME_LEN];
  int line;
  int off;
  file_cache_t *fc;
  char *fmt;
  char *p;

  sscanf(str, "%*d%*d%*[ ]%s%n", file, &off);
  p = strrchr(file, ':');
  if (p)
  {
	  *p = 0;
	  p++;
	  sscanf(p, "%d", &line);
  }
  if (get_file_cache(file, &fc))
  {
    fputs(str, outf);
    return;
  }
  if (get_format(fc, line, &fmt))
  {
    fputs(str, outf);
    return;
  }

  //fputs(str, outf);
  fwrite(str, off + 1, 1, outf);
  parse_dump(fmt, str + off + 1, outf);
  fflush(outf);

}


static int get_file_cache(char *name, file_cache_t **fc)
{
  int i;
  for (i = 0 ; i < n_files ; ++i)
  {
    if (!strcmp(name, files[i].file_name))
    {
      *fc = &files[i];
      return 0;
    }
  }
  *fc = &files[i];
  n_files++;
  return load_file(name, &files[i]);
}


static int get_format(file_cache_t *fc, int line, char **fmt)
{
  int i;
  for (i = 0 ; i < fc->n_lines ; ++i)
  {
    if (fc->lines[i].line1 == line || fc->lines[i].line2 == line)
    {
      *fmt = fc->lines[i].fmt;
      return 0;
    }
  }
  return -1;
}


/* trace function which does not use printf() */

#define PRINTU_MACRO(v, f)                      \
{                                               \
  char s[10];                                   \
  int i = 10;                            \
  do                                            \
  {                                             \
    s[--i] = '0' + (v) % 10;                    \
    v /= 10;                                    \
  }                                             \
  while (v);                                    \
  while (i < 10)                                \
  {                                             \
    putc(s[i], f);                              \
    i++;                                        \
  }                                             \
}

static char s_x_tbl[] = "0123456789abcdef";

static void printx2(unsigned char v, FILE *f)
{
  putc(s_x_tbl[((v) >> 4) & 0xf], f);
  putc(s_x_tbl[((v) & 0xf)], f);
}

#define PRINTX(v, f)                            \
{                                               \
  char s[10];                                   \
  int i = 10;                            \
  do                                            \
  {                                             \
    s[--i] = s_x_tbl[(v) & 0xf];                \
    (v) >>= 4;                                  \
  }                                             \
  while (v);                                    \
  while (i < 10)                                \
  {                                             \
    putc(s[i], f);                              \
    i++;                                        \
  }                                             \
}

/**
   Output trace message.

   @param file_name - source file name
   @param line_number - source file line
   @param mask - layers mask of the current message. Do trace if mask&ZB_TRACE_MASK != 0
   @param level - message trace level. Do trace if level <= ZB_TRACE_LEVEL
   @param format - printf-like format string
 */

static void parse_dump(char *format, char *s, FILE *f)
{
  if (!format || !(*format))
  {
    putc('\n', f);
  }
  else
  {
    /*
      Implement printf-like formatting.

      Understands %u, %d, %i, %c, %x, %s, %p, %ld, %lu, %lx.
      Not so complex as SDCC's printf_large but in differs with its printf_small
      understands %p. Do not understand %o - we never use it.
    */
    for ( ; *format ; format++)
    {
      int l;
      int h;
      int u = 0;
      if (*format != '%')
      {
        putc(*format, f);
        continue;
      }
      format++;
      l = (*format == 'l');
      h = (*format == 'h');
      if (l || h)
      {
        format++;
      }
      switch (*format)
      {
        case 'u':
          u = 1;
          /* FALLTHROUGH */
        case 'd':
        case 'i':
          if (l)
          {
            unsigned int v;
            s += get_bytes_be(s, (unsigned char *)&v, 4);
            if (!u && (int)v < 0)
            {
              putc('-', f);
              v = -((int)v);
            }
            PRINTU_MACRO(v, f);
          }
          else if (h)
          {
            unsigned char v;
            s += get_bytes(s, &v, 1);
            if (!u && (signed char)v < 0)
            {
              putc('-', f);
              v = -((signed char)v);
            }
            PRINTU_MACRO(v, f);
          }
          else
          {
            unsigned short v;
            s += get_bytes_be(s, (unsigned char *)&v, 2);
            if (!u && (short)v < 0)
            {
              putc('-', f);
              v = -((short)v);
            }
            PRINTU_MACRO(v, f);
          }
          continue;
        case 'x':
          if (l)
          {
            unsigned int v;
            s += get_bytes_be(s, (unsigned char *)&v, 4);
            PRINTX(v, f);
          }
          else if (h)
          {
            unsigned char v;
            s += get_bytes(s, &v, 1);
            PRINTX(v, f);
          }
          else
          {
            unsigned short v;
            s += get_bytes_be(s, (unsigned char *)&v, 2);
            PRINTX(v, f);
          }
          continue;
        case 'c':
        {
          unsigned char v;
          s += get_bytes(s, &v, 1);
          putc(v, f);
          continue ;
        }
        case 'p':
        {
          unsigned char b[3];
          if (!is_iar)
          {
            s += get_bytes(s, b, 3);
            printx2(b[0], f);
            printx2(b[1], f);
            printx2(b[2], f);
          }
          else
          {
            /* IAR use 2-bytes pointers */
            s += get_bytes(s, b, 2);
            printx2(b[0], f);
            printx2(b[1], f);
          }
        }
        continue;

        case 'A':
        {
          int i;
          unsigned char b[8];
          s += get_bytes(s, b, 8);
          for (i = 0 ; i < 8 ; ++i)
          {
            printx2(b[i], f);
          }
          continue ;
        }
        case 's':
        {
          fprintf(f, "%%s not supported!");
          continue;
        }
      }
    }
    if (format[-1] != '\n')
    {
      putc('\n', f);
    }
  }
}


static int get_bytes(char *s, unsigned char *buf, int n_bytes)
{
  int i;

  for (i = 0 ; i < n_bytes ; ++i)
  {
    int v;
    sscanf(s, "%02x", &v);
    buf[i] = v;
    s += 2;
  }

  /* IAR casts bytes to 16-bit integer when pass it as varargs. Also, IAR is
   * little-endian, so can safely skip second byte */
  return (n_bytes + (is_iar && n_bytes == 1)) * 2;
}


static int get_bytes_be(char *s, unsigned char *buf, int n_bytes)
{
  if (!is_iar)
  {
    int i = n_bytes - 1;
    while (i >= 0)
    {
      int v;
      sscanf(s, "%02x", &v);
      buf[i] = v;
      i--;
      s += 2;
    }

    return n_bytes * 2;
  }
  else
  {
    /* IAR is little-endian while Keil is big-endian */
    return get_bytes(s, buf, n_bytes);
  }
}



static int load_file(char *name, file_cache_t *fc)
{
  char buf[512];
  FILE *f = open_source_file(name);
  int n_lines = 0;
  int line = 0;
  char *p;
  char *p_end;
  char *p_end2;
  char *p_o;
  int skip;
  int trace_msg = 0;

  if (!f)
  {
    return -1;
  }
  strcpy(fc->file_name, name);
  while (fgets(buf, sizeof(buf), f))
  {
    line++;
    if (strstr(buf, "TRACE_MSG"))
    {
      trace_msg = 1;
      fc->lines[n_lines].line1 = line;
    }
    if (trace_msg
        && ((p = strchr(buf, '\"'))
            || (p = strstr(buf, "TRACE_FORMAT_64"))))
    {
      trace_msg = 0;
      p_end2 = strstr(p+1, "TRACE_FORMAT_64");
      while (p_end2)
      {
        p_end = strstr(p_end2+1, "TRACE_FORMAT_64");
        if (p_end)
        {
          p_end2 = p_end;
        }
        else
        {
          break;
        }
      }

      p_end = strrchr(buf, '\"');
      
      if (!p_end || p_end == p || (p_end && p_end2 && p_end2 > p_end))
      {
        if (!p_end2)
        {
          continue;
        }
        p_end = p_end2 + strlen("TRACE_FORMAT_64");
      }
      p++;

      fc->lines[n_lines].fmt = (char *)malloc(p_end - p + 1);

      p_o = fc->lines[n_lines].fmt;
      skip = 0;
      while (p != p_end)
      {
        if (!strncmp(p, "TRACE_FORMAT_64", 15))
        {
          *p_o++ = '%';
          *p_o++ = 'A';
          p += 15;
          skip = !skip;
          continue;
        }
        if (*p == '\"')
        {
          skip = !skip;
        }
        if (!skip)
        {
          *p_o++ = *p;
        }
        p++;
      }
      *p_o = 0;

      if (!strchr(p_end, ';'))
      {
        while (!strchr(buf, ';'))
        {
          if (!fgets(buf, sizeof(buf), f))
          {
            break;
          }
          line++;
        }
      }
      fc->lines[n_lines].line2 = line;

      n_lines++;
    }
    fc->n_lines = n_lines;
  }

  fclose(f);
  return 0;
}


/**
   Try to open source file.
   
   If name has no path and file can't be open, search in subdirectories.
 */
static FILE *open_source_file(char *name)
{
  FILE *f;
  if (!strchr(name, '/') && !strchr(name, '\\'))
  {
    /* name without path - search in subdirs */
    f = open_source_recursive(name, ".");
  }
  else
  {
    f = fopen(name, "r");
  }
  return f;
}


static FILE *open_source_recursive(char *name, char *root)
{
  char pattern[512];
  HANDLE h;
  WIN32_FIND_DATAA find_data;
  FILE *f;
  BOOL valid = TRUE;
  
  strcpy(pattern, root);
  strcat(pattern, "\\");
  strcat(pattern, name);
  f = fopen(pattern, "r");

  if (f)
  {
    return f;
  }

  strcpy(pattern, root);
  strcat(pattern, "\\*.*");
  for (h = FindFirstFileA(pattern, &find_data);
       f == NULL && h != INVALID_HANDLE_VALUE && valid ;
       valid = FindNextFileA(h, &find_data))
  {
    if (strcmp(find_data.cFileName, ".svn")
		&& strcmp(find_data.cFileName, ".")
		&& strcmp(find_data.cFileName, "..")
		&& strcmp(find_data.cFileName, "mk")
        && find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
      strcpy(pattern, root);
      strcat(pattern, "\\");
      strcat(pattern, find_data.cFileName);
      f = open_source_recursive(name, pattern);
    }
  }
  if (h != INVALID_HANDLE_VALUE)
  {
    FindClose(h);
  }
  return f;
}
