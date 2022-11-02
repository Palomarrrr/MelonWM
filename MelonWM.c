//This is a mess
//I don't know what I'm doing
//How do I get out of here
//Please help

//TEMPORARY TODO LIST:
//
//FIX CHARACTER CODES FROM BEING IN BASE 10 TO BASE 16

#include "Melon.c"

//This is used in resizing and moving windows
#define MAX(a, b) ((a) > (b) ? (a) : (b))


//Declaring variables. I know I should make these as local as possible.
//I'll be working on that sometime soon because it bothers me too.
Display *dpy;
XWindowAttributes attr;
XEvent ev, start;
static Window root;
int scr;
unsigned long int lastFocusedClient, numMini;
char *configPath = NULL, *logFilePath = NULL;
Client *clients;
size_t numClients = 0;
FILE *logFile = NULL;

unsigned int findModCode(int modNum){
	switch(modNum){
		case 0:
			return None;
		case 1:
			return Mod1Mask;
		case 2:
			return Mod2Mask;
		case 3:
			return Mod3Mask;
		case 4:
			return Mod4Mask;
		case 5:
			return Mod5Mask;
		default:
			return None;
	}
}

//This reads the config file
//It's really inefficient but works I guess
void readConfig()
{
	FILE *config;
	char *currLine = malloc(sizeof(char) * 200);
	char *buffer = malloc(sizeof(char) * 200);
	char *ptr;
	int fieldToFill = -1;
	int category = 0;

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
				category = 0;
				continue;
			}
			else if(strcmp(currLine, "[FUNCTION KEYS]\n") == 0)
			{
				memset(currLine, 0, sizeof(char) * 200);
				category = 1;
				continue;
			}
			else if(category == 0)
			{
				for(int x = 0; x < strlen(currLine); x++)
					{
						if(currLine[x] == ' ')
						{
							continue;
						}
						else if(currLine[x] == '='){
							if(strcmp(buffer, "BORDER_INACTIVE_COLOR") == 0){
								fieldToFill = COLOR_INACTIVE;
							}else if(strcmp(buffer, "BORDER_INACTIVE_THICKNESS") == 0){
								fieldToFill = THICKNESS_INACTIVE;
							}else if(strcmp(buffer, "BORDER_ACTIVE_COLOR") == 0){
								fieldToFill = COLOR_ACTIVE;
							}else if(strcmp(buffer, "BORDER_ACTIVE_THICKNESS") == 0){
								fieldToFill = THICKNESS_ACTIVE;
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
								case 0:
									wmParams[COLOR_INACTIVE] = strtoul(convertColorString(buffer), &ptr, 16);
									fieldToFill = -1;
									break;
								case 1:
									wmParams[THICKNESS_INACTIVE] = atoi(buffer);
									fieldToFill = -1;
									break;
								case 2:
									wmParams[COLOR_ACTIVE] = strtoul(convertColorString(buffer), &ptr, 16);
									fieldToFill = -1;
									break;
								case 3:
									wmParams[THICKNESS_ACTIVE] = atoi(buffer);
									fieldToFill = -1;
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
			}else if(category == 1){
				int keyCategory = 0;
				for(int x = 0; x < strlen(currLine); x++){
					if(currLine[x] == ' '){
						continue;
					}else if(currLine[x] == '='){
						if(strcmp(buffer, "EXIT_WM") == 0){
							fieldToFill = EXIT_KEY;
						}else if(strcmp(buffer, "FOCUS_WIN") == 0){
							fieldToFill = FOCUS_KEY;
						}else if(strcmp(buffer, "MINI_WIN") == 0){
							fieldToFill = MINI_KEY;
						}else if(strcmp(buffer, "RESTO_WIN") == 0){
							fieldToFill = RESTO_KEY;
						}else if(strcmp(buffer, "KILL_WIN") == 0){
							fieldToFill = KILL_KEY;
						}else if(strcmp(buffer, "MOVE_WIN") == 0){
							fieldToFill = MOVE_KEY;
						}else if(strcmp(buffer, "SIZE_WIN") == 0){
							fieldToFill = SIZE_KEY;
						}else{
							fprintf(logFile, "%ld: INVALID CONFIG FIELD FOUND: %s\n", clock(), buffer);
							exit(1);
						}
						memset(buffer, 0, sizeof(char) * 200);
						j = 0;
						continue;
					}else if(currLine[x] == ',' && keyCategory == 0 && fieldToFill != -1){ //INPUT FIELDS
						key[INPUT_FIELD][fieldToFill] = (strcmp(buffer, "Key") == 0) ? KeyPress : ButtonPress;
						keyCategory++;
						j = 0;
						memset(buffer, 0, sizeof(char) * 200);
						continue;
					}else if(currLine[x] == ',' && keyCategory == 1){ //KEY FIELDS
						if(key[INPUT_FIELD][fieldToFill] == KeyPress){
							key[KEY_FIELD][fieldToFill] = buffer[0];
						}else{
							key[KEY_FIELD][fieldToFill] = atoi(buffer);
						}
						keyCategory++;
						j = 0;
						memset(buffer, 0, sizeof(char) * 200);
						continue;
					}else if(x == strlen(currLine) - 1){
						key[MOD_FIELD][fieldToFill] = findModCode(atoi(buffer));
						j = 0;
						memset(buffer, 0, sizeof(char) * 200);
						continue;
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

int errHandle(Display *dis, XErrorEvent *evnt) // error handler. i really dont know what this does exactly but if i remove it the wm becomes unstable so im keeping it here
{
	(void)dis;
	(void)evnt;
	fprintf(logFile, "%ld: SOME KIND OF NONFATAL X ERROR HAPPENED\n", clock());
	return 0;
}

void initKeys(){
	for(int i = 0; i < LAST_KEY; i++){
		if(key[INPUT_FIELD][i] == ButtonPress){
			XGrabButton(dpy, key[KEY_FIELD][i], key[MOD_FIELD][i], root, True, ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
		}else{
			XGrabKey(dpy, XKeysymToKeycode(dpy, (char)key[KEY_FIELD][i]), key[MOD_FIELD][i], root, True, GrabModeAsync, GrabModeAsync);
		}
	}
}

int init() // inits the server and its basic attributes
{
	if(!(dpy = XOpenDisplay(0)))
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

	//Init only the keys needed
	initKeys();

	//Telling X what inputs to accept
	XSelectInput(dpy, root, ExposureMask|ButtonPressMask|ButtonReleaseMask|KeyPressMask|KeyReleaseMask|PointerMotionMask|SubstructureNotifyMask); 

	//Setting up some atoms for the wm
	(void)XChangeProperty(dpy, root, XInternAtom(dpy, "_NET_WM_NAME", False), XInternAtom(dpy, "UTF8_STRING", False), 8, PropModeReplace, (unsigned char *)"MelonWM", 7);
	(void)XChangeProperty(dpy, root, XInternAtom(dpy, "_NET_SUPPORTING_WM_CHECK", False), XA_WINDOW, 32, PropModeAppend, (unsigned char *)&root, 1);

	// telling X to monitor keys/buttons with a mod key for input events

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
	clients[numClients - 1].borderColor = wmParams[COLOR_INACTIVE];
	clients[numClients - 1].indPrev = -1;
	clients[numClients - 1].isMini = 0;

	XTextProperty winName;
	XGetWMName(dpy, win, &winName);
	fprintf(logFile, "%ld: CREATED NEW CLIENT #%ld NAMED: %s\n", clock(), numClients, winName.value);


	drawBorders(numClients - 1, wmParams[THICKNESS_INACTIVE]);
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
		clients[clients[i].indPrev].borderColor = wmParams[COLOR_INACTIVE];

		drawBorders(clients[i].indPrev, wmParams[THICKNESS_INACTIVE]);
		fprintf(logFile, "%ld: FOCUS SWITCHED FROM CLIENT #%ld TO CLIENT #%ld\n", clock(), clients[i].indPrev + 1, i + 1);
	}


	//setting the borders of the current window
	clients[i].borderColor = wmParams[COLOR_ACTIVE];
	XSetInputFocus(dpy, clients[i].win, RevertToParent, CurrentTime);
	XRaiseWindow(dpy, clients[i].win);
	lastFocusedClient = i;

	drawBorders(i, wmParams[THICKNESS_ACTIVE]);

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
	//get config and log file paths
	configPath = getPath("HOME", "/.config/MelonWM/MelonWM.conf");
	logFilePath = getPath("HOME", "/.local/share/MelonWM.log");

	//Initialize the log file
	//Open first as write so the program doesn't segfault if the file isn't already there
	//Then set it to read only so the program won't segfault if the user doesnt want a log file
	logFile = fopen(logFilePath, "w");
	logFile = freopen(logFilePath, "r", logFile);

	//Process the command line flags
	processFlags(argc, argv, &configPath, logFilePath, &logFile);
	fprintf(logFile, "%ld: FINISHED PROCESSING FLAGS\n", clock());


	//Check if the config path is blank in case the user ran the program with no config
	if(strcmp(configPath, "0") != 0){
		fprintf(logFile, "%ld: NONNULL CONFIG PATH, READING FROM %s\n", clock(), configPath);
		readConfig();
	}else{
		fprintf(logFile, "%ld: NULL CONFIG FILE, USING DEFAULTS\n", clock());
		wmParams[COLOR_INACTIVE] = 0x000000;
		wmParams[THICKNESS_INACTIVE] = 2;
		wmParams[COLOR_ACTIVE] = 0x00ff00;
		wmParams[THICKNESS_ACTIVE] = 2;

		key[INPUT_FIELD][EXIT_KEY] = ButtonPress;
		key[KEY_FIELD][EXIT_KEY] = 1;
		key[MOD_FIELD][EXIT_KEY] = Mod1Mask;

		key[INPUT_FIELD][FOCUS_KEY] = KeyPress;
		key[KEY_FIELD][FOCUS_KEY] = XK_f;
		key[MOD_FIELD][FOCUS_KEY] = Mod4Mask;
		
		key[INPUT_FIELD][MINI_KEY] = KeyPress;
		key[KEY_FIELD][MINI_KEY] = XK_a;
		key[MOD_FIELD][MINI_KEY] = Mod4Mask;

		key[INPUT_FIELD][RESTO_KEY] = KeyPress;
		key[KEY_FIELD][RESTO_KEY] = XK_s;
		key[MOD_FIELD][RESTO_KEY] = Mod4Mask;

		key[INPUT_FIELD][KILL_KEY] = KeyPress;
		key[KEY_FIELD][KILL_KEY] = XK_q;
		key[MOD_FIELD][KILL_KEY] = Mod4Mask;

		key[INPUT_FIELD][MOVE_KEY] = ButtonPress;
		key[KEY_FIELD][MOVE_KEY] = 1;
		key[MOD_FIELD][MOVE_KEY] = Mod4Mask;

		key[INPUT_FIELD][SIZE_KEY] = ButtonPress;
		key[KEY_FIELD][SIZE_KEY] = 3;
		key[MOD_FIELD][SIZE_KEY] = Mod4Mask;
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
				if(start.type == ButtonPress){
					fprintf(logFile, "%ld: BUTTON PRESS (%d) DETECTED\n", clock(), start.xbutton.button);
					XGetWindowAttributes(dpy, start.xbutton.subwindow, &attr);
					XSetInputFocus(dpy, start.xbutton.subwindow, RevertToParent, CurrentTime);
				}else if(start.type == KeyPress){
					fprintf(logFile, "%ld: KEY PRESS (%c) DETECTED\n", clock(), (char)start.xkey.keycode);
					XGetWindowAttributes(dpy, start.xkey.subwindow, &attr);
					XSetInputFocus(dpy, start.xkey.subwindow, RevertToParent, CurrentTime);
				}

				if(start.type == ButtonPress ? (start.xbutton.button == key[KEY_FIELD][KILL_KEY] && start.xbutton.state == key[MOD_FIELD][KILL_KEY]) : (start.xkey.keycode == XKeysymToKeycode(dpy, key[KEY_FIELD][KILL_KEY]) && start.xkey.state == key[MOD_FIELD][KILL_KEY]))
				{
					fprintf(logFile, "%ld: KILL WINDOW CALLED\n", clock());
					//Kills the selected window
					killWin(start.type == ButtonPress ? start.xbutton.subwindow : start.xkey.subwindow);
					continue;
				}
				else if(start.type == ButtonPress ? (start.xbutton.button == key[KEY_FIELD][FOCUS_KEY] && start.xbutton.state == key[MOD_FIELD][FOCUS_KEY]) : start.xkey.keycode == XKeysymToKeycode(dpy, key[KEY_FIELD][FOCUS_KEY]) && start.xkey.state == key[MOD_FIELD][FOCUS_KEY])
				{
					fprintf(logFile, "%ld: FOCUS WINDOW CALLED\n", clock());
					//Raises and focuses the window
					focusWin(start.type == ButtonPress ? start.xbutton.subwindow : start.xkey.subwindow);
					continue;
				}
				else if(start.type == ButtonPress ? (start.xbutton.button == key[KEY_FIELD][RESTO_KEY] && start.xbutton.state == key[MOD_FIELD][RESTO_KEY]) : (start.xkey.keycode == XKeysymToKeycode(dpy, key[KEY_FIELD][RESTO_KEY]) && start.xkey.state == key[MOD_FIELD][RESTO_KEY]))
				{
					fprintf(logFile, "%ld: RESTORE WINDOW CALLED\n", clock());
					//Checks if the window is already minimized
					if(attr.height == 15 && attr.width == 15)
					{
						//Restores the window to its previous size
						restoreWin(start.type == ButtonPress ? start.xbutton.subwindow : start.xkey.subwindow);
					}
					continue;
				}
				else if(start.type == ButtonPress ? (start.xbutton.button == key[KEY_FIELD][MINI_KEY] && start.xbutton.state == key[MOD_FIELD][MINI_KEY]) : (start.xkey.keycode == XKeysymToKeycode(dpy, key[KEY_FIELD][MINI_KEY]) && start.xkey.state == key[MOD_FIELD][MINI_KEY]))
				{
					fprintf(logFile, "%ld: MINIMIZE WINDOW CALLED\n", clock());
					//Checks if the window is not minimized
					if(attr.height != 15 && attr.width != 15)
					{
						//Minimizes the window
						minimizeWin(start.type == ButtonPress ? start.xbutton.subwindow : start.xkey.subwindow);
					}
					continue;
				}
				else if(start.type == ButtonPress ? (start.xbutton.button == key[KEY_FIELD][EXIT_KEY] && start.xbutton.state == key[KEY_FIELD][EXIT_KEY]) : (start.xkey.keycode == XKeysymToKeycode(dpy, key[KEY_FIELD][EXIT_KEY]) && start.xkey.state == key[MOD_FIELD][EXIT_KEY]))
				{
					fprintf(logFile, " %ld: KILL WM CALLED\n", clock());
					//Kills the WM
					break;
				}
			}
			else if(ev.type == MotionNotify && start.xbutton.subwindow != None && start.xbutton.state == key[MOD_FIELD][MOVE_KEY]) //VERY TEMPORARY IMPLEMENTATION
			{
				if(start.type == ButtonPress && (start.xbutton.button == key[KEY_FIELD][MOVE_KEY] || start.xbutton.button == key[KEY_FIELD][SIZE_KEY])){
					int xDiff = ev.xbutton.x_root - start.xbutton.x_root;
					int yDiff = ev.xbutton.y_root - start.xbutton.y_root;

					//Long ass complicated function to move or resize windows
					XMoveResizeWindow(dpy, start.xbutton.subwindow, attr.x + (start.xbutton.button == key[KEY_FIELD][MOVE_KEY] ? xDiff : 0), attr.y + (start.xbutton.button == key[KEY_FIELD][MOVE_KEY] ? yDiff : 0), MAX(1, attr.width + (start.xbutton.button == key[KEY_FIELD][SIZE_KEY] ? xDiff : 0)), MAX(1, attr.height + (start.xbutton.button == key[KEY_FIELD][SIZE_KEY] ? yDiff : 0)));
					focusWin(ev.xbutton.subwindow);
				}
				continue;
			}
			//If the button or key is released
			else if(ev.type == ButtonRelease || ev.type == KeyRelease)
			{
				//Set the subwindow to None
				(ev.type == ButtonRelease ? start.xbutton.subwindow = None : start.xkey.subwindow);
				continue;
			}
			else if(ev.type == DestroyNotify)
			{
				//Remove the client of the window that was destroyed
				fprintf(logFile, "%ld: CLIENT DELETION DETECTED\n", clock());
				killWin(ev.xdestroywindow.window);
				continue;
			}
	}

	//Nicely shutting down the display server instead of doing it forceably
	fprintf(logFile, "%ld: SHUTTING OFF\n", clock());
	fclose(logFile);
	free(clients);
	XCloseDisplay(dpy);
}
