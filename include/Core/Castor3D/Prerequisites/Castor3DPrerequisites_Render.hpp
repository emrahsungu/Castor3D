/*
See LICENSE file in root folder
*/
#ifndef ___C3D_PREREQUISITES_RENDER_H___
#define ___C3D_PREREQUISITES_RENDER_H___

#include "Castor3D/Texture/TextureConfiguration.hpp"

#include <CastorUtils/Design/Factory.hpp>
#include <CastorUtils/Graphics/Position.hpp>
#include <CastorUtils/Graphics/Size.hpp>

namespace castor3d
{
	/**@name Render */
	//@{

	inline VkClearValue makeClearValue( float depth, uint32_t stencil = 0u )
	{
		return ashes::makeClearValue( VkClearDepthStencilValue{ depth, stencil } );
	}

	inline VkClearValue makeClearValue( float r, float g, float b, float a = 1.0f )
	{
		return ashes::makeClearValue( { r, g, b, a } );
	}

	static VkClearValue const defaultClearDepthStencil{ makeClearValue( 1.0f, 0u ) };
	static VkClearValue const opaqueBlackClearColor{ makeClearValue( 0.0f, 0.0f, 0.0f, 1.0f ) };
	static VkClearValue const transparentBlackClearColor{ makeClearValue( 0.0f, 0.0f, 0.0f, 0.0f ) };
	static VkClearValue const opaqueWhiteClearColor{ makeClearValue( 1.0f, 1.0f, 1.0f, 1.0f ) };
	static VkClearValue const transparentWhiteClearColor{ makeClearValue( 1.0f, 1.0f, 1.0f, 0.0f ) };
	static VkColorComponentFlags const defaultColorWriteMask{ VK_COLOR_COMPONENT_R_BIT| VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT };

	struct NonTexturedQuad
	{
		struct Vertex
		{
			castor::Point2f position;
		};

		Vertex vertex[6];
	};

	struct TexturedQuad
	{
		struct Vertex
		{
			castor::Point2f position;
			castor::Point2f texture;
		};

		Vertex vertex[6];
	};

	struct NonTexturedCube
	{
		struct Quad
		{
			struct Vertex
			{
				castor::Point3f position;
			};

			Vertex vertex[6];
		};

		Quad faces[6];
	};

	struct TexturedCube
	{
		struct Quad
		{
			struct Vertex
			{
				castor::Point3f position;
				castor::Point2f texture;
			};

			Vertex vertex[6];
		};

