#include <Bounce2.h>
#include <elapsedMillis.h>

#include <LCD.h>
#include <LiquidCrystal_SR3W.h>
// LiquidCrystal_SR3W lcd(DataPin, ClockPin, LatchPin, sr_enable, sr_rw, sr_rs, sr_d4, sr_d5, sr_d6, sr_d7, sr_bl, bl_pol);
LiquidCrystal_SR3W lcd(4, 3, 2, 6, 7, 0 , 5, 4, 3, 2, 1, POSITIVE); 

#define SPI_SPEED 1000000

#define CLKPIN 13
#define MOSIPIN 11
#define CSPIN 10
#include<SPIMemory.h>
SPIFlash flash;

//uint32_t addr = 0x539;
//uint32_t addr = 0xAEAFD;
uint32_t str_addr = 0x1337;
char flashstring[17];

elapsedMillis elapsed;
elapsedMillis elapsed_flashread;
Bounce debouncer = Bounce(); // Instantiate a Bounce object

bool spi_taken = false;
bool spi_released = false;

void release_spi()
{
#ifdef NORELEASE_SPI
    return;
#endif
    if (spi_released)
    {
       return;
    }
    SPI.end();
    pinMode(CLKPIN, INPUT);
    pinMode(MOSIPIN, INPUT);
    pinMode(CSPIN, INPUT);
    spi_released = true;
    spi_taken = false;
}

void take_spi()
{
    if (spi_taken)
    {
        return;
    }
    if (spi_released)
    {
        pinMode(CLKPIN, OUTPUT);
        pinMode(MOSIPIN, OUTPUT);
        pinMode(CSPIN, OUTPUT);
        SPI.begin();
    }
    spi_released = false;
    // Init flash chip
    flash.setClock(SPI_SPEED);
    flash.begin();
    flash.error(VERBOSE);
    spi_taken=true;
}

void setup()
{
    debouncer.attach(22,INPUT_PULLUP);
    debouncer.interval(25);
    
    // Init LCD
    lcd.begin ( 16, 2 );
    lcd.clear();
    lcd.print(F("Hello World"));
    lcd.on();

}

bool flashread_cleared;
void loop()
{
    debouncer.update();

    if (debouncer.fell())
    {
        take_spi();
        uint32_t id = flash.getJEDECID();
        if (Serial) Serial.print(F("getJEDECID=0x"));
        if (Serial) Serial.println(id, HEX);
    
        elapsed_flashread = 0;
        flashread_cleared = 0;
        lcd.clear();
        bool stat = flash.readCharArray(str_addr, flashstring, sizeof(flashstring)-1);
        flash.error(VERBOSE);
        release_spi();
        if (stat)
        {
            if (Serial)
            {
               Serial.print(F("Got string: "));
               Serial.println(flashstring);
            }
            lcd.print(flashstring);
        }
        else
        {
           lcd.print(F("Read fail"));
        }
    }
    if (!flashread_cleared && elapsed_flashread > 5000)
    {
        lcd.clear();
        lcd.print("Push the button!");
        flashread_cleared = 1;
    }
    if (elapsed > 500)
    {
        elapsed = 0;
        lcd.setCursor(0,1);
        lcd.print("t: ");
        lcd.print(millis(), DEC);
    }
}
