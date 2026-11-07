#include "rr/line_emit.h"
#include "types.h"

/* Car / course setup controller (one player). */

typedef struct {
    s16 x00, y02;
    s32 words04[4];
} OptionsView14;

typedef struct {
    s32 scene00, reserved04, selection08;
    s32 step0C, detail10, next14, kind18;
    s32 angle1C, phase20, fade24;
    u16 buttons28;
    s16 course2A;
    s32 ready2C;
    u8 pad30[8];
    OptionsView14 view38;
    u8 pad4C[0x3A];
    u16 rotation86;
    u8 pad88[4];
    s16 offset8C, offset8E, offset90;
    u8 pad92[2];
    s16 offset94, offset96, offset98;
    u8 pad9A[6];
} OptionsCarState;

typedef struct {
    u8 byte00, course01;
    u8 pad02[6];
    u16 buttons08;
    u8 pad0A[0x12];
    s16 option1C;
    u8 pad1E[2];
    s16 balance20;
    u8 pad22[6];
    s32 confirm28;
    u8 pad2C[0x14];
    u8 ready40;
    s8 tilt41;
    u8 pad42[0x1A];
} OptionsPad5C;

typedef struct {
    struct { s16 car00, colour02; } a[2];
    struct { s16 h[8]; } b[2];
    struct { s8 c[4]; } c[2];
} OptionsSettings;

typedef struct {
    u8 pad00[4];
    u8 b4, b5, b6, b7;
    u8 pad08[0x20];
} OptionsEntry28;

typedef struct {
    u8 byte00, byte01;
    s16 h02;
    s16 x04, y06;
    u8 pad08[8];
    s16 state10;
} OptionsPanel;

typedef struct {
    u8 pad00[0xC];
    u8 cells[8];
    u8 byte14, byte15, byte16;
    u8 pad17[0xD];
} OptionsRow24;

typedef struct {
    u8 pad00[4];
    u8 b4;
} OptionsText;

typedef struct {
    u8 pad00[0x70];
    u32 ordering70;
} OptionsFrame;

typedef struct {
    u32 tag;
    u32 command;
} OptionsPacket;

typedef struct {
    u8 a, pad1, b, pad3;
} OptionsColour4;

typedef struct {
    s16 x0, y0, x1, y1;
} OptionsLine;

extern OptionsCarState options_menu_state;
extern s32 options_pad_mode_ready;
extern OptionsPad5C PlayerControllers[2];
extern OptionsSettings Options;
extern OptionsEntry28 PadPorts[];
extern OptionsPanel *BadgeRows;
extern OptionsRow24 *UiRows;
extern OptionsText *DialogueState;
extern OptionsFrame *CurrentDrawBuf;
extern s16 MenuFadeTimer;
extern s32 SelectionPlayer;
extern s32 MenuActivePlayerPlusOne;
extern u8 ScreenFadeState;
extern OptionsView14 options_controller_view_presets[];
extern OptionsColour4 options_pad_layout_colors[][5];
extern OptionsColour4 options_negcon_layout_colors[][5];
extern OptionsColour4 options_jogcon_layout_colors[][5];
extern void *options_pad_button_guides[][5];
extern void *options_negcon_button_guides[][5];
extern void *options_dualshock_button_guides[][5];
extern void *options_jogcon_button_guides[][5];
extern s32 options_twist_play_swing_table[];
extern s32 options_twist_play_marker_offsets[];
extern u8 options_marker_line_color[];
extern u8 options_left_arrow_uv[], options_right_arrow_uv[];
extern s16 SteeringRangeTable[];
extern s16 PadRangeLimits[];
extern s32 WheelDeadzoneTable[];

extern s32 PadGetState(s32);
extern void PadSetMainMode(s32, s32, s32);
extern s32 LoadModelBank8Step(void);
extern s32 options_get_controller_kind(s32);
extern void SetMotionControllerVisible(s32);
extern void StepMenuValue(s32, s32, void *, s32, s32);
extern void SetPadButtonMapLayout(s32, s32, s32, s32);
/* Sound cue; several call sites pass the freshly computed offset as a
 * second (ignored) argument, so it is declared without a prototype. */
