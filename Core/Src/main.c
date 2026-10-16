/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "iwdg.h"
#include "stm32f1xx_hal_iwdg.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "OLED.h"
#include <stdint.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct {
    uint32_t power_count;
    uint32_t iwdg_count;
    uint32_t wwdg_count;
} ResetCounts;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define FLASH_MAGIC_ADDR   0x0800FC00
#define FLASH_POWER_ADDR   0x0800FC04
#define FLASH_IWDG_ADDR    0x0800FC08
#define FLASH_WWDG_ADDR    0x0800FC0C
#define FLASH_MAGIC        0x12345678
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint32_t reset_flags;
uint32_t reset_flags_after_clear;

uint8_t is_iwdg_reset;
uint8_t is_wwdg_reset; 
ResetCounts counts;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void Flash_ReadCounts(ResetCounts *counts);
void Flash_SaveCounts(ResetCounts *counts);


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  reset_flags = RCC->CSR;
  

  // 检查复位标志
  is_iwdg_reset = (reset_flags & RCC_CSR_IWDGRSTF) ? 1 : 0;
  is_wwdg_reset = (reset_flags & RCC_CSR_WWDGRSTF) ? 1 : 0;
  
  __HAL_RCC_CLEAR_RESET_FLAGS();//把 RCC 中记录的复位标志全部清掉。
  reset_flags_after_clear = RCC->CSR;

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  
  /* USER CODE BEGIN 2 */
  
 
  
  

   
  
  // 软件 I2C 的 OLED：SCL/SDA 引脚在 OLED_Init() 里配置，必须先调用它屏幕才会亮
  OLED_Init();
  OLED_Clear();
  HAL_Delay(100);

  Flash_ReadCounts(&counts);

  if(is_iwdg_reset)
  {
    counts.iwdg_count++;
  }
  else if(is_wwdg_reset)
  {
    counts.wwdg_count++;
  }
  else if(reset_flags & RCC_CSR_PORRSTF)
  {
    counts.power_count++;
  }
  
  
 
  Flash_SaveCounts(&counts);
  OLED_ShowString(1,1,"POWER:");
  OLED_ShowNum(1,8,counts.power_count,5);
  OLED_ShowString(2,1,"IWDG:");
  OLED_ShowNum(2,8,counts.iwdg_count,5);
  OLED_ShowString(3,1,"WWDG:");
  OLED_ShowNum(3,8,counts.wwdg_count,5);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  MX_IWDG_Init();
  while (1)
  {
    /* USER CODE END WHILE */
    
    /* USER CODE BEGIN 3 */
    HAL_IWDG_Refresh( &hiwdg);
    HAL_Delay(300);
  }
  /* USER CODE END 3 */
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void Flash_ReadCounts(ResetCounts *counts)
{
  uint32_t magic ;
  magic = *(const uint32_t *)FLASH_MAGIC_ADDR;
  if (magic == FLASH_MAGIC)
  {
    counts->power_count = *(const uint32_t *)FLASH_POWER_ADDR;
    counts->iwdg_count = *(const uint32_t *)FLASH_IWDG_ADDR;
    counts->wwdg_count = *(const uint32_t *)FLASH_WWDG_ADDR;
  }
  else
  {
    // 如果不匹配，说明数据无效，初始化为0
    counts->power_count = 0;
    counts->iwdg_count = 0;
    counts->wwdg_count = 0;
  }

}
void Flash_SaveCounts(ResetCounts *counts)
{
  FLASH_EraseInitTypeDef EraseInitStruct;
  uint32_t PageError = 0;

  // FLASH_MAGIC_ADDR = 0x0800FC00 是 1KB 页大小器件的最后一页起始地址（第 63 页），
  // 4 个变量都在这一页内，所以只擦 1 页即可；也避免擦到前面的程序代码。
  EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.PageAddress = FLASH_MAGIC_ADDR;
  EraseInitStruct.NbPages = 1;

  HAL_FLASH_Unlock();
  HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);

  HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_MAGIC_ADDR, FLASH_MAGIC);
  HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_POWER_ADDR, counts->power_count);
  HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_IWDG_ADDR, counts->iwdg_count);
  HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_WWDG_ADDR, counts->wwdg_count);
  HAL_FLASH_Lock();
} 
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
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
