/*
This is the c configuration file for the keymap

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

#pragma once

// LEDのアニメーションは led_anim.c で自前描画するため、QMK標準のエフェクトはビルドしない

// レイヤー4の光のスクロール用に、マスター側のボールの移動量をスレーブへ送る
#define SPLIT_TRANSACTION_IDS_USER USER_SYNC_LED_MOTION

// 左右同期を切る
#undef  RGBLED_SPLIT
#undef  RGBLED_NUM
#define RGBLED_NUM 37

// keyball61.c の keyball_on_adjust_layout() を keymap.c 側で上書きする
#define KEYBALL_ADJUST_LAYOUT_USER

// レイヤー状態だけは左右で共有する（レイヤー連動ライティング用）
#define SPLIT_LAYER_STATE_ENABLE

#define TAP_CODE_DELAY 5

// ボール操作後の一定時間だけ、マウス専用のレイヤー4に切り替える
#define POINTING_DEVICE_AUTO_MOUSE_ENABLE
#define AUTO_MOUSE_DEFAULT_LAYER 4

// VIA のデフォルトは4レイヤーまでなので、レイヤー4を含む5レイヤーに拡張する
#define DYNAMIC_KEYMAP_LAYER_COUNT 5