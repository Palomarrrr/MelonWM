#include "Melon.c"

Display *dpy;
int scr;
Window win;
GC gc; 
int numCmds;
Command commands[100];
BoxParams boxParams;
FontContext fontCtx;
char *font;
char *config_path = NULL; 
char *logFilePath = NULL;
FILE *logFile;

void readConfig(char *config_path) {

	FILE *config;
	char *currLine = malloc(sizeof(char) * 200);
	char *buff = malloc(sizeof(char) * 200);
	char *ptr;
	int i = 0; //incrementer for the command array
	int writeException = 0; //flag to detect if the current field is a command since some commands may have a period in them
	int paramsOrScripts = 0;
	int fieldToFill = 0;
	int scriptField = 0;

	config = fopen(config_path, "r");

	if(config == NULL){
		fprintf(stdout, "Failed to read the config file at path: %s\n", config_path);
		exit(1);
	}

	memset(buff, 0, sizeof(char) * 200);

	while(fgets(currLine, 200, config) != NULL){ //this reads through the file line by line

		int j = 0; //this is the incrementer for the buffer

		if(currLine[0] == '\n' || currLine[0] == '#'){
			continue;
		}else if(strcmp(currLine, "[BOX_PARAMETERS]\n") == 0){
			paramsOrScripts = 0;
			memset(currLine, 0, sizeof(char) * 200);
			continue;
		}else if(strcmp(currLine, "[SCRIPTS]\n") == 0){
			paramsOrScripts = 1;
			memset(currLine, 0, sizeof(char) * 200);
			continue;
		}else if(paramsOrScripts == 0){ //Getting box parameters
			for(int x = 0; x < strlen(currLine); x++){
				if(currLine[x] == ' '){
					continue;
				}else if(currLine[x] == '=' && writeException == 0){ //IMPLEMENT CMDBUFF
					if(strcmp(buff, "BACKGROUND_COLOR") == 0){
						fieldToFill = 1;
					}else if(strcmp(buff, "FOREGROUND_COLOR") == 0){
						fieldToFill = 2;
					}else if(strcmp(buff, "REFRESH_RATE") == 0){
						fieldToFill = 3;
					}else if(strcmp(buff, "BOX_WID") == 0){
						fieldToFill = 4;
					}else if(strcmp(buff, "BOX_HGT") == 0){
						fieldToFill = 5;
					}else if(strcmp(buff, "BOX_X_COORD") == 0){
						fieldToFill = 6;
					}else if(strcmp(buff, "BOX_Y_COORD") == 0){
						fieldToFill = 7;
					}else if(strcmp(buff, "FONT") == 0){
						fieldToFill = 8;
					}else if(strcmp(buff, "FONT_COLOR") == 0){
						fieldToFill = 9;
					}else{
						printf("INVALID FIELD \"%s\" FOUND: LINE 109\n", buff);
						exit(1);
					}
					memset(buff, 0, sizeof(char) * 200);
					j = 0;
					writeException = 0;
					continue;
				}else if(x == strlen(currLine) - 1){
					switch(fieldToFill){
						case 1:
							boxParams.backgroundColor = strtoul(convertColorString(buff), &ptr, 16);
							fieldToFill = 0;
							break;
						case 2:
							boxParams.foregroundColor = strtoul(convertColorString(buff), &ptr, 16);
							fieldToFill = 0;
							break;
						case 3:
							boxParams.refreshRate = strtoul(buff, &ptr, 10);
							fieldToFill = 0;
							break;
						case 4:
							boxParams.wid = atoi(buff);
							fieldToFill = 0;
							break;
						case 5:
							boxParams.hgt = atoi(buff);
							fieldToFill = 0;
							break;
						case 6:
							boxParams.x_coord = atoi(buff);
							fieldToFill = 0;
							break;
						case 7 :
							boxParams.y_coord = atoi(buff);
							fieldToFill = 0;
							break;
						case 8:
							boxParams.fontCtx.fontName = malloc(sizeof(char) * strlen(buff));
							memcpy(boxParams.fontCtx.fontName, buff, sizeof(char) * strlen(buff));
							fieldToFill = 0;
							break;
						case 9:
							boxParams.fontCtx.colorName = malloc(sizeof(char) * strlen(buff));
							memcpy(boxParams.fontCtx.colorName , buff, sizeof(char) * strlen(buff));
							fieldToFill = 0;
							break;
							
						default:
							printf("INVALID ASSIGNMENT \"%s\" to field %d: LINE 141", buff, fieldToFill);
							exit(1);
					}
					j = 0;
					writeException = 0;
					memset(buff, 0, sizeof(char) * 200);
					break;
				}else if(currLine[x] == '`'){
					writeException = (writeException == 0) ? 1 : 0;
					continue;
				}
				buff[j] = currLine[x];
				j++;
			}
		}else if(paramsOrScripts == 1){ //Getting scripts
			for(int x = 0; x < strlen(currLine); x++){
				if(currLine[x] == ' ' && writeException == 0){
					continue;
				}else if(currLine[x] == '=' && x + 1 != strlen(currLine) && writeException == 0){
					j = 0;
					scriptField = 1;
					memset(buff, 0, sizeof(char) * 200);
					continue;
				}else if(x == strlen(currLine) - 1){
					commands[i].y = atoi(buff);
					j = 0;
					scriptField = 0;
					memset(buff, 0, sizeof(char) * 200);
					continue;
				}else if(currLine[x] == ',' && writeException == 0){
					if(scriptField == 1){
						memcpy(commands[i].command, buff, sizeof(char) * 200);
						j = 0;
						memset(buff, 0, sizeof(char) * 200);
						scriptField++;
						continue;
					}else if(scriptField == 2){
						commands[i].x = atoi(buff);
						j = 0;
						memset(buff, 0, sizeof(char) * 200);
						continue;
					}else{
						printf("TRIED TO PUT \"%s\" INTO INVALID SCRIPT FIELD %d: LINE 183\n", buff, scriptField);
						exit(1);
					}
				}else if(currLine[x] == '`'){
					writeException = (writeException == 0) ? 1 : 0;
					continue;
				}else{
					buff[j] = currLine[x];
					j++;
				}
			}
			i++;
		}
	}
	numCmds = i;

	fclose(config);

	if(config != NULL){
		printf("YOUR CONFIG FILE FAILED TO CLOSE\n");
	}
}