extern void PlaySfx();
extern void options_draw_button_guides(void *, s32);
extern void *AddSpritePrim(void *, void *, s32, s32, void *, s32, s32);
extern void options_draw_controller_model(void);
extern void DrawUiRow(s32);
extern void AddScreenFadeTiles(void *, s32, s32);
extern void options_init_main_menu(void);
extern void ClearScreenOverlayState();
extern void InitBadgeRow(s32, s32, s32, s32, s32, s32, s32);
extern void SetMenuFrameOptions(s32, s32, s32);
extern s32 rsin(s32);
extern void PadSetRumbleFlags(s32, s32);
extern void options_draw_angle_marker(s32);
extern void UpdateBadgeRowAnim(s32);
extern void UpdateDialogue(s32, s32, void *);
extern void UpdateScreenFade(s32);

#define ST options_menu_state

typedef struct {
    void *cursor;
} OptionsScratch;

#define SCRATCH ((OptionsScratch *)0x1F800000)
#define CURSOR (*(void **)0x1F800000)

#define CLOSE_ROWS(n)                                   \
    BadgeRows->state10 = 4;                            \
    for (i = 0; i < (n); i++) {                         \
        if (UiRows[i].byte14 == 2) {                \
            UiRows[i].byte14 = 3;                   \
        }                                               \
    }

#define COPY_COLOURS(src)                               \
    for (i = 0; i < 5; i++) {                           \
        UiRows[i].byte15 = (src)->a;                \
        UiRows[i].byte16 = (src)->b;                \
        (src)++;                                        \
    }

/* Links a texture-page command packet into the frame ordering list. */
#define LINK_PACKET()                                                       \
    {                                                                       \
        OptionsFrame *frame;                                                 \
        packet = SCRATCH->cursor;                                           \
        ((u8 *)&packet->tag)[3] = 1;                                        \
        frame = CurrentDrawBuf;                                                 \
        packet->command = 0xE1000259;                                       \
        packet->tag = (packet->tag & 0xFF000000) | (frame->ordering70 & 0xFFFFFF); \
        frame->ordering70 = (frame->ordering70 & 0xFF000000) | ((u32)packet & 0xFFFFFF); \
        packet++;                                                           \
        SCRATCH->cursor = packet;                                           \
    }

