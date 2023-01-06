#include "matrix.h"
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>

vec3d * make_vec3d(float a1, float a2, float a3);
float calc_abs_sq_v3(struct vec3d * v);
struct vec3d * normalize_v3(struct vec3d * v);
vec4d * make_vec4d(float a1, float a2, float a3, float a4);
vec3d * scale_3d(vec3d * v, float f);
vec4d * scale_4d(vec4d * v, float f);
vec3d * add_3d(vec3d * v1, vec3d * v2);
vec4d * add_4d(vec4d * v1, vec4d * v2);
float dot_3d(struct vec3d * v1, struct vec3d * v2);
float dot_4d(struct vec4d * v1, struct vec4d * v2);
float calc_abs_sq_v4(struct vec4d * v);
struct vec4d * normalize_v4(struct vec4d * v);
sqmatrix3 * make_sqmatrix3(vec3d * v1, vec3d * v2, vec3d * v3);
sqmatrix3 * mult_3b3(struct sqmatrix3 * m1, struct sqmatrix3 * m2);
vec3d * mult_3b1(struct sqmatrix3 * m, vec3d * v);
float det_m3(sqmatrix3 * m);
sqmatrix4 * make_sqmatrix4(vec4d * v1, vec4d * v2, vec4d * v3, vec4d * v4);
sqmatrix4 * mult_4b4(struct sqmatrix4 * m1, struct sqmatrix4 * m2);
vec4d * mult_4b1(struct sqmatrix4 * m, vec4d * v);
float det_m4(sqmatrix4 * m);


vec3d * make_vec3d(float a1, float a2, float a3){
    vec3d * v = (vec3d * )malloc(sizeof(vec3d));
    v->elems[0] = a1;
    v->elems[1] = a2;
    v->elems[2] = a3;
    v->normalize = normalize_v3;
    v->abs = sqrt(calc_abs_sq_v3(v));
    v->abs_squared = calc_abs_sq_v3(v);
    v->dot = dot_3d;
    v->scale = scale_3d;
    v->add = add_3d;
    return v;
}

float calc_abs_sq_v3(struct vec3d * v){
    float sum = 0;
    for(int i=0;i<3;i++){
        sum += v->elems[i] * v->elems[i];
    }
    return sum;
}

struct vec3d * normalize_v3(struct vec3d * v){
    float sqroot = sqrt(calc_abs_sq_v3(v));
    for(int i=0;i<3;i++){
        v->elems[i] /= sqroot;
    }
    return v;
}

vec4d * make_vec4d(float a1, float a2, float a3, float a4){
    vec4d * v = (vec4d * )malloc(sizeof(vec4d));
    v->elems[0] = a1;
    v->elems[1] = a2;
    v->elems[2] = a3;
    v->elems[3] = a4;
    v->normalize = normalize_v4;
    v->abs = sqrt(calc_abs_sq_v4(v));
    v->dot = dot_4d;
    v->abs_squared = calc_abs_sq_v4(v);
    v->scale = scale_4d;
    v->add = add_4d;
    return v;
}

vec3d * scale_3d(vec3d * v, float f){
    for(int i=0;i<3;i++){
        v->elems[i] *= f;
    }
    return v;
}

vec4d * scale_4d(vec4d * v, float f){
    for(int i=0;i<4;i++){
        v->elems[i] *= f;
    }
    return v;
}

vec3d * add_3d(vec3d * v1, vec3d * v2){
    float a[3] = {0};
    for(int i=0;i<3;i++){
        a[i] = v1->elems[i] + v2->elems[i];
    }
    return make_vec3d(a[0], a[1], a[2]);
}

vec4d * add_4d(vec4d * v1, vec4d * v2){
    float a[4] = {0};
    for(int i=0;i<4;i++){
        a[i] = v1->elems[i] + v2->elems[i];
    }
    return make_vec4d(a[0], a[1], a[2], a[4]);
}


float dot_3d(struct vec3d * v1, struct vec3d * v2){
    float sum = 0;
    for(int i=0;i<3;i++){
        sum += v1->elems[i] * v2->elems[i];
    }
    return sum;
}

