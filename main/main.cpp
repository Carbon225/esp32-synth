#include <tuple>
#include <i2sbuf.h>
#include <Synth.h>
#include <esp32_midi.h>

#include <esp_log.h>
static const char TAG[] = "main";


#define I2S_NUM (I2S_NUM_0)
#define I2S_WS (GPIO_NUM_16)
#define I2S_DO (GPIO_NUM_17)
#define I2S_CK (GPIO_NUM_5)

#define SAMPLE_RATE (48000)

#define BUF_LEN (256)
#define BUF_COUNT (4)


static Synth g_synth;


static void audio_callback(int16_t buf[][2], int n_samples, void *user_data)
{
	for (int i = 0; i < n_samples; ++i)
	{
		float left, right;
        std::tie(left, right) = g_synth.getSample();

		buf[i][0] = (int16_t) (left * (float) INT16_MAX);
		buf[i][1] = (int16_t) (right * (float) INT16_MAX);
	}
}

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "Starting");

    i2sbuf_config_t config = {
        .i2s_port = I2S_NUM,
        .ws_io = I2S_WS,
        .do_io = I2S_DO,
        .clk_io = I2S_CK,
        .sample_rate = SAMPLE_RATE,
        .use_apll = false,
        .buf_len = BUF_LEN,
        .buf_count = BUF_COUNT,
        .callback = audio_callback,
        .user_data = NULL,
    };

    ESP_ERROR_CHECK(i2sbuf_install(&config));

    ESP_LOGI(TAG, "Running");
}
