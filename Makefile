# makefile for usb pd tests

appname := usbpd_test
appobjs := main.o decoder.o encoder.o usbpd.o debug_usbpd.o

CFLAGS := -DMAX_PAYLOAD_LENGTH=1024 
LDFLAGS := 
LIBS := -lm

all: $(appname)

$(appname): $(appobjs)
	$(CC) -o $@ $(LDFLAGS) $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(appname) $(appobjs)

.PHONY: all clean