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

//initing variables

Display *dpy;
XWindowAttributes attr;
XEvent ev, start;
static Window root;
int scr, ticker = 0, inactBorderThcknss = 1, actBorderThcknss = 1;
unsigned long int inactiveHex = 0x000000, activeHex = 0x0000ff, lastFocusedClient, numMini, modKey, killWMKey, focusWinKey, miniWinKey, restoreWinKey, moveWinKey, killWinKey, resizeWinKey;
vector<Client> clients;
fstream config;

void readConfig()
{
	config.open("/home/koishi/.config/cppwm/config", std::ios::in);


	string line, data;

	config >> line;
	data = line.substr(line.find_last_of(".") + 1);
	inactiveHex = strtoul(data.c_str(), nullptr, 16);

	config >> line;
	data = line.substr(line.find_last_of(".") + 1);
	inactBorderThcknss = atoi(data.c_str());

	config >> line;
	data = line.substr(line.find_last_of(".") + 1);
	activeHex = strtoul(data.c_str(), nullptr, 16);

	config >> line;
	data = line.substr(line.find_last_of(".") + 1);
	actBorderThcknss = atoi(data.c_str());

	config >> line;
	data = line.substr(line.find_last_of(".") + 1);
	modKey = strtoul(data.c_str(), nullptr, 10);

	if(modKey == 1)
	{
		modKey = Mod4Mask;
	}
	else if(modKey == 2)
	{
		modKey = Mod2Mask;
	}
	else if(modKey == 3)
	{
		modKey = Mod3Mask;
	}
	else if(modKey == 4)
	{
		modKey = Mod4Mask;
	}
	else if(modKey == 5)
	{
		modKey = Mod5Mask;
	}
	else
	{
		modKey = Mod4Mask;
	}

	config >> line;
	data = line.substr(line.find_last_of(".") + 1);
	killWMKey = strtoul(data.c_str(), nullptr, 10);

	config >> line;
	data = line.substr(line.find_last_of(".") + 1);
	focusWinKey = strtoul(data.c_str(), nullptr, 10);

	config >> line;
	data = line.substr(line.find_last_of(".") + 1);
	miniWinKey = strtoul(data.c_str(), nullptr, 10);

	config >> line;
	data = line.substr(line.find_last_of(".") + 1);
	restoreWinKey = strtoul(data.c_str(), nullptr, 10);

	config >> line;
	data = line.substr(line.find_last_of(".") + 1);
	killWinKey = strtoul(data.c_str(), nullptr, 10);

	config >> line;
	data = line.substr(line.find_last_of(".") + 1);
	moveWinKey = strtoul(data.c_str(), nullptr, 10);

	config >> line;
	data = line.substr(line.find_last_of(".") + 1);
	resizeWinKey = strtoul(data.c_str(), nullptr, 10);

	config.close();
}


void die(Display *dis) // kills the WM
{
	if(ticker > 4)
	{
		XCloseDisplay(dis);
	}

	ticker++;
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

	XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("x")), modKey, root, True, GrabModeAsync, GrabModeAsync);
	XGrabButton(dpy, 1, modKey, root, True, ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(dpy, 2, modKey, root, True, ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(dpy, 3, modKey, root, True, ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(dpy, 4, modKey, root, True, ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(dpy, 5, modKey, root, True, ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
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
	readConfig();
	init(); //init the server

	start.xbutton.subwindow = None; //make sure that start.subwindow (the window that will be selected) is first set to null
	start.xkey.subwindow = None; //make sure that start.subwindow (the window that will be selected) is first set to null

	system("pbox &");

	while(True) // the main big boy loop to handle events and such
	{
		XNextEvent(dpy, &ev); // grabs next event

		if(ev.type == CreateNotify) // checks if a window has just been mapped, scans the available clients to check if it
		{
			winToClient(ev.xcreatewindow.window);
		//	moveClientToPointer(ev.xcreatewindow.window, ev.xbutton.x_root, ev.xbutton.y_root); // this breaking shit so i removed it for now
		}
		/*
		else if(ev.xkey.keycode == XKeysymToKeycode(dpy, XStringToKeysym("x"))) // checking whether the key pressed is the "x" key and also makes sure its being pressed with Mod4
		{
			die(dpy);
		}
		*/
		else if(ev.type == ButtonPress && ev.xbutton.subwindow != None) // checking whether a button was pressed and also if theres a window that exists where the button was pressed
		{
			start = ev;
			XGetWindowAttributes(dpy, start.xbutton.subwindow, &attr);
			XSetInputFocus(dpy, start.xbutton.subwindow, RevertToParent, CurrentTime);


			if(start.xbutton.button == 2 && start.xbutton.state == modKey) // checks if the second (middle mouse) button was pressed with the Mod4 key
			{
				//kills the window
				killWin(start.xbutton.subwindow);
			}
			else if(start.xbutton.button == focusWinKey)// checks whether button 9 (the front side button on my mouse) is pressed. this check does not need the Mod4 key to be pressed since i specified "None" in the init() function
			{
				// raises and focuses the window
				focusWin(start.xbutton.subwindow);
			}
			else if(start.xbutton.button == restoreWinKey && start.xbutton.state == modKey) // look above ( or at the one after that for an explanation on what this is doing
			{
				//checks if the window is already minimized
				if(attr.height == 15 && attr.width == 15)
				{
					restoreWin(start.xbutton.subwindow); // restores the window to its previous size if true
				}
			}
			else if(start.xbutton.button == miniWinKey && start.xbutton.state == modKey)
			{
				//checks if the window is not minimized
				if(attr.height != 15 && attr.width != 15)
				{
					minimizeWin(start.xbutton.subwindow);
				}
			}
		}
		else if(ev.type == MotionNotify && start.xbutton.subwindow != None && start.xbutton.state == modKey) // now this is where things get a little tricky. this function checks whether the mouse pointer was moved
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
