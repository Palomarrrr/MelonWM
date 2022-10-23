//This is a mess
//I don't know what I'm doing
//How do I get out of here
//Please help

#include "common.h"

//This is used in resizing and moving windows
#define MAX(a, b) ((a) > (b) ? (a) : (b))

//This holds all data that I'd need to manipulate windows
typedef struct Client
{
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
struct _FunctionKeys{
	unsigned int modKey;
	char *killWMKey;
	char *focusWinKey;
	char *miniWinKey;
	char *restoreWinKey;
	char *killWinKey;

} FunctionKeys_default = {Mod4Mask, "e", "9", "q", "w", "2"};

typedef struct _FunctionKeys FunctionKeys;

//Declaring variables. I know I should make these as local as possible.
//I'll be working on that sometime soon because it bothers me too.
Display *dpy;
XWindowAttributes attr;
XEvent ev, start;
static Window root;
int scr, inactBorderThcknss = 1, actBorderThcknss = 1;
unsigned long int inactiveHex = 0x000000, activeHex = 0xf0fff0, lastFocusedClient, numMini;
char *configPath = NULL, *logFilePath = NULL;
Client *clients;
size_t numClients = 0;
FunctionKeys funcKeys;
FILE *logFile = NULL;

//This reads the config file
//It's really inefficient but works I guess
void readConfig()
{
	FILE *config;
	char *currLine = malloc(sizeof(char) * 200);
	char *buffer = malloc(sizeof(char) * 200);
	char *ptr;
	int fieldToFill = 0;

	config = fopen(configPath, "r");

	if(config == NULL)
	{
		fprintf(logFile, "%ld: FAILED TO OPEN CONFIG FILE\n", clock());
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
				else if(currLine[x] == '='){
					if(strcmp(buffer, "BORDER_INACTIVE_COLOR") == 0){
						fieldToFill = 1;
					}else if(strcmp(buffer, "BORDER_INACTIVE_THICKNESS") == 0){
						fieldToFill = 2;
					}else if(strcmp(buffer, "BORDER_ACTIVE_COLOR") == 0){
						fieldToFill = 3;
					}else if(strcmp(buffer, "BORDER_ACTIVE_THICKNESS") == 0){
						fieldToFill = 4;
					}else if(strcmp(buffer, "MODIFIER_KEY") == 0){
						fieldToFill = 5;
					}else if(strcmp(buffer, "KILL_WM_KEY") == 0){
						fieldToFill = 6;
					}else if(strcmp(buffer, "FOCUS_WINDOW_BUTTON") == 0){
						fieldToFill = 7;
					}else if(strcmp(buffer, "MINI_WINDOW_BUTTON") == 0){
						fieldToFill = 8;
					}else if(strcmp(buffer, "RESTORE_WINDOW_BUTTON") == 0){
						fieldToFill = 9;
					}else if(strcmp(buffer, "KILL_WINDOW_BUTTON") == 0){
						fieldToFill = 10;
					}else{
						fprintf(logFile, "%ld: INVALID CONFIG FIELD FOUND: %s\n", clock(), buffer);
						exit(1);
					}
					memset(buffer, 0, sizeof(char) * 200);
					j = 0;
					continue;
				}
				else if(x == strlen(currLine) - 1){
					switch(fieldToFill){
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
							switch(funcKeys.modKey){
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
									funcKeys.modKey = None;
									break;
							}
							break;
						case 6:
							funcKeys.killWMKey = (char*)malloc(200);
							memcpy(funcKeys.killWMKey, buffer, sizeof(char) * 200);
							fieldToFill = 0;
							break;
						case 7:
							funcKeys.focusWinKey = (char*)malloc(200);
							memcpy(funcKeys.focusWinKey, buffer, sizeof(char) * 200);
							fieldToFill = 0;
							break;
						case 8:
							funcKeys.miniWinKey = (char*)malloc(200);
							memcpy(funcKeys.miniWinKey, buffer, sizeof(char) * 200);
							fieldToFill = 0;
							break;
						case 9:
							funcKeys.restoreWinKey = (char*)malloc(200);
							memcpy(funcKeys.restoreWinKey, buffer, sizeof(char) * 200);
							fieldToFill = 0;
							break;
						case 10:
							funcKeys.killWinKey = (char*)malloc(200);
							memcpy(funcKeys.killWinKey, buffer, sizeof(char) * 200);
							fieldToFill = 0;
							break;
						default:
							fprintf(logFile, "%ld: INVALID ASSIGNMENT: %s\n", clock(), buffer);
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


	if(fclose(config) != 0){
		fprintf(logFile, "%ld: FAILED TO CLOSE YOUR CONFIG FILE\n", clock());
	}

	fprintf(logFile, "%ld: FINISHED CONFIG PROCESSING\n", clock());
}

int findFlagRequest(char* argv){

	if(strcmp("-h", argv) == 0 || strcmp("--help", argv) == 0){
		return 0;
	}
	else if(strcmp("-c", argv) == 0){
		return 1;
	}
	else if(strcmp("-n", argv) == 0){
		return 2;
	}
	else if(strcmp("-l", argv) == 0){
		return 3;
	}else{
		return -1;
	}

}

void processFlags(int argc, char** argv){
	if(argc == 0)
		return;

	for(int i = 1; i < argc; i++){
		int request = findFlagRequest(argv[i]);

		switch(request){
			case 0:
				fprintf(logFile, "%ld: -h FLAG USED\n", clock());
				printf("MELON WINDOW MANAGER\n");
				printf("  -c /path/to/file  Uses a specific path for config | default: %s\n", configPath);
				printf("  -n                Runs without a config and use defaults");
				printf("  -l                Turns the debug log on | default location: %s \n", logFilePath);
				printf("  -h                Displays this message\n");
				exit(1);
			case 1:
				fprintf(logFile, "%ld: -c FLAG USED\n", clock());
				i++;
				configPath = argv[i];
				break;
			case 2:
				fprintf(logFile, "%ld: -n FLAG USED\n", clock());
				configPath = NULL;
				break;
			case 3:
				fprintf(logFile, "%ld: -l FLAG USED. LOG FILE SWITCHED ON\n", clock());
				logFile = freopen(logFilePath, "w", logFile);
				break;
			default:
				printf("Invalid flag: %s\n", argv[i]);
				printf("Check -h for more information\n");
				fprintf(logFile, "%ld: INVALID FLAG REQUEST: %s\n", clock(), argv[i]);
				exit(1);
		}
	}
}

int errHandle(Display *dis, XErrorEvent *evnt) // error handler. i really dont know what this does exactly but if i remove it the wm becomes unstable so im keeping it here
{
	(void)dis;
	(void)evnt;
	fprintf(logFile, "%ld: SOME KIND OF NONFATAL X ERROR HAPPENED\n", clock());
	return 0;
}

int init() // inits the server and its basic attributes
{
	if(!(dpy = XOpenDisplay(NULL)))
	{
		fprintf(logFile, "%ld: FAILED TO OPEN DISPLAY\n", clock());
		// if X fails to open the display this message will be triggered
		exit(1);
	}

	fprintf(logFile, "%ld: OPENING DISPLAY AND SETTING UP INPUTS\n", clock());

	//Screen and root window
	scr = XDefaultScreen(dpy);
	root = XDefaultRootWindow(dpy);

	//Defining a default cursor
	XDefineCursor(dpy, root, XCreateFontCursor(dpy, 1));

	//Setting up the error handler
	XSetErrorHandler(errHandle);

	//Telling X what inputs to accept
	XSelectInput(dpy, root, ExposureMask|ButtonPressMask|ButtonReleaseMask|KeyPressMask|KeyReleaseMask|PointerMotionMask|SubstructureNotifyMask); 
	
	//Setting up some atoms for the wm
	(void)XChangeProperty(dpy, root, XInternAtom(dpy, "_NET_WM_NAME", False), XInternAtom(dpy, "UTF8_STRING", False), 8, PropModeReplace, (unsigned char *)"MelonWM", 7);
	(void)XChangeProperty(dpy, root, XInternAtom(dpy, "_NET_SUPPORTING_WM_CHECK", False), XA_WINDOW, 32, PropModeAppend, (unsigned char *)&root, 1);

	// telling X to monitor keys/buttons with a mod key for input events
	XGrabKey(dpy, AnyKey, funcKeys.modKey, root, True, GrabModeAsync, GrabModeAsync);
	XGrabButton(dpy, AnyButton, funcKeys.modKey, root, True, ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);

	fprintf(logFile, "%ld: FINISHED INIT PROCESS\n", clock());
	return 0;
}

void drawBorders(long unsigned int i, int thknss) // draws borders. takes in the client index and a thickness for the border;
{
	XSetWindowBorder(dpy, clients[i].win, clients[i].borderColor);
	XSetWindowBorderWidth(dpy, clients[i].win, thknss);
}

void winToClient(Window win) // the client creator. packages the window being passed into a client
{

	//Get name of the window

	fprintf(logFile, "%ld: ATTEMPTING TO CREATE NEW CLIENT\n", clock());

	//Increment the amount of clients 
	numClients++;

	//Increase the size of the array
	clients = realloc(clients, sizeof(Client) * numClients);

	//If allocation fails
	if(clients == NULL){
		fprintf(logFile, "%ld: FAILED TO ALLOCATE ENOUGH MEMORY TO THE CLIENT ARRAY\nPANIC QUITTING\n", clock());
		free(clients);
		exit(1);
	}

	//Get the attributes for the window
	XWindowAttributes winAt;
	XGetWindowAttributes(dpy, win, &winAt);
	clients[numClients - 1].dpy = dpy;
	clients[numClients - 1].parent = &root;
	clients[numClients - 1].win = win;
	clients[numClients - 1].coords[0] = winAt.x;
	clients[numClients - 1].coords[1] = winAt.y;
	clients[numClients - 1].dimensions[0] = winAt.width;
	clients[numClients - 1].dimensions[1] = winAt.height;
	clients[numClients - 1].borderColor = inactiveHex;
	clients[numClients - 1].indPrev = -1;
	clients[numClients - 1].isMini = 0;

	XTextProperty winName;
	XGetWMName(dpy, win, &winName);
	fprintf(logFile, "%ld: CREATED NEW CLIENT #%ld NAMED: %s\n", clock(), numClients, winName.value);


	drawBorders(numClients - 1, inactBorderThcknss);
}

long unsigned int checkClient(Window win) // checkes what client is being passed and returns its index in the master list
{
	long unsigned int i;

	for(i = 0; i < numClients; i++)
	{
		if(clients[i].win == win)
		{
			return i;
		}
	}

	winToClient(win);
	return numClients - 1;
}

void focusWin(Window win) // changes the focused window
{
	long unsigned int i = checkClient(win);

	if(lastFocusedClient != i) // resetting the borders of the last focused window
	{
		clients[i].indPrev = lastFocusedClient;	// setting the current client as the last focused for the next execution of this function
		clients[clients[i].indPrev].borderColor = inactiveHex;

		drawBorders(clients[i].indPrev, inactBorderThcknss);
		fprintf(logFile, "%ld: FOCUS SWITCHED FROM CLIENT #%ld TO CLIENT #%ld\n", clock(), clients[i].indPrev + 1, i + 1);
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


	fprintf(logFile, "%ld: DELETING CLIENT #%ld FROM THE LIST\n", clock(), i + 1);

	if(numClients > 1){
		numClients--;
	}

	clients = realloc(clients, sizeof(Client) * numClients);

	if(clients == NULL){
		fprintf(logFile, "%ld: FAILED TO ALLOCATE ENOUGH MEMORY TO THE CLIENT ARRAY\nPANIC QUITTING\n", clock());
		free(clients);
		exit(1);
	}

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
	clients[i].isMini = 0;
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
		clients[i].isMini = 0;
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
	clients[i].isMini = 1;
}

//And heres the big boy
int main(int argc, char **argv){ 
	//Get the users home directory
	char *userHome = getenv("HOME");

	//Set the path for the default config file
	configPath = malloc(sizeof(userHome) * strlen(userHome));
	memcpy(configPath, userHome, strlen(userHome) * 8);
	strcat(configPath, "/.config/MelonWM/MelonWM.conf");

	//Set the path for the log file
	logFilePath = malloc(sizeof(userHome) * strlen(userHome));
	memcpy(logFilePath, userHome, strlen(userHome) * 8);
	strcat(logFilePath, "/.local/share/MelonWM.log");

	//Initialize the log file but as read only so we can block all writes to it and keep the program from segfaulting
	logFile = fopen(logFilePath, "r");

	//Process the command line flags
	processFlags(argc, argv);
	fprintf(logFile, "%ld: FINISHED PROCESSING FLAGS\n", clock());


	//Check if the config path is blank in case the user ran the program with no config
	if(configPath != NULL){
		fprintf(logFile, "%ld: NONNULL CONFIG PATH, READING FROM %s\n", clock(), configPath);
		readConfig();
	}else{
		fprintf(logFile, "%ld: NULL CONFIG FILE, USING DEFAULTS\n", clock());
		funcKeys = FunctionKeys_default;
	}

	//Start the display server
	fprintf(logFile, "%ld: INITING XORG\n", clock());
	init();

	//Make sure that start.subwindow is first set to None just in case
	start.xbutton.subwindow = None;
	start.xkey.subwindow = None;

	fprintf(logFile, "%ld: WM START\n", clock());

	//The big one
	//I need to turn most of these things into their own
	//function but that'll take a bit to do.
	while(1)
	{
		//Grabs next event
		XNextEvent(dpy, &ev);

		//Checks if a window has just been mapped, scans the available clients to check if it
		if(ev.type == CreateNotify)
		{
			//Create a new client for the window created
			fprintf(logFile, "%ld: CLIENT CREATION DETECTED\n", clock());
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
				fprintf(logFile, "%ld: BUTTON PRESS DETECTED\n", clock());
				XGetWindowAttributes(dpy, start.xbutton.subwindow, &attr);
				XSetInputFocus(dpy, start.xbutton.subwindow, RevertToParent, CurrentTime);
			}
			else if(start.type == KeyPress)
			{
				fprintf(logFile, "%ld: KEY PRESS DETECTED\n", clock());
				XGetWindowAttributes(dpy, start.xkey.subwindow, &attr);
				XSetInputFocus(dpy, start.xkey.subwindow, RevertToParent, CurrentTime);
			}

			if(start.type == ButtonPress ? (start.xbutton.button == atoi(funcKeys.killWinKey) && start.xbutton.state == funcKeys.modKey) : (start.xkey.keycode == XKeysymToKeycode(dpy, XStringToKeysym(funcKeys.killWinKey)) && start.xkey.state == funcKeys.modKey))
			{
				fprintf(logFile, "%ld: KILL WINDOW CALLED\n", clock());
				//Kills the selected window
				killWin(start.type == ButtonPress ? start.xbutton.subwindow : start.xkey.subwindow);
			}
			else if(start.type == ButtonPress ? (start.xbutton.button == atoi(funcKeys.focusWinKey) && start.xbutton.state == funcKeys.modKey) : start.xkey.keycode == XKeysymToKeycode(dpy, XStringToKeysym(funcKeys.focusWinKey)) && start.xkey.state == funcKeys.modKey)
			{
				fprintf(logFile, "%ld: FOCUS WINDOW CALLED\n", clock());
				//Raises and focuses the window
				focusWin(start.type == ButtonPress ? start.xbutton.subwindow : start.xkey.subwindow);
			}
			else if(start.type == ButtonPress ? (start.xbutton.button == atoi(funcKeys.restoreWinKey) && start.xbutton.state == funcKeys.modKey) : (start.xkey.keycode == XKeysymToKeycode(dpy, XStringToKeysym(funcKeys.restoreWinKey)) && start.xkey.state == funcKeys.modKey))
			{
				fprintf(logFile, "%ld: RESTORE WINDOW CALLED\n", clock());
				//Checks if the window is already minimized
				if(attr.height == 15 && attr.width == 15)
				{
					//Restores the window to its previous size
					restoreWin(start.type == ButtonPress ? start.xbutton.subwindow : start.xkey.subwindow);
				}
			}
			else if(start.type == ButtonPress ? (start.xbutton.button == atoi(funcKeys.miniWinKey) && start.xbutton.state == funcKeys.modKey) : (start.xkey.keycode == XKeysymToKeycode(dpy, XStringToKeysym(funcKeys.miniWinKey)) && start.xkey.state == funcKeys.modKey))
			{
				fprintf(logFile, "%ld: MINIMIZE WINDOW CALLED\n", clock());
				//Checks if the window is not minimized
				if(attr.height != 15 && attr.width != 15)
				{
					//Minimizes the window
					minimizeWin(start.type == ButtonPress ? start.xbutton.subwindow : start.xkey.subwindow);
				}
			}
			else if(start.type == ButtonPress ? (start.xbutton.button == atoi(funcKeys.killWMKey) && start.xbutton.state == funcKeys.modKey) : (start.xkey.keycode == XKeysymToKeycode(dpy, XStringToKeysym(funcKeys.killWMKey)) && start.xkey.state == funcKeys.modKey))
			{
				fprintf(logFile, " %ld: KILL WM CALLED\n", clock());
				//Kills the WM
				break;
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
			(ev.type == ButtonRelease ? start.xbutton.subwindow = None : start.xkey.subwindow);
		}
		else if(ev.type == DestroyNotify)
		{
			//Remove the client of the window that was destroyed
			fprintf(logFile, "%ld: CLIENT DELETION DETECTED\n", clock());
			killWin(ev.xdestroywindow.window);
		}
	}

	//Nicely shutting down the display server instead of doing it forceably
	fprintf(logFile, "%ld: SHUTTING OFF\n", clock());
	fclose(logFile);
	free(clients);
	XCloseDisplay(dpy);
}