		Quad faces[6];
	};
	struct GeometryBuffers
	{
		ashes::BufferCRefArray vbo;
		ashes::UInt64Array vboOffsets;
		uint32_t vtxCount;
		ashes::PipelineVertexInputStateCreateInfoCRefArray layouts;
		ashes::BufferBase const * ibo{ nullptr };
		uint64_t iboOffset;
		uint32_t idxCount;
	};
	/*!
	\version	0.6.1.0
	\~english
	\brief		RenderTarget supported types
	\~french
	\brief		Types de RenderTarget supportés
	*/
	enum class TargetType
		: uint8_t
	{
		eWindow,
		eTexture,
		CU_ScopedEnumBounds( eWindow )
	};
	castor::String getName( TargetType value );
	/*!
	\version	0.7.0
	\~english
	\brief		Frustum view planes enumeration
	\~french
	\brief		Enumération des plans du frustum de vue
	*/
	enum class FrustumCorner
	{
		//!\~english	Far left bottom corner.
		//!\~french		Coin éloigné bas gauche.
		eFarLeftBottom,
		//!\~english	Far left top corner.
		//!\~french		Coin éloigné haut gauche.
		eFarLeftTop,
		//!\~english	Far right top corner.
		//!\~french		Coin éloigné haut droit.
		eFarRightTop,
		//!\~english	Far right bottom corner.
		//!\~french		Coin éloigné bas droit.
		eFarRightBottom,
		//!\~english	Near left bottom corner.
		//!\~french		Coin proche bas gauche.
		eNearLeftBottom,
		//!\~english	Near left top corner.
		//!\~french		Coin proche haut gauche.
		eNearLeftTop,
		//!\~english	Near right top corner.
		//!\~french		Coin proche haut droit.
		eNearRightTop,
		//!\~english	Near right bottom corner.
		//!\~french		Coin proche bas droit.
		eNearRightBottom,
		CU_ScopedEnumBounds( eFarLeftBottom )
	};
	castor::String getName( FrustumCorner value );
	/*!
	\version	0.7.0
	\~english
	\brief		Frustum view planes enumeration
	\~french
	\brief		Enumération des plans du frustum de vue
	*/
	enum class FrustumPlane
	{
		//!\~english	Near plane.
		//!\~french		Plan proche.
		eNear,
		//!\~english	Far plane.
		//!\~french		Plan éloigné.
		eFar,
		//!\~english	Left plane.
		//!\~french		Plan gauche.
		eLeft,
		//!\~english	Right plane.
		//!\~french		Plan droit.
		eRight,
		//!\~english	Top plane.
		//!\~french		Plan haut.
		eTop,
		//!\~english	Bottom plane.
		//!\~french		Plan bas.
		eBottom,
		CU_ScopedEnumBounds( eNear )
	};
	castor::String getName( FrustumPlane value );
	/*!
	\~english
	\brief		The  viewport projection types enumeration
	\~french
	\brief		Enumération des types de projection de viewport
	*/
	enum class ViewportType
		: uint8_t
	{
		eOrtho,
		ePerspective,
		eFrustum,
		CU_ScopedEnumBounds( eOrtho )
	};
	castor::String getName( ViewportType value );
	/*!
	\version	0.11.0
	\~english
	\brief		The picking node types.
	\~french
	\brief		Les types de noeud de picking.
	*/
	enum class PickNodeType
		: uint8_t
	{
		eNone,
		eStatic,
		eInstantiatedStatic,
		eSkinning,
		eInstantiatedSkinning,
		eMorphing,
		eBillboard
	};
	castor::String getName( PickNodeType value );
	/*!
	\version	0.6.1.0
	\~english
	\brief		Element usage enumeration
	\~french
	\brief		Enumération des utilisations d'éléments de tampon
	*/
	enum class ElementUsage
		: uint32_t
	{
		//! Position coords
		eUnknown = 0x000,
		//! Position coords
		ePosition = 0x001,
		//! Normal coords
		eNormal = 0x002,
		//! Tangent coords
		eTangent = 0x004,
		//! Bitangent coords
		eBitangent = 0x008,
		//! Diffuse colour
		eDiffuse = 0x010,
		//! Texture coordinates
		eTexCoords = 0x020,
		//! Bone IDs 0
		eBoneIds0 = 0x040,
		//! Bone IDs 1
		eBoneIds1 = 0x080,
		//! Bone weights 0
		eBoneWeights0 = 0x100,
		//! Bone weights 1
		eBoneWeights1 = 0x200,
		//! Instantiation matrix
		eTransform = 0x400,
		//! Instantiation material index
		eMatIndex = 0x800,
	};
	CU_ImplementFlags( ElementUsage )
	castor::String getName( ElementUsage value );
	/*!
	\~english
	\brief		Texture channels flags.
	\~french
	\brief		Indicateurs des canaux de texture.
	*/
	enum class TextureFlag
		: uint16_t
	{
		//!\~english	No texture.
		//!\~french		Pas de texture.
		eNone = 0x0000,
		//!\~english	Diffuse map.
		//!\~french		Map de diffuse.
		eDiffuse = 0x0001,
		//!\~english	Albedo map.
		//!\~french		Map d'albedo.
		eAlbedo = eDiffuse,
		//!\~english	Normal map.
		//!\~french		Map de normales.
		eNormal = 0x0002,
		//!\~english	Opacity map.
		//!\~french		Map d'opacité.
		eOpacity = 0x0004,
		//!\~english	Specular map.
		//!\~french		Map de spéculaire.
		eSpecular = 0x0008,
		//!\~english	Metalness map.
		//!\~french		Map de metalness.
		eMetalness = eSpecular,
		//!\~english	Height map.
		//!\~french		Map de hauteur.
		eHeight = 0x0010,
		//!\~english	Glossiness map.
		//!\~french		Map de glossiness.
		eGlossiness = 0x0020,
		//!\~english	Shininess map.
		//!\~french		Map de shininess.
		eShininess = eGlossiness,
		//!\~english	Roughness map.
		//!\~french		Map de roughness.
		eRoughness = eGlossiness,
		//!\~english	Emissive map.
		//!\~french		Map d'émissive.
		eEmissive = 0x040,
		//!\~english	Reflection map.
		//!\~french		Map de réflexion.
		eReflection = 0x0080,
		//!\~english	Refraction map.
		//!\~french		Map de réfraction.
		eRefraction = 0x0100,
		//!\~english	Occlusion map.
		//!\~french		Map d'occlusion.
		eOcclusion = 0x0200,
		//!\~english	Light transmittance map.
		//!\~french		Map de transmission de lumière.
		eTransmittance = 0x0400,
		//!\~english	Mask for all the texture channels except for opacity and colour.
		//!\~french		Masque pour les canaux de texture sauf l'opacité et la couleur.
		eAllButColourAndOpacity = 0x07FA,
		//!\~english	Mask for all the texture channels except for opacity.
		//!\~french		Masque pour les canaux de texture sauf l'opacité.
		eAllButOpacity = eAllButColourAndOpacity | eDiffuse,
		//!\~english	Mask for all the texture channels.
		//!\~french		Masque pour les canaux de texture.
		eAll = eAllButOpacity | eOpacity,
	};
	CU_ImplementFlags( TextureFlag )
	castor::String getName( TextureFlag value
		, MaterialType material );
	/*!
	\version	0.9.0
	\~english
	\brief		Pipeline flags.
	\~french
	\brief		Indicateurs de pipeline.
	*/
	struct PipelineFlags
	{
		BlendMode colourBlendMode{ BlendMode::eNoBlend };
		BlendMode alphaBlendMode{ BlendMode::eNoBlend };
		PassFlags passFlags{ PassFlag::eNone };
		TextureFlags textures{ TextureFlag::eNone };
		uint32_t texturesCount{ 0u };
		uint32_t heightMapIndex{ InvalidIndex };
		ProgramFlags programFlags{ ProgramFlag::eNone };
		SceneFlags sceneFlags{ SceneFlag::eNone };
		VkPrimitiveTopology topology{ VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST };
		VkCompareOp alphaFunc{ VK_COMPARE_OP_ALWAYS };
	};
	C3D_API bool operator<( PipelineFlags const & lhs, PipelineFlags const & rhs );

