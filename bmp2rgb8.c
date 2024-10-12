// bmp2rgb8.c
// converts 24bit windows bmp files to 8bit rgb for V99x8 mode 7
// designed by Vossi on 01/2019 in Hamburg/Germany
// version 1.4 copyright (c) 2019-2024 Vossi. All rights reserved.

// option -p (preview) outputs a bmp file with color reduced to 8bit rgb
// turns bottom to top bmp file (standard)
// saves files directly to drive

// known BUG: works only with even x-size!

// rgb format: 1. word cbm load address $0400
//             3. byte x resolution (1-255. 0=256)
//             4. byte y resolution (1-212)
//             5- data bytes 3bit G + 3 bit R + 2 bit B

// V1.4 added load address, fixed segmentation fault for preview: close only the opened files!

#include <stdio.h>
#include <string.h>

typedef unsigned char byte;
typedef unsigned short word;

static int preview;

int main(int argc,char *argv[])
{
  FILE *f, *f2, *f3;
  int i, n, x, y;
  unsigned int calc;
  char source[40], target[40], previewfile[40];
  char sourceext[5]=".bmp";
  char targetext[5]=".rgb";
  char previewext[7]="_8.bmp";
 
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
    fprintf(stderr,"bmp2rgb8 v1.4 by Vossi 06/2024\n");
    fprintf(stderr,"converts 24bit windows bmp files to 8bit rgb for V99x8 mode 7\n");
    fprintf(stderr,"Attention! This version works only with even x-size!\n");
    fprintf(stderr,"max size: 256 x 212, byte 0+1 = cbm load address $0400, byte 2+3 = dx,dy\n");
    fprintf(stderr,"usage: %s [-p] file[w/o extension]\n",argv[0]);
    fprintf(stderr,"  -p - preview -> writes bmp reduced to 8bit rgb colors;)\n");
    return(1);
  }
  
  strcpy(source,argv[n]);
  strcpy(target,argv[n]);
  strcpy(previewfile,argv[n]);
  strcat(source,sourceext);
  strcat(target,targetext);
  strcat(previewfile,previewext);
  if(!(f=fopen(source,"rb")))
    {printf("\n%s: Can't open file %s\n",argv[0],argv[n]); return(1);}
  if(fread(buf,2,1,f)&&buf[0]=='B'&&buf[1]=='M')
  {
    if(preview) 
    {
      if(!(f3=fopen(previewfile,"wb")))
        {printf("\n%s: Can't open preview target file %s\n",argv[0],previewfile); fclose(f); return(1);}
      if(!(fwrite(buf,2,1,f3)))
        {printf("\n%s: Write error %s\n",argv[0],previewfile); fclose(f); fclose(f3); return(1);}
    }
    if(fread(buf,16,1,f))
    {
      if(preview)
      {
        if(!(fwrite(buf,16,1,f3)))
          {printf("\n%s: Write error %s\n",argv[0],previewfile); fclose(f); fclose(f3); return(1);}
      }
      headersize = buf[12] - 4;
      if(buf[13]+buf[14]+buf[15] == 0)
      {
        if(fread(buf,headersize,1,f))
        {
          if(preview)
          {
            if(!(fwrite(buf,headersize,1,f3)))
              {printf("\n%s: Write error %s\n",argv[0],previewfile); fclose(f); fclose(f3); return(1);}
          }
          xres=buf[0] + 256 * buf[1];
          yres=buf[4] + 256 * buf[5];
          if(xres > 0 && yres > 0 && xres <= 256 && yres <= 212)
          {
            if(preview)
            {
              while(fread(buf,3,1,f))
              {
                buf[0]=((buf[0]&0xc0)>>6)*0x55; buf[1]=rg[(buf[1]&0xe0)>>5]; buf[2]=rg[(buf[2]&0xe0)>>5];
                if(!(fwrite(buf,3,1,f3)))
                  {printf("\n%s: Write error %s\n",argv[0],previewfile); fclose(f); fclose(f3); return(1);}
              }
              fclose(f3);
            }
            else
            {
              if(!(f2=fopen(target,"wb")))
                {printf("\n%s: Can't open target file %s\n",argv[0],target); fclose(f); fclose(f3); return(1);}
              byte databuf[xres*yres*3];
              if(fread(databuf,xres*yres*3,1,f))
              {
                buf[0]=0x00; buf[1]=0x04;    // cbm loadaddress $0400
                if(!(fwrite(buf,2,1,f2)))
                  {printf("\n%s: Write error %s\n",argv[0],target); fclose(f); fclose(f2); return(1);}                        
                xrgb = xres;
                if(xrgb == 256) xrgb = 0;       // 0 -> 256
                buf[0]=(char)xrgb; buf[1]=(char)yres;
                if(!(fwrite(buf,2,1,f2)))
                  {printf("\n%s: Write error %s\n",argv[0],target); fclose(f); fclose(f2); return(1);}                        
                for(y=yres-1;y>=0;y--)
                  for(x=0;x<xres;x++)
                  {
                    calc=(y*xres*3)+(x*3);
                    buf[0]=(databuf[calc+1]&0xe0)|((databuf[calc+2]>>3)&0x1c)|(databuf[calc]>>6);
                    if(!(fwrite(buf,1,1,f2)))
                      {printf("\n%s: Write error %s\n",argv[0],target); fclose(f); fclose(f2); return(1);}                        
                  }
                fclose(f2);
              }
            }
          }
          else
          {
            if(xres == 0 || yres == 0)
            	printf("BMP image size < 1!\n");  
            else 
              if(yres > 0x7fff)
            		printf("BMP image is top to bottom!\n");  
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
