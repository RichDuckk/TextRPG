// Здесь я подключаю стандартные библиотеки для файлов сохранения, текста,
// поиска предметов и хранения сцен с вариантами выбора.
#include <iostream>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <limits>
#include <map>
#include <string>
#include <vector>

// Здесь я храню состояние героя. Все три характеристики начинаются с пяти.
struct Player {
    int strength = 5;
    int frostMagic = 5;
    int dexterity = 5;
    int freePoints = 3;
    std::map<std::string, int> inventory{{"Обычная стрела", 2}};
};

enum class RequirementType { stat, item };
struct Requirement { RequirementType type; std::string name; int amount; };
struct Reward { std::string item; int amount; };
struct StatReward { std::string stat; int amount; };
struct Choice {
    std::string text;
    std::string nextSceneId;
    std::vector<Requirement> requirements;
    std::vector<Requirement> costs;
};
struct Scene {
    std::string id;
    std::vector<Choice> choices;
    std::vector<Reward> rewards;
    std::vector<StatReward> statRewards;
};
struct SpeakerStyle { std::string name; std::string color; };

// Здесь я выбираю отдельный цвет для каждой подписи говорящего.
const std::vector<SpeakerStyle> speakerStyles{
    {"Вальфрост:", "\033[1;34m"}, // Синий: голос главного героя.
    {"Утер:", "\033[1;33m"},      // Тёплый жёлтый: спокойный голос деда.
    {"Пе́р:", "\033[1;38;5;208m"}, // Оранжевый: углежог и его лагерь.
    {"Пер:", "\033[1;38;5;208m"},  // Здесь я поддерживаю цвет, даже если в тексте нет ударения.
    {"Гордон:", "\033[1;36m"},    // Голубой: дружелюбный Гордон.
    {"Чак:", "\033[1;32m"},        // Зелёный: приветливый страж у ворот.
    {"Углежог:", "\033[1;38;5;130m"} // Охристый: остальные жители лагеря.
};

#include "scene_data.hpp"

void clearScreen();
const Scene* findScene(const std::string& sceneId);
int readNumber(int minValue, int maxValue, const std::string& prompt);

void waitForEnter() {
    std::cout << "\nНажми Enter, чтобы продолжить...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
    clearScreen();
}

int getStat(const Player& player, const std::string& name) {
    if (name == "Сила") return player.strength;
    if (name == "Стужа") return player.frostMagic;
    if (name == "Ловкость") return player.dexterity;
    return 0;
}

int getItemCount(const Player& player, const std::string& item) {
    const auto found = player.inventory.find(item);
    return found == player.inventory.end() ? 0 : found->second;
}

bool meetsRequirements(const Player& player, const std::vector<Requirement>& requirements) {
    for (const Requirement& requirement : requirements) {
        const int current = requirement.type == RequirementType::stat
                                ? getStat(player, requirement.name)
                                : getItemCount(player, requirement.name);
        if (current < requirement.amount) return false;
    }
    return true;
}

void printRequirement(const Requirement& requirement) {
    std::cout << requirement.name
              << (requirement.type == RequirementType::stat ? " ≥ " : " × ")
              << requirement.amount;
}

void applyCosts(Player& player, const Choice& choice) {
    for (const Requirement& cost : choice.costs) {
        if (cost.type == RequirementType::item) player.inventory[cost.name] -= cost.amount;
    }
}

bool saveGame(const Player& player, const std::string& sceneId) {
    std::ofstream saveFile("savegame.txt");
    if (!saveFile) return false;
    saveFile << "ICE_THRONE_SAVE_V2\n" << std::quoted(sceneId) << '\n'
             << player.strength << ' ' << player.frostMagic << ' '
             << player.dexterity << ' ' << player.freePoints << '\n'
             << player.inventory.size() << '\n';
    for (const auto& item : player.inventory) {
        saveFile << std::quoted(item.first) << ' ' << item.second << '\n';
    }
    return static_cast<bool>(saveFile);
}

// Я сначала проверяю все поля сохранения, и только потом заменяю состояние героя.
bool loadGame(Player& player, std::string& sceneId) {
    std::ifstream saveFile("savegame.txt");
    std::string version, savedSceneId;
    int strength = 0, frost = 0, dexterity = 0, freePoints = 0, itemCount = 0;
    if (!(saveFile >> version) || version != "ICE_THRONE_SAVE_V2"
        || !(saveFile >> std::quoted(savedSceneId) >> strength >> frost
             >> dexterity >> freePoints >> itemCount)) return false;
    if (strength < 0 || frost < 0 || dexterity < 0 || freePoints < 0
        || itemCount < 0 || itemCount > 1000 || findScene(savedSceneId) == nullptr) return false;

    std::map<std::string, int> inventory;
    for (int i = 0; i < itemCount; ++i) {
        std::string name;
        int amount = 0;
        if (!(saveFile >> std::quoted(name) >> amount) || amount < 0) return false;
        inventory[name] = amount;
    }
    player.strength = strength;
    player.frostMagic = frost;
    player.dexterity = dexterity;
    player.freePoints = freePoints;
    player.inventory = inventory;
    sceneId = savedSceneId;
    return true;
}

