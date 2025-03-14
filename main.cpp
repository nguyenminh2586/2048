#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <map>
#include <fstream>
#include <vector>
#include <cmath>

using namespace std;

const int SCREEN_WIDTH = 500;
const int SCREEN_HEIGHT = 600;
const int GRID_SIZE = 4;
const int TILE_SIZE = 100;
const int BOARD_X = 50;
const int BOARD_Y = 150;
const int winPoint = 2048;
const string SAVE_FILE = "2048_save.dat";
const int ANIMATION_SPEED = 50;  // Tốc độ animation (pixels per frame)
const int ANIMATION_DURATION = 4; // Số frame cho mỗi animation
const float SCALE_SPEED = 0.5f;  // Tốc độ hiệu ứng scale

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
TTF_Font* font = nullptr;
TTF_Font* menuFont = nullptr;
int board[GRID_SIZE][GRID_SIZE];
int previousBoard[GRID_SIZE][GRID_SIZE]; // Lưu trạng thái trước đó để tính toán animation
int score = 0;
int bestScore = 0;
bool showMenu = true;
bool hasWon = false;
bool gameOver = false;
bool animating = false;
bool menuButtonHighlighted = false;
Uint32 lastFrameTime = 0;
const int FPS = 144;
const int FRAME_DELAY = 1000 / FPS;

// Âm thanh
Mix_Chunk* moveSound = nullptr;
Mix_Chunk* mergeSound = nullptr;
Mix_Chunk* winSound = nullptr;
Mix_Chunk* loseSound = nullptr;
Mix_Music* backgroundMusic = nullptr;
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

vector<Animation> animations;

int dirLine[] = {1, 0, -1, 0};
int dirColumn[] = {0, 1, 0, -1};
// Cập nhật bảng màu cho các ô theo màu chuẩn của game 2048 chính thức
map<int, SDL_Color> tileColors = {
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

// Cấu trúc cho các nút menu
struct Button {
    SDL_Rect rect;
    string text;
    bool highlighted;
};

Button newGameButton = {{150, 250, 200, 50}, "New Game", false};
Button continueButton = {{150, 320, 200, 50}, "Continue", false};
Button exitButton = {{150, 390, 200, 50}, "Exit", false};

// Hàm tải âm thanh

bool loadSounds() {
    // Khởi tạo SDL_mixer với tham số phù hợp
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        cout << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << endl;
        return false;
    }

    // Tải các file âm thanh
    moveSound = Mix_LoadWAV("sounds/move.wav");
    mergeSound = Mix_LoadWAV("sounds/merge.wav");
    winSound = Mix_LoadWAV("sounds/win.wav");
    loseSound = Mix_LoadWAV("sounds/lose.wav");

    // Tải nhạc nền
    backgroundMusic = Mix_LoadMUS("sounds/menu_music.mp3");

    // Kiểm tra lỗi (nếu không tìm thấy file âm thanh, game vẫn hoạt động)
    if (!moveSound || !mergeSound || !winSound || !loseSound) {
        cout << "Warning: Unable to load sound effects! SDL_mixer Error: " << Mix_GetError() << endl;
        cout << "Game will continue without sound effects." << endl;
    }

    if (!backgroundMusic) {
        cout << "Warning: Unable to load background music! SDL_mixer Error: " << Mix_GetError() << endl;
        cout << "Game will continue without background music." << endl;
    }

    return true;
}

// Giải phóng tài nguyên âm thanh
void closeSounds() {
    if (moveSound) Mix_FreeChunk(moveSound);
    if (mergeSound) Mix_FreeChunk(mergeSound);
    if (winSound) Mix_FreeChunk(winSound);
    if (loseSound) Mix_FreeChunk(loseSound);
    if (backgroundMusic) Mix_FreeMusic(backgroundMusic);

    moveSound = nullptr;
    mergeSound = nullptr;
    winSound = nullptr;
    loseSound = nullptr;
    backgroundMusic = nullptr;

    Mix_CloseAudio();
}

// Thêm hàm mới để quản lý nhạc nền
void controlBackgroundMusic(bool playMusic) {
    if (backgroundMusic) {
        if (playMusic) {
            // Nếu nhạc chưa phát, bắt đầu phát
            if (!Mix_PlayingMusic()) {
                Mix_PlayMusic(backgroundMusic, -1); // Tham số -1 để lặp vô hạn
            } else if (Mix_PausedMusic()) {
                // Nếu nhạc đang tạm dừng, tiếp tục phát
                Mix_ResumeMusic();
            }
        } else {
            // Dừng nhạc nếu cần tắt
            if (Mix_PlayingMusic()) {
                Mix_HaltMusic();
            }
        }
    }
}

