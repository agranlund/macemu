/*
 * resource set indices for basilisk
 *
 * created by ORCS 2.18
 */

/*
 * Number of Strings:        89
 * Number of Bitblks:        0
 * Number of Iconblks:       0
 * Number of Color Iconblks: 0
 * Number of Color Icons:    0
 * Number of Tedinfos:       10
 * Number of Free Strings:   0
 * Number of Free Images:    0
 * Number of Objects:        88
 * Number of Trees:          3
 * Number of Userblks:       0
 * Number of Images:         0
 * Total file size:          3324
 */

#undef RSC_NAME
#ifndef __ALCYON__
#define RSC_NAME "basilisk"
#endif
#undef RSC_ID
#ifdef basilisk
#define RSC_ID basilisk
#else
#define RSC_ID 0
#endif

#ifndef RSC_STATIC_FILE
# define RSC_STATIC_FILE 0
#endif
#if !RSC_STATIC_FILE
#define NUM_STRINGS 89
#define NUM_FRSTR 0
#define NUM_UD 0
#define NUM_IMAGES 0
#define NUM_BB 0
#define NUM_FRIMG 0
#define NUM_IB 0
#define NUM_CIB 0
#define NUM_TI 10
#define NUM_OBS 88
#define NUM_TREE 3
#endif



#define FORM_MAIN                          0 /* form/dialog */
#define CTL_START                          1 /* BUTTON in tree FORM_MAIN */
#define CTL_ROM                            3 /* BOXTEXT in tree FORM_MAIN */
#define CTL_RAM                            6 /* FBOXTEXT in tree FORM_MAIN */
#define CTL_RAM_MAX                        7 /* BUTTON in tree FORM_MAIN */
#define CTL_CPU                           10 /* BOXTEXT in tree FORM_MAIN */
#define CTL_FPU_ON                        13 /* BUTTON in tree FORM_MAIN */
#define CTL_FPU_OFF                       14 /* BUTTON in tree FORM_MAIN */
#define CTL_MODEL_5                       16 /* BUTTON in tree FORM_MAIN */
#define CTL_MODEL_14                      17 /* BUTTON in tree FORM_MAIN */
#define CTL_SOUND_ON                      21 /* BUTTON in tree FORM_MAIN */
#define CTL_SOUND_OFF                     22 /* BUTTON in tree FORM_MAIN */
#define CTL_DISKS                         23 /* BUTTON in tree FORM_MAIN */
#define CTL_VERSION                       24 /* TEXT in tree FORM_MAIN */

#define MENU_MAIN                          1 /* menu */
#define MENU_T0                            3 /* TITLE in tree MENU_MAIN */
#define MENU_T_FILE                        4 /* TITLE in tree MENU_MAIN */
#define MENU_T_SND                         5 /* TITLE in tree MENU_MAIN */
#define MENU_T_VIDEO                       6 /* TITLE in tree MENU_MAIN */
#define MENU_ABOUT                         9 /* STRING in tree MENU_MAIN */
#define MENU_START                        18 /* STRING in tree MENU_MAIN */
#define MENU_DBG_LOG_OFF                  20 /* STRING in tree MENU_MAIN */
#define MENU_DBG_LOG_FILE                 21 /* STRING in tree MENU_MAIN */
#define MENU_DBG_LOG_SERIAL               22 /* STRING in tree MENU_MAIN */
#define MENU_DBG_LOG_SCREEN               23 /* STRING in tree MENU_MAIN */
#define MENU_DBG_SAFEIRQ                  25 /* STRING in tree MENU_MAIN */
#define MENU_QUIT                         27 /* STRING in tree MENU_MAIN */
#define MENU_SND_ENABLE                   29 /* STRING in tree MENU_MAIN */
#define MENU_SND_DRV_DUMMY                31 /* STRING in tree MENU_MAIN */
#define MENU_SND_DRV_DMA                  32 /* STRING in tree MENU_MAIN */
#define MENU_SND_DRV_YM                   33 /* STRING in tree MENU_MAIN */
#define MENU_SND_DRV_COVOX                34 /* STRING in tree MENU_MAIN */
#define MENU_SND_DRV_MV16                 35 /* STRING in tree MENU_MAIN */
#define MENU_SND_DRV_REPLAY8              36 /* STRING in tree MENU_MAIN */
#define MENU_SND_DRV_REPLAY8S             37 /* STRING in tree MENU_MAIN */
#define MENU_SND_DRV_REPLAY16             38 /* STRING in tree MENU_MAIN */
#define MENU_GFX_EMUDISABLE               40 /* STRING in tree MENU_MAIN */
#define MENU_GFX_MMUACCEL                 41 /* STRING in tree MENU_MAIN */
#define MENU_GFX_CMPACCEL                 42 /* STRING in tree MENU_MAIN */
#define MENU_GFX_MODE_CUR                 44 /* STRING in tree MENU_MAIN */
#define MENU_GFX_MODE_1B                  45 /* STRING in tree MENU_MAIN */
#define MENU_GFX_MODE_4B                  46 /* STRING in tree MENU_MAIN */
#define MENU_GFX_MODE_8B                  47 /* STRING in tree MENU_MAIN */
#define MENU_GFX_MODE_16B                 48 /* STRING in tree MENU_MAIN */

#define MENU_DUMMY                         2 /* menu */




#ifdef __STDC__
#ifndef _WORD
#  ifdef WORD
#    define _WORD WORD
#  else
#    define _WORD short
#  endif
#endif
extern _WORD basilisk_rsc_load(_WORD wchar, _WORD hchar);
extern _WORD basilisk_rsc_gaddr(_WORD type, _WORD idx, void *gaddr);
extern _WORD basilisk_rsc_free(void);
#endif
