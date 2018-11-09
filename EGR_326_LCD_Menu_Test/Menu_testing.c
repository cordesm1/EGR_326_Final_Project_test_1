//**********************************************************************************
// Name(s): Nicklas Koeller && Mason Cordes
// Date: October 5, 2018
// Professor: Dr. Krug
// Description: This program is testing LCD for final Project Trying to get a main menu with highlighting different options
// Source(s):   The ST7735 Library, and code from Prof. Jim Valvano at Univ. of Texas. , and
//              code from Dr. Krug's Lecture 9 for EGR326-01 for the Fall 2018 Semester was used/modified
//**********************************************************************************
#include "driverlib.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "ST7735.h"
#include "Images.h"

#define writeIdleScreen     0
#define writeMainMenu       1
#define writeSetTimeSubMenu 2


uint32_t SMCLKfreq,MCLKfreq; //Variable to store the clock frequencies
uint8_t nextDirection = 0; //no selections
uint8_t contact1 = 0, contact2 = 0, direction = 0;

void clockInit48MHzXTL(void); //Function to set the clk to 48MHz external
void PORT1_IRQHandler(void);  //for both on board pushbuttons
void push_btn_init(void);


int main(void)
{
    MAP_WDT_A_holdTimer(); // Stop Watchdog
    clockInit48MHzXTL(); //Sets MCLK and SMCLK to the external HFXT crystal
    MAP_CS_initClockSignal(CS_SMCLK, CS_HFXTCLK_SELECT, CS_CLOCK_DIVIDER_4);
    SMCLKfreq=MAP_CS_getSMCLK(); // get SMCLK value to verify it was set to 12 MHz
    MCLKfreq=MAP_CS_getMCLK(); // get MCLK value and verify it also
    Systick_Init(); //Initilaizes SysTick Timer
    ST7735_InitR(INITR_REDTAB); //Initializes LCD

    //variabls
    uint8_t direction = 0;

    //inits
    push_btn_init();
    rotaryPinInit();                   //init rotary encoder

    uint8_t state  = 0, nextState = 0;//controls the state
    uint8_t userSelection = 0;
    MAP_Interrupt_enableMaster();           //enable interrupts

    while(1)
    {

        switch (state)
        {
            case writeIdleScreen:
                userSelection = idleScreen(direction, 22, 72);

                if(userSelection)
                     nextState = writeMainMenu;
                else nextState = writeIdleScreen;

            break;
            case writeMainMenu:
                userSelection = writeMenu(direction);

                if(userSelection == 0)
                    nextState = writeMainMenu;
                else if(userSelection == 1)
                    nextState = writeSetTimeSubMenu;
                else if(userSelection == 2)
                    nextState = writeIdleScreen;//nextState = something
                else if(userSelection == 3)
                    nextState = writeIdleScreen;//nextState = something
                else if(userSelection == 4)
                    nextState = writeIdleScreen;//nextState = something
                else
                    nextState = writeIdleScreen;


                break;

            case writeSetTimeSubMenu:
                userSelection = setTimeSubMenu(direction);

                if (userSelection)
                    nextState = writeIdleScreen;
                break;

            default:
                ;//do nothing
        }
        //Stuff that needs to happen as often as possible without being super timing critical
        //aka:watchdog?
        //encoder direction setting
        direction = nextDirection;
        nextDirection = 0;
        //controling next state
        state = nextState;
        topBannerPrint(22,72);
    }
}

/******************************
Name:         clockInit48MHzXTL
Description:  Sets up MSP to run off the external 48MHz Clock Source
Input:        none
Output:       none
Source(s):    Some of this code was taken from Dr. Krug's Lecture 7 for EGR 326-01 Fall 2018.
*******************************/
void clockInit48MHzXTL(void)
{  // sets the clock module to use the external 48 MHz crystal
    /* Configuring pins for peripheral/crystal usage */
    MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_PJ,GPIO_PIN3 | GPIO_PIN2,GPIO_PRIMARY_MODULE_FUNCTION);
    CS_setExternalClockSourceFrequency(32000, 48000000); // enables getMCLK, getSMCLK to know externally set frequencies
    /* Starting HFXT in non-bypass mode without a timeout. Before we start
     * we have to change VCORE to 1 to support the 48MHz frequency */
    MAP_PCM_setCoreVoltageLevel(PCM_VCORE1);
    MAP_FlashCtl_setWaitState(FLASH_BANK0, 2);
    MAP_FlashCtl_setWaitState(FLASH_BANK1, 2);
    CS_startHFXT(false); // false means that there are no timeouts set,will return when stable
    /* Initializing MCLK to HFXT (effectively 48MHz) */
    MAP_CS_initClockSignal(CS_MCLK, CS_HFXTCLK_SELECT, CS_CLOCK_DIVIDER_1);
}


void push_btn_init(void)
{
    //push btn p1.1
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1);
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN1);
    MAP_GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN1);
    MAP_Interrupt_enableInterrupt(INT_PORT1);

    //push btn p1.4
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN4);
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN4);
    MAP_GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN4);
    MAP_Interrupt_enableInterrupt(INT_PORT1);
}



//interrupts



void PORT1_IRQHandler(void)
{
    uint32_t status = MAP_GPIO_getEnabledInterruptStatus(GPIO_PORT_P1);
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P1, status);

    if(status & GPIO_PIN1){
        nextDirection = 1;      //forward
    }
    if(status & GPIO_PIN4){
        nextDirection = 3;      //backwards
    }

}

/******************************
 Name:         PORT6IRQ_Handler
 Description:  Handler for the Port 4 Interrupt
 Input:        none
 Output:       none
 Source(s):    Some of this code was taken from Texas Instrument Example Code.
 *******************************/
void PORT6_IRQHandler(void)
{
    uint32_t status;   //Variable to hold the interrupt status

    status = MAP_GPIO_getEnabledInterruptStatus(GPIO_PORT_P6);  //Saves the interrupt status to status variable
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P6, status);  //Clears the interrupt flag

    if (status & GPIO_PIN0) //If P4.0 caused the interrupt
    {
        if(contact1 == 1){
            contact1 = 0;
            contact2 = 0;
        }
        if ((contact2) == 0)
        {
            contact1 = 1;
        }
        if (contact2 == 1)
        {
            contact1 = 2;
        }
    }

    if (status & GPIO_PIN1) //If P4.1 caused the interrupt
        {
        if(contact1 == 2){
                    contact1 = 0;
                    contact2 = 0;
                }
        if ((contact1) == 0)
            {
                contact2 = 1;
            }
            if (contact1 == 1)
            {
                contact2 = 2;
            }
        }

    if (contact2 == 2 || contact1 == 2)
    {
        nextDirection = contact1;
        contact1 = 0;
        contact2 = 0;
    }

    if (status & GPIO_PIN2) //If P4.2 caused the interrupt
    {
        nextDirection = 3;      //if the btn is pushed
    }

}

