/*
   notamesh - IR controlled 'synchronized' led lighting effects using FastLED across multiple Arduino controllers.

         By: Andrew Tuline
       Date: October, 2018
        URL: www.tuline.com
      Email: atuline@gmail.com
       Gist: https://gist.github.com/atuline
     GitHub: https://github.com/atuline
   Pastebin: http://pastebin.com/u/atuline
    Youtube: https://www.youtube.com/user/atuline/videos
*/

// Переделка и дополнение
// весна 2021
// Волковыский Борис
// boris@volkovyski.ru
// Добавлено управление с помощью 5-ти кнопок на основе GyverButton
// Добавлено отображение режима работы на индикаторе
// Инфракрасный пульт не помещается в память и выпилен
// Аналоговые кнопки тоже выпилены (обычных достаточно)
// Приводятся в порядок комментарии по коду

// Если вы прошили вашу Arduino, все работет, а потом при повторной прошивке IDE ругается, прошейте блинком (blink) и потом уже боевой прошивкой. 
// Я не уверен, но похоже работа программы не оставляет ресурсов на обратную связь при прошивке. 
// Буду рад если напишете мне прав я или нет и в чем на самом деле дело.

//  Переделка и дополнение
//  Декабрь 2018
//  Бикин Дмитрий
//  d.bikin@bk.ru
//  SysWork.ru
//  Как подключить пульт
//  https://mysku.ru/blog/aliexpress/68990.html

// Добавлено управление одной кнопкой, декабрь 2019
// AlexGyver
// https://AlexGyver.ru/
// https://www.youtube.com/c/alexgyvershow/
// 1.1 - исправлено невключение предыдущего режима

//=============================================================================================================================================================
//               ПАРАМЕТРЫ
//=============================================================================================================================================================

/////////////////////////////////////////////////////////////////////////////////
///                               Параметры индикатора
/////////////////////////////////////////////////////////////////////////////////
#define USE_INDICATOR 1   // использовать индикатор

// Настройка пинов подключения индикатора
#define CLK_PIN 4
#define DIO_PIN 5

/////////////////////////////////////////////////////////////////////////////////
///                               Параметры кнопок
/////////////////////////////////////////////////////////////////////////////////

// Настройка действия по кнопке осуществляется в файле command.h
#define USE_BTN 1   // использовать управление кнопками (1 - да, 0 - нет)

#if USE_BTN == 1
  // Настройка пинов подключения кнопок
  #define BTN_PIN_1 6   // пин кнопки 1
  #define BTN_PIN_2 7   // пин кнопки 2
  #define BTN_PIN_3 8   // пин кнопки 3
  #define BTN_PIN_4 9   // пин кнопки 4
  #define BTN_PIN_5 10  // пин кнопки 5
#endif

/////////////////////////////////////////////////////////////////////////////////
///                               Параметры гирлянды
/////////////////////////////////////////////////////////////////////////////////
#define KOL_LED 100       // Сколько светодиодов в гирлянде при первом включении

#define CHIPSET       WS2811     // Тип микросхемы светодиодов в гирлянде (установка WS2812B для ленты WS2811 тоже работает)
#define LED_DT        2          // Номер пина куда подключена гирлянда
//#define LED_CK      11         // Номер пина для подключения тактов, применяется для светодиодов WS2801 или APA102
                                 // ЕСЛИ НЕ ИСПОЛЬЗУЕТСЯ ЗАКОМЕНТИРОВАТЬ

#define COLOR_ORDER   GRB        // Очередность цвета светодиодов  RGB или GRB

#define POWER_V       5          // напряжение блока питания в Вольтах
#define POWER_I       5000       // Ток блока питания в милиАмперах


/////////////////////////////////////////////////////////////////////////////////
///                               Параметры вывода отладочного журнала
/////////////////////////////////////////////////////////////////////////////////
#define LOG_ON        1           // Включить лог  1 - включить лог


/////////////////////////////////////////////////////////////////////////////////
///                               Параметры EEPROM
/////////////////////////////////////////////////////////////////////////////////
#define USE_EEPROM   0                                         // Использовать EEPROM для хранения настроек (1 - да, 0 - нет)

#if USE_EEPROM == 1
  // Подключение библиотеки для работы с EEPROM
  #include "EEPROM.h"     

  // EEPROM location definitions.
  #define STARTMODE 0                                           // EEPROM location for the starting mode.
  #define STRANDLEN 1                                           // EEPROM location for the actual Length of the strand, which is < MAX_LEDS
  #define STRANDEL  3                                           // EEPROM location for the mesh delay value.
  #define ISINIT    4                                           // EEPROM location used to verify that this Arduino has been initialized

