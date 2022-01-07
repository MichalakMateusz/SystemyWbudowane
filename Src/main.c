/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stdlib.h"
#include "stdio.h"
/** @addtogroup STM32L0xx_HAL_Examples
  * @{
  */

/** @addtogroup Templates
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
typedef struct {
	uint8_t x;
	uint8_t y;
} Segment;
/* Private define ------------------------------------------------------------*/
#define N 3
#define M 8
#define LCD_HEIGHT 18
#define LCD_WIDTH 168
#define SQUARE_WIDTH LCD_WIDTH/M
#define SQUARE_HEIGHT LCD_HEIGHT/N
#define LINEAR_DETECT  ((MyLinRots[0].p_Data->StateId == TSL_STATEID_DETECT))
#define LINEAR_POSITION (MyLinRots[0].p_Data->Position)
#define LINEAR_IDLE (MyLinRots[0].p_Data->StateId == TSL_STATEID_RELEASE)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t numbers[M][N] = {{0}}; //tablica przechowujaca wygenerowane liczby, na konkretnych pozycjach na planszy gry
uint8_t checked[M][N] = {{0}}; //tablica przechowujaca sprawdzone liczby, na konkretnych pozycjach na planszy gry
Segment current_position; //obecna pozycja kursora
uint8_t game_state = 0; // stan gry, zmienna pomocnicza, mozna ogolnie powiedziec ze 0 - stan poczatkowy po ekranie startowym, 1 - stan poruszania kursorem po ekranie gry, 2 - stan generowania i rysowania liczb na ekranie, 3 - stan konca gry
uint8_t checkCounter = 0; // liczba zgadnietych liczb w danym poziomie, zmienna pomocnicza
uint8_t flag = 0; //flaga pomocnicza dla sensora dotyku
uint8_t level = 1; // poziom gry
uint8_t order [M*N][2] = {{M*N+1}}; //tablica przechowujaca pozycje wszystkich wygenerowanych liczb w odpowieddniej kolejnosci, inicjowana wspolrzednymi które nigdy nie beda osiagniete
uint32_t seed = 0; //ziarno losowosci
TSC_HandleTypeDef TscHandle; //sensor
/* Private function prototypes -----------------------------------------------*/
//funkcje opisane przy definicjach funkcji
static void SystemClock_Config(void);
static void Error_Handler(void);
void DrawHead(void);
void MoveUp(void);
void MoveDown(void);
void MoveLeft(void);
void MoveRight(void);
void GameOver(void);
void Process_Sensors(tsl_user_status_t status);
void GenerateNumbers(void);
void ShowNumbers(void);
void CheckDecision(void);
void YouWon(void);
void GenerateStartScreen(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
	tsl_user_status_t tsl_status;

  /* STM32L0xx HAL library initialization:
       - Configure the Flash prefetch, Flash preread and Buffer caches
       - Systick timer is configured by default as source of time base, but user 
             can eventually implement his proper time base source (a general purpose 
             timer for example or other time source), keeping in mind that Time base 
             duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and 
             handled in milliseconds basis.
       - Low Level Initialization
     */
  HAL_Init();
  BSP_EPD_Init();
	BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_GPIO);
	BSP_LED_Init(LED3);
  BSP_LED_Init(LED4);
  /* Configure the System clock to have a frequency of 2 MHz (Up to 32MHZ possible) */
  SystemClock_Config();
	

  /* Add your application code here
     */
	
	//opcje innicjalizacyjne czujnika dotyku
	
	TscHandle.Instance = TSC;
  TscHandle.Init.AcquisitionMode         = TSC_ACQ_MODE_NORMAL;
  TscHandle.Init.CTPulseHighLength       = TSC_CTPH_2CYCLES;
  TscHandle.Init.CTPulseLowLength        = TSC_CTPL_2CYCLES;
  TscHandle.Init.IODefaultMode           = TSC_IODEF_OUT_PP_LOW;
  TscHandle.Init.MaxCountInterrupt       = DISABLE;
  TscHandle.Init.MaxCountValue           = TSC_MCV_8191;
  TscHandle.Init.PulseGeneratorPrescaler = TSC_PG_PRESC_DIV2;
  TscHandle.Init.SpreadSpectrum          = DISABLE;
  TscHandle.Init.SpreadSpectrumDeviation = 127;
  TscHandle.Init.SpreadSpectrumPrescaler = TSC_SS_PRESC_DIV1;
  TscHandle.Init.SynchroPinPolarity      = TSC_SYNC_POL_FALL;
  TscHandle.Init.ChannelIOs              = TSC_GROUP1_IO3 | TSC_GROUP2_IO3 | TSC_GROUP3_IO2;
  TscHandle.Init.SamplingIOs             = TSC_GROUP1_IO4 | TSC_GROUP2_IO4 | TSC_GROUP3_IO3;
  TscHandle.Init.ShieldIOs               = 0;
	
  if (HAL_TSC_Init(&TscHandle) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }
	
	tsl_user_Init();
	current_position.x = 0;
	current_position.y = 0;
	GenerateStartScreen();  
  /* Infinite loop */
  while (1)
  {
	if (BSP_PB_GetState(BUTTON_KEY) == 1 && game_state == 0) //jezeli mamy stan 0 - poczatek gry i nacisniemy przycisk przechodzimy do stanu 2
	{
		game_state = 2;
	}
	if (BSP_PB_GetState(BUTTON_KEY) == 1 && game_state == 1) //jezeli stan gry jest rowny 1 i wcisniemy przycisk przechodzimy do stanu poruszania kursorem
	{
		DrawHead(); //rysujemy kursor i zaznaczone wczesniej pozycje na ekranie gry
		while (game_state == 1) // tak dlugo jak stan gry jest rowny 0, sprawdzamy dotyk sensora (poruszamy sie kursorem)
		{
			tsl_status = tsl_user_Exec();
			if (tsl_status != TSL_USER_STATUS_BUSY)
			{
				Process_Sensors(tsl_status);
			}
			if (BSP_PB_GetState(BUTTON_KEY) == 1) //jezeli wcisniemy przycisk (sprawdzimy liczbe), na obecnej pozycji kursora wywolujemy funkcje sprawdzajaca czy nasze sprawdzenie bylo poprawne
			{
				BSP_LED_On(LED3);
				CheckDecision();
				HAL_Delay(1000);
				BSP_LED_Off(LED3);
			}
		}
	}
	if(game_state == 2) // jezeli mamy stan 2 - generujemy i rysujemy liczby, zmieniamy stan gry na 1
	{
		GenerateNumbers();
		ShowNumbers();
		game_state = 1;
	}
  }
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = MSI
  *            SYSCLK(Hz)                     = 2000000
  *            HCLK(Hz)                       = 2000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 1
  *            Flash Latency(WS)              = 0
  *            Main regulator output voltage  = Scale3 mode
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  
  /* Enable Power Control clock */
  __PWR_CLK_ENABLE();
  
  /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  
  /* Enable MSI Oscillator */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_5;
  RCC_OscInitStruct.MSICalibrationValue=0x00;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }
  
  
  /* Select MSI as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;  
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  /* User may add here some code to deal with this error */
  while(1)
  {
  }
}

