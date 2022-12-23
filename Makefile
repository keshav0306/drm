client:
	gcc client.c client_lib.c sparkle.c -o c -lm

server:
	gcc serverfg.c display_drm.c list.c requests.c compositor.c
