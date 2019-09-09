/*
See LICENSE file in root folder
*/
#ifndef ___C3D_PREREQUISITES_H___
#define ___C3D_PREREQUISITES_H___

#undef RGB
#include <CastorUtils/CastorUtilsPrerequisites.hpp>

#if !defined( CU_PlatformWindows )
#	define C3D_API
#else
#	ifdef MemoryBarrier
#		undef MemoryBarrier
#	endif
#	if defined( Castor3D_EXPORTS )
#		define C3D_API __declspec( dllexport )
#	else
#		define C3D_API __declspec( dllimport )
#	endif
#endif

#include <CastorUtils/Design/Collection.hpp>
#include <CastorUtils/Design/FlagCombination.hpp>
#include <CastorUtils/Design/OwnedBy.hpp>
#include <CastorUtils/Math/Point.hpp>
#include <CastorUtils/Graphics/Size.hpp>
#include <CastorUtils/Math/Quaternion.hpp>
#include <CastorUtils/Math/SquareMatrix.hpp>
#include <CastorUtils/Miscellaneous/StringUtils.hpp>

#include <ShaderWriter/Source.hpp>
#include <ashespp/AshesPPPrerequisites.hpp>
#include <ashespp/Core/Device.hpp>

namespace castor3d
{
	static uint32_t constexpr InvalidIndex = ~( 0u );

	using castor::real;
	using castor::UInt32StrMap;
	using castor::UInt64StrMap;
	using castor::UInt32Array;
}

#include "Castor3D/Prerequisites/Castor3DPrerequisites_Animation.hpp"
#include "Castor3D/Prerequisites/Castor3DPrerequisites_Camera.hpp"
#include "Castor3D/Prerequisites/Castor3DPrerequisites_Geometry.hpp"
#include "Castor3D/Prerequisites/Castor3DPrerequisites_Light.hpp"
#include "Castor3D/Prerequisites/Castor3DPrerequisites_Material.hpp"
#include "Castor3D/Prerequisites/Castor3DPrerequisites_Overlay.hpp"
#include "Castor3D/Prerequisites/Castor3DPrerequisites_Scene.hpp"
#include "Castor3D/Prerequisites/Castor3DPrerequisites_Shader.hpp"
#include "Castor3D/Prerequisites/Castor3DPrerequisites_Render.hpp"
#include "Castor3D/Prerequisites/Castor3DPrerequisites_Event.hpp"

namespace castor3d
{
	castor::String const RENDERER_TYPE_UNDEFINED = cuT( "Undefined" );

	/**@name Importer */
	//@{

	class Importer;
	CU_DeclareSmartPtr( Importer );

	//@}
	/**@name General */
	//@{

	/*!
	\author 	Sylvain DOREMUS
	\version	0.1
	\date		09/02/2010
	\~english
	\brief		Movable object types enumerator
	\remark		There are four movable object types : camera, geometry, light and billboard.
	\~french
	\brief		Enumération des types de MovableObject
	\remark		Il y a quatre types d'objets déplaçables : caméra, géométrie, lumière et billboard
	*/
	enum class MovableType
		: uint8_t
	{
		eCamera,
		eGeometry,
		eLight,
		eBillboard,
		eParticleEmitter,
		CU_ScopedEnumBounds( eCamera )
	};
	/*!
	\author		Sylvain DOREMUS
	\version	0.6.1.0
	\date		19/10/2011
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
	/*!
	\author 	Sylvain DOREMUS
	\version	0.6.1.0
	\date		03/01/2011
	\~english
	\brief		Plugin types enumeration
	\~french
	\brief		Enumération des types de plug-ins
	*/
	enum class PluginType
		: uint8_t
	{
		//!\~english	Renderer plug-in.
		//!\~french		Plug-in de rendu.
		eRenderer,
		//!\~english	Importer plug-in.
		//!\~french		Plug-in d'importation.
		eImporter,
		//!\~english	Mesh Divider plug-in.
		//!\~french		Plug-in de subdivision de maillage.
		eDivider,
		//!\~english	Procedural generator.
		//!\~french		Plug-in de génération procédurale.
		eGenerator,
		//!\~english	Render technique plug-in.
		//!\~french		Plug-in de technique de rendu.
		eTechnique,
		//!\~english	Tone mapping plug-in.
		//!\~french		Plug-in d'effet de mappage de tons.
		eToneMapping,
		//!\~english	Post effect plug-in.
		//!\~french		Plug-in d'effet post-rendu.
		ePostEffect,
		//!\~english	Generic plug-in.
		//!\~french		Plug-in générique.
		eGeneric,
		//!\~english	Particle plug-in.
		//!\~french		Plug-in de particule.
		eParticle,
		CU_ScopedEnumBounds( eRenderer )
	};
	/*!
	\author		Sylvain DOREMUS
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
	/*!
	\author 	Sylvain DOREMUS
	\date 		10/06/2013
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
	/*!
	\author 	Sylvain DOREMUS
	\date 		10/06/2013
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
	/*!
	\author 	Sylvain DOREMUS
	\~english
	\brief		Frame Event Type enumeration
	\~french
	\brief		Enumération des types d'évènement de frame
	*/
	enum class EventType
		: uint8_t
	{
		//!\~english	This kind of event happens before any render, device context is active (so be fast !!).
		//!\~french		Ce type d'évènement est traité avant le rendu, le contexte de rendu est actif (donc soyez rapide !!)
		ePreRender,
		//!\~english	This kind of event happens after the render, before buffers' swap.
		//!\~french		Ce type d'évènement est traité après le rendu, avant l'échange des tampons.
		eQueueRender,
		//!\~english	This kind of event happens after the buffer' swap.
		//!\~french		Ce type d'évènement est traité après l'échange des tampons.
		ePostRender,
		CU_ScopedEnumBounds( ePreRender )
	};

