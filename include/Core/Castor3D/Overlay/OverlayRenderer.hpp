/*
See LICENSE file in root folder
*/
#ifndef ___C3D_OverlayRenderer_H___
#define ___C3D_OverlayRenderer_H___

#include "Castor3D/Overlay/TextOverlay.hpp"

#include "Castor3D/Buffer/GeometryBuffers.hpp"
#include "Castor3D/Buffer/UniformBuffer.hpp"
#include "Castor3D/Render/Node/PassRenderNode.hpp"
#include "Castor3D/Render/RenderPassTimer.hpp"
#include "Castor3D/Shader/Ubos/MatrixUbo.hpp"
#include "Castor3D/Shader/Ubos/OverlayUbo.hpp"
#include "Castor3D/Shader/Ubos/TexturesUbo.hpp"

#include <CastorUtils/Graphics/Size.hpp>

#include <ashespp/Command/CommandBuffer.hpp>
#include <ashespp/Descriptor/DescriptorSetLayout.hpp>
#include <ashespp/Descriptor/DescriptorSetPool.hpp>
#include <ashespp/Image/ImageView.hpp>
#include <ashespp/Pipeline/GraphicsPipeline.hpp>
#include <ashespp/Pipeline/PipelineLayout.hpp>
#include <ashespp/Pipeline/PipelineVertexInputStateCreateInfo.hpp>
#include <ashespp/Sync/Semaphore.hpp>

