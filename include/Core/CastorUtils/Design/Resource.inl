/*
See LICENSE file in root folder
*/
namespace castor
{
	template< class ResType >
	Resource< ResType >::Resource( Collection< ResType, String > & collection
		, String const & name )
		: Named{ name }
		, m_collection{ &collection }
	{
	}
	
	template< class ResType >
	Resource< ResType >::Resource( String const & name )
		: Named{ name }
		, m_collection{ nullptr }
	{
	}

	template< class ResType >
	void Resource< ResType >::changeName( String const & name )
	{
		if ( m_name == name )
		{
			return;
		}

		if ( !m_collection )
		{
			m_name = name;
			return;
		}

		if ( m_collection->has( name ) )
		{
			Logger::logWarning( details::WARNING_COLLECTION_DUPLICATE_OBJECT + string::toString( name ) );
		}
		else
		{
			std::shared_ptr< ResType > pThis = m_collection->erase( m_name );
			CU_Require( pThis );
			auto res = m_collection->insert( name, pThis );
			CU_Require( res );
			m_name = name;
		}
	}
}