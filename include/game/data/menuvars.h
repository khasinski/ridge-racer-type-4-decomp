#ifndef RR_GAME_DATA_MENUVARS_H
#define RR_GAME_DATA_MENUVARS_H

/* Menu/frame globals whose signedness differs between the rr/ headers; only units that were checked
 * against these spellings include this header (globals.h would hand them to every includer). */
extern unsigned short ModeSubState;
extern unsigned short ModeGroup;

#endif
