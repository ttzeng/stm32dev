#include <math.h>
#include "main.h"
#include "cmsis_os.h"
#include "led-blackpill.hpp"
#include "sytft240-blackpill.hpp"

UART_HandleTypeDef huart1;

osThreadId_t BlinkyTaskHandle;
const osThreadAttr_t blinkyTask_attributes = {
    .name = "BlinkyTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t) osPriorityNormal,
};
void BlinkyTask(void *argument);

osThreadId_t DemoTaskHandle;
const osThreadAttr_t demoTask_attributes = {
    .name = "DemoTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t) osPriorityNormal,
};
void DemoTask(void *argument);

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);

void mdelay(uint32_t msec)
{
    vTaskDelay(msec / portTICK_PERIOD_MS);
}

int main(void)
{
    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* Configure the system clock */
    SystemClock_Config();

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_USART1_UART_Init();

    /* Init scheduler */
    osKernelInitialize();

    /* Create the thread(s) */
    BlinkyTaskHandle = osThreadNew(BlinkyTask, NULL, &blinkyTask_attributes);
    DemoTaskHandle = osThreadNew(DemoTask, NULL, &demoTask_attributes);

    /* Start scheduler */
    osKernelStart();

    /* We should never get here as control is now taken by the scheduler */

    /* Infinite loop */
    while (1) {
    }
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

    /* Initializes the CPU, AHB and APB buses clocks */
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
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart1) != HAL_OK) {
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
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
}

void BlinkyTask(void *argument)
{
    int state = 0;
    Led *led = new Led_Stm32_BlackPill();
    /* Infinite loop */
    while (1) {
        osDelay(1000);
        *led = (state ^= 1);
    }
}

static void DrawRandomLines(sytft240* tft, int count)
{
    tft->clear(COLOR_BLACK);
    tft->draw_string(10, 10, "Random Lines", COLOR_WHITE, COLOR_BLACK);

    uint16_t colors[] = {COLOR_RED, COLOR_GREEN, COLOR_BLUE, COLOR_YELLOW, 
                        COLOR_CYAN, COLOR_MAGENTA, COLOR_WHITE, COLOR_ORANGE};

    for (int i = 0; i < count; i++) {
        int x1 = rand() % 240;
        int y1 = 40 + rand() % 240;
        int x2 = rand() % 240;
        int y2 = 40 + rand() % 240;
        uint16_t color = colors[rand() % 8];

        tft->line(x1, y1, x2, y2, color);
        HAL_Delay(100);
    }
}

static void DrawDemo(sytft240* tft)
{
    // Demo 1: Basic lines
    tft->clear(COLOR_BLACK);
    tft->draw_string(10, 10, "Basic Lines", COLOR_WHITE, COLOR_BLACK);

    // Horizontal lines
    for (int i = 0; i < 10; i++) {
        tft->line(20, 40 + i * 10, 200, 40 + i * 10, COLOR_RED);
        HAL_Delay(100);
    }

    // Vertical lines
    for (int i = 0; i < 10; i++) {
        tft->line(20 + i * 18, 40, 20 + i * 18, 140, COLOR_GREEN);
        HAL_Delay(100);
    }

    // Diagonal lines
    for (int i = 0; i < 8; i++) {
        tft->line(20, 160 + i * 10, 200, 160 + i * 10, COLOR_BLUE);
        tft->line(20 + i * 22, 160, 20 + i * 22, 240, COLOR_YELLOW);
        HAL_Delay(150);
    }

    HAL_Delay(2000);

    // Demo 2: Line patterns
    tft->clear(COLOR_BLACK);
    tft->draw_string(10, 10, "Line Patterns", COLOR_WHITE, COLOR_BLACK);

    // Radial lines from center
    int centerX = 120, centerY = 180;
    for (int angle = 0; angle < 360; angle += 15) {
        float rad = angle * 3.14159 / 180.0;
        int x = centerX + (int)(80 * cos(rad));
        int y = centerY + (int)(80 * sin(rad));
        tft->line(centerX, centerY, x, y, COLOR_CYAN);
        HAL_Delay(100);
    }

    HAL_Delay(2000);

    // Demo 3: Grid pattern
    tft->clear(COLOR_BLACK);
    tft->draw_string(10, 10, "Grid Pattern", COLOR_WHITE, COLOR_BLACK);

    // Draw grid
    for (int x = 20; x <= 220; x += 20) {
        tft->line(x, 40, x, 280, COLOR_GRAY);
    }
    for (int y = 40; y <= 280; y += 20) {
        tft->line(20, y, 220, y, COLOR_GRAY);
    }

    HAL_Delay(2000);

    // Demo 4: Random lines
    DrawRandomLines(tft, 50);

    HAL_Delay(3000);

    // Demo 5: Box drawing
    tft->clear(COLOR_BLACK);
    tft->draw_string(10, 10, "Rectangles", COLOR_WHITE, COLOR_BLACK);

    for (int i = 0; i < 8; i++) {
        uint16_t color = (i % 2) ? COLOR_RED : COLOR_GREEN;
        tft->rect(30 + i * 15, 50 + i * 15, 160 - i * 15, 120 - i * 15, color);
        HAL_Delay(300);
    }

    HAL_Delay(2000);

    // Demo 6: Sine wave
    tft->clear(COLOR_BLACK);
    tft->draw_string(10, 10, "Sine Wave", COLOR_WHITE, COLOR_BLACK);

    int prevX = 0, prevY = 160;
    for (int x = 0; x < 240; x++) {
        int y = 160 + (int)(50 * sin(x * 3.14159 / 30));
        if (x > 0)
            tft->line(prevX, prevY, x, y, COLOR_YELLOW);
        prevX = x;
        prevY = y;
        HAL_Delay(20);
    }

    HAL_Delay(2000);
}

/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
void DemoTask(void *argument)
{
    sytft240 *tft = new sytft240_Stm32_BlackPill(mdelay);
    while (1) {
        DrawDemo(tft);
    }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
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
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
