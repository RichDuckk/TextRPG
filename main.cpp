// Здесь я подключаю стандартные библиотеки для ввода/вывода, текста,
// списка предметов и сцен, а также для очистки неверного ввода.
#include <iostream>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <limits>
#include <map>
#include <string>
#include <vector>

// Здесь я храню состояние героя: характеристики и количество каждого предмета.
struct Player {
    int frostMagic = 3;
    int strength = 4;
    int freePoints = 3;
    // В этой map я связываю название предмета с его количеством.
    // В паре map поле first — название предмета, а second — его количество.
    std::map<std::string, int> inventory{{"Обычная стрела", 2}};
};

// Этим значением я различаю требования к характеристике и к предмету.
enum class RequirementType {
    stat,
    item
};

// Каждое требование хранит название характеристики или предмета и нужное число.
struct Requirement {
    RequirementType type;
    std::string name;
    int amount;
};

// Наградой я добавляю предметы в инвентарь при входе в сцену.
struct Reward {
    std::string item;
    int amount;
};

// Наградой я могу увеличить характеристику, например Стужу после изучения льда.
struct StatReward {
    std::string stat;
    int amount;
};

// У каждого действия есть текст, следующая сцена, требования и затраты.
struct Choice {
    std::string text;
    std::string nextSceneId;
    std::vector<Requirement> requirements;
    std::vector<Requirement> costs;
};

// Сцена хранит скрытый ID для загрузки текста и переходов, выборы и награды.
// ID не показывается игроку как заголовок.
struct Scene {
    std::string id;
    std::vector<Choice> choices;
    std::vector<Reward> rewards;
    std::vector<StatReward> statRewards;
};

// Здесь я задаю стиль только для имён говорящих; остальной рассказ будет курсивным.
struct SpeakerStyle {
    std::string name;
    std::string color;
};

const std::vector<SpeakerStyle> speakerStyles{
    // Здесь я задаю ANSI-стиль: 1 включает жирность, 34/33 выбирают синий/жёлтый цвет.
    {"Вальфрост:", "\033[1;34m"},
    {"Утер:", "\033[1;33m"},
    {"Пе́р:", "\033[1;38;5;208m"},
    {"Гордон:", "\033[1;36m"}
};

