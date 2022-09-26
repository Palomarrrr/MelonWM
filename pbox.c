#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#define BOX_SIZE 200
#define COLOR(r, g, b) ((r << 16) + (g << 8) + b)
Display *dpy;
int scr;
Window win;
XEvent event;
GC gc;


int sys_output(char **buf, char *command)
{
        FILE *fp;
        char output[1035];
        int size;

        /* Execute the command */
        fp = popen(command, "r");
        if(fp == NULL){
                fprintf(stderr, "Failed to run command: %s\n", command);
                exit(1);
        }

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

void init() {
	dpy = XOpenDisplay(NULL);
	scr = DefaultScreen(dpy);
	win = XCreateSimpleWindow(dpy, RootWindow(dpy, scr), 10, 10, BOX_SIZE, BOX_SIZE, 1, COLOR(0, 43, 54), COLOR(253, 246, 227));
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
		draw(54, 25, "echo \"* Henlo $USER *\" ");
		draw(54, 25, "echo ________________");
		draw(17, 50, "date");
		draw(5, 62, "echo --------------------------------");
		draw(5, 90, "~/Scripts/net.sh");
		//draw(5, 75, "~/Scripts/music.sh");
		draw(5, 105, "~/Scripts/battery.sh");
		draw(5, 120, "~/Scripts/memuse.sh");
		draw(5, 135, "~/Scripts/cputemp.sh");
		draw(5, 150, "~/Scripts/getkern.sh");
		draw(5, 165, "~/Scripts/getdefshell.sh");
		draw(5, 180, "echo WDM: cppwm");
		draw(5, 192, "echo --------------------------------");
		usleep(1000000); // in microseconds since its more flexible than sleep()
		XClearWindow(dpy, win);
	}

	return 0;
}

