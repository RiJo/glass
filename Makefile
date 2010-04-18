CC          = g++
CCFLAGS     = -O2 -march=core2 -Wall
CCDEBUG     = -g -D=_DEBUG_
CCPROFILE   = -pg
INCLUDES    = -I/usr/include
LDPATH      = -L/usr/lib64
LIBS        = -lXext -lX11
NAME        = glass

OBJS        = $(NAME).o               \
              windowmanager.o         \
              client.o                \
              resources.o             \
              widget.o                \
              foobar.o                \
              point.o                 \

all: $(NAME)

debug: CCFLAGS += $(CCDEBUG) $(CCPROFILE)
debug: $(NAME)

$(NAME): $(OBJS) Makefile
	$(CC) $(CCFLAGS) $(OBJS) $(LDPATH) $(LIBS) -o $@

$(OBJS): %.o: src/%.cpp src/%.h
	$(CC) $(CCFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(NAME) $(OBJS)