void renderText(const string &text, int x, int y, TTF_Font* usedFont = nullptr) {
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
void renderTile(int value, int x, int y, float scale = 1.0f) {
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
        string valueStr = to_string(value);

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
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 50 - i*5);
        renderRoundedRect(renderer, lightRect, 10 - i, SDL_Color{255, 255, 255, 50 - i*5});
    }

    // Tiêu đề menu với màu sắc nổi bật
    TTF_Font* titleFont = TTF_OpenFont("ARLRDBD.TTF", 42); // Font to hơn
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
    SDL_Surface* scoreSurface = TTF_RenderText_Solid(font, ("Score: " + to_string(score)).c_str(), textColor);
    SDL_Texture* scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);
    SDL_Rect scoreRect = {50, 50, scoreSurface->w, scoreSurface->h};
    SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreRect);
    SDL_FreeSurface(scoreSurface);
    SDL_DestroyTexture(scoreTexture);

    // Render điểm cao với màu đã cập nhật
    SDL_Surface* bestSurface = TTF_RenderText_Solid(font, ("Best: " + to_string(bestScore)).c_str(), textColor);
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

pair<int, int> generateUnoccupiedPosition() {
    int line, column;
    do {
        line = rand() % GRID_SIZE;
        column = rand() % GRID_SIZE;
    } while (board[line][column] != 0);
    return {line, column};
}

void addPiece() {
    pair<int, int> pos = generateUnoccupiedPosition();
    board[pos.first][pos.second] = (rand() % 10 == 0) ? 4 : 2;

    // Thêm animation cho block mới
    Animation newTileAnim;
    newTileAnim.value = board[pos.first][pos.second];
    newTileAnim.startX = BOARD_X + pos.second * TILE_SIZE;
    newTileAnim.startY = BOARD_Y + pos.first * TILE_SIZE;
    newTileAnim.targetX = newTileAnim.startX;
    newTileAnim.targetY = newTileAnim.startY;
    newTileAnim.currentX = newTileAnim.startX;
    newTileAnim.currentY = newTileAnim.startY;
    newTileAnim.active = true;
    newTileAnim.isNew = true;
    newTileAnim.isMerging = false;
    newTileAnim.scale = 0.1f;
    newTileAnim.framesLeft = ANIMATION_DURATION;
    animations.push_back(newTileAnim);

    animating = true;
}

void newGame() {
    memset(board, 0, sizeof(board));
    score = 0;
    gameOver = false;
    hasWon = false;
    showMenu = false;
    animating = false;
    animations.clear();
    addPiece();

    // Tắt nhạc nền khi bắt đầu game
    controlBackgroundMusic(false);
}


// Lưu trạng thái trò chơi
void saveGame() {
    ofstream saveFile(SAVE_FILE, ios::binary);
    if (saveFile.is_open()) {
        saveFile.write((char*)board, sizeof(board));
        saveFile.write((char*)&score, sizeof(score));
        saveFile.write((char*)&bestScore, sizeof(bestScore));
        saveFile.close();
    }
}

// Tải trạng thái trò chơi đã lưu
// Cập nhật hàm loadGame để tắt nhạc khi tiếp tục game đã lưu
bool loadGame() {
    ifstream saveFile(SAVE_FILE, ios::binary);
    if (saveFile.is_open()) {
        saveFile.read((char*)board, sizeof(board));
        saveFile.read((char*)&score, sizeof(score));
        saveFile.read((char*)&bestScore, sizeof(bestScore));
        saveFile.close();
        gameOver = false;
        hasWon = false;
        showMenu = false;
        animating = false;
        animations.clear();

        // Tắt nhạc nền khi tiếp tục game
        controlBackgroundMusic(false);

        return true;
    }
    return false;
}

bool canDoMove(int line, int column, int nextLine, int nextColumn) {
    if (nextLine < 0 || nextColumn < 0 || nextLine >= GRID_SIZE || nextColumn >= GRID_SIZE)
        return false;
    return (board[line][column] == board[nextLine][nextColumn] || board[nextLine][nextColumn] == 0);
}

// Kiểm tra xem còn nước đi không
bool movesAvailable() {
    // Kiểm tra xem còn ô trống không
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (board[i][j] == 0) return true;
        }
    }

    // Kiểm tra xem có thể hợp nhất không
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            for (int dir = 0; dir < 4; dir++) {
                int nextI = i + dirLine[dir];
                int nextJ = j + dirColumn[dir];
                if (nextI >= 0 && nextI < GRID_SIZE && nextJ >= 0 && nextJ < GRID_SIZE) {
                    if (board[i][j] == board[nextI][nextJ]) return true;
                }
            }
        }
    }

    return false;
}

