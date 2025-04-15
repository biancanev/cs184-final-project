#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform float time;

void main() {
    // Basic parameters
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // Calculate diffuse lighting
    float diff = max(dot(norm, lightDir), 0.0);
    
    // Cel-shading: Quantize the diffuse value into distinct bands
    // Here we use 4 bands but you can adjust this
    if (diff > 0.8) diff = 1.0;
    else if (diff > 0.5) diff = 0.6;
    else if (diff > 0.25) diff = 0.35;
    else diff = 0.1;
    
    // Calculate specular highlight (also quantized)
    float specularStrength = 0.5;
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16.0);
    
    // Quantize specular to get a toon highlight
    if (spec > 0.6) spec = 1.0;
    else spec = 0.0;
    
    // Rim lighting (edge highlight)
    float rimAmount = 0.6;
    float rimThreshold = 0.3;
    float rimDot = 1.0 - dot(viewDir, norm);
    float rim = smoothstep(rimThreshold - 0.01, rimThreshold + 0.01, rimDot);
    vec3 rimColor = rimAmount * rim * lightColor;
    
    // Combine lighting components
    vec3 ambient = 0.1 * lightColor;
    vec3 diffuse = diff * lightColor;
    vec3 specular = specularStrength * spec * lightColor;
    
    // Final color calculation
    vec3 result = (ambient + diffuse + specular + rimColor) * objectColor;
    
    // Apply the result
    FragColor = vec4(result, 1.0);
    
    // Edge detection (outline) can be added in a second pass
    // or using techniques like edge detection in a post-process
}