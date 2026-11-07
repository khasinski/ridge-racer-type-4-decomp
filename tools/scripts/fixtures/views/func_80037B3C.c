typedef struct { char pad[0x320]; } E;

extern int InitCarTable();

extern int InitPlayerCar();

extern int InitComCars();

extern int InitReplayBuffers();

extern int PsyqBzero();

extern E PlayerCars[];

extern short D_800AC3FC;

extern int CarDrawSlotOffset;

extern short NumCars;

extern signed char LinkEntryState[];

extern int CarTable[];

extern int TrackLength;

extern short PlayerCount;

extern int SavedRaceState;

extern int TotalRaceTime;

void InitRaceCars(int a0)
{
    int s1, s2, i, t;
    signed char *s3;
    int *s0;
    D_800AC3FC = 0;
    InitCarTable();
    CarDrawSlotOffset = 0;
    switch (a0) {
    case 0:
    default:
        i = 0;
        InitPlayerCar(PlayerCars, i, i);
        InitReplayBuffers(0, 0);
        break;
    case 1:
        InitPlayerCar(PlayerCars, 0, 7);
        InitComCars(7);
        InitReplayBuffers(0, 0);
        break;
    case 2:
        i = 0;
        InitPlayerCar(PlayerCars, i, i);
        i = 1;
        InitPlayerCar(PlayerCars + 1, i, i);
        InitReplayBuffers(1, 0);
        break;
    case 3:
        InitPlayerCar(PlayerCars, 0, NumCars - 2);
        InitPlayerCar(PlayerCars + 1, 1, NumCars - 1);
        InitComCars(NumCars - 2);
        InitReplayBuffers(1, 0);
        break;
    case 4:
        s1 = 0;
        s2 = 0;
        if (NumCars > 0) {
            s3 = LinkEntryState;
            s0 = CarTable;
            do {
                if (*(signed char *)(s1 + (int)s3) == 1) {
                    InitPlayerCar(*s0, s1, s2);
                    s2++;
                } else {
                    *(short *)(*(int *)s0 + 0x1E4) = 0;
                    i = *(int *)s0;
                    t = TrackLength;
                    *(short *)(i + 0x1E8) = -1;
                    *(int *)(*(int *)s0 + 0x184) = -(t * 300);
                }
                s0++;
                s1++;
            } while (s1 < NumCars);
        }
        PlayerCount = s2;
        InitReplayBuffers(2, 0);
        break;
    case 5:
        PsyqBzero(PlayerCars, 0x320);
        InitComCars(7);
        InitReplayBuffers(1, 0);
        SavedRaceState = 2;
        break;
    case 6:
        InitPlayerCar(PlayerCars, 0, 1);
        InitComCars(1);
        InitReplayBuffers(0, 0);
        break;
    }
    TotalRaceTime = 0;
}
