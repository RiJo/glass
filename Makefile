CC          = g++
CCFLAGS     = -Wall -Werror
CCOPTS      = -O3 -march=core2 -mtune=core2
CCDEBUG     = -g -D=_DEBUG_
CCPROFILE   = -pg
INCLUDES    = -I/usr/include
LDPATH      = -L/usr/lib64
LIBS        = -lXext -lX11
NAME        = glass

SRC         = src
BIN         = bin

OBJS        = $(NAME).o               \
              windowmanager.o         \
              client.o                \
              resources.o             \
              widget.o                \
              foobar.o                \
              point.o                 \

all: $(NAME) $(SRC)/$(NAME).h

debug: CCFLAGS += $(CCDEBUG) $(CCPROFILE)
debug: $(NAME)

$(NAME): $(OBJS) Makefile
	$(CC) $(CCFLAGS) $(OBJS) $(LDPATH) $(LIBS) -o $(BIN)/$@

%.o: $(SRC)/%.cpp $(SRC)/%.h
	$(CC) $(CCFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(BIN)/$(NAME) $(OBJS)