void options_update_controller_config(s32 player) {
    OptionsColour4 *src;
    OptionsEntry28 *entry;
    s32 changed;
    s32 i;
    s32 kind;
    s32 index;
    s32 old;
    s32 value;
    s32 phase;
    s32 wave;
    s32 arg;
    s16 timer;
    void *cursor;
    OptionsLine line;
    OptionsPacket *packet;

    ST.buttons28 = PlayerControllers[player].buttons08;
    entry = &PadPorts[player];
    changed = 0;
    switch (ST.step0C) {
    case 0:
        if (options_pad_mode_ready == 0) {
            if (PlayerControllers[player].ready40 == 1) {
                if (ST.ready2C == 0) {
                    if (PlayerControllers[player].course01 != 0xE3) {
                        if (PadGetState(player << 4) == 6) {
                            PadSetMainMode(player << 4, 1, 0);
                        }
                    } else {
                        ST.ready2C = 1;
                        options_pad_mode_ready = 1;
                    }
                }
            } else {
                options_pad_mode_ready = 1;
            }
        }
        SelectionPlayer = player;
        if (LoadModelBank8Step() == 0) {
            break;
        }
        if (options_pad_mode_ready == 0) {
            break;
        }
        kind = options_get_controller_kind(player);
        ST.kind18 = kind;
        if (kind >= 0) {
            switch (kind) {
            case 0:
            case 1:
                index = Options.a[player].car00;
                src = options_pad_layout_colors[index];
                index <<= 12;
                ST.rotation86 = index;
                break;
            case 2:
                index = Options.b[player].h[6];
                src = options_negcon_layout_colors[index];
                index <<= 12;
                ST.rotation86 = index;
                break;
            case 3:
                if (PlayerControllers[player].course01 != 0xE3) {
                    index = Options.a[player].car00;
                    src = options_pad_layout_colors[index];
                    index <<= 12;
                ST.rotation86 = index;
                } else {
                    index = Options.c[player].c[3];
                    src = options_jogcon_layout_colors[index];
                    index <<= 12;
                ST.rotation86 = index;
                }
                ST.course2A = PlayerControllers[player].course01;
                break;
            }
            ST.view38 = options_controller_view_presets[ST.kind18];
            COPY_COLOURS(src);
        } else {
            CLOSE_ROWS(8);
            MenuFadeTimer = 64;
            ST.step0C = 2;
        }
        ST.detail10 = 0;
        ST.angle1C = 0;
        ST.fade24 = 0;
        ST.offset8C = 0;
        ST.offset8E = 0;
        ST.offset90 = 0;
        ST.offset94 = 0;
        ST.offset96 = 0;
        ST.offset98 = 0;
        SetMotionControllerVisible(0);
        ST.step0C++;
        break;
    case 1:
        if (BadgeRows->state10 == 3 && UiRows[0].byte14 == 2) {
            if (options_get_controller_kind(player) < 0) {
                CLOSE_ROWS(8);
                MenuFadeTimer = 64;
                ST.step0C = 2;
                ST.next14 = 0;
            }
            switch (ST.kind18) {
            case 1:
                if (PlayerControllers[player].option1C != 0) {
                    UiRows[5].cells[0] = 1;
                    UiRows[Options.a[player].colour02 + 6].cells[0] = 0;
                    StepMenuValue(player * 4, 2, &Options.a[player].colour02, 2, 0);
                    UiRows[Options.a[player].colour02 + 6].cells[0] = 2;
                }
            case 0:
                old = Options.a[player].car00;
                StepMenuValue(player * 4 | 1, 2, &Options.a[player].car00, 8, 0);
                if (old != Options.a[player].car00) {
                    if (Options.a[player].car00 < old) {
                        ST.angle1C -= 0x1000;
                    } else {
                        ST.angle1C += 0x1000;
                    }
                    src = options_pad_layout_colors[Options.a[player].car00];
                    COPY_COLOURS(src);
                    ST.fade24 = 0;
                    SetPadButtonMapLayout(player, Options.a[player].car00, Options.b[player].h[6],
                                  Options.c[player].c[3]);
                }
                if (PlayerControllers[player].confirm28 != 0) {
                    PlaySfx(1);
                    CLOSE_ROWS(5);
                    MenuFadeTimer = 64;
                    ST.step0C = 2;
                    ST.next14 = 0;
                }
                options_draw_button_guides(options_pad_button_guides[Options.a[player].car00], ST.fade24);
                cursor = CURSOR;
                if (Options.a[player].car00 > 0) {
                    cursor = AddSpritePrim(&CurrentDrawBuf->ordering70, cursor, 56, 280, options_left_arrow_uv, 0x7F38, 255);
                }
                if (Options.a[player].car00 < 7) {
                    cursor = AddSpritePrim(&CurrentDrawBuf->ordering70, cursor, 254, 280, options_right_arrow_uv, 0x7F38, 255);
                }
                CURSOR = cursor;
                break;
            case 2:
                old = Options.b[player].h[6];
                StepMenuValue(player * 4 | 1, 2, &Options.b[player].h[6], 8, 0);
                if (old != Options.b[player].h[6]) {
                    if (Options.b[player].h[6] < old) {
                        ST.angle1C -= 0x1000;
                    } else {
                        ST.angle1C += 0x1000;
                    }
                    src = options_negcon_layout_colors[Options.b[player].h[6]];
                    COPY_COLOURS(src);
                    ST.fade24 = 0;
                    SetPadButtonMapLayout(player, Options.a[player].car00, Options.b[player].h[6],
                                  Options.c[player].c[3]);
                }
                value = PlayerControllers[player].balance20 * 3800 / 256 - 1900;
                ST.offset8C = value / 2;
                ST.offset94 = -value / 2;
                if (PlayerControllers[player].confirm28 != 0) {
                    PlaySfx(1, value);
                    CLOSE_ROWS(5);
                    MenuFadeTimer = 64;
                    ST.step0C = 2;
                    ST.next14 = 3;
                }
                options_draw_button_guides(options_negcon_button_guides[Options.b[player].h[6]], ST.fade24);
                cursor = CURSOR;
                if (Options.b[player].h[6] > 0) {
                    cursor = AddSpritePrim(&CurrentDrawBuf->ordering70, cursor, 56, 280, options_left_arrow_uv, 0x7F38, 255);
                }
                if (Options.b[player].h[6] < 7) {
                    cursor = AddSpritePrim(&CurrentDrawBuf->ordering70, cursor, 254, 280, options_right_arrow_uv, 0x7F38, 255);
                }
                CURSOR = cursor;
                break;
            case 3:
                if (ST.course2A != PlayerControllers[player].course01) {
                    changed = 1;
                    ST.course2A = PlayerControllers[player].course01;
                }
                if (PlayerControllers[player].course01 != 0xE3) {
                    old = Options.a[player].car00;
                    StepMenuValue(player * 4 | 1, 1, &Options.a[player].car00, 8, 0);
                    if (old != Options.a[player].car00) {
                        if (Options.a[player].car00 < old) {
                            ST.angle1C -= 0x1000;
                        } else {
                            ST.angle1C += 0x1000;
                        }
                        changed = 1;
                        SetPadButtonMapLayout(player, Options.a[player].car00, Options.b[player].h[6],
                                      Options.c[player].c[3]);
                    }
                    if (changed) {
                        src = options_pad_layout_colors[Options.a[player].car00];
                        COPY_COLOURS(src);
                        ST.fade24 = 0;
                    }
                    if (PlayerControllers[player].confirm28 != 0) {
                        PlaySfx(1);
                        CLOSE_ROWS(5);
                        MenuFadeTimer = 64;
                        ST.step0C = 2;
                        ST.next14 = 0;
                    }
                    options_draw_button_guides(options_dualshock_button_guides[Options.a[player].car00], ST.fade24);
                    cursor = CURSOR;
                    if (Options.a[player].car00 > 0) {
                        cursor = AddSpritePrim(&CurrentDrawBuf->ordering70, cursor, 56, 280, options_left_arrow_uv, 0x7F38, 255);
                    }
                    if (Options.a[player].car00 < 7) {
                        cursor = AddSpritePrim(&CurrentDrawBuf->ordering70, cursor, 254, 280, options_right_arrow_uv, 0x7F38, 255);
                    }
                    CURSOR = cursor;
                } else {
                    old = Options.c[player].c[3];
                    StepMenuValue(player * 4 | 1, 1, &Options.c[player].c[3], 8, 0);
                    if (old != Options.c[player].c[3]) {
                        if (Options.c[player].c[3] < old) {
                            ST.angle1C -= 0x1000;
                        } else {
                            ST.angle1C += 0x1000;
                        }
                        changed = 1;
                        SetPadButtonMapLayout(player, Options.a[player].car00, Options.b[player].h[6],
                                      Options.c[player].c[3]);
                    }
                    if (changed) {
                        src = options_jogcon_layout_colors[Options.c[player].c[3]];
                        COPY_COLOURS(src);
                        ST.fade24 = 0;
                    }
                    value = PlayerControllers[player].tilt41 * 16;
                    ST.offset90 = value;
                    if (PlayerControllers[player].confirm28 != 0) {
                        PlaySfx(1, value);
                        CLOSE_ROWS(5);
                        MenuFadeTimer = 64;
                        ST.step0C = 2;
                        ST.next14 = 4;
                    }
                    options_draw_button_guides(options_jogcon_button_guides[Options.c[player].c[3]], ST.fade24);
                    cursor = CURSOR;
                    if (Options.c[player].c[3] > 0) {
                        cursor = AddSpritePrim(&CurrentDrawBuf->ordering70, cursor, 56, 280, options_left_arrow_uv, 0x7F38, 255);
                    }
                    if (Options.c[player].c[3] < 7) {
                        cursor = AddSpritePrim(&CurrentDrawBuf->ordering70, cursor, 254, 280, options_right_arrow_uv, 0x7F38, 255);
                    }
                    CURSOR = cursor;
                }
                break;
            }
            ST.fade24++;
            if (ST.fade24 > 16) {
                ST.fade24 = 16;
            }
        }
        ST.angle1C = ST.angle1C * 15 / 16;
        ST.rotation86 = ST.angle1C;
        if ((u32)(ST.angle1C + 128) > 256) {
            ST.fade24 = 0;
        }
        LINK_PACKET();
        options_draw_controller_model();
        for (i = 0; i < 5; i++) {
            DrawUiRow(i);
        }
        if (ST.kind18 == 1 && PlayerControllers[player].option1C != 0) {
            for (i = 5; i < 8; i++) {
                DrawUiRow(i);
            }
        }
        break;
    case 2:
        timer = MenuFadeTimer - 1;
        MenuFadeTimer = timer;
        AddScreenFadeTiles(&CurrentDrawBuf->ordering70, (64 - timer) * 4, 69);
        if (MenuFadeTimer <= 0) {
            if (ST.next14 == 0) {
                options_init_main_menu();
                ST.step0C = 0;
                SelectionPlayer = 0;
                ST.scene00--;
                return;
            }
            ST.step0C = ST.next14;
            ST.detail10 = 0;
            ST.next14 = 0;
        }
        options_draw_controller_model();
        for (i = 0; i < 5; i++) {
            DrawUiRow(i);
        }
        break;
    case 3:
        switch (ST.detail10) {
        case 0:
            ClearScreenOverlayState();
            ScreenFadeState = 2;
            InitBadgeRow(0, 12, 304, 296, 88, 0, 0);
            BadgeRows->h02 = 1;
            SetMenuFrameOptions(1, 0, 0);
            ST.view38 = options_controller_view_presets[4];
            ST.rotation86 = 0;
            ST.angle1C = 0;
            DialogueState->b4 = 9;
            ST.detail10++;
            break;
        case 1:
            value = PlayerControllers[player].balance20 * 3800 / 256 - 1900;
            ST.offset8C = value / 2;
            ST.offset94 = -value / 2;
            if (PlayerControllers[player].buttons08 & 0x800) {
                PlaySfx(1, value);
                Options.b[player].h[0] = entry->b4 - 128;
                Options.b[player].h[1] = entry->b5;
                Options.b[player].h[2] = entry->b6;
                Options.b[player].h[3] = entry->b7;
                DialogueState->b4 = Options.b[player].h[4] + 10;
                ST.view38 = options_controller_view_presets[5];
                ST.phase20 = 0;
                ST.detail10++;
                SetMenuFrameOptions(1, 1, 0);
            }
            break;
        case 2:
            old = Options.b[player].h[4];
            StepMenuValue(player * 4 | 1, 1, &Options.b[player].h[4], 4, 0);
            if (old != Options.b[player].h[4]) {
                DialogueState->b4 = Options.b[player].h[4] + 10;
            }
            value = rsin(ST.phase20) * options_twist_play_swing_table[Options.b[player].h[4]] / 512;
            ST.offset8C = value;
            ST.offset94 = -value;
            phase = ST.phase20;
            ST.phase20 = phase + 32;
            if (ST.phase20 >= 4096) {
                ST.phase20 = phase - 4064;
            }
            if (PlayerControllers[player].buttons08 & 0x840) {
                PlaySfx(1, value);
                DialogueState->b4 = Options.b[player].h[5] + 14;
                ST.view38 = options_controller_view_presets[5];
                ST.phase20 = 0;
                ST.detail10++;
            }
            line.x0 = 148;
            line.x1 = 172;
            line.y0 = line.y1 = ST.view38.y02;
            AddLineF2Prim((const s16 *)&line, options_marker_line_color, 0, 0);
            if (Options.b[player].h[4] > 0) {
                line.y0 = line.y1 = ST.view38.y02 + options_twist_play_marker_offsets[Options.b[player].h[4] - 1];
                AddLineF2Prim((const s16 *)&line, options_marker_line_color, 0, 0);
                line.y0 = line.y1 = ST.view38.y02 - options_twist_play_marker_offsets[Options.b[player].h[4] - 1];
                AddLineF2Prim((const s16 *)&line, options_marker_line_color, 0, 0);
            }
            cursor = CURSOR;
            if (Options.b[player].h[4] > 0) {
                cursor = AddSpritePrim(&CurrentDrawBuf->ordering70, cursor, 56, 182, options_left_arrow_uv, 0x7F38, 255);
            }
            if (Options.b[player].h[4] < 3) {
                cursor = AddSpritePrim(&CurrentDrawBuf->ordering70, cursor, 254, 182, options_right_arrow_uv, 0x7F38, 255);
            }
            CURSOR = cursor;
            break;
        case 3:
            old = Options.b[player].h[5];
            StepMenuValue(player * 4 | 1, 1, &Options.b[player].h[5], 4, 0);
            if (old != Options.b[player].h[5]) {
                DialogueState->b4 = Options.b[player].h[5] + 14;
            }
            value = rsin(ST.phase20) * SteeringRangeTable[Options.b[player].h[5]] / 512;
            ST.offset8C = value;
            ST.offset94 = -value;
            phase = ST.phase20;
            ST.phase20 = phase + 32;
            if (ST.phase20 >= 4096) {
                ST.phase20 = phase - 4064;
            }
            if (PlayerControllers[player].buttons08 & 0x840) {
                PlaySfx(1, value);
                CLOSE_ROWS(5);
                MenuFadeTimer = 64;
                ST.step0C = 2;
            }
            cursor = CURSOR;
            if (Options.b[player].h[5] > 0) {
                cursor = AddSpritePrim(&CurrentDrawBuf->ordering70, cursor, 56, 182, options_left_arrow_uv, 0x7F38, 255);
            }
            if (Options.b[player].h[5] < 3) {
                cursor = AddSpritePrim(&CurrentDrawBuf->ordering70, cursor, 254, 182, options_right_arrow_uv, 0x7F38, 255);
            }
            CURSOR = cursor;
            break;
        }
        if (options_get_controller_kind(player) < 0) {
            CLOSE_ROWS(8);
            MenuFadeTimer = 64;
            ST.step0C = 2;
            ST.next14 = 0;
        }
        LINK_PACKET();
        options_draw_controller_model();
        break;
    case 4:
        switch (ST.detail10) {
        case 0:
            ClearScreenOverlayState();
            ScreenFadeState = 2;
            InitBadgeRow(0, 12, 304, 296, 88, 0, 0);
            BadgeRows->h02 = 1;
            SetMenuFrameOptions(1, 0, 0);
            ST.view38 = options_controller_view_presets[6];
            ST.rotation86 = 0;
            ST.angle1C = 0;
            DialogueState->b4 = 8;
            ST.detail10++;
            break;
        case 1:
            value = PlayerControllers[player].tilt41 * 16;
            ST.offset90 = value;
            if (PlayerControllers[player].buttons08 & 0x800) {
                PlaySfx(1, value);
                PadSetRumbleFlags(player, 3);
                DialogueState->b4 = Options.c[player].c[0] + 10;
                ST.phase20 = 0;
                ST.detail10++;
                SetMenuFrameOptions(1, 1, 0);
            }
            break;
        case 2:
            old = Options.c[player].c[0];
            StepMenuValue(player * 4 | 1, 1, &Options.c[player].c[0], 4, 0);
            if (old != Options.b[player].h[4]) {
                DialogueState->b4 = Options.c[player].c[0] + 10;
            }
            value = rsin(ST.phase20) * WheelDeadzoneTable[Options.c[player].c[0]] / 128;
            ST.offset90 = value;
            phase = ST.phase20;
            ST.phase20 = phase + 32;
            if (ST.phase20 >= 4096) {
                ST.phase20 = phase - 4064;
            }
            if (PlayerControllers[player].confirm28 != 0) {
                PlaySfx(1, value);
                DialogueState->b4 = Options.c[player].c[1] + 18;
                ST.phase20 = 0;
                ST.detail10++;
            }
            if (Options.c[player].c[0] > 0) {
                options_draw_angle_marker(WheelDeadzoneTable[Options.c[player].c[0]] << 5);
                options_draw_angle_marker(-WheelDeadzoneTable[Options.c[player].c[0]] << 5);
            }
            cursor = CURSOR;
            if (Options.c[player].c[0] > 0) {
                cursor = AddSpritePrim(&CurrentDrawBuf->ordering70, cursor, 56, 182, options_left_arrow_uv, 0x7F38, 255);
            }
            if (Options.c[player].c[0] < 3) {
                cursor = AddSpritePrim(&CurrentDrawBuf->ordering70, cursor, 254, 182, options_right_arrow_uv, 0x7F38, 255);
            }
            CURSOR = cursor;
            break;
        case 3:
            old = Options.c[player].c[1];
            StepMenuValue(player * 4 | 1, 1, &Options.c[player].c[1], 5, 0);
            if (old != Options.c[player].c[1]) {
                DialogueState->b4 = Options.c[player].c[1] + 18;
            }
            value = rsin(ST.phase20) * PadRangeLimits[Options.c[player].c[1]] / 256;
            ST.offset90 = value;
            phase = ST.phase20;
            ST.phase20 = phase + 32;
            if (ST.phase20 >= 4096) {
                ST.phase20 = phase - 4064;
            }
            if (PlayerControllers[player].confirm28 != 0) {
                PlaySfx(1, value);
                DialogueState->b4 = Options.c[player].c[2] + 23;
                ST.phase20 = 0;
                ST.detail10++;
            }
            cursor = CURSOR;
            if (Options.c[player].c[1] > 0) {
                cursor = AddSpritePrim(&CurrentDrawBuf->ordering70, cursor, 56, 182, options_left_arrow_uv, 0x7F38, 255);
            }
            if (Options.c[player].c[1] < 4) {
                cursor = AddSpritePrim(&CurrentDrawBuf->ordering70, cursor, 254, 182, options_right_arrow_uv, 0x7F38, 255);
            }
            CURSOR = cursor;
            break;
        case 4:
            old = Options.c[player].c[2];
            StepMenuValue(player * 4 | 1, 1, &Options.c[player].c[2], 3, 0);
            if (old != Options.c[player].c[2]) {
                DialogueState->b4 = Options.c[player].c[2] + 23;
            }
            wave = rsin(ST.phase20) * 21;
            value = wave * Options.c[player].c[2] / 256;
            ST.offset90 = value;
            phase = ST.phase20;
            ST.phase20 = phase + 32;
            if (ST.phase20 >= 4096) {
                ST.phase20 = phase - 4064;
            }
            if (PlayerControllers[player].confirm28 != 0) {
                PlaySfx(1, value);
                CLOSE_ROWS(5);
                MenuFadeTimer = 64;
                ST.step0C = 2;
            }
            cursor = CURSOR;
            if (Options.c[player].c[2] > 0) {
                cursor = AddSpritePrim(&CurrentDrawBuf->ordering70, cursor, 56, 182, options_left_arrow_uv, 0x7F38, 255);
            }
            if (Options.c[player].c[2] < 2) {
                cursor = AddSpritePrim(&CurrentDrawBuf->ordering70, cursor, 254, 182, options_right_arrow_uv, 0x7F38, 255);
            }
            CURSOR = cursor;
            break;
        }
        if (options_get_controller_kind(player) < 0 || PlayerControllers[player].course01 != 0xE3) {
            CLOSE_ROWS(8);
            MenuFadeTimer = 64;
            ST.step0C = 2;
            ST.next14 = 0;
        }
        LINK_PACKET();
        options_draw_controller_model();
        break;
    }
    if (ST.step0C < 3) {
        UpdateBadgeRowAnim(0);
        MenuActivePlayerPlusOne = player + 1;
        arg = 0x10072;
    } else {
        UpdateBadgeRowAnim(0);
        if (BadgeRows->state10 == 3) {
            UpdateDialogue(BadgeRows->x04 + 10, BadgeRows->y06 + 40, DialogueState);
        }
        arg = 0x83;
    }
    UpdateScreenFade(arg);
}
