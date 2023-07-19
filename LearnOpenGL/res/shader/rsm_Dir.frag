#version 450 core
layout(location = 0) out vec4 gPositionDepth;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec3 gFlux;

in vec2 texCoord;
in vec3 fragPos;
in vec3 normal;

uniform sampler2D texture_diffuse;
uniform vec3 light;

const float PI = 3.14159265359;

void main()
{

    gPositionDepth.rgb = fragPos;

    gPositionDepth.a = gl_FragCoord.z;

    gNormal = normalize(normal);

    gFlux.rgb = light * texture(texture_diffuse, texCoord).rgb / PI;
}