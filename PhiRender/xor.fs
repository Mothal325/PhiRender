// xor.fs
#version 330
in vec2 fragTexCoord;
out vec4 finalColor;
uniform sampler2D texture0;
uniform sampler2D texture1;

void main() {
    float p = texture(texture0, fragTexCoord).r;
    float n = texture(texture1, fragTexCoord).r;

    bool inPos = p > 0.5;
    bool inNeg = n > 0.5;

    if (inPos != inNeg) {
        finalColor = vec4(0.6, 0.0, 0.0, 0.25);
    } else {
        finalColor = vec4(0.0);
    }
}