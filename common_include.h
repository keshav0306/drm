#define PORT 1000
#define NUM_ARGS 5
#define COMMON_FILE_NAME "kawaii"

enum opcodes{
	CREATE_WINDOW,
	MAP_WINDOW,
	UNMAP_WINDOW,
	CURRENT_EVENT,
	DESTROY_WINDOW,
};

struct request{
	int opcode;
	int args[NUM_ARGS];
	int num_args;
};

struct response{
	int response[NUM_ARGS];
	int num_responses;
	int return_value;
};