#include <SPI.h>


char sendbuffer[] ="Hello world!";

void setup()
{
  SPI.begin();  

}

void loop()
{
    SPI.beginTransaction(SPISettings(20000, MSBFIRST, SPI_MODE0));
    SPI.transfer(sendbuffer, sizeof(sendbuffer)-1);
    SPI.endTransaction();
    delay(1000);

}
