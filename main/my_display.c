#include <my_display.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <lvgl.h>
#include <lvgl_helpers.h>
#include <volume_knob.h>

#include <esp_log.h>
static const char TAG[] = "my_display";


#define LV_TICK_PERIOD_MS 1


SemaphoreHandle_t g_gui_sem;
lv_obj_t *g_volume_slider;


static void lv_tick_task(void *arg)
{
    lv_tick_inc(LV_TICK_PERIOD_MS);
}

static void btn_event_cb(lv_obj_t * btn, lv_event_t event)
{
    if(event == LV_EVENT_CLICKED) {
        static uint8_t cnt = 0;
        cnt++;
        lv_obj_t * label = lv_obj_get_child(btn, NULL);
        lv_label_set_text_fmt(label, "Button: %d", cnt);
    }
}

static void slider_event_cb(lv_obj_t * slider, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) {
        int volume = lv_slider_get_value(slider);
        volume_knob_override(volume);
    }
}

static void lv_create_ui(void)
{
    lv_obj_t *scr = lv_scr_act();

    lv_obj_t *btn = lv_btn_create(scr, NULL);
    lv_obj_set_pos(btn, 10, 10);
    lv_obj_set_size(btn, 120, 50);
    lv_obj_set_event_cb(btn, btn_event_cb);

    lv_obj_t *label = lv_label_create(btn, NULL);
    lv_label_set_text(label, "Button");

    lv_obj_t *slider = lv_slider_create(scr, NULL);
    lv_obj_set_size(slider, 200, 20);
    lv_obj_align(slider, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_slider_set_range(slider, 0, 100);
    lv_slider_set_value(slider, 0, LV_ANIM_OFF);
    lv_obj_set_event_cb(slider, slider_event_cb);
    g_volume_slider = slider;
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

    for (;;)
    {
        vTaskDelay(pdMS_TO_TICKS(10));
        if (xSemaphoreTake(g_gui_sem, portMAX_DELAY) == pdTRUE)
        {
            int volume = volume_knob_get();
            lv_slider_set_value(g_volume_slider, volume, LV_ANIM_OFF);

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
