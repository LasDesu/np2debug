#include	"compiler.h"
#include	"getsnd.h"


#if defined(__GNUC__)
typedef struct {
	char	head[4];
	BYTE	size[4];
	char	fmt[4];
} __attribute__ ((packed)) RIFF_HEADER;

typedef struct {
	char	head[4];
	BYTE	size[4];
} __attribute__ ((packed)) WAVE_HEADER;

typedef struct {
	BYTE	format[2];
	BYTE	channel[2];
	BYTE	rate[4];
	BYTE	rps[4];
	BYTE	block[2];
	BYTE	bit[2];
} __attribute__ ((packed)) WAVE_INFOS;

typedef struct {
	BYTE	exsize[2];
	BYTE	spb[2];
	BYTE	numcoef[2];
} __attribute__ ((packed)) WAVE_MSA_INFO;

#else /* __GNUC__ */

#pragma pack(push, 1)
typedef struct {
	char	head[4];
	BYTE	size[4];
	char	fmt[4];
} RIFF_HEADER;

typedef struct {
	char	head[4];
	BYTE	size[4];
} WAVE_HEADER;

typedef struct {
	BYTE	format[2];
	BYTE	channel[2];
	BYTE	rate[4];
	BYTE	rps[4];
	BYTE	block[2];
	BYTE	bit[2];
} WAVE_INFOS;

typedef struct {
	BYTE	exsize[2];
	BYTE	spb[2];
	BYTE	numcoef[2];
} WAVE_MSA_INFO;
#pragma pack(pop)

#endif /* __GNUC__ */

static const char fmt_riff[] = "RIFF";
static const char fmt_wave[] = "WAVE";
static const char fmt_rmp3[] = "RMP3";
static const char chunk_fmt[] = "fmt ";
static const char chunk_data[] = "data";


// ---- PCM

#if defined(BYTESEX_LITTLE)
static UINT pcm_dec(GETSND snd, void *dst) {

	UINT	size;

	size = min(snd->blocksize, snd->datsize);
	if (size) {
		CopyMemory(dst, snd->datptr, size);
		snd->datptr += size;
		snd->datsize -= size;
		size >>= (int)(long)snd->snd;
	}
	return(size);
}
#elif defined(BYTESEX_BIG)
static UINT pcm_dec(SMIXTRACK *trk, short *dst) {

#error pcm_dec: unsupported
	return(0);
}
#endif

static const BYTE abits[4] = {0, 1, 0, 2};

static BOOL pcm_open(GETSND snd) {

	UINT	align;

	if ((snd->bit != 8) && (snd->bit != 16)) {
		goto pcmopn_err;
	}
	align = snd->channels * (snd->bit >> 3);
	if (snd->blocksize != align) {
		goto pcmopn_err;
	}
	snd->blocksamples = 0x800;					// �K���ɁB
	snd->blocksize *= snd->blocksamples;
	snd->snd = (void *)(long)abits[align - 1];
	snd->dec = pcm_dec;
	return(SUCCESS);

pcmopn_err:
	return(FAILURE);
}


// ---- MS-ADPCM

typedef struct {
	short Coef1;
	short Coef2;
} __COEFPAIR;

static const int MSADPCMTable[16] = {
	230, 230, 230, 230, 307, 409, 512, 614,
	768, 614, 512, 409, 307, 230, 230, 230 
};

