/* W2DEN_GPS_v1.01
This is the first publishes version of a GPS display sketch
written for the Teensy 3.1
most of the code was 'borrowed' from other sources
GitHub: https://github.com/W2DEN/GPS
*/
//for the GPS
#include "TinyGPS++.h"

// for the display
#include "SPI.h"
#include "ILI9341_t3.h"

//Now set up the GPS
static const uint32_t GPSBaud = 9600;
static const int line=25;
#define thisver "1.0" ////////////////////////////////// VERSION
// The TinyGPS++ object
TinyGPSPlus gps;
// The serial connection to the GPS device
#define ss Serial1

//Now set up the display
/*
Parameter Defs: ILI9341_t3(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_CLK, TFT_MISO); 
to free up D13 for future PTT; TFT_CLK is SPI.sck changed 13 to 14
*/
ILI9341_t3 tft = ILI9341_t3(10, 9, 8, 11, 14, 12);

void setup()
{
  tft.begin();  
  tft.fillScreen(ILI9341_BLACK);
  tft.setRotation(0);
  ss.begin(GPSBaud);
  analogReference(EXTERNAL);
  analogReadResolution(12);
  analogReadAveraging(32);
}

void loop()
{
  printDateTime(gps.date, gps.time);
  //printLocation(gps.location.lat,gps.location.lng);
  //tft.println();
  if (gps.location.isValid())
  {
    tft.setTextSize(3);
    tft.setCursor(0, line*3);
    tft.setTextColor(ILI9341_GREEN,ILI9341_BLACK);  
    toDegrees(gps.location.lat());
    
    tft.setTextSize(2);
    if (gps.location.lat() <0)
    {
      tft.println(" S");
    }
     else
    {
       tft.println(" N"); 
    }
    tft.setTextSize(3);
    tft.setCursor(0, line*5);
    tft.setTextColor(ILI9341_YELLOW,ILI9341_BLACK); 
    toDegrees(gps.location.lng());
    tft.setTextSize(2);
    if (gps.location.lng() <0)
    {
      tft.println(" W");
    }
     else
    {
       tft.println(" E"); 
    }
    tft.setTextSize(3);
  }
  else
{
  tft.setCursor(0, line*3);
  tft.println("*lat/long*"); 
}
  //tft.println();
  tft.setCursor(0, line*7);
  tft.setTextColor(ILI9341_MAGENTA,ILI9341_BLACK);
  tft.setTextSize(4);
  tft.print("          ");
  tft.setCursor(0, line*7);
  printFloat(gps.speed.mph(), gps.speed.isValid(), 6,1,false ); 
  tft.setTextSize(3);
  printStr(" mph",4,true);
  tft.setCursor(0, line*9);
  tft.setTextSize(4);
  tft.print("          ");
  tft.setTextSize(3);
  tft.setCursor(0, line*9);
  tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
  printFloat(gps.course.deg(), gps.course.isValid(), 7, 1,false);
  printStr(" deg",4,true);
  tft.setCursor(0, 280);
  tft.setTextSize(2);
  printStr("Sats:", 6,false);
  printInt(gps.satellites.value(), gps.satellites.isValid(), 5,false);
  /*
  VCC measurement: this code comes from the pjrc forum to attempt to display low VCC voltages.
  handy if opperating from a battery. 3 AAA bats will run the GPS for ~ 12 hrs with the display 0n
  the display LED could be controlled through a xistor.
  Note vcc= k/A39
  https://forum.pjrc.com/threads/26117-Teensy-3-1-Voltage-sensing-and-low-battery-alert
  */
  float x = analogRead(39);
  if (x<=1500) // approximately 3 vdc
  {
    tft.setTextColor(ILI9341_GREEN);
    tft.print("V+ OK");
  }
  else
  {
  tft.setTextColor(ILI9341_RED);
  tft.print("V+ ");
  printFloat((((178*x*x + 2688757565 - 1184375 * x) / 372346))/1000,true,4,2,true);
  }
  tft.setCursor(170,300);
  tft.setTextColor(ILI9341_GREEN);
  tft.setTextSize(1);
  printStr(thisver,6,false); // display version
  
  smartDelay(6000); //this is the cycle time and gets GPS data
}

/* /////////////////////////// functions ////////////////////////////
//
//
//////////////////////////////////////////////////////////////// */
static void toDegrees(float val)
// converts decimal degrees to degrees and decimal minutes (APRS format)
{
  float wlong = val;
  char charVal[10];
  int deglong = wlong; //long;
  wlong -= deglong; // remove the degrees from the calculation
  wlong *= 60; // convert to minutes  
  float mlong = wlong;
  String pDegrees = String(abs(deglong));
  printStr(" "+pDegrees,4,false);
  mlong=abs(mlong);
  dtostrf(mlong,6,4,charVal);
  pDegrees = "";
  
  for(unsigned int i=0;i<sizeof(charVal);i++) // was sizeof
  {
    pDegrees+=charVal[i];
  }
  if (mlong <10)
  {
    printStr(" ",2,false);
    printStr(pDegrees,5,false);
  }
  else
  {
    printStr(" ",1,false);
    printStr(pDegrees,6,false);
  }
}

// This custom version of delay() ensures that the gps object
// is being "fed".
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}

static void printFloat(float val, bool valid, int len, int prec,bool rtn)
{
  if (!valid)
  {
    while (len-- > 1)
      tft.print('*');
      tft.print(' ');
  }
  else
  {
    if (rtn)
    {
      tft.println(val, prec);
    }
    else{
      tft.print(val, prec);
    }
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
   
  }
}

static void printInt(unsigned long val, bool valid, int len,bool rtn)
{
  char sz[32] = "*****************";
  if (valid)
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i=strlen(sz); i<len; ++i)
    sz[i] = ' ';
  if (rtn)
  {
    tft.println(sz);
  }
  else
  {
    tft.print(sz);
  }
}

static void printDateTime(TinyGPSDate &d, TinyGPSTime &t)

{
  tft.setTextSize(3);
  tft.setTextColor(ILI9341_WHITE,ILI9341_BLACK);  
  tft.setCursor(0, 0);
  if (!t.isValid())
  {
    tft.print(F("*No Time* ")); // uses the macro F() from WString.h
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d:%02d:%02d ", t.hour(), t.minute(), t.second());
    tft.println(sz);
  }
  tft.setCursor(0, line*1);
  if (!d.isValid())
  {
    tft.print(F("*No Date* "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d ", d.month(), d.day(), d.year());
    tft.println(sz);
  }  
}

static void printStr(String istring, unsigned int len, boolean rtn)
{
  
  String sout = ""; 
  unsigned int slen = istring.length();// this how long it is
  istring = istring.trim();
  if (slen > len)
  {
    sout = istring.substring(0,len);
  }
  else
  {
    sout = istring;
    while (sout.length() < len)
    {
      sout = " "+sout;
    } 
  }
  if (rtn)
  {
    tft.println(sout);
  } 
  else
  {
    tft.print(sout);
  }
}
