#include "Castor3D/Overlay/DebugOverlays.hpp"

#include "Castor3D/Engine.hpp"
#include "Castor3D/Cache/MaterialCache.hpp"
#include "Castor3D/Event/Frame/FunctorEvent.hpp"
#include "Castor3D/Overlay/Overlay.hpp"
#include "Castor3D/Overlay/PanelOverlay.hpp"
#include "Castor3D/Overlay/TextOverlay.hpp"
#include "Castor3D/Render/RenderPassTimer.hpp"
#include "Castor3D/Render/RenderSystem.hpp"

#include <ashespp/Core/Device.hpp>
#include <ashespp/Miscellaneous/QueryPool.hpp>

#include <iomanip>

using namespace castor;

namespace castor3d
{
	//*********************************************************************************************

	namespace
	{
		String getFullName( RenderPassTimer & timer )
		{
			auto timerName = timer.getName();
			auto timerCategory = timer.getCategory();

			if ( timerName.find( cuT( "ShadowMap" ) ) != String::npos )
			{
				if ( timerName.find( cuT( "Directional" ) ) != String::npos )
				{
					return cuT( "ShadowMap: Directional" );
				}

				if ( timerName.find( cuT( "Point" ) ) != String::npos )
				{
					return cuT( "ShadowMap: Point" );
				}

				if ( timerName.find( cuT( "Spot" ) ) != String::npos )
				{
					return cuT( "ShadowMap: Spot" );
				}
			}

			if ( timerName.find( cuT( "UBOs" ) ) != String::npos )
			{
				return cuT( "UBOs" );
			}

			if ( timerCategory == cuT( "Light Propagation Volumes" )
				|| timerCategory == cuT( "Reflective Shadow Maps" )
				|| timerCategory == cuT( "Scalable Ambient Obscurance" ) )
			{
				return timerCategory;
			}

			return timerCategory == timerName
				? timerCategory
				: timerCategory + cuT( ": " ) + timerName;
		}
	}

	//*********************************************************************************************

	DebugOverlays::MainDebugPanel::MainDebugPanel( OverlayCache & cache )
		: m_cache{ cache }
		, m_panel{ m_cache.add( cuT( "MainDebugPanel" )
			, OverlayType::ePanel
			, nullptr
			, nullptr )->getPanelOverlay() }
		, m_times{ std::make_unique< DebugPanels< castor::Nanoseconds > >( cuT( "Times" ), m_panel, cache ) }
		, m_fps{ std::make_unique< DebugPanels< float > >( cuT( "FPS" ), m_panel, cache ) }
		, m_counts{ std::make_unique< DebugPanels< uint32_t > >( cuT( "Counts" ), m_panel, cache ) }
	{
		auto & materials = m_cache.getEngine()->getMaterialCache();
		m_panel->setPixelPosition( Position{ 0, 0 } );
		m_panel->setPixelSize( Size{ 320, 20 } );
		m_panel->setMaterial( materials.find( cuT( "AlphaDarkBlue" ) ) );
		m_panel->setVisible( true );
	}

	DebugOverlays::MainDebugPanel::~MainDebugPanel()
	{
		m_times.reset();
		m_fps.reset();
		m_counts.reset();

		if ( m_panel )
		{
			m_cache.remove( m_panel->getOverlay().getName() );
			m_panel.reset();
		}
	}

	void DebugOverlays::MainDebugPanel::update()
	{
		m_times->update();
		m_fps->update();
		m_counts->update();
	}

	void DebugOverlays::MainDebugPanel::setVisible( bool visible )
	{
		m_panel->setVisible( visible );
	}

	void DebugOverlays::MainDebugPanel::updatePosition()
	{
		int y = m_times->updatePosition( 0 );
		y = m_fps->updatePosition( y );
		y = m_counts->updatePosition( y );
		m_panel->setPixelSize( Size{ 320, uint32_t( y ) } );
	}

	void DebugOverlays::MainDebugPanel::addTimePanel( castor::String const & name
		, castor::String const & label
		, castor::Nanoseconds const & value )
	{
		m_times->add( name
			, label
			, value );
	}

	void DebugOverlays::MainDebugPanel::addCountPanel( castor::String const & name
		, castor::String const & label
		, uint32_t const & value )
	{
		m_counts->add( name
			, label
			, value );
	}

	void DebugOverlays::MainDebugPanel::addFpsPanel( castor::String const & name
		, castor::String const & label
		, float const & value )
	{
		m_fps->add( name
			, label
			, value );
	}

	//*********************************************************************************************

