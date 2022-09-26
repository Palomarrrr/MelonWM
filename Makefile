all:
	c++ cppwm.cpp -Wall -I/usr/X11R6/include -L/usr/X11R6/lib -lX11 -Ofast -o cppwm
	cc pbox.c -Wall -I/usr/X11R6/include -L/usr/X11R6/lib -lX11 -Ofast -o pbox

install:
	c++ cppwm.cpp -Wall -I/usr/X11R6/include -L/usr/X11R6/lib -lX11 -Ofast -o cppwm
	cc pbox.c -Wall -I/usr/X11R6/include -L/usr/X11R6/lib -lX11 -Ofast -o pbox
	install -m 755 cppwm /usr/local/bin/cppwm
	install -m 755 pbox /usr/local/bin/pbox

config:
	install -m 755 config ~/.config/cppwm/config
