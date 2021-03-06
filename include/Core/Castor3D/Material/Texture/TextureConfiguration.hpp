/*
See LICENSE file in root folder
*/
#ifndef ___C3D_TextureConfiguration_H___
#define ___C3D_TextureConfiguration_H___
#pragma once

#include "TextureModule.hpp"

#include <CastorUtils/Data/TextWriter.hpp>

namespace castor3d
{
	/**
	*\~english
	*\brief
	*	Specifies the usages of a texture, per image component.
	*\~french
	*\brief
	*	Définit les utilisations d'une texture, par composante d'image.
	*/
	struct TextureConfiguration
	{
	public:
		/**
		\author		Sylvain DOREMUS
		\date		14/02/2010
		\~english
		\brief		TextureConfiguration loader.
		\~french
		\brief		Loader de TextureConfiguration.
		*/
		class TextWriter
			: public castor::TextWriter< TextureConfiguration >
		{
		public:
			/**
			 *\~english
			 *\brief		Constructor.
			 *\~french
			 *\brief		Constructeur.
			 */
			C3D_API explicit TextWriter( castor::String const & tabs
				, MaterialType type );
			/**
			 *\~english
			 *\brief		Writes a TextureConfiguration into a text file.
			 *\param[in]	file	The file.
			 *\param[in]	object	The TextureConfiguration.
			 *\~french
			 *\brief		Ecrit une TextureConfiguration dans un fichier texte.
			 *\param[in]	file	Le fichier.
			 *\param[in]	object	La TextureConfiguration.
			 */
			C3D_API bool operator()( TextureConfiguration const & object, castor::TextFile & file )override;

		private:
			MaterialType m_type;
		};

		/**
		*\~english
		*name
		*	Masks, used to filter image components (AABBGGRR).
		*\~french
		*name
		*	Masques, utilisés pour filtrer les composantes d'une image (AABBGGRR).
		*/
		/**@{*/
		castor::Point2ui colourMask{ 0u, 0u };
		castor::Point2ui specularMask{ 0u, 0u };
		castor::Point2ui glossinessMask{ 0u, 0u };
		castor::Point2ui opacityMask{ 0u, 0u };
		castor::Point2ui emissiveMask{ 0u, 0u };
		castor::Point2ui normalMask{ 0u, 0u };
		castor::Point2ui heightMask{ 0u, 0u };
		castor::Point2ui occlusionMask{ 0u, 0u };
		castor::Point2ui transmittanceMask{ 0u, 0u };
		/**@}*/
		/**
		*\~english
		*name
		*	Factors.
		*\~french
		*name
		*	Facteurs.
		*/
		/**@{*/
		float normalFactor{ 1.0f };
		float heightFactor{ 0.1f };
		/**@}*/
		/**
		*\~english
		*name
		*	Miscellaneous.
		*\~french
		*name
		*	Divers.
		*/
		/**@{*/
		float normalGMultiplier{ 1.0f };
		uint32_t needsGammaCorrection{ 0u };
		uint32_t needsYInversion{ 0u };
		TextureSpace textureSpace{ TextureSpace::eColour };
		/**@}*/
		/**
		*\~english
		*name
		*	Component masks.
		*\~french
		*name
		*	Masques de composante.
		*/
		/**@{*/
		static uint32_t constexpr AlphaMask = 0xFF000000;
		static uint32_t constexpr BlueMask = 0x00FF0000;
		static uint32_t constexpr GreenMask = 0x0000FF00;
		static uint32_t constexpr RedMask = 0x000000FF;
		static uint32_t constexpr RgMask = RedMask | GreenMask;
		static uint32_t constexpr RgbMask = RgMask | BlueMask;
		static uint32_t constexpr RgbaMask = RgbMask | AlphaMask;
		/**@}*/
		/**@}*/
		/**
		*\~english
		*name
		*	Predefined texture configurations.
		*\~french
		*name
		*	Configurations de texture prédéfinies.
		*/
		/**@{*/
		C3D_API static TextureConfiguration const DiffuseTexture;
		C3D_API static TextureConfiguration const AlbedoTexture;
		C3D_API static TextureConfiguration const SpecularTexture;
		C3D_API static TextureConfiguration const MetalnessTexture;
		C3D_API static TextureConfiguration const GlossinessTexture;
		C3D_API static TextureConfiguration const ShininessTexture;
		C3D_API static TextureConfiguration const RoughnessTexture;
		C3D_API static TextureConfiguration const OpacityTexture;
		C3D_API static TextureConfiguration const EmissiveTexture;
		C3D_API static TextureConfiguration const NormalTexture;
		C3D_API static TextureConfiguration const HeightTexture;
		C3D_API static TextureConfiguration const OcclusionTexture;
		C3D_API static TextureConfiguration const TransmittanceTexture;
		/**@}*/
	};
	/**
	*\~english
	*name
	*	Comparison operators.
	*\~french
	*name
	*	Opérateurs de comparaison.
	*/
	/**@{*/
	C3D_API bool operator==( TextureConfiguration const & lhs, TextureConfiguration const & rhs );
	C3D_API bool operator!=( TextureConfiguration const & lhs, TextureConfiguration const & rhs );
	/**@}*/
	C3D_API TextureFlags getFlags( TextureConfiguration const & config );
}

#endif
