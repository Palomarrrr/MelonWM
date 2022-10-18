all:
	cc MelonWM.c -Wall -I/usr/X11R6/include -L/usr/X11R6/lib -lX11 -Ofast -o MelonWM
	cc MelonSeed.c -Wall -I/usr/X11R6/include -L/usr/X11R6/lib -lX11 -Ofast -o MelonSeed

wm:
	cc MelonWM.c -Wall -I/usr/X11R6/include -L/usr/X11R6/lib -lX11 -Ofast -o MelonWM

box:
	cc MelonSeed.c -Wall -I/usr/X11R6/include -L/usr/X11R6/lib -lX11 -Ofast -o MelonSeed

install:
	cc MelonWM.c -Wall -I/usr/X11R6/include -L/usr/X11R6/lib -lX11 -Ofast -o MelonWM
	cc MelonSeed.c -Wall -I/usr/X11R6/include -L/usr/X11R6/lib -lX11 -Ofast -o MelonSeed
	install -m 755 MelonWM /usr/local/bin/MelonWM
	install -m 755 MelonSeed /usr/local/bin/MelonSeed

config:
	install MelonWM.conf ~/.config/MelonWM/
	install MelonSeed.conf ~/.config/MelonWM/
