#ifndef __OBJECT_BASE
#define __OBJECT_BASE

#include <Variable_Base.h>

#include <Renderer.h>

namespace LEti
{

	class Object_Base : public LV::Variable_Base
	{
	public:
		DECLARE_VARIABLE;

	public:
		Object_Base() { }
		virtual ~Object_Base() { }

	public:
        virtual void update(float _ratio = 1.0f) = 0;
        virtual void draw(const LEti::Renderer& _renderer) const = 0;

	};

}



#endif // __OBJECT_BASE
