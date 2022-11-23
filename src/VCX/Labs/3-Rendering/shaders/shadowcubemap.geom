#version 410 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

uniform mat4 u_LightMatrices[6];

layout(location = 0) in  vec2 v_TexCoord[];

layout(location = 0) out vec4 g_Position;
layout(location = 1) out vec2 g_TexCoord;

void main() {
    for (int face = 0; face < 6; ++face) {
        gl_Layer = face; // built-in variable that specifies to which face we render.
        for (int i = 0; i < 3; ++i) { // for each triangle's vertices
            g_Position = gl_in[i].gl_Position;
            gl_Position = u_LightMatrices[face] * g_Position;
            g_TexCoord = v_TexCoord[i];
            EmitVertex();
        }    
        EndPrimitive();
    }
}