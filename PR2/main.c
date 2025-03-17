/******************************************************************
* Area Ingenieria de Sistemas y Automatica
* Sistemas Electronicos Programables
*******************************************************************
* nombre fichero : main.c
*******************************************************************
* descripcion    : Practica PR6 medicion y gestion tiempo
* programador    : JL Villarroel (JLV)
* lenguaje       : ANSI C
* fecha          : 16/02/2022
********************************************************************/

/******************************************************************
*                   Modulos usados
*******************************************************************/

#include <stdlib.h>
#include <msp430.h>
#include "InitSystem.h"
#include "clock.h"
#include "display.h"


/******************************************************************
*                   Variables globales
*******************************************************************/
unsigned int siguiente, periodo ;
unsigned int t_reaccion ;

Timer_id Ti ;

unsigned char disp = 0 ;
unsigned char Centesimas, Decimas, Segundos ;

// Mis variables
bool button_pushed = false;
unsigned int t_before_push, t_after_push, ticks_elapsed;
/******************************************************************
*                   Prototipos funciones
*******************************************************************/

void Init_GPIO(void) ;
void random_wait(void) ; 
void display_score(void) ;
void turn_on_led(void) ;
/******************************************************************
*                   Main y funciones
*******************************************************************/

int main(void)
{
    Stop_Watchdog () ;                  // Stop watchdog timer

    Init_CS () ;                        // MCLK = 8 MHz, SMCLK = 4 MHz
    Init_Clock () ;
    Init_GPIO () ;
    Init_Display () ;

    while(1)
    {
        random_wait();
        turn_on_led();
        t_before_push = Get_Time(); 
        while(!button_pushed);  // Espera activa (seguro q se puede hacer durmiendo a la cpu zzzzzzzzz.....)
        t_after_push = Get_Time(); 
        ticks_elapsed = t_after_push - t_before_push;
        turn_off_led();
        display_score();
        while(!button_pushed); // Esperar a q le vuelva a dar para resetear el juego
        reset_game();
    }
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

void random_wait(void) {
    float random_wait = (rand() % 4000) + 1000; // Espera entre 1s y 5s
    delay(random_wait); // TODO ESPERAR IDK COMO SE HACE EN EL MICRO SIN LA GUARRADA DE LOS CICLOS creo q asi   
}

void display_score(void) {
    // Cada tick => 1ms
    // TODO checkear esto q creo q está bien pero no estoy seguro
    Segundos = ticks_elapsed % 1000
    Decimas = (ticks_elapsed % 100) - Segundos*10;
    Centesimas = (ticks_elapsed % 10) - Decimas*10 - Segundos*100;

    // TODO VER SI SE PUEDE USAR LA FUNCION DISPLAY O LA HACEMOS NOSOTROS
    display(2, Segundos);
    display(1, Decimas);
    display(0, Centesimas);
}
void turn_on_led(void) {
    P2OUT = 1; // Puerto P2.0 idk si está bien
}
void turn_off_led(void) {
    P2OUT = 0;  
}
void reset_game(void) {
    // Resetear timers
    display(0,0);
    display(1,0);
    display(2,0);
}

// Interrupcion del botón q está en el P1.1
#pragma vector = PORT1_VECTOR 
__interrupt void Pulso (void) {
    if (P1IV) {
        button_pushed = (P1IN != 0); // Boton pulsado si hay algo en P1IN
    }
    return ;
}

