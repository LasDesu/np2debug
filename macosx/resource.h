
#define	IDM_MAINMENU	128

#define	MAKE_MENUID(a, b)	((a << 16) | b)

// --------------------------------------------------------------- system menu

#define	IDM_APPLE			128
#define	IDM_EMULATE			129
#define	IDM_FDD1			132
#define	IDM_FDD2			133
#define	IDM_HARDDISK		134
	#define	IDM_SASI1		135
	#define	IDM_SASI2		136
#define	IDM_SCREEN			137
#define	IDM_DEVICE			138
	#define	IDM_KEYBOARD	139
	#define	IDM_SOUND		140
	#define	IDM_MEMORY		141
#define	IDM_OTHER			142

#define	IDM_ABOUT			MAKE_MENUID(IDM_APPLE, 1)

#define	IDM_RESET			MAKE_MENUID(IDM_EMULATE, 1)
#define	IDM_CONFIGURE		MAKE_MENUID(IDM_EMULATE, 3)
#define	IDM_NEWFDD			MAKE_MENUID(IDM_EMULATE, 4)
#define	IDM_NEWHDD			MAKE_MENUID(IDM_EMULATE, 5)
#define	IDM_FONT			MAKE_MENUID(IDM_EMULATE, 6)
#define	IDM_EXIT			MAKE_MENUID(IDM_EMULATE, 8)

#define	IDM_FDD1OPEN		MAKE_MENUID(IDM_FDD1, 1)
#define	IDM_FDD1EJECT		MAKE_MENUID(IDM_FDD1, 3)

#define	IDM_FDD2OPEN		MAKE_MENUID(IDM_FDD2, 1)
#define	IDM_FDD2EJECT		MAKE_MENUID(IDM_FDD2, 3)

#define	IDM_SASI1OPEN		MAKE_MENUID(IDM_SASI1, 1)
#define	IDM_SASI1REMOVE		MAKE_MENUID(IDM_SASI1, 3)
#define	IDM_SASI2OPEN		MAKE_MENUID(IDM_SASI2, 1)
#define	IDM_SASI2REMOVE		MAKE_MENUID(IDM_SASI2, 3)

#define	IDM_MOUSE			MAKE_MENUID(IDM_DEVICE, 4)

#define	IDM_FULLSCREEN		MAKE_MENUID(IDM_SCREEN, 1)

#define	IDM_ROLNORMAL		MAKE_MENUID(IDM_SCREEN, 1+2)
#define	IDM_ROLLEFT			MAKE_MENUID(IDM_SCREEN, 2+2)
#define	IDM_ROLRIGHT		MAKE_MENUID(IDM_SCREEN, 3+2)
#define	IDM_DISPSYNC		MAKE_MENUID(IDM_SCREEN, 5+2)
#define	IDM_RASTER			MAKE_MENUID(IDM_SCREEN, 6+2)
#define	IDM_NOWAIT			MAKE_MENUID(IDM_SCREEN, 7+2)
#define	IDM_AUTOFPS			MAKE_MENUID(IDM_SCREEN, 8+2)
#define	IDM_60FPS			MAKE_MENUID(IDM_SCREEN, 9+2)
#define	IDM_30FPS			MAKE_MENUID(IDM_SCREEN, 10+2)
#define	IDM_20FPS			MAKE_MENUID(IDM_SCREEN, 11+2)
#define	IDM_15FPS			MAKE_MENUID(IDM_SCREEN, 12+2)
#define	IDM_SCREENOPT		MAKE_MENUID(IDM_SCREEN, 14+2)

#define	IDM_KEY				MAKE_MENUID(IDM_KEYBOARD, 1)
#define	IDM_JOY1			MAKE_MENUID(IDM_KEYBOARD, 2)
#define	IDM_JOY2			MAKE_MENUID(IDM_KEYBOARD, 3)
#define	IDM_MOUSEKEY		MAKE_MENUID(IDM_KEYBOARD, 4)
#define	IDM_XSHIFT			MAKE_MENUID(IDM_KEYBOARD, 6)
#define	IDM_XCTRL			MAKE_MENUID(IDM_KEYBOARD, 7)
#define	IDM_XGRPH			MAKE_MENUID(IDM_KEYBOARD, 8)
#define	IDM_F12MOUSE		MAKE_MENUID(IDM_KEYBOARD, 10)
#define	IDM_F12COPY			MAKE_MENUID(IDM_KEYBOARD, 11)
#define	IDM_F12STOP			MAKE_MENUID(IDM_KEYBOARD, 12)

#define	IDM_BEEPOFF			MAKE_MENUID(IDM_SOUND, 1)
#define	IDM_BEEPLOW			MAKE_MENUID(IDM_SOUND, 2)
#define	IDM_BEEPMID			MAKE_MENUID(IDM_SOUND, 3)
#define	IDM_BEEPHIGH		MAKE_MENUID(IDM_SOUND, 4)
#define	IDM_NOSOUND			MAKE_MENUID(IDM_SOUND, 6)
#define	IDM_PC9801_14		MAKE_MENUID(IDM_SOUND, 7)
#define	IDM_PC9801_26K		MAKE_MENUID(IDM_SOUND, 8)
#define	IDM_PC9801_86		MAKE_MENUID(IDM_SOUND, 9)
#define	IDM_PC9801_26_86	MAKE_MENUID(IDM_SOUND, 10)
#define	IDM_PC9801_86_CB	MAKE_MENUID(IDM_SOUND, 11)
#define	IDM_PC9801_118		MAKE_MENUID(IDM_SOUND, 12)
#define	IDM_SPEAKBOARD		MAKE_MENUID(IDM_SOUND, 13)
#define	IDM_SPARKBOARD		MAKE_MENUID(IDM_SOUND, 14)
#define	IDM_SEEKSND			MAKE_MENUID(IDM_SOUND, 16)

#define	IDM_MEM640			MAKE_MENUID(IDM_MEMORY, 1)
#define	IDM_MEM16			MAKE_MENUID(IDM_MEMORY, 2)
#define	IDM_MEM36			MAKE_MENUID(IDM_MEMORY, 3)
#define	IDM_MEM76			MAKE_MENUID(IDM_MEMORY, 4)

#define	IDM_BMPSAVE			MAKE_MENUID(IDM_OTHER, 1)
#define	IDM_S98LOGGING		MAKE_MENUID(IDM_OTHER, 2)
#define	IDM_DISPCLOCK		MAKE_MENUID(IDM_OTHER, 3)
#define	IDM_DISPFLAME		MAKE_MENUID(IDM_OTHER, 4)
#define	IDM_JOYX			MAKE_MENUID(IDM_OTHER, 5)
#define	IDM_RAPID			MAKE_MENUID(IDM_OTHER, 6)
#define IDM_MSRAPID			MAKE_MENUID(IDM_OTHER, 7)
#define	IDM_I286SAVE		MAKE_MENUID(IDM_OTHER, 8)

// -------------------------------------------------------------------- dialog

#define	IDOK				1
#define	IDCANCEL			2

#define	IDD_ABOUT			128

#define	IDD_NEWDISK			129

#define	IDC_DISKLABEL		4
#define	IDC_MAKE2DD			6
#define	IDC_MAKE2HD			7


#define	IDD_NEWHDDDISK		130

#define	IDC_HDDSIZE			3


// ------------------------------------------------------------------ Messages

#define	IDT_TITLE			128
#define	IDT_NEWDISK			129
#define	IDT_NEWHDDDISK		130

