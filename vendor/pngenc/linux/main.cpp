//
//  main.cpp
//  pngenc_test
//
//  Created by Larry Bank on 6/27/21.
//  Demonstrates the PNGenc library
//  by either compressing a given BMP file
//  or generating one dynamically
//
#include "../src/PNGenc.h"

PNG png; // static instance of class

uint8_t localPalette[] = {0,0,0,0,255,0};
uint8_t ucAlphaPalette[] = {0,255}; // first color is transparent

// Disable this macro to use a memory buffer to 'catch' the PNG output
// otherwise it will write the data incrementally to the output file
// using the 5 user-defined callback functions
#define USE_FILES
//
// Read a Windows BMP file into memory
//
uint8_t * ReadBMP(const char *fname, int *width, int *height, int *bpp, unsigned char *pPal)
{
    int y, w, h, bits, offset;
    uint8_t *s, *d, *pTemp, *pBitmap;
    int pitch, bytewidth, colors;
    int iSize, iDelta;
    FILE *infile;
    
    infile = fopen(fname, "r+b");
    if (infile == NULL) {
        printf("Error opening input file %s\n", fname);
        return NULL;
    }
    // Read the bitmap into RAM
    fseek(infile, 0, SEEK_END);
    iSize = (int)ftell(infile);
    fseek(infile, 0, SEEK_SET);
    pBitmap = (uint8_t *)malloc(iSize);
    pTemp = (uint8_t *)malloc(iSize);
    fread(pTemp, 1, iSize, infile);
    fclose(infile);
    
    if (pTemp[0] != 'B' || pTemp[1] != 'M' || pTemp[14] < 0x28) {
        free(pBitmap);
        free(pTemp);
        printf("Not a Windows BMP file!\n");
        return NULL;
    }
    w = *(int32_t *)&pTemp[18];
    h = *(int32_t *)&pTemp[22];
    bits = *(int16_t *)&pTemp[26] * *(int16_t *)&pTemp[28];
    offset = *(int32_t *)&pTemp[10]; // offset to bits
    colors = pTemp[46]; // colors used
    if (colors == 0 || colors > (1 <<bits)) colors = 1 << bits;
    if (bits <= 8) { // it has a palette, copy it
        uint8_t *p = pPal;
        for (int i=0; i<(1<<bits); i++)
        {
           *p++ = pTemp[offset-(colors*4)+(i*4)+0];
           *p++ = pTemp[offset-(colors*4)+(i*4)+1];
           *p++ = pTemp[offset-(colors*4)+(i*4)+2];
        }
    }
    bytewidth = (w * bits) >> 3;
    pitch = (bytewidth + 3) & 0xfffc; // DWORD aligned
// move up the pixels
    d = pBitmap;
    s = &pTemp[offset];
    iDelta = pitch;
    if (h > 0) {
        iDelta = -pitch;
        s = &pTemp[offset + (h-1) * pitch];
    } else {
        h = -h;
    }
    for (y=0; y<h; y++) {
        if (bits == 32) {// need to swap red and blue
            for (int i=0; i<bytewidth; i+=4) {
                d[i] = s[i+2];
                d[i+1] = s[i+1];
                d[i+2] = s[i];
                d[i+3] = s[i+3];
            }
        } else if (bits == 24) {
            for (int i=0; i<bytewidth; i+=3) {
                d[i] = s[i+2]; 
                d[i+1] = s[i+1];
                d[i+2] = s[i];
            }
        } else {
            memcpy(d, s, bytewidth);
        }
        d += bytewidth;
        s += iDelta;
    }
    *width = w;
    *height = h;
    *bpp = bits;
    free(pTemp);
    return pBitmap;
    
} /* ReadBMP() */

#define ITERATION_COUNT 1

int32_t myWrite(PNGFILE *pFile, uint8_t *pBuf, int32_t iLen)
{
    FILE *ohandle = (FILE *)pFile->fHandle;
    return (int32_t)fwrite(pBuf, 1, iLen, ohandle);
} /* myWrite() */

int32_t myRead(PNGFILE *pFile, uint8_t *pBuf, int32_t iLen)
{
    FILE *ohandle = (FILE *)pFile->fHandle;
    return (int32_t)fread(pBuf, 1, iLen, ohandle);
} /* myRead() */

int32_t mySeek(PNGFILE *pFile, int32_t iPosition)
{
    FILE *f = (FILE *)pFile->fHandle;
    fseek(f, iPosition, SEEK_SET);
    return(int32_t)ftell(f);

} /* mySeek() */

void * myOpen(const char *szFilename)
{
    return fopen(szFilename, "w+b");
} /* myOpen() */

void myClose(PNGFILE *pHandle)
{
    FILE *f = (FILE *)pHandle->fHandle;
    fclose(f);
} /* myClose() */

