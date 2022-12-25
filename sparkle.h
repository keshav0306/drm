struct context{
    int height;
    int width;
    char * addr;
};

struct context * new_context(int height, int width, char * addr);

int draw_point(struct context * context, int x, int y, int colour);

int draw_line(struct context * context, int x1, int y1, int x2, int y2, int colour);

int draw_circle(struct context * context, int x, int y, int radius, int colour);

int draw_text(struct context * context, char * string, int x, int y, int colour);
