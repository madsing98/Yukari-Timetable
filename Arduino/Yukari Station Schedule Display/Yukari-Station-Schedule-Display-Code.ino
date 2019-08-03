/*
   Shin-Yukary Station Schedule Display application for Arduino Mega
   Uses
   - 240x240 pixel ST7789 TFT LCD display
   - Connected using the SPI bus
   - To an Arduino Mega
   - Pinout:
    - D3  : BLK Backlight
    - D8  : DC  Data/Command
    - D9  : RST Reset
    - D51 : SDA Data
    - D52 : SCK Clock
  
   The C bitmap arrays are generated
   - Using Adobe Photoshop Elements
   - To save BMP files (Save As... BMP RGB565 Flip Row Order)
   - Converted to C arrays using the bmp2C application
 */
 
//#define DEBUG  // Uncomment to send debugging messages to the serial line

#include <Adafruit_GFX.h>    // Core graphics library
#include <Arduino_ST7789.h>  // Hardware-specific library for ST7789 (with or without CS pin), modified to add the ::pushColorBuf() function
#include <SPI.h>

//#define TFT_CS    10   // Not used
#define TFT_RST    9
#define TFT_DC     8

// initialize ST7789 to use the hardware SPI pins
Arduino_ST7789 tft = Arduino_ST7789(TFT_DC, TFT_RST); //for display without CS pin

// Description of the bitmap C arrays
#define YS_TRACK 0
#define yukariBmpDataTrack yukariBmpData00
#define YS_TRACK_1 0
#define YS_TRACK_2 1
#define YS_TRACK_3 2
#define YS_TRACK_4 3
#define YS_TRACK_5 4
#define YS_TRACK_6 5
#define YS_TRACK_7 6

#define YS_TIME 1
#define yukariBmpDataTime yukariBmpData01

#define YS_LINE 2
#define yukariBmpDataLine yukariBmpData02
#define YS_LINE_JAP 5
#define yukariBmpDataLineJap yukariBmpData05
#define YS_LINE_KAGAYAKI_538     0
#define YS_LINE_KAGAYAKI_528     1
#define YS_LINE_KAGAYAKI_542     2
#define YS_LINE_NEX_19           3
#define YS_LINE_NEX_23           4   
#define YS_LINE_NEX_25           5   
#define YS_LINE_YAMANOTE         6
#define YS_LINE_KEIHIN_TOHOKU    7
#define YS_LINE_YOKOSUKA         8
#define YS_LINE_BOSO_VIEW        9
#define YS_LINE_SEIBU_SHINKUKU  10

#define YS_DESTINATION 3
#define yukariBmpDataDestination yukariBmpData03
#define YS_DESTINATION_JAP 6
#define yukariBmpDataDestinationJap yukariBmpData06
#define YS_DESTINATION_KANAZAWA       0
#define YS_DESTINATION_NARITA         1
#define YS_DESTINATION_HARAJUKU       2
#define YS_DESTINATION_YOKOHAMA       3
#define YS_DESTINATION_KURIHAMA       4
#define YS_DESTINATION_AWA_KAMOGAWA   5
#define YS_DESTINATION_HON_KAWAGOE    6

#define YS_REMARK 4
#define yukariBmpDataRemark yukariBmpData04
#define YS_REMARK_JAP 7
#define yukariBmpDataRemarkJap yukariBmpData07
#define YS_REMARK_NONE                0
#define YS_REMARK_6CARS               1
#define YS_REMARK_10CARS              2

#define YS_ALERT_JAP 8
#define yukariBmpDataAlertJap yukariBmpData08

// Description of the display
const uint16_t displayLineNum = 5;        // Number of lines to display from the timetable YukariTimetable[]
const uint16_t displayLineLeading = yukariBmpLeading + 2;   // Distance in pixels on the display between two timetable lines
      uint16_t displayLinePos[displayLineNum+1]; // Position of each of the timetable lines. The last one (linePos[lineNum]) is for the Alerts
