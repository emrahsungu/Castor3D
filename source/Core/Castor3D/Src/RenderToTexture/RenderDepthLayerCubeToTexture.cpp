#include "RenderDepthLayerCubeToTexture.hpp"

#include "Engine.hpp"

#include "Mesh/Vertex.hpp"
#include "Mesh/Buffer/Buffer.hpp"
#include "Render/RenderPipeline.hpp"
#include "Shader/ShaderProgram.hpp"
#include "Texture/Sampler.hpp"
#include "Texture/TextureLayout.hpp"

#include <GlslSource.hpp>

using namespace Castor;

namespace Castor3D
{
	RenderDepthLayerCubeToTexture::RenderDepthLayerCubeToTexture( Context & p_context
		, UniformBuffer & p_matrixUbo )
		: OwnedBy< Context >{ p_context }
		, m_matrixUbo{ p_matrixUbo }
		, m_viewport{ *p_context.GetRenderSystem()->GetEngine() }
		, m_bufferVertex
		{
			{
				0, 0, 0, 0, 1, 1, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 1
			}
		}
		, m_declaration
		{
			{
				BufferElementDeclaration{ ShaderProgram::Position, uint32_t( ElementUsage::ePosition ), ElementType::eVec2 },
				BufferElementDeclaration{ ShaderProgram::Texture, uint32_t( ElementUsage::eTexCoords ), ElementType::eVec2 }
			}
		}
	{
		uint32_t i = 0;

		for ( auto & l_vertex : m_arrayVertex )
		{
			l_vertex = std::make_shared< BufferElementGroup >( &reinterpret_cast< uint8_t * >( m_bufferVertex.data() )[i++ * m_declaration.stride()] );
		}
	}

	RenderDepthLayerCubeToTexture::~RenderDepthLayerCubeToTexture()
	{
	}

	void RenderDepthLayerCubeToTexture::Initialise()
	{
		m_viewport.Initialise();
		auto & l_program = *DoCreateProgram();
		auto & l_renderSystem = *GetOwner()->GetRenderSystem();
		l_program.Initialise();
		m_vertexBuffer = std::make_shared< VertexBuffer >( *l_renderSystem.GetEngine()
			, m_declaration );
		m_vertexBuffer->Resize( uint32_t( m_arrayVertex.size()
			* m_declaration.stride() ) );
		m_vertexBuffer->LinkCoords( m_arrayVertex.begin()
			, m_arrayVertex.end() );
		m_vertexBuffer->Initialise( BufferAccessType::eStatic
			, BufferAccessNature::eDraw );
		m_geometryBuffers = l_renderSystem.CreateGeometryBuffers( Topology::eTriangles
			, l_program );
		m_geometryBuffers->Initialise( { *m_vertexBuffer }
		, nullptr );

		DepthStencilState l_dsState;
		l_dsState.SetDepthTest( false );
		m_pipeline = l_renderSystem.CreateRenderPipeline( std::move( l_dsState )
			, RasteriserState{}
			, BlendState{}
			, MultisampleState{}
			, l_program
			, PipelineFlags{} );
		m_pipeline->AddUniformBuffer( m_matrixUbo );

		m_sampler = GetOwner()->GetRenderSystem()->GetEngine()->GetSamplerCache().Add( cuT( "RenderDepthLayerCubeToTexture" ) );
		m_sampler->SetInterpolationMode( InterpolationFilter::eMin, InterpolationMode::eLinear );
		m_sampler->SetInterpolationMode( InterpolationFilter::eMag, InterpolationMode::eLinear );
		m_sampler->SetWrappingMode( TextureUVW::eU, WrapMode::eClampToEdge );
		m_sampler->SetWrappingMode( TextureUVW::eV, WrapMode::eClampToEdge );
		m_sampler->SetWrappingMode( TextureUVW::eW, WrapMode::eClampToEdge );
	}

	void RenderDepthLayerCubeToTexture::Cleanup()
	{
		m_sampler.reset();
		m_pipeline->Cleanup();
		m_pipeline.reset();
		m_vertexBuffer->Cleanup();
		m_vertexBuffer.reset();
		m_geometryBuffers->Cleanup();
		m_geometryBuffers.reset();
		m_viewport.Cleanup();
	}

