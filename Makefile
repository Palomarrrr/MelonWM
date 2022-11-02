# paths to certain things
PATH_LOCAL ?= /usr/local
PATH_X11 ?= /usr/X11R6
PATH_MAN ?= ${PATH_LOCAL}/share/man #SOON
PATH_CONFIG ?= ${HOME}/.config
PATH_LOCAL_INC ?= ${PATH_LOCAL}/include
PATH_LOCAL_LIB ?= ${PATH_LOCAL}/lib
PATH_X11_INC ?= ${PATH_X11}/include
PATH_X11_LIB ?= ${PATH_X11}/lib
PATH_FREETYPE_INC ?= /usr/include/freetype2

# includes/libs
INCLUDE += -I${PATH_LOCAL_INC} -I${PATH_X11_INC} -I${PATH_FREETYPE_INC} -I${PATH_X11_INC}/freetype2
LIB += -L${PATH_LOCAL_LIB} -L${PATH_X11_LIB} -lfontconfig -lXft -lX11 -lXinerama -lXrender -lXext

# flags
CFLAGS += -Wall -pedantic -Wextra -Wno-unused-parameter -g

# program aliases
WM = MelonWM
BOX = MelonSeed
OBJ = Melon

all: ${OBJ} ${WM} ${BOX}

${WM}:
	${CC} ${WM}.c ${INCLUDE} ${LIB} ${CFLAGS} -o ${WM}

${BOX}:
	${CC} ${BOX}.c ${INCLUDE} ${LIB} ${CFLAGS} -o ${BOX}

${OBJ}:
	${CC} ${OBJ}.c ${INCLUDE} ${LIB} ${CFLAGS} -O -c 

config:
	mkdir -p ${PATH_CONFIG}/${WM}
	install ${WM}.conf ${PATH_CONFIG}/${WM}/${WM}.conf
	install ${BOX}.conf ${PATH_CONFIG}/${WM}/${BOX}.conf

clean:
	rm -f ${WM} ${BOX} ${OBJ}.o

install: all
	install -m 755 ${WM} ${PATH_LOCAL}/bin/${WM}
	install -m 755 ${BOX} ${PATH_LOCAL}/bin/${BOX}

uninstall: clean
	rm -f ${PATH_LOCAL}/bin/${WM}
	rm -f ${PATH_LOCAL}/bin/${BOX}

test: all
	xinit ${XINITRC} -- `which Xephyr` :1 -screen 1024x768 +xinerama

.PHONY: all clean install uninstall test
