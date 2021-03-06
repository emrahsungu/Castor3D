#include "Castor3D/Render/RenderModule.hpp"

#include "Castor3D/Engine.hpp"
#include "Castor3D/Render/RenderDevice.hpp"
#include "Castor3D/Render/RenderSystem.hpp"
#include "Castor3D/Scene/Scene.hpp"

CU_ImplementExportedOwnedBy( castor3d::RenderSystem, RenderSystem )
CU_ImplementExportedOwnedBy( castor3d::RenderDevice, RenderDevice )

namespace castor3d
{
	castor::String getName( FrustumCorner value )
	{
		switch ( value )
		{
		case FrustumCorner::eFarLeftBottom:
			return cuT( "far_left_bottom" );
		case FrustumCorner::eFarLeftTop:
			return cuT( "far_left_top" );
		case FrustumCorner::eFarRightTop:
			return cuT( "far_right_top" );
		case FrustumCorner::eFarRightBottom:
			return cuT( "far_right_bottom" );
		case FrustumCorner::eNearLeftBottom:
			return cuT( "near_left_bottom" );
		case FrustumCorner::eNearLeftTop:
			return cuT( "near_left_top" );
		case FrustumCorner::eNearRightTop:
			return cuT( "near_right_top" );
		case FrustumCorner::eNearRightBottom:
			return cuT( "near_right_bottom" );
		default:
			CU_Failure( "Unsupported FrustumCorner" );
			return castor::cuEmptyString;
		}
	}

	castor::String getName( FrustumPlane value )
	{
		switch ( value )
		{
		case FrustumPlane::eNear:
			return cuT( "near" );
		case FrustumPlane::eFar:
			return cuT( "far" );
		case FrustumPlane::eLeft:
			return cuT( "left" );
		case FrustumPlane::eRight:
			return cuT( "right" );
		case FrustumPlane::eTop:
			return cuT( "top" );
		case FrustumPlane::eBottom:
			return cuT( "bottom" );
		default:
			CU_Failure( "Unsupported FrustumPlane" );
			return castor::cuEmptyString;
		}
	}

	castor::String getName( PickNodeType value )
	{
		switch ( value )
		{
		case PickNodeType::eNone:
			return cuT( "none" );
		case PickNodeType::eStatic:
			return cuT( "static" );
		case PickNodeType::eInstantiatedStatic:
			return cuT( "instantiated_static" );
		case PickNodeType::eSkinning:
			return cuT( "skinning" );
		case PickNodeType::eInstantiatedSkinning:
			return cuT( "instantiated_skinning" );
		case PickNodeType::eMorphing:
			return cuT( "morphing" );
		case PickNodeType::eBillboard:
			return cuT( "billboard" );
		default:
			CU_Failure( "Unsupported PickNodeType" );
			return castor::cuEmptyString;
		}
	}

	castor::String getName( TargetType value )
	{
		switch ( value )
		{
		case TargetType::eWindow:
			return cuT( "window" );
		case TargetType::eTexture:
			return cuT( "texture" );
		default:
			CU_Failure( "Unsupported TargetType" );
			return castor::cuEmptyString;
		}
	}

	castor::String getName( ViewportType value )
	{
		switch ( value )
		{
		case ViewportType::eOrtho:
			return cuT( "ortho" );
		case ViewportType::ePerspective:
			return cuT( "perspective" );
		case ViewportType::eFrustum:
			return cuT( "frustum" );
		default:
			CU_Failure( "Unsupported ViewportType" );
			return castor::cuEmptyString;
		}
	}

	TextureFlagsArray::const_iterator checkFlags( TextureFlagsArray const & flags, TextureFlag flag )
	{
		auto it = std::find_if( flags.begin()
			, flags.end()
			, [flag]( TextureFlagsId const & lookup )
			{
				return checkFlag( lookup.flags, flag );
			} );
		return it;
	}

	TextureFlags merge( TextureFlagsArray const & flags )
	{
		TextureFlags result = TextureFlag::eNone;

		for ( auto flag : flags )
		{
			result |= flag.flags;
		}

		return result;
	}

	bool operator<( PipelineFlags const & lhs, PipelineFlags const & rhs )
	{
		return lhs.alphaFunc < rhs.alphaFunc
			|| ( lhs.alphaFunc == rhs.alphaFunc
				&& ( lhs.topology < rhs.topology
					|| ( lhs.topology == rhs.topology
						&& ( lhs.colourBlendMode < rhs.colourBlendMode
							|| ( lhs.colourBlendMode == rhs.colourBlendMode
								&& ( lhs.alphaBlendMode < rhs.alphaBlendMode
									|| ( lhs.alphaBlendMode == rhs.alphaBlendMode
										&& ( lhs.textures.size() < rhs.textures.size()
											|| ( lhs.textures.size() == rhs.textures.size()
												&& ( lhs.texturesFlags < rhs.texturesFlags
													|| ( lhs.texturesFlags == rhs.texturesFlags
														&& ( lhs.heightMapIndex < rhs.heightMapIndex
															|| ( lhs.heightMapIndex == rhs.heightMapIndex
																&& ( lhs.programFlags < rhs.programFlags
																	|| ( lhs.programFlags == rhs.programFlags
																		&& ( lhs.passFlags < rhs.passFlags
																			|| ( lhs.passFlags == rhs.passFlags
																				&& lhs.sceneFlags < rhs.sceneFlags )
																			)
																		)
																	)
																)
															)
														)
													)
												)
											)
										)
									)
								)
							)
						)
					)
				);
	}

	RenderDevice const & getCurrentRenderDevice( RenderDevice const & obj )
	{
		return getCurrentRenderDevice( obj.renderSystem );
	}

	RenderDevice const & getCurrentRenderDevice( RenderSystem const & obj )
	{
		return obj.getCurrentRenderDevice();
	}

	RenderDevice const & getCurrentRenderDevice( Engine const & obj )
	{
		return getCurrentRenderDevice( *obj.getRenderSystem() );
	}

	RenderDevice const & getCurrentRenderDevice( Scene const & obj )
	{
		return getCurrentRenderDevice( *obj.getEngine() );
	}
}
