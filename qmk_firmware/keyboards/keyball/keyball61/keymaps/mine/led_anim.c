#include QMK_KEYBOARD_H

#include "transactions.h"
#include "led_anim.h"

#ifdef RGBLIGHT_ENABLE

// 描画間隔。LED更新中は割り込みが止まるので、左右通信を妨げないよう間引く
#    define FRAME_MS 20
// マスターからスレーブへボール移動量を送る間隔
#    define MOTION_SYNC_MS 30

#    define LED_COUNT_BALL_SIDE 34
#    define LED_COUNT_NOBALL_SIDE 37
// レイヤー0で明滅させるLED（シルク番号8 = 左手の外側最上段 = ESC）
#    define ESC_LED_INDEX 7

// 座標の単位: キー1つ分 = 16
#    define POS_MAX_D 96 // 中央からの距離の最大（外側の列）
#    define POS_MAX_Y 64 // 上端からの距離の最大（最下段）
#    define CENTER_GAP 8 // 左右の間の空き（レイヤー4の模様を左右でつなげるため）

// 起動時: 中央から外側へ流れる光
#    define BOOT_MS_PER_UNIT 5
#    define BOOT_TAIL 48
// レイヤー1・3: 順番に点灯していく速さと、1個が点き切るまでの幅
#    define REVEAL_MS_PER_UNIT 4
#    define REVEAL_RAMP 16
// レイヤー0: ESCの明滅（2048ms周期。16bitタイマーの周期を割り切るので折り返しで乱れない）
#    define BREATH_SHIFT 3
// レイヤー4: ボールの移動量を模様の移動量にする割合（大きいほどゆっくり動く）
#    define MOTION_SHIFT 1

// 色相（QMK の HSV_* の色相部分）
#    define HUE_RED 0
#    define HUE_ORANGE 21
#    define HUE_GREEN 85
#    define HUE_CYAN 128
#    define HUE_BLUE 170

typedef struct {
    uint8_t d; // 中央（親指側）からの距離
    uint8_t y; // 上端からの距離
} led_pos_t;

// clang-format off
// インデックス = 基板シルクのLED番号 - 1。左右は同じ基板を裏返して使うので、
// 中央からの距離で表せば左右共通の表になる。
// 1〜7 は裏面のアンダーグロー、35〜37 はボール無し側の親指キーのみ（位置は推定）
static const led_pos_t PROGMEM LED_POS[LED_COUNT_NOBALL_SIDE] = {
    { 0, 60}, {24, 52}, {40, 52}, {96, 56}, {96, 24}, {72,  4}, {40,  4}, //  1- 7
    {96,  0}, {96, 16}, {96, 32}, {96, 48}, {96, 64},                     //  8-12 外側の列
    {80,  0}, {80, 16}, {80, 32}, {80, 48}, {80, 64},                     // 13-17
    {64,  0}, {64, 16}, {64, 32}, {64, 48},                               // 18-21
    {48,  0}, {48, 16}, {48, 32}, {48, 48},                               // 22-25
    {32,  0}, {32, 16}, {32, 32}, {32, 48},                               // 26-29
    {16,  0}, {16, 16}, {16, 32}, {16, 48},                               // 30-33 内側の列
    { 0, 56},                                                             // 34    親指
    {32, 64}, {48, 64}, {64, 64},                                         // 35-37
};
// clang-format on

typedef enum {
    ANIM_OFF,
    ANIM_BOOT,   // 中央から外側へ流れて消える
    ANIM_BREATH, // ESCだけ明滅
    ANIM_RISE,   // 下から順に点灯して点きっぱなし
    ANIM_SPREAD, // 中央から順に点灯して点きっぱなし
    ANIM_MOTION, // マウスカーソルの移動方向に模様がスクロール
} anim_t;

typedef struct {
    uint8_t anim; // anim_t
    uint8_t hue;
    // オレンジ・水色などの混色は2チャンネル同時点灯で単色の約2倍の電流が流れ、
    // USB給電不足でスレーブ側がリセットされるため、明るさを半分に抑える
    bool dim;
} light_t;

// clang-format off
// [0 = 左, 1 = 右]
static const light_t BOOT_LIGHT[2] = {{ANIM_BOOT, HUE_RED, false}, {ANIM_BOOT, HUE_BLUE, false}};

