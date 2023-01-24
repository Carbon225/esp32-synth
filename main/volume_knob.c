#include <volume_knob.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <driver/gpio.h>

#include <esp_log.h>
static const char TAG[] = "volume_knob";


#define VOLUME_KNOB_GPIO_A 34
#define VOLUME_KNOB_GPIO_B 35
#define VOLUME_MAX 100
#define DEBOUNCE_TIME_US 1000


static bool g_last_state = false;
static int g_last_isr = 0;
static int g_volume = 50;
static portMUX_TYPE g_volume_mux = portMUX_INITIALIZER_UNLOCKED;


static IRAM_ATTR void encoder_isr_handler(void *arg)
{
    int now = esp_timer_get_time();

    bool a = gpio_get_level(VOLUME_KNOB_GPIO_A);
    bool b = gpio_get_level(VOLUME_KNOB_GPIO_B);

    if (a == g_last_state) return;

    if (now - g_last_isr < DEBOUNCE_TIME_US) return;

    g_last_isr = now;
    g_last_state = a;

    if (a == b) g_volume -= 1;
    else g_volume += 1;

    if (g_volume < 0) g_volume = 0;
    else if (g_volume > VOLUME_MAX) g_volume = VOLUME_MAX;
}

void volume_knob_init(void)
{
    ESP_LOGI(TAG, "Initializing volume knob");

    // configure falling interrupt for GPIO A
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    io_conf.pin_bit_mask = (1ULL << VOLUME_KNOB_GPIO_A);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    // configure GPIO B as input
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pin_bit_mask = (1ULL << VOLUME_KNOB_GPIO_B);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    // install ISR service with default configuration
    gpio_install_isr_service(ESP_INTR_FLAG_IRAM);

    g_last_state = gpio_get_level(VOLUME_KNOB_GPIO_A);

    // attach the interrupt service routine
    gpio_isr_handler_add(VOLUME_KNOB_GPIO_A, encoder_isr_handler, NULL);
}

void volume_knob_override(int volume)
{
    portENTER_CRITICAL(&g_volume_mux);
    g_volume = volume;
    portEXIT_CRITICAL(&g_volume_mux);
}

int volume_knob_get(void)
{
    portENTER_CRITICAL(&g_volume_mux);
    int volume = g_volume;
    portEXIT_CRITICAL(&g_volume_mux);
    return volume;
}
