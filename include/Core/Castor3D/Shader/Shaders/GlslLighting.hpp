/*
See LICENSE file in root folder
*/
#ifndef ___C3D_GlslLighting_H___
#define ___C3D_GlslLighting_H___

#include "SdwModule.hpp"
#include "Castor3D/Scene/Light/LightModule.hpp"

#include <ShaderWriter/Intrinsics/Intrinsics.hpp>

namespace castor3d
{
	namespace shader
	{
		struct FragmentInput
		{
			C3D_API FragmentInput( FragmentInput const & rhs );
			C3D_API explicit FragmentInput( sdw::ShaderWriter & writer );
			C3D_API FragmentInput( sdw::InVec2 const & clipVertex
				, sdw::InVec3 const & viewVertex
				, sdw::InVec3 const & worldVertex
				, sdw::InVec3 const & worldNormal );

			C3D_API ast::expr::Expr * getExpr()const;
			C3D_API ast::Shader * getShader()const;
			C3D_API void setVar( ast::var::VariableList::const_iterator & var );

			sdw::InVec2 m_clipVertex;
			sdw::InVec3 m_viewVertex;
			sdw::InVec3 m_worldVertex;
			sdw::InVec3 m_worldNormal;

		private:
			ast::expr::ExprPtr m_expr;
		};

		C3D_API ast::expr::ExprList makeFnArg( ast::Shader & shader
			, FragmentInput const & value );

		class LightingModel
		{
		public:
			C3D_API LightingModel( sdw::ShaderWriter & writer
				, Utils & utils
				, ShadowOptions shadowOptions
				, bool isOpaqueProgram );
			C3D_API void declareModel( uint32_t & index);
			C3D_API void declareDirectionalModel( bool lightUbo
				, uint32_t & index );
			C3D_API void declarePointModel( bool lightUbo
				, uint32_t & index );
			C3D_API void declareSpotModel( bool lightUbo
				, uint32_t & index );
			// Calls
			C3D_API DirectionalLight getDirectionalLight( sdw::Int const & index )const;
			C3D_API PointLight getPointLight( sdw::Int const & index )const;
			C3D_API SpotLight getSpotLight( sdw::Int const & index )const;

			inline Shadow const & getShadowModel()const
			{
				return *m_shadowModel;
			}

		protected:
			C3D_API Light getBaseLight( sdw::Int const & value )const;
			C3D_API void doDeclareLight();
			C3D_API void doDeclareDirectionalLight();
			C3D_API void doDeclarePointLight();
			C3D_API void doDeclareSpotLight();
			C3D_API void doDeclareDirectionalLightUbo();
			C3D_API void doDeclarePointLightUbo();
			C3D_API void doDeclareSpotLightUbo();
			C3D_API void doDeclareGetBaseLight();
			C3D_API void doDeclareGetDirectionalLight();
			C3D_API void doDeclareGetPointLight();
			C3D_API void doDeclareGetSpotLight();
			C3D_API void doDeclareGetCascadeFactors();

			virtual void doDeclareModel() = 0;
			virtual void doDeclareComputeDirectionalLight() = 0;
			virtual void doDeclareComputePointLight() = 0;
			virtual void doDeclareComputeSpotLight() = 0;
			virtual void doDeclareComputeOneDirectionalLight() = 0;
			virtual void doDeclareComputeOnePointLight() = 0;
			virtual void doDeclareComputeOneSpotLight() = 0;

		public:
			C3D_API static uint32_t const UboBindingPoint;

		protected:
			sdw::ShaderWriter & m_writer;
			Utils & m_utils;
			bool m_isOpaqueProgram;
			std::shared_ptr< Shadow > m_shadowModel;
			sdw::Function< sdw::Vec3
				, sdw::InVec3
				, sdw::InVec4
				, sdw::InUInt > m_getCascadeFactors;
			sdw::Function< shader::Light
				, sdw::InInt > m_getBaseLight;
			sdw::Function< shader::DirectionalLight
				, sdw::InInt > m_getDirectionalLight;
			sdw::Function< shader::PointLight
				, sdw::InInt > m_getPointLight;
			sdw::Function< shader::SpotLight
				, sdw::InInt > m_getSpotLight;
			std::unique_ptr< sdw::Struct > m_lightType;
			std::unique_ptr< sdw::Struct > m_directionalLightType;
			std::unique_ptr< sdw::Struct > m_pointLightType;
			std::unique_ptr< sdw::Struct > m_spotLightType;
		};
	}
}

#endif
