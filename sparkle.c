#include "sparkle.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <linux/input-event-codes.h>
#include "font.h"

const int * font_map[128] = {0};
char kbd_map[256] = {0}; // only for text editing applications ... otherwise applications must process raw events from server
int iec_alphabets[26] = {30, 48, 46, 32, 18, 33, 34, 35, 23, 36, 37, 38, 50, 49,\
                        24, 25, 16, 19, 31, 20, 22, 47, 17, 45, 21, 44};

void initialize_font(){
    const int * lower_case_alphabets[26] = {a, b, c, d, e, f, g, h, i, j, k, l, m, n, o,\
                                            p, q, r, s, t, u, v, w, x, y, z};

    const int * upper_case_alphabets[26] = {A, B, C, D, E, F, G, H, I, J, K, L, M, N, O,\
                                            P, Q, R, S, T, U, V, W, X, Y, Z};

    const int * numbers[10] = {zero, one, two, three, four, five, six, seven, eight, nine};

    for(int i=0;i<10;i++){
        font_map[i + 48] = numbers[i];
    }
    for(int i=0;i<26;i++){
        font_map[i + 65] = upper_case_alphabets[i];
    }
    for(int i=0;i<26;i++){
        font_map[i + 97] = lower_case_alphabets[i];
    }
}

void initialze_kbd_map(){
    // based on linux/input-event-codes.h

    //numbers
    for(int i=2;i<11;i++){
        kbd_map[i] = i + 47;
    }
    kbd_map[11] = 48;
    //alphabets (only lower case)
    for(int i=0;i<26;i++){
        kbd_map[iec_alphabets[i]] = i + 97;
    }

    //backspace, enter -> mapping them to ascii '\b' and '\n'

    kbd_map[14] = '\b';
    kbd_map[28] = '\n';

}

char to_char(int key_code){
    return kbd_map[key_code];
}

struct context * new_context(int height, int width, char * addr){
    struct context * context = (struct context *)malloc(sizeof(struct context));
    context->height = height;
    context->width = width;
    context->addr = addr;
    initialize_font();
    initialze_kbd_map();
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

int draw_line(struct context * context, int x1, int y1, int x2, int y2, int colour){
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

int draw_text(struct context * context, char * string, int x, int y, int colour){
    char * addr = context->addr;
    int height = context->height;
    int width = context->width;
    int * pixel = (int *) addr;
    int curr_x = x, curr_y = y;

    for(char * c = &string[0]; *c != '\0'; c++){
        char character = *c;
        int * pixel_map = (int *)(font_map[character]);
        if(!pixel_map){
            continue;
        }
        int x_dim = FONT_SIZE_X;
        int y_dim = FONT_SIZE_Y;
        int cond1 = x_dim < width  - curr_x;
        
        if(!cond1){
            curr_y += y_dim;
            curr_x = 0;
        }
        int cond2 = y_dim < height - curr_y;
        if(!cond2){
            curr_x = 0;
            curr_y = 0;
        }
        
        for(int i=0;i<y_dim;i++){
            for(int j=0;j<x_dim;j++){
                if(pixel_map[x_dim * i + j]){
                    pixel[width * (i + curr_y) + j + curr_x] = pixel_map[x_dim * i + j] * colour;
                }
            }
        }
        curr_x += x_dim;
    }
}
