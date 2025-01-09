#version 330 core
out vec4 FragColor;
in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    sampler2D emission;
    float brightness; // for emission
    float shininess;
}; 

struct Light {
    float type;
    vec3 direction;
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;

    // spotlight
    float innerCutOff;
    float outerCutOff;
};

uniform Light light;

uniform Material material;
uniform vec3 viewPos;

void main()
{
    vec4 albedo = texture(material.diffuse, TexCoords);
    vec3 specMask = vec3(texture(material.specular, TexCoords));
    vec3 lightDir;
    float lightAtten;

    if (light.type == 0.0)
    {
        lightDir = normalize(-light.direction);
        lightAtten = 1.0;
    }
    else if (light.type == 1.0)
    {
        lightDir = light.position - FragPos;
        float dist = length(lightDir);
        lightDir = normalize(lightDir);
        lightAtten = 1.0 / (light.constant + light.linear * dist + light.quadratic * (dist * dist));
    }
    else
    {
        lightDir = normalize(light.position - FragPos);
        float epsilon = light.innerCutOff - light.outerCutOff;
        float cosTheta = dot(normalize(light.direction), -lightDir);
        lightAtten = clamp((cosTheta - light.outerCutOff) / epsilon, 0.0, 1.0);
    }

    vec3 ambient = vec3(albedo) * light.ambient;

    vec3 norm = normalize(Normal);
    // vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = (diff * vec3(albedo)) * light.diffuse;

    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = (spec * specMask) * light.specular;

    vec3 result = mix(ambient + (diffuse + specular) * lightAtten, vec3(texture(material.emission, TexCoords)), material.brightness);
    FragColor = vec4(result, 1.0);
}