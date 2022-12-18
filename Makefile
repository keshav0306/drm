client:
	gcc client.c client_lib.c -o c

server:
	gcc serverfg.c display_drm.c list.c 