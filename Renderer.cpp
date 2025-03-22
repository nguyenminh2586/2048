#include "Renderer.h"
#include "Constants.h"
#include "Board.h"
#include "Button.h"
#include "Animation.h"
#include <cmath>
#include <string>

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
TTF_Font* font = nullptr;
TTF_Font* menuFont = nullptr;

// Cập nhật bảng màu cho các ô theo màu chuẩn của game 2048 chính thức
std::map<int, SDL_Color> tileColors = {
    {0, {205, 193, 180}},       // Màu nền ô trống (eee4da với độ trong suốt)
    {2, {238, 228, 218}},       // Màu chuẩn cho ô số 2
    {4, {237, 224, 200}},       // Màu chuẩn cho ô số 4
    {8, {242, 177, 121}},       // Màu chuẩn cho ô số 8
    {16, {245, 149, 99}},       // Màu chuẩn cho ô số 16
    {32, {246, 124, 95}},       // Màu chuẩn cho ô số 32
    {64, {246, 94, 59}},        // Màu chuẩn cho ô số 64
    {128, {237, 207, 114}},     // Màu chuẩn cho ô số 128
    {256, {237, 204, 97}},      // Màu chuẩn cho ô số 256
    {512, {237, 200, 80}},      // Màu chuẩn cho ô số 512
    {1024, {237, 197, 63}},     // Màu chuẩn cho ô số 1024
    {2048, {237, 194, 46}}      // Màu chuẩn cho ô số 2048
};

void renderText(const std::string &text, int x, int y, TTF_Font* usedFont) {
    if (usedFont == nullptr) usedFont = font;
    SDL_Surface* textSurface = TTF_RenderText_Solid(usedFont, text.c_str(), {0, 0, 0});
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect textRect = {x, y, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

// Thêm hàm vẽ hình chữ nhật bo tròn
void renderRoundedRect(SDL_Renderer* renderer, const SDL_Rect& rect, int radius, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);

    // Vẽ phần thân chính
    SDL_Rect body = {rect.x + radius, rect.y, rect.w - 2*radius, rect.h};
    SDL_RenderFillRect(renderer, &body);

    // Vẽ hai bên
    SDL_Rect left = {rect.x, rect.y + radius, radius, rect.h - 2*radius};
    SDL_RenderFillRect(renderer, &left);
    SDL_Rect right = {rect.x + rect.w - radius, rect.y + radius, radius, rect.h - 2*radius};
    SDL_RenderFillRect(renderer, &right);

    // Vẽ các góc tròn
    for(int i = 0; i <= radius; i++) {
        for(int j = 0; j <= radius; j++) {
            float distance = sqrt(i*i + j*j);
            if(distance <= radius) {
                // Góc trên trái
                SDL_RenderDrawPoint(renderer, rect.x + radius - i, rect.y + radius - j);
                // Góc trên phải
                SDL_RenderDrawPoint(renderer, rect.x + rect.w - radius + i, rect.y + radius - j);
                // Góc dưới trái
                SDL_RenderDrawPoint(renderer, rect.x + radius - i, rect.y + rect.h - radius + j);
                // Góc dưới phải
                SDL_RenderDrawPoint(renderer, rect.x + rect.w - radius + i, rect.y + rect.h - radius + j);
            }
        }
    }
}

// Cập nhật hàm renderTile
void renderTile(int value, int x, int y, float scale) {
    int width = static_cast<int>(TILE_SIZE * scale);
    int height = static_cast<int>(TILE_SIZE * scale);

    // Điều chỉnh vị trí để ô nằm giữa
    int adjustedX = x + (TILE_SIZE - width) / 2;
    int adjustedY = y + (TILE_SIZE - height) / 2;

    SDL_Rect tile = {adjustedX, adjustedY, width - 5, height - 5};
    SDL_Color color = tileColors[value];

    // Vẽ ô với viền bo tròn
    renderRoundedRect(renderer, tile, 8, color);

    if (value != 0) {
        // Điều chỉnh kích thước font dựa trên giá trị
        int fontSize = 32;  // Kích thước mặc định
        if (value >= 100) fontSize = 28;
        if (value >= 1000) fontSize = 24;

        // Màu chữ phụ thuộc vào giá trị ô
        SDL_Color textColor = (value >= 8) ? SDL_Color{255, 255, 255} : SDL_Color{119, 110, 101};
        std::string valueStr = std::to_string(value);

        // Tạo bề mặt chữ để tính toán kích thước chính xác
        SDL_Surface* textSurface = TTF_RenderText_Blended(font, valueStr.c_str(), textColor);

        // Tính toán vị trí chính xác để chữ nằm hoàn toàn ở giữa ô
        int textWidth = textSurface->w;
        int textHeight = textSurface->h;

        // Căn giữa chính xác
        int textX = adjustedX + (width - textWidth) / 2;
        int textY = adjustedY + (height - textHeight) / 2;

        // Tạo texture từ surface
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

        // Render chữ tại vị trí đã tính toán
        SDL_Rect textRect = {textX, textY, textWidth, textHeight};
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

        // Giải phóng tài nguyên
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
    }
}

void renderBoard() {
    // Tăng kích thước viền từ 5 lên 10 pixel
    SDL_Rect boardRect = {BOARD_X - 10, BOARD_Y - 10, TILE_SIZE * GRID_SIZE + 20, TILE_SIZE * GRID_SIZE + 20};
    SDL_Color boardColor = {187, 173, 160, 255};
    renderRoundedRect(renderer, boardRect, 12, boardColor); // Tăng bán kính bo góc một chút

    // Sử dụng khoảng cách đồng nhất cho các ô
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            // Đảm bảo các ô có khoảng cách đều nhau (sử dụng 8 thay vì 5)
            SDL_Rect emptyTile = {BOARD_X + j * TILE_SIZE + 4, BOARD_Y + i * TILE_SIZE + 4, TILE_SIZE - 8, TILE_SIZE - 8};
            SDL_Color emptyColor = {205, 193, 180, 255};
            renderRoundedRect(renderer, emptyTile, 8, emptyColor);
        }
    }

    // Nếu đang có animation, chỉ vẽ các block đang di chuyển
    if (animating) {
        for (auto& anim : animations) {
            if (anim.active) {
                renderTile(anim.value, anim.currentX, anim.currentY, anim.scale);
            }
        }
    } else {
        // Vẽ bảng hiện tại nếu không có animation
        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                if (board[i][j] != 0) {
                    renderTile(board[i][j], BOARD_X + j * TILE_SIZE, BOARD_Y + i * TILE_SIZE);
                }
            }
        }
    }
}

