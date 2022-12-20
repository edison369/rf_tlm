project(CFE_RF_TLM C)

include_directories(fsw/mission_inc)
include_directories(fsw/platform_inc)
include_directories(${rf_tlm_MISSION_DIR}/fsw/platform_inc)

# Create the app module
add_cfe_app(rf_tlm fsw/src/rf_tlm.c)

# Add table
# add_cfe_tables(sampleAppTable fsw/tables/sample_app_tbl.c)
