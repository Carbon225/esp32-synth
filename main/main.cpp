#include <i2sbuf.h>
#include <Synth.h>
#include <esp32_midi.h>
#include <my_display.h>
#include <volume_knob.h>

#include <esp_log.h>
static const char TAG[] = "main";


#define MIDI_UART UART_NUM_0
#define MIDI_RX_GPIO GPIO_NUM_3

#define I2S_NUM (I2S_NUM_0)
#define I2S_WS (GPIO_NUM_16)
#define I2S_DO (GPIO_NUM_17)
#define I2S_CK (GPIO_NUM_5)

#define SAMPLE_RATE (44100)

#define BUF_LEN (64)
#define BUF_COUNT (2)


static SerialMIDI g_midi;
static Synth g_synth;
static SemaphoreHandle_t g_synth_mtx;


static void audio_callback(int16_t buf[][2], int n_samples, void *user_data)
{
    xSemaphoreTake(g_synth_mtx, portMAX_DELAY);
	for (int i = 0; i < n_samples; ++i)
	{
		float sample = g_synth.getSample();
		buf[i][0] = buf[i][1] = (int16_t) (sample * (float) INT16_MAX);
	}
	xSemaphoreGive(g_synth_mtx);
}

static void note_on(uint8_t note, uint8_t vel)
{
	xSemaphoreTake(g_synth_mtx, portMAX_DELAY);
    g_synth.pressKey(note, vel);
	xSemaphoreGive(g_synth_mtx);
}

static void note_off(uint8_t note)
{
	xSemaphoreTake(g_synth_mtx, portMAX_DELAY);
    g_synth.releaseKey(note);
	xSemaphoreGive(g_synth_mtx);
}

static void handle_midi_message(midi_message_t msg)
{
	switch (msg.event)
	{
	// 0 data bytes
	case midi_event_t::TimingClock:
		ESP_LOGI(TAG, "Timing clock");
		break;

	case midi_event_t::Undefined:
		ESP_LOGI(TAG, "Undefined");
		break;

	case midi_event_t::Start:
		ESP_LOGI(TAG, "Start");
		break;

	case midi_event_t::Continue:
		ESP_LOGI(TAG, "Continue");
		break;

	case midi_event_t::Stop:
		ESP_LOGI(TAG, "Stop");
		break;

	case midi_event_t::ActiveSense:
		ESP_LOGI(TAG, "Active sense");
		break;

	case midi_event_t::SystemReset:
		ESP_LOGI(TAG, "System reset");
		break;

	// 1 data byte
	case midi_event_t::ProgramChange:
		ESP_LOGI(TAG, "Program change");
		break;

	case midi_event_t::ChannelPressure:
		ESP_LOGI(TAG, "Channel pressure");
		break;

	case midi_event_t::MidiTimeCode:
		ESP_LOGI(TAG, "Midi time code");
		break;

	case midi_event_t::SongSelect:
		ESP_LOGI(TAG, "System reset");
		break;

	// 2 data bytes
	case midi_event_t::NoteOff:
		ESP_LOGI(TAG, "Note off");
        note_off(msg.data[0]);
		break;

	case midi_event_t::NoteOn:
		ESP_LOGI(TAG, "Note on %d %d", msg.data[0], msg.data[1]);
        // velocity 0 is note off
        msg.data[1] != 0 ? note_on(msg.data[0], msg.data[1]) : note_off(msg.data[0]);
		break;

	case midi_event_t::PolyPressure:
		ESP_LOGI(TAG, "Poly pressure");
		break;

	case midi_event_t::ControlChange:
		ESP_LOGI(TAG, "Control change %d %d", msg.data[0], msg.data[1]);
		break;

	case midi_event_t::PitchBend:
		ESP_LOGI(TAG, "Pitch bend");
		break;

	case midi_event_t::SongPosition:
		ESP_LOGI(TAG, "Song position");
		break;
	}
}

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "Starting");

	// g_synth_mtx = xSemaphoreCreateMutex();

    // i2sbuf_config_t config = {
    //     .i2s_port = I2S_NUM,
    //     .ws_io = I2S_WS,
    //     .do_io = I2S_DO,
    //     .clk_io = I2S_CK,
    //     .sample_rate = SAMPLE_RATE,
    //     .use_apll = true,
    //     .buf_len = BUF_LEN,
    //     .buf_count = BUF_COUNT,
    //     .callback = audio_callback,
    //     .user_data = NULL,
    // };

    // ESP_ERROR_CHECK(i2sbuf_install(&config));

    // ESP_ERROR_CHECK(g_midi.RegisterCallback(handle_midi_message));
	// ESP_ERROR_CHECK(g_midi.Install(MIDI_UART, MIDI_RX_GPIO));

	volume_knob_init();
	my_display_init();

    ESP_LOGI(TAG, "Running");
}
