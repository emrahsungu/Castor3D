#include "Castor3D/Cache/ShaderCache.hpp"

#include "Castor3D/Engine.hpp"

#include "Castor3D/Event/Frame/CleanupEvent.hpp"
#include "Castor3D/Event/Frame/InitialiseEvent.hpp"
#include "Castor3D/Material/Pass.hpp"
#include "Castor3D/Render/RenderPass.hpp"
#include "Castor3D/Shader/Program.hpp"

#include <ShaderWriter/Source.hpp>

#include <ashespp/Core/Device.hpp>

using namespace castor;

namespace castor3d
{
	ShaderProgramCache::ShaderProgramCache( Engine & engine )
		: OwnedBy< Engine >( engine )
	{
	}

	ShaderProgramCache::~ShaderProgramCache()
	{
	}

	void ShaderProgramCache::cleanup()
	{
		auto lock = makeUniqueLock( m_mutex );

		for ( auto & program : m_arrayPrograms )
		{
			getEngine()->postEvent( makeFunctorEvent( EventType::ePreRender
				, [&program]()
				{
					program->cleanup();
				} ) );
		}
	}

	void ShaderProgramCache::clear()
	{
		auto lock = makeUniqueLock( m_mutex );
		m_mapBillboards.clear();
		m_mapAutogenerated.clear();
		m_arrayPrograms.clear();
	}

	ShaderProgramSPtr ShaderProgramCache::getNewProgram( bool initialise )
	{
		return doAddProgram( std::make_shared< ShaderProgram >( *getEngine()->getRenderSystem() ), initialise );
	}

	ShaderProgramSPtr ShaderProgramCache::getAutomaticProgram( RenderPass const & renderPass
		, PipelineFlags const & flags )
	{
		if ( checkFlag( flags.programFlags, ProgramFlag::eBillboards ) )
		{
			auto lock = makeUniqueLock( m_mutex );
			auto const & it = m_mapBillboards.find( flags );

			if ( it != m_mapBillboards.end() )
			{
				return it->second;
			}
			else
			{
				auto result = doCreateBillboardProgram( renderPass
					, flags );
				CU_Require( result );
				return doAddBillboardProgram( std::move( result )
					, flags );
			}
		}
		else
		{
			auto lock = makeUniqueLock( m_mutex );
			auto it = m_mapAutogenerated.find( flags );

			if ( it != m_mapAutogenerated.end() )
			{
				return it->second;
			}
			else
			{
				auto result = doCreateAutomaticProgram( renderPass
					, flags );
				CU_Require( result );
				return doAddAutomaticProgram( std::move( result )
					, flags );
			}
		}
	}

	ShaderProgramSPtr ShaderProgramCache::doAddProgram( ShaderProgramSPtr program
		, bool initialise )
	{
		auto lock = makeUniqueLock( m_mutex );
		m_arrayPrograms.push_back( program );

		if ( initialise )
		{
			getEngine()->sendEvent( makeFunctorEvent( EventType::ePreRender
				, [program]()
				{
 					program->initialise();
				} ) );
		}

		return m_arrayPrograms.back();
	}

	ShaderProgramSPtr ShaderProgramCache::doCreateAutomaticProgram( RenderPass const & renderPass
		, PipelineFlags const & flags )const
	{
		VkShaderStageFlags matrixUboShaderMask = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		ShaderProgramSPtr result = std::make_shared< ShaderProgram >( *getEngine()->getRenderSystem() );
		result->setSource( VK_SHADER_STAGE_VERTEX_BIT
			, renderPass.getVertexShaderSource( flags ) );
		result->setSource( VK_SHADER_STAGE_FRAGMENT_BIT
			, renderPass.getPixelShaderSource( flags ) );
		auto geometry = renderPass.getGeometryShaderSource( flags );

		if ( geometry )
		{
			matrixUboShaderMask |= VK_SHADER_STAGE_GEOMETRY_BIT;
			result->setSource( VK_SHADER_STAGE_GEOMETRY_BIT
				, std::move( geometry ) );
		}

		return result;
	}

	ShaderProgramSPtr ShaderProgramCache::doAddAutomaticProgram( ShaderProgramSPtr program
		, PipelineFlags const & flags )
	{
		auto lock = makeUniqueLock( m_mutex );
		auto const & it = m_mapAutogenerated.find( flags );

		if ( it == m_mapAutogenerated.end() )
		{
			m_mapAutogenerated.insert( { flags, program } );
			return doAddProgram( std::move( program ), true );
		}

		return it->second;
	}

