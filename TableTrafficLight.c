// ***** 0. Documentation Section *****
// TableTrafficLight.c 
// Author: Timothy Nguyen
// Credits: Dr. Jonathan Valvano and Dr. Ramesh Yerraballi

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

#define GOSOUTH 0
#define WAITSOUTH 1
#define GOWEST 2
#define WAITWEST 3
#define WALK 4
#define DONTWALK1 5
#define ALLOFF1 6
#define DONTWALK2 7
#define ALLOFF2 8


// ***** 2. Global Declarations Section *****

struct state{
		unsigned long walkLEDs; //PF3 & PF1
		unsigned long trafficLEDs; //PB5:0
		unsigned long time; //delay before transition to next state
		unsigned long nextState[8]; //eight possible next states depending on three inputs
};

typedef const struct state s;

// FUNCTION PROTOTYPES: Each subroutine defined

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void initPortF(void);
void initPortE(void); 
void initPortB(void);
void initSysTick(void);
void setSysTick(unsigned long delay);
void SysTick_Wait10ms(unsigned long delay);

// ***** 3. Subroutines Section *****

int main(void){ 
	
		s states[9] = {{0x2, 0x21, 500, {GOSOUTH, WAITSOUTH, GOSOUTH, WAITSOUTH, WAITSOUTH, WAITSOUTH, WAITSOUTH, WAITSOUTH}},
                 {0x2, 0x22, 250, {WAITSOUTH, GOWEST, GOSOUTH, GOWEST, WALK, WALK, WALK, WALK}},
								 {0x2, 0xc, 500, {GOWEST, GOWEST, WAITWEST, WAITWEST, WAITWEST, WAITWEST, WAITWEST, WAITWEST}},
								 {0x2, 0x14, 250, {WAITWEST, GOWEST, GOSOUTH, GOSOUTH, WALK, WALK, WALK, GOSOUTH}},
								 {0x8, 0x24, 500, {WALK, DONTWALK1, DONTWALK1, DONTWALK1, WALK, DONTWALK1, DONTWALK1, DONTWALK1}}, 
								 {0x2, 0x24,  150, {DONTWALK1, ALLOFF1, ALLOFF1, ALLOFF1, WALK, ALLOFF1, ALLOFF1, ALLOFF1}}, 
								 {0, 0x24,  150, {ALLOFF1, DONTWALK2, DONTWALK2, DONTWALK2, WALK, DONTWALK2, DONTWALK2, DONTWALK2}},
								 {0x2, 0x24,  150, {DONTWALK2, ALLOFF2, ALLOFF2, ALLOFF2, WALK, ALLOFF2, ALLOFF2, ALLOFF2}},
								 {0, 0x24,  150, {ALLOFF2, GOWEST, GOSOUTH, GOWEST, WALK, GOWEST, GOSOUTH, GOWEST}}}; 
		
		unsigned long presentState = ALLOFF2;
		unsigned long input;
		
  TExaS_Init(SW_PIN_PE210, LED_PIN_PB543210,ScopeOff); 
	initPortF();
	initPortE();
	initPortB();
	initSysTick();
  EnableInterrupts();
								 
  while(1){
		 GPIO_PORTF_DATA_R = states[presentState].walkLEDs; //output based on present state
     GPIO_PORTB_DATA_R = states[presentState].trafficLEDs;
		 SysTick_Wait10ms(states[presentState].time); //delay for certain time relevant to state
		 input = GPIO_PORTE_DATA_R & 0x7; //get intput
     presentState = states[presentState].nextState[input]; //transition to next state depending on present state and inputs
  }
	
}

void initPortF(){
	unsigned long delay;
	SYSCTL_RCGC2_R |= 0x20;
	delay = SYSCTL_RCGC2_R;
	GPIO_PORTF_DIR_R |= 0xa; //enable PF3 and PF1 as output pins
	GPIO_PORTF_DEN_R |= 0xa;
	
}

void initPortE(){
	unsigned long delay;
	SYSCTL_RCGC2_R |= 0x10;
	delay = SYSCTL_RCGC2_R;
	GPIO_PORTE_DEN_R |= 0x7; //enable PE2:0 as digital pins
}

void initPortB(){
	unsigned long delay; 
	SYSCTL_RCGC2_R |= 0x2;
	delay = SYSCTL_RCGC2_R;
	GPIO_PORTB_DIR_R |= 0x3f; //enable PB5:0 as output pins
	GPIO_PORTB_DEN_R |= 0x3f;
}

void initSysTick(void){
	NVIC_ST_RELOAD_R = 79999999;
	NVIC_ST_CURRENT_R = 0;
	NVIC_ST_CTRL_R = 0x5; //start timer with system clock
}

void setSysTick(unsigned long delay){
	NVIC_ST_RELOAD_R = delay - 1; // timer starts at 0 and wraps around to reload value 
	NVIC_ST_CURRENT_R = 0;
	while((NVIC_ST_CTRL_R & 0x00010000) == 0){} //wait for COUNT flag to be set
}

// 10000us equals 10ms
void SysTick_Wait10ms(unsigned long delay){
  unsigned long i;
  for(i=0; i<delay; i++){
    setSysTick(800000);  // wait 10ms
  }
}   

