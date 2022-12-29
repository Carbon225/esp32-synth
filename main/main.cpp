#include <esp_log.h>
#include <Synth.h>

static const char TAG[] = "main";

extern "C" void app_main(void)
{
    Synth synth;
    ESP_LOGI(TAG, "Hello world!");
}