void applyMove(int direction) {
    if (animating) return; // Không cho phép di chuyển khi đang animation

    // Lưu trạng thái bảng trước khi di chuyển
    memcpy(previousBoard, board, sizeof(board));

    int startLine = 0, startColumn = 0, lineStep = 1, columnStep = 1;
    if (direction == 0) {
        startLine = GRID_SIZE - 1;
        lineStep = -1;
    }
    if (direction == 1) {
        startColumn = GRID_SIZE - 1;
        columnStep = -1;
    }

    int movePossible, canAddPiece = 0;
    bool hasMerged = false; // Biến cờ kiểm tra xem đã có merge hay chưa

    // Map để theo dõi các ô đã hợp nhất trong lượt này
    bool merged[GRID_SIZE][GRID_SIZE] = {false};

    do {
        movePossible = 0;
        for (int i = startLine; i >= 0 && i < GRID_SIZE; i += lineStep)
            for (int j = startColumn; j >= 0 && j < GRID_SIZE; j += columnStep) {
                if (board[i][j] == 0) continue;

                int nextI = i + dirLine[direction];
                int nextJ = j + dirColumn[direction];

                if (nextI >= 0 && nextI < GRID_SIZE && nextJ >= 0 && nextJ < GRID_SIZE) {
                    if (board[nextI][nextJ] == 0) {
                        // Di chuyển
                        board[nextI][nextJ] = board[i][j];
                        board[i][j] = 0;
                        movePossible = 1;
                        canAddPiece = 1;

                        // Thêm animation di chuyển
                        Animation moveAnim;
                        moveAnim.value = board[nextI][nextJ];
                        moveAnim.startX = BOARD_X + j * TILE_SIZE;
                        moveAnim.startY = BOARD_Y + i * TILE_SIZE;
                        moveAnim.targetX = BOARD_X + nextJ * TILE_SIZE;
                        moveAnim.targetY = BOARD_Y + nextI * TILE_SIZE;
                        moveAnim.currentX = moveAnim.startX;
                        moveAnim.currentY = moveAnim.startY;
                        moveAnim.active = true;
                        moveAnim.isNew = false;
                        moveAnim.isMerging = false;
                        moveAnim.scale = 1.0f;
                        moveAnim.framesLeft = ANIMATION_DURATION;
                        animations.push_back(moveAnim);

                    } else if (board[nextI][nextJ] == board[i][j] && !merged[nextI][nextJ]) {
                        // Hợp nhất
                        board[nextI][nextJ] *= 2;
                        score += board[nextI][nextJ];
                        if (score > bestScore) bestScore = score;
                        board[i][j] = 0;
                        merged[nextI][nextJ] = true;
                        movePossible = 1;
                        canAddPiece = 1;
                        hasMerged = true; // Đã có merge

                        // Thêm animation di chuyển và hợp nhất
                        Animation mergeAnim;
                        mergeAnim.value = board[nextI][nextJ] / 2; // Giá trị ban đầu (trước khi hợp nhất)
                        mergeAnim.startX = BOARD_X + j * TILE_SIZE;
                        mergeAnim.startY = BOARD_Y + i * TILE_SIZE;
                        mergeAnim.targetX = BOARD_X + nextJ * TILE_SIZE;
                        mergeAnim.targetY = BOARD_Y + nextI * TILE_SIZE;
                        mergeAnim.currentX = mergeAnim.startX;
                        mergeAnim.currentY = mergeAnim.startY;
                        mergeAnim.active = true;
                        mergeAnim.isNew = false;
                        mergeAnim.isMerging = true;
                        mergeAnim.scale = 1.0f;
                        mergeAnim.framesLeft = ANIMATION_DURATION;
                        animations.push_back(mergeAnim);
                    }
                }
            }
    } while (movePossible);

    if (canAddPiece) {
        animating = true;

        // Phát âm thanh di chuyển
        if (moveSound) {
            Mix_PlayChannel(-1, moveSound, 0);
        }

        // Phát âm thanh hợp nhất nếu có
        if (hasMerged && mergeSound) {
            Mix_PlayChannel(-1, mergeSound, 0);
        }

        // Tạo ô mới sau khi hoàn thành animation di chuyển
        SDL_Delay(200); // Đợi một chút trước khi thêm ô mới
        addPiece();

        // Kiểm tra chiến thắng (đã đạt 2048)
        bool justWon = false;
        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                if (board[i][j] >= winPoint && !hasWon) {
                    hasWon = true;
                    justWon = true;
                    break;
                }
            }
            if (justWon) break;
        }

        // Phát âm thanh chiến thắng
        if (justWon && winSound) {
            Mix_PlayChannel(-1, winSound, 0);
        }

        // Kiểm tra trò chơi kết thúc
        if (!movesAvailable() && !gameOver) {
            gameOver = true;
            // Phát âm thanh thua cuộc
            if (loseSound) {
                Mix_PlayChannel(-1, loseSound, 0);
            }
        }
    }
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

