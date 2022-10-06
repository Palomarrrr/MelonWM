#include "common.hpp" // having a common header file makes things easier on my half

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::fstream;

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

typedef struct FunctionKeys
{
	unsigned int modKey = Mod4Mask;
	string killWMKey = "x";
	string focusWinKey = "f";
	string miniWinKey = "q";
	string restoreWinKey = "w";
	string killWinKey = "2";

} FunctionKeys;

//initing variables

Display *dpy;
XWindowAttributes attr;
XEvent ev, start;
static Window root;
int scr, inactBorderThcknss = 1, actBorderThcknss = 1;
unsigned long int inactiveHex = 0x000000, activeHex = 0xf0fff0, lastFocusedClient, numMini;
string configPath;
vector<Client> clients;
FunctionKeys funcKeys;
fstream config;

void readConfig()
{
	config.open(configPath, std::ios::in);

	string line, data;

	config >> line;
	data = line.substr(line.find_last_of("=") + 1);
	inactiveHex = strtoul(data.c_str(), nullptr, 16);

	config >> line;
	data = line.substr(line.find_last_of("=") + 1);
	inactBorderThcknss = atoi(data.c_str());

	config >> line;
	data = line.substr(line.find_last_of("=") + 1);
	activeHex = strtoul(data.c_str(), nullptr, 16);

	config >> line;
	data = line.substr(line.find_last_of("=") + 1);
	actBorderThcknss = atoi(data.c_str());

	config >> line;
	data = line.substr(line.find_last_of("=") + 1);
	funcKeys.modKey = atoi(data.c_str());

	if( funcKeys.modKey == 1)
	{
		funcKeys.modKey = Mod4Mask;
	}
	else if(funcKeys.modKey == 2)
	{
		funcKeys.modKey = Mod2Mask;
	}
	else if(funcKeys.modKey == 3)
	{
		funcKeys.modKey = Mod3Mask;
	}
	else if(funcKeys.modKey == 4)
	{
		funcKeys.modKey = Mod4Mask;
	}
	else if(funcKeys.modKey == 5)
	{
		funcKeys.modKey = Mod5Mask;
	}
	else
	{
		funcKeys.modKey = Mod4Mask;
	}

	config >> line;
	data = line.substr(line.find_last_of("=") + 1);
	funcKeys.killWMKey = data;

	config >> line;
	data = line.substr(line.find_last_of("=") + 1);
	funcKeys.focusWinKey = data;

	config >> line;
	data = line.substr(line.find_last_of("=") + 1);
	funcKeys.miniWinKey = data;

	config >> line;
	data = line.substr(line.find_last_of("=") + 1);
	funcKeys.restoreWinKey = data;

	config >> line;
	data = line.substr(line.find_last_of("=") + 1);
	funcKeys.killWinKey = data;

	config.close();
}

int findFlagRequest(char* argv)
{
	int requestType;

	if(strcmp("-h", argv) == 0 || strcmp("--help", argv) == 0)
	{
		requestType = 0;
	}
	else if(strcmp("-c", argv) == 0 || strcmp("--config", argv) == 0)
	{
		requestType = 1;
	}
	else if(strcmp("-n", argv) == 0 || strcmp("--no-config", argv) == 0)
	{
		requestType = 2;
	}
	else
	{
		requestType = -1;
	}

	return requestType;
}

int processFlags(int argc, char** argv)
{
	if(argc == 0)
		return 1;

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
				return 0;
			case 2:
				configPath = "";
				return 0;
			default:
				cout << "Invalid flag \"" << argv[i] << "\"" << endl;
				cout << "Check -h for more information" << endl;
				exit(1);
		}
	}
	return 0;
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