	template< class TParsed >
	class BinaryWriter;
	template< class TParsed >
	class BinaryParser;

	class DebugOverlays;
	class Engine;
	class Plugin;
	class RendererPlugin;
	class ImporterPlugin;
	class DividerPlugin;
	class GenericPlugin;
	class PostFxPlugin;
	class ToneMappingPlugin;
	class ParticlePlugin;
	class FrameEvent;
	class FrameListener;
	class Version;
	class Parameters;
	class PickingPass;
	class TransformFeedback;
	class ComputePipeline;
	struct SsaoConfig;
	class RenderPassTimer;
	class GaussianBlur;

	CU_DeclareSmartPtr( Engine );
	CU_DeclareSmartPtr( Plugin );
	CU_DeclareSmartPtr( RendererPlugin );
	CU_DeclareSmartPtr( ImporterPlugin );
	CU_DeclareSmartPtr( DividerPlugin );
	CU_DeclareSmartPtr( GenericPlugin );
	CU_DeclareSmartPtr( PostFxPlugin );
	CU_DeclareSmartPtr( ParticlePlugin );
	CU_DeclareSmartPtr( FrameEvent );
	CU_DeclareSmartPtr( FrameListener );
	CU_DeclareSmartPtr( PickingPass );
	CU_DeclareSmartPtr( TransformFeedback );
	CU_DeclareSmartPtr( ComputePipeline );
	CU_DeclareSmartPtr( RenderPassTimer );
	CU_DeclareSmartPtr( GaussianBlur );

	C3D_API castor::Matrix4x4f convert( std::array< float, 16 > const & value );
	C3D_API VkClearColorValue convert( castor::RgbaColour const & value );
	C3D_API castor::RgbaColour convert( VkClearColorValue const & value );

	using ParticleFactory = castor::Factory< CpuParticleSystem, castor::String, CpuParticleSystemUPtr, std::function< CpuParticleSystemUPtr( ParticleSystem & ) > >;

	/*!
	*\author
	*	Sylvain DOREMUS
	*\date
	*	04/02/2016
	*\version
	*	0.8.0
	*\~english
	*\brief
	*	Helper structure to specialise a cache behaviour.
	*\remarks
	*	Must hold:
	*	<ul>
	*	<li>Name: The element type name.</li>
	*	<li>Producer: The element creation function prototype.</li>
	*	<li>Merger: The prototype of the function use to merge a cache element into another cache.</li>
	*	</ul>
	*\~french
	*\brief
	*	Structure permettant de spécialiser le comportement d'un cache.
	*\remarks
	*	Doit contenir:
	*	<ul>
	*	<li>Name: Le nom du type d'élément.</li>
	*	<li>Producer: Le prototype de la fonction de création d'un élément.</li>
	*	<li>Merger: Le prototype de la fonction pour fusionner un élément d'un cache dans un autre cache.</li>
	*	</ul>
	*/
	template< typename ElementType, typename KeyType >
	struct CacheTraits;

	template< typename ElementType, typename KeyType >
	class CacheBase;

	template< typename ElementType, typename KeyType >
	class Cache;

	template< typename ElementType, typename KeyType >
	struct ElementProducer;

	template< typename ElementType >
	using ElementInitialiser = std::function< void( std::shared_ptr< ElementType > ) >;

	template< typename ElementType >
	using ElementCleaner = std::function< void( std::shared_ptr< ElementType > ) >;

