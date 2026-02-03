#version 430 core

uniform sampler2D mapKd;
uniform sampler2D mapKs;
uniform sampler2D mapKa;

uniform vec3 mtl_Kd;
uniform vec3 mtl_Ks;
uniform vec3 mtl_Ka;
uniform float mtl_Ns;

uniform vec3 lightPosition;
uniform float lightIntensity;
uniform float lightAmbientIntensity;

in vec3 vPosition;
in vec3 vNormal;
in vec3 vUv;

out vec4 fColor;

void main()
{
    //vec3 Kd = mtl_Kd;
    //vec3 Ka = mtl_Ka;
    //vec3 Ks = mtl_Ks;

    vec3 Kd = texture(mapKd, vUv.xy).rgb;
    vec3 Ks = texture(mapKs, vUv.xy).rgb;
    vec3 Ka = texture(mapKa, vUv.xy).rgb;

    vec3 lightDir = normalize(lightPosition - vPosition);
    vec3 viewDirection = normalize(-vPosition);
    vec3 halfVector = normalize(viewDirection + lightDir);

    float geometryTerm = max(dot(lightDir, vNormal), 0.0);
    float specularTerm = max(dot(halfVector, vNormal), 0.0);

    bool inShadow = geometryTerm <= 0;

    vec3 litColorDirect;
    if (inShadow)
    {
        litColorDirect = lightIntensity * (geometryTerm*Kd);        
    }
    else
    {
        litColorDirect = lightIntensity * (geometryTerm*Kd + Ks*pow(specularTerm, mtl_Ns));
    }

    vec3 litColorIndirect = lightAmbientIntensity * Ka;
    
    fColor = vec4(litColorDirect + litColorIndirect, 1.0);
}