#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    sampler2D texture_reflect1;
    sampler2D texture_normal1;
    sampler2D texture_height1;
    float shininess;
}; 

struct DirLight {
    vec3 direction;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

#define NR_POINT_LIGHTS 1

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec4 FragSpaceLightPos;
    mat3 TBN;
} fs_in;

uniform vec3 viewPos;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform Material material;
uniform sampler2D dirShadowMap;
uniform samplerCube pointShadowMap;

uniform float far_plane;
uniform float bumpScale;
uniform float heightScale;

// function prototypes
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec2 texCoords);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 texCoords);
vec2 ParallaxMapping(vec2 texCoords, vec3 viewDirTangentSpace);

uniform float gamma;

vec3 sampleOffsetDirections[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);

float PointShadowCalculation(vec3 fragPos, float bias, int pcf_radius = 0)
{
    vec3 fragToLight = fragPos - pointLights[0].position;
    float shadow = 0.0;
    int samples = 20;
    float viewDistance = length(viewPos - fragPos);
    float diskRadius = (0.1 + (viewDistance / far_plane)) / 25.0;
    float currentDepth = length(fragToLight);
    for (int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(pointShadowMap, fragToLight + sampleOffsetDirections[i] * diskRadius).r;
        closestDepth *= far_plane;
        shadow += currentDepth - bias > closestDepth ? 1.0 : 0.0;
    }
    shadow /= float(samples);
    return shadow;
}

float DirShadowCalculation(vec4 fragPosLightSpace, float bias, int pcf_radius = 0)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.z > 1.0)
        return 0.0;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(dirShadowMap, 0);
    for (int x = -pcf_radius; x <= pcf_radius; ++x)
    {
        for (int y = -pcf_radius; y <= pcf_radius; ++y)
        {
            float pcf_depth = texture(dirShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcf_depth ? 1.0 : 0.0;
        }
    }
    shadow /= pow(2.0 * pcf_radius + 1.0, 2.0);
    return shadow;
}

void main()
{    
    // properties
    mat3 worldToTangent = transpose(fs_in.TBN);
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 viewDirTangentSpace = normalize(worldToTangent * viewDir);
    vec2 texCoords = ParallaxMapping(fs_in.TexCoords, viewDirTangentSpace);
    if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
        discard;
    vec3 tNormal = texture(material.texture_normal1, texCoords).rgb;
    tNormal = normalize(tNormal * 2.0 - 1.0);
    tNormal = normalize(vec3(tNormal.xy * bumpScale, tNormal.z));
    vec3 normal = normalize(fs_in.TBN * tNormal);
    
    // == =====================================================
    // Our lighting is set up in 3 phases: directional, point lights and an optional flashlight
    // For each phase, a calculate function is defined that calculates the corresponding color
    // per lamp. In the main() function we take all the calculated colors and sum them up for
    // this fragment's final color.
    // == =====================================================
    // phase 1: directional lighting
    vec3 result = vec3(0.0);
    result += CalcDirLight(dirLight, normal, viewDir, texCoords);
    // phase 2: point lights
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], normal, fs_in.FragPos, viewDir, texCoords);
    
    FragColor = vec4(pow(result, vec3(1 / gamma)), 1.0);
}

// calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec2 texCoords)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 halfwayVector = normalize(lightDir + viewDir);
    float spec = pow(max(dot(halfwayVector, normal), 0.0), material.shininess);
    // combine results

    vec3 albedo = pow(vec3(texture(material.texture_diffuse1, texCoords)), vec3(gamma));
    vec3 smoothness = pow(vec3(texture(material.texture_specular1, texCoords)), vec3(gamma));

    vec3 ambient = light.ambient * albedo;
    vec3 diffuse = light.diffuse * diff * albedo;
    vec3 specular = light.specular * spec * smoothness;
    float bias = max(0.02 * (1.0 - dot(normal, lightDir)), 0.01);
    float shadow = DirShadowCalculation(fs_in.FragSpaceLightPos, bias, 3);
    return (ambient + (diffuse + specular) * (1.0 - shadow));
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 texCoords)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 halfwayVector = normalize(lightDir + viewDir);
    float spec = pow(max(dot(halfwayVector, normal), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    

    vec3 albedo = pow(vec3(texture(material.texture_diffuse1, texCoords)), vec3(gamma));
    vec3 smoothness = pow(vec3(texture(material.texture_specular1, texCoords)), vec3(gamma));

    // combine results
    vec3 ambient = light.ambient * albedo;
    vec3 diffuse = light.diffuse * diff * albedo;
    vec3 specular = light.specular * spec * smoothness;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    // return ambient + (diffuse + specular);
    float bias = max(0.02 * (1.0 - dot(normal, lightDir)), 0.01);
    float shadow = PointShadowCalculation(fs_in.FragPos, bias);
    return (ambient + (diffuse + specular) * (1.0 - shadow));
}

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDirTangentSpace)
{
    const float numLayers = 30;
    float layerDepth = 1.0 / numLayers;
    float currentLayerDepth = 0.0;
    vec2 P = viewDirTangentSpace.xy / clamp(viewDirTangentSpace.z, 0.1, 1.0) * heightScale;
    vec2 deltaTexCoords = P / numLayers;
    vec2 currentTexCoords = texCoords;
    float currentDepthMapValue = texture(material.texture_height1, currentTexCoords).r;
    while (currentLayerDepth < currentDepthMapValue)
    {
        currentTexCoords -= deltaTexCoords;
        currentDepthMapValue = texture(material.texture_height1, currentTexCoords).r;
        currentLayerDepth += layerDepth;
    }
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    float afterDepth = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(material.texture_height1, prevTexCoords).r - currentLayerDepth + layerDepth;

    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = currentTexCoords * (1.0 - weight) + prevTexCoords * weight;
    
    return finalTexCoords;
}