#endif


/////////////////////////////////////////////////////////////////////////////////
///                               Параметры эффектов
/////////////////////////////////////////////////////////////////////////////////
#define BLACKSTART    0           // Первый запуск делать с черного экрана 0- начинать с эффекта, 1- начинать с черного экрана

#define GLITTER_ON     1           // Включить блеск 0 - блеск отключен, 1- блеск включен

#define CANDLE_KOL    3           // если больше 0 то будут включена имитация свечей
                                  // число задает через какое расстояние друг от друга располагать свечи
                                  
#define CANDLE_ON     0           // Если включена имитация свечей то 1 = при запуске она будет включена
                                  // 0 = при запуске она будет выключена

#define CANDLE_40     1           // Управление 40 эффектом (свечи)   1 = при запуске она будет включена по варианту efimova1969
                                  // иначе при запуске она будет включена по моему варианту

#define BACKGR_ON     1           // Включить заполнение черного цвета фоном

#define DEMO_MODE     1           // Режим при включении
                                  // 0 - последняя запущенная программа
                                  // 1 - демо режим с последовательным перебором эффектов
                                  // 2 - демо режим с случайным перебором эффектов
                                  // 3 - демо режим с последовательным перебором выбранных эффектов
                                  // 4 - демо режим с случайным перебором выбранных эффектов

// Выбранные эффекты пишутся номера через запятую, можно писать и одинаковые что актуально в DEMO_MODE 3
// Если закоментировать, то демо 3 и 4 отключаются
#define MY_MODE       0,1,2,4,5,9,10,11,14,16,17,18,19,21,22,28,30,32,34,36,37,38

#define DEMO_TIME     30           // Время отображение эффекта в демо режиме в секундах

#define CHANGE_ON     1           // Включить плавную смену эффектов

#define CHANGE_TIME   7           // Время в секундах, за которое произойдет плавная смена эффектов

#define CHANGE_SPARK  1           // Эффект смены 0 - без эффекта, 1 - искры (бенгальский огонь) 2 - яркий свет
                                  // 3 - метеорит,    4 - случайный эффект (из 1-3)

#define DIRECT_TIME   20          // Через сколько секунд менять направление если 0 - не меняем

#define PALETTE_TIME  30          // Через сколько секунд менять палитру если 0 - не меняем


//=============================================================================================================================================================
//               НАСТРОЙКИ ПРОГРАММЫ
//=============================================================================================================================================================

#define qsubd(x, b)  ((x>b)?wavebright:0)                     // A digital unsigned subtraction macro. if result <0, then => 0. Otherwise, take on fixed value.
#define qsuba(x, b)  ((x>b)?x-b:0)                            // Unsigned subtraction macro. if result <0, then => 0.

#define NOTAMESH_VERSION 103                                  // Just a continuation of seirlight and previously aalight.

#define FASTLED_INTERNAL                                      // Don't show pragma message with verion number
#define FASTLED_HAS_CLOCKLESS
#include "FastLED.h"                                          // https://github.com/FastLED/FastLED

#if FASTLED_VERSION < 3001000
  #error "Requires FastLED 3.1 or later; check github for latest code."
#endif


#if ( USE_BTN == 1 )
  uint8_t  IR_New_Mode = 0;                                      //Выбор эффекта
  uint32_t IR_Time_Mode = 0;                                     //время последнего нажатия
#endif

// Serial definition
#define SERIAL_BAUDRATE 115200                                 // 57600 or 115200

// Fixed definitions cannot change on the fly.
#define MAX_LEDS  KOL_LED

// Initialize changeable global variables.
#if MAX_LEDS < 255
  uint8_t NUM_LEDS;                                          // Number of LED's we're actually using, and we can change this only the fly for the strand length.
  uint8_t KolLed;
#else
  uint16_t NUM_LEDS;                                         // Number of LED's we're actually using, and we can change this only the fly for the strand length.
  uint16_t KolLed;
#endif

int max_bright = 255;                                        // Overall brightness definition. It can be changed on the fly.

struct CRGB leds[MAX_LEDS];                                  // Initialize our LED array. We'll be using less in operation.

CRGBPalette16 gCurrentPalette;                               // Use palettes instead of direct CHSV or CRGB assignments
CRGBPalette16 gTargetPalette;                                // Also support smooth palette transitioning
CRGB solid = CRGB::Black;                                    // Цвет закраски


