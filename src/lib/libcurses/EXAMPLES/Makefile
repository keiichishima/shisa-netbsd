#$Id: Makefile,v 1.1 2007/01/21 11:38:59 blymn Exp $
#$Log: Makefile,v $
#Revision 1.1  2007/01/21 11:38:59  blymn
#Wide curses merge
#
#Revision 1.1  2005/08/31 14:52:08  ruibiao
#
#A patach for the current source
#
#Revision 1.2  2005/08/23 21:29:08  ruibiao
#
#Changed to the right libraries
#
#Revision 1.1  2005/08/23 16:38:22  ruibiao
#
#Initial version, to build all versions of file views and the unit tester
#

RM = /bin/rm -f
CFLAGS =

.if defined(DEBUG)
CFLAGS+= -g
.endif

all: wcview nwview ccview tcview ncview ex1

wcview: view.c
	gcc -DHAVE_WCHAR -o wcview view.c -I.. $(CFLAGS) -lwcurses -Wl,-rpath,.. -L..

nwview: view.c
	gcc -DHAVE_WCHAR -o nwview view.c -DNCURSES -I/usr/pkg/include $(CFLAGS) -lncursesw -Wl,-rpath,/usr/pkg/lib -L/usr/pkg/lib

ccview: view.c
	gcc -o ccview view.c -I.. $(CFLAGS) -lcurses -Wl,-rpath,.. -L..

tcview: view.c
	gcc -o tcview view.c -L/usr/lib -Wl,-rpath,/usr/lib $(CFLAGS) -lcurses

ncview: view.c
	gcc -o ncview view.c -I/usr/pkg/include -DNCURSES -L/usr/pkg/lib -Wl,-rpath,/usr/pkg/lib $(CFLAGS) -lncurses

ex1: ex1.c
