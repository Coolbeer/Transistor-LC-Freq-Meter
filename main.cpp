/*

HD44780 Connected on:
    DB4 = PB0
    DB5 = PB1
    DB6 = PB2
    DB7 = PB3

    RS  = PD2
    RW  = PD6
    E	= PD7

Buttons:
    PD3
*/

#define F_CPU 10000000

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "lcd.h"

#define PIN0_100K PD0
#define DDR0_100K DDRD
#define PORT0_100K PORTD

#define PIN1_100K PD1
#define DDR1_100K DDRD
#define PORT1_100K PORTD

#define PIN2_100K PC3
#define DDR2_100K DDRC
#define PORT2_100K PORTC

#define PIN0_100 PC4
#define DDR0_100 DDRC
#define PORT0_100 PORTC

#define PIN1_100 PC5
#define DDR1_100 DDRC
#define PORT1_100 PORTC

#define PIN2_100 PB4
#define DDR2_100 DDRB
#define PORT2_100 PORTB


#define SOURCE_0_100K DDR0_100K |= (1 << PIN0_100K); PORT0_100K |= (1 << PIN0_100K)
#define SINK_0_100K DDR0_100K |= (1 << PIN0_100K); PORT0_100K &= ~(1 << PIN0_100K)
#define HIZ_0_100K PORT0_100K &= ~(1 << PIN0_100K); DDR0_100K &= ~(1 << PIN0_100K)

#define SOURCE_1_100K DDR1_100K |= (1 << PIN1_100K); PORT1_100K |= (1 << PIN1_100K)
#define SINK_1_100K DDR1_100K |= (1 << PIN1_100K); PORT1_100K &= ~(1 << PIN1_100K)
#define HIZ_1_100K PORT1_100K &= ~(1 << PIN1_100K); DDR1_100K &= ~(1 << PIN1_100K)

#define SOURCE_2_100K DDR2_100K |= (1 << PIN2_100K); PORT2_100K |= (1 << PIN2_100K)
#define SINK_2_100K DDR2_100K |= (1 << PIN2_100K); PORT2_100K &= ~(1 << PIN2_100K)
#define HIZ_2_100K PORT2_100K &= ~(1 << PIN2_100K); DDR2_100K &= ~(1 << PIN2_100K)


#define SOURCE_0_100 DDR0_100 |= (1 << PIN0_100); PORT0_100 |= (1 << PIN0_100)
#define SINK_0_100 DDR0_100 |= (1 << PIN0_100); PORT0_100 &= ~(1 << PIN0_100)
#define HIZ_0_100 PORT0_100 &= ~(1 << PIN0_100); DDR0_100 &= ~(1 << PIN0_100)

#define SOURCE_1_100 DDR1_100 |= (1 << PIN1_100); PORT1_100 |= (1 << PIN1_100)
#define SINK_1_100 DDR1_100 |= (1 << PIN1_100); PORT1_100 &= ~(1 << PIN1_100)
#define HIZ_1_100 PORT1_100 &= ~(1 << PIN1_100); DDR1_100 &= ~(1 << PIN1_100)

#define SOURCE_2_100 DDR2_100 |= (1 << PIN2_100); PORT2_100 |= (1 << PIN2_100)
#define SINK_2_100 DDR2_100 |= (1 << PIN2_100); PORT2_100 &= ~(1 << PIN2_100)
#define HIZ_2_100 PORT2_100 &= ~(1 << PIN2_100); DDR2_100 &= ~(1 << PIN2_100)


uint8_t volatile b,c,e;
uint16_t volatile hFE;
bool volatile isNPN;
bool volatile haveTrans;
uint16_t volatile tim0_ovf;
uint16_t volatile tim1_ovf;

uint8_t volatile mode;
uint32_t volatile freq;

#define BJT 1
#define FREQ 2

void init(void)
{
    mode = BJT;
    InitLCD(0);
    LCDClear();

    //Init ADC
    ADCSRA |= (1 << ADEN);
    ADCSRA |= (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2);

    //Pullup Button
    //PORTD |= (1 << PORTD3);
    sei();

}


void baseSource100k(void)
{
    if(b == 0)
        return;
    if(b == 1)
    {
        SOURCE_0_100K;
    }
    else if(b == 2)
    {
        SOURCE_1_100K;
    }
    else if(b == 3)
    {
        SOURCE_2_100K;
    }
}

void emitterSource100(void)
{
    if(e == 0)
        return;
    if(e == 1)
    {
        SOURCE_0_100;
    }
    else if(e == 2)
    {
        SOURCE_1_100;
    }
    else if(e == 3)
    {
        SOURCE_2_100;
    }
}

void baseSink100k(void)
{
    if(b == 0)
        return;
    if(b == 1)
    {
        SINK_0_100K;
    }
    else if(b == 2)
    {
        SINK_1_100K;
    }
    else if(b == 3)
    {
        SINK_2_100K;
    }
}

