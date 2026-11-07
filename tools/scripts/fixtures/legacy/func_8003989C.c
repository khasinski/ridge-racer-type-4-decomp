extern unsigned char D_8009F810[];
extern unsigned char SelectionState[];
void SetSelectionTableIndex(void) {
    int v;
    if (SelectionState[0x1E] == 0x12) {
        int i = SelectionState[0x1C] * 2 + (SelectionState[0x1D] >> 1);
        v = D_8009F810[i];
    } else {
        v = 1;
    }
    if (v < 0) {
        v = 0;
    } else if (v >= 9) {
        v = 8;
    }
    SelectionState[0x12] = v;
    if (v == 1) {
        SelectionState[0x13] = SelectionState[0x18];
    }
}