	DebugOverlays::RenderPassOverlays::RenderPassOverlays( castor::String const & category
		, OverlayCache & cache )
		: m_cache{ cache }
	{
		auto baseName = cuT( "RenderPassOverlays-" ) + category;
		m_panel = cache.add( baseName
			, OverlayType::ePanel
			, nullptr
			, nullptr )->getPanelOverlay();
		m_titlePanel = cache.add( baseName + cuT( "_Title" )
			, OverlayType::ePanel
			, nullptr
			, m_panel->getOverlay().shared_from_this() )->getPanelOverlay();
		m_titleText = cache.add( baseName + cuT( "_TitleText" )
			, OverlayType::eText
			, nullptr
			, m_titlePanel->getOverlay().shared_from_this() )->getTextOverlay();
		m_cpu.name = cache.add( baseName + cuT( "_CPUName" )
			, OverlayType::eText
			, nullptr
			, m_panel->getOverlay().shared_from_this() )->getTextOverlay();
		m_cpu.value = cache.add( baseName + cuT( "_CPUValue" )
			, OverlayType::eText
			, nullptr
			, m_panel->getOverlay().shared_from_this() )->getTextOverlay();
		m_gpu.name = cache.add( baseName + cuT( "_GPUName" )
			, OverlayType::eText
			, nullptr
			, m_panel->getOverlay().shared_from_this() )->getTextOverlay();
		m_gpu.value = cache.add( baseName + cuT( "_GPUValue" )
			, OverlayType::eText
			, nullptr
			, m_panel->getOverlay().shared_from_this() )->getTextOverlay();

		m_panel->setPixelPosition( Position{ 330, 0u } );
		m_titlePanel->setPixelPosition( Position{ 0, 0 } );
		m_titleText->setPixelPosition( Position{ 10, 0 } );
		m_cpu.name->setPixelPosition( Position{ 10, 20 } );
		m_cpu.value->setPixelPosition( Position{ 45, 20 } );
		m_gpu.name->setPixelPosition( Position{ 130, 20 } );
		m_gpu.value->setPixelPosition( Position{ 165, 20 } );

		m_panel->setPixelSize( Size{ 250, 40 } );
		m_titlePanel->setPixelSize( Size{ 250, 20 } );
		m_titleText->setPixelSize( Size{ 230, 20 } );
		m_cpu.name->setPixelSize( Size{ 30, 20 } );
		m_cpu.value->setPixelSize( Size{ 75, 20 } );
		m_gpu.name->setPixelSize( Size{ 30, 20 } );
		m_gpu.value->setPixelSize( Size{ 75, 20 } );

		m_titleText->setFont( cuT( "Arial20" ) );
		m_cpu.name->setFont( cuT( "Arial10" ) );
		m_gpu.name->setFont( cuT( "Arial10" ) );
		m_cpu.value->setFont( cuT( "Arial10" ) );
		m_gpu.value->setFont( cuT( "Arial10" ) );

		m_titleText->setCaption( category );
		m_cpu.name->setCaption( cuT( "CPU:" ) );
		m_gpu.name->setCaption( cuT( "GPU:" ) );

		auto & materials = cache.getEngine()->getMaterialCache();
		m_panel->setMaterial( materials.find( cuT( "AlphaDarkBlue" ) ) );
		m_titlePanel->setMaterial( materials.find( cuT( "AlphaDarkBlue" ) ) );
		m_titleText->setMaterial( materials.find( cuT( "White" ) ) );
		m_cpu.name->setMaterial( materials.find( cuT( "White" ) ) );
		m_gpu.name->setMaterial( materials.find( cuT( "White" ) ) );
		m_cpu.value->setMaterial( materials.find( cuT( "White" ) ) );
		m_gpu.value->setMaterial( materials.find( cuT( "White" ) ) );

		m_titleText->setVAlign( VAlign::eCenter );
		m_titleText->setHAlign( HAlign::eLeft );

		m_panel->setVisible( true );
		m_titlePanel->setVisible( true );
		m_titleText->setVisible( true );
		m_cpu.name->setVisible( true );
		m_gpu.name->setVisible( true );
		m_cpu.value->setVisible( true );
		m_gpu.value->setVisible( true );
	}

	void DebugOverlays::RenderPassOverlays::initialise( uint32_t index )
	{
		m_panel->setPixelPosition( Position{ 330, int32_t( 40 * index ) } );
	}

	DebugOverlays::RenderPassOverlays::~RenderPassOverlays()
	{
		if ( m_cpu.name )
		{
			m_cache.remove( m_cpu.name->getOverlayName() );
			m_cache.remove( m_gpu.name->getOverlayName() );
			m_cache.remove( m_gpu.value->getOverlayName() );
			m_cache.remove( m_cpu.value->getOverlayName() );
			m_cache.remove( m_titleText->getOverlayName() );
			m_cache.remove( m_titlePanel->getOverlayName() );
			m_cache.remove( m_panel->getOverlayName() );
		}
	}

