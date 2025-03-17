/******************************************************************
* Area Ingenieria de Sistemas y 
* Automatica y Arquitectura y tecnologia de Computadores
* Sistemas Empotrados 1
*******************************************************************
* nombre fichero : main.c
*******************************************************************
* descripcion    : Practica PR1 comprobación placa
* programador    : JL Villarroel (JLV) & Enrique Torres
* lenguaje       : ANSI C
* fecha          : 8/03/2022
********************************************************************/

/******************************************************************
*                   Modulos usados
*******************************************************************/
#include <stdlib.h>
#include <msp430.h>
#include "InitSystem.h"
#include "display.h"

/******************************************************************
*                   Variables compartidas
*******************************************************************/

static volatile unsigned char pulso_flag = 0;
unsigned int antes_pulsado[6] = {0,0,0,0,0,0};
unsigned int contador_1 = 0, contador_2 = 0, contador_3 = 0;

/******************************************************************
*                   Prototipos funciones
*******************************************************************/
void Init_GPIO (void) ;
void prueba_leds(void);
void prueba_botones(void);

/******************************************************************
*                   Main y funciones
*******************************************************************/
int main(void)
{


    Stop_Watchdog () ;                  // Stop watchdog timer

    Init_CS () ;                        // MCLK = 8 MHz, SMCLK = 4 MHz
    Init_GPIO () ;
    Init_Display () ;

    __bis_SR_register(GIE);             // Activamos interrupciones en la palabra de estado

//    P1IE |= BIT1 ;                    // P1.1 Int enabled
//    P1IES &= ~BIT1 ;                  // P1.1 Int raising edge

    prueba_leds();
    P1IE |= (BIT1 + BIT2); //Interruption enable
    P1IES |= (BIT1 + BIT2); //Tipo de flanco (Bajada)

    //prueba_boton();
    while(1)
    {

        //CONTADOR 2
        if ((P1IN & BIT3) && !antes_pulsado[2]) {//detector de flancos
            antes_pulsado[2] = 1 ;
            if(contador_2 == 0){
                   contador_2 = 9;
               }
               else{
                   contador_2 = (contador_2 - 1)%10 ;
               }
        }
        else if (!(P1IN & BIT3)) antes_pulsado[2] = 0 ;

        if ((P1IN & BIT4) && !antes_pulsado[3]) {//detector de flancos
            antes_pulsado[3] = 1 ;
            contador_2 = (contador_2 + 1)%10 ;
        }
        else if (!(P1IN & BIT4)) antes_pulsado[3] = 0 ;

        //CONTADOR 3
        if ((P1IN & BIT5) && !antes_pulsado[4]) {//detector de flancos
            antes_pulsado[4] = 1 ;
            if(contador_3 == 0){
               contador_3 = 9;
           }
           else{
               contador_3 = (contador_3 - 1)%10 ;
           }
        }
        else if (!(P1IN & BIT5)) antes_pulsado[4] = 0 ;

        if ((P1IN & BIT6) && !antes_pulsado[5]) {//detector de flancos
            antes_pulsado[5] = 1 ;
            contador_3 = (contador_3 + 1)%10 ;
        }
        else if (!(P1IN & BIT6)) antes_pulsado[5] = 0 ;

        display (2, contador_1);
        display (1, contador_2);
        display (0, contador_3);

        if (pulso_flag > 0){
            pulso_flag = 0;     // OjO, secion critica? 
            //...
        } 
    }
}

void prueba_leds() {
    unsigned int i = 0;
    for (i; i < 7; i++) {
        if (i < 3){
            P2OUT = 1 << i;
        }
        else {
            P2OUT = 0;
            P4OUT = 1 << i+1;
        }
        __delay_cycles(2000000);
    }
    P4OUT = 0;
}

void prueba_boton() {
    unsigned int i = 0;
    for (i; i < 7; i++) {
        while (!(P1IN & (1 << i))) ;
        if (i < 3){
            P2OUT = 1 << i;
        }
        else {
            P2OUT = 0;
            P4OUT = 1 << i+1;
        }
    }
    __delay_cycles(2000000);
    P4OUT = 0;
    P2OUT = 0;
}

void Init_GPIO (void) {
    // P2.6,P2.7: XIN, XOUT, reloj externo
    // P1.0 salida, led de la tarjeta
    // P6.6 salida, led de la tarjeta
    // P1.1 .. P1.6 entradas, pulsadores del ascensor
    // P3 control display 7 segmentos
    // P6.0 y P6.1 --> entradas encoder
    // P6.2 y P6.3 --> salidas PWM
    // resto: sin uso

    P1DIR |= BIT0 ;                                          // led de la tarjeta
    // P1.1 - P1.6 entradas para pulsadores
    P1REN |= BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 ;       // pull-down/pull-up
    P1OUT |= BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 ;       // pull-up
    P1DIR |= BIT7 ;                                          // sin uso
    // P2.0 - P2.2 salidas para led
    P2DIR |= BIT0 | BIT1 | BIT2  ;                           // salida led
    P2DIR |= BIT3 | BIT4 | BIT5 ;                            // sin uso
    P2OUT = 0x00 ;                                           // sin uso
    // P3 control display 7 segmentos
    P3DIR |= BIT0 | BIT1 | BIT2 ;                            // D0, D1, D2
    P3DIR |= BIT3 | BIT4 | BIT5 | BIT6 ;                     // A, B, C, D
    P3DIR |= BIT7 ;                                          // sin uso
    P3OUT = 0x00 ;
    // P4.5 - P4.7 salidas para led
    P4DIR |= BIT0 | BIT1 | BIT2 | BIT3 | BIT4 ;              // sin uso
    P4DIR |= BIT5 | BIT6 | BIT7 ;                            // salida led
    P4OUT = 0x00 ;
    // P5.2 - P5.3 ADC
    P5DIR |= BIT0 | BIT1 | BIT4 ;                            // sin uso
    P5OUT = 0x00 ;
    // P6.0 - P6.1 entradas encoder
    // P6.2 - P6.3 salidas PWM
    P6DIR |= BIT2 | BIT3 ;                                   // PWM
    P6DIR |= BIT4 | BIT5 ;                                   // sin uso
    P6DIR |= BIT6 ;                                          // led de la tarjeta
    P6OUT = 0x00 ;


    PM5CTL0 &= ~LOCKLPM5;                   // Disable the GPIO power-on default high-impedance mode
                                            // to activate previously configured port settings
}

#pragma vector = PORT1_VECTOR
__interrupt void RSI_Pulso (void) {
    pulso_flag++;
    unsigned int valueP1IV = P1IV;
    if (valueP1IV == 4) {
            if(contador_1 == 0){
                contador_1 = 9;
            }
            else{
                contador_1 = (contador_1 - 1)%10 ;
            }
    }
    if (valueP1IV == 6) {
            contador_1 = (contador_1 + 1)%10 ;
    }
    P1IFG &= ~(BIT1 + BIT2);   // Limpiar flags
    return ;
}