void GenerateNumbers() //funkcja odpowiedzialna za generowanie liczb w tablicy gry, na kazdym poziomie
{
	seed = HAL_GetTick();
	srand(seed);
	uint8_t number_x = 0;
	uint8_t number_y = 0;
	//wyczyszczenie tablicy, z numerów z poprzedniego poziomu
	for(uint8_t i = 0; i < M ; i++)
		{
			for(uint8_t j = 0; j < N ; j++)
				{
					numbers[i][j] = 0;
				}
		}
	for(uint8_t i = 0; i < level ; i++) //generowanie pozycji liczb, generuje tyle pozycji, jaki mamy poziom
	{
		number_x = rand()%M;
		number_y = rand()%N;
		while(numbers[number_x][number_y] != i+1) //jezeli na wygenerowanej pozycji nie ma i-tej liczby, wykonuj petle
		{
			if(numbers[number_x][number_y]==0) // jezeli na wygenerowanej pozycji w tablicy gry znajduje sie 0, wstaw liczbe do tablicy gry, na tej pozycji
			{
				numbers[number_x][number_y]=i+1;
				order[i][0] = number_x;
				order[i][1] = number_y;
			}
			else // w przeciwnym przypadku wygeneruj nowa pozycje
			{
				seed++;
				srand(seed);
				number_x = rand()%M;
				number_y = rand()%N;
			}
		}	
	}
}