extern const TProgmemRGBGradientPalettePtr gGradientPalettes[]; // These are for the fixed palettes in gradient_palettes.h
extern const uint8_t gGradientPaletteCount;                     // Total number of fixed palettes to display.
uint8_t gCurrentPaletteNumber = 0;                              // Current palette number from the 'playlist' of color palettes
uint8_t currentPatternIndex = 0;                                // Index number of which pattern is current
uint32_t demo_time = 0;                                         // время демо режима

TBlendType currentBlending = LINEARBLEND;                     // NOBLEND or LINEARBLEND


#define INITVAL   0x55                                        // If this is the value in ISINIT, then the Arduino has been initialized. Startmode should be 0 and strandlength should be 
#define INITMODE  0                                           // Startmode is 0, which is black.
#define INITLEN   KOL_LED                                     // Start length LED's.
#define INITDEL   0                                           // Starting mesh delay value of the strand in milliseconds.

uint16_t meshdelay;                                           // Timer for the notamesh. Works with INITDEL.

uint8_t ledMode = 0;                                          // Номер текущего режима

#if CHANGE_ON == 1
  uint8_t newMode = 0;                                        // Номер нового режима
  #if MAX_LEDS < 255
    uint8_t StepMode = MAX_LEDS;                              // Текущий шаг перехода от нового режима до старого
  #else
    uint16_t StepMode = MAX_LEDS;                             // Текущий шаг перехода от нового режима до старого
  #endif
#endif

uint8_t demorun = DEMO_MODE;                                    // 0 = regular mode, 1 = demo mode, 2 = shuffle mode.
#define maxMode  41                                           // Maximum number of modes.

uint8_t Protocol = 0;                                         // Temporary variables to save latest IR input
uint32_t Command = 0;

//==============================================================================================================
//       Общие переменные
//==============================================================================================================
uint8_t allfreq = 32;                                         // Меняет частоту. Переменная для эффектов one_sin_pal и two_sin.
uint8_t bgclr = 0;                                            // Общий цвет фона. Переменная для эффектов matrix_pal и one_sin_pal.
uint8_t bgbri = 0;                                            // Общая фоновая яркость. Переменная для эффектов matrix_pal и one_sin_pal.
bool    glitter = GLITTER_ON;                                  // Флаг включения блеска
bool    background = BACKGR_ON;                               // Флаг включения заполнения фона
#if CANDLE_KOL >0
  bool    candle = CANDLE_ON;                                 // Флаг включения свечей
#endif
uint8_t palchg = 3;                                           // Управление палитрой  3 - менять палитру автоматически иначе нет
uint8_t startindex = 0;                                       // С какого цвета начинать. Переменная для эффектов one_sin_pal.
uint8_t thisbeat;                                             // Переменная для эффектов juggle_pal.
uint8_t thiscutoff = 192;                                     // Если яркость ниже этой, то яркость = 0. Переменная для эффектов one_sin_pal и two_sin.
uint16_t thisdelay = 0;                                       // Задержка delay
uint8_t thisdiff = 1;                                         // Шаг изменения палитры. Переменная для эффектов confetti_pal, juggle_pal и rainbow_march.
int8_t  thisdir = 1;                                          // Направление движения эффекта. принимает значение -1 или 1.
uint8_t thisfade = 224;                                       // Скорость затухания. Переменная для эффектов confetti_pal и juggle_pal.
uint8_t thishue = 0;                                          // Оттенок Переменная для эффектов two_sin.
uint8_t thisindex = 0;                                        // Указатель на элемент палитры
uint8_t thisinc = 1;                                          // Изменение начального цвета после каждого прохода. Переменная для эффектов confetti_pal и one_sin_pal.
int     thisphase = 0;                                        // Изменение фазы. Переменная для эффектов one_sin_pal, plasma и two_sin.
uint8_t thisrot = 1;                                          // Изменение скорости вращения волны. В настоящее время 0.
int8_t  thisspeed = 4;                                        // Изменение стандартной скорости
uint8_t wavebright = 255;                                     // Вы можете изменить яркость волн / полос, катящихся по экрану.

#if MY_MODE
  const PROGMEM uint8_t my_mode[] = { MY_MODE };            //массив выбранных режимов
  const uint8_t my_mode_count = sizeof( my_mode );          //количество выбрано режимов
  uint8_t tek_my_mode = 0;                                  //Указатель на текущий режим
#endif

#if CHANGE_SPARK == 4
  uint8_t rand_spark = 0;
#endif

// TODO: похоже переменная summ нигде не используется и ее можно удалить
//long summ = 0;

void strobe_mode(uint8_t newMode, bool mc);
void bootme();
void meshwait();
void getirl();
void demo_check();

//==============================================================================================================
//       Display functions
//==============================================================================================================
// Support functions
#include "addings.h"