// Здесь я храню ID сцен и игровые переходы; сам рассказ лежит в data/scenes/<ID>.txt.
// ID нужны программе и не выводятся на экран.
const std::vector<Scene> scenes{
    {
        "first_hunt",
            {
            {
                "Выстрелить в зайца у кустов",
                "after_first_shot",
                {{RequirementType::item, "Обычная стрела", 1}},
                {{RequirementType::item, "Обычная стрела", 1}}
            },
            {
                "Выстрелить в зайца под елью",
                "after_first_shot",
                {{RequirementType::item, "Обычная стрела", 1}},
                {{RequirementType::item, "Обычная стрела", 1}}
            }
        }
    },
    {
        "after_first_shot",
            {
            {
                "Выстрелить во второго зайца",
                "shoot_second_rabbit",
                {{RequirementType::item, "Обычная стрела", 1}},
                {{RequirementType::item, "Обычная стрела", 1}}
            },
            {
                "Сковать его лапы льдом",
                "freeze_rabbit",
                {{RequirementType::stat, "Стужа", 2}},
                {}
            }
        }
    },
    {
        "shoot_second_rabbit",
            {{"Забрать добычу", "gather_rabbits"}}
    },
    {
        "gather_rabbits",
            {{"Отправиться на встречу с Утером", "meet_uter"}},
            {{"Заяц", 2}}
    },
    {
        "freeze_rabbit",
            {
            {"Дать ему уйти", "let_rabbit_escape"},
            {
                "Выстрелить в замороженного зайца",
                "shoot_frozen_rabbit",
                {{RequirementType::item, "Обычная стрела", 1}},
                {{RequirementType::item, "Обычная стрела", 1}}
            }
        }
    },
    {
        "let_rabbit_escape",
            {},
            {{"Заяц", 1}}
    },
    {
        "shoot_frozen_rabbit",
            {{"Отправиться на встречу с Утером", "meet_uter_ice"}},
            {{"Заяц", 2}}
    },
    {
        "meet_uter",
            {
            {"Посмеяться", "laugh_at_uter"},
            {"Пошутить в ответ", "joke_with_uter"},
            {"Промолчать", "stay_silent"}
        }
    },
    {
        "meet_uter_ice",
            {{"Далее", "wolf_tracks"}}
    },
    {
        "laugh_at_uter",
            {{"Далее", "thoughts_about_hunt"}}
    },
    {
        "joke_with_uter",
            {{"Далее", "boar_debate"}}
    },
    {
        "stay_silent",
            {{"Далее", "uter_notices_silence"}}
    },
    {
        "thoughts_about_hunt",
            {{"Далее", "hunting_advice"}}
    },
    {
        "hunting_advice",
            {{"Далее", "village_fork"}}
    },
    {
        "boar_debate",
            {{"Далее", "uter_pride"}}
    },
    {
        "uter_pride",
            {{"Далее", "village_fork"}}
    },
    {
        "uter_notices_silence",
            {{"Далее", "village_fork"}}
    },
    {
        "village_fork",
            {
            {"Свернуть к углежогам", "charcoal_camp"},
            {"Идти дальше до деревни", "short_way_home"}
        }
    },
    {
        "charcoal_camp",
            {
            {"Осмотреться", "coal_bags"},
            {"Попрощаться и пойти домой", "leave_camp"}
        }
    },
    {
        "short_way_home",
            {
            {"Рассказать Утеру о гильдии", "tell_uter_about_guild"},
            {"Промолчать", "keep_dream_to_myself"}
        }
    },
    {
        "coal_bags",
            {
            {"Помочь перенести мешки", "help_carry_coal", {{RequirementType::stat, "Сила", 5}}},
            {"Попрощаться и пойти домой", "leave_camp"}
        }
    },
    {
        "help_carry_coal",
            {{"Попрощаться и пойти домой", "leave_camp"}}
    },
    {
        "leave_camp",
            {
            {"Поговорить с Гордоном", "talk_to_gordon"},
            {"Вернуться в деревню", ""}
        }
    },
    {
        "talk_to_gordon",
            {{"Вернуться в деревню", ""}}
    },
    {
        "tell_uter_about_guild",
            {{"Вернуться в деревню", ""}}
    },
    {
        "keep_dream_to_myself",
            {{"Вернуться в деревню", ""}}
    },
    {
        "wolf_tracks",
            {
            {"Остаться на тропе", "stay_on_trail"},
            {"Пойти по следам", "follow_wolf_tracks"}
        }
    },
    {
        "stay_on_trail",
            {{"Заглянуть к углежогам", "warn_wolf_at_camp"}}
    },
    {
        "warn_wolf_at_camp",
            {
            {"Осмотреться", "coal_bags"},
            {"Попрощаться и пойти домой", "leave_camp"}
        }
    },
    {
        "follow_wolf_tracks",
            {{"Продолжить по следу", "wolf_in_thicket"}}
    },
    {
        "wolf_in_thicket",
            {
            {"Предложить тихо уйти", "wolf_retreat"},
            {
                "Выстрелить лютоволку в лапу",
                "wolf_arrow_shot",
                {{RequirementType::item, "Обычная стрела", 1}},
                {{RequirementType::item, "Обычная стрела", 1}}
            },
            {
                "Попробовать сковать лапы лютоволка стужей",
                "wolf_freeze_plan",
                {{RequirementType::stat, "Стужа", 5}},
                {}
            }
        }
    },
    {
        "wolf_retreat",
            {{"Заглянуть к углежогам", "warn_wolf_at_camp"}}
    },
    {
        "wolf_arrow_shot",
            {}
    },
    {
        "wolf_freeze_plan",
            {
            {"Удерживать лёд, пока Утер добивает зверя", "wolf_death"},
            {"Присмотреться ко льду", "inspect_ice"}
        }
    },
    {
        "inspect_ice",
            {
            {
                "Вырастить ледяную пику",
                "ice_spear",
                {{RequirementType::stat, "Стужа", 6}},
                {}
            },
            {"Удерживать лёд, пока Утер добивает зверя", "wolf_death"}
        },
            {},
            {{"Стужа", 1}}
    },
    {
        "wolf_death",
            {{"Поднять лютоволка", "lift_wolf"}}
    },
    {
        "lift_wolf",
            {}
    },
    {
        "ice_spear",
            {}
    }
};

