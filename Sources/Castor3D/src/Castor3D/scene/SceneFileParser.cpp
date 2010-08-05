#include "PrecompiledHeader.h"

#include "scene/SceneFileParser.h"
#include "scene/SceneFileParser_Parsers.h"
#include "scene/SceneManager.h"

#include "Log.h"

using namespace Castor3D;
using namespace General::Utils;

SceneFileParser :: SceneFileParser()
	:	m_pContext( new SceneFileContext)
{
	m_mapRootParsers			["object"]				= Parser_RootObject;
	m_mapRootParsers			["light"]				= Parser_RootLight;
	m_mapRootParsers			["scene_node"]			= Parser_RootSceneNode;
	m_mapRootParsers			["camera"]				= Parser_RootCamera;
	m_mapRootParsers			["material"]			= Parser_RootMaterial;

	m_mapLightParsers			["type"]				= Parser_LightType;
	m_mapLightParsers			["position"]			= Parser_LightPosition;
	m_mapLightParsers			["ambient"]				= Parser_LightAmbient;
	m_mapLightParsers			["diffuse"]				= Parser_LightDiffuse;
	m_mapLightParsers			["specular"]			= Parser_LightSpecular;
	m_mapLightParsers			["attenuation"]			= Parser_LightAttenuation;
	m_mapLightParsers			["cut_off"]				= Parser_LightCutOff;
	m_mapLightParsers			["exponent"]			= Parser_LightExponent;
	m_mapLightParsers			["orientation"]			= Parser_LightOrientation;
	m_mapLightParsers			["}"]					= Parser_LightEnd;

	m_mapSceneNodeParsers		["attach_to"]			= Parser_NodeParent;
	m_mapSceneNodeParsers		["position"]			= Parser_NodePosition;
	m_mapSceneNodeParsers		["orientation"]			= Parser_NodeOrientation;
	m_mapSceneNodeParsers		["scale"]				= Parser_NodeScale;
	m_mapSceneNodeParsers		["}"]					= Parser_NodeEnd;

	m_mapObjectParsers			["parent"]				= Parser_ObjectParent;
	m_mapObjectParsers			["mesh"]				= Parser_ObjectMesh;
	m_mapObjectParsers			["}"]					= Parser_ObjectEnd;

	m_mapMeshParsers			["type"]				= Parser_MeshType;
	m_mapMeshParsers			["file"]				= Parser_MeshFile;
	m_mapMeshParsers			["material"]			= Parser_MeshMaterial;
	m_mapMeshParsers			["submesh"]				= Parser_MeshSubmesh;
	m_mapMeshParsers			["}"]					= Parser_MeshEnd;

	m_mapSubmeshParsers			["material"]			= Parser_SubmeshMaterial;
	m_mapSubmeshParsers			["vertex"]				= Parser_SubmeshVertex;
	m_mapSubmeshParsers			["smoothing_group"]		= Parser_SubmeshSmoothingGroup;
	m_mapSubmeshParsers			["}"]					= Parser_SubmeshEnd;

	m_mapSmoothingGroupParsers	["face"]				= Parser_SmoothingGroupFace;
	m_mapSmoothingGroupParsers	["}"]					= Parser_SmoothingGroupEnd;

	m_mapMaterialParsers		["pass"]				= Parser_MaterialPass;
	m_mapMaterialParsers		["}"]					= Parser_MaterialEnd;
	
	m_mapPassParsers			["ambient"]				= Parser_PassAmbient;
	m_mapPassParsers			["diffuse"]				= Parser_PassDiffuse;
	m_mapPassParsers			["specular"]			= Parser_PassSpecular;
	m_mapPassParsers			["emissive"]			= Parser_PassEmissive;
	m_mapPassParsers			["shininess"]			= Parser_PassShininess;
	m_mapPassParsers			["texture_unit"]		= Parser_PassTextureUnit;
	m_mapPassParsers			["shader_program"]		= Parser_PassShader;
	m_mapPassParsers			["}"]					= Parser_PassEnd;

	m_mapTextureUnitParsers		["image"]				= Parser_UnitImage;
	m_mapTextureUnitParsers		["colour"]				= Parser_UnitColour;
	m_mapTextureUnitParsers		["map_type"]			= Parser_UnitMapType;
	m_mapTextureUnitParsers		["}"]					= Parser_UnitEnd;

	m_mapShaderParsers			["vertex_program"]		= Parser_ShaderVertexProgram;
	m_mapShaderParsers			["pixel_program"]		= Parser_ShaderPixelProgram;
	m_mapShaderParsers			["geometry_program"]	= Parser_ShaderGeometryProgram;
	m_mapShaderParsers			["vertex_file"]			= Parser_ShaderVertexFile;
	m_mapShaderParsers			["pixel_file"]			= Parser_ShaderPixelFile;
	m_mapShaderParsers			["geometry_file"]		= Parser_ShaderGeometryFile;
	m_mapShaderParsers			["variable"]			= Parser_ShaderVariable;
	m_mapShaderParsers			["}"]					= Parser_ShaderEnd;

	m_mapShaderVariableParsers	["type"]				= Parser_ShaderVariableType;
	m_mapShaderVariableParsers	["name"]				= Parser_ShaderVariableName;
	m_mapShaderVariableParsers	["value"]				= Parser_ShaderVariableValue;
	m_mapShaderVariableParsers	["}"]					= Parser_ShaderVariableEnd;
}

