#include "Castor3D/Render/Technique/RenderTechnique.hpp"

#include "Castor3D/Engine.hpp"
#include "Castor3D/Cache/AnimatedObjectGroupCache.hpp"
#include "Castor3D/Cache/BillboardUboPools.hpp"
#include "Castor3D/Cache/GeometryCache.hpp"
#include "Castor3D/Cache/LightCache.hpp"
#include "Castor3D/Cache/ParticleSystemCache.hpp"
#include "Castor3D/Material/Texture/TextureLayout.hpp"
#include "Castor3D/Render/RenderTarget.hpp"
#include "Castor3D/Render/Technique/Opaque/OpaquePass.hpp"
#include "Castor3D/Render/Technique/Transparent/TransparentPass.hpp"
#include "Castor3D/Scene/Camera.hpp"
#include "Castor3D/Scene/Scene.hpp"
#include "Castor3D/Scene/SceneNode.hpp"
#include "Castor3D/Scene/Background/Background.hpp"
#include "Castor3D/Scene/Light/Light.hpp"
#include "Castor3D/Scene/ParticleSystem/ParticleSystem.hpp"

using namespace castor;

#define C3D_UseWeightedBlendedRendering 1
#define C3D_UseDeferredRendering 1

namespace castor3d
{
	//*************************************************************************************************

	namespace
	{
		class IntermediatesLister
			: public RenderTechniqueVisitor
		{
		public:
			static void submit( RenderTechnique & technique
				, std::vector< IntermediateView > & intermediates )
			{
				PipelineFlags flags{};
				IntermediatesLister visOpaque{ flags, *technique.getRenderTarget().getScene(), intermediates };
				technique.accept( visOpaque );

				flags.passFlags |= PassFlag::eAlphaBlending;
				IntermediatesLister visTransparent{ flags, *technique.getRenderTarget().getScene(), intermediates };
				technique.accept( visTransparent );
			}

			void visit( castor::String const & name
				, ashes::ImageView const & view
				, TextureFactors const & factors )override
			{
				m_result.push_back( { name, view, factors } );
			}

		private:
			IntermediatesLister( PipelineFlags const & flags
				, Scene const & scene
				, std::vector< IntermediateView > & result )
				: RenderTechniqueVisitor{ flags, scene, true }
				, m_result{ result }
			{
			}

		private:
			std::vector< IntermediateView > & m_result;
		};

		std::map< double, LightSPtr > doSortLights( LightCache const & cache
			, LightType type
			, Camera const & camera )
		{
			using LockType = std::unique_lock< LightCache const >;
			LockType lock{ castor::makeUniqueLock( cache ) };
			std::map< double, LightSPtr > lights;

			for ( auto & light : cache.getLights( type ) )
			{
				light->setShadowMap( nullptr );

				if ( light->isShadowProducer()
					&& ( light->getLightType() == LightType::eDirectional
						|| camera.isVisible( light->getBoundingBox()
							, light->getParent()->getDerivedTransformationMatrix() ) ) )
				{
					lights.emplace( point::distanceSquared( camera.getParent()->getDerivedPosition()
						, light->getParent()->getDerivedPosition() )
						, light );
				}
			}

			return lights;
		}

		void doPrepareShadowMap( LightCache const & cache
			, LightType type
			, Camera const & camera
			, ShadowMapUPtr & shadowMap
			, ShadowMapLightTypeArray & activeShadowMaps
			, RenderQueueArray & queues )
		{
			auto lights = doSortLights( cache, type, camera );
			size_t count = std::min( shadowMap->getCount(), uint32_t( lights.size() ) );

			if ( count > 0 )
			{
				uint32_t index = 0u;
				auto lightIt = lights.begin();
				activeShadowMaps[size_t( type )].emplace_back( std::ref( *shadowMap ), UInt32Array{} );
				auto & active = activeShadowMaps[size_t( type )].back();

				for ( auto i = 0u; i < count; ++i )
				{
					lightIt->second->setShadowMap( shadowMap.get(), index );
					active.second.push_back( index );
					shadowMap->update( camera
						, queues
						, *( lightIt->second )
						, index );
					++index;
					++lightIt;
				}
			}
		}

