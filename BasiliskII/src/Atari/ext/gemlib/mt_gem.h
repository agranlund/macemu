/**@file
 * main GEMlib header
 */

#ifndef _MT_GEMLIB_H_
# define _MT_GEMLIB_H_

# include <compiler.h>

#ifndef INT16  /* keep compatibility with Mgemlib */
#define INT16 short  /**< 16 bits signed integer */
#endif

# ifdef __GEMLIB_OLDBIND		/* Backward-compatibility */
#  undef _MT_GEMLIB_H_		/* For old bindings, these header had to be multi-included. */

#  ifndef __GEMLIB_HAVE_DEFS	/* first include via aesbind/vdibind/gemfast */
#   define __GEMLIB_HAVE_DEFS
#  else
#   undef __GEMLIB_DEFS
#  endif

# else				/* New include scheme: one header defines all */
#  define __GEMLIB_DEFS  /**< told the compiler to parse both AES and VDI definitions */
#  ifndef _GEM_VDI_P_
#   define __GEMLIB_AES  /**< told the compiler to parse AES prototypes */
#  endif
#  ifndef _GEM_AES_P_
#   define __GEMLIB_VDI  /**< told the compiler to parse VDI prototypes */
#  endif
# endif

__BEGIN_DECLS


/*******************************************************************************
 * The version of the gem-lib
 */

/* Major and minor version number of the GEMLib package.  Use these macros
   to test for features in specific releases.  */
#define __GEMLIB__		__GEMLIB_MAJOR__ 		/**< tell the world this is GEMLIB */
#define	__GEMLIB_MAJOR__	0				/**< MAJOR version number of gemlib */
#define	__GEMLIB_MINOR__	44				/**< MINOR version number of gemlib */
#define __GEMLIB_REVISION__	0				/**< REVISION version number of gemlib */
#define __GEMLIB_BETATAG__	""				/**< BETATAG of gemlib */

/* the other name of this release is MGEMLIB 44 */
#define MGEMLIB			44						/**< this gemlib is compatible with MGEMLIB */
#define __MGEMLIB__		44						/**< another identifier for MGEMLIB */


#ifdef __GEMLIB_DEFS

/*******************************************************************************
 * The AES specific stuff from old gemfast.h
 */

#define NIL 			(-1)					/**< Value for no more object in object tree */
#define DESKTOP_HANDLE		0					/**< TODO */
#define DESK			DESKTOP_HANDLE		/**< TODO */

/* mt_appl_control() mode */
#define APC_SYSTEM		 2
#define APC_HIDE		10  /**< Hide application -- see mt_appl_control() */
#define APC_SHOW		11  /**< Show application -- see mt_appl_control() */
#define APC_TOP 		12  /**< Bring application to front -- see mt_appl_control() */
#define APC_HIDENOT 		13  /**< Hide all applications except the one referred to by ap_cid -- see mt_appl_control() */
#define APC_INFO		14  /**< Get the application parameter -- see mt_appl_control() */
#define APC_MENU		15  /**< The last used menu tree is returned -- see mt_appl_control() */
#define APC_WIDGETS		16  /**< Inquires or sets the 'default' positions of the window widgets -- see mt_appl_control() */
#define APC_APP_CONFIG		17  /**< Change some way to manage application by AES most of them can be already set in configuration file -- see mt_appl_control() */
#define APC_INFORM_MESAG	18  /**< Request/Remove the sent an user Unix Signal to application when AES message is available -- see mt_appl_control() */

/* APC_INFO bits */
#define APCI_HIDDEN		0x01  /**< the application is hidden -- subopcode for #APC_INFO */
#define APCI_HASMBAR		0x02  /**< the application has a menu bar -- subopcode for #APC_INFO */
#define APCI_HASDESK		0x04  /**< the application has a own desk -- subopcode for #APC_INFO */

/* appl_getinfo modes */
#define AES_LARGEFONT		0	/**< see  mt_appl_getinfo() */
#define AES_SMALLFONT		1	/**< see  mt_appl_getinfo() */
#define AES_SYSTEM		2	/**< see  mt_appl_getinfo() */
#define AES_LANGUAGE 		3	/**< see  mt_appl_getinfo() */
#define AES_PROCESS 		4	/**< see  mt_appl_getinfo() */
#define AES_PCGEM		5	/**< see  mt_appl_getinfo() */
#define AES_INQUIRE 		6	/**< see  mt_appl_getinfo() */
#define AES_WDIALOG 		7	/**< see  mt_appl_getinfo() */
#define AES_MOUSE		8	/**< see  mt_appl_getinfo() */
#define AES_MENU		9	/**< see  mt_appl_getinfo() */
#define AES_SHELL		10	/**< see  mt_appl_getinfo() */
#define AES_WINDOW		11	/**< see  mt_appl_getinfo() */
#define AES_MESSAGE		12	/**< see  mt_appl_getinfo() */
#define AES_OBJECT		13	/**< see  mt_appl_getinfo() */
#define AES_FORM		14	/**< see  mt_appl_getinfo() */
#define AES_EXTENDED		64	/**< see  mt_appl_getinfo() */
#define AES_NAES		65	/**< see  mt_appl_getinfo() */
#define AES_VERSION         	96  	/**< see  mt_appl_getinfo() and  mt_appl_getinfo_str() */
#define AES_WOPTS           	97  	/**< see  mt_appl_getinfo() */
#define AES_WFORM		98	/**< see  mt_appl_getinfo() */
#define AES_FUNCTIONS			AES_WFORM	/**< see  mt_appl_getinfo() */
#define AES_AOPTS		99	/**< see  mt_appl_getinfo() */
#define AES_APPL_OPTION		AES_AOPTS	/**< see  mt_appl_getinfo() */
#define AES_WINX		22360	/**< AES WINX information, see  mt_appl_getinfo() */

/* appl_getinfo return values (AES_LARGEFONT, AES_SMALLFONT) */
#define SYSTEM_FONT		0	/**< see  mt_appl_getinfo() */
#define OUTLINE_FONT 		1	/**< see  mt_appl_getinfo() */

/* appl_getinfo return values (AES_LANGUAGE) */
#define AESLANG_ENGLISH		0	/**< see  mt_appl_getinfo() */
#define AESLANG_GERMAN		1	/**< see  mt_appl_getinfo() */
#define AESLANG_FRENCH		2	/**< see  mt_appl_getinfo() */
#define AESLANG_SPANISH 	4	/**< see  mt_appl_getinfo() */
#define AESLANG_ITALIAN 	5	/**< see  mt_appl_getinfo() */
#define AESLANG_SWEDISH 	6	/**< see  mt_appl_getinfo() */

/* appl_getinfo(AES_VERSION) return values */
#define AES_DEVSTATUS_ALPHA		0	/**< see  mt_appl_getinfo() */
#define AES_DEVSTATUS_BETA		1	/**< see  mt_appl_getinfo() */
#define AES_DEVSTATUS_RELEASE	2	/**< see  mt_appl_getinfo() */
#define AES_FDEVSTATUS_STABLE	0x100	/**< see  mt_appl_getinfo() */

#define AES_ARCH_M68000	    0	/**< see  mt_appl_getinfo() */
#define AES_ARCH_M68010     1	/**< see  mt_appl_getinfo() */
#define AES_ARCH_M68020     2	/**< see  mt_appl_getinfo() */
#define AES_ARCH_M68030    	3	/**< see  mt_appl_getinfo() */
#define AES_ARCH_M68040    	4	/**< see  mt_appl_getinfo() */
#define AES_ARCH_M68060  	5	/**< see  mt_appl_getinfo() */
#define AES_ARCH_M6802060	6	/**< see  mt_appl_getinfo() */
#define AES_ARCH_COLDFIRE	7	/**< see  mt_appl_getinfo() */

/* appl_getinfo(AES_FUNCTIONS) return values */
#define AGI_WFORM			1	/**< see  mt_appl_getinfo() */
#define AGI_AOPTS			2	/**< see  mt_appl_getinfo() */

/* appl_options(AES_AOPTS) return values and appl_options() settings */
#define AO0_WF_SLIDE		1	/**< see  mt_appl_options() */
#define AO0_OBJC_EDIT		2	/**< see  mt_appl_options() */


/* appl_read modes */
#define APR_NOWAIT		-1	/**< Do not wait for message -- see mt_appl_read() */

/* appl_search modes */
#define APP_FIRST 		0	/**< see mt_appl_search() */
#define APP_NEXT		1	/**< see mt_appl_search() */
#define APP_DESK		2	/**< see mt_appl_search() */

/* appl_search return values*/
#define APP_SYSTEM			0x01	/**< see mt_appl_search() */
#define APP_APPLICATION		0x02	/**< see mt_appl_search() */
#define APP_ACCESSORY		0x04	/**< see mt_appl_search() */
#define APP_SHELL 			0x08	/**< see mt_appl_search() */
#define APP_AESSYS			0x10	/**< see mt_appl_search() */
#define APP_AESTHREAD		0x20	/**< see mt_appl_search() */

/* appl_trecord types */
#define APPEVNT_TIMER	 	0	/**< see struct pEvntrec */
#define APPEVNT_BUTTON	 	1	/**< see struct pEvntrec */
#define APPEVNT_MOUSE	 	2	/**< see struct pEvntrec */
#define APPEVNT_KEYBOARD 	3	/**< see struct pEvntrec */

/** struct used by mt_appl_trecord() and mt_appl_tplay()
 *
 * \a ap_event defines the required interpretation of \a ap_value
 *  as follows:
 *  <table>
 *  <tr><td>\a ap_event <td> \a ap_value
 *  <tr><td> #APPEVNT_TIMER (0) <td> Elapsed Time (in milliseconds)
 *  <tr><td> #APPEVNT_BUTTON (1) <td> low word  = state (1 = down), high word = # of clicks
 *  <tr><td> #APPEVNT_MOUSE (2) <td> low word  = X pos, high word = Y pos
 *  <tr><td> #APPEVNT_KEYBOARD (3) <td> bits 0-7 = ASCII code, bits 8-15 = scan code, bits 16-31 = shift key
 *
 *  Please read documentation of mt_appl_trecord() and mt_appl_tplay() for more details and
 *  known bugs related to this structure.
 */
typedef struct pEvntrec
{
	long ap_event;		/**< one of the APPEVNT_XXX constant */
	long ap_value;		/**< kind of data depends on \a ap_event */
} EVNTREC;

/* evnt_button flags */
#define LEFT_BUTTON		0x0001	/**< mask for left mouse button, see mt_evnt_button() */
#define RIGHT_BUTTON 	0x0002	/**< mask for right mouse button, see mt_evnt_button() */
#define MIDDLE_BUTTON	0x0004	/**< mask for middle mouse button, see mt_evnt_button() */

#define K_RSHIFT		0x0001	/**< mask for right shift key, see mt_evnt_button() */
#define K_LSHIFT		0x0002	/**< mask for left shift key, see mt_evnt_button() */
#define K_CTRL 			0x0004	/**< mask for control key, see mt_evnt_button() */
#define K_ALT			0x0008	/**< mask for alternate key, see mt_evnt_button() */

/* evnt_dclick flags */
#define EDC_INQUIRE		0	/**< inquire double-clic rate, see mt_evnt_dclick() */
#define EDC_SET			1	/**< set double-clic rate, see mt_evnt_dclick() */

/* event message values */

/** message received when a menu entry has been selected
 *
 * - msg[0] = #MN_SELECTED
 * - msg[1] = AES application ID of the sender
 * - msg[2] = 0
 * - msg[3] = title menu index
 * - msg[4] = item menu index
 * - msg[5] = address of the menu root object (most significant bits)
 * - msg[6] = address of the menu root object (less significant bits)
 * - msg[7] = index of object parent's selected item
 *
 *  Values of word 5, 6 and 7 are an extension of AES 3.3. This feature could
 *  be check by calling mt_appl_getinfo() with the parameter #AES_MENU.
 *
 */
#define MN_SELECTED	 	10

/** message received when a portion of the screen has to be redrawn
 *
 * - msg[0] = #WM_REDRAW
 * - msg[1] = AES application ID of the sender
 * - msg[2] = 0
 * - msg[3] = handle of the window to redraw
 * - msg[4] = x,
 * - msg[5] = y,
 * - msg[6] = w,
 * - msg[7] = h: rectangle of the area to be redrawn.
 *
 *  When the message is received the window contents should be drawn (or a
 *  representative icon if the window is iconified).
 * 
 */
#define WM_REDRAW 	 	20

/** message received when a window which is not the top window has been clicked on
 *  by the user.
 *
 * - msg[0] = #WM_TOPPED
 * - msg[1] = AES application ID of the sender
 * - msg[2] = 0
 * - msg[3] = handle of the window 
 * - msg[4] = 
 * - msg[5] = 
 * - msg[6] = 
 * - msg[7] = 
 *
 *  When an application received this message, it should sets the window to the top
 *  by calling mt_wind_set() with #WF_TOP parameter.
 */
#define WM_TOPPED 	 	21

/** message received when the user clicks on a window's close box.
 *
 * - msg[0] = #WM_CLOSED
 * - msg[1] = AES application ID of the sender
 * - msg[2] = 0
 * - msg[3] = handle of the window 
 * - msg[4] = 
 * - msg[5] = 
 * - msg[6] = 
 * - msg[7] = 
 *
 *  When an application received this message, it should closes the window 
 *  by calling mt_wind_close()
 */
#define WM_CLOSED 	 	22

/** message received when the user clicks on a window's full box.
 *
 * - msg[0] = #WM_FULLED
 * - msg[1] = AES application ID of the sender
 * - msg[2] = 0
 * - msg[3] = handle of the window 
 * - msg[4] = 
 * - msg[5] = 
 * - msg[6] = 
 * - msg[7] = 
 *
 *  When an application received this message, if the window is not at
 *  full size, the window should ne resized using mt_wind_set() and #WF_CURRXYWH
 *  to occupy the entire screen area minus the menu bar (see mt_wind_get()).
 *  If the window was previously 'fulled' and has not been resized since, the
 *  application should return the window to its previous size.
 */
#define WM_FULLED 	 	23

/** message received when the user clicks on one of the slider gadgets of a window.
 *
 * - msg[0] = #WM_ARROWED
 * - msg[1] = AES application ID of the sender
 * - msg[2] = 0
 * - msg[3] = handle of the window 
 * - msg[4] = one of the following operation code:
 *            - #WA_UPPAGE ( Page Up )
 *            - #WA_DNPAGE ( Page Down )
 *            - #WA_UPLINE ( Row Up )
 *            - #WA_DNLINE ( Row Down )
 *            - #WA_LFPAGE ( Page Left )
 *            - #WA_RTPAGE ( Page Right )
 *            - #WA_LFLINE ( Column Left )
 *            - #WA_RTLINE ( Column Right )
 * - msg[5] = either 'MW' (0x4d57) on the first wheel click
 *            or 'Mw' (0x4d77) on subsequent wheel clicks.
 * - msg[6] = 0
 * - msg[7] = original wheel amount
 *
 *  msg[5] to msg[7] are only available since XaAES with mouse wheel support. These
 *  fields are filled by the AES when the AES automatically transforms mouse wheel
 *  event (that appears on a window with arrow widget) to arrow messages.
 *
 *  A row or column message is sent when a slider arrow is selected. A 'page'
 *  message is sent when a darkened area of the scroll bar is clicked. This usually
 *  indicates that the application should adjust the window's contents by a larger
 *  amount than with the row or column messages.
 *
 *  If the application manage itself wheel event on a particular window (see
 *  mt_wind_set()  with #WF_WHEEL parameter and #WHEEL_ARROWED mode), then on mouse
 *  wheel events, the window receives the following message:
 *
 * - msg[0] = #WM_ARROWED
 * - msg[1] = AES application ID of the sender
 * - msg[2] = 0
 * - msg[3] = handle of the window 
 * - msg[4] = #WA_WHEEL
 * - msg[5] = 0
 * - msg[6] = wheel number
 * - msg[7] = wheel amount
 *
 */
#define WM_ARROWED	 	24

/** message received when the user moves the horizontal slider of a window.
 *
 * - msg[0] = #WM_HSLID
 * - msg[1] = AES application ID of the sender
 * - msg[2] = 0
 * - msg[3] = handle of the window 
 * - msg[4] = new slider position from 0 to 1000.
 * - msg[5] = 
 * - msg[6] = 
 * - msg[7] = 
 *
 *  Note: Slider position is relative and not related to slider size.
 */
#define WM_HSLID		25

/** message received when the user moves the vertical slider of a window.
 *
 * - msg[0] = #WM_VSLID
 * - msg[1] = AES application ID of the sender
 * - msg[2] = 0
 * - msg[3] = handle of the window 
 * - msg[4] = new slider position from 0 to 1000.
 * - msg[5] = 
 * - msg[6] = 
 * - msg[7] = 
 *
 *  Note: Slider position is relative and not related to slider size.
 */
#define WM_VSLID		26

/** message received when the user drags the window sizing gadget.
 *
 * - msg[0] = #WM_SIZED
 * - msg[1] = AES application ID of the sender
 * - msg[2] = 0
 * - msg[3] = handle of the window 
 * - msg[4] = x
 * - msg[5] = y
 * - msg[6] = w
 * - msg[7] = h of the new window location
 *
 *  Use mt_wind_set() with the command #WF_CURRXYWH to change the size
 *  of the window. 
 *
 *  #WM_SIZED and #WM_MOVED usually share common handling code.
 *
 *  Affected by WCOWORK mode.
 */
#define WM_SIZED		27

/** message received when the user moves the window by dragging the
 *  window title bar.
 *
 * - msg[0] = #WM_MOVED
 * - msg[1] = AES application ID of the sender
 * - msg[2] = 0
 * - msg[3] = handle of the window 
 * - msg[4] = x
 * - msg[5] = y
 * - msg[6] = w
 * - msg[7] = h of the new window location
 *
 *  Use mt_wind_set() with the command #WF_CURRXYWH to change the size
 *  of the window. 
 *
 *  #WM_SIZED and #WM_MOVED usually share common handling code.
 *
 *  Affected by WCOWORK mode.
 */
#define WM_MOVED		28

