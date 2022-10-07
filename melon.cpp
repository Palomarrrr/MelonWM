//Headers needed to make things work
//I can probably get rid of some of these and
//probably will sometime in the future

#include <iostream>
#include <cstdlib>
#include <vector>
#include <fstream>
#include <cmath>
#include <string>
#include <X11/X.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>
#include <X11/XF86keysym.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//This is used in resizing and moving windows
#define MAX(a, b) ((a) > (b) ? (a) : (b))

//This just makes things easier for me so I don't have to keep typing std::
using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::fstream;

//This holds all data that I'd need to manipulate windows
typedef struct Client
{
	Display *dpy; // pointer to the display the window is active on
	Window *parent, win; //the windows parent and the window being given an id
	unsigned int coords[2]; //0 = x, 1 = y
	unsigned int dimensions[2]; //0 = wid, 1 = hgt
	unsigned long int borderColor;
	bool isMini;
	int indPrev, indNext; //the previous and next client in the list
} Client;

//Stores all the keys that perform important functions
//These all have really bad defaults just in case something breaks
typedef struct FunctionKeys
{
	unsigned int modKey = Mod4Mask;
	string killWMKey = "x";
	string focusWinKey = "f";
	string miniWinKey = "q";
	string restoreWinKey = "w";
	string killWinKey = "2";

} FunctionKeys;

//Initing variables. I know I should make these as local as possible.
//I'll be working on that sometime soon because it bothers me too.
Display *dpy;
XWindowAttributes attr;
XEvent ev, start;
static Window root;
int scr, inactBorderThcknss = 1, actBorderThcknss = 1;
unsigned long int inactiveHex = 0x000000, activeHex = 0xf0fff0, lastFocusedClient, numMini;
string configPath;
vector<Client> clients;
FunctionKeys funcKeys;

void readConfig()
{
	FILE *config;
	char *currLine = new char[200];
	char *buffer = new char[200];
	char *ptr;
	int fieldToFill = 0;

	cout << configPath << endl;

	config = fopen(configPath.c_str(), "r");

	if(config == NULL)
	{
		cout << "FAILED TO READ THE CONFIG FILE AT PATH: " << configPath << endl;
		exit(1);
	}

	memset(buffer, 0, sizeof(char) * 200);

	while(fgets(currLine, 200, config) != NULL)
	{
		int j = 0;

		if(currLine[0] == '\n' || currLine[0] == '#')
		{
			continue;
		}
		else if(strcmp(currLine, "[WM PARAMETERS]\n") == 0)
		{
			memset(currLine, 0, sizeof(char) * 200);
			continue;
		}
		else if(strcmp(currLine, "[FUNCTION KEYS]\n") == 0)
		{
			memset(currLine, 0, sizeof(char) * 200);
			continue;
		}
		else
		{
			for(int x = 0; x < strlen(currLine); x++)
			{
				if(currLine[x] == ' ')
				{
					continue;
				}
				else if(currLine[x] == '=')
				{
					if(strcmp(buffer, "BORDER_INACTIVE_COLOR") == 0)
					{
						fieldToFill = 1;
					}
					else if(strcmp(buffer, "BORDER_INACTIVE_THICKNESS") == 0)
					{
						fieldToFill = 2;
					}
					else if(strcmp(buffer, "BORDER_ACTIVE_COLOR") == 0)
					{
						fieldToFill = 3;
					}
					else if(strcmp(buffer, "BORDER_ACTIVE_THICKNESS") == 0)
					{
						fieldToFill = 4;
					}
					else if(strcmp(buffer, "MODIFIER_KEY") == 0)
					{
						fieldToFill = 5;
					}
					else if(strcmp(buffer, "KILL_WM_KEY") == 0)
					{
						fieldToFill = 6;
					}
					else if(strcmp(buffer, "FOCUS_WINDOW_BUTTON") == 0)
					{
						fieldToFill = 7;
					}
					else if(strcmp(buffer, "MINI_WINDOW_BUTTON") == 0)
					{
						fieldToFill = 8;
					}
					else if(strcmp(buffer, "RESTORE_WINDOW_BUTTON") == 0)
					{
						fieldToFill = 9;
					}
					else if(strcmp(buffer, "KILL_WINDOW_BUTTON") == 0)
					{
						fieldToFill = 10;
					}
					else
					{
						cout << "INVALID FIELD " << buffer << " FOUND" << endl;
						exit(1);
					}
					memset(buffer, 0, sizeof(char) * 200);
					j = 0;
					continue;
				}
				else if(x == strlen(currLine) - 1)
				{
					switch(fieldToFill)
					{
						case 1:
							inactiveHex = strtoul(buffer, &ptr, 16);
							fieldToFill = 0;
							break;
						case 2:
							inactBorderThcknss = atoi(buffer);
							fieldToFill = 0;
							break;
						case 3:
							activeHex = strtoul(buffer, &ptr, 16);
							fieldToFill = 0;
							break;
						case 4:
							actBorderThcknss = atoi(buffer);
							fieldToFill = 0;
							break;
						case 5:
							funcKeys.modKey = atoi(buffer);
							switch(funcKeys.modKey)
							{
								case 1:
									funcKeys.modKey = Mod1Mask;
									break;
								case 2:
									funcKeys.modKey = Mod2Mask;
									break;
								case 3:
									funcKeys.modKey = Mod3Mask;
									break;
								case 4:
									funcKeys.modKey = Mod4Mask;
									break;
								case 5:
									funcKeys.modKey = Mod5Mask;
									break;
								default:
									funcKeys.modKey = Mod4Mask;
									break;
							}
							break;
						case 6:
							funcKeys.killWMKey = buffer;
							fieldToFill = 0;
							break;
						case 7:
							funcKeys.focusWinKey = buffer;
							fieldToFill = 0;
							break;
						case 8:
							funcKeys.miniWinKey = buffer;
							fieldToFill = 0;
							break;
						case 9:
							funcKeys.restoreWinKey = buffer;
							fieldToFill = 0;
							break;
						case 10:
							funcKeys.killWinKey = buffer;
							fieldToFill = 0;
							break;
						default:
							cout << "INVALID ASSIGNMENT " << buffer << " TO FIELD " << fieldToFill << endl;
							exit(1);
					}
					j = 0;
					memset(buffer, 0, sizeof(char) * 200);
					break;
				}
				buffer[j] = currLine[x];
				j++;
			}
		}
	}


	if(fclose(config) != 0)
	{
		cout << "THERE WAS AN ERROR CLOSING THE CONFIG FILE" << endl;
	}
}

