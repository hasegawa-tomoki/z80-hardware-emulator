default: build/z80emu

build/Makefile:
	cmake -S . -B build

.PHONY: build/z80emu
build/z80emu: build/Makefile
	cmake --build build

.PHONY: clean
clean:
	rm -rf build
