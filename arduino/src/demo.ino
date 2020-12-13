#include <avr/sleep.h>

#define LED_CLK 1
#define LED_DATA 0

#define BUTTON_PIN 2
//Pin change interrupt setup also has to be changed, if you change BUTTON_PIN

#define LEDS 17

#include "scroll.h"

typedef struct
{
  uint8_t r;
  uint8_t g;
  uint8_t b;
} rgb_t;

rgb_t leds[LEDS];

void sendBit(uint8_t b)
{
  //delayMicroseconds(2);
  digitalWrite(LED_DATA, b);
  //delayMicroseconds(2);
  digitalWrite(LED_CLK, 1);
  //delayMicroseconds(2);
  digitalWrite(LED_CLK, 0);
  //delayMicroseconds(2);
}

void clear()
{
  uint8_t i;

  for (i=0; i<LEDS; i++)
  {
    leds[i].r=0;
    leds[i].g=0;
    leds[i].b=0;
  }
}

void setPixel(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b)
{
  uint8_t idx;
  if (y==0)
  {
    if (x==0)
    {
      return;
    }
    else
    {
      idx=x-1;
    }
  }
  else if (y==1)
  {
    idx=10-x;
  }
  else if (y==2)
  {
    idx=11+x;
  }
  else
  {
    return;
  }
  leds[idx].r=r;
  leds[idx].g=g;
  leds[idx].b=b;
}

// -----------------------------
// |          FAST SIN/COS     |
// -----------------------------
static const int8_t PROGMEM sinTable[17]={0, 12, 25, 37, 49, 60, 71, 81, 90, 98, 106, 112, 117, 122, 125, 126, 127};

int8_t fs(uint8_t a) //Full wave: 0..63, values: -127..127
{
  uint8_t n;
  int8_t r;
  a&=63;
  n=(a>=32);
  a&=31;
  a=(a<=16)?a:32-a;
  r=pgm_read_byte(&(sinTable[a]));
  return n?-r:r;
}

int8_t fc(uint8_t a)
{
  uint8_t n;
  int8_t r;
  a+=16;
  a&=63;
  n=(a>=32);
  a&=31;
  a=(a<=16)?a:32-a;
  r=pgm_read_byte(&(sinTable[a]));
  return n?-r:r;
}

void updateLeds()
{
  uint8_t i;
  int8_t j;
  for (j=0; j<32; j++)
  {
    sendBit(0);
  }
  for (i=0; i<LEDS; i++)
  {
    sendBit(1);
    sendBit(1);
    sendBit(1);
    
    sendBit(0);
    sendBit(0);
    sendBit(0);
    sendBit(0);
    sendBit(1);

    for (j=7; j>=0; j--)
    {
      sendBit((leds[i].b>>j) & 1);
    }
    for (j=7; j>=0; j--)
    {
      sendBit((leds[i].g>>j) & 1);
    }
    for (j=7; j>=0; j--)
    {
      sendBit((leds[i].r>>j) & 1);
    }
  }
  for (j=0; j<32; j++)
  {
    sendBit(1);
  }
  digitalWrite(LED_DATA, 0);
  digitalWrite(LED_CLK, 0);
  delay(1);
}


void newColor(uint8_t idx)
{
  if (!(leds[idx].r | leds[idx].g | leds[idx].b))
  {
    int8_t a, b, c;
    a=rand() % 3;
    b=rand() % 20;
    c=15-b;
    b=b-5;
    if (b<0)
    {
      b=0;
    }
    if (c<0)
    {
      c=0;
    }
    if (b>10)
    {
      b=10;
    }
    if (c>10)
    {
      c=10;
    }
    if (a==0)
    {
      leds[idx].r=b;
      leds[idx].g=c;
      leds[idx].b=0;
    }
    else if (a==1)
    {
      leds[idx].r=0;
      leds[idx].g=b;
      leds[idx].b=c;
    }
    else
    {
      leds[idx].r=c;
      leds[idx].g=0;
      leds[idx].b=b;
    }
  }
}

