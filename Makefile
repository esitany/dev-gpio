

.PHONY: all lib gio devmem bin clean install

all:
	make clean
	make lib
	make bin

gio:
	make -C ./gio COMPILER_PREFIX=$(COMPILER_PREFIX) ARCH=$(ARCH) DEFINES=$(DEFINES) INC_DIR=$(INC_DIR) lib

devmem:
	make -C ./devmem COMPILER_PREFIX=$(COMPILER_PREFIX) ARCH=$(ARCH) DEFINES=$(DEFINES) INC_DIR=$(INC_DIR) lib

lib:
	make gio
	make devmem

bin:
	make -C ./gio COMPILER_PREFIX=$(COMPILER_PREFIX) ARCH=$(ARCH) DEFINES=$(DEFINES) INC_DIR=$(INC_DIR) bin
	make -C ./devmem COMPILER_PREFIX=$(COMPILER_PREFIX) ARCH=$(ARCH) DEFINES=$(DEFINES) INC_DIR=$(INC_DIR) bin

clean:
	make -C ./gio clean
	make -C ./devmem clean

install:
	make -C ./gio DESTDIR=$(DESTDIR) install
	make -C ./devmem DESTDIR=$(DESTDIR) install

