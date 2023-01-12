project(CFE_RF_TLM C)

include_directories(fsw/mission_inc)
include_directories(fsw/platform_inc)
include_directories(${rf_tlm_MISSION_DIR}/fsw/platform_inc)

# Includes for the apps that send telemetry
include_directories(${blinky_MISSION_DIR}/fsw/platform_inc)
include_directories(${blinky_MISSION_DIR}/fsw/src)

include_directories(${imu_app_MISSION_DIR}/fsw/platform_inc)
include_directories(${imu_app_MISSION_DIR}/fsw/src)

include_directories(${altitude_app_MISSION_DIR}/fsw/platform_inc)
include_directories(${altitude_app_MISSION_DIR}/fsw/src)

# Create the app module
add_cfe_app(rf_tlm fsw/src/rf_tlm.c)