void drawInit(){ //INIT ALL YOUR XFT DRAWING SHIT HERE OR YOUR BOX WILL SEGFAULT

	//create the gc
	gc = XCreateGC(dpy, win, 0, 0);
	
	//get the font
	boxParams.fontCtx.font = XftFontOpenName(dpy, scr, boxParams.fontCtx.fontName);

	//get default things needed for drawing the text
	boxParams.fontCtx.vis = DefaultVisual(dpy, scr);
	boxParams.fontCtx.colorMap = DefaultColormap(dpy, scr);

	//allocate font color
	XftColorAllocName(dpy, boxParams.fontCtx.vis, boxParams.fontCtx.colorMap, boxParams.fontCtx.colorName, &boxParams.fontCtx.fontColor);
}

void init() {

	//create display stuff
	dpy = XOpenDisplay(NULL);
	scr = DefaultScreen(dpy);
	win = XCreateSimpleWindow(dpy, RootWindow(dpy, scr), boxParams.x_coord, boxParams.y_coord, boxParams.wid, boxParams.hgt, 1, boxParams.foregroundColor, boxParams.backgroundColor);

	//tell the window to ignore all inputs
	XSelectInput(dpy, win, ExposureMask);

	//actually make the window
	XMapWindow(dpy, win);
	XFlush(dpy);

	//init all the things needed to draw text
	drawInit();

	//set the windows fg and bg
	XSetForeground(dpy, gc, boxParams.foregroundColor);
	XSetBackground(dpy, gc, boxParams.backgroundColor);
	//parsefonts(boxParams.font);

	XChangeProperty(dpy, win, XInternAtom(dpy, "WM_NAME", False), XA_STRING, 8, PropModeReplace, (unsigned char *)"MelonSeed", 9);
	XChangeProperty(dpy, win, XInternAtom(dpy, "_NET_WM_NAME", False), XA_STRING, 8, PropModeReplace, (unsigned char *)"MelonSeed", 9);
	XChangeProperty(dpy, win, XInternAtom(dpy, "WM_CLASS", False), XA_STRING, 8, PropModeReplace, (unsigned char *)"MelonSeed", 9);
	XChangeProperty(dpy,win, XInternAtom(dpy, "_NET_SUPPORTING_WM_CHECK", False), XA_WINDOW, 32, PropModeAppend, (unsigned char *)&win, 1);
}

int sys_output(char **buf, char *command) {
        FILE *fp;
        char output[1035];
        int size;

        fp = popen(command, "r");
        if(fp == NULL){
                fprintf(stdout, "Failed to run command: %s\n", command);
                exit(1); 
	}

        fgets(output, sizeof(output) - 1, fp);

        /* Copy the command into the buffer */
        size = strlen(output);
        *buf = malloc(size + 1);
        strcpy(*buf, output);
        (*buf)[size - 1] = '\0';

        pclose(fp);

        return size;
}

void draw() {
	XClearWindow(dpy, win);

	char *output = NULL;
	boxParams.fontCtx.draw = XftDrawCreate(dpy, win, boxParams.fontCtx.vis, boxParams.fontCtx.colorMap);
	XGlyphInfo ext;

	for(int i = 0; i < numCmds ; i++){
		size_t len = sys_output(&output, commands[i].command) - 1;
		//XDrawString(dpy, win, gc, commands[i].x, commands[i].y, output, len); //the old way of doing this just incase something breaks

		XftTextExtentsUtf8(dpy, boxParams.fontCtx.font, (XftChar8 *)output, len, &ext);
		XftDrawStringUtf8(boxParams.fontCtx.draw, &boxParams.fontCtx.fontColor, boxParams.fontCtx.font, commands[i].x, commands[i].y, (XftChar8 *)output, len);
		free(output);
	}

	XftDrawDestroy(boxParams.fontCtx.draw);

	XSync(dpy, True);
}

int main(int argc, char **argv) {
	config_path = getPath("HOME", "/.config/MelonWM/MelonSeed.conf");
	logFilePath = getPath("HOME", "/.local/share/MelonSeed.log");

	logFile = fopen(logFilePath, "w");
	logFile = freopen(logFilePath, "r", logFile);

	processFlags(argc, argv, &config_path, logFilePath, &logFile);

	if(config_path != NULL){
		readConfig(config_path);
	}

	init();

	//Check for initial expose event
	int checkIfExpose = 0;

	while(checkIfExpose == 0) {
		XEvent event;
		XNextEvent(dpy, &event);
		if(event.type == Expose) {
			checkIfExpose = 1;
		}
	}

	while(1){
		draw();
		sleep(boxParams.refreshRate); 
	}

	XFreeGC(dpy, gc);
	XUnmapWindow(dpy, win);
	XCloseDisplay(dpy);

	return 0;
}