int findFlagRequest(char* argv)
{

	if(strcmp("-h", argv) == 0 || strcmp("--help", argv) == 0)
	{
		return 0;
	}
	else if(strcmp("-c", argv) == 0)
	{
		return 1;
	}
	else if(strcmp("-n", argv) == 0)
	{
		return 2;
	}
	else
	{
		return -1;
	}

}

void processFlags(int argc, char** argv)
{
	if(argc == 0)
		return;

	for(int i = 1; i < argc; i++)
	{
		int request = findFlagRequest(argv[i]);

		switch(request){
			case 0:
				cout << "MELON WINDOW MANAGER" << endl;
				cout << "  -c /path/to/file  Uses a specific path for config " << endl;
				cout << "  -n                Runs without a config" << endl;
				cout << "  -h                Displays this message" << endl;
				exit(1);
			case 1:
				i++;
				configPath = argv[i];
				break;
			case 2:
				configPath = "";
				break;
			default:
				cout << "Invalid flag \"" << argv[i] << "\"" << endl;
				cout << "Check -h for more information" << endl;
				exit(1);
		}
	}
}



void die(Display *dis) // kills the WM
{
	XCloseDisplay(dis);
}

int errHandle(Display *dis, XErrorEvent *evnt) // error handler. i really dont know what this does exactly but if i remove it the wm becomes unstable so im keeping it here
{
	(void)dis;
	(void)evnt;
	return 0;
}

