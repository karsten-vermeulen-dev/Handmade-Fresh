in vec3 vertexIn;
in vec4 colourIn;
out vec4 colourOut;

uniform mat4 projection;

void main()
{
    colourOut = colourIn;
    gl_Position = projection * vec4(vertexIn, 1.0);
}