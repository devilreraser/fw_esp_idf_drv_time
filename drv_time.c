/* *****************************************************************************
 * File:   drv_time.c
 * Author: Dimitar Lilov
 *
 * Created on 2022 06 18
 * 
 * Description: ...
 * 
 **************************************************************************** */

/* *****************************************************************************
 * Header Includes
 **************************************************************************** */
#include "drv_time.h"

#if CONFIG_DRV_RTC_USE
#include "drv_rtc.h"
#endif


#if CONFIG_DRV_SNTP_USE
#include "drv_sntp.h"
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "esp_log.h"


/* *****************************************************************************
 * Configuration Definitions
 **************************************************************************** */
#define TAG "drv_time"

/* *****************************************************************************
 * Constants and Macros Definitions
 **************************************************************************** */
#define USE_SNTP    1

/* *****************************************************************************
 * Enumeration Definitions
 **************************************************************************** */

/* *****************************************************************************
 * Type Definitions
 **************************************************************************** */

/* *****************************************************************************
 * Function-Like Macros
 **************************************************************************** */

/* *****************************************************************************
 * Variables Definitions
 **************************************************************************** */
int8_t s8UTCOffsetHours = 0;        /* Used for local debug print */
struct tm timeinfo_get;
bool bTimeStampUseManual = false;

/* *****************************************************************************
 * Prototype of functions definitions
 **************************************************************************** */

/* *****************************************************************************
 * Functions
 **************************************************************************** */
void time_sync_notification_cb(struct timeval *tv)
{
    #if CONFIG_DRV_RTC_USE
    struct timeval tv_now1;
    struct timeval tv_now2;
    time_t local_now;
    do
    {
        gettimeofday(&tv_now1, NULL);
        time(&local_now);
        gettimeofday(&tv_now2, NULL); 
        //to do exit if time not rolling 
    } while (tv_now1.tv_sec == tv_now2.tv_sec);
    
    bool bWriteSuccess = drv_rtc_time_wr(tv_now2.tv_sec);

    if (bWriteSuccess)
    {
        ESP_LOGI(TAG, "RTC Write Success");
    }
    else
    {
        ESP_LOGE(TAG, "RTC Write Failure");
    }
    #endif  /* #if CONFIG_DRV_RTC_USE */

    ESP_LOGW(  TAG, LOG_COLOR(LOG_COLOR_CYAN)"Notification of a time synchronization event");
    drv_time_print("On SNTP Sync");
}


void drv_time_init(void)
{
    #if CONFIG_DRV_RTC_USE

    struct tm rtc_time_info;
    char strftime_buf_get[64];
    //struct tm* p_rtc_time_info = drv_rtc_read();

    time_t now = drv_rtc_time_rd();
    localtime_r(&now, &rtc_time_info);  //strftime - exception if not used localtime_r

    //memcpy((uint8_t*)&rtc_time_info, (uint8_t*)p_rtc_time_info, sizeof(struct tm));
    
    ESP_LOGI(TAG, "Try Setting time from RTC next");  

    if (rtc_time_info.tm_year >= (2020 - 1900))
    {
        ESP_LOGI(TAG, "rtc_time_info.tm_year >= (2020 - 1900)"); 
        time_t new_set = mktime(&rtc_time_info);
        struct timeval now = { .tv_sec = new_set };
        settimeofday(&now, NULL);

        strftime(strftime_buf_get, sizeof(strftime_buf_get)-1, "%c", &rtc_time_info);
        ESP_LOGW(TAG, "Setting time from RTC: %s", strftime_buf_get);
        //ESP_LOGW(TAG, "Setting time from RTC: %s", asctime(p_rtc_time_info));
    }
    else
    {
        ESP_LOGI(TAG, "rtc_time_info.tm_year < (2020 - 1900)");  
        //vTaskDelay(pdMS_TO_TICKS(100));
        //drv_cfg_print_heap(1);
        //drv_cfg_print_stack(1);
        //cmd_system_tasks_info();


        strftime(strftime_buf_get, sizeof(strftime_buf_get)-1, "%c", &rtc_time_info);  //strftime - exception if not used localtime_r
        ESP_LOGE(TAG, "Invalid time from RTC: %s", strftime_buf_get);
        ESP_LOGE(TAG, "Invalid time from RTC: %s", asctime(&rtc_time_info)); //asctime - exception if not used localtime_r

        struct tm rtc_time_info_set;
        
        time(&now);
        localtime_r(&now, &rtc_time_info_set);
        ESP_LOGE(TAG, "Setting time in I2C RTC: %s", asctime(&rtc_time_info_set)); //asctime - exception if not used localtime_r
        if (drv_rtc_time_wr(now))
        {

        }
    }

    #endif /* #if CONFIG_DRV_RTC_USE */

    #if CONFIG_DRV_SNTP_USE
    drv_time_print("On Init Before sntp");
    drv_sntp_initialize_sntp(time_sync_notification_cb);
    drv_sntp_set_time_manual(bTimeStampUseManual);
    drv_time_print("On Init After  sntp");
    #endif

}

void drv_time_print(char* tag)
{
    #if DEBUG_NO_TIME == 0

    char strftime_buf[64];
    struct tm local_timeinfo;
    time_t local_now;

    time(&local_now);   /* get local time */
    
    //timezones list
    /* https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv */

    // // Set timezone to Eastern Standard Time and print local time
    // setenv("TZ", "EST5EDT,M3.2.0/2,M11.1.0", 1);
    // tzset();
    // localtime_r(&local_now, &local_timeinfo);
    // strftime(strftime_buf, sizeof(strftime_buf), "%c", &local_timeinfo);
    // ESP_LOGI(TAG, "The current date/time in New York is: %s", strftime_buf);

    // // Set timezone to China Standard Time
    // setenv("TZ", "CST-8", 1);
    // tzset();
    // localtime_r(&local_now, &local_timeinfo);
    // strftime(strftime_buf, sizeof(strftime_buf), "%c", &local_timeinfo);
    // ESP_LOGI(TAG, "The current date/time in Shanghai is: %s", strftime_buf);

    // // Set timezone to Europe/Sofia Time
    // setenv("TZ", "EET-2EEST,M3.5.0/3,M10.5.0/4", 1);
    // tzset();
    // localtime_r(&local_now, &local_timeinfo);
    // strftime(strftime_buf, sizeof(strftime_buf), "%c", &local_timeinfo);
    // ESP_LOGI(TAG, "The current date/time in Sofia    is: %s", strftime_buf);

    // Set timezone to UTC

    sprintf(strftime_buf, "GMT-%d",s8UTCOffsetHours);
    setenv("TZ", strftime_buf, 1);

    //setenv("TZ", "GMT0", 1);
    tzset();
    localtime_r(&local_now, &local_timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &local_timeinfo);
    ESP_LOGI(TAG, "%s date/time GMT-%d -------- is: %s", tag, s8UTCOffsetHours, strftime_buf);
    #endif
}

struct tm* drv_time_get (suseconds_t* pusec)
{
    #if DEBUG_NO_TIME == 0
    time_t now_get;
    //time_t now1;
    struct timeval tv_now1;
    //time_t now2;
    struct timeval tv_now2;

    do
    {
        gettimeofday(&tv_now1, NULL);
        time(&now_get);
        gettimeofday(&tv_now2, NULL);  
        
    } while (tv_now1.tv_sec != tv_now2.tv_sec);
    
    *pusec = tv_now2.tv_usec;

    // Set timezone to UTC
    setenv("TZ", "GMT0", 1);
    tzset();
    
    localtime_r(&now_get, &timeinfo_get);

    char strftime_buf_get[64];
    strftime(strftime_buf_get, sizeof(strftime_buf_get), "%c", &timeinfo_get);  //strftime - exception if not used localtime_r
    ESP_LOGD(TAG, "The current date/time is: %s ms:%6d", strftime_buf_get, (size_t)tv_now2.tv_usec);
    #endif
    return &timeinfo_get;
}


void drv_time_set_time_manual(bool input)
{
    #if CONFIG_DRV_SNTP_USE
    bTimeStampUseManual = input;
    drv_sntp_set_time_manual(bTimeStampUseManual);
    #endif
}