void plasma_part(uint8_t sub)
{
  static uint16_t cl=0;
  int8_t x, y;
  static int8_t mx=7;
  if (sub==1 && cl>1836 && (cl & 15) == 0 && mx>0)
  {
    mx--;
  }
  for (y=0; y<=2; y++)
  {
    for (x=0; x<6; x++)
    {
      int16_t r, g, b;
      int16_t cx, cy;
      uint8_t d, q;
      //cx=x*fc(cl)+y*fs(cl);
      //cy=x*fs(cl)-y*fc(cl);
      if (sub==0)
      {
        cx=x;
        cy=y;
        r=fs(cx*3.264+1.472*cy+0.93*cl)*fs(2.368*cx-2.176*cy+0.51*cl)/64;
        g=fc(cx*4.672+2.752*cy+0.51*cl)*fc(4.224*cx+2.240*cy-0.36*cl-7)/64;
        b=fs(cx*4.224+3.392*cy+0.69*cl+9)*fc(1.600*cx-1.984*cy+0.93*cl)/64;
      }
      else
      {
        cx=x*fc(cl)+y*fs(cl);
        cy=x*64;
        r=fs(cx*0.051+0.31*cl);
        g=fc(cx*0.073+0.17*cl);
        b=fs(cx*0.066+0.23*cl+9);
      }
      if (r<0)
      {
        r=-r;
      }
      if (r>255)
      {
        r=255;
      }
      if (g<0)
      {
        g=-g;
      }
      if (g>255)
      {
        g=255;
      }
      if (b<0)
      {
        b=-b;
      }
      if (b>255)
      {
        b=255;
      }
      d=r;
      if (g<d)
      {
        d=g;
      }
      if (b<d)
      {
        d=b;
      }
      q=r;
      if (g>q)
      {
        q=g;
      }
      if (b>q)
      {
        q=b;
      }
      q-=d;
      if (q<4)
      {
        d-=(4-q);
      }
      r-=d;
      g-=d;
      b-=d;
      if (x>=mx)
      {
        r=0;
      }
      if (x+1>=mx)
      {
        g=0;
      }
      if (x+2>=mx)
      {
        b=0;
      }
      setPixel(x, y, r, g, b);
    }
  }
  cl++;
}

