/***************************************************************************
*                                                                          *
* INSERT COPYRIGHT HERE!                                                   *
*                                                                          *
****************************************************************************
PURPOSE: Console Windows utility which reads serial port and writes trace and
traffic dump to files.
*/

#include <windows.h>
#include <stdio.h>

#define BAUD_RATE CBR_115200
void parse_trace_line(char *str, FILE *outf);

int is_iar = 0;


int
main(
  int argc,
  char **argv)
{
  HANDLE comf;
  FILE *tracef;
  FILE *dumpf;
  DCB dcb;
  int ret = 0;
  int eof = 0;
  int rd;
  unsigned char buf[256];
  int shift = 0;
  int i = 1;

  FILE *fraw = fopen("fraw.bin", "wb");

  if (argc < 4)
  {
	  fprintf(stderr, "Usage: win_com_dump {-I} {com_port_name} {trace_file_name} {dump_file_name}\n"
            "Example: win_com_dump.exe -I \\\\.\\COM11 trace.log traf.dump\n"
			);
    return -1;
  }
  if (!strcmp(argv[i], "-I"))
  {
    i++;
    is_iar = 1;
  }
//  comf = CreateFile(argv[i], GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
  comf = CreateFileA(argv[i], GENERIC_READ|GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
  if (comf == INVALID_HANDLE_VALUE)
  {
    fprintf(stderr, "Can't open serial port %s : %d\n", argv[i], GetLastError());
    return -1;
  }
  i++;
  tracef = fopen(argv[i], "wb");
  if (!tracef)
  {
    fprintf(stderr, "Can't open trace file %s\n", argv[i]);
    return -1;
  }
  i++;
  dumpf = fopen(argv[i], "wb");
  if (!dumpf)
  {
    fprintf(stderr, "Can't open dump file %s\n", argv[3]);
    return -1;
  }

  SetupComm(comf, 1024, 1024);


  if (GetCommState(comf, &dcb) == 0)
  {
    fprintf(stderr, "GetCommState error %d\n", GetLastError());
    return -1;
  }

  dcb.DCBlength = sizeof(DCB);
  dcb.BaudRate = BAUD_RATE;
  dcb.ByteSize = 8;
  dcb.StopBits = ONESTOPBIT;
  dcb.fOutxDsrFlow = 0;
  dcb.fDtrControl = DTR_CONTROL_DISABLE;
  dcb.fOutxCtsFlow = 0;
  dcb.fRtsControl = RTS_CONTROL_ENABLE;
  dcb.XonLim = 100;
  dcb.XoffLim = 100;
  //  Set the software flow control
  dcb.fInX = dcb.fOutX = 0;
#define ASCII_XON 0x11
#define ASCII_XOFF  0x13
  dcb.XonChar = ASCII_XON;
  dcb.XoffChar = ASCII_XOFF;
  //  Set other stuff
  dcb.fBinary = TRUE;
  dcb.fParity = FALSE;
  dcb.fDsrSensitivity = 0;
  dcb.fNull = 0;
  dcb.fAbortOnError = 0;
  dcb.Parity = NOPARITY;
  dcb.fErrorChar = 0;
  dcb.ErrorChar = 0;
  if (!SetCommState(comf, &dcb))
  {
    fprintf(stderr, "SetCommState error %d\n", GetLastError());
    return -1;
  }

  rd = 0;
  shift = 0;
  while (!eof)
  {
    while (!eof && shift != 4)
    {
      if (!ReadFile(comf, buf + shift, 4 - shift, &ret, 0))
      {
        eof = 1;
        continue;
      }
      shift += ret;
    }
    
    /* sync with signature */
    if (!(buf[0] == 0xde && buf[1] == 0xad && buf[2] > 2 &&
          (buf[3] == 2 || buf[3] == 0x81 || buf[3] == 1)))
    {
      /* resyncronize */
      memmove(&buf[0], &buf[1], 3);
      shift = 3;
      continue;
    }

    memmove(&buf[0], &buf[2], 2);

    rd = 2;

    while (!eof && rd < buf[0])
    {
      if (!ReadFile(comf, buf + rd, buf[0] - rd, &ret, 0))
      {
        eof = 1;
      }
      else
      {
        rd += ret;
      }
    }

    if (!eof)
    {
      fwrite(buf, 1, buf[0], fraw);
      fflush(fraw);

      if (buf[1] == 2)
      {
        static unsigned char big_buf[512];
        /* trace */
//        fwrite(buf + 4, 1, buf[0] - 4, tracef);
        buf[buf[0]] = 0;
        if (buf[buf[0] - 1] != '\n' || *big_buf != 0)
        {
          strcat(big_buf, buf + 4);
        }
        if (buf[buf[0] - 1] == '\n')
        {
          if (*big_buf)
          {
            parse_trace_line(big_buf, tracef);
            *big_buf = 0;
          }
          else
          {
            parse_trace_line(buf + 4, tracef);
          }
        }
        fflush(tracef);
      }
      else
      {
        /* dump */
        fwrite(buf, 1, buf[0], dumpf);
        fflush(dumpf);
      }
    }
  } /* while */

  CloseHandle(comf);
  fclose(tracef);
  fclose(dumpf);

  return 0;
}
