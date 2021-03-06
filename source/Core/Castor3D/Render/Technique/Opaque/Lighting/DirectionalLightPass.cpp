#include "Castor3D/Render/Technique/Opaque/Lighting/DirectionalLightPass.hpp"

#include "Castor3D/Engine.hpp"
#include "Castor3D/Buffer/GpuBuffer.hpp"
#include "Castor3D/Material/Texture/TextureLayout.hpp"
#include "Castor3D/Material/Texture/TextureUnit.hpp"
#include "Castor3D/Miscellaneous/PipelineVisitor.hpp"
#include "Castor3D/Render/RenderPipeline.hpp"
#include "Castor3D/Render/RenderSystem.hpp"
#include "Castor3D/Render/Technique/Opaque/Lighting/LightPassResult.hpp"
#include "Castor3D/Scene/Camera.hpp"
#include "Castor3D/Scene/Scene.hpp"
#include "Castor3D/Scene/Light/Light.hpp"
#include "Castor3D/Scene/Light/DirectionalLight.hpp"
#include "Castor3D/Shader/Shaders/GlslLight.hpp"
#include "Castor3D/Shader/Shaders/GlslUtils.hpp"
#include "Castor3D/Shader/Ubos/GpInfoUbo.hpp"

#include <ashespp/Buffer/UniformBuffer.hpp>
#include <ashespp/Buffer/VertexBuffer.hpp>
#include <ashespp/Image/ImageView.hpp>
#include <ashespp/RenderPass/RenderPass.hpp>
#include <ashespp/RenderPass/RenderPassCreateInfo.hpp>

#include <ShaderWriter/Source.hpp>

using namespace castor;
using namespace castor3d;

namespace castor3d
{
	//*********************************************************************************************

	namespace
	{
		static constexpr uint32_t VertexCount = 6u;

