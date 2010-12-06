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
#ifndef ___GL3_SubmeshRenderer___
#define ___GL3_SubmeshRenderer___

#include "Module_GLRender.h"

namespace Castor3D
{
	class C3D_GL3_API GL3SubmeshRenderer : public SubmeshRenderer
	{
	protected:
		eDRAW_TYPE m_eDisplayMode;
		size_t m_uiVertextArrayObject;

	public:
		GL3SubmeshRenderer();
		virtual ~GL3SubmeshRenderer();

		virtual void Cleanup();
		virtual void Initialise();
		virtual void Draw( eDRAW_TYPE p_displayMode, PassPtr p_pass);
		virtual void ShowVertex( eDRAW_TYPE p_displayMode);
		virtual void HideVertex();

	private:
		void _createVBOs();
		void _createVAOs( eDRAW_TYPE p_eDisplayMode, PassPtr p_pPass);
	};
}

#endif