	ShaderProgramSPtr ShaderProgramCache::doCreateBillboardProgram( RenderPass const & renderPass
		, PipelineFlags const & flags )const
	{
		ShaderProgramSPtr result = std::make_shared< ShaderProgram >( *getEngine()->getRenderSystem() );
		auto & engine = *getEngine();
		{
			using namespace sdw;
			VertexWriter writer;

			// Shader inputs
			auto position = writer.declInput< Vec4 >( cuT( "position" ), 0u );
			auto uv = writer.declInput< Vec2 >( cuT( "uv" ), 1u );
			auto center = writer.declInput< Vec3 >( cuT( "center" ), 2u );
			auto in = writer.getIn();
			UBO_MATRIX( writer, MatrixUbo::BindingPoint, 0 );
			UBO_SCENE( writer, SceneUbo::BindingPoint, 0 );
			UBO_MODEL_MATRIX( writer, ModelMatrixUbo::BindingPoint, 0 );
			UBO_MODEL( writer, ModelUbo::BindingPoint, 0 );
			UBO_BILLBOARD( writer, BillboardUbo::BindingPoint, 0 );

			// Shader outputs
			auto vtx_worldPosition = writer.declOutput< Vec3 >( cuT( "vtx_worldPosition" )
				, RenderPass::VertexOutputs::WorldPositionLocation );
			auto vtx_curPosition = writer.declOutput< Vec3 >( cuT( "vtx_curPosition" )
				, RenderPass::VertexOutputs::CurPositionLocation );
			auto vtx_prvPosition = writer.declOutput< Vec3 >( cuT( "vtx_prvPosition" )
				, RenderPass::VertexOutputs::PrvPositionLocation );
			auto vtx_normal = writer.declOutput< Vec3 >( cuT( "vtx_normal" )
				, RenderPass::VertexOutputs::NormalLocation );
			auto vtx_tangent = writer.declOutput< Vec3 >( cuT( "vtx_tangent" )
				, RenderPass::VertexOutputs::TangentLocation );
			auto vtx_bitangent = writer.declOutput< Vec3 >( cuT( "vtx_bitangent" )
				, RenderPass::VertexOutputs::BitangentLocation );
			auto vtx_texture = writer.declOutput< Vec3 >( cuT( "vtx_texture" )
				, RenderPass::VertexOutputs::TextureLocation );
			auto vtx_instance = writer.declOutput< UInt >( cuT( "vtx_instance" )
				, RenderPass::VertexOutputs::InstanceLocation );
			auto vtx_material = writer.declOutput< UInt >( cuT( "vtx_material" )
				, RenderPass::VertexOutputs::MaterialLocation );
			auto out = writer.getOut();

			writer.implementFunction< Void >( cuT( "main" )
				, [&]()
				{
					auto curBbcenter = writer.declLocale( cuT( "curBbcenter" )
						, writer.paren( c3d_curMtxModel * vec4( center, 1.0_f ) ).xyz() );
					auto prvBbcenter = writer.declLocale( cuT( "prvBbcenter" )
						, writer.paren( c3d_prvMtxModel * vec4( center, 1.0_f ) ).xyz() );
					auto curToCamera = writer.declLocale( cuT( "curToCamera" )
						, c3d_cameraPosition.xyz() - curBbcenter );
					curToCamera.y() = 0.0_f;
					curToCamera = normalize( curToCamera );
					auto right = writer.declLocale( cuT( "right" )
						, vec3( c3d_curView[0][0], c3d_curView[1][0], c3d_curView[2][0] ) );
					auto up = writer.declLocale( cuT( "up" )
						, vec3( c3d_curView[0][1], c3d_curView[1][1], c3d_curView[2][1] ) );

					if ( !checkFlag( flags.programFlags, ProgramFlag::eSpherical ) )
					{
						right = normalize( vec3( right.x(), 0.0, right.z() ) );
						up = vec3( 0.0_f, 1.0f, 0.0f );
					}

					vtx_material = writer.cast< UInt >( c3d_materialIndex );
					vtx_normal = curToCamera;
					vtx_tangent = up;
					vtx_bitangent = right;

					auto width = writer.declLocale( cuT( "width" ), c3d_dimensions.x() );
					auto height = writer.declLocale( cuT( "height" ), c3d_dimensions.y() );

					if ( checkFlag( flags.programFlags, ProgramFlag::eFixedSize ) )
					{
						width = c3d_dimensions.x() / c3d_clipInfo.x();
						height = c3d_dimensions.y() / c3d_clipInfo.y();
					}

					vtx_worldPosition = curBbcenter
						+ right * position.x() * width
						+ up * position.y() * height;
					auto prvPosition = writer.declLocale( cuT( "prvPosition" )
						, vec4( prvBbcenter + right * position.x() * width + up * position.y() * height, 1.0 ) );

					vtx_texture = vec3( uv, 0.0 );
					vtx_instance = writer.cast< UInt >( in.gl_InstanceID );
					auto curPosition = writer.declLocale( cuT( "curPosition" )
						, c3d_curView * vec4( vtx_worldPosition, 1.0 ) );
					prvPosition = c3d_prvView * vec4( prvPosition );
					curPosition = c3d_projection * curPosition;
					prvPosition = c3d_projection * prvPosition;

					// Convert the jitter from non-homogeneous coordiantes to homogeneous
					// coordinates and add it:
					// (note that for providing the jitter in non-homogeneous projection space,
					//  pixel coordinates (screen space) need to multiplied by two in the C++
					//  code)
					curPosition.xy() -= c3d_jitter * curPosition.w();
					prvPosition.xy() -= c3d_jitter * prvPosition.w();
					out.gl_out.gl_Position = curPosition;

					vtx_curPosition = curPosition.xyw();
					vtx_prvPosition = prvPosition.xyw();
					// Positions in projection space are in [-1, 1] range, while texture
					// coordinates are in [0, 1] range. So, we divide by 2 to get velocities in
					// the scale (and flip the y axis):
					vtx_curPosition.xy() *= vec2( 0.5_f, -0.5_f );
					vtx_prvPosition.xy() *= vec2( 0.5_f, -0.5_f );
				} );

			auto & vtxShader = writer.getShader();
			result->setSource( VK_SHADER_STAGE_VERTEX_BIT, std::make_unique< sdw::Shader >( std::move( vtxShader ) ) );
		}

		auto pxlShader = renderPass.getPixelShaderSource( flags );
		result->setSource( VK_SHADER_STAGE_FRAGMENT_BIT, std::move( pxlShader ) );
		return result;
	}

	ShaderProgramSPtr ShaderProgramCache::doAddBillboardProgram( ShaderProgramSPtr program
		, PipelineFlags const & flags )
	{
		auto lock = makeUniqueLock( m_mutex );
		auto const & it = m_mapBillboards.find( flags );

		if ( it == m_mapBillboards.end() )
		{
			m_mapBillboards.insert( { flags, program } );
			return doAddProgram( std::move( program ), true );
		}

		return it->second;
	}
}