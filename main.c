#include <stdint.h>
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>

#define ENABLE_PIN 2
#define MOTOR_INPUT1_PIN 3
#define MOTOR_INPUT2_PIN 4
void PortF_Init(void);
void PortA_Init(void);
void PortE_Init(void);
void PortB_Init(void);
void PortD_Init(void);


static void Jamming (void *pvParameters);
static void Up_driver (void *pvParameters);
static void Up_passenger (void *pvParameters);
static void Down_passenger (void *pvParameters);
static void Down_driver (void *pvParameters);
static void Lock (void *pvParameters);




void delay_ms(int n);
//define a Semaphore handle
xTaskHandle xJamming;
xTaskHandle xUp_driver;
xTaskHandle xUp_passenger;
xTaskHandle xDown_passenger;
xTaskHandle xDown_driver;
//xTaskHandle xLimit_switch;

xSemaphoreHandle xJamming_Semaphore;
xSemaphoreHandle xUp_driver_Semaphore;
xSemaphoreHandle xUp_passenger_Semaphore;
xSemaphoreHandle xDown_passenger_Semaphore;
xSemaphoreHandle xDown_driver_Semaphore;
xSemaphoreHandle xLock_Semaphore;




 //                   /main function
//------------------------------------------------------------------------//
int main( void )
{
    PortF_Init();
	  PortA_Init();
	  PortB_Init();
	  PortD_Init();
	  PortE_Init();

		__ASM("CPSIE i");
			 
		vSemaphoreCreateBinary(xJamming_Semaphore);
	  vSemaphoreCreateBinary(xUp_driver_Semaphore);
		vSemaphoreCreateBinary(xUp_passenger_Semaphore);
		vSemaphoreCreateBinary(xDown_passenger_Semaphore);
		vSemaphoreCreateBinary(xDown_driver_Semaphore);
		vSemaphoreCreateBinary(xLock_Semaphore);


	
	
		//xBinarySemaphore = xSemaphoreCreateBinary();
	if( (xJamming_Semaphore  && xUp_driver_Semaphore && xUp_passenger_Semaphore && xDown_passenger_Semaphore && xDown_driver_Semaphore && xLock_Semaphore )!= NULL)
		{
			/* Create the 'handler' task. This is the task that will be synchronized
			with the interrupt. The handler task is created with a high priority to
			ensure it runs immediately after the interrupt exits. In this case a
			priority of 3 is chosen. */
			xTaskCreate( Jamming, "Move Up Task", 240, NULL, 5, &xJamming );
    	xTaskCreate( Up_driver , "Move auto Task", 240, NULL, 4, &xUp_driver );
			xTaskCreate( Up_passenger , "Move auto Task", 240, NULL, 2, &xUp_passenger );
			xTaskCreate( Down_passenger , "Move auto Task", 240, NULL, 2, &xDown_passenger );
			xTaskCreate( Down_driver , "Move auto Task", 240, NULL, 4, &xDown_driver );
			xTaskCreate( Lock , "Lock Task", 240, NULL, 2, NULL );


			/* Create the task that will periodically generate a software interrupt.
			This is created with a priority below the handler task to ensure it will
			get preempted each time the handler task exits the Blocked state. */
			//xTaskCreate( vPeriodicTask, "Periodic", 240, NULL, 1, NULL );
			/* Start the scheduler so the created tasks start executing. */
			vTaskStartScheduler();
		}

    /* If all is well we will never reach here as the scheduler will now be
    running the tasks.  If we do reach here then it is likely that there was
    insufficient heap memory available for a resource to be created. */
    for( ;; );
}