/** message received when the user has topped a window
 *
 * - msg[0] = #WM_NEWTOP
 * - msg[1] = AES application ID of the sender
 * - msg[2] = 0
 * - msg[3] = handle of the window 
 * - msg[4] = 
 * - msg[5] = 
 * - msg[6] = 
 * - msg[7] = 
 *
 */
#define WM_NEWTOP		29

/** message received when the window is sent behind one or more windows
 *  as the result of another window being topped.
 *
 * - msg[0] = #WM_UNTOPPED
 * - msg[1] = AES application ID of the sender
 * - msg[2] = 0
 * - msg[3] = handle of the window 
 * - msg[4] = 
 * - msg[5] = 
 * - msg[6] = 
 * - msg[7] = 
 *
 *  The application need take no action. The message is for informational
 *  use only.
 *
 *  @since AES version 4.0.  The availability of this feature could
 *  be check by calling mt_appl_getinfo() with the parameter #AES_MESSAGE.
 */
#define WM_UNTOPPED		30

/** message received when the window is brought to the front on a multitasking
*   AES.
 *
 * - msg[0] = #WM_ONTOP
 * - msg[1] = AES application ID of the sender
 * - msg[2] = 0
 * - msg[3] = handle of the window 
 * - msg[4] = 
 * - msg[5] = 
 * - msg[6] = 
 * - msg[7] = 
 *
 *  The application need take no action. The message is for informational
 *  use only.
 *
 *  @since AES version 4.0.  The availability of this feature could
 *  be check by calling mt_appl_getinfo() with the parameter #AES_MESSAGE.
 */
#define WM_ONTOP		31

/** message received when the user act on the window to sent it to the bottom
 *  of the window stack.
 *
 * - msg[0] = #WM_BOTTOM
 * - msg[1] = AES application ID of the sender
 * - msg[2] = 0
 * - msg[3] = handle of the window 
 * - msg[4] = 
 * - msg[5] = 
 * - msg[6] = 
 * - msg[7] = 
 *
 *  The application should then sent the window to the bottom of the windows stack
 *  by calling mt_wind_set() with #WF_BOTTOM command.
 *
 *  Note: the way the user may send a window to the bottom depend on the AES and its
 *  configuration.
 *
 *  @since AES version 4.1.  The availability of this feature could
 *  be check by calling mt_appl_getinfo() with the parameter #AES_MESSAGE.
 *
 *  Affected by WCOWORK mode.
 */
#define WM_BOTTOM		33
#define WM_BOTTOMED		WM_BOTTOM	/**< see #WM_BOTTOM */

/** message received when the user clicks on the #SMALLER window gadget
 *
 * - msg[0] = #WM_ICONIFY
 * - msg[1] = AES application ID of the sender
 * - msg[2] = 0
 * - msg[3] = handle of the window 
 * - msg[4] = x
 * - msg[5] = y
 * - msg[6] = w
 * - msg[7] = h : position of the iconified window
 *
 *  The application should then iconify the window
 *  by calling mt_wind_set() with #WF_ICONIFY command.
 *
 *  @since AES version 4.1.  The availability of this feature could
 *  be check by calling mt_appl_getinfo() with the parameter #AES_MESSAGE.
 *
 *  Affected by WCOWORK mode.
 */
#define WM_ICONIFY		34

/** message received when the user double-clicks on the iconified window
 *
 * - msg[0] = #WM_UNICONIFY
 * - msg[1] = AES application ID of the sender
 * - msg[2] = 0
 * - msg[3] = handle of the window 
 * - msg[4] = x
 * - msg[5] = y
 * - msg[6] = w
 * - msg[7] = h : position of the iconified window
 *
 *  The application should then uniconify the window
 *  by calling mt_wind_set() with #WF_UNICONIFY command.
 *
 *  @since AES version 4.1.  The availability of this feature could
 *  be check by calling mt_appl_getinfo() with the parameter #AES_MESSAGE.
 *
 *  Affected by WCOWORK mode.
 */
#define WM_UNICONIFY		35

/** message received when the user clicks on the #SMALLER window gadget while
 *  the CONTROL key is pressed.
 *
 * - msg[0] = #WM_ALLICONIFY
 * - msg[1] = AES application ID of the sender
 * - msg[2] = 0
 * - msg[3] = handle of the window 
 * - msg[4] = x
 * - msg[5] = y
 * - msg[6] = w
 * - msg[7] = h : position of the iconified window
 *
 *  The application should then close all opened windows, and open a new iconified
 *  window at the position indicated which represents the application.
 *
 *  @since AES version 4.1.  The availability of this feature could
 *  be check by calling mt_appl_getinfo() with the parameter #AES_MESSAGE.
 *
 *  Affected by WCOWORK mode.
 */
#define WM_ALLICONIFY		36

/** message received when the user clicks on a toolbar object
 *
 * - msg[0] = #WM_TOOLBAR
 * - msg[1] = AES application ID of the sender
 * - msg[2] = 0
 * - msg[3] = handle of the window 
 * - msg[4] = index of the object clicked
 * - msg[5] = number of clicks
 * - msg[6] = state of the keyboard shift keys (see mt_evnt_keybd())
 * - msg[7] = 
 *
 *  @since AES version 4.1.  The availability of this feature could
 *  be check by calling mt_appl_getinfo() with the parameter #AES_MENU (BUG?FIXME!).
 */
#define WM_TOOLBAR		37

/** Message received when the user resizes the window using the
 *  left/upper parts of the window border.
 *
 * - msg[0] = #WM_REPOSED
 * - msg[1] = AES application ID of the sender
 * - msg[2] = 0
 * - msg[3] = handle of the window 
 * - msg[4] = x
 * - msg[5] = y
 * - msg[6] = w
 * - msg[7] = h of the new window location
 *  
 *  See mt_wind_set() with command #WF_OPTS, bit #WO0_SENDREPOS for
 *  details about the availability of this message.
 *
 *  Use mt_wind_set() with the command #WF_CURRXYWH to change the position
 *  of the window. 
 *
 *  Affected by WCOWORK mode.
 */
#define WM_REPOSED		38

/** message received when the user has selected a desk accessory to open
 *
 * - msg[0] = #AC_OPEN
 * - msg[1] = AES application ID of the sender
 * - msg[2] = 0
 * - msg[3] =  
 * - msg[4] = menu ID of the accessory to open
 * - msg[5] = 
 * - msg[6] = 
 * - msg[7] = 
 *
 *  The menu ID is the value returned by mt_menu_register()
 */
#define AC_OPEN			40

/** message received by a desk accessory when the accessory
 *  should be closed.
 *
 * - msg[0] = #AC_CLOSE
 * - msg[1] = AES application ID of the sender
 * - msg[2] = 0
 * - msg[3] = menu ID of the accessory to close.
 * - msg[4] = 
 * - msg[5] = 
 * - msg[6] = 
 * - msg[7] = 
 *
 *  The menu ID is the value returned by mt_menu_register()
 *
 *  Do not close any windows your accessory had open, the system will do this for you.
 *  Also, do not require any feedback from the user when this is received. Treat this
 *  message as a 'Cancel' from the user.
 */
#define AC_CLOSE		41

/** message received when the system requests that the application
 *  terminate.
 *
 * - msg[0] = #AP_TERM
 * - msg[1] = AES application ID of the sender
 * - msg[2] = 0
 * - msg[3] = 
 * - msg[4] = 
 * - msg[5] = reason for the shut down:
 *            - #AP_TERM (just shut down)
 *            - #AP_RESCHG (resolution change)
 * - msg[6] = 
 * - msg[7] = 
 *
 *  This message is usually the result of a resolution change, but it may
 *  laso occur if another application sends this message to gain total
 *  control of the system.
 *
 *  The application should shutdown immediately after closing windows,
 *  freeing resources, etc... If for some reason your application cannot
 *  shutdown, you must inform the AES by sending an #AP_TFAIL message
 *  by using mt_shel_write() mode #SWM_AESMSG.
 *
 *  Note: Desk accessory will always be sent #AC_CLOSE message, not
 *  #AP_TERM
 *
 *  @since AES version 4.0.  The availability of this feature could
 *  be check by calling mt_appl_getinfo() with the parameter #AES_MESSAGE.
 */
#define AP_TERM			50

/** message sent to the system when the application has received an
 *  #AP_TERM message and cannot shutdown.
 *
 * - msg[0] = #AP_TFAIL
 * - msg[1] = application error code
 * - msg[2] = 0
 * - msg[3] = 
 * - msg[4] = 
 * - msg[5] = 
 * - msg[6] = 
 * - msg[7] = 
 */
#define AP_TFAIL		51

#define AP_RESCHG 		57  /**< indicates a resolution change, see #AP_TERM */

/* Xcontrol messages */
#define CT_UPDATE		50	/**< TODO */
#define CT_MOVE			51	/**< TODO */
#define CT_NEWTOP		52	/**< TODO */
#define CT_KEY			53	/**< TODO */

/** message received by the application which requested a shutdown, when the
 *  shutdown is complete and was successful.
 *
 * - msg[0] = #SHUT_COMPLETED
 * - msg[1] = AES application ID of the sender
 * - msg[2] = 0
 * - msg[3] = 
 * - msg[4] = 
 * - msg[5] = 
 * - msg[6] = 
 * - msg[7] = 
 *
 *  @since AES version 4.0.  The availability of this feature could
 *  be check by calling mt_appl_getinfo() with the parameter #AES_MESSAGE.
 */
#define SHUT_COMPLETED		60

/** message received by the application which requested a resolution change,
 *  when the resolution change is complete.
 *
 * - msg[0] = #RESCHG_COMPLETED
 * - msg[1] = AES application ID of the sender
 * - msg[2] = 0
 * - msg[3] = indicate the success of the resolution change:
 *            - 1 if the resolution change was succesful
 *            - 0 if an error occured.
 * - msg[4] = 
 * - msg[5] = 
 * - msg[6] = 
 * - msg[7] = 
 *
 *
 *  @since AES version 4.0.  The availability of this feature could
 *  be check by calling mt_appl_getinfo() with the parameter #AES_MESSAGE.
 */
#define RESCHG_COMPLETED	61

/** see #RESCHG_COMPLETED */
#define RESCH_COMPLETED		RESCHG_COMPLETED

/** message received when another apllication wishes to initiate a
 *  drag and drop session.
 *
 * - msg[0] = #AP_DRAGDROP
 * - msg[1] = AES application ID of the sender
 * - msg[2] = 0
 * - msg[3] = handle of the window which had an object dropped on,
 *            or -1 if no specific window was targeted
 * - msg[4] = x
 * - msg[5] = y position of the mouse when the object was dropped
 * - msg[6] = keybord shift state at the time of the drop as
 *            in mt_evnt_keybd()
 * - msg[7] = two bytes ASCII packed pipe identifier which gives the
 *            file extension of the pipe to open.
 *
 *  TODO: add more informations about drag 'n drop protocol...
 */
#define AP_DRAGDROP 		63

/** message sent to the desktop to ask it to update an open drive
 *  window.
 *
 * - msg[0] = #SH_WDRAW
 * - msg[1] = AES application ID of the sender
 * - msg[2] = 0
 * - msg[3] = drive number to update (0=A, 1=B, etc...) or -1
 *            to update all windows
 * - msg[4] = 
 * - msg[5] = 
 * - msg[6] = 
 * - msg[7] =
 */
#define SH_WDRAW			72

#define SC_CHANGED			80	/**< TODO */
#define PRN_CHANGED 		82	/**< TODO */
#define FNT_CHANGED 		83	/**< TODO */

/** message received when a child thread that this application has
 *  started returns.
 *
 * - msg[0] = #THR_EXIT
 * - msg[1] = AES application ID of the sender
 * - msg[2] = 0
 * - msg[3] = application identifier of the terminated thread
 * - msg[4] = exit code of the thread (HIGH WORD)
 * - msg[5] = exit code of the thread (LOW WORD)
 * - msg[6] = 
 * - msg[7] =
 *
 *  Warning: the error code is a LONG (32 bits) value !
 *
 *  @since MagiC (?).
 */
#define THR_EXIT			88

#define PA_EXIT 			89	/**< TODO */

/** message received when a child process that this application has
 *  started returns.
 *
 * - msg[0] = #CH_EXIT
 * - msg[1] = AES application ID of the sender
 * - msg[2] = 0
 * - msg[3] = child's application identifier
 * - msg[4] = exit code of the child application
 * - msg[5] = 
 * - msg[6] = 
 * - msg[7] =
 *
 *  @since AES version 4.0.  The availability of this feature could
 *  be check by calling mt_appl_getinfo() with the parameter #AES_MESSAGE.
 */
#define CH_EXIT 			90

#define WM_M_BDROPPED		100		/* KAOS 1.4  */	/**< TODO */

/** MAG!X screen manager extension
 *
 *	Applications may send such message to the MagiC Screen Manager
 *	(SCREENMGR, application ID = 1) to control AES applications.
 *	The message is [SM_M_SPECIAL apid 0 0 'MA' 'GX' SMC_XXX 0]
 *	where SMC_XXX contains the action SCREENMGR has to perform.
 *
 *	SMC_XXX may be one of the following value:
 *	-  #SMC_TIDY_UP	
 *	-  #SMC_TERMINATE 
 *	-  #SMC_SWITCH	 
 *	-  #SMC_FREEZE	 
 *	-  #SMC_UNFREEZE  
 *	-  #SMC_RES5 	 
 *	-  #SMC_UNHIDEALL 
 *	-  #SMC_HIDEOTHERS
 *	-  #SMC_HIDEACT
 *
 *  @sa mt_appl_control()
 */
#define SM_M_SPECIAL		101
#define SM_M_RES2			102		/**< MAG!X screen manager extension */
#define SM_M_RES3			103		/**< MAG!X screen manager extension */
#define SM_M_RES4			104		/**< MAG!X screen manager extension */
#define SM_M_RES5			105		/**< MAG!X screen manager extension */
#define SM_M_RES6			106		/**< MAG!X screen manager extension */
#define SM_M_RES7			107		/**< MAG!X screen manager extension */
#define SM_M_RES8			108		/**< MAG!X screen manager extension */
#define SM_M_RES9			109		/**< MAG!X screen manager extension */
#define WM_SHADED			22360	/**< TODO [WM_SHADED apid 0 win 0 0 0 0]   */
#define WM_UNSHADED 		22361	/**< TODO [WM_UNSHADED apid 0 win 0 0 0 0] */

/* SM_M_SPECIAL sub-opcodes
	used:
	INT16 msg[8];

	msg[0] = SM_M_SPECIAL;	 101
	msg[1] = ap_id;	 ap_id, with SMC_TIDY_UP and SMC_UNHIDEALL own
	msg[2] = 0;		 No overrun
	msg[3] = 0;
	msg[4] = 'MA';
	msg[5] = 'GX';
	msg[6] = ..subcode..;
	msg[7] = 0;
	appl_write(msg, 16, 1);		 apid #1 is the SCREENMGR
*/
#define SMC_TIDY_UP			0    /* MagiC 2  */ 	 /**< TODO */
#define SMC_TERMINATE		1    /* MagiC 2  */ 	 /**< TODO */
#define SMC_SWITCH			2    /* MagiC 2  */ 	 /**< TODO */
#define SMC_FREEZE			3    /* MagiC 2  */ 	 /**< TODO */
#define SMC_UNFREEZE		4    /* MagiC 2  */ 	 /**< TODO */
#define SMC_RES5			5    /* MagiC 2  */ 	 /**< TODO */
#define SMC_UNHIDEALL		6    /* MagiC 3.1	*/   /**< TODO */
#define SMC_HIDEOTHERS		7    /* MagiC 3.1	*/   /**< TODO */
#define SMC_HIDEACT			8    /* MagiC 3.1	*/   /**< TODO */

/** Message received when an event occurs on mouse wheel.
 *
 * - msg[0] = #WM_WHEEL
 * - msg[1] = AES application ID of the sender
 * - msg[2] = 0
 * - msg[3] = handle of the window 
 * - msg[4] = x-position of the mouse
 * - msg[5] = y-position of the mouse
 * - msg[6] = keyboard shift status
 * - msg[7] = wheel info
 *
 * 'wheel info' consists of the following data;
 * - bits 0-7    - Wheel amount
 * - bits 8-11   - orient
 * - bits 12-15  - Wheel ID
 *
 * 'orient' is a bitmask where bit 0 indicates direction, and
 * bit 1 indicates orientation;
 *  - bit 0  :  0 = up or left; 1 = down or right
 *  - bit 1  :  0 = vertical (bit 0 indicates UP/DOWN); 1 = horizontal (bit 0 indicates LEFT/RIGHT)
 *
 * 'wheel ID' contains the wheel number. 0 is wheel #1, etc..
 * 'wheel amount' holds the number of wheel-turns.
 *  
 *  See mt_wind_set() with command #WF_WHEEL for
 *  details about the availability of this message.
 *
 */
#define WM_WHEEL		345

/* evnt_mouse modes */
#define MO_ENTER		0	/**< Wait for mouse to enter rectangle, see mt_evnt_mouse() */
#define MO_LEAVE		1	/**< Wait for mouse to leave rectangle, see mt_evnt_mouse() */

/* evnt_multi flags */
#define MU_KEYBD			0x0001	/**< Wait for a user keypress, see mt_evnt_multi() */
#define MU_BUTTON			0x0002	/**< Wait for the specified mouse button state, see mt_evnt_multi() */
#define MU_M1				0x0004	/**< Wait for a mouse/rectangle event as specified, see mt_evnt_multi() */
#define MU_M2				0x0008	/**< Wait for a mouse/rectangle event as specified, see mt_evnt_multi()  */
#define MU_MESAG			0x0010	/**< Wait for a message, see mt_evnt_multi() */
#define MU_TIMER			0x0020	/**< Wait the specified amount of time, see mt_evnt_multi() */
#define MU_WHEEL			0x0040	/**< TODO (XaAES) */
#define MU_MX				0x0080	/**< TODO (XaAES) */
#define MU_NORM_KEYBD		0x0100	/**< TODO (XaAES) */
#define MU_DYNAMIC_KEYBD	0x0200	/**< TODO (XaAES) */

