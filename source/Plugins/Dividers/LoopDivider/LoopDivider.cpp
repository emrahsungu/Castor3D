#include "LoopDivider/LoopDivider.hpp"

#include "LoopDivider/LoopVertex.hpp"
#include "LoopDivider/LoopFaceEdges.hpp"

#include <CastorUtils/Math/Point.hpp>
#include <CastorUtils/Math/Angle.hpp>

using namespace castor;

namespace Loop
{
	namespace
	{
		double getAlpha( uint32_t n )
		{
			double tmp = 3.0 + 2.0 * cos( 2.0 * Pi< float > / n );
			double beta = 1.25 - ( tmp * tmp ) / 32.0;
			return n * ( 1 - beta ) / beta;
		}
	}

	String const Subdivider::Name = cuT( "Loop Divider" );
	String const Subdivider::Type = cuT( "loop" );

	Subdivider::Subdivider()
		: castor3d::MeshSubdivider()
	{
	}

	Subdivider::~Subdivider()
	{
		cleanup();
	}

	castor3d::MeshSubdividerUPtr Subdivider::create()
	{
		return std::make_unique< Subdivider >();
	}

	void Subdivider::cleanup()
	{
		castor3d::MeshSubdivider::cleanup();
		m_facesEdges.clear();
		m_vertices.clear();
	}

	VertexSPtr Subdivider::addPoint( float x, float y, float z )
	{
		VertexSPtr result = std::make_shared< Vertex >( castor3d::MeshSubdivider::addPoint( x, y, z ) );
		m_vertices.emplace( result->getIndex(), result );
		return result;
	}

	VertexSPtr Subdivider::addPoint( castor::Point3f const & v )
	{
		VertexSPtr result = std::make_shared< Vertex >( castor3d::MeshSubdivider::addPoint( v ) );
		m_vertices.emplace( result->getIndex(), result );
		return result;
	}

	VertexSPtr Subdivider::addPoint( float * v )
	{
		VertexSPtr result = std::make_shared< Vertex >( castor3d::MeshSubdivider::addPoint( v ) );
		m_vertices.emplace( result->getIndex(), result );
		return result;
	}

	void Subdivider::doInitialise()
	{
		castor3d::MeshSubdivider::doInitialise();
		m_indexMapping = m_submesh->getComponent< castor3d::TriFaceMapping >();
		uint32_t index = 0;

		for ( auto & point : getPoints() )
		{
			m_vertices.emplace( index, std::make_shared< Vertex >( castor3d::SubmeshVertex{ index, point } ) );
			++index;
		}

		for ( auto & face : m_indexMapping->getFaces() )
		{
			m_facesEdges.emplace_back( std::make_shared< FaceEdges >( this, face, m_vertices ) );
		}

		m_indexMapping->clearFaces();
	}

	void Subdivider::doAddGeneratedFaces()
	{
		for ( auto const & face : m_arrayFaces )
		{
			m_indexMapping->addFace( face[0], face[1], face[2] );
		}
	}

	void Subdivider::doSubdivide()
	{
		doDivide();
		doAverage();
	}

	void Subdivider::doDivide()
	{
		FaceEdgesPtrArray old;
		std::swap( old, m_facesEdges );

		for ( auto & faceEdges : old )
		{
			faceEdges->divide( 0.5f, m_vertices, m_facesEdges );
		}
	}

	void Subdivider::doAverage()
	{
		std::map< uint32_t, castor::Point3f > positions;

		for ( auto & it : m_vertices )
		{
			castor::Point3f point = it.second->getPoint().pos;
			positions.emplace( it.first, point );
		}

		for ( auto & it : m_vertices )
		{
			VertexSPtr vertex = it.second;
			uint32_t nbEdges = vertex->size();
			castor::Point3f & position = vertex->getPoint().pos;
			float alpha = float( getAlpha( nbEdges ) );
			position *= alpha;

			for ( auto & itI : *vertex )
			{
				position += positions[itI.first];
			}

			position /= alpha + nbEdges;
		}

		for ( auto & face : m_arrayFaces )
		{
			castor::Coords3f dump;
			m_submesh->getPoint( face[0] ).pos = m_vertices[face[0]]->getPoint().pos;
			m_submesh->getPoint( face[1] ).pos = m_vertices[face[1]]->getPoint().pos;
			m_submesh->getPoint( face[2] ).pos = m_vertices[face[2]]->getPoint().pos;
		}

		m_vertices.clear();
	}
}