//------------------------------------------------------------------------/
//Initialize the hardware of Port-F
void PortA_Init(void){ 
  SYSCTL->RCGCGPIO |= 0x00000001;    // 1) F clock
  GPIOA->LOCK = 0x4C4F434B;  				 // 2) unlock PortF PF0  
  GPIOA->CR = 0xFF;          				 // allow changes to PF4-0       
  GPIOA->AMSEL= 0x00;       				 // 3) disable analog function
  GPIOA->PCTL = 0x00000000;  				 // 4) GPIO clear bit PCTL  
  GPIOA->DIR = 0x1C;         				 // 5) PA7,PA6 input, PF3,PF2,PF1 output   
  GPIOA->AFSEL = 0x00;      				 // 6) no alternate function
  GPIOA->PUR = 0xE3;       				   // enable pullup resistors on PF4,PF0       
  GPIOA->DEN = 0xFF;       				   // 7) enable digital pins PF4-PF0
	GPIOA->DATA = 0x00;
	
	// Setup the interrupt on PortF
	GPIOA->ICR = 0xFF;     // Clear any Previous Interrupt 
	GPIOA->IM |=0x01;      // Unmask the interrupts for PF0 and PF4
	GPIOA->IS |= ~(0x01);     // Make bits PF0 and PF4 level sensitive
	GPIOA->IEV &= ~(0x01);   // Sense on Low Level
  
	NVIC_EnableIRQ(0);        // Enable the Interrupt for PortF in NVIC
}


void PortB_Init(void){ 
  SYSCTL->RCGCGPIO |= 0x00000002;    // 1) F clock
  GPIOB->LOCK = 0x4C4F434B;  				 // 2) unlock PortF PF0  
  GPIOB->CR = 0xFF;          				 // allow changes to PF4-0       
  GPIOB->AMSEL= 0x00;       				 // 3) disable analog function
  GPIOB->PCTL = 0x00000000;  				 // 4) GPIO clear bit PCTL  
  GPIOB->DIR = 0x00;         				 // 5) PF4,PF0 input, PF3,PF2,PF1 output   
  GPIOB->AFSEL = 0x00;      				 // 6) no alternate function
  GPIOB->PUR = 0xFF;       				   // enable pullup resistors on PF4,PF0       
  GPIOB->DEN = 0xFF;       				   // 7) enable digital pins PF4-PF0
	GPIOB->DATA = 0x00;
	
	// Setup the interrupt on PortF
	GPIOB->ICR = (1<<0) | (1<<1);     // Clear any Previous Interrupt 
	GPIOB->IM |= (1<<0) | (1<<1);      // Unmask the interrupts for PF0 and PF4
	GPIOB->IS &= ~((1<<0) | (1<<1));     // Make bits PF0 and PF4 level sensitive
	GPIOB->IEV &= ~((1<<0) | (1<<1));   // Sense on Low Level

	NVIC_EnableIRQ(1);        // Enable the Interrupt for PortF in NVIC
}

void PortC_Init(void){ 
  SYSCTL->RCGCGPIO |= 0x00000004;    // 1) F clock
  GPIOC->LOCK = 0x4C4F434B;  				 // 2) unlock PortF PF0  
  GPIOC->CR = 0xE0;          				 // allow changes to PF4-0       
  GPIOC->AMSEL= 0x00;       				 // 3) disable analog function
  GPIOC->PCTL = 0x00000000;  				 // 4) GPIO clear bit PCTL  
  GPIOC->DIR = 0x00;         				 // 5) PF4,PF0 input, PF3,PF2,PF1 output   
  GPIOC->AFSEL = 0x00;      				 // 6) no alternate function
  GPIOC->PUR = 0xE0;       				   // enable pullup resistors on PF4,PF0       
  GPIOC->DEN = 0xE0;       				   // 7) enable digital pins PF4-PF0
	GPIOC->DATA = 0x00;
	
	// Setup the interrupt on PortF
	GPIOC->ICR = 0xFF;     // Clear any Previous Interrupt 
	GPIOC->IM |=0x80;      // Unmask the interrupts for PF0 and PF4
	GPIOC->IS &= ~(0x80);     // Make bits PF0 and PF4 level sensitive
	GPIOC->IEV &= ~(0x80);   // Sense on Low Level

	NVIC_EnableIRQ(2);        // Enable the Interrupt for PortF in NVIC
}

