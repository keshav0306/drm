#define MAX_NO_OF_CLIENTS 10
#define BUFF_SIZE 1024
#define MOUSE_HEIGHT 20
#define MOUSE_WIDTH 20
#define MOUSE_ID 1024
#define WINDOW_BAR_HEIGHT 20


struct window{
	int window_id;
	int shm_id;
	int conn_id; // fd
	int height;
	int size;
	int width;
	int mapped;
	char * addr;
	int x;
	int y;
};

struct mouse_window{
	int window_id;
	int shm_id;
	int conn_id;
	int height;
	int size;
	int width;
	int mapped;
	char * addr;
	int x;
	int y;


	int left_clicked;
	int right_clicked;
	int mid_clicked;
};
