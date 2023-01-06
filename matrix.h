typedef struct vec3d{
    float elems[3];
    float abs;
    float abs_squared;
    struct vec3d * (* normalize)(struct vec3d * v);
    float (* dot)(struct vec3d * v1, struct vec3d * v2);
    vec3d * (*scale)(struct vec3d * v, float f);
    vec3d * (*add)(struct vec3d * v1, struct vec3d * v2);
}vec3d;

typedef struct vec4d{
    float elems[4];
    float abs;
    float abs_squared;
    struct vec4d * (* normalize)(struct vec4d * v);
    float (* dot)(struct vec4d * v1, struct vec4d * v2);
    vec4d * (*scale)(struct vec4d * v, float f);
    vec4d * (*add)(struct vec4d * v1, struct vec4d * v2);
}vec4d;

typedef struct sqmatrix3{
    vec3d * cols[3];
    float (* determinant)(struct sqmatrix3 * m);
    vec3d * (* mult_vec3d)(struct sqmatrix3 * m, vec3d * v);
    struct sqmatrix3 * (* mult_sqmatrix3)(struct sqmatrix3 * m1, struct sqmatrix3 * m2);
}sqmatrix3;

typedef struct sqmatrix4{
    vec4d * cols[4];
    float (* determinant)(struct sqmatrix4 * m);
    vec4d * (* mult_vec4d)(struct sqmatrix4 * m, vec3d * v);
    struct sqmatrix4 * (* mult_sqmatrix4)(struct sqmatrix4 * m1, struct sqmatrix4 * m2);
}sqmatrix4;

vec3d * make_vec3d(float a1, float a2, float a3);
vec4d * make_vec4d(float a1, float a2, float a3, float a4);
sqmatrix3 * make_sqmatrix3(vec3d * v1, vec3d * v2, vec3d * v3);
sqmatrix4 * make_sqmatrix4(vec4d * v1, vec4d * v2, vec4d * v3, vec4d * v4);