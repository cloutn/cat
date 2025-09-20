#include "cat/skybox.h"

namespace cat {

Skybox::Skybox() : 
	m_transform(NULL)
{

}

Transform* Skybox::_transform()
{
	if (NULL == m_transform)	
		m_transform = new Transform;
	return m_transform;
}

} // namespace cat




