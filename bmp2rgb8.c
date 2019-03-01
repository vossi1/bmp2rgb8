// bmp2rgb8.c
// converts 24bit windows bmp files to 8bit rgb for V99x8 mode 7
// designed by Vossi on 01/2019 in Hamburg/Germany
// version 1.2 copyright (c) 2019 Vossi. All rights reserved.

// option -p outputs a bmp file with color reduced to 8bit rgb
// turns upside down bmp

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
  int i, n, x, y;
  byte buf[256];
  byte rg[8] = {0,0x24,0x49,0x6d,0x92,0xb6,0xdb,0xff};
  word headersize;
  word xres, yres, xrgb;

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
    fprintf(stderr,"bmp2rgb8 v.1.2 by Vossi 01/2019\n");
    fprintf(stderr,"converts 24bit windows bmp files to 8bit rgb for V99x8 mode 7\n");
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
          yres=buf[4] + 256 * buf[5];
          if(xres > 0 && yres > 0 && xres <= 256 && yres <= 212)
          {
            if(preview)
              while(fread(buf,3,1,f))
                printf("%c%c%c",((buf[0]&0xc0)>>6)*0x55, rg[(buf[1]&0xe0)>>5], rg[(buf[2]&0xe0)>>5]);
            else
            {
              byte databuf[xres*yres*3];
              if(fread(databuf,xres*yres*3,1,f))
              {
                xrgb = xres;
                if(xrgb == 256) xrgb = 0;       // 0 -> 256
                printf("%c%c", (char)xrgb, (char)yres);
                for(y=yres-1;y>=0;y--)
                  for(x=0;x<xres;x++)
                    printf("%c",(databuf[y*xres*3+x*3+1]&0xe0)+((databuf[y*xres*3+x*3+2]>>3)&0x1c)+(databuf[y*xres*3+x*3]>>6));
              }
            }
          }
          else
          {
            if(xres == 0 || yres == 0)
            	printf("BMP image size < 1!\n");  
            else 
              if(yres > 0x7fff)
            		printf("BMP image is bottom to top!\n");  
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
