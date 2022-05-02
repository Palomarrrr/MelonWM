all:
	g++ -O3 -pedantic -Wall -I/usr/X11R6/include cppwm.cpp -L/usr/X11R6/lib -lX11 -o cppwm
install:
	g++ -O3 -pedantic -Wall -I/usr/X11R6/include cppwm.cpp -L/usr/X11R6/lib -lX11 -o cppwm -std=c++17
#	g++ -O3 -pedantic -Wall -I/usr/X11R6/include stable.cpp -L/usr/X11R6/lib -lX11 -o cppwmStable -std=c++17
	install -m 755 cppwm /usr/local/bin/cppwm