// Display routines
#include "confetti_pal.h"
#include "gradient_palettes.h"
#include "juggle_pal.h"
#include "matrix_pal.h"
#include "noise16_pal.h"
#include "noise8_pal.h"
#include "one_sin_pal.h"
#include "plasma.h"
#include "rainbow_march.h"
#include "rainbow_beat.h"
#include "serendipitous_pal.h"
#include "three_sin_pal.h"
#include "two_sin.h"
#include "blendwave.h"
#include "fire.h"
#include "candles.h"
#include "colorwave.h"


//==============================================================================================================
//       Подключение определений команд управления гирляндой
//==============================================================================================================
#include "commands.h"            
#include "getirl.h"                             


//==============================================================================================================
//       Инициализация кнопок
//==============================================================================================================
#if USE_BTN == 1
  #include "GyverButton.h"
  GButton btn1(BTN_PIN_1);
  GButton btn2(BTN_PIN_2);
  GButton btn3(BTN_PIN_3);
  GButton btn4(BTN_PIN_4);
  GButton btn5(BTN_PIN_5);
#endif


//==============================================================================================================
//       Инициализация индикатора
//==============================================================================================================
#if USE_INDICATOR == 1
  #include "2indicator.h"
#endif



//=============================================================================================================================================================
//               ОСНОВНОЙ КОД
//=============================================================================================================================================================
/**
 * Инициализация программы
 * 
 * @return void
 */
void setup() {

#if USE_INDICATOR == 1
  show_welcome_animation();
#endif

#if LOG_ON == 1
  Serial.begin(SERIAL_BAUDRATE);                                                  // Setup serial baud rate
  Serial.println(F(" ")); Serial.println(F("---SETTING UP---"));
#endif

  delay(1000);                                                                    // Slow startup so we can re-upload in the case of errors.

  LEDS.setBrightness(max_bright);                                                 // Set the generic maximum brightness value.

#if LED_CK
  LEDS.addLeds<CHIPSET, LED_DT, LED_CK, COLOR_ORDER>(leds, MAX_LEDS);
#else
  LEDS.addLeds<CHIPSET, LED_DT, COLOR_ORDER >(leds, MAX_LEDS);                   //Для светодиодов WS2812B
#endif
  
set_max_power_in_volts_and_milliamps(POWER_V, POWER_I);                         //Настройка блока питания

random16_set_seed(4832);                                                        // Awesome randomizer of awesomeness
random16_add_entropy(analogRead(2));

ledMode = INITMODE;
NUM_LEDS = KOL_LED;
meshdelay = INITDEL;

#if LOG_ON == 1
  Serial.print(F("Initial delay: ")); Serial.print(meshdelay * 100); Serial.println(F("ms delay."));
  Serial.print(F("Initial strand length: ")); Serial.print(NUM_LEDS); Serial.println(F(" LEDs"));
#endif

#if BLACKSTART == 1
  solid = CRGB::Black;                 //Запуск с пустого поля
  newMode = ledMode;
  ledMode = 100;
  StepMode = 1;
#else
#if MY_MODE
  switch (demorun)  {
    case 3:   ledMode = pgm_read_byte(my_mode + tek_my_mode); break;            // демо 3
    case 4:   ledMode = pgm_read_byte(my_mode + random8(my_mode_count)); break;   // демо 4
  }
#endif
#endif
  gCurrentPalette = gGradientPalettes[0];
  gTargetPalette = gGradientPalettes[0];
  strobe_mode(ledMode, 1);                                                        // Initialize the first sequence

#if LOG_ON == 1
  if (DEMO_MODE) {
    Serial.print(F("DEMO MODE ")); Serial.println(DEMO_MODE);
  }
  Serial.println(F("---SETUP COMPLETE---"));
#endif
} // setup()


bool onFlag = true;

/**
 * Главный цикл программы
 * 
 * @return void
 */