std::string showParameters(Player& player, const std::string& currentSceneId,
                           bool& loadedSave, bool& shouldQuit) {
    loadedSave = false;
    shouldQuit = false;
    clearScreen();
    std::cout << "=== ПАРАМЕТРЫ ===\n\n"
              << "1. Вернуться к игре\n"
              << "2. Сохранить игру\n"
              << "3. Загрузить игру\n"
              << "4. Выйти из игры\n";
    const int choice = readNumber(1, 4, "Твой выбор: ");
    if (choice == -1 || choice == 1) return currentSceneId;
    if (choice == 2) {
        std::cout << (saveGame(player, currentSceneId)
                          ? "\nИгра сохранена.\n" : "\nНе удалось сохранить игру.\n");
        waitForEnter();
        return currentSceneId;
    }
    if (choice == 4) { shouldQuit = true; return currentSceneId; }
    std::string savedSceneId;
    if (loadGame(player, savedSceneId)) {
        loadedSave = true;
        return savedSceneId;
    }
    std::cout << "\nНе удалось загрузить сохранение.\n";
    waitForEnter();
    return currentSceneId;
}

// Инвентарь показывает и вещи героя, и его характеристики.
void showInventory(const Player& player) {
    clearScreen();
    std::cout << "=== ИНВЕНТАРЬ ===\n\nХАРАКТЕРИСТИКИ\n"
              << "Сила: " << player.strength << '\n'
              << "Стужа: " << player.frostMagic << '\n'
              << "Ловкость: " << player.dexterity << "\n\nПРЕДМЕТЫ\n";
    if (player.inventory.empty()) std::cout << "Пока пусто.\n";
    for (const auto& item : player.inventory) std::cout << item.first << ": " << item.second << '\n';
    waitForEnter();
}

void clearScreen() { std::cout << "\033[2J\033[H" << std::flush; }