	template< typename T >
	class CpuBuffer;

	struct RenderDevice;

	class Context;
	class EnvironmentMap;
	class EnvironmentMapPass;
	class GeometryPassResult;
	class GpuInformations;
	class HdrConfig;
	class IblTextures;
	class PostEffect;
	class RenderColourCubeLayerToTexture;
	class RenderColourCubeToTexture;
	class RenderColourLayerToTexture;
	class RenderColourToCube;
	class RenderQuad;
	class RenderDepthCubeLayerToTexture;
	class RenderDepthCubeToTexture;
	class RenderDepthLayerToTexture;
	class RenderDepthToCube;
	class RenderLoop;
	class RenderLoopAsync;
	class RenderLoopSync;
	class RenderPass;
	class RenderPipeline;
	class RenderQueue;
	class RenderSystem;
	class RenderTarget;
	class RenderTechnique;
	class RenderWindow;
	class SceneCuller;
	class ShadowMap;
	class ShadowMapPass;
	class TextureProjection;
	class ToneMapping;
	struct BillboardRenderNode;
	struct ParticleElementDeclaration;
	struct DistanceRenderNodeBase;
	struct MorphingRenderNode;
	struct PassRenderNode;
	struct PassRenderNodeUniforms;
	struct RenderInfo;
	struct SceneRenderNode;
	struct SkinningRenderNode;
	struct StaticRenderNode;

	CU_DeclareSmartPtr( ParticleElementDeclaration );
	CU_DeclareSmartPtr( Context );
	CU_DeclareSmartPtr( EnvironmentMap );
	CU_DeclareSmartPtr( EnvironmentMapPass );
	CU_DeclareSmartPtr( IblTextures );
	CU_DeclareSmartPtr( PostEffect );
	CU_DeclareSmartPtr( RenderColourCubeLayerToTexture );
	CU_DeclareSmartPtr( RenderColourCubeToTexture );
	CU_DeclareSmartPtr( RenderColourLayerToTexture );
	CU_DeclareSmartPtr( RenderColourToCube );
	CU_DeclareSmartPtr( RenderQuad );
	CU_DeclareSmartPtr( RenderDepthCubeLayerToTexture );
	CU_DeclareSmartPtr( RenderDepthCubeToTexture );
	CU_DeclareSmartPtr( RenderDepthLayerToTexture );
	CU_DeclareSmartPtr( RenderDepthToCube );
	CU_DeclareSmartPtr( RenderDevice );
	CU_DeclareSmartPtr( RenderLoop );
	CU_DeclareSmartPtr( RenderPipeline );
	CU_DeclareSmartPtr( RenderSystem );
	CU_DeclareSmartPtr( RenderTarget );
	CU_DeclareSmartPtr( RenderTechnique );
	CU_DeclareSmartPtr( RenderWindow );
	CU_DeclareSmartPtr( SceneCuller );
	CU_DeclareSmartPtr( ShadowMap );
	CU_DeclareSmartPtr( ShadowMapPass );
	CU_DeclareSmartPtr( TextureProjection );
	CU_DeclareSmartPtr( ToneMapping );

	CU_DeclareMap( RenderWindow *, ContextSPtr, ContextPtr );
	CU_DeclareMap( std::thread::id, ContextPtrMap, ContextPtrMapId );
	using RenderQueueArray = std::vector< std::reference_wrapper< RenderQueue > >;
	using ShadowMapRefIds = std::pair< std::reference_wrapper< ShadowMap >, UInt32Array >;
	using ShadowMapRefArray = std::vector< ShadowMapRefIds >;
	using ShadowMapLightTypeArray = std::array< ShadowMapRefArray, size_t( LightType::eCount ) >;

