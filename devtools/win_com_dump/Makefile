all: win_com_dump.exe

win_com_dump.exe: win_com_dump.c parse_trace.c
	cl win_com_dump.c parse_trace.c

clean:
	rm -f *.obj *.exe

rebuild: clean all