	void RenderDepthLayerCubeToTexture::Render( Position const & p_position
		, Size const & p_size
		, TextureLayout const & p_texture
		, uint32_t p_layer )
	{
		int l_w = p_size.width();
		int l_h = p_size.height();
		DoRender( p_position + Position{ l_w * 2, l_h * 1 }
			, p_size
			, p_texture
			, Point3f{ 1, 0, 0 }
			, Point2f{ 1, 1 }
			, *m_pipeline
			, m_matrixUbo
			, *m_geometryBuffers
			, p_layer );
		DoRender( p_position + Position{ l_w * 1, l_h * 1 }
			, p_size
			, p_texture
			, Point3f{ 0, -1, 0 }
			, Point2f{ 1, 1 }
			, *m_pipeline
			, m_matrixUbo
			, *m_geometryBuffers
			, p_layer );
		DoRender( p_position + Position{ l_w * 0, l_h * 1 }
			, p_size
			, p_texture
			, Point3f{ -1, 0, 0 }
			, Point2f{ -1, 1 }
			, *m_pipeline
			, m_matrixUbo
			, *m_geometryBuffers
			, p_layer );
		DoRender( p_position + Position{ l_w * 3, l_h * 1 }
			, p_size
			, p_texture
			, Point3f{ 0, 1, 0 }
			, Point2f{ 1, -1 }
			, *m_pipeline
			, m_matrixUbo
			, *m_geometryBuffers
			, p_layer );
		DoRender( p_position + Position{ l_w * 1, l_h * 0 }
			, p_size
			, p_texture
			, Point3f{ 0, 0, -1 }
			, Point2f{ 1, 1 }
			, *m_pipeline
			, m_matrixUbo
			, *m_geometryBuffers
			, p_layer );
		DoRender( p_position + Position{ l_w * 1, l_h * 2 }
			, p_size
			, p_texture
			, Point3f{ 0, 0, 1 }
			, Point2f{ 1, 1 }
			, *m_pipeline
			, m_matrixUbo
			, *m_geometryBuffers
			, p_layer );
	}

	void RenderDepthLayerCubeToTexture::DoRender( Point2i const & p_position
		, Size const & p_size
		, TextureLayout const & p_texture
		, Point3f const & p_face
		, Point2f const & p_uvMult
		, RenderPipeline & p_pipeline
		, UniformBuffer & p_matrixUbo
		, GeometryBuffers const & p_geometryBuffers
		, uint32_t p_layer )
	{
		REQUIRE( p_texture.GetType() == TextureType::eCubeArray );
		m_viewport.SetPosition( Position{ p_position[0], p_position[1] } );
		m_viewport.Resize( p_size );
		m_viewport.Update();
		m_viewport.Apply();
		p_pipeline.SetProjectionMatrix( m_viewport.GetProjection() );

		REQUIRE( m_faceUniform );
		m_faceUniform->SetValue( p_face );

		REQUIRE( m_uvUniform );
		m_uvUniform->SetValue( p_uvMult );

		REQUIRE( m_layerIndexUniform );
		m_layerIndexUniform->SetValue( p_layer );

		p_pipeline.ApplyProjection( p_matrixUbo );
		p_matrixUbo.Update();
		p_pipeline.Apply();
		m_faceUniform->Update();
		m_uvUniform->Update();
		m_layerIndexUniform->Update();

		p_texture.Bind( 0u );
		m_sampler->Bind( 0u );
		p_geometryBuffers.Draw( uint32_t( m_arrayVertex.size() ), 0 );
		m_sampler->Unbind( 0u );
		p_texture.Unbind( 0u );
	}

