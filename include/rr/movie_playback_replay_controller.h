#ifndef RR_MOVIE_PLAYBACK_REPLAY_CONTROLLER_H
#define RR_MOVIE_PLAYBACK_REPLAY_CONTROLLER_H
#include "rr/movie_playback_camera.h"
#include "rr/icon_sprites.h"
#include "rr/tables.h"
typedef struct {s32 key,action;} RrReplayEvent;
extern RrReplayEvent movie_playback_credits_events[3];
extern RrIconSpriteContext *CurrentDrawBuf;
extern RrMenuItemState5C PlayerControllers[2];
extern s16 ModeGroup,ModeId,ModeSubState;
void AddScreenFadeTiles(u32*,s32,u32);void CdAudioBeginPlay(s32);void CdAudioFade(u16,s32);void SetupPlayerView(s32);void DrawTrackSceneryForPlayer(s32);
void movie_playback_add_letterbox_packets(s32);void movie_playback_draw_credits_row(s32,s32);
/* The three-entry event table contains a negative sentinel within its owner.
 * The camera owner covers byte0..4B7, including the low scene flag atB0 and
 * event counter at4B4. Services can mutate all camera words, input, text words
 * and the projection root; each fresh root selects a complete context.
 * Counter additions, fade steps and text subtraction wrap32; division by6
 * truncates toward zero. Each active text phase submits exactly62 entries. */
void movie_playback_credits_update(void);
#endif
