idf_component_register(SRCS "drv_time.c" 
                    INCLUDE_DIRS "." 
                    REQUIRES "drv_rtc" "drv_sntp"
                                      )
                 

