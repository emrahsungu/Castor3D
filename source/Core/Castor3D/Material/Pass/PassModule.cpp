#include "Castor3D/Material/Pass/PassModule.hpp"

namespace castor3d
{
	castor::String getName( BlendMode value )
	{
		switch ( value )
		{
		case BlendMode::eNoBlend:
			return cuT( "none" );
		case BlendMode::eAdditive:
			return cuT( "additive" );
		case BlendMode::eMultiplicative:
			return cuT( "multiplicative" );
		case BlendMode::eInterpolative:
			return cuT( "interpolative" );
		case BlendMode::eABuffer:
			return cuT( "a_buffer" );
		case BlendMode::eDepthPeeling:
			return cuT( "depth_peeling" );
		default:
			CU_Failure( "Unsupported BlendMode" );
			return castor::cuEmptyString;
		}
	}

	castor::String getName( PassFlag value )
	{
		switch ( value )
		{
		case PassFlag::eNone:
			return cuT( "none" );
		case PassFlag::eAlphaBlending:
			return cuT( "alpha_blending" );
		case PassFlag::eAlphaTest:
			return cuT( "alpha_test" );
		case PassFlag::eMetallicRoughness:
			return cuT( "metallic_roughness" );
		case PassFlag::eSpecularGlossiness:
			return cuT( "specular_glossiness" );
		case PassFlag::eSubsurfaceScattering:
			return cuT( "subsurface_scattering" );
		case PassFlag::eDistanceBasedTransmittance:
			return cuT( "distance_based_transmittance" );
		case PassFlag::eParallaxOcclusionMapping:
			return cuT( "parallax_occlusion_mapping" );
		case PassFlag::eReflection:
			return cuT( "reflection" );
		case PassFlag::eRefraction:
			return cuT( "refraction" );
		default:
			CU_Failure( "Unsupported PassFlag" );
			return castor::cuEmptyString;
		}
	}
}
