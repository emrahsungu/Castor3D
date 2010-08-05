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
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
*/
#ifndef ___C3D_Md2Importer___
#define ___C3D_Md2Importer___

#include "ExternalImporter.h"

// These are the needed defines for the max values when loading .MD2 files
#define MD2_MAX_TRIANGLES		4096
#define MD2_MAX_VERTICES		2048
#define MD2_MAX_TEXCOORDS		2048
#define MD2_MAX_FRAMES			512
#define MD2_MAX_SKINS			32
#define MD2_MAX_FRAMESIZE		(MD2_MAX_VERTICES * 4 + 128)

namespace Castor3D
{
	struct CS3D_API Md2Header
	{ 
	   int m_magic;
	   int m_version;
	   int m_skinWidth;
	   int m_skinHeight;
	   int m_frameSize;
	   int m_numSkins;
	   int m_numVertices;
	   int m_numTexCoords;
	   int m_numTriangles;
	   int m_numGlCommands;
	   int m_numFrames;
	   int m_offsetSkins;
	   int m_offsetTexCoords;
	   int m_offsetTriangles;
	   int m_offsetFrames;
	   int m_offsetGlCommands;
	   int om_ffsetEnd;
	};

	struct CS3D_API Md2AliasTriangle
	{
	   unsigned char m_vertex[3];
	   unsigned char m_lightNormalIndex;
	};

	struct CS3D_API Md2Triangle
	{
	   float m_vertex[3];
	   float m_normal[3];
	};

	struct CS3D_API Md2Face
	{
	   short m_vertexIndices[3];
	   short m_textureIndices[3];
	};

	struct CS3D_API Md2TexCoord
	{
	   short u, v;
	};

	struct CS3D_API Md2AliasFrame
	{
	   float m_scale[3];
	   float m_translate[3];
	   char m_name[16];
	   Md2AliasTriangle m_aliasVertices[1];
	};

	struct CS3D_API Md2Frame
	{
	   char m_strName[16];
	   Md2Triangle * m_vertices;
	};

	typedef char Md2Skin[64];

	class CS3D_API Md2Importer : public ExternalImporter
	{
	private:
		Md2Header		m_header;
		Md2Skin		*	m_skins;
		Md2TexCoord	*	m_texCoords;
		Md2Face		*	m_triangles;
		Md2Frame	*	m_frames;
		String			m_textureName;
		File		*	m_pFile;

	public:
		Md2Importer( const String & p_textureName = C3DEmptyString);// This inits the data members

	private:
		virtual bool _import();
		void _readMD2Data();
		void _convertDataStructures( Imported3DModel * p_model);
		void _cleanUp();
	};
}


#endif