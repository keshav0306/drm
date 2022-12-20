#include "common_include.h"

struct window{
	int window_id;
	int height;
	int width;
    int size;
	int mapped;
    char * addr;
};

struct event{
	int event_bits;
	int x;
	int y;
	int left_clicked;
	int right_clicked;
	int mid_clicked;
	char key;
};

int connect_to_server(char * ip_address);
struct window * create_window(int height, int width, int handle);
int map_window(struct window * window, int handle);
int unmap_window(struct window * window, int handle);
int destroy_window(struct window * window, int handle);
struct event * get_current_event(struct window * window, int handle);