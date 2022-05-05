in vec3 vertexIn;
in vec4 colourIn;
out vec4 colourOut;

void main()
{
    colourOut = colourIn;
    gl_Position = vec4(vertexIn, 1.0);
}