#ifndef RR_OVERLAY667_PACKET_H
#define RR_OVERLAY667_PACKET_H
#include "rr/menu_choice.h"
#include "rr/overlay667_events.h"
/* Twelve-byte serial record: four header bytes followed by two four-byte
 * choice records. XOR covers the entire eight-byte payload. */
typedef struct {
    u8 flags00,checksum01,status02,phase03;
    RrMenuChoice4 first04,second08;
} RrOverlay667SerialPacket;
/* Initialization clears aligned words; serial transfer and payload copy use
 * the byte-aligned record view. Both views belong to the same full owner. */
typedef union {
    RrOverlay667SerialPacket packet;
    u32 words[3];
} RrOverlay667SerialStorage;
typedef union {
    RrOverlay667SerialPacket packets[8];
    u32 words[24];
} RrOverlay667SerialRing;
typedef union {
    RrMenuChoice4 choices[2];
    u32 words[2];
} RrOverlay667ChoicePair;
extern RrOverlay667SerialStorage serial_link_tx_packet,serial_link_rx_packet;
extern RrOverlay667ChoicePair LinkLocalInputs;
extern u8 serial_link_tx_sequence,serial_link_checksum_error;
extern s32 RaceState,serial_link_local_menu_buttons;
/* BIOS B0 service35: write count bytes, returning the transferred count. */
s32 FileWrite(s32 descriptor,const void *buffer,s32 count);
s32 serial_link_send_packet(void);
/* Eight stored serial records, from 80118F20 through 80118F7F. Ring indices
 * used for payload access must remain in0..7, including after service calls. */
extern RrOverlay667SerialRing serial_link_rx_ring;
extern RrOverlay667ChoicePair LinkRemoteInputs,LinkSentLocalInputs;
extern s32 serial_link_stop_race_state,serial_link_shutdown_state;
extern s32 serial_link_sync_state,serial_link_rx_idle_flag;
extern s32 serial_link_rx_count,serial_link_rx_miss_count;
extern s32 serial_link_tx_count,serial_link_tx_fail_count,serial_link_tx_state;
extern u8 serial_link_rx_sequence,serial_link_rx_cursor,serial_link_remote_menu_buttons,serial_link_sent_menu_buttons;
extern u8 serial_link_last_rx_cursor;
extern u8 serial_link_rx_flag_kind;
void serial_link_init_connection(unsigned long configuration);
/* An event-ready packet's flags byte must be a valid ring index0..7: the
 * original uses the full byte as the index, without stripping control bits.
 * Serial services may mutate globals; every later ring access stays in0..7. */
s32 serial_link_receive_packet(void);
/* Receive selection and transmit retry state machines. Counter increments wrap
 * modulo2^32. Payloads and state are reread after the serial service returns.
 * The transmitted phase is the low byte of the little-endian EEC word. */
void serial_link_update_receive(void);
void serial_link_update_send(void);
RR_ASSERT_SIZE(RrOverlay667SerialPacket,12);
RR_ASSERT_OFFSET(RrOverlay667SerialPacket,first04,4);
RR_ASSERT_OFFSET(RrOverlay667SerialPacket,second08,8);
RR_ASSERT_SIZE(RrOverlay667SerialStorage,12);
RR_ASSERT_SIZE(RrOverlay667SerialRing,96);
RR_ASSERT_SIZE(RrOverlay667ChoicePair,8);
#endif
