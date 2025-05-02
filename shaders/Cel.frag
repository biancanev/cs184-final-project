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

uniform sampler2D texture_diffuse1;
uniform bool hasTexture;

void main() {
    // Normalize vectors
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);

    // Choose base color from texture or fallback to objectColor
    vec3 finalColor;
    if (hasTexture) {
        finalColor = texture(texture_diffuse1, TexCoords).rgb;
    } else {
        finalColor = objectColor;
    }

    // Quantized diffuse lighting
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 toonColor;
    if (diff > 0.8) toonColor = finalColor;
    else if (diff > 0.5) toonColor = finalColor * 0.8;
    else if (diff > 0.2) toonColor = finalColor * 0.5;
    else toonColor = finalColor * 0.2;

    // Quantized specular
    float specularStrength = 0.5;
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16.0);
    spec = spec > 0.6 ? 1.0 : 0.0;

    // Rim lighting
    float rimAmount = 0.3;
    float rimThreshold = 0.4;
    float rimDot = 1.0 - dot(viewDir, norm);
    float rim = smoothstep(rimThreshold - 0.01, rimThreshold + 0.01, rimDot);
    vec3 rimColor = rimAmount * rim * lightColor;

    // Lighting components
    vec3 ambient = 0.1 * lightColor;
    vec3 diffuse = toonColor * lightColor;

    // Final color output
    vec3 result = (ambient + diffuse + rimColor);
    FragColor = vec4(result, 1.0);
}