	inline VkDescriptorSetLayoutBinding makeDescriptorSetLayoutBinding( uint32_t binding
		, VkDescriptorType descriptorType
		, VkShaderStageFlags stageFlags
		, uint32_t descriptorCount = 1u
		, VkSampler const * pImmutableSamplers = nullptr )
	{
		return
		{
			binding,
			descriptorType,
			descriptorCount,
			stageFlags,
			pImmutableSamplers,
		};
	}

	inline VkExtent2D makeExtent2D( castor::Coords2ui const & size )
	{
		return VkExtent2D
		{
			size[0],
			size[1],
		};
	}

	inline VkExtent2D makeExtent2D( castor::Point2ui const & size )
	{
		return VkExtent2D
		{
			size[0],
			size[1],
		};
	}

	inline VkExtent3D makeExtent3D( castor::Coords2ui const & size )
	{
		return VkExtent3D
		{
			size[0],
			size[1],
			1u,
		};
	}

	inline VkExtent3D makeExtent3D( castor::Point2ui const & size )
	{
		return VkExtent3D
		{
			size[0],
			size[1],
			1u,
		};
	}

	inline VkOffset2D makeOffset2D( castor::Coords2i const & pos )
	{
		return VkOffset2D
		{
			pos[0],
			pos[1],
		};
	}

	inline VkOffset2D makeOffset2D( castor::Point2i const & pos )
	{
		return VkOffset2D
		{
			pos[0],
			pos[1],
		};
	}

	inline VkOffset3D makeOffset3D( castor::Coords2i const & pos )
	{
		return VkOffset3D
		{
			pos[0],
			pos[1],
			0u,
		};
	}

	inline VkOffset3D makeOffset3D( castor::Point2i const & pos )
	{
		return VkOffset3D
		{
			pos[0],
			pos[1],
			0u,
		};
	}

	inline VkViewport makeViewport( castor::Coords2ui const & size
		, float zMin = 0.0f
		, float zMax = 1.0f )
	{
		return ashes::makeViewport( {}
			, makeExtent2D( size )
			, zMin
			, zMax );
	}

	inline VkViewport makeViewport( castor::Point2ui const & size
		, float zMin = 0.0f
		, float zMax = 1.0f )
	{
		return ashes::makeViewport( {}
			, makeExtent2D( size )
			, zMin
			, zMax );
	}

	inline VkViewport makeViewport( castor::Coords2i const & pos
		, castor::Coords2ui const & size
		, float zMin = 0.0f
		, float zMax = 1.0f )
	{
		return ashes::makeViewport( makeOffset2D( pos )
			, makeExtent2D( size )
			, zMin
			, zMax );
	}

	inline VkViewport makeViewport( castor::Point2i const & pos
		, castor::Coords2ui const & size
		, float zMin = 0.0f
		, float zMax = 1.0f )
	{
		return ashes::makeViewport( makeOffset2D( pos )
			, makeExtent2D( size )
			, zMin
			, zMax );
	}

	inline VkViewport makeViewport( castor::Coords2i const & pos
		, castor::Point2ui const & size
		, float zMin = 0.0f
		, float zMax = 1.0f )
	{
		return ashes::makeViewport( makeOffset2D( pos )
			, makeExtent2D( size )
			, zMin
			, zMax );
	}

	inline VkViewport makeViewport( castor::Point2i const & pos
		, castor::Point2ui const & size
		, float zMin = 0.0f
		, float zMax = 1.0f )
	{
		return ashes::makeViewport( makeOffset2D( pos )
			, makeExtent2D( size )
			, zMin
			, zMax );
	}

	inline VkRect2D makeScissor( castor::Coords2ui const & size )
	{
		return ashes::makeScissor( {}
		, makeExtent2D( size ) );
	}

	inline VkRect2D makeScissor( castor::Point2ui const & size )
	{
		return ashes::makeScissor( {}
		, makeExtent2D( size ) );
	}

	inline VkRect2D makeScissor( castor::Coords2i const & pos
		, castor::Coords2ui const & size )
	{
		return ashes::makeScissor( makeOffset2D( pos )
			, makeExtent2D( size ) );
	}

	inline VkRect2D makeScissor( castor::Point2i const & pos
		, castor::Coords2ui const & size )
	{
		return ashes::makeScissor( makeOffset2D( pos )
			, makeExtent2D( size ) );
	}

	inline VkRect2D makeScissor( castor::Coords2i const & pos
		, castor::Point2ui const & size )
	{
		return ashes::makeScissor( makeOffset2D( pos )
			, makeExtent2D( size ) );
	}

	inline VkRect2D makeScissor( castor::Point2i const & pos
		, castor::Point2ui const & size )
	{
		return ashes::makeScissor( makeOffset2D( pos )
			, makeExtent2D( size ) );
	}

	//@}
}

#endif
