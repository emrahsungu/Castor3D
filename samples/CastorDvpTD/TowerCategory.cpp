#include "CastorDvpTD/TowerCategory.hpp"

using namespace castor;
using namespace castor3d;

namespace castortd
{
	LongRangeTower::LongRangeTower()
		: Category{ Tower::Category::Kind::eLongRange
			, cuT( "armature_short_range.1|attack" ) }
	{
		auto costIncrement = []( uint32_t p_inc
			, uint32_t p_value
			, uint32_t p_level )
		{
			return p_value + p_inc + p_inc * ( p_level / 5 );
		};
		auto uintIncrement = []( uint32_t p_inc
			, uint32_t p_value
			, uint32_t p_level )
		{
			return p_value + p_inc;
		};
		auto floatIncrement = []( float p_inc
			, float p_value
			, uint32_t p_level )
		{
			return p_value + p_inc;
		};
		auto decrement = []( Milliseconds p_inc
			, Milliseconds p_value
			, uint32_t p_level )
		{
			p_value -= p_inc;

			if ( p_value < p_inc )
			{
				p_value = p_inc;
			}

			return p_value;
		};
		m_damage.initialise( 5u
			, std::bind( uintIncrement, 9u, std::placeholders::_1, std::placeholders::_2 )
			, 400u
			, std::bind( costIncrement, 30u, std::placeholders::_1, std::placeholders::_2 )
			, 15u );

		m_initialCooldown = Milliseconds{ 6000u };
		m_cooldown.initialise( m_initialCooldown
			, std::bind( decrement, Milliseconds{ 240u }, std::placeholders::_1, std::placeholders::_2 )
			, 200u
			, std::bind( costIncrement, 20u, std::placeholders::_1, std::placeholders::_2 ) );

		m_range.initialise( 100.0f
			, std::bind( floatIncrement, 20.0f, std::placeholders::_1, std::placeholders::_2 )
			, 150u
			, std::bind( costIncrement, 10u, std::placeholders::_1, std::placeholders::_2 ) );

		m_bulletSpeed = 96.0f;
		m_towerCost = 250u;
		m_material = cuT( "OrangeTowerCube" );
		m_colour = RgbColour::fromComponents( 1.0f, 1.0f, 0.0f );
	}

	ShortRangeTower::ShortRangeTower()
		: Category{ Tower::Category::Kind::eShortRange
			, cuT( "armature_short_range.1|attack" ) }
	{
		auto costIncrement = []( uint32_t p_inc
			, uint32_t p_value
			, uint32_t p_level )
		{
			return p_value + p_inc + p_inc * ( p_level / 5 );
		};
		auto uintIncrement = []( uint32_t p_inc
			, uint32_t p_value
			, uint32_t p_level )
		{
			return p_value + p_inc;
		};
		auto floatIncrement = []( float p_inc
			, float p_value
			, uint32_t p_level )
		{
			return p_value + p_inc;
		};
		auto decrement = []( Milliseconds p_inc
			, Milliseconds p_value
			, uint32_t p_level )
		{
			p_value -= p_inc;

			if ( p_value < p_inc )
			{
				p_value = p_inc;
			}

			return p_value;
		};
		m_damage.initialise( 3u
			, std::bind( uintIncrement, 5u, std::placeholders::_1, std::placeholders::_2 )
			, 400u
			, std::bind( costIncrement, 30u, std::placeholders::_1, std::placeholders::_2 ) );

		m_initialCooldown = Milliseconds{ 1000u };
		m_cooldown.initialise( m_initialCooldown
			, std::bind( decrement, Milliseconds{ 50u }, std::placeholders::_1, std::placeholders::_2 )
			, 150u
			, std::bind( costIncrement, 10u, std::placeholders::_1, std::placeholders::_2 ) );

		m_range.initialise( 40.0f
			, std::bind( floatIncrement, 4.0f, std::placeholders::_1, std::placeholders::_2 )
			, 200u
			, std::bind( costIncrement, 20u, std::placeholders::_1, std::placeholders::_2 ) );

		m_bulletSpeed = 120.0f;
		m_towerCost = 170u;
		m_material = cuT( "BlueTowerCube" );
		m_colour = RgbColour::fromComponents( 0.0f, 0.0f, 1.0f );
	}
}