void setup()
{
  pinMode(LED_CLK, OUTPUT);
  pinMode(LED_DATA, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  digitalWrite(LED_DATA, 0);
  digitalWrite(LED_CLK, 0);
  delay(10);

  GIMSK = 0b00100000;    // turns on pin change interrupts
  PCMSK = 0b00000100;    // turn on interrupts on pins PB2
  sei();                 // enables interrupts

  clear();
  updateLeds();
}

static uint8_t scroll_pos=0;
void scroll_part(uint8_t sub)
{
  uint8_t x, y, r, g, b;
  clear();
  for (y=0; y<=2; y++)
  {
    for (x=0; x<=5; x++)
    {
      uint8_t p=x+scroll_pos;
      r=30;
      g=30;
      b=30;
      if (sub==0)
      {
        if (p>68)
        {
          r=60;
          g=0;
          b=6;
        }
        else if (p>53)
        {
          r=35;
          g=0;
          b=35;
        }
      }
      else if (sub==1)
      {
        if (p>56)
        {
        }
        else if (p>52)
        {
          r=0;
          g=0;
          b=64;
        }
        else if (p>48)
        {
          r=0;
          g=64;
          b=0;
        }
        else if (p>44)
        {
          r=64;
          g=0;
          b=0;
        }
      }
      else if (sub==2)
      {
        if (p>33)
        {
          r=72;
          g=5;
          b=5;
        }
      }
      if (! (x==0 && y==0))
      {
        int8_t d;
        if (sub==0)
        { 
          d=pgm_read_byte(&(scroll1[y][p>>3]));
        }
        else if (sub==1)
        {
          d=pgm_read_byte(&(scroll2[y][p>>3]));
        }
        else if (sub==2)
        {
          d=pgm_read_byte(&(scroll3[y][p>>3]));
        }
        else if (sub==3)
        {
          d=pgm_read_byte(&(scroll4[y][p>>3]));
        }
        else if (sub==4)
        {
          d=pgm_read_byte(&(scroll5[y][p>>3]));
        }
        if (d & (1<<(p&7)))
        {
          setPixel(x, y, r, g, b);
        }
      }
    }
  }
  scroll_pos++;
}

void snake_part()
{
#define SL 4
  static uint8_t p=0;
  static uint8_t snaker[SL]={1*16+0, 1*16+0, 1*16+0, 1*16+0};
  static uint8_t snakeb[SL]={1*16+5, 1*16+5, 1*16+5, 1*16+5};
  static const int8_t PROGMEM dirr[]={  1,   1,   1, 16,   1, -16, -16,  -1,  -1,  -1,  16,   1,  1,  1, 0};
  static const int8_t PROGMEM dirb[]={ -1, -16,  -1, -1,  -1,  16,  16,   1,   1,   1,   1, -16, -1, -1, 0};
  static uint8_t st=0;
  static uint8_t gover=0;
  int8_t i;
  clear();
  for (i=SL-1; i>=0; i--)
  {
    setPixel(snaker[i] & 15, snaker[i] >> 4, 40, i == 0 ? p & 1 ? 40 : 0 : 0, i == 0 ? p & 1 ? 40 : 0 : 0);
    setPixel(snakeb[i] & 15, snakeb[i] >> 4, i == 0 ? p & 1 ? 40 : 0 : 0, i == 0 ? p & 1 ? 40 : 0 : 0, 40);
  }
  if (p % 8 == 0)
  {
    if (!gover)
    {
      for (i=SL-2; i>=0; i--)
      {
        snaker[i+1]=snaker[i];
        snakeb[i+1]=snakeb[i];
      }
      snaker[0]+=pgm_read_byte(&(dirr[st]));
      snakeb[0]+=pgm_read_byte(&(dirb[st]));
      st++;
    }
    if (pgm_read_byte(&(dirr[st]))==0)
    {
      gover=1;
    }
  }
  p++;
}

void noise_part()
{
  uint8_t r, x, y;
  for (y=0; y<3; y++)
  {
    r=rand();
    for (x=0; x<6; x++)
    {
      uint8_t b;
      b=r & 1;
      r>>=1;
      b<<=4;
      setPixel(x, y, b, b, b);
    }
  }
}

void(* soft_reset) (void) = 0;

ISR(PCINT0_vect)
{
  soft_reset();
}

void loop()
{
  static uint16_t frame=0;

  //noise_part();
  if (frame<78)
  {
    scroll_part(0);
    delay(100);
  }
  else if (frame<296)
  {
    snake_part();
    delay(10);
    scroll_pos=0;
  }
  else if (frame<364)
  {
    scroll_part(1);
    delay(100);
  }
  else if (frame<1300)
  {
    plasma_part(0);
    scroll_pos=0;
  }
  else if (frame<1350)
  {
    scroll_part(2);
    delay(100);
  }
  else if (frame<2400)
  {
    plasma_part(1);
    scroll_pos=0;
  }
  else if (frame<2460)
  {
    scroll_part(3);
    delay(100);
  }
  else if (frame<2475)
  {
    delay(100);
    scroll_pos=0;
  }
  else if (frame<2535)
  {
    scroll_part(4);
    delay(100);
  }
  else if (frame<2700)
  {
    delay(100);
  }
  else
  {
    clear();
    updateLeds();
    //sleep_cpu();
    soft_reset();
  }
  updateLeds();

  frame++;
}
