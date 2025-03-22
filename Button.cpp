#include "Button.h"
#include "Renderer.h"
#include "Util.h"
#include <SDL_ttf.h>

Button newGameButton = {{150, 250, 200, 50}, "New Game", false};
Button continueButton = {{150, 320, 200, 50}, "Continue", false};
Button exitButton = {{150, 390, 200, 50}, "Exit", false};
bool menuButtonHighlighted = false;

void renderButton(Button &button) {
    // Màu sắc mới cho các nút, lấy cảm hứng từ màu sắc chính của game 2048
    SDL_Color backgroundColor, textColor;

    if (button.highlighted) {
        // Nút được hover - sử dụng màu cam nổi bật (tương tự ô số 16)
        backgroundColor = {245, 149, 99, 255}; // Màu cam đậm hơn
        textColor = {255, 255, 255, 255}; // Văn bản màu trắng để dễ đọc
    } else {
        // Nút thường - sử dụng màu xanh nhạt (giống màu ô số 2 nhưng đặc biệt hơn)
        backgroundColor = {236, 240, 241, 255}; // Màu xanh nhạt đẹp mắt
        textColor = {52, 73, 94, 255}; // Màu xanh đậm cho văn bản
    }

    // Vẽ nút với góc bo tròn lớn hơn
    renderRoundedRect(renderer, button.rect, 12, backgroundColor);

    // Tạo hiệu ứng viền nhẹ
    SDL_Rect borderRect = {button.rect.x - 2, button.rect.y - 2, button.rect.w + 4, button.rect.h + 4};
    SDL_Color borderColor = button.highlighted ?
                           SDL_Color{243, 156, 18, 255} : // Viền vàng cam cho nút được hover
                           SDL_Color{189, 195, 199, 255}; // Viền xám nhạt cho nút thường
    renderRoundedRect(renderer, borderRect, 14, borderColor);

    // Văn bản nút với màu đã cập nhật và vị trí chuẩn hơn
    TTF_Font* boldFont = TTF_OpenFont("ARLRDBD.TTF", 30); // Font lớn hơn
    if (!boldFont) boldFont = menuFont; // Dùng font dự phòng nếu không tìm thấy

    SDL_Surface* textSurface = TTF_RenderText_Blended(boldFont, button.text.c_str(), textColor);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

    // Căn giữa chính xác
    int textX = button.rect.x + (button.rect.w - textSurface->w) / 2;
    int textY = button.rect.y + (button.rect.h - textSurface->h) / 2;

    SDL_Rect textRect = {textX, textY, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
    if (boldFont != menuFont) TTF_CloseFont(boldFont);
}

void processMenuClick(int x, int y) {
    // Kiểm tra click vào nút New Game
    if (x >= newGameButton.rect.x && x <= newGameButton.rect.x + newGameButton.rect.w &&
        y >= newGameButton.rect.y && y <= newGameButton.rect.y + newGameButton.rect.h) {
        newGame();
    }
    // Kiểm tra click vào nút Continue
    else if (x >= continueButton.rect.x && x <= continueButton.rect.x + continueButton.rect.w &&
             y >= continueButton.rect.y && y <= continueButton.rect.y + continueButton.rect.h) {
        if (!loadGame()) {
            newGame(); // Nếu không có game đã lưu, bắt đầu game mới
        }
    }
    // Kiểm tra click vào nút Exit
    else if (x >= exitButton.rect.x && x <= exitButton.rect.x + exitButton.rect.w &&
             y >= exitButton.rect.y && y <= exitButton.rect.y + exitButton.rect.h) {
        saveGame(); // Lưu game trước khi thoát
        exit(0);
    }
}