/* constants for form_alert */
#define FA_NOICON   "[0]"	/**< display no icon, see mt_form_alert() */
#define FA_ERROR    "[1]"	/**< display Exclamation icon, see mt_form_alert() */
#define FA_QUESTION "[2]"	/**< display Question icon, see mt_form_alert() */
#define FA_STOP     "[3]"	/**< display Stop icon, see mt_form_alert() */
#define FA_INFO     "[4]"	/**< display Info icon, see mt_form_alert() */
#define FA_DISK     "[5]"	/**< display Disk icon, see mt_form_alert() */

/* form_dial opcodes */
#define FMD_START 		0	/**< reserves the screen space for a dialog, see mt_form_dial() */
#define FMD_GROW		1	/**< draws an expanding box, see mt_form_dial() */
#define FMD_SHRINK		2	/**< draws a shrinking box, see mt_form_dial()  */
#define FMD_FINISH		3	/**< releases the screen space for a dialog, see mt_form_dial() */

/* form_error modes */
#define FERR_FILENOTFOUND	 2	/**< File Not Found (GEMDOS error -33), see mt_form_error() */
#define FERR_PATHNOTFOUND	 3	/**< Path Not Found (GEMDOS error -34), see mt_form_error() */
#define FERR_NOHANDLES		 4	/**< No More File Handles (GEMDOS error -35), see mt_form_error() */
#define FERR_ACCESSDENIED	 5	/**< Access Denied (GEMDOS error -36), see mt_form_error() */
#define FERR_LOWMEM			 8	/**< Insufficient Memory (GEMDOS error -39), see mt_form_error() */
#define FERR_BADENVIRON 	10	/**< Invalid Environment (GEMDOS error -41), see mt_form_error() */
#define FERR_BADFORMAT		11	/**< Invalid Format (GEMDOS error -42) */
#define FERR_BADDRIVE		15	/**< Invalid Drive Specification (GEMDOS error -46), see mt_form_error() */
#define FERR_DELETEDIR		16	/**< Attempt To Delete Working Directory (GEMDOS error -47), see mt_form_error() */
#define FERR_NOFILES 		18	/**< No More Files (GEMDOS error -49), see mt_form_error() */

/* fsel_(ex)input return values */
#define FSEL_CANCEL		 0	/**< the fileselector has been closed by using the CANCEL button, see mt_fsel_exinput() */
#define FSEL_OK			 1	/**< the fileselector has been closed by using the OK button, see mt_fsel_exinput()  */

/* menu_attach modes */
#define ME_INQUIRE		0	/**< inquire information on a sub-menu attached, see mt_menu_attach() */
#define ME_ATTACH 		1	/**< attach or change a sub-menu, see mt_menu_attach() */
#define ME_REMOVE 		2	/**< remove a sub-menu. see mt_menu_attach() */

/* menu_attach attributes */
#define SCROLL_NO 		0	/**< the menu will not scroll, see MENU::mn_scroll structure */
#define SCROLL_YES		1	/**< menu may scroll if it is too high, see MENU::mn_scroll structure  */

/* menu_bar modes */
#define MENU_INQUIRE		-1	/**< inquire the AES application ID of the process which own the displayed menu, see mt_menu_bar() */
#define MENU_REMOVE			0	/**< remove a menu bar, see mt_menu_bar() */
#define MENU_INSTALL		1	/**< install a menu bar, see mt_menu_bar() */
#define MENU_GETMODE		3	/**< Get the menu bar mode, see mt_menu_bar() */
#define MENU_SETMODE		4	/**< Set the menu bar mode, see mt_menu_bar() */
#define MENU_UPDATE 		5	/**< Update the system part of the menu bar, see mt_menu_bar() */
#define MENU_INSTL		  100	/**< Install a menu without switching the top application (Magic), see mt_menu_bar() */


/* MENU_GETMODE and MENU_SETMODE bits */
#define  MENU_HIDDEN		0x0001 /**< menu bar only visible when needed, see #MENU_GETMODE or #MENU_SETMODE */
#define  MENU_PULLDOWN		0x0002 /**< Pulldown-Menus, see #MENU_GETMODE or #MENU_SETMODE */
#define  MENU_SHADOWED		0x0004 /**< menu bar with shadows, see #MENU_GETMODE or #MENU_SETMODE */

/* menu_icheck modes */
#define UNCHECK			0	/**< remove the check mark of a menu item, see mt_menu_icheck() */
#define CHECK			1	/**< set a check mark of a menu item, see mt_menu_icheck() */

/* menu_ienable modes */
#define DISABLE			0	/**< disable a menu item, see mt_menu_ienable() */
#define ENABLE 			1	/**< enable a menu item, see mt_menu_ienable() */

/* menu_istart modes */
#define MIS_GETALIGN 		0	/**< get the alignment of a parent menu item with a sub-menu item, see mt_menu_istart() */
#define MIS_SETALIGN 		1	/**< set the alignment of a parent menu item with a sub-menu item, see mt_menu_istart() */

/* menu_popup modes */
#define SCROLL_LISTBOX		-1	/**< display a drop-down list (with slider) instead of popup menu, see MENU::mn_scroll */

/* menu_register modes */
#define REG_NEWNAME		-1	/**< register your application with a new name, see mt_menu_register() */

/* menu_settings modes */
#define MN_INQUIRE		0  /**< inquire the current menu settings, see mt_menu_settings() */
#define MN_CHANGE		1  /**< set the menu settings, see mt_menu_settings() */

/* menu_tnormal modes */
#define HIGHLIGHT		0	/**< display the title in reverse mode, see mt_menu_tnormal() */
#define UNHIGHLIGHT		1	/**< display the title in normal mode, see mt_menu_tnormal() */

/** menu_settings uses a new structure for setting and inquiring the submenu
    delay values and the menu scroll height.*/
typedef struct _mn_set
{
	long  display;		/**< the submenu display delay in milliseconds */
	long  drag;			/**< the submenu drag delay in milliseconds */
	long  delay;		/**< the single-click scroll delay in milliseconds */
	long  speed;		/**< the continuous scroll delay in milliseconds */
	short height; 		/**< the menu scroll height (in items) */
} MN_SET;

/* shel_get modes */
#define SHEL_BUFSIZE (-1)	/**< return the size of AES shell buffer, see mt_shel_read() */

/* shel_help mode */
#define SHP_HELP 0      /**< see mt_shel_help() */

/** structure for extended modes of mt_shel_write() */
typedef struct
{
	/** points to the filename formatted in the manner indicated
	 *  above
	 */
	char *newcmd;
	/** contains the maximum memory size available to the process
	 *  only used if the extended mode #SW_PSETLIMIT is set
	 */
	long psetlimit;
	/** contains the process priority of the process to launch
	 *  only used if the extended mode #SW_PRENICE is set
	 */
	long prenice;
	/** points to a character string containing the default directory
	 *  for the application begin launched.
	 *  only used if the extended mode #SW_DEFDIR is set
	 */
	char *defdir;
	/** points to a valid environment string for the process
	 *  only used if the extended mode #SW_ENVIRON is set
	 */
	char *env;
	/** New child's UID (user id)
	 *  only used if the extended mode #SW_UID is set
	 */
	short uid;
	/** New child's GID (group id)
	 *  only used if the extended mode #SW_GID is set
	 */
	short gid;
} SHELW;

/** similar to ::SHELW, with MagiC 6 only specificity, and without XaAES extensions */
typedef struct
{
	char	*command;	/**< see SHELW::newcmd */
	long	limit;		/**< see SHELW::psetlimit */
	long	nice;		/**< see SHELW::prenice */
	char	*defdir;	/**< see SHELW::defdir */
	char	*env;		/**< see SHELW::env */
	long	flags;		/**< since MagiC 6.  only used if the extended mode #SHW_XMDFLAGS is set */
} XSHW_COMMAND;

/** description of a thread, see mt_shel_write() with #SWM_THRCREATE mode */
typedef struct
{
	long			__CDECL (*proc)(void *par);	/**< entry point of the thread */
	/** user stack for the thread
	 *  If \a  user_stack = NULL, then the system creates the stack itself. If 
	 *  the thread terminates then the system releases the stack anyway.
	 */
	void			*user_stack;
	/** size of the user stack 
	 *  \a stacksize has to be specified in each case so that the system can set 
	 *  the stack pointer to the end of the thread. The supervisor stack is 
	 *  set by the system itself; its size cannot be influenced.
	 */
	unsigned long	stacksize;
	short			mode;						/**< always set to 0 (reserved) */
	long			res1;						/**< always set to 0 (reserved) */
} THREADINFO;

/** tail for default shell (MagiC), see mt_shel_write() */
typedef  struct
{
	short   dummy;		/**< a null  word 			   */ 
	long    magic;		/**< 'SHEL', if it's a Shell   */ 
	/** first call of the Shell 
	 *  If \a isfirst is set then the status is to be read from something like 
	 *  \c DESKTOP.INF, if \a isfirst is not set then one takes the temporary file or 
	 *  shell-buffer.
	 */
	short   isfirst;
	/** last error 
	 *  \a lasterr is the return value of the program running previously. If this 
	 *  was a GEM program then the error will already have been displayed in an 
	 *  alert box. It is well known that the longword is negative if the error
	 *  occurred with Pexec itself; a program return value always has the high
	 *  word 0.
	 */
	long    lasterr;
	short   wasgr;		/**< Program was a grafic app. */
} SHELTAIL;


/* shel_write modes */
/** Launch a GEM or TOS application or GEM desk accessory
 *  depending on the extension of the file. See mt_shel_write()
 */
#define SWM_LAUNCH		0
/** Launch a GEM or TOS application based on the value of \a wisgr, see mt_shel_write()
 */
#define SWM_LAUNCHNOW		1
#define SWM_LAUNCHACC		3	/**< Launch a GEM desk accessory, see mt_shel_write() */
#define SWM_SHUTDOWN		4	/**< Manipulate 'Shutdown' mode, see mt_shel_write() */
#define SWM_REZCHANGE		5	/**< Change screen resolution, see mt_shel_write() */
#define SWM_BROADCAST		7	/**< Broadcast an AES message to all processes, see mt_shel_write() */
#define SWM_ENVIRON			8	/**< Manipulate the AES environment, see mt_shel_write() */
/** Inform the AES of a new message the current application understands,
 *  see mt_shel_write()
 */
#define SWM_NEWMSG			9
#define SWM_AESMSG			10	/**< Send a message to the AES, see mt_shel_write() */
#define SWM_THRCREATE		20	/**< create a new thread, see mt_shel_write() */
#define SWM_THREXIT			21	/**< thread terminates itself, see mt_shel_write() */
#define SWM_THRKILL			22	/**< parent kills a thread, see mt_shel_write() */

/* other names for shel_write modes */
#define SHW_NOEXEC		SWM_LAUNCH		/**< alias */
#define SHW_EXEC		SWM_LAUNCHNOW	/**< alias */
#define SHW_EXEC_ACC	SWM_LAUNCHACC	/**< alias */
#define SHW_SHUTDOWN	SWM_SHUTDOWN	/**< alias */
#define SHW_RESCHNG		SWM_REZCHANGE	/**< alias */
#define SHW_BROADCAST	SWM_BROADCAST	/**< alias */
#define SHW_INFRECGN	SWM_NEWMSG		/**< alias */
#define SHW_AESSEND		SWM_AESMSG		/**< alias */
#define SHW_THR_CREATE	SWM_THRCREATE	/**< alias */
#define SHW_THR_EXIT	SWM_THREXIT		/**< alias */
#define SHW_THR_KILL	SWM_THRKILL		/**< alias */

/* shel_write, parameter wisgr */
#define TOSAPP			0  /**< application launched as TOS application, see mt_shel_write() */
#define GEMAPP			1  /**< application launched as GEM application, see mt_shel_write() */

/* shel_write, parameter wiscr when wodex=1 (SWM_LAUNCHNOW) */
#define SHW_IMMED		0	/**< unsupported (PC-GEM  2.x) */
#define SHW_CHAIN		1	/**< TOS way, see mt_shel_write() */
#define SHW_DOS			2	/**< unsupported (PC-GEM  2.x) */
#define SHW_PARALLEL		100	/**< create a new application to be ran in parallel, see mt_shel_write() */
#define SHW_SINGLE		101	/**< run an application in single mode (all other applications but apid 0 and 1 are frozen), see mt_shel_write() */

/* command line parser (shel_write: parameter "wiscr") */
#define CL_NORMAL		0	/**< command line passed normaly, see mt_shel_write() */
#define CL_PARSE		1	/**< command line passed in ARGV environment string, see mt_shel_write() */

/* shutdown action (shel_write: mode SWM_SHUTDOWN, parameter "wiscr") */
#define SD_ABORT		0		/**< Abort shutdown mode, see mt_shel_write() */
#define SD_PARTIAL		1		/**< Partial shutdown mode, see mt_shel_write() */
#define SD_COMPLETE		2		/**< Complete shutdown mode, see mt_shel_write() */

/* shel_write: mode SWM_ENVIRON, parameter 'wisgr' */
#define ENVIRON_SIZE		0	/**< returns the current size of the environment string, see mt_shel_write() */
#define ENVIRON_CHANGE		1	/**< modify an environment variable, see mt_shel_write() */
#define ENVIRON_COPY		2	/**< copy the evironment string in a buffer, see mt_shel_write() */

/* shel_write: mode SWM_NEWMSG, parameter 'wisgr' */
#define NM_APTERM		0x0001	/**< the application understands #AP_TERM messages, see mt_shel_write() and #SWM_NEWMSG */
#define NM_INHIBIT_HIDE		0x0002	/**< the application won't be hidden, see mt_shel_write() and #SWM_NEWMSG */

/* Werte fr Modus SWM_AESMSG (fr shel_write) */
#define AP_AESTERM		52     /* Mode 10: N.AES komplett terminieren. */	/**< TODO */

/* extended shel_write() modes */
#define SW_PSETLIMIT	0x0100	/**< Initial Psetlimit() , see SHELW::psetlimit */
#define SW_PRENICE		0x0200	/**< Initial Prenice() , see SHELW::prenice */
#define SW_DEFDIR 		0x0400	/**< Default Directory , see SHELW::defdir */
#define SW_ENVIRON		0x0800	/**< Environment , see SHELW::env */

/* XaAES extensions for shel_write() extended modes*/
#define SW_UID			0x1000	/**< Set user id of launched child, see SHELW::uid */
#define	SW_GID			0x2000	/**< Set group id of launched child, see SHELW::gid */

/* MagiC 6 extensions for shel_write() extended modes*/
#define SHW_XMDFLAGS		0x1000	/**< magiC 6 extension, see XSHW_COMMAND::flags*/

/* other names... */
#define SHW_XMDLIMIT	SW_PSETLIMIT	/**< alias */
#define SHW_XMDNICE		SW_PRENICE		/**< alias */
#define SHW_XMDDEFDIR	SW_DEFDIR		/**< alias */
#define SHW_XMDENV		SW_ENVIRON		/**< alias */

/* rsrc_gaddr structure types */
#define R_TREE			0	/**< Object tree, see mt_rsrc_gaddr() */
#define R_OBJECT		1	/**< Individual object, see mt_rsrc_gaddr() */
#define R_TEDINFO 		2	/**< TEDINFO structure, see mt_rsrc_gaddr() */
#define R_ICONBLK 		3	/**< ICONBLK structure, see mt_rsrc_gaddr() */
#define R_BITBLK		4	/**< BITBLK structure, see mt_rsrc_gaddr() */
#define R_STRING		5	/**< Free String data, see mt_rsrc_gaddr() */
#define R_IMAGEDATA		6	/**< Free Image data, see mt_rsrc_gaddr() */
#define R_OBSPEC		7	/**< ob_spec field within OBJECTs, see mt_rsrc_gaddr() */
#define R_TEPTEXT 		8	/**< te_ptext within TEDINFOs, see mt_rsrc_gaddr() */
#define R_TEPTMPLT		9	/**< te_ptmplt within TEDINFOs, see mt_rsrc_gaddr() */
#define R_TEPVALID		10	/**< te_pvalid within TEDINFOs, see mt_rsrc_gaddr() */
#define R_IBPMASK		11	/**< ib_pmask within ICONBLKs, see mt_rsrc_gaddr() */
#define R_IBPDATA		12	/**< ib_pdata within ICONBLKs, see mt_rsrc_gaddr() */
#define R_IBPTEXT		13	/**< ib_ptext within ICONBLKs, see mt_rsrc_gaddr() */
#define R_BIPDATA		14	/**< bi_pdata within BITBLKs, see mt_rsrc_gaddr() */
#define R_FRSTR			15	/**< Free string, see mt_rsrc_gaddr() */
#define R_FRIMG			16	/**< Free image, see mt_rsrc_gaddr() */

/* scrap_read return values */
#define SCRAP_CSV 		0x0001  /**< clipboard has a scrap.csv file, see mt_scrap_read() */
#define SCRAP_TXT		0x0002  /**< clipboard has a scrap.txt file, see mt_scrap_read() */
#define SCRAP_GEM		0x0004  /**< clipboard has a scrap.gem file, see mt_scrap_read() */
#define SCRAP_IMG		0x0008  /**< clipboard has a scrap.img file, see mt_scrap_read() */
#define SCRAP_DCA		0x0010  /**< clipboard has a scrap.dca file, see mt_scrap_read() */
#define SCRAP_DIF		0x0020  /**< clipboard has a scrap.dif file, see mt_scrap_read() */
#define SCRAP_USR		0x8000  /**< clipboard has a scrap.usr file, see mt_scrap_read() */

