/* Panorama_Tools       -       Generate, Edit and Convert Panoramic Images
   Copyright (C) 1998,1999 - Helmut Dersch  der@fh-furtwangen.de
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/*------------------------------------------------------------*/

// Functions to read and write ppm format


#include "filter.h"
#include "metadata.h"
#include "file.h"

static int readPPMFileHeader(file_spec src, Image * im);


// image is allocated, but not image data

int readPPM(Image * im, fullPath * sfile)
{
    file_spec src;
    size_t count;

    if (myopen(sfile, read_bin, src))
    {
        PrintError("Error Opening Image File");
        return -1;
    }


    if (readPPMFileHeader(src, im) != 0)
    {
        PrintError("Error Reading File Header");
        myclose(src);
        return -1;
    }

    im->bitsPerPixel = 24;
    im->bytesPerLine = im->width * 3;
    im->dataSize = im->bytesPerLine * im->height;

    im->data =
        (unsigned char **) mymalloc((size_t) (im->width * im->height * 4));
    if (im->data == NULL)
    {
        PrintError("Not enough memory");
        myclose(src);
        return -1;
    }

    count = (size_t) (im->width * im->height * 3);

    myread(src, count, *(im->data));

    if (count != im->width * im->height * 3)
    {
        PrintError("Error Reading Image Data");
        myclose(src);
        return -1;
    }

    myclose(src);
    ThreeToFourBPP(im);
    return 0;
}


static int readPPMFileHeader(file_spec src, Image * im)
{
    int i;
    char smallBuf[32], c;
    size_t count = 1;

    /* read the file header (including height and width) */

    im->height = -1;
    while (im->height == -1)
    {
        myread(src, count, &c);
        if (count != 1)
            return 1;
        switch (c)
        {
        case '#':
            /* comment line -- skip it */
            while (c != 0x0A && c != 0x0D && count == 1)
                myread(src, count, &c);
            break;
        case ' ':
        case '\012':
        case '\t':
        case '\015':
            /* random whitespace... just ignore it. */
            break;
        case 'P':
            /* magic number */
            myread(src, count, &c);
            if (c != '6')
            {
                PrintError("Bad magic number in input file");
                return (-1);
            }
            /* there should be one whitespace character */
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            /* read width */
            for (i = 0; isdigit(c) && (i + 1 < sizeof(smallBuf)); i++)
            {
                smallBuf[i] = c;
                myread(src, count, &c);
                if (count != 1)
                    return -1;
            }
            if (!isspace(c))
            {
                PrintError("Bad input file format");
                return -1;
            }
            smallBuf[i] = 0;
            im->width = atoi(smallBuf);
            /* read height */
            myread(src, count, &c);
            if (count != 1)
                return -1;
            for (i = 0; isdigit(c) && (i + 1 < sizeof(smallBuf)); i++)
            {
                smallBuf[i] = c;
                myread(src, count, &c);
                if (count != 1)
                    return -1;
            }
            if (!isspace(c))
            {
                PrintError("Bad input file format");
                return -1;
            }
            smallBuf[i] = 0;
            im->height = atoi(smallBuf);
            /* read numColors */
            myread(src, count, &c);
            if (count != 1)
                return -1;
            for (i = 0; isdigit(c) && (i + 1 < sizeof(smallBuf)); i++)
            {
                smallBuf[i] = c;
                myread(src, count, &c);
                if (count != 1)
                    return -1;
            }
            if (!isspace(c))
            {
                PrintError("Bad input file format");
                return -1;
            }
            smallBuf[i] = 0;
            if (atoi(smallBuf) != 255)
                PrintError("Warning: Wrong Colordepth!");
            break;
        default:
            PrintError("Bad input file format");
            return (-1);
            break;
        }
    }

    /* the header has been taken care of.  The rest of the file is just image data. */
    return (0);
}



int writePPM(Image * im, fullPath * sfile)
{
    char header[30];
    size_t count;
    file_spec fnum;
    int y, cy, cpy;
    unsigned char *data;



    if (myopen(sfile, write_bin, fnum))
    {
        PrintError("Error Writing Image File");
        return -1;
    }

    if (im->bitsPerPixel == 32)
        FourToThreeBPP(im);

    if (im->bytesPerLine != im->width * 3)      // Eliminate Pad bytes
    {
        data = *(im->data);
        for (y = 0; y < im->height; y++)
        {
            cy = im->height * im->bytesPerLine;
            cpy = im->height * im->width * 3;

            memcpy(data + cpy, data + cy, (size_t) (im->width * 3));
        }
        im->bytesPerLine = im->width * 3;
        im->dataSize = im->height * im->bytesPerLine;
    }


    sprintf(header, "P6\n" FMT_INT32 " " FMT_INT32 "\n%ld\n", im->width,
            im->height, 255L);

    count = strlen(header);
    mywrite(fnum, count, header);

    if (count != strlen(header))
    {
        PrintError("Error writing file header");
        return -1;
    }

    count = im->dataSize;
    mywrite(fnum, count, *(im->data));

    if (count != im->dataSize)
    {
        PrintError("Error writing image data");
        return -1;
    }

    myclose(fnum);
    return 0;
}


int panoPPMRead(Image * im, fullPath * sfile)
{
  if (readPPM(im, sfile) == 0) {
      return panoMetadataUpdateFromImage(im);
  } else
      return FALSE;
}
