all:
	g++ -Os -pedantic -Wall -I/usr/X11R6/include cppwm.cpp -L/usr/X11R6/lib -lX11 -o cppwm
install:
	g++ -Os -pedantic -Wall -I/usr/X11R6/include cppwm.cpp -L/usr/X11R6/lib -lX11 -o cppwm
	install -m 755 cppwm /usr/local/bin/cppwm
