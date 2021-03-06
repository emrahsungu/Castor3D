/*
See LICENSE file in root folder
*/
#ifndef ___C3D_DIRECTIONAL_LIGHT_H___
#define ___C3D_DIRECTIONAL_LIGHT_H___

#include "Castor3D/Scene/Light/LightCategory.hpp"

#include <CastorUtils/Math/SquareMatrix.hpp>

namespace castor3d
{
	struct DirectionalLightCascade
	{
		castor::Matrix4x4f viewMatrix;
		castor::Matrix4x4f projMatrix;
		castor::Matrix4x4f viewProjMatrix;
		castor::Point2f splitDepthScale;
	};

	C3D_API bool operator==( DirectionalLightCascade const & lhs
		, DirectionalLightCascade  const & rhs );

	inline bool operator!=( DirectionalLightCascade const & lhs
		, DirectionalLightCascade  const & rhs )
	{
		return !( lhs == rhs );
	}

	class DirectionalLight
		: public LightCategory
	{
	public:
		using Cascade = DirectionalLightCascade;
		/**
		\author 	Sylvain DOREMUS
		\date 		14/02/2010
		\~english
		\brief		DirectionalLight loader
		\~french
		\brief		Loader de DirectionalLight
		*/
		class TextWriter
			: public LightCategory::TextWriter
		{
		public:
			/**
			 *\~english
			 *\brief		Constructor
			 *\~french
			 *\brief		Constructeur
			 */
			C3D_API explicit TextWriter( castor::String const & tabs, DirectionalLight const * category = nullptr );
			/**
			 *\~english
			 *\brief		Writes a light into a text file
			 *\param[in]	file	The file to save the cameras in
			 *\param[in]	light	The light to save
			 *\~french
			 *\brief		Ecrit une lumière dans un fichier texte
			 *\param[in]	file	Le fichier
			 *\param[in]	light	La lumière
			 */
			C3D_API bool operator()( DirectionalLight const & light, castor::TextFile & file );
			/**
			 *\copydoc		castor3d::LightCategory::TextWriter::writeInto
			 */
			C3D_API bool writeInto( castor::TextFile & file )override;

		private:
			DirectionalLight const * m_category;
		};

	private:
		friend class Scene;

	private:
		/**
		 *\~english
		 *\brief		Constructor.
		 *\param[in]	light	The parent Light.
		 *\~french
		 *\brief		Constructeur.
		 *\param[in]	light	La Light parente.
		 */
		C3D_API explicit DirectionalLight( Light & light );

	public:
		/**
		 *\~english
		 *\brief		Destructor
		 *\~french
		 *\brief		Destructeur
		 */
		C3D_API ~DirectionalLight();
		/**
		 *\~english
		 *\brief		Creation function, used by Factory.
		 *\return		A light source.
		 *\~french
		 *\brief		Fonction de création utilisée par Factory.
		 *\return		Une source lumineuse.
		 */
		C3D_API static LightCategoryUPtr create( Light & light );
		/**
		 *\copydoc		castor3d::LightCategory::update
		 */
		C3D_API void update()override;
		/**
		 *\~english
		 *\brief		Updates the shadow cascades informations.
		 *\param[in]	sceneCamera		The viewer camera.
		 *\return		\p false if nothing changed.
		 *\~french
		 *\brief		Met à jour les information de shadow cascades.
		 *\param[in]	sceneCamera		La caméra de la scène.
		 *\return		\p false si rien n'a changé.
		 */
		C3D_API bool updateShadow( Camera const & sceneCamera );
		/**
		 *\copydoc		castor3d::LightCategory::createTextWriter
		 */
		C3D_API std::unique_ptr< LightCategory::TextWriter > createTextWriter( castor::String const & tabs )override
		{
			return std::make_unique< TextWriter >( tabs, this );
		}
		/**
		*\~english
		*name
		*	Getters.
		*\~french
		*name
		*	Accesseurs.
		*/
		/**@{*/
		inline castor::Point3f const & getDirection()const
		{
			return m_direction;
		}

		inline float getSplitDepth( uint32_t cascadeIndex )const
		{
			return m_cascades[cascadeIndex].splitDepthScale->x;
		}

		inline float getSplitScale( uint32_t cascadeIndex )const
		{
			return m_cascades[cascadeIndex].splitDepthScale->y;
		}

		inline castor::Matrix4x4f const & getProjMatrix( uint32_t cascadeIndex )const
		{
			return m_cascades[cascadeIndex].projMatrix;
		}

		inline castor::Matrix4x4f const & getViewMatrix( uint32_t cascadeIndex )const
		{
			return m_cascades[cascadeIndex].viewMatrix;
		}

		inline castor::Matrix4x4f const & getLightSpaceTransform( uint32_t cascadeIndex )const
		{
			return m_cascades[cascadeIndex].viewProjMatrix;
		}
		/**@}*/

	private:
		/**
		 *\copydoc		castor3d::LightCategory::updateNode
		 */
		C3D_API void updateNode( SceneNode const & node )override;
		/**
		 *\copydoc		castor::LightCategory::doBind
		 */
		C3D_API void doBind( castor::Point4f * buffer )const override;

	private:
		castor::Point3f m_direction;
		std::vector< Cascade > m_cascades;
		std::vector< Cascade > m_prvCascades;
	};
}

#endif