// [レイヤー][0 = 左, 1 = 右]
static const light_t LAYER_LIGHTS[][2] = {
    [0] = {{ANIM_BREATH, HUE_RED,    false}, {ANIM_OFF}},
    [1] = {{ANIM_RISE,   HUE_ORANGE, true},  {ANIM_RISE,   HUE_CYAN,  true}},
    [2] = {{ANIM_OFF},                       {ANIM_OFF}},
    [3] = {{ANIM_SPREAD, HUE_RED,    false}, {ANIM_SPREAD, HUE_BLUE,  false}},
    [4] = {{ANIM_MOTION, HUE_GREEN,  false}, {ANIM_MOTION, HUE_GREEN, false}},
};
// clang-format on

typedef struct {
    int16_t x;
    int16_t y;
} motion_t;

static struct {
    light_t  light;
    uint16_t start;
    uint16_t last_frame;
    uint8_t  layer;
    bool     booting;
    bool     finished; // 点灯し終わって描き直す必要がない
    bool     enabled;
    uint8_t  val;
    motion_t drawn_motion;
} state;

// マスターはマウスレポートから、スレーブはRPCで受け取って更新する（RPCは割り込み中に来る）
static volatile motion_t motion;

// 0 → 254 → 0 の三角波
static uint8_t triangle(uint8_t phase) {
    return phase < 128 ? phase * 2 : (255 - phase) * 2;
}

static uint8_t scale(uint8_t v, uint8_t b) {
    return ((uint16_t)v * b) >> 8;
}

// 距離 dist の位置まで波の先頭 front が来たら 0 → 255 に明るくなり、そのまま点きっぱなし
static uint8_t reveal(int16_t front, uint8_t dist) {
    int16_t diff = front - dist;
    if (diff <= 0) {
        return 0;
    }
    return diff >= REVEAL_RAMP ? 255 : diff * 255 / REVEAL_RAMP;
}

static bool is_finished(uint16_t elapsed) {
    switch (state.light.anim) {
        case ANIM_BOOT:
            return elapsed / BOOT_MS_PER_UNIT >= POS_MAX_D + BOOT_TAIL;
        case ANIM_RISE:
            return elapsed / REVEAL_MS_PER_UNIT >= POS_MAX_Y + REVEAL_RAMP;
        case ANIM_SPREAD:
            return elapsed / REVEAL_MS_PER_UNIT >= POS_MAX_D + REVEAL_RAMP;
        case ANIM_BREATH:
            return false;
        default:
            return true;
    }
}

// 各LEDの明るさ（0〜255）
static uint8_t brightness(uint8_t index, uint16_t elapsed, motion_t m) {
    led_pos_t pos = {pgm_read_byte(&LED_POS[index].d), pgm_read_byte(&LED_POS[index].y)};
    switch (state.light.anim) {
        case ANIM_BOOT: {
            // 先頭が一番明るく、後ろに向かって尾を引いて消える
            int16_t diff = (int16_t)(elapsed / BOOT_MS_PER_UNIT) - pos.d;
            if (diff < 0 || diff >= BOOT_TAIL) {
                return 0;
            }
            return 255 - diff * 255 / BOOT_TAIL;
        }
        case ANIM_BREATH: {
            if (index != ESC_LED_INDEX) {
                return 0;
            }
            uint8_t t = triangle((uint8_t)(elapsed >> BREATH_SHIFT));
            return scale(t, t); // 暗い所をゆっくりにして呼吸っぽくする
        }
        case ANIM_RISE:
            return reveal(elapsed / REVEAL_MS_PER_UNIT, POS_MAX_Y - pos.y);
        case ANIM_SPREAD:
            return reveal(elapsed / REVEAL_MS_PER_UNIT, pos.d);
        case ANIM_MOTION: {
            // 左右をまたいだ座標（右がプラス、下がプラス）で水玉模様を置き、
            // カーソルの移動量だけずらすことで、移動方向に光が流れて見える
            int16_t gx = CENTER_GAP + pos.d;
            if (is_keyboard_left()) {
                gx = -gx;
            }
            uint8_t px = triangle((uint8_t)(gx * 4 - (m.x >> MOTION_SHIFT)));
            uint8_t py = triangle((uint8_t)(pos.y * 4 - (m.y >> MOTION_SHIFT)));
            uint8_t b  = scale(px, py);
            return scale(b, b); // 水玉の輪郭をはっきりさせる
        }
        default:
            return 0;
    }
}

static void render(uint16_t elapsed) {
    motion_t m;
    ATOMIC_BLOCK_FORCEON {
        m.x = motion.x;
        m.y = motion.y;
    }
    uint8_t val = MIN(rgblight_get_val(), RGBLIGHT_LIMIT_VAL);
    if (state.light.dim) {
        val /= 2;
    }
    RGB     base  = hsv_to_rgb((HSV){state.light.hue, 255, val});
    uint8_t count = keyball.this_have_ball ? LED_COUNT_BALL_SIDE : LED_COUNT_NOBALL_SIDE;
    for (uint8_t i = 0; i < count; i++) {
        uint8_t b = brightness(i, elapsed, m);
        setrgb(scale(base.r, b), scale(base.g, b), scale(base.b, b), &led[i]);
    }
    rgblight_set();
    state.drawn_motion = m;
}

