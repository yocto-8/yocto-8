//
// PNG encoder test sketch
// written by Larry Bank
//
// Creates a 128x128x8-bpp image 'on-the-fly' and
// compresses it as a PNG file. The image is a green square with an X in the middle
// on a transparent background
//
// This program can run as either a memory-to-memory test for measuring
// performance or can write the PNG file to a micro-SD card
// Disable the macro below (WRITE_TO_SD) to use only memory
//

#include <PNGenc.h>
PNG png; // static instance of the PNG encoder lass

// Add comment bars (//) in front of this macro to send the output to RAM only
#define WRITE_TO_SD

#ifdef WRITE_TO_SD
#include <SD.h>

// Functions to access a file on the SD card
File myfile;

void * myOpen(const char *filename) {
  Serial.printf("Attempting to open %s\n", filename);
  // IMPORTANT!!! - don't use FILE_WRITE because it includes O_APPEND
  // this will cause the file to be written incorrectly
  myfile = SD.open(filename, O_READ| O_WRITE | O_CREAT);
  return &myfile;
}
void myClose(PNGFILE *handle) {
  File *f = (File *)handle->fHandle;
  f->close();
}
int32_t myRead(PNGFILE *handle, uint8_t *buffer, int32_t length) {
  File *f = (File *)handle->fHandle;
  return f->read(buffer, length);
}
int32_t myWrite(PNGFILE *handle, uint8_t *buffer, int32_t length) {
  File *f = (File *)handle->fHandle;
  return f->write(buffer, length);
}
int32_t mySeek(PNGFILE *handle, int32_t position) {
  File *f = (File *)handle->fHandle;
  return f->seek(position);
}
#endif // WRITE_TO_SD

#define WIDTH 128
#define HEIGHT 128

uint8_t ucPal[768] = {0,0,0,0,255,0}; // black, green
uint8_t ucAlphaPal[256] = {0,255}; // first color (black) is fully transparent

// Memory to hold the output file
#ifndef WRITE_TO_SD
uint8_t ucOut[4096];
#endif

void setup() {
int rc, iDataSize, x, y;
uint8_t ucLine[WIDTH];
long l;

  Serial.begin(115200);
  while (!Serial) {};

#ifdef WRITE_TO_SD
  while (!SD.begin(32/*4*//*BUILTIN_SDCARD*/)) {
    Serial.println("Unable to access SD Card");
    delay(1000);
  }
  Serial.println("SD Card opened successfully");
#endif
  
  l = micros();
#ifdef WRITE_TO_SD
  rc = png.open("/testimg.png", myOpen, myClose, myRead, myWrite, mySeek);
#else
  rc = png.open(ucOut, sizeof(ucOut));
#endif
  if (rc == PNG_SUCCESS) {
        rc = png.encodeBegin(WIDTH, HEIGHT, PNG_PIXEL_INDEXED, 8, ucPal, 3);
        png.setAlphaPalette(ucAlphaPal);
        if (rc == PNG_SUCCESS) {
            for (int y=0; y<HEIGHT && rc == PNG_SUCCESS; y++) {
              // prepare a line of image to create a red box with an x on a transparent background
              if (y==0 || y == HEIGHT-1) {
                memset(ucLine, 1, WIDTH); // top+bottom green lines 
              } else {
                memset(ucLine, 0, WIDTH);
                ucLine[0] = ucLine[WIDTH-1] = 1; // left/right border
                ucLine[y] = ucLine[WIDTH-1-y] = 1; // X in the middle
              }
                rc = png.addLine(ucLine);
            } // for y
            iDataSize = png.close();
            l = micros() - l;
            Serial.printf("%d bytes of data written to file in %d us\n", iDataSize, (int)l);
        }
  } else {
    Serial.println("Failed to create the file on the SD card!");
  }
} /* setup() */

void loop() {
  // nothing to do
} /* loop() */
