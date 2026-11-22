#include "locker.h"

int main() {
    ScoreBoard board("leader/scoreBoard.txt");               // owned by main
    GameEngine* engine = GameEngine::getInstance(&board);    // Singleton, board is only associated

    engine->showTitle();

    while (engine->isRunning()) {
        try {
            engine->step();
        }
        catch (const GameException& e) {
            cout << "\n[!] " << e.what() << "\n";
            engine->onError(e);          // applies the credibility and turn penalty if any
        }
        catch (...) {
            cout << "\n[!] Unexpected error. Shutting down.\n";
            engine->stop();
        }
    }

    GameEngine::destroyInstance();
    cout << "\nGoodbye, Detective.\n";

    system("pause");
    return 0;
}