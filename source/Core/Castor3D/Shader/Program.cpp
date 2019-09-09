#include "Castor3D/Shader/Program.hpp"

#include "Castor3D/Render/RenderSystem.hpp"

#include <CastorUtils/Stream/StreamPrefixManipulators.hpp>

using namespace castor;

namespace castor3d
{
	namespace
	{
		template< typename CharType, typename PrefixType >
		inline std::basic_ostream< CharType > & operator<<( std::basic_ostream< CharType > & stream
			, format::BasePrefixer< CharType, PrefixType > const & prefix )
		{
			auto * sbuf = dynamic_cast< format::BasicPrefixBuffer< format::BasePrefixer< CharType, PrefixType >, CharType > * >( stream.rdbuf() );

			if ( !sbuf )
			{
				format::installPrefixBuffer< PrefixType >( stream );
				stream.register_callback( format::callback< PrefixType, CharType >, 0 );
			}

			return stream;
		}

		auto doAddModule( VkShaderStageFlagBits stage
			, std::string const & name
			, std::map< VkShaderStageFlagBits, ShaderModule > & modules )
		{
			auto it = modules.find( stage );

			if ( it == modules.end() )
			{
				it = modules.emplace( stage, ShaderModule{ stage, name } ).first;
			}

			return it;
		}
	}

	//*************************************************************************************************

	ShaderProgram::TextWriter::TextWriter( String const & tabs
		, String const & name )
		: castor::TextWriter< ShaderProgram >{ tabs }
		, m_name{ name }
	{
	}

	bool ShaderProgram::TextWriter::operator()( ShaderProgram const & shaderProgram
		, TextFile & file )
	{
		bool result = false;
		bool hasFile = false;

		for ( auto file : shaderProgram.m_files )
		{
			hasFile |= !file.second.empty();
		}

		if ( hasFile )
		{
			auto tabs = m_tabs + cuT( "\t" );
			result = file.writeText( cuT( "\n" ) + m_tabs + m_name + cuT( "\n" ) ) > 0
				&& file.writeText( m_tabs + cuT( "{\n" ) ) > 0;
			checkError( result, "Shader program" );

			for ( auto & module : shaderProgram.m_modules )
			{
				//result = ShaderObject::TextWriter( tabs )( *shaderProgram.m_shaders[i], file );
			}

			if ( result )
			{
				result = file.writeText( m_tabs + cuT( "}\n" ) ) > 0;
			}
		}
		else
		{
			result = true;
		}

		return result;
	}

	//*************************************************************************************************

	struct VariableApply
	{
		template <class T> void operator()( T & p ) const
		{
			p->apply();
		}
	};

	//*************************************************************************************************

	const String ShaderProgram::Position = cuT( "position" );
	const String ShaderProgram::Normal = cuT( "normal" );
	const String ShaderProgram::Tangent = cuT( "tangent" );
	const String ShaderProgram::Bitangent = cuT( "bitangent" );
	const String ShaderProgram::Texture = cuT( "texture" );
	const String ShaderProgram::Colour = cuT( "colour" );
	const String ShaderProgram::Position2 = cuT( "position2" );
	const String ShaderProgram::Normal2 = cuT( "normal2" );
	const String ShaderProgram::Tangent2 = cuT( "tangent2" );
	const String ShaderProgram::Bitangent2 = cuT( "bitangent2" );
	const String ShaderProgram::Texture2 = cuT( "texture2" );
	const String ShaderProgram::Colour2 = cuT( "colour2" );
	const String ShaderProgram::Text = cuT( "text" );
	const String ShaderProgram::BoneIds0 = cuT( "bone_ids0" );
	const String ShaderProgram::BoneIds1 = cuT( "bone_ids1" );
	const String ShaderProgram::Weights0 = cuT( "weights0" );
	const String ShaderProgram::Weights1 = cuT( "weights1" );
	const String ShaderProgram::Transform = cuT( "transform" );
	const String ShaderProgram::Material = cuT( "material" );

