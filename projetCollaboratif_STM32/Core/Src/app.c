#include "stdio.h"
#include "gpio.h"
#include "app.h"

joystick stickB2;


void joystickReader(joystick *stick){

stick->up=HAL_GPIO_ReadPin(JOY_UP_GPIO_Port,JOY_UP_Pin);
stick->down=HAL_GPIO_ReadPin(JOY_DOWN_GPIO_Port,JOY_DOWN_Pin);
stick->left=HAL_GPIO_ReadPin(JOY_LEFT_GPIO_Port,JOY_LEFT_Pin);
stick->right=HAL_GPIO_ReadPin(JOY_RIGHT_GPIO_Port,JOY_RIGHT_Pin);
stick->center=HAL_GPIO_ReadPin(JOY_SEL_GPIO_Port,JOY_SEL_Pin);

}


void app_init(void){


}

void app_periodic(void){

joystickReader(&stickB2);


}