// Я объявляю эту функцию заранее, потому что пауза использует её выше по файлу.
void clearScreen();

// Здесь я жду Enter, чтобы игрок успел прочитать экран.
void waitForEnter() {
    std::cout << "\nНажми Enter, чтобы продолжить...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
    clearScreen();
}

// По названию я возвращаю текущее значение характеристики героя.
int getStat(const Player& player, const std::string& name) {
    if (name == "Стужа") {
        return player.frostMagic;
    }
    if (name == "Сила") {
        return player.strength;
    }
    return 0;
}

// Если предмета нет в инвентаре, его количество равно нулю.
int getItemCount(const Player& player, const std::string& item) {
    // Здесь я ищу предмет через find: в отличие от [], он не добавит отсутствующий предмет.
    const auto found = player.inventory.find(item);
    // Если find вернул end(), значит, предмет не найден и его запас равен нулю.
    if (found == player.inventory.end()) {
        return 0;
    }
    return found->second;
}

// Здесь я проверяю все требования действия: характеристику и/или предметы.
bool meetsRequirements(const Player& player, const std::vector<Requirement>& requirements) {
    for (const Requirement& requirement : requirements) {
        int currentAmount = 0;
        if (requirement.type == RequirementType::stat) {
            currentAmount = getStat(player, requirement.name);
        } else {
            currentAmount = getItemCount(player, requirement.name);
        }

        // Здесь я проверяю, что у героя не меньше предметов или очков, чем требует выбор.
        if (currentAmount < requirement.amount) {
            return false;
        }
    }
    return true;
}

// Эта функция печатает требование рядом с действием в подходящем формате.
void printRequirement(const Requirement& requirement) {
    std::cout << requirement.name;
    if (requirement.type == RequirementType::stat) {
        std::cout << " ≥ ";
    } else {
        std::cout << " × ";
    }
    std::cout << requirement.amount;
}

// После удачного выбора я вычитаю из инвентаря потраченные предметы.
void applyCosts(Player& player, const Choice& choice) {
    for (const Requirement& cost : choice.costs) {
        if (cost.type == RequirementType::item) {
            // После проверки требования я уменьшаю запас предмета на его стоимость.
            player.inventory[cost.name] -= cost.amount;
        }
    }
}

const Scene* findScene(const std::string& sceneId);
int readNumber(int minValue, int maxValue, const std::string& prompt);

// Я записываю текущую сцену и состояние героя в обычный текстовый файл.
bool saveGame(const Player& player, const std::string& sceneId) {
    std::ofstream saveFile("savegame.txt");
    if (!saveFile) {
        return false;
    }

    saveFile << "TEXTRPG_SAVE_V1\n"
             << std::quoted(sceneId) << '\n'
             << player.strength << ' ' << player.frostMagic << ' ' << player.freePoints << '\n'
             << player.inventory.size() << '\n';
    for (const auto& item : player.inventory) {
        // std::quoted сохраняет названия предметов с пробелами как одну строку.
        saveFile << std::quoted(item.first) << ' ' << item.second << '\n';
    }
    return static_cast<bool>(saveFile);
}