void renderMenu() {
    // Màu nền gradient cho menu
    SDL_Rect bgRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_Color bgColor = {41, 128, 185, 255}; // Màu xanh dương đậm
    SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, 255);
    SDL_RenderFillRect(renderer, &bgRect);

    // Tạo hiệu ứng gradient đơn giản
    for (int i = 0; i < SCREEN_HEIGHT; i += 2) {
        SDL_SetRenderDrawColor(renderer, 52, 152, 219, 150 - i / 4);
        SDL_RenderDrawLine(renderer, 0, i, SCREEN_WIDTH, i);
    }

    // Thêm khung trang trí cho tiêu đề
    SDL_Rect titleFrame = {80, 100, 340, 100};
    SDL_Color frameColor = {243, 156, 18, 255}; // Màu vàng cam đẹp mắt
    renderRoundedRect(renderer, titleFrame, 15, frameColor);

    // Nền trong cho tiêu đề
    SDL_Rect titleBackground = {90, 110, 320, 80};
    SDL_Color titleBgColor = {236, 240, 241, 255}; // Màu xanh nhạt đẹp mắt
    renderRoundedRect(renderer, titleBackground, 10, titleBgColor);

    // Tạo hiệu ứng ánh sáng
    for (int i = 0; i < 10; i++) {
        SDL_Rect lightRect = {90 + i, 110 + i, 320 - i*2, 80 - i*2};
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, static_cast<Uint8>(50 - i*5));
        renderRoundedRect(renderer, lightRect, 10 - i, SDL_Color{255, 255, 255, static_cast<Uint8>(50 - i*5)});
    }

    // Tiêu đề menu với màu sắc nổi bật
    TTF_Font* titleFont = TTF_OpenFont("assets/fonts/ARLRDBD.TTF", 42); // Font to hơn
    if (!titleFont) titleFont = menuFont; // Dùng font dự phòng

    SDL_Color titleColor = {52, 73, 94, 255}; // Màu xanh đậm đẹp mắt
    SDL_Surface* titleSurface = TTF_RenderText_Blended(titleFont, "2048 Game", titleColor);
    SDL_Texture* titleTexture = SDL_CreateTextureFromSurface(renderer, titleSurface);

    // Căn giữa chính xác
    int titleX = (SCREEN_WIDTH - titleSurface->w) / 2;
    int titleY = 110 + (80 - titleSurface->h) / 2;

    SDL_Rect titleRect = {titleX, titleY, titleSurface->w, titleSurface->h};
    SDL_RenderCopy(renderer, titleTexture, NULL, &titleRect);

    SDL_FreeSurface(titleSurface);
    SDL_DestroyTexture(titleTexture);
    if (titleFont != menuFont) TTF_CloseFont(titleFont);

    // Vẽ các nút với khoảng cách lớn hơn
    newGameButton.rect = {150, 240, 200, 60}; // Nút to hơn
    continueButton.rect = {150, 320, 200, 60};
    exitButton.rect = {150, 400, 200, 60};

    renderButton(newGameButton);
    renderButton(continueButton);
    renderButton(exitButton);
}

