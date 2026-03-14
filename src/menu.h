#pragma once

#include "game.h"
#include "world.h"

void MenuInit(void);
// MenuUpdate returns true if the menu was exited (Start or OK selected)
bool MenuUpdate(int menuId);
// Returns the currently highlighted option index
int MenuGetSelected(int menuId);
void MenuDraw(int menuId);