const uint16_t colPosTime = 0;                                                                                                 // Train time column position
const uint16_t colPosLine = colPosTime + yukariBmpWidth[YS_TIME] + 7;                                                          // Train line column position
const uint16_t colPosDestination = colPosLine + max(yukariBmpWidth[YS_LINE],yukariBmpWidth[YS_LINE_JAP]) + 7;                  // Train destination column position
const uint16_t colPosTrack = colPosDestination + max(yukariBmpWidth[YS_DESTINATION],yukariBmpWidth[YS_DESTINATION_JAP]) + 7;   // Track track column position
const uint16_t colPosRemark = colPosTrack + yukariBmpWidth[YS_TRACK] + 7;                                                      // Train remark column position
const uint16_t colPosEnd = colPosRemark + max(yukariBmpWidth[YS_REMARK],yukariBmpWidth[YS_REMARK_JAP]);                        // End column position

// Timetable, train schedule entries, will be displayed in that order from top to bottom until the last row then start over
#define YS_TT_TRACK       0
#define YS_TT_LINE        1
#define YS_TT_DESTINATION 2
#define YS_TT_REMARK      3
const uint8_t timetableNbrOfLines = 19;
const uint8_t YukariTimetable[timetableNbrOfLines][4] = {
  {YS_TRACK_6, YS_LINE_YAMANOTE,       YS_DESTINATION_HARAJUKU,     YS_REMARK_NONE},
  {YS_TRACK_3, YS_LINE_KAGAYAKI_528,   YS_DESTINATION_KANAZAWA,     YS_REMARK_6CARS},
  {YS_TRACK_4, YS_LINE_NEX_19,         YS_DESTINATION_NARITA,       YS_REMARK_6CARS},
  {YS_TRACK_6, YS_LINE_YAMANOTE,       YS_DESTINATION_HARAJUKU,     YS_REMARK_NONE},
  {YS_TRACK_7, YS_LINE_KEIHIN_TOHOKU,  YS_DESTINATION_YOKOHAMA,     YS_REMARK_NONE},
  {YS_TRACK_7, YS_LINE_YOKOSUKA,       YS_DESTINATION_KURIHAMA,     YS_REMARK_NONE},
  {YS_TRACK_6, YS_LINE_YAMANOTE,       YS_DESTINATION_HARAJUKU,     YS_REMARK_NONE},
  {YS_TRACK_1, YS_LINE_BOSO_VIEW,      YS_DESTINATION_AWA_KAMOGAWA, YS_REMARK_NONE},
  {YS_TRACK_4, YS_LINE_NEX_23,         YS_DESTINATION_NARITA,       YS_REMARK_6CARS},
  {YS_TRACK_6, YS_LINE_YAMANOTE,       YS_DESTINATION_HARAJUKU,     YS_REMARK_NONE},
  {YS_TRACK_1, YS_LINE_SEIBU_SHINKUKU, YS_DESTINATION_HON_KAWAGOE,  YS_REMARK_NONE},
  {YS_TRACK_3, YS_LINE_KAGAYAKI_538,   YS_DESTINATION_KANAZAWA,     YS_REMARK_10CARS},
  {YS_TRACK_6, YS_LINE_YAMANOTE,       YS_DESTINATION_HARAJUKU,     YS_REMARK_NONE},
  {YS_TRACK_7, YS_LINE_YOKOSUKA,       YS_DESTINATION_KURIHAMA,     YS_REMARK_NONE},
  {YS_TRACK_1, YS_LINE_SEIBU_SHINKUKU, YS_DESTINATION_HON_KAWAGOE,  YS_REMARK_NONE},
  {YS_TRACK_7, YS_LINE_KEIHIN_TOHOKU,  YS_DESTINATION_YOKOHAMA,     YS_REMARK_NONE},
  {YS_TRACK_4, YS_LINE_NEX_25,         YS_DESTINATION_NARITA,       YS_REMARK_6CARS},
  {YS_TRACK_1, YS_LINE_BOSO_VIEW,      YS_DESTINATION_AWA_KAMOGAWA, YS_REMARK_NONE},
  {YS_TRACK_3, YS_LINE_KAGAYAKI_542,   YS_DESTINATION_KANAZAWA,     YS_REMARK_10CARS},
 };

