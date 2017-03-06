namespace GLSL
{
	//*****************************************************************************************

	namespace details
	{
		template< typename ValueT >
		struct Mat2Traits;

		template<>
		struct Mat2Traits< Float >
		{
			static xchar const * const GetName()
			{
				static xchar const * const l_name{ "mat2 " };
				return l_name;
			}
		};

		template<>
		struct Mat2Traits< Int >
		{
			static xchar const * const GetName()
			{
				static xchar const * const l_name{ "imat2 " };
				return l_name;
			}
		};

		template<>
		struct Mat2Traits< Boolean >
		{
			static xchar const * const GetName()
			{
				static xchar const * const l_name{ "bmat2 " };
				return l_name;
			}
		};

		//*****************************************************************************************

		template< typename ValueT >
		struct Mat3Traits;

		template<>
		struct Mat3Traits< Float >
		{
			static xchar const * const GetName()
			{
				static xchar const * const l_name{ "mat3 " };
				return l_name;
			}
		};

		template<>
		struct Mat3Traits< Int >
		{
			static xchar const * const GetName()
			{
				static xchar const * const l_name{ "imat3 " };
				return l_name;
			}
		};

		template<>
		struct Mat3Traits< Boolean >
		{
			static xchar const * const GetName()
			{
				static xchar const * const l_name{ "bmat3 " };
				return l_name;
			}
		};

		//*****************************************************************************************

		template< typename ValueT >
		struct Mat4Traits;

		template<>
		struct Mat4Traits< Float >
		{
			static xchar const * const GetName()
			{
				static xchar const * const l_name{ "mat4 " };
				return l_name;
			}
		};

		template<>
		struct Mat4Traits< Int >
		{
			static xchar const * const GetName()
			{
				static xchar const * const l_name{ "imat4 " };
				return l_name;
			}
		};

		template<>
		struct Mat4Traits< Boolean >
		{
			static xchar const * const GetName()
			{
				static xchar const * const l_name{ "bmat4 " };
				return l_name;
			}
		};
	}

	//*****************************************************************************************

	template< typename ValueT >
	Mat2T< ValueT >::Mat2T()
		: Type( details::Mat2Traits< ValueT >::GetName() )
	{
	}

	template< typename ValueT >
	Mat2T< ValueT >::Mat2T( GlslWriter * p_writer, Castor::String const & p_name )
		: Type( details::Mat2Traits< ValueT >::GetName(), p_writer, p_name )
	{
	}

	template< typename ValueT >
	Mat2T< ValueT > & Mat2T< ValueT >::operator=( Mat2T< ValueT > const & p_rhs )
	{
		if ( m_writer )
		{
			m_writer->WriteAssign( *this, p_rhs );
		}
		else
		{
			Type::operator=( p_rhs );
			m_writer = p_rhs.m_writer;
		}

		return *this;
	}

	template< typename ValueT >
	template< typename RhsT >
	Mat2T< ValueT > & Mat2T< ValueT >::operator=( RhsT const & p_rhs )
	{
		UpdateWriter( p_rhs );
		m_writer->WriteAssign( *this, p_rhs );
		return *this;
	}

	template< typename ValueT >
	template< typename IndexT >
	Vec2T< ValueT > Mat2T< ValueT >::operator[]( IndexT const & p_rhs )
	{
		Vec2T< ValueT > l_return{ m_writer, Castor::String( *this ) + cuT( "[" ) + Castor::String( p_rhs ) + cuT( "]" ) };
		return l_return;
	}

	template< typename ValueT >
	Vec2T< ValueT > Mat2T< ValueT >::operator[]( int const & p_rhs )
	{
		Vec2T< ValueT > l_return{ m_writer, Castor::String( *this ) + cuT( "[" ) + Castor::string::to_string( p_rhs ) + cuT( "]" ) };
		return l_return;
	}

	//*****************************************************************************************

	template< typename ValueT >
	Mat3T< ValueT >::Mat3T()
		: Type( details::Mat3Traits< ValueT >::GetName() )
	{
	}

	template< typename ValueT >
	Mat3T< ValueT >::Mat3T( GlslWriter * p_writer, Castor::String const & p_name )
		: Type( details::Mat3Traits< ValueT >::GetName(), p_writer, p_name )
	{
	}

	template< typename ValueT >
	Mat3T< ValueT > & Mat3T< ValueT >::operator=( Mat3T< ValueT > const & p_rhs )
	{
		if ( m_writer )
		{
			m_writer->WriteAssign( *this, p_rhs );
		}
		else
		{
			Type::operator=( p_rhs );
			m_writer = p_rhs.m_writer;
		}

		return *this;
	}

	template< typename ValueT >
	template< typename RhsT >
	Mat3T< ValueT > & Mat3T< ValueT >::operator=( RhsT const & p_rhs )
	{
		UpdateWriter( p_rhs );
		m_writer->WriteAssign( *this, p_rhs );
		return *this;
	}

	template< typename ValueT >
	template< typename IndexT >
	Vec3T< ValueT > Mat3T< ValueT >::operator[]( IndexT const & p_rhs )
	{
		Vec3T< ValueT > l_return{ m_writer, Castor::String( *this ) + cuT( "[" ) + Castor::String( p_rhs ) + cuT( "]" ) };
		return l_return;
	}

	template< typename ValueT >
	Vec3T< ValueT > Mat3T< ValueT >::operator[]( int const & p_rhs )
	{
		Vec3T< ValueT > l_return{ m_writer, Castor::String( *this ) + cuT( "[" ) + Castor::string::to_string( p_rhs ) + cuT( "]" ) };
		return l_return;
	}

	//*****************************************************************************************

	template< typename ValueT >
	Mat4T< ValueT >::Mat4T()
		: Type( details::Mat4Traits< ValueT >::GetName() )
	{
	}

	template< typename ValueT >
	Mat4T< ValueT >::Mat4T( GlslWriter * p_writer, Castor::String const & p_name )
		: Type( details::Mat4Traits< ValueT >::GetName(), p_writer, p_name )
	{
	}

	template< typename ValueT >
	Mat4T< ValueT > & Mat4T< ValueT >::operator=( Mat4T< ValueT > const & p_rhs )
	{
		if ( m_writer )
		{
			m_writer->WriteAssign( *this, p_rhs );
		}
		else
		{
			Type::operator=( p_rhs );
			m_writer = p_rhs.m_writer;
		}

		return *this;
	}

	template< typename ValueT >
	template< typename RhsT >
	Mat4T< ValueT > & Mat4T< ValueT >::operator=( RhsT const & p_rhs )
	{
		UpdateWriter( p_rhs );
		m_writer->WriteAssign( *this, p_rhs );
		return *this;
	}

	template< typename ValueT >
	template< typename IndexT >
	Vec4T< ValueT > Mat4T< ValueT >::operator[]( IndexT const & p_rhs )
	{
		Vec4T< ValueT > l_return{ m_writer, Castor::String( *this ) + cuT( "[" ) + Castor::String( p_rhs ) + cuT( "]" ) };
		return l_return;
	}

	template< typename ValueT >
	Vec4T< ValueT > Mat4T< ValueT >::operator[]( int const & p_rhs )
	{
		Vec4T< ValueT > l_return{ m_writer, Castor::String( *this ) + cuT( "[" ) + Castor::string::to_string( p_rhs ) + cuT( "]" ) };
		return l_return;
	}

	//*****************************************************************************************
}