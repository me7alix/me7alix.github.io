CC ?= gcc
FLAGS ?= -std=c99 -g

main: build tmpls md2html src/main.c
	$(CC) $(FLAGS) src/main.c '-DINDEX_TEMPLATE="../build/index.c"' -o build/main
	./build/main

tmpls: build src/tmpls.c
	$(CC) $(FLAGS) src/tmpls.c -o build/tmpls
	./build/tmpls

md2html: build src/md2html.c
	$(CC) $(FLAGS) src/md2html.c -o build/md2html

build:
	mkdir -p ./build
	rm -rf ./pages
	mkdir -p ./pages

.PHONY: build