static UINT msa_dec(GETSND snd, short *dst) {

	BYTE		*buf;
	UINT		size;
	UINT		outsamples;
	int			pred[2], delta[2], nibble;
	UINT		i;
	BYTE		indata;
	__COEFPAIR	*coef;
	UINT		ch;
	long		outdata;

	buf = snd->datptr;						// ���[�N�g���Ă܂���B
	size = min(snd->datsize, snd->blocksize);
	snd->datptr += size;
	snd->datsize -= size;

	outsamples = 0;
	if (snd->channels == 1) {
		if (size >= 7) {
			pred[0]  = buf[0];
			pred[1]  = 0;
			delta[0] = LOADINTELWORD(buf+1);
			delta[1] = 0;
			*dst++   = (short)LOADINTELWORD(buf+5);
			*dst++   = (short)LOADINTELWORD(buf+3);
			buf += 7;
			outsamples = 2 + ((size - 7) * 2);
		}
	}
	else {
		if (size >= 14) {
			pred[0]  = buf[0];
			pred[1]  = buf[1];
			delta[0] = LOADINTELWORD(buf+2);
			delta[1] = LOADINTELWORD(buf+4);
			*dst++   = (short)LOADINTELWORD(buf+10);
			*dst++   = (short)LOADINTELWORD(buf+12);
			*dst++   = (short)LOADINTELWORD(buf+6);
			*dst++   = (short)LOADINTELWORD(buf+8);
			buf += 14;
			outsamples = 2 + (size - 14);
		}
	}
	coef = (__COEFPAIR *)snd->snd;
	nibble = 0;
	indata = 0;			// for gcc
	for (i=2; i<outsamples; i++) {
		for (ch=0; ch<snd->channels; ch++) {
			int d = delta[ch], p, data;
			if (!nibble) {
				indata = *buf++;
				data = indata >> 4;
			}
			else {
				data = indata & 15;
			}
			nibble ^= 1;
			delta[ch] = (MSADPCMTable[data] * d) >> 8;
			if (delta[ch] < 16) {
				delta[ch] = 16;
			}
			p = (((*(dst - snd->channels  )) * coef[pred[ch]].Coef1)
				+((*(dst - snd->channels*2)) * coef[pred[ch]].Coef2));
			p >>= 8;
			outdata = (((data>=8)?(data-16):data) * d) + p;
			if (outdata > 32767) {
				outdata = 32767;
			}
			else if (outdata < -32768) {
				outdata = -32768;
			}
			*dst++ = (short)outdata;
		}
	}
	return(outsamples);
}

static void msa_decend(GETSND snd) {

	_MFREE(snd->snd);
}

static BOOL msa_open(GETSND snd, WAVE_INFOS *wavehead, UINT headsize) {

	WAVE_MSA_INFO	*info;
	UINT			exsize;
	UINT			numcoef;
	UINT			spb;
	UINT			blk;
	__COEFPAIR		*coef;
	BYTE			*p;

	if (snd->bit != 4) {
		goto msaopn_err;
	}
	info = (WAVE_MSA_INFO *)(wavehead + 1);
	headsize -= sizeof(WAVE_INFOS);
	headsize -= sizeof(WAVE_MSA_INFO);
	if ((signed long)headsize < 0) {
		goto msaopn_err;
	}
	exsize = LOADINTELWORD(info->exsize);
	spb = LOADINTELWORD(info->spb);
	numcoef = LOADINTELWORD(info->numcoef);
	blk = snd->blocksize;
	blk /= snd->channels;
	blk -= 6;
	blk *= 2;
	TRACEOUT(("wav: msa: ExtraSize %d / SPB=%d NumOfCoefs=%d",
													exsize, spb, numcoef));

	if (blk != spb) {
		TRACEOUT(("wav: msa: block size error"));
		goto msaopn_err;
	}
	if (exsize < (numcoef * 4 + 4)) {
		TRACEOUT(("wav: msa: extra info size error"));
		goto msaopn_err;
	}
	if (numcoef == 0) {
		TRACEOUT(("wav: msa: coef == 0"));
		goto msaopn_err;
	}

	coef = (__COEFPAIR*)_MALLOC(numcoef * sizeof(__COEFPAIR), "adpcm coefs");
	if (coef == NULL) {
		goto msaopn_err;
	}

	snd->blocksamples = spb;
	snd->bit = 16;
	snd->dec = (GSDEC)msa_dec;
	snd->decend = msa_decend;
	snd->snd = (void *)coef;

	p = (BYTE *)(info + 1);
	do {
		coef->Coef1 = LOADINTELWORD(p + 0);
		coef->Coef2 = LOADINTELWORD(p + 2);
		coef++;
		p += 4;
	} while(--numcoef);
	return(SUCCESS);

msaopn_err:
	return(FAILURE);
}


