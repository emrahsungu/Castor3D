#include "Castor3D/Scene/Background/FramePreparator.hpp"

#include "Castor3D/Scene/Background/Colour.hpp"
#include "Castor3D/Scene/Background/Image.hpp"
#include "Castor3D/Scene/Background/Skybox.hpp"

#include <ashespp/Command/CommandBuffer.hpp>

namespace castor3d
{
	BackgroundFramePreparator::BackgroundFramePreparator( ashes::CommandBuffer & commandBuffer
		, ashes::RenderPass const & renderPass
		, ashes::FrameBuffer const & frameBuffer )
		: m_commandBuffer{ commandBuffer }
	{
	}

	BackgroundFramePreparator::~BackgroundFramePreparator()
	{
		m_commandBuffer.end();
	}

	void BackgroundFramePreparator::visit( ColourBackground const & background )
	{
	}

	void BackgroundFramePreparator::visit( SkyboxBackground const & background )
	{
	}

	void BackgroundFramePreparator::visit( ImageBackground const & background )
	{
	}
}