void emitterSink100(void)
{
    if(e == 0)
        return;
    if(e == 1)
    {
        SINK_0_100;
    }
    else if(e == 2)
    {
        SINK_1_100;
    }
    else if(e == 3)
    {
        SINK_2_100;
    }
}

void collectorSink100(void)
{
    if(c == 0)
        return;
    if(c == 1)
    {
        SINK_0_100;
    }
    else if(c == 2)
    {
        SINK_1_100;
    }
    else if(c == 3)
    {
        SINK_2_100;
    }
}

void collectorSource100(void)
{
    if(c == 0)
        return;
    if(c == 1)
    {
        SOURCE_0_100;
    }
    else if(c == 2)
    {
        SOURCE_1_100;
    }
    else if(c == 3)
    {
        SOURCE_2_100;
    }
}

void allHIZ(void)
{
    HIZ_0_100K;
    HIZ_0_100;
    HIZ_1_100K;
    HIZ_1_100;
    HIZ_2_100K;
    HIZ_2_100;
}

uint16_t runADC(void)
{
    ADCSRA |= (1 << ADSC);
    _delay_ms(10);
    while(!ADCSRA & (1 << ADIF))
        asm volatile ( "nop" );
    return ADC;
}

void checkhFE(void)
{
    allHIZ();

    if(isNPN)
    {
        baseSource100k();
        collectorSource100();
        emitterSink100();
        ADMUX = e-1;
    }
    else
    {
        baseSink100k();
        collectorSink100();
        emitterSource100();
        ADMUX = c-1;
    }
    _delay_ms(1);
    runADC();
    hFE = (uint32_t)5000000 / (uint32_t)(1024000/ADC);
    hFE /= 5;
    allHIZ();
}

void findPins(void)
{
    haveTrans = true;
    uint8_t sightings = 0;
    uint8_t data[6] = {0,0,0,0,0,0};

    allHIZ();

    ADMUX = 0;
    SOURCE_0_100K;
    SINK_1_100;
    _delay_ms(1);

    runADC();
    if(ADC < 175)
    {
        ++sightings;
        data[0] = 1;
        data[1] = 2;
    }
//	LCDWriteIntXY(0,0, ADC, 4);

    allHIZ();
    ADMUX = 1;
    SOURCE_1_100K;
    SINK_0_100;
    _delay_ms(1);

    runADC();

    if(ADC < 175)
    {
        data[sightings*2] = 2;
        data[sightings*2+1] = 1;
        ++sightings;
    }
//	LCDWriteIntXY(5,0, ADC, 4);

    allHIZ();
    ADMUX = 1;
    SOURCE_1_100K;
    SINK_2_100;
    _delay_ms(1);

    runADC();

    if(ADC < 175)
    {
        data[sightings*2] = 2;
        data[sightings*2+1] = 3;
        ++sightings;
    }
//	LCDWriteIntXY(10,0, ADC, 4);

    allHIZ();
    ADMUX = 2;
    SOURCE_2_100K;
    SINK_1_100;
    _delay_ms(1);

    runADC();

    if(ADC < 175)
    {
        data[sightings*2] = 3;
        data[sightings*2+1] = 2;
        ++sightings;
    }
//	LCDWriteIntXY(0,1, ADC, 4);

    allHIZ();
    ADMUX = 0;
    SOURCE_0_100K;
    SINK_2_100;
    _delay_ms(1);

    runADC();

    if(ADC < 175)
    {
        data[sightings*2] = 1;
        data[sightings*2+1] = 3;
        ++sightings;
    }
//	LCDWriteIntXY(5,1, ADC, 4);

    allHIZ();
    ADMUX = 2;
    SOURCE_2_100K;
    SINK_0_100;
    _delay_ms(1);

    runADC();

    if(ADC < 175)
    {
        data[sightings*2] = 3;
        data[sightings*2+1] = 1;
        ++sightings;
    }
//	LCDWriteIntXY(10,1, ADC, 4);

    allHIZ();

    if(sightings != 0)
    {
        if((data[1] == data[3]) || (data[1] == data[5]))
        {
            isNPN = false;
            b = data[1];
        }
        else if(data[3] == data[5])
        {
            isNPN = false;
            b = data[3];
        }
        else if(data[0] == data[2])
        {
            isNPN = true;
            b = data[0];
        }

        if(b == 1)
        {
            c = 2;
            e = 3;
        }
        else if(b == 2)
        {
            c = 1;
            e = 3;
        }
        else if(b == 3)
        {
            c = 1;
            e = 2;
        }

        checkhFE();
        uint16_t firsthFE = hFE;



        uint8_t thmp = c;
        c = e;
        e = thmp;

        checkhFE();
        if(firsthFE > hFE)
        {
            thmp = c;
            c = e;
            e = thmp;
            hFE = firsthFE;
        }
    }
    else
        haveTrans = false;
}

