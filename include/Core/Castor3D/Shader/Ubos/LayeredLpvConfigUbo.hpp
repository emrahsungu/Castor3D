/*
See LICENSE file in root folder
*/
#ifndef ___C3D_LayeredLpvConfigUbo_H___
#define ___C3D_LayeredLpvConfigUbo_H___

#include "UbosModule.hpp"

#include "Castor3D/Buffer/UniformBuffer.hpp"

#include <CastorUtils/Graphics/Grid.hpp>

namespace castor3d
{
	class LayeredLpvConfigUbo
	{
	public:
		using Configuration = LayeredLpvConfigUboConfiguration;

	public:
		/**
		 *\~english
		 *\name			Copy/Move construction/assignment operation.
		 *\~french
		 *\name			Constructeurs/Opérateurs d'affectation par copie/déplacement.
		 */
		/**@{*/
		C3D_API LayeredLpvConfigUbo( LayeredLpvConfigUbo const & ) = delete;
		C3D_API LayeredLpvConfigUbo & operator=( LayeredLpvConfigUbo const & ) = delete;
		C3D_API LayeredLpvConfigUbo( LayeredLpvConfigUbo && ) = default;
		C3D_API LayeredLpvConfigUbo & operator=( LayeredLpvConfigUbo && ) = delete;
		/**@}*/
		/**
		 *\~english
		 *\brief		Constructor.
		 *\param[in]	engine	The engine.
		 *\~french
		 *\brief		Constructeur.
		 *\param[in]	engine	Le moteur.
		 */
		explicit LayeredLpvConfigUbo( Engine & engine );
		/**
		 *\~english
		 *\brief		Updates UBO data.
		 *\~french
		 *\brief		Met à jour les données de l'UBO.
		 */
		C3D_API void update( std::array< castor::Grid, shader::DirectionalMaxCascadesCount > const & grids
			, float indirectAttenuation = 1.7f );
		/**
		 *\~english
		 *\name			getters.
		 *\~french
		 *\name			getters.
		 */
		inline UniformBuffer< Configuration > & getUbo()
		{
			return *m_ubo;
		}

		inline UniformBuffer< Configuration > const & getUbo()const
		{
			return *m_ubo;
		}
		/**@}*/

	public:
		C3D_API static const std::string LayeredLpvConfig;
		C3D_API static const std::string AllMinVolumeCorners;
		C3D_API static const std::string AllCellSizes;
		C3D_API static const std::string GridSize;
		C3D_API static const std::string Config;

	private:
		Engine & m_engine;
		UniformBufferUPtr< Configuration > m_ubo;
	};
}

#define UBO_LAYERED_LPVCONFIG( writer, binding, set )\
	sdw::Ubo layeredLpvConfig{ writer\
		, castor3d::LayeredLpvConfigUbo::LayeredLpvConfig\
		, binding\
		, set\
		, ast::type::MemoryLayout::eStd140 };\
	auto c3d_allMinVolumeCorners = layeredLpvConfig.declMember< Vec4 >( castor3d::LayeredLpvConfigUbo::AllMinVolumeCorners, 4u );\
	auto c3d_allCellSizes = layeredLpvConfig.declMember< Vec4 >( castor3d::LayeredLpvConfigUbo::AllCellSizes );\
	auto c3d_gridSize = layeredLpvConfig.declMember< UVec4 >( castor3d::LayeredLpvConfigUbo::GridSize );\
	auto c3d_config = layeredLpvConfig.declMember< Vec4 >( castor3d::LayeredLpvConfigUbo::Config );\
	layeredLpvConfig.end()

#endif
