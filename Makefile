CC       =  g++
CCFLAGS  =  -g -O2 -march=core2 -Wall
INCLUDES =  -I/usr/include
LDPATH   =  -L/usr/lib64
LIBS     =  -lXext -lX11
NAME     =  glass

HEADERS  =  $(NAME).h 		\
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

clean:
	rm -f $(NAME) $(OBJS)