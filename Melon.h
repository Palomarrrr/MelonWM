//Standard headers
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <locale.h>
#include <err.h>
#include <limits.h>
#include <time.h>

//X11 Headers
#include <X11/X.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>
#include <X11/XF86keysym.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/xpm.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <X11/Xproto.h>
#include <X11/Xresource.h>
#include <X11/Xthreads.h>
#include <X11/Xft/Xft.h>

enum{
	EXIT_KEY,
	FOCUS_KEY,
	MINI_KEY,
	RESTO_KEY,
	KILL_KEY,
	MOVE_KEY,
	SIZE_KEY,
	
	LAST_KEY
};

enum{
	INPUT_FIELD,
	KEY_FIELD,
	MOD_FIELD,

	LAST_FIELD
};

enum{
	COLOR_INACTIVE,
	THICKNESS_INACTIVE,
	COLOR_ACTIVE,
	THICKNESS_ACTIVE,

	LAST_PARAMETER
};

typedef struct FontContext{
	XftDraw *draw;
	XftColor fontColor;
	char *colorName;
	Drawable pixmap;
	Visual *vis;
	Colormap colorMap;
	int depth;
	char *fontName;
	XftFont *font;
}FontContext;

typedef struct Client{
	Display *dpy; // pointer to the display the window is active on
	Window *parent, win; //the windows parent and the window being given an id
	unsigned int coords[2]; //0 = x, 1 = y
	unsigned int dimensions[2]; //0 = wid, 1 = hgt
	unsigned long int borderColor;
	int isMini;
	unsigned long indPrev; //the previous client in the list
} Client;

//Stores all the keys that perform important functions
//These all have really bad defaults just in case something breaks
unsigned int key[LAST_FIELD][LAST_KEY];
unsigned long wmParams[LAST_PARAMETER];

typedef struct WMProps {
	Display *dpy;
	XWindowAttributes attr;
	XEvent ev, start;
	Window root;
	int scr;
	size_t numClients;
	Client *clients;
	char *configPath, logFilePath;
}WMProps;

typedef struct Command {
	char command[200];
	unsigned int x, y;
}Command;

typedef struct BoxParams {
	long refreshRate;
	int wid, hgt, x_coord, y_coord;
	unsigned long backgroundColor, foregroundColor;
	FontContext fontCtx;
}BoxParams;

//Implementing this soon
typedef struct BoxProps {
	Display *dpy;
	int scr;
	Window win;
	GC gc;
	Command *commands;
	int numCmds;
	BoxParams boxParams;
	char *configPath, *logFilePath;
	char *fontName;
} BoxProps;
