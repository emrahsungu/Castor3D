#include "Castor3D/Render/Technique/Voxelize/Voxelizer.hpp"

#include "Castor3D/Engine.hpp"
#include "Castor3D/Material/Texture/Sampler.hpp"
#include "Castor3D/Material/Texture/TextureLayout.hpp"
#include "Castor3D/Render/Technique/RenderTechniqueVisitor.hpp"
#include "Castor3D/Scene/Camera.hpp"
#include "Castor3D/Scene/Scene.hpp"

#include <ashespp/RenderPass/FrameBuffer.hpp>

using namespace castor;

namespace castor3d
{
	//*********************************************************************************************

	namespace
	{
		TextureUnit createTexture( Engine & engine
			, String const & name
			, VkExtent3D const & size )
		{
			auto & renderSystem = *engine.getRenderSystem();
			ashes::ImageCreateInfo image
			{
				VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT,
				VK_IMAGE_TYPE_3D,
				VK_FORMAT_R8_UNORM,
				size,
				1u,
				1u,
				VK_SAMPLE_COUNT_1_BIT,
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			};
			auto layout = std::make_shared< TextureLayout >( renderSystem
				, std::move( image )
				, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
				, name );
			layout->initialise();
			TextureUnit result{ engine };
			result.setSampler( createSampler( engine, name, VK_FILTER_NEAREST, nullptr ) );
			result.setTexture( layout );
			return result;
		}
	}

	//*********************************************************************************************

	Voxelizer::Voxelizer( Engine & engine
		, VkExtent3D const & resultDimensions
		, Scene & scene
		, SceneCuller & culler
		, ashes::ImageView colourView )
		: m_engine{ engine }
		, m_matrixUbo{ engine }
		, m_size{ resultDimensions }
		, m_result{ createTexture( engine, "VoxelizerResult", resultDimensions ) }
		, m_voxelizePass{ engine
			, m_matrixUbo
			, culler
			, m_result.getTexture()
			, colourView }
	{
		m_voxelizePass.initialise( { resultDimensions.width, resultDimensions.height } );
	}

	Voxelizer::~Voxelizer()
	{
	}

	void Voxelizer::update( RenderQueueArray & queues )
	{
		m_voxelizePass.update( queues );
	}

	void Voxelizer::update( RenderInfo & info
		, castor::Point2f const & jitter )
	{
		m_voxelizePass.update( jitter, info );
	}

	ashes::Semaphore const & Voxelizer::render( ashes::Semaphore const & toWait )
	{
		ashes::Semaphore const * result = &toWait;
		result = &m_voxelizePass.render( *result );
		return *result;
	}

	void Voxelizer::accept( RenderTechniqueVisitor & visitor )
	{
		m_voxelizePass.accept( visitor );
	}
}