// Я читаю сохранение во временный объект и меняю героя только после проверки всех данных.
bool loadGame(Player& player, std::string& sceneId) {
    std::ifstream saveFile("savegame.txt");
    std::string version;
    std::string savedSceneId;
    int strength = 0;
    int frostMagic = 0;
    int freePoints = 0;
    int itemCount = 0;

    if (!(saveFile >> version) || version != "TEXTRPG_SAVE_V1"
        || !(saveFile >> std::quoted(savedSceneId)
             >> strength >> frostMagic >> freePoints >> itemCount)) {
        return false;
    }
    if (strength < 0 || frostMagic < 0 || freePoints < 0 || itemCount < 0 || itemCount > 1000
        || findScene(savedSceneId) == nullptr) {
        return false;
    }

    std::map<std::string, int> inventory;
    for (int index = 0; index < itemCount; ++index) {
        std::string itemName;
        int amount = 0;
        if (!(saveFile >> std::quoted(itemName) >> amount) || amount < 0) {
            return false;
        }
        inventory[itemName] = amount;
    }

    player.strength = strength;
    player.frostMagic = frostMagic;
    player.freePoints = freePoints;
    player.inventory = inventory;
    sceneId = savedSceneId;
    return true;
}

// Здесь я показываю предметы и даю вернуться, сохранить или загрузить игру.
std::string showInventory(Player& player, const std::string& currentSceneId, bool& loadedSave) {
    loadedSave = false;
    clearScreen();
    std::cout << "=== ИНВЕНТАРЬ И ХАРАКТЕРИСТИКИ ===\n"
              << "Сила: " << player.strength << '\n'
              << "Стужа: " << player.frostMagic << "\n\n"
              << "Предметы:\n";

    for (const auto& item : player.inventory) {
        // В паре map поле first — название предмета, а second — его количество.
        std::cout << item.first << ": " << item.second << '\n';
    }

    std::cout << "\n1. Вернуться к сцене\n"
              << "2. Сохранить игру\n"
              << "3. Загрузить игру\n";
    const int choice = readNumber(1, 3, "Твой выбор: ");
    if (choice == -1 || choice == 1) {
        return currentSceneId;
    }
    if (choice == 2) {
        std::cout << (saveGame(player, currentSceneId)
                          ? "\nИгра сохранена.\n"
                          : "\nНе удалось сохранить игру.\n");
        waitForEnter();
        return currentSceneId;
    }

    std::string savedSceneId;
    if (loadGame(player, savedSceneId)) {
        loadedSave = true;
        return savedSceneId;
    }
    std::cout << "\nНе удалось загрузить сохранение.\n";
    waitForEnter();
    return currentSceneId;
}

// Эта функция очищает терминал и ставит курсор в его левый верхний угол.
void clearScreen() {
    // Здесь я стираю экран кодом 2J, ставлю курсор в начало через H и сразу отправляю команды через flush.
    std::cout << "\033[2J\033[H" << std::flush;
}

