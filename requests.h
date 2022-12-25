struct kbd_state{
    int key;
    int num;
};

void init_request_globals();
struct response * handle_request(struct request * request);
struct response * request_destroy_window(struct request * request);
struct response * request_current_event(struct request * request);
struct response * request_unmap_window(struct request * request);
struct response * request_map_window(struct request * request);
int change_window_map_data(struct list * list, int window_id, int map);
struct response * request_create_window(struct request * request);
struct response * error_response();
char * make_file(int * num_file);