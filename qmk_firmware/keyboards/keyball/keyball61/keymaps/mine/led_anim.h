#pragma once

#include <stdint.h>

// レイヤー連動のLEDアニメーション（左右それぞれが自分側のLEDだけを描画する）

// keyboard_post_init_user から呼ぶ
void led_anim_init(void);

// housekeeping_task_user から毎ループ呼ぶ
void led_anim_task(void);

// マスター側のマウスレポートを渡す（レイヤー4の光のスクロール用）
void led_anim_add_motion(int16_t x, int16_t y);
