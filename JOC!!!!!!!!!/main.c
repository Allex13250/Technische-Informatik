#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stm32g431xx.h>

#define SHORT_LED_SIGNAL 350
#define LONG_LED_SIGNAL 1000
#define LEVEL_COMPLETED_TRANSITION_TIME 3000
#define LEVEL_NOT_PASSED_TRANSITION_TIME 3000
#define MAX_LEVELS 5

void delay();

void play_game() {
    int level = 1;
    int sequence_length = 5;
    int sequence[9];
    int input[9];
    int i, j;
    int correct = 1;
    
    // Initialize GPIO pins
    GPIOA->PUPDR &= ~(1 << 0);      // Disable PA0 pull-up/pull-down
    GPIOA->PUPDR &= ~(1 << 8);      // Disable PA8 pull-up/pull-down
    
    SYSCFG->EXTICR[1] &= ~(0xF);    // Select Port A as interrupt source for EXTI line 0 
    SYSCFG->EXTICR[3] &= ~(0xF);    // Select Port A as interrupt source for EXTI line 8 
    
    EXTI->IMR1 |= 1 << 0;           // Enable EXTI line 0
    EXTI->IMR1 |= 1 << 8;           // Enable EXTI line 8
    
    EXTI->FTSR1 |= 1 << 0;          // Enable falling edge for trigger
    EXTI->FTSR1 |= 1 << 8;          // Enable falling edge for trigger
    
    // Enable Interrupts
    NVIC_EnableIRQ(EXTI0_IRQn);     // Enable EXTI0 interrupt vector in NVIC
    NVIC_EnableIRQ(EXTI9_5_IRQn);   // Enable EXTI9_5 interrupt vector in NVIC
    __enable_irq();                 // Enable all interrupts (global)
    
    // Initialize random seed
    srand(time(NULL));
    
    while (level <= MAX_LEVELS) {
        // Generate random sequence
        for (i = 0; i < sequence_length; i++) {
            sequence[i] = rand() % 2;
            
            // Turn on LED based on sequence
            if (sequence[i] == 1) {
                GPIOA->BSRR = GPIO_BSRR_BS1;
                delay(SHORT_LED_SIGNAL);
                GPIOA->BSRR = GPIO_BSRR_BR1;
            } else {
                GPIOA->BSRR = GPIO_BSRR_BS1;
                delay(LONG_LED_SIGNAL);
                GPIOA->BSRR = GPIO_BSRR_BR1;
            }
            
            // Wait for transition time
            delay(LEVEL_COMPLETED_TRANSITION_TIME);
        }
        
        // Get input from user
        for (i = 0; i < sequence_length; i++) {
            // Wait for input
            while (1) {
                if ((GPIOA->IDR & GPIO_PIN_2) != 0) {
                    input[i] = 0;
                    GPIOA->BSRR = GPIO_BSRR_BS2;
                    delay(SHORT_LED_SIGNAL);
                    GPIOA->BSRR = GPIO_BSRR_BR2;
                    break;
                } else if ((GPIOA->IDR & GPIO_PIN_0) != 0) {
                    input[i] = 1;
                    GPIOA->BSRR = GPIO_BSRR_BS0;
                    delay(LONG_LED_SIGNAL);
                    GPIOA->BSRR = GPIO_BSRR_BR0;
                    break;
                }
            }
            
            // Check input
            if (input[i] != sequence[i]) {
                correct = 0;
                break;
            }
        }
        
        // Check if level was completed
        if (correct) {
            level++;
            sequence_length++;
        } else {
            level = 1;
            sequence_length = 5;
            
            // Flash LED 3 times
            for (j = 0; j < 3; j++) {
                GPIOA->BSRR = GPIO_BSRR_BS1;
                delay(SHORT_LED_SIGNAL);
                GPIOA->BSRR = GPIO_BSRR_BR1;
                delay(SHORT_LED_SIGNAL);
            }
            
            // Wait for transition time
            delay(LEVEL_NOT_PASSED_TRANSITION_TIME);
        }
    }
}

void delay(int milliseconds) {
    long int i;
    for (i = 0; i < milliseconds * 1000; i++);
}

void EXTI0_IRQHandler(void)     // for PA0
{
    EXTI->PR1 |= (1 << 0);    // Clear PR to re-enable EXTI interrupt
    GPIOB->ODR ^= 1 << 8;     // Toggle green LED
}

void EXTI9_5_IRQHandler(void)   // for PA8
{
    if ((EXTI->PR1 & (1 << 8)) != 0) // Test for line 8 pending interrupt
    {
        EXTI->PR1 |= (1 << 8);    // Clear PR to re-enable EXTI interrupt
        GPIOB->ODR ^= 1 << 8;     // Toggle green LED
    }
}

int main(void) 
{
    play_game();
    return 0;
}
