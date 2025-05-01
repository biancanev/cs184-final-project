#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;

in vec2 TexCoords;  
uniform sampler2D texture_diffuse1; 
uniform bool hasTexture;   

uniform float ambientStrength;
uniform float specularStrength;
uniform float shininess;

void main() {
    // Use a different lighting approach - light based on view direction
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 lightDir = normalize(lightPos - FragPos);
    
    // Ambient
    vec3 ambient = ambientStrength * lightColor;
    
    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Specular
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 finalColor;
    if (hasTexture) {
        finalColor = texture(texture_diffuse1, TexCoords).rgb;
    } else {
        finalColor = objectColor;
    }
    
    // finalColor = objectColor;
    // finalColor 
    // Combine results
    vec3 result = (ambient + diffuse + specular) * finalColor;
    FragColor = vec4(result, 1.0);
}