		TextureLayoutSPtr doCreateTexture( Engine & engine
			, Size const & size
			, VkFormat format
			, VkImageUsageFlags usage
			, castor::String debugName )
		{
			ashes::ImageCreateInfo image
			{
				0u,
				VK_IMAGE_TYPE_2D,
				format,
				makeExtent3D( size ),
				1u,
				1u,
				VK_SAMPLE_COUNT_1_BIT,
				VK_IMAGE_TILING_OPTIMAL,
				usage | VK_IMAGE_USAGE_SAMPLED_BIT,
			};
			auto result = std::make_shared< TextureLayout >( *engine.getRenderSystem()
				, image
				, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
				, std::move( debugName ) );
			result->initialise();
			return result;
		}
	}

	//*************************************************************************************************

	RenderTechnique::RenderTechnique( String const & name
		, RenderTarget & renderTarget
		, RenderSystem & renderSystem
		, Parameters const & parameters
		, SsaoConfig const & ssaoConfig )
		: OwnedBy< Engine >{ *renderSystem.getEngine() }
		, Named{ name }
		, m_renderTarget{ renderTarget }
		, m_renderSystem{ renderSystem }
		, m_matrixUbo{ *renderSystem.getEngine() }
		, m_hdrConfigUbo{ *renderSystem.getEngine() }
		, m_gpInfoUbo{ *renderSystem.getEngine() }
#if C3D_UseDeferredRendering
		, m_opaquePass{ std::make_unique< OpaquePass >( m_matrixUbo
			, renderTarget.getCuller()
			, ssaoConfig ) }
#else
		, m_opaquePass{ std::make_unique< ForwardRenderTechniquePass >( cuT( "opaque_pass" )
			, m_matrixUbo
			, renderTarget.getCuller()
			, false
			, nullptr
			, config ) }
#endif
#if C3D_UseWeightedBlendedRendering
		, m_transparentPass{ std::make_unique< TransparentPass >( m_matrixUbo
			, renderTarget.getCuller()
			, ssaoConfig ) }
#else
		, m_transparentPass{ std::make_unique< ForwardRenderTechniquePass >( cuT( "transparent_pass" )
			, m_matrixUbo
			, renderTarget.getCuller()
			, false
			, false
			, nullptr
			, config ) }
#endif
		, m_initialised{ false }
		, m_ssaoConfig{ ssaoConfig }
		, m_depthBuffer{ *renderSystem.getEngine() }
	{
		doCreateShadowMaps();
	}

	RenderTechnique::~RenderTechnique()
	{
		m_cbgCommandBuffer.reset();
		m_bgCommandBuffer.reset();
		m_bgFrameBuffer.reset();
		m_bgRenderPass.reset();
		m_signalFinished.reset();

		for ( auto & array : m_activeShadowMaps )
		{
			array.clear();
		}

		m_directionalShadowMap.reset();
		m_pointShadowMap.reset();
		m_spotShadowMap.reset();
		m_deferredRendering.reset();
		m_weightedBlendRendering.reset();
		m_transparentPass.reset();
		m_opaquePass.reset();
	}

