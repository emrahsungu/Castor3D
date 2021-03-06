#if defined( CU_CompilerMSVC )
#	pragma warning( disable:4503 )
#endif

#include "Castor3D/Render/RenderQueue.hpp"

#include "Castor3D/Engine.hpp"
#include "Castor3D/Render/RenderDevice.hpp"
#include "Castor3D/Render/RenderPass.hpp"
#include "Castor3D/Render/Culling/SceneCuller.hpp"
#include "Castor3D/Render/Node/SceneCulledRenderNodes.hpp"

using namespace castor;

using ashes::operator==;
using ashes::operator!=;

namespace castor3d
{
	RenderQueue::RenderQueue( RenderPass & renderPass
		, bool opaque
		, SceneNode const * ignored )
		: OwnedBy< RenderPass >{ renderPass }
		, m_opaque{ opaque }
		, m_ignoredNode{ ignored }
		, m_culler{ getOwner()->getCuller() }
		, m_onCullerCompute( m_culler.onCompute.connect( [this]( SceneCuller const & culler )
			{
				doOnCullerCompute( culler );
			} ) )
		, m_renderNodes{ new SceneRenderNodes( m_culler.getScene() ) }
		, m_culledRenderNodes{ new SceneCulledRenderNodes( m_culler.getScene() ) }
		, m_viewport{ castor::makeChangeTracked< ashes::Optional< VkViewport > >( ashes::nullopt ) }
		, m_scissor{ castor::makeChangeTracked< ashes::Optional< VkRect2D > >( ashes::nullopt ) }
	{
		getOwner()->getEngine()->sendEvent( makeFunctorEvent( EventType::ePreRender
			, [this]()
			{
				auto & device = getCurrentRenderDevice( *getOwner()->getEngine() );
				m_commandBuffer = device.graphicsCommandPool->createCommandBuffer( "RenderQueue", VK_COMMAND_BUFFER_LEVEL_SECONDARY );
			} ) );
	}

	void RenderQueue::cleanup()
	{
		m_culledRenderNodes.reset();
		m_renderNodes.reset();
		m_commandBuffer.reset();
	}

	void RenderQueue::update( ShadowMapLightTypeArray & shadowMaps )
	{
		if ( m_allChanged )
		{
			doParseAllRenderNodes( shadowMaps );
			m_allChanged = false;
		}

		bool commandBuffersChanged = m_culledChanged
			|| m_viewport.isDirty()
			|| m_scissor.isDirty();

		if ( m_culledChanged )
		{
			doParseCulledRenderNodes();
			m_culledChanged = false;
		}

		if ( commandBuffersChanged )
		{
			// Force regeneration of CommandBuffers if running.
			m_preparation = ( m_preparation == Preparation::eWaiting )
				? Preparation::eWaiting
				: Preparation::eDone;
		}

		if ( commandBuffersChanged
			&& m_preparation == Preparation::eDone )
		{
			m_preparation = Preparation::eWaiting;
			getOwner()->getEngine()->sendEvent( makeFunctorEvent( EventType::ePreRender
				, [this]()
				{
					m_preparation = Preparation::eRunning;
					m_commandBuffer->reset();
					doPrepareCommandBuffer();
					m_preparation = ( m_preparation == Preparation::eWaiting )
						? Preparation::eWaiting
						: Preparation::eDone;
				} ) );
		}
	}

	void RenderQueue::update( ShadowMapLightTypeArray & shadowMaps
		, VkViewport const & viewport
		, VkRect2D const & scissor )
	{
		m_viewport = viewport;
		m_scissor = scissor;
		update( shadowMaps );
	}

	void RenderQueue::update( ShadowMapLightTypeArray & shadowMaps
		, VkRect2D const & scissor )
	{
		m_scissor = scissor;
		update( shadowMaps );
	}

	bool RenderQueue::hasNodes()const
	{
		return ( m_renderNodes
			? m_renderNodes->hasNodes()
			: false );
	}

	void RenderQueue::doPrepareCommandBuffer()
	{
		auto & culledNodes = getCulledRenderNodes();
		culledNodes.prepareCommandBuffers( *this
			, m_viewport.value()
			, m_scissor.value() );
		m_viewport.reset();
		m_scissor.reset();
	}

	void RenderQueue::doParseAllRenderNodes( ShadowMapLightTypeArray & shadowMaps )
	{
		auto & allNodes = getAllRenderNodes();
		allNodes.parse( *this, shadowMaps );
	}

	void RenderQueue::doParseCulledRenderNodes()
	{
		auto & culledNodes = getCulledRenderNodes();
		culledNodes.parse( *this );
	}

	void RenderQueue::doOnCullerCompute( SceneCuller const & culler )
	{
		m_allChanged = culler.areAllChanged();
		m_culledChanged = m_allChanged || culler.areCulledChanged();
	}
}
