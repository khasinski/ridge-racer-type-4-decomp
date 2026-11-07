#include "game/car/car.h"
extern Car PlayerCars[];
extern int PlayerCar1Speed;
void CopyCarToPlayerSlot1(Car *a0) {
    PlayerCars[1] = *a0;
    *(short *)&a0->unk1E4 = *(unsigned char *)&a0->unk1E4;
    PlayerCar1Speed = a0->unk1E0 + 2;
}