void ShowNumbers(void) //funkcja odpowiedzialna za wyswietlanie liczb na wygenerowanych pozycjach
{
	BSP_EPD_Clear(EPD_COLOR_WHITE);
	BSP_EPD_SetFont(&Font20);
	char num[5];
	
	for(uint8_t i = 0; i < M ; i++)
		{
			for(uint8_t j = 0; j < N ; j++)
				{
					if(numbers[i][j] != 0)
					{
						sprintf(num,"%d",numbers[i][j]);
						BSP_EPD_DisplayStringAt(i*SQUARE_WIDTH, j*SQUARE_HEIGHT, (uint8_t*)num, LEFT_MODE); //jezeli w tablicy gry jakas pozycja zawiera cos innego niz "0", to wyswietlamy ja
					}
					BSP_EPD_DrawRect(i*SQUARE_WIDTH, j*SQUARE_HEIGHT, SQUARE_WIDTH, SQUARE_HEIGHT); //na calej planszy rysujemy prostokaty
				}
		}
	BSP_EPD_RefreshDisplay();
}

void CheckDecision(void) //funkcja odpowiedzialna za sprawdzenie czy liczby zgadywane sa poprawnie
{
	if(order[checkCounter][0] == current_position.x && order[checkCounter][1] == current_position.y) //sprawdzamy czy obecna pozycja kursora pokrywa sie z pozycja obecnie szukanej liczby
		{
			checkCounter++; //jezeli tak to zwiekszamy licznik, aby nastepne sprawdzenie bylo dla nastepnej szukanej liczby
			checked[current_position.x][current_position.y] = 1; //dodajemy te pozycje to tablicy, aby pozniej narysowac sprawdzone juz pozycje
		}
	else //jezeli pozycja kursora nie jest poprawna konczymy gre
		{
			game_state = 3;
			GameOver();
		}
	if(checkCounter == level && level == M*N) //jezeli zgadlismy ostatnia liczbe w momencie kiedy cala plansza byla zapelniona liczbami - konczymy gre
		{
			game_state = 3;
			YouWon();
		}
		
	if(checkCounter == level) //jezeli zgadlismy wszystkie liczby na danym poziomie, zwiekszamy poziom, zerujemy zmienne i zmieniamy stan gry na 2
		{
			game_state = 2;
			level ++;
			for(uint8_t i = 0; i < M ; i++)
			{
				for(uint8_t j = 0; j < N ; j++)
					{
						checked[i][j] = 0;
					}
			}
			
			checkCounter = 0;
		}
		
}


void DrawHead(void) //funkcja opowiedzialna za rysowanie pozycji kursora i zaznaczonych wczesniej pozycji na planszy gry
{
	uint8_t head_x = current_position.x;
	uint8_t head_y = current_position.y;
	
	BSP_EPD_Clear(EPD_COLOR_WHITE);
	for(uint8_t i = 0; i < M ; i++)
		{
			for(uint8_t j = 0; j < N ; j++)
				{
					if(checked[i][j] == 1)
					{
						BSP_EPD_DisplayStringAt(i*SQUARE_WIDTH+(SQUARE_WIDTH/2)-5, j*SQUARE_HEIGHT,(uint8_t*)"!" ,LEFT_MODE); //reysowanie wczesniej zaznaczonych pozycji za pomoca znaku "!"
					}
				}
		}
	BSP_EPD_DisplayStringAt(head_x*SQUARE_WIDTH+(SQUARE_WIDTH/2)-6, head_y*SQUARE_HEIGHT, (uint8_t*)"O", LEFT_MODE); //rysowanie pozycji kursora
	for(uint8_t i = 0; i < M ; i++)
		{
			for(uint8_t j = 0; j < N ; j++)
				{
					BSP_EPD_DrawRect(i*SQUARE_WIDTH, j*SQUARE_HEIGHT, SQUARE_WIDTH, SQUARE_HEIGHT); //na calej planszy rysujemy prostokaty
				}
		}
	BSP_EPD_RefreshDisplay();
}

void MoveDown(void) //funkcja odpowiedzialna za zmiane pozycji kursora
{
	uint8_t head_y = current_position.y;
	uint8_t next_row = head_y-1;

	if (head_y == 0) //jezeli kursor wychodzilby za pole gry
	{
		next_row = N-1;
	}

	current_position.y = next_row;
	DrawHead();
}

void MoveUp(void)
{
	uint8_t head_y = current_position.y;
	uint8_t next_row = head_y+1;

	if (head_y == N-1)
	{
		next_row = 0;
	}
	current_position.y = next_row;
	DrawHead();
}