	void DebugOverlays::RenderPassOverlays::addTimer( RenderPassTimer & timer )
	{
		m_timers.emplace_back( std::ref( timer ) );
	}

	bool DebugOverlays::RenderPassOverlays::removeTimer( RenderPassTimer & timer )
	{
		auto it = std::find_if( m_timers.begin()
			, m_timers.end()
			, [&timer]( auto const & lookup )
			{
				return &lookup.get() == &timer;
			} );

		if ( it != m_timers.end() )
		{
			m_timers.erase( it );
		}

		return m_timers.empty();
	}

	void DebugOverlays::RenderPassOverlays::update()
	{
		m_cpu.time = 0_ns;
		m_gpu.time = 0_ns;

		for ( auto & timer : m_timers )
		{
			m_cpu.time += timer.get().getCpuTime();
			m_gpu.time += timer.get().getGpuTime();
			timer.get().reset();
		}

		m_cpu.value->setCaption( makeStringStream() << m_cpu.time );
		m_gpu.value->setCaption( makeStringStream() << m_gpu.time );
	}

	void DebugOverlays::RenderPassOverlays::retrieveGpuTime()
	{
		for ( auto & timer : m_timers )
		{
			timer.get().retrieveGpuTime();
		}
	}

	void DebugOverlays::RenderPassOverlays::setVisible( bool visible )
	{
		m_panel->setVisible( visible );
	}

	//*********************************************************************************************

	DebugOverlays::DebugOverlays( Engine & engine )
		: OwnedBy< Engine >( engine )
	{
	}

	DebugOverlays::~DebugOverlays()
	{
	}

	void DebugOverlays::initialise( OverlayCache & cache )
	{
		doCreateDebugPanel( cache );
	}

	void DebugOverlays::cleanup()
	{
		m_debugPanel.reset();
	}

	RenderInfo & DebugOverlays::beginFrame()
	{
		if ( m_visible )
		{
			m_gpuTime = 0_ns;
			m_cpuTime = 0_ns;
			m_taskTimer.getElapsed();
		}

		m_renderInfo = RenderInfo{};

		if ( m_dirty )
		{
			uint32_t index = 0u;

			for ( auto & renderPass : m_renderPasses )
			{
				renderPass.second.initialise( index++ );
			}
		}

		m_externalTime = m_frameTimer.getElapsed();
		return m_renderInfo;
	}

	uint32_t DebugOverlays::registerTimer( RenderPassTimer & timer )
	{
		auto & cache = getEngine()->getOverlayCache();
		auto fullName = getFullName( timer );
		auto it = m_renderPasses.find( fullName );

		if ( it == m_renderPasses.end() )
		{
			it = m_renderPasses.emplace( fullName
				, RenderPassOverlays{ fullName, cache } ).first;
			it->second.setVisible( m_visible );
		}

		it->second.addTimer( std::ref( timer ) );
		m_dirty = true;
		return m_queriesCount++;
	}

	void DebugOverlays::unregisterTimer( RenderPassTimer & timer )
	{
		m_dirty = true;
		auto fullName = getFullName( timer );
		auto it = m_renderPasses.find( fullName );

		if ( it != m_renderPasses.end() )
		{
			if ( it->second.removeTimer( timer ) )
			{
				m_renderPasses.erase( it );
			}
		}
	}

	castor::Microseconds DebugOverlays::endFrame()
	{
		m_totalTime = m_frameTimer.getElapsed() + m_externalTime;

		if ( m_visible )
		{
			m_framesTimes[m_frameIndex] = m_totalTime;
			m_averageTime = std::accumulate( m_framesTimes.begin(), m_framesTimes.end(), 0_ns ) / m_framesTimes.size();
			m_averageFps = 1000000.0f / std::chrono::duration_cast< std::chrono::microseconds >( m_averageTime ).count();
			m_fps = 1000000.0f / std::chrono::duration_cast< std::chrono::microseconds >( m_totalTime ).count();
			m_gpuTotalTime = 0_ns;
			m_gpuClientTime = 0_ns;

			for ( auto & pass : m_renderPasses )
			{
				pass.second.update();
				m_gpuTotalTime += pass.second.getGpuTime();
				m_gpuClientTime += pass.second.getCpuTime();
			}

			//m_gpuClientTime = m_gpuTime - m_gpuTotalTime;
			m_debugPanel->update();
			getEngine()->getRenderSystem()->resetGpuTime();

			m_frameIndex = ++m_frameIndex % FRAME_SAMPLES_COUNT;
			m_frameTimer.getElapsed();
		}

#if defined( NDEBUG )

		auto total = std::chrono::duration_cast< std::chrono::microseconds >( m_totalTime );
		fprintf( stdout
			, "\r%0.7f ms, %0.7f fps"
			, total.count() / 1000.0f
			, ( 1000000.0 / total.count() ) );

#else

		std::cout << "\rTime: " << std::setw( 7 ) << m_totalTime;
		std::cout << " - FPS: " << std::setw( 7 ) << std::setprecision( 4 ) << ( 1000000.0 / std::chrono::duration_cast< std::chrono::microseconds >( m_totalTime ).count() );

#endif

		return std::chrono::duration_cast< castor::Microseconds >( m_totalTime );
	}

