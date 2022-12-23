#include "sparkle.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

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

int draw_line_mid_pt(struct context * context, int x1, int y1, int x2, int y2, int colour){
    // if the slope is less than 1 and is positive then, we have to find for a current pixel
    // as (x0, y0), what the next pixel will be, either (x0 + 1, y0) or (x0 + 1, y0 +1)
    // the deciding parameter p(k+1) = p(k) + 2del(y) - 2del(x)(y(k+1) - y(k))
    // p0 = 2del(y) - del(x)
    int delx = x2 - x1;
    int dely = y2 - y1;
    int add1 = 2 * dely;
    int add2 = 2 * dely - 2 * delx;
    float slope = (y2 - y1) / (float)(x2 - x1);
    int y = y1;
    if (slope < 1){
        draw_point(context, x1, y1, colour);
        int p = 2 * dely - delx;
        for(int i=x1 + 1;i<=x2;i++){
            if(p<0){
                p = p + add1;
                draw_point(context, i, y, colour);
            }
            else{
                p = p + add2;
                draw_point(context, i, y++, colour);
            }

        }
    }
    
}

int draw_line_simple(struct context * context, int x1, int y1, int x2, int y2, int colour){
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

int draw_line_simple(struct context * context, int x1, int y1, int x2, int y2, int colour){
    return draw_line_mid_pt(context, x1, y1, x2, y2 ,colour);
}

int draw_circle(struct context * context, int x, int y, int radius, int colour){
    for(int i = x - radius; i < x + radius; i++){
        int base = sqrt((radius * radius) - (i - x) * (i - x));
        int y1 = base + y;
        int y2 = (-1 * base) + y;
        draw_point(context, i, y1, colour);
        draw_point(context, i, y2, colour);
    }
}