	bool RenderTechnique::initialise( std::vector< IntermediateView > & intermediates )
	{
		if ( !m_initialised )
		{
			auto & device = getCurrentRenderDevice( *this );
			m_size = m_renderTarget.getSize();
			m_colourTexture = doCreateTexture( *getEngine()
				, m_size
				, VK_FORMAT_R16G16B16A16_SFLOAT
				, ( VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
					| VK_IMAGE_USAGE_TRANSFER_DST_BIT
					| VK_IMAGE_USAGE_TRANSFER_SRC_BIT )
				, cuT( "RenderTechnique_Colour" ) );

			auto depthBuffer = doCreateTexture( *getEngine()
				, m_size
				, device.selectSuitableDepthStencilFormat( VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
					| VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT
					| VK_FORMAT_FEATURE_TRANSFER_DST_BIT
					| VK_FORMAT_FEATURE_TRANSFER_SRC_BIT )
				, ( VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT )
				, cuT( "RenderTechnique_Depth" ) );
			m_depthBuffer.setTexture( depthBuffer );
			VkImageSubresourceRange range{ 0u, 0u, 1u, 0u, 1u };
			m_depthBuffer.setSampler( createSampler( *getEngine()
				, cuT( "RenderTechnique_Depth" )
				, VK_FILTER_LINEAR
				, &range ) );
			m_depthBuffer.initialise();
			m_signalFinished = device->createSemaphore( "RenderTechnique" );
			m_gpInfoUbo.initialise();
			m_hdrConfigUbo.initialise();
			m_matrixUbo.initialise();

			m_stagingBuffer = std::make_unique< ashes::StagingBuffer >( *device
				, VK_BUFFER_USAGE_TRANSFER_SRC_BIT
				, m_matrixUbo.getUbo().getAlignedSize() );
			m_uploadCommandBuffer = device.graphicsCommandPool->createCommandBuffer( "RenderTechniqueUpload" );

			//m_voxelizer = std::make_unique< Voxelizer >( *m_renderSystem.getEngine()
			//	, VkExtent3D{ 64u, 64u, 64u }
			//	, *m_renderTarget.getScene()
			//	, m_renderTarget.getCuller()
			//	, m_renderTarget.getTexture().getTexture()->getDefaultView().getTargetView() );

			doInitialiseShadowMaps();
#if C3D_UseDepthPrepass
			doInitialiseDepthPass();
#endif
			doInitialiseBackgroundPass();
			doInitialiseOpaquePass();
			doInitialiseTransparentPass();
			IntermediatesLister::submit( *this, intermediates );

			m_particleTimer = std::make_shared< RenderPassTimer >( *getEngine(), cuT( "Particles" ), cuT( "Particles" ) );
			m_initialised = true;
		}

		return m_initialised;
	}

	void RenderTechnique::cleanup()
	{
		m_onBgChanged.disconnect();
		m_bgCommandBuffer.reset();
		m_cbgCommandBuffer.reset();
		m_bgFrameBuffer.reset();
		m_bgRenderPass.reset();
		m_particleTimer.reset();
		m_weightedBlendRendering.reset();
		m_deferredRendering.reset();
		doCleanupShadowMaps();
		m_transparentPass->cleanup();
		m_opaquePass->cleanup();
		m_voxelizer.reset();
#if C3D_UseDepthPrepass
		m_depthPass->cleanup();
		m_depthPass.reset();
#endif
		m_gpInfoUbo.cleanup();
		m_hdrConfigUbo.cleanup();
		m_matrixUbo.cleanup();
		m_uploadCommandBuffer.reset();
		m_stagingBuffer.reset();
		m_initialised = false;
		m_depthBuffer.cleanup();

		if ( m_colourTexture )
		{
			m_colourTexture->cleanup();
			m_colourTexture.reset();
		}

		auto scene = m_renderTarget.getScene();

		if ( scene )
		{
			scene->getGeometryCache().cleanupUbos();
			scene->getBillboardPools().cleanupUbos();
			scene->getAnimatedObjectGroupCache().cleanupUbos();
		}
	}

	void RenderTechnique::update( RenderQueueArray & queues )
	{
#if C3D_UseDepthPrepass
		m_depthPass->update( queues );
#endif
		m_renderTarget.update();
		//m_voxelizer->update( queues );
		m_opaquePass->update( queues );
		m_transparentPass->update( queues );
		doUpdateShadowMaps( queues );
		auto & maps = m_renderTarget.getScene()->getEnvironmentMaps();

		for ( auto & map : maps )
		{
			map.get().update( queues );
		}
	}

