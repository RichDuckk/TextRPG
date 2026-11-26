// Здесь я подключаю стандартные библиотеки для файлов сохранения, текста,
// поиска предметов и хранения сцен с вариантами выбора.
#include <iostream>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <map>
#include <sstream>
#include <string>
#include <vector>

// Я задаю единую ширину всех экранов и оставляю терминалу запас до правого края.
constexpr std::size_t screenWidth = 108;

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
    std::string ignoredLine;
    std::getline(std::cin, ignoredLine);
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

// Я считаю экранные колонки, а не байты UTF-8: знак ударения не занимает отдельную колонку.
bool isCombiningMark(unsigned int codePoint) {
    return (codePoint >= 0x0300 && codePoint <= 0x036F)
        || (codePoint >= 0x1AB0 && codePoint <= 0x1AFF)
        || (codePoint >= 0x1DC0 && codePoint <= 0x1DFF)
        || (codePoint >= 0x20D0 && codePoint <= 0x20FF)
        || (codePoint >= 0xFE20 && codePoint <= 0xFE2F);
}

unsigned int readUtf8CodePoint(const std::string& text, std::size_t position,
                               std::size_t& byteCount) {
    const unsigned char first = static_cast<unsigned char>(text[position]);
    if ((first & 0x80) == 0) { byteCount = 1; return first; }
    if ((first & 0xE0) == 0xC0 && position + 1 < text.size()) {
        byteCount = 2;
        return ((first & 0x1F) << 6)
             | (static_cast<unsigned char>(text[position + 1]) & 0x3F);
    }
    if ((first & 0xF0) == 0xE0 && position + 2 < text.size()) {
        byteCount = 3;
        return ((first & 0x0F) << 12)
             | ((static_cast<unsigned char>(text[position + 1]) & 0x3F) << 6)
             | (static_cast<unsigned char>(text[position + 2]) & 0x3F);
    }
    if ((first & 0xF8) == 0xF0 && position + 3 < text.size()) {
        byteCount = 4;
        return ((first & 0x07) << 18)
             | ((static_cast<unsigned char>(text[position + 1]) & 0x3F) << 12)
             | ((static_cast<unsigned char>(text[position + 2]) & 0x3F) << 6)
             | (static_cast<unsigned char>(text[position + 3]) & 0x3F);
    }
    byteCount = 1;
    return first;
}

std::size_t utf8Length(const std::string& text) {
    std::size_t count = 0;
    for (std::size_t position = 0; position < text.size();) {
        std::size_t byteCount = 1;
        const unsigned int codePoint = readUtf8CodePoint(text, position, byteCount);
        if (!isCombiningMark(codePoint)) ++count;
        position += byteCount;
    }
    return count;
}

std::string takeUtf8Characters(const std::string& text, std::size_t count) {
    std::size_t position = 0;
    std::size_t cells = 0;
    while (position < text.size()) {
        std::size_t byteCount = 1;
        const unsigned int codePoint = readUtf8CodePoint(text, position, byteCount);
        if (!isCombiningMark(codePoint) && cells >= count) break;
        if (!isCombiningMark(codePoint)) ++cells;
        position += byteCount;
    }
    return text.substr(0, position);
}

std::string fitText(const std::string& text, std::size_t width) {
    const std::size_t length = utf8Length(text);
    if (length >= width) return takeUtf8Characters(text, width);
    return text + std::string(width - length, ' ');
}

// Здесь я переношу длинные фразы по словам, чтобы они не ломали рамки интерфейса.
std::vector<std::string> wrapLine(const std::string& line, std::size_t width) {
    std::istringstream words(line);
    std::vector<std::string> result;
    std::string word;
    std::string current;
    while (words >> word) {
        if (utf8Length(word) > width) {
            if (!current.empty()) {
                result.push_back(current);
                current.clear();
            }
            while (utf8Length(word) > width) {
                result.push_back(takeUtf8Characters(word, width));
                word = word.substr(takeUtf8Characters(word, width).size());
            }
        }
        if (current.empty()) current = word;
        else if (utf8Length(current) + 1 + utf8Length(word) <= width) current += " " + word;
        else {
            result.push_back(current);
            current = word;
        }
    }
    if (!current.empty() || result.empty()) result.push_back(current);
    return result;
}