// Я читаю число в заданном диапазоне и повторяю запрос, если ввод неверный.
int readNumber(int minValue, int maxValue, const std::string& prompt) {
    while (true) {
        std::cout << prompt;
        int number = 0;

        // Здесь я читаю число; при ошибке cin сообщает об этом условии if.
        if (std::cin >> number) {
            if (number >= minValue && number <= maxValue) {
                return number;
            }
            std::cout << "Введи число от " << minValue << " до " << maxValue << ".\n";
        } else if (std::cin.eof()) {
            // Если ввод закрыт (например, нажали Ctrl+D), я завершаю текущий экран.
            return -1;
        } else {
            std::cout << "Введи номер цифрой.\n";
            // clear сбрасывает ошибку чтения, а ignore удаляет неверный текст до конца строки.
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
}

// Я прошу выбрать существующий вариант и проверяю его требования.
int readChoice(const Scene& scene, const Player& player) {
    while (true) {
        const int choice = readNumber(0, static_cast<int>(scene.choices.size()), "Твой выбор: ");
        if (choice == -1) {
            return -1;
        }
        if (choice == 0) {
            return -2;
        }

        // Игрок видит нумерацию с 1, а vector начинается с индекса 0.
        const int selectedIndex = choice - 1;
        const Choice& selected = scene.choices[selectedIndex];
        if (meetsRequirements(player, selected.requirements)) {
            return selectedIndex;
        }

        std::cout << "Не хватает: ";
        bool firstMissingRequirement = true;
        for (const Requirement& requirement : selected.requirements) {
            if (!meetsRequirements(player, {requirement})) {
                if (!firstMissingRequirement) {
                    std::cout << ", ";
                }
                printRequirement(requirement);
                firstMissingRequirement = false;
            }
        }
        std::cout << ". Попробуй другой вариант.\n";
    }
}

// Здесь я показываю характеристики и предметы, чтобы видеть состояние героя.
// Перед началом игры я даю потратить все три свободных очка.
bool configurePlayer(Player& player, std::string& firstSceneId) {
    while (true) {
        clearScreen();
        std::cout << "=== СОЗДАНИЕ ГЕРОЯ ===\n"
                  << "Сила: " << player.strength << '\n'
                  << "Стужа: " << player.frostMagic << '\n'
                  << "Свободные очки: " << player.freePoints << "\n\n";

        if (player.freePoints == 0) {
            std::cout << "1. Начать игру\n"
                      << "2. Загрузить сохранение\n"
                      << "3. Назад\n";
            const int choice = readNumber(1, 3, "Твой выбор: ");
            if (choice == -1 || choice == 3) {
                return false;
            }
            if (choice == 1) {
                firstSceneId = "first_hunt";
                return true;
            }

            if (loadGame(player, firstSceneId)) {
                return true;
            }
            std::cout << "\nНе удалось загрузить сохранение.\n";
            waitForEnter();
            continue;
        }

        std::cout << "1. Добавить 1 к Силе\n"
                  << "2. Добавить 1 к Стуже\n"
                  << "3. Загрузить сохранение\n"
                  << "4. Назад\n";
        const int choice = readNumber(1, 4, "Твой выбор: ");
        if (choice == -1 || choice == 4) {
            return false;
        }

        if (choice == 3) {
            if (loadGame(player, firstSceneId)) {
                return true;
            }
            std::cout << "\nНе удалось загрузить сохранение.\n";
            waitForEnter();
            continue;
        }

        if (choice == 1) {
            ++player.strength;
        } else {
            ++player.frostMagic;
        }
        --player.freePoints;
    }
}

// По внутреннему ID я нахожу сцену, на которую ведёт выбранное действие.
const Scene* findScene(const std::string& sceneId) {
    for (const Scene& scene : scenes) {
        if (scene.id == sceneId) {
            // Здесь я возвращаю адрес найденной сцены, не копируя весь её текст.
            return &scene;
        }
    }
    // Если ID не найден, я возвращаю nullptr — специальное значение «нет сцены».
    return nullptr;
}

// Здесь я печатаю повествование курсивом, а имена говорящих — цветными и жирными.
void printNarrativeText(const std::string& text) {
    const std::string italic = "\033[3m";
    const std::string reset = "\033[0m";
    std::size_t cursor = 0;

    std::cout << italic;
    while (cursor < text.size()) {
        std::size_t nextSpeakerPosition = std::string::npos;
        const SpeakerStyle* nextSpeakerStyle = nullptr;

        for (const SpeakerStyle& style : speakerStyles) {
            // Здесь я ищу UTF-8 последовательность метки и получаю позицию её байтов в строке.
            const std::size_t position = text.find(style.name, cursor);
            if (position != std::string::npos && position < nextSpeakerPosition) {
                nextSpeakerPosition = position;
                nextSpeakerStyle = &style;
            }
        }

        if (nextSpeakerStyle == nullptr) {
            std::cout << text.substr(cursor);
            break;
        }

        std::cout << text.substr(cursor, nextSpeakerPosition - cursor)
                  << reset << nextSpeakerStyle->color << nextSpeakerStyle->name
                  << reset << italic;
        // После метки я продолжаю поиск со следующего участка текста.
        cursor = nextSpeakerPosition + nextSpeakerStyle->name.size();
    }
    std::cout << reset;
}

// Я загружаю текст отдельно от кода, используя ID сцены как имя файла.
std::string loadSceneText(const std::string& sceneId) {
    const std::string filePath = "data/scenes/" + sceneId + ".txt";
    std::ifstream sceneFile(filePath);
    if (!sceneFile) {
        std::cerr << "Не удалось открыть файл сцены: " << filePath << '\n';
        return "";
    }

    // Здесь я читаю весь файл целиком, включая переносы строк между абзацами.
    return std::string(std::istreambuf_iterator<char>(sceneFile),
                       std::istreambuf_iterator<char>());
}

// Я прохожу сцены одну за другой, пока игрок не дойдёт до конца ветки.
void playScene(const std::string& firstSceneId, Player& player) {
    // Здесь я храню адрес сцены, которая сейчас показывается игроку.
    const Scene* currentScene = findScene(firstSceneId);
    bool skipSceneRewards = false;

    while (currentScene != nullptr) {
        // Перед новой сценой я убираю предыдущий текст с экрана.
        clearScreen();

        if (!skipSceneRewards) {
            // Награду я добавляю один раз при входе; после загрузки сохранения она уже учтена.
            for (const Reward& reward : currentScene->rewards) {
                player.inventory[reward.item] += reward.amount;
            }
            for (const StatReward& reward : currentScene->statRewards) {
                // Когда сцена даёт Стужу или Силу, я прибавляю награду к нужной характеристике.
                if (reward.stat == "Стужа") {
                    player.frostMagic += reward.amount;
                } else if (reward.stat == "Сила") {
                    player.strength += reward.amount;
                }
            }
        }
        skipSceneRewards = false;

        const std::string sceneText = loadSceneText(currentScene->id);
        if (sceneText.empty()) {
            waitForEnter();
            return;
        }

        std::cout << '\n';
        printNarrativeText(sceneText);
        std::cout << "\n\n";

        if (currentScene->choices.empty()) {
            break;
        }

        std::cout << "0. Инвентарь и характеристики\n";
        for (int index = 0; index < static_cast<int>(currentScene->choices.size()); ++index) {
            const Choice& choice = currentScene->choices[index];
            std::cout << index + 1 << ". " << choice.text;
            if (!choice.requirements.empty()) {
                std::cout << " [требуется: ";
                for (int requirementIndex = 0;
                     requirementIndex < static_cast<int>(choice.requirements.size());
                     ++requirementIndex) {
                    if (requirementIndex > 0) {
                        std::cout << ", ";
                    }
                    printRequirement(choice.requirements[requirementIndex]);
                }
                std::cout << ']';
            }
            std::cout << '\n';
        }

        const int selectedChoice = readChoice(*currentScene, player);
        if (selectedChoice == -1) {
            return;
        }
        if (selectedChoice == -2) {
            bool loadedSave = false;
            const std::string nextSceneId = showInventory(player, currentScene->id, loadedSave);
            currentScene = findScene(nextSceneId);
            skipSceneRewards = loadedSave;
            continue;
        }

        // Я беру выбранное действие, применяю его стоимость и перехожу по его ID.
        const Choice& choice = currentScene->choices[selectedChoice];
        applyCosts(player, choice);
        const std::string nextSceneId = choice.nextSceneId;
        currentScene = findScene(nextSceneId);
    }

    waitForEnter();
}

// Здесь начинается программа: главное меню запускает создание героя или закрывает игру.
int main() {
    bool isRunning = true;
    Player player;

    while (isRunning) {
        clearScreen();
        std::cout << "=== СНЕЖНЫЙ СЕВЕР ===\n"
                  << "1. Начать игру\n"
                  << "2. Выход\n";

        const int choice = readNumber(1, 2, "Твой выбор: ");
        if (choice == -1) {
            break;
        }

        if (choice == 1) {
            // Начало новой игры возвращает героя к стартовым характеристикам и предметам.
            player = Player{};
            std::string firstSceneId;
            if (configurePlayer(player, firstSceneId)) {
                playScene(firstSceneId, player);
            }
        } else {
            isRunning = false;
        }
    }

    std::cout << "До встречи!\n";
    return 0;
}