	void RenderTechnique::update( castor::Point2f const & jitter
		, RenderInfo & info )
	{
		auto & scene = *m_renderTarget.getScene();
		auto & camera = *m_renderTarget.getCamera();
		auto jitterProjSpace = jitter * 2.0f;
		jitterProjSpace[0] /= camera.getWidth();
		jitterProjSpace[1] /= camera.getHeight();
		m_matrixUbo.update( camera.getView()
			, camera.getProjection()
			, jitterProjSpace );
		m_gpInfoUbo.update( m_size
			, camera );
		m_hdrConfigUbo.update( m_renderTarget.getHdrConfig() );

#if C3D_UseDepthPrepass
		m_depthPass->update( info, jitter );
#endif
		//m_voxelizer->update( info, jitter );
#if C3D_UseDeferredRendering
		m_deferredRendering->update( info, scene, camera, jitter );
#else
		static_cast< ForwardRenderTechniquePass & >( *m_opaquePass ).update( info, jitter );
#endif
#if C3D_UseWeightedBlendedRendering
		m_weightedBlendRendering->update( info, scene, camera, jitter );
#else
		static_cast< ForwardRenderTechniquePass & >( *m_transparentPass ).update( info, jitter );
#endif

		scene.getLightCache().updateLightsTexture( camera );
		scene.updateDeviceDependent( camera );

		for ( auto & map : m_renderTarget.getScene()->getEnvironmentMaps() )
		{
			map.get().update();
		}

		for ( auto & maps : m_activeShadowMaps )
		{
			for ( auto & map : maps )
			{
				for ( auto & id : map.second )
				{
					map.first.get().updateDeviceDependent( id );
				}
			}
		}
	}

	ashes::Semaphore const & RenderTechnique::render( ashes::SemaphoreCRefArray const & waitSemaphores
		, RenderInfo & info )
	{
		doUpdateParticles( info );
#if C3D_UseDepthPrepass
		auto * semaphore = &doRenderDepth( waitSemaphores );
		semaphore = &doRenderBackground( *semaphore );
#else
		auto * semaphore = &doRenderBackground( waitSemaphores );
#endif
		semaphore = &doRenderEnvironmentMaps( *semaphore );
		semaphore = &doRenderShadowMaps( *semaphore );

		// Render part
		//semaphore = &m_voxelizer->render( *semaphore );
		semaphore = &doRenderOpaque( *semaphore );
		semaphore = &doRenderTransparent( *semaphore );
		return *semaphore;
	}

	bool RenderTechnique::writeInto( castor::TextFile & file )
	{
		return true;
	}

	void RenderTechnique::accept( RenderTechniqueVisitor & visitor )
	{
		visitor.visit( "Technique Colour", m_colourTexture->getDefaultView().getSampledView() );
		visitor.visit( "Technique Depth", m_depthBuffer.getTexture()->getDefaultView().getSampledView() );

		if ( checkFlag( visitor.getFlags().passFlags, PassFlag::eAlphaBlending ) )
		{
#if C3D_UseWeightedBlendedRendering
			m_weightedBlendRendering->accept( visitor );
#else
			m_transparentPass->accept( visitor );
#endif
		}
		else
		{
#if C3D_UseDeferredRendering
			m_deferredRendering->accept( visitor );
#else
			m_opaquePass->accept( visitor );
#endif
		}

		m_directionalShadowMap->accept( visitor );
		m_spotShadowMap->accept( visitor );
		m_pointShadowMap->accept( visitor );
	}

	void RenderTechnique::doCreateShadowMaps()
	{
		auto & scene = *m_renderTarget.getScene();
		auto & engine = *m_renderTarget.getEngine();
		m_directionalShadowMap = std::make_unique< ShadowMapDirectional >( engine
			, scene );
		m_allShadowMaps[size_t( LightType::eDirectional )].emplace_back( std::ref( *m_directionalShadowMap ), UInt32Array{} );
		m_spotShadowMap = std::make_unique< ShadowMapSpot >( engine
			, scene );
		m_allShadowMaps[size_t( LightType::eSpot )].emplace_back( std::ref( *m_spotShadowMap ), UInt32Array{} );
		m_pointShadowMap = std::make_unique< ShadowMapPoint >( engine
			, scene );
		m_allShadowMaps[size_t( LightType::ePoint )].emplace_back( std::ref( *m_pointShadowMap ), UInt32Array{} );
	}

	void RenderTechnique::doInitialiseShadowMaps()
	{
		m_directionalShadowMap->initialise();
		m_spotShadowMap->initialise();
		m_pointShadowMap->initialise();
	}

