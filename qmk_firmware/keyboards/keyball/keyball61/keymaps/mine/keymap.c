/*
Copyright 2022 @Yowkees
Copyright 2022 MURAOKA Taro (aka KoRoN, @kaoriya)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include QMK_KEYBOARD_H

#include "quantum.h"
#include "led_anim.h"

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  [0] = LAYOUT_universal(
    KC_ESC   , KC_1     , KC_2     , KC_3     , KC_4     , KC_5     ,                                  KC_6     , KC_7     , KC_8     , KC_9     , KC_0     , KC_MINS  ,
    KC_DEL   , KC_Q     , KC_W     , KC_E     , KC_R     , KC_T     ,                                  KC_Y     , KC_U     , KC_I     , KC_O     , KC_P     , KC_INT3  ,
    KC_TAB   , KC_A     , KC_S     , KC_D     , KC_F     , KC_G     ,                                  KC_H     , KC_J     , KC_K     , KC_L     , KC_SCLN  , S(KC_7)  ,
    MO(1)    , KC_Z     , KC_X     , KC_C     , KC_V     , KC_B     , KC_RBRC  ,              KC_NUHS, KC_N     , KC_M     , KC_COMM  , KC_DOT   , KC_SLSH  , KC_RSFT  ,
    _______  , KC_LCTL  , KC_LALT  , KC_LGUI,LT(1,KC_LNG2),LT(2,KC_SPC),LT(3,KC_LNG1),    KC_BSPC,LT(2,KC_ENT),LT(1,KC_LNG2),KC_RGUI, _______ , KC_RALT  , KC_PSCR
  ),

  [1] = LAYOUT_universal(
    S(KC_ESC), S(KC_1)  , KC_LBRC  , S(KC_3)  , S(KC_4)  , S(KC_5)  ,                                  KC_EQL   , S(KC_6)  ,S(KC_QUOT), S(KC_8)  , S(KC_9)  ,S(KC_INT1),
    S(KC_DEL), S(KC_Q)  , S(KC_W)  , S(KC_E)  , S(KC_R)  , S(KC_T)  ,                                  S(KC_Y)  , S(KC_U)  , S(KC_I)  , S(KC_O)  , S(KC_P)  ,S(KC_INT3),
    S(KC_TAB), S(KC_A)  , S(KC_S)  , S(KC_D)  , S(KC_F)  , S(KC_G)  ,                                  S(KC_H)  , S(KC_J)  , S(KC_K)  , S(KC_L)  , KC_QUOT  , S(KC_2)  ,
    _______  , S(KC_Z)  , S(KC_X)  , S(KC_C)  , S(KC_V)  , S(KC_B)  ,S(KC_RBRC),           S(KC_NUHS), S(KC_N)  , S(KC_M)  ,S(KC_COMM), S(KC_DOT),S(KC_SLSH),S(KC_RSFT),
    _______  ,S(KC_LCTL),S(KC_LALT),S(KC_LGUI), _______  , _______  , _______  ,            _______  , _______  , _______  ,S(KC_RGUI), _______  , S(KC_RALT), _______
  ),

  [2] = LAYOUT_universal(
    SSNP_FRE , KC_F1    , KC_F2    , KC_F3    , KC_F4    , KC_F5    ,                                  KC_F6    , KC_F7    , KC_F8    , KC_F9    , KC_F10   , KC_F11   ,
    SSNP_VRT , _______  , KC_7     , KC_8     , KC_9     , _______  ,                                  _______  , KC_LEFT  , KC_UP    , KC_RGHT  , _______  , KC_F12   ,
    SSNP_HOR , _______  , KC_4     , KC_5     , KC_6     ,S(KC_SCLN),                                  KC_PGUP  , KC_BTN1  , KC_DOWN  , KC_BTN2  , KC_BTN3  , _______  ,
    _______  , _______  , KC_1     , KC_2     , KC_3     ,S(KC_MINS), S(KC_8)  ,            S(KC_9)  , KC_PGDN  , _______  , _______  , _______  , _______  , _______  ,
    _______  , _______  , KC_0     , KC_DOT   , _______  , _______  , _______  ,             KC_DEL  , _______  , _______  , _______  , _______  , _______  , _______
  ),

  [3] = LAYOUT_universal(
    RGB_TOG  , AML_TO   , AML_I50  , AML_D50  , _______  , _______  ,                                  RGB_M_P  , RGB_M_B  , RGB_M_R  , RGB_M_SW , RGB_M_SN , RGB_M_K  ,
    RGB_MOD  , RGB_HUI  , RGB_SAI  , RGB_VAI  , _______  , _______  ,                                  RGB_M_X  , RGB_M_G  , RGB_M_T  , RGB_M_TW , _______  , _______  ,
    RGB_RMOD , RGB_HUD  , RGB_SAD  , RGB_VAD  , _______  , _______  ,                                  CPI_D1K  , CPI_D100 , CPI_I100 , CPI_I1K  , KBC_SAVE , KBC_RST  ,
    _______  , _______  , SCRL_DVD , SCRL_DVI , SCRL_MO  , SCRL_TO  , EE_CLR   ,            EE_CLR   , KC_HOME  , KC_PGDN  , KC_PGUP  , KC_END   , _______  , _______  ,
    QK_BOOT  , _______  , KC_LEFT  , KC_DOWN  , KC_UP    , KC_RGHT  , _______  ,            _______  , KC_BSPC  , _______  , _______  , _______  , _______  , QK_BOOT
  ),

  // オートマウスレイヤー: J=左クリック, K=ホイール押し込み, L=右クリック, ;=押している間スクロール
  [4] = LAYOUT_universal(
    _______  , _______  , _______  , _______  , _______  , _______  ,                                  _______  , _______  , _______  , _______  , _______  , _______  ,
    _______  , _______  , _______  , _______  , _______  , _______  ,                                  _______  , _______  , _______  , _______  , _______  , _______  ,
    _______  , _______  , _______  , _______  , _______  , _______  ,                                  _______  , KC_BTN1  , KC_BTN3  , KC_BTN2  , SCRL_MO  , _______  ,
    _______  , _______  , _______  , _______  , _______  , _______  , _______  ,            _______  , _______  , _______  , _______  , _______  , _______  , _______  ,
    _______  , _______  , _______  , _______  , _______  , _______  , _______  ,            _______  , _______  , _______  , _______  , _______  , _______  , _______
  ),
};
// clang-format on

void keyboard_post_init_user(void) {
    // オートマウスの判定はUSB側（マスター）だけで行う。
    // スレーブ側でも有効だと、同期されたレイヤー4を自分のタイマーで消そうとして
    // レイヤーとLEDが毎ループ切り替わり、左右通信を妨げてしまう
    if (!is_keyboard_master()) {
        set_auto_mouse_enable(false);
    }
    led_anim_init();
}

// PCのIMEの状態は読めないので、最後に押された かな(LNG1)／英数(LNG2) キーで全角入力中かを推測する
static bool is_kana_input = false;

static void track_input_mode(uint16_t keycode, keyrecord_t *record) {
    if (!record->event.pressed) {
        return;
    }
    if (IS_QK_LAYER_TAP(keycode)) {
        // レイヤーキーはタップした時だけ かな／英数 キーとして働く
        if (record->tap.count == 0) {
            return;
        }
        keycode = QK_LAYER_TAP_GET_TAP_KEYCODE(keycode);
    }
    if (keycode == KC_LNG1) {
        is_kana_input = true;
    } else if (keycode == KC_LNG2) {
        is_kana_input = false;
    }
}

layer_state_t layer_state_set_user(layer_state_t state) {
    // レイヤー3が有効な間はスクロールモード
    // ・オートマウスのレイヤー4が上に重なっても解除されないよう最上位レイヤーでは判定しない
    // ・レイヤー4の SCRL_MO を妨げないよう、レイヤー3のON/OFFが変わった時だけ切り替える
    static bool was_layer3 = false;
    bool        is_layer3  = layer_state_cmp(state, 3);
    if (is_layer3 != was_layer3) {
        was_layer3 = is_layer3;
        keyball_set_scroll_mode(is_layer3);
        // レイヤー3の間は入力モードを半角英数にし、抜けたら全角入力だった場合だけ元に戻す
        // （キー送信はUSB側だけ。is_kana_input はここでは変えないので、抜ける時に元の状態がわかる）
        if (is_keyboard_master()) {
            if (is_layer3) {
                tap_code(KC_LNG2);
            } else if (is_kana_input) {
                tap_code(KC_LNG1);
            }
        }
    }
    return state;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    // オートマウスのレイヤー4中に左手側のキーを押したら、キーの種類に関係なくレイヤー4を抜ける
    // （標準では修飾キーや、ボールを動かした直後のキー入力では抜けないため）
    // レイヤーを消すだけでなく判定状態もリセットし、ボールの惰性ですぐ再点灯しないようにする
    bool is_left_key = record->event.key.row < MATRIX_ROWS / 2;
    if (record->event.pressed && is_left_key && layer_state_is(AUTO_MOUSE_DEFAULT_LAYER)) {
        auto_mouse_reset_trigger(true);
    }
    track_input_mode(keycode, record);
    return true;
}

report_mouse_t pointing_device_task_user(report_mouse_t report) {
    led_anim_add_motion(report.x, report.y);
    return report;
}

// 左右両方で実行される（レイヤー状態は SPLIT_LAYER_STATE_ENABLE で共有）
void housekeeping_task_user(void) {
    led_anim_task();
}

#ifdef OLED_ENABLE

#    include "lib/oledkit/oledkit.h"

void oledkit_render_info_user(void) {
    keyball_oled_render_keyinfo();
    keyball_oled_render_ballinfo();
    keyball_oled_render_layerinfo();
}
#endif
