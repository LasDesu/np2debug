
#ifdef __cplusplus
extern "C" {
#endif

#if !defined(MEMTRACE) && !defined(MEMCHECK)

#define	_MEM_INIT()				
#define	_MALLOC(a, b)			malloc(a)
#define	_MFREE(a)				free(a)
#define	_HANDLE_ADD(a, b)		
#define	_HANDLE_REM(a)			
#define	_MEM_USED(a)			

#elif defined(MEMTRACE)

#define	_MEM_INIT()				_meminit()
#define	_MALLOC(a, b)			_memalloc((a), (b))
#define	_MFREE(a)				_memfree(a)
#define	_HANDLE_ADD(a, b)		_handle_append((a), (b))
#define	_HANDLE_REM(a)			_handle_remove(a)
#define	_MEM_USED(a)			_memused(a)

void _meminit(void);
void *_memalloc(int size, const OEMCHAR *name);
void _memfree(void *hdl);
void _handle_append(void *hdl, const OEMCHAR *name);
void _handle_remove(void *hdl);
void _memused(const OEMCHAR *filename);

#elif defined(MEMCHECK)

extern	BOOL	chgmemory;
extern	UINT	usedmemory;

#define	_MEM_INIT()				_meminit()
#define	_MALLOC(a, b)			_memalloc(a)
#define	_MFREE(a)				_memfree(a)
#define	_HANDLE_ADD(a, b)		
#define	_HANDLE_REM(a)			
#define	_MEM_USED(a)			

void _meminit(void);
void *_memalloc(int size);
void _memfree(void *hdl);

#endif

#if !defined(__GNUC__)
void *memmem(const void *haystack, size_t hlen, const void *needle, size_t nlen);
#endif
size_t memchr_len(const void* ptr, const char chr, const size_t len);

#ifdef __cplusplus
}
#endif
