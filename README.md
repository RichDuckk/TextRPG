# Ледяной трон / The Ice Throne

## Русский

«Ледяной трон» — одиночная текстовая RPG на стандартном C++ для запуска в терминале. Первая глава основана на материалах Ripple. Тексты сцен хранятся отдельно от игрового кода.

### Сборка и запуск

Нужны компилятор C++17 и CMake 3.16 или новее.

```sh
cmake -S . -B build
cmake --build build
./build/game
```

Для рамок псевдографики используй UTF-8-терминал шириной не менее 112 столбцов.

В главном меню выбери «Начать игру» или «Выход». Сила, Стужа и Ловкость начинаются с 5; перед игрой можно распределить ещё три очка между характеристиками.

Во время сцены нажми `0`, чтобы открыть инвентарь с характеристиками и предметами, или `9`, чтобы открыть параметры. В параметрах можно вернуться к игре, сохранить или загрузить сохранение либо выйти. Сохранение находится в локальном файле `savegame.txt`.

Тексты сцен находятся в `data/scenes/`. Запускай игру из корневой папки проекта, чтобы программа могла найти эти файлы.

## English

*The Ice Throne* is a single-player terminal RPG written in standard C++. Its first chapter is based on the Ripple story files. Scene text is kept separate from the game code.

### Build and run

You need a C++17 compiler and CMake 3.16 or newer.

```sh
cmake -S . -B build
cmake --build build
./build/game
```

The pseudo-graphic frame needs a UTF-8 terminal at least 112 columns wide.

Choose **Start Game** or **Exit** from the main menu. Strength, Frost, and Dexterity start at 5. Before playing, you can distribute three additional points among these attributes.

During a scene, press `0` to open the inventory, which shows your attributes and items, or `9` to open Settings. Settings lets you return to the game, save or load, or quit. The save is stored locally in `savegame.txt`.

Scene text is stored in `data/scenes/`. Run the game from the project root so it can find these files.