void loop() {
#if (USE_BTN == 1)
  static bool stepFlag = false;
  static bool brightDir = true;

  // Опрос кнопок
  btn1.tick();
  btn2.tick();
  btn3.tick();
  btn4.tick();
  btn5.tick();

  // Включение/Выключение
  if (btn1.isSingle()) {
    onFlag = !onFlag;
    FastLED.setBrightness(onFlag ? max_bright : 0);
    FastLED.show();
  }

  // Следующий эффект
  if (btn2.isSingle()) {
#if MY_MODE
    if (++tek_my_mode >= (my_mode_count - 1)) tek_my_mode = 0;
    newMode = pgm_read_byte(my_mode + tek_my_mode);
#else
    if (++newMode >= maxMode) newMode = 0;
#endif
    SetMode(newMode);
  }

  // Предыдущий эффект
  if (btn3.isSingle()) {
#if MY_MODE
    if (--tek_my_mode <= 0) tek_my_mode = my_mode_count - 1;
    newMode = pgm_read_byte(my_mode + tek_my_mode);
#else
    if (--newMode <= 0) newMode = maxMode;
#endif
    SetMode(newMode);
  }

  // Включение вспышек
  if (btn4.isSingle()) {
    glitter = !glitter;
  }

    // Управление яркостью
  if (stepFlag && btn5.isRelease()) {
    stepFlag = false;
    brightDir = !brightDir;
  }

  if (btn5.isStep()) {
    stepFlag = true;
    max_bright += (brightDir ? 20 : -20);
    max_bright = constrain(max_bright, 0, 255);
    FastLED.setBrightness(max_bright);
  }
#endif

#if ( USE_BTN == 1 )
  getirl();                                                                   // Обработка команд с пульта и аналоговых кнопок
#endif

  if (onFlag) {
    demo_check();                                                                 // Работа если включен демонстрационный режим

    EVERY_N_MILLISECONDS(50) {                                                  // Плавный переход от одной палитры в другую
      uint8_t maxChanges = 24;
#if CHANGE_ON == 1
      if (StepMode == MAX_LEDS )
#endif
        nblendPaletteTowardPalette(gCurrentPalette, gTargetPalette, maxChanges);
    }

#if PALETTE_TIME>0
    if (palchg) {
      EVERY_N_SECONDS(PALETTE_TIME) {                                            // Смена палитры
        if (palchg == 3) {
          if (gCurrentPaletteNumber < (gGradientPaletteCount - 1))  gCurrentPaletteNumber++;
          else                                                    gCurrentPaletteNumber = 0;
#if LOG_ON == 1
          Serial.print(F("New Palette: "));  Serial.println(gCurrentPaletteNumber);
#endif
        }
        gTargetPalette = gGradientPalettes[gCurrentPaletteNumber];                // We're just ensuring that the gTargetPalette WILL be assigned.
      }
    }
#endif

#if DIRECT_TIME > 0
    EVERY_N_SECONDS(DIRECT_TIME) {                                            // Меняем направление
      thisdir = thisdir * -1;
    }
#endif

    EVERY_N_MILLIS_I(thistimer, thisdelay) {                                    // Sets the original delay time.
      thistimer.setPeriod(thisdelay);                                           // This is how you update the delay value on the fly.
      KolLed = NUM_LEDS;                                                        // Выводим Эффект на все светодиоды
      strobe_mode(ledMode, 0);                                                  // отобразить эффект;
#if CHANGE_ON == 1
      if (StepMode < NUM_LEDS) {                                                // требуется наложить новый эффект
        KolLed = StepMode;                                                      // Выводим Эффект на все светодиоды
        if (StepMode > 10)  strobe_mode(newMode, 0);                            // отобразить эффект;
#if CHANGE_SPARK == 4
        sparkler(rand_spark);
#else
        sparkler(CHANGE_SPARK);                                                             // бенгальский огонь
#endif
      }
#endif
    }

#if CHANGE_ON == 1
    EVERY_N_MILLISECONDS(CHANGE_TIME * 1000 / NUM_LEDS) {                      // Движение плавной смены эффектов
      if ( StepMode < NUM_LEDS)
      { StepMode++;
        if (StepMode == 10) strobe_mode(newMode, 1);
        if (StepMode >= NUM_LEDS)
        { ledMode = newMode;
          StepMode = MAX_LEDS;
#if LOG_ON == 1
          Serial.println(F("End SetMode"));
#endif
        }
        nblendPaletteTowardPalette(gCurrentPalette, gTargetPalette, NUM_LEDS);
      }
    }
#endif

    if (glitter) addglitter(10);                                                // If the glitter flag is set, let's add some.
#if CANDLE_KOL >0
    if (candle)  addcandle();
#endif

    if (background) addbackground();                                            // Включить заполнение черного цвета фоном
  }

#if ( USE_BTN == 1 )
  if ( (IR_Time_Mode > 0) &&                                                //Идет отчет времени
       ((millis() - IR_Time_Mode) >= 2000 )  ) {                            //и прошло больше 2 секунд
    IR_Time_Mode = 0;
    if (IR_New_Mode <= maxMode) SetMode(IR_New_Mode);
    IR_New_Mode = 0;
  }
#endif

  static uint32_t showTimer = 0;
  if (onFlag && millis() - showTimer >= 10) {
    showTimer = millis();
    FastLED.show();
  }
} // loop()


