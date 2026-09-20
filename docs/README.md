# docs/ — Індекс документації

Цей каталог містить детальнішу документацію, на яку посилається
[AGENTS.md](../AGENTS.md). Усі нові агенти мають прочитати файли в такому
порядку:

| Файл                                          | Що там                                                       |
|-----------------------------------------------|--------------------------------------------------------------|
| [PROJECT_OVERVIEW.md](PROJECT_OVERVIEW.md)     | Архітектура, FSM, потік даних TX/RX, формат пейлоада        |
| [HARDWARE.md](HARDWARE.md)                     | SoC ASR Tremo, розпіновка RA-08, ADXL345, OLED, SX1262      |
| [CODING_CONVENTIONS.md](CODING_CONVENTIONS.md) | Стиль C, ідіоми проєкту, найменування, Makefile             |
| [BUILD_AND_FLASH.md](BUILD_AND_FLASH.md)       | Toolchain, збірка, прошивка через UART, типові проблеми      |

## Швидкий старт

```bash
# Прочитати перед першою зміною
cat AGENTS.md
cat docs/PROJECT_OVERVIEW.md
cat docs/HARDWARE.md

# Зібрати
make clean && ROLE=TX make && ROLE=RX make

# Прошити
make clean && PORT=/dev/ttyUSB0 ROLE=TX make flash
```

## Як додати нову документацію

1. Створи файл у `docs/`.
2. Додай посилання в цей індекс.
3. Додай коротке посилання в `AGENTS.md` (розділ «Додаткова документація»).