void runTest(void)
{
    uint16_t volatile be, bc;
    ADMUX = 0;

    //test BE drop
    if(isNPN)
    {
        baseSource100k();
        emitterSink100();
        ADMUX = b - 1;
    }
    else
    {
        baseSink100k();
        emitterSource100();
        ADMUX = b - 1;
    }
    runADC();
    be = ADC;

    allHIZ();

    //test BC drop
    if(isNPN)
    {
        baseSource100k();
        collectorSink100();
        ADMUX = b - 1;
    }
    else
    {
        baseSink100k();
        collectorSource100();
        ADMUX = b - 1;
    }
    runADC();
    bc = ADC;

    allHIZ();


    if(isNPN)
    {
        LCDWriteStringXY(0,0, "BE=");
        LCDWriteStringXY(8,1, " NPN/");
    }
    else
    {
        LCDWriteStringXY(0,0, "EB=");
        LCDWriteStringXY(8,1, " PNP/");
    }

    LCDWriteIntXY(3, 0, (uint32_t)5000000 / (uint32_t)(1024000/be), 3);
    LCDWriteString("mV");

    if(isNPN)
    {
        LCDWriteStringXY(0,1, "BC=");
    }
    else
    {
        LCDWriteStringXY(0,1, "CB=");
    }
    LCDWriteIntXY(3, 1, (uint32_t)5000000 / (uint32_t)(1024000/bc), 3);
    LCDWriteString("mV");

    LCDWriteStringXY(8,0, " hFE=");
    LCDWriteIntXY(13, 0, hFE, 3);

    LCDGotoXY(13,1);
    if(b == 1)
    {
        LCDWriteString("B");
    }
    else if(c == 1)
    {
        LCDWriteString("C");
    }
    else if(e == 1)
    {
        LCDWriteString("E");
    }

    if(b == 2)
    {
        LCDWriteString("B");
    }
    else if(c == 2)
    {
        LCDWriteString("C");
    }
    else if(e == 2)
    {
        LCDWriteString("E");
    }

    if(b == 3)
    {
        LCDWriteString("B");
    }
    else if(c == 3)
    {
        LCDWriteString("C");
    }
    else if(e == 3)
    {
        LCDWriteString("E");
    }

}


void printMes()
{
    LCDWriteStringXY(0,0, "Please          ");
    if(mode == BJT)
    {
        LCDWriteStringXY(0,1, "Insert BJT      ");
    }
}

void checkKeys(void)
{
    uint8_t volatile pins = PIND;
    uint8_t volatile static debounce;

    if(!(pins & (1 << PIND3)))
    {
        if(debounce == 2)
        {
            if(mode == BJT)
            {
                mode = FREQ;
            }
            else if(mode == FREQ)
            {
                mode = BJT;
            }
            ++debounce;
        }
        else if(debounce < 10)
            ++debounce;
    }
    else
        debounce = 0;

}

char returnCharNumber(uint8_t number)
{
    const char numbers[] = { '0','1','2','3','4','5','6','7','8','9'};
    return numbers[number];
}

void SpaceNumber(uint32_t number, char *returnValue)
{
    uint32_t divisor = 10000000;
    uint8_t numCount = 9;
    for(uint8_t teller = 0; teller != 10; ++teller)
    {
        if(teller == 2 || teller == 6)
            continue;
        returnValue[teller] = returnCharNumber(number / divisor);
        if(returnValue[teller] == '0')
        {
            if(numCount == 9)
            {
                returnValue[teller] = ' ';
            }
        }
        else
            numCount = 8;
        number = number % divisor;
        divisor /= 10;
    }
}

void checkFreq(void)
{
    char tmpp[14] = { ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','H','z','\0'};
    LCDWriteStringXY(0,0, "Frequency:      ");
    tim0_ovf = 0;
    tim1_ovf = 0;
    freq = 1;
    TCNT1 = 0;
    TCNT0 = 0;
    TIMSK0 |= (1 << TOIE0);
    TIMSK1 |= (1 << TOIE1);
    TCCR0B |= (1 << CS00) ; //| (1 << CS00);
    TCCR1B |= (1 << CS10) | (1 << CS11) | (1 << CS12);	//External clock source, trigger on rising edge.
    while(tim0_ovf != 39060)
        asm volatile ( "nop" );
    TCCR0B = 0;
    TCCR1B = 0;
    TIMSK0 &= ~(1 << TOIE0);
    freq = ((uint32_t)tim1_ovf << 16) + TCNT1;;
    SpaceNumber(freq, tmpp);
    LCDWriteStringXY(0,1, tmpp);
}

int main(void)
{
    init();

    allHIZ();

    while(1)
    {
        if(mode == BJT)
        {
            findPins();
            if(haveTrans)
                runTest();
            else
                printMes();
        }
        else if(mode == FREQ)
        {
            checkFreq();
            if(freq == 0)
                printMes();
        }
        checkKeys();
    }
    return 0;
}


ISR(TIMER0_OVF_vect)
{
    ++tim0_ovf;
}

ISR(TIMER1_OVF_vect)
{
    ++tim1_ovf;
}