/* Window Attributes */
#define NAME			0x0001	/**< Window has a title bar */
#define CLOSER 			0x0002	/**< Window has a close box */
#define FULLER 			0x0004	/**< Window has a fuller box */
#define MOVER			0x0008	/**< Window may be moved by the user */
#define INFO			0x0010	/**< Window has an information line */
#define SIZER			0x0020	/**< Window has a sizer box */
#define UPARROW			0x0040	/**< Window has an up arrow */
#define DNARROW			0x0080	/**< Window has a down arrow */
#define VSLIDE 			0x0100	/**< Window has a vertical slider */
#define LFARROW			0x0200	/**< Window has a left arrow */
#define RTARROW			0x0400	/**< Window has a right arrow */
#define HSLIDE 			0x0800	/**< Window has a horizontal slider */
#define HOTCLOSEBOX		0x1000  /**< Window has "hot close box" box (GEM 2.x) */
#define MENUBAR			0x1000	/**< Window has a menu bar (XaAES) */
#define BACKDROP		0x2000	/**< Window has a backdrop box */
#define SMALLER			0x4000	/**< Window has an iconifier */
#define BORDER			0x8000	/**< Window has border-resizable capability (XaAES newer than Nov 8 2004) */
#define ICONIFIER		SMALLER	/**< Window has an iconifier */

/* wind_create flags */
#define WC_BORDER 	 	0	/**< compute the extent of a window from its work area, see mt_wind_calc() */
#define WC_WORK		 	1	/**< compute the work_area of a window from its extent, see mt_wind_calc() */

/* wind_get flags */
#define WF_KIND			1	/**< get     the actual window attributes, see mt_wind_get() */
#define WF_NAME			2 	/**< get/set title name of the window, see mt_wind_get() and mt_wind_set() */
#define WF_INFO			3 	/**< get/set info line of the window, see mt_wind_get() and mt_wind_set() */
#define WF_WORKXYWH		4 	/**< get     the work area coordinates of the work area, see mt_wind_get() */
#define WF_CURRXYWH		5 	/**< get/set current coordinates of the window (external area), see mt_wind_get() and mt_wind_set()  */
#define WF_PREVXYWH		6 	/**< get     the previous coordinates of the window (external area), see mt_wind_get() */
#define WF_FULLXYWH		7 	/**< get     the coordinates of the window when "fulled" the screen, see mt_wind_get() */
#define WF_HSLIDE 		8 	/**< get/set position of the horizontal slider, see mt_wind_get() and mt_wind_set() */
#define WF_VSLIDE 		9 	/**< get/set position of the vertical slider, see mt_wind_get() and mt_wind_set() */
#define WF_TOP 			10 	/**< get/set top window, see mt_wind_get() and mt_wind_set() */
#define WF_FIRSTXYWH 	11 	/**< get     the first rectangle in the list of rectangles for this window, see mt_wind_get() */
#define WF_NEXTXYWH		12 	/**< get     the next rectangle in the list of rectangles for this window, see mt_wind_get() */
#define WF_FIRSTAREAXYWH	13 	/**< get     the first rectangle in the list of rectangles for this window, see mt_wind_xget()*/
#define WF_NEWDESK		14 	/**< get/set OBJECT tree installed as desktop, see mt_wind_get() and mt_wind_set() */
#define WF_HSLSIZE		15 	/**< get/set size of the horizontal slider, see mt_wind_get() and mt_wind_set() */
#define WF_VSLSIZE		16 	/**< get/set size of the vertical slider, see mt_wind_get() and mt_wind_set() */
#define WF_SCREEN 		17 	/**< get     current AES menu/alert buffer and its size, see mt_wind_get() */
#define WF_COLOR		18 	/**< get/set current color of widget, see mt_wind_get() and mt_wind_set() */
#define WF_DCOLOR 		19 	/**< get/set default color of widget, see mt_wind_get() and mt_wind_set() */
#define WF_OWNER		20 	/**< get     the owner of the window, see mt_wind_get() */
#define WF_BEVENT 		24 	/**< get/set window feature on mouse button event, see mt_wind_get() and mt_wind_set() */
#define WF_BOTTOM 		25 	/**< get/set bottom window, see mt_wind_get() and mt_wind_set() */
#define WF_ICONIFY		26 	/**< get/set iconification of the window, see mt_wind_get() and mt_wind_set() */
#define WF_UNICONIFY 		27 	/**< get/set un-iconification of the window, see mt_wind_get() and mt_wind_set() */
#define WF_UNICONIFYXYWH	28 	/**<     set window coordinates when uniconified , see mt_wind_set() */
#define WF_TOOLBAR		30 	/**< get/set tool bar attached to a window, see mt_wind_get() and mt_wind_set() */
#define WF_FTOOLBAR		31 	/**< get     the first rectangle of the toolbar area, see mt_wind_get() */
#define WF_NTOOLBAR		32 	/**< get     the next rectangle of the toolbar area, see mt_wind_get() */
#define WF_MENU			33 	/**<         TODO (XaAES) */
#define WF_WHEEL		40 	/**<     set window feature on mouse wheel event, see mt_wind_set() */
#define WF_OPTS			41	/**< get/set window options. See mt_wind_set() and mt_wind_get() for details. */
#define WF_CALCF2W		42	/**< Convert FULL coordinates to WORK coordinates */
#define WF_CALCW2F		43	/**< Convert WORK coordinates to FULL coordinates */
#define WF_CALCF2U		44	/**< Convert FULL coordinates to USER coordinates */
#define WF_CALCU2F		45	/**< Convert USER coordinates to FULL coordinates */
#define WF_MAXWORKXYWH	46	/**< Get MAX coordinates for this window - WCOWORK mode only*/
#define WF_M_BACKDROP	100 /**<		 TODO (KAOS 1.4) */
#define WF_M_OWNER		101 /**<		 TODO (KAOS 1.4) */
#define WF_M_WINDLIST	102 /**<		 TODO (KAOS 1.4) */
#define WF_MINXYWH		103 /**<		 TODO (MagiC 6) */
#define WF_INFOXYWH		104 /**<		 TODO (MagiC 6.10) */
#define WF_WIDGETS		200	/**< get/set actual positions of the slider widgets, see mt_wind_get() and mt_wind_set() */
#define WF_USER_POINTER	230	/**< MyAES - attach a 32 bit value to window see  mt_wind_set() see mt_wind_get() */
#define WF_WIND_ATTACH	231	/**< MyAES - attach a window to another, see mt_wind_set() */
#define WF_TOPMOST		232	/**< MyAES    set actual window at TOPMOST level, see mt_wind_set() */
#define WF_BITMAP		233	/**< MyAES 0.96	get bitmap of the window, see  mt_wind_get() */
#define WF_OPTIONS		234	/**< MyAES 0.96 at this time use only to request automaticaly close if application lost focus and appear when focus is back, see mt_wind_set() */
#define WF_FULLSCREEN	235	/**< MyAES 0.96 set window in fullscreen without widget, see mt_wind_set() */
#define WF_OBFLAG		1001	/**< get/set (doc: TODO) (FreeGEM) */
#define WF_OBTYPE		1002	/**< get     (doc: TODO) (FreeGEM) */
#define WF_OBSPEC		1003	/**< get/set (doc: TODO) (FreeGEM) */
#define X_WF_MENU		4352	/**<     set (doc: TODO) (Geneva)  */
#define X_WF_DIALOG		4608	/**<     set (doc: TODO) (Geneva)  */
#define X_WF_DIALWID	4864	/**<     set (doc: TODO) (Geneva)  */
#define X_WF_DIALHT		5120	/**<     set (doc: TODO) (Geneva)  */
#define X_WF_DFLTDESK	5376	/**<     set (doc: TODO) (Geneva)  */
#define X_WF_MINMAX		5632	/**< get/set (doc: TODO) (Geneva)  */
#define X_WF_HSPLIT		5888	/**< get/set (doc: TODO) (Geneva)  */
#define X_WF_VSPLIT		6144	/**< get/set (doc: TODO) (Geneva)  */
#define X_WF_SPLMIN		6400	/**< get/set (doc: TODO) (Geneva)  */
#define X_WF_HSLIDE2	6656	/**< get/set (doc: TODO) (Geneva)  */
#define X_WF_VSLIDE2	6144	/**< get/set (doc: TODO) (Geneva)  */
#define X_WF_HSLSIZE2	7168	/**< get/set (doc: TODO) (Geneva)  */
#define X_WF_VSLSIZE2	7424	/**< get/set (doc: TODO) (Geneva)  */
#define X_WF_DIALFLGS	7680	/**< get/set (doc: TODO) (Geneva)  */
#define X_WF_OBJHAND	7936	/**< get/set (doc: TODO) (Geneva)  */
#define X_WF_DIALEDIT	9182	/**< get/set (doc: TODO) (Geneva)  */
#define X_WF_DCOLSTAT	8448	/**< get/set (doc: TODO) (Geneva)  */
#define WF_WINX			22360	/**<		 TODO */
#define WF_WINXCFG		22361	/**<		 TODO */
#define WF_DDELAY		22362	/**<		 TODO */
#define WF_SHADE		22365	/**<		 TODO (WINX 2.3) */
#define WF_STACK		22366	/**<		 TODO (WINX 2.3) */
#define WF_TOPALL		22367	/**<		 TODO (WINX 2.3) */
#define WF_BOTTOMALL	22368	/**<		 TODO (WINX 2.3) */
#define WF_XAAES		0x5841	/**<		 TODO (XaAES) : 'XA' */

/* wind_set(WF_BEVENT) */
#define BEVENT_WORK		0x0001    /**< window not topped when click on the work area, see #WF_BEVENT */
#define BEVENT_INFO		0x0002    /**< window not topped when click on the info area, see #WF_BEVENT */

/* wind_set(WF_OPTS) bitmask flags */
#define WO0_WHEEL		0x0001  /**< see mt_wind_set() with #WF_OPTS mode */
#define WO0_FULLREDRAW	0x0002  /**< see mt_wind_set() with #WF_OPTS mode */
#define WO0_NOBLITW		0x0004  /**< see mt_wind_set() with #WF_OPTS mode */
#define WO0_NOBLITH		0x0008  /**< see mt_wind_set() with #WF_OPTS mode */
#define WO0_SENDREPOS	0x0010  /**< see mt_wind_set() with #WF_OPTS mode */
#define WO0_WCOWORK		0x0020  /**< see mt_wind_set() with #WF_OPTS mode */
#define WO1_NONE		0x0000  /**< see mt_wind_set() with #WF_OPTS mode */
#define WO2_NONE		0x0000  /**< see mt_wind_set() with #WF_OPTS mode */

/* wind_set(WF_WHEEL) modes */
#define WHEEL_MESAG			0	/**< AES will send #WM_WHEEL messages */
#define WHEEL_ARROWED		1   /**< AES will send #WM_ARROWED messages */
#define WHEEL_SLIDER		2   /**< AES will convert mouse wheel events to slider events */

/* window elements */
#define W_BOX			0	/**< widget index of root object, see #WF_COLOR */
#define W_TITLE			1	/**< widget index of titlebar, see #WF_COLOR */
#define W_CLOSER		2	/**< widget index of closer gadget, see #WF_COLOR */
#define W_NAME 			3	/**< widget index of window name, see #WF_COLOR */
#define W_FULLER		4	/**< widget index of fuller gadget, see #WF_COLOR */
#define W_INFO 			5	/**< widget index of info bar, see #WF_COLOR */
#define W_DATA 			6	/**< widget index of ???, see #WF_COLOR */
#define W_WORK 			7	/**< widget index of work area, see #WF_COLOR */
#define W_SIZER			8	/**< widget index of sizer gadget, see #WF_COLOR */
#define W_VBAR 			9	/**< widget index of vertical bar background, see #WF_COLOR */
#define W_UPARROW		10	/**< widget index of up arrow, see #WF_COLOR */
#define W_DNARROW		11	/**< widget index of down arrow, see #WF_COLOR */
#define W_VSLIDE		12	/**< widget index of vertical slider, see #WF_COLOR */
#define W_VELEV			13	/**< widget index of vertical bar, see #WF_COLOR */
#define W_HBAR			14	/**< widget index of horizontal bar background, see #WF_COLOR */
#define W_LFARROW		15	/**< widget index of left arrow, see #WF_COLOR */
#define W_RTARROW		16	/**< widget index of right arrow, see #WF_COLOR */
#define W_HSLIDE		17	/**< widget index of horizontal slider, see #WF_COLOR */
#define W_HELEV			18	/**< widget index of horizontal bar, see #WF_COLOR */
#define W_SMALLER		19	/**< widget index of smaller gadget, see #WF_COLOR */
#define W_BOTTOMER		20 	/**< widget index of bottomer widget, see #WF_COLOR */
#define W_HIDER			30	/**< widget index of hide widget, see #WF_COLOR */

/* arrow message */
#define WA_UPPAGE 		0	/**< Page Up,      see #WM_ARROWED  */
#define WA_DNPAGE 		1	/**< Page Down,    see #WM_ARROWED  */
#define WA_UPLINE 		2	/**< Row Up,       see #WM_ARROWED  */
#define WA_DNLINE 		3	/**< Row Down,     see #WM_ARROWED  */
#define WA_LFPAGE 		4	/**< Page Left ,   see #WM_ARROWED  */
#define WA_RTPAGE 		5	/**< Page Right,   see #WM_ARROWED  */
#define WA_LFLINE 		6	/**< Column Left,  see #WM_ARROWED  */
#define WA_RTLINE 		7	/**< Column Right, see #WM_ARROWED  */
#define WA_WHEEL		8	/**< deprecated (introduced in XaAES release 0.964)  */
#define WA_UPSCAN		8	/* XaAES only */
#define WA_DNSCAN		9	/* XaAES only */
#define WA_LFSCAN		10	/* XaAES only */
#define WA_RTSCAN		11	/* XaAES only */

/* wind_update flags */
#define END_UPDATE		0	/**< release the screen lock, see mt_wind_update() */
#define BEG_UPDATE		1	/**< lock the screen, see mt_wind_update() */
#define END_MCTRL		2	/**< release the mouse control to the AES, see mt_wind_update() */
#define BEG_MCTRL		3	/**< mouse button message only sent to the application, see mt_wind_update() */
#define NO_BLOCK		0x100  /**< prevent the application from blocking, see mt_wind_update()*/

/* graf_mouse mouse types */
#define ARROW			 0	/**< see mt_graf_mouse() */
#define TEXT_CRSR 		 1	/**< see mt_graf_mouse() */
#define BEE 			 2	/**< see mt_graf_mouse() */
#define BUSY_BEE		BEE 	/**< alias for #BEE */
#define BUSYBEE			BEE 	/**< alias for #BEE */
#define HOURGLASS 		 2	/**< see mt_graf_mouse() */
#define POINT_HAND		 3	/**< see mt_graf_mouse() */
#define FLAT_HAND 		 4	/**< see mt_graf_mouse() */
#define THIN_CROSS		 5	/**< see mt_graf_mouse() */
#define THICK_CROSS		 6	/**< see mt_graf_mouse() */
#define OUTLN_CROSS		 7	/**< see mt_graf_mouse() */
#define USER_DEF		255	/**< see mt_graf_mouse() */
#define M_OFF			256	/**< see mt_graf_mouse() */
#define M_ON			257	/**< see mt_graf_mouse() */
#define M_SAVE 			258	/**< see mt_graf_mouse() */
#define M_RESTORE 		259	/**< see mt_graf_mouse() */
#define M_LAST 			260	/**< see mt_graf_mouse() */
#define M_PREVIOUS		M_LAST /**< alias for #M_LAST */
#define X_MRESET      1000	/* geneva */
#define X_MGET        1001	/* geneva */
#define X_MSET_SHAPE  1100	/* geneva */
#define M_FORCE			0x8000	/**< see mt_graf_mouse() */

/* objects - general */
#define ROOT		 	0 	 /**< index of the root object of a formular */
#define MAX_LEN			81 	 /* max string length */	/**< TODO */
#define MAX_DEPTH		8 	 /* max depth of search or draw */	/**< TODO */

/* inside fill patterns */
#define IP_HOLLOW		0	/**< TODO */
#define IP_1PATT		1	/**< TODO */
#define IP_2PATT		2	/**< TODO */
#define IP_3PATT		3	/**< TODO */
#define IP_4PATT		4	/**< TODO */
#define IP_5PATT		5	/**< TODO */
#define IP_6PATT		6	/**< TODO */
#define IP_SOLID		7	/**< TODO */

/* font types */
#define GDOS_PROP		0	/**< TODO */
#define GDOS_MONO		1	/**< TODO */
#define GDOS_BITM		2	/**< TODO */
#define IBM				3	/**< TODO */
#define SMALL			5	/**< TODO */

/* object types */
#define G_BOX			20	/**< TODO */
#define G_TEXT			21	/**< TODO */
#define G_BOXTEXT		22	/**< TODO */
#define G_IMAGE			23	/**< TODO */
#define G_USERDEF		24	/**< TODO */
#define G_PROGDEF		G_USERDEF	/**< TODO */
#define G_IBOX			25	/**< TODO */
#define G_BUTTON		26	/**< TODO */
#define G_BOXCHAR		27	/**< TODO */
#define G_STRING		28	/**< TODO */
#define G_FTEXT			29	/**< TODO */
#define G_FBOXTEXT		30	/**< TODO */
#define G_ICON			31	/**< TODO */
#define G_TITLE			32	/**< TODO */
#define G_CICON			33	/**< TODO */

/* extended object types, MagiC only */
#define G_SWBUTTON		34	/**< TODO */
#define G_POPUP			35	/**< TODO */
#define G_WINTITLE		36	/**< TODO */
#define G_EDIT			37	/**< TODO */
#define G_SHORTCUT		38	/**< TODO */


/* object flags */
#define OF_NONE			0x0000	/**< TODO */
#define OF_SELECTABLE	0x0001	/**< TODO */
#define OF_DEFAULT		0x0002	/**< TODO */
#define OF_EXIT			0x0004	/**< TODO */
#define OF_EDITABLE		0x0008	/**< TODO */
#define OF_RBUTTON		0x0010	/**< TODO */
#define OF_LASTOB		0x0020	/**< TODO */
#define OF_TOUCHEXIT	0x0040	/**< TODO */
#define OF_HIDETREE		0x0080	/**< TODO */
#define OF_INDIRECT		0x0100	/**< TODO */
#define OF_FL3DIND		0x0200	/* bit 9 */	/**< TODO */
#define OF_FL3DBAK		0x0400	/* bit 10 */	/**< TODO */
#define OF_FL3DACT		0x0600	/**< TODO */
#define OF_FL3DMASK		(OF_FL3DIND|OF_FL3DBAK|OF_FL3DACT)
#define OF_SUBMENU		0x0800	/* bit 11 */	/**< TODO */
#define OF_FLAG11		OF_SUBMENU	/**< TODO */
#define OF_FLAG12		0x1000	/**< TODO */
#define OF_FLAG13		0x2000	/**< TODO */
#define OF_FLAG14		0x4000	/**< TODO */
#define OF_FLAG15		0x8000	/**< TODO */

