/******************************************************************************/
/*                                                                            */
/*                 Area Ingenieria de Sistemas y Automatica                   */
/*           Departamento de Informatica e Ingenieria de Sistemas             */
/*                         Universidad de Zaragoza                            */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/* proyecto  : PRACTICAS SEP - SEI UNIZAR                                     */
/* fichero   : pwm.c                                                          */
/* version   : 1                                                              */
/* fecha     : 19/02/2022                                                     */
/* autor     : Jose Luis Villarroel                                           */
/* descripcion : Practica PR3 (ascensor) (MSP430)                             */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                        Modulos usados                                      */
/******************************************************************************/

#include <msp430.h>
#include "InitSystem.h"
#include "display.h"
#include "motor_asc.h"


/******************************************************************************/
/*                        Simbolos                                            */
/******************************************************************************/

/* Las entradas se leen del puerto P1
     P1.1 --> B_PB
     P1.2 --> B_P1
     P1.3 --> B_P2
     P1.4 --> FCB
     P1.5 --> FC1
     P1.6 --> FC2
 Las salidas led se envian por el puerto P2 y P4
     P2.0 --> Led piso Bajo
     P2.1 --> Led piso 1
     P2.2 --> Led piso 2
     P4.4 --> Led emergencia
     P2.4 --> ciclo */


//Sensores de planta
#define FCB      0x10     // 0001_0000
#define FC1      0x20     // 0010_0000
#define FC2      0x40     // 0100_0000

//Botones para indicar planta a la que debe ir el ascensor
#define B_PB      0x02     // 0000_0010
#define B_P1      0x04     // 0000_0100
#define B_P2      0x08     // 0000_1000

//Indicador de Planta
#define Led_PB      0x01   // 0000_0001
#define Led_P1      0x02   // 0000_0010
#define Led_P2      0x04   // 0000_0100
#define Led_EMG     0x08   // 0000_1000


/******************************************************************************/
/*                        Variables globales                                  */
/******************************************************************************/

enum {INICIAL, P0, P1, P2, P0_S_P2, P0_S_P1, P1_S_P2, P2_B_P1, P2_B_P0, P1_B_P0} estado ;  // Definir los estados del ascensor

ORDEN Orden_Ascensor = PARAR ;

unsigned char Entrada;
unsigned char Salida;


/******************************************************************************/
/*                        Prototipos funciones                                */
/******************************************************************************/

void Init_GPIO (void) ;

static unsigned char LeerEntrada(void);
static void GenerarSalida(unsigned char Salida);

Timer_id Ti ;



/******************************************************************************/
/*                        Main y funciones                                    */
/******************************************************************************/

int main(void)
{
Stop_Watchdog();                  // Stop watchdog timer

Init_CS();                        // MCLK = 8 MHz, SMCLK = 4 MHz
Init_GPIO();

Init_motor_asc();

Salida = Led_EMG;
estado = INICIAL;


while (1)
{

    P2OUT ^= BIT4; // estopaq
    Entrada = LeerEntrada();

    switch (estado) // TODO: añadir 7segmentos y salida en el automata son los led??
    {
    case INICIAL:
        Salida = Led_EMG | Led_PB | Led_P1 | Led_P2; // q hace led_EMG y que hace salida
        Orden_Ascensor = BAJAR;
        if (Entrada & FCB)
           estado = P0;
        break;
    // cuando llego a cada piso el estado se actualiza auto?
    case P0:
        Salida = Led_PB;
        // si B1 o B2 pulsados
        if (Entrada & B_P1)
            Orden_Ascensor = SUBIR;
            estado = P0_S_P1;

        if (Entrada & B_P2)
            Orden_Ascensor = SUBIR;
            estado = P0_S_P2;

        break;

    case P1:
        Salida = Led_P1;

        if (Entrada & B_PB)
            Orden_Ascensor = BAJAR;
            estado = P1_B_P0;

        if (Entrada & B_P2)
            Orden_Ascensor = SUBIR;
            estado = P1_S_P2;

        break;
    case P2:
        Salida = Led_P2;
        //Ti = Set_Timer(5000, ONE_SHOT, 0); // TODO: q pasa si quiero poner uno y ya esta puesto?
        if (Entrada & B_PB)
            Orden_Ascensor = BAJAR;
            estado = P2_B_P0;

        if (Entrada & B_P1)
            Orden_Ascensor = BAJAR;
            estado = P2_B_P1;
        break;

    case P0_S_P2:
        Orden_Ascensor = SUBIR;
        Salida = Led_PB;
        if (FC1)
            estado = P1_S_P2;
        break;

    case P0_S_P1: // caundo llego aqui led piso
        Orden_Ascensor = SUBIR;
        Salida = Led_P1;
        if (FC1) // TODO: habria que poner aqui el actualizar el led??
            estado = P1;
        break;

    case P1_S_P2: // caundo llego aqui led piso 1
        Orden_Ascensor = SUBIR;
        Salida = Led_PB;

        if (FC2)
            estado = P2;
        break;

    case P2_B_P0:
        Orden_Ascensor = BAJAR;
        Salida = Led_P2;
        if (FC1)
            estado = P1_B_P0;
        break;

    case P2_B_P1: // caundo llego aqui led piso
        Orden_Ascensor = BAJAR;
        Salida = Led_P2;
        if (FC1)
            estado = P1;
        break;

    case P1_B_P0: // caundo llego aqui led piso 1
        Orden_Ascensor = BAJAR;
        Salida = Led_P1;

        if (FC2)
            estado = P2;
        break;

    }
    /*if(Time_Out(Ti) ) { // TODO: poner en otro lao?
        // Si pasan los 5 seg bajo
        Remove_Timer(Ti);
        if (estado == P2) {
            estado = P2_B_P0;
        }

    }*/

    GenerarSalida(Salida);  //generacion de salida
    display(0, Salida); //muestra en el display 0 el piso en el que está
    Orden_motor_asc(Orden_Ascensor);


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
    // P1.1 - P1.7 entradas para pulsadores
    P1REN |= BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7 ;// pull-down/pull-up
    P1OUT |= BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 ;       // pull-up
    P1OUT &= ~BIT7 ;                                         // pull-down
    // P2.0 - P2.2 salidas para led
    P2DIR |= BIT0 | BIT1 | BIT2  ;                           // salida led
    P2DIR |= BIT3 | BIT4 | BIT5 ;                            // sin uso
    P2OUT = 0x00 ;                                           // sin uso
    // P3 control display 7 segmentos
    P3DIR |= BIT0 | BIT1 | BIT2 ;                            // D0, D1, D2
    P3DIR |= BIT3 | BIT4 | BIT5 | BIT6 ;                     // A, B, C, D
    P3REN |= BIT7 ;                                          // pull-down/pull-up
    P3OUT &= ~BIT7 ;                                         // pull-down
    P3OUT = 0x00 ;
    // P4.5 - P4.7 salidas para led
    P4DIR |= BIT0 | BIT1 | BIT2 | BIT3  ;                    // sin uso
    P4DIR |= BIT4 | BIT5 | BIT6 | BIT7 ;                     // salida led
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

static unsigned char LeerEntrada(void)
{
   return (P1IN)&0x7E ;
}

static void GenerarSalida(unsigned char Salida)
{
  P2OUT = (Salida & 0x07) | (P2OUT & 0xF8);
  if (Salida & Led_EMG) P4OUT |= BIT4 ;
  else P4OUT &= ~BIT4 ;
}


