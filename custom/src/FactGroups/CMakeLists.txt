find_package(Qt6 REQUIRED COMPONENTS Core)

target_sources(VehicleFactGroups
    PRIVATE
        GCSFactGroup.cc GCSFactGroup.h
        GPUFactGroup.cc GPUFactGroup.h
        NextVisionFactGroup.cc NextVisionFactGroup.h
        PayloadFactGroup.cc PayloadFactGroup.h
        ViewproFactGroup.cc ViewproFactGroup.h
)

target_include_directories(VehicleFactGroups PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
