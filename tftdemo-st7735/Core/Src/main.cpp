/* Includes ------------------------------------------------------------------*/
#include <math.h>
#include "main.h"
#include "cmsis_os.h"
#include "led-blackpill.hpp"
#include "tft-st7735-blackpill.hpp"

/* Private includes ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;

/* Definitions for Application Tasks */
osThreadId_t blinkyTaskHandle;
const osThreadAttr_t blinkyTask_attributes = {
    .name = "blinkyTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t) osPriorityNormal,
};
osThreadId_t demoTaskHandle;
const osThreadAttr_t demoTask_attributes = {
    .name = "demoTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI1_Init(void);
void BlinkyTask(void *argument);
void DemoTask(void *argument);

/* Private user code ---------------------------------------------------------*/

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* Configure the system clock */
    SystemClock_Config();

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_SPI1_Init();

    /* Init scheduler */
    osKernelInitialize();

    /* Create the thread(s) */
    blinkyTaskHandle = osThreadNew(BlinkyTask, NULL, &blinkyTask_attributes);
    demoTaskHandle = osThreadNew(DemoTask, NULL, &demoTask_attributes);

    /* Start scheduler */
    osKernelStart();

    /* We should never get here as control is now taken by the scheduler */

    /* Infinite loop */
    while (1);
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Initializes the RCC Oscillators according to the specified parameters
    * in the RCC_OscInitTypeDef structure.
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        Error_Handler();
    }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 100000;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
        Error_Handler();
    }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{
    /* SPI1 parameter configuration*/
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 10;
    if (HAL_SPI_Init(&hspi1) != HAL_OK) {
        Error_Handler();
    }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
}

static void demo_text(tft *display)
{
    display->clear(RGB565_BLACK);

    display->draw_string(5, 5, "STM32F103C8", RGB565_WHITE, RGB565_BLACK, &Font11x18);
    display->draw_string(10, 28, "Black Pill", RGB565_CYAN, RGB565_BLACK);
    display->draw_string(5, 51, "ST7735 TFT", RGB565_YELLOW, RGB565_BLACK, &Font8x8);
    display->draw_string(35, 74, "HAL SPI", RGB565_GREEN, RGB565_BLACK, &Font7x10);
    display->draw_string(20, 97, "Graphics!", RGB565_MAGENTA, RGB565_BLACK);

    HAL_Delay(2000);
}

static void demo_fillScreen(tft *tft)
{
    tft->clear(RGB565_BLACK);
    tft->draw_string(10, 10, "Fill Screen", RGB565_WHITE, RGB565_BLACK);

    HAL_Delay(500);
    tft->clear(RGB565_RED);
    HAL_Delay(500);
    tft->clear(RGB565_GREEN);
    HAL_Delay(500);
    tft->clear(RGB565_BLUE);
    HAL_Delay(500);
    tft->clear(RGB565_YELLOW);
    HAL_Delay(500);
    tft->clear(RGB565_CYAN);
    HAL_Delay(500);
    tft->clear(RGB565_MAGENTA);
    HAL_Delay(500);
}

static void demo_lines(tft *tft)
{
    tft->clear(RGB565_BLACK);
    tft->draw_string(10, 10, "Lines", RGB565_WHITE, RGB565_BLACK);

    uint16_t width = tft->width(), height = tft->height();

    // Horizontal lines
    for (int y = 0; y < height; y += 8) {
        tft->line(0, y, width-1, y, RGB565_RED);
    }
    HAL_Delay(1000);

    tft->clear(RGB565_BLACK);
    // Vertical lines
    for (int x = 0; x < width; x += 8) {
        tft->line(x, 0, x, height-1, RGB565_GREEN);
    }
    HAL_Delay(1000);

    // Diagonal lines from corners
    tft->clear(RGB565_BLACK);
    for (int i = 0; i < width; i += 8) {
        tft->line(0, 0, i, height-1, RGB565_BLUE);
        tft->line(width-1, 0, width-1-i, height-1, RGB565_YELLOW);
    }
    for (int i = 0; i < height; i += 8) {
        tft->line(0, 0, width-1, i, RGB565_CYAN);
        tft->line(width-1, 0, 0, height-1-i, RGB565_MAGENTA);
    }
    HAL_Delay(1500);
}