	void RenderTechnique::doInitialiseBackgroundPass()
	{
		auto & renderSystem = *getEngine()->getRenderSystem();
		auto & device = getCurrentRenderDevice( renderSystem );
		m_bgCommandBuffer = device.graphicsCommandPool->createCommandBuffer( "Background" );
		m_cbgCommandBuffer = device.graphicsCommandPool->createCommandBuffer( "ColourBackground" );
		auto depthLoadOp = C3D_UseDepthPrepass
			? VK_ATTACHMENT_LOAD_OP_LOAD
			: VK_ATTACHMENT_LOAD_OP_CLEAR;
		auto depthInitialLayout = C3D_UseDepthPrepass
			? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
			: VK_IMAGE_LAYOUT_UNDEFINED;

		ashes::VkAttachmentDescriptionArray attachments
		{
			{
				0u,
				m_depthBuffer.getTexture()->getPixelFormat(),
				VK_SAMPLE_COUNT_1_BIT,
				depthLoadOp,
				VK_ATTACHMENT_STORE_OP_STORE,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				VK_ATTACHMENT_STORE_OP_DONT_CARE,
				depthInitialLayout,
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			},
			{
				0u,
				m_colourTexture->getPixelFormat(),
				VK_SAMPLE_COUNT_1_BIT,
				VK_ATTACHMENT_LOAD_OP_CLEAR,
				VK_ATTACHMENT_STORE_OP_STORE,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				VK_ATTACHMENT_STORE_OP_DONT_CARE,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			},
		};
		ashes::SubpassDescriptionArray subpasses;
		subpasses.emplace_back( ashes::SubpassDescription
			{
				0u,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				{},
				{ { 1u, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL } },
				{},
				VkAttachmentReference{ 0u, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL },
				{},
			} );
		ashes::VkSubpassDependencyArray dependencies
		{
			{
				VK_SUBPASS_EXTERNAL,
				0u,
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_ACCESS_MEMORY_READ_BIT,
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
				VK_DEPENDENCY_BY_REGION_BIT,
			},
			{
				0u,
				VK_SUBPASS_EXTERNAL,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
				VK_ACCESS_MEMORY_READ_BIT,
				VK_DEPENDENCY_BY_REGION_BIT,
			},
		};
		ashes::RenderPassCreateInfo createInfo
		{
			0u,
			std::move( attachments ),
			std::move( subpasses ),
			std::move( dependencies ),
		};
		m_bgRenderPass = device->createRenderPass( "Background"
			, std::move( createInfo ) );

		ashes::ImageViewCRefArray attaches
		{
			m_depthBuffer.getTexture()->getDefaultView().getTargetView(),
			m_colourTexture->getDefaultView().getTargetView(),
		};
		m_bgFrameBuffer = m_bgRenderPass->createFrameBuffer( "Background"
			, { m_depthBuffer.getTexture()->getWidth(), m_depthBuffer.getTexture()->getHeight() }
			, std::move( attaches ) );

		auto & background = *m_renderTarget.getScene()->getBackground();
		auto prepareBackground = [&background, this]()
		{
			background.initialise( *m_bgRenderPass, m_hdrConfigUbo );
			background.prepareFrame( *m_bgCommandBuffer
				, Size{ m_colourTexture->getWidth(), m_colourTexture->getHeight() }
				, *m_bgRenderPass
				, *m_bgFrameBuffer );
		};
		prepareBackground();
		m_onBgChanged = background.onChanged.connect( [prepareBackground, this]( SceneBackground const & )
			{
				getEngine()->postEvent( makeFunctorEvent( EventType::ePreRender, prepareBackground ) );
			} );
		auto & cbackground = m_renderTarget.getScene()->getColourBackground();
		auto prepareCBackground = [&cbackground, this]()
		{
			cbackground.initialise( *m_bgRenderPass, m_hdrConfigUbo );
			cbackground.prepareFrame( *m_cbgCommandBuffer
				, Size{ m_colourTexture->getWidth(), m_colourTexture->getHeight() }
				, *m_bgRenderPass
				, *m_bgFrameBuffer );
		};
		prepareCBackground();
		m_onCBgChanged = cbackground.onChanged.connect( [prepareCBackground, this]( SceneBackground const & )
			{
				getEngine()->postEvent( makeFunctorEvent( EventType::ePreRender, prepareCBackground ) );
			} );
	}

#if C3D_UseDepthPrepass

	void RenderTechnique::doInitialiseDepthPass()
	{
		m_depthPass = std::make_unique< DepthPass >( cuT( "Depth Prepass" )
			, m_matrixUbo
			, m_renderTarget.getCuller()
			, m_ssaoConfig
			, m_depthBuffer.getTexture() );
		m_depthPass->initialise( m_size );
	}

#endif

	void RenderTechnique::doInitialiseOpaquePass()
	{
#if C3D_UseDeferredRendering

		m_opaquePass->initialise( m_size );
		m_deferredRendering = std::make_unique< DeferredRendering >( *getEngine()
			, static_cast< OpaquePass & >( *m_opaquePass )
			, m_depthBuffer
			, m_renderTarget.getVelocity()
			, m_colourTexture
			, m_directionalShadowMap->getShadowPassResult()
			, m_pointShadowMap->getShadowPassResult()
			, m_spotShadowMap->getShadowPassResult()
			, m_renderTarget.getSize()
			, *m_renderTarget.getScene()
			, m_hdrConfigUbo
			, m_gpInfoUbo
			, m_ssaoConfig );

#else

		m_opaquePass->initialise( m_size );
		static_cast< ForwardRenderTechniquePass & >( *m_opaquePass ).initialiseRenderPass( m_colourTexture->getDefaultView().getView()
			, m_depthBuffer.getTexture()->getDefaultView().getView()
			, m_size
			, false );

#endif
	}

	void RenderTechnique::doInitialiseTransparentPass()
	{
#if C3D_UseWeightedBlendedRendering

		m_transparentPass->initialise( m_size );
		m_weightedBlendRendering = std::make_unique< WeightedBlendRendering >( *getEngine()
			, static_cast< TransparentPass & >( *m_transparentPass )
			, m_depthBuffer
			, m_colourTexture->getDefaultView().getTargetView()
			, m_renderTarget.getVelocity()
			, m_renderTarget.getSize()
			, *m_renderTarget.getScene()
			, m_hdrConfigUbo
			, m_gpInfoUbo );

#else

		m_transparentPass->initialise( m_size );
		static_cast< ForwardRenderTechniquePass & >( *m_transparentPass ).initialiseRenderPass( m_colourTexture->getDefaultView().getView()
			, m_depthBuffer.getTexture()->getDefaultView().getView()
			, m_size
			, false );

#endif
	}

	void RenderTechnique::doCleanupShadowMaps()
	{
		m_directionalShadowMap->cleanup();
		m_spotShadowMap->cleanup();
		m_pointShadowMap->cleanup();
	}

	void RenderTechnique::doUpdateShadowMaps( RenderQueueArray & queues )
	{
		auto & scene = *m_renderTarget.getScene();

		if ( scene.hasShadows() )
		{
			for ( auto & array : m_activeShadowMaps )
			{
				array.clear();
			}

			auto & camera = *m_renderTarget.getCamera();
			auto & cache = scene.getLightCache();
			doPrepareShadowMap( cache
				, LightType::eDirectional
				, camera
				, m_directionalShadowMap
				, m_activeShadowMaps
				, queues );
			doPrepareShadowMap( cache
				, LightType::ePoint
				, camera
				, m_pointShadowMap
				, m_activeShadowMaps
				, queues );
			doPrepareShadowMap( cache
				, LightType::eSpot
				, camera
				, m_spotShadowMap
				, m_activeShadowMaps
				, queues );
		}
	}
	
