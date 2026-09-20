# Coding Conventions — стандарти коду

> Доповнення до `AGENTS.md`. Конкретні правила оформлення, найменування,
> типові патерни проєкту. Дотримуйся їх для збереження єдиного стилю.

---

## 1. Загальні правила

- **Мова**: C99 (`-std=gnu99`), допускаються GNU-extensions.
- **Без C++**: навіть якщо заголовок може компілюватись у C++, не додавай
  класи чи простори імен. У заголовках, які можуть бути включені з C++,
  використовуй `extern "C"` (приклад — `inc/oled_min.h`).
- **Без коментарів у коді**. Якщо треба пояснити рішення — пиши у відповідь
  автору в чаті. Існуючі коментарі в коді — це спадщина від автора і шаблонів
  Semtech/Ai-Thinker, не додавай нові.
- **`#pragma once`** для нових заголовків. Не використовуй
  `#ifndef FILE_H / #define FILE_H / #endif`.
- **Без libc**: `printf`, `sprintf`, `snprintf` обгорнуті через `--wrap` у
  linker, реальна реалізація — `platform/system/printf-stdarg.c`. Не додавай
  залежностей від libc — лінкер викине все (`/DISCARD/` у `cfg/gcc.ld`).
- **Без `malloc`** — `_HEAP_SIZE = 0`. Усі буфери статичні.

---

## 2. Стиль відступів і дужок

Див. `.editorconfig`:

- Відступи — пробіли (НЕ табуляція), окрім `[Makefile]` де табуляція.
- Відкриваюча дужка блоку — на тому ж рядку з пробілом перед нею
  (`cpp_space_before_block_open_brace=true`):
  ```c
  if (cond) {
      do_thing();
  }
  ```
- `else` / `while` / `catch` — на новому рядку
  (`cpp_new_line_before_else=true`):
  ```c
  if (a) {
      ...
  }
  else {
      ...
  }
  ```
- Пробіл після коми, пробіл до/після бінарних операторів
  (`cpp_space_around_binary_operator=insert`).
- Без пробілу перед крапкою з комою, без пробілу після.
- Вказівники: `int *p` (лівостороннє вирівнювання).

---

## 3. Найменування

| Тип              | Стиль                | Приклади                          |
|------------------|----------------------|------------------------------------|
| Функції (public) | `lower_snake_case`   | `lora_init`, `adxl345_init`        |
| Функції (static) | `lower_snake_case`   | `setRegisterBit`, `writeTo`        |
| Функції з SDK    | `lowerCamelCase` або `snake_case` | `timer_init`, `i2c_master_send_start` |
| Змінні (локальні)| `lower_snake_case`   | `buf`, `buffer_size`, `tick`        |
| Змінні (глобальні) | `lower_snake_case`  | `Buffer`, `BufferSize`, `State`     |
| Константи        | `UPPER_SNAKE_CASE`   | `BUFFER_SIZE`, `RX_TIMEOUT_VALUE`  |
| Макроси          | `UPPER_SNAKE_CASE`   | `ROLE_TX`, `USE_OLED`               |
| Enum-имена       | `_t`-суфікс          | `States_t`, `RadioModems_t`         |
| Enum-значення    | `UPPER_SNAKE_CASE`   | `MODE_SENSOR`, `RF_IDLE`            |
| Структури        | `_s`-суфікс (Semtech) або `_t`   | `RadioEvents_t`, `RtcCalendar_s`   |

**Увага**: автор свідомо змішує стилі, бо частина коду — спадщина шаблонів
Semtech (`Buffer`, `BufferSize`, `State` — CamelCase-глобали). Не виправляй
це без прохання.

---

## 4. Типові ідіоми проєкту

### 4.1 Ініціалізація в `static void init()` всередині `main_*.c`

```c
static void init()
{
    board_init();
    lora_init();
    board_led_init();
    I2C_init();
    OLED_init();                  // або adxl345_init(sensitivity)
    // налаштування переривань, таймерів
}
```

### 4.2 Оголошення функцій у Semtech-стилі

```c
/*!
 * \brief Function to be executed on Radio Tx Done event
 */
void OnTxDone( void );
```

Це формат Doxygen для сумісності з оригінальними заголовками Semtech.
Дотримуйся його, коли додаєш функції, що стосуються LoRa-стека.

