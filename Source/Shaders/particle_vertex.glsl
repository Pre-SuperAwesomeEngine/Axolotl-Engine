#version 460 

layout(location = 0) uniform mat4 proj;
layout(location = 1) uniform mat4 view;
layout(location = 2) uniform mat4 model;

layout(location = 0) in vec3 vertexPos;
layout(location = 2) in vec2 vertexUv0;

layout(location = 3) in vec3 instanceRight;
layout(location = 4) in vec3 instanceUp;
layout(location = 5) in vec3 instanceFront;
layout(location = 6) in vec3 instanceTrans;
layout(location = 7) in vec4 instanceColor;

out vec2 fragUv0;
out vec3 fragPos;
out vec4 fragColor;

void main()
{
	mat4 transform = mat4(
		vec4(instanceRight, 0.0), 
		vec4(instanceUp, 0.0), 
		vec4(instanceFront, 0.0), 
		vec4(instanceTrans, 1.0));

	fragPos = (model*transform*vec4(vertexPos, 1.0)).xyz;
	fragColor = instanceColor;
	fragUv0 = vertexUv0;


	gl_Position = proj*view*vec4(fragPos, 1.0);
}