#include <i2sbuf.h>
#include <Synth.h>
#include <esp32_midi.h>
#include <lvgl.h>
#include <lvgl_helpers.h>

#include <esp_log.h>
static const char TAG[] = "main";


#define LV_TICK_PERIOD_MS 1

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

static void btn_event_cb(lv_obj_t * btn, lv_event_t event)
{
    if(event == LV_EVENT_CLICKED) {
        static uint8_t cnt = 0;
        cnt++;

        /*Get the first child of the button which is the label and change its text*/
        lv_obj_t * label = lv_obj_get_child(btn, NULL);
        lv_label_set_text_fmt(label, "Button: %d", cnt);
    }
}

/**
 * Create a button with a label and react on Click event.
 */
static void lv_ex_get_started_1(void)
{
    lv_obj_t * btn = lv_btn_create(lv_scr_act(), NULL);     /*Add a button the current screen*/
    lv_obj_set_pos(btn, 10, 10);                            /*Set its position*/
    lv_obj_set_size(btn, 120, 50);                          /*Set its size*/
    lv_obj_set_event_cb(btn, btn_event_cb);                 /*Assign a callback to the button*/

    lv_obj_t * label = lv_label_create(btn, NULL);          /*Add a label to the button*/
    lv_label_set_text(label, "Button");                     /*Set the labels text*/
}

static void lv_tick_task(void *arg) {
    (void) arg;

    lv_tick_inc(LV_TICK_PERIOD_MS);
}

static void guiTask(void *pvParameter) {

    (void) pvParameter;
    SemaphoreHandle_t xGuiSemaphore = xSemaphoreCreateMutex();

    lv_init();

    /* Initialize SPI or I2C bus used by the drivers */
    lvgl_driver_init();

    lv_color_t* buf1 = (lv_color_t*) heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf1 != NULL);

    /* Use double buffered when not working with monochrome displays */
    lv_color_t* buf2 = (lv_color_t*) heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf2 != NULL);


    static lv_disp_buf_t disp_buf;

    uint32_t size_in_px = DISP_BUF_SIZE;

    /* Initialize the working buffer depending on the selected display.
     * NOTE: buf2 == NULL when using monochrome displays. */
    lv_disp_buf_init(&disp_buf, buf1, buf2, size_in_px);

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = disp_driver_flush;

	#if defined CONFIG_DISPLAY_ORIENTATION_PORTRAIT || defined CONFIG_DISPLAY_ORIENTATION_PORTRAIT_INVERTED
    disp_drv.rotated = 1;
	#endif

    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);

	lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.read_cb = touch_driver_read;
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    lv_indev_drv_register(&indev_drv);

    /* Create and start a periodic timer interrupt to call lv_tick_inc */
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_gui"
    };
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));

    /* Create the demo application */
    lv_ex_get_started_1();

    while (1) {
        /* Delay 1 tick (assumes FreeRTOS tick is 10ms */
        vTaskDelay(pdMS_TO_TICKS(10));

        /* Try to take the semaphore, call lvgl related function on success */
        if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY)) {
            lv_task_handler();
            xSemaphoreGive(xGuiSemaphore);
       }
    }

    /* A task should NEVER return */
    free(buf1);
    free(buf2);
    vTaskDelete(NULL);
}

extern "C" void app_main(void)
{
	guiTask(NULL);

    // ESP_LOGI(TAG, "Starting");

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

    // ESP_LOGI(TAG, "Running");
}