void PortD_Init(void){ 
  SYSCTL->RCGCGPIO |= 0x00000008;    // 1) F clock
  GPIOD->LOCK = 0x4C4F434B;  				 // 2) unlock PortF PF0  
  GPIOD->CR = 0x1F;          				 // allow changes to PF4-0       
  GPIOD->AMSEL= 0x00;       				 // 3) disable analog function
  GPIOD->PCTL = 0x00000000;  				 // 4) GPIO clear bit PCTL  
  GPIOD->DIR = 0x00;         				 // 5) PF4,PF0 input, PF3,PF2,PF1 output   
  GPIOD->AFSEL = 0x00;      				 // 6) no alternate function
  GPIOD->PUR = 0xFF;       				   // enable pullup resistors on PF4,PF0       
  GPIOD->DEN = 0xFF;       				   // 7) enable digital pins PF4-PF0
	GPIOD->DATA = 0x00;
	
	// Setup the interrupt on PortF
	GPIOD->ICR = (1<<0) | (1<<1);     // Clear any Previous Interrupt 
	GPIOD->IM |= (1<<0) | (1<<1);      // Unmask the interrupts for PF0 and PF4
	GPIOD->IS &= ~((1<<0) | (1<<1));     // Make bits PF0 and PF4 level sensitive
	GPIOD->IEV &= ~((1<<0) | (1<<1));   // Sense on Low Level

	NVIC_EnableIRQ(3);        // Enable the Interrupt for PortF in NVIC
}


void PortE_Init(void){ 
  SYSCTL->RCGCGPIO |= 0x00000010;    // 1) F clock
  GPIOE->LOCK = 0x4C4F434B;  				 // 2) unlock PortF PF0  
  GPIOE->CR = 0x1F;          				 // allow changes to PF4-0       
  GPIOE->AMSEL= 0x00;       				 // 3) disable analog function
  GPIOE->PCTL = 0x00000000;  				 // 4) GPIO clear bit PCTL  
  GPIOE->DIR = 0x0C;         				 // 5) PF4,PF0 input, PF3,PF2,PF1 output   
  GPIOE->AFSEL = 0x00;      				 // 6) no alternate function
  GPIOE->PUR = (1<<0) | (1<<1);       				   // enable pullup resistors on PF4,PF0       
  GPIOE->DEN = 0x1F;       				   // 7) enable digital pins PF4-PF0
	GPIOE->DATA = 0x00;
	
	// Setup the interrupt on PortF
	GPIOE->ICR = (1<<0) | (1<<1);     // Clear any Previous Interrupt 
	GPIOE->IM |= (1<<0) | (1<<1);      // Unmask the interrupts for PF0 and PF4
	GPIOE->IS &= ~((1<<0) | (1<<1));     // Make bits PF0 and PF4 level sensitive
	GPIOE->IEV &= ~((1<<0) | (1<<1));   // Sense on Low Level

	NVIC_EnableIRQ(4);        // Enable the Interrupt for PortF in NVIC
}

