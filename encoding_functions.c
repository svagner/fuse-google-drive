#include <iconv.h>
#include <unistd.h>
#include <string.h>

#define GOOGLE_CODE_PAGE    "UTF-8"
	
int
encode(char *input, char *buffer, size_t buffsize, const char *codeto)
{
	size_t insize;
	const char* in = input;
	const char* out = buffer;
	int iconverr;

	iconv_t d = iconv_open(codeto, GOOGLE_CODE_PAGE);
	if (d == (iconv_t)(-1))
		return -1;
	insize = strlen(input);
	bzero(buffer, buffsize);
	if((iconverr = iconv(d, &in, &insize, &out, &buffsize))==-1)
		return -1;
	iconv_close(d);
}

int
decode(char *input, char *buffer, size_t buffsize, const char *codefrom)
{
	size_t insize;
	const char* in = input;
	const char* out = buffer;
	int iconverr;

	iconv_t d = iconv_open(GOOGLE_CODE_PAGE, codefrom);
	if (d == (iconv_t)(-1))
		return -1;
	insize = strlen(input);
	bzero(buffer, buffsize);
	if((iconverr = iconv(d, &in, &insize, &out, &buffsize))==-1)
		return -1;
	iconv_close(d);
}
