CFLAGS += $(shell yed --print-cflags)
CFLAGS += $(shell yed --print-ldflags)
install:
	gcc $(CFLAGS) go.c -o go.so
	cp ./go.so ~/.config/yed/mpy/plugins/lang/syntax/.
