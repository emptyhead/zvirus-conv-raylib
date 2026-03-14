#include <string.h>
#include "menu.h"
#include "audio.h"
#include "raylib.h"
#include "util.h"
#include "constants.h"

void MenuInit(void) {
    // Menu 0: Options
    gMenus[0].min_option = 1;
    gMenus[0].max_option = 6;
    gMenus[0].y_scale = 128;
    gMenus[0].y_spacing = 20;
    gMenus[0].x_spacing = 64;
    strcpy(gMenus[0].name[0], "O P T I O N S");
    strcpy(gMenus[0].name[1], "Difficulty");
    strcpy(gMenus[0].name[2], "Wave");
    strcpy(gMenus[0].name[3], "Control");
    strcpy(gMenus[0].name[4], "Camera");
    strcpy(gMenus[0].name[5], "Test Level");
    strcpy(gMenus[0].name[6], "Start");

    gMenus[0].sub_min[1] = 0; gMenus[0].sub_max[1] = 2; gMenus[0].sub_current[1] = 1;
    strcpy(gMenus[0].sub_name[1], "Easy        Medium      Hard");
    
    gMenus[0].sub_min[2] = 0; gMenus[0].sub_max[2] = 5; gMenus[0].sub_current[2] = 0;
    strcpy(gMenus[0].sub_name[2], "1           5           9           13           17          21 ");

    gMenus[0].sub_min[3] = 0; gMenus[0].sub_max[3] = 3; gMenus[0].sub_current[3] = 1;
    strcpy(gMenus[0].sub_name[3], "Demo        Mouse       Original    Joypad      ");

    gMenus[0].sub_min[4] = 0; gMenus[0].sub_max[4] = 2; gMenus[0].sub_current[4] = 0;
    strcpy(gMenus[0].sub_name[4], "Original    Chase       Rear        ");

    gMenus[0].sub_min[5] = -1; // Test Level has no sub
    gMenus[0].sub_min[6] = -1; // Start has no sub

    // Menu 1: Wave Complete
    gMenus[1].min_option = 4;
    gMenus[1].max_option = 4;
    gMenus[1].y_scale = 128;
    gMenus[1].y_spacing = 24;
    gMenus[1].x_spacing = 84;
    strcpy(gMenus[1].name[0], "WAVE COMPLETE");
    strcpy(gMenus[1].name[1], "Infected");
    strcpy(gMenus[1].name[2], "Un-infected");
    strcpy(gMenus[1].name[3], "Area Bonus");
    strcpy(gMenus[1].name[4], "Continue");
    gMenus[1].sub_min[4] = -1;

    // Menu 2: Game Over
    gMenus[2].min_option = 1;
    gMenus[2].max_option = 1;
    gMenus[2].y_scale = 32;
    gMenus[2].y_spacing = 12;
    gMenus[2].x_spacing = 64;
    strcpy(gMenus[2].name[0], "G A M E  O V E R");
    strcpy(gMenus[2].name[1], "OK");
    gMenus[2].sub_min[1] = -1;

    // Menu 3: Paused
    gMenus[3].min_option = 1;
    gMenus[3].max_option = 2;
    gMenus[3].y_scale = 64;
    gMenus[3].y_spacing = 20;
    gMenus[3].x_spacing = 64;
    strcpy(gMenus[3].name[0], "P A U S E D");
    strcpy(gMenus[3].name[1], "Continue");
    strcpy(gMenus[3].name[2], "Quit");
    gMenus[3].sub_min[1] = -1;
    gMenus[3].sub_min[2] = -1;

    // Menu 4: Game Complete
    gMenus[4].min_option = 1;
    gMenus[4].max_option = 1;
    gMenus[4].y_scale = 32;
    gMenus[4].y_spacing = 12;
    gMenus[4].x_spacing = 64;
    strcpy(gMenus[4].name[0], "G A M E  C O M P L E T E");
    strcpy(gMenus[4].name[1], "Well Done !!!");
    gMenus[4].sub_min[1] = -1;
}

