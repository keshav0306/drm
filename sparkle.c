#include "sparkle.h"
#include <math.h>

struct context * new_context(int height, int width, char * addr){
    struct context * context = (struct context *)malloc(sizeof(struct context));
    context->height = height;
    context->width = width;
    context->addr = addr;
    return context;
}

int draw_point(struct context * context, int x, int y, int colour){
    char * addr = context->addr;
    int height = context->height;
    int width = context->width;
    int * pixel = (int *) addr;
    if(x < 0 || y < 0 || x > width || y > height){
        return -1;
    }
    *(pixel + y * width + x) = colour;
}

int draw_line(struct context * context, int x1, int y1, int x2, int y2, int colour){
    float slope = (y2 - y1) / (float)(x2 - x1);
    for(int i=x1;i<=x2;i++){
        float y = (i - x1) * slope + y1;
        int gif = y;
        int num = gif;
        if(y - gif >= 0.5){
            num++;
        }
        draw_point(context, i, num, colour);
    }
}

int draw_circle(struct context * context, int x, int y, int radius, int colour){
    for(int i = x - radius; i < x + radius; i++){
        int base = sqrt((radius * radius) - (i - x) * (i - x));
        int y1 = base + y1;
        int y2 = (-1 * base) + y1;
        draw_point(context, i, y1, colour);
        draw_point(context, i, y2, colour);
    }
}
