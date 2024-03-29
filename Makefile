all:server terminal client render

client:
	gcc client.c client_lib.c sparkle.c -o c -lm

render:
	gcc client_render.c client_lib.c sparkle.c matrix.c -o r -lm

server:
	gcc serverfg.c display_drm.c list.c requests.c compositor.c

terminal:
	gcc terminal.c client_lib.c sparkle.c -o t -lm