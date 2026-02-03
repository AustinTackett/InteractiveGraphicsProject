#version 430 core

uniform mat4 mvp;
uniform mat4 mv;
uniform mat3 mvNormal;

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aUv;

out vec3 vPosition;
out vec3 vNormal;
out vec3 vLightPos;
out vec3 vUv;

void main()
{
    vUv = aUv;
    
    vPosition = vec3( (mv * vec4(aPosition, 1.0)) );
    vNormal = normalize(mvNormal * aNormal);
    gl_Position = mvp * vec4(aPosition, 1.0);
}