#include <iostream>
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
#include <Imlib2.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <bits/stdc++.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define COLOR(r, g, b) ((r<<16) + (g<<8) + b)
#define DEBUG(txt) std::cout << "ERROR ON LINE " << __LINE__ << " :" << txt << std::endl

