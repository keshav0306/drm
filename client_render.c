#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <stdint.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <math.h>
#include <assert.h>

#include "list.h"
#include "client_include.h"
#include "sparkle.h"
#include "matrix.h"


int main(int argc, char ** argv){
	int handle = connect_to_server("127.0.0.1");
    int h = 400;
    int w = 400;
	struct window * window = create_window(h, w, handle);
	if(window == NULL){
		exit(1);
	}
	map_window(window, handle);
	struct context * context = new_context(window->height, window->width, window->addr);
	memset(window->addr, 255, window->size);
	unsigned int * addr = (unsigned int *) window->addr;
	while(1){
        vec3d * center = make_vec3d(0, 0, 10);
        vec3d * up, * direction, * horizontal;
        float angle = 30 * (3.141592) / 180;
        int height = h;
        int width = w;
        float radius = 1;
        vec3d * circle_center = make_vec3d(0, 0, 0);
        vec3d * bg_color = make_vec3d(0.0, 0.0, 0.0);
        vec3d * light_color = make_vec3d(0.9, 0.9, 0.9);
        vec3d * sphere_color = make_vec3d(1, 0, 0);
        vec3d * light_center = make_vec3d(0, -5,  5);
        vec3d * ambient = make_vec3d(0, 0, 0);
        vec3d * spec = make_vec3d(0.5, 1, 1);
        float shine = 20;

        for(int i=0;i<height;i++){
            for(int j=0;j<width;j++){
                float x = ((float)j / (float)height) - 0.5;
                float y = 0.5 - ((float)i / width);
                float z = (float)1 /(float)(2*tan(angle/2));
                vec3d * dir = make_vec3d(x, y, -z);
                dir->normalize(dir);
                vec3d * c = center->add(circle_center->scale(circle_center, -1), center);
                float A = 1;
                float B = 2 * c->dot(dir, c);
                float C = c->abs_squared - (radius * radius);
                if(B*B - 4*A*C < 0){
                draw_point(context, j, i, 0x000000);
                continue;
                }
                else{
                    float t = (-B - sqrt(B*B - 4*A*C)) / (2 * A);
                    assert(t > 0);
                    vec3d * point = center->add(dir->scale(dir, t), center);
                    vec3d *  normal = point->add(point,  circle_center->scale(circle_center, -1));
                    normal->normalize(normal);
                    vec3d * to_source = light_center->add(light_center, point->scale(point, -1));
                    to_source->normalize(to_source);
                    light_center->normalize(light_center);
                    float dot = point->dot(light_center, normal);
                    float dir_n_dot = dir->dot(dir, normal);
                    vec3d * reflect = dir->add(dir, normal->scale(normal, (-2 * dir_n_dot)));
                    if(dot < 0){
                    dot = 0;
                    }
                    float clamp = fabs(dot);
                    reflect->normalize(reflect);
                    float specdot = reflect->dot(reflect, light_center);
                    if(specdot > 0){
                    }
                    if(specdot < 0){
                    specdot = 0;
                    }
                    clamp = dot;
                    float effect_spec = pow(specdot, shine);
                    float r = sphere_color->elems[0] * clamp * light_color->elems[0] + ambient->elems[0] * sphere_color->elems[0] + light_color->elems[0] * spec->elems[0] * effect_spec;
                    float g = sphere_color->elems[1] * clamp * light_color->elems[1] + ambient->elems[1] * sphere_color->elems[1] + light_color->elems[1] * spec->elems[1] * effect_spec;
                    float b = sphere_color->elems[2] * clamp * light_color->elems[2] + ambient->elems[2] * sphere_color->elems[2] + light_color->elems[2] * spec->elems[2] * effect_spec;
                    int red = r * 255;
                    int blue = b * 255;
                    int green = g * 255;
                    int colour = (0 << 24) + (red << 16) + (green << 8) + blue; 
                    draw_point(context, j, i, colour);
                }
            }
        }

	}
}
