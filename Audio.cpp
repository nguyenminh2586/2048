#include "Audio.h"
#include <iostream>

// Âm thanh
Mix_Chunk* moveSound = nullptr;
Mix_Chunk* mergeSound = nullptr;
Mix_Chunk* winSound = nullptr;
Mix_Chunk* loseSound = nullptr;
Mix_Music* backgroundMusic = nullptr;

bool loadSounds() {
    // Khởi tạo SDL_mixer với tham số phù hợp
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cout << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << std::endl;
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
        std::cout << "Warning: Unable to load sound effects! SDL_mixer Error: " << Mix_GetError() << std::endl;
        std::cout << "Game will continue without sound effects." << std::endl;
    }

    if (!backgroundMusic) {
        std::cout << "Warning: Unable to load background music! SDL_mixer Error: " << Mix_GetError() << std::endl;
        std::cout << "Game will continue without background music." << std::endl;
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

// Quản lý nhạc nền
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
