#include<SPIMemory.h>
SPIFlash flash;

//#define SPISPEED 25000000
//#define SPISPEED 35000000
#define SPISPEED 5000000
//#define FORCESIZE MB(8)

constexpr uint8_t NUM_STRINGS=3;
constexpr uint8_t MAX_LEN=17; // 16+null terminator

char write_strings[NUM_STRINGS][MAX_LEN]={
  {"*    (.)(.)    *"},
  {"* Hello,   T2  *"},
  {"* Hello, Flash *"},
};

uint32_t addresses[NUM_STRINGS] = {
  0xAEAFD,
  0x539,
  0x1337,
};

char outputString[MAX_LEN];


void write_stringno(uint8_t stringno)
{
    uint8_t err;
    uint8_t retry_count=0;
    bool eraseinit=false;
    do
    {
        WRITE_RETRY:
        if (retry_count > 15)
        {
            if (Serial) Serial.println(F("Write retry count exceeded"));
            break;
        }
        if (Serial) Serial.print(F("Writing to 0x"));
        if (Serial) Serial.println(addresses[stringno], HEX);
        flash.writeCharArray(addresses[stringno], write_strings[stringno], MAX_LEN-1);
        err = flash.error(VERBOSE);
        if (err == 0x7)
        {
            if (!eraseinit)
            {
                if (Serial) Serial.println(F("Erasing"));
                flash.eraseSection(addresses[stringno], MAX_LEN);
                //flash.eraseSector(addresses[stringno]);
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
            if (Serial) Serial.print(write_strings[stringno]);
            if (Serial) Serial.print(F(" to 0x"));
            if (Serial) Serial.println(addresses[stringno], HEX);
            break;
        }
        delay(100);
    } while (err == 0x4); // busy

    bool readstat = flash.readCharArray(addresses[stringno], outputString, sizeof(outputString)-1);
    err = flash.error(VERBOSE);
    if (readstat)
    {
      if (Serial) Serial.print(F("Read string: "));
      if (Serial) Serial.print(outputString);
      if (Serial) Serial.print(F(" from 0x"));
      if (Serial) Serial.println(addresses[stringno], HEX);
    }
    else
    {
      if (Serial) Serial.print(F("Read failed, err=0x"));
      if (Serial) Serial.println(err, HEX);
    }


}

void setup()
{
    while(!Serial);
    // Init flash chip
    flash.setClock(SPISPEED);
#ifdef FORCESIZE
    flash.begin(FORCESIZE);
#else
    flash.begin();
#endif
    flash.error(VERBOSE);

    uint32_t id = flash.getJEDECID();
    if (Serial) Serial.print(F("getJEDECID=0x"));
    if (Serial) Serial.println(id, HEX);

    for (uint8_t i=0; i < NUM_STRINGS; i++)
    {
        write_stringno(i);
    }

    if (Serial) Serial.println(F("Done"));
}

void loop() {
  // put your main code here, to run repeatedly:

}
