# makefile for usb pd tests

appname := usbpd_test
appobjs := usbpd_test.o decoder.o encoder.o usbpd.o debug_usbpd.o

CFLAGS := -DMAX_PAYLOAD_LENGTH=1024 
LDFLAGS := 
LIBS := -lm

all: test fw

fw: 
	$(MAKE) -f Makefile.fw build_fw


test: $(appname)

$(appname): $(appobjs)
	$(CC) -o $@ $(LDFLAGS) $^ $(LIBS)
	rm -f $(appobjs)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(appname) $(appobjs)
	$(MAKE) -f Makefile.fw clean

.PHONY: all clean