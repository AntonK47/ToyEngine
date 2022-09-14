#version 460

layout(set = 0, binding = 0) uniform block
{
	vec3 a;
	float b;
};

layout(location = 0) out vec4 color;

#ifdef DEFINE1
void entry()
{
	color = vec4(a,b);
}
#else
void entry1()
{
	color = vec4(a,b);
}
#endif