int init() // inits the server and its basic attributes
{
	if(!(dpy = XOpenDisplay(NULL)))
	{
		cout << "FAILED TO OPEN DISPLAY" << endl;
		// if X fails to open the display this message will be triggered
		return 1;
	}

	scr = XDefaultScreen(dpy);
	root = XDefaultRootWindow(dpy);
	XDefineCursor(dpy, root, XCreateFontCursor(dpy, 1));
	XSetErrorHandler(errHandle);
	XSelectInput(dpy, root, ExposureMask|ButtonPressMask|ButtonReleaseMask|KeyPressMask|KeyReleaseMask|PointerMotionMask|SubstructureNotifyMask); // telling X to accept these types of inputs as valid

	// telling X to monitor these keys/buttons for input events

	XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("a")), funcKeys.modKey, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("b")), funcKeys.modKey, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("c")), funcKeys.modKey, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("d")), funcKeys.modKey, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("e")), funcKeys.modKey, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("f")), funcKeys.modKey, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("g")), funcKeys.modKey, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("h")), funcKeys.modKey, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("i")), funcKeys.modKey, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("j")), funcKeys.modKey, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("k")), funcKeys.modKey, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("l")), funcKeys.modKey, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("m")), funcKeys.modKey, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("n")), funcKeys.modKey, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("o")), funcKeys.modKey, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("p")), funcKeys.modKey, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("q")), funcKeys.modKey, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("r")), funcKeys.modKey, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("s")), funcKeys.modKey, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("t")), funcKeys.modKey, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("u")), funcKeys.modKey, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("v")), funcKeys.modKey, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("w")), funcKeys.modKey, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("x")), funcKeys.modKey, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("y")), funcKeys.modKey, root, True, GrabModeAsync, GrabModeAsync);
	XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("z")), funcKeys.modKey, root, True, GrabModeAsync, GrabModeAsync);
	XGrabButton(dpy, 1, funcKeys.modKey, root, True, ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(dpy, 2, funcKeys.modKey, root, True, ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(dpy, 3, funcKeys.modKey, root, True, ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(dpy, 4, funcKeys.modKey, root, True, ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(dpy, 5, funcKeys.modKey, root, True, ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(dpy, 9, None, root, True, ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);

	return 0;
}


void drawBorders(long unsigned int i, int thknss) // draws borders. takes in the client index and a thickness for the border;
{
	XSetWindowBorder(dpy, clients[i].win, clients[i].borderColor);
	XSetWindowBorderWidth(dpy, clients[i].win, thknss);
}

void winToClient(Window win) // the client creator. packages the window being passed into a client
{
	XWindowAttributes winAt;
	Client tmpCli; // creates a temp client to add the attributes into

	XGetWindowAttributes(dpy, win, &winAt);
	tmpCli.dpy = dpy;
	tmpCli.parent = &root;
	tmpCli.win = win;
	tmpCli.coords[0] = winAt.x;
	tmpCli.coords[1] = winAt.y;
	tmpCli.dimensions[0] = winAt.width;
	tmpCli.dimensions[1] = winAt.height;
	tmpCli.borderColor = inactiveHex;
	tmpCli.indPrev = -1;
	tmpCli.indNext = -1;
	tmpCli.isMini = false;


	clients.push_back(tmpCli); // pushing the package into the main list

	drawBorders(clients.size() - 1, inactBorderThcknss);
}

long unsigned int checkClient(Window win) // checkes what client is being passed and returns its index in the master list
{
	long unsigned int i;

	for(i = 0; i < clients.size() - 1; i++)
	{
		if(clients[i].win == win)
		{
			return i;
		}
	}

	winToClient(win);
	cout << "created new client #" << clients.size() - 1 << endl;
	return clients.size() - 1;
}

void focusWin(Window win) // changes the focused window
{
	long unsigned int i = checkClient(win);

	if(lastFocusedClient != i) // resetting the borders of the last focused window
	{
		clients[i].indPrev = lastFocusedClient;	// setting the current client as the last focused for the next execution of this function
		clients[clients[i].indPrev].borderColor = inactiveHex;
		clients[clients[i].indPrev].indNext = i;

		drawBorders(clients[i].indPrev, inactBorderThcknss);
	}

	//setting the borders of the current window
	clients[i].borderColor = activeHex;
	XSetInputFocus(dpy, clients[i].win, RevertToParent, CurrentTime);
	XRaiseWindow(dpy, clients[i].win);
	lastFocusedClient = i;

	drawBorders(i, actBorderThcknss);

}

void moveClientToPointer(Window win, int pointerX, int pointerY)
{
	long unsigned int i = checkClient(win);

	XMoveWindow(dpy, clients[i].win, pointerX, pointerY);
}

void killWin(Window win) //kills the client
{
	long unsigned int i = checkClient(win);

	XEvent evnt;

	// This was weird to figure out at first but basically it sends the current window a message to politely shut off instead of being forceful and using something like XDestroyWindow()

	evnt.type = ClientMessage;
	evnt.xclient.window = clients[i].win;
	evnt.xclient.message_type = XInternAtom(dpy, "WM_PROTOCOLS", True);
	evnt.xclient.format = 32;
	evnt.xclient.data.l[0] = XInternAtom(dpy, "WM_DELETE_WINDOW", True);
	evnt.xclient.data.l[1] = CurrentTime;


	if(clients[i].indPrev != -1)
	{
		focusWin(clients[clients[i].indPrev].win);
	}
	if(clients[i].isMini)
	{
		if(numMini != 0)
		{
			numMini--;
		}
	}


	XSendEvent(dpy, clients[i].win, False, NoEventMask, &evnt);

	clients.erase(clients.begin() + i); // remove the client from the list

}
void restoreWin(Window win)
{
	long unsigned int i = checkClient(win); // you should get the jist of what im doing with these by now. if not scroll back up in the code and read the explanation on previous time i use these next two functions

	//focuses the window and restores it to the last size saved in the clients attributes

	focusWin(clients[i].win);
	XResizeWindow(dpy, win, clients[i].dimensions[0], clients[i].dimensions[1]);
	XMoveWindow(dpy, win, clients[i].coords[0], clients[i].coords[1]);
	numMini--;
	if(numMini != 0)
	{
		numMini--;
	}
	clients[i].isMini = false;
}

void maximizeWin(Window win)
{
	long unsigned int i = checkClient(win); // look at the function above

	focusWin(clients[i].win);

	//saving the unmodified dimensions of the client

	clients[i].dimensions[0] = attr.width;
	clients[i].dimensions[1] = attr.height;
	clients[i].coords[0] = attr.x;
	clients[i].coords[1] = attr.y;

	XResizeWindow(dpy, win, 1920, 1080); // this is a lazy solution that i plan on making better in the future
	XMoveWindow(dpy, win, 0, 0);

	if(clients[i].isMini)
	{
		clients[i].isMini = false;
		if(numMini != 0)
		{
			numMini--;
		}
	}

}

void  minimizeWin(Window win)
{

	long unsigned int i = checkClient(win); // again look above


	//saving the attributes of the current client

	clients[i].dimensions[0] = attr.width;
	clients[i].dimensions[1] = attr.height;
	clients[i].coords[0] = attr.x;
	clients[i].coords[1] = attr.y;

	//lowering the window and iconifying it. in the future this will be an actual icon but for now its just a shitty 15x15 box

	XLowerWindow(dpy, clients[i].win);
	XResizeWindow(dpy, win, 15, 15);
	XMoveWindow(dpy, win, 20 * (numMini), (XDisplayHeight(dpy, scr) - 20)); // increments the icon along the bottom based on its position in numClients
	numMini++;
	clients[i].isMini = true;
}

//And heres the main function
int main(int argc, char **argv)
{
	//Create the default config path. This is broken for some reason but I'll try and fix it soon
	configPath = getenv("HOME");
	configPath = configPath + "/.config/melon/melon.conf";

	//Process the command line flags
	processFlags(argc, argv);

	//Check if the config path is blank in case the user ran the program with no config
	if(configPath != "")
		readConfig();

	//Start the display server
	init();

	//Make sure that start.subwindow is first set to None just in case
	start.xbutton.subwindow = None;
	start.xkey.subwindow = None;

	//Commented out for now. Working on getting this fixed
	//system("seed &");

	//The main loop that everything takes place in
	while(True)
	{
		//Grabs next event
		XNextEvent(dpy, &ev);

		//Checks if a window has just been mapped, scans the available clients to check if it
		if(ev.type == CreateNotify)
		{
			//Create a new client for the window created
			winToClient(ev.xcreatewindow.window);
			//This breaking shit so I removed it for now
			//moveClientToPointer(ev.xcreatewindow.window, ev.xbutton.x_root, ev.xbutton.y_root);
		}
		//Checking whether a button was pressed and also if theres a window that exists where the button was pressed
		else if((ev.type == ButtonPress && ev.xbutton.subwindow != None) || (ev.type == KeyPress && ev.xkey.subwindow != None))
		{
			//Save a copy of the current event. This will be used later for things like moving and resizing windows
			start = ev;

			//Assigns attributes based whether a key or mouse button was pressed
			if(start.type == ButtonPress)
			{
				XGetWindowAttributes(dpy, start.xbutton.subwindow, &attr);
				XSetInputFocus(dpy, start.xbutton.subwindow, RevertToParent, CurrentTime);
			}
			else if(start.type == KeyPress)
			{
				XGetWindowAttributes(dpy, start.xkey.subwindow, &attr);
				XSetInputFocus(dpy, start.xkey.subwindow, RevertToParent, CurrentTime);
			}

			if(start.type == ButtonPress ? start.xbutton.button == atoi(funcKeys.killWinKey.c_str()) : start.xkey.keycode == XKeysymToKeycode(dpy, XStringToKeysym(funcKeys.killWinKey.c_str())))
			{
				//Kills the selected window
				killWin(start.type == ButtonPress ? start.xbutton.subwindow : start.xkey.subwindow);
			}
			else if(start.type == ButtonPress ? start.xbutton.button == atoi(funcKeys.focusWinKey.c_str()) : start.xkey.keycode == XKeysymToKeycode(dpy, XStringToKeysym(funcKeys.focusWinKey.c_str())))
			{
				//Raises and focuses the window
				focusWin(start.type == ButtonPress ? start.xbutton.subwindow : start.xkey.subwindow);
			}
			else if(start.type == ButtonPress ? start.xbutton.button == atoi(funcKeys.restoreWinKey.c_str()) : start.xkey.keycode == XKeysymToKeycode(dpy, XStringToKeysym(funcKeys.restoreWinKey.c_str())))
			{
				//Checks if the window is already minimized
				if(attr.height == 15 && attr.width == 15)
				{
					//Restores the window to its previous size
					restoreWin(start.type == ButtonPress ? start.xbutton.subwindow : start.xkey.subwindow);
				}
			}
			else if(start.type == ButtonPress ? start.xbutton.button == atoi(funcKeys.miniWinKey.c_str()) : start.xkey.keycode == XKeysymToKeycode(dpy, XStringToKeysym(funcKeys.miniWinKey.c_str())))
			{
				//Checks if the window is not minimized
				if(attr.height != 15 && attr.width != 15)
				{
					//Minimizes the window
					minimizeWin(start.type == ButtonPress ? start.xbutton.subwindow : start.xkey.subwindow);
				}
			}
			else if(start.type == ButtonPress ? start.xbutton.button == atoi(funcKeys.killWMKey.c_str()) : start.xkey.keycode == XKeysymToKeycode(dpy, XStringToKeysym(funcKeys.killWMKey.c_str())))
			{
				//Kills the WM
				die(dpy);
			}
		}
		else if(ev.type == MotionNotify && start.xbutton.subwindow != None && start.xbutton.state == funcKeys.modKey)
		{
			int xDiff = ev.xbutton.x_root - start.xbutton.x_root;
			int yDiff = ev.xbutton.y_root - start.xbutton.y_root;

			//Long ass complicated function to move or resize windows
			XMoveResizeWindow(dpy, start.xbutton.subwindow, attr.x + (start.xbutton.button == 1 ? xDiff : 0), attr.y + (start.xbutton.button == 1 ? yDiff : 0), MAX(1, attr.width + (start.xbutton.button == 3 ? xDiff : 0)), MAX(1, attr.height + (start.xbutton.button == 3 ? yDiff : 0)));
			focusWin(ev.xbutton.subwindow);
		}
		//If the button or key is released
		else if(ev.type == ButtonRelease || ev.type == KeyRelease)
		{
			//Set the subwindow to None
			(ev.type == ButtonRelease ? start.xbutton.subwindow = None : start.xkey.subwindow = None);
		}
	}

	//Nicely shutting down the display server instead of doing it forceably
	XCloseDisplay(dpy);
}
