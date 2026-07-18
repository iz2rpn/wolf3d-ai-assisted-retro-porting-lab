#ifndef Z6S_INPUT_H
#define Z6S_INPUT_H

enum Z6S_InputMode
{
    Z6S_INPUT_GAME = 0,
    Z6S_INPUT_MENU = 1
};

void Z6S_InputPump(void);
void Z6S_InputSetMode(int mode);
int Z6S_InputTakeMenuTap(int *x, int *y);

#endif
