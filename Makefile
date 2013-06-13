
LIBUSB_CFLAGS = $(shell pkg-config --cflags libusb-1.0)
LIBUSB_LDFLAGS = $(shell pkg-config --libs libusb-1.0)

SDL_CFLAGS = $(shell pkg-config --cflags sdl)
SDL_LDFLAGS = $(shell pkg-config --libs sdl)

OPENCV_CFLAGS = $(shell pkg-config --cflags opencv)
OPENCV_LDFLAGS = $(shell pkg-config --libs opencv)

CFLAGS = -O2 -Wall -DNDEBUG
LDFLAGS =

CC = gcc

all: low-level-leap display-leap-data-sdl display-leap-data-opencv

clean:
	rm -f low-level-leap low-level-leap.o

leap_libusb_init.c.inc:
	@echo "Use make_leap_usbinit.sh to generate leap_libusb_init.c.inc."

low-level-leap.o: low-level-leap.c leap_libusb_init.c.inc
	$(CC) -c $(CFLAGS) $(LIBUSB_CFLAGS) -o $@ $<

low-level-leap: low-level-leap.o
	$(CC) -o $@ $< $(LDFLAGS) $(LIBUSB_LDFLAGS)


display-leap-data-sdl.o: display-leap-data-sdl.c
	$(CC) -c $(CFLAGS) $(SDL_CFLAGS) -o $@ $<

display-leap-data-sdl: display-leap-data-sdl.o
	$(CC) -o $@ $< $(LDFLAGS) $(SDL_LDFLAGS)


display-leap-data-opencv.o: display-leap-data-opencv.c
	$(CC) -c $(CFLAGS) $(OPENCV_CFLAGS) -o $@ $<

display-leap-data-opencv: display-leap-data-opencv.o
	$(CC) -o $@ $< $(LDFLAGS) $(OPENCV_LDFLAGS)