	template< typename ElementType, typename KeyType >
	using ElementMerger = std::function< void( CacheBase< ElementType, KeyType > const &, castor::Collection< ElementType, KeyType > &, std::shared_ptr< ElementType > ) >;

	class ShaderProgramCache;
	class RenderTargetCache;

	using ListenerCache = Cache< FrameListener, castor::String >;
	using MeshCache = Cache< Mesh, castor::String >;
	using SamplerCache = Cache< Sampler, castor::String >;
	using SceneCache = Cache< Scene, castor::String >;
	using RenderTechniqueCache = Cache< RenderTechnique, castor::String >;
	using RenderWindowCache = Cache< RenderWindow, castor::String >;

	CU_DeclareSmartPtr( ListenerCache );
	CU_DeclareSmartPtr( MeshCache );
	CU_DeclareSmartPtr( SceneCache );
	CU_DeclareSmartPtr( SamplerCache );
	CU_DeclareSmartPtr( ShaderProgramCache );
	CU_DeclareSmartPtr( RenderTargetCache );
	CU_DeclareSmartPtr( RenderTechniqueCache );
	CU_DeclareSmartPtr( RenderWindowCache );

	/*!
	*\author
	*	Sylvain DOREMUS
	*\date
	*	04/02/2016
	*\version
	*	0.8.0
	*\~english
	*\brief
	*	Helper structure to specialise a cache behaviour.
	*\remarks
	*	Must hold:
	*	<ul>
	*	<li>Name: The element type name.</li>
	*	<li>Producer: The element creation function prototype.</li>
	*	<li>Merger: The prototype of the function use to merge a cache element into another cache.</li>
	*	</ul>
	*\~french
	*\brief
	*	Structure permettant de spécialiser le comportement d'un cache.
	*\remarks
	*	Doit contenir:
	*	<ul>
	*	<li>Name: Le nom du type d'élément.</li>
	*	<li>Producer: Le prototype de la fonction de création d'un élément.</li>
	*	<li>Merger: Le prototype de la fonction pour fusionner un élément d'un cache dans un autre cache.</li>
	*	</ul>
	*/
	template< typename ElementType, typename KeyType >
	struct ObjectCacheTraits;

	template< typename ElementType >
	using ElementAttacher = std::function< void( std::shared_ptr< ElementType >, SceneNodeSPtr, SceneNodeSPtr, SceneNodeSPtr, SceneNodeSPtr ) >;

	template< typename ElementType >
	using ElementDetacher = std::function< void( std::shared_ptr< ElementType > ) >;

	template< typename ElementType, typename KeyType >
	class ObjectCacheBase;

	template< typename ElementType, typename KeyType >
	class ObjectCache;

	using CameraCache = ObjectCache< Camera, castor::String >;

	CU_DeclareSmartPtr( CameraCache );

	template< typename ResourceType, typename CacheType, EventType EventType >
	class CacheView;

	//! real array
	CU_DeclareVector( real, Real );
	//! RenderWindow pointer array
	CU_DeclareVector( RenderWindowSPtr, RenderWindowPtr );
	//! FrameEvent pointer array
	CU_DeclareVector( FrameEventUPtr, FrameEventPtr );
	//! FrameListener pointer map, sorted by name
	CU_DeclareMap( castor::String, FrameListenerSPtr, FrameListenerPtrStr );
	//! RenderWindow pointer map, sorted by index
	CU_DeclareMap( uint32_t, RenderWindowSPtr, RenderWindow );
	//! Plugin map, sorted by name
	CU_DeclareMap( castor::String, PluginSPtr, PluginStr );
	CU_DeclareMap( int, castor::String, StrInt );
	CU_DeclareVector( PostEffectSPtr, PostEffectPtr );
	CU_DeclareVector( BillboardListSPtr, BillboardList );

	CU_DeclareMap( castor::String, RendererPluginSPtr, RendererPtr );
	CU_DeclareArray( PluginStrMap, PluginType::eCount, PluginStrMap );
	CU_DeclareMap( castor::Path, castor::DynamicLibrarySPtr, DynamicLibraryPtrPath );
	CU_DeclareArray( DynamicLibraryPtrPathMap, PluginType::eCount, DynamicLibraryPtrPathMap );
	CU_DeclareMap( castor::Path, PluginType, PluginTypePath );
	CU_DeclareMap( castor::String, BillboardListSPtr, BillboardListStr );

	typedef std::map< castor::String, RenderWindowSPtr > WindowPtrStrMap;

