#define MAX_NO_OF_CLIENTS 10
#define BUFF_SIZE 1024
#define MOUSE_HEIGHT 20
#define MOUSE_WIDTH 20
#define MOUSE_ID 1024

struct window{
	int window_id;
	int height;
	int size;
	int width;
	int mapped;
	char * addr;
	int x;
	int y;
};
