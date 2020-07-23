#version 330 core
out vec4 FragColor;

in vec3 color;

uniform vec3 colorDepth;

void main()
{
    vec2 circCoord = 8.0f * gl_PointCoord - 1.0f;
    if(dot(circCoord, circCoord) > 1.0f)
    {
        discard;
    }
    FragColor = vec4(colorDepth * color, 1.0f);
}