// Function to draw a bitmap in a window on the display
// The window on the display is defined by windowX, windowY, imageWidth, imageHeight
// imagePtr is the pointer to the high PROGMEM memory buffer containing the image
// lineNbr is line number (within the bitmap) to display
// scrollOffset is a horizontal (x) offset used to implement scrolling of the Alerts
// pgm_read_word_far() retrieves data from the Arduino's program memory above the 64K limit
// The data is buffered into displayBuf[] before being pushed to the display as tft.pushColorBuf() cannot read from above the 64K limit.
void drawRGBBitmap(uint16_t windowX, uint16_t windowY, uint16_t imageWidth, uint16_t imageHeight, uint32_t imagePtr, uint16_t lineNbr, uint16_t scrollOffset=0) {
    uint16_t x,y;
    uint32_t linePtr,pixelPtr;
    uint16_t windowWidth,windowHeight;
    uint16_t displayWidth;
    uint16_t displayBuf[240];
    
    // Display size clipping (in Y coordinates only)
    displayWidth = tft.width();
    if (windowX + imageWidth > displayWidth)
      windowWidth = displayWidth - windowX;
    else
      windowWidth = imageWidth;

    windowHeight = imageHeight;

    // Set the window coordinates on the display for pushing pixels
    tft.setAddrWindow(windowX, windowY, windowX+windowWidth-1, windowY+windowHeight-1);
    
    // linePtr points to each beginning of line in the source image bitmap
    linePtr = imagePtr + (scrollOffset + lineNbr * imageHeight * imageWidth) * sizeof(uint16_t);
    for(y=0; y<imageHeight; y++) {
      
      // pixelPtr points to each pixel to be transferred
      pixelPtr = linePtr;
      
      // Copy one whole line of pixels to the buffer displayBuf[]
      for(x=0; x<windowWidth; x++) {
        displayBuf[x] = pgm_read_word_far(pixelPtr);
        pixelPtr += sizeof(uint16_t);  // Next word (2 bytes)
        }
     
     // Push it to the display
     tft.pushColorBuf(displayBuf,windowWidth * sizeof(uint16_t));
     
     // Increment linePtr to point to the next line in the source image bitmap
     linePtr += imageWidth * sizeof(uint16_t);
     }
  }


