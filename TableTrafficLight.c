// ***** 0. Documentation Section *****
// TableTrafficLight.c for Lab 10
// Runs on LM4F120/TM4C123
// Index implementation of a Moore finite state machine to operate a traffic light.  
// Daniel Valvano, Jonathan Valvano
// January 15, 2016

// east/west red light connected to PB5
// east/west yellow light connected to PB4
// east/west green light connected to PB3
// north/south facing red light connected to PB2
// north/south facing yellow light connected to PB1
// north/south facing green light connected to PB0
// pedestrian detector connected to PE2 (1=pedestrian present)
// north/south car detector connected to PE1 (1=car present)
// east/west car detector connected to PE0 (1=car present)
// "walk" light connected to PF3 (built-in green LED)
// "don't walk" light connected to PF1 (built-in red LED)


// ***** 1. Pre-processor Directives Section *****
#include "TExaS.h"
#include "tm4c123gh6pm.h"
#include "PLL.h" 
#include "SysTick.h" 

// ***** 2. Global Declarations Section *****

// FUNCTION PROTOTYPES: Each subroutine defined
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts

// ***** 3. Subroutines Section *****
//output port
void PortF_Init(void){ volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000020;     // 1) activate clock for Port F
  delay = SYSCTL_RCGC2_R;           // allow time for clock to start
  GPIO_PORTF_LOCK_R = 0x4C4F434F;   // 2) unlock GPIO Port F - only  for PF0
  GPIO_PORTF_CR_R = 0x0A;
  GPIO_PORTF_AMSEL_R = 0x00;        // 2) disable analog on PF
  GPIO_PORTF_PCTL_R &= ~0x00FFFFFF;   // 3) PCTL GPIO on PF4-0
  GPIO_PORTF_DIR_R |= 0x0A;         // 4) PF1, PF3 out
  GPIO_PORTF_AFSEL_R &= ~0x10;      // 5) disable alt funct on PF7-0
//  GPIO_PORTF_PUR_R = 0x00;          // no pull up (enable pull-up on PF4)
  GPIO_PORTF_DEN_R |= 0x0A;         // 6) enable digital I/O on PF4
} 

//input port
void PortE_Init(void){ volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000010;     // 1) activate clock for Port E
  delay = SYSCTL_RCGC2_R;           // allow time for clock to start
  GPIO_PORTE_AMSEL_R = 0x00;        // 2) disable analog on PE
  GPIO_PORTE_PCTL_R &= ~0x00FFFFFF;   // 3) PCTL GPIO on PF4-0
  GPIO_PORTE_DIR_R &= ~0x07;        // 4) PE0-2 in
  GPIO_PORTE_AFSEL_R &= ~0x10;      // 5) disable alt funct on PF7-0
//  GPIO_PORTE_PUR_R = 0x00;          // no pull up (enable pull-up on PF4)
  GPIO_PORTE_DEN_R |= 0x07;         // 6) enable digital I/O on PE0-2
}

//output port
void PortB_Init(void){ volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000002;     // 1) activate clock for Port B
  delay = SYSCTL_RCGC2_R;           // allow time for clock to start
  GPIO_PORTB_AMSEL_R = 0x00;        // 3) disable analog on PB
  GPIO_PORTB_PCTL_R &= ~0x00FFFFFF;   // 4) PCTL GPIO on PB0
  GPIO_PORTB_DIR_R |= 0x3F;         // 5) PB0-5 are out
  GPIO_PORTB_AFSEL_R &= ~0x3F;      // 6) disable alt funct on PB0-5
  GPIO_PORTB_DEN_R |= 0x3F;         // 7) enable digital I/O on PB0-5
}

// represents a State of the FSM 
struct State {
   unsigned char outF;   // output F port for the state
   unsigned char outB;   // output B port for the state
   unsigned short wait;     // Time to wait when in this state
   unsigned long next[8]; // Next state array
};

typedef const struct State StateType;

//Shortcuts to refer to the various states in the FSM array
#define GoE       0
#define WaitE     1
#define GoN       2
#define WaitN     3
#define Walk      4
#define Wait_Walk 5
#define Hurry     6


//The data structure that captures the FSM state transition graph
StateType Fsm[7] = { 
  {0x02, 0x0C, 10, {GoE, GoE, WaitE, WaitE, WaitE, WaitE, WaitE, WaitE}},         //GoE
  {0x02, 0x14, 10, {GoN, GoN, GoN, GoN, Walk, Walk, WaitE, GoN}},                 //WaitE
  {0x02, 0x21, 10, {GoN, WaitN, GoN, WaitN, WaitN, WaitN, WaitN, WaitN}},         //GoN
  {0x02, 0x22, 10, {GoE, GoE, GoE, GoE, Walk, GoE, Walk, Walk}},                  //WaitN
  {0x08, 0x24, 10, {Walk, Hurry, Hurry, Hurry, Walk, Hurry, Hurry, Hurry}},       //Walk
  {0x02, 0x24, 10, {Wait_Walk, GoE, GoN, GoN, Walk, GoE, GoN, GoE}},              //Wait_Walk
  {0x08, 0x24, 10, {WaitE, GoE, GoN, GoN, GoN, GoE, GoN, GoE}}                    //Hurry
}; 

unsigned long cState; //Current state is Even/Odd
unsigned long sw;     //use switch as cars, pedestrians
unsigned long j;      //Blink 10 times for pedestrian light

int main(void){ 

  TExaS_Init(SW_PIN_PE210, LED_PIN_PB543210,ScopeOff); // activate grader and set system clock to 80 MHz
	PLL_Init();       // 80 MHz, Program 10.1 
  SysTick_Init();   // Program 10.2
  PortF_Init();
  PortE_Init();
  PortB_Init();
  EnableInterrupts();
  cState = GoN;
  while(1){
    GPIO_PORTB_DATA_R = Fsm[cState].outB;
    if (cState!=6) {
			GPIO_PORTF_DATA_R = Fsm[cState].outF;
		} else {
			j = 10;
			GPIO_PORTF_DATA_R = Fsm[cState].outF;
			while (j!=0) {
				SysTick_Wait(6000);
				GPIO_PORTF_DATA_R ^= 0x08;
				j -= 1;
			}
		}
    SysTick_Wait10ms(Fsm[cState].wait);
    sw = GPIO_PORTE_DATA_R & 0x07;
    cState = Fsm[cState].next[sw];
  }
}

void SysTick_Init(void){
  NVIC_ST_CTRL_R = 0;               // disable SysTick during setup
  NVIC_ST_CTRL_R = 0x00000005;      // enable SysTick with core clock
}
// The delay parameter is in units of the 80 MHz core clock. (12.5 ns)
void SysTick_Wait(unsigned long delay){
  NVIC_ST_RELOAD_R = delay-1;  // number of counts to wait
  NVIC_ST_CURRENT_R = 0;       // any value written to CURRENT clears
  while((NVIC_ST_CTRL_R&0x00010000)==0){ // wait for count flag
  }
}
// 10000us equals 10ms
void SysTick_Wait10ms(unsigned long delay){
  unsigned long i;
  for(i=0; i<delay; i++){
    SysTick_Wait(800000);  // wait 10ms
  }
}
