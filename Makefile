CC=gcc
CFLAGS=-I.

all: clean desktop web

desktop: main.c player.c queue.c
	$(CC) -o rimuru main.c player.c queue.c -lraylib -lGL -lopenal -lm -pthread -ldl 

web:
	emcc -o index.html main.c player.c queue.c -O1 -s -Wall -std=c99 -D_DEFAULT_SOURCE \
		-Wno-missing-braces -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 --profiling  \
		-I./WEB_ARCH ./WEB_ARCH/libraylib.bc -DPLATFORM_WEB --shell-file html_template/raylib_shell.html

run:
	./rimuru
	
run_web:
	emrun index.html

clean:
	rm -rf rimuru* index* || true