#include <iostream>
#include <limits>

void showIntroduction() {
    std::cout << "\nСнежный север\n"
              << "Ты возвращаешься в сознание среди пепла.\n"
              << "Деревня, где прошли твои детство и юность, уничтожена.\n"
              << "Впереди — путь к правде и мести.\n\n"
              << "Нажми Enter, чтобы вернуться в меню...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

int main() {
    bool isRunning = true;

    while (isRunning) {
        std::cout << "=== СНЕЖНЫЙ СЕВЕР ===\n"
                  << "1. Начать игру\n"
                  << "2. Выход\n"
                  << "Твой выбор: ";

        int choice = 0;
        if (!(std::cin >> choice)) {
            std::cout << "Пожалуйста, введи номер пункта.\n\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        if (choice == 1) {
            showIntroduction();
        } else if (choice == 2) {
            isRunning = false;
        } else {
            std::cout << "Такого пункта нет. Выбери 1 или 2.\n\n";
        }
    }

    std::cout << "До встречи!\n";
    return 0;
}
