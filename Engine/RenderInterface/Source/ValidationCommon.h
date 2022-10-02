#pragma once
#include <assert.h>
namespace toy::renderer::validation
{
//#define LOG(message)
#define TOY_ASSERT(expression) assert(expression)
#define TOY_ASSERT_BREAK(expression) if(!(expression)) __debugbreak()

#ifdef TOY_ENGINE_ENABLE_RENDERER_INTERFACE_VALIDATION
#define DECLARE_VALIDATOR(type) type validatorObject_{}
#define VALIDATE(expression) TOY_ASSERT_BREAK(validatorObject_.##expression)
#else
#define VALIDATE(expression)
#define DECLARE_VALIDATOR(type)
#endif
}