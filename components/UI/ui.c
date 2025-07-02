#if defined(EEZ_FOR_LVGL)
#include <eez/core/vars.h>
#endif

#include "ui.h"
#include "screens.h"
#include "images.h"
#include "actions.h"
#include "vars.h"


volatile bool GIF_end_flag = false;

LV_IMG_DECLARE(LEBREW);


#if defined(EEZ_FOR_LVGL)

void ui_init() {
    eez_flow_init(assets, sizeof(assets), (lv_obj_t **)&objects, sizeof(objects), images, sizeof(images), actions);
}

void ui_tick() {
    eez_flow_tick();
    tick_screen(g_currentScreen);
}

#else

static int16_t currentScreen = -1;

static lv_obj_t *getLvglObjectFromIndex(int32_t index) {
    if (index == -1) {
        return 0;
    }
    return ((lv_obj_t **)&objects)[index];
}

void loadScreen(enum ScreensEnum screenId) {
    currentScreen = screenId - 1;
    lv_obj_t *screen = getLvglObjectFromIndex(currentScreen);
    lv_scr_load_anim(screen, LV_SCR_LOAD_ANIM_FADE_IN, 200, 0, false);
}

void ui_init() {
    create_screens();
    // loadScreen(SCREEN_ID_MEASURE_TDS);
    lv_example_gif2();
}

void ui_tick() {
    tick_screen(currentScreen);
}
static void delayed_delete_cb(lv_timer_t * timer) {
    printf("delayed_delete_cb\r\n");  // 打印回调信息
    lv_timer_del(timer);;
    loadScreen(SCREEN_ID_MEASURE_TDS);
    GIF_end_flag = true;
}
// GIF 播放结束的回调函数
static void gif_end_cb(lv_event_t * e) {

    // printf("GIF 播放结束\n");

    // 获取 GIF 控件对象
    // lv_obj_t * gif = lv_event_get_target(e);
    lv_event_code_t event = lv_event_get_code(e);
    if (event == LV_EVENT_READY)
    {
        lv_timer_create(delayed_delete_cb, 300, NULL);
    }

}

void  lv_example_gif2(void) {

     // 创建一个新的容器对象，作为当前屏幕的子对象
    lv_obj_t* prj_parent_cont = lv_obj_create(lv_scr_act());
    // 设置容器的内边距为0，即容器内容与容器边缘没有间隙
    lv_obj_set_style_pad_all(prj_parent_cont, 0, 0);
    // 设置容器的边框宽度为0，即容器没有边框
    lv_obj_set_style_border_width(prj_parent_cont, 0, 0);
    // 设置容器的大小为当前屏幕的100%，即容器会填满整个屏幕
    lv_obj_set_size(prj_parent_cont, LV_PCT(100), LV_PCT(100));
    // 调用宏UI_PARENT_INIT来执行额外的初始化操作（如样式、事件等），这个宏的内容可能在其他地方定义
    UI_PARENT_INIT(prj_parent_cont);
    // 设置容器的水平滚动对齐方式为居中（内容会自动居中显示）
    lv_obj_set_scroll_snap_x(prj_parent_cont, LV_SCROLL_SNAP_CENTER);
    // 设置容器的垂直滚动对齐方式为居中（内容会自动居中显示）
    lv_obj_set_scroll_snap_y(prj_parent_cont, LV_SCROLL_SNAP_CENTER);
    // 禁用容器的滚动条，滚动时不显示滚动条
    lv_obj_set_scrollbar_mode(prj_parent_cont, LV_SCROLLBAR_MODE_OFF);
    // 给容器对象添加一个事件回调函数，这里是一个通用的回调（`LV_EVENT_ALL`），处理所有类型的事件
    // 事件回调函数 `event_handler` 将会处理容器上的触摸、点击等事件
    // 传递的 `NULL` 表示没有附加的用户数据
    // lv_obj_add_event_cb(prj_parent_cont, event_handler, LV_EVENT_ALL, NULL);
    // 创建 GIF 控件并从 C 数组中加载数据
    lv_obj_t * gif1 = lv_gif_create(prj_parent_cont);  // 在当前屏幕上创建 GIF 控件
    lv_gif_set_src(gif1, &LEBREW);             // 使用嵌入式的 GIF 数据
    lv_obj_align(gif1, LV_ALIGN_CENTER, 0, 0); // 设置 GIF 在屏幕中央显示
    ((lv_gif_t*)gif1)->gif->loop_count = 1;//设置只播放一次
    // 添加播放结束事件回调
    lv_obj_add_event_cb(gif1, gif_end_cb, LV_EVENT_READY, NULL);


    //     // 在当前屏幕上直接创建 GIF 控件
    // lv_obj_t * gif = lv_gif_create(lv_scr_act());
    // lv_gif_set_src(gif, &LEBREW);
    // lv_obj_center(gif); // 居中显示
    // // 不需要设置 loop_count，也不用加事件回调
}



