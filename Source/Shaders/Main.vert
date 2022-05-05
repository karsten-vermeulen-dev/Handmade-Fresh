in vec3 vertexIn;
in vec4 colourIn;
out vec4 colourOut;

uniform float index;
uniform mat4 projection;

void main()
{
    colourOut = colourIn;

    vec3 vertex = vertexIn;
    vertex.y += index * 0.05;

    gl_Position = projection * vec4(vertex, 1.0);
}