all:
	c++ melon.cpp -Wall -I/usr/X11R6/include -L/usr/X11R6/lib -lX11 -Ofast -o melon
	cc seed.c -Wall -I/usr/X11R6/include -L/usr/X11R6/lib -lX11 -Ofast -o seed

wm:
	c++ melon.cpp -Wall -I/usr/X11R6/include -L/usr/X11R6/lib -lX11 -Ofast -o melon

box:
	cc seed.c -Wall -I/usr/X11R6/include -L/usr/X11R6/lib -lX11 -Ofast -o seed

install:
	c++ melon.cpp -Wall -I/usr/X11R6/include -L/usr/X11R6/lib -lX11 -Ofast -o melon
	cc seed.c -Wall -I/usr/X11R6/include -L/usr/X11R6/lib -lX11 -Ofast -o seed
	install -m 755 melon /usr/local/bin/melon
	install -m 755 seed /usr/local/bin/seed

config:
	install melon.conf ~/.config/melon/
	install seed.conf ~/.config/melon/
