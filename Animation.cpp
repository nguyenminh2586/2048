#include "Animation.h"
#include "Constants.h"

std::vector<Animation> animations;
bool animating = false;

// Cập nhật trạng thái animation
void updateAnimations() {
    bool stillAnimating = false;

    for (auto& anim : animations) {
        if (!anim.active) continue;

        anim.framesLeft--;

        if (anim.framesLeft <= 0) {
            anim.active = false;
            continue;
        }

        stillAnimating = true;

        // Tính vị trí hiện tại dựa trên interpolation
        float progress = 1.0f - (float)anim.framesLeft / ANIMATION_DURATION;
        anim.currentX = anim.startX + (anim.targetX - anim.startX) * progress;
        anim.currentY = anim.startY + (anim.targetY - anim.startY) * progress;

        // Hiệu ứng scale
        if (anim.isNew) {
            // Block mới xuất hiện sẽ scale từ 0.1 đến 1.0
            anim.scale = 0.1f + 0.9f * progress;
        } else if (anim.isMerging) {
            // Block hợp nhất sẽ lớn lên và nhỏ đi
            if (progress < 0.5f) {
                anim.scale = 1.0f + 0.2f * (progress * 2);  // Scale up
            } else {
                anim.scale = 1.2f - 0.2f * ((progress - 0.5f) * 2);  // Scale down
            }
        }
    }

    animating = stillAnimating;

    // Nếu tất cả animation kết thúc, xóa danh sách
    if (!animating) {
        animations.clear();
    }
}
