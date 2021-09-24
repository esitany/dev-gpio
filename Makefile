

.PHONY: all lib bin clean install

all:
	make clean
	make lib
	make bin

lib:
	make -C ./gio COMPILER_PREFIX=$(COMPILER_PREFIX) ARCH=$(ARCH) DEFINES=$(DEFINES) INC_DIR=$(INC_DIR) lib

bin:
	make -C ./gio bin

clean:
	make -C ./gio clean

install:
	make -C ./gio DESTDIR=$(DESTDIR) install