### 4.3 Volatile / atomic

Використовуй `volatile` для змінних, що змінюються в IRQ і читаються в
основному циклі: `volatile bool activity_trigged`, `volatile State`,
`volatile bool tx_in_progress`. Для серійних операцій (типу `tick--`) це
нормально на Cortex-M4 (atomic в межах uint).

### 4.4 Стани-машина

Копіюй патерн `enum States_t { LOWPOWER, RX, RX_TIMEOUT, ... }` і обробляй
через `switch (State) { ... }` у `main loop`. Після обробки повертай `State
= LOWPOWER`.

### 4.5 Callbacks радіо

```c
RadioEvents.TxDone = OnTxDone;
RadioEvents.RxDone = OnRxDone;
RadioEvents.TxTimeout = OnTxTimeout;
RadioEvents.RxTimeout = OnRxTimeout;
RadioEvents.RxError = OnRxError;
Radio.Init(&RadioEvents);
```

У `OnRxDone` копіюй payload в `Buffer` через `memcpy(Buffer, payload,
BufferSize)`, не через `Buffer = payload` (тип — `uint8_t *`).

### 4.6 Уникай плаваючої точки

`float`/`double` не використовуються в прошивці. Навіть RSSI/SNR —
цілі числа (`int8_t`). Для масштабування — фіксована крапка з масштабом
1:1 (dBm ≈ int).

---

## 5. Заголовки

- Підключай тільки те, що реально використовуєш.
- Порядок підключень (див. `main_*.c`):
  1. `<stdio.h>`, `<string.h>`, `<stdlib.h>` (стандартні)
  2. `tremo_*.h` (SDK периферія)
  3. `board_led.h`, `oled_min.h`, `i2c.h`, `adxl345.h`, `lora.h`
     (власні модулі)
  4. `init.h`, `role.h` (мета)
  5. `radio.h` (LoRa API)
- `#pragma once` — на початку файлу.
- `#ifdef __cplusplus extern "C" { ... } #endif` — для заголовків, які
  можуть включатись з C++ (наприклад, шрифти, OLED, I2C).

---

## 6. Змінні / буфери

- `Buffer[5]`, `BufferSize` — глобальні `uint8_t`/`uint16_t`, оголошені в
  кожному `main_*.c` окремо (не в заголовку!). Це свідомо дублюється, бо
  main-файли не включають один одного.
- Без великих локальних масивів (RAM 16 КБ).
- Стан — через `volatile State` і FSM, без RTOS-черг.

---

## 7. Переривання (ISR)

- Handler має те саме ім'я, що й у `startup_cm4.S` (бо .S визначає лише
  weak alias).
- Мінімум логіки в ISR: лише встановити `volatile`-прапор, у крайньому
  випадку — `tick++` / `state = ...`.
- Не викликай `printf` з ISR — не reentrant.

Приклад:
```c
void GPIO_IRQHandler(void) {
    if (gpio_get_interrupt_status(GPIOA, GPIO_PIN_11) == SET) {
        gpio_clear_interrupt(GPIOA, GPIO_PIN_11);
        activity_trigged = true;
    }
}
```

---

## 8. Makefile

- Табуляція для команд рецепту.
- `EXCLUDES := src/main_TX.c src/main_RX.c` — спеціально виключає обидва
  файли, щоб додати лише один через `src/main_$(ROLE).c`. Не змінюй без
  потреби.
- `$(BUILD_DIR)/%.o: %.c` — авто-створення каталогів через `mkdir -p $(dir $@)`.
- `@` на початку рядка — щоб прибрати echo (окрім рецепта `flash`).

---

## 9. Git / коміти

**Не коміть** без явного прохання користувача (`NEVER commit changes unless
the user explicitly asks you to`). Стиль повідомлень — українською, у стилі
наявних комітів (`Шозанах? Перестала прошиватись.`).

---

## 10. Перевірка перед комітом (якщо прохання було)

1. `make clean && ROLE=TX make` — переконатись, що TX збирається.
2. `make clean && ROLE=RX make` — переконатись, що RX збирається.
3. Перевірити розмір `.elf` / `.bin` (Flash ≤ 128 КБ, RAM ≤ 16 КБ — див.
   вивід `-Wl,--print-memory-usage`).
4. Якщо змінено ABI — повідомити користувача.
