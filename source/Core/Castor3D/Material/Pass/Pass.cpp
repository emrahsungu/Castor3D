#include "Castor3D/Material/Pass/Pass.hpp"

#include "Castor3D/Engine.hpp"
#include "Castor3D/Material/Material.hpp"
#include "Castor3D/Material/Texture/TextureConfiguration.hpp"
#include "Castor3D/Material/Texture/TextureLayout.hpp"
#include "Castor3D/Material/Texture/TextureUnit.hpp"
#include "Castor3D/Render/Node/PassRenderNode.hpp"
#include "Castor3D/Shader/Program.hpp"

#include <CastorUtils/Graphics/PixelFormat.hpp>

#include <algorithm>

using namespace castor;

namespace castor3d
{
	//*********************************************************************************************

	namespace
	{
		void mergeMasks( uint32_t lhs
			, uint32_t & rhs )
		{
			rhs |= lhs;
		}

		void mergeFactors( float lhs
			, float & rhs
			, float ref )
		{
			if ( lhs != rhs )
			{
				rhs = ( lhs == ref
					? rhs
					: lhs );
			}
		}

		void mergeConfigs( TextureConfiguration lhs
			, TextureConfiguration & rhs )
		{
			mergeMasks( lhs.colourMask[0], rhs.colourMask[0] );
			mergeMasks( lhs.specularMask[0], rhs.specularMask[0] );
			mergeMasks( lhs.glossinessMask[0], rhs.glossinessMask[0] );
			mergeMasks( lhs.opacityMask[0], rhs.opacityMask[0] );
			mergeMasks( lhs.emissiveMask[0], rhs.emissiveMask[0] );
			mergeMasks( lhs.normalMask[0], rhs.normalMask[0] );
			mergeMasks( lhs.heightMask[0], rhs.heightMask[0] );
			mergeMasks( lhs.occlusionMask[0], rhs.occlusionMask[0] );
			mergeMasks( lhs.transmittanceMask[0], rhs.transmittanceMask[0] );
			mergeMasks( lhs.needsGammaCorrection, rhs.needsGammaCorrection );
			mergeMasks( lhs.needsYInversion, rhs.needsYInversion );
			mergeFactors( lhs.heightFactor, rhs.heightFactor, 0.1f );
			mergeFactors( lhs.normalFactor, rhs.normalFactor, 1.0f );
			mergeFactors( lhs.normalGMultiplier, rhs.normalGMultiplier, 1.0f );
		}
	}

	//*********************************************************************************************
	
	Pass::TextWriter::TextWriter( String const & tabs )
		: castor::TextWriter< Pass >{ tabs }
	{
	}