	//@}

#define MAKE_CACHE_NAME( className )\
	Cache< className, castor::String >

#define DECLARE_CACHE_MEMBER( memberName, className )\
	public:\
		inline MAKE_CACHE_NAME( className ) & get##className##Cache()\
		{\
			return *m_##memberName##Cache;\
		}\
		inline MAKE_CACHE_NAME( className ) const & get##className##Cache()const\
		{\
			return *m_##memberName##Cache;\
		}\
	private:\
		std::unique_ptr< MAKE_CACHE_NAME( className ) > m_##memberName##Cache

#define DECLARE_NAMED_CACHE_MEMBER( memberName, className )\
	public:\
		inline className##Cache & get##className##Cache()\
		{\
			return *m_##memberName##Cache;\
		}\
		inline className##Cache const & get##className##Cache()const\
		{\
			return *m_##memberName##Cache;\
		}\
	private:\
		std::unique_ptr< className##Cache > m_##memberName##Cache

#define MAKE_OBJECT_CACHE_NAME( className )\
	ObjectCache< className, castor::String >

#define DECLARE_OBJECT_CACHE_MEMBER( memberName, className )\
	public:\
		inline MAKE_OBJECT_CACHE_NAME( className ) & get##className##Cache()\
		{\
			return *m_##memberName##Cache;\
		}\
		inline MAKE_OBJECT_CACHE_NAME( className ) const & get##className##Cache()const\
		{\
			return *m_##memberName##Cache;\
		}\
	private:\
		castor::Connection< MAKE_OBJECT_CACHE_NAME( className )::OnChanged > m_on##className##Changed;\
		std::unique_ptr< MAKE_OBJECT_CACHE_NAME( className ) > m_##memberName##Cache

#define DECLARE_CACHE_VIEW_MEMBER( memberName, className, eventType )\
	public:\
		inline CacheView< className, MAKE_CACHE_NAME( className ), eventType > & get##className##View()\
		{\
			return *m_##memberName##CacheView;\
		}\
		inline CacheView< className, MAKE_CACHE_NAME( className ), eventType > const & get##className##View()const\
		{\
			return *m_##memberName##CacheView;\
		}\
	private:\
		std::unique_ptr< CacheView< className, MAKE_CACHE_NAME( className ), eventType > > m_##memberName##CacheView

#define DECLARE_CACHE_VIEW_MEMBER_CU( memberName, className, eventType )\
	public:\
		inline CacheView< castor::className, castor::className##Cache, eventType > & get##className##View()\
		{\
			return *m_##memberName##CacheView;\
		}\
		inline CacheView< castor::className, castor::className##Cache, eventType > const & get##className##View()const\
		{\
			return *m_##memberName##CacheView;\
		}\
	private:\
		std::unique_ptr< CacheView< castor::className, castor::className##Cache, eventType > > m_##memberName##CacheView

#define DECLARE_CACHE_VIEW_MEMBER_EX( memberName, mgrName, className, eventType )\
	public:\
		inline CacheView< MAKE_CACHE_NAME( className ), mgrName##Cache, eventType > & get##className##View()\
		{\
			return *m_##memberName##CacheView;\
		}\
		inline CacheView< MAKE_CACHE_NAME( className ), mgrName##Cache, eventType > const & get##className##View()const\
		{\
			return *m_##memberName##CacheView;\
		}\
	private:\
		std::unique_ptr< CacheView< MAKE_CACHE_NAME( className ), mgrName##Cache, eventType > > m_##memberName##CacheView
}

CU_DeclareExportedOwnedBy( C3D_API, castor3d::Engine, Engine )
CU_DeclareExportedOwnedBy( C3D_API, castor3d::RenderSystem, RenderSystem )
CU_DeclareExportedOwnedBy( C3D_API, castor3d::Scene, Scene )
CU_DeclareExportedOwnedBy( C3D_API, castor3d::RenderDevice, RenderDevice )

namespace castor3d
{
	C3D_API RenderDevice const & getCurrentRenderDevice( RenderDevice const & obj );
	C3D_API RenderDevice const & getCurrentRenderDevice( RenderSystem const & obj );
	C3D_API RenderDevice const & getCurrentRenderDevice( Engine const & obj );
	C3D_API RenderDevice const & getCurrentRenderDevice( Scene const & obj );

	inline RenderDevice const & getCurrentRenderDevice( castor::OwnedBy< RenderSystem > const & obj )
	{
		return getCurrentRenderDevice( *obj.getRenderSystem() );
	}

	inline RenderDevice const & getCurrentRenderDevice( castor::OwnedBy< Engine > const & obj )
	{
		return getCurrentRenderDevice( *obj.getEngine() );
	}

	inline RenderDevice const & getCurrentRenderDevice( castor::OwnedBy< Scene > const & obj )
	{
		return getCurrentRenderDevice( *obj.getScene() );
	}
}

#endif