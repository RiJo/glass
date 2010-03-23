CC       =  g++
CCFLAGS  =  -O2 -march=core2 -Wall
DEBUG    =  -g -D=DEBUG
INCLUDES =  -I/usr/include
LDPATH   =  -L/usr/lib64
LIBS     =  -lXext -lX11
NAME     =  glass

SRC=src/
DST=bin/

HEADERS  =  $(SRC)$(NAME).h 		\
            $(SRC)client.h 		\
            $(SRC)resources.h 		\
            $(SRC)foobar.h 		\
            $(SRC)windowmanager.h

OBJS     =  windowmanager.o 	\
            client.o 		\
            resources.o 		\
            foobar.o 		\
            main.o

all: $(NAME)

debug: CCFLAGS += $(DEBUG)
debug: $(NAME)

$(NAME): $(OBJS) Makefile
	$(CC) $(OBJS) $(LDPATH) $(LIBS) -o $@

$(OBJS): %.o: ./src/%.cpp $(HEADERS)
	$(CC) $(CFLAGS) $(CCFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(NAME) $(OBJS)