/* Object states */
#define OS_NORMAL		0x0000	/**< TODO */
#define OS_SELECTED		0x0001	/**< TODO */
#define OS_CROSSED		0x0002	/**< TODO */
#define OS_CHECKED		0x0004	/**< TODO */
#define OS_DISABLED		0x0008	/**< TODO */
#define OS_OUTLINED		0x0010	/**< TODO */
#define OS_SHADOWED		0x0020	/**< TODO */
#define OS_WHITEBAK		0x0040	/**< TODO */
#define OS_DRAW3D		0x0080	/**< TODO */
#define OS_STATE08		0x0100	/**< TODO */
#define OS_STATE09		0x0200	/**< TODO */
#define OS_STATE10		0x0400	/**< TODO */
#define OS_STATE11		0x0800	/**< TODO */
#define OS_STATE12		0x1000	/**< TODO */
#define OS_STATE13		0x2000	/**< TODO */
#define OS_STATE14		0x4000	/**< TODO */
#define OS_STATE15		0x8000	/**< TODO */

/* Object colors - default pall. */
#define G_WHITE			0	/**< TODO */
#define G_BLACK			1	/**< TODO */
#define G_RED			2	/**< TODO */
#define G_GREEN			3	/**< TODO */
#define G_BLUE			4	/**< TODO */
#define G_CYAN			5	/**< TODO */
#define G_YELLOW		6	/**< TODO */
#define G_MAGENTA		7	/**< TODO */
#define G_LWHITE		8	/**< TODO */
#define G_LBLACK		9	/**< TODO */
#define G_LRED			10	/**< TODO */
#define G_LGREEN		11	/**< TODO */
#define G_LBLUE			12	/**< TODO */
#define G_LCYAN			13	/**< TODO */
#define G_LYELLOW		14	/**< TODO */
#define G_LMAGENTA		15	/**< TODO */


#ifdef __GEMLIB_OLDNAMES

/* object flags */
#define NONE		 	0x0000	/**< TODO */
#define SELECTABLE		0x0001	/**< TODO */
#define DEFAULT			0x0002	/**< TODO */
#define EXIT			0x0004	/**< TODO */
#define EDITABLE		0x0008	/**< TODO */
#define RBUTTON			0x0010	/**< TODO */
#define LASTOB			0x0020	/**< TODO */
#define TOUCHEXIT		0x0040	/**< TODO */
#define HIDETREE		0x0080	/**< TODO */
#define INDIRECT		0x0100	/**< TODO */
#define FL3DIND			0x0200	/* bit 9 */	/**< TODO */
#define FL3DBAK			0x0400	/* bit 10 */	/**< TODO */
#define FL3DACT			0x0600	/**< TODO */
#define SUBMENU			0x0800	/* bit 11 */	/**< TODO */
#define FLAG11			SUBMENU	/**< TODO */
#define FLAG12			0x1000	/**< TODO */
#define FLAG13			0x2000	/**< TODO */
#define FLAG14			0x4000	/**< TODO */
#define FLAG15			0x8000	/**< TODO */

/* Object states */
#define NORMAL			0x0000	/**< TODO */
#define SELECTED		0x0001	/**< TODO */
#define CROSSED			0x0002	/**< TODO */
#define CHECKED			0x0004	/**< TODO */
#define DISABLED		0x0008	/**< TODO */
#define OUTLINED		0x0010	/**< TODO */
#define SHADOWED		0x0020	/**< TODO */
#define WHITEBAK		0x0040	/**< TODO */
#define DRAW3D			0x0080	/**< TODO */
#define STATE08			0x0100	/**< TODO */
#define STATE09			0x0200	/**< TODO */
#define STATE10			0x0400	/**< TODO */
#define STATE11			0x0800	/**< TODO */
#define STATE12			0x1000	/**< TODO */
#define STATE13			0x2000	/**< TODO */
#define STATE14			0x4000	/**< TODO */
#define STATE15			0x8000	/**< TODO */

/* Object colors - default pall. */
#define WHITE			0	/**< TODO */
#define BLACK			1	/**< TODO */
#define RED				2	/**< TODO */
#define GREEN			3	/**< TODO */
#define BLUE			4	/**< TODO */
#define CYAN			5	/**< TODO */
#define YELLOW			6	/**< TODO */
#define MAGENTA			7	/**< TODO */
#define LWHITE			8	/**< TODO */
#define LBLACK			9	/**< TODO */
#define LRED			10	/**< TODO */
#define LGREEN			11	/**< TODO */
#define LBLUE			12	/**< TODO */
#define LCYAN			13	/**< TODO */
#define LYELLOW			14	/**< TODO */
#define LMAGENTA		15	/**< TODO */

#endif


/* editable text field definitions */
#define ED_START		0	/**< Reserved. Do not use, see mt_objc_edit() */
#define ED_INIT 		1	/**< turn ON the cursor, see mt_objc_edit() */
#define ED_CHAR 		2	/**< insert a character in the editable field, see mt_objc_ecit() */
#define ED_END			3	/**< turn OFF the cursor, see mt_objc_edit() */

#define EDSTART			ED_START	/**< alias */
#define EDINIT			ED_INIT		/**< alias */
#define EDCHAR			ED_CHAR		/**< alias */
#define EDEND 			ED_END		/**< alias */

/* XaAES extensions to objc_edit() */
#define ED_DISABLE		5
#define ED_ENABLE		6
#define ED_CRSRON		7
#define ED_CRSROFF		8

#define ED_MARK			9
#define ED_STRING		10
#define ED_SETPTEXT		11
#define ED_SETPTMPLT	12
#define ED_SETPVALID	13
#define ED_GETPTEXT		14
#define ED_GETPTMPLT	15
#define ED_GETPVALID	16

#define ED_CRSR			100	/**< TO BE COMPLETED (MagiC), see mt_objc_edit() */
#define ED_DRAW			103	/**< TO BE COMPLETED (MagiC), see mt_objc_edit() */

#define ED_REDRAW		200		/* XaAES only */
#define ED_XINIT		201
#define ED_XCHAR		202
#define ED_XEND			203		/* Redraw cursor, XaAES only */
#define ED_CHGTEXTPTR	204
#define ED_CHGTMPLPTR	205

/* editable text justification */
#define TE_LEFT			0	/**< TODO */
#define TE_RIGHT		1	/**< TODO */
#define TE_CNTR			2	/**< TODO */

/* objc_change modes */
#define NO_DRAW			0	/**< object will not be redrawn, see mt_objc_change() */
#define REDRAW 			1	/**< object will be redrawn, see mt_objc_change() */

/* objc_order modes */
#define OO_LAST			-1	/**< make object the last child, see mt_objc_order() */
#define OO_FIRST		0	/**< make object the first child, see mt_objc_order() */

/* objc_sysvar modes */
#define SV_INQUIRE		0	/**< inquire sysvar data, see mt_objc_sysvar() */
#define SV_SET 			1	/**< set sysvar data, see mt_objc_sysvar() */

/* objc_sysvar values */
#define LK3DIND			1	/**< text of indicator object moves when selected, see mt_objc_sysvar() */
#define LK3DACT			2	/**< text of activator object moves when selected, see mt_objc_sysvar()*/
#define INDBUTCOL 		3	/**< default color for indicator objects, see mt_objc_sysvar() */
#define ACTBUTCOL 		4	/**< default color for activator objects, see mt_objc_sysvar() */
#define BACKGRCOL 		5	/**< default color for background objects, see mt_objc_sysvar()  */
#define AD3DVAL			6	/**< number of extra pixels to accomodate 3D effects, see mt_objc_sysvar() */
#define MX_ENABLE3D		10  /**< enable or disable the 3D look (MagiC 3), see mt_objc_sysvar() */
#define MENUCOL			11  /**< TO BE COMPLETED (MagiC 6), see mt_objc_sysvar() */

/** Mouse Form Definition Block, see mt_graf_mouse() */
typedef struct mouse_form
{
	/** x-location of the mouse hot-spot. 
	 * This value should be in the range 0 to 15 and defines what offset
	 * into the bitmap is actually the 'point'.
	 */
	short		mf_xhot;

	/** y-location of the mouse hot-spot. 
	 * This value should be in the range 0 to 15 and defines what offset
	 * into the bitmap is actually the 'point'.
	 */
	short		mf_yhot;

	/** the number of bit-planes used by the mouse pointer.
	 * Currently, the value of 1 is the only legal value.
	 */
	short		mf_nplanes;

	/** palette index of the color used for the data.
	 *  this data is usually 1.
	 */
	short		mf_fg;

	/** palette index of the color used for the mask
	 *  this data is usually 0.
	 */
	short		mf_bg;

	/** array of 16 WORD's which define the mask
     *  portion of the mouse form
	 */
	short		mf_mask[16];

	/** array of 16 WORD's which define the data
     *  portion of the mouse form
	 */
	short		mf_data[16];
} MFORM;

#ifndef __PXY
# define __PXY	/**< TODO */
/** TODO */
typedef struct point_coord
{
	short p_x;			/**< TODO */
	short p_y;			/**< TODO */
} PXY;
#endif

#ifndef __GRECT
# define __GRECT	/**< TODO */
/** TODO */
typedef struct graphic_rectangle
{
	short g_x;			/**< TODO */
	short g_y;			/**< TODO */
	short g_w;			/**< TODO */
	short g_h;			/**< TODO */
} GRECT;
#endif

/** TODO */
typedef struct objc_colorword
{
	unsigned	borderc : 4;			/**< TODO */
	unsigned	textc   : 4;			/**< TODO */
	unsigned	opaque  : 1;			/**< TODO */
	unsigned	pattern : 3;			/**< TODO */
	unsigned	fillc   : 4;			/**< TODO */
} OBJC_COLORWORD;

/** TODO */
typedef struct text_edinfo
{
	char		*te_ptext;		/**< ptr to text */
	char		*te_ptmplt;		/**< ptr to template */
	char		*te_pvalid;		/**< ptr to validation chrs. */
	short		te_font; 		/**< font */
	short		te_fontid;		/**< font id */
	short		te_just; 		/**< justification */
	short		te_color;		/**< color information word */
	short		te_fontsize;	/**< font size */
	short		te_thickness;	/**< border thickness */
	short		te_txtlen;		/**< length of text string */
	short		te_tmplen;		/**< length of template string */
} TEDINFO;

/** TODO */
typedef struct icon_block
{
	short		*ib_pmask;			/**< Ptr to the icon mask*/
	short		*ib_pdata;			/**< Ptr to the icon image */
	char		*ib_ptext;			/**< Ptr to the icon text */
	short		ib_char;			/**< Icon character, foreground and background color \n Bits : \n 15-12 Icon foreground color\n 11-8 Icon Background color \n 7-0 ASCCI value of the Character to be displayed*/
	short		ib_xchar;			/**< X coordinate of the character */
	short		ib_ychar;			/**< Y coordinate of the character */
	short		ib_xicon;			/**< X coordinate of the icon */
	short		ib_yicon;			/**< Y coordinate of the icon */
	short		ib_wicon;			/**< Width of the icon */
	short		ib_hicon;			/**< Height of the icon */
	short		ib_xtext;			/**< X coordinate of the text (relative to the position of the icon) */
	short		ib_ytext;			/**< Y coordinate of the text (relative to the position of the icon)*/
	short		ib_wtext;			/**< Width in pixels of the text */
	short		ib_htext;			/**< Height in pixels of the text */
} ICONBLK;

/** TODO */
typedef struct bit_block
{
	short		*bi_pdata;	/**< ptr to bit forms data  */
	short		bi_wb;		/**< width of form in bytes */
	short		bi_hl;		/**< height in lines */
	short		bi_x; 		/**< source x in bit form */
	short		bi_y; 		/**< source y in bit form */
	short		bi_color;	/**< fg color of blt */
} BITBLK;

/** TODO */
typedef struct cicon_data
{
	short 		num_planes;			/**< Number of color planes */
	short 		*col_data;			/**< Ptr to colour bitmap in standard form */
	short 		*col_mask;			/**< Ptr to individual colour plane mask (1 bit)*/
	short 		*sel_data;			/**< Ptr to colour bitmap of selected icon in standard format */
	short 		*sel_mask;			/**< Ptr to individual plane mask of the icon (1 bit)*/
	struct cicon_data *next_res;	/**< Next resolution definition of the icon*/
} CICON;

/** TODO */
typedef struct cicon_blk
{
	ICONBLK		monoblk;			/**< ICONBLK definition (B&W icon) */
	CICON 		*mainlist;			/**< list of the different color icon resolution */
} CICONBLK;

/** TODO */
typedef struct
{
	 unsigned	character   :  8;			/**< TODO */
	 signed		framesize   :  8;			/**< TODO */
	 unsigned	framecol    :  4;			/**< TODO */
	 unsigned	textcol	    :  4;			/**< TODO */
	 unsigned	textmode    :  1;			/**< TODO */
	 unsigned	fillpattern :  3;			/**< TODO */
	 unsigned	interiorcol :  4;			/**< TODO */
} BFOBSPEC;

/** TODO */
struct user_block;	/* forward declaration */

/** TODO */
typedef union obspecptr
{
	long		index;				/**< TODO */
	union obspecptr	*indirect;		/**< TODO */
	BFOBSPEC 	obspec;				/**< TODO */
	TEDINFO		*tedinfo;			/**< TODO */
	BITBLK		*bitblk;			/**< TODO */
	ICONBLK		*iconblk;			/**< TODO */
	CICONBLK 	*ciconblk;			/**< TODO */
	struct user_block *userblk;		/**< TODO */
	char		*free_string;		/**< TODO */
} OBSPEC;

/** TODO */
typedef struct object
{
	short 		ob_next;		/**< object's next sibling */
	short 		ob_head; 		/**< head of object's children */
	short 		ob_tail; 		/**< tail of object's children */
	unsigned short	ob_type; 	/**< type of object */
	unsigned short	ob_flags;	/**< flags */
	unsigned short	ob_state;	/**< state */
	OBSPEC		ob_spec; 		/**< object-specific data */
	short 		ob_x; 			/**< upper left corner of object */
	short 		ob_y; 			/**< upper left corner of object */
	short 		ob_width;		/**< width of obj */
	short 		ob_height;		/**< height of obj */
} OBJECT;

typedef struct parm_block
{
	OBJECT	*pb_tree;			/**< Pointer to the object tree         */
	short	pb_obj;				/**< Index of the object                */
	short	pb_prevstate;		/**< Previous object status             */
	short	pb_currstate;		/**< New object status            	    */
	short	pb_x;				/**< X-position of the object           */
	short	pb_y;				/**< Y-position of the object           */
	short	pb_w;				/**< Width of the object                */
	short	pb_h;				/**< Height of the object               */
	short	pb_xc;				/**< X-position of the clipping region  */
	short	pb_yc;				/**< Y-position of the clipping region  */
	short	pb_wc;				/**< Width of the clipping region       */
	short	pb_hc;				/**< Height of the clipping region      */
	long	pb_parm;			/**< Parameter of USERBLK structure     */
} PARMBLK;

/** TODO */
typedef struct user_block
{
	short __CDECL (*ub_code)(PARMBLK *parmblock);	/**< TODO */
	long ub_parm;									/**< TODO */
} USERBLK;

/** TODO */
typedef struct rshdr
{
	short			rsh_vrsn;			/**< TODO */
	unsigned short	rsh_object;			/**< TODO */
	unsigned short	rsh_tedinfo;		/**< TODO */
	unsigned short	rsh_iconblk;		/**< list of ICONBLKS */
	unsigned short	rsh_bitblk;			/**< TODO */
	unsigned short	rsh_frstr;			/**< TODO */
	unsigned short	rsh_string;			/**< TODO */
	unsigned short	rsh_imdata;			/**< image data */
	unsigned short	rsh_frimg;			/**< TODO */
	unsigned short	rsh_trindex;		/**< TODO */
	short			rsh_nobs;			/**< counts of various structs */
	short			rsh_ntree;			/**< TODO */
	short			rsh_nted;			/**< TODO */
	short			rsh_nib;			/**< TODO */
	short			rsh_nbb;			/**< TODO */
	short			rsh_nstring;		/**< TODO */
	short			rsh_nimages;		/**< TODO */
	unsigned short	rsh_rssize;			/**< total bytes in resource */
} RSHDR;

#ifndef __RSXHDR
#define __RSXHDR
typedef struct rsxhdr
{
	unsigned short	rsh_vrsn;     /* should be 3                                 */
	unsigned short	rsh_extvrsn;  /* not used, initialised to 'IN' for Interface */

	unsigned long	rsh_object;
	unsigned long	rsh_tedinfo;
	unsigned long	rsh_iconblk;  /* list of ICONBLKS                            */
	unsigned long	rsh_bitblk;
	unsigned long	rsh_frstr;
	unsigned long	rsh_string;
	unsigned long	rsh_imdata;   /* image data                                  */
	unsigned long	rsh_frimg;
	unsigned long	rsh_trindex;

	unsigned long	rsh_nobs;     /* counts of various structs                   */
	unsigned long	rsh_ntree;
	unsigned long	rsh_nted;
	unsigned long	rsh_nib;
	unsigned long	rsh_nbb;
	unsigned long	rsh_nstring;
	unsigned long	rsh_nimages;
	unsigned long	rsh_rssize;   /* total bytes in resource                     */
} RSXHDR;
#endif

/** MENU structure, used by mt_menu_attach() and mt_menu_popup() */
typedef struct _menu
{
	OBJECT *mn_tree;    /**< Points to the OBJECT tree of the sub-menu */
	short  mn_menu;     /**< Is an index to the parent object of the menu items. */
	short  mn_item;     /**< Is the starting menu item */
	short  mn_scroll;   /**< contains one of the following values:
	                         - #SCROLL_NO , the menu will not scroll.
                             - #SCROLL_YES , if the number of menu
                             items exceed the menu scroll height, arrows
                             will appear which allow the user to scroll
                             selections.
							 - #SCROLL_LISTBOX to display a drop-down list
							 instead of a popup menu, see mt_menu_popup() */
	short  mn_keystate; /**< key state */
} MENU;

