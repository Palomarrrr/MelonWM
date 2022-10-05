#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#define COLOR(r, g, b) ((r << 16) + (g << 8) + b)

typedef struct Command {

	char command[200];
	unsigned int x, y;

}Command;

typedef struct BoxParams {

	long refreshRate;
	int wid, hgt, x_coord, y_coord;
	unsigned long int backgroundColor, foregroundColor;

}BoxParams;

Display *dpy;
int scr;
Window win;
XEvent event;
GC gc; int numCmds;
Command commands[100];
BoxParams boxParams;


int sys_output(char **buf, char *command) {

        FILE *fp;
        char output[1035];
        int size;

        /* Execute the command */
        fp = popen(command, "r");
        if(fp == NULL){
                fprintf(stdout, "Failed to run command: %s\n", command);
                exit(1); }

        /* Read the output from the command's filepointer */
        fgets(output, sizeof(output) - 1, fp);

        /* Copy the command into the buffer */
        size = strlen(output);
        *buf = malloc(size * 8);
        strcpy(*buf, output);
        (*buf)[size - 1] = '\0';

        fclose(fp);

        return size;
}

void readConfig(char *config_path) {

	FILE *config;
	char *currLine = malloc(sizeof(char) * 200), *buff = malloc(sizeof(char) * 200);
	int i = 0; //incrementer for the command array
	int writeToBuff = 0; //flag to write contents to buffer
	int writeCmdToBuff = 0; //special flag to detect if the current field is a command since some commands may have a period in them
	int paramsOrScripts = 0;
	int fieldToFill = 0;
	int scriptField = 0;
	char *ptr;

	config = fopen(config_path, "r");

	if(config == NULL){
		printf("Failed to read the config file at path: %s\n", config_path);
		exit(1);
	}

	memset(buff, 0, sizeof(char) * 200);

	while(fgets(currLine, 200, config) != NULL){ //this reads through the file line by line

		int j = 0; //this is the incrementer for the buffer

		if(strcmp(currLine, "[BOX_PARAMETERS]\n") == 0){
			paramsOrScripts = 0;
			writeToBuff = 0;
			memset(currLine, 0, sizeof(char) * 200);
			continue;
		}else if(strcmp(currLine, "[SCRIPTS]\n") == 0){
			paramsOrScripts = 1;
			writeToBuff = 0;
			memset(currLine, 0, sizeof(char) * 200);
			continue;
		}else if(paramsOrScripts == 0){ //Getting box parameters
			for(int x = 0; x < strlen(currLine); x++){
				if(currLine[x] == '.' && x + 1 != strlen(currLine) - 1){
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
					}else{
						printf("INVALID FIELD \"%s\" FOUND: LINE 109\n", buff);
						exit(1);
					}
					writeToBuff = (writeToBuff == 1) ? 0 : 1;
					memset(buff, 0, sizeof(char) * 200);
					j = 0;
					continue;
				}else if(x + 1 == strlen(currLine) - 1){
					switch(fieldToFill){
						case 1:
							boxParams.backgroundColor = strtoul(buff, &ptr, 16);
							break;
						case 2:
							boxParams.foregroundColor = strtoul(buff, &ptr, 16);
							break;
						case 3:
							boxParams.refreshRate = strtoul(buff, &ptr, 10);
							break;
						case 4:
							boxParams.wid = atoi(buff);
							break;
						case 5:
							boxParams.hgt = atoi(buff);
							break;
						case 6:
							boxParams.x_coord = atoi(buff);
							break;
						case 7:
							boxParams.y_coord = atoi(buff);
							break;
						default:
							printf("INVALID ASSIGNMENT \"%s\" to field %d: LINE 141", buff, fieldToFill);
							exit(1);
					}
					j = 0;
					writeToBuff = 0;
					writeCmdToBuff = 0; //this shouldn't be toggled but just incase
					memset(buff, 0, sizeof(char) * 200);
					j = 0;
					break;
				}
				if(writeToBuff == 1){
					buff[j] = currLine[x];
					j++;
				}else if(writeToBuff == 0){
					buff[j] = currLine[x];
					j++;
				}else{
					printf("FAILED PROCESSING CONFIG: LINE 115\n");
					exit(1);
				}
			}
		}else if(paramsOrScripts == 1){ //Getting scripts
			for(int x = 0; x < strlen(currLine); x++){
				if(currLine[x] == '.' && x + 1 != strlen(currLine) && writeCmdToBuff == 0){
					j = 0;
					writeToBuff = 1;
					scriptField = 1;
					memset(buff, 0, sizeof(char) * 200);
					continue;
				}else if(x == strlen(currLine) - 1){
					commands[i].y = atoi(buff);
					j = 0;
					scriptField = 0;
					memset(buff, 0, sizeof(char) * 200);
					writeToBuff = 0;
					continue;
				}else if(currLine[x] == ',' && writeCmdToBuff == 0){
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
					writeCmdToBuff = (writeCmdToBuff == 0) ? 1 : 0;
					continue;
				}else if(writeToBuff == 1 && writeCmdToBuff == 1){
					buff[j] = currLine[x];
					j++;
				}else if(writeToBuff == 1){
					buff[j] = currLine[x];
					j++;
				}else if(writeToBuff == 0){
					printf("yes");
				}else{
					printf("FAILED PROCESSING CONFIG: LINE 214\n");
					exit(1);
				}
			}
			i++;
		}
	}

	numCmds = i;
//	}while(fgets(buff, 200, config) != NULL);
	fclose(config);
}

void init() {
	dpy = XOpenDisplay(NULL);
	scr = DefaultScreen(dpy);
	win = XCreateSimpleWindow(dpy, RootWindow(dpy, scr), boxParams.x_coord, boxParams.y_coord, boxParams.wid, boxParams.hgt, 1, boxParams.foregroundColor, boxParams.backgroundColor);
	gc = XCreateGC(dpy, win, 0, 0);
	XSelectInput(dpy, win, ExposureMask|ButtonPressMask|ButtonReleaseMask);

	XMapWindow(dpy, win);
	XFlush(dpy);
}

void draw(int x, int y, char *cmd) {
	char *output = malloc(sizeof(char));
	unsigned int len = sys_output(&output, cmd) - 1;
	XDrawString(dpy, win, gc, x, y, output, len);

	if(output == NULL){
		XSync(dpy, True);
	} else {
		free(output);
		XSync(dpy, True);
	}

}

int main(int argc, char **argv) {

	readConfig("/home/koishi/.config/melon/seed.conf");

	init();

	int checkIfExpose = 0;

	while(checkIfExpose == 0) {
		XNextEvent(dpy, &event);
		if(event.type == Expose) {
			checkIfExpose = 1;
		} else if (event.type == ButtonPress){
        		//to be added later
		} else {
        		//to be added later
		}
	}

	while(1){
		for(int i = 0; i < numCmds ; i++)
		{
			draw(commands[i].x, commands[i].y, commands[i].command);
		}

		usleep(boxParams.refreshRate); // in microseconds since its more flexible than sleep()
		XClearWindow(dpy, win);
	}

	return 0;
}
