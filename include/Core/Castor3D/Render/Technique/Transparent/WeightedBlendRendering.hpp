/*
See LICENSE file in root folder
*/
#ifndef ___C3D_WeightedBlendRendering_H___
#define ___C3D_WeightedBlendRendering_H___

#include "TransparentModule.hpp"

#include "Castor3D/Render/Technique/Transparent/TransparentPass.hpp"
#include "Castor3D/Render/Technique/Transparent/TransparentPassResult.hpp"
#include "Castor3D/Render/Technique/Transparent/TransparentResolvePass.hpp"

namespace castor3d
{
	class WeightedBlendRendering
	{
	public:
		/**
		 *\~english
		 *\brief		Initialises deferred rendering related stuff.
		 *\param[in]	engine			The engine.
		 *\param[in]	transparentPass	The transparent nodes render pass.
		 *\param[in]	depthView		The target depth buffer.
		 *\param[in]	colourView		The target colour buffer.
		 *\param[in]	velocityTexture	The velocity buffer.
		 *\param[in]	size			The render dimensions.
		 *\param[in]	scene			The rendered scene.
		 *\param[in]	hdrConfigUbo	The HDR configuration UBO.
		 *\~french
		 *\brief		Initialise les données liées au deferred rendering.
		 *\param[in]	engine			Le moteur.
		 *\param[in]	transparentPass	La passe de rendu des noeuds transparents.
		 *\param[in]	depthView		Le tampon de profondeur cible.
		 *\param[in]	colourView		Le tampon de couleurs cible.
		 *\param[in]	velocityTexture	Le tampon de vélocité.
		 *\param[in]	size			Les dimensions du rendu.
		 *\param[in]	scene			La scène rendue.
		 *\param[in]	hdrConfigUbo	L'UBO de configuration HDR.
		 */
		WeightedBlendRendering( Engine & engine
			, TransparentPass & transparentPass
			, TextureUnit const & depthView
			, ashes::ImageView const & colourView
			, TextureUnit const & velocityTexture
			, castor::Size const & size
			, Scene const & scene
			, HdrConfigUbo & hdrConfigUbo
			, GpInfoUbo const & gpInfoUbo );
		/**
		 *\~english
		 *\brief		Renders opaque nodes.
		 *\param[out]	info		Receives the render informations.
		 *\param[in]	scene		The rendered scene.
		 *\param[in]	camera		The viewer camera.
		 *\param[in]	jitter		The jittering value.
		 *\~french
		 *\brief		Dessine les noeuds opaques.
		 *\param[out]	info		Reçoit les informations de rendu.
		 *\param[in]	scene		La scène rendue.
		 *\param[in]	camera		La caméra par laquelle la scène est rendue.
		 *\param[in]	jitter		La valeur de jittering.
		 */
		void update( RenderInfo & info
			, Scene const & scene
			, Camera const & camera
			, castor::Point2f const & jitter );
		/**
		 *\~english
		 *\brief		Renders opaque nodes.
		 *\param[in]	scene	The rendered scene.
		 *\param[in]	toWait	The semaphore to wait for.
		 *\~french
		 *\brief		Dessine les noeuds opaques.
		 *\param[in]	scene	La scène rendue.
		 *\param[in]	toWait	Le sémaphore à attendre.
		 */
		ashes::Semaphore const & render( Scene const & scene
			, ashes::Semaphore const & toWait );
		/**
		 *\~english
		 *\brief		Lists the pass' intermediate views.
		 *\~french
		 *\brief		Liste les vues intermédiaires de la passe.
		 */
		void listIntermediates( std::vector< IntermediateView > & intermediates )const;
		/**
		 *\copydoc		castor3d::RenderTechniquePass::accept
		 */
		void accept( RenderTechniqueVisitor & visitor );

	private:
		Engine & m_engine;
		TransparentPass & m_transparentPass;
		castor::Size m_size;
		TransparentPassResult m_transparentPassResult;
		TransparentResolvePass m_finalCombinePass;
	};
}

#endif
