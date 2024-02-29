find_package(Qt6 REQUIRED COMPONENTS Core Network Positioning Location)

qt_add_library(Terrain STATIC
	TerrainQuery.cc
	TerrainQuery.h
    TerrainTile.cc
    TerrainTile.h
)

target_link_libraries(Terrain
	PRIVATE
		Qt6::LocationPrivate
		qgc
		QtLocationPlugin
	PUBLIC
		Qt6::Core
		Qt6::Positioning
		Qt6::Network
)

target_include_directories(Terrain
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)