static void demo_rectangles(tft *tft)
{
    tft->clear(RGB565_BLACK);
    tft->draw_string(10, 10, "Rectangles", RGB565_WHITE, RGB565_BLACK);

    uint16_t width = tft->width(), height = tft->height();

    // Concentric rectangles
    for (int i = 0; i < 40; i += 4) {
        uint16_t color = RGB565(i*6, 255-i*6, i*3);
        tft->rect(i, i, width-1-2*i, height-1-2*i, color);
    }
    HAL_Delay(1500);

    // Filled rectangles grid
    tft->clear(RGB565_BLACK);
    uint16_t colors[] = {RGB565_RED, RGB565_GREEN, RGB565_BLUE,
                         RGB565_YELLOW, RGB565_CYAN, RGB565_MAGENTA,
                         RGB565_ORANGE, RGB565_PURPLE, RGB565_PINK};

    int w = width / 3;
    int h = height / 3;

    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            tft->rect(col*w, row*h, w-2, h-2, colors[row*3+col]);
        }
    }
    HAL_Delay(1500);
}

static void demo_circles(tft *tft)
{
    tft->clear(RGB565_BLACK);
    tft->draw_string(10, 10, "Circles", RGB565_WHITE, RGB565_BLACK);

    uint16_t width = tft->width(), height = tft->height();

    // Concentric circles
    int centerX = tft->width() / 2;
    int centerY = tft->height() / 2;

    for (int r = 10; r < 60; r += 8) {
        uint16_t color = RGB565(r*4, 255-r*4, 128);
        tft->circle(centerX, centerY, r, color);
    }
    HAL_Delay(1500);

    // Filled circles pattern
    tft->clear(RGB565_BLACK);
    tft->fill(40, 40, 30, RGB565_RED);
    tft->fill(120, 40, 30, RGB565_GREEN);
    tft->fill(40, 88, 30, RGB565_BLUE);
    tft->fill(120, 88, 30, RGB565_YELLOW);
    tft->fill(80, 64, 25, RGB565_MAGENTA);
    HAL_Delay(1500);
}

static void demo_colorPalette(tft *tft)
{
    tft->clear(RGB565_BLACK);
    tft->draw_string(10, 10, "Color Palette", RGB565_WHITE, RGB565_BLACK);

    uint16_t width = tft->width(), height = tft->height();

    // RGB color bars
    int barHeight = height / 3;

    for (int x = 0; x < width; x++) {
        uint8_t intensity = (x * 255) / width;

        // Red gradient
        tft->line(x, 0, x, barHeight-1, RGB565(intensity, 0, 0));

        // Green gradient
        tft->line(x, barHeight, x, 2*barHeight-1, RGB565(0, intensity, 0));

        // Blue gradient
        tft->line(x, 2*barHeight, x, height-1, RGB565(0, 0, intensity));
    }

    HAL_Delay(2000);
}

static void demo_gradient(tft *tft)
{
    tft->clear(RGB565_BLACK);
    tft->draw_string(10, 10, "Gradient", RGB565_WHITE, RGB565_BLACK);

    uint16_t width = tft->width(), height = tft->height();

    // Horizontal gradient (red to blue)
    for (int x = 0; x < width; x++) {
        uint8_t red = 255 - (x * 255) / width;
        uint8_t blue = (x * 255) / width;
        uint16_t color = RGB565(red, 0, blue);
        tft->line(x, 0, x, height/2, color);
    }

    // Vertical gradient (green to yellow)
    for (int y = height/2; y < height; y++) {
        uint8_t red = ((y - height/2) * 255) / (height/2);
        uint16_t color = RGB565(red, 255, 0);
        tft->line(0, y, width-1, y, color);
    }

    HAL_Delay(2000);
}

static void demo_sinewave(tft *tft)
{
    tft->clear(RGB565_BLACK);
    tft->draw_string(10, 10, "Sine Wave", RGB565_WHITE, RGB565_BLACK);

    uint16_t width = tft->width(), height = tft->height();
    int prevX = 0, prevY = height/2;
    for (int x = 0; x < width; x++) {
        int y = height/2 + (int)(50 * sin(x * 3.14159 / 30));
        if (x > 0)
            tft->line(prevX, prevY, x, y, RGB565_YELLOW);
        prevX = x;
        prevY = y;
    }
    HAL_Delay(2000);
}

void DemoTask(void *argument)
{
    tft *tft = new tft_st7735_blackpill(&hspi1);
    while (1) {
        demo_text(tft);
        demo_fillScreen(tft);
        demo_lines(tft);
        demo_rectangles(tft);
        demo_circles(tft);
        demo_colorPalette(tft);
        demo_gradient(tft);
        demo_sinewave(tft);
    }
}

/**
  * @brief  Function implementing the blinkyTask thread.
  * @param  argument: Not used
  * @retval None
  */
void BlinkyTask(void *argument)
{
    int state = 0;
    Led *led = new Led_Stm32_BlackPill();
    while (1) {
        *led = (state ^= 1);
        HAL_Delay(1000);
    }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1) {
    }
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
      ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
}
#endif /* USE_FULL_ASSERT */