		ashes::RenderPassPtr doCreateRenderPass( Engine & engine
			, LightPassResult const & lpResult
			, bool first )
		{
			auto & device = getCurrentRenderDevice( engine );
			VkImageLayout layout = first
				? VK_IMAGE_LAYOUT_UNDEFINED
				: VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			VkAttachmentLoadOp loadOp = first
				? VK_ATTACHMENT_LOAD_OP_CLEAR
				: VK_ATTACHMENT_LOAD_OP_LOAD;
			ashes::VkAttachmentDescriptionArray attaches
			{
				{
					0u,
					lpResult[LpTexture::eDepth].getTexture()->getPixelFormat(),
					VK_SAMPLE_COUNT_1_BIT,
					VK_ATTACHMENT_LOAD_OP_LOAD,
					VK_ATTACHMENT_STORE_OP_STORE,
					VK_ATTACHMENT_LOAD_OP_DONT_CARE,
					VK_ATTACHMENT_STORE_OP_DONT_CARE,
					VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
					VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
				},
				{
					0u,
					lpResult[LpTexture::eDiffuse].getTexture()->getPixelFormat(),
					VK_SAMPLE_COUNT_1_BIT,
					loadOp,
					VK_ATTACHMENT_STORE_OP_STORE,
					VK_ATTACHMENT_LOAD_OP_DONT_CARE,
					VK_ATTACHMENT_STORE_OP_DONT_CARE,
					layout,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				},
				{
					0u,
					lpResult[LpTexture::eSpecular].getTexture()->getPixelFormat(),
					VK_SAMPLE_COUNT_1_BIT,
					loadOp,
					VK_ATTACHMENT_STORE_OP_STORE,
					VK_ATTACHMENT_LOAD_OP_DONT_CARE,
					VK_ATTACHMENT_STORE_OP_DONT_CARE,
					layout,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				},
			};
			ashes::SubpassDescriptionArray subpasses;
			subpasses.emplace_back( ashes::SubpassDescription
				{
					0u,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					{},
					{
						{ 1u, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
						{ 2u, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
					},
					{},
					VkAttachmentReference{ 0u, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL },
					{},
				} );
			ashes::VkSubpassDependencyArray dependencies
			{
				{
					VK_SUBPASS_EXTERNAL,
					0u,
					VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
					VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
					VK_ACCESS_SHADER_READ_BIT,
					VK_DEPENDENCY_BY_REGION_BIT,
				},
				{
					0u,
					VK_SUBPASS_EXTERNAL,
					VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
					VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
					VK_ACCESS_SHADER_READ_BIT,
					VK_DEPENDENCY_BY_REGION_BIT,
				}
			};
			ashes::RenderPassCreateInfo createInfo
			{
				0u,
				std::move( attaches ),
				std::move( subpasses ),
				std::move( dependencies ),
			};
			auto result = device->createRenderPass( "DirectionalLightPass"
				, std::move( createInfo ) );
			return result;
		}
	}

	//*********************************************************************************************

	DirectionalLightPass::Program::Program( Engine & engine
		, DirectionalLightPass & pass
		, ShaderModule const & vtx
		, ShaderModule const & pxl
		, bool hasShadows )
		: LightPass::Program{ engine, pass.getName(), vtx, pxl, hasShadows }
		, m_lightPass{ pass }
	{
	}

	DirectionalLightPass::Program::~Program()
	{
	}

	ashes::GraphicsPipelinePtr DirectionalLightPass::Program::doCreatePipeline( ashes::PipelineVertexInputStateCreateInfo const & vertexLayout
		, ashes::RenderPass const & renderPass
		, bool blend )
	{
		ashes::PipelineDepthStencilStateCreateInfo dsstate
		{
			0u,
			false,
			false
		};
		ashes::VkPipelineColorBlendAttachmentStateArray attachs;
		ashes::PipelineColorBlendStateCreateInfo blstate;

		if ( blend )
		{
			attachs.push_back( VkPipelineColorBlendAttachmentState
				{
					true,
					VK_BLEND_FACTOR_ONE,
					VK_BLEND_FACTOR_ONE,
					VK_BLEND_OP_ADD,
					VK_BLEND_FACTOR_ONE,
					VK_BLEND_FACTOR_ONE,
					VK_BLEND_OP_ADD,
					defaultColorWriteMask,
				} );
			blstate = ashes::PipelineColorBlendStateCreateInfo
			{ 
				0u,
				VK_FALSE,
				VK_LOGIC_OP_COPY,
				std::move( attachs )
			};
		}

		blstate.attachments.push_back( blstate.attachments.back() );
		auto & device = getCurrentRenderDevice( m_engine );
		return device->createPipeline( m_name + ( blend ? std::string{ "Blend" } : std::string{ "First" } )
			, ashes::GraphicsPipelineCreateInfo
			{
				0u,
				m_program,
				vertexLayout,
				ashes::PipelineInputAssemblyStateCreateInfo{ 0u, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST },
				ashes::nullopt,
				ashes::PipelineViewportStateCreateInfo{},
				ashes::PipelineRasterizationStateCreateInfo{ 0u, VK_FALSE, VK_FALSE, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE },
				ashes::PipelineMultisampleStateCreateInfo{},
				std::move( dsstate ),
				std::move( blstate ),
				ashes::PipelineDynamicStateCreateInfo{ 0u, { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR } },
				*m_pipelineLayout,
				renderPass
			} );
	}

	void DirectionalLightPass::Program::doBind( Light const & light )
	{
		auto & data = m_lightPass.m_ubo->getData( 0u );
		light.bind( &data.base.colourIndex );
		m_lightPass.m_ubo->upload();
	}

	//*********************************************************************************************

	DirectionalLightPass::DirectionalLightPass( Engine & engine
		, LightPassResult const & lpResult
		, GpInfoUbo const & gpInfoUbo
		, bool hasShadows )
		: LightPass{ engine
			, "Directional"
			, doCreateRenderPass( engine, lpResult, true )
			, doCreateRenderPass( engine, lpResult, false )
			, lpResult
			, gpInfoUbo
			, hasShadows }
		, m_ubo{ makeUniformBuffer< Config >( *engine.getRenderSystem()
			, 1u
			, VK_BUFFER_USAGE_TRANSFER_DST_BIT
			, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			, "SsaoConfigUbo" ) }
		, m_viewport{ engine }
	{
		m_baseUbo = m_ubo.get();
		m_viewport.setOrtho( -1, 1, -1, 1, -1, 1 );
		log::trace << cuT( "Created DirectionalLightPass" ) << ( hasShadows ? castor::String{ cuT( "Shadow" ) } : cuEmptyString ) << std::endl;
	}

	void DirectionalLightPass::initialise( Scene const & scene
		, OpaquePassResult const & gp
		, SceneUbo & sceneUbo
		, RenderPassTimer & timer )
	{
		auto & renderSystem = *m_engine.getRenderSystem();
		auto & device = getCurrentRenderDevice( renderSystem );
		m_vertexBuffer = makeVertexBuffer< float >( device
			, 12u
			, 0u
			, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			, "DirectionalLightPassVbo" );

		if ( auto buffer = m_vertexBuffer->getBuffer().lock( 0u
			, ~( 0ull )
			, 0u ) )
		{
			float data[] =
			{
				-1.0f, -1.0f,
				-1.0f, +1.0f,
				+1.0f, -1.0f,
				+1.0f, -1.0f,
				-1.0f, +1.0f,
				+1.0f, +1.0f,
			};
			std::memcpy( buffer, data, sizeof( data ) );
			m_vertexBuffer->getBuffer().flush( 0u, ~( 0ull ) );
			m_vertexBuffer->getBuffer().unlock();
		}
		
		m_vertexLayout = std::make_unique< ashes::PipelineVertexInputStateCreateInfo >( 0u
			, ashes::VkVertexInputBindingDescriptionArray
			{
				{ 0u, sizeof( Point2f ), VK_VERTEX_INPUT_RATE_VERTEX },
			}
			, ashes::VkVertexInputAttributeDescriptionArray
			{
				{ 0u, 0u, VK_FORMAT_R32G32_SFLOAT, 0u },
			} );
		doInitialise( scene
			, gp
			, LightType::eDirectional
			, *m_vertexBuffer
			, *m_vertexLayout
			, sceneUbo
			, nullptr
			, timer );
		m_viewport.update();
	}

	void DirectionalLightPass::cleanup()
	{
		doCleanup();
		m_vertexLayout.reset();
		m_vertexBuffer.reset();
	}

	void DirectionalLightPass::accept( PipelineVisitorBase & visitor )
	{
		if ( m_vertexShader.shader )
		{
			visitor.visit( m_vertexShader );
		}

		if ( m_pixelShader.shader )
		{
			visitor.visit( m_pixelShader );
		}
	}

	uint32_t DirectionalLightPass::getCount()const
	{
		return VertexCount;
	}

	void DirectionalLightPass::doUpdate( bool first
		, Size const & size
		, Light const & light
		, Camera const & camera
		, ShadowMap const * shadowMap
		, uint32_t shadowMapIndex )
	{
		m_viewport.resize( size );
		m_matrixUbo.update( camera.getView(), m_viewport.getProjection() );
		m_pipeline->program->bind( light );
	}

	ShaderPtr DirectionalLightPass::doGetVertexShaderSource( SceneFlags const & sceneFlags )const
	{
		using namespace sdw;
		VertexWriter writer;

		// Shader inputs
		UBO_MATRIX( writer, MatrixUbo::BindingPoint, 0u );
		UBO_GPINFO( writer, GpInfoUbo::BindingPoint, 0u );
		auto position = writer.declInput< Vec2 >( "position", 0u );

		// Shader outputs
		auto out = writer.getOut();

		writer.implementFunction< sdw::Void >( "main"
			, [&]()
			{
				out.vtx.position = c3d_projection * vec4( position, 0.0_f, 1.0_f );
			} );

		return std::make_unique< ast::Shader >( std::move( writer.getShader() ) );
	}

	LightPass::ProgramPtr DirectionalLightPass::doCreateProgram()
	{
		return std::make_unique< Program >( m_engine
			, *this
			, m_vertexShader
			, m_pixelShader
			, m_shadows );
	}
}
