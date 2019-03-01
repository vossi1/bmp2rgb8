// bmp2rgb8.c
// converts 24bit bmp files to 8bit rgb for V99x8 mode 7
// written by Vossi on 01/2019 in Hamburg/Germany
// version 1.1 copyright (c) 2019 Vossi. All rights reserved.

// option -p outputs a bmp file with color reduced to 8bit rgb
// needs top-to-bottom bmp with negative heigh!

// rgb format: 1. byte x resolution (1-255. 0=256)
//             2. byte y resolution (1-212)
//             3- data bytes 3bit G + 3 bit R + 2 bit B

#include <stdio.h>
#include <string.h>

typedef unsigned char byte;
typedef unsigned short word;

static int preview;

int main(int argc,char *argv[])
{
  FILE *f;
  int i, n;
  byte buf[256];
  byte rg[8] = {0,0x24,0x49,0x6d,0x92,0xb6,0xdb,0xff};
  word headersize;
  word xres, yres;

  preview=0;

  for(n=1;(n<argc)&&(*argv[n]=='-');n++)  // loop for argc>1 & starting with -
    switch(argv[n][1])                    // check first character
    {
      case 'p': preview=1; break;
      default:
        fprintf(stderr,"%s: Unknown option -%c\n",argv[0],argv[n][1]);
    }
  
  if(n==argc)  
  {
    fprintf(stderr,"bmp2rgb8 v.1.1 by Vossi 01/2019\n");
    fprintf(stderr,"converts 24bit bmp files to 8bit rgb for V99x8 mode 7\n");
    fprintf(stderr,"usage: %s [-p] file\n",argv[0]);
    fprintf(stderr,"  -p - preview -> bmp reduced to 8bit rgb colors;)\n");
    return(1);
  }
    
  if(!(f=fopen(argv[n],"rb")))
  { printf("\n%s: Can't open file %s\n",argv[0],argv[n]); return(1); }
  if(fread(buf,2,1,f)&&buf[0]=='B'&&buf[1]=='M')
  {
    if(preview) printf("%c%c",buf[0], buf[1]);
    if(fread(buf,16,1,f))
    {
      if(preview) for(i=0;i<16;i++) printf("%c",buf[i]);
      headersize = buf[12] - 4;
      if(buf[13]+buf[14]+buf[15] == 0)
      {
        if(fread(buf,headersize,1,f))
        {
          if(preview) for(i=0;i<headersize;i++) printf("%c",buf[i]);
          xres=buf[0] + 256 * buf[1];
          yres=0 - (buf[4] + 256 * buf[5]);
          if(xres > 0 && yres > 0 && xres <= 256 && yres <= 212)
          {
            if(xres == 256) xres = 0;				// 0 -> 256
            if(!preview) printf("%c%c", (char)xres, (char)yres);
            while(fread(buf,3,1,f))
            {
              if(preview)
                printf("%c%c%c",((buf[0]&0xc0)>>6)*0x55, rg[(buf[1]&0xe0)>>5], rg[(buf[2]&0xe0)>>5]);
              else
                printf("%c",(buf[1]&0xe0)+((buf[2]>>3)&0x1c)+(buf[0]>>6));
            }
          }
          else
          {
            if(xres == 0 || yres == 0)
            	printf("BMP image size < 1!\n");  
            else 
              if(yres > 0x7fff)
            		printf("BMP image is upside down!\n");  
            	else
            		printf("BMP image is too large (max x=256, y=212!\n");  
          }
        }
        else printf("BMP header corrupt!\n");  
      }
      else printf("BMP header too large!\n");  
    }
    else printf("BMP header corrupt!\n");  
  }
  fclose(f); return(0);
}
