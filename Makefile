CC       =  g++
CCFLAGS  =  -O2 -march=core2 -Wall
DEBUG    =  -g -D=_DEBUG_
PROFILE  =  -pg
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

debug: CCFLAGS += $(DEBUG) $(PROFILE)
debug: $(NAME)

$(NAME): $(OBJS) $(HEADERS) Makefile
	$(CC) $(CCFLAGS) $(OBJS) $(LDPATH) $(LIBS) -o $@

$(OBJS): %.o: src/%.cpp
	$(CC) $(CCFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(NAME) $(OBJS)