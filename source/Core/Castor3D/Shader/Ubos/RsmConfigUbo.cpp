#include "Castor3D/Shader/Ubos/RsmConfigUbo.hpp"

#include "Castor3D/Engine.hpp"
#include "Castor3D/Render/RenderSystem.hpp"
#include "Castor3D/Render/Technique/Opaque/ReflectiveShadowMapGI/RsmConfig.hpp"
#include "Castor3D/Scene/Light/DirectionalLight.hpp"
#include "Castor3D/Scene/Light/PointLight.hpp"
#include "Castor3D/Scene/Light/SpotLight.hpp"
#include "Castor3D/Scene/Light/Light.hpp"

namespace castor3d
{
	std::string const RsmConfigUbo::BufferRsmConfig = "RsmConfig";
	std::string const RsmConfigUbo::Intensity = "c3d_rsmIntensity";
	std::string const RsmConfigUbo::RMax = "c3d_rsmRMax";
	std::string const RsmConfigUbo::SampleCount = "c3d_rsmSampleCount";
	std::string const RsmConfigUbo::Index = "c3d_rsmIndex";

	RsmConfigUbo::RsmConfigUbo( Engine & engine )
		: m_engine{ engine }
	{
		if ( engine.getRenderSystem()->hasCurrentRenderDevice() )
		{
			initialise();
		}
	}

	RsmConfigUbo::~RsmConfigUbo()
	{
	}

	void RsmConfigUbo::initialise()
	{
		if ( !m_ubo )
		{
			m_ubo = m_engine.getRsmConfigUboPool().getBuffer( VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );
		}
	}

	void RsmConfigUbo::cleanup()
	{
		if ( m_ubo )
		{
			m_engine.getRsmConfigUboPool().putBuffer( m_ubo );
		}
	}

	void RsmConfigUbo::update( Light const & light )
	{
		CU_Require( m_ubo );
		auto & config = light.getRsmConfig();
		auto & data = m_ubo.getData();
		data.intensity = config.intensity;
		data.maxRadius = config.maxRadius;
		data.sampleCount = config.sampleCount.value().value();
		data.index = light.getShadowMapIndex();
	}
}
