#ifndef PINS_H
#define PINS_H

#include "main.h"

void PINB_2_INIT(void);
void TIM2_INIT(void);
void Blink_Start(void);
void Blink_Stop(void);
void Blink(int t);

#endif