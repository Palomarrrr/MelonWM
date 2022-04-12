#include "common.h"

typedef struct Client
{
	Window win;
	unsigned int coords[2];
	unsigned int dimentions[2];
	bool isFocused;
	std::vector<Client> siblings;
} Client;

Display *dpy;
XWindowAttributes attr;
XButtonEvent start;
XEvent ev;
static Window root;
int scr, numClients = 0, ticker = 0;
std::vector<Client> clients;

void winToClient(Window win)
{
	std::cout << "Entering client creation..." << std::endl;

	XWindowAttributes winAt;
	Client tmpCli;

	XGetWindowAttributes(dpy, win, &winAt);
	tmpCli.win = win;
	tmpCli.coords[0] = winAt.x;
	tmpCli.coords[1] = winAt.y;
	tmpCli.dimentions[0] = winAt.width;
	tmpCli.dimentions[1] = winAt.height;
	tmpCli.siblings.push_back(tmpCli);
	
	clients.push_back(tmpCli);
	numClients++;
}

void die(Display *dis)
{
	if(ticker > 2)
	{
		std::cout << "Recieved Die CMD... Killing display server" << std::endl;
		XCloseDisplay(dis);
	}
	
	ticker++;
}

int errHandle(Display *dis, XErrorEvent *evnt)
{
	(void)dis;
	(void)evnt;
	return 0;
}

int init()
{
	if(!(dpy = XOpenDisplay(0x0)))
	{
		DEBUG("XOpenDisplay");
		return 1;
	}	
	
	scr = XDefaultScreen(dpy);
	root = XDefaultRootWindow(dpy);
	XDefineCursor(dpy, root, XCreateFontCursor(dpy, 8));
	XSetErrorHandler(errHandle);

	XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("x")), Mod4Mask, root, True, GrabModeAsync, GrabModeAsync);
	XGrabButton(dpy, 1, Mod4Mask, root, True, ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(dpy, 2, Mod4Mask, root, True, ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(dpy, 3, Mod4Mask, root, True, ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(dpy, 4, Mod4Mask, root, True, ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(dpy, 5, Mod4Mask, root, True, ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(dpy, 8, Mod4Mask, root, True, ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(dpy, 9, Mod4Mask, root, True, ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
	XGrabButton(dpy, 9, None, root, True, ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);	

	return 0;
}

void drawBorders(Window win) 
{
	XSetWindowBorder(dpy, win, COLOR(0, 43, 54));
	XSetWindowBorderWidth(dpy, win, 1);	
}

int checkClient(Window win)
{
	int i;

	for( i = 0; i < numClients; i++)
	{
		if(clients[i].win == win)
		{
			return i;	
		}
	}
	
	return -1;
	std::cout << "Could not find client..." << std::endl;
}

void killWin(Window win)
{
	int i = checkClient(win);
	clients.erase(clients.begin() + i);
	std::cout << "Removing client " << i << " from the list" << std::endl;
	numClients--;	

	XEvent evnt;
	
	evnt.type = ClientMessage;
	evnt.xclient.window = win;
	evnt.xclient.message_type = XInternAtom(dpy, "WM_PROTOCOLS", True);
	evnt.xclient.format = 32;
	evnt.xclient.data.l[0] = XInternAtom(dpy, "WM_DELETE_WINDOW", True);	
	evnt.xclient.data.l[1] = CurrentTime;	
	
	XSendEvent(dpy, win, False, NoEventMask, &evnt);
}
void restoreWin(Window win)
{
	unsigned int i = checkClient(win);
	
	
	XRaiseWindow(dpy, win);
	XResizeWindow(dpy, win, clients[i].dimentions[0], clients[i].dimentions[1]);
	XMoveWindow(dpy, win, clients[i].coords[0], clients[i].coords[1]);
}

void maximizeWin(Window win)
{
	unsigned int i = checkClient(win);	

	XRaiseWindow(dpy, win);
	clients[i].dimentions[0] = attr.width;
	clients[i].dimentions[1] = attr.height;
	clients[i].coords[0] = attr.x;
	clients[i].coords[1] = attr.y;
	XResizeWindow(dpy, win, 1920, 1080);
	XMoveWindow(dpy, win, 0, 0);

}

void  minimizeWin(Window win)
{
	
	unsigned int i = checkClient(win);

	XLowerWindow(dpy, win);
	clients[i].dimentions[0] = attr.width;
	clients[i].dimentions[1] = attr.height;
	clients[i].coords[0] = attr.x;
	clients[i].coords[1] = attr.y;
	XResizeWindow(dpy, win, 15, 15);
	XMoveWindow(dpy, win, 20 * (i + 1), (XDisplayHeight(dpy, scr) - 20)); 
}

int main(int argc, char **argv)
{
	init();

	start.subwindow = None;

	while(True)
	{
		XNextEvent(dpy, &ev);
			
		if(ev.type == KeyPress && ev.xkey.subwindow != None)
		{
			XRaiseWindow(dpy, ev.xkey.subwindow);
			XSetInputFocus(dpy, ev.xkey.subwindow, RevertToParent, CurrentTime);	
			
			if(ev.xkey.keycode == XKeysymToKeycode(dpy, XStringToKeysym("x")))
			{
				die(dpy);
			}
		}
		else if(ev.type == ButtonPress && ev.xbutton.subwindow != None)
		{
			start = ev.xbutton;

			XGetWindowAttributes(dpy, start.subwindow, &attr);
			XSetInputFocus(dpy, start.subwindow, RevertToParent, CurrentTime);
			XRaiseWindow(dpy, start.subwindow);
			drawBorders(start.subwindow);

			if(checkClient(start.subwindow) == -1)
			{
				winToClient(start.subwindow);
				std::cout << "Creating new client entry" << std::endl;
			}

			if(start.button == 2)
			{
				killWin(start.subwindow);	
			}
			else if(start.button == 9)
			{
				XRaiseWindow(dpy, start.subwindow);
				XSetInputFocus(dpy, start.subwindow, RevertToParent, CurrentTime);
			}
			else if(start.button == 4)
			{
				if(attr.height == 15 && attr.width == 15)
				{
					restoreWin(start.subwindow);
				}
			}
			else if(start.button == 5)
			{
				if(attr.height != 15 && attr.width != 15)
				{
					minimizeWin(start.subwindow);	
				}
			}
			else if(start.button == 8)
			{
				if(attr.width == (XDisplayWidth(dpy, scr)) && attr.height == (XDisplayHeight(dpy, scr)))
				{
					restoreWin(start.subwindow);
				}
				else
				{
					XMoveWindow(dpy, start.subwindow, 0, 0);
					maximizeWin(start.subwindow);
				}
			}
		}
		else if(ev.type == MotionNotify && start.subwindow != None)
		{
			int xDiff = ev.xbutton.x_root - start.x_root;
			int yDiff = ev.xbutton.y_root - start.y_root;
			
			XMoveResizeWindow(dpy, start.subwindow, attr.x + (start.button == 1 ? xDiff : 0), attr.y + (start.button == 1 ? yDiff : 0), MAX(1, attr.width + (start.button == 3 ? xDiff : 0)), MAX(1, attr.height + (start.button == 3 ? yDiff : 0)));
		}
		else if(ev.type == ButtonRelease)
		{
			start.subwindow = None;
		}
	}
	
	std::cout << "Attempting to close display..." << std::endl;
	XCloseDisplay(dpy);
}
