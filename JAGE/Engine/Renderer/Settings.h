namespace OpenGL::Settings {
    /* General */
    #define MAX_MESHES 10000
    #define MAX_MESH_INSTANCES 20000
    #define MAX_MATERIAL_INSTANCES 200
    #define MAX_POINT_LIGHTS 128
    #define MAX_TEXTURES 10000

    /* Shadows */
    #define SHADOW_MAP_RESOLUTION 1024
    #define SHADOW_MAP_N_CASCADES 4
    #define SHADOW_MAP_MAX_CASCADES 16

    /* Screen Space Ambinet Occlusion */
    #define SSAO_KERNEL_SIZE 64
    #define SSAO_RADIUS 1.8f
    #define SSAO_BIAS 0.025f
    #define SSAO_POWER .5f

    /* Post-processing */
    #define GAMMA 2.2f
};