void PortF_Init(void){ 
  SYSCTL->RCGCGPIO |= 0x00000020;    // 1) F clock
  GPIOF->LOCK = 0x4C4F434B;  				 // 2) unlock PortF PF0  
  GPIOF->CR = 0x1F;          				 // allow changes to PF4-0       
  GPIOF->AMSEL= 0x00;       				 // 3) disable analog function
  GPIOF->PCTL = 0x00000000;  				 // 4) GPIO clear bit PCTL  
  GPIOF->DIR = 0x00;         				 // 5) PF4,PF0 input, PF3,PF2,PF1 output   
  GPIOF->AFSEL = 0x00;      				 // 6) no alternate function
  GPIOF->PUR = 0x1F;       				   // enable pullup resistors on PF4,PF0       
  GPIOF->DEN = 0x1F;       				   // 7) enable digital pins PF4-PF0
	GPIOF->DATA = 0x00;
	
	// Setup the interrupt on PortF
	GPIOF->ICR = 0x1F;   // Clear any Previous Interrupt 
	GPIOF->IM |=(1<<0) | (1<<1);   // Unmask the interrupts for PF0 and PF4/
	GPIOF->IS &= ~((1<<0) | (1<<1));    // Make bits PF0 and PF4 level sensitive
	GPIOF->IEV &= ~((1<<0) | (1<<1));    // Sense on Low Level

	NVIC_EnableIRQ(30);        // Enable the Interrupt for PortF in NVIC
  NVIC_SetPriority( 30, 0xe0 );
	
}
static void Jamming (void *pvParameters)
{
	//PF0, PF1
	xSemaphoreTake(xJamming_Semaphore,0);
	while(1)
	{
	
	 xSemaphoreTake(xJamming_Semaphore,portMAX_DELAY);
			int ticks=0;
		
		//0.5 seconds delay
    while (ticks < (80*3180)){		
    GPIOA->DATA &= ~(1 << MOTOR_INPUT2_PIN);
    GPIOA->DATA |= (1 << MOTOR_INPUT1_PIN); 
		GPIOA->DATA |= (1<<ENABLE_PIN);		
			ticks++;

}
   GPIOA->DATA &= ~(1 << ENABLE_PIN );
   vTaskSuspend(xUp_driver);
   vTaskSuspend(xUp_passenger);
}
	

	}

	
//	static void Limit_Switch (void *pvParameters)
//	{
//		xSemaphoreTake(xLimit_switch_Semaphore,0);
//	while(1)
//	{
//	
//	 xSemaphoreTake(xLimit_switch_Semaphore,portMAX_DELAY);
//		GPIOA->DATA &= ~(1 << ENABLE_PIN );
//		
//		
//	}
//		
//		
//	}


static void Down_passenger (void *pvParameters)
	{
		
		xSemaphoreTake(xDown_passenger_Semaphore,0);
	while(1)
	{
	  xSemaphoreTake(xDown_passenger_Semaphore,portMAX_DELAY);
		
		int ticks = 0 ; 
		if ((GPIOD->DATA & (1 << 1)) == 0 && ((GPIOE->DATA & (1<<0)) != 0) && ((GPIOF->DATA & (1<<1)) != 0)){
			while(ticks < (150*3180)){
		GPIOA->DATA |= (1 << MOTOR_INPUT1_PIN);
    GPIOA->DATA &= ~(1 << MOTOR_INPUT2_PIN); 
		GPIOA->DATA |= (1<<ENABLE_PIN);		
			ticks++;
}
			}
			while ((GPIOD->DATA & (1 << 1)) == 0 && ((GPIOE->DATA & (1<<0)) != 0)&&((GPIOF->DATA & (1<<1)) != 0) ){
				if ((GPIOD->DATA & (1 << 1)) == 0 && ((GPIOE->DATA & (1<<0)) != 0)&&((GPIOF->DATA & (1<<1)) != 0)){
		GPIOA->DATA |= (1 << MOTOR_INPUT1_PIN);
    GPIOA->DATA &= ~(1 << MOTOR_INPUT2_PIN); 
		GPIOA->DATA |= (1<<ENABLE_PIN);	

			}
     GPIOA->DATA &= ~ (1<<ENABLE_PIN);		

		}
	vTaskResume(xUp_driver);
  vTaskResume(xUp_passenger);
	}
	
		
	}

static void Up_driver (void *pvParameters)
{
	//PB2,PB3
	xSemaphoreTake(xUp_driver_Semaphore,0);
	while(1)
	{
	  xSemaphoreTake(xUp_driver_Semaphore,portMAX_DELAY);
		
		int ticks = 0 ; 
		
		if ((GPIOB->DATA & (1 << 0)) == 0 && ((GPIOE->DATA & (1<<1)) != 0)){
			while(ticks < (150*3180)){
		GPIOA->DATA |= (1 << MOTOR_INPUT2_PIN);
    GPIOA->DATA &= ~(1 << MOTOR_INPUT1_PIN); 
		GPIOA->DATA |= (1<<ENABLE_PIN);		
			ticks++;
}
			}
			while ((GPIOB->DATA & (1 << 0)) == 0 && ((GPIOE->DATA & (1<<1)) != 0)){
				if ((GPIOB->DATA & (1 << 0)) == 0 && ((GPIOE->DATA & (1<<1)) != 0) ){
		GPIOA->DATA |= (1 << MOTOR_INPUT2_PIN);
    GPIOA->DATA &= ~(1 << MOTOR_INPUT1_PIN); 
		GPIOA->DATA |= (1<<ENABLE_PIN);	
			}
			GPIOA->DATA &= ~ (1<<ENABLE_PIN);		
		}
	}
}