	bool Pass::TextWriter::operator()( Pass const & pass, TextFile & file )
	{
		static std::map< VkCompareOp, String > strAlphaFuncs
		{
			{ VK_COMPARE_OP_ALWAYS, cuT( "always" ) },
			{ VK_COMPARE_OP_LESS, cuT( "less" ) },
			{ VK_COMPARE_OP_LESS_OR_EQUAL, cuT( "less_or_equal" ) },
			{ VK_COMPARE_OP_EQUAL, cuT( "equal" ) },
			{ VK_COMPARE_OP_NOT_EQUAL, cuT( "not_equal" ) },
			{ VK_COMPARE_OP_GREATER_OR_EQUAL, cuT( "greater_or_equal" ) },
			{ VK_COMPARE_OP_GREATER, cuT( "greater" ) },
			{ VK_COMPARE_OP_NEVER, cuT( "never" ) },
		};
		static const String StrBlendModes[uint32_t( BlendMode::eCount )] =
		{
			cuT( "none" ),
			cuT( "additive" ),
			cuT( "multiplicative" ),
			cuT( "interpolative" ),
			cuT( "a_buffer" ),
			cuT( "depth_peeling" ),
		};

		bool result = true;

		if ( pass.getOpacity() < 1 )
		{
			result = file.writeText( m_tabs + cuT( "\talpha " ) + string::toString( pass.getOpacity(), std::locale{ "C" } ) + cuT( "\n" ) ) > 0;
			castor::TextWriter< Pass >::checkError( result, "Pass opacity" );

			if ( result && pass.getBWAccumulationOperator() != 0u )
			{
				result = file.writeText( m_tabs + cuT( "\tbw_accumulation " ) + string::toString( uint32_t( pass.getBWAccumulationOperator() ), std::locale{ "C" } ) + cuT( "\n" ) ) > 0;
				castor::TextWriter< Pass >::checkError( result, "Pass opacity" );
			}
		}

		if ( result && pass.getEmissive() > 0 )
		{
			result = file.writeText( m_tabs + cuT( "\temissive " ) + string::toString( pass.getEmissive(), std::locale{ "C" } ) + cuT( "\n" ) ) > 0;
			castor::TextWriter< Pass >::checkError( result, "Pass emissive" );
		}

		if ( result && pass.IsTwoSided() )
		{
			result = file.writeText( m_tabs + cuT( "\ttwo_sided " ) + String( pass.IsTwoSided() ? cuT( "true" ) : cuT( "false" ) ) + cuT( "\n" ) ) > 0;
			castor::TextWriter< Pass >::checkError( result, "Pass sidedness" );
		}

		if ( result && ( pass.getColourBlendMode() != BlendMode::eNoBlend ) )
		{
			result = file.writeText( m_tabs + cuT( "\tcolour_blend_mode " ) + StrBlendModes[uint32_t( pass.getColourBlendMode() )] + cuT( "\n" ) ) > 0;
			castor::TextWriter< Pass >::checkError( result, "Pass colour blend mode" );
		}

		if ( result )
		{
			if ( pass.hasAlphaTest() )
			{
				result = file.writeText( m_tabs + cuT( "\talpha_func " )
					+ strAlphaFuncs[pass.getAlphaFunc()] + cuT( " " )
					+ string::toString( pass.getAlphaValue(), std::locale{ "C" } ) + cuT( "\n" ) ) > 0;
				castor::TextWriter< Pass >::checkError( result, "Pass alpha function" );
			}
			else if ( pass.hasAlphaBlending()
				&& pass.getAlphaBlendMode() != BlendMode::eNoBlend )
			{
				result = file.writeText( m_tabs + cuT( "\talpha_blend_mode " ) + StrBlendModes[uint32_t( pass.getAlphaBlendMode() )] + cuT( "\n" ) ) > 0;
				castor::TextWriter< Pass >::checkError( result, "Pass alpha blend mode" );
			}
		}

		if ( result && pass.hasParallaxOcclusion() )
		{
			result = file.writeText( m_tabs + cuT( "\tparallax_occlusion true\n" ) ) > 0;
		}

		if ( result && pass.hasEnvironmentMapping() )
		{
			result = file.writeText( m_tabs + cuT( "\tparallax_occlusion true\n" ) ) > 0;
		}

		if ( result )
		{
			for ( auto unit : pass )
			{
				result = result && TextureUnit::TextWriter{ m_tabs + cuT( "\t" ), pass.getType() }( *unit, file );
			}
		}

		if ( result && pass.hasSubsurfaceScattering() )
		{
			result = SubsurfaceScattering::TextWriter{ m_tabs + cuT( "\t" ) }( pass.getSubsurfaceScattering(), file );
		}

		return result;
	}

	//*********************************************************************************************

	Pass::Pass( Material & parent )
		: OwnedBy< Material >{ parent }
		, m_flags{ ( getType() == MaterialType::eMetallicRoughness
			? PassFlag::eMetallicRoughness 
			: ( getType() == MaterialType::eSpecularGlossiness
				? PassFlag::eSpecularGlossiness
				: PassFlag::eNone ) ) }
	{
	}

	Pass::~Pass()
	{
		m_textureUnits.clear();
	}

	void Pass::initialise()
	{
		prepareTextures();

		for ( auto unit : m_textureUnits )
		{
			unit->initialise();
		}

		doInitialise();
	}

	void Pass::cleanup()
	{
		doCleanup();

		for ( auto unit : m_textureUnits )
		{
			unit->cleanup();
		}
	}

	void Pass::addTextureUnit( TextureUnitSPtr unit )
	{
		m_textures |= unit->getFlags();
		auto image = unit->toString();
		auto it = std::find_if( m_textureUnits.begin()
			, m_textureUnits.end()
			, [&image]( TextureUnitSPtr lookup )
			{
				return lookup->toString() == image;
			} );

		if ( it == m_textureUnits.end() )
		{
			if ( unit->getConfiguration().heightMask[0] )
			{
				m_heightTextureIndex = uint32_t( m_textureUnits.size() );
			}

			m_textureUnits.push_back( std::move( unit ) );
		}
		else
		{
			auto lhsConfig = unit->getConfiguration();
			unit = *it;
			auto rhsConfig = unit->getConfiguration();
			mergeConfigs( std::move( lhsConfig ), rhsConfig );
			unit->setConfiguration( std::move( rhsConfig ) );

			if ( unit->getConfiguration().heightMask[0] )
			{
				m_heightTextureIndex = uint32_t( std::distance( m_textureUnits.begin(), it ) );
			}
		}

		updateFlag( PassFlag::eAlphaBlending, hasAlphaBlending() );
		m_texturesReduced = false;
	}

	void Pass::removeTextureUnit( uint32_t index )
	{
		CU_Require( index < m_textureUnits.size() );
		log::info << cuT( "Destroying TextureUnit " ) << index << std::endl;
		auto it = m_textureUnits.begin() + index;
		auto config = ( *it )->getConfiguration();
		m_textureUnits.erase( it );
		remFlag( m_textures, TextureFlag( uint16_t( getFlags( config ) ) ) );
		updateFlag( PassFlag::eAlphaBlending, hasAlphaBlending() );
		m_texturesReduced = false;
	}

