#include <my_display.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <lvgl.h>
#include <lvgl_helpers.h>
#include <volume_knob.h>
#include <main.h>

#include <esp_log.h>
static const char TAG[] = "my_display";


#define LV_TICK_PERIOD_MS 1


static SemaphoreHandle_t g_gui_sem;
static lv_obj_t *g_volume_slider;
static lv_obj_t *g_volume_label;
static TickType_t g_last_play = -1;


static void lv_tick_task(void *arg)
{
    lv_tick_inc(LV_TICK_PERIOD_MS);
}

static void mute_btn_event_cb(lv_obj_t * btn, lv_event_t event)
{
    if(event == LV_EVENT_CLICKED)
    {
        volume_knob_override(0);
    }
}

static void play_btn_event_cb(lv_obj_t * btn, lv_event_t event)
{
    if(event == LV_EVENT_CLICKED)
    {
        synth_note_on(72, 100);
        g_last_play = xTaskGetTickCount();
    }
}

static void slider_event_cb(lv_obj_t * slider, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED)
    {
        int volume = lv_slider_get_value(slider);
        volume_knob_override(volume);
    }
}

static void lv_create_ui(void)
{
    lv_obj_t *scr, *label, *btn, *slider;
    
    scr = lv_scr_act();

    // mute
    btn = lv_btn_create(scr, NULL);
    lv_obj_set_pos(btn, 10, 10);
    lv_obj_set_size(btn, 120, 50);
    lv_obj_set_event_cb(btn, mute_btn_event_cb);
    label = lv_label_create(btn, NULL);
    lv_label_set_text(label, "Mute");

    // volume
    slider = lv_slider_create(scr, NULL);
    lv_obj_set_size(slider, 200, 20);
    lv_obj_align(slider, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_slider_set_range(slider, 0, 100);
    lv_slider_set_value(slider, 0, LV_ANIM_OFF);
    lv_obj_set_event_cb(slider, slider_event_cb);
    g_volume_slider = slider;
    label = lv_label_create(scr, NULL);
    lv_label_set_text(label, "Volume: 0%%");
    lv_obj_align(label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_16);
    g_volume_label = label;

    // play note
    btn = lv_btn_create(scr, NULL);
    lv_obj_set_pos(btn, 10, 70);
    lv_obj_set_size(btn, 120, 50);
    lv_obj_set_event_cb(btn, play_btn_event_cb);
    label = lv_label_create(btn, NULL);
    lv_label_set_text(label, "Play note");
}

static void gui_task(void *arg)
{
    g_gui_sem = xSemaphoreCreateMutex();

    lv_init();

    lvgl_driver_init();

    lv_color_t* buf1 = (lv_color_t*) heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf1 != NULL);

    lv_color_t* buf2 = (lv_color_t*) heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf2 != NULL);

    static lv_disp_buf_t disp_buf;

    uint32_t size_in_px = DISP_BUF_SIZE;

    lv_disp_buf_init(&disp_buf, buf1, buf2, size_in_px);

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = disp_driver_flush;

    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);

	lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.read_cb = touch_driver_read;
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    lv_indev_drv_register(&indev_drv);

    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_gui"
    };
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));

    lv_create_ui();

    int last_volume = -1;

    for (;;)
    {
        vTaskDelay(pdMS_TO_TICKS(10));
        if (xSemaphoreTake(g_gui_sem, portMAX_DELAY) == pdTRUE)
        {
            int volume = volume_knob_get();
            if (volume != last_volume)
            {
                lv_slider_set_value(g_volume_slider, volume, LV_ANIM_ON);
                lv_label_set_text_fmt(g_volume_label, "Volume: %d%%", volume);
                last_volume = volume;
            }

            if (g_last_play != -1 && xTaskGetTickCount() - g_last_play > pdMS_TO_TICKS(200))
            {
                synth_note_off(72);
                g_last_play = -1;
            }

            lv_task_handler();
            xSemaphoreGive(g_gui_sem);
       }
    }
}

void my_display_init(void)
{
    ESP_LOGI(TAG, "Initializing display");
    xTaskCreate(gui_task, "gui", 4096, NULL, 5, NULL);
}
