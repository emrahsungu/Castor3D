#include "Castor3D/Shader/Ubos/LpvConfigUbo.hpp"

#include "Castor3D/Engine.hpp"
#include "Castor3D/Render/RenderSystem.hpp"
#include "Castor3D/Scene/Light/Light.hpp"
#include "Castor3D/Scene/Light/DirectionalLight.hpp"
#include "Castor3D/Scene/Light/PointLight.hpp"
#include "Castor3D/Scene/Light/SpotLight.hpp"

#include <CastorUtils/Graphics/Grid.hpp>

namespace castor3d
{
	std::string const LpvConfigUbo::LpvConfig = "LpvConfig";
	std::string const LpvConfigUbo::LightView = "c3d_lightView";
	std::string const LpvConfigUbo::MinVolumeCorner = "c3d_minVolumeCorner";
	std::string const LpvConfigUbo::GridSizes = "c3d_gridSizes";
	std::string const LpvConfigUbo::Config = "c3d_lpvConfig";

	LpvConfigUbo::LpvConfigUbo( Engine & engine
		, uint32_t index )
		: m_engine{ engine }
		, m_ubo{ makeUniformBuffer< Configuration >( *m_engine.getRenderSystem()
			, 1u
			, VK_BUFFER_USAGE_TRANSFER_DST_BIT
			, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			, "LpvConfigUbo" + std::to_string( index ) ) }
	{
	}

	void LpvConfigUbo::update( castor::Grid const & grid
		, Light const & light
		, uint32_t cascadeIndex
		, float texelAreaModifier
		, float indirectAttenuation )
	{
		CU_Require( light.getLightType() == LightType::eDirectional );
		CU_Require( m_ubo );

		auto minVolumeCorner = grid.getMin();
		auto gridSize = grid.getDimensions();
		auto cellSize = grid.getCellSize();

		auto & data = m_ubo->getData();
		data.minVolumeCorner = castor::Point4f{ minVolumeCorner->x, minVolumeCorner->y, minVolumeCorner->z, cellSize };
		data.gridSizes = castor::Point4f{ gridSize->x, gridSize->y, gridSize->z, light.getBufferIndex() };
		data.lightView = light.getDirectionalLight()->getViewMatrix( cascadeIndex );
		data.config = castor::Point4f{ indirectAttenuation, 1.0f, texelAreaModifier, 0.0f };
		m_ubo->upload();
	}

	void LpvConfigUbo::update( castor::Grid const & grid
		, Light const & light
		, float texelAreaModifier
		, float indirectAttenuation )
	{
		CU_Require( light.getLightType() != LightType::eDirectional );
		CU_Require( m_ubo );

		auto minVolumeCorner = grid.getMin();
		auto gridSize = grid.getDimensions();
		auto cellSize = grid.getCellSize();

		auto & data = m_ubo->getData();
		data.minVolumeCorner = castor::Point4f{ minVolumeCorner->x, minVolumeCorner->y, minVolumeCorner->z, cellSize };
		data.gridSizes = castor::Point4f{ gridSize->x, gridSize->y, gridSize->z, light.getBufferIndex() };

		switch ( light.getLightType() )
		{
		case LightType::ePoint:
			data.config = castor::Point4f{ indirectAttenuation, 1.0f, texelAreaModifier, 0.0f };
			break;
		case LightType::eSpot:
			{
				auto & spotLight = *light.getSpotLight();
				data.lightView = spotLight.getViewMatrix();
				auto lightFov = spotLight.getCutOff(); //in degrees, one must convert to radians
				data.config = castor::Point4f{ indirectAttenuation, ( lightFov * 0.5 ).tan(), texelAreaModifier, 0.0f };
			}
			break;
		}

		m_ubo->upload();
	}
}
