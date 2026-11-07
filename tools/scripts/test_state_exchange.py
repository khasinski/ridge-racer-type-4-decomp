"""Check PAL state accessors against typed RAM pages on the host.

The original PAL addresses are replaced by offsets in host arrays. Empty
register annotations are omitted; fd.py and the full executable check verify
the actual target addresses, registers and instruction sequences.
"""
import ctypes as C
from pathlib import Path
import re
import struct
import subprocess
from tempfile import TemporaryDirectory
import unittest


EXCHANGES = [
    ('SetVideoMode', 0x800A63D0),
    ('CdSetDebug', 0x800A6490),
    ('CdSyncCallback', 0x800A6488),
    ('CdReadyCallback', 0x800A648C),
    ('CdReadCallback', 0x800A63F0),
    ('CdReadMode', 0x800A63F4),
]
SETTERS = [('_SsVmDamperOn', 0x801100A0, 2), ('SsSetMono', 0x801100E8, 1)]
AfterPair = C.CFUNCTYPE(None)


class StateExchangeTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = TemporaryDirectory()
        cls.addClassCleanup(cls.temp.cleanup)
        directory = Path(cls.temp.name)
        root = Path(__file__).resolve().parents[2]
        source = '''#include <stdint.h>
typedef int16_t s16;
typedef uint16_t u16;
typedef int32_t s32;
volatile u16 test_counter;
s32 test_page_800A[0x4000];
s16 test_page_8011[0x8000];
s32 test_page_8011_words[0x4000];
typedef struct RrLibQueueRecord32 RrLibQueueRecord32;
RrLibQueueRecord32 *test_queue;
#define StRingHeaders test_queue
'''
        names = ([name for name, _ in EXCHANGES] + ['_padChkVsync']
                 + [name for name, _, _ in SETTERS]
                 + ['_spu_setInTransfer', 'SpuSetTransferMode', 'StSetRing', 'setRC2wait',
                    '_cardCmdReset'])
        for name in names:
            body = (root / 'tools/scripts/fixtures/legacy' / (name + '.c')).read_text() if (root / 'tools/scripts/fixtures/legacy' / (name + '.c')).exists() else (root / 'tools/scripts/fixtures/legacy' / ('func_%08X.c' % __import__('c_functions').address_of(name))).read_text()
            body = body.replace('#include "types.h"', '')
            body = body.replace('#include "rr/library.h"', '')

            def global_alias(match):
                symbol = match[1]
                address = __import__('c_functions').address_of(symbol)
                region = 0x80110000 if address >= 0x80110000 else 0x800A0000
                page = 'test_page_8011_words' if region == 0x80110000 else 'test_page_800A'
                return f'#define {symbol} {page}[(0x{address:X} - 0x{region:X}) / 4]'

            body = re.sub(r'extern s32 (D_[0-9A-F]+|D_800ABAC0|CdCacheFileDirNotFoundMsg|TeamAbbrevMicroMouseMappy|CdNewMediaFormatErrorMsg|CdSearchFileDiscErrorMsg|SaveSlotSelectionsBackup|StrCdNewMediaDirCountFmt|CdReadSavedSyncCallback|FlashingLightColorTable|MenuActivePlayerPlusOne|PadButtonLayoutPresetsA|PadButtonLayoutPresetsB|StrControllerNoErrorFmt|TeamNameMicroMouseMappy|CdNewMediaReadErrorStr|CourseSceneryRangePtrs|InterruptCallbackTable|PadSaveCommandCallback|StrCdCacheFileCountFmt|StrCdNewmediaReadError|StrFmtTwoDigitsOpening|StrNowAcceptingEntries|UnexpectedInterruptFmt|CourseBgmChoiceTable3|CourseSkyKeyframePtrs|GpuQueueSavedIntrMask|LoadingTrianglePacket|PadChannelDispatchPtr|PadChannelRecordArray|PadPacketDoneCallback|PlayerVoiceParamTable|RivalTeamNamePtrTable|SecondsUntilCloseText|StrActuatorNoErrorFmt|StrSpuDmaClearWaitMsg|_spu_transferCallback|_spu_voice_centerNote|CD_comHasCompleteInt|CardFileNameTemplate|CardFirstFileHandler|CardFormatDirRecords|CdCacheFileSearchMsg|CdReadSectorErrorMsg|CourseBestLapRecords|CourseBestTotalTimes|DefaultRecordEntries|DispDotClockDivisors|LoadingTriangleAngle|PadPortTimeoutCounts|SpuReverbPresetTable|SsSemitonePitchTable|StrCardDirPatternFmt|StrFmtTwoDigitsTitle|TrackEventSfxEnabled|TrackObjectKeyframes|ViewUsesCarTrackNode|_spu_dma_block_count|CD_timeout_deadline|CardTargetFileNames|CdCommandParamTable|CdFileStateHandlers|CdPathLevelErrorFmt|CdTrackTitleStrings|CourseFarColorTable|CourseMenuSelection|DrawSyncCallbackFmt|HalfTransViewMatrix|LoadingTriangleFlip|ModeMenuCursorSaved|MotionSequenceIndex|MotionSequenceTable|PadAdvanceChannelFn|PadPortToChannelPtr|PlayerCourseChoices|ReplayFocusCarIndex|StrCdDirNotFoundFmt|VabOpenHeadErrorStr|_spu_mem_mode_unitM|_spu_rev_offsetaddr|_spu_rev_reserve_wa|BgmNumberToCdTrack|CD_str_init_prefix|CardBytesRemaining|CardCommandRequest|CardCurrentCommand|CardPortLastResult|CardTargetFileName|CdFileIndexCurrent|CdFileSectorOffset|CdTimeoutPrefixStr|CdTimeoutStatusFmt|CourseBestLapTimes|DefaultCourseTimes|DiskErrorPrefixStr|DmaChannelRegsBase|GpuTimeoutDeadline|IntrDriverTablePtr|LapAnnouncePending|LapTimeTemplateStr|MenuConfirmPressed|PadChannelToPortFn|PadMultitapEnabled|PadPacketBytesLeft|RelFileOffsetPtr12|RelFileOffsetPtr13|RelFileViewRecords|SaveSlotSelections|SelectionCountdown|SioRxFifoSizeTable|SoundSequenceTable|SsCtrlHandlerCC121|StNextSectorOffset|SteeringRangeTable|StrCdReadShellOpen|StrCdSearchFileFmt|StrFileNotFoundFmt|TeamStringPtrTable|TileFadeColorTable|VSyncCallbackTable|WheelDeadzoneTable|_spu_AllocBlockNum|_spu_mem_mode_plus|_spu_mem_mode_unit|_spu_rev_startaddr|_svm_auto_kof_mode|BadgePatternTable|CardCmdResultPtrA|CardCmdResultPtrB|CardCommandReader|CardCommandStatus|CardIrqQueueEntry|CdAudioTrackTable|CdCommandErrorStr|CdDatasyncNameStr|CdDirectoryBuffer|CdIntrStatusBytes|CourseChoiceTable|CourseLightColors|DefaultPlayerName|DmaStatusErrorFmt|HudMapSpriteRects|LoadImage2NameStr|MenuStepDirection|MenuTextTemplates|OpeningSlideTimer|PadChannelRecords|PadRecvStageTable|PadResetChannelFn|PadTxByteCallback|PasswordCipherKey|PlayerControllers|RaceLoadStepIndex|RaceTimeRemaining|RelFileOffsetPtr3|RelFileOffsetPtr6|RelFileOffsetPtr7|RelFileOffsetPtr9|SioReceiveRequest|SsCtrlHandlerCC64|SsCtrlHandlerCC98|StRingHeaderIndex|StrDrawOTagEnvFmt|StrHexDigitsLower|StrSetDispMaskFmt|StrStarPleaseWait|StrTotalTimeLabel|SvmPitchFineTable|TrackObjectStates|UnknownRroTypeFmt|_snd_seq_tick_env|_spu_AllocLastNum|CDROM_DELAY_SIZE|CD_fmt_intr_code|CD_read_dma_mode|CD_timeout_count|CameraViewMatrix|CardHeaderTitles|CardReadErrorMsg|CardReceiveIndex|CardSaveFileName|CardTitleStrings|CardWriteHandler|CdAudioFadeState|CdFileSyncResult|CdUnknownIntrMsg|CdWaitCallerName|CourseColorTable|CourseMenuCursor|CurrentVideoMode|EntryAcceptedStr|GameTypeSceneIds|GpuSavedIntrMask|HudNumberTargets|MenuCardStepRows|MenuSceneEntries|MenuTextRowCount|ModelTexturePage|MotionController|MotionTrackTable|MoveImageNameStr|NodeLerpFraction|OpeningFadeTimer|PadActAlignBytes|PadActiveChannel|PadChannelBusyFn|PadDeadzoneTable|PadIntQueueEntry|RCntIntrMaskBits|RaceBgmListIndex|RaceLoadSubState|RelFileSection00|RelFileSection01|RelFileSection05|RelFileSection11|RelFileSection15|SavedCourseIndex|ScreenFadeColors|ScreenFadeLayout|SelectedCarIndex|SetGraphDebugFmt|SioIrqQueueEntry|SoundSequenceSet|SparkFrameRanges|SsCtrlHandlerCC6|SsSeqOpenFailMsg|StRingSectorDest|StRingStartIndex|StRingWriteIndex|StrClearOTagRFmt|StrInitMainMenu0|StrLinkBattleEnd|StrPacRacingClub|StrPutDispEnvFmt|StrReadSectorFmt|StrResetGraphFmt|StrSioDeviceName|SvmSemitoneTable|TeamNameAssoluto|TrackEventTables|TrackLaneOffsets|TrackObjectCount|TrackPreviewTick|VoiceVolumeScale|_svm_stereo_mono|CarOffsetTables|CarUpdateStages|CardCheckCounts|CardFilePathFmt|CardRetriesLeft|CdFileBytesRead|CdFileSyncCount|ClearImage2Name|CourseBestTotal|CourseLapCounts|CourseListIndex|DispHRangeTable|DmaMadrDebugFmt|FileNotFoundFmt|FontGlyphWidths|HudGlyphUvRects|HwCardEvTimeout|I_STAT_PadVsync|LinkWriteHandle|LoadingCarIndex|MenuGridRecords|MenuPlayerIndex|ModelBankFormat|PadFirstChannel|ParticleEntries|RubberBandCurve|SavedLoadCursor|ScreenFadeState|SelectionPlayer|SeqOldFormatMsg|SplashLogoAlpha|SplashLogoFrame|SplitDeltaTimer|SpuDmaStartAddr|SpuWaitResetStr|SsVoiceInitRegs|StDataReadyFunc|StrCdComCodeFmt|StrCdNoParamFmt|StrMainMenu0Log|StrTotalInitLog|SwCardEvTimeout|TitleSceneStage|TrackLightTable|TrackObjectDefs|VSyncLastVcount|_SsMarkCallback|_err_math_param|_spu_inTransfer|_spu_trans_mode|_svm_sreg_dirty|BestSplitTimes|CarBoundsTable|CarSlotBuffers|CardCmdSioRegs|CardDirEntries|CardFileCounts|CardFileHandle|CardPortLoaded|CardPortResult|CdFileEntryFmt|CdFileLocation|CdIsoSectorBuf|CdReadyNameStr|CdTestParamNum|CurrentDrawBuf|DmaBusErrorFmt|DsDirFileCache|FontGlyphCells|GpuDriverTable|GpuMovePayload|HexDigitsUpper|I_STAT_CardSel|IntrStuckCount|IntrTimeoutFmt|LapFrameCounts|LinkEntryState|LinkReadHandle|LoadDestCursor|MaskBitSetting|MenuFrameState|MenuScriptStep|MenuStateIndex|MenuStateTable|ModelBankTable|NextSplitIndex|PadLastChannel|PadRangeLimits|ParticleSystem|PressStartText|RaceLoadCursor|RaceTimeDigits|RaceVoiceFlags|Rect16RowCount|SaveDataBackup|SavedRaceState|SceneLoadState|SelectionState|SetGraphQueFmt|SioDeviceBlock|SplitDistances|SsFCALLCcEntry|StRingFullFunc|StrCdReadRetry|StrFmtPercentS|StrPlayerLabel|StrSizeEqLdFmt|SvmLastChannel|TeamNameLizard|TrackNodeCount|TrackStartNode|VSyncRcnt1Last|VSyncThreshold|_err_math_mode|_snd_seq_s_max|_snd_seq_t_max|_spu_dmaIsRead|_spu_transMode|_svm_envx_hist|_svm_vab_count|_svm_vab_start|_svm_vab_total|BadgeRowCount|CD_syncResult|CarRecordPtrs|CardCmdBuffer|CardCmdSector|CardPortState|CardPortTimer|CardSlotState|CdComDelayReg|CdFileInfoFmt|CdFileSectors|CdReadyResult|CourseBestLap|DialogueState|DrawSubOtFlag|FrmTaskRecord|GlyphRowCount|GpuAlarmCount|GpuTimeoutFmt|HwCardEvError|ImageRowCount|IntrStackArea|IntroTimeline|JOY_DATA_Pad2|LoadImageName|D_800A41EC|MenuArenaBase|MenuFadeTimer|MenuItemCount|MenuScriptOps|ModeFuncTable|NodeLerpIndex|NowLoadingFmt|PadPulseState|PadRcnt2Start|PasswordBytes|PasswordChars|PendingModeId|PressStartStr|PutDrawEnvFmt|RaceLoadState|RaceStandings|ResetGraphFmt|ReverseCourse|SaveStoreRect|SelectionRows|SioDeviceName|SioShadowRegs|SlotBankSetup|SpuTimeoutFmt|StMaskEnabled|StRingHeaders|StrBadRectFmt|StrClearImage|StrNotSeqData|StrPleaseWait|StrPressStart|StrStoreImage|SwCardEvError|TeamAbbrevDRT|TeamAbbrevPRC|TeamAbbrevRTS|TileRectScene|TileRectState|TotalRaceTime|TrackNodeGrid|TrackNodesEnd|_snd_openflag|_spu_addrMode|_spu_isCalled|_spu_mem_mode|_spu_rev_attr|_spu_rev_flag|_svm_envx_ptr|_svm_sreg_buf|_svm_vab_used|spuVmMaxVoice|BadgeUvTable|CD_REG0_STRM|CD_REG2_STRM|CarSnapshots|CarSpecTable|CardChecksum|CardSio0Base|CardSio0Regs|CdSpuRegBase|ClearOTagFmt|DS_cachedDir|DS_pathTable|DrawBufIndex|FontGlyphMap|FractionText|FrameCounter|FrmTaskState|GpuFuncTable|LapTimeLabel|MenuDispMode|MenuScriptId|MenuTextRows|NodeLerpStep|PadButtonMap|PadComActive|PadRecvStage|PadTxBuffers|PadVsyncHook|PanGainTable|RC2WaitLimit|ReplayInputs|ReplayLength|SaveDataWork|SaveProgress|SpuZeroBlock|StCurFrameNo|StStartFrame|StrIso9660Id|StrLinkError|StrNewCarLog|TimeTextMMSS|UiFontGlyphs|VBLANK_MINUS|_snd_ev_flag|_spu_RQvoice|_spu_keystat|_spu_memList|kMaxPrograms|BestLapTime|CD_com_info|CD_dirNopen|CD_str_none|CardRxCount|CardRxTrace|CardTxCount|CdFileState|CdFileTable|CdReadState|ColorMatrix|CourseIndex|CourseNames|DS_dma_regs|DrawSyncFmt|GpuCmdQueue|HudMapPulse|HwCardEvIoe|HwCardEvNew|I_STAT_Card|InitPadFlag|LightMatrix|MenuScripts|NameCharset|Pad0Buttons|PadSio0Base|PauseCursor|PlayerCount|RCNT1_VALUE|RaceContext|RaceRecords|RankEntries|RepeatLabel|ReplayFrame|ReplayState|ScreenFrame|SioPollHook|SoundPaused|StCurHeader|StrFoundFmt|StrNoPlayer|SwCardEvIoe|SwCardEvNew|TrackLength|VabLoadArgs|ViewOffsetX|ViewOffsetY|ViewScreenH|XaRepeatMsg|_spu_RQmask|_svm_damper|_svm_ptable|_svm_vab_pg|_svm_vab_tn|_svm_vab_vh|BackColorB|BackColorG|BackColorR|CD_cbready|CD_pathtbl|CD_status1|DICR_VSync|DS_cd_reg3|GPU_STATUS|GPU_printf|I_STAT_Pad|I_STAT_Sio|LapRecords|LoadCursor|MenuArenas|MenuCursor|NewCar0Msg|NumPlayers|PlayerCars|RCNT1_MODE|RacePaused|Rect16Rows|SaveBuffer|SioDataReg|SioRequest|SoundSlots|SoundState|SplitTimes|StEndFrame|StRingBase|StrAddrFmt|StrAgeSolo|StrRectFmt|StrTerrazi|TrackNodes|UiRowCount|_snd_vmask|_spu_EVdma|_svm_okof1|_svm_okof2|_svm_okon1|_svm_okon2|_svm_onos1|_svm_onos2|_svm_orev1|_svm_orev2|_svm_rattr|_svm_voice|AtanTable|BadgeRows|CD_cbread|CD_cbsync|CD_comstr|CD_intstr|CD_status|COM_DELAY|CarInputs|CdCwLabel|DMA2_CHCR|DMA2_MADR|DMA3_CHCR|DMA3_MADR|DMA4_CHCR|DMA4_MADR|DMA6_CHCR|DMA6_MADR|DPCR_Intr|FarColorB|FarColorG|FarColorR|FrameEnvs|GlyphRows|ImageRows|LinkState|LoadIndex|LoadState|ModeFrame|ModeGroup|R4BinPath|R4StrPath|RCNT_REGS|RaceState|RankTable|SIO1_REGS|SPU_DELAY|SizeLdFmt|SlotOrder|StrCdSync|TeamLabel|_ss_score|ratan_tbl|CD_DELAY|CD_debug|CD_nopen|CarTable|CardSlot|DMA2_BCR|DMA3_BCR|DMA4_BCR|DMA6_BCR|Dialogue|GPU_DATA|JOY_DATA|LapTimes|PadPorts|RaceType|SPU_DPCR|SinTable|StrCd001|StrMaker|_autopan|_autovol|_spu_RXX|_spu_env|_spu_tsa|_svm_cur|CD_REG1|CD_REG2|CD_dpcr|CD_reg0|CD_reg3|ComCars|GPUSTAT|NumCars|NumLaps|Options|SsFCALL|StrName|_spu_RQ|_svm_pg|_svm_tn|_svm_vh|CD_pos|I_MASK|I_STAT|ModeId|UiRows|Vcount|_qout|DICR|DPCR|_qin);', global_alias, body)
            body = re.sub(r'\s+__asm__\("\$[0-9]+"\)', '', body)
            body = body.replace('"$1", ', '').replace(' : "$1"', '')
            body = body.replace('(s32 *)0x800A0000U', 'test_page_800A')
            body = body.replace('(s32 *)0x800B0000U', '(test_page_800A + 0x4000)')
            body = body.replace('(s32 *)0x80110000U', 'test_page_8011_words')
            body = body.replace('(s16 *)0x80110000U', 'test_page_8011')
            body = body.replace('*(volatile u16 *)0x1F801120', 'test_counter')
            source += body
        source += '''
void (*test_pair_after)(void);
void StClearRing(void) { test_pair_after(); }
'''
        (directory / 'functions.c').write_text(source)
        subprocess.run(['cc', '-shared', '-fPIC', '-O2', str(directory / 'functions.c'),
                        '-o', str(directory / 'functions.so')], check=True, capture_output=True)
        cls.library = C.CDLL(str(directory / 'functions.so'))
        cls.words = (C.c_int32 * 0x4000).in_dll(cls.library, 'test_page_800A')
        cls.halves = (C.c_int16 * 0x8000).in_dll(cls.library, 'test_page_8011')
        cls.pair_words = (C.c_int32 * 0x4000).in_dll(cls.library, 'test_page_8011_words')

    def assert_only_write(self, page, before, offset, encoded):
        expected = before[:offset] + encoded + before[offset + len(encoded):]
        self.assertEqual(C.string_at(C.addressof(page), C.sizeof(page)), expected)

    def test_exchange_returns_previous_word_and_changes_only_its_state(self):
        values = (-(1 << 31), -1, 0, 1, 0x12345678, (1 << 31) - 1)
        for name, address in EXCHANGES:
            function = getattr(self.library, name)
            function.argtypes = [C.c_int32]
            function.restype = C.c_int32
            offset = address - 0x800A0000
            for previous in values:
                for value in values:
                    C.memset(C.addressof(self.words), 0xA5, C.sizeof(self.words))
                    self.words[offset // 4] = previous
                    before = C.string_at(C.addressof(self.words), C.sizeof(self.words))
                    self.assertEqual(function(value), previous)
                    self.assert_only_write(self.words, before, offset, struct.pack('<i', value))

    def test_read_and_clear_returns_previous_state(self):
        function = self.library._padChkVsync
        function.argtypes = []
        function.restype = C.c_int32
        offset = 0xBCD0
        for previous in (-(1 << 31), -1, 0, 1, (1 << 31) - 1):
            C.memset(C.addressof(self.words), 0xA5, C.sizeof(self.words))
            self.words[offset // 4] = previous
            before = C.string_at(C.addressof(self.words), C.sizeof(self.words))
            self.assertEqual(function(), previous)
            self.assert_only_write(self.words, before, offset, b'\0' * 4)

    def test_constant_setters_change_only_the_expected_halfword(self):
        for name, address, value in SETTERS:
            function = getattr(self.library, name)
            function.argtypes = []
            function.restype = None
            C.memset(C.addressof(self.halves), 0xA5, C.sizeof(self.halves))
            before = C.string_at(C.addressof(self.halves), C.sizeof(self.halves))
            function()
            self.assert_only_write(self.halves, before, address - 0x80110000,
                                   struct.pack('<h', value))

    def test_mode_branch_sets_the_expected_boolean(self):
        function = self.library._spu_setInTransfer
        function.argtypes = [C.c_int32]
        function.restype = None
        for mode in (-(1 << 31), -1, 0, 1, 2, (1 << 31) - 1):
            C.memset(C.addressof(self.words), 0xA5, C.sizeof(self.words))
            before = C.string_at(C.addressof(self.words), C.sizeof(self.words))
            function(mode)
            self.assert_only_write(self.words, before, 0x480C, struct.pack('<i', int(mode != 1)))

    def test_mode_selection_records_both_fields(self):
        function = self.library.SpuSetTransferMode
        function.argtypes = [C.c_int32]
        function.restype = None
        for mode in (-(1 << 31), -1, 0, 1, 2, (1 << 31) - 1):
            C.memset(C.addressof(self.words), 0xA5, C.sizeof(self.words))
            expected = bytearray(C.string_at(C.addressof(self.words), C.sizeof(self.words)))
            struct.pack_into('<i', expected, 0x4848, mode)
            struct.pack_into('<i', expected, 0x47F4, int(mode == 1))
            function(mode)
            self.assertEqual(C.string_at(C.addressof(self.words), C.sizeof(self.words)), expected)

    def test_pair_update_is_visible_to_the_following_call(self):
        function = self.library.StSetRing
        function.argtypes = [C.c_void_p, C.c_int32]
        function.restype = None
        queues = [C.create_string_buffer(32) for _ in range(3)]
        cases = [(None, 0)] + [(C.addressof(q), second) for q, second in
                               zip(queues, (1, -1, (1 << 31) - 1))]
        for first, second in cases:
            C.memset(C.addressof(self.pair_words), 0xA5, C.sizeof(self.pair_words))
            before = C.string_at(C.addressof(self.pair_words), C.sizeof(self.pair_words))
            calls = []

            @AfterPair
            def after_pair():
                calls.append((C.c_void_p.in_dll(self.library, 'test_queue').value,
                              self.pair_words[0x314 // 4]))

            C.c_void_p.in_dll(self.library, 'test_pair_after').value = C.cast(after_pair, C.c_void_p).value
            function(first, second)
            self.assertEqual(calls, [(first, second)])
            self.assertEqual(C.c_void_p.in_dll(self.library, 'test_queue').value, first)
            self.assert_only_write(self.pair_words, before, 0x314, struct.pack('<i', second))

    def test_timer_snapshot_and_limit_use_separate_full_words(self):
        function = self.library.setRC2wait
        function.argtypes = [C.c_int32]
        function.restype = None
        counter = C.c_uint16.in_dll(self.library, 'test_counter')
        for tick in (0, 1, 0x7FFF, 0x8000, 0xFFFF):
            for limit in (-(1 << 31), -1, 0, 1, (1 << 31) - 1):
                C.memset(C.addressof(self.pair_words), 0xA5, C.sizeof(self.pair_words))
                before = C.string_at(C.addressof(self.pair_words), C.sizeof(self.pair_words))
                counter.value = tick
                function(limit)
                self.assertEqual(counter.value, tick)
                self.assert_only_write(self.pair_words, before, 0x46B8, struct.pack('<Ii', tick, limit))

    def test_reset_requires_the_enable_bit_and_preserves_other_state(self):
        function = self.library._cardCmdReset
        function.argtypes = [C.c_int32]
        function.restype = C.c_int32
        for flags in (-(1 << 31), -1, 0, 1, 2, 3, 4, (1 << 31) - 1):
            for value in (-(1 << 31), -1, 0, 1, (1 << 31) - 1):
                C.memset(C.addressof(self.words), 0xA5, C.sizeof(self.words))
                C.memset(C.addressof(self.pair_words), 0x5A, C.sizeof(self.pair_words))
                self.words[0xBDE4 // 4] = flags
                expected = bytearray(C.string_at(C.addressof(self.words), C.sizeof(self.words)))
                expected_other = bytearray(C.string_at(C.addressof(self.pair_words), C.sizeof(self.pair_words)))
                if flags & 1:
                    for offset in (0xBDF4, 0xBDF8, 0xBDE8):
                        struct.pack_into('<i', expected, offset, 0)
                    struct.pack_into('<i', expected, 0xBDEC, value)
                    struct.pack_into('<i', expected_other, 0x4730, 0)
                self.assertEqual(function(value), int(bool(flags & 1)))
                self.assertEqual(C.string_at(C.addressof(self.words), C.sizeof(self.words)), expected)
                self.assertEqual(C.string_at(C.addressof(self.pair_words), C.sizeof(self.pair_words)), expected_other)


if __name__ == '__main__':
    unittest.main()
