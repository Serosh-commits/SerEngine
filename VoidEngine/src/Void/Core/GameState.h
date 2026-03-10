#ifndef GAME_STATE_H
#define GAME_STATE_H

namespace Void {

    enum class GameState {
        MainMenu,
        Playing,
        Paused,
        GameOver
    };

    struct GameData {
        GameState State = GameState::Playing;
        float Health = 100.0f;
        float MaxHealth = 100.0f;
        float Sanity = 100.0f;
        float MaxSanity = 100.0f;
        float PlayTime = 0.0f;
        int ItemsCollected = 0;
        int TotalItems = 5;
        bool FlashlightBattery = true;
        float BatteryLife = 60.0f;
        float MaxBatteryLife = 60.0f;
    };

}

#endif
