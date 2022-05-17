in vec2 vertexIn;
in vec4 colourIn;
out vec4 colourOut;

uniform float zPos;
uniform mat4 model;
uniform mat4 projection;

void main()
{
    colourOut = colourIn;
    gl_Position = projection * model * vec4(vertexIn, zPos, 1.0);
}