void printBoxTop(const std::string& title, std::size_t width = screenWidth) {
    const std::string centered = " " + title + " ";
    const std::size_t titleWidth = utf8Length(centered);
    const std::size_t left = (width - titleWidth) / 2;
    const std::size_t right = width - titleWidth - left;
    std::string leftBorder, rightBorder;
    for (std::size_t i = 0; i < left; ++i) leftBorder += "─";
    for (std::size_t i = 0; i < right; ++i) rightBorder += "─";
    std::cout << "┌" << leftBorder << centered << rightBorder << "┐\n";
}

void printBoxLine(const std::string& text, std::size_t width = screenWidth) {
    std::cout << "│" << fitText(text, width) << "│\n";
}

void printBoxBottom(std::size_t width = screenWidth) {
    std::string border;
    for (std::size_t i = 0; i < width; ++i) border += "─";
    std::cout << "└" << border << "┘\n";
}

std::string horizontalBorder(std::size_t width) {
    std::string border;
    for (std::size_t i = 0; i < width; ++i) border += "─";
    return border;
}

void showSimpleScreen(const std::string& title, const std::vector<std::string>& lines) {
    clearScreen();
    printBoxTop(title);
    for (const std::string& line : lines) {
        for (const std::string& wrapped : wrapLine(line, screenWidth)) printBoxLine(wrapped);
    }
    printBoxBottom();
}

std::string showParameters(Player& player, const std::string& currentSceneId,
                           bool& loadedSave, bool& shouldQuit) {
    loadedSave = false;
    shouldQuit = false;
    showSimpleScreen("ПАРАМЕТРЫ", {
        "1. Вернуться к игре",
        "2. Сохранить игру",
        "3. Загрузить игру",
        "4. Выйти из игры"
    });
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
    std::vector<std::string> lines{
        "ХАРАКТЕРИСТИКИ",
        "Сила: " + std::to_string(player.strength),
        "Стужа: " + std::to_string(player.frostMagic),
        "Ловкость: " + std::to_string(player.dexterity),
        "",
        "ПРЕДМЕТЫ"
    };
    if (player.inventory.empty()) lines.push_back("Пока пусто.");
    for (const auto& item : player.inventory) {
        lines.push_back(item.first + ": " + std::to_string(item.second));
    }
    showSimpleScreen("ИНВЕНТАРЬ", lines);
    waitForEnter();
}

void clearScreen() { std::cout << "\033[2J\033[H" << std::flush; }

int readNumber(int minValue, int maxValue, const std::string& prompt) {
    while (true) {
        std::cout << prompt;
        std::string input;
        if (!std::getline(std::cin, input)) return -1;

        // Я разбираю целую строку, чтобы после числового выбора не оставался Enter в потоке ввода.
        std::istringstream line(input);
        int number = 0;
        char extraCharacter = '\0';
        if (line >> number && !(line >> extraCharacter)) {
            if (number >= minValue && number <= maxValue) return number;
            std::cout << "Введи число от " << minValue << " до " << maxValue << ".\n";
        } else {
            std::cout << "Введи номер цифрой.\n";
        }
    }
}