void MoveLeft(void)
{
	uint8_t head_x = current_position.x;
	uint8_t next_column = head_x-1;

	if (head_x == 0)
	{
		next_column = M-1;
	}
	current_position.x = next_column;
	DrawHead();
}

void MoveRight(void)
{
	uint8_t head_x = current_position.x;
	uint8_t next_column = head_x+1;

	if (head_x == M-1)
	{
		next_column = 0;
	}
	current_position.x = next_column;
	DrawHead();
}
void GameOver()
	{
		char num[5];
		sprintf(num,"%d",level-1);
		BSP_EPD_Clear(EPD_COLOR_WHITE);
		BSP_EPD_DisplayStringAt(0,(N-1)*SQUARE_HEIGHT,(uint8_t*)"Koniec gry",CENTER_MODE);
		BSP_EPD_DisplayStringAt(0,(N-2)*SQUARE_HEIGHT,(uint8_t*)"Wynik:",CENTER_MODE);
		BSP_EPD_DisplayStringAt(0,(N-3)*SQUARE_HEIGHT,(uint8_t*)num,CENTER_MODE);
		BSP_EPD_RefreshDisplay();
	}
	
void YouWon()
	{
		char num[5];
		sprintf(num,"%d",level);
		BSP_EPD_Clear(EPD_COLOR_WHITE);
		BSP_EPD_DisplayStringAt(0,(N-1)*SQUARE_HEIGHT,(uint8_t*)"Wygrales",CENTER_MODE);
		BSP_EPD_DisplayStringAt(0,(N-2)*SQUARE_HEIGHT,(uint8_t*)"Wynik:",CENTER_MODE);
		BSP_EPD_DisplayStringAt(0,(N-3)*SQUARE_HEIGHT,(uint8_t*)num,CENTER_MODE);
		BSP_EPD_RefreshDisplay();
	}
void GenerateStartScreen()
{
	BSP_EPD_Clear(EPD_COLOR_WHITE);
	BSP_EPD_DisplayStringAt(0,(N-1)*SQUARE_HEIGHT,(uint8_t*)"Nacisnij",CENTER_MODE);
	BSP_EPD_DisplayStringAt(0,(N-2)*SQUARE_HEIGHT,(uint8_t*)"przycisk, aby",CENTER_MODE);
	BSP_EPD_DisplayStringAt(0,(N-3)*SQUARE_HEIGHT,(uint8_t*)"wystartowac",CENTER_MODE);
	BSP_EPD_RefreshDisplay();
}

void Process_Sensors(tsl_user_status_t status) //funkcja odpowiedzialna za wykrywanie dotyku na sensorze 
{
  if (LINEAR_DETECT) //jezeli wykryty zostanie dotyk
  {
		//led zielony zapala sie kiedy wyczuty jest dotyk, czerwony gasnie
		BSP_LED_On(LED3);
		BSP_LED_Off(LED4);
		
		if (LINEAR_POSITION < 4 && LINEAR_POSITION >= 1 && flag == 0) //sprawdzamy czy dotyk byl w pierwszej czesci sensora
		{
			MoveUp(); //poruszamy sie do gory
			flag = 1;
		}
		
		if (LINEAR_POSITION >= 4 && LINEAR_POSITION < 8 && flag == 0) //sprawdzamy czy dotyk byl w pierwszej drugiej sensora
		{
			MoveDown(); //poruszamy sie w dol
			flag = 1;
		}
		
		if (LINEAR_POSITION >= 8 && LINEAR_POSITION < 12 && flag == 0) //sprawdzamy czy dotyk byl w trzeciej czesci sensora
		{
			MoveRight(); //poruszamy sie w prawo
			flag = 1;
		}
		
		if (LINEAR_POSITION >= 12 && flag == 0) //sprawdzamy czy dotyk byl w czwartej czesci sensora
		{
			MoveLeft(); //poruszamy sie w lewo
			flag = 1;
		}
  }
  if (LINEAR_IDLE)// kiedy nie ma dotyku czerwony led sie swieci, zielony gasnie
  {
		flag = 0;
		LINEAR_POSITION = 0;
    BSP_LED_On(LED4);
		BSP_LED_Off(LED3);
  }
#ifdef ECS_INFO  
  /* ECS information */
  switch (status)
  {
  case TSL_USER_STATUS_OK_ECS_OFF:
    break;
    
  case TSL_USER_STATUS_OK_ECS_ON:
    break;
    
  default:
    break;
  }
#endif /* ECS_INFO */  
}


