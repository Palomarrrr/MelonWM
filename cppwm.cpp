#include "common.h" // having a common header file makes things easier on my half

typedef struct Client
{
	Display *dpy; // pointer to the display the window is active on
	Window *parent, win; //the windows parent and the window being packaged
	unsigned int coords[2]; //0 = x, 1 = y
	unsigned int dimentions[2]; //0 = wid, 1 = hgt
	unsigned int borderColor[3]; //0 = r, 1 = g, 2 = b
	int indPrev, indNext; //the previous and next client in the list 
} Client;

//initing variables

Display *dpy;
XWindowAttributes attr;
XButtonEvent start;
XEvent ev;
static Window root;
int scr, numClients = -1, ticker = 0, lastFocusedClient = -1;
std::vector<Client> clients;

void die(Display *dis) // kills the WM
{
	if(ticker > 4)
	{
		std::cout << "Recieved Die CMD... Killing display server" << std::endl;
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
		DEBUG("XOpenDisplay");
		// if X fails to open the display this message will be triggered
		return 1;
	}	
	
	scr = XDefaultScreen(dpy);
	root = XDefaultRootWindow(dpy);
	XDefineCursor(dpy, root, XCreateFontCursor(dpy, 8));
	XSetErrorHandler(errHandle);

	// telling X to monitor these keys/buttons for input events

	XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("x")), Mod4Mask, root, True, GrabModeAsync, GrabModeAsync);
	XGrabButton(dpy, 1, Mod4Mask, root, True, ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(dpy, 2, Mod4Mask, root, True, ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(dpy, 3, Mod4Mask, root, True, ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(dpy, 4, Mod4Mask, root, True, ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(dpy, 5, Mod4Mask, root, True, ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(dpy, 8, Mod4Mask, root, True, ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(dpy, 9, None, root, True, ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);	

	return 0;
}

int checkClient(Window win) // checkes what client is being passed and returns its index in the master list
{
	int i;

	for(i = 0; i < numClients; i++)
	{
		if(clients[i].win == win)
		{
			return i;	
		}
	}
	
	return -1;
	std::cout << "Could not find client..." << std::endl;
}

void drawBorders(int i, int thknss) // draws borders. takes in the client index and a thickness for the border;
{
	XSetWindowBorder(dpy, clients[i].win, COLOR(clients[i].borderColor[0], clients[i].borderColor[1], clients[i].borderColor[2]));
	XSetWindowBorderWidth(dpy, clients[i].win, thknss);	
}

void winToClient(Window win) // the client creator. packages the window being passed into a client
{
	std::cout << "Entering client creation..." << std::endl;

	XWindowAttributes winAt;
	Client tmpCli; // creates a temp client to add the attributes into

	XGetWindowAttributes(dpy, win, &winAt);
	tmpCli.dpy = dpy;
	tmpCli.parent = &root;
	tmpCli.win = win;
	tmpCli.coords[0] = winAt.x;
	tmpCli.coords[1] = winAt.y;
	tmpCli.dimentions[0] = winAt.width;
	tmpCli.dimentions[1] = winAt.height;
	tmpCli.borderColor[0] = 0; //r
	tmpCli.borderColor[1] = 43; //g
	tmpCli.borderColor[2] = 54; //b
	tmpCli.indPrev = -1;
	tmpCli.indNext = -1;


	std::cout << "Adding client to the master list" << std::endl;
	clients.push_back(tmpCli); // pushing the package into the main list
	numClients++; // incrementing the num of clients that exist. inb4 someone says sizeof()... it doesnt want to work for some reason
	std::cout << "Exiting client creator" << std::endl;
}

void focusWin(Window win) // changes the focused window
{
	int i = checkClient(win);
	
	if( i == -1 ) // checking if the client check returned that there is no client that exists and then creating a new client for it
	{
		std::cout << "Passed an invalid client to function \"focusWin\" on line " << __LINE__ << std::endl;
		std::cout << "Will attempt to pass this error to the \"winToClient\" function" << std::endl;
		winToClient(win);
	}
	
	if(lastFocusedClient != i && lastFocusedClient != -1) // resetting the borders of the last focused window
	{
		clients[i].indPrev = lastFocusedClient;	// setting the current client as the last focused for the next execution of this function
		std::cout << "resetting old borders and switching lastfocus" << std::endl;
		clients[clients[i].indPrev].borderColor[0] = 0;
		clients[clients[i].indPrev].borderColor[1] = 43;
		clients[clients[i].indPrev].borderColor[2] = 54;
		clients[clients[i].indPrev].indNext = i;


		drawBorders(clients[i].indPrev, 1);
	}	

	//setting the borders of the current window
	clients[i].borderColor[0] = 133;
	clients[i].borderColor[1] = 153;
	clients[i].borderColor[2] = 0;
	lastFocusedClient = i;
	XSetInputFocus(dpy, clients[i].win, RevertToParent, CurrentTime);
	XRaiseWindow(dpy, clients[i].win);

	drawBorders(i, 1);	

}


void killWin(Window win) //kills the client
{
	int i = checkClient(win);

	if( i == -1 )// checking if the client check returned that there is no client that exists and then creating a new client for it
	{
		std::cout << "Passed an invalid client to function \"killWin\" on line " << __LINE__ << std::endl;
		std::cout << "Will attempt to pass this error to the \"winToClient\" function" << std::endl;
		winToClient(win);
	}
	
	std::cout << "Removing client " << i << " from the list" << std::endl;

	XEvent evnt;
	
	// This was weird to figure out at first but basically it sends the current window a message to politely shut off instead of being forceful and using something like XDestroyWindow()

	std::cout<< "Setting up Client Message..." << std::endl;
	evnt.type = ClientMessage;
	evnt.xclient.window = clients[i].win;
	evnt.xclient.message_type = XInternAtom(dpy, "WM_PROTOCOLS", True);
	evnt.xclient.format = 32;
	evnt.xclient.data.l[0] = XInternAtom(dpy, "WM_DELETE_WINDOW", True);	
	evnt.xclient.data.l[1] = CurrentTime;	
	
	std::cout << "Event Sent!" << std::endl;

	if(clients[i].indPrev != -1)
	{
		focusWin(clients[clients[i].indPrev].win);
	}

	XSendEvent(dpy, clients[i].win, False, NoEventMask, &evnt);

	clients.erase(clients.begin() + i); // remove the client from the list
	
	numClients--; // setting the number of clients to reflect the actual amount in the list	
}
void restoreWin(Window win)
{
	int i = checkClient(win); // you should get the jist of what im doing with these by now. if not scroll back up in the code and read the explanation on previous time i use these next two functions
	
	if( i == -1 )
	{
		std::cout << "Passed an invalid client to function \"restoreWin\" on line " << __LINE__ << std::endl;
		std::cout << "Will attempt to pass this error to the \"winToClient\" function" << std::endl;
		winToClient(win);
	}

	//focuses the window and restores it to the last size saved in the clients attributes

	focusWin(clients[i].win);
	XResizeWindow(dpy, win, clients[i].dimentions[0], clients[i].dimentions[1]);
	XMoveWindow(dpy, win, clients[i].coords[0], clients[i].coords[1]);
}

void maximizeWin(Window win)
{
	int i = checkClient(win); // look at the function above 	

	if( i == -1 )
	{
		std::cout << "Passed an invalid client to function \"maximizeWin\" on line " << __LINE__ << std::endl;
		std::cout << "Will attempt to pass this error to the \"winToClient\" function" << std::endl;
		winToClient(win);
	}
	
	focusWin(clients[i].win);
	
	//saving the unmodified dimentions of the client

	clients[i].dimentions[0] = attr.width;
	clients[i].dimentions[1] = attr.height;
	clients[i].coords[0] = attr.x;
	clients[i].coords[1] = attr.y;

	XResizeWindow(dpy, win, 1920, 1080); // this is a lazy solution that i plan on making better in the future
	XMoveWindow(dpy, win, 0, 0);

}

void  minimizeWin(Window win)
{
	
	int i = checkClient(win); // again look above
	
	if( i == -1 )
	{
		std::cout << "Passed an invalid client to function \"minimizeWin\" on line " << __LINE__ << std::endl;
		std::cout << "Will attempt to pass this error to the \"winToClient\" function" << std::endl;
		winToClient(win);
	}

	//saving the attributes of the current client

	clients[i].dimentions[0] = attr.width;
	clients[i].dimentions[1] = attr.height;
	clients[i].coords[0] = attr.x;
	clients[i].coords[1] = attr.y;

	//lowering the window and iconifying it. in the future this will be an actual icon but for now its just a shitty 15x15 box

	XLowerWindow(dpy, clients[i].win);
	XResizeWindow(dpy, win, 15, 15);
	XMoveWindow(dpy, win, 20 * (i + 1), (XDisplayHeight(dpy, scr) - 20)); // increments the icon along the bottom based on its position in numClients
}

int main(int argc, char **argv) //aaaaaand here we go the big boy function
{
	init(); //init the server 

	start.subwindow = None; //make sure that start.subwindow (the window that will be selected) is first set to null

	while(True) // the main big boy loop to handle events and such
	{
		XNextEvent(dpy, &ev); // grabs next event
			
		if(ev.type == KeyPress) // checks whether the event is a keypress event
		{
			if(ev.xkey.keycode == XKeysymToKeycode(dpy, XStringToKeysym("x"))) // checking whether the key pressed is the "x" key and also makes sure its being pressed with Mod4 
			{
				die(dpy);
			}
		}
		else if(ev.type == ButtonPress && ev.xbutton.subwindow != None) // checking whether a button was pressed and also if theres a window that exists where the button was pressed
		{
			start = ev.xbutton; // setting start so we dont have to do ev.xbutton.subwindow 

			//doing general things that will come in handy later in the loop

			XGetWindowAttributes(dpy, start.subwindow, &attr);
			XSetInputFocus(dpy, start.subwindow, RevertToParent, CurrentTime);

			if(checkClient(start.subwindow) == -1) // checking if the selected window is in the client list and making a new one if it isnt
			{
				winToClient(start.subwindow);
				std::cout << "Creating new client entry" << std::endl;
			}

			if(start.button == 2) // checks if the second (middle mouse) button was pressed with the Mod4 key
			{
				//kills the window
				killWin(start.subwindow);	
			}
			else if(start.button == 9) // checks whether button 9 (the front side button on my mouse) is pressed. this check does not need the Mod4 key to be pressed since i specified "None" in the init() function
			{
				// raises and focuses the window
				//XSetInputFocus(dpy, start.subwindow, RevertToParent, CurrentTime);
				focusWin(start.subwindow);
			}
			else if(start.button == 4) // look above ( or at the one after that for an explanation on what this is doing
			{
				//checks if the window is already minimized
				if(attr.height == 15 && attr.width == 15)
				{
					restoreWin(start.subwindow); // restores the window to its previous size if true
				}
			}
			else if(start.button == 5)
			{
				//checks if the window is not mimimized
				if(attr.height != 15 && attr.width != 15)
				{
					minimizeWin(start.subwindow);	
				}
			}
			else if(start.button == 8)
			{
				//checks if the window is maximized and restores to original size if true
				if(attr.width == 1920 && attr.height == 1080)
				{
					restoreWin(start.subwindow);
				}
				//else it maximizes the window
				else
				{
					maximizeWin(start.subwindow);
				}
			}
		}
		else if(ev.type == MotionNotify && start.subwindow != None) // now this is where things get a little tricky. this function checks whether the mouse pointer was moved
		{
			//setting up vars for the x and y difference which will be used for resizing or moving the window
			int xDiff = ev.xbutton.x_root - start.x_root;
			int yDiff = ev.xbutton.y_root - start.y_root;
			
			//long ass complicated ass function to move or resize windows to where ever the user wants
			XMoveResizeWindow(dpy, start.subwindow, attr.x + (start.button == 1 ? xDiff : 0), attr.y + (start.button == 1 ? yDiff : 0), MAX(1, attr.width + (start.button == 3 ? xDiff : 0)), MAX(1, attr.height + (start.button == 3 ? yDiff : 0)));
			focusWin(start.subwindow);
		}
		else if(ev.type == ButtonRelease) // when the button is released, it sets the start var to none in preparation for the next event
		{
			start.subwindow = None;
		}
	}
	
	//nicely shutting down the server when the user wishes to shut down the WM
	std::cout << "Attempting to close display..." << std::endl;
	XCloseDisplay(dpy);
}