// ---- IMA

#define IMA_MAXSTEP		89

static	BOOL	ima_init = FALSE;
static	BYTE	ima_statetbl[IMA_MAXSTEP][8];

static const int ima_stateadj[8] = {-1, -1, -1, -1, 2, 4, 6, 8};

static const int ima_steptable[IMA_MAXSTEP] = {
	    7,    8,    9,   10,   11,   12,   13,   14,   16,   17,   19,
	   21,   23,   25,   28,   31,   34,   37,   41,   45,   50,   55,
	   60,   66,   73,   80,   88,   97,  107,  118,  130,  143,  157,
	  173,  190,  209,  230,  253,  279,  307,  337,  371,  408,  449,
	  494,  544,  598,  658,  724,  796,  876,  963, 1060, 1166, 1282,
	 1411, 1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024, 3327, 3660,
	 4026, 4428, 4871, 5358, 5894, 6484, 7132, 7845, 8630, 9493,10442,
	11487,12635,13899,15289,16818,18500,20350,22385,24623,27086,29794,32767};

static void ima_inittable(void) {

	int		i, j, k;

	for (i=0; i<IMA_MAXSTEP; i++) {
		for (j=0; j<8; j++) {
			k = i + ima_stateadj[j];
			if (k < 0) {
				k = 0;
			}
			else if (k >= IMA_MAXSTEP) {
				k = IMA_MAXSTEP - 1;
			}
			ima_statetbl[i][j] = (BYTE)k;
		}
	}
}

static UINT ima_dec(GETSND snd, short *dst) {

	UINT	c;
	int		val[2], state[2];
	BYTE	*src;
	UINT	blk;

	if (snd->blocksize > snd->datsize) {
		goto imadec_err;
	}
	src = snd->datptr;						// ���[�N�g���Ă܂���B
	snd->datptr += snd->blocksize;
	snd->datsize -= snd->blocksize;

	blk = snd->blocksamples;
	for (c=0; c<snd->channels; c++) {
		short tmp;
		tmp = LOADINTELWORD(src);
		val[c] = tmp;
		*dst++ = (short)val[c];
		state[c] = src[2];
		if (state[c] >= IMA_MAXSTEP) {
			state[c] = 0;
		}
		src += 4;
	}
	blk--;

	while(blk >= 8) {
		blk -= 8;
		for (c=0; c<snd->channels; c++) {
			UINT samp, r;
			samp = 0;			// for gcc
			r = 8;
			do {
				int step, dp;
				if (!(r & 1)) {
					samp = *src++;
				}
				else {
					samp >>= 4;
				}
				step = ima_steptable[state[c]];
				state[c] = ima_statetbl[state[c]][samp & 7];
				dp = ((((samp & 7) << 1) + 1) * step) >> 3;
				if (!(samp & 8)) {
					val[c] += dp;
					if (val[c] > 32767) {
						val[c] = 32767;
					}
				}
				else {
					val[c] -= dp;
					if (val[c] < -32768) {
						val[c] = -32768;
					}
				}
				*dst = (short)val[c];
				dst += snd->channels;
			} while(--r);
			dst -= (8 * snd->channels);
			dst++;
		}
		dst += (7 * snd->channels);
	}
	return(snd->blocksamples);

imadec_err:
	return(0);
}

static BOOL ima_open(GETSND snd) {

	int		blk;

	if (snd->bit != 4) {
		goto imaopn_err;
	}
	blk = snd->blocksize;
	blk /= snd->channels;
	if (blk & 3) {
		goto imaopn_err;
	}
	blk -= 4;				// first block;
	blk *= 2;
	blk += 1;
	snd->blocksamples = blk;
	snd->bit = 16;
	snd->dec = (GSDEC)ima_dec;
	if (!ima_init) {
		ima_init = TRUE;
		ima_inittable();
	}
	return(SUCCESS);

imaopn_err:
	return(FAILURE);
}


// ---- MP3

