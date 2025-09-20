#include "cat/light.h"

namespace cat {

Light::Light() : 
	m_transform(NULL)
{

}

Transform* Light::_transform()
{
	if (NULL == m_transform)	
		m_transform = new Transform;
	return m_transform;
}

} // namespace cat

