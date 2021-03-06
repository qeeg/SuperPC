#include "interface.h"
#include "cga.h"

namespace CGA
{

#ifdef USE_SDL
SDL_Surface* cgascr;
#else
u8 cgascr[640*400];
#endif
u8 ROM[0x2000];
u8 crtcindex = 0;
u8 htotal = 0;
u8 hdisp = 0;
u8 hsyncp = 0;
u8 hsyncw = 0;
u8 vtotal = 0;
u8 vtotala = 0;
u8 vdisp = 0;
u8 vsyncp = 0;
u8 maxscan = 1;
u8 curstart = 0;
u8 curend = 0;
u16 startaddr = 0;
u16 curaddr = 0;

int framecount = 0;

bool crt = false;

u8 dispmode = 0;
u8 status = 0x80;
u8 color = 0;

u8 palette[16][3] =
{
    {0x00,0x00,0x00},
    {0xFF,0xFF,0xFF},
    {0xAA,0xAA,0xAA},
    {0x55,0x55,0x55},
    {0xFF,0xFF,0x55},
    {0xAA,0x55,0x00},
    {0xFF,0x55,0x55},
    {0xAA,0x00,0x00},
    {0x55,0xFF,0x55},
    {0x00,0xAA,0x00},
    {0x55,0xFF,0xFF},
    {0x00,0xAA,0xAA},
    {0x55,0x55,0xFF},
    {0x00,0x00,0xAA},
    {0xFF,0x55,0xFF},
    {0xAA,0x00,0xAA},
};

u8 status_r(u16 addr)
{
    return status;
}

void putpix(int x, int y, u8 r, u8 g, u8 b)
{
#ifdef USE_SDL
    u8* p = static_cast<u8*>(cgascr->pixels);//(u8*)INTERFACE::screen->pixels;
    p[(((y*640)+x)*3)] = b;
    p[(((y*640)+x)*3)+1] = g;
    p[(((y*640)+x)*3)+2] = r;
#else
    cgascr[(((y*640)+x)*4)] = r;
    cgascr[(((y*640)+x)*4)+1] = g;
    cgascr[(((y*640)+x)*4)+2] = b;
    cgascr[(((y*640)+x)*4)+3] = 0xff;
#endif
}

void tick_frame()
{
    if(dispmode & 2)
    {
        if(!(dispmode & 0x10))
        {
        for(int y = 0;y<(vdisp*(maxscan+1));y++)
        {
            for(int x = 0; x<((hdisp+1)*8);x++)
            {
                int tmp = (x+(y*((hdisp+1)*8)));
                u8 data = RAM::RAM[((y&1) ? 0xB8000 : 0xBA000) + (tmp>>2)];
                u8 pixel = (data >> (((tmp % 8) & 3) << 1)) & 3;
                switch(pixel)
                {
                    case 0:
                    {
                        putpix(x,y,palette[13][0],palette[13][1],palette[13][2]);
                        break;
                    }
                    case 1:
                    {
                        putpix(x,y,palette[11][0],palette[11][1],palette[11][2]);
                        break;
                    }
                    case 2:
                    {
                        putpix(x,y,palette[15][0],palette[15][1],palette[15][2]);
                        break;
                    }
                    case 3:
                    {
                        putpix(x,y,palette[1][0],palette[1][1],palette[1][2]);
                        break;
                    }
                }
            }
        }
        }
        else
        {
        for(int y = 0;y<(vdisp*(maxscan+1));y++)
        {
            for(int x = 0; x<((hdisp+1)*8);x++)
            {
                int tmp = (x+(y*((hdisp+1)*8)));
                u8 data = RAM::RAM[((y&1) ? 0xB8000 : 0xBA000) + (tmp>>3)];
                u8 pixel = (data >> (((tmp % 8) & 7) << 1)) & 1;
                switch(pixel)
                {
                    case 0:
                    {
                        putpix(x,y,0,0,0);
                        break;
                    }
                    case 1:
                    {
                        putpix(x,y,255,255,255);
                        break;
                    }
                }
            }
        }
        }
    }
    else
    {
        for(int y = 0; y<(vdisp); y++)
        {
            for(int x = 0; x<(hdisp+1); x++)
            {
                u8 chr = RAM::RAM[0xB8000 + (x + (y*(hdisp+1))<<1)];
                u8 attr = RAM::RAM[0xB8001 + (x + (y*(hdisp+1))<<1)];
                u8 fg[3], bg[3];
                bool blink = false;
                bool underline = false;
                fg[0] = fg[1] = fg[2] = 0;
                bg[0] = bg[1] = bg[2] = 0;
                
                if(attr & 0x08)
                {
                    if(attr & 1) fg[2] = 0xFF;
                    if(attr & 2) fg[1] = 0xFF;
                    if(attr & 4) fg[0] = 0xFF;
                }
                else
                {
                    if(attr & 1) fg[2] = 0xAA;
                    if(attr & 2) fg[1] = 0xAA;
                    if(attr & 4) fg[0] = 0xAA;
                }
                if(attr & 0x80)
                {
                    if(attr & 0x10) bg[2] = 0xFF;
                    if(attr & 0x20) bg[1] = 0xFF;
                    if(attr & 0x40) bg[0] = 0xFF;
                }
                else
                {
                    if(attr & 0x10) bg[2] = 0xAA;
                    if(attr & 0x20) bg[1] = 0xAA;
                    if(attr & 0x40) bg[0] = 0xAA;
                }
                for(int iy = 0; iy<(maxscan+1); iy++)
                {
                    int tmp = iy % (maxscan+1);
                    u8 chrdata;
                    if(tmp & 8) chrdata = ROM[(0x1800 | (tmp & 7)) + (chr*8)];
                    else chrdata = ROM[0x1000 + (chr*8) + tmp];
                    if(chrdata & 0x80) putpix(x*8,(y*(maxscan+1))+iy,fg[0],fg[1],fg[2]);
                    if(blink && (framecount & 0x10)) putpix(x*8,(y*(maxscan+1))+iy,bg[2],bg[2],bg[2]);
                    if(chrdata & 0x40) putpix((x*8)+1,(y*(maxscan+1))+iy,fg[2],fg[2],fg[2]);
                    if(blink && (framecount & 0x10)) putpix((x*8)+1,(y*(maxscan+1))+iy,bg[2],bg[2],bg[2]);
                    if(chrdata & 0x20) putpix((x*8)+2,(y*(maxscan+1))+iy,fg[2],fg[2],fg[2]);
                    if(blink && (framecount & 0x10)) putpix((x*8)+2,(y*(maxscan+1))+iy,bg[2],bg[2],bg[2]);
                    if(chrdata & 0x10) putpix((x*8)+3,(y*(maxscan+1))+iy,fg[2],fg[2],fg[2]);
                    if(blink && (framecount & 0x10)) putpix((x*8)+3,(y*(maxscan+1))+iy,bg[2],bg[2],bg[2]);
                    if(chrdata & 0x08) putpix((x*8)+4,(y*(maxscan+1))+iy,fg[2],fg[2],fg[2]);
                    if(blink && (framecount & 0x10)) putpix((x*8)+4,(y*(maxscan+1))+iy,bg[2],bg[2],bg[2]);
                    if(chrdata & 0x04) putpix((x*8)+5,(y*(maxscan+1))+iy,fg[2],fg[2],fg[2]);
                    if(blink && (framecount & 0x10)) putpix((x*8)+5,(y*(maxscan+1))+iy,bg[2],bg[2],bg[2]);
                    if(chrdata & 0x02) putpix((x*8)+6,(y*(maxscan+1))+iy,fg[2],fg[2],fg[2]);
                    if(blink && (framecount & 0x10)) putpix((x*8)+6,(y*(maxscan+1))+iy,bg[2],bg[2],bg[2]);
                    if(chrdata & 0x01) putpix((x*8)+7,(y*(maxscan+1))+iy,fg[2],fg[2],fg[2]);
                    if(blink && (framecount & 0x10)) putpix((x*8)+7,(y*(maxscan+1))+iy,bg[2],bg[2],bg[2]);
                }
            }
        }
        framecount++;
        if(framecount == 0x1F) framecount = 0;
    }
#ifdef USE_SDL
    SDL_BlitSurface(cgascr, nullptr, INTERFACE::screen, NULL);
#endif
}
void cga_w(u16 addr, u8 value)
{
    switch(addr)
    {
    case 4:
        crtcindex = value;
        break;
    case 5:
    {
        switch(crtcindex)
        {
        case 0:
        {
            htotal = value;
            break;
        }
        case 1:
        {
            hdisp = value;
            break;
        }
        case 2:
        {
            hsyncp = value;
            break;
        }
        case 3:
        {
            hsyncw = value;
            break;
        }
        case 4:
        {
            vtotal = value;
            break;
        }
        case 5:
        {
            vtotala = value;
            break;
        }
        case 6:
        {
            vdisp = value;
            break;
        }
        case 7:
        {
            vsyncp = value;
            break;
        }
        case 9:
        {
            maxscan = value;
            break;
        }
        case 0x0A:
        {
            curstart = value;
            break;
        }
        case 0x0B:
        {
            curend = value;
            break;
        }
        case 0x0C:
        {
            startaddr = (startaddr & 0x00FF) | (value << 8);
            break;
        }
        case 0x0D:
        {
            startaddr = (startaddr & 0xFF00) | value;
            break;
        }
        case 0x0E:
        {
            curaddr = (curaddr & 0x00FF) | (value << 8);
            break;
        }
        case 0x0F:
        {
            curaddr = (curaddr & 0xFF00) | value;
            break;
        }
        }
        break;
    }
    case 8:
    {
        dispmode = value;
        break;
    }
    case 9:
    {
        color = value;
        break;
    }
    }
}

iohandler cgacrtc = {0x03D0, 0x03DA, NULL, cga_w};
}
