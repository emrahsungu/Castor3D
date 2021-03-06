/*
See LICENSE file in root folder
*/
#ifndef ___C3D_Voxelizer_H___
#define ___C3D_Voxelizer_H___

#include "Castor3D/Material/Texture/TextureUnit.hpp"
#include "Castor3D/Render/Technique/Voxelize/VoxelizePass.hpp"
#include "Castor3D/Shader/Ubos/MatrixUbo.hpp"

namespace castor3d
{
	class Voxelizer
	{
	public:
		/**
		 *\~english
		 *\param[in]	engine			The engine.
		 *\param[in]	size			The render dimensions.
		 *\param[in]	scene			The rendered scene.
		 *\~french
		 *\param[in]	engine			Le moteur.
		 *\param[in]	size			Les dimensions du rendu.
		 *\param[in]	scene			La scène rendue.
		 */
		C3D_API Voxelizer( Engine & engine
			, VkExtent3D const & size
			, Scene & scene
			, SceneCuller & culler
			, ashes::ImageView colourView );
		C3D_API ~Voxelizer();
		/**
		 *\~english
		 *\brief		Updates pass.
		 *\param[out]	info	Receives the render informations.
		 *\~french
		 *\brief		Met à jour la passe.
		 *\param[out]	info	Reçoit les informations de rendu.
		 */
		C3D_API void update( RenderQueueArray & queues );
		/**
		 *\~english
		 *\brief		Updates pass.
		 *\param[out]	info	Receives the render informations.
		 *\~french
		 *\brief		Met à jour la passe.
		 *\param[out]	info	Reçoit les informations de rendu.
		 */
		C3D_API void update( RenderInfo & info
			, castor::Point2f const & jitter );
		/**
		 *\~english
		 *\brief		Renders nodes.
		 *\param[in]	toWait	The semaphore from the previous render pass.
		 *\~french
		 *\brief		Dessine les noeuds.
		 *\param[in]	toWait	Le sémaphore de la passe de rendu précédente.
		 */
		C3D_API ashes::Semaphore const & render( ashes::Semaphore const & toWait );
		/**
		 *\copydoc		castor3d::RenderTechniquePass::accept
		 */
		C3D_API void accept( RenderTechniqueVisitor & visitor );

	private:
		Engine & m_engine;
		MatrixUbo m_matrixUbo;
		VkExtent3D m_size;
		TextureUnit m_result;
		VoxelizePass m_voxelizePass;
	};
}

#endif
