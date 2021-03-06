#include	"compiler.h"
#include	"resource.h"
#include	"np2.h"
#include	"viewer.h"
#include	"viewcmn.h"
#include	"viewmenu.h"
#include	"viewmem.h"
#include	"viewreg.h"
#include	"viewseg.h"
#include	"viewstk.h"
#include	"view1mb.h"
#include	"viewasm.h"
#include	"viewsnd.h"
#include	"cpucore.h"
#include	"pccore.h"
#include	"iocore.h"
#include	"bios/bios.h"


const char viewcmn_hex[16] = {
				'0', '1', '2', '3', '4', '5', '6', '7',
				'8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};


void viewcmn_caption(NP2VIEW_T *view, TCHAR *buf) {

	int		num;
	TCHAR	*p;

	num = ((int)view - (int)np2view) / sizeof(NP2VIEW_T);

	if (view->lock) {
		p = _T("Locked");
	}
	else {
		p = _T("Realtime");
	}
	wsprintf(buf, _T("%d.%s - NP2 Debug Utility"), num + 1, p);
}


void viewcmn_putcaption(NP2VIEW_T *view) {

	TCHAR	buf[256];

	viewcmn_caption(view, buf);
	SetWindowText(view->hwnd, buf);
}



// ----

BOOL viewcmn_alloc(VIEWMEMBUF *buf, UINT32 size) {

	if (buf->type == ALLOCTYPE_ERROR) {
		return(FAILURE);
	}
	if (buf->size < size) {
		if (buf->ptr) {
			free(buf->ptr);
		}
		ZeroMemory(buf, sizeof(VIEWMEMBUF));
		buf->ptr = malloc(size);
		if (!buf->ptr) {
			buf->type = ALLOCTYPE_ERROR;
			return(FAILURE);
		}
		buf->size = size;
	}
	return(SUCCESS);
}


void viewcmn_free(VIEWMEMBUF *buf) {

	if (buf->ptr) {
		free(buf->ptr);
	}
	ZeroMemory(buf, sizeof(VIEWMEMBUF));
}


// ----

NP2VIEW_T *viewcmn_find(HWND hwnd) {

	int			i;
	NP2VIEW_T	*view;

	view = np2view;
	for (i=0; i<NP2VIEW_MAX; i++, view++) {
		if ((view->alive) && ((view->hwnd == hwnd) || view->clientwnd == hwnd)) {
			return(view);
		}
	}
	return(NULL);
}


void viewcmn_setmode(NP2VIEW_T *dst, NP2VIEW_T *src, UINT8 type) {

	POINTS* size = NULL;
	switch(type) {
		case VIEWMODE_REG:
			viewreg_init(dst, src);
			size = &viewcfg.size_reg;
			break;

		case VIEWMODE_SEG:
			viewseg_init(dst, src);
			size = &viewcfg.size_mem;
			break;

		case VIEWMODE_STK:
			viewstk_init(dst, src);
			size = &viewcfg.size_stk;
			break;

		case VIEWMODE_1MB:
			view1mb_init(dst, src);
			size = &viewcfg.size_mem;
			break;

		case VIEWMODE_ASM:
			viewasm_init(dst, src);
			size = &viewcfg.size_asm;
			break;

		case VIEWMODE_SND:
			viewsnd_init(dst, src);
			size = &viewcfg.size_snd;
			break;
	}
	if(size && size->x && size->y)	{
		SetWindowPos(dst->hwnd, NULL, 0, 0, size->x, size->y, SWP_NOMOVE);
	}
	viewmenu_mode(dst);
}

LRESULT CALLBACK viewcmn_dispat(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {

	NP2VIEW_T *dbg;

	dbg = viewcmn_find(hwnd);
	if (dbg) {
		switch(dbg->type) {
			case VIEWMODE_REG:
				return(viewreg_proc(dbg, hwnd, msg, wp, lp));

			case VIEWMODE_SEG:
				return(viewseg_proc(dbg, hwnd, msg, wp, lp));

			case VIEWMODE_STK:
				return(viewstk_proc(dbg, hwnd, msg, wp, lp));

			case VIEWMODE_1MB:
				return(view1mb_proc(dbg, hwnd, msg, wp, lp));

			case VIEWMODE_ASM:
				return(viewasm_proc(dbg, hwnd, msg, wp, lp));

			case VIEWMODE_SND:
				return(viewsnd_proc(dbg, hwnd, msg, wp, lp));
		}
	}
	return(0L);
}

void viewcmn_setbank(NP2VIEW_T *view) {

	VIEWMEM_T	*dmem;

	dmem = &view->dmem;
	dmem->vram = gdcs.disp;
	dmem->itf = CPU_ITFBANK;
	dmem->A20 = (UINT8)((CPU_ADRSMASK >> 20) & 1);
}

void viewcmn_reload(NP2VIEW_T *view) {

	switch(view->type)	{
		case VIEWMODE_ASM:
			viewasm_reload(view);
			break;
		case VIEWMODE_STK:
			viewstk_reload(view);
			break;
	}
}


// ----

static void modmenu(HMENU hmenu, int pos, UINT16 id, const TCHAR *seg, UINT16 value) {

	TCHAR	buf[256];

	wsprintf(buf, _T("Seg = &%s [%04x]"), seg, value);
	ModifyMenu(hmenu, pos, MF_BYPOSITION | MF_STRING, id, buf);
}

void viewcmn_setmenuseg(HWND hwnd) {

	HMENU	hparent;
	HMENU	hmenu;

	hparent = GetMenu(hwnd);
	if (hparent == NULL) {
		return;
	}
	hmenu = GetSubMenu(hparent, 2);

	if (hmenu) {
		modmenu(hmenu, 2, IDM_SEGCS, _T("CS"), CPU_CS);
		modmenu(hmenu, 3, IDM_SEGDS, _T("DS"), CPU_DS);
		modmenu(hmenu, 4, IDM_SEGES, _T("ES"), CPU_ES);
		modmenu(hmenu, 5, IDM_SEGSS, _T("SS"), CPU_SS);
		DrawMenuBar(hwnd);
	}
}

// -----

void viewcmn_setvscroll(NP2VIEW_T *view) {

	ZeroMemory(&(view->si), sizeof(SCROLLINFO));
	view->si.cbSize = sizeof(SCROLLINFO);
	view->si.fMask = SIF_ALL;
	view->si.nMin = 0;
	view->si.nPage = view->step / view->mul;
	view->si.nMax = ((view->maxline) / view->mul) - 1;
	view->si.nPos = view->pos / view->mul;
	SetScrollInfo(view->clientwnd, SB_VERT, &(view->si), TRUE);
}

// -----

void viewcmn_paint(NP2VIEW_T *view, void (*callback)(NP2VIEW_T *view, RECT *rc, HDC hdc)) {

	HDC			hdc;
	PAINTSTRUCT	ps;
	RECT		rc;
	HDC			hmemdc;
	HBITMAP		hbitmap;
	HBRUSH		hbrush;

	hdc = BeginPaint(view->clientwnd, &ps);
	GetClientRect(view->clientwnd, &rc);
	hmemdc = CreateCompatibleDC(hdc);
	hbitmap = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
	hbitmap = (HBITMAP)SelectObject(hmemdc, hbitmap);
	hbrush = (HBRUSH)SelectObject(hmemdc, CreateSolidBrush(viewcfg.color_back));
	PatBlt(hmemdc, 0, 0, rc.right, rc.bottom, PATCOPY);
	DeleteObject(SelectObject(hmemdc, hbrush));

	SetTextColor(hmemdc, viewcfg.color_text);
	SetBkColor(hmemdc, viewcfg.color_back);
	SelectObject(hmemdc, np2viewfont);

	callback(view, &rc, hmemdc);

	BitBlt(hdc, 0, 0, rc.right, rc.bottom, hmemdc, 0, 0, SRCCOPY);
	DeleteObject(SelectObject(hmemdc, hbitmap));
	DeleteDC(hmemdc);
	EndPaint(view->clientwnd, &ps);
}
