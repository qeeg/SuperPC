namespace MDA
{
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

	u8 dispmode = 2; //Bit 1 must be set at all times, unless the processor wants to be halted.
	u8 status = 0x80;

	u8 status_r(u16 addr)
	{
		return status;
	}
	void tick_frame()
	{
		for(int y = 0;y<(vdisp*(maxscan+1));y++)
		{
			for(int x = 0;x<(hdisp*9);x++)
			{
				u8 chr = RAM::RAM[0xB0000 + (((x/9) + (y/(maxscan+1)))>>1)];
				u8 attr = RAM::RAM[0xB0001 + (((x/9) + (y/(maxscan+1)))>>1)];
				//TODO: This next part is highly inaccurate and ignores everything but the character chosen.
				u8 chrdata = ROM[(chr * 14) + (y%(maxscan+1))];
				if((x % 9) == 0)
			}
		}
		framecount++;
	}
	void crtc_w(u16 addr, u8 value)
	{
		switch(addr&1)
		{
			case 0: crtcindex = value; break;
			case 1:
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
		}
	}
}