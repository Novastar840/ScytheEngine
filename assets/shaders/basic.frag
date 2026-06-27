#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

uniform bool hasTexture;
uniform sampler2D texture_diffuse1;

void main() {
    vec3 objectColor;

    // If the mesh has a texture, sample it. Otherwise, use a fallback color.
    if (hasTexture) {
        objectColor = texture(texture_diffuse1, TexCoords).rgb;
    } else {
        objectColor = vec3(0.9, 0.3, 0.1); // Fallback Orange
    }

    // Basic Directional Lighting
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0);
    vec3 ambient = vec3(0.2);

    vec3 result = (ambient + diffuse) * objectColor;
    FragColor = vec4(result, 1.0);
}