	const String ShaderProgram::Lights = cuT( "c3d_sLights" );
	const String ShaderProgram::MapDiffuse = cuT( "c3d_mapDiffuse" );
	const String ShaderProgram::MapAlbedo = cuT( "c3d_mapAlbedo" );
	const String ShaderProgram::MapSpecular = cuT ("c3d_mapSpecular");
	const String ShaderProgram::MapRoughness = cuT( "c3d_mapRoughness" );
	const String ShaderProgram::MapEmissive = cuT( "c3d_mapEmissive" );
	const String ShaderProgram::MapNormal = cuT( "c3d_mapNormal" );
	const String ShaderProgram::MapOpacity = cuT( "c3d_mapOpacity" );
	const String ShaderProgram::MapGloss = cuT( "c3d_mapGloss" );
	const String ShaderProgram::MapMetallic = cuT ("c3d_mapMetallic");
	const String ShaderProgram::MapHeight = cuT( "c3d_mapHeight" );
	const String ShaderProgram::MapEnvironment = cuT( "c3d_mapEnvironment" );
	const String ShaderProgram::MapAmbientOcclusion = cuT( "c3d_mapAmbientOcclusion" );
	const String ShaderProgram::MapTransmittance = cuT( "c3d_mapTransmittance" );
	const String ShaderProgram::MapIrradiance = cuT( "c3d_mapIrradiance" );
	const String ShaderProgram::MapPrefiltered = cuT( "c3d_mapPrefiltered" );
	const String ShaderProgram::MapBrdf = cuT( "c3d_mapBrdf" );
	const String ShaderProgram::MapText = cuT( "c3d_mapText" );

	//*************************************************************************************************

	ShaderProgram::ShaderProgram( RenderSystem & renderSystem )
		: OwnedBy< RenderSystem >( renderSystem )
	{
	}

	ShaderProgram::~ShaderProgram()
	{
	}

	bool ShaderProgram::initialise()
	{
		if ( m_states.empty() )
		{
			auto & device = getCurrentRenderDevice( *this );

			auto loadShader = [this, &device]( VkShaderStageFlagBits stage )
			{
				static std::map< VkShaderStageFlagBits, std::string > type
				{
					{ VK_SHADER_STAGE_VERTEX_BIT, "Vtx" },
					{ VK_SHADER_STAGE_GEOMETRY_BIT, "Geo" },
					{ VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, "Tsc" },
					{ VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, "Tse" },
					{ VK_SHADER_STAGE_FRAGMENT_BIT, "Pxl" },
				};
				auto itModule = m_modules.find( stage );

				if ( itModule != m_modules.end() )
				{
					auto & module = itModule->second;
					auto itFile = m_files.find( stage );

					if ( !itFile->second.empty() )
					{
						TextFile file{ getFile( stage ), File::OpenMode::eRead };
						file.copyToString( module.source );
					}

					if ( module.shader || !module.source.empty() )
					{
						m_states.push_back( makeShaderState( device, module ) );
					}
				}
			};

			if ( hasSource( VK_SHADER_STAGE_COMPUTE_BIT )
				|| hasFile( VK_SHADER_STAGE_COMPUTE_BIT ) )
			{
				loadShader( VK_SHADER_STAGE_COMPUTE_BIT );
			}
			else
			{
				loadShader( VK_SHADER_STAGE_VERTEX_BIT );
				loadShader( VK_SHADER_STAGE_GEOMETRY_BIT );
				loadShader( VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT );
				loadShader( VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT );
				loadShader( VK_SHADER_STAGE_FRAGMENT_BIT );
			}
		}

		return !m_states.size() == m_modules.size();
	}

	void ShaderProgram::cleanup()
	{
		m_states.clear();
	}

	void ShaderProgram::setFile( VkShaderStageFlagBits target, Path const & pathFile )
	{
		m_files[target] = pathFile;
		doAddModule( target, "", m_modules );
	}

	Path ShaderProgram::getFile( VkShaderStageFlagBits target )const
	{
		auto it = m_files.find( target );
		CU_Require( it != m_files.end() );
		return it->second;
	}

	bool ShaderProgram::hasFile( VkShaderStageFlagBits target )const
	{
		auto it = m_files.find( target );
		return it != m_files.end()
			&& !it->second.empty();
	}

	void ShaderProgram::setSource( VkShaderStageFlagBits target, String const & source )
	{
		m_files[target].clear();
		auto it = doAddModule( target, "", m_modules );
		it->second.source = source;
		it->second.shader = nullptr;
	}

	void ShaderProgram::setSource( VkShaderStageFlagBits target, ShaderPtr shader )
	{
		m_files[target].clear();
		auto it = doAddModule( target, "", m_modules );
		it->second.source.clear();
		it->second.shader = std::move( shader );
	}

	ShaderModule const & ShaderProgram::getSource( VkShaderStageFlagBits target )const
	{
		auto it = m_modules.find( target );
		CU_Require( it != m_modules.end() );
		return it->second;
	}

	bool ShaderProgram::hasSource( VkShaderStageFlagBits target )const
	{
		auto it = m_modules.find( target );
		return it != m_modules.end()
			&& ( !it->second.source.empty() || it->second.shader != nullptr );
	}

	UInt32Array compileShader( RenderSystem const & renderSystem
		, ShaderModule const & module )
	{
		return renderSystem.compileShader( module );
	}

	UInt32Array compileShader( RenderDevice const & device
		, ShaderModule const & module )
	{
		return compileShader( device.renderSystem, module );
	}
}