static void Up_passenger (void *pvParameters)
{
	//PB2,PB3
	xSemaphoreTake(xUp_passenger_Semaphore,0);
	while(1)
	{
	  xSemaphoreTake(xUp_passenger_Semaphore,portMAX_DELAY);
		
		int ticks = 0 ; 
		if ((GPIOD->DATA & (1 << 0)) == 0 && ((GPIOE->DATA & (1<<1)) != 0)&&((GPIOF->DATA & (1<<1)) != 0)){
			while(ticks < (150*3180)){
		GPIOA->DATA |= (1 << MOTOR_INPUT2_PIN);
    GPIOA->DATA &= ~(1 << MOTOR_INPUT1_PIN); 
		GPIOA->DATA |= (1<<ENABLE_PIN);		
			ticks++;
}
			}
			while ((GPIOD->DATA & (1 << 0)) == 0 && ((GPIOE->DATA & (1<<1)) != 0)&&((GPIOF->DATA & (1<<1)) != 0)){
				if ((GPIOD->DATA & (1 << 0)) == 0 && ((GPIOE->DATA & (1<<1)) != 0)&&((GPIOF->DATA & (1<<1)) != 0)){
		GPIOA->DATA |= (1 << MOTOR_INPUT2_PIN);
    GPIOA->DATA &= ~(1 << MOTOR_INPUT1_PIN); 
		GPIOA->DATA |= (1<<ENABLE_PIN);	

			}
     GPIOA->DATA &= ~ (1<<ENABLE_PIN);		

		}
	}
}



static void Down_driver (void *pvParameters)
{
	
			xSemaphoreTake(xDown_driver_Semaphore,0);
	while(1)
	{
	  xSemaphoreTake(xDown_driver_Semaphore,portMAX_DELAY);
		
		int ticks = 0 ; 
		if ((GPIOB->DATA & (1 << 1)) == 0 && ((GPIOE->DATA & (1<<0)) != 0)){
			while(ticks < (150*3180)){
		GPIOA->DATA |= (1 << MOTOR_INPUT1_PIN);
    GPIOA->DATA &= ~(1 << MOTOR_INPUT2_PIN); 
		GPIOA->DATA |= (1<<ENABLE_PIN);		
			ticks++;
}
			}
		
			while ((GPIOB->DATA & (1 << 1)) == 0 && ((GPIOE->DATA & (1<<0)) != 0) ){
				if ((GPIOB->DATA & (1 << 1)) == 0 && ((GPIOE->DATA & (1<<0)) != 0)){
		GPIOA->DATA |= (1 << MOTOR_INPUT1_PIN);
    GPIOA->DATA &= ~(1 << MOTOR_INPUT2_PIN); 
		GPIOA->DATA |= (1<<ENABLE_PIN);	

			}
     GPIOA->DATA &= ~ (1<<ENABLE_PIN);	
    
		}
			vTaskResume(xUp_driver);
    vTaskResume(xUp_passenger);			
   
	}
		
	
	
	
}
static void Lock (void *pvParameters)
{
	xSemaphoreTake(xLock_Semaphore,0);
	while(1)
	{
	  xSemaphoreTake(xLock_Semaphore,portMAX_DELAY);
		while (GPIOF->DATA && (1<<1) != 1)
		{
			GPIOA->DATA &= ~ (1<<ENABLE_PIN);	
		}
	}
	
}

	
void GPIOF_Handler(void){

	uint32_t i;
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	delay_ms(30);
	if (GPIOF->MIS & (1<<0))
	{
	//Give the semaphore to the Task named handler
  xSemaphoreGiveFromISR(xJamming_Semaphore,&xHigherPriorityTaskWoken);
	//mainCLEAR_INTERRUPT();
	GPIOF->ICR = 0x01;        // clear the interrupt flag of PORTF
  i= GPIOF->ICR ;           // Reading the register to force the flag to be cleared
	portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
	}
	if (GPIOF->MIS & (1<<1))
	{
	//Give the semaphore to the Task named handler
  xSemaphoreGiveFromISR(xLock_Semaphore,&xHigherPriorityTaskWoken);
	//mainCLEAR_INTERRUPT();
	GPIOF->ICR = 0xFF;        // clear the interrupt flag of PORTF
  i= GPIOF->ICR ;           // Reading the register to force the flag to be cleared
	portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
	}
	
	
}

