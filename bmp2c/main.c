//
//  main.c
//  Bmp2C
//
//  Conversion of a 16bpp Windows BMP files into C arrays
//  Used to generate the C code for Yukari Station Timetable Display
//
//

#include <stdio.h>

void bmp2C(char *,char *);

int main(int argc, const char * argv[]) {
    char srcfilename[256];
    char destfilename[256];

    sprintf(srcfilename,"/Users/Marc/Documents/Arduino/Yukari Station Schedule Display/Yukari Schedule Fonts 9px.bmp");
    sprintf(destfilename,"/Users/Marc/Documents/Arduino/Yukari Station Schedule Display/Yukari Schedule Fonts 9px.c");
    bmp2C(srcfilename,destfilename);

    return 0;
}

void bmp2C(char *srcfilename,char *destfilename) {
    
    FILE     *bmpFile, *cFile;
    uint32_t bmpWidth, bmpHeight;   // W+H in pixels
    uint16_t bmpTag;
    uint16_t bmpPlanes;
    uint8_t  bmpDepth;              // Bit depth (must be 8 here)
    uint32_t bmpImageoffset;        // Start of image data in file
    uint32_t bmpHeaderSize;         // bmp header size in bytes
    uint32_t bmpImageSize;          // bmp image data size in bytes
    uint32_t bmpCreator;
    uint8_t  textType, textTypeLine;
    uint16_t bmpImageCurrentRow;
    uint16_t i;
    uint16_t row, col;
    uint16_t pixel;

    const uint16_t YS_leading = 9;  // Number of rows of one line of text in the bitmap

    const uint16_t YS_numberOfTextTypes = 9;
    const uint16_t YS_numberOfTextLines[YS_numberOfTextTypes] = { 7,  // Tracks
                                                                 19,  // Times
                                                                 11,  // Lines
                                                                  7,  // Destinations
                                                                  3,  // Remarks
                                                                 11,  // Lines Japanese
                                                                  7,  // Destinations Japanese
                                                                  3,  // Remarks Japanese
                                                                  1}; // Alerts Japanese
    
    const uint16_t YS_textTypeWidth[YS_numberOfTextTypes] = { 6,  // Tracks
                                                             18,  // Times
                                                             76,  // Lines
                                                             73,  // Destinations
                                                             38,  // Remarks
                                                             76,  // Lines Japanese
                                                             73,  // Destinations Japanese
                                                             38,  // Remarks Japanese
                                                           1400}; // Alerts Japanese
    
    printf("\nImage: %s ",srcfilename);
    
    // Open requested file
    if ((bmpFile = fopen(srcfilename, "rb")) == NULL) {
        printf(" | File not found");
        return;
    }
    
    // Parse BMP header
    fread(&bmpTag,sizeof(uint16_t),1,bmpFile);
    if (bmpTag != 0x4D42) {
        printf(" | Tag error");
        fclose(bmpFile);
        return;
    }

    fread(&bmpImageSize,sizeof(uint32_t),1,bmpFile);
    printf(" | File size: %i", bmpImageSize);

    fread(&bmpCreator,sizeof(uint32_t),1,bmpFile);

    fread(&bmpImageoffset,sizeof(uint32_t),1,bmpFile);
    printf(" | Offset: %i", bmpImageoffset);

    fread(&bmpHeaderSize,sizeof(uint32_t),1,bmpFile);
    printf(" | Header size: %i", bmpImageoffset);

    fread(&bmpWidth,sizeof(uint32_t),1,bmpFile);
    fread(&bmpHeight,sizeof(uint32_t),1,bmpFile);
    printf(" | %ix%i", bmpWidth, bmpHeight);

    fread(&bmpPlanes,sizeof(uint16_t),1,bmpFile);
    fread(&bmpDepth,sizeof(uint16_t),1,bmpFile);
    printf(" | Depth: %i ", bmpDepth);

    if(bmpDepth != 16)  {
        fclose(bmpFile);
        printf(" | Incorrect depth/pixel size. Exiting.");
        return;
    }

    if ((cFile = fopen(destfilename, "w")) == NULL) {
        printf(" | Cannot open destination C file");
        return;
    }
    
    bmpImageCurrentRow = 0;
    
    fprintf(cFile, "// Bitmap data. Format is RGB565.\n");
    for (textType = 0; textType < YS_numberOfTextTypes; textType++)  {
        fprintf(cFile, "const uint16_t yukariBmpData%02d[] PROGMEM_LATE = {\n",textType);
        printf (" | textType = %d", textType);
        for (textTypeLine = 0; textTypeLine < YS_numberOfTextLines[textType]; textTypeLine++)  {
            printf (" | textTypeLine = %d", textTypeLine);
            
            for (row=0; row < YS_leading; row++) {
                if (fseek(bmpFile,bmpImageoffset + (bmpWidth * sizeof(uint16_t) * bmpImageCurrentRow),SEEK_SET) != 0)
                    printf (" | fseek error");
                for (col=0; col < YS_textTypeWidth[textType]; col++) {
                    if (fread(&pixel,sizeof(uint16_t),1,bmpFile) != 1)
                        printf (" | fread error");
                    pixel = pixel >> 8 | pixel << 8;
                    fprintf(cFile, "0x%04X",pixel);
                    fprintf(cFile,", ");
                }
                fprintf(cFile, "\n");
                bmpImageCurrentRow++;
            }
        }
        fprintf(cFile, "0xFFFF};\n\n");
    }

    fprintf(cFile, "// Number of lines/pixels between two lines of text = height of each line of text\n");
    fprintf(cFile, "const uint8_t yukariBmpLeading = %d;\n\n", YS_leading);

    fprintf(cFile, "// Table containing the width (number of pixels) of each line of text\n");
    fprintf(cFile, "const uint16_t yukariBmpWidth[] = {");
    for (i = 0; i < YS_numberOfTextTypes; i++) {
        fprintf(cFile, "%d",YS_textTypeWidth[i]);
        if (i != YS_numberOfTextTypes-1) fprintf(cFile,", ");
    }
    fprintf(cFile, "};\n\n");

    fclose(bmpFile);
    fclose(cFile);
    printf(" | Closing files. Done. \n");
}