/**
 * strobe_mode 
 * Выполнение шага анимации гирлянды
 * 
 * @param mode Режим анимации.
 * @param mc Флаг смены режима анимации. 0 - выполняем текущую анимацию, 1 - меняем режим
 * 
 * @return void
 */
void strobe_mode(uint8_t mode, bool mc) {

  if (mc) {
    fill_solid(leds, NUM_LEDS, CRGB(0, 0, 0));                // Clean up the array for the first time through. Don't show display though, so you may have a smooth transition.

    #if LOG_ON == 1
        Serial.print(F("Mode: ")); Serial.println(mode);
        Serial.print(F("Millis: ")); Serial.println(millis());
    #endif

    #if USE_INDICATOR == 1
        show_status(mode, glitter); 
    #endif

    #if PALETTE_TIME>0
        if (palchg == 0) palchg = 3;
    #else
        if (palchg == 0) palchg = 1;
    #endif
  }

  switch (mode) {                                          // First time through a new mode, so let's initialize the variables for a given display.

    case  0: if (mc) {
        thisdelay = 10;
        palchg = 0;
      } blendwave(); break;
    case  1: if (mc) {
        thisdelay = 10;
        palchg = 0;
      } rainbow_beat(); break;
    case  2: if (mc) {
        thisdelay = 10;
        allfreq = 2;
        thisspeed = 1;
        thatspeed = 2;
        thishue = 0;
        thathue = 128;
        thisdir = 1;
        thisrot = 1;
        thatrot = 1;
        thiscutoff = 128;
        thatcutoff = 192;
      } two_sin(); break;
    case  3: if (mc) {
        thisdelay = 20;
        allfreq = 4;
        bgclr = 0;
        bgbri = 0;
        startindex = 64;
        thisinc = 2;
        thiscutoff = 224;
        thisphase = 0;
        thiscutoff = 224;
        thisrot = 0;
        thisspeed = 4;
        wavebright = 255;
      } one_sin_pal(); break;
    case  4: if (mc) {
        thisdelay = 10;
      } noise8_pal(); break;
    case  5: if (mc) {
        thisdelay = 10;
        allfreq = 4;
        thisspeed = -1;
        thatspeed = 0;
        thishue = 64;
        thathue = 192;
        thisdir = 1;
        thisrot = 0;
        thatrot = 0;
        thiscutoff = 64;
        thatcutoff = 192;
      } two_sin(); break;
    case  6: if (mc) {
        thisdelay = 20;
        allfreq = 10;
        bgclr = 64;
        bgbri = 4;
        startindex = 64;
        thisinc = 2;
        thiscutoff = 224;
        thisphase = 0;
        thiscutoff = 224;
        thisrot = 0;
        thisspeed = 4;
        wavebright = 255;
      } one_sin_pal(); break;
    case  7: if (mc) {
        thisdelay = 10;
        numdots = 2;
        thisfade = 16;
        thisbeat = 8;
        thisdiff = 64;
      } juggle_pal(); break;
    case  8: if (mc) {
        thisdelay = 40;
        thisindex = 128;
        thisdir = 1;
        thisrot = 0;
        bgclr = 200;
        bgbri = 6;
      } matrix_pal(); break;
    case  9: if (mc) {
        thisdelay = 10;
        allfreq = 6;
        thisspeed = 2;
        thatspeed = 3;
        thishue = 96;
        thathue = 224;
        thisdir = 1;
        thisrot = 0;
        thatrot = 0;
        thiscutoff = 64;
        thatcutoff = 64;
      } two_sin(); break;
    case 10: if (mc) {
        thisdelay = 20;
        allfreq = 16;
        bgclr = 0;
        bgbri = 0;
        startindex = 64;
        thisinc = 2;
        thiscutoff = 224;
        thisphase = 0;
        thiscutoff = 224;
        thisrot = 0;
        thisspeed = 4;
        wavebright = 255;
      } one_sin_pal(); break;
    case 11: if (mc) {
        thisdelay = 50;
        mul1 = 5;
        mul2 = 8;
        mul3 = 7;
      } three_sin_pal(); break;
    case 12: if (mc) {
        thisdelay = 10;
      } serendipitous_pal(); break;
    case 13: if (mc) {
        thisdelay = 20;
        allfreq = 8;
        bgclr = 0;
        bgbri = 4;
        startindex = 64;
        thisinc = 2;
        thiscutoff = 224;
        thisphase = 0;
        thiscutoff = 224;
        thisrot = 0;
        thisspeed = 4;
        wavebright = 255;
      } one_sin_pal(); break;
    case 14: if (mc) {
        thisdelay = 10;
        allfreq = 20;
        thisspeed = 2;
        thatspeed = -1;
        thishue = 24;
        thathue = 180;
        thisdir = 1;
        thisrot = 0;
        thatrot = 1;
        thiscutoff = 64;
        thatcutoff = 128;
      } two_sin(); break;
    case 15: if (mc) {
        thisdelay = 50;
        thisindex = 64;
        thisdir = -1;
        thisrot = 1;
        bgclr = 100;
        bgbri = 10;
      } matrix_pal(); break;
    case 16: if (mc) {
        thisdelay = 10;
      } noise8_pal(); break; // By: Andrew Tuline
    case 17: if (mc) {
        thisdelay = 10;
      } plasma(11, 23, 4, 18); break;
    case 18: if (mc) {
        thisdelay = 20;
        allfreq = 10;
        thisspeed = 1;
        thatspeed = -2;
        thishue = 48;
        thathue = 160;
        thisdir = -1;
        thisrot = 1;
        thatrot = -1;
        thiscutoff = 128;
        thatcutoff = 192;
      } two_sin(); break;
    case 19: if (mc) {
        thisdelay = 50;
        palchg = 0;
        thisdir = 1;
        thisrot = 1;
        thisdiff = 1;
      } rainbow_march(); break;
    case 20: if (mc) {
        thisdelay = 10;
        mul1 = 6;
        mul2 = 9;
        mul3 = 11;
      } three_sin_pal(); break;
    case 21: if (mc) {
        thisdelay = 10;
        palchg = 0;
        thisdir = 1;
        thisrot = 2;
        thisdiff = 10;
      } rainbow_march(); break;
    case 22: if (mc) {
        thisdelay = 20;
        palchg = 0;
        hxyinc = random16(1, 15);
        octaves = random16(1, 3);
        hue_octaves = random16(1, 5);
        hue_scale = random16(10, 50);
        x = random16();
        xscale = random16();
        hxy = random16();
        hue_time = random16();
        hue_speed = random16(1, 3);
        x_speed = random16(1, 30);
      } noise16_pal(); break;
    case 23: if (mc) {
        thisdelay = 20;
        allfreq = 6;
        bgclr = 0;
        bgbri = 0;
        startindex = 64;
        thisinc = 2;
        thiscutoff = 224;
        thisphase = 0;
        thiscutoff = 224;
        thisrot = 0;
        thisspeed = 4;
        wavebright = 255;
      } one_sin_pal(); break;
    case 24: if (mc) {
        thisdelay = 10;
      } plasma(23, 15, 6, 7); break;
    case 25: if (mc) {
        thisdelay = 20;
        thisinc = 1;
        thisfade = 2;
        thisdiff = 32;
      } confetti_pal(); break;
    case 26: if (mc) {
        thisdelay = 10;
        thisspeed = 2;
        thatspeed = 3;
        thishue = 96;
        thathue = 224;
        thisdir = 1;
        thisrot = 1;
        thatrot = 2;
        thiscutoff = 128;
        thatcutoff = 64;
      } two_sin(); break;
    case 27: if (mc) {
        thisdelay = 30;
        thisindex = 192;
        thisdir = -1;
        thisrot = 0;
        bgclr = 50;
        bgbri = 0;
      } matrix_pal(); break;
    case 28: if (mc) {
        thisdelay = 20;
        allfreq = 20;
        bgclr = 0;
        bgbri = 0;
        startindex = 64;
        thisinc = 2;
        thiscutoff = 224;
        thisphase = 0;
        thiscutoff = 224;
        thisrot = 0;
        thisspeed = 4;
        wavebright = 255;
      } one_sin_pal(); break;
    case 29: if (mc) {
        thisdelay = 20;
        thisinc = 2;
        thisfade = 8;
        thisdiff = 64;
      } confetti_pal(); break;
    case 30: if (mc) {
        thisdelay = 10;
      } plasma(8, 7, 9, 13); break;
    case 31: if (mc) {
        thisdelay = 10;
        numdots = 4;
        thisfade = 32;
        thisbeat = 12;
        thisdiff = 20;
      } juggle_pal(); break;
    case 32: if (mc) {
        thisdelay = 30;
        allfreq = 4;
        bgclr = 64;
        bgbri = 4;
        startindex = 64;
        thisinc = 2;
        thiscutoff = 224;
        thisphase = 0;
        thiscutoff = 128;
        thisrot = 1;
        thisspeed = 8;
        wavebright = 255;
      } one_sin_pal(); break;
    case 33: if (mc) {
        thisdelay = 50;
        mul1 = 3;
        mul2 = 4;
        mul3 = 5;
      } three_sin_pal(); break;
    case 34: if (mc) {
        thisdelay = 10;
        palchg = 0;
        thisdir = -1;
        thisrot = 1;
        thisdiff = 5;
      } rainbow_march(); break;
    case 35: if (mc) {
        thisdelay = 10;
      } plasma(11, 17, 20, 23); break;
    case 36: if (mc) {
        thisdelay = 20;
        thisinc = 1;
        thisfade = 1;
      } confetti_pal(); break;
    case 37: if (mc) {
        thisdelay = 20;
        palchg = 0;
        octaves = 1;
        hue_octaves = 2;
        hxy = 6000;
        x = 5000;
        xscale = 3000;
        hue_scale = 50;
        hue_speed = 15;
        x_speed = 100;
      } noise16_pal(); break;
    case 38: if (mc) {
        thisdelay = 10;
      } noise8_pal(); break;
    case 39: if (mc) {
        thisdelay = 10;
        palchg = 0;
      } fire(); break;
    case 40: if (mc) {
        thisdelay = 10;
        palchg = 0;
      } candles(); break;
    case 41: if (mc) {
        thisdelay = 10;
      } colorwaves(); break;
    case 100: if (mc) {
        palchg = 0;
      } fill_solid(leds, NUM_LEDS,  solid); break;    //Установить цвет
    case 200: fill_solid(leds, MAX_LEDS, CRGB::Black); fill_solid(leds, NUM_LEDS, CRGB(255, 255, 255)); break; //Зажечь гирлянду длинной NUM_LEDS (регулировка длинны гирлянды)
    case 201: fill_solid(leds, MAX_LEDS, CRGB::Black); fill_solid(leds, meshdelay, CRGB(255, 255, 255)); break; //Зажечь гирлянду длинной meshdelay
    default : ledMode = 0;  break;        //нет такого режима принудительно ставим нулевой

  } // switch mode

#if LOG_ON == 1
  if (mc) {
    if ( palchg == 0 ) Serial.println(F("Change palette off"));
    else if ( palchg == 1 ) Serial.println(F("Change palette Stop"));
    else if ( palchg == 3 ) Serial.println(F("Change palette ON"));
  }
#endif

} // strobe_mode()

