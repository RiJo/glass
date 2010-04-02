CC       =  g++
CCFLAGS  =  -O2 -march=core2 -Wall
DEBUG    =  -g -D=_DEBUG_
INCLUDES =  -I/usr/include
LDPATH   =  -L/usr/lib64
LIBS     =  -lXext -lX11
NAME     =  glass

HEADERS  =  src/$(NAME).h         \
            src/client.h          \
            src/resources.h       \
            src/foobar.h          \
            src/windowmanager.h   \

OBJS     =  $(NAME).o               \
            windowmanager.o         \
            client.o                \
            resources.o             \
            foobar.o                \

all: $(NAME)

debug: CCFLAGS += $(DEBUG)
debug: $(NAME)

$(NAME): $(OBJS) Makefile
	$(CC) $(OBJS) $(LDPATH) $(LIBS) -o $@

$(OBJS): %.o: src/%.cpp $(HEADERS)
	$(CC) $(CFLAGS) $(CCFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(NAME) $(OBJS)