#pragma once

#include "raylib.h"

// Loads a Blitz3D .my3d format file into a Raylib Model.
// Reverses Z-axis and triangle winding for right-handed coordinates.
// Applies OSCALE to the vertices.
Model LoadMy3D(const char *fileName);

// state=1: Infected, state=2: Destroyed (implode), state=3: Destroyed (squash)
Model LoadMy3DState(const char *fileName, int state);