SceneFileParser :: ~SceneFileParser()
{
}

bool SceneFileParser :: ParseFile( const String & p_strFileName)
{
	bool l_bReturn = false;

	FileStream * l_pFile = new FileStream( p_strFileName, FileBase::eRead);

	if (l_pFile->IsOk())
	{
		m_pContext->Initialise();
		m_pContext->pFile = l_pFile;
		m_pContext->pScene = SceneManager::GetSingleton().GetElementByName( "MainScene");

		bool l_bNextIsOpenBrace = false;
		bool l_bCommented = false;

		Log::LogMessage( "SceneFileParser : Parsing scene file [" + l_pFile->GetFileName() + "]");

		m_pContext->eSection = SceneFileContext::eNone;
		m_pContext->ui64Line = 0;

		m_pContext->strName.clear();

		bool l_bReuse = false;

		String l_strLine;

		while (l_pFile->IsOk())
		{
			if ( ! l_bReuse)
			{
				l_pFile->ReadLine( l_strLine, 1000);
				m_pContext->ui64Line++;
			}
			else
			{
				l_bReuse = false;
			}

			l_strLine.Trim();

			if (l_strLine.empty())
			{
				continue;
			}

			if (l_strLine.size() >= 2 && l_strLine.substr( 0, 2) == "//")
			{
				continue;
			}

			if ( ! l_bCommented)
			{
				if (l_strLine.size() >= 2 && l_strLine.substr( 0, 2) == "/*")
				{
					l_bCommented = true;
				}
				else
				{
					if (l_bNextIsOpenBrace)
					{
						if (l_strLine != "{")
						{
							String l_strEndLine = "}";
							l_bNextIsOpenBrace = _parseScriptLine( l_strEndLine);
							l_bReuse = true;
						}
						else
						{
							l_bNextIsOpenBrace = false;
						}
					}
					else
					{
						l_bNextIsOpenBrace = _parseScriptLine( l_strLine);
					}
				}
			}
			else
			{
				if (l_strLine.size() >= 2 && l_strLine.substr( 0, 2) == "*/")
				{
					l_bCommented = false;
				}
			}
		}

		if (m_pContext->eSection != SceneFileContext::eNone)
		{
			ParseError( "Parsing Error : ParseScript -> unexpected end of file", m_pContext);
		}

		Log::LogMessage( "SceneFileParser : Finished parsing script [" + l_pFile->GetFileName() + "]");

		l_bReturn = true;
	}

	delete l_pFile;

	return l_bReturn;
}

bool SceneFileParser::_parseScriptLine( String & p_line)
{
	switch (m_pContext->eSection)
	{
	case SceneFileContext::eNone:				return _invokeParser( p_line, m_mapRootParsers);
	case SceneFileContext::eObject:				return _invokeParser( p_line, m_mapObjectParsers);
	case SceneFileContext::eMesh:				return _invokeParser( p_line, m_mapMeshParsers);
	case SceneFileContext::eSubmesh:			return _invokeParser( p_line, m_mapSubmeshParsers);
	case SceneFileContext::eSmoothingGroup:		return _invokeParser( p_line, m_mapSmoothingGroupParsers);
	case SceneFileContext::eNode:				return _invokeParser( p_line, m_mapSceneNodeParsers);
	case SceneFileContext::eLight:				return _invokeParser( p_line, m_mapLightParsers);
	case SceneFileContext::eCamera:				return _invokeParser( p_line, m_mapCameraParsers);
	case SceneFileContext::eMaterial:			return _invokeParser( p_line, m_mapMaterialParsers);
	case SceneFileContext::ePass:				return _invokeParser( p_line, m_mapPassParsers);
	case SceneFileContext::eTextureUnit:		return _invokeParser( p_line, m_mapTextureUnitParsers);
	case SceneFileContext::eShader:				return _invokeParser( p_line, m_mapShaderParsers);
	case SceneFileContext::eShaderVariable:		return _invokeParser( p_line, m_mapShaderVariableParsers);
	default : 
		return _delegateParser( p_line);
	}

	return false;
}

bool SceneFileParser::_invokeParser( String & p_line, const AttributeParserMap & p_parsers)
{
	bool l_bReturn = false;

    StringArray splitCmd = p_line.Split( " \t", 1);
	const AttributeParserMap::const_iterator & l_iter = p_parsers.find( splitCmd[0]);

    if (l_iter == p_parsers.end())
    {
		Log::LogMessage( "Parser not found @ line #%i : %s", m_pContext->ui64Line, p_line.c_str());
    }
	else
	{
		if (splitCmd.size() >= 2)
		{
			splitCmd[1].Trim();

			l_bReturn = l_iter->second( splitCmd[1], m_pContext);
		}
		else
		{
			l_bReturn = l_iter->second( C3DEmptyString, m_pContext);
		}
	}

	return l_bReturn;
}
