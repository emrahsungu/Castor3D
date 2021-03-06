#include "Castor3D/Render/Passes/DownscalePass.hpp"

#include "Castor3D/Engine.hpp"
#include "Castor3D/Cache/SamplerCache.hpp"
#include "Castor3D/Material/Texture/Sampler.hpp"
#include "Castor3D/Material/Texture/TextureLayout.hpp"
#include "Castor3D/Miscellaneous/PipelineVisitor.hpp"
#include "Castor3D/Render/RenderPassTimer.hpp"

#include <CastorUtils/Graphics/RgbaColour.hpp>

#include <ashespp/Image/ImageView.hpp>

using namespace castor;

namespace castor3d
{
	//*********************************************************************************************

	SamplerSPtr doCreateSampler( Engine & engine
		, String const & name
		, VkSamplerAddressMode mode )
	{
		SamplerSPtr sampler;

		if ( engine.getSamplerCache().has( name ) )
		{
			sampler = engine.getSamplerCache().find( name );
		}
		else
		{
			sampler = engine.getSamplerCache().add( name );
			sampler->setMinFilter( VK_FILTER_LINEAR );
			sampler->setMagFilter( VK_FILTER_LINEAR );
			sampler->setWrapS( mode );
			sampler->setWrapT( mode );
		}

		return sampler;
	}

	TextureUnit doCreateTexture( Engine & engine
		, castor::String const & name
		, VkFormat format
		, VkExtent2D const & size )
	{
		auto & renderSystem = *engine.getRenderSystem();
		auto sampler = doCreateSampler( engine, name, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE );

		ashes::ImageCreateInfo image
		{
			0u,
			VK_IMAGE_TYPE_2D,
			format,
			{ size.width, size.height, 1u },
			1u,
			1u,
			VK_SAMPLE_COUNT_1_BIT,
			VK_IMAGE_TILING_OPTIMAL,
			( VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
			| VK_IMAGE_USAGE_SAMPLED_BIT ),
		};
		auto layout = std::make_shared< TextureLayout >( renderSystem
			, image
			, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
			, name );
		TextureUnit result{ engine };
		result.setTexture( layout );
		result.setSampler( sampler );
		result.initialise();
		return result;
	}

	TextureUnitArray doCreateTextures( Engine & engine
		, castor::String const & name
		, ashes::ImageViewArray const & views
		, VkExtent2D const & size )
	{
		TextureUnitArray result;

		for ( auto & view : views )
		{
			result.emplace_back( doCreateTexture( engine
				, name + string::toString( result.size() )
				, view->format
				, size ) );
		}

		return result;
	}

	//*********************************************************************************************

	DownscalePass::DownscalePass( Engine & engine
		, castor::String const & category
		, ashes::ImageViewArray const & srcViews
		, VkExtent2D const & dstSize )
		: m_engine{ engine }
		, m_result{ doCreateTextures( engine, "Downscaled", srcViews, dstSize ) }
		, m_timer{ std::make_shared< RenderPassTimer >( engine, category, cuT( "Downscale" ) ) }
		, m_commandBuffer{ getCurrentRenderDevice( engine ).graphicsCommandPool->createCommandBuffer( "Downscale", VK_COMMAND_BUFFER_LEVEL_PRIMARY ) }
		, m_finished{ getCurrentRenderDevice( engine )->createSemaphore( "Downscale" ) }
	{
		m_commandBuffer->begin( VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT );
		m_timer->beginPass( *m_commandBuffer, 0u );
		m_commandBuffer->beginDebugBlock(
			{
				"Downscale",
				makeFloatArray( m_engine.getNextRainbowColour() ),
			} );
		auto it = m_result.begin();

		for ( auto & srcView : srcViews )
		{
			auto srcSize = srcView.image->getDimensions();
			auto dstView = it->getTexture()->getDefaultView().getTargetView();

			m_commandBuffer->memoryBarrier( VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
				, VK_PIPELINE_STAGE_TRANSFER_BIT
				, srcView.makeTransferSource( VK_IMAGE_LAYOUT_UNDEFINED ) );
			m_commandBuffer->memoryBarrier( VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
				, VK_PIPELINE_STAGE_TRANSFER_BIT
				, dstView.makeTransferDestination( VK_IMAGE_LAYOUT_UNDEFINED ) );
			m_commandBuffer->blitImage( *srcView.image
				, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
				, *dstView.image
				, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
				, {
					VkImageBlit
					{
						{
							srcView->subresourceRange.aspectMask,
							srcView->subresourceRange.baseMipLevel,
							srcView->subresourceRange.baseArrayLayer,
							srcView->subresourceRange.layerCount,
						},
						{
							{ 0, 0, 0 },
							{ int32_t( srcSize.width ), int32_t( srcSize.height ), 1 },
						},
						{
							dstView->subresourceRange.aspectMask,
							dstView->subresourceRange.baseMipLevel,
							dstView->subresourceRange.baseArrayLayer,
							dstView->subresourceRange.layerCount,
						},
						{
							{ 0, 0, 0 },
							{ int32_t( dstSize.width ), int32_t( dstSize.height ), 1 },
						},
					}
				}
				, VK_FILTER_NEAREST );
			m_commandBuffer->memoryBarrier( VK_PIPELINE_STAGE_TRANSFER_BIT
				, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
				, srcView.makeShaderInputResource( VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL ) );
			m_commandBuffer->memoryBarrier( VK_PIPELINE_STAGE_TRANSFER_BIT
				, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
				, dstView.makeShaderInputResource( VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL ) );
			++it;
		}

		m_commandBuffer->endDebugBlock();
		m_timer->endPass( *m_commandBuffer, 0u );
		m_commandBuffer->end();
	}

	ashes::Semaphore const & DownscalePass::compute( ashes::Semaphore const & toWait )
	{
		auto & device = getCurrentRenderDevice( m_engine );
		RenderPassTimerBlock timerBlock{ m_timer->start() };
		timerBlock->notifyPassRender();
		auto * result = &toWait;

		device.graphicsQueue->submit( *m_commandBuffer
			, *result
			, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
			, *m_finished
			, nullptr );
		result = m_finished.get();

		return *result;
	}

	void DownscalePass::accept( PipelineVisitorBase & visitor )
	{
		uint32_t index{};

		for ( auto & unit : m_result )
		{
			visitor.visit( "Downscale" + string::toString( index++ )
				, unit.getTexture()->getDefaultView().getSampledView() );
		}
	}

	//*********************************************************************************************
}