// Cập nhật gameLoop để xử lý nhạc nền khi chuyển đổi giữa menu và game
void gameLoop() {
    bool running = true;
    SDL_Event e;

    // Phát nhạc nền khi ở menu ngay từ đầu
    if (showMenu) {
        controlBackgroundMusic(true);
    }

    while (running) {
        Uint32 frameStart = SDL_GetTicks();

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                saveGame(); // Lưu game khi thoát
                running = false;
            }
            if (e.type == SDL_MOUSEMOTION) {
                int mouseX = e.motion.x;
                int mouseY = e.motion.y;

                // Kiểm tra di chuột trên các nút menu
                if (showMenu) {
                    newGameButton.highlighted = (mouseX >= newGameButton.rect.x && mouseX <= newGameButton.rect.x + newGameButton.rect.w &&
                                               mouseY >= newGameButton.rect.y && mouseY <= newGameButton.rect.y + newGameButton.rect.h);
                    continueButton.highlighted = (mouseX >= continueButton.rect.x && mouseX <= continueButton.rect.x + continueButton.rect.w &&
                                                mouseY >= continueButton.rect.y && mouseY <= continueButton.rect.y + continueButton.rect.h);
                    exitButton.highlighted = (mouseX >= exitButton.rect.x && mouseX <= exitButton.rect.x + exitButton.rect.w &&
                                            mouseY >= exitButton.rect.y && mouseY <= exitButton.rect.y + exitButton.rect.h);
                } else {
                    // Kiểm tra nút Menu trong game
                    SDL_Rect menuButtonRect = {SCREEN_WIDTH - 100, 50, 80, 30};
                    menuButtonHighlighted = (mouseX >= menuButtonRect.x && mouseX <= menuButtonRect.x + menuButtonRect.w &&
                                           mouseY >= menuButtonRect.y && mouseY <= menuButtonRect.y + menuButtonRect.h);
                }
            }
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                int mouseX = e.button.x;
                int mouseY = e.button.y;

                if (showMenu) {
                    processMenuClick(mouseX, mouseY);
                } else {
                    // Kiểm tra nút Menu trong game
                    SDL_Rect menuButtonRect = {SCREEN_WIDTH - 100, 50, 80, 30};
                    if (mouseX >= menuButtonRect.x && mouseX <= menuButtonRect.x + menuButtonRect.w &&
                        mouseY >= menuButtonRect.y && mouseY <= menuButtonRect.y + menuButtonRect.h) {
                        showMenu = true;
                        // Bật nhạc nền khi quay lại menu
                        controlBackgroundMusic(true);
                    }
                }
            }
            if (e.type == SDL_KEYDOWN && !showMenu && !animating) {
                switch (e.key.keysym.sym) {
                    case SDLK_UP: applyMove(2); break;
                    case SDLK_DOWN: applyMove(0); break;
                    case SDLK_LEFT: applyMove(3); break;
                    case SDLK_RIGHT: applyMove(1); break;
                    case SDLK_n:
                        newGame();
                        gameOver = false; // Đảm bảo flag được reset
                        hasWon = false;   // Đảm bảo flag được reset
                        break;
                    case SDLK_ESCAPE:
                        showMenu = true;
                        // Bật nhạc nền khi nhấn ESC để quay lại menu
                        controlBackgroundMusic(true);
                        break;
                }
            }
        }

        // Cập nhật animation nếu đang có
        if (animating) {
            updateAnimations();
        }

        render();

        // Điều chỉnh FPS
        Uint32 frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < FRAME_DELAY) {
            SDL_Delay(FRAME_DELAY - frameTime);
        }
    }
}
void initSDL() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    TTF_Init();
    window = SDL_CreateWindow("2048 Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Sử dụng font đậm với kích thước lớn hơn
    font = TTF_OpenFont("ARLRDBD.TTF", 24);
    menuFont = TTF_OpenFont("ARLRDBD.TTF", 28);
    if (!font || !menuFont) {
        cout << "Failed to load font: " << TTF_GetError() << endl;
        // Thử dùng font thường nếu không có font đậm
        font = TTF_OpenFont("arial.ttf", 24);
        menuFont = TTF_OpenFont("arial.ttf", 28);
    }

    loadSounds();
}

void closeSDL() {
    closeSounds();
    TTF_CloseFont(font);
    TTF_CloseFont(menuFont);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}

int main() {
    srand(time(0));
    initSDL();
    memset(board, 0, sizeof(board));
    memset(previousBoard, 0, sizeof(previousBoard));
    addPiece();
    gameLoop();
    closeSDL();
    return 0;
}