/** TODO */
typedef struct
{
	short m_out;	/**< TODO */
	short m_x;		/**< TODO */
	short m_y;		/**< TODO */
	short m_w;		/**< TODO */
	short m_h;		/**< TODO */
} MOBLK;

/** TODO */
typedef struct mouse_event_type
{
	short *x;	/**< TODO */
	short *y;	/**< TODO */
	short *b;	/**< TODO */
	short *k;	/**< TODO */
} MOUSE_EVENT;

/** structure comprising the most of the input arguments of mt_evnt_multi()
 */
typedef struct {
	short emi_flags;          /**< the event mask to watch */
	short emi_bclicks;		  /**< see mt_evnt_multi() */
	short emi_bmask;		  /**< see mt_evnt_multi() */
	short emi_bstate;		  /**< see mt_evnt_multi() */
	short emi_m1leave;		  /**< TODO */
	GRECT emi_m1;             /**< the first rectangle to watch */
	short emi_m2leave;		  /**< TODO */
	GRECT emi_m2;             /**< the second rectangle to watch */
	short emi_tlow;		  	  /**< see mt_evnt_multi() */
	short emi_thigh;          /**< the timer 32-bit value of interval split into short type member */
} EVMULT_IN;

/** structure comprising the output arguments of mt_evnt_multi()
 *
 * @note For undocumented members consult the mt_evnt_multi() documentation.
 */
typedef struct {
	short emo_events;	/**< the bitfield of events occured (also a return value of mt_evnt_multi_fast() */
	PXY   emo_mouse;	/**< TODO */
	short emo_mbutton;	/**< TODO */
	short emo_kmeta;	/**< TODO */
	short emo_kreturn;	/**< TODO */
	short emo_mclicks;	/**< TODO */
} EVMULT_OUT;

#endif

#ifdef __GEMLIB_AES

/*******************************************************************************
 * The AES bindings from old aesbind.h
 */

/** @addtogroup a_appl
 *  @{
 */
short	mt_appl_bvset   (short bvdisk, short bvhard, short *global_aes);
short	mt_appl_control (short ap_cid, short ap_cwhat, void *ap_cout, short *global_aes);
short	mt_appl_exit    (short *global_aes);
short	mt_appl_find    (const char *name, short *global_aes);
short	mt_appl_getinfo (short type,
						 short *out1, short *out2, short *out3, short *out4, short *global_aes);
short	mt_appl_getinfo_str (short type,
						 char *out1, char *out2, char *out3, char *out4, short *global_aes);
short	mt_appl_init    (short *global_aes);
short	mt_appl_read    (short ap_id, short length, void *ap_pbuff, short *global_aes);
short	mt_appl_search  (short mode, char *fname, short *type, short *ap_id, short *global_aes);
short	mt_appl_tplay   (void *mem, short num, short scale, short *global_aes);
short	mt_appl_trecord (void *mem, short count, short *global_aes);
short	mt_appl_write   (short ap_id, short length, void *ap_pbuff, short *global_aes);
short	mt_appl_yield   (short *global_aes);
short   mt_appl_get_cicon (short type,const char *name,short size,short *cicon_width,short *cicon_height,CICON **cicon_data, short *global_aes);

/** fast version of appl_yield that does not use the AES arrays */
void _appl_yield(void);
/**@}*/

/** @addtogroup a_evnt
 *  @{
 */
short	mt_evnt_button (short Clicks, short WhichButton, short WhichState,
						short *Mx, short *My, short *ButtonState, short *KeyState, short *global_aes);
short	mt_evnt_dclick (short ToSet, short SetGet, short *global_aes);
short	mt_evnt_keybd  (short *global_aes);
short	mt_evnt_mesag  (short MesagBuf[], short *global_aes);
short	mt_evnt_mouse  (short EnterExit, short InX, short InY, short InW, short InH,
						short *OutX, short *OutY, short *ButtonState, short *KeyState, short *global_aes);
short   mt_evnt_multi  (short Type, short Clicks, short WhichButton, short WhichState,
						short EnterExit1, short In1X, short In1Y, short In1W, short In1H,
						short EnterExit2, short In2X, short In2Y, short In2W, short In2H,
						short MesagBuf[], unsigned long Interval, short *OutX, short *OutY,
						short *ButtonState, short *KeyState, short *Key, short *ReturnCount, short *global_aes);
short   mt_evnt_multi_fast (const EVMULT_IN * em_i,
							short MesagBuf[], EVMULT_OUT * em_o, short *global_aes);
short	mt_evnt_timer  (unsigned long Interval, short *global_aes);
/**@}*/

/** @addtogroup a_form
 *  @{
 */
short mt_form_alert  (short DefButton, const char *Str, short *global_aes);
short mt_form_button (OBJECT *, short Bobject, short Bclicks, short *Bnxtobj, short *global_aes);
short mt_form_center (OBJECT *, short *Cx, short *Cy, short *Cw, short *Ch, short *global_aes);
short mt_form_center_grect (OBJECT *, GRECT *r, short *global_aes);
short mt_form_dial   (short Flag, short Sx, short Sy, short Sw, short Sh,
                               short Bx, short By, short Bw, short Bh, short *global_aes);
short mt_form_dial_grect(short Flag, const GRECT *little, const GRECT *big, short *global_aes);
short mt_form_do     (OBJECT *, short StartObj, short *global_aes);
short mt_form_error  (short ErrorCode, short *global_aes);
short mt_form_keybd  (OBJECT *, short Kobject, short Kobnext, short Kchar,
                             short *Knxtobject, short *Knxtchar, short *global_aes);
/**@}*/

/** @addtogroup a_fsel
 *  @{
 */
 
/** callback function used by BoxKite file selector. See mt_fsel_boxinput() */
typedef void __CDECL (* FSEL_CALLBACK)( short *msg);

short 	mt_fsel_exinput	(char *path, char *file, short *exit_button, const char *title, short *global);
short 	mt_fsel_input	(char *path, char *file, short *exit_button, short *global);
short 	mt_fsel_boxinput(char *path, char *file, short *exit_button, const char *title, FSEL_CALLBACK callback, short *global);
/**@}*/

/** @addtogroup a_graf
 *  @{
 */
short	mt_graf_dragbox	(short Sw, short Sh, short Sx, short Sy, short Bx, short By, short Bw, short Bh, short *Fw, short *Fh, short *global_aes);
short	mt_graf_growbox	(short Sx, short Sy, short Sw, short Sh, short Fx, short Fy, short Fw, short Fh, short *global_aes);
short	mt_graf_growbox_grect	(const GRECT *in, const GRECT *out, short *global_aes);
short	mt_graf_handle	(short *Wchar, short *Hchar, short *Wbox, short *Hbox, short *global_aes);
short	mt_graf_xhandle	(short *Wchar, short *Hchar, short *Wbox, short *Hbox, short * device, short *global_aes);
short	mt_graf_mbox	(short Sw, short Sh, short Sx, short Sy, short Dx, short Dy, short *global_aes);
short	mt_graf_mkstate	(short *Mx, short *My, short *ButtonState, short *KeyState, short *global_aes);
short	mt_graf_mouse	(short Form, const MFORM *FormAddress, short *global_aes);
short	mt_graf_rubberbox	(short Ix, short Iy, short Iw, short Ih, short *Fw, short *Fh, short *global_aes);
short	mt_graf_multirubber (short bx, short by, short mw, short mh, GRECT *rec, short *rw, short *rh, short *global_aes);
short	mt_graf_shrinkbox	(short Fx, short Fy, short Fw, short Fh, short Sx, short Sy, short Sw, short Sh, short *global_aes);
short	mt_graf_shrinkbox_grect	(const GRECT *in, const GRECT *out, short *global_aes);
short	mt_graf_slidebox	(OBJECT *, short Parent, short Object, short Direction, short *global_aes);
short	mt_graf_watchbox	(OBJECT *, short Object, short InState, short OutState, short *global_aes);
short	mt_graf_wwatchbox	(OBJECT *, short Object, short InState, short OutState, short whandle, short *global_aes);
/**@}*/

/** @addtogroup a_menu
 *  @{
 */
short 	mt_menu_attach		   (short me_flag, OBJECT *me_tree, short me_item, MENU *me_mdata, short *global);
short 	mt_menu_bar 		   (OBJECT *me_tree, short me_mode, short *global);
short	mt_menu_click		   (short click, short setit, short *global);
short 	mt_menu_icheck		   (OBJECT *me_tree, short me_item, short me_check, short *global);
short 	mt_menu_ienable 	   (OBJECT *me_tree, short me_item, short me_enable, short *global);
short 	mt_menu_istart		   (short me_flag, OBJECT *me_tree, short me_imenu, short me_item, short *global);
short 	mt_menu_popup		   (MENU *me_menu, short me_xpos, short me_ypos, MENU *me_mdata, short *global);
short 	mt_menu_register	   (short ap_id, const char *me_text, short *global);
short 	mt_menu_settings	   (short me_flag, MN_SET *me_values, short *global);
short 	mt_menu_text		   (OBJECT *me_tree, short me_item, const char *me_text, short *global);
short 	mt_menu_tnormal 	   (OBJECT *me_tree, short me_item, short me_normal, short *global);
short	mt_menu_unregister	   (short id, short *global);
/**@}*/

/** @addtogroup a_objc
 *  @{
 */
short	mt_objc_add		(OBJECT *, short Parent, short Child, short *global);
short	mt_objc_change	(OBJECT *, short Object, short Res,
						 short Cx, short Cy, short Cw, short Ch,
						 short NewState,short Redraw, short *global_aes);
short	mt_objc_change_grect	(OBJECT *, short Start, short Depth, const GRECT *r, short NewState,short Redraw, short *global_aes);
short	mt_objc_delete	(OBJECT *, short Object, short *global_aes);
short	mt_objc_draw	(OBJECT *, short Start, short Depth,
						 short Cx, short Cy, short Cw, short Ch, short *global_aes);
short	mt_objc_draw_grect	(OBJECT *, short Start, short Depth, const GRECT *r, short *global_aes);
short	mt_objc_edit	(OBJECT *, short Object, short Char, short *Index, short Kind, short *global_aes);
short	mt_objc_find	(OBJECT *, short Start, short Depth, short Mx, short My, short *global_aes);
short	mt_objc_xfind	(OBJECT *, short Start, short Depth, short Mx, short My, short *global_aes);
short	mt_objc_offset	(OBJECT *, short Object, short *X, short *Y, short *global_aes);
short	mt_objc_order	(OBJECT *, short Object, short NewPos, short *global_aes);
short	mt_objc_sysvar	(short mode, short which, short in1, short in2, short *out1, short *out2, short *global_aes);
/**@}*/

/** @addtogroup a_rsrc
 *  @{
 */
short	mt_rsrc_free	(short *global_aes);
short	mt_rsrc_gaddr	(short Type, short Index, void *Address, short *global_aes);
short	mt_rsrc_load	(const char *Name, short *global_aes);
short	mt_rsrc_obfix	(OBJECT *, short Index, short *global_aes);
short	mt_rsrc_rcfix	(void *rc_header, short *global_aes);
short	mt_rsrc_saddr	(short Type, short Index, void *Address, short *global_aes);
/**@}*/

/** @addtogroup a_scrp
 *  @{
 */
short	mt_scrp_clear 	(short *global_aes);
short	mt_scrp_read 	(char *Scrappath, short *global_aes);
short	mt_scrp_write 	(const char *Scrappath, short *global_aes);
/**@}*/

/** @addtogroup a_shel
 *  @{
 */
short	mt_shel_envrn 	(char **result, const char *param, short *global_aes);
short	mt_shel_find 	(char *buf, short *global_aes);
short 	mt_shel_get 	(char *Buf, short Len, short *global_aes);
short	mt_shel_help	(short sh_hmode, const char *sh_hfile, const char *sh_hkey, short *global_aes);
short 	mt_shel_put		(const char *Buf, short Len, short *global_aes);
void	mt_shel_rdef	(char *lpcmd, char *lpdir, short *global_aes);
short	mt_shel_read 	(char *Command, char *Tail, short *global_aes);
void	mt_shel_wdef	(const char *lpcmd, const char *lpdir, short *global_aes);
short	mt_shel_write	(short Exit, short Graphic, short Aes, const void *Command, const char *Tail, short *global_aes);
/**@}*/

/** @addtogroup a_wind
 *  @{
 */
short	mt_wind_calc 	(short Type, short Parts, short InX, short InY, short InW, short InH, short *OutX, short *OutY, short *OutW, short *OutH, short *global_aes);
short	mt_wind_close 	(short WindowHandle, short *global_aes);
short	mt_wind_create 	(short Parts, short Wx, short Wy, short Ww, short Wh, short *global_aes);
short	mt_wind_xcreate	(short Parts, short Wx, short Wy, short Ww, short Wh, short *OutX, short *OutY, short *OutW, short *OutH, short *global_aes);
short	mt_wind_delete 	(short WindowHandle, short *global_aes);
short	mt_wind_draw 	(short WindowHandle, short startob, short *global_aes);
short	mt_wind_find 	(short X, short Y, short *global_aes);
short	mt_wind_get 	(short WindowHandle, short What, short *W1, short *W2, short *W3, short *W4, short *global_aes);
short	mt_wind_new 	(short *global_aes);
short	mt_wind_open 	(short WindowHandle, short Wx, short Wy, short Ww, short Wh, short *global_aes);
short	mt_wind_set 	(short WindowHandle, short What, short W1, short W2, short W3, short W4, short *global_aes);
short	mt_wind_update 	(short Code, short *global_aes);

/*
 * Some useful extensions
 */
short	mt_wind_calc_grect   (short Type, short Parts, const GRECT *In, GRECT *Out, short *global_aes);
short	mt_wind_create_grect (short Parts, const GRECT *r, short *global_aes);
short	mt_wind_xcreate_grect (short Parts, const GRECT *r, GRECT *ret, short *global_aes);
short	mt_wind_get_grect    (short WindowHandle, short What, GRECT *r, short *global_aes);
short	mt_wind_xget_grect   (short WindowHandle, short What, const GRECT *clip, GRECT *r, short *global_aes);
short	mt_wind_open_grect   (short WindowHandle, const GRECT *r, short *global_aes);
short	mt_wind_set_grect    (short WindowHandle, short What, const GRECT *r, short *global_aes);
short	mt_wind_xset_grect   (short WindowHandle, short What, const GRECT *s, GRECT *r, short *global_aes);
short	mt_wind_set_str      (short WindowHandle, short What, const char *str, short *global_aes);
short   mt_wind_get_int      (short WindowHandle, short What, short *W1, short *global_aes);
short   mt_wind_get_ptr      (short WindowHandle, short What, void **v, short *global_aes);
short   mt_wind_set_int      (short WindowHandle, short What, short W1, short *global_aes);
short   mt_wind_set_ptr      (short WindowHandle, short What, void *v, short *global_aes);
short   mt_wind_set_ptr_int  (short WindowHandle, short What, void *v, short W3, short *global_aes);
/**@}*/

/** @addtogroup a_util
 *  @{
 */
void	rc_copy        (const GRECT *src, GRECT *dst);
short	rc_equal       (const GRECT *r1,  const GRECT *r2);
short	rc_intersect   (const GRECT *src, GRECT *dst);
GRECT * array_to_grect (const short *array, GRECT *area);
short * grect_to_array (const GRECT *area, short *array);
/**@}*/


/** @addtogroup AES
 *  @{
 */

/*
 * aes trap interface
 */

/* Array sizes in aes control block */

/** size of the aes_control[] array */
#define AES_CTRLMAX		5
/** size of the aes_global[] array */
#define AES_GLOBMAX		16
/** size of the aes_intin[] array */
#define AES_INTINMAX 		16
/** size of the aes_intout[] array */
#define AES_INTOUTMAX		16
/** size of the aes_addrin[] array */
#define AES_ADDRINMAX		16
/** size of the aes_addrout[] array */
#define AES_ADDROUTMAX		16

/** AES version number */
#define mt_AESversion(aes_global)   (aes_global[0])

/** Number of concurrent applications possible (normally 1).
    MultiTOS will return -1. */
#define mt_AESnumapps(aes_global)   (aes_global[1])

/** Application identifier (same as mt_appl_init() return value). */
#define mt_AESapid(aes_global)          (aes_global[2])

/** LONG global available for use by the application */
#define mt_AESappglobal(aes_global) (*((long *)&aes_global[3]))

/** Pointer to the base of the trees loaded via rsrc_load(). */
#define	mt_AESrscfile(aes_global)   (*((OBJECT ***)&aes_global[5]))

/** Pointer to the memory of the resource loaded via mt_rsrc_load(). */
#define	mt_AESrscmem(aes_global)   (*((void **)&aes_global[7]))

/* length of that memory. Only valid for resource files <64K */
#define	mt_AESrsclen(aes_global)   (aes_global[9])

/** Current maximum character used by the AES to do vst_height() prior to
    writing to the screen. This entry is only present as of AES version 0x0400.*/
#define mt_AESmaxchar(aes_global)   (aes_global[13])

/** Current minimum character used by the AES to do vst_height() prior to
    writing to the screen. This entry is only present as of AES version 0x0400.*/
#define mt_AESminchar(aes_global)   (aes_global[14])

/** AES parameter block structure */
typedef struct
{
	short       *control;   /**< aes_control[] array */
	short       *global;    /**< aes_global[]  array */
	const short *intin;     /**< aes_intin[]   array */
	short       *intout;    /**< aes_intout[]  array */
	const long  *addrin;    /**< aes_addrin[]  array */
	long        *addrout;   /**< aes_addrout[] array */
} AESPB;

/** perform AES trap */
extern void aes (AESPB *pb);

/**@}*/

#endif /* AES */



#ifdef __GEMLIB_DEFS

/*******************************************************************************
 * The VDI specific stuff from old gemfast.h
 */