	ShaderProgramSPtr RenderDepthLayerCubeToTexture::DoCreateProgram()
	{
		auto & l_renderSystem = *GetOwner()->GetRenderSystem();
		String l_vtx;
		{
			using namespace GLSL;
			auto l_writer = l_renderSystem.CreateGlslWriter();

			UBO_MATRIX( l_writer );

			// Shader inputs
			auto position = l_writer.GetAttribute< Vec2 >( ShaderProgram::Position );
			auto texture = l_writer.GetAttribute< Vec2 >( ShaderProgram::Texture );

			// Shader outputs
			auto vtx_texture = l_writer.GetOutput< Vec2 >( cuT( "vtx_texture" ) );
			auto gl_Position = l_writer.GetBuiltin< Vec4 >( cuT( "gl_Position" ) );

			l_writer.ImplementFunction< void >( cuT( "main" ), [&]()
			{
				vtx_texture = texture;
				gl_Position = c3d_mtxProjection * vec4( position.x(), position.y(), 0.0, 1.0 );
			} );
			l_vtx = l_writer.Finalise();
		}

		String l_pxl;
		{
			using namespace GLSL;
			auto l_writer = l_renderSystem.CreateGlslWriter();

			// Shader inputs
			auto c3d_mapDiffuse = l_writer.GetUniform< SamplerCubeArray >( ShaderProgram::MapDiffuse );
			auto c3d_v3Face = l_writer.GetUniform< Vec3 >( cuT( "c3d_v3Face" ) );
			auto c3d_v2UvMult = l_writer.GetUniform< Vec2 >( cuT( "c3d_v2UvMult" ) );
			auto c3d_iIndex = l_writer.GetUniform< Int >( cuT( "c3d_iIndex" ) );
			auto vtx_texture = l_writer.GetInput< Vec2 >( cuT( "vtx_texture" ) );

			// Shader outputs
			auto plx_v4FragColor = l_writer.GetFragData< Vec4 >( cuT( "plx_v4FragColor" ), 0 );

			l_writer.ImplementFunction< void >( cuT( "main" ), [&]()
			{
				auto l_mapCoord = l_writer.GetLocale( cuT( "l_mapCoord" ), vtx_texture * 2.0_f - 1.0_f );
				auto l_uv = l_writer.GetLocale< Vec3 >( cuT( "l_uv" )
					, l_writer.Ternary( c3d_v3Face.x() != 0.0_f
						, vec3( c3d_v3Face.x(), l_mapCoord )
						, l_writer.Ternary( c3d_v3Face.y() != 0.0_f
							, vec3( l_mapCoord.x(), c3d_v3Face.y(), l_mapCoord.y() )
							, vec3( l_mapCoord, c3d_v3Face.z() ) ) ) );
				auto l_depth = l_writer.GetLocale( cuT( "l_depth" ), texture( c3d_mapDiffuse, vec4( l_uv, l_writer.Cast< Float >( c3d_iIndex ) ) ).x() );
				plx_v4FragColor = vec4( l_depth, l_depth, l_depth, 1.0 );
			} );
			l_pxl = l_writer.Finalise();
		}

		auto l_model = l_renderSystem.GetGpuInformations().GetMaxShaderModel();
		auto & l_cache = l_renderSystem.GetEngine()->GetShaderProgramCache();
		auto l_program = l_cache.GetNewProgram( false );
		l_program->CreateObject( ShaderType::eVertex );
		l_program->CreateObject( ShaderType::ePixel );
		l_program->SetSource( ShaderType::eVertex, l_model, l_vtx );
		l_program->SetSource( ShaderType::ePixel, l_model, l_pxl );
		l_program->CreateUniform< UniformType::eInt >( ShaderProgram::MapDiffuse, ShaderType::ePixel )->SetValue( 0u );
		m_faceUniform = l_program->CreateUniform< UniformType::eVec3f >( cuT( "c3d_v3Face" ), ShaderType::ePixel );
		m_uvUniform = l_program->CreateUniform< UniformType::eVec2f >( cuT( "c3d_v2UvMult" ), ShaderType::ePixel );
		m_layerIndexUniform = l_program->CreateUniform< UniformType::eInt >( cuT( "c3d_iIndex" ), ShaderType::ePixel );
		l_program->Initialise();
		return l_program;
	}
}