void renderScore() {
    // Màu text cho điểm số - sử dụng màu text của game 2048
    SDL_Color textColor = {119, 110, 101, 255};

    // Render điểm số với màu đã cập nhật
    SDL_Surface* scoreSurface = TTF_RenderText_Solid(font, ("Score: " + std::to_string(score)).c_str(), textColor);
    SDL_Texture* scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);
    SDL_Rect scoreRect = {50, 50, scoreSurface->w, scoreSurface->h};
    SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreRect);
    SDL_FreeSurface(scoreSurface);
    SDL_DestroyTexture(scoreTexture);

    // Render điểm cao với màu đã cập nhật
    SDL_Surface* bestSurface = TTF_RenderText_Solid(font, ("Best: " + std::to_string(bestScore)).c_str(), textColor);
    SDL_Texture* bestTexture = SDL_CreateTextureFromSurface(renderer, bestSurface);
    SDL_Rect bestRect = {250, 50, bestSurface->w, bestSurface->h};
    SDL_RenderCopy(renderer, bestTexture, NULL, &bestRect);
    SDL_FreeSurface(bestSurface);
    SDL_DestroyTexture(bestTexture);

    // Nút Menu với màu sắc từ bảng màu 2048 và hiệu ứng hover
    SDL_Rect menuButtonRect = {SCREEN_WIDTH - 100, 50, 80, 30};

    // Màu sắc cho nút dựa vào trạng thái hover
    SDL_Color menuButtonColor, menuTextColor;

    if (menuButtonHighlighted) {
        // Khi di chuột đến - sử dụng màu cam nổi bật (tương tự nút menu được hover)
        menuButtonColor = {245, 149, 99, 255}; // Màu cam đậm giống các nút menu
        menuTextColor = {255, 255, 255, 255}; // Văn bản màu trắng
    } else {
        // Nút bình thường
        menuButtonColor = {238, 228, 218, 255}; // Màu của ô số 2
        menuTextColor = {119, 110, 101, 255}; // Màu text của game 2048
    }

    // Vẽ nút với góc bo tròn
    renderRoundedRect(renderer, menuButtonRect, 8, menuButtonColor);

    // Thêm hiệu ứng viền tương tự như nút menu
    if (menuButtonHighlighted) {
        SDL_Rect borderRect = {menuButtonRect.x - 2, menuButtonRect.y - 2,
                               menuButtonRect.w + 4, menuButtonRect.h + 4};
        SDL_Color borderColor = {243, 156, 18, 255}; // Viền vàng cam cho nút được hover
        renderRoundedRect(renderer, borderRect, 10, borderColor);
    }

    // Văn bản "Menu" với màu dựa vào trạng thái hover
    SDL_Surface* menuSurface = TTF_RenderText_Solid(font, "Menu", menuTextColor);
    SDL_Texture* menuTexture = SDL_CreateTextureFromSurface(renderer, menuSurface);

    // Căn giữa chính xác văn bản trong nút
    int textX = menuButtonRect.x + (menuButtonRect.w - menuSurface->w) / 2;
    int textY = menuButtonRect.y + (menuButtonRect.h - menuSurface->h) / 2;

    SDL_Rect menuTextRect = {textX, textY, menuSurface->w, menuSurface->h};
    SDL_RenderCopy(renderer, menuTexture, NULL, &menuTextRect);
    SDL_FreeSurface(menuSurface);
    SDL_DestroyTexture(menuTexture);
}

void render() {
    SDL_SetRenderDrawColor(renderer, 250, 248, 239, 255); // Màu nền sáng hơn
    SDL_RenderClear(renderer);

    if (showMenu) {
        renderMenu();
    } else {
        renderScore();
        renderBoard();

        // Hiển thị thông báo thắng game nếu cần
        if (hasWon) {
            SDL_Rect winRect = {50, 510, 400, 40};
            SDL_SetRenderDrawColor(renderer, 220, 250, 220, 255); // Màu xanh nhạt cho thông báo thắng
            SDL_RenderFillRect(renderer, &winRect);
            renderText("You Win! Press 'N' for New Game", 60, 520);
        }
        // Hiển thị thông báo game over nếu cần
        if (gameOver) {
            SDL_Rect gameOverRect = {50, 550, 400, 40};
            SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
            SDL_RenderFillRect(renderer, &gameOverRect);
            renderText("Game Over! Press 'N' for New Game", 60, 560);
        }
    }

    SDL_RenderPresent(renderer);
}
