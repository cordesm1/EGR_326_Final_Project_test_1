#include <driverlib.h>


const eUSCI_UART_Config uartConfig =
{
 EUSCI_A_UART_CLOCKSOURCE_SMCLK,
 13, //
 0,
 37,
 EUSCI_A_UART_NO_PARITY,
 EUSCI_A_UART_LSB_FIRST,
 EUSCI_A_UART_ONE_STOP_BIT,
 EUSCI_A_UART_MODE,
 EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION
};




char enter[] = "Enter Duty Cycle\r\n";
char enter_new[] = "\r\nEnter New Duty Cycle\r\n";
char digits[3];
uint8_t RXByteCounter = 0;
int newSpeed = 0;
int oldSpeed = 0;

void driveMotor(int cycles);
void transmit(char *str);
int convert(uint32_t ByteCounter, char *data);

void main(void)
{
    WDT_A_holdTimer();

        MAP_GPIO_setAsOutputPin(GPIO_PORT_P10, GPIO_PIN1+GPIO_PIN2+GPIO_PIN3+GPIO_PIN4);    //Sets up output pins for the motor
        MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P10, GPIO_PIN1+GPIO_PIN2+GPIO_PIN3+GPIO_PIN4); //Sets all pins to low to start
        MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P10, GPIO_PIN1+GPIO_PIN3);    //Turns on one pin for each coil of the motor
        driveMotor(160);
        driveMotor(-160);

    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1, GPIO_PIN2|GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN4, GPIO_PRIMARY_MODULE_FUNCTION);

    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_12);

    UART_initModule(EUSCI_A0_BASE, &uartConfig);
    UART_enableModule(EUSCI_A0_BASE);
    UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);

    Interrupt_enableInterrupt(INT_EUSCIA0);
    Interrupt_enableSleepOnIsrExit();
    Interrupt_enableMaster();

    transmit(enter);

    while(1);
}

void EUSCIA0_IRQHandler(void){
    char RXData;
    uint32_t status = UART_getEnabledInterruptStatus(EUSCI_A0_BASE);
    UART_clearInterruptFlag(EUSCI_A0_BASE, status);

    if(status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG){
        RXData = UART_receiveData(EUSCI_A0_BASE);
    if(RXData != 0x0D && RXByteCounter <= 3)
        {
        digits[RXByteCounter++] = RXData;
        UART_transmitData(EUSCI_A0_BASE, RXData);
        }

    else{
        newSpeed = convert(RXByteCounter, digits);
        RXByteCounter = 0;
        transmit(enter_new);
        if(newSpeed != oldSpeed)
                                {
                                    driveMotor((newSpeed - oldSpeed));
                                    oldSpeed = newSpeed;
                                }

    }}
}

void transmit(char *str)
{
    while(*str != 0){
        UART_transmitData(EUSCI_A0_BASE, *str++);}
}

int convert(uint32_t ByteCounter, char *data){
    char hundreds = '0', tens = '0', ones = '0';
    if(ByteCounter == 1) ones = data[0];
    else if(ByteCounter == 2){
        ones = data[1];
        tens = data[0];}
    else{
        ones = data[2];
        tens = data[1];
        hundreds = data[0];}
    return (int)(((hundreds-0x30)*100)+((tens-0x30)*10)+(ones-0x30));
}

void driveMotor(int cycles)
{

    if(cycles > 0)
    {

        while(cycles > 0)
            {
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P10, GPIO_PIN1);
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P10, GPIO_PIN2);
            _delay_cycles(12000);
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P10, GPIO_PIN3);
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P10, GPIO_PIN4);
            _delay_cycles(12000);
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P10, GPIO_PIN2);
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P10, GPIO_PIN1);
            _delay_cycles(12000);
            MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P10, GPIO_PIN4);
            MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P10, GPIO_PIN3);
            _delay_cycles(12000);
            cycles--;
            }
    }

    if(cycles < 0)
    {
        cycles = cycles *(-1);
    while(cycles > 0)
        {
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P10, GPIO_PIN3);
    MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P10, GPIO_PIN4);
    _delay_cycles(12000);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P10, GPIO_PIN1);
    MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P10, GPIO_PIN2);
    _delay_cycles(12000);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P10, GPIO_PIN4);
    MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P10, GPIO_PIN3);
    _delay_cycles(12000);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P10, GPIO_PIN2);
    MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P10, GPIO_PIN1);
    _delay_cycles(12000);
    cycles--;
        }
    }
}