int main(int argc, const char * argv[]) {
    int rc, iOutIndex;
    int iDataSize, iBufferSize;
#ifndef USE_FILES
    FILE *ohandle;
    uint8_t *pOutput;
#endif
    int iWidth, iHeight, iBpp, iPitch;
    uint8_t *pBitmap;
    uint8_t ucPalette[1024];
    uint8_t ucBitSize = 8, ucPixelType=0, *pPalette=NULL;
    
    if (argc != 3 && argc != 2) {
       printf("PNG encoder library demo program\n");
       printf("Usage: png_demo <infile.bmp> <outfile.png>\n");
       printf("\nor (to generate an image dynamically)\n       png_demo <outfile.png>\n");
       return 0;
    }
    printf("RAM needed for png class/struct = %d bytes\n", (int)sizeof(png));
    
    if (argc == 3) {
       iOutIndex = 2; // argv index of output filename
       pBitmap = ReadBMP(argv[1], &iWidth, &iHeight, &iBpp, ucPalette);
       if (pBitmap == NULL)
       {
           fprintf(stderr, "Unable to open file: %s\n", argv[1]);
           return -1; // bad filename passed?
       }
    } else { // create the bitmap in code
       iOutIndex = 1;  // argv index of output filename
       iWidth = iHeight = 128;
       iPitch = iWidth*sizeof(uint16_t); // create a RGB565 image
       iBpp = 8;
       //memcpy(ucPalette, localPalette, sizeof(localPalette));
       pBitmap = (uint8_t *)malloc(128*128*sizeof(uint16_t));
       for (int y=0; y<iHeight; y++) {
           uint16_t *pLine = (uint16_t *)&pBitmap[iPitch*y];
           if (y==0 || y == iHeight-1) {
               for (int x=0; x<iWidth; x++) { // top+bottom red lines
                   pLine[x] = 0xf800; // pure red
               } // for x
           } else {
               memset(pLine, 0, iWidth*sizeof(uint16_t)); // black background
               pLine[0] = pLine[iWidth-1] = 0xf800; // left/right border = red
               pLine[y] = pLine[iWidth-1-y] = 0x1f; // blue X in the middle
           }
       } // for y
    }
    for (int j=0; j<ITERATION_COUNT; j++) {
        iBufferSize = iWidth * iHeight;
#ifndef USE_FILES
        pOutput = (uint8_t *)malloc(iBufferSize);
#endif
        iPitch = iWidth;
        switch (iBpp) {
            case 8:
            case 4:
            case 2:
                ucPixelType = PNG_PIXEL_INDEXED;
                iPitch = (iWidth * iBpp) >> 3;
                pPalette = ucPalette;
                ucBitSize = (uint8_t)iBpp;
                break;

            case 1:
                ucPixelType = PNG_PIXEL_GRAYSCALE;
                iPitch = (iWidth * iBpp) >> 3;
                pPalette = NULL;
                ucBitSize = (uint8_t)iBpp;
                break;

            case 24:
                ucPixelType = PNG_PIXEL_TRUECOLOR;
                iPitch = iWidth * 3;
                ucBitSize = 24;
                break;
            case 32:
                ucPixelType = PNG_PIXEL_TRUECOLOR_ALPHA;
                iPitch = iWidth * 4;
                ucBitSize = 32;
                break;
        } // switch on pixel type
        
#ifdef USE_FILES
        rc = png.open(argv[iOutIndex], myOpen, myClose, myRead, myWrite, mySeek);
#else
        rc = png.open(pOutput, iBufferSize);
#endif
        if (rc != PNG_SUCCESS) {
            printf("Error opening output file %s, exiting...\n", argv[iOutIndex]);
            return -1;
        }
        if (argc == 3)
           rc = png.encodeBegin(iWidth, iHeight, ucPixelType, ucBitSize, pPalette, 9);
        else
           rc = png.encodeBegin(iWidth, iHeight, PNG_PIXEL_TRUECOLOR, 24, NULL, 9);
//        if (argc == 2)
//            png.setAlphaPalette(ucAlphaPalette);

        if (rc == PNG_SUCCESS) {
            for (int y=0; y<iHeight && rc == PNG_SUCCESS; y++) {
                if (argc == 3)
                    rc = png.addLine(&pBitmap[iPitch * y]);
                else
                    rc = png.addRGB565Line((uint16_t *)&pBitmap[iWidth*sizeof(uint16_t)*y], (void *)ucPalette); // use palette as temporary buffer for internal pixel conversion
            }
            iDataSize = png.close();
            printf("PNG image successfully created (%d bytes)\n", iDataSize);
#ifndef USE_FILES
            if (rc == PNG_SUCCESS) { // good output, write it to a file
                ohandle = fopen(argv[iOutIndex], "w+b");
                if (ohandle != NULL) {
                    fwrite(pOutput, 1, iDataSize, ohandle);
                    fclose(ohandle);
                }
            }
            free(pOutput);
#endif
        }
    } // for j
    free(pBitmap);
    return 0;
}
