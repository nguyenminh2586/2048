#ifndef ANIMATION_H
#define ANIMATION_H

#include <vector>

// Cấu trúc cho animation
struct Animation {
    int startX, startY;
    int targetX, targetY;
    int currentX, currentY;
    int value;
    float scale;
    bool active;
    bool isMerging;
    bool isNew;
    int framesLeft;
};

extern std::vector<Animation> animations;
extern bool animating;

void updateAnimations();

#endif // ANIMATION_H 