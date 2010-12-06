/*
This source file is part of Castor3D (http://dragonjoker.co.cc

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
the program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
*/
#ifndef ___GL4_MaterialRenderer___
#define ___GL4_MaterialRenderer___

#include "Module_GLRender.h"

namespace Castor3D
{
	class C3D_GL3_API GL4PassRenderer : public PassRenderer
	{
	protected:
		Point4f			m_varAmbient;
		Point4f			m_varDiffuse;
		Point4f			m_varSpecular;
		Point4f			m_varEmissive;
		float			m_varShininess;
		bool			m_bChanged;

		GLUBOPoint4fVariablePtr 	m_pAmbient;
		GLUBOPoint4fVariablePtr 	m_pDiffuse;
		GLUBOPoint4fVariablePtr 	m_pEmissive;
		GLUBOPoint4fVariablePtr 	m_pSpecular;
		GLUBOFloatVariablePtr 		m_pShininess;
		GLUniformBufferObjectPtr	m_pUniformBuffer;

	public:
		GL4PassRenderer();
		virtual ~GL4PassRenderer(){ Cleanup(); }

		virtual void Cleanup();
		virtual void Initialise();

		virtual void Render( eDRAW_TYPE p_displayMode);
		virtual void Render2D( eDRAW_TYPE p_displayMode);
		virtual void EndRender();
	};
}

#endif