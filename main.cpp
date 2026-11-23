#include <iostream>
#include <limits>
#include <map>
#include <string>
#include <vector>

struct Player {
    int frostMagic = 5;
    int strength = 5;
    std::map<std::string, int> inventory{{"Обычная стрела", 2}};
};

enum class RequirementType {
    stat,
    item
};

struct Requirement {
    RequirementType type;
    std::string name;
    int amount;
};

struct Reward {
    std::string item;
    int amount;
};

struct Choice {
    std::string text;
    std::string nextSceneId;
    std::vector<Requirement> requirements;
    std::vector<Requirement> costs;
};

struct Scene {
    std::string id;
    std::string text;
    std::vector<Choice> choices;
    std::vector<Reward> rewards;
};

const std::vector<Scene> scenes{
    {
        "first_hunt",
        "С утра мороз щипал щёки, а к полудню ветер погнал снег за воротник. "
        "У поваленной сосны мы с дедом разошлись. Утер пошёл вверх по склону, "
        "а я свернул в ельник. До встречи оставалось время, но пояс у меня "
        "всё ещё был пустой.\n\n"
        "Вальфрост: Хоть бы сегодня вернуться с добычей. А то дед опять скажет, "
        "что я по лесу гулять ходил.\n\n"
        "У корней ели я заметил свежие следы. Под ветками шевельнулось белое ухо. "
        "Я снял лук с плеча и увидел второго зайца у самых кустов.\n\n"
        "Вальфрост: Вот бы добыть обоих. Хотя дед, чего доброго, опять притащит кабана.",
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
        "Потрачено: Обычная стрела ×1\n\n"
        "Тетива щёлкнула. Один заяц упал в снег, второй вскинул уши и приготовился бежать.\n\n"
        "Вальфрост: Вот ведь. Теперь бы его не упустить.",
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
        "Потрачено: Обычная стрела ×1.\n\n"
        "Я успел наложить стрелу на тетиву, прежде чем заяц сорвался с места. "
        "Он прыгнул к кустам, но до них так и не добрался.\n\n"
        "Вальфрост: Оба. Вот теперь можно идти к деду.",
        {{"Забрать добычу", "gather_rabbits"}}
    },
    {
        "gather_rabbits",
        "Я опустил лук и пошёл за добычей. Первый заяц лежал под елью, второй — у самого края кустов. "
        "Я подобрал обоих, стряхнул с них снег и привязал их к поясу.\n\n"
        "Получено: Заяц ×2\n\n"
        "Вальфрост: Оба. Вот теперь можно идти к деду.",
        {{"Отправиться на встречу с Утером", "meet_uter"}},
        {{"Заяц", 2}}
    },
    {
        "freeze_rabbit",
        "Я протянул руку. По снегу пробежал иней и прихватил задние лапы зайца. "
        "Тот рванулся, и лёд тихо хрустнул.\n\n"
        "Вальфрост: Долго не продержится. Одного выстрела хватит.",
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
        "Я уже взялся за вторую стрелу, но передумал. Заяц метнулся в кусты, только ветки качнулись. "
        "Я подошёл к ели и привязал добычу к поясу.\n\n"
        "Получено: Заяц ×1.\n\n"
        "Вальфрост: Одного на ужин хватит. Хотя дед наверняка спросит, куда делся второй.",
        {},
        {{"Заяц", 1}}
    },
    {
        "shoot_frozen_rabbit",
        "Потрачено: Обычная стрела ×1\n\n"
        "Лёд у заячьих лап уже потрескивал. Я наложил стрелу на тетиву и выстрелил прежде, "
        "чем заяц успел вырваться. Он упал на бок, и снег вокруг него осыпался мелкими льдинками.\n\n"
        "Вальфрост: Сработало. Теперь можно забирать обоих.\n\n"
        "Получено: Заяц ×2",
        {{"Отправиться на встречу с Утером", "meet_uter_ice"}},
        {{"Заяц", 2}}
    },
    {
        "meet_uter",
        "Утер ждал меня у поваленной сосны. На снегу у его ног лежал кабан. Дед заметил зайцев у меня на поясе и приподнял брови.\n\n"
        "Утер: Ого, двоих взял, малой. Ловко. А я-то всего одного добыл.\n\n"
        "Он нарочно помолчал, потом носком сапога ткнул кабана в бок.\n\n"
        "Утер: Правда, мой один, пожалуй, потянет на десяток твоих.\n\n"
        "Дед ухмыльнулся и посмотрел на меня. Ждал, что я скажу.",
        {
            {"Посмеяться", "laugh_at_uter"},
            {"Пошутить в ответ", "joke_with_uter"},
            {"Промолчать", "stay_silent"}
        }
    },
    {
        "meet_uter_ice",
        "Когда я вернулся к поваленной сосне, Утер уже ждал меня. Он сидел верхом на огромном кабане и разглядывал свой охотничий нож. Завидев меня, дед убрал нож в ножны и ухмыльнулся.\n\n"
        "Утер: Всего два зайца, малой? Я-то думал, ты за это время хоть одного кабана помельче найдёшь.\n\n"
        "Я глянул на кабана и поправил зайцев на поясе.\n\n"
        "Вальфрост: Зато своих я донесу до дома без передышки.\n\n"
        "Дед поднялся, подошёл ко мне и приподнял одного зайца за лапу. Между задними лапами ещё держалась тонкая полоска льда.\n\n"
        "Утер: Это ты его стужей прихватил?\n\n"
        "Вальфрост: Он уже прыгнул к кустам, но лёд продержался, пока я доставал стрелу.\n\n"
        "Утер: А в прошлом году ты бы всю поляну инеем покрыл, пока до лап добрался.\n\n"
        "Вальфрост: Думаешь, я теперь и волка так удержу?\n\n"
        "Утер: На миг, может, и удержишь, только сначала прикинь, что за этот миг успеешь сделать.",
        {}
    },
    {
        "laugh_at_uter",
        "Вальфрост: Ха-ха. А я-то виноват, что мне только зайцы попались? Да и второй был слишком юркий.\n\n"
        "Не скажу же я деду что во время озоты отпустил зайца просто так...",
        {}
    },
    {
        "joke_with_uter",
        "Вальфрост: Ты с этим кабаном вернулся весь мокрый и запыханный. Поди гонял беднягу по всему лесу.",
        {}
    },
    {
        "stay_silent",
        "Я поправил ремень. Зайцы тут же снова стукнулись о ножны.",
        {}
    }
};