	TextureUnitSPtr Pass::getTextureUnit( uint32_t index )const
	{
		CU_Require( index < m_textureUnits.size() );
		return m_textureUnits[index];
	}

	bool Pass::hasAlphaBlending()const
	{
		return ( checkFlag( m_textures, TextureFlag::eOpacity ) || m_opacity < 1.0f )
			&& !hasAlphaTest();
	}

	bool Pass::hasAlphaTest()const
	{
		return getAlphaFunc() != VK_COMPARE_OP_ALWAYS;
	}

	void Pass::prepareTextures()
	{
		if ( !m_texturesReduced )
		{
			for ( auto & unit : m_textureUnits )
			{
				auto configuration = unit->getConfiguration();

				if ( unit->getRenderTarget() )
				{
					configuration.needsGammaCorrection = 0u;
				}
				else if ( configuration.colourMask[0]
					|| configuration.emissiveMask[0] )
				{
					auto format = unit->getTexture()->getPixelFormat();
					configuration.needsGammaCorrection = !PF::isFloatingPoint( convert( format ) );
				}

				unit->setConfiguration( configuration );
			}

			std::sort( m_textureUnits.begin()
				, m_textureUnits.end()
				, []( TextureUnitSPtr const & lhs, TextureUnitSPtr const & rhs )
				{
					return lhs->getFlags() < rhs->getFlags();
				} );

			m_texturesReduced = true;
		}
	}

	MaterialType Pass::getType()const
	{
		return getOwner()->getType();
	}

	void Pass::setOpacity( float value )
	{
		m_opacity = value;
		updateFlag( PassFlag::eAlphaBlending, hasAlphaBlending() );

		if ( hasAlphaBlending()
			&& m_alphaBlendMode == BlendMode::eNoBlend )
		{
			m_alphaBlendMode = BlendMode::eInterpolative;
		}

		doSetOpacity( value );
		onChanged( *this );
	}

	PassFlags Pass::getPassFlags()const
	{
		return m_flags;
	}

	void Pass::setSubsurfaceScattering( SubsurfaceScatteringUPtr value )
	{
		m_subsurfaceScattering = std::move( value );
		updateFlag( PassFlag::eSubsurfaceScattering, m_subsurfaceScattering != nullptr );
		m_sssConnection = m_subsurfaceScattering->onChanged.connect( [this]( SubsurfaceScattering const & sss )
			{
				onSssChanged( sss );
			} );
		onChanged( *this );
	}

	bool Pass::needsGammaCorrection()const
	{
		return m_textureUnits.end() != std::find_if( m_textureUnits.begin()
			, m_textureUnits.end()
			, []( TextureUnitSPtr unit )
			{
				return unit->getConfiguration().needsGammaCorrection != 0u;
			} );
	}

	TextureUnitPtrArray Pass::getTextureUnits( TextureFlags mask )const
	{
		TextureUnitPtrArray result;

		for ( auto & unit : m_textureUnits )
		{
			auto & config = unit->getConfiguration();

			if ( ( checkFlag( mask, TextureFlag::eAlbedo ) && config.colourMask[0] )
				|| ( checkFlag( mask, TextureFlag::eSpecular ) && config.specularMask[0] )
				|| ( checkFlag( mask, TextureFlag::eGlossiness ) && config.glossinessMask[0] )
				|| ( checkFlag( mask, TextureFlag::eOpacity ) && config.opacityMask[0] )
				|| ( checkFlag( mask, TextureFlag::eEmissive ) && config.emissiveMask[0] )
				|| ( checkFlag( mask, TextureFlag::eNormal ) && config.normalMask[0] )
				|| ( checkFlag( mask, TextureFlag::eHeight ) && config.heightMask[0] )
				|| ( checkFlag( mask, TextureFlag::eOcclusion ) && config.occlusionMask[0] ) 
				|| ( checkFlag( mask, TextureFlag::eTransmittance ) && config.transmittanceMask[0] ) )
			{
				result.push_back( unit );
			}
		}

		return result;
	}

	TextureFlagsArray Pass::getTextures( TextureFlags mask )const
	{
		auto units = getTextureUnits( mask );
		TextureFlagsArray result;

		for ( auto & unit : units )
		{
			result.push_back( { unit->getFlags(), unit->getId() } );
		}

		return result;
	}

	uint32_t Pass::getTextureUnitsCount( TextureFlags mask )const
	{
		return uint32_t( getTextureUnits( mask ).size() );
	}

	void Pass::onSssChanged( SubsurfaceScattering const & sss )
	{
		onChanged( *this );
	}
}
