#include <Bounce2.h>
#include <elapsedMillis.h>
// You will likely need https://github.com/rambo/SPIMemory/tree/adjust_clock_b4_begin
#include<SPIMemory.h>

#define OEPIN 9
#define DIRPIN 8
#define CS_SENSE_PIN 14
#define CLKPIN 13
#define MOSIPIN 11
#define CSPIN 10


SPIFlash flash;

bool spi_begun = false;
void release_spi()
{
    if (spi_begun) SPI.end();
    spi_begun = false;
    pinMode(CLKPIN, INPUT);
    pinMode(MOSIPIN, INPUT);
    pinMode(CSPIN, INPUT);
}

void take_spi()
{
    pinMode(CLKPIN, OUTPUT);
    pinMode(MOSIPIN, OUTPUT);
    pinMode(CSPIN, OUTPUT);
    SPI.begin();
    spi_begun = true;
    flash_init();
}


void buf_enable()
{
  if (!digitalRead(CS_SENSE_PIN))
  {
      if (Serial) Serial.println(F("Waiting for CS to release"));
      while (!digitalRead(CS_SENSE_PIN));
  }
//  digitalWrite(DIRPIN, HIGH);
  digitalWrite(OEPIN, LOW);
}

void buf_disable()
{
  digitalWrite(OEPIN, HIGH);
//  digitalWrite(DIRPIN, LOW);
  release_spi();
}

/*
char inputString[] = "* Hello, Flash *";
uint32_t addr = 0x1337;
*/
/*
char inputString[]   = "* Hello,   T2  *";
uint32_t addr = 0x539;
*/
/*
char inputString[]   = "*    (.)(.)    *";
uint32_t addr = 0xAEAFD;
*/

uint32_t addr = 0x1337;
char inputString[]   = "*    (.)(.)    *";

char outputString[17];

bool flash_init_done = 0;

void flash_init()
{
    flash.setClock(150000);
    flash.begin();
    flash.error(VERBOSE);
    uint32_t id = flash.getJEDECID();
    uint8_t err = flash.error(VERBOSE);
    if (Serial) Serial.print(F("getJEDECID=0x"));
    if (Serial) Serial.println(id, HEX);
    if (err == 0x0)
    {
        flash_init_done = 1;
    }
}


void write_and_read_back()
{
    uint8_t err;
    uint8_t retry_count=0;
    bool eraseinit=false;
    buf_enable();
    take_spi();
    if (!flash_init_done)
    {
        flash_init();
    }
    do
    {
        WRITE_RETRY:
        if (retry_count > 5)
        {
            if (Serial) Serial.println(F("Write retry count exceeded"));
            break;
        }
        if (Serial) Serial.println(F("Writing"));
        flash.writeCharArray(addr, inputString, sizeof(inputString)-1);
        err = flash.error(VERBOSE);
        if (err == 0x7)
        {
            if (!eraseinit)
            {
                if (Serial) Serial.println(F("Erasing"));
                //flash.eraseSection(addr, sizeof(inputString)-1);
                flash.eraseSector(addr);
                err = flash.error(VERBOSE);
                eraseinit = true;
            }
            delay(1000);
            retry_count++;
            goto WRITE_RETRY;
        }
        if (err == 0x0)
        {
            if (Serial) Serial.print(F("Wrote string: "));
            if (Serial) Serial.println(inputString);
            break;
        }
        delay(100);
    } while (err == 0x4); // busy
    /*
    buf_disable();
    buf_enable();
    */
    bool readstat = flash.readCharArray(addr, outputString, sizeof(outputString)-1);
    err = flash.error(VERBOSE);
    if (readstat)
    {
      if (Serial) Serial.print(F("Read string: "));
      if (Serial) Serial.println(outputString);
    }
    else
    {
      if (Serial) Serial.print(F("Read failed, err=0x"));
      if (Serial) Serial.println(err, HEX);
    }
    buf_disable();
}

Bounce debouncer = Bounce(); // Instantiate a Bounce object

void setup()
{
    // Use as gnd for pin 2 pullup button
    pinMode(3, OUTPUT);
    digitalWrite(3, LOW);
    debouncer.attach(2,INPUT_PULLUP);
    debouncer.interval(25);

    pinMode(CS_SENSE_PIN, INPUT_PULLUP);
    
    pinMode(DIRPIN, OUTPUT);
    pinMode(OEPIN, OUTPUT);
    digitalWrite(DIRPIN, HIGH);

    /*
    buf_enable();
    flash_init();
    */

    
    buf_disable();
    
    if (Serial) Serial.println(F("Booted"));
}

elapsedMillis elapsed;

void loop()
{
    debouncer.update();
    if (debouncer.fell())
    {
        if (Serial) Serial.println(F("Button pushed"));
        write_and_read_back();
    }
    if (elapsed > 5000)
    {
        elapsed = 0;
        if (Serial) Serial.println(F("Still alive"));
    }

}