bool MenuUpdate(int menuId) {
    if (menuId < 0 || menuId > MENU_COUNT) return false;
    Menu *m = &gMenus[menuId];
    
    // Lazy init current option
    static int lastMenuId = -1;
    if (menuId != lastMenuId) {
        m->current = (menuId == 0) ? m->max_option : m->min_option;
        lastMenuId = menuId;
    }

    int dy = IsKeyPressed(BIND_MENU_DOWN_KEY) - IsKeyPressed(BIND_MENU_UP_KEY);
    int dx = IsKeyPressed(BIND_MENU_RIGHT_KEY) - IsKeyPressed(BIND_MENU_LEFT_KEY);
    bool select = IsKeyPressed(BIND_MENU_SELECT_1) || IsKeyPressed(BIND_MENU_SELECT_2);

    if (dy != 0) {
        AudioPlay(gSoundMenu, 1.0f, 0.5f);
        m->current = clampi(m->current + dy, m->min_option, m->max_option);
    }

    if (dx != 0 && m->sub_min[m->current] != -1) {
        AudioPlay(gSoundMenu, 1.0f, 0.5f);
        m->sub_current[m->current] = clampi(m->sub_current[m->current] + dx, 
                                           m->sub_min[m->current], 
                                           m->sub_max[m->current]);
    }

    if (select && m->sub_min[m->current] == -1) {
        AudioPlay(gSoundMenu, 1.0f, 0.5f);
        lastMenuId = -1; // Reset for next time
        return true;
    }

    return false;
}

int MenuGetSelected(int menuId) {
    if (menuId < 0 || menuId > MENU_COUNT) return -1;
    return gMenus[menuId].current;
}

void MenuDraw(int menuId) {
    if (menuId < 0 || menuId > MENU_COUNT) return;
    Menu *m = &gMenus[menuId];

    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    int centerX = screenWidth / 2;
    int centerY = screenHeight / 2;

    // Better centering: base everything on a fixed height for titles and options
    int titleSize = 40;
    int optionSize = 20;
    int totalHeight = (m->max_option + 1) * m->y_spacing * 2;
    int startY = centerY - (totalHeight / 2);

    // Title
    DrawText(m->name[0], centerX - MeasureText(m->name[0], titleSize) / 2, startY, titleSize, YELLOW);

    for (int i = 1; i <= m->max_option; i++) {
        Color col = (i == m->current) ? WHITE : GRAY;
        int y = startY + (titleSize + 20) + (i-1) * m->y_spacing * 2;
        
        // Option Name
        DrawText(m->name[i], centerX - 250, y, optionSize, col);

        // Sub Option
        if (m->sub_min[i] != -1) {
            const char *sub = "";
            if (menuId == 0) { // Options Menu
                switch(i) {
                    case 1: // Difficulty
                        if (m->sub_current[i] == 0) sub = "Easy";
                        else if (m->sub_current[i] == 1) sub = "Medium";
                        else sub = "Hard";
                        break;
                    case 2: // Wave
                        sub = TextFormat("%d", m->sub_current[i] * 4 + 1);
                        break;
                    case 3: // Control
                        if (m->sub_current[i] == 0) sub = "Demo";
                        else if (m->sub_current[i] == 1) sub = "Mouse";
                        else if (m->sub_current[i] == 2) sub = "Original";
                        else sub = "Joypad";
                        break;
                    case 4: // Camera
                        if (m->sub_current[i] == 0) sub = "Original";
                        else if (m->sub_current[i] == 1) sub = "Chase";
                        else sub = "Rear";
                        break;
                }
            } else if (menuId == 1) { // Stats
                 sub = m->sub_name[i];
            }
            DrawText(sub, centerX + 50, y, optionSize, (i == m->current) ? MAROON : DARKBROWN);
        }
    }
}
