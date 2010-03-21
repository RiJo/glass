CC       =  g++
CCFLAGS  =  -g -O2 -march=core2 -Wall

#prefix   = /usr
INCLUDES =  -I$/usr/X11R6
LDPATH   =  -L/usr/X11R6/lib
LIBS     =  -lXext -lX11
NAME     =  glass

HEADERS  =  glass.h 		\
            client.h 		\
            foobar.h 		\
            windowmanager.h

OBJS     =  windowmanager.o 	\
            client.o 		\
            foobar.o 		\
            main.o

all: $(NAME)

$(NAME): $(OBJS) Makefile
	$(CC) $(OBJS) $(LDPATH) $(LIBS) -o $@

$(OBJS): %.o: %.cc $(HEADERS)
	$(CC) $(CFLAGS) $(CCFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

#install: all
#	mkdir -p $(DESTDIR)$(prefix)/bin
#	install -s $(NAME) $(DESTDIR)$(prefix)/bin

clean:
	rm -f $(NAME) $(OBJS)