	void RenderTechnique::doUpdateParticles( RenderInfo & info )
	{
		auto & scene = *m_renderTarget.getScene();
		auto & cache = scene.getParticleSystemCache();
		auto lock( castor::makeUniqueLock( cache ) );

		if ( !cache.isEmpty() )
		{
			auto count = 2u * cache.getObjectCount();

			if ( m_particleTimer->getCount() < count )
			{
				m_particleTimer->updateCount( count );
			}

			RenderPassTimerBlock timerBlock{ m_particleTimer->start() };
			uint32_t index = 0u;

			for ( auto & particleSystem : cache )
			{
				particleSystem.second->update( *m_particleTimer, index );
				info.m_particlesCount += particleSystem.second->getParticlesCount();
			}
		}
	}

	ashes::Semaphore const & RenderTechnique::doRenderShadowMaps( ashes::Semaphore const & semaphore )
	{
		ashes::Semaphore const * result = &semaphore;
		auto & scene = *m_renderTarget.getScene();

		if ( scene.hasShadows() )
		{
			for ( auto & array : m_activeShadowMaps )
			{
				for ( auto & shadowMap : array )
				{
					for ( auto & index : shadowMap.second )
					{
						result = &shadowMap.first.get().render( *result, index );
					}
				}
			}
		}

		return *result;
	}

	ashes::Semaphore const & RenderTechnique::doRenderEnvironmentMaps( ashes::Semaphore const & semaphore )
	{
		ashes::Semaphore const * result = &semaphore;

		for ( auto & map : m_renderTarget.getScene()->getEnvironmentMaps() )
		{
			result = &map.get().render( *result );
		}

		return *result;
	}

#if C3D_UseDepthPrepass

	ashes::Semaphore const & RenderTechnique::doRenderDepth( ashes::SemaphoreCRefArray const & semaphores )
	{
		return m_depthPass->render( semaphores );
	}

	ashes::Semaphore const & RenderTechnique::doRenderBackground( ashes::Semaphore const & semaphore )
	{
		return doRenderBackground( { 1u, std::ref( semaphore ) } );
	}

#endif

	ashes::Semaphore const & RenderTechnique::doRenderBackground( ashes::SemaphoreCRefArray const & semaphores )
	{
		auto const & queue = *getCurrentRenderDevice( *this ).graphicsQueue;
		ashes::VkPipelineStageFlagsArray const stages( semaphores.size(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT );

		if ( m_renderTarget.getScene()->getFog().getType() != FogType::eDisabled )
		{
			auto & background = m_renderTarget.getScene()->getColourBackground();
			auto & bgSemaphore = background.getSemaphore();
			auto timerBlock = background.start();
			timerBlock->notifyPassRender();
			queue.submit( { *m_cbgCommandBuffer }
				, semaphores
				, stages
				, { bgSemaphore }
				, nullptr );
			return bgSemaphore;
		}

		auto & background = *m_renderTarget.getScene()->getBackground();
		auto & bgSemaphore = background.getSemaphore();
		auto timerBlock = background.start();
		timerBlock->notifyPassRender();
		queue.submit( { *m_bgCommandBuffer }
			, semaphores
			, stages
			, { bgSemaphore }
			, nullptr );
		return bgSemaphore;
	}

	ashes::Semaphore const & RenderTechnique::doRenderOpaque( ashes::Semaphore const & semaphore )
	{
		ashes::Semaphore const * result = &semaphore;

		if ( m_opaquePass->hasNodes() )
		{
			auto & scene = *m_renderTarget.getScene();
			auto & camera = *m_renderTarget.getCamera();

#if C3D_UseDeferredRendering
			result = &m_deferredRendering->render( scene, camera, *result );
#else
			result = &static_cast< ForwardRenderTechniquePass & >( *m_opaquePass ).render( *result );
#endif
		}

		return *result;
	}

	ashes::Semaphore const & RenderTechnique::doRenderTransparent( ashes::Semaphore const & semaphore )
	{
		ashes::Semaphore const * result = &semaphore;

		if ( m_transparentPass->hasNodes() )
		{
			auto & scene = *m_renderTarget.getScene();

#if C3D_UseWeightedBlendedRendering
			result = &m_weightedBlendRendering->render( scene, *result );
#else
			result = &static_cast< ForwardRenderTechniquePass & >( *m_transparentPass ).render( *result );
#endif
		}

		return *result;
	}
}