void ui_event_page_load_tds_screen(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    if(code == LV_EVENT_SCREEN_LOAD_START)
    {
        switch (Screens_ID)
        {
        case SCREEN_ID_MEASURE_TDS:
            left_or_right_Animation(objects.tds_container, 0, 26, 500, 1, 100);
            left_or_right_Animation(objects.tds_temp_container, 232, 214, 500, 1, 100);
            break;
        case SCREEN_ID_MEASURE_TEMP:
            top_or_bottom_Animation(objects.temp_container, 81, 46, 500, 1, 100);
            top_or_bottom_Animation(objects.language_switch_prompt, 76, 52, 500, 1, 100);
            break;
        case SCREEN_ID_LANGUAGE_PAGE:
            left_or_right_Animation(objects.chinese_bt, 0, 18, 500, 1, 100);
            left_or_right_Animation(objects.engilsh_bt, 186, 168, 500, 1, 100);
            break;
        case SCREEN_ID_CALIBRATION_PAGE:
            top_or_bottom_Animation(objects.calibration_bt, -86, 0, 500, 1, 100);
            top_or_bottom_Animation(objects.calibration_prompt, 76, 52, 500, 1, 100);
            break;
        case SCREEN_ID_ABOUT_PAGE:
                // left_or_right_Animation(objects.)
            break;

        default:
            break;
        }
    }
}

volatile bool calibration_flags = false;
void ui_calibration_event(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    if(code == LV_EVENT_CLICKED)
    {
        // lv_obj_set_style_bg_color(target, lv_color_hex(0xff6b45dd), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_text(lv_obj_get_child(objects.calibration_bt,0), UI_STRING[4][SYS_DATA.language_id]);
        calibration_flags = false;
    }
}




/**
 * @brief 让目标对象在X方向从100滑动到0，有弹性动画
 * @param TargetObject 要动画的对象
 * @param delay        延时多少ms后再开始动画
 */
// void left_Animation(lv_obj_t * TargetObject, int32_t start_value, int32_t end_value, uint32_t anim_time, uint16_t cnt, int delay)
// {
//     lv_anim_t a;
//     lv_anim_init(&a);   // 初始化动画对象结构体（所有成员清零）
//     /* 必需的设置
//     *-----------------*/
//     /* 设置动画回调函数（每帧调用此函数执行动画，比如设置x坐标） */
//     lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t) lv_obj_set_x);
//     /* 设置动画目标对象 */
//     lv_anim_set_var(&a, TargetObject);
//     /* 设置动画总时长（单位ms） */
//     lv_anim_set_time(&a, anim_time);
//     /* 设置动画的起始值和结束值，比如0到150 */
//     lv_anim_set_values(&a, start_value, end_value);
//     /* 可选设置
//     *------------------*/
//     /* 设置动画启动前的延时（ms） */
//     lv_anim_set_delay(&a, delay);
//     /* 设置动画的曲线（插值方式），默认线性，这里用“ease_in” */
//     lv_anim_set_path(&a, lv_anim_path_ease_in);
//     /* 动画完成（到达终点并空闲）时的回调函数 */
//     // lv_anim_set_ready_cb(&a, ready_cb);
//     // /* 动画被删除（idle）时的回调函数 */
//     // lv_anim_set_deleted_cb(&a, deleted_cb);
//     // /* 动画实际启动（延时后）时的回调函数 */
//     // lv_anim_set_start_cb(&a, start_cb);
//     // /* 动画完成后，是否自动反向播放，以及反向时长。0表示不反向 */
//     // lv_anim_set_playback_time(&a, time);
//     /* 反向播放前的延迟，0为不延迟 */
//     // lv_anim_set_playback_delay(&a, delay);
//     // /* 动画重复次数，1为播放一次，LV_ANIM_REPEAT_INFINITE为无限循环 */
//     lv_anim_set_repeat_count(&a, cnt);
//     /* 每次重复前的延迟，默认为0 */
//     // lv_anim_set_repeat_delay(&a, delay);
//     /*
//     * 是否提前应用初始值（true=立刻生效，false=等到动画真正启动再生效）。
//     * 默认true。部分场合（如淡入淡出）建议用false更自然。
//     */
//     lv_anim_set_early_apply(&a, true/false);
//     /* 启动动画
//     *------------------*/
//     lv_anim_start(&a);   // 正式启动动画

