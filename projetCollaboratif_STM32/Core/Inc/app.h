#include "stdbool.h"


typedef struct {
    bool up,down,left,right,center;
} joystick;


void joystickReader(joystick *stick);
void app_periodic(void);
void app_init(void);