	void DebugOverlays::endGpuTask()
	{
		for ( auto & renderPass : m_renderPasses )
		{
			renderPass.second.retrieveGpuTime();
		}

		m_gpuTime += m_taskTimer.getElapsed();
	}

	void DebugOverlays::endCpuTask()
	{
		m_cpuTime += m_taskTimer.getElapsed();
	}

	void DebugOverlays::show( bool show )
	{
		m_visible = show;
		m_debugPanel->setVisible( m_visible );

		for ( auto & pass : m_renderPasses )
		{
			pass.second.setVisible( m_visible );
		}
	}

	void DebugOverlays::doCreateDebugPanel( OverlayCache & cache )
	{
		m_debugPanel = std::make_unique< MainDebugPanel >( cache );
		m_debugPanel->addTimePanel( cuT( "CpuTime" )
			, cuT( "CPU Time:" )
			, m_cpuTime );
		m_debugPanel->addTimePanel( cuT( "GpuClientTime" )
			, cuT( "GPU Client Time:" )
			, m_gpuClientTime );
		m_debugPanel->addTimePanel( cuT( "GpuServerTime" )
			, cuT( "GPU Server Time:" )
			, m_gpuTotalTime );
		m_debugPanel->addTimePanel( cuT( "ExternalTime" )
			, cuT( "External Time:" )
			, m_externalTime );
		m_debugPanel->addTimePanel( cuT( "TotalTime" )
			, cuT( "Total Time:" )
			, m_totalTime );
		m_debugPanel->addTimePanel( cuT( "AverageTime" )
			, cuT( "Average Time:" )
			, m_averageTime );
		m_debugPanel->addFpsPanel( cuT( "FPS" )
			, cuT( "FPS:" )
			, m_fps );
		m_debugPanel->addFpsPanel( cuT( "AverageFPS" )
			, cuT( "Average FPS:" )
			, m_averageFps );
		m_debugPanel->addCountPanel( cuT( "TotalVertexCount" )
			, cuT( "Vertices Count:" )
			, m_renderInfo.m_totalVertexCount );
		m_debugPanel->addCountPanel( cuT( "TotalFaceCount" )
			, cuT( "Faces Count:" )
			, m_renderInfo.m_totalFaceCount );
		m_debugPanel->addCountPanel( cuT( "TotalObjectCount" )
			, cuT( "Objects Count:" )
			, m_renderInfo.m_totalObjectsCount );
		m_debugPanel->addCountPanel( cuT( "VisibleVertexCount" )
			, cuT( "Visible Vertices Count:" )
			, m_renderInfo.m_visibleVertexCount );
		m_debugPanel->addCountPanel( cuT( "VisibleFaceCount" )
			, cuT( "Visible Faces Count:" )
			, m_renderInfo.m_visibleFaceCount );
		m_debugPanel->addCountPanel( cuT( "VisibleObjectCount" )
			, cuT( "Visible Objects Count:" )
			, m_renderInfo.m_visibleObjectsCount );
		m_debugPanel->addCountPanel( cuT( "ParticlesCount" )
			, cuT( "Particles Count:" )
			, m_renderInfo.m_particlesCount );
		m_debugPanel->addCountPanel( cuT( "LightCount" )
			, cuT( "Lights Count:" )
			, m_renderInfo.m_totalLightsCount );
		m_debugPanel->addCountPanel( cuT( "VisibleLightCount" )
			, cuT( "Visible Lights Count:" )
			, m_renderInfo.m_visibleLightsCount );
		m_debugPanel->addCountPanel( cuT( "DrawCalls" )
			, cuT( "Draw calls:" )
			, m_renderInfo.m_drawCalls );
		m_debugPanel->updatePosition();
		m_debugPanel->setVisible( m_visible );
	}

	//*********************************************************************************************
}