#if defined(SUPPORT_MP3)
extern BOOL __mp3_open(GETSND snd, BYTE *ptr, UINT size);
#endif


// ----

BOOL getwave_open(GETSND snd, BYTE *ptr, UINT size) {

	RIFF_HEADER		*riff;
	WAVE_HEADER		*head;
	WAVE_INFOS		*info;
	UINT			pos;
	UINT			format;
	BOOL			r;
	UINT			headsize = 0;
	UINT			datasize;

	info = NULL;		// for gcc

	// RIFF�̃`�F�b�N
	riff = (RIFF_HEADER *)ptr;
	pos = sizeof(RIFF_HEADER);
	if (size < pos) {
		TRACEOUT(("wav: error RIFF header"));
		goto gwopn_err;
	}
	if (memcmp(riff->head, fmt_riff, 4)) {
		TRACEOUT(("wav: error RIFF header"));
		goto gwopn_err;
	}
	if (!memcmp(riff->fmt, fmt_wave, 4)) {
		// �t�H�[�}�b�g�w�b�_�`�F�b�N
		head = (WAVE_HEADER *)(ptr + pos);
		pos += sizeof(WAVE_HEADER);
		if (size < pos) {
			TRACEOUT(("wav: error fmt header"));
			goto gwopn_err;
		}
		if (memcmp(head->head, chunk_fmt, 4)) {
			TRACEOUT(("wav: error fmt header"));
			goto gwopn_err;
		}
		headsize = LOADINTELDWORD(head->size);
		if (headsize < sizeof(WAVE_INFOS)) {
			TRACEOUT(("wav: error fmt length"));
			goto gwopn_err;
		}

		// �t�H�[�}�b�g�`�F�b�N
		info = (WAVE_INFOS *)(ptr + pos);
		pos += headsize;
		if (size < pos) {
			TRACEOUT(("wav: error fmt data"));
			goto gwopn_err;
		}
		format = LOADINTELWORD(info->format);
		snd->channels = LOADINTELWORD(info->channel);
		snd->samplingrate = LOADINTELDWORD(info->rate);
		snd->blocksize = LOADINTELWORD(info->block);
		snd->bit = LOADINTELWORD(info->bit);
		TRACEOUT(("wav: fmt: %x / %dch %dHz %dbit",
					format, snd->channels, snd->samplingrate, snd->bit));
		if ((UINT)(snd->channels - 1) >= 2) {
			TRACEOUT(("wav: channels err"));
			goto gwopn_err;
		}
	}
	else if (!memcmp(riff->fmt, fmt_rmp3, 4)) {
		format = 0x55;
	}
	else {
		TRACEOUT(("wav: error WAVE header"));
		goto gwopn_err;
	}

	// data�܂ňړ��B
	while(1) {
		head = (WAVE_HEADER *)(ptr + pos);
		pos += sizeof(WAVE_HEADER);
		if (size < pos) {
			TRACEOUT(("wav: error data header"));
			goto gwopn_err;
		}
		if (!memcmp(head->head, chunk_data, 4)) {
			break;
		}
		pos += LOADINTELDWORD(head->size);
	}
	ptr += pos;
	size -= pos;
	datasize = LOADINTELDWORD(head->size);
	size = min(size, datasize);

	switch(format) {
		case 0x01:				// PCM
			r = pcm_open(snd);
			break;

		case 0x02:
			r = msa_open(snd, info, headsize);
			break;

		case 0x11:
			r = ima_open(snd);
			break;

#if defined(SUPPORT_MP3)
		case 0x55:
			return(__mp3_open(snd, ptr, size));
#endif

#if defined(SUPPORT_OGG)
		case 0x6751:
			return(getogg_open(snd, ptr, size));
#endif

		default:
			r = FAILURE;
			break;
	}
	if (r != SUCCESS) {
		TRACEOUT(("wav: decord open error"));
		goto gwopn_err;
	}

	// �o�^�`
	snd->datptr = ptr;
	snd->datsize = size;
	return(SUCCESS);

gwopn_err:
	return(FAILURE);
}
