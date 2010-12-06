#include "PrecompiledHeader.h"

#include "OpenGLCommon/GLShaderObject.h"
#include "OpenGLCommon/GLShaderProgram.h"

using namespace Castor3D;

GLShaderObject :: GLShaderObject()
	:	ShaderObject(),
		m_shaderObject( 0),
		m_pShaderProgram( NULL)
{
}

GLShaderObject :: ~GLShaderObject()
{
	Detach();

	if (m_isCompiled)
	{
		glDeleteShader( m_shaderObject);
		CheckGLError( CU_T( "GLShaderObject :: ~GLShaderObject - glDeleteShader"));
	}
}

int GLShaderObject :: Load( const String & p_filename)
{
	m_bError = false;

	File l_file( p_filename.c_str(), File::eRead);

	if ( ! l_file.IsOk())
	{
		return -1;
	}

	size_t l_len = size_t( l_file.GetLength());

	if (l_len == 0)
	{
		return -2;
	}

	l_file.CopyToString( m_shaderSource);

	return 0;
}

int GLShaderObject :: LoadFromMemory( const String & p_program)
{
	m_bError = false;

	m_shaderSource = p_program;

	return 0;
}

void GLShaderObject :: RetrieveCompilerLog( String & p_strCompilerLog)
{
	int infologLength = 0;
	int charsWritten  = 0;
	glGetShaderiv( m_shaderObject, GL_INFO_LOG_LENGTH, & infologLength);
	CheckGLError( CU_T( "GLShaderObject :: RetrieveCompilerLog - glGetShaderiv"));

	if (infologLength > 0)
	{
		char * infoLog = new char[infologLength];
		glGetShaderInfoLog( m_shaderObject, infologLength, & charsWritten, infoLog);
		CheckGLError( CU_T( "GLShaderObject :: RetrieveCompilerLog - glGetShaderInfoLog"));
		p_strCompilerLog = infoLog;
		delete [] infoLog;
	}

	if (p_strCompilerLog.size() > 0)
	{
		p_strCompilerLog = p_strCompilerLog.substr( 0, p_strCompilerLog.size() - 1);
	}
}

bool GLShaderObject :: Compile()
{
	bool l_bReturn = false;

	if (RenderSystem::UseShaders() && ! m_bError && ! m_shaderSource.empty())
	{
		m_isCompiled = false;

		GLint l_iCompiled = 0;
		GLint l_iLength = m_shaderSource.size();
		const char * l_pTmp = m_shaderSource.char_str();

		glShaderSource( m_shaderObject, 1, & l_pTmp, & l_iLength);
		CheckGLError( CU_T( "GLShaderObject :: Compile - glShaderSource"));

		glCompileShader( m_shaderObject); 
		CheckGLError( CU_T( "GLShaderObject :: Compile - glCompileShader"));

		glGetShaderiv( m_shaderObject, GL_COMPILE_STATUS, & l_iCompiled);
		CheckGLError( CU_T( "GLShaderObject :: Compile - glGetShaderiv"));

		if (l_iCompiled != 0)
		{
			m_isCompiled = true;
		}
		else
		{
			m_bError = true;
		}

		RetrieveCompilerLog( m_compilerLog);

		if (m_compilerLog.size() > 0)
		{
			Logger::LogMessage( m_compilerLog);
		}

		l_bReturn = m_isCompiled;
	}

	return l_bReturn;
}

void GLShaderObject :: Detach()
{
	if (m_isCompiled && m_pShaderProgram != NULL && ! m_bError)
	{
		try
		{
			glDetachShader( m_pShaderProgram->GetProgramObject(), m_shaderObject);
			CheckGLError( CU_T( "GLShaderObject :: Detach - glDetachShader"));
		}
		catch ( ... )
		{
			Logger::LogMessage( CU_T( "GLShaderObject :: Detach - glDetachShader - Exception"));
		}

		m_pShaderProgram = NULL;
		// if you get an error here, you deleted the Program object first and then
		// the ShaderObject! Always delete ShaderObjects last!
	}
}

void GLShaderObject :: AttachTo( GLShaderProgram * p_pProgram)
{
	Detach();

	if (m_isCompiled && ! m_bError)
	{
		m_pShaderProgram = p_pProgram;
		glAttachShader( m_pShaderProgram->GetProgramObject(), m_shaderObject);
		CheckGLError( CU_T( "GLShaderObject :: AttachTo - glAttachShader"));
	}
}

GLVertexShader::GLVertexShader()
	:	GLShaderObject()
{
	m_programType = ShaderObject::eVertex; 

	if (RenderSystem::UseShaders())
	{
		m_shaderObject = glCreateShader( GL_VERTEX_SHADER);
		CheckGLError( CU_T( "GLVertexShader :: GLVertexShader - glCreateShader"));
	}
}

GLVertexShader::~GLVertexShader()
{
}

GLFragmentShader::GLFragmentShader()
	:	GLShaderObject()
{
	m_programType = ShaderObject::eFragment;

	if (RenderSystem::UseShaders())
	{
		m_shaderObject = glCreateShader( GL_FRAGMENT_SHADER);
		CheckGLError( CU_T( "GLFragmentShader :: GLFragmentShader - glCreateShader"));
	}
}

GLFragmentShader::~GLFragmentShader()
{
}


GLGeometryShader::GLGeometryShader()
	:	GLShaderObject()
{
	m_programType = ShaderObject::eGeometry; 

	if (RenderSystem::UseShaders() && RenderSystem::HasGeometryShader())
	{
		m_shaderObject = glCreateShader( GL_GEOMETRY_SHADER);
		CheckGLError( CU_T( "GLGeometryShader :: GLGeometryShader - glCreateShader"));
	}
}

GLGeometryShader::~GLGeometryShader()
{
}