int readChoice(const Scene& scene, const Player& player) {
    while (true) {
        const std::string prompt = scene.choices.size() == 1
                                       ? "Твой выбор (Enter — выбрать вариант 1): "
                                       : "Твой выбор: ";
        std::cout << prompt;
        std::string input;
        if (!std::getline(std::cin, input)) return -1;

        const bool isBlank = input.find_first_not_of(" \t\r") == std::string::npos;
        int choice = 0;
        if (isBlank && scene.choices.size() == 1) {
            choice = 1;
        } else {
            std::istringstream line(input);
            char extraCharacter = '\0';
            if (isBlank || !(line >> choice) || (line >> extraCharacter)) {
                std::cout << "Введи номер варианта из списка.\n";
                continue;
            }
        }

        if (choice < 0 || choice > 9) {
            std::cout << "Введи номер варианта из списка.\n";
            continue;
        }
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
        std::vector<std::string> lines{
            "Сила: " + std::to_string(player.strength),
            "Стужа: " + std::to_string(player.frostMagic),
            "Ловкость: " + std::to_string(player.dexterity),
            "Свободные очки: " + std::to_string(player.freePoints),
            ""
        };
        if (player.freePoints == 0) {
            lines.insert(lines.end(), {"1. Начать игру", "2. Загрузить сохранение", "3. Назад"});
            showSimpleScreen("СОЗДАНИЕ ГЕРОЯ", lines);
            const int choice = readNumber(1, 3, "Твой выбор: ");
            if (choice < 0 || choice == 3) return false;
            if (choice == 1) { firstSceneId = "narration_63"; return true; }
            if (loadGame(player, firstSceneId)) return true;
            std::cout << "\nНе удалось загрузить сохранение.\n";
            waitForEnter();
            continue;
        }
        lines.insert(lines.end(), {
            "1. Добавить 1 к Силе",
            "2. Добавить 1 к Стуже",
            "3. Добавить 1 к Ловкости",
            "4. Загрузить сохранение",
            "5. Назад"
        });
        showSimpleScreen("СОЗДАНИЕ ГЕРОЯ", lines);
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

struct ScreenLine {
    std::string text;
    bool isNarrative = false;
};

void appendWrappedText(std::vector<ScreenLine>& output, const std::string& text,
                       std::size_t width, bool isNarrative) {
    std::istringstream sourceLines(text);
    std::string sourceLine;
    bool hasPreviousContent = false;
    bool previousWasSpeaker = false;
    while (std::getline(sourceLines, sourceLine)) {
        if (sourceLine.find_first_not_of(" \t\r") == std::string::npos) {
            // Я схлопываю пустые строки и решаю по соседним типам текста, нужен ли отступ.
            continue;
        }
        bool currentIsSpeaker = false;
        for (const SpeakerStyle& style : speakerStyles) {
            if (sourceLine.rfind(style.name, 0) == 0) currentIsSpeaker = true;
        }
        // Я отделяю повествование пустым рядом, а последовательные реплики оставляю вместе.
        if (isNarrative && hasPreviousContent && (!previousWasSpeaker || !currentIsSpeaker)
            && (output.empty() || !output.back().text.empty())) {
            // Я разделяю повествовательные абзацы и переходы к репликам одним пустым рядом.
            output.push_back({"", isNarrative});
        }
        for (const std::string& wrapped : wrapLine(sourceLine, width)) {
            output.push_back({wrapped, isNarrative});
        }
        hasPreviousContent = true;
        previousWasSpeaker = currentIsSpeaker;
    }
    // Я убираю пустые строки в конце сцены, чтобы перед вариантами не появлялся лишний зазор.
    while (!output.empty() && output.back().text.empty()) output.pop_back();
}

std::string inventoryLabel(const std::string& itemName) {
    if (itemName == "Обычная стрела") return "Стрела";
    return itemName;
}

// Здесь я собираю экран из двух колонок и нижней полосы предметов, как на эскизе.
void showSceneScreen(const Scene& scene, const Player& player, const std::string& narrative) {
    // Я оставляю запас по ширине, чтобы терминал не переносил правую стенку рамки.
    constexpr std::size_t leftWidth = 32;
    constexpr std::size_t rightWidth = screenWidth - leftWidth - 1;
    constexpr std::size_t narrativeWidth = rightWidth - 2;
    std::vector<ScreenLine> rightLines;
    appendWrappedText(rightLines, narrative, narrativeWidth, true);

    if (!scene.choices.empty()) {
        rightLines.push_back({"", false});
        for (int i = 0; i < static_cast<int>(scene.choices.size()); ++i) {
            const Choice& choice = scene.choices[i];
            std::string label = std::to_string(i + 1) + ". " + choice.text;
            if (!choice.requirements.empty()) {
                label += meetsRequirements(player, choice.requirements)
                             ? " [требуется: "
                             : " [недоступно — требуется: ";
                for (int j = 0; j < static_cast<int>(choice.requirements.size()); ++j) {
                    if (j > 0) label += ", ";
                    label += choice.requirements[j].name;
                    label += choice.requirements[j].type == RequirementType::stat ? " ≥ " : " × ";
                    label += std::to_string(choice.requirements[j].amount);
                }
                label += "]";
            }
            appendWrappedText(rightLines, label, narrativeWidth, false);
        }
    }

    std::vector<std::string> statusLines{
        "Сила: " + std::to_string(player.strength),
        "Стужа: " + std::to_string(player.frostMagic),
        "Ловкость: " + std::to_string(player.dexterity)
    };
    const std::size_t contentHeight = std::max(statusLines.size(), rightLines.size());

    clearScreen();
    printBoxTop("ЛЕДЯНОЙ ТРОН", screenWidth);
    std::cout << "├" << horizontalBorder(leftWidth) << "┬"
              << horizontalBorder(rightWidth) << "┤\n";
    std::cout << "│" << fitText("ХАРАКТЕРИСТИКИ", leftWidth) << "│"
              << fitText("ИСТОРИЯ И ВЫБОР", rightWidth) << "│\n";
    std::cout << "├" << horizontalBorder(leftWidth) << "┼"
              << horizontalBorder(rightWidth) << "┤\n";

    for (std::size_t row = 0; row < contentHeight; ++row) {
        const std::string left = row < statusLines.size() ? statusLines[row] : "";
        std::cout << "│" << fitText(left, leftWidth) << "│";
        if (row < rightLines.size() && rightLines[row].isNarrative) {
            printNarrativeText(rightLines[row].text);
            std::cout << std::string(rightWidth - utf8Length(rightLines[row].text), ' ');
        } else if (row < rightLines.size()) {
            std::cout << fitText(rightLines[row].text, rightWidth);
        } else {
            std::cout << std::string(rightWidth, ' ');
        }
        std::cout << "│\n";
    }

    std::cout << "├" << horizontalBorder(screenWidth) << "┤\n";
    constexpr std::size_t inventoryWidth = 64;
    constexpr std::size_t actionsWidth = screenWidth - inventoryWidth - 1;
    std::cout << "│" << fitText("ИНВЕНТАРЬ", inventoryWidth) << "│"
              << fitText("ДЕЙСТВИЯ", actionsWidth) << "│\n";
    std::cout << "├" << horizontalBorder(inventoryWidth) << "┼"
              << horizontalBorder(actionsWidth) << "┤\n";
    std::vector<std::string> slots;
    for (const auto& item : player.inventory) {
        if (item.second > 0) {
            slots.push_back("[" + inventoryLabel(item.first) + " ×" + std::to_string(item.second) + "]");
        }
    }
    while (slots.size() < 6) slots.push_back("[пусто]");
    for (std::size_t row = 0; row < 2; ++row) {
        std::string slotRow;
        for (std::size_t column = 0; column < 3; ++column) {
            if (column > 0) slotRow += "  ";
            slotRow += fitText(slots[row * 3 + column], 20);
        }
        const std::string action = row == 0 ? "[0] Инвентарь" : "[9] Параметры";
        std::cout << "│" << fitText(slotRow, inventoryWidth) << "│"
                  << fitText(action, actionsWidth) << "│\n";
    }
    printBoxBottom(screenWidth);
}

bool playScene(const std::string& firstSceneId, Player& player) {
    const Scene* current = findScene(firstSceneId);
    bool sceneRewardApplied = false;
    while (current != nullptr) {
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
        showSceneScreen(*current, player, text);
        if (current->choices.empty()) break;
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
        showSimpleScreen("ЛЕДЯНОЙ ТРОН", {"1. Начать игру", "2. Выход"});
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