void waitForEnter() {
    std::cout << "\nНажми Enter, чтобы вернуться в меню...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

int getStat(const Player& player, const std::string& name) {
    if (name == "Стужа") {
        return player.frostMagic;
    }
    if (name == "Сила") {
        return player.strength;
    }
    return 0;
}

int getItemCount(const Player& player, const std::string& item) {
    const auto found = player.inventory.find(item);
    if (found == player.inventory.end()) {
        return 0;
    }
    return found->second;
}

bool meetsRequirements(const Player& player, const std::vector<Requirement>& requirements) {
    for (const Requirement& requirement : requirements) {
        int currentAmount = 0;
        if (requirement.type == RequirementType::stat) {
            currentAmount = getStat(player, requirement.name);
        } else {
            currentAmount = getItemCount(player, requirement.name);
        }

        if (currentAmount < requirement.amount) {
            return false;
        }
    }
    return true;
}

void printRequirement(const Requirement& requirement) {
    std::cout << requirement.name;
    if (requirement.type == RequirementType::stat) {
        std::cout << " ≥ ";
    } else {
        std::cout << " × ";
    }
    std::cout << requirement.amount;
}

void applyCosts(Player& player, const Choice& choice) {
    for (const Requirement& cost : choice.costs) {
        if (cost.type == RequirementType::item) {
            player.inventory[cost.name] -= cost.amount;
        }
    }
}

void showStatus(const Player& player) {
    std::cout << "\n=== ХАРАКТЕРИСТИКИ ВАЛЬФРОСТА ===\n"
              << "Стужа: " << player.frostMagic << '\n'
              << "Сила: " << player.strength << '\n';
    std::cout << "\n=== ПРЕДМЕТЫ ===\n";
    for (const auto& item : player.inventory) {
        std::cout << item.first << ": " << item.second << '\n';
    }
    waitForEnter();
}

int readChoice(const Scene& scene, const Player& player) {
    while (true) {
        std::cout << "Твой выбор: ";
        int choice = 0;

        if (std::cin >> choice) {
            if (choice >= 1 && choice <= static_cast<int>(scene.choices.size())) {
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
                continue;
            }
            std::cout << "Выбери номер от 1 до " << scene.choices.size() << ".\n";
        } else if (std::cin.eof()) {
            return -1;
        } else {
            std::cout << "Введи номер варианта цифрой.\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
}

const Scene* findScene(const std::string& sceneId) {
    for (const Scene& scene : scenes) {
        if (scene.id == sceneId) {
            return &scene;
        }
    }
    return nullptr;
}

void playScene(const std::string& firstSceneId, Player& player) {
    const Scene* currentScene = findScene(firstSceneId);

    while (currentScene != nullptr) {
        for (const Reward& reward : currentScene->rewards) {
            player.inventory[reward.item] += reward.amount;
        }

        std::cout << "\n" << currentScene->text << "\n\n";

        if (currentScene->choices.empty()) {
            break;
        }

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

        const Choice& choice = currentScene->choices[selectedChoice];
        applyCosts(player, choice);
        const std::string nextSceneId = choice.nextSceneId;
        currentScene = findScene(nextSceneId);
    }

    waitForEnter();
}

int main() {
    bool isRunning = true;
    Player player;

    while (isRunning) {
        std::cout << "=== СНЕЖНЫЙ СЕВЕР ===\n"
                  << "1. Начать игру\n"
                  << "2. Характеристики\n"
                  << "3. Выход\n"
                  << "Твой выбор: ";

        int choice = 0;
        if (!(std::cin >> choice)) {
            if (std::cin.eof()) {
                break;
            }

            std::cout << "Пожалуйста, введи номер пункта.\n\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        if (choice == 1) {
            player = Player{};
            playScene("first_hunt", player);
        } else if (choice == 2) {
            showStatus(player);
        } else if (choice == 3) {
            isRunning = false;
        } else {
            std::cout << "Такого пункта нет. Выбери 1, 2 или 3.\n\n";
        }
    }

    std::cout << "До встречи!\n";
    return 0;
}
