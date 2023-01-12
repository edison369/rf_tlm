# Core Flight System : Framework : App : RF Telemetry Output

This application is a non-flight utility. It is intended to be located in the apps/rf_tlm subdirectory of a cFS Mission Tree.

rf_tlm is cFS telemetry app for RTEMS Beaglebone Black that sends data packets over a I2C to a RF system, this is not portable. This app subscribes and consumes data from different cFS applications.