namespace castor3d
{
	class OverlayRenderer
		: public castor::OwnedBy< RenderSystem >
	{
	private:
		class Preparer
			: public OverlayVisitor
		{
		public:
			explicit Preparer( OverlayRenderer & renderer );

			void visit( PanelOverlay const & overlay )override;
			void visit( BorderPanelOverlay const & overlay )override;
			void visit( TextOverlay const & overlay )override;

		private:
			template< typename QuadT, typename OverlayT, typename BufferIndexT, typename BufferPoolT, typename VertexT >
			void doPrepareOverlay( OverlayT const & overlay
				, Pass const & pass
				, std::map< size_t, BufferIndexT > & overlays
				, std::vector< BufferPoolT > & vertexBuffers
				, std::vector < VertexT > const & vertices
				, FontTextureSPtr fontTexture );

		private:
			OverlayRenderer & m_renderer;
		};

	public:
		/**
		 *\~english
		 *\brief		Constructor.
		 *\param[in]	renderSystem	The RenderSystem.
		 *\param[in]	target			The target texture view.
		 *\~french
		 *\brief		Constructeur.
		 *\param[in]	renderSystem	Le RenderSystem.
		 *\param[in]	target			La vue de texture cible.
		 */
		C3D_API OverlayRenderer( RenderSystem & renderSystem
			, ashes::ImageView const & target );
		/**
		 *\~english
		 *\brief		Destructor.
		 *\~french
		 *\brief		Destructeur.
		 */
		C3D_API ~OverlayRenderer();
		/**
		 *\~english
		 *\brief		Initialises the buffers.
		 *\~french
		 *\brief		Initialise les tampons.
		 */
		C3D_API void initialise();
		/**
		*\~english
		*\brief		Flushes the renderer.
		*\~french
		*\brief		Nettoie le renderer.
		*/
		C3D_API void cleanup();
		/**
		 *\~english
		 *\brief		Updates the GPU data.
		 *\param[in]	camera	The render window camera.
		 *\~french
		 *\brief		Met à jour les données GPU.
		 *\param[in]	camera	La caméra de la fenêtre de rendu.
		 */
		C3D_API void update( Camera const & camera );
		/**
		 *\~english
		 *\brief		Begins the overlays preparation.
		 *\param[in]	timer	The render pass timer.
		 *\param[in]	toWait	The semaphore from the previous render pass.
		 *\~french
		 *\brief		Commence la préparation des incrustations.
		 *\param[in]	timer	Le timer de la passe de rendu.
		 *\param[in]	toWait	Le sémaphore de la passe de rendu précédente.
		 */
		C3D_API void beginPrepare( RenderPassTimer const & timer
			, ashes::Semaphore const & toWait );
		/**
		 *\~english
		 *\brief		Ends the overlays preparation.
		 *\param[in]	timer	The render pass timer.
		 *\~french
		 *\brief		Termine la préparation des incrustations.
		 *\param[in]	timer	Le timer de la passe de rendu.
		 */
		C3D_API void endPrepare( RenderPassTimer const & timer );
		/**
		 *\~english
		 *\brief		Ends the overlays preparation.
		 *\param[in]	timer	The render pass timer.
		 *\~french
		 *\brief		Termine la préparation des incrustations.
		 *\param[in]	timer	Le timer de la passe de rendu.
		 */
		C3D_API void render( RenderPassTimer & timer );
		/**
		*\~english
		*name
		*	Getters.
		*\~french
		*name
		*	Accesseurs.
		**/
		/**@{*/
		inline ashes::CommandBuffer const & getCommands()const
		{
			CU_Require( m_commandBuffer );
			return *m_commandBuffer;
		}

		castor::Size const & getSize()const
		{
			return m_size;
		}

		bool isSizeChanged()const
		{
			return m_sizeChanged;
		}

		Preparer getPreparer()
		{
			return Preparer{ *this };
		}

		ashes::Semaphore const & getSemaphore()
		{
			return *m_finished;
		}
		/**@}*/

	private:
		struct Configuration
		{
			castor::Point4f positionRatio;
			castor::Point4i renderSizeIndex;
		};

		struct Pipeline
		{
			ashes::DescriptorSetLayoutPtr descriptorLayout;
			ashes::DescriptorSetPoolPtr descriptorPool;
			ashes::PipelineLayoutPtr pipelineLayout;
			ashes::GraphicsPipelinePtr pipeline;
		};

	public:
		struct OverlayRenderNode
		{
			Pipeline & pipeline;
			Pass const & pass;
		};

	private:
		struct OverlayGeometryBuffers
		{
			GeometryBuffers noTexture;
			GeometryBuffers textured;
		};

		template< typename VertexT, uint32_t CountT >
		struct VertexBufferIndex;

		template< typename VertexT, uint32_t CountT >
		struct VertexBufferPool
		{
			using MyBufferIndex = VertexBufferIndex< VertexT, CountT >;
			using Quad = std::array< VertexT, CountT >;

			VertexBufferPool( RenderDevice const & device
				, ashes::PipelineVertexInputStateCreateInfo const & declaration
				, uint32_t count );
			VertexBufferIndex< VertexT, CountT > allocate( OverlayRenderNode & node );
			void deallocate( VertexBufferIndex< VertexT, CountT > const & index );
			void upload();

			uint32_t const maxCount;
			std::vector< Quad > data;
			ashes::PipelineVertexInputStateCreateInfo const & declaration;
			ashes::VertexBufferPtr< Quad > buffer;
			std::set< uint32_t > free;
			UniformBufferUPtr< Configuration > overlayUbo;
			UniformBufferUPtr< TexturesUbo::Configuration > texturesUbo;
		};

		template< typename VertexT, uint32_t CountT >
		struct VertexBufferIndex
		{
			operator bool()const
			{
				return index != InvalidIndex;
			}

			VertexBufferPool< VertexT, CountT > & pool;
			OverlayRenderNode & node;
			uint32_t index;
			OverlayGeometryBuffers geometryBuffers;
			ashes::DescriptorSetPtr descriptorSet;
			FontTexture::OnChanged::connection connection;
		};

		using PanelVertexBufferPool = VertexBufferPool< OverlayCategory::Vertex, 6u >;
		using PanelVertexBufferIndex = PanelVertexBufferPool::MyBufferIndex;
		using BorderPanelVertexBufferPool = VertexBufferPool< OverlayCategory::Vertex, 8u * 6u >;
		using BorderPanelVertexBufferIndex = BorderPanelVertexBufferPool::MyBufferIndex;
		using TextVertexBufferPool = VertexBufferPool< TextOverlay::Vertex, 600u >;
		using TextVertexBufferIndex = TextVertexBufferPool::MyBufferIndex;

		OverlayRenderNode & doGetPanelNode( Pass const & pass );
		OverlayRenderNode & doGetTextNode( Pass const & pass
			, TextureLayout const & texture
			, Sampler const & sampler );
		Pipeline doCreatePipeline( Pass const & pass
			, ashes::PipelineShaderStageCreateInfoArray program
			, bool text );
		Pipeline & doGetPipeline( Pass const & pass
			, std::map< uint32_t, Pipeline > & pipelines
			, bool text );
		ashes::PipelineShaderStageCreateInfoArray doCreateOverlayProgram( TextureFlagsArray const & textures
			, bool text );
		ashes::DescriptorSetPtr doCreateDescriptorSet( OverlayRenderer::Pipeline & pipeline
			, TextureFlags textures
			, Pass const & pass
			, UniformBuffer< Configuration > const & overlayUbo
			, UniformBuffer< TexturesUbo::Configuration > const & texturesUbo
			, uint32_t index
			, bool update = true );
		ashes::DescriptorSetPtr doCreateDescriptorSet( OverlayRenderer::Pipeline & pipeline
			, TextureFlags textures
			, Pass const & pass
			, UniformBuffer< Configuration > const & overlayUbo
			, UniformBuffer< TexturesUbo::Configuration > const & texturesUbo
			, uint32_t index
			, TextureLayout const & texture
			, Sampler const & sampler );
		void doCreateRenderPass();

	private:
		ashes::ImageView const & m_target;
		ashes::CommandBufferPtr m_commandBuffer;
		std::vector< std::unique_ptr< PanelVertexBufferPool > > m_panelVertexBuffers;
		std::vector< std::unique_ptr< BorderPanelVertexBufferPool > > m_borderVertexBuffers;
		std::vector< std::unique_ptr< TextVertexBufferPool > > m_textVertexBuffers;
		std::map< size_t, PanelVertexBufferIndex > m_panelOverlays;
		std::map< size_t, BorderPanelVertexBufferIndex > m_borderPanelOverlays;
		std::map< size_t, TextVertexBufferIndex > m_textOverlays;
		ashes::PipelineVertexInputStateCreateInfo m_declaration;
		ashes::PipelineVertexInputStateCreateInfo m_textDeclaration;
		castor::Size m_size;
		std::map< Pass const *, OverlayRenderNode > m_mapPanelNodes;
		std::map< Pass const *, OverlayRenderNode > m_mapTextNodes;
		ashes::RenderPassPtr m_renderPass;
		ashes::FrameBufferPtr m_frameBuffer;
		std::map< uint32_t, Pipeline > m_panelPipelines;
		std::map< uint32_t, Pipeline > m_textPipelines;
		int m_previousBorderZIndex{ 0 };
		int m_previousPanelZIndex{ 0 };
		int m_previousTextZIndex{ 0 };
		castor::String m_previousCaption;
		bool m_sizeChanged{ true };
		MatrixUbo m_matrixUbo;
		ashes::Semaphore const * m_toWait{ nullptr };
		ashes::SemaphorePtr m_finished;
	};
}

#endif