int main(int argc, char **argv) //aaaaaand here we go the big boy function
{
	configPath = getenv("HOME");
	configPath = configPath + "/.config/cppwm/config";
	int request = processFlags(argc, argv);

	if(configPath != "")
		readConfig();

	init(); //init the server

	start.xbutton.subwindow = None; //make sure that start.subwindow (the window that will be selected) is first set to null
	start.xkey.subwindow = None; //make sure that start.subwindow (the window that will be selected) is first set to null

	//commented out for now. working on getting this fixed
	//system("seed &");

	while(True) // the main big boy loop to handle events and such
	{
		XNextEvent(dpy, &ev); // grabs next event

		if(ev.type == CreateNotify) // checks if a window has just been mapped, scans the available clients to check if it
		{
			winToClient(ev.xcreatewindow.window);
		//	moveClientToPointer(ev.xcreatewindow.window, ev.xbutton.x_root, ev.xbutton.y_root); // this breaking shit so i removed it for now
		}
		else if((ev.type == ButtonPress && ev.xbutton.subwindow != None) || (ev.type == KeyPress && ev.xkey.subwindow != None)) // checking whether a button was pressed and also if theres a window that exists where the button was pressed
		{
			start = ev;
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

			XGetWindowAttributes(dpy, start.xbutton.subwindow, &attr);
			XSetInputFocus(dpy, start.xbutton.subwindow, RevertToParent, CurrentTime);


			if(start.type == ButtonPress ? start.xbutton.button == atoi(funcKeys.killWinKey.c_str()) : start.xkey.keycode == XKeysymToKeycode(dpy, XStringToKeysym(funcKeys.killWinKey.c_str()))) // checks if the second (middle mouse) button was pressed with the Mod4 key
			{
				//kills the window
				killWin(start.type == ButtonPress ? start.xbutton.subwindow : start.xkey.subwindow);
			}
			else if(start.type == ButtonPress ? start.xbutton.button == atoi(funcKeys.focusWinKey.c_str()) : start.xkey.keycode == XKeysymToKeycode(dpy, XStringToKeysym(funcKeys.focusWinKey.c_str())))// checks whether button 9 (the front side button on my mouse) is pressed. this check does not need the Mod4 key to be pressed since i specified "None" in the init() function
			{
				// raises and focuses the window
				focusWin(start.type == ButtonPress ? start.xbutton.subwindow : start.xkey.subwindow);
			}
			else if(start.type == ButtonPress ? start.xbutton.button == atoi(funcKeys.restoreWinKey.c_str()) : start.xkey.keycode == XKeysymToKeycode(dpy, XStringToKeysym(funcKeys.restoreWinKey.c_str()))) // look above ( or at the one after that for an explanation on what this is doing
			{
				//checks if the window is already minimized
				if(attr.height == 15 && attr.width == 15)
				{
					restoreWin(start.type == ButtonPress ? start.xbutton.subwindow : start.xkey.subwindow); // restores the window to its previous size if true
				}
			}
			else if(start.type == ButtonPress ? start.xbutton.button == atoi(funcKeys.miniWinKey.c_str()) : start.xkey.keycode == XKeysymToKeycode(dpy, XStringToKeysym(funcKeys.miniWinKey.c_str())))
			{
				//checks if the window is not minimized
				if(attr.height != 15 && attr.width != 15)
				{
					minimizeWin(start.type == ButtonPress ? start.xbutton.subwindow : start.xkey.subwindow);
				}
			}
			else if(start.type == ButtonPress ? start.xbutton.button == atoi(funcKeys.killWMKey.c_str()) : start.xkey.keycode == XKeysymToKeycode(dpy, XStringToKeysym(funcKeys.killWMKey.c_str())))// checks whether button 9 (the front side button on my mouse) is pressed. this check does not need the Mod4 key to be pressed since i specified "None" in the init() function
			{
				die(dpy);
			}
		}
		else if(ev.type == MotionNotify && start.xbutton.subwindow != None && start.xbutton.state == funcKeys.modKey) // now this is where things get a little tricky. this function checks whether the mouse pointer was moved
		{
			//setting up vars for the x and y difference which will be used for resizing or moving the window
			int xDiff = ev.xbutton.x_root - start.xbutton.x_root;
			int yDiff = ev.xbutton.y_root - start.xbutton.y_root;

			//long ass complicated ass function to move or resize windows to where ever the user wants
			XMoveResizeWindow(dpy, start.xbutton.subwindow, attr.x + (start.xbutton.button == 1 ? xDiff : 0), attr.y + (start.xbutton.button == 1 ? yDiff : 0), MAX(1, attr.width + (start.xbutton.button == 3 ? xDiff : 0)), MAX(1, attr.height + (start.xbutton.button == 3 ? yDiff : 0)));
			focusWin(ev.xbutton.subwindow);
		}
		else if(ev.type == ButtonRelease || ev.type == KeyRelease)
		{
			(ev.type == ButtonRelease ? start.xbutton.subwindow = None : start.xkey.subwindow = None);
		}
	}

	//nicely shutting down the server when the user wishes to shut down the WM
	XCloseDisplay(dpy);
}