void demo_check() {

  if (demorun) {
    if ((millis() - demo_time) >= (DEMO_TIME * 1000L )) {         //Наступило время смены эффекта
      demo_time = millis();                                       //Запомним время
      gCurrentPaletteNumber = random8(0, gGradientPaletteCount);  //Случайно поменяем палитру
#if CHANGE_ON == 1
      switch (demorun)  {
        case 2:   newMode = random8(0, maxMode);                // демо 2
          break;
#if MY_MODE
        case 3:   if (tek_my_mode >= (my_mode_count - 1)) tek_my_mode = 0; // демо 3
          else tek_my_mode++;
          newMode = pgm_read_byte(my_mode + tek_my_mode);
          break;
        case 4:   newMode = pgm_read_byte(my_mode + random8(my_mode_count));  // демо 4
          break;
#endif
        default : if (newMode >= maxMode) newMode = 0;          // демо 1
          else newMode++;
          break;
      }
      StepMode = 1;
#if CHANGE_SPARK == 4
      rand_spark = random8(3) + 1;
#endif

#if LOG_ON == 1
      Serial.println(F("Start SetMode"));
#endif
#else
      gTargetPalette = gGradientPalettes[gCurrentPaletteNumber];  //Применим палитру
#if LOG_ON == 1
      Serial.print(F("New Palette: "));  Serial.println(gCurrentPaletteNumber);
#endif
      switch (demorun)  {
        case 2:   ledMode = random8(0, maxMode);                // демо 2
          break;
#if MY_MODE
        case 3:   if (tek_my_mode >= (my_mode_count - 1)) tek_my_mode = 0; // демо 3
          else tek_my_mode++;
          ledMode = pgm_read_byte(my_mode + tek_my_mode);
          break;
        case 4:   ledMode = pgm_read_byte(my_mode + random8(my_mode_count));  // демо 4
          break;
#endif
        default : if (ledMode >= maxMode) ledMode = 0;          // демо 1
          else ledMode++;
          break;
      }
      strobe_mode(ledMode, 1);                                // Does NOT reset to 0.
#if CANDLE_KOL >0
      PolCandle = random8(CANDLE_KOL);
#endif
#endif
    } // if lastSecond
  } // if demorun

} // demo_check()
