#version 460
  
in vec2 TexCoord;

layout(binding = 0) uniform sampler2D image;
  
uniform bool horizontal;
uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

out vec4 outColor;

void main()
{             
    vec2 texOffset = 1.0 / textureSize(image, 0);
    vec3 result = texture(image, TexCoord).rgb * weight[0]; // current fragment's contribution
    if (horizontal)
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(image, TexCoord + vec2(texOffset.x * i, 0.0)).rgb * weight[i];
            result += texture(image, TexCoord - vec2(texOffset.x * i, 0.0)).rgb * weight[i];
        }
    }
    else
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(image, TexCoord + vec2(0.0, texOffset.y * i)).rgb * weight[i];
            result += texture(image, TexCoord - vec2(0.0, texOffset.y * i)).rgb * weight[i];
        }
    }
    outColor = vec4(result, 1.0);
}