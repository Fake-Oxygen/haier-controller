target_include_directories(app PRIVATE inc/haierctrl/drivers)
target_include_directories(app PRIVATE drivers/heatpump)

target_sources(app PRIVATE heatpump/heatpump.c heatpump/haier_cmd.c)