/* normal graphics drawing modes */
#define MD_REPLACE		1	/**< TODO */
#define MD_TRANS		2	/**< TODO */
#define MD_XOR			3	/**< TODO */
#define MD_ERASE		4	/**< TODO */

/* bit blt rules */
#define ALL_WHITE		0	/**< TODO */
#define S_AND_D			1	/**< TODO */
#define S_AND_NOTD		2	/**< TODO */
#define S_ONLY			3	/**< TODO */
#define NOTS_AND_D		4	/**< TODO */
#define D_ONLY			5	/**< TODO */
#define S_XOR_D			6	/**< TODO */
#define S_OR_D			7	/**< TODO */
#define NOT_SORD		8	/**< TODO */
#define NOT_SXORD		9	/**< TODO */
#define D_INVERT		10	/**< TODO */
#define NOT_D			10	/**< TODO */
#define S_OR_NOTD		11	/**< TODO */
#define NOT_S			12	/**< TODO */
#define NOTS_OR_D		13	/**< TODO */
#define NOT_SANDD		14	/**< TODO */
#define ALL_BLACK		15	/**< TODO */

/* v_bez modes */
#define BEZ_BEZIER		0x01	/**< TODO */
#define BEZ_POLYLINE	0x00	/**< TODO */
#define BEZ_NODRAW		0x02	/**< TODO */

/* v_bit_image modes */
#define IMAGE_LEFT		0	/**< TODO */
#define IMAGE_CENTER	1	/**< TODO */
#define IMAGE_RIGHT		2	/**< TODO */
#define IMAGE_TOP 		0	/**< TODO */
#define IMAGE_BOTTOM	2	/**< TODO */

/* v_justified modes */
#define NOJUSTIFY		0	/**< TODO */
#define JUSTIFY			1	/**< TODO */

/* vq_color modes */
#define COLOR_REQUESTED		0	/**< TODO */
#define COLOR_ACTUAL		1	/**< TODO */

/* return values for vq_vgdos() inquiry */
#define GDOS_NONE		(-2L) 		 /* no GDOS installed */	/**< TODO */
#define GDOS_FSM		0x5F46534DL /* '_FSM' */	/**< TODO */
#define GDOS_FNT		0x5F464E54L /* '_FNT' */	/**< TODO */

/* vqin_mode & vsin_mode modes */
#define VINMODE_LOCATOR		1	/**< TODO */
#define VINMODE_VALUATOR	2	/**< TODO */
#define VINMODE_CHOICE		3	/**< TODO */
#define VINMODE_STRING		4	/**< TODO */

#ifdef __GEMLIB_OLDNAMES
#define LOCATOR			1	/**< TODO */
#define VALUATOR		2	/**< TODO */
#define CHOICE			3	/**< TODO */
#define STRING			4	/**< TODO */
#endif

/* vqt_cachesize modes */
#define CACHE_CHAR		0	/**< TODO */
#define CACHE_MISC		1	/**< TODO */

/* vqt_devinfo return values */
#define DEV_MISSING		0	/**< TODO */
#define DEV_INSTALLED	1	/**< TODO */

/* vqt_name return values */
#define BITMAP_FONT		0	/**< TODO */

/* vsf_interior modes */
#define FIS_HOLLOW		0	/**< TODO */
#define FIS_SOLID		1	/**< TODO */
#define FIS_PATTERN		2	/**< TODO */
#define FIS_HATCH		3	/**< TODO */
#define FIS_USER		4	/**< TODO */

/* vsf_perimeter modes */
#define PERIMETER_OFF		0	/**< TODO */
#define PERIMETER_ON		1	/**< TODO */

/* vsl_ends modes */
#define SQUARE			0	/**< TODO */
#define ARROWED			1	/**< TODO */
#define ROUND			2	/**< TODO */

/* other names */
#define LE_SQUARED		SQUARE	/**< TODO */
#define LE_ARROWED		ARROWED	/**< TODO */
#define LE_ROUNDED		ROUND	/**< TODO */

/* vsl_type modes */
#define SOLID			1	/**< TODO */
#define LDASHED			2	/**< TODO */
#define DOTTED			3	/**< TODO */
#define DASHDOT			4	/**< TODO */
#define DASH			5	/**< TODO */
#define DASHDOTDOT		6	/**< TODO */
#define USERLINE		7	/**< TODO */

/* other names */
#define LT_SOLID		SOLID		/**< TODO */
#define LT_LONGDASH		LDASHED		/**< TODO */
#define LT_DOTTED		DOTTED		/**< TODO */
#define LT_DASHDOT		DASHDOT		/**< TODO */
#define LT_DASHED		DASH		/**< TODO */
#define LT_DASHDOTDOT	DASHDOTDOT	/**< TODO */
#define LT_USERDEF		USERLINE	/**< TODO */
#define LONGDASH		LDASHED		/**< TODO */
#define DOT				DOTTED		/**< TODO */
#define DASH2DOT		DASHDOTDOT	/**< TODO */

/* vsm_type modes */
#define MRKR_DOT		1	/**< TODO */
#define MRKR_PLUS 		2	/**< TODO */
#define MRKR_ASTERISK	3	/**< TODO */
#define MRKR_BOX		4	/**< TODO */
#define MRKR_CROSS		5	/**< TODO */
#define MRKR_DIAMOND	6	/**< TODO */

/* other names */
#define MT_DOT			MRKR_DOT		/**< TODO */
#define MT_PLUS			MRKR_PLUS		/**< TODO */
#define MT_ASTERISK		MRKR_ASTERISK	/**< TODO */
#define MT_SQUARE		MRKR_BOX		/**< TODO */
#define MT_DCROSS		MRKR_CROSS		/**< TODO */
#define MT_DIAMOND		MRKR_DIAMOND	/**< TODO */

/* vst_alignment modes */
#define TA_LEFT         	0 /* horizontal */	/**< TODO */
#define TA_CENTER       	1	/**< TODO */
#define TA_RIGHT        	2	/**< TODO */
#define TA_BASE         	0 /* vertical */	/**< TODO */
#define TA_HALF         	1	/**< TODO */
#define TA_ASCENT       	2	/**< TODO */
#define TA_BOTTOM       	3	/**< TODO */
#define TA_DESCENT      	4	/**< TODO */
#define TA_TOP          	5	/**< TODO */

/* vst_charmap modes */
#define MAP_BITSTREAM		0	/**< TODO */
#define MAP_ATARI			1	/**< TODO */
#define MAP_UNICODE			2 /* for vst_map_mode, NVDI 4 */	/**< TODO */

/* vst_effects modes */
#define TXT_NORMAL			0x0000	/**< TODO */
#define TXT_THICKENED		0x0001	/**< TODO */
#define TXT_LIGHT			0x0002	/**< TODO */
#define TXT_SKEWED			0x0004	/**< TODO */
#define TXT_UNDERLINED		0x0008	/**< TODO */
#define TXT_OUTLINED		0x0010	/**< TODO */
#define TXT_SHADOWED		0x0020	/**< TODO */

/* other names */
#define	TF_NORMAL			TXT_NORMAL		/**< TODO */
#define TF_THICKENED		TXT_THICKENED	/**< TODO */
#define TF_LIGHTENED		TXT_LIGHT		/**< TODO */
#define TF_SLANTED			TXT_SKEWED		/**< TODO */
#define TF_UNDERLINED		TXT_UNDERLINED	/**< TODO */
#define TF_OUTLINED			TXT_OUTLINED	/**< TODO */
#define TF_SHADOWED			TXT_SHADOWED	/**< TODO */


/* vst_error modes */
#define APP_ERROR			0	/**< TODO */
#define SCREEN_ERROR		1	/**< TODO */

/* vst_error return values */
#define NO_ERROR			0	/**< TODO */
#define CHAR_NOT_FOUND		1	/**< TODO */
#define FILE_READERR 		8	/**< TODO */
#define FILE_OPENERR 		9	/**< TODO */
#define BAD_FORMAT			10	/**< TODO */
#define CACHE_FULL			11	/**< TODO */
#define MISC_ERROR			(-1)	/**< TODO */

/* vst_kern tmodes */
#define TRACK_NONE			0	/**< TODO */
#define TRACK_NORMAL 		1	/**< TODO */
#define TRACK_TIGHT			2	/**< TODO */
#define TRACK_VERYTIGHT		3	/**< TODO */

/* vst_kern pmodes */
#define PAIR_OFF		0	/**< TODO */
#define PAIR_ON			1	/**< TODO */

/* vst_scratch modes */
#define SCRATCH_BOTH		0	/**< TODO */
#define SCRATCH_BITMAP		1	/**< TODO */
#define SCRATCH_NONE		2	/**< TODO */

/* v_updwk return values */
#define SLM_OK			0	/* no error */
#define SLM_ERROR		2	/* general printer error */
#define SLM_NOTONER		3	/* toner empty */
#define SLM_NOPAPER		5	/* paper empty */

/** VDI Memory Form Definition Block */
#ifndef __MFDB__
#define __MFDB__
typedef struct memory_form
{
	void	*fd_addr;	/**< TODO */
	short 	fd_w;		/**< Form Width in Pixels */
	short 	fd_h; 		/**< Form Height in Pixels */
	short 	fd_wdwidth;	/**< Form Width in shorts(fd_w/sizeof(short) */
	short 	fd_stand;	/**< Form format 0= device spec 1=standard */
	short 	fd_nplanes;	/**< Number of memory planes */
	short 	fd_r1;		/**< Reserved */
	short 	fd_r2;		/**< Reserved */
	short 	fd_r3;		/**< Reserved */
} MFDB;
#endif

/** RGB intesities in promille */
typedef struct rgb_1000
{
	short  red;    /**< Red-Intensity in range [0..1000] */
 	short  green;  /**< Green-Intensity in range [0..1000] */
 	short  blue;   /**< Blue-Intensity in range [0..1000] */
} RGB1000;

#endif

#ifdef __GEMLIB_VDI

/*******************************************************************************
 * The VDI bindings from old vdibind.h
 */

typedef short VdiHdl;   /**< for better readability */


/*
 * attribute functions
 */

/** @addtogroup v_attr
 *  @{
 */
void  vs_color  (VdiHdl , short color_idx, const short rgb[]);
short vswr_mode (VdiHdl , short mode);

short vsf_color     (VdiHdl , short color_idx);
short vsf_interior  (VdiHdl , short style);
short vsf_perimeter (VdiHdl , short vis);
short vsf_xperimeter(VdiHdl , short vis, short style);
short vsf_style     (VdiHdl , short style);
void  vsf_udpat     (VdiHdl , short pat[], short planes);

short vsl_color (VdiHdl , short color_idx);
void  vsl_ends  (VdiHdl , short begstyle, short endstyle);
short vsl_type  (VdiHdl , short style);
void  vsl_udsty (VdiHdl , short pat);
short vsl_width (VdiHdl , short width);

short vsm_color  (VdiHdl , short color_idx);
short vsm_height (VdiHdl , short height);
short vsm_type   (VdiHdl , short symbol);

void  vst_alignment (VdiHdl , short hin, short vin, short *hout, short *vout);
short vst_color     (VdiHdl , short color_idx);
short vst_effects   (VdiHdl , short effects);

void  vst_error     (VdiHdl , short mode, short *errorvar);

short vst_font      (VdiHdl , short font);
void  vst_height    (VdiHdl , short height, short *charw, short *charh,
                                            short *cellw, short *cellh);
short vst_point     (VdiHdl , short point, short *charw, short *charh,
                                           short *cellw, short *cellh);
short vst_rotation  (VdiHdl , short ang);
void  vst_scratch   (VdiHdl , short mode);

/**@}*/


/** UDEF version of vs_color(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vs_color vs_color
/** UDEF version of vswr_mode(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vswr_mode vswr_mode

/** UDEF version of vsf_color(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vsf_color vsf_color
/** UDEF version of vsf_interior(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vsf_interior vsf_interior
/** UDEF version of vsf_perimeter(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vsf_perimeter vsf_perimeter
/** UDEF version of vsf_xperimeter(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vsf_xperimeter vsf_xperimeter
/** UDEF version of vsf_style(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vsf_style vsf_style
/** UDEF version of vsf_udpat(). See \ref overviewUDEF for details about UDEF feature */
#define  udef_vsf_udpat vsf_udpat

/** UDEF version of vsl_color(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vsl_color vsl_color
/** UDEF version of vsl_ends(). See \ref overviewUDEF for details about UDEF feature */
#define  udef_vsl_ends vsl_ends
/** UDEF version of vsl_type(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vsl_type vsl_type
/** UDEF version of vsl_udsty(). See \ref overviewUDEF for details about UDEF feature */
#define  udef_vsl_udsty vsl_udsty
/** UDEF version of vsl_width(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vsl_width vsl_width

/** UDEF version of vsm_color(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vsm_color vsm_color
/** UDEF version of vsm_height(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vsm_height vsm_height
/** UDEF version of vsm_type(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vsm_type vsm_type

/** UDEF version of vst_alignment(). See \ref overviewUDEF for details about UDEF feature */
#define  udef_vst_alignment vst_alignment
/** UDEF version of vst_color(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vst_color vst_color
/** UDEF version of vst_effects(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vst_effects vst_effects

/** @ingroup force_udef */
void  udef_vst_error     (VdiHdl , short mode, short *errorvar);

/** UDEF version of vst_font(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vst_font vst_font
/** UDEF version of vst_height(). See \ref overviewUDEF for details about UDEF feature */
#define  udef_vst_height vst_height
/** UDEF version of vst_point(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vst_point vst_point
/** UDEF version of vst_rotation(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vst_rotation vst_rotation
/** UDEF version of vst_scratch(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vst_scratch vst_scratch


#ifdef FORCE_GEMLIB_UDEF
#define vst_error udef_vst_error
#endif


/*
 * control functions
 */

/** @addtogroup v_ctrl
 *  @{
 */
void  v_clrwk          (VdiHdl );
void  v_clsvwk         (VdiHdl );
void  v_clswk          (VdiHdl );
short v_flushcache     (VdiHdl );
short v_loadcache      (VdiHdl , const char *filename, short mode);
void  v_opnvwk         (short work_in[], VdiHdl *, short work_out[]);
void  v_opnwk          (short work_in[], VdiHdl *, short work_out[]);
short v_savecache      (VdiHdl , const char *filename);
void  v_set_app_buff   (VdiHdl , void *buf_p, short size);
void  v_updwk          (VdiHdl );
void  vs_clip          (VdiHdl , short clip_flag, short pxy[]);
void  vs_clip_pxy      (VdiHdl , PXY pxy[]);
void  vs_clip_off      (VdiHdl );
short vst_load_fonts   (VdiHdl , short /* select */);
void  vst_unload_fonts (VdiHdl , short /* select */);
/**@}*/

/** UDEF version of v_clrwk(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_clrwk v_clrwk
/** UDEF version of v_clsvwk(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_clsvwk v_clsvwk
/** UDEF version of v_clswk(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_clswk v_clswk
/** UDEF version of v_flushcache(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_flushcache v_flushcache
/** @ingroup force_udef */
short udef_v_loadcache      (VdiHdl , const char *filename, short mode);
/** UDEF version of v_opnvwk(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_opnvwk v_opnvwk
/** UDEF version of v_opnwk(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_opnwk v_opnwk
/** @ingroup force_udef */
short udef_v_savecache      (VdiHdl , const char *filename);
/** UDEF version of v_set_app_buff(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_set_app_buff v_set_app_buff
/** UDEF version of v_updwk(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_updwk v_updwk
/** UDEF version of vs_clip(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vs_clip vs_clip
/** UDEF version of vs_clip_pxy(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vs_clip_pxy vs_clip_pxy
/** UDEF version of vs_clip_off(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vs_clip_off vs_clip_off
/** UDEF version of vst_load_fonts(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vst_load_fonts vst_load_fonts
/** UDEF version of vst_unload_fonts(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vst_unload_fonts vst_unload_fonts

#ifdef FORCE_GEMLIB_UDEF
#define v_loadcache udef_v_loadcache
#define v_savecache udef_v_savecache
#endif

/*
 * escape functions
 */

/** @addtogroup v_escp
 *  @{
 */


void  v_bit_image       (VdiHdl , const char *filename, short aspect,
                                  short x_scale, short y_scale,
                                  short h_align, short v_align, short *pxy);

void  v_clear_disp_list (VdiHdl );
short v_copies          (VdiHdl , short count);
void  v_dspcur          (VdiHdl , short x, short y);
void  v_form_adv        (VdiHdl );
void  v_hardcopy        (VdiHdl );
short v_orient          (VdiHdl , short orientation);
void  v_output_window   (VdiHdl , short *pxy);
short v_page_size       (VdiHdl , short page_id);
void  v_rmcur           (VdiHdl );
short v_trays           (VdiHdl , short input, short output,
                                  short *set_input, short *set_output);
short vq_calibrate      (VdiHdl , short *flag);
short vq_page_name      (VdiHdl , short page_id, char *page_name,
                                  long *page_width, long *page_height);
void  vq_scan           (VdiHdl , short *g_slice, short *g_page,
                                  short *a_slice, short *a_page, short *div_fac);
short vq_tabstatus      (VdiHdl );
short vq_tray_names     (VdiHdl , char *input_name, char *output_name,
                                  short *input, short *output);
short vs_calibrate      (VdiHdl , short flag, short *rgb);
short vs_palette        (VdiHdl , short palette);

void  v_sound		(VdiHdl, short freq, short duration);
short vs_mute		(VdiHdl, short action);

void vq_tdimensions (VdiHdl , short *xdimension, short *ydimension);
void vt_alignment   (VdiHdl , short dx, short dy);
void vt_axis        (VdiHdl , short xres, short yres, short *xset, short *yset);
void vt_origin      (VdiHdl , short xorigin, short yorigin);
void vt_resolution  (VdiHdl , short xres, short yres, short *xset, short *yset);

void v_meta_extents (VdiHdl , short min_x, short min_y,
                              short max_x, short max_y);
void v_write_meta   (VdiHdl , short numvdi_intin, short *avdi_intin,
                              short num_ptsin, short *a_ptsin);
void vm_coords      (VdiHdl , short llx, short lly, short urx, short ury);

