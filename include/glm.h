#ifndef GLM_H
#define GLM_H

typedef struct {
    float x, y;
} Vector2;

typedef struct {
    float x, y, z;
} Vector3;

typedef struct {
    float x, y, z, w;
} Vector4;



typedef struct {
    Vector3 Position, Rotation, Scale;
} Transform;

typedef struct {
    float m[4][4];
} Matrix4x4;


Vector4 multiply_matrix_vector(Matrix4x4 *m, Vector4 *v);
Matrix4x4 multiply_matrices(const Matrix4x4* a, const Matrix4x4* b);


Matrix4x4 translation(float t_x, float t_y, float t_z);

Matrix4x4 scaling(float s_x, float s_y, float s_z);
Matrix4x4 rotation_x(float theta);
Matrix4x4 rotation_y(float theta);
Matrix4x4 rotation_z(float theta);

Vector4 transform_vertex(const Vector4 *vertex, const Transform *transform);
Matrix4x4 create_perspective_matrix(float fov, float aspect_ratio, float near, float far);
Matrix4x4 create_transform_matrix(const Transform *transform);
// uint16_Vector3_t rotate_point_x(uint16_Vector3_t point, float angle);
// uint16_Vector3_t rotate_point_y(uint16_Vector3_t point, float angle);

Vector2 project_point(Vector4 ndc_vertex, Vector2 fbo_size);


#endif // GLM_H