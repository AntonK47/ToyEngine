#pragma once

namespace toy::graphics::rhi::validation
{
//#define LOG(message)

#ifdef TOY_ENGINE_ENABLE_RENDERER_INTERFACE_VALIDATION
#define DECLARE_VALIDATOR(type) type validatorObject_{}
#define VALIDATE(expression) TOY_ASSERT_BREAK(validatorObject_.##expression)
#else
#define VALIDATE(expression)
#define DECLARE_VALIDATOR(type)
#endif
}