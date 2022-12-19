#define MAX_NO_OF_CLIENTS 10
#define BUFF_SIZE 1024


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