float dot_4d(struct vec4d * v1, struct vec4d * v2){
    float sum = 0;
    for(int i=0;i<4;i++){
        sum += v1->elems[i] * v2->elems[i];
    }
    return sum;
}

float calc_abs_sq_v4(struct vec4d * v){
    float sum = 0;
    for(int i=0;i<4;i++){
        sum += v->elems[i] * v->elems[i];
    }
    return sum;
}

struct vec4d * normalize_v4(struct vec4d * v){
    float sqroot = sqrt(calc_abs_sq_v4(v));
    for(int i=0;i<4;i++){
        v->elems[i] /= sqroot;
    }
    return v;
}

sqmatrix3 * make_sqmatrix3(vec3d * v1, vec3d * v2, vec3d * v3){
    sqmatrix3 * m = (sqmatrix3 * )malloc(sizeof(sqmatrix3));
    m->cols[0] = v1;
    m->cols[1] = v2;
    m->cols[3] = v3;
    m->determinant = det_m3;
    m->mult_sqmatrix3 = mult_3b3;
    m->mult_vec3d = mult_3b1;
    return m;
}

sqmatrix3 * mult_3b3(struct sqmatrix3 * m1, struct sqmatrix3 * m2){
    float a[9] = {0};
    for(int i=0;i<3;i++){
        for(int j=0;j<3;j++){
            for(int k=0;k<3;k++){
                a[3*i + j] += m1->cols[k]->elems[i] * m2->cols[j]->elems[k];
            }
        }
    }
    vec3d * v1 = make_vec3d(a[0], a[3], a[6]);
    vec3d * v2 = make_vec3d(a[1], a[4], a[7]);
    vec3d * v3 = make_vec3d(a[2], a[5], a[8]);

    return make_sqmatrix3(v1, v2, v3);
}

vec3d * mult_3b1(struct sqmatrix3 * m, vec3d * v){
    float a[3] = {0};
    for(int i=0;i<3;i++){
        for(int k=0;k<3;k++){
            a[i] += m->cols[k]->elems[i] * v->elems[k];
        }
    }
    return make_vec3d(a[0], a[1], a[2]);
}

float det_m3(sqmatrix3 * m){
    return m->cols[0]->elems[0];
}

sqmatrix4 * make_sqmatrix4(vec4d * v1, vec4d * v2, vec4d * v3, vec4d * v4){
    sqmatrix4 * m = (sqmatrix4 * )malloc(sizeof(sqmatrix4));
    m->cols[0] = v1;
    m->cols[1] = v2;
    m->cols[2] = v3;
    m->cols[3] = v4;
    m->determinant = det_m4;
    m->mult_sqmatrix4 = mult_4b4;
    m->mult_vec4d = mult_4b1;
    return m;
}

sqmatrix4 * mult_4b4(struct sqmatrix4 * m1, struct sqmatrix4 * m2){
    float a[16] = {0};
    for(int i=0;i<4;i++){
        for(int j=0;j<4;j++){
            for(int k=0;k<4;k++){
                a[4*i + j] += m1->cols[k]->elems[i] * m2->cols[j]->elems[k];
            }
        }
    }

    vec4d * v1 = make_vec4d(a[0], a[4], a[8], a[12]);
    vec4d * v2 = make_vec4d(a[1], a[5], a[9], a[13]);
    vec4d * v3 = make_vec4d(a[2], a[6], a[10], a[14]);
    vec4d * v4 = make_vec4d(a[3], a[7], a[11], a[15]);

    return make_sqmatrix4(v1, v2, v3, v4);
}

vec4d * mult_4b1(struct sqmatrix4 * m, vec4d * v){
    float a[4] = {0};
    for(int i=0;i<4;i++){
        for(int k=0;k<4;k++){
            a[i] += m->cols[k]->elems[i] * v->elems[k];
        }
    }
    return make_vec4d(a[0], a[1], a[2], a[3]);
}

float det_m4(sqmatrix4 * m){
    return m->cols[0]->elems[0];
}