// }

void left_or_right_Animation(lv_obj_t * TargetObject, int32_t start_value, int32_t end_value, uint32_t anim_time, uint16_t cnt, uint32_t delay)
{
    lv_anim_t a;
    lv_anim_init(&a);   // 初始化动画对象结构体（所有成员清零）
    /* 设置动画回调函数（每帧调用此函数执行动画，比如设置x坐标） */
    lv_anim_set_delay(&a, delay);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t) lv_obj_set_x);
    /* 设置动画目标对象 */
    lv_anim_set_var(&a, TargetObject);
    /* 设置动画总时长（单位ms） */
    lv_anim_set_time(&a, anim_time);
    /* 设置动画的起始值和结束值，比如0到150 */
    lv_anim_set_values(&a, start_value, end_value);
    /* 设置动画的曲线（插值方式），默认线性，这里用“ease_in” */
    lv_anim_set_path_cb(&a, lv_anim_path_overshoot);
    // /* 动画重复次数，1为播放一次，LV_ANIM_REPEAT_INFINITE为无限循环 */
    lv_anim_set_repeat_count(&a, cnt);
    /* 每次重复前的延迟，默认为0 */
    // lv_anim_set_repeat_delay(&a, delay);
    /*
    * 是否提前应用初始值（true=立刻生效，false=等到动画真正启动再生效）。
    * 默认true。部分场合（如淡入淡出）建议用false更自然。
    */
    lv_anim_set_early_apply(&a, true/false);
    lv_anim_start(&a);   // 正式启动动画

}
void top_or_bottom_Animation(lv_obj_t * TargetObject, int32_t start_value, int32_t end_value, uint32_t anim_time, uint16_t cnt, uint32_t delay)
{
    lv_anim_t a;
    lv_anim_init(&a);   // 初始化动画对象结构体（所有成员清零）
    /* 设置动画回调函数（每帧调用此函数执行动画，比如设置x坐标） */
    lv_anim_set_delay(&a, delay);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t) lv_obj_set_y);
    /* 设置动画目标对象 */
    lv_anim_set_var(&a, TargetObject);
    /* 设置动画总时长（单位ms） */
    lv_anim_set_time(&a, anim_time);
    /* 设置动画的起始值和结束值，比如0到150 */
    lv_anim_set_values(&a, start_value, end_value);
    /* 设置动画的曲线（插值方式），默认线性，这里用“ease_in” */
    lv_anim_set_path_cb(&a, lv_anim_path_overshoot);
    // /* 动画重复次数，1为播放一次，LV_ANIM_REPEAT_INFINITE为无限循环 */
    lv_anim_set_repeat_count(&a, cnt);
    /* 每次重复前的延迟，默认为0 */
    // lv_anim_set_repeat_delay(&a, delay);
    /*
    * 是否提前应用初始值（true=立刻生效，false=等到动画真正启动再生效）。
    * 默认true。部分场合（如淡入淡出）建议用false更自然。
    */
    lv_anim_set_early_apply(&a, true/false);
    lv_anim_start(&a);   // 正式启动动画

}



#endif