void GPIOB_Handler(void){

	uint32_t i;
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	delay_ms(30);
	if (GPIOB->MIS & (1<<0))
	{
	//Give the semaphore to the Task named handler
  xSemaphoreGiveFromISR(xUp_driver_Semaphore,&xHigherPriorityTaskWoken);
	//mainCLEAR_INTERRUPT();
	GPIOB->ICR = 0x01;        // clear the interrupt flag of PORTF
  i= GPIOB->ICR ;           // Reading the register to force the flag to be cleared
	portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
	}
	if (GPIOB->MIS & (1<<1))
	{
	//Give the semaphore to the Task named handler
  xSemaphoreGiveFromISR(xDown_driver_Semaphore,&xHigherPriorityTaskWoken);
	//mainCLEAR_INTERRUPT();
	GPIOB->ICR = (1<<1);        // clear the interrupt flag of PORTF
  i= GPIOB->ICR ;           // Reading the register to force the flag to be cleared
	portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
	}
	
}


void GPIOD_Handler(void){

	uint32_t i;
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	delay_ms(30);
	if (GPIOD->MIS & (1<<0))
	{
	//Give the semaphore to the Task named handler
  xSemaphoreGiveFromISR(xUp_passenger_Semaphore,&xHigherPriorityTaskWoken);
	//mainCLEAR_INTERRUPT();
	GPIOD->ICR = 0x01;        // clear the interrupt flag of PORTF
  i= GPIOD->ICR ;           // Reading the register to force the flag to be cleared
	portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
	}
	if (GPIOD->MIS & (1<<1))
	{
	//Give the semaphore to the Task named handler
  xSemaphoreGiveFromISR(xDown_passenger_Semaphore,&xHigherPriorityTaskWoken);
	//mainCLEAR_INTERRUPT();
	GPIOD->ICR = (1<<1);        // clear the interrupt flag of PORTF
  i= GPIOD->ICR ;           // Reading the register to force the flag to be cleared
	portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
	}
	
}


//Port-E handler

void GPIOE_Handler(void){

	uint32_t i;
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	delay_ms(30);
	//Give the semaphore to the Task named handler
 // xSemaphoreGiveFromISR(xLimit_switch_Semaphore,&xHigherPriorityTaskWoken);
		if (GPIOE->MIS & (1<<1)){
	//mainCLEAR_INTERRUPT();
	GPIOE->ICR = (1<<1);        // clear the interrupt flag of PORTF
  i= GPIOE->ICR ;           // Reading the register to force the flag to be cleared
	portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
					GPIOA->DATA &= ~(1 << ENABLE_PIN );

		}
		if (GPIOE->MIS & (1<<0)){
	//mainCLEAR_INTERRUPT();
	GPIOE->ICR = (1<<0);        // clear the interrupt flag of PORTF
  i= GPIOE->ICR ;           // Reading the register to force the flag to be cleared
	portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
					GPIOA->DATA &= ~(1 << ENABLE_PIN );

	
			
		}
	

}
void delay_ms(int n)
{
 int i,j;
 for(i=0;i<n;i++)
 for(j=0;j<3180;j++)
 {}
}