int readNumber(int minValue, int maxValue, const std::string& prompt) {
    while (true) {
        std::cout << prompt;
        int number = 0;
        if (std::cin >> number) {
            if (number >= minValue && number <= maxValue) return number;
            std::cout << "Введи число от " << minValue << " до " << maxValue << ".\n";
        } else if (std::cin.eof()) {
            return -1;
        } else {
            std::cout << "Введи номер цифрой.\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
}

int readChoice(const Scene& scene, const Player& player) {
    while (true) {
        const int choice = readNumber(0, 9, "Твой выбор: ");
        if (choice < 0) return -1;
        if (choice == 0) return -2;
        if (choice == 9) return -3;
        if (choice > static_cast<int>(scene.choices.size())) {
            std::cout << "Такого варианта нет. Выбери пункт из списка.\n";
            continue;
        }
        const int index = choice - 1;
        const Choice& selected = scene.choices[index];
        if (meetsRequirements(player, selected.requirements)) return index;
        std::cout << "Не хватает: ";
        bool first = true;
        for (const Requirement& requirement : selected.requirements) {
            if (!meetsRequirements(player, {requirement})) {
                if (!first) std::cout << ", ";
                printRequirement(requirement);
                first = false;
            }
        }
        std::cout << ". Попробуй другой вариант.\n";
    }
}

bool configurePlayer(Player& player, std::string& firstSceneId) {
    while (true) {
        clearScreen();
        std::cout << "=== СОЗДАНИЕ ГЕРОЯ ===\n"
                  << "Сила: " << player.strength << '\n'
                  << "Стужа: " << player.frostMagic << '\n'
                  << "Ловкость: " << player.dexterity << '\n'
                  << "Свободные очки: " << player.freePoints << "\n\n";
        if (player.freePoints == 0) {
            std::cout << "1. Начать игру\n2. Загрузить сохранение\n3. Назад\n";
            const int choice = readNumber(1, 3, "Твой выбор: ");
            if (choice < 0 || choice == 3) return false;
            if (choice == 1) { firstSceneId = "narration_63"; return true; }
            if (loadGame(player, firstSceneId)) return true;
            std::cout << "\nНе удалось загрузить сохранение.\n";
            waitForEnter();
            continue;
        }
        std::cout << "1. Добавить 1 к Силе\n2. Добавить 1 к Стуже\n"
                  << "3. Добавить 1 к Ловкости\n4. Загрузить сохранение\n5. Назад\n";
        const int choice = readNumber(1, 5, "Твой выбор: ");
        if (choice < 0 || choice == 5) return false;
        if (choice == 4) {
            if (loadGame(player, firstSceneId)) return true;
            std::cout << "\nНе удалось загрузить сохранение.\n";
            waitForEnter();
            continue;
        }
        if (choice == 1) ++player.strength;
        else if (choice == 2) ++player.frostMagic;
        else ++player.dexterity;
        --player.freePoints;
    }
}

const Scene* findScene(const std::string& sceneId) {
    for (const Scene& scene : scenes) if (scene.id == sceneId) return &scene;
    return nullptr;
}

void printNarrativeText(const std::string& text) {
    const std::string italic = "\033[3m", reset = "\033[0m";
    std::size_t cursor = 0;
    std::cout << italic;
    while (cursor < text.size()) {
        std::size_t next = std::string::npos;
        const SpeakerStyle* speaker = nullptr;
        for (const SpeakerStyle& style : speakerStyles) {
            const std::size_t position = text.find(style.name, cursor);
            if (position != std::string::npos && position < next) { next = position; speaker = &style; }
        }
        if (speaker == nullptr) { std::cout << text.substr(cursor); break; }
        std::cout << text.substr(cursor, next - cursor) << reset << speaker->color
                  << speaker->name << reset << italic;
        cursor = next + speaker->name.size();
    }
    std::cout << reset;
}

std::string loadSceneText(const std::string& sceneId) {
    const std::string filePath = "data/scenes/" + sceneId + ".txt";
    std::ifstream sceneFile(filePath);
    if (!sceneFile) { std::cerr << "Не удалось открыть файл сцены: " << filePath << '\n'; return ""; }
    return std::string(std::istreambuf_iterator<char>(sceneFile), std::istreambuf_iterator<char>());
}

bool playScene(const std::string& firstSceneId, Player& player) {
    const Scene* current = findScene(firstSceneId);
    bool sceneRewardApplied = false;
    while (current != nullptr) {
        clearScreen();
        if (!sceneRewardApplied) {
            for (const Reward& reward : current->rewards) player.inventory[reward.item] += reward.amount;
            for (const StatReward& reward : current->statRewards) {
                if (reward.stat == "Стужа") player.frostMagic += reward.amount;
                else if (reward.stat == "Сила") player.strength += reward.amount;
                else if (reward.stat == "Ловкость") player.dexterity += reward.amount;
            }
        }
        // Открытие инвентаря повторно выводит ту же сцену, поэтому её награды не начисляются снова.
        sceneRewardApplied = true;
        const std::string text = loadSceneText(current->id);
        if (text.empty()) { waitForEnter(); return false; }
        std::cout << '\n';
        printNarrativeText(text);
        std::cout << "\n\n";
        if (current->choices.empty()) break;

        std::cout << "0. Инвентарь\n";
        for (int i = 0; i < static_cast<int>(current->choices.size()); ++i) {
            const Choice& choice = current->choices[i];
            std::cout << i + 1 << ". " << choice.text;
            if (!choice.requirements.empty()) {
                std::cout << " [требуется: ";
                for (int j = 0; j < static_cast<int>(choice.requirements.size()); ++j) {
                    if (j > 0) std::cout << ", ";
                    printRequirement(choice.requirements[j]);
                }
                std::cout << ']';
            }
            std::cout << '\n';
        }
        std::cout << "9. Параметры\n";
        const int selection = readChoice(*current, player);
        if (selection == -1) return false;
        if (selection == -2) { showInventory(player); continue; }
        if (selection == -3) {
            bool loaded = false, quit = false;
            const std::string next = showParameters(player, current->id, loaded, quit);
            if (quit) return true;
            current = findScene(next);
            sceneRewardApplied = loaded;
            continue;
        }
        const Choice& choice = current->choices[selection];
        applyCosts(player, choice);
        current = findScene(choice.nextSceneId);
        sceneRewardApplied = false;
    }
    waitForEnter();
    return false;
}

int main() {
    bool running = true;
    Player player;
    while (running) {
        clearScreen();
        std::cout << "=== ЛЕДЯНОЙ ТРОН ===\n1. Начать игру\n2. Выход\n";
        const int choice = readNumber(1, 2, "Твой выбор: ");
        if (choice < 0) break;
        if (choice == 1) {
            player = Player{};
            std::string firstSceneId;
            if (configurePlayer(player, firstSceneId) && playScene(firstSceneId, player)) running = false;
        } else running = false;
    }
    std::cout << "До встречи!\n";
    return 0;
}