void setup(void) {
  uint8_t i = 0;
  uint8_t displayLine = 0;
  uint8_t language = 0;
  uint8_t timetableOffset = 0;
  uint16_t alertScrollOffset = 0;
  uint16_t displayWidth = 0;
  unsigned long startTime,endTime;

  pinMode(3, OUTPUT);             // Pin 3 connected to BLK to control display brightness
  analogWrite(3, 0);              // Set brightness to zero (backlight off) during display initialization
                                  
  #ifdef DEBUG
    Serial.begin(115200);
    Serial.println("Starting");
  #endif

  tft.init(240, 240);   // initialize the ST7789 chip, 240x240 pixels
  tft.setRotation(2);
  // Clear screen
  tft.fillScreen(BLACK);
  
  #ifdef DEBUG
    Serial.println("ST7789 initialized");
  #endif
    
  displayWidth = tft.width();
  
  // Initialize the displayLinePos array and draw background rectangle for each display line (NOW COMMENTED OUT)
  for (displayLine = 0; displayLine < displayLineNum+1; displayLine++)  {
    displayLinePos[displayLine] = 240 - displayLineLeading * (displayLineNum + 1) + displayLineLeading * displayLine;
//    tft.fillRect(0, displayLinePos[displayLine], displayWidth, yukariBmpLeading, color565(30,30,30));
  }

  analogWrite(3,64);             // pin 3 connected to BLK to control brightness, set at 25%

  #ifdef DEBUG
    Serial.println("--> Display parameters ----------------------------------");
    Serial.print("colPosTime = ");  Serial.println(colPosTime);
    Serial.print("colPosLine = ");  Serial.println(colPosLine);
    Serial.print("colPosDestination = ");  Serial.println(colPosDestination);
    Serial.print("colPosTrack = ");  Serial.println(colPosTrack);
    Serial.print("colPosRemark = ");  Serial.println(colPosRemark);
    Serial.print("colPosEnd = ");  Serial.println(colPosEnd);
    Serial.println("---------------------------------------------------------");
  #endif

  while(1) {
    // Display lineNum lines from the timetable, going down in the table until we reach the end, then start over
    for (timetableOffset = 0; timetableOffset <= timetableNbrOfLines - displayLineNum; timetableOffset++) {
      
      // Display the timetable six times, three times in English alternating with three times in Japanese
      for (language = 0; language < 6; language++) {
        
        #ifdef DEBUG
          startTime = millis();
        #endif
        
        for (displayLine = 0;displayLine < displayLineNum;displayLine++) {
          // Display tracks and times
          drawRGBBitmap(colPosTrack, displayLinePos[displayLine], yukariBmpWidth[YS_TRACK], yukariBmpLeading, pgm_get_far_address(yukariBmpDataTrack), YukariTimetable[displayLine+timetableOffset][YS_TT_TRACK]);
          drawRGBBitmap(colPosTime,  displayLinePos[displayLine], yukariBmpWidth[YS_TIME],  yukariBmpLeading, pgm_get_far_address(yukariBmpDataTime),  displayLine+timetableOffset);
          if ((language&1) == 0) {
            // Display train lines, destination and remarks in English
            drawRGBBitmap(colPosLine,        displayLinePos[displayLine], yukariBmpWidth[YS_LINE],        yukariBmpLeading, pgm_get_far_address(yukariBmpDataLine),        YukariTimetable[displayLine+timetableOffset][YS_TT_LINE]);
            drawRGBBitmap(colPosDestination, displayLinePos[displayLine], yukariBmpWidth[YS_DESTINATION], yukariBmpLeading, pgm_get_far_address(yukariBmpDataDestination), YukariTimetable[displayLine+timetableOffset][YS_TT_DESTINATION]);
            drawRGBBitmap(colPosRemark,      displayLinePos[displayLine], yukariBmpWidth[YS_REMARK],      yukariBmpLeading, pgm_get_far_address(yukariBmpDataRemark),      YukariTimetable[displayLine+timetableOffset][YS_TT_REMARK]);
          } else {
            // Display train lines, destination and remarks in Japanese
            drawRGBBitmap(colPosLine,        displayLinePos[displayLine], yukariBmpWidth[YS_LINE_JAP],        yukariBmpLeading, pgm_get_far_address(yukariBmpDataLineJap),        YukariTimetable[displayLine+timetableOffset][YS_TT_LINE]);
            drawRGBBitmap(colPosDestination, displayLinePos[displayLine], yukariBmpWidth[YS_DESTINATION_JAP], yukariBmpLeading, pgm_get_far_address(yukariBmpDataDestinationJap), YukariTimetable[displayLine+timetableOffset][YS_TT_DESTINATION]);
            drawRGBBitmap(colPosRemark,      displayLinePos[displayLine], yukariBmpWidth[YS_REMARK_JAP],      yukariBmpLeading, pgm_get_far_address(yukariBmpDataRemarkJap),      YukariTimetable[displayLine+timetableOffset][YS_TT_REMARK]);
          }
        }        
        
        #ifdef DEBUG
          endTime = millis();
          Serial.print("Display time = ");  Serial.print(endTime-startTime);  Serial.println(" ms");
        #endif
        
        // Display the Alert text 250 times, scrolling left
        for (i = 0; i < 250; i++) {
          if(i != 0)  
            delay(20);       // Delay 20 ms, except the first loop to try to keep scrolling regular
          drawRGBBitmap(0, displayLinePos[displayLineNum], yukariBmpWidth[YS_ALERT_JAP], yukariBmpLeading, pgm_get_far_address(yukariBmpDataAlertJap), 0,alertScrollOffset++);
          if(alertScrollOffset + displayWidth >= yukariBmpWidth[YS_ALERT_JAP])
            alertScrollOffset = 0;
        // When it's done, continue with the display of the timetable
        }
      }
    }
  }

}

void loop() {
}

