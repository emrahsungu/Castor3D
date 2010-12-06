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
#ifndef ___GL2_TextureRenderer___
#define ___GL2_TextureRenderer___

#include "Module_GLRender.h"

namespace Castor3D
{	
	class C3D_GL2_API GL2TextureRenderer : public GLTextureRenderer
	{
	protected:
		GLTextureBufferObjectPtr m_pTextureBufferObject;

	public:
		GL2TextureRenderer()
			:	GLTextureRenderer()
		{}
		virtual ~GL2TextureRenderer()
		{}

		virtual bool Initialise();

	private:
		void _cleanup		();
		bool _initialise	();
		void _render		();
		void _endRender		();
	};
}

#endif