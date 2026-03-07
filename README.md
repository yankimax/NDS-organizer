# Nintendo DS Organizer

English and Russian documentation are available in this file.

- [English](#english)
- [Русский](#russian)

<a id="english"></a>
## English

Minimal organizer homebrew app for Nintendo DS with two modes:
- Clock
- Stopwatch

The project is written in C with `libnds` (ARM9) and uses both screens:
- Top screen: large time rendered with sprites
- Bottom screen: text UI, controls, and settings

### Emulator

This project targets **melonDS**.

`DeSmuME` is not supported for this project.

To use `organizer.sav` in melonDS, enable DLDI for homebrew.

### Features

- Real-time clock mode
- Stopwatch with start/pause/reset
- Light/dark theme switching
- First-run language selection
- Saved language in `organizer.sav`
- Dynamic localization from `locales/*.lng` (new file = new language in menu)
- Buttons and touchscreen input

### Controls

- `START` - toggle light/dark theme
- `SELECT` - switch mode (`Clock` / `Stopwatch`)
- `L+R` - open/close `Settings`
- `A` - start/stop stopwatch (in stopwatch mode)
- `B` - reset stopwatch (in stopwatch mode)
- `UP/DOWN + A` - select language on first-run language screen
- In `Settings`: `A` - open language selection, `B` - back
- Touch buttons `Clock` / `Stopwatch` - switch mode
- Touch buttons `Start/Stop` and `Reset` - control stopwatch

### Screenshots

![Clock, dark theme](screenshots/01-clock-dark-theme.png)
![Clock, light theme](screenshots/02-clock-light-theme.png)
![Stopwatch, dark theme](screenshots/03-stopwatch-dark-theme.png)
![Stopwatch, light theme](screenshots/04-stopwatch-light-theme.png)

![Clock on real hardware](screenshots/clock.png)
![Stopwatch on real hardware](screenshots/timer.png)

### Project Structure

- `source/main.c` - entry point (`appRun()`)
- `source/app.c` - main loop, input handling, mode/state logic
- `source/save.c` - read/write `organizer.sav`
- `source/localization.c` - NitroFS locale loading and `.lng` parsing
- `source/ui.c` - bottom-screen UI rendering and touch hit-tests
- `source/top_display.c` - top-screen large time rendering
- `include/app.h` - app state and mode types
- `include/save.h` - save module API
- `include/localization.h` - localization API and field map
- `include/ui.h` - UI API and `UiAction`
- `include/top_display.h` - top display API
- `locales/` - localization files (`*.lng`)
- `gfx/` - generated graphics and grit descriptors
- `tools/ttf2nds.py` - TTF to NDS bitmap font generator
- `Makefile` - build pipeline

### Localization

Locales are loaded from NitroFS path `nitro:/locales`.

During `make`, files from `locales/*.lng` are copied into `.nitrofs/locales` and packed into the ROM.

Localization is strict:
- all keys are required,
- invalid locale files are ignored,
- no fallback strings are used,
- if there are no valid locales, app shows localization error screen.

To add a new key, update `LOCALIZATION_FIELD_MAP` in `include/localization.h`, then add this key in every `locales/*.lng` file.

`.lng` files can be UTF-8. Text is converted for the console font at runtime.

Supported keys in `.lng`:
- `name`
- `mode_title`
- `mode_clock`
- `mode_stopwatch`
- `control_title`
- `action_start`
- `action_stop`
- `action_reset`
- `status_run`
- `status_pause`
- `settings_title`
- `settings_language`
- `settings_hint_a`
- `settings_hint_b`
- `settings_hint_lr`
- `language_select_title`
- `language_select_hint`
- `hint_start`
- `hint_select`
- `hint_a`
- `hint_b`
- `hint_lr`

Example:

```ini
name=English
mode_title=Mode
mode_clock=Clock
mode_stopwatch=Stopwatch
control_title=Controls
action_start=Start
action_stop=Stop
action_reset=Reset
status_run=Running
status_pause=Paused
settings_title=Settings
settings_language=Language
settings_hint_a=A: choose language
settings_hint_b=B: back
settings_hint_lr=L+R: close settings
language_select_title=Select language
language_select_hint=UP/DOWN + A to select
hint_start=START: toggle theme
hint_select=SELECT: switch mode
hint_a=A: start/stop
hint_b=B: reset
hint_lr=L+R: settings
```

### Build

#### Dependencies

Required devkitPro components:
- `devkitARM`
- `libnds`
- `libfat`
- `grit`

Also required:
- `python3` (for `tools/ttf2nds.py`)

`DEVKITARM` must be set.

Example:

```bash
export DEVKITPRO=/opt/devkitpro
export DEVKITARM=$DEVKITPRO/devkitARM
```

#### Commands

Build project:

```bash
make
```

`make` does:
- cleans previous artifacts,
- generates `gfx/font.png` and `gfx/digits.png`,
- syncs `locales/*.lng` into NitroFS source dir,
- compiles sources,
- builds ROM `dist/organizer.nds`,
- copies locale files to `dist/locales/*.lng`.

Clean artifacts:

```bash
make clean
```

### Output

- `dist/organizer.nds` - ROM for melonDS
- `dist/organizer.elf` - ELF build artifact

### Run

1. Open melonDS.
2. Load `dist/organizer.nds`.
3. Use controls from this README.

<a id="russian"></a>
## Русский

Минималистичный homebrew-органайзер для Nintendo DS с двумя режимами:
- Часы
- Секундомер

Проект написан на C с использованием `libnds` (ARM9) и использует оба экрана:
- Верхний экран: крупное время спрайтами
- Нижний экран: текстовый интерфейс, кнопки и настройки

### Эмулятор

Проект рассчитан на **melonDS**.

`DeSmuME` для этого проекта не поддерживается.

Чтобы работал `organizer.sav` в melonDS, включите DLDI для homebrew.

### Возможности

- Отображение текущего времени
- Секундомер со стартом, паузой и сбросом
- Переключение светлой/тёмной темы
- Выбор языка при первом запуске
- Сохранение выбранного языка в `organizer.sav`
- Динамические локализации из `locales/*.lng` (новый файл = новый язык в меню)
- Управление кнопками и сенсором

### Управление

- `START` - переключить светлую/тёмную тему
- `SELECT` - переключить режим (`Часы` / `Секундомер`)
- `L+R` - открыть/закрыть `Settings`
- `A` - старт/стоп секундомера (в режиме секундомера)
- `B` - сброс секундомера (в режиме секундомера)
- `UP/DOWN + A` - выбор языка на экране первичной настройки
- В `Settings`: `A` - открыть выбор языка, `B` - назад
- Сенсорные кнопки `Часы` / `Секундомер` - переключение режима
- Сенсорные кнопки `Старт/Стоп` и `Сброс` - управление секундомером

### Скриншоты

![Часы, тёмная тема](screenshots/01-clock-dark-theme.png)
![Часы, светлая тема](screenshots/02-clock-light-theme.png)
![Секундомер, тёмная тема](screenshots/03-stopwatch-dark-theme.png)
![Секундомер, светлая тема](screenshots/04-stopwatch-light-theme.png)

![Часы на реальном устройстве](screenshots/clock.png)
![Секундомер на реальном устройстве](screenshots/timer.png)

### Структура проекта

- `source/main.c` - точка входа (`appRun()`)
- `source/app.c` - основной цикл, обработка ввода, логика режимов и состояния
- `source/save.c` - чтение/запись `organizer.sav`
- `source/localization.c` - загрузка языков из NitroFS и парсинг `.lng`
- `source/ui.c` - интерфейс нижнего экрана и hit-тест сенсора
- `source/top_display.c` - отрисовка времени на верхнем экране
- `include/app.h` - типы состояния и режимов приложения
- `include/save.h` - API модуля сохранений
- `include/localization.h` - API локализации и карта полей
- `include/ui.h` - API UI и `UiAction`
- `include/top_display.h` - API верхнего экрана
- `locales/` - файлы локализаций (`*.lng`)
- `gfx/` - графические ресурсы и grit-описания
- `tools/ttf2nds.py` - генератор bitmap-шрифтов из TTF
- `Makefile` - сценарий сборки

### Локализация

Языки загружаются из NitroFS-пути `nitro:/locales`.

Во время `make` файлы `locales/*.lng` копируются в `.nitrofs/locales` и вшиваются в ROM.

Система локализации строгая:
- все ключи обязательны,
- невалидные файлы локализаций игнорируются,
- fallback-строки не используются,
- если валидных языков нет, показывается экран ошибки локализации.

Чтобы добавить новый ключ, обновите `LOCALIZATION_FIELD_MAP` в `include/localization.h`, затем добавьте этот ключ во все `locales/*.lng`.

Файлы `.lng` можно хранить в UTF-8. Для консольного шрифта строки конвертируются во время выполнения.

Поддерживаемые ключи в `.lng`:
- `name`
- `mode_title`
- `mode_clock`
- `mode_stopwatch`
- `control_title`
- `action_start`
- `action_stop`
- `action_reset`
- `status_run`
- `status_pause`
- `settings_title`
- `settings_language`
- `settings_hint_a`
- `settings_hint_b`
- `settings_hint_lr`
- `language_select_title`
- `language_select_hint`
- `hint_start`
- `hint_select`
- `hint_a`
- `hint_b`
- `hint_lr`

Пример:

```ini
name=English
mode_title=Mode
mode_clock=Clock
mode_stopwatch=Stopwatch
control_title=Controls
action_start=Start
action_stop=Stop
action_reset=Reset
status_run=Running
status_pause=Paused
settings_title=Settings
settings_language=Language
settings_hint_a=A: choose language
settings_hint_b=B: back
settings_hint_lr=L+R: close settings
language_select_title=Select language
language_select_hint=UP/DOWN + A to select
hint_start=START: toggle theme
hint_select=SELECT: switch mode
hint_a=A: start/stop
hint_b=B: reset
hint_lr=L+R: settings
```

### Сборка

#### Зависимости

Нужные компоненты devkitPro:
- `devkitARM`
- `libnds`
- `libfat`
- `grit`

Также требуется:
- `python3` (для `tools/ttf2nds.py`)

Переменная `DEVKITARM` должна быть установлена.

Пример:

```bash
export DEVKITPRO=/opt/devkitpro
export DEVKITARM=$DEVKITPRO/devkitARM
```

#### Команды

Собрать проект:

```bash
make
```

`make` выполняет:
- очистку старых артефактов,
- генерацию `gfx/font.png` и `gfx/digits.png`,
- синхронизацию `locales/*.lng` в NitroFS-каталог,
- компиляцию исходников,
- сборку ROM `dist/organizer.nds`,
- копирование локализаций в `dist/locales/*.lng`.

Очистить артефакты:

```bash
make clean
```

### Выходные файлы

- `dist/organizer.nds` - ROM для melonDS
- `dist/organizer.elf` - ELF-артефакт сборки

### Запуск

1. Откройте melonDS.
2. Загрузите `dist/organizer.nds`.
3. Используйте управление из README.
