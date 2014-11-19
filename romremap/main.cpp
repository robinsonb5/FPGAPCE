#include <iostream>
#include <getopt.h>

#include "binaryblob.h"

using namespace std;

enum ROM_RemapType {REMAP_NONE,REMAP_ABBC,REMAP_ABCC};

class ROM : public BinaryBlob
{
	public:
	ROM(const char *filename) : BinaryBlob(filename)
	{
	}
	ROM(int size) : BinaryBlob(size)
	{
	}
	virtual ROM *Remap(ROM_RemapType remaptype=REMAP_ABBC)
	{
		ROM *result=0;
		const unsigned char *src=GetPointer();
		unsigned char *dst;
		int srcsize=GetSize();
		int dstsize;
		int chunksize;
		if(srcsize&511)	// Strip header if present.
			src+=512;
		srcsize&=(~511);
		switch(remaptype)
		{
			case REMAP_NONE:
				chunksize=srcsize;
				dstsize=srcsize;
				break;			
			case REMAP_ABBC:
			case REMAP_ABCC:
				chunksize=srcsize/3;
				dstsize=chunksize*4;
				break;
		}
		result=new ROM(dstsize);
		dst=result->GetPointer();
		switch(remaptype)
		{
			case REMAP_NONE:
				memcpy(dst,src,dstsize);
				break;			
			case REMAP_ABBC:
				memcpy(dst,src,chunksize);	// A
				memcpy(dst+chunksize,src+chunksize,chunksize);	// B
				memcpy(dst+chunksize*2,src+chunksize,chunksize*2);	// BC
				break;
			case REMAP_ABCC:
				memcpy(dst,src,chunksize);	// A
				memcpy(dst+chunksize,src+chunksize,chunksize*2);	// BC
				memcpy(dst+chunksize*3,src+chunksize*2,chunksize);	// C
				break;
		}
		return(result);
	}
	virtual ~ROM()
	{
	}

	protected:
};

class ROMMap_Options
{
	public:
	ROMMap_Options(int argc,char *argv[])
	{
		static struct option long_options[] =
		{
			{"help",no_argument,NULL,'h'},
			{"mapping",required_argument,NULL,'m'},
			{"outfile",required_argument,NULL,'o'},
			{0, 0, 0, 0}
		};
		outfile=0;
		while(1)
		{
			int c;
			c = getopt_long(argc,argv,"hs:r:o:bm",long_options,NULL);
			if(c==-1)
				break;
			switch (c)
			{
				case 'h':
					printf("Usage: %s [options]\n",argv[0]);
					printf("    -h --help\t\tdisplay this message\n");
					printf("    -m --mapping\tSpecify a ROM mapping. 0 - None, 1 - ABBC, 2 - ABCC\n");
					printf("    -o --outfile\tspecify output filename.\n");
					break;
				case 'o':
					outfile=optarg;
					break;
				case 'm':
					mapping=ROM_RemapType(atoi(optarg));
					break;
			}
		}
	}
	~ROMMap_Options()
	{
	}
	enum ROM_RemapType mapping;
	const char *outfile;
};


int main(int argc,char **argv)
{
	ROMMap_Options opts(argc,argv);
	if(optind<argc)
	{
		ROM rom(argv[optind]);
		ROM *out=rom.Remap(opts.mapping);
		if(out && opts.outfile)
		{
			out->Save(opts.outfile);
		}
	}
}

