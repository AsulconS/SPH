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
    if(dot(circCoord, circCoord) > 0.5f)
    {
        FragColor = vec4(0.0f, 0.0f, 0.5f, 1.0f);
    }
    else
    {
        FragColor = vec4(colorDepth * color, 1.0f);
    }
}