void vm_filename    (VdiHdl , const char *filename);
void vm_pagesize    (VdiHdl , short pgwidth, short pgheight);

void  vsc_expose	(VdiHdl , short state);
void  vsp_film		(VdiHdl , short color_idx, short lightness);
short vqp_filmname	(VdiHdl , short _index, char * name);

void v_offset		(VdiHdl , short offset);
void v_fontinit		(VdiHdl , const void * font_header);

void v_escape2000 (VdiHdl , short times);

void v_alpha_text  (VdiHdl , const char *str);
void v_alpha_text16n  (VdiHdl , const short * wstr, short num);
void v_curdown     (VdiHdl );
void v_curhome     (VdiHdl );
void v_curleft     (VdiHdl );
void v_curright    (VdiHdl );
void v_curtext     (VdiHdl , const char *str);
void v_curtext16n     (VdiHdl , const short * wstr, short num);
void v_curup       (VdiHdl );
void v_eeol        (VdiHdl );
void v_eeos        (VdiHdl );
void v_enter_cur   (VdiHdl );
void v_exit_cur    (VdiHdl );
void v_rvoff       (VdiHdl );
void v_rvon        (VdiHdl );
void vq_chcells    (VdiHdl , short *n_rows, short *n_cols);
void vq_curaddress (VdiHdl , short *cur_row, short *cur_col);
void vs_curaddress (VdiHdl , short row, short col);
/** alternative name for vs_curaddress */
#define v_curaddress vs_curaddress

/**@}*/

/** @ingroup force_udef */
void  udef_v_bit_image       (VdiHdl , const char *filename, short aspect,
                                  short x_scale, short y_scale,
                                  short h_align, short v_align, short *pxy);

/** UDEF version of v_clear_disp_list(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_clear_disp_list v_clear_disp_list
/** UDEF version of v_copies(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_copies v_copies
/** UDEF version of v_dspcur(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_dspcur v_dspcur
/** UDEF version of v_form_adv(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_form_adv v_form_adv
/** UDEF version of v_hardcopy(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_hardcopy v_hardcopy
/** UDEF version of v_orient(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_orient v_orient
/** UDEF version of v_output_window(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_output_window v_output_window
/** UDEF version of v_page_size(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_page_size v_page_size
/** UDEF version of v_rmcur(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_rmcur v_rmcur
/** UDEF version of v_trays(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_trays v_trays
/** UDEF version of vq_calibrate(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vq_calibrate vq_calibrate
/** UDEF version of vq_page_name(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vq_page_name vq_page_name
/** UDEF version of vq_scan(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vq_scan vq_scan
/** UDEF version of vq_tabstatus(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vq_tabstatus vq_tabstatus
/** UDEF version of vq_tray_names(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vq_tray_names vq_tray_names
/** UDEF version of vs_calibrate(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vs_calibrate vs_calibrate
/** UDEF version of vs_palette(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vs_palette vs_palette

/** UDEF version of v_sound(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_sound v_sound
/** UDEF version of vs_mute(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vs_mute vs_mute

/** UDEF version of vq_tdimensions(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vq_tdimensions vq_tdimensions
/** UDEF version of vt_alignment(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vt_alignment vt_alignment
/** UDEF version of vt_axis(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vt_axis vt_axis
/** UDEF version of vt_origin(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vt_origin vt_origin
/** UDEF version of vt_resolution(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vt_resolution vt_resolution

/** UDEF version of v_meta_extents(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_meta_extents v_meta_extents
/** UDEF version of v_write_meta(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_write_meta v_write_meta
/** UDEF version of vm_coords(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vm_coords vm_coords
/** @ingroup force_udef */
void udef_vm_filename    (VdiHdl , const char *filename);
/** UDEF version of vm_pagesize(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vm_pagesize vm_pagesize

/** UDEF version of vsc_expose(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vsc_expose vsc_expose
/** UDEF version of vsp_film(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vsp_film vsp_film
/** UDEF version of vqp_filmname(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vqp_filmname vqp_filmname

/** UDEF version of v_offset(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_offset v_offset
/** UDEF version of v_fontinit(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_fontinit v_fontinit

/** UDEF version of v_escape2000(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_escape2000 v_escape2000

/** @ingroup force_udef */
void udef_v_alpha_text  (VdiHdl , const char *str);
/** UDEF version of v_alpha_text16n(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_alpha_text16n v_alpha_text16n
/** UDEF version of v_curdown(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_curdown v_curdown
/** UDEF version of v_curhome(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_curhome v_curhome
/** UDEF version of v_curleft(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_curleft v_curleft
/** UDEF version of v_curright(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_curright v_curright
/** @ingroup force_udef */
void udef_v_curtext     (VdiHdl , const char *str);
/** UDEF version of v_curtext16n(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_curtext16n v_curtext16n
/** UDEF version of v_curup(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_curup v_curup
/** UDEF version of v_eeol(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_eeol v_eeol
/** UDEF version of v_eeos(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_eeos v_eeos
/** UDEF version of v_enter_cur(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_enter_cur v_enter_cur
/** UDEF version of v_exit_cur(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_exit_cur v_exit_cur
/** UDEF version of v_rvoff(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_rvoff v_rvoff
/** UDEF version of v_rvon(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_rvon v_rvon
/** UDEF version of vq_chcells(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vq_chcells vq_chcells
/** UDEF version of vq_curaddress(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vq_curaddress vq_curaddress
/** UDEF version of vs_curaddress(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vs_curaddress vs_curaddress
/** UDEF version of vs_curaddress(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_curaddress udef_vs_curaddress

#ifdef FORCE_GEMLIB_UDEF
#define v_bit_image udef_v_bit_image
#define vm_filename udef_vm_filename
#define v_alpha_text udef_v_alpha_text
#define v_curtext udef_v_curtext
#endif
 

/*
 * inquiry functions
 */

/** @addtogroup v_inqr
 *  @{
 */
void  vq_cellarray   (VdiHdl , short pxy[], short row_len, short nrows,
                               short *el_used, short *rows_used,
                               short *status, short color[]);
short vq_color       (VdiHdl , short color_idx, short flag, short rgb[]);
void  vq_extnd       (VdiHdl , short flag, short work_out[]);
void  vqf_attributes (VdiHdl , short atrib[]);
void  vqin_mode      (VdiHdl , short dev, short *mode);
void  vql_attributes (VdiHdl , short atrib[]);
void  vqm_attributes (VdiHdl , short atrib[]);
void  vqt_attributes (VdiHdl , short atrib[]);
void  vqt_cachesize  (VdiHdl , short which_cache, long *size);
void  vqt_extent     (VdiHdl , const char *str, short extent[]);
void  vqt_extentn    (VdiHdl , const char *str, short num, short extent[]);
void  vqt_extent16   (VdiHdl , const short *wstr, short extent[]);
void  vqt_extent16n  (VdiHdl , const short *wstr, short num, short extent[]);
void  vqt_fontinfo   (VdiHdl , short *minade, short *maxade, short distances[],
                               short *maxwidth, short effects[]);
void  vqt_get_table  (VdiHdl , short **map);
short vqt_name       (VdiHdl , short element, char *name);
short vqt_width      (VdiHdl , short chr, short *cw,
                               short *ldelta, short *rdelta);
/** check if a GDOS is installed
 *
 * @return 1 if any version of GDOS is installed, otherwise 0
 *
 * @sa vq_vgdos()
 */
short vq_gdos  (void);

/** check which GDOS version is installed
 *
 * @return one of the following values:
 *        - #GDOS_NONE: GDOS is not installed
 *        - #GDOS_FNT: FontGDOS is installed
 *        - #GDOS_FSM: FSM GDOS or SpeedoGDOS is installed; check the 'FSMC' cookie to find which one
 *        - any other value: GDOS 1.x is installed
 */
long  vq_vgdos (void);
/**@}*/


/** UDEF version of vq_cellarray(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vq_cellarray vq_cellarray
/** UDEF version of vq_color(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vq_color vq_color
/** UDEF version of vq_extnd(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vq_extnd vq_extnd
/** UDEF version of vqf_attributes(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vqf_attributes vqf_attributes
/** UDEF version of vqin_mode(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vqin_mode vqin_mode
/** UDEF version of vql_attributes(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vql_attributes vql_attributes
/** UDEF version of vqm_attributes(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vqm_attributes vqm_attributes
/** UDEF version of vqt_attributes(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vqt_attributes vqt_attributes
/** UDEF version of vqt_cachesize(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vqt_cachesize vqt_cachesize
/** @ingroup force_udef */
void  udef_vqt_extent     (VdiHdl , const char *str, short extent[]);
/** UDEF version of vqt_extent16(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vqt_extent16 vqt_extent16
/** UDEF version of vqt_extent16n(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vqt_extent16n vqt_extent16n
/** UDEF version of vqt_fontinfo(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vqt_fontinfo vqt_fontinfo
/** UDEF version of vqt_get_table(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vqt_get_table vqt_get_table
/** UDEF version of vqt_name(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vqt_name vqt_name
/** UDEF version of vqt_width(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vqt_width vqt_width
/** UDEF version of vq_gdos(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vq_gdos vq_gdos
/** UDEF version of vq_vgdos(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vq_vgdos vq_vgdos

#ifdef FORCE_GEMLIB_UDEF
#define vqt_extent udef_vqt_extent
#endif



/*
 * input function
 */

/** @addtogroup v_inpt
 *  @{
 */
void  v_hide_c     (VdiHdl );
void  v_show_c     (VdiHdl , short reset);
void  vex_butv     (VdiHdl , void *pusrcode, void **psavcode);
void  vex_curv     (VdiHdl , void *pusrcode, void **psavcode);
void  vex_motv     (VdiHdl , void *pusrcode, void **psavcode);
void  vex_wheelv   (VdiHdl , void *pusrcode, void **psavcode);
void  vex_timv     (VdiHdl , void *time_addr,
                             void **otime_addr, short *time_conv);
void  vq_key_s     (VdiHdl , short *state);
void  vq_mouse     (VdiHdl , short *pstatus, short *x, short *y);
void  vrq_choice   (VdiHdl , short cin, short *cout);
void  vrq_locator  (VdiHdl , short x, short y,
                             short *xout, short *yout, short *term);
void  vrq_string   (VdiHdl , short len, short echo, short echoxy[], char *str);
void  vrq_valuator (VdiHdl , short in, short *out, short *term);
void  vsc_form     (VdiHdl , short form[]);
short vsin_mode    (VdiHdl , short dev, short mode);
short vsm_choice   (VdiHdl , short *choice);
short vsm_locator  (VdiHdl , short x, short y,
                             short *xout, short *yout, short *term);
short vsm_string   (VdiHdl , short len, short echo, short echoxy[], char *str);
void  vsm_valuator (VdiHdl , short in, short *out, short *term, short *status);
/**@}*/


/** UDEF version of v_hide_c(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_hide_c v_hide_c
/** UDEF version of v_show_c(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_show_c v_show_c
/** UDEF version of vex_butv(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vex_butv vex_butv
/** UDEF version of vex_curv(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vex_curv vex_curv
/** UDEF version of vex_motv(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vex_motv vex_motv
/** UDEF version of vex_wheelv(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vex_wheelv vex_wheelv
/** UDEF version of vex_timv(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vex_timv vex_timv
/** UDEF version of vq_key_s(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vq_key_s vq_key_s
/** UDEF version of vq_mouse(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vq_mouse vq_mouse
/** UDEF version of vrq_choice(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vrq_choice vrq_choice
/** UDEF version of vrq_locator(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vrq_locator vrq_locator
/** @ingroup force_udef */
void  udef_vrq_string   (VdiHdl , short len, short echo, short echoxy[], char *str);
/** UDEF version of vrq_valuator(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vrq_valuator vrq_valuator
/** UDEF version of vsc_form(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vsc_form vsc_form
/** UDEF version of vsin_mode(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vsin_mode vsin_mode
/** UDEF version of vsm_choice(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vsm_choice vsm_choice
/** UDEF version of vsm_locator(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vsm_locator vsm_locator
/** @ingroup force_udef */
short udef_vsm_string   (VdiHdl , short len, short echo, short echoxy[], char *str);
/** UDEF version of vsm_valuator(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vsm_valuator vsm_valuator


#ifdef FORCE_GEMLIB_UDEF
#define vrq_string udef_vrq_string
#define vsm_string udef_vsm_string
#endif



/*
 * output functions
 */

/** @addtogroup v_outp
 *  @{
 */
void v_arc         (VdiHdl , short x, short y,
                             short radius, short begang, short endang);
void v_bar         (VdiHdl , short pxy[]);
void v_cellarray   (VdiHdl , short pxy[], short row_length, short elements,
                             short nrows, short write_mode, short colarray[]);
void v_circle      (VdiHdl , short x, short y, short radius);
void v_contourfill (VdiHdl , short x, short y, short color_idx);
void v_ellarc      (VdiHdl , short x, short y, short xrad, short yrad,
                             short begang, short endang);
void v_ellipse     (VdiHdl , short x, short y, short xrad, short yrad);
void v_ellpie      (VdiHdl , short x, short y, short xrad, short yrad,
                             short begang, short endang);
void v_fillarea    (VdiHdl , short count, short pxy[]);
void v_gtext       (VdiHdl , short x, short y, const char *str);
void v_gtextn      (VdiHdl , short x, short y, const char *str, short num);
void v_gtext16     (VdiHdl , short x, short y, const short *wstr);
void v_gtext16n    (VdiHdl , short x, short y, const short *wstr, short num);
/* old version, using an PXY parameter */
#define v_gtext16n_pxy(h, pos, wstr, num) v_gtext16n(h, (pos).p_x, (pos).p_y, wstr, num)
void v_justified   (VdiHdl , short x, short y, const char *str,
                             short len, short word_space, short char_space);
void v_justified16n   (VdiHdl , short x, short y, const short * wstr, short num,
                             short len, short word_space, short char_space);
void v_pieslice    (VdiHdl , short x, short y,
                             short radius, short begang, short endang);
void v_pline       (VdiHdl , short count, short pxy[]);
void v_pmarker     (VdiHdl , short count, short pxy[]);
void v_rbox        (VdiHdl , short pxy[]);
void v_rfbox       (VdiHdl , short pxy[]);
void vr_recfl      (VdiHdl , short pxy[]);
/**@}*/

/** UDEF version of v_arc(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_arc v_arc
/** UDEF version of v_bar(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_bar v_bar
/** UDEF version of v_cellarray(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_cellarray v_cellarray
/** UDEF version of v_circle(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_circle v_circle
/** UDEF version of v_contourfill(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_contourfill v_contourfill
/** UDEF version of v_ellarc(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_ellarc v_ellarc
/** UDEF version of v_ellipse(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_ellipse v_ellipse
/** UDEF version of v_ellpie(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_ellpie v_ellpie
/** UDEF version of v_fillarea(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_fillarea v_fillarea
/** @ingroup force_udef */
void udef_v_gtext       (VdiHdl , short x, short y, const char *str);
/** UDEF version of v_gtext16(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_gtext16 v_gtext16
/** UDEF version of v_gtext16n(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_gtext16n v_gtext16n
/** @ingroup force_udef */
void udef_v_justified   (VdiHdl , short x, short y, const char *str,
                             short len, short word_space, short char_space);
/** UDEF version of v_justified16n(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_justified16n v_justified16n
/** UDEF version of v_pieslice(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_pieslice v_pieslice
/** UDEF version of v_pline(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_pline v_pline
/** UDEF version of v_pmarker(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_pmarker v_pmarker
/** UDEF version of v_rbox(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_rbox v_rbox
/** UDEF version of v_rfbox(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_rfbox v_rfbox
/** UDEF version of vr_recfl(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vr_recfl vr_recfl


#ifdef FORCE_GEMLIB_UDEF
#define v_gtext udef_v_gtext
#define v_justified udef_v_justified
#endif


/*
 * raster functions
 */

/** @addtogroup v_rstr
 *  @{
 */
void v_get_pixel (VdiHdl , short x, short y, short *pel, short *color_idx);
void vr_trnfm    (VdiHdl , MFDB *src, MFDB *dst);
void vro_cpyfm   (VdiHdl , short mode, short pxy[], MFDB *src, MFDB *dst);
void vrt_cpyfm   (VdiHdl , short mode, short pxy[], MFDB *src, MFDB *dst,
                           short color[]);
/**@}*/

/** UDEF version of v_get_pixel(). See \ref overviewUDEF for details about UDEF feature */
#define udef_v_get_pixel v_get_pixel
/** UDEF version of vr_trnfm(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vr_trnfm vr_trnfm
/** UDEF version of vro_cpyfm(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vro_cpyfm vro_cpyfm
/** UDEF version of vrt_cpyfm(). See \ref overviewUDEF for details about UDEF feature */
#define udef_vrt_cpyfm vrt_cpyfm



/*
 * Some usefull extensions.
 */

/** @addtogroup v_util
 *  @{
 */
void  vdi_array2str (const short *src, char  *des, short len);
short vdi_str2array (const char  *src, short *des);
short vdi_str2array_n (const char  *src, short *des, int nmax);
short vdi_wstrlen   (const short *wstr);
/**@}*/

/*
 * vdi trap interface
 */

/** @addtogroup VDI
 *  @{
 */

/* Array sizes in vdi control block as defined in NVDI 4 documentation */
#define VDI_CNTRLMAX		15		/**< max size of vdi_control[] */
#define VDI_INTINMAX		1024	/**< max size of vdi_intin[] */
#define VDI_INTOUTMAX		512		/**< max size of vdi_intout[] */
#define VDI_PTSINMAX		1024	/**< max size of vdi_ptsin[] */
#define VDI_PTSOUTMAX		256		/**< max size of vdi_ptsout[] */

/** TODO */
typedef struct
{
	short       *control;	/**< TODO */
	const short *intin;		/**< TODO */
	const short *ptsin;		/**< TODO */
	short       *intout;	/**< TODO */
	short       *ptsout;	/**< TODO */
} VDIPB;

/** TODO */
void vdi (VDIPB *pb);
/**@}*/


#endif /* VDI */


__END_DECLS

#endif /* _MT_GEMLIB_H_ */