static void start_light(light_t light, uint16_t now) {
    state.light    = light;
    state.start    = now;
    state.finished = false;
}

static light_t layer_light(uint8_t layer) {
    light_t light = {ANIM_OFF};
    if (layer < sizeof(LAYER_LIGHTS) / sizeof(LAYER_LIGHTS[0])) {
        light = LAYER_LIGHTS[layer][is_keyboard_left() ? 0 : 1];
    }
    return light;
}

// 起動アニメーションが終わったら、以降はレイヤーが変わるたびにアニメーションを切り替える
static void update_light(uint16_t now) {
    uint8_t layer = get_highest_layer(layer_state);
    if (state.booting) {
        if (!state.finished) {
            return;
        }
        state.booting = false;
    } else if (layer == state.layer) {
        return;
    }
    state.layer = layer;
    start_light(layer_light(layer), now);
}

static void motion_rpc_handler(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data) {
    if (in_buflen != sizeof(motion_t)) {
        return;
    }
    const motion_t *m = (const motion_t *)in_data;
    motion.x          = m->x;
    motion.y          = m->y;
}

// レイヤー4の間だけ、マスターのボール移動量をスレーブに送る
static void sync_motion(uint16_t now) {
    static motion_t sent;
    static uint16_t last_sync;
    if (!is_keyboard_master() || state.light.anim != ANIM_MOTION) {
        return;
    }
    if ((sent.x == motion.x && sent.y == motion.y) || TIMER_DIFF_16(now, last_sync) < MOTION_SYNC_MS) {
        return;
    }
    last_sync  = now;
    motion_t m = {motion.x, motion.y};
    if (transaction_rpc_send(USER_SYNC_LED_MOTION, sizeof(m), &m)) {
        sent = m;
    }
}

void led_anim_add_motion(int16_t x, int16_t y) {
    // あふれても模様の周期（256）の倍数で折り返すので見た目は途切れない
    motion.x = (int16_t)((uint16_t)motion.x + (uint16_t)x);
    motion.y = (int16_t)((uint16_t)motion.y + (uint16_t)y);
}

// レイヤー4で、前回描いた後にボールが動いたか
static bool is_moved(void) {
    if (state.light.anim != ANIM_MOTION) {
        return false;
    }
    bool moved;
    ATOMIC_BLOCK_FORCEON {
        moved = state.drawn_motion.x != motion.x || state.drawn_motion.y != motion.y;
    }
    return moved;
}

void led_anim_init(void) {
    if (!is_keyboard_master()) {
        transaction_register_rpc(USER_SYNC_LED_MOTION, motion_rpc_handler);
    }
    // QMK標準のエフェクトは使わず、LEDの色は毎回こちらで書き込む
    rgblight_mode_noeeprom(RGBLIGHT_MODE_STATIC_LIGHT);
    state.booting = true;
    state.layer   = 0xFF;
    start_light(BOOT_LIGHT[is_keyboard_left() ? 0 : 1], timer_read());
}

void led_anim_task(void) {
    uint16_t now = timer_read();
    sync_motion(now);
    if (TIMER_DIFF_16(now, state.last_frame) < FRAME_MS) {
        return;
    }
    state.last_frame = now;

    // RGB_TOG でOFFの間は描かない。ONに戻った時や明るさが変わった時は描き直す
    bool enabled = rgblight_is_enabled();
    if (!enabled) {
        state.enabled = false;
        return;
    }
    uint8_t val = rgblight_get_val();
    bool    changed = !state.enabled || val != state.val;
    state.enabled   = true;
    state.val       = val;

    update_light(now);
    uint16_t elapsed = TIMER_DIFF_16(now, state.start);
    if (state.finished && !changed && !is_moved()) {
        return;
    }
    render(elapsed);
    state.finished = is_finished(elapsed);
}

// 左右非同期のため、自分側のLEDだけを描画・エフェクト対象にする
void keyball_on_adjust_layout(keyball_adjust_t v) {
    uint8_t lednum_this = keyball.this_have_ball ? LED_COUNT_BALL_SIDE : LED_COUNT_NOBALL_SIDE;
    rgblight_set_clipping_range(0, lednum_this);
    rgblight_set_effect_range(0, lednum_this);
}

#else

void led_anim_init(void) {}
void led_anim_task(void) {}
void led_anim_add_motion(int16_t x, int16_t y) {}

#endif
