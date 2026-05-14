#pragma once
#include "raylib.h"

enum LocationType { LOC_FOREST = 0, LOC_DESERT, LOC_SNOW, LOC_SPACE };

LocationType GetLocationByScore(int currentScore);
Color GetUITextColor(LocationType loc);