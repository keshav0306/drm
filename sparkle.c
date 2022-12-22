#include "sparkle.h"

struct context * new_context(int height, int width, char * addr){
    struct context * context = (struct context *)malloc(sizeof(struct context));
    context->height = height;
    context->height = width;
    context->addr = addr;
    return context;
}

void draw_point(struct context * context, int x, int y, int colour){
    char * addr = context->addr;
    int height = context->height;
    int width = context->width;
    int * pixel = (int *) addr;
    *(pixel + y * width + x) = colour;
}

void draw_line(struct context * context, int x1, int y1, int x2, int y2, int colour){
    int slope = (y2 - y1) / (x2 - x1);
    for(int i=x1;i<=x2;i++){
        int y = (i - x1) * slope + y1;
        draw_point(context, i, y, colour);
    }
}