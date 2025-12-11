#include "esp_rom_sys.h"
#include "net_time_sntp.h"
#include "esp_sntp.h"
#include "esp_log.h"
#include <time.h>
#include <stdlib.h>

static const char *TAG = "SNTP";

bool sntpSyncOnce(const char *server, uint32_t timeoutMs)
{
    // Prevent re-initialization
    sntp_stop();

    esp_rom_printf("[SNTP] Initializing SNTP using server: %s\n", server);
    
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, (char*)server);
    sntp_init();

    uint32_t waited = 0;
    bool syncSuccess = false; // Used to track successful sync
    
    while (waited < timeoutMs) {
        sntp_sync_status_t status = sntp_get_sync_status();
        
        // Lock success state immediately upon receiving a successful sync status
        if (status == SNTP_SYNC_STATUS_COMPLETED) {
            esp_rom_printf("[SNTP] Status: 1 (Success!) -> Locking success state.\n");
            syncSuccess = true; // Lock success state
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
        waited += 1000;
        
        esp_rom_printf("[SNTP] Waiting... (%lu/%lu ms) Status: %d\n", 
                       (unsigned long)waited, (unsigned long)timeoutMs, status);
    }
    
    // Use syncSuccess variable for final check instead of re-calling sntp_get_sync_status()
    // This ensures success is reported even if the status reverts later
    if (syncSuccess) {
        esp_rom_printf("[SNTP] Time synced successfully!\n");
        
        // Print time
        setenv("TZ", "EST5EDT,M3.2.0,M11.1.0", 1); 
        tzset(); 
        
        time_t now;
        struct tm timeinfo;
        time(&now);
        localtime_r(&now, &timeinfo);
        char strftime_buf[128];
        strftime(strftime_buf, sizeof(strftime_buf), "%Y-%m-%d %H:%M:%S", &timeinfo);

        esp_rom_printf("[SNTP] Current time: %s\n", strftime_buf);
        
    } else {
        esp_rom_printf("[SNTP] Timeout - System time may be wrong\n");
    }
    
    return syncSuccess;
}