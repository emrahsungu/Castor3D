#include "PnTrianglesDivider/PnTrianglesDivider.hpp"

using namespace castor;
using namespace castor3d;

namespace PnTriangles
{
	namespace
	{
		castor::Point3f barycenter( float u, float v, castor::Point3f const & p1, castor::Point3f const & p2, castor::Point3f const & p3 )
		{
			float w = float( 1.0 - u - v );
			CU_Ensure( std::abs( u + v + w - 1.0 ) < 0.0001 );
			return castor::Point3f{ p1 * u + p2 * v + p3 * w };
		}
	}

	Patch::Patch( Plane const & p1
		, Plane const & p2
		, Plane const & p3 )
	{
		b300 = barycenter( 3 / 3.0f, 0 / 3.0f, p1.point, p2.point, p3.point );
		b030 = barycenter( 0 / 3.0f, 3 / 3.0f, p1.point, p2.point, p3.point );
		b003 = barycenter( 0 / 3.0f, 0 / 3.0f, p1.point, p2.point, p3.point );
		b201 = p1.plane.project( barycenter( 2 / 3.0f, 0 / 3.0f, p1.point, p2.point, p3.point ) );
		b210 = p1.plane.project( barycenter( 2 / 3.0f, 1 / 3.0f, p1.point, p2.point, p3.point ) );
		b120 = p2.plane.project( barycenter( 1 / 3.0f, 2 / 3.0f, p1.point, p2.point, p3.point ) );
		b021 = p2.plane.project( barycenter( 0 / 3.0f, 2 / 3.0f, p1.point, p2.point, p3.point ) );
		b102 = p3.plane.project( barycenter( 1 / 3.0f, 0 / 3.0f, p1.point, p2.point, p3.point ) );
		b012 = p3.plane.project( barycenter( 0 / 3.0f, 1 / 3.0f, p1.point, p2.point, p3.point ) );
		castor::Point3f e = ( b210 + b120 + b021 + b012 + b102 + b201 ) / 6.0f;
		b111 = e + ( e - barycenter( 1 / 3.0f, 1 / 3.0f, p1.point, p2.point, p3.point ) ) / 2.0f;
	}

	String const Subdivider::Name = cuT( "PN-Triangles Divider" );
	String const Subdivider::Type = cuT( "pn_tri" );

	Subdivider::Subdivider()
		: castor3d::MeshSubdivider()
		, m_occurences( 1 )
	{
	}

	Subdivider::~Subdivider()
	{
		cleanup();
	}

	MeshSubdividerUPtr Subdivider::create()
	{
		return std::make_unique< Subdivider >();
	}

	void Subdivider::cleanup()
	{
		castor3d::MeshSubdivider::cleanup();
	}

	void Subdivider::subdivide( SubmeshSPtr submesh
		, int occurences
		, bool generateBuffers
		, bool threaded )
	{
		m_occurences = occurences;
		m_submesh = submesh;
		m_generateBuffers = generateBuffers;
		m_submesh->computeContainers();

		doInitialise();
		m_threaded = threaded;

		if ( threaded )
		{
			m_thread = std::make_shared< std::thread >( std::bind( &Subdivider::doSubdivideThreaded, this ) );
		}
		else
		{
			doSubdivide();
			doAddGeneratedFaces();
			doSwapBuffers();
		}
	}

	void Subdivider::doSubdivide()
	{
		auto facesArray = m_indexMapping->getFaces();
		m_indexMapping->clearFaces();
		std::map< uint32_t, Plane > posnml;
		uint32_t i = 0;

		for ( auto const & point : m_submesh->getPoints() )
		{
			castor::Point3f position = point.pos;
			castor::Point3f normal = point.nml;
			posnml.emplace( i++, Plane{ castor::PlaneEquation( normal, position ), position } );
		}

		for ( auto const & face : facesArray )
		{
			doComputeFaces( 0.0, 0.0, 1.0, 1.0, m_occurences, Patch( posnml[face[0]], posnml[face[1]], posnml[face[2]] ) );
		}
	}

	void Subdivider::doInitialise()
	{
		for ( uint32_t i = 0; i < m_submesh->getPointsCount(); ++i )
		{
			m_points.emplace_back( std::make_unique< SubmeshVertex >( SubmeshVertex{ i, m_submesh->getPoint( i ) } ) );
		}

		castor3d::MeshSubdivider::doInitialise();
		m_indexMapping = m_submesh->getComponent< TriFaceMapping >();
	}

	void Subdivider::doAddGeneratedFaces()
	{
		for ( auto const & face : m_arrayFaces )
		{
			m_indexMapping->addFace( face[0], face[1], face[2] );
		}
	}

	void Subdivider::doComputeFaces( double u0
		, double v0
		, double u2
		, double v2
		, int occurences
		, Patch const & patch )
	{
		double u1 = ( u0 + u2 ) / 2.0;
		double v1 = ( v0 + v2 ) / 2.0;

		if ( occurences > 1 )
		{
			doComputeFaces( u0, v0, u1, v1, occurences - 1, patch );
			doComputeFaces( u0, v1, u1, v2, occurences - 1, patch );
			doComputeFaces( u1, v0, u2, v1, occurences - 1, patch );
			doComputeFaces( u1, v1, u0, v0, occurences - 1, patch );
		}
		else
		{
			auto & a = doComputePoint( double( u0 ), double( v0 ), patch );
			auto & b = doComputePoint( double( u2 ), double( v0 ), patch );
			auto & c = doComputePoint( double( u0 ), double( v2 ), patch );
			auto & d = doComputePoint( double( u1 ), double( v0 ), patch );
			auto & e = doComputePoint( double( u1 ), double( v1 ), patch );
			auto & f = doComputePoint( double( u0 ), double( v1 ), patch );
			doSetTextCoords( a, b, c, d, e, f );
		}
	}

	castor3d::SubmeshVertex & Subdivider::doComputePoint( double u, double v, Patch const & patch )
	{
		double w = 1.0 - u - v;
		double u2 = double( u * u );
		double v2 = double( v * v );
		double w2 = double( w * w );
		double u3 = double( u2 * u );
		double v3 = double( v2 * v );
		double w3 = double( w2 * w );

		castor::Point3f point = castor::Point3f( patch.b300 * float( w3 )
			+ patch.b030 * float( u3 )
			+ patch.b003 * float( v3 )
			+ patch.b210 * float( 3.0 * w2 * u )
			+ patch.b120 * float( 3.0 * w * u2 )
			+ patch.b201 * float( 3.0 * w2 * v )
			+ patch.b021 * float( 3.0 * u2 * v )
			+ patch.b102 * float( 3.0 * w * v2 )
			+ patch.b012 * float( 3.0 * u * v2 )
			+ patch.b111 * float( 6.0 * w * u * v ) );

		return doTryAddPoint( point );
	}
}
