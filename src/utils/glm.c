#include "math.h" // For sinf and cosf functions
#include "util.h"
#include "glm.h"

Vector4 multiply_matrix_vector(Matrix4x4 *m, Vector4 *v) {
    Vector4 result;
    result.x = m->m[0][0] * v->x + m->m[0][1] * v->y + m->m[0][2] * v->z + m->m[0][3] * v->w;
    result.y = m->m[1][0] * v->x + m->m[1][1] * v->y + m->m[1][2] * v->z + m->m[1][3] * v->w;
    result.z = m->m[2][0] * v->x + m->m[2][1] * v->y + m->m[2][2] * v->z + m->m[2][3] * v->w;
    result.w = m->m[3][0] * v->x + m->m[3][1] * v->y + m->m[3][2] * v->z + m->m[3][3] * v->w;
    return result;
}

// Function to multiply two 4x4 matrices
Matrix4x4 multiply_matrices(const Matrix4x4* a, const Matrix4x4* b) {
    Matrix4x4 result = {{{0}}};
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            for (int k = 0; k < 4; ++k) {
                result.m[i][j] += a->m[i][k] * b->m[k][j];
            }
        }
    }
    return result;
}

Matrix4x4 create_perspective_matrix(float fov, float aspect_ratio, float near, float far) {
    Matrix4x4 mat = {{{0}}};
    float tan_half_fov = tanf(fov / 2.0f);
    float z_range = near - far;

    mat.m[0][0] = 1.0f / (tan_half_fov * aspect_ratio);
    mat.m[1][1] = 1.0f / tan_half_fov;
    mat.m[2][2] = (-near - far) / z_range;
    mat.m[2][3] = 2.0f * far * near / z_range;
    mat.m[3][2] = 1.0f;

    return mat;
}

// Function to create a translation matrix
Matrix4x4 translation(float t_x, float t_y, float t_z) {
    Matrix4x4 result = {{
        {1, 0, 0, t_x},
        {0, 1, 0, t_y},
        {0, 0, 1, t_z},
        {0, 0, 0, 1}
    }};
    return result;
}

// Function to create a scaling matrix
Matrix4x4 scaling(float s_x, float s_y, float s_z) {
    Matrix4x4 result = {{
        {s_x, 0,   0,   0},
        {0,   s_y, 0,   0},
        {0,   0,   s_z, 0},
        {0,   0,   0,   1}
    }};
    return result;
}

// Function to create a rotation matrix around the X-axis
Matrix4x4 rotation_x(float theta) {
    float cos_theta = cosf(theta);
    float sin_theta = sinf(theta);
    Matrix4x4 result = {{
        {1, 0,        0,         0},
        {0, cos_theta, -sin_theta, 0},
        {0, sin_theta, cos_theta,  0},
        {0, 0,        0,         1}
    }};
    return result;
}

// Function to create a rotation matrix around the Y-axis
Matrix4x4 rotation_y(float theta) {
    float cos_theta = cosf(theta);
    float sin_theta = sinf(theta);
    Matrix4x4 result = {{
        {cos_theta,  0, sin_theta, 0},
        {0,          1, 0,         0},
        {-sin_theta, 0, cos_theta, 0},
        {0,          0, 0,         1}
    }};
    return result;
}

// Function to create a rotation matrix around the Z-axis
Matrix4x4 rotation_z(float theta) {
    float cos_theta = cosf(theta);
    float sin_theta = sinf(theta);
    Matrix4x4 result = {{
        {cos_theta, -sin_theta, 0, 0},
        {sin_theta, cos_theta,  0, 0},
        {0,         0,          1, 0},
        {0,         0,          0, 1}
    }};
    return result;
}

// Function to transform a vertex using a Transform structure
Vector4 transform_vertex(const Vector4 *vertex, const Transform *transform) {
    // Create individual transformation matrices
    Matrix4x4 scale_matrix = scaling(transform->Scale.x, transform->Scale.y, transform->Scale.z);
    Matrix4x4 rotation_x_matrix = rotation_x(transform->Rotation.x);
    Matrix4x4 rotation_y_matrix = rotation_y(transform->Rotation.y);
    Matrix4x4 rotation_z_matrix = rotation_z(transform->Rotation.z);
    Matrix4x4 translation_matrix = translation(transform->Position.x, transform->Position.y, transform->Position.z);

    // Combine the transformations: Scale * Rotation * Translation
    Matrix4x4 combined_matrix = multiply_matrices(&scale_matrix, &rotation_x_matrix);
    combined_matrix = multiply_matrices(&combined_matrix, &rotation_y_matrix);
    combined_matrix = multiply_matrices(&combined_matrix, &rotation_z_matrix);
    combined_matrix = multiply_matrices(&combined_matrix, &translation_matrix);

    // Apply the combined transformation to the vertex
    Vector4 transformed_vertex = multiply_matrix_vector(&combined_matrix, vertex);
    return transformed_vertex;
}


// Function to transform a vertex using a Transform structure
Matrix4x4 create_transform_matrix(const Transform *transform) {
    // Create individual transformation matrices
    Matrix4x4 scale_matrix = scaling(transform->Scale.x, transform->Scale.y, transform->Scale.z);
    Matrix4x4 rotation_x_matrix = rotation_x(transform->Rotation.x);
    Matrix4x4 rotation_y_matrix = rotation_y(transform->Rotation.y);
    Matrix4x4 rotation_z_matrix = rotation_z(transform->Rotation.z);
    Matrix4x4 translation_matrix = translation(transform->Position.x, transform->Position.y, transform->Position.z);

    // Combine the transformations: Translation * RotationZ * RotationY * RotationX * Scale
    Matrix4x4 combined_matrix = multiply_matrices(&translation_matrix, &rotation_z_matrix);
    combined_matrix = multiply_matrices(&combined_matrix, &rotation_y_matrix);
    combined_matrix = multiply_matrices(&combined_matrix, &rotation_x_matrix);
    combined_matrix = multiply_matrices(&combined_matrix, &scale_matrix);


    return combined_matrix;
}




uint16_Vector3_t rotate_point_x(uint16_Vector3_t point, float angle) {
    return (uint16_Vector3_t){
        point.x,
        point.y * cos(angle) - point.z * sin(angle),
        point.y * sin(angle) + point.z * cos(angle)
    };
}

// Function to rotate a 3D point around the Y-axis
uint16_Vector3_t rotate_point_y(uint16_Vector3_t point, float angle) {
    return (uint16_Vector3_t){
        point.x * cos(angle) + point.z * sin(angle),
        point.y,
        -point.x * sin(angle) + point.z * cos(angle)
    };
}

Vector2 project_point(Vector4 ndc_vertex, Vector2 fbo_size) {
    // Perspective division to convert from clip space to NDC
    Vector3 normalized_device_coords = {
        ndc_vertex.x / ndc_vertex.w,
        ndc_vertex.y / ndc_vertex.w,
        ndc_vertex.z / ndc_vertex.w
    };

    // Apply viewport transformation
    Vector2 screen_coords = {
        (normalized_device_coords.x + 1.0f) * 0.5f * fbo_size.x,
        (1.0f - (normalized_device_coords.y + 1.0f) * 0.5f) * fbo_size.y
    };

    return screen_coords;
}