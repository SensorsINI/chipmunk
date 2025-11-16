#define MYLIB_G
#define XK_MISCELLANY

/* #define ENABLE_DEBUGGING */

/* Trying to speed up graphics */
#define SAVECURSOR
#undef EXTRA_BUFFERING

/* Support for 24-plane TrueColor X */
#define HIRES
/* #define HIRESDB */   /* debugging only */

#define __TYPES__

#include <stdio.h>
#include <string.h>
#ifdef HIRES
#include <stdlib.h>
#endif  /* HIRES */
#include <math.h>

#if defined(aux)
# include <sys/types.h>   
# include <sys/time.h>
# include <time.h>
#elif defined(__sgi)
# include <sys/types.h>   
# include <sys/time.h>
# include <time.h>
#elif defined(rs6000)
# include <sys/time.h>
# include <time.h>
#elif defined(BSD)
# include <sys/time.h>
#elif defined(linux)
# include <sys/time.h>
#else
# include <time.h>
#endif
#ifdef OS2
# include <sys/timeb.h>
#endif

#include <p2c/p2c.h>
#include <p2c/mylib.h>
  
#include <X11/X.h>
#ifdef OS2
#include <X11/Xlib.h>
#endif  /* OS2 */
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/keysymdef.h>

#ifdef HIRES
/* Current settings for X function parameters */
typedef struct {
    XFontStruct 	*font;
    Cursor		cursor;
    int			fontSize;
    int			depth;
    int			maskmod;
    Window 		window;
} GR_CURRENT;

#ifdef RGBFILE    
/* In case someone wants to implement reading RGB from
   a file like Magic does. */
unsigned long grPixels[256];
#endif  /* RGBFILE */
GR_CURRENT grCurrent;
#endif  /* HIRES */

#if 0
/* previously missing defs in keysymdef */
#define XK_Reset                0x1000FF6C  /* HP -- The shift of Break */
#endif


#if defined(rs6000)
/* previously missing defs in keysymdef -- may be needed for RS/6000 */
#define XK_System               0x1000FF6D  /* HP */
#define XK_User                 0x1000FF6E  /* HP */
#define XK_ClearLine            0x1000FF6F  /* HP */
#define XK_InsertLine           0x1000FF70  /* HP */
#define XK_DeleteLine           0x1000FF71  /* HP */
#define XK_InsertChar           0x1000FF72  /* HP */
#define XK_DeleteChar           0x1000FF73  /* HP */
#define XK_BackTab              0x1000FF74  /* HP */
#define XK_KP_BackTab           0x1000FF75  /* HP */
#endif


/* daveg, 10/6/89:  Just to improve readability of the rest of the code! */

static int show_all_mylib_calls,
           show_all_X_calls,
	   show_flushes,
	   show_pen_calls,
	   show_key_calls,
	   sync_all_calls;

/* Static variables to track last known good window size for geometry persistence */
static unsigned int last_window_width = 0;
static unsigned int last_window_height = 0;

static void init_debug_flags()
{
  show_all_mylib_calls = (getenv("SHOW_ALL_MYLIB_CALLS") != NULL);
  show_all_X_calls = (getenv("SHOW_ALL_X_CALLS") != NULL);
  show_flushes = (getenv("SHOW_FLUSHES") != NULL);
  show_pen_calls = (getenv("SHOW_PEN_CALLS") != NULL);
  show_key_calls = (getenv("SHOW_KEY_CALLS") != NULL);
  sync_all_calls = (getenv("SYNC_ALL_CALLS") != NULL);
}

extern void nc_refreshScreen PP((void));
extern void nc_cursor_on PP((void));
extern void nc_cursor_off PP((void));

#define zfprintf fprintf        /* MDG */
extern int zfprintf(FILE *, const char *, ... );

#ifdef ENABLE_DEBUGGING
# define Mfprintf show_all_mylib_calls && fprintf
# define Xfprintf show_all_X_calls && fprintf
# define Ffprintf show_flushes && fprintf
# define Pfprintf show_pen_calls && fprintf
# define Kfprintf show_key_calls && fprintf
#else
# define Mfprintf 0 && fprintf
# define Xfprintf 0 && fprintf
# define Ffprintf 0 && fprintf
# define Pfprintf 0 && fprintf
# define Kfprintf 0 && fprintf
#endif

/* #define XFlush(d)  if (sync_all_calls) XSync(d,False); else XFlush(d) */



/*  newcrt stuff  */
#define nc_fontwidth 8
#define nc_fontheight 13
extern Window nc_window;
extern int nc_initialized;




#define MIN(a,b)     ((a)<(b) ? (a) : (b))
#define MAX(a,b)     ((a)>(b) ? (a) : (b))

#define TRNSFRM(x,y)	 if (flip)                          \
			   y = m_down-(y);                  \
			 else if (trans)                    \
			   GeneralTransform(&(x), &(y));
#define DTRNSFRM(x,y)	 if (flip)                          \
			   y = -y;                          \
			 else if (trans)                    \
			   DeltaTransform(&(x), &(y));
#define LTRNSFRM(x,y)	 if (flip)                          \
			   y = m_down-(y);                  \
			 else if (trans)                    \
			   LimitedTransform(&(x), &(y));
#define UNTRNSFRM(x,y)	 if (flip)                          \
			   y = m_down-(y);                  \
			 else if (trans)                    \
			   GeneralUnTransform(&(x), &(y));
#define pi               3.1415926535897932384626433833    
#define dr               0.0174532925199433
#define WinBorder	 1
#define WinDepth         4

#if defined(MYLIB_FONT)
#define DefaultFont      MYLIB_FONT
#else
#define DefaultFont      "6x10"
#endif


#ifdef OS2
static struct timeb first, second, lapsed;
#else   
static struct timeval first, second, lapsed;
static struct timezone tzp;
#endif

static char *timername;

void starttimer(name)
char *name;
{

  timername = name;

#ifdef OS2
  _ftime(&first);
#else
  gettimeofday(&first, &tzp);
#endif

}

void stoptimer()
{

#ifdef OS2
  _ftime(&second);

  if (first.millitm  > second.millitm) {
     second.millitm += 1000;
     second.time--;
  }
  lapsed.time = second.time - first.time;
  lapsed.millitm = second.millitm - first.millitm;
  printf("%s:  %f seconds\n", timername,
                              lapsed.time+lapsed.millitm/1000.0);
 
#else

  gettimeofday(&second, &tzp);

  if (first.tv_usec > second.tv_usec) {
    second.tv_usec += 1000000;
    second.tv_sec--;
  }
  lapsed.tv_usec = second.tv_usec-first.tv_usec;
  lapsed.tv_sec = second.tv_sec-first.tv_sec;

  printf("%s:  %f seconds\n", timername,
                              lapsed.tv_sec+lapsed.tv_usec/1000000.0);
#endif
}

/* Added for command line display specification. - stafford 7/17/91 */

#define       DISPLAY_NAME_LENGTH 100
char  m_display_name[DISPLAY_NAME_LENGTH] = "";

/* Added flag for window pop-up mode -- jl */

boolean m_autoraise = false;

Display *m_display;
Window m_window;
int screennum;
int BlackAndWhite = False;
int m_events_received;

/*int m_across, m_down, m_maxcolor;*/

static int nocache;
static int flip, trans;
static int trans_XtoX, trans_XtoY, trans_YtoY, trans_YtoX, trans_denom,
           trans_addx, trans_addy;
static GC gc[ColorsInSet];
static GC CursorGC, CursorGC2, CursorGC3;
static Pixmap UnderCursor;
static int currentcolor = -1;
static int currentmode = 0;
static Font fontnum;
static XFontStruct *currentfont;
static int fontasc;

static int RealWinDepth = WinDepth;
static Colormap colormap;
static unsigned long plane_masks[1<<WinDepth];
static unsigned long plane_mask;
static unsigned long pixel;
#ifdef HIRES
static int planeCount;
#endif  /* HIRES */

static unsigned long notAllPlanes;

static struct grid {
  int dx, dy, ax, ay;
  unsigned long color;
  Pixmap p1, p2;
  GC gc1, gc2;
} grid1 = { -1, -1, -1, -1, 0, 0, 0, 0, 0 },
  grid2 = { -1, -1, -1, -1, 0, 0, 0, 0, 0 },
  *newgrid, *oldgrid;

static Cursor blankcursor;
static struct cursor {
  Cursor sub;
  Pixmap c1, c2;
  int xoff, yoff, w, h;
} cursors[4];

#define AllColors DoRed | DoGreen | DoBlue
XColor m_colors[ColorSets+1][ColorsInSet] = {
  {
    { 0,     0,     0,     0, AllColors, 0 },
    { 0, 65535,     0,     0, AllColors, 0 },
    { 0,     0, 65535,     0, AllColors, 0 },
    { 0, 65535, 65535,     0, AllColors, 0 },
    { 0,     0,     0, 65535, AllColors, 0 },
    { 0, 65535,     0, 65535, AllColors, 0 },
    { 0,     0, 65535, 65535, AllColors, 0 },
    { 0, 65535, 65535, 65535, AllColors, 0 },
    { 0, 52428, 52428, 52428, AllColors, 0 },
    { 0, 34952,     0,     0, AllColors, 0 },
    { 0,     0, 34952,     0, AllColors, 0 },
    { 0, 34952, 34952,     0, AllColors, 0 },
    { 0,     0,     0, 34952, AllColors, 0 },
    { 0, 34952,     0, 34952, AllColors, 0 },
    { 0,     0, 34952, 34952, AllColors, 0 },
    { 0, 44952, 44952, 44952, AllColors, 0 },
  },
  {
    { 0,     0,     0,     0, AllColors, 0 },
    { 0, 52428,     0,     0, AllColors, 0 },
    { 0,     0, 52428,     0, AllColors, 0 },
    { 0, 52428, 52428,     0, AllColors, 0 },
    { 0,     0,     0, 52428, AllColors, 0 },
    { 0, 52428,     0, 52428, AllColors, 0 },
    { 0,     0, 52428, 52428, AllColors, 0 },
    { 0, 65535, 65535, 65535, AllColors, 0 },
    { 0, 52428, 52428, 26214, AllColors, 0 },
    { 0, 21845, 39321, 34952, AllColors, 0 },
    { 0, 34952, 30583, 43690, AllColors, 0 },
    { 0, 43690, 21845, 30583, AllColors, 0 },
    { 0, 52428, 26214, 21845, AllColors, 0 },
    { 0, 65535, 39321,  8738, AllColors, 0 },
    { 0, 48059, 34952, 26214, AllColors, 0 },
    { 0, 44952, 44952, 44952, AllColors, 0 },
  },
  {
    { 0,     0,     0,     0, AllColors, 0 },
    { 0,  4369,  4369,  4369, AllColors, 0 },
    { 0,  8738,  8738,  8738, AllColors, 0 },
    { 0, 13107, 13107, 13107, AllColors, 0 },
    { 0, 17496, 17496, 17496, AllColors, 0 },
    { 0, 21845, 21845, 21845, AllColors, 0 },
    { 0, 26214, 26214, 26214, AllColors, 0 },
    { 0, 30583, 30583, 30583, AllColors, 0 },
    { 0, 34952, 34952, 34952, AllColors, 0 },
    { 0, 39321, 39321, 39321, AllColors, 0 },
    { 0, 43690, 43690, 43690, AllColors, 0 },
    { 0, 48059, 48059, 48059, AllColors, 0 },
    { 0, 52428, 52428, 52428, AllColors, 0 },
    { 0, 56797, 56797, 56797, AllColors, 0 },
    { 0, 61166, 61166, 61166, AllColors, 0 },
    { 0, 65535, 65535, 65535, AllColors, 0 },
  },
  {
    { 0,     0,     0,     0, AllColors, 0 },
    { 0, 26214, 26214, 26214, AllColors, 0 },
    { 0, 43690, 43690, 43690, AllColors, 0 },
    { 0, 65535, 65535, 65535, AllColors, 0 },
    { 0, 21845,     0,     0, AllColors, 0 },
    { 0, 34952,     0,     0, AllColors, 0 },
    { 0, 48059,     0,     0, AllColors, 0 },
    { 0, 65535,     0,     0, AllColors, 0 },
    { 0,     0, 21845,     0, AllColors, 0 },
    { 0,     0, 34952,     0, AllColors, 0 },
    { 0,     0, 48059,     0, AllColors, 0 },
    { 0,     0, 65535,     0, AllColors, 0 },
    { 0,     0,     0, 21845, AllColors, 0 },
    { 0,     0,     0, 34952, AllColors, 0 },
    { 0,     0,     0, 48059, AllColors, 0 },
    { 0,     0,     0, 65535, AllColors, 0 },
  },
  {
    { 0,     0,     0,     0, AllColors, 0 },
    { 0, 65535,     0,     0, AllColors, 0 },
    { 0,     0, 65535,     0, AllColors, 0 },
    { 0, 65535, 65535,     0, AllColors, 0 },
    { 0,     0,     0, 65535, AllColors, 0 },
    { 0, 65535,     0, 65535, AllColors, 0 },
    { 0,     0, 65535, 65535, AllColors, 0 },
    { 0, 65535, 65535, 65535, AllColors, 0 },
    { 0, 52428, 52428, 52428, AllColors, 0 },
    { 0, 34952,     0,     0, AllColors, 0 },
    { 0,     0, 34952,     0, AllColors, 0 },
    { 0, 34952, 34952,     0, AllColors, 0 },
    { 0,     0,     0, 34952, AllColors, 0 },
    { 0, 34952,     0, 34952, AllColors, 0 },
    { 0,     0, 34952, 34952, AllColors, 0 },
    { 0, 44952, 44952, 44952, AllColors, 0 },
  },
};

static long WindowEventMask = ExposureMask | KeyPressMask |
                              ButtonPressMask | ButtonReleaseMask |
                              PointerMotionMask | StructureNotifyMask |
                              EnterWindowMask | LeaveWindowMask |
			      OwnerGrabButtonMask | FocusChangeMask;

static unsigned long WinAttrMask =
    CWBackPixel | CWBorderPixel | CWEventMask | GCForeground | GCBackground;

static XSetWindowAttributes WinAttr = {
  None,				      /*  background_pixmap      */
  0,				      /*  background_pixel       */
  CopyFromParent,		      /*  border_pixmap		 */
  0,				      /*  border_pixel		 */
  ForgetGravity,		      /*  bit_gravity		 */
  NorthWestGravity,		      /*  win_gravity		 */
  NotUseful,			      /*  backing_store		 */
  0,     			      /*  backing_planes	 */
  0,				      /*  backing_pixel		 */
  False,			      /*  save_under		 */
  0,             		      /*  event_mask		 */
  0,				      /*  do_not_propogate_mask  */
  False,			      /*  override_redirect      */
  CopyFromParent,		      /*  colormap               */
  None,				      /*  cursor                 */
};

static XWMHints WinWMHints = {
  InputHint,                          /*  flags                  */
  True,                               /*  input                  */
  NormalState,                        /*  initial_state          */
  None,                               /*  icon_pixmap            */
  None,                               /*  icon_window            */
  0, 0,                               /*  icon_x, icon_y         */
  None,                               /*  icon_mask              */
  None,                               /*  window_group           */
};

static XSizeHints WinSizeHints = {
  PMinSize|PMaxSize,                 /*  flags                  */
  0, 0,                              /*  x, y                   */
  512, 390,                          /*  width, height          */
  1, 1,                              /*  min_width, min_height  */
  2048, 2048,                        /*  max_width, max_height  */
  0, 0,                              /*  width_inc, height_inc  */
  { 0, 0 },                          /*  min_aspect             */
  { 0, 0 },                          /*  max_aspect             */
};

static char progname[256] = "mylib";

void GeneralTransform(x, y)
int *x, *y;
{
  int newx, newy;

  newx = (*x*trans_XtoX + *y*trans_YtoX) / trans_denom + trans_addx;
  newy = (*x*trans_XtoY + *y*trans_YtoY) / trans_denom + trans_addy;

  *x = newx;
  *y = newy;
}

void DeltaTransform(x, y)
int *x, *y;
{
  int newx, newy;

  newx = (*x*trans_XtoX + *y*trans_YtoX) / trans_denom;
  newy = (*x*trans_XtoY + *y*trans_YtoY) / trans_denom;

  *x = newx;
  *y = newy;
}

void LimitedTransform(x, y)
int *x, *y;
{
  int newx, newy;

  newx = *x*trans_XtoX / trans_denom + trans_addx;
  newy = *y*trans_YtoY / trans_denom + trans_addy;

  *x = newx;
  *y = newy;
}

void GeneralUnTransform(x, y)
int *x, *y;
{
  int x1, y1, newx, newy, det;

  x1 = *x*trans_denom-trans_addx;
  y1 = *y*trans_denom-trans_addy;

  det = (trans_XtoX*trans_YtoY - trans_YtoX*trans_XtoY);
  newx = (x1*trans_YtoY - y1*trans_YtoX) / det;
  newy = (y1*trans_XtoX - x1*trans_XtoY) / det;

  *x = newx;
  *y = newy;
}

void m_cache(newstate)
int newstate;
{
  if (!newstate && !nocache) {
    Ffprintf(stderr, "XFlush()\n");
    XFlush(m_display);
  }
  nocache = !newstate;
}

void m_choosecolors(colorset)
int colorset;
{
  int i;

  if ((colorset >= ColorSets) || (colorset < 0))
    fprintf(stderr, 
	    "Mylib:  m_choosecolors received colorset #%d\n", colorset);
  else {
    if ((! BlackAndWhite) && (grCurrent.depth <=8)) {
      Xfprintf(stderr, "XStoreColors()\n");
      XStoreColors(m_display, colormap, m_colors[colorset], ColorsInSet);
    }
    for (i = 0; i < ColorsInSet; i++) {
      m_colors[ColorSets][i].red = m_colors[colorset][i].red;
      m_colors[ColorSets][i].green = m_colors[colorset][i].green;
      m_colors[ColorSets][i].blue = m_colors[colorset][i].blue;
    }
  }
}

/* Added X display name support.  stafford 7/17/91 */

static int ScrDepth;
int red_size, green_size, blue_size, red_shift, green_shift, blue_shift;
int red_mask, green_mask, blue_mask;

void DisplayInitialize()
{
    int i, j, numcolors;
    char *bwdef ; /* mike godfrey */
#ifdef HIRES
    static int firstcall = 1;
    int colorCount, realColors, usableColors, status;
    unsigned long basepixel, pixelvalue;
    int grNumBitPlanes;
    long grCompleteMask;
    XColor colors[256*3];               /* for TrueColor */
    char *pmap, *p;
    Visual *grvisual;
    XVisualInfo grvisual_info, *grvisual_get, grtemplate;
    int gritems, gritems_list, grcolorCount, grclass;
    char *visual_type[6];
#define visual_table_len  7
    int visual_table[visual_table_len];
    char *log_color;
#else
    char *getenv();
#endif  /* HIRES */
    char *ep;
    extern int nc_text_in_window;
    char * m_usrfont = NULL;

    if (nc_text_in_window == 1 && (ep = getenv("AUTORAISE")) != NULL) {
      if (strcicmp(ep, "true") == 0) 
        m_autoraise = true;
      else if (strcicmp(ep, "false") == 0)
        m_autoraise = false;
    }

    oldgrid = &grid1;
    newgrid = &grid2;

   Xfprintf(stderr, "m_display = XOpenDisplay(\"%s\")\n", m_display_name);

   /* use the m_display_name variable as the name WES 7/17/91 */

   if ('\0' == *m_display_name )
     m_display = XOpenDisplay(NULL);
   else
     m_display = XOpenDisplay(m_display_name);

    if (! m_display)
      {
	if ('\0' == *m_display_name ) {
	  fprintf(stderr,
		  "Could not open display. (DISPLAY=%s)\n",
		  getenv("DISPLAY"));
	} else {
	  fprintf(stderr, "Could not open display \"%s\"\n", m_display_name);
	}
	exit(1);
      }
    screennum = DefaultScreen(m_display);
    colormap = DefaultColormap(m_display, screennum);

    WinSizeHints.max_width = XDisplayWidth(m_display, screennum);
    WinSizeHints.max_height = XDisplayHeight(m_display, screennum);
#ifdef HIRES
#ifdef HIRESDB
    fprintf(stderr, "Server Vendor: %s\n", ServerVendor(m_display));
    fprintf(stderr, "Vendor Release: %d\n", VendorRelease(m_display));
    fprintf(stderr, "Protocol Version: %d\n", ProtocolVersion(m_display));
    fprintf(stderr, "Protocol Revision: %d\n", ProtocolRevision(m_display));
#ifdef OS2
    fprintf(stderr, "HOSTTYPE: i386-OS/2\n");
#else
    fprintf(stderr, "HOSTTYPE: %s\n", getenv("HOSTTYPE"));
#endif  /* OS2 */
#endif  /* HIRESDB */

/* Discover properties of Server.  */
         visual_type[0] = (char *)&"StaticGrey";
         visual_type[1] = (char *)&"GreyScale";
         visual_type[2] = (char *)&"StaticColor";
         visual_type[3] = (char *)&"PseudoColor";
         visual_type[4] = (char *)&"TrueColor";
         visual_type[5] = (char *)&"DirectColor";

         grvisual = XDefaultVisual(m_display, screennum);
         grtemplate.screen = screennum;
         grtemplate.depth = 0; 
         grvisual_get = XGetVisualInfo(m_display,(long) 0x0, &grtemplate, &gritems);
         if(grvisual_get == NULL) {
           fprintf(stderr, "Could not obtain Visual Info from Server %s. Will attempt default.\n",
              getenv("DISPLAY")); }
         else {
           gritems_list = gritems;
#ifdef HIRESDB
           printf("XGetVisualInfo returned visuals list of length %d:\n", gritems);
           while(gritems) { gritems--;
             printf("Found Visual Class %s with:\n"
               "     depth %d, colormap_size %d, bits_per_rgb %d.\n",
               visual_type[grvisual_get[gritems].class],
               grvisual_get[gritems].depth,
               grvisual_get[gritems].colormap_size,
               grvisual_get[gritems].bits_per_rgb);
             if(grvisual_get[gritems].class == 4) {
               printf("     TrueColor masks: red %06x, green %06x, blue %06x\n",
                 grvisual_get[gritems].red_mask, grvisual_get[gritems].green_mask,
                 grvisual_get[gritems].blue_mask); }
         }
#endif  /* HIRESDB */
/* Unfortunately, the list returned by Xservers has classes in random order.
   Therefore, a search is needed to find a good choice. The only currently
   supported classes are PseudoColor at depth 8 and TrueColor at depth 24.
   It is likely that TrueColor depths 8 through 32 will work, but these have
   not been tested.

   At present the preferred order is: PseudoColor at 8, then TrueColor at 24,
   then TrueColor at 16, ...
   Unless this is overridden by the LOG_COLOR environment variable, which
   can be: bw, 8, 15, 16, 24
*/
         for(j = 0; j < visual_table_len; j++) { visual_table[j] = -1; }
         for(j = 0; j < gritems_list; j++) {
           if((grvisual_get[j].class == 0) && (grvisual_get[j].depth == 8))
             visual_table[1] = j;              /* StaticGrey */
           if((grvisual_get[j].class == 1) && (grvisual_get[j].depth == 8))
             visual_table[2] = j;              /* GreyScale */
           if((grvisual_get[j].class == 3) && (grvisual_get[j].depth == 8))
             visual_table[3] = j;              /* Psuedocolor */
           if((grvisual_get[j].class == 4) && (grvisual_get[j].depth == 15))
             visual_table[4] = j;              /* TrueColor */
           if((grvisual_get[j].class == 4) && (grvisual_get[j].depth == 16))
             visual_table[5] = j;              /* TrueColor */
           if((grvisual_get[j].class == 4) && (grvisual_get[j].depth == 24))
             visual_table[6] = j;              /* TrueColor */
         }
         log_color = getenv("LOG_COLOR");
#ifdef HIRESDB
printf("log_color: %s visual_table: %d %d %d %d %d %d %d\n", log_color, visual_table[0],
      visual_table[1],visual_table[2],visual_table[3],visual_table[4],visual_table[5],
      visual_table[6]);
#endif  /* HIRESDB */
         gritems = -1;
         if(log_color != NULL) {
            if(strncmp(log_color, "8bit", 4) == 0)  gritems = visual_table[3];
            if(strncmp(log_color, "15bit", 5) == 0) gritems = visual_table[4];
            if(strncmp(log_color, "16bit", 5) == 0) gritems = visual_table[5];
            if(strncmp(log_color, "24bit", 5) == 0) gritems = visual_table[6];
/* I think bw will work with Staticgray, Grayscale, Staticcolor or Pseudocolor     */
            if(strncmp(log_color, "bw", 2) == 0) { 
               gritems = visual_table[3];
               if(gritems == -1) gritems= visual_table[0];
               if(gritems == -1) gritems= visual_table[1];
               if(gritems == -1) gritems= visual_table[2]; }
            if(gritems == -1) {
               fprintf(stderr, "The LOG_COLOR mode %s is not available.\n"
                   "Choose from bw, 8bit, 15bit, 16bit, or 24bit.\n", log_color);
               XFree(grvisual_get);
               exit(1); }
         }
         else {
            if(visual_table[3] != -1)      gritems = visual_table[3];
            else if(visual_table[6] != -1) gritems = visual_table[6];
            else if(visual_table[5] != -1) gritems = visual_table[5];
            else if(visual_table[4] != -1) gritems = visual_table[4];
            else if(visual_table[1] != -1) gritems = visual_table[1];
         }
         if(gritems == -1) {
           fprintf(stderr, "Neither TrueColor 24, 16, 15, nor PseudoColor 8, nor BW found. "
              "Cannot initialize DISPLAY %s\n", getenv("DISPLAY"));
           XFree(grvisual_get);
           exit(1); }
#ifdef HIRESDB
         else {
           printf("Using %s, depth %d\n", visual_type[grvisual_get[gritems].class],
            grvisual_get[gritems].depth); }
#endif  /* HIRESDB */
           grclass         = grvisual_get[gritems].class;
           grvisual        = grvisual_get[gritems].visual;
           grcolorCount    = grvisual_get[gritems].colormap_size;
           grCurrent.depth = grvisual_get[gritems].depth;
           red_mask        = grvisual_get[gritems].red_mask;
           green_mask      = grvisual_get[gritems].green_mask;
           blue_mask       = grvisual_get[gritems].blue_mask; }
         XFree(grvisual_get);
         planeCount = grCurrent.depth;
         colorCount = grcolorCount;
         realColors = colorCount;
         if(planeCount <= 8) {
             ScrDepth = DefaultDepth(m_display,screennum);
             Xfprintf(stderr,"Screendepth = %d\n",ScrDepth);
             bwdef = XGetDefault( m_display, "mylib", "color");  /* **mdg** */
             if(log_color == NULL)log_color = "color";
             if(bwdef == NULL) bwdef = "color";    /* required since some strcmp's
                                                   cannot handle NULL   */

             Xfprintf(stderr, "bwdef: %s\n", bwdef);

             if ((ScrDepth < 4) || 
        	((bwdef != NULL) && (!strcmp(bwdef, "black_and_white")))
                || !strcmp(log_color,"bw"))
             {
             /* Assuming black-and-white */
        	BlackAndWhite = True;
             } 
             else 
             {
	        Xfprintf(stderr, "if (! XAllocColorCells())...\n");  
	        if (! XAllocColorCells(m_display, colormap, True, plane_masks,
			       WinDepth, &pixel, 1)) 
	        {
	           numcolors = DefaultVisual(m_display,
				      DefaultScreen(m_display))->map_entries;
	           if (numcolors < 16) {
	              fprintf(stderr, "Not enough color planes on screen\n");
	              exit(-1);
	           }
	           Xfprintf(stderr, "colormap = XCreateColormap()\n");
	           colormap = XCreateColormap(m_display, DefaultRootWindow(m_display),
				       DefaultVisual(m_display,
						     DefaultScreen(m_display)),
				       AllocAll);
	           Xfprintf(stderr, "XInstallColormap()\n");
	           XInstallColormap(m_display, colormap);
	           Xfprintf(stderr, "XAllocColorCells()\n");
	           if (!XAllocColorCells(m_display, colormap, True, plane_masks,
				  WinDepth, &pixel, 1))
	           {
  		     BlackAndWhite = True;
#ifdef HIRESDB
                     fprintf(stderr, "Could not Allocate Color Cells -- using bw\n");
#endif  /* HIRESDB */
	           }
                }
	     }
             for (i = 0; i < 1<<WinDepth; i++) {
                pixelvalue = pixel;
                for (j = 0; j < WinDepth; j++)
                   pixelvalue += (i & (1<<j)) == 0 ? 0 : plane_masks[j];

             for (j = 0; j <= ColorSets; j++)
                m_colors[j][i].pixel = pixelvalue;
             }

	if(planeCount >  8){
        plane_mask = AllPlanes;
        notAllPlanes = m_colors[0][(1<<WinDepth)-1].pixel - pixel; }
        else {
	  plane_mask = m_colors[0][(1<<WinDepth)-1].pixel - pixel; }

        if (! BlackAndWhite) {   
          Xfprintf(stderr, "XStoreColors()\n");
          XStoreColors(m_display, colormap, m_colors[0], ColorsInSet);
          Xfprintf(stderr, "XSetWindowColormap()\n");
          XSetWindowColormap(m_display, DefaultRootWindow(m_display), colormap);
        }
    }
    else {
                /*   code for HIRES goes here. */
                /* ========================================= */
          basepixel = 0;
          usableColors = colorCount;
	  colormap = XCreateColormap(m_display, DefaultRootWindow(m_display),
		       DefaultVisual(m_display, DefaultScreen(m_display)),
				       AllocNone);
 	  for (i=0; i < usableColors; i++) colors[i].pixel = i;
          XQueryColors(m_display, XDefaultColormap(m_display, 
                    screennum), colors, usableColors); 

	  realColors = (basepixel+colorCount > usableColors)?
		  				usableColors-basepixel:
						colorCount;
#ifdef HIRESDB
printf("realColors %d\n", realColors);
#endif  /* HIRESDB */
	  if (realColors != colorCount)
	  {
             printf("Only %d contigous colors were available.\n",realColors);
                  colorCount=realColors;
	  }
           if(planeCount == 16) {
             red_size = 5; green_size = 6; blue_size = 5; }
           else if(planeCount == 15) {
             red_size = 5; green_size = 5; blue_size = 5; }
           else {
             red_size = 8; green_size = 8; blue_size = 8; }
           red_shift = green_size + blue_size;
           green_shift = blue_size;
           if((planeCount == 24) & (red_mask == 0xff)) {
           /* this is SUN Solaris doing it backwards: BGR  */
             red_shift = 0; green_shift = red_size;
             blue_shift = green_size + red_size; }
#ifdef RGBFILE
  /* This code is just to indicate what would be required if LOG ever wanted to
     provide an RGB data file (like Magic).                                     */
           p = pmap;
           for (i=0;i != colorCount;i++) {
              if((planeCount == 16) || (planeCount == 15)) {
                 grPixels[i] = (long)(((*p++) >> (8 - red_size))
                      << (green_size + blue_size)) & red_mask;                      /* red */
                 grPixels[i] |= (long)(((*p++) >> (8 - green_size))
                      << blue_size) & green_mask;                                   /* green */
                 grPixels[i] |= (long)((*p++) >> (8 - blue_size)) & blue_mask; }    /* blue */
              else if((planeCount == 24) & (red_mask == 0xff)) {
              /* this is SUN Solaris doing it backwards: BGR  */
                 grPixels[i] = (long)((*p++) & red_mask);  /* DO NOT MODIFY!       */
                 grPixels[i] |= (long)(((*p++) << green_shift) & green_mask); /* gcc blows up without */
                 grPixels[i] |= (long)(((*p++) << blue_shift) & blue_mask);                  /* the masks!           */
              }
              else {
                 grPixels[i] = (long)(((*p++)<< red_shift) & red_mask);  /* DO NOT MODIFY!       */
                 grPixels[i] |= (long)(((*p++)<< green_shift) & green_mask); /* gcc blows up without */
                 grPixels[i] |= (long)((*p++) & blue_mask);                  /* the masks!           */
              }
           }
#endif  /* RGBFILE */
           plane_mask = AllPlanes;
           notAllPlanes = 0x0a0a0a0a;
#ifdef HIRESDB
fprintf(stderr, "plane_mask %x, notAllPlanes %x\n", plane_mask, notAllPlanes);
#endif  /* HIRESDB */
           for (i = 0; i < 1<<WinDepth; i++) {
             for (j = 0; j <= ColorSets; j++) {
              plane_masks[i] = AllPlanes;
               if((planeCount == 24) & (red_mask == 0xff)) {
               /* this is SUN Solaris doing it backwards: BGR  */
                pixelvalue = (m_colors[j][i].red >> (16 - red_size))
                    & red_mask;
                pixelvalue |= ((m_colors[j][i].green >> (16 - green_size))
                    << red_size) & green_mask;
                m_colors[j][i].pixel = pixelvalue |
		  (((m_colors[j][i].blue >> (16 - blue_size))
                    << (red_size + green_size)) & blue_mask); }
               else {
                pixelvalue = ((m_colors[j][i].red >> (16 - red_size))
                    << (green_size + blue_size)) & red_mask;
                pixelvalue |= ((m_colors[j][i].green >> (16 - green_size))
                    << blue_size) & green_mask;
                m_colors[j][i].pixel = pixelvalue |
                    ((m_colors[j][i].blue >> (16 - blue_size)) & blue_mask); }
             }
           }
#ifdef HIRESDB
           printf("m_colors[0-1][3-4].pixel: %6x %6x %6x %6x\n", m_colors[0][3].pixel,
                m_colors[0][4].pixel,m_colors[1][4].pixel,m_colors[1][5].pixel);
#endif  /* HIRESDB */
    }
#endif  /* HIRES ffff */
  if (!(m_usrfont = XGetDefault (m_display, "mylib", "font")))
    m_usrfont = DefaultFont;
  Xfprintf(stderr, "fontnum = XLoadFont()\n");
  fontnum = XLoadFont(m_display, m_usrfont);
  Xfprintf(stderr, "currentfont = XQueryFont()\n");
  currentfont = XQueryFont(m_display, fontnum);
  fontasc = currentfont->max_bounds.ascent;
}

/* These are WOL-versions of function-keys */
#define wol_select "\003"
#define wol_left_arrow "\b"
#define wol_right_arrow "\034"
#define wol_down_arrow "\n"
#define wol_up_arrow "\037"

void mapkey()
{
  Xfprintf(stderr, "several XRebindKeysym()'s\n");
   XRebindKeysym(m_display,XK_Find,NULL,0,(unsigned char *)"",0) ;
   XRebindKeysym(m_display,XK_Insert,NULL,0,(unsigned char *)"",0) ;
   XRebindKeysym(m_display,XK_Delete,NULL,0,(unsigned char *)"",0) ;
   XRebindKeysym(m_display,XK_Select,NULL,0,(unsigned char *)wol_select,1) ;
   XRebindKeysym(m_display,XK_Prior,NULL,0,(unsigned char *)"",0) ;
   XRebindKeysym(m_display,XK_Next,NULL,0,(unsigned char *)"",0) ;
   XRebindKeysym(m_display,XK_Home,NULL,0,(unsigned char *)"",0) ;
   XRebindKeysym(m_display,XK_Up,NULL,0,(unsigned char *)wol_up_arrow,1) ;
   XRebindKeysym(m_display,XK_Down,NULL,0,(unsigned char *)wol_down_arrow,1) ;
   XRebindKeysym(m_display,XK_Left,NULL,0,(unsigned char *)wol_left_arrow,1) ;
   XRebindKeysym(m_display,XK_Right,NULL,0,(unsigned char *)wol_right_arrow,1);
#ifdef XK_Reset
   XRebindKeysym(m_display,XK_Reset,NULL,0,(unsigned char *)"",0) ;
   XRebindKeysym(m_display,XK_System,NULL,0,(unsigned char *)"",0) ;
   XRebindKeysym(m_display,XK_User,NULL,0,(unsigned char *)"",0) ;
   XRebindKeysym(m_display,XK_ClearLine,NULL,0,(unsigned char *)"",0) ;
   XRebindKeysym(m_display,XK_InsertLine,NULL,0,(unsigned char *)"",0) ;
   XRebindKeysym(m_display,XK_DeleteLine,NULL,0,(unsigned char *)"",0) ;
   XRebindKeysym(m_display,XK_DeleteChar,NULL,0,(unsigned char *)"",0) ;
   XRebindKeysym(m_display,XK_BackTab,NULL,0,(unsigned char *)"",0) ;
   XRebindKeysym(m_display,XK_KP_BackTab,NULL,0,(unsigned char *)"",0) ;
#endif
 }

void WindowInitialize()
{
  Window root;
  int i, x, y;
  unsigned int w, h, bw, d;
  Pixmap p;
  GC tempgc;
  XEvent event;
  char *usrgeo,*defgeo = {"1280x960+0+0"};  /* Default window size - can be overridden with -geometry or X resources */
  int  WinX, WinY, x_pad, y_pad;
  int WinW, WinH;
  char saved_geo[256];
  FILE *geo_file;
  char *home;


  root = DefaultRootWindow(m_display);
  
  /* Try to load saved geometry from ~/.chipmunk_geometry */
  home = getenv("HOME");
  if (home != NULL) {
    snprintf(saved_geo, sizeof(saved_geo), "%s/.chipmunk_geometry", home);
    geo_file = fopen(saved_geo, "r");
    if (geo_file != NULL) {
      char line[256];
      if (fgets(line, sizeof(line), geo_file) != NULL) {
        /* Remove trailing newline */
        line[strcspn(line, "\r\n")] = '\0';
        /* Validate it looks like a geometry string (WxH format) */
        if (strchr(line, 'x') != NULL && strlen(line) < 64) {
          defgeo = strdup(line);  /* Use saved geometry as default */
        }
      }
      fclose(geo_file);
    }
  }

  WinAttr.background_pixel = m_colors[m_black]->pixel;
  WinAttr.border_pixel = WhitePixel(m_display, DefaultScreen(m_display));
  WinAttr.event_mask = WindowEventMask;


  /* This next section allows entries in .Xdefaults to set initial
     size and position of the mylib window. 
     Jim Clark 7/13/92 */

  usrgeo = XGetDefault (m_display, "mylib", "geometry");
    
  if (usrgeo == NULL)
    {
      WinSizeHints.flags |= PPosition;
      WinSizeHints.flags |= PSize;
    }
  else
    {
      WinSizeHints.flags |= USPosition;
      WinSizeHints.flags |= USSize;
    }
  x_pad = 0;
  y_pad = 0;

  XGeometry(m_display,screennum, usrgeo, defgeo, WinBorder, nc_fontwidth,
	    nc_fontheight, x_pad, y_pad, &WinX, &WinY,
	    &WinW, &WinH);

/*    End of revised section.  */


  Xfprintf(stderr, "m_window = XCreateWindow()\n");
  m_window = XCreateWindow(m_display, root, WinX, WinY, WinW, WinH, WinBorder,
			   CopyFromParent, InputOutput, 
			   DefaultVisual(m_display, screennum),
			   WinAttrMask, &WinAttr);

  /* Set window name based on program name */
  {
    char winname[256];
    char *basename;
    char *chipmunk_mode = getenv("CHIPMUNK_MODE");
    
    /* If CHIPMUNK_MODE=analog is set, use "analog" as window name */
    if (chipmunk_mode && strcmp(chipmunk_mode, "analog") == 0) {
      strcpy(winname, "analog");
    } else if (P_argv && P_argv[0] && *P_argv[0]) {
      /* Extract basename from path */
      basename = strrchr(P_argv[0], '/');
      if (basename)
        basename++;
      else
        basename = P_argv[0];
      strncpy(winname, basename, sizeof(winname) - 1);
      winname[sizeof(winname) - 1] = '\0';
      /* Remove any extension */
      {
        char *dot = strrchr(winname, '.');
        if (dot && dot != winname)
          *dot = '\0';
      }
    } else {
      strcpy(winname, "analog");
    }
    Xfprintf(stderr, "XStoreName()\n");
    XStoreName(m_display, m_window, winname);
    Xfprintf(stderr, "XSetIconName()\n");
    XSetIconName(m_display, m_window, winname);
    /* Update progname for XSetCommand */
    strncpy(progname, winname, sizeof(progname) - 1);
    progname[sizeof(progname) - 1] = '\0';
  }
  Xfprintf(stderr, "XSetCommand()\n");
  {
    /* XSetCommand expects char **argv, not char (*)[256] */
    char *argv_array[2];
    argv_array[0] = progname;
    argv_array[1] = NULL;
    XSetCommand(m_display, m_window, argv_array, 1);
  }
  Xfprintf(stderr, "XSetWMHints()\n");
  XSetWMHints(m_display, m_window, &WinWMHints);
  Xfprintf(stderr, "XSetNormalHints()\n");
  XSetNormalHints(m_display, m_window, &WinSizeHints);

  Xfprintf(stderr, "XMapWindow()\n");
  XMapWindow(m_display, m_window);

  /* Initialize window size tracking for geometry save */
  last_window_width = WinW;
  last_window_height = WinH;

  for (i = 0; i < ColorsInSet; i++) {
    Xfprintf(stderr, "gc[%d] = XCreateGC()\n", i);
    gc[i] = XCreateGC(m_display, m_window, 0, NULL);
    Xfprintf(stderr, "XSetFillRule()\n");
    XSetFillRule(m_display, gc[i], WindingRule);
    Xfprintf(stderr, "XSetFont()\n");
    XSetFont(m_display, gc[i], fontnum);
    Xfprintf(stderr, "XSetGraphicsExposures()\n");
    XSetGraphicsExposures(m_display, gc[i], False);
  }
  Xfprintf(stderr, "newgrid->gc1 = XCreateGC()\n");
  newgrid->gc1 = XCreateGC(m_display, m_window, 0, NULL);
  Xfprintf(stderr, "newgrid->gc2 = XCreateGC()\n");
  newgrid->gc2 = XCreateGC(m_display, m_window, 0, NULL);
  Xfprintf(stderr, "oldgrid->gc1 = XCreateGC()\n");
  oldgrid->gc1 = XCreateGC(m_display, m_window, 0, NULL);
  Xfprintf(stderr, "oldgrid->gc2 = XCreateGC()\n");
  oldgrid->gc2 = XCreateGC(m_display, m_window, 0, NULL);
  Xfprintf(stderr, "XSetFunction(m_display, newgrid->gc1)\n");
  XSetFunction(m_display, newgrid->gc1, GXor);
  Xfprintf(stderr, "XSetFunction(m_display, newgrid->gc2)\n");
  XSetFunction(m_display, newgrid->gc2, GXand);
  Xfprintf(stderr, "XSetFunction(m_display, oldgrid->gc1)\n");
  XSetFunction(m_display, oldgrid->gc1, GXor);
  Xfprintf(stderr, "XSetFunction(m_display, oldgrid->gc2)\n");
  XSetFunction(m_display, oldgrid->gc2, GXand);
  Xfprintf(stderr, "XSetFillStyle(m_display, newgrid->gc1)\n");
  XSetFillStyle(m_display, newgrid->gc1, FillTiled);
  Xfprintf(stderr, "XSetFillStyle(m_display, newgrid->gc2)\n");
  XSetFillStyle(m_display, newgrid->gc2, FillTiled);
  Xfprintf(stderr, "XSetFillStyle(m_display, oldgrid->gc1)\n");
  XSetFillStyle(m_display, oldgrid->gc1, FillTiled);
  Xfprintf(stderr, "XSetFillStyle(m_display, oldgrid->gc2)\n");
  XSetFillStyle(m_display, oldgrid->gc2, FillTiled);
  Xfprintf(stderr, "XSetGraphicsExposures(m_display, newgrid->gc1)\n");
  XSetGraphicsExposures(m_display, newgrid->gc1, False);
  Xfprintf(stderr, "XSetGraphicsExposures(m_display, newgrid->gc2)\n");
  XSetGraphicsExposures(m_display, newgrid->gc2, False);
  Xfprintf(stderr, "XSetGraphicsExposures(m_display, oldgrid->gc1)\n");
  XSetGraphicsExposures(m_display, oldgrid->gc1, False);
  Xfprintf(stderr, "XSetGraphicsExposures(m_display, oldgrid->gc2)\n");
  XSetGraphicsExposures(m_display, oldgrid->gc2, False);

  mapkey();

  Xfprintf(stderr, "XCreatePixmap()\n");
  p = XCreatePixmap(m_display, m_window, 1, 1, 1);
  Xfprintf(stderr, "XCreateGC()\n");
  tempgc = XCreateGC(m_display, p, 0, NULL);
  Xfprintf(stderr, "XSetForeground()\n");
  XSetForeground(m_display, tempgc, 0);
  Xfprintf(stderr, "XDrawPoint()\n");
  XDrawPoint(m_display, p, tempgc, 0, 0);
  Xfprintf(stderr, "XCreatePixmapCursor()\n");
  blankcursor = XCreatePixmapCursor(m_display, p, p, &m_colors[0][m_black],
				    &m_colors[0][m_black], 0, 0);
  Xfprintf(stderr, "cursors[0].sub = XCreatFontCursor()\n");
  cursors[0].sub = XCreateFontCursor(m_display, XC_tcross);
  Xfprintf(stderr, "XRecolorCursor(m_display, cursors[0].sub)\n");
  XRecolorCursor(m_display, cursors[0].sub, &m_colors[0][m_white],
                                            &m_colors[0][m_black]);
  Xfprintf(stderr, "cursors[1].sub = XCreatFontCursor()\n");
  cursors[1].sub = XCreateFontCursor(m_display, XC_left_ptr);
  Xfprintf(stderr, "XRecolorCursor(m_display, cursors[1].sub)\n");
  XRecolorCursor(m_display, cursors[1].sub, &m_colors[0][m_white],
                                            &m_colors[0][m_black]);
  Xfprintf(stderr, "cursors[2].sub = XCreatFontCursor()\n");
  cursors[2].sub = XCreateFontCursor(m_display, XC_X_cursor);
  Xfprintf(stderr, "XRecolorCursor(m_display, cursors[2].sub)\n");
  XRecolorCursor(m_display, cursors[2].sub, &m_colors[0][15],
		                            &m_colors[0][m_black]);
  Xfprintf(stderr, "cursors[3].sub = XCreatFontCursor()\n");
  cursors[3].sub = XCreateFontCursor(m_display, XC_gobbler);
  Xfprintf(stderr, "XRecolorCursor(m_display, cursors[3].sub)\n");
  XRecolorCursor(m_display, cursors[3].sub, &m_colors[0][m_yellow],
		                            &m_colors[0][m_black]);
  Xfprintf(stderr, "XDefineCursor()\n");
/*  fprintf(stderr, "XDefineCursor() (0)\n");  */
  XDefineCursor(m_display, m_window, cursors[0].sub);
  Xfprintf(stderr, "XFreePixmap()\n");
  XFreePixmap(m_display, p);
  Xfprintf(stderr, "XFreeGC()\n");
  XFreeGC(m_display, tempgc);

  Xfprintf(stderr, "XGetGeometry()\n");
  XGetGeometry(m_display, m_window, &root, &x, &y, &w, &h, &bw, &d);
  m_across = w;
  m_down = h;
  RealWinDepth = d;

  Xfprintf(stderr, "UnderCursor = XCreatePixmap()\n");
  UnderCursor = XCreatePixmap(m_display, m_window, 24, 24, RealWinDepth);

  Xfprintf(stderr, "cursors[0].c1 = XCreatePixmap()\n");
  cursors[0].c1 = XCreatePixmap(m_display, m_window, 11, 11, RealWinDepth);
  Xfprintf(stderr, "cursors[0].c2 = XCreatePixmap()\n");
  cursors[0].c2 = XCreatePixmap(m_display, m_window, 11, 11, RealWinDepth);
  cursors[0].xoff = 5;
  cursors[0].yoff = 5;
  cursors[0].w = 11;
  cursors[0].h = 11;

  Xfprintf(stderr, "cursors[1].c1 = XCreatePixmap()\n");
  cursors[1].c1 = XCreatePixmap(m_display, m_window, 8, 8, RealWinDepth);
  Xfprintf(stderr, "cursors[1].c2 = XCreatePixmap()\n");
  cursors[1].c2 = XCreatePixmap(m_display, m_window, 8, 8, RealWinDepth);
  cursors[1].xoff = 0;
  cursors[1].yoff = 0;
  cursors[1].w = 7;
  cursors[1].h = 7;

  Xfprintf(stderr, "cursors[2].c1 = XCreatePixmap()\n");
  cursors[2].c1 = XCreatePixmap(m_display, m_window, 9, 9, RealWinDepth);
  Xfprintf(stderr, "cursors[2].c2 = XCreatePixmap()\n");
  cursors[2].c2 = XCreatePixmap(m_display, m_window, 9, 9, RealWinDepth);
  cursors[2].xoff = 5;
  cursors[2].yoff = 5;
  cursors[2].w = 8;
  cursors[2].h = 8;

  Xfprintf(stderr, "cursors[3].c1 = XCreatePixmap()\n");
  cursors[3].c1 = XCreatePixmap(m_display, m_window, 11, 11, RealWinDepth);
  Xfprintf(stderr, "cursors[3].c2 = XCreatePixmap()\n");
  cursors[3].c2 = XCreatePixmap(m_display, m_window, 11, 11, RealWinDepth);
  cursors[3].xoff = 5;
  cursors[3].yoff = 5;
  cursors[3].w = 10;
  cursors[3].h = 10;

  Xfprintf(stderr, "tempgc = XCreateGC()\n");
  tempgc = XCreateGC(m_display, cursors[0].c1, 0, NULL);
  Xfprintf(stderr, "XSetPlaneMask(m_display, tempgc)\n");
  XSetPlaneMask(m_display, tempgc, plane_mask);

  Xfprintf(stderr, "XSetForeground(m_display, tempgc, 0)\n");
  XSetForeground(m_display, tempgc, 0);

  Xfprintf(stderr, "XFillRectangle(m_display, cursors[0].c1)\n");
  XFillRectangle(m_display, cursors[0].c1, tempgc, 0, 0, 11, 11);

  Xfprintf(stderr, "XFillRectangle(m_display, cursors[1].c1)\n");
  XFillRectangle(m_display, cursors[1].c1, tempgc, 0, 0, 8, 8);

  Xfprintf(stderr, "XFillRectangle(m_display, cursors[2].c1)\n");
  XFillRectangle(m_display, cursors[2].c1, tempgc, 0, 0, 9, 9);

  Xfprintf(stderr, "XFillRectangle(m_display, cursors[3].c1)\n");
  XFillRectangle(m_display, cursors[3].c1, tempgc, 0, 0, 11, 11);

  Xfprintf(stderr, "XSetForeground(m_display, tempgc, -1)\n");
  XSetForeground(m_display, tempgc, -1);

  Xfprintf(stderr, "XFillRectangle(m_display, cursors[0].c1)\n");
  XFillRectangle(m_display, cursors[0].c2, tempgc, 0, 0, 11, 11);

  Xfprintf(stderr, "XFillRectangle(m_display, cursors[1].c1)\n");
  XFillRectangle(m_display, cursors[1].c2, tempgc, 0, 0, 8, 8);

  Xfprintf(stderr, "XFillRectangle(m_display, cursors[2].c1)\n");
  XFillRectangle(m_display, cursors[2].c2, tempgc, 0, 0, 9, 9);

  Xfprintf(stderr, "XFillRectangle(m_display, cursors[3].c1)\n");
  XFillRectangle(m_display, cursors[3].c2, tempgc, 0, 0, 11, 11);

  Xfprintf(stderr, "XSetForeground(m_display, tempgc)\n");
  XSetForeground(m_display, tempgc, m_colors[0][m_white].pixel);

  Xfprintf(stderr, "XDrawLine(m_display, cursors[0].c1)\n");
  XDrawLine(m_display, cursors[0].c1, tempgc, 5, 0, 5, 10);

  Xfprintf(stderr, "XDrawLine(m_display, cursors[0].c1)\n");
  XDrawLine(m_display, cursors[0].c1, tempgc, 0, 5, 10, 5);

  Xfprintf(stderr, "XDrawLine(m_display, cursors[0].c2)\n");
  XDrawLine(m_display, cursors[0].c2, tempgc, 5, 0, 5, 10);

  Xfprintf(stderr, "XDrawLine(m_display, cursors[0].c2)\n");
  XDrawLine(m_display, cursors[0].c2, tempgc, 0, 5, 10, 5);

  Xfprintf(stderr, "XDrawLine(m_display, cursors[2].c1)\n");
  XDrawLine(m_display, cursors[2].c1, tempgc, 0, 0, 2, 2);

  Xfprintf(stderr, "XDrawLine(m_display, cursors[2].c1)\n");
  XDrawLine(m_display, cursors[2].c1, tempgc, 7, 0, 5, 2);

  Xfprintf(stderr, "XDrawLine(m_display, cursors[2].c1)\n");
  XDrawLine(m_display, cursors[2].c1, tempgc, 7, 7, 5, 5);

  Xfprintf(stderr, "XDrawLine(m_display, cursors[2].c1)\n");
  XDrawLine(m_display, cursors[2].c1, tempgc, 0, 7, 2, 5);

  Xfprintf(stderr, "XDrawLine(m_display, cursors[2].c2)\n");
  XDrawLine(m_display, cursors[2].c2, tempgc, 0, 0, 2, 2);

  Xfprintf(stderr, "XDrawLine(m_display, cursors[2].c2)\n");
  XDrawLine(m_display, cursors[2].c2, tempgc, 7, 0, 5, 2);

  Xfprintf(stderr, "XDrawLine(m_display, cursors[2].c2)\n");
  XDrawLine(m_display, cursors[2].c2, tempgc, 7, 7, 5, 5);

  Xfprintf(stderr, "XDrawLine(m_display, cursors[2].c2)\n");
  XDrawLine(m_display, cursors[2].c2, tempgc, 0, 7, 2, 5);

  Xfprintf(stderr, "XSetForeground(m_display, tempgc)\n");
  XSetForeground(m_display, tempgc, m_colors[0][m_white].pixel);

  Xfprintf(stderr, "XDrawLine(m_display, cursors[1].c1)\n");
  XDrawLine(m_display, cursors[1].c1, tempgc, 0, 0, 4, 0);

  Xfprintf(stderr, "XDrawLine(m_display, cursors[1].c1)\n");
  XDrawLine(m_display, cursors[1].c1, tempgc, 0, 0, 0, 4);

  Xfprintf(stderr, "XDrawLine(m_display, cursors[1].c1)\n");
  XDrawLine(m_display, cursors[1].c1, tempgc, 0, 0, 7, 7);

  Xfprintf(stderr, "XDrawLine(m_display, cursors[1].c2)\n");
  XDrawLine(m_display, cursors[1].c2, tempgc, 0, 0, 4, 0);

  Xfprintf(stderr, "XDrawLine(m_display, cursors[1].c2)\n");
  XDrawLine(m_display, cursors[1].c2, tempgc, 0, 0, 0, 4);

  Xfprintf(stderr, "XDrawLine(m_display, cursors[1].c2)\n");
  XDrawLine(m_display, cursors[1].c2, tempgc, 0, 0, 7, 7);

  Xfprintf(stderr, "XFreeGC(m_display, tempgc)\n");
  XFreeGC(m_display, tempgc);

  Xfprintf(stderr, "CursorGC = XCreateGC()\n");
  CursorGC = XCreateGC(m_display, m_window, 0, NULL);

  Xfprintf(stderr, "CursorGC2 = XCreateGC()\n");
  CursorGC2 = XCreateGC(m_display, m_window, 0, NULL);

  Xfprintf(stderr, "CursorGC3 = XCreateGC()\n");
  CursorGC3 = XCreateGC(m_display, m_window, 0, NULL);

  Xfprintf(stderr, "XSetPlaneMask(m_display, CursorGC)\n");
  XSetPlaneMask(m_display, CursorGC, plane_mask);

  Xfprintf(stderr, "XSetPlaneMask(m_display, CursorGC2)\n");
  XSetPlaneMask(m_display, CursorGC2, plane_mask);

  Xfprintf(stderr, "XSetPlaneMask(m_display, CursorGC3)\n");
  XSetPlaneMask(m_display, CursorGC3, plane_mask);

  Xfprintf(stderr, "XSetGraphicsExposures(m_display, CursorGC)\n");
  XSetGraphicsExposures(m_display, CursorGC, False);

  Xfprintf(stderr, "XSetGraphicsExposures(m_display, CursorGC2)\n");
  XSetGraphicsExposures(m_display, CursorGC2, False);

  Xfprintf(stderr, "XSetGraphicsExposures(m_display, CursorGC3)\n");
  XSetGraphicsExposures(m_display, CursorGC3, False);

  Xfprintf(stderr, "XSetFunction(m_display, CursorGC2, GXor)\n");
  XSetFunction(m_display, CursorGC2, GXor);

  Xfprintf(stderr, "XSetFunction(m_display, CursorGC3, GXand)\n");
  XSetFunction(m_display, CursorGC3, GXand);

  Xfprintf(stderr, "XSetFont(m_display, CursorGC)\n");
  XSetFont(m_display, CursorGC, XLoadFont(m_display, "cursor"));

  Xfprintf(stderr, "XWindowevent(m_display, m_window, ExposureMask)\n");
  XWindowEvent(m_display, m_window, ExposureMask, &event);
}

 
 
#define LINESTIPPLELENGTH	4
 
 
static unsigned char dotted[LINESTIPPLELENGTH] = {2, 1, 2, 1};
static unsigned char dot_dashed[LINESTIPPLELENGTH] = {3, 4, 3, 1};
static unsigned char sh_dashed[LINESTIPPLELENGTH] = {4, 3, 4, 3};
static unsigned char lo_dashed[LINESTIPPLELENGTH] = {7, 1, 7, 1};
static unsigned char odd_dashed[LINESTIPPLELENGTH] = {1, 2, 3, 4};
static unsigned char black_dashed[LINESTIPPLELENGTH] = {1, 7, 1, 7};

static unsigned char *default_linestyle[ColorsInSet];

static unsigned char *FigureOutBWLine(r, g, b)
     int r, g, b;
{
  /* This is just a matter of taste */

  if ((r > 10) && (g == 0) && (b == 0)) 
    /* red */
    return lo_dashed;
  else if ((r > 11) && (g > 12) && (b == 0))
    /* yellow */
    return sh_dashed;
  else if ((r < 2) && (g < 2) && (b < 2))
    /* black */
    return lo_dashed;
  else
    return NULL;
}


static void do_init_screen(full)
int full;
{
  Window root;
  int i, x, y;
  unsigned int w, h, bw, d;

  init_debug_flags();

  if (! m_initialized)
    DisplayInitialize();

  if (! m_initialized)
    WindowInitialize();

  if (full)
   m_clear();

  Xfprintf(stderr, "XGetGeometry()\n");
  XGetGeometry(m_display, m_window, &root, &x, &y, &w, &h, &bw, &d);
  m_across = w;
  m_down = h;
  RealWinDepth = d;
  m_across--;
  m_down--;

  if (BlackAndWhite) {
    /* setup black-and-white attributes */
    m_maxcolor = 16;
    /* m_black         0 */
    Xfprintf(stderr, "XSetForeground(m_display,gc[m_black=%d])\n",m_black);
    XSetBackground(m_display,gc[m_red],WhitePixel(m_display,screennum));
    XSetForeground(m_display,gc[m_red],BlackPixel(m_display,screennum));
    XSetDashes(m_display, gc[m_black], 0, (char*)lo_dashed, LINESTIPPLELENGTH);
    XSetLineAttributes(m_display,gc[m_red],1,LineDoubleDash,CapButt,JoinMiter);
    default_linestyle[0] = lo_dashed;

    /* m_red           1 */
    Xfprintf(stderr, 
     "XSetForeground(m_display,gc[m_red], WhitePixel(m_display,screennum))\n");
    XSetForeground(m_display,gc[m_red],WhitePixel(m_display,screennum));
    XSetDashes(m_display, gc[m_red], 0, (char*)lo_dashed, LINESTIPPLELENGTH);
    XSetLineAttributes(m_display,gc[m_red],1,LineDoubleDash,CapButt,JoinMiter);
    default_linestyle[1] = lo_dashed;

    /* m_green         2 */
    Xfprintf(stderr,
    "XSetForeground(m_display,gc[m_green],WhitePixel(m_display,screennum))\n");
    XSetForeground(m_display,gc[m_green],WhitePixel(m_display,screennum));
    default_linestyle[2] = NULL;
 
    /* m_yellow        3 */
    Xfprintf(stderr, 
     "XSetForeground(m_display,gc[m_yellow],WhitePixel(m_display,screennum))\n"
	     );
    XSetForeground(m_display,gc[m_yellow],WhitePixel(m_display,screennum));
    XSetDashes(m_display, gc[m_yellow], 0, (char*)sh_dashed,
	       LINESTIPPLELENGTH);
    XSetLineAttributes(m_display,gc[m_yellow],1,
		       LineDoubleDash,CapButt,JoinMiter);
    default_linestyle[3] = sh_dashed;

    /* m_blue          4 */
    Xfprintf(stderr, 
     "XSetForeground(m_display,gc[m_blue],WhitePixel(m_display,screennum))\n");
    XSetForeground(m_display,gc[m_blue],WhitePixel(m_display,screennum));
    default_linestyle[4] = NULL;

    /* m_purple        5 */
    Xfprintf(stderr, 
    "XSetForeground(m_display,gc[m_purple],WhitePixel(m_display,screennum))\n"
	     );
    XSetForeground(m_display,gc[m_purple],WhitePixel(m_display,screennum));
    default_linestyle[5] = NULL;
 
 
    /* m_cyan          6 */
    Xfprintf(stderr, 
    "XSetForeground(m_display,gc[m_cyan],WhitePixel(m_display,screennum))\n"
	     );
    XSetForeground(m_display,gc[m_cyan],WhitePixel(m_display,screennum));
    default_linestyle[6] = NULL;
 
    /* m_white         7 */
    Xfprintf(stderr, 
    "XSetForeground(m_display,gc[m_white],WhitePixel(m_display,screennum))\n");
    XSetForeground(m_display,gc[m_white],WhitePixel(m_display,screennum));
    default_linestyle[7] = NULL;

 
    Xfprintf(stderr, 
    "XSetForeground(m_display,gc[8],WhitePixel(m_display,screennum))\n");
    XSetForeground(m_display,gc[8],WhitePixel(m_display,screennum));
    default_linestyle[8] = NULL;

    XSetForeground(m_display,gc[9],WhitePixel(m_display,screennum));
    default_linestyle[9] = NULL;
    XSetForeground(m_display,gc[10],WhitePixel(m_display,screennum));
    default_linestyle[10] = NULL;
    XSetForeground(m_display,gc[11],WhitePixel(m_display,screennum));
    default_linestyle[11] = NULL;
    XSetForeground(m_display,gc[12],WhitePixel(m_display,screennum));
    default_linestyle[12] = NULL;
    XSetForeground(m_display,gc[13],WhitePixel(m_display,screennum));
    default_linestyle[13] = NULL;
    XSetForeground(m_display,gc[14],WhitePixel(m_display,screennum));
    default_linestyle[14] = NULL;
    XSetForeground(m_display,gc[15],WhitePixel(m_display,screennum));
    default_linestyle[15] = NULL;

  }  
  else 
    {
      m_maxcolor = (1L << WinDepth) - 1;
      for (i = 0; i < ColorsInSet; i++) {
	Xfprintf(stderr, "XSetPlaneMask(m_display, gc[%d])\n", i);
	XSetPlaneMask(m_display, gc[i], plane_mask);
 
	Xfprintf(stderr, "XSetForeground(m_display, gc[%d])\n", i);
	XSetForeground(m_display, gc[i], m_colors[0][i].pixel);
      }
    }

  i = m_white;


  Xfprintf(stderr, "XSetPlaneMask(m_display, newgrid->gc1)\n");
  XSetPlaneMask(m_display, newgrid->gc1, plane_mask);

  Xfprintf(stderr, "XSetForeground(m_display, newgrid->gc1)\n");
  XSetForeground(m_display, newgrid->gc1, m_colors[0][i].pixel);
  XSetBackground(m_display, newgrid->gc1, BlackPixel(m_display,screennum));

  Xfprintf(stderr, "XSetPlaneMask(m_display, newgrid->gc2)\n");
  XSetPlaneMask(m_display, newgrid->gc2, plane_mask);

  Xfprintf(stderr, "XSetForeground(m_display, newgrid->gc2)\n");
  XSetForeground(m_display, newgrid->gc2, m_colors[0][i].pixel);
  XSetBackground(m_display, newgrid->gc2, BlackPixel(m_display,screennum));

  Xfprintf(stderr, "XSetPlaneMask(m_display, oldgrid->gc1)\n");
  XSetPlaneMask(m_display, oldgrid->gc1, plane_mask);

  Xfprintf(stderr, "XSetForeground(m_display, oldgrid->gc1)\n");
  XSetForeground(m_display, oldgrid->gc1, m_colors[0][i].pixel);
  XSetBackground(m_display, oldgrid->gc1, BlackPixel(m_display,screennum));

  Xfprintf(stderr, "XSetPlaneMask(m_display, oldgrid->gc2)\n");
  XSetPlaneMask(m_display, oldgrid->gc2, plane_mask);

  Xfprintf(stderr, "XSetForeground(m_display, oldgrid->gc2)\n");
  XSetForeground(m_display, oldgrid->gc2, m_colors[0][i].pixel);
  XSetBackground(m_display, oldgrid->gc2, BlackPixel(m_display,screennum));
  m_color(m_red);
  m_choosecursor(0);


  Ffprintf(stderr, "XFlush()\n");
  XFlush(m_display);

  nocache = sync_all_calls;
  flip = 1;
  trans = 0;

  m_initialized = 1;
}

/* Added X display name support.  WES 7/17/91 */


void m_set_display_name(display_name)
char* display_name;
{
  Mfprintf(stderr, "m_set_display_name(\"%s\")\n", display_name);

  if (strlen(display_name) >= DISPLAY_NAME_LENGTH)
    strncpy(m_display_name, display_name, DISPLAY_NAME_LENGTH);
  else
    strcpy(m_display_name, display_name);

}

void m_init_screen()
{
  Mfprintf(stderr, "m_init_screen()\n");

  do_init_screen(0);
}

void m_init_graphics()
{
  Mfprintf(stderr, "m_init_graphics()\n");

  do_init_screen(1);
  m_choosecolors(0);
}


#ifndef ENABLE_DEBUGGING
# define nocache  0
#endif


void m_init_graphics_nopen()
{
  Mfprintf(stderr, "m_init_graphics_nopen()\n");
  do_init_screen(1);

}

void m_init_dzg()
{
  Mfprintf(stderr, "m_init_dzg()\n");

  m_init_graphics();
  flip = 0;
}

void m_init_colors()
{
  Mfprintf(stderr, "m_init_colors()\n");

  m_choosecolors(0);
}

void m_init_pen(hpib_address)
int hpib_address;
{
  Mfprintf(stderr, "m_init_pen(%d)\n", hpib_address);
}

void m_version(version)
int version;
{
  Mfprintf(stderr, "m_version(%d)\n", version);

  if (version != 0)
    fprintf(stderr, "Mylib:  m_version(%d) not supported\n", version);
}

long m_curversion()
{
  Mfprintf(stderr, "m_curversion()\n");

  return(0);
}

void m_modern(flag)
int flag;
{
  Mfprintf(stderr, "m_modern(%d)\n", flag);

  if (flag)
    m_version(1);
  else
    m_version(0);
}

/*****************************************************************************/
/*            These are internal routines to do better buffering             */


#define BUF_SIZE 1024


static XPoint pointbuf[16][BUF_SIZE];
static int pointbuf_size[16];

static void buffer_point(color, x, y)
int color, x, y;
{
  pointbuf[color][pointbuf_size[color]].x = x;
  pointbuf[color][pointbuf_size[color]].y = y;
  if (++pointbuf_size[color] == BUF_SIZE) {
    XDrawPoints(m_display, m_window, gc[color],
		pointbuf[color], BUF_SIZE, CoordModeOrigin);
    pointbuf_size[color] = 0;
  }
}


static void flush_points()
{
  int color;

  for (color = 0; color < 16; color++) {
    if (pointbuf_size[color]) {
      XDrawPoints(m_display, m_window, gc[color],
		  pointbuf[color], pointbuf_size[color], CoordModeOrigin);
      pointbuf_size[color] = 0;
    }
  }
}

static XSegment linebuf[16][BUF_SIZE];
static int linebuf_size[16];

static void buffer_line(color, x1, y1, x2, y2)
int color, x1, y1, x2, y2;
{
  linebuf[color][linebuf_size[color]].x1 = x1;
  linebuf[color][linebuf_size[color]].y1 = y1;
  linebuf[color][linebuf_size[color]].x2 = x2;
  linebuf[color][linebuf_size[color]].y2 = y2;
  if (++linebuf_size[color] == BUF_SIZE) {
    XDrawSegments(m_display, m_window, gc[color], linebuf[color], BUF_SIZE);
    linebuf_size[color] = 0;
  }
}


static void flush_lines()
{
  int color;

  for (color = 0; color < 16; color++) {
    if (linebuf_size[color]) {
      XDrawSegments(m_display, m_window, gc[color],
		    linebuf[color], linebuf_size[color]);
      linebuf_size[color] = 0;
    }
  }
}


static XRectangle rectbuf[16][BUF_SIZE];
static int rectbuf_size[16];

static void buffer_rect(color, x, y, width, height)
int color, x, y, width, height;
{
  rectbuf[color][rectbuf_size[color]].x = x;
  rectbuf[color][rectbuf_size[color]].y = y;
  rectbuf[color][rectbuf_size[color]].width = width;
  rectbuf[color][rectbuf_size[color]].height = height;
  if (++rectbuf_size[color] == BUF_SIZE) {
    XDrawRectangles(m_display, m_window, gc[color], rectbuf[color], BUF_SIZE);
    rectbuf_size[color] = 0;
  }
}


static void flush_rects()
{
  int color;

  for (color = 0; color < 16; color++) {
    if (rectbuf_size[color]) {
      XDrawRectangles(m_display, m_window, gc[color],
		      rectbuf[color], rectbuf_size[color]);
      rectbuf_size[color] = 0;
    }
  }
}


static XRectangle fillrectbuf[16][BUF_SIZE];
static int fillrectbuf_size[16];

static void buffer_fillrect(color, x, y, width, height)
int color, x, y, width, height;
{
  fillrectbuf[color][fillrectbuf_size[color]].x = x;
  fillrectbuf[color][fillrectbuf_size[color]].y = y;
  fillrectbuf[color][fillrectbuf_size[color]].width = width;
  fillrectbuf[color][fillrectbuf_size[color]].height = height;
  if (++fillrectbuf_size[color] == BUF_SIZE) {
    XFillRectangles(m_display, m_window, gc[color],
		    fillrectbuf[color], BUF_SIZE);
    fillrectbuf_size[color] = 0;
  }
}


static void flush_fillrects()
{
  int color;

  for (color = 0; color < 16; color++) {
    if (fillrectbuf_size[color]) {
      XFillRectangles(m_display, m_window, gc[color],
		    fillrectbuf[color], fillrectbuf_size[color]);
      fillrectbuf_size[color] = 0;
    }
  }
}


static void flush_buffers()
{
  flush_points();
  flush_lines();
  flush_rects();
  flush_fillrects();
}


/*                                                                           */
/*****************************************************************************/


void m_clear()
{
  Mfprintf(stderr, "m_clear()\n");

#ifdef EXTRA_BUFFERING
  flush_buffers();
#endif /* EXTRA_BUFFERING */
  Xfprintf(stderr, "XClearWindow()\n");
  XClearWindow(m_display, m_window);

  if (nocache) {
    Ffprintf(stderr, "XFlush()\n");
    XFlush(m_display);
  }
}

void m_clearwindow(from, lines)
int from, lines;
{
  Mfprintf(stderr, "m_clearwindow(%d, %d)\n", from, lines);

#ifdef EXTRA_BUFFERING
  flush_buffers();
#endif /* EXTRA_BUFFERING */
  Xfprintf(stderr, "XClearArea()\n");
  XClearArea(m_display, m_window, 0, from, 0, lines, False);

  if (nocache) {
    Ffprintf(stderr, "XFlush()\n");
    XFlush(m_display);
  }
}

void m_vsync()
{
  Mfprintf(stderr, "m_vsync()\n");
}

int m_clip_x1, m_clip_y1, m_clip_x2, m_clip_y2;

void m_clip(x1, y1, x2, y2)
int x1, y1, x2, y2;
{
  XRectangle rect;
  int i;

  Mfprintf(stderr, "m_clip(%d, %d, %d, %d)\n", x1, y1, x2, y2);

  if (x2 == 32767) {
    m_noclip();
    return;
  }

  LTRNSFRM(x1, y1);
  LTRNSFRM(x2, y2);

  if (x1 > x2)
    i = x1, x1 = x2, x2 = i;
  if (y1 > y2)
    i = y1, y1 = y2, y2 = i;

  if ((m_clip_x1 != x1) || (m_clip_y1 != y1) ||
      (m_clip_x2 != x2) || (m_clip_y2 != y2)) {
#ifdef EXTRA_BUFFERING
    flush_buffers();
#endif /* EXTRA_BUFFERING */
    rect.x = x1;
    rect.y = y1;
    rect.width = x2 - x1;
    rect.height = y2 - y1;

    for (i = 0; i < ColorsInSet; i++) {
      Xfprintf(stderr, "XSetClipRectanges(m_display, gc[%d])\n", i);
      XSetClipRectangles(m_display, gc[i], 0, 0, &rect, 1, Unsorted);
    }
    Xfprintf(stderr, "XSetClipRectanges(m_display, newgrid->gc1)\n");
    XSetClipRectangles(m_display, newgrid->gc1, 0, 0, &rect, 1, Unsorted);

    Xfprintf(stderr, "XSetClipRectanges(m_display, newgrid->gc2)\n");
    XSetClipRectangles(m_display, newgrid->gc2, 0, 0, &rect, 1, Unsorted);

    Xfprintf(stderr, "XSetClipRectanges(m_display, oldgrid->gc1)\n");
    XSetClipRectangles(m_display, oldgrid->gc1, 0, 0, &rect, 1, Unsorted);

    Xfprintf(stderr, "XSetClipRectanges(m_display, oldgrid->gc2)\n");
    XSetClipRectangles(m_display, oldgrid->gc2, 0, 0, &rect, 1, Unsorted);
    m_clip_x1 = x1;
    m_clip_y1 = y1;
    m_clip_x2 = x2;
    m_clip_y2 = y2;
  }
}

void m_noclip()
{
  int i;

  Mfprintf(stderr, "m_noclip()\n");

  if ((m_clip_x1 != 0) || (m_clip_y1 != 0) ||
      (m_clip_x2 != 32767) || (m_clip_y2 != 32767)) {
#ifdef EXTRA_BUFFERING
    flush_buffers();
#endif /* EXTRA_BUFFERING */
    for (i = 0; i < ColorsInSet; i++) {
      Xfprintf(stderr, "XSetClipMask(m_display, gc[%d])\n", i);
      XSetClipMask(m_display, gc[i], None);
    }
    
    Xfprintf(stderr, "XSetClipMask(m_display, newgrid->gc1)\n");
    XSetClipMask(m_display, newgrid->gc1, None);
    
    Xfprintf(stderr, "XSetClipMask(m_display, newgrid->gc2)\n");
    XSetClipMask(m_display, newgrid->gc2, None);
    
    Xfprintf(stderr, "XSetClipMask(m_display, oldgrid->gc1)\n");
    XSetClipMask(m_display, oldgrid->gc1, None);
    
    Xfprintf(stderr, "XSetClipMask(m_display, oldgrid->gc2)\n");
    XSetClipMask(m_display, oldgrid->gc2, None);
    m_clip_x1 = 0;
    m_clip_y1 = 0;
    m_clip_x2 = 32767;
    m_clip_y2 = 32767;
  }
}

void m_transform(xx, yx, xy, yy, d, ax, ay)
int xx, yx, xy, yy, d, ax, ay;
{
  flip = 0;
  trans = 1;
  
  Mfprintf(stderr, "m_transform(%d, %d, %d, %d, %d, %d, %d)\n",
	                        xx, yx, xy, yy,  d, ax, ay   );

  trans_XtoX = xx;
  trans_XtoY = xy;
  trans_YtoY = yy;
  trans_YtoX = yx;
  trans_denom = d;
  trans_addx = ax;
  trans_addy = ay;
}

void m_notransform()
{
  Mfprintf(stderr, "m_notransform()\n");

  flip = 1;
  trans = 0;
}

void m_upside_down()
{
  Mfprintf(stderr, "m_upside_down()\n");

  flip = 0;
  trans = 0;
}

void m_rotscale(rot, scale, ax, ay)
double rot, scale;
int ax, ay;
{
  int s, c;

  Mfprintf(stderr, "m_rotscale(%f, %f, %d, %d)\n", rot, scale, ax, ay);

  s = (int)(sin(rot)*scale * 256);
  c = (int)(cos(rot)*scale * 256);

  m_transform(c, -s, -s, -c, 256, ax, m_down-ay);
}

void m_rotscaled(rot, scale, ax, ay)
double rot, scale;
int ax, ay;
{
  Mfprintf(stderr, "m_rotscaled(%f, %f, %d, %d)\n", rot, scale, ax, ay);

  m_rotscale(rot*dr, scale, ax, ay);
}

static int cursor_is_on = 0;
static int curcursor = 1000000;
static int cursx, cursy;

static void turncursoroff()
{
  Mfprintf(stderr, "turncursoroff()\n");
/*  fprintf(stderr, "turncursoroff() at (%d,%d)\n", cursx, cursy);  */

  if (cursor_is_on) {
    Xfprintf(stderr, "XCopyArea()   (turncursoroff)\n");
    XCopyArea(m_display, UnderCursor, m_window, CursorGC, 0, 0,
	      cursors[curcursor].w, cursors[curcursor].h,
	      cursx-cursors[curcursor].xoff, cursy-cursors[curcursor].yoff);
    if (nocache) {
      Ffprintf(stderr, "XFlush()\n");
      XFlush(m_display);
    }
  }
}

static void turncursoron()
{
  Mfprintf(stderr, "turncursoron() at (%d,%d)\n", cursx, cursy);
/*  fprintf(stderr, "turncursoron() at (%d,%d)\n", cursx, cursy);  */

  if (cursor_is_on) {
    Xfprintf(stderr, "XCopyArea()   (turncursoron)\n");
    XCopyArea(m_display, m_window, UnderCursor, CursorGC,
	      cursx-cursors[curcursor].xoff, cursy-cursors[curcursor].yoff, 
	      cursors[curcursor].w, cursors[curcursor].h, 0, 0);
    Xfprintf(stderr, "XCopyArea()   (turncursoron)\n");
    XCopyArea(m_display, cursors[curcursor].c1, m_window, CursorGC2, 0, 0,
	      cursors[curcursor].w, cursors[curcursor].h,
	      cursx-cursors[curcursor].xoff, cursy-cursors[curcursor].yoff);
    Xfprintf(stderr, "XCopyArea()   (turncursoron)\n");
    XCopyArea(m_display, cursors[curcursor].c2, m_window, CursorGC3, 0, 0,
	      cursors[curcursor].w, cursors[curcursor].h,
	      cursx-cursors[curcursor].xoff, cursy-cursors[curcursor].yoff);
    if (nocache) {
      Ffprintf(stderr, "XFlush()\n");
      XFlush(m_display);
    }
  }
}

void m_nocursor()
{
  Mfprintf(stderr, "m_nocursor()\n");

  if (cursor_is_on) {
    turncursoroff();
    cursor_is_on = 0;
/*    fprintf(stderr, "XDefineCursor() (%d)\n", curcursor);  */
    Xfprintf(stderr, "XDefineCursor()  (m_nocursor)\n");
    XDefineCursor(m_display, m_window, cursors[curcursor].sub);
  }
}

static m_tablet_info mouse;

void m_cursor(x, y)
int x, y;
{
  Mfprintf(stderr, "m_cursor(%d, %d)\n", x, y);
/*  fprintf(stderr, "m_cursor(%d, %d)\n", x, y);  */

  TRNSFRM(x, y);

  if (x != cursx || y != cursy || !cursor_is_on) {
    turncursoroff();
    cursx = x;
    cursy = y;
    Xfprintf(stderr, "XCopyArea()  (m_cursor)\n");
    XCopyArea(m_display, m_window, UnderCursor, CursorGC,
	      cursx-cursors[curcursor].xoff, cursy-cursors[curcursor].yoff, 
	      cursors[curcursor].w, cursors[curcursor].h, 0, 0);
    Xfprintf(stderr, "XCopyArea()  (m_cursor)\n");
    XCopyArea(m_display, cursors[curcursor].c1, m_window, CursorGC2, 0, 0,
	      cursors[curcursor].w, cursors[curcursor].h,
	      cursx-cursors[curcursor].xoff, cursy-cursors[curcursor].yoff);
    Xfprintf(stderr, "XCopyArea()  (m_cursor)\n");
    XCopyArea(m_display, cursors[curcursor].c2, m_window, CursorGC3, 0, 0,
	      cursors[curcursor].w, cursors[curcursor].h,
	      cursx-cursors[curcursor].xoff, cursy-cursors[curcursor].yoff);
    if (nocache) {
      Ffprintf(stderr, "XFlush()\n");
      XFlush(m_display);
    }
    if (! cursor_is_on) {
      cursor_is_on = 1;
/*      fprintf(stderr, "XDefineCursor() (blankcursor)\n");  */
      Xfprintf(stderr, "XDefineCursor()  (m_cursor)\n");
      XDefineCursor(m_display, m_window, blankcursor);
    }
  }
}

static struct {
  unsigned int cursor;
  unsigned long color;
} xcursors[] = {
  { XC_tcross, 15 },
  { XC_left_ptr, m_white },
  { XC_X_cursor, 15 },
  { XC_gobbler, m_yellow },
};

void m_choosecursor(n)
int n;
{
  Cursor newcursor;

  Mfprintf(stderr, "m_choosecursor(%d)\n", n);

  if ((n >= 0) && (n <= 3) && (n != curcursor)) {
    if (cursor_is_on) {
      m_nocursor();
      curcursor = n;
/*      fprintf(stderr, "m_cursor(%d, %d)   from m_choosecursor\n",
	      mouse.x, mouse.y);  */
      m_cursor(mouse.x, mouse.y);
    } else {
      curcursor = n;
/*      fprintf(stderr, "XDefineCursor() (%d)\n", n);  */
      Xfprintf(stderr, "XDefineCursor()  (m_choosecursor)\n");
      XDefineCursor(m_display, m_window, cursors[n].sub);
    }
  }
/*
    newcursor = XCreateFontCursor(m_display, xcursors[n].cursor);
    XRecolorCursor(m_display, newcursor, &m_colors[0][xcursors[n].color],
                                         &m_colors[0][m_black]);
    
    XDefineCursor(m_display, m_window, newcursor);
  }
*/
  if (nocache) {
    Ffprintf(stderr, "XFlush()\n");
    XFlush(m_display);
  }
}

void m_colormode(c)
int c;
{
  int i;

  Mfprintf(stderr, "m_colormode(%d)\n", c);

  if (c != currentmode) {
#ifdef EXTRA_BUFFERING
    flush_buffers();
#endif /* EXTRA_BUFFERING */
    for (i = 0; i < ColorsInSet; i++)
      switch (c) {
      case m_normal:
	Xfprintf(stderr, "XSetFunction(m_display, gc[%d], GXcopy)\n", i);
	XSetFunction(m_display, gc[i], GXcopy);
	break;
      case m_xor:
	if ((BlackAndWhite) && 
 	    (WhitePixel(m_display,screennum) == 0)) {
	  Xfprintf(stderr, "XSetFunction(m_display, gc[%d], GXequiv)\n", i);
	  XSetFunction(m_display, gc[i], GXequiv);
 	} else {
	  Xfprintf(stderr, "XSetFunction(m_display, gc[%d], GXxor)\n", i);
	  XSetFunction(m_display, gc[i], GXxor);
 	}
	break;
      case m_over:
	Xfprintf(stderr, "XSetFunction(m_display, gc[%d], GXor)\n", i);
	XSetFunction(m_display, gc[i], GXor);
	break;
      case m_mask:
	Xfprintf(stderr, "XSetFunction(m_display, gc[%d], GXandInverted)\n", i);
	XSetFunction(m_display, gc[i], GXandInverted);
	break;
      case m_erase:
	Xfprintf(stderr, "XSetFunction(m_display, gc[%d], GXclear)\n", i);
	XSetFunction(m_display, gc[i], GXclear);
	break;
      case m_trans:
	Xfprintf(stderr, "XSetFunction(m_display, gc[%d], GXnoop)\n", i);
	XSetFunction(m_display, gc[i], GXnoop);
	break;
      default:
	break;
      }
    switch (c) {
    case m_normal:
      Xfprintf(stderr, "XSetFunction(m_display, newgrid->gc1, GXor)\n");
      XSetFunction(m_display, newgrid->gc1, GXor);

      Xfprintf(stderr, "XSetFunction(m_display, newgrid->gc2, GXand)\n");
      XSetFunction(m_display, newgrid->gc2, GXand);

      Xfprintf(stderr, "XSetFunction(m_display, oldgrid->gc1, GXor)\n");
      XSetFunction(m_display, oldgrid->gc1, GXor);

      Xfprintf(stderr, "XSetFunction(m_display, oldgrid->gc2, GXand)\n");
      XSetFunction(m_display, oldgrid->gc2, GXand);

#ifdef old_linux  /* linux fix by harold levy */
      XSetPlaneMask (m_display, newgrid->gc1, AllPlanes);
      XSetPlaneMask (m_display, newgrid->gc2, AllPlanes);
      XSetPlaneMask (m_display, oldgrid->gc1, AllPlanes);
      XSetPlaneMask (m_display, oldgrid->gc2, AllPlanes);
      XSetPlaneMask (m_display, CursorGC, AllPlanes);
      XSetPlaneMask (m_display, CursorGC2, AllPlanes);
      XSetPlaneMask (m_display, CursorGC3, AllPlanes);
      for (i = 0; i < ColorsInSet; i ++)
	XSetPlaneMask(m_display, gc[i], AllPlanes);
#endif
      break;
    case m_xor:
      if ((BlackAndWhite) && 
	  (WhitePixel(m_display,screennum) == 0)) {
	Xfprintf(stderr, "XSetFunction(m_display, newgrid->gc1, GXequiv)\n");
	XSetFunction(m_display, newgrid->gc1, GXequiv);
	Xfprintf(stderr, "XSetFunction(m_display, oldgrid->gc1, GXequiv)\n");
	XSetFunction(m_display, oldgrid->gc1, GXequiv);
      } else {
	Xfprintf(stderr, "XSetFunction(m_display, newgrid->gc1, GXxor)\n");
	XSetFunction(m_display, newgrid->gc1, GXxor);
	
	Xfprintf(stderr, "XSetFunction(m_display, oldgrid->gc1, GXxor)\n");
	XSetFunction(m_display, oldgrid->gc1, GXxor);

#ifdef old_linux        /* linux fix by Harold Levy */
	XSetPlaneMask (m_display, newgrid->gc1, notAllPlanes); 
	XSetPlaneMask (m_display, newgrid->gc2, notAllPlanes);
	XSetPlaneMask (m_display, oldgrid->gc1, notAllPlanes);
	XSetPlaneMask (m_display, oldgrid->gc2, notAllPlanes);
	XSetPlaneMask (m_display, CursorGC, notAllPlanes);
	XSetPlaneMask (m_display, CursorGC2, notAllPlanes);
	XSetPlaneMask (m_display, CursorGC3, notAllPlanes);
	for (i = 0; i < ColorsInSet; i ++) 
	  XSetPlaneMask(m_display, gc[i], notAllPlanes);
#endif
      }
      break;
    case m_over:
      Xfprintf(stderr, "XSetFunction(m_display, newgrid->gc1, GXor)\n");
      XSetFunction(m_display, newgrid->gc1, GXor);

      Xfprintf(stderr, "XSetFunction(m_display, oldgrid->gc1, GXor)\n");
      XSetFunction(m_display, oldgrid->gc1, GXor);
      break;
    case m_mask:
      Xfprintf(stderr, "XSetFunction(m_display, newgrid->gc2, GXandInverted)\n");
      XSetFunction(m_display, newgrid->gc2, GXandInverted);

      Xfprintf(stderr, "XSetFunction(m_display, oldgrid->gc2, GXandInverted)\n");
      XSetFunction(m_display, oldgrid->gc2, GXandInverted);
      break;
    case m_erase:
      Xfprintf(stderr, "XSetFunction(m_display, newgrid->gc1, GXclear)\n");
      XSetFunction(m_display, newgrid->gc1, GXclear);

      Xfprintf(stderr, "XSetFunction(m_display, oldgrid->gc1, GXclear)\n");
      XSetFunction(m_display, oldgrid->gc1, GXclear);
      break;
    case m_trans:
      break;
    case m_hitdet:
      break;
    default:
      fprintf(stderr, "mylib: unsupported m_colormode requested (%d)\n", c);
      break;
    }
    currentmode = c;
  }
}

void m_color(newcolor)
int newcolor;
{
  Mfprintf(stderr, "m_color(%d)\n", newcolor);

  if (newcolor != currentcolor) {
    if (newcolor != m_trans) {
      if (newcolor > m_maxcolor)
	newcolor = m_maxcolor;
      else if (newcolor < 0)
	newcolor = 0;
    }
    currentcolor = newcolor;
  }
}

long m_curcolor()
{
  Mfprintf(stderr, "m_curcolor() = %d\n", currentcolor);

  return(currentcolor);
}

long m_curcolormode()
{
  Mfprintf(stderr, "m_curcolormode() = %d\n", currentmode);

  return(currentmode);
}

void m_setcolor(c, r, g, b)
int c, r, g, b;
{

  unsigned char *d;
#ifdef HIRES
  unsigned long pixelvalue;
#endif  /* HIRES */
  Mfprintf(stderr, "m_setcolor(%d, %d, %d, %d)\n", c, r, g, b);

  if (BlackAndWhite) {
    if ((c == 0) || ((r < 2) && (g < 2) && (b < 2))) {
      /* background or black */
      XSetBackground(m_display,gc[c],WhitePixel(m_display,screennum));
      XSetForeground(m_display,gc[c],BlackPixel(m_display,screennum));
    } else {
      XSetBackground(m_display,gc[c],BlackPixel(m_display,screennum));
      XSetForeground(m_display,gc[c],WhitePixel(m_display,screennum));
    }
    if ((d = FigureOutBWLine(r, g, b)) != NULL) {
      default_linestyle[b] = d;
      XSetDashes(m_display, gc[c], 0, (char*)d, LINESTIPPLELENGTH);
      XSetLineAttributes(m_display,gc[c],1,LineDoubleDash,
			 CapButt,JoinMiter);
    } else {
      XSetLineAttributes(m_display, gc[c], 1, LineSolid,
			 CapButt, JoinMiter);
    }
  };

  if (c >= 0 && c <= m_maxcolor) {
    m_colors[ColorSets][c].red = r*4369;
    m_colors[ColorSets][c].green = g*4369;
    m_colors[ColorSets][c].blue = b*4369;
#ifdef HIRES
    if ((!BlackAndWhite) && (grCurrent.depth > 8)){
               if((grCurrent.depth == 24) & (red_mask == 0xff)) {
               /* this is SUN Solaris doing it backwards: BGR  */
                pixelvalue = (m_colors[ColorSets][c].red >> (16 - red_size))
                    & red_mask;
                pixelvalue |= ((m_colors[ColorSets][c].green >> (16 - green_size))
                    << red_size) & green_mask;
                m_colors[ColorSets][c].pixel = pixelvalue |
		  (((m_colors[ColorSets][c].blue >> (16 - blue_size))
                    << (red_size + green_size)) & blue_mask); }
               else {
      pixelvalue = ((m_colors[ColorSets][c].red >> (16 - red_size))
          << (green_size + blue_size)) & red_mask;
      pixelvalue |= ((m_colors[ColorSets][c].green >> (16 - green_size))
          << blue_size) & green_mask;
      m_colors[ColorSets][c].pixel = pixelvalue |
        ((m_colors[ColorSets][c].blue >> (16 - blue_size)) & blue_mask); }
      XSetForeground(m_display, gc[c], m_colors[ColorSets][c].pixel); }
    else {
      if ((!BlackAndWhite) && (grCurrent.depth <= 8)) {
        Xfprintf(stderr, "XStoreColor()\n");
        XStoreColor(m_display, colormap, &m_colors[ColorSets][c]);
      }
    }
#else
    if(!BlackAndWhite) {
      Xfprintf(stderr, "XStoreColor()\n");
      XStoreColor(m_display, colormap, &m_colors[ColorSets][c]);
    }
#endif  /* HIRES */
  }
}








void m_seecolor(c, r, g, b)
int c, *r, *g, *b;
{
  Mfprintf(stderr, "m_seecolor(%d)\n", c);

  if (c >= 0 && c <= m_maxcolor) {
    *r = m_colors[ColorSets][c].red / 4369;
    *g = m_colors[ColorSets][c].green / 4369;
    *b = m_colors[ColorSets][c].blue / 4369;
  } else
    *r = *g = *b = 0;
}

void m_setcolors(r, g, b)
m_colorarray r, g, b;
{
  int i;
  unsigned char *d;
#ifdef HIRES
  unsigned long pixelvalue;
#endif  /* HIRES */

  Mfprintf(stderr, "m_setcolors(r, g, b)\n");

  for (i = 0; i <= m_maxcolor; i++) {
    if (BlackAndWhite) {
      if ((i == 0) || ((r[i+1] < 2) && (g[i+1] < 2) && (b[i+1] < 2))) {
	/* background */
	XSetBackground(m_display,gc[i],WhitePixel(m_display,screennum));
	XSetForeground(m_display,gc[i],BlackPixel(m_display,screennum));
      } else {
	XSetBackground(m_display,gc[i],BlackPixel(m_display,screennum));
	XSetForeground(m_display,gc[i],WhitePixel(m_display,screennum));
      }
      if ((d = FigureOutBWLine(r[i+1], g[i+1], b[i+1])) != NULL) {
	default_linestyle[i] = d;
	XSetDashes(m_display, gc[i], 0, (char*)d, LINESTIPPLELENGTH);
	XSetLineAttributes(m_display,gc[i],1,LineDoubleDash,
			   CapButt,JoinMiter);
      } else {
	XSetLineAttributes(m_display, gc[i], 1, LineSolid,
 			   CapButt, JoinMiter);
      }
    }
    m_colors[ColorSets][i].red = r[i+1]*4369;
    m_colors[ColorSets][i].green = g[i+1]*4369;
    m_colors[ColorSets][i].blue = b[i+1]*4369;
#ifdef HIRES
    if ((!BlackAndWhite) && (grCurrent.depth > 8)){
               if((grCurrent.depth == 24) & (red_mask == 0xff)) {
               /* this is SUN Solaris doing it backwards: BGR  */
                pixelvalue = (m_colors[ColorSets][i].red >> (16 - red_size))
                    & red_mask;
                pixelvalue |= ((m_colors[ColorSets][i].green >> (16 - green_size))
                    << red_size) & green_mask;
                m_colors[ColorSets][i].pixel = pixelvalue |
		  (((m_colors[ColorSets][i].blue >> (16 - blue_size))
                    << (red_size + green_size)) & blue_mask); }
               else {
      pixelvalue = ((m_colors[ColorSets][i].red >> (16 - red_size))
          << (green_size + blue_size)) & red_mask;
      pixelvalue |= ((m_colors[ColorSets][i].green >> (16 - green_size))
          << blue_size) & green_mask;
      m_colors[ColorSets][i].pixel = pixelvalue |
        ((m_colors[ColorSets][i].blue >> (16 - blue_size)) & blue_mask); }
      XSetForeground(m_display, gc[i], m_colors[ColorSets][i].pixel); }
  }
  if ((!BlackAndWhite) && (grCurrent.depth <= 8)){
    Xfprintf(stderr, "XStoreColors()\n");
    XStoreColors(m_display, colormap, m_colors[ColorSets], m_maxcolor+1);
  } 
#else
  if(!BlackAndWhite) {
    Xfprintf(stderr, "XStoreColors()\n");
    XStoreColors(m_display, colormap, m_colors[ColorSets], m_maxcolor+1);
  }
#endif  /* HIRES */
}

void m_seecolors(r, g, b)
m_colorarray r, g, b;
{
  int i;

  Mfprintf(stderr, "m_seecolors(r, g, b)\n");

  for (i = 0; i <= m_maxcolor; i++) {
    r[i+1] = m_colors[ColorSets][i].red / 4369;
    g[i+1] = m_colors[ColorSets][i].green / 4369;
    b[i+1] = m_colors[ColorSets][i].blue / 4369;
  }
}

void m_vsetcolors(first, num, r, g, b)
int first, num;
m_vcolorarray r, g, b;
{
  int i;
  unsigned char *d;
#ifdef HIRES
  unsigned long pixelvalue;
#endif  /* HIRES */

  Mfprintf(stderr, "m_vsetcolors(%d, %d, r, g, b)\n", first, num);

  if (first + num > m_maxcolor)
    num = m_maxcolor - first + 1;
  if (num < 0)
    return;
  for (i = first; i <= m_maxcolor && i < first+num; i++) {

    if (BlackAndWhite) {
      if ((i == 0) || ((r[i-first] < 2) && (g[i-first] < 2)
		       && (b[i-first] < 2))) {
	/* background */
	XSetBackground(m_display,gc[i],WhitePixel(m_display,screennum));
	XSetForeground(m_display,gc[i],BlackPixel(m_display,screennum));
      }
      else {
	XSetBackground(m_display,gc[i],BlackPixel(m_display,screennum));
	XSetForeground(m_display,gc[i],WhitePixel(m_display,screennum));
      }
      if ((d = FigureOutBWLine(r[i-first], g[i-first], b[i-first])) != NULL) {
	default_linestyle[i] = d;
	XSetDashes(m_display, gc[i], 0, (char*)d, LINESTIPPLELENGTH);
	XSetLineAttributes(m_display,gc[i],1,LineDoubleDash,
			   CapButt,JoinMiter);
      } else {
	XSetLineAttributes(m_display, gc[i], 1, LineSolid,
 			   CapButt, JoinMiter);
      }
    }

    m_colors[ColorSets][i].red = r[i-first]*257;
    m_colors[ColorSets][i].green = g[i-first]*257;
    m_colors[ColorSets][i].blue = b[i-first]*257;

#ifdef HIRES
    if ((!BlackAndWhite) && (grCurrent.depth > 8)) {
               if((grCurrent.depth == 24) & (red_mask == 0xff)) {
               /* this is SUN Solaris doing it backwards: BGR  */
                pixelvalue = (m_colors[ColorSets][i].red >> (16 - red_size))
                    & red_mask;
                pixelvalue |= ((m_colors[ColorSets][i].green >> (16 - green_size))
                    << red_size) & green_mask;
                m_colors[ColorSets][i].pixel = pixelvalue |
		  (((m_colors[ColorSets][i].blue >> (16 - blue_size))
                    << (red_size + green_size)) & blue_mask); }
               else {
      pixelvalue = ((m_colors[ColorSets][i].red >> (16 - red_size))
          << (green_size + blue_size)) & red_mask;
      pixelvalue |= ((m_colors[ColorSets][i].green >> (16 - green_size))
          << blue_size) & green_mask;
      m_colors[ColorSets][i].pixel = pixelvalue |
        ((m_colors[ColorSets][i].blue >> (16 - blue_size)) & blue_mask); }
      XSetForeground(m_display, gc[i], m_colors[ColorSets][i].pixel); }
  }
  if ((!BlackAndWhite) && (grCurrent.depth <= 8)) {
    Xfprintf(stderr, "XStoreColors()\n");
    XStoreColors(m_display, colormap, m_colors[ColorSets] + first, num);
  }
#else
  }
  if(!BlackAndWhite) {
    Xfprintf(stderr, "XStoreColors()\n");
    XStoreColors(m_display, colormap, m_colors[ColorSets] + first, num);
  }
#endif  /* HIRES */
}

void m_vseecolors(first, num, r, g, b)
int first, num;
m_colorarray r, g, b;
{
  int i;

  Mfprintf(stderr, "m_vseecolors(%d, %d, r, g, b)\n", first, num);

  for (i = first; i < first+num; i++) {
    r[i-first] = m_colors[ColorSets][i].red / 257;
    g[i-first] = m_colors[ColorSets][i].green / 257;
    b[i-first] = m_colors[ColorSets][i].blue / 257;
  }
}

static int linestyles[16] = {
  0xffff,
  0x8000,
  0x8080,
  0x8888,
  0xff00,
  0xf0f0,
  0xcccc,
  0xaaaa,
  0xfafa,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
};
static int currentlinestyle = 0;
static int linewidth = 0;
  
void m_linestyle(s)
int s;
{
  int st, i, j, onoff, firstisoff = 0;
  char dashlist[16];

  Mfprintf(stderr, "m_linestyle(%d)\n", s);

  if (s != currentlinestyle) {
#ifdef EXTRA_BUFFERING
    flush_buffers();
#endif /* EXTRA_BUFFERING */
    if (linestyles[s] == 0xffff) {
      for (i = 0; i < ColorsInSet; i++) {
	Xfprintf(stderr, "XSetLineAttributes(m_display, gc[%d])\n", i);
	if (default_linestyle[i] == NULL)
	  XSetLineAttributes(m_display, gc[i], linewidth, LineSolid,
			     CapButt, JoinMiter);
	else {
	  XSetDashes(m_display, gc[i], 0, 
		     (char*)default_linestyle[i], LINESTIPPLELENGTH);
	  XSetLineAttributes(m_display,gc[i],linewidth,LineDoubleDash,
			     CapButt,JoinMiter);
	}
      }
      Xfprintf(stderr, "XSetLineAttributes(m_display, newgrid->gc1)\n");
      XSetLineAttributes(m_display, newgrid->gc1, linewidth, LineSolid,
			 CapButt, JoinMiter);
      Xfprintf(stderr, "XSetLineAttributes(m_display, newgrid->gc2)\n");
      XSetLineAttributes(m_display, newgrid->gc2, linewidth, LineSolid,
			 CapButt, JoinMiter);
      Xfprintf(stderr, "XSetLineAttributes(m_display, oldgrid->gc1)\n");
      XSetLineAttributes(m_display, oldgrid->gc1, linewidth, LineSolid,
			 CapButt, JoinMiter);
      Xfprintf(stderr, "XSetLineAttributes(m_display, oldgrid->gc2)\n");
      XSetLineAttributes(m_display, oldgrid->gc2, linewidth, LineSolid,
			 CapButt, JoinMiter);
    } else {
      st = linestyles[s];
      for (i = 0, j = -1, onoff = -1; i<16; i++, st <<= 1) {
	if ((st & (1 << 15)) == onoff)
	  dashlist[j]++;
        else {
	  if ((onoff == -1) && (!(st & (1 << 15))))
	    firstisoff = 1;
	  
	  onoff = (st & (1 << 15));
	  dashlist[++j] = 1;
	  
	}
      }
      if (firstisoff) {
	if (onoff)
	  dashlist[j+1] = dashlist[0];
	else
	  dashlist[j--] += dashlist[0];
	for (i = 0; i <= j; i++)
	  dashlist[i] = dashlist[i+1];
      }
      
      if (! (j % 2))
	dashlist[0] += dashlist[j--];
      if (! currentlinestyle) {
	for (i = 0; i < ColorsInSet; i++) {
	  Xfprintf(stderr, "XSetLineAttributes(m_display, gc[%d])\n", i);
	  if (default_linestyle[i] == NULL)
	  XSetLineAttributes(m_display, gc[i], linewidth, LineOnOffDash,
			     CapButt, JoinMiter);
	  else {
	    XSetDashes(m_display, gc[i], 0, 
		       (char*)default_linestyle[i], LINESTIPPLELENGTH);
	    XSetLineAttributes(m_display,gc[i],linewidth,LineDoubleDash,
			       CapButt,JoinMiter);
	  }
	}
	Xfprintf(stderr, "XSetLineAttributes(m_display, newgrid->gc1)\n");
	XSetLineAttributes(m_display, newgrid->gc1, linewidth, LineOnOffDash,
			   CapButt, JoinMiter);     
	Xfprintf(stderr, "XSetLineAttributes(m_display, newgrid->gc2)\n");
	XSetLineAttributes(m_display, newgrid->gc2, linewidth, LineOnOffDash,
			   CapButt, JoinMiter);     
	Xfprintf(stderr, "XSetLineAttributes(m_display, oldgrid->gc1)\n");
	XSetLineAttributes(m_display, oldgrid->gc1, linewidth, LineOnOffDash,
			   CapButt, JoinMiter);     
	Xfprintf(stderr, "XSetLineAttributes(m_display, oldgrid->gc2)\n");
	XSetLineAttributes(m_display, oldgrid->gc2, linewidth, LineOnOffDash,
			   CapButt, JoinMiter);     
      }
      for (i = 0; i < ColorsInSet; i++) {
	Xfprintf(stderr, "XSetDashes(m_display, gc[%d])\n", i);
	XSetDashes(m_display, gc[i], 0, dashlist, j+1);
      }
      Xfprintf(stderr, "XSetDashes(m_display, newgrid->gc1)\n");
      XSetDashes(m_display, newgrid->gc1, 0, dashlist, j+1);

      Xfprintf(stderr, "XSetDashes(m_display, newgrid->gc2)\n");
      XSetDashes(m_display, newgrid->gc2, 0, dashlist, j+1);

      Xfprintf(stderr, "XSetDashes(m_display, oldgrid->gc1)\n");
      XSetDashes(m_display, oldgrid->gc1, 0, dashlist, j+1);

      Xfprintf(stderr, "XSetDashes(m_display, oldgrid->gc2)\n");
      XSetDashes(m_display, oldgrid->gc2, 0, dashlist, j+1);
    }
    currentlinestyle = s;
  }
}

void m_nolinestyle()
{
  int i;

  Mfprintf(stderr, "m_nolinestyle()\n");

  if (currentlinestyle) {
#ifdef EXTRA_BUFFERING
    flush_buffers();
#endif /* EXTRA_BUFFERING */
    for (i = 0; i < ColorsInSet; i++) {
      Xfprintf(stderr, "XSetLineAttributes(m_display, gc[%d])\n", i);
      XSetLineAttributes(m_display, gc[i], linewidth, LineSolid, CapButt, JoinMiter);
    }
    Xfprintf(stderr, "XSetLineAttributes(m_display, newgrid->gc1)\n");
    XSetLineAttributes(m_display, newgrid->gc1, linewidth, LineSolid, CapButt, JoinMiter);

    Xfprintf(stderr, "XSetLineAttributes(m_display, newgrid->gc2)\n");
    XSetLineAttributes(m_display, newgrid->gc2, linewidth, LineSolid, CapButt, JoinMiter);

    Xfprintf(stderr, "XSetLineAttributes(m_display, oldgrid->gc1)\n");
    XSetLineAttributes(m_display, oldgrid->gc1, linewidth, LineSolid, CapButt, JoinMiter);

    Xfprintf(stderr, "XSetLineAttributes(m_display, oldgrid->gc2)\n");
    XSetLineAttributes(m_display, oldgrid->gc2, linewidth, LineSolid, CapButt, JoinMiter);
    currentlinestyle = 0;
  }
}

long m_curlinestyle()
{
  Mfprintf(stderr, "m_curlinestyle()\n");

  return(currentlinestyle);
}

void m_setlinestyle(s, mask)
int s, mask;
{
  Mfprintf(stderr, "m_setlinestyle(%d, %d)\n", s, mask);

  if (s >= 0 && s <16) {
    linestyles[s] = mask;
    if (s == currentlinestyle) {
      currentlinestyle = -1;
      m_linestyle(s);
    }
  } else
    fprintf(stderr, "mylib:  invalid linestyle number (%d) passed to m_setlinestyle", s);
}

void m_seelinestyle(s, mask)
int s, *mask;
{
  Mfprintf(stderr, "m_seelinestyle(%d, mask)\n", s);

  *mask = linestyles[s];
}

void m_linewidth(w)
int w;
{
  int i;

  Mfprintf(stderr, "m_linewidth(%d)\n", w);

  if (w <= 1)
    w = 0;
  if (w != linewidth) {
#ifdef EXTRA_BUFFERING
    flush_buffers();
#endif /* EXTRA_BUFFERING */
    linewidth = w;
    for (i = 0; i < ColorsInSet; i++)
      if (linestyles[currentlinestyle] == 65535) {
	Xfprintf(stderr, "XSetLineAttributes(m_display, gc[%d])\n", i);
	if (default_linestyle[i] == NULL)
	  XSetLineAttributes(m_display, gc[i], linewidth, LineSolid,
			     CapButt, JoinMiter);
	else {
	  XSetDashes(m_display, gc[i], 0, 
		     (char*)default_linestyle[i], LINESTIPPLELENGTH);
	  XSetLineAttributes(m_display,gc[i],linewidth,LineDoubleDash,
			     CapButt,JoinMiter);
 	}
      } else {
	Xfprintf(stderr, "XSetLineAttributes(m_display, gc[%d])\n", i);
	XSetLineAttributes(m_display, gc[i], linewidth, LineOnOffDash,
			   CapButt, JoinMiter);
      }
    if (linestyles[currentlinestyle] == 65535) {
      Xfprintf(stderr, "XSetLineAttributes(m_display, newgrid->gc1)\n");
      XSetLineAttributes(m_display, newgrid->gc1, linewidth, LineSolid,
			 CapButt, JoinMiter);

      Xfprintf(stderr, "XSetLineAttributes(m_display, newgrid->gc2)\n");
      XSetLineAttributes(m_display, newgrid->gc2, linewidth, LineSolid,
			 CapButt, JoinMiter);

      Xfprintf(stderr, "XSetLineAttributes(m_display, oldgrid->gc1)\n");
      XSetLineAttributes(m_display, oldgrid->gc1, linewidth, LineSolid,
			 CapButt, JoinMiter);

      Xfprintf(stderr, "XSetLineAttributes(m_display, oldgrid->gc2)\n");
      XSetLineAttributes(m_display, oldgrid->gc2, linewidth, LineSolid,
			 CapButt, JoinMiter);
    } else {
      Xfprintf(stderr, "XSetLineAttributes(m_display, newgrid->gc1)\n");
      XSetLineAttributes(m_display, newgrid->gc1, linewidth, LineOnOffDash,
			 CapButt, JoinMiter);

      Xfprintf(stderr, "XSetLineAttributes(m_display, newgrid->gc2)\n");
      XSetLineAttributes(m_display, newgrid->gc2, linewidth, LineOnOffDash,
			 CapButt, JoinMiter);

      Xfprintf(stderr, "XSetLineAttributes(m_display, oldgrid->gc1)\n");
      XSetLineAttributes(m_display, oldgrid->gc1, linewidth, LineOnOffDash,
			 CapButt, JoinMiter);

      Xfprintf(stderr, "XSetLineAttributes(m_display, oldgrid->gc2)\n");
      XSetLineAttributes(m_display, oldgrid->gc2, linewidth, LineOnOffDash,
			 CapButt, JoinMiter);
    }
  }
}

void m_nolinewidth()
{
  int i;

  Mfprintf(stderr, "m_nolinewidth()\n");

  if (linewidth) {
#ifdef EXTRA_BUFFERING
    flush_buffers();
#endif /* EXTRA_BUFFERING */
    linewidth = 0;
    for (i = 0; i < ColorsInSet; i++) {
      if (currentlinestyle == 0) {
	Xfprintf(stderr, "XSetLineAttributes(m_display, gc[%d])\n", i);
	if (default_linestyle[i] == NULL)
	  XSetLineAttributes(m_display, gc[i], linewidth, LineSolid,
			     CapButt, JoinMiter);
	else {
	  XSetDashes(m_display, gc[i], 0, 
		     (char*)default_linestyle[i], LINESTIPPLELENGTH);
	  XSetLineAttributes(m_display, gc[i], linewidth, LineDoubleDash,
			     CapButt, JoinMiter);
 	}
      } else {
	Xfprintf(stderr, "XSetLineAttributes(m_display, gc[%d])\n", i);
	XSetLineAttributes(m_display, gc[i], linewidth, LineOnOffDash,
			   CapButt, JoinMiter);
      }
      if (currentlinestyle == 0) {
	Xfprintf(stderr, "XSetLineAttributes(m_display, newgrid->gc1)\n");
	XSetLineAttributes(m_display, newgrid->gc1, linewidth, LineSolid,
			   CapButt, JoinMiter);

	Xfprintf(stderr, "XSetLineAttributes(m_display, newgrid->gc2)\n");
	XSetLineAttributes(m_display, newgrid->gc2, linewidth, LineSolid,
			   CapButt, JoinMiter);

	Xfprintf(stderr, "XSetLineAttributes(m_display, oldgrid->gc1)\n");
	XSetLineAttributes(m_display, oldgrid->gc1, linewidth, LineSolid,
			   CapButt, JoinMiter);

	Xfprintf(stderr, "XSetLineAttributes(m_display, oldgrid->gc2)\n");
	XSetLineAttributes(m_display, oldgrid->gc2, linewidth, LineSolid,
			   CapButt, JoinMiter);
      } else {
	Xfprintf(stderr, "XSetLineAttributes(m_display, newgrid->gc1)\n");
	XSetLineAttributes(m_display, newgrid->gc1, linewidth, LineOnOffDash,
			   CapButt, JoinMiter);

	Xfprintf(stderr, "XSetLineAttributes(m_display, newgrid->gc2)\n");
	XSetLineAttributes(m_display, newgrid->gc2, linewidth, LineOnOffDash,
			   CapButt, JoinMiter);

	Xfprintf(stderr, "XSetLineAttributes(m_display, oldgrid->gc1)\n");
	XSetLineAttributes(m_display, oldgrid->gc1, linewidth, LineOnOffDash,
			   CapButt, JoinMiter);

	Xfprintf(stderr, "XSetLineAttributes(m_display, oldgrid->gc2)\n");
	XSetLineAttributes(m_display, oldgrid->gc2, linewidth, LineOnOffDash,
			   CapButt, JoinMiter);
      }
    }
  }
}

static int curx = 0, cury = 0;

void m_move(x, y)
int x, y;
{
  Mfprintf(stderr, "m_move(%d, %d)\n", x, y);

  TRNSFRM(x, y);

  curx = x;
  cury = y;
}

void m_moverel(dx, dy)
int dx, dy;
{
  Mfprintf(stderr, "m_moverel(%d, %d)\n", dx, dy);

  DTRNSFRM(dx, dy);

  curx += dx;
  cury += dy;
}

int hitdet_line(x1, y1, x2, y2)
int x1, y1, x2, y2;
{
  int oc1 = 0, oc2 = 0;
  int val;
  static int table[9][9] = {
    0, 0, 0,  0, 1, 2,  0, 4, 4,
    0, 0, 0,  4, 1, 5,  4, 1, 5,
    0, 0, 0,  2, 1, 0,  5, 5, 0,
    0, 2, 2,  0, 1, 1,  0, 4, 4,
    1, 1, 1,  1, 1, 1,  1, 1, 1,
    2, 2, 0,  1, 1, 0,  5, 5, 0,
    0, 4, 4,  0, 1, 4,  0, 0, 0,
    4, 1, 5,  4, 1, 5,  0, 0, 0,
    5, 5, 0,  5, 1, 0,  0, 0, 0
  };

  if (x1 > m_clip_x2)
    oc1 += 2;
  else if (x1 >= m_clip_x1)
    oc1 += 1;
  if (y1 > m_clip_y2)
    oc1 += 6;
  else if (y1 >= m_clip_y1)
    oc1 += 3;
  if (x2 > m_clip_x2)
    oc2 += 2;
  else if (x2 >= m_clip_x1)
    oc2 += 1;
  if (y2 > m_clip_y2)
    oc2 += 6;
  else if (y2 >= m_clip_y1)
    oc2 += 3;
  switch (table[oc1][oc2]) {

  case 0:   /* trivial reject */
    return 0;

  case 1:   /* trivial accept */
    return 1;

  case 2:   /* crosses m_clip_y1 */
    val = x1 + (x2-x1)*(m_clip_y1-y1)/(y2-y1);
    if (y1 < y2)
      return (hitdet_line(x1, y1, val, m_clip_y1-1) ||
	      hitdet_line(val, m_clip_y1, x2, y2));
    else
      return (hitdet_line(x2, y2, val, m_clip_y1-1) ||
	      hitdet_line(val, m_clip_y1, x1, y1));

  case 4:   /* crosses m_clip_x1 */
    val = y1 + (y2-y1)*(m_clip_x1-x1)/(x2-x1);
    if (x1 < x2)
      return (hitdet_line(x1, y1, m_clip_x1-1, val) ||
	      hitdet_line(m_clip_x1, val, x2, y2));
    else
      return (hitdet_line(x2, y2, m_clip_x1-1, val) ||
	      hitdet_line(m_clip_x1, val, x1, y1));

  case 5:   /* crosses m_clip_x2 */
    val = y1 + (y2-y1)*(m_clip_x2-x1)/(x2-x1);
    if (x1 < x2)
      return (hitdet_line(x1, y1, m_clip_x2, val) ||
	      hitdet_line(m_clip_x2+1, val, x2, y2));
    else
      return (hitdet_line(x2, y2, m_clip_x2, val) ||
	      hitdet_line(m_clip_x2+1, val, x1, y1));
  }
  return 0;  /* should never occur */
}

void m_draw(x, y)
int x, y;
{
  Mfprintf(stderr, "m_draw(%d, %d)\n", x, y);

  TRNSFRM(x, y);

  if (currentmode == m_hitdet) {
    m_hitcount += hitdet_line(curx, cury, x, y);
    curx = x;
    cury = y;
    return;
  }

#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoroff();
#endif

#ifdef EXTRA_BUFFERING
  buffer_line(currentcolor, curx, cury, x, y);
#else
  Xfprintf(stderr, "XDrawLine()\n");
  XDrawLine(m_display, m_window, gc[currentcolor], curx, cury, x, y);
#endif /* EXTRA_BUFFERING */
  curx = x;
  cury = y;

#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoron();
#endif

  if (nocache) {
    Ffprintf(stderr, "XFlush()\n");
    XFlush(m_display);
  }
}

void m_drawrel(dx, dy)
int dx, dy;
{
  Mfprintf(stderr, "m_drawrel(%d, %d)\n", dx, dy);

  DTRNSFRM(dx, dy);

  if (currentmode == m_hitdet) {
    m_hitcount += hitdet_line(curx, cury, curx+dx, cury+dy);
    curx += dx;
    cury += dy;
    return;
  }

#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoroff();
#endif

#ifdef EXTRA_BUFFERING
  buffer_line(currentcolor, curx, cury, curx+dx, cury+dy);
#else
  Xfprintf(stderr, "XDrawLine()\n");
  XDrawLine(m_display, m_window, gc[currentcolor], curx, cury, curx+dx, cury+dy);
#endif /* EXTRA_BUFFERING */
  curx += dx;
  cury += dy;

#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoron();
#endif

  if (nocache) {
    Ffprintf(stderr, "XFlush()\n");
    XFlush(m_display);
  }
}

void m_move2(x, y)
int x, y;
{
  Mfprintf(stderr, "m_move2(%d, %d)\n", x, y);

  TRNSFRM(x, y);

  curx = x;
  cury = y;
}

void m_moverel2(dx, dy)
int dx, dy;
{
  Mfprintf(stderr, "m_moverel2(%d, %d)\n", dx, dy);

  DTRNSFRM(dx, dy);

  curx += dx;
  cury += dy;
}

void m_seeposn(x, y)
int *x, *y;
{
  int tx, ty;

  Mfprintf(stderr, "m_seeposn(x, y)\n");

  tx = curx;
  ty = cury;

  UNTRNSFRM(tx, ty);

  *x = curx;
  *y = cury;
}

void m_drawline(x1, y1, x2, y2)
int x1, y1, x2, y2;
{
  Mfprintf(stderr, "m_drawline(%d, %d, %d, %d)\n", x1, y1, x2, y2);

  TRNSFRM(x1, y1);
  TRNSFRM(x2, y2);

  if (currentmode == m_hitdet) {
    m_hitcount += hitdet_line(x1, y1, x2, y2);
    return;
  }

#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoroff();
#endif

#ifdef EXTRA_BUFFERING
  if ((x1 == x2) && (y1 == y2))
    buffer_point(currentcolor, x1, y1);
  else
    buffer_line(currentcolor, x1, y1, x2, y2);
#else
  Xfprintf(stderr, "XDrawLine()\n");
  if ((x1 == x2) && (y1 == y2))
    XDrawPoint(m_display, m_window, gc[currentcolor], x1, y1);
  else
    XDrawLine(m_display, m_window, gc[currentcolor], x1, y1, x2, y2);
#endif /* EXTRA_BUFFERING */

#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoron();
#endif
  if (nocache) {
    Ffprintf(stderr, "XFlush()\n");
    XFlush(m_display);
  }
}

int hitdet_drawrect(x1, y1, x2, y2)
int x1, y1, x2, y2;
{
  if (((x1 >= m_clip_x1 && x1 <= m_clip_x2) ||
       (x2 >= m_clip_x1 && x2 <= m_clip_x2)) &&
      (y1 >= m_clip_y1 || y2 >= m_clip_y1) &&
      (y1 <= m_clip_y2 || y2 <= m_clip_y2))
    return 1;
  if (((y1 >= m_clip_y1 && y1 <= m_clip_y2) ||
       (y2 >= m_clip_y1 && y2 <= m_clip_y2)) &&
      (x1 >= m_clip_x1 || x2 >= m_clip_x1) &&
      (x1 <= m_clip_x2 || x2 <= m_clip_x2))
    return 1;
  return 0;
}

void m_drawrect(x1, y1, x2, y2)
int x1, y1, x2, y2;
{
  int x, y;

  Mfprintf(stderr, "m_drawrect(%d, %d, %d, %d)\n", x1, y1, x2, y2);

  LTRNSFRM(x1, y1);
  LTRNSFRM(x2, y2);

  if (currentmode == m_hitdet) {
    m_hitcount += hitdet_drawrect(x1, y1, x2, y2);
    return;
  }

  x = MIN(x1,x2);
  y = MIN(y1,y2);

#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoroff();
#endif
  
#ifdef EXTRA_BUFFERING
  if (x1 == x2)
    if (y1 == y2)
      buffer_point(currentcolor, x, y);
    else
      buffer_line(currentcolor, x1, y1, x1, y2);
  else
    if (y1 == y2)
      buffer_line(currentcolor, x1, y1, x2, y1);
    else
      buffer_rect(currentcolor, x, y, x1+x2-x-x, y1+y2-y-y);
#else
  Xfprintf(stderr, "XDrawRectangle()\n");
  if (x1 == x2)
    if (y1 == y2)
      XDrawPoint(m_display, m_window, gc[currentcolor], x, y);
    else
      XDrawLine(m_display, m_window, gc[currentcolor], x1, y1, x1, y2);
  else
    if (y1 == y2)
      XDrawLine(m_display, m_window, gc[currentcolor], x1, y1, x2, y1);
    else
      XDrawRectangle(m_display, m_window, gc[currentcolor],
		     x, y, x1+x2-x-x, y1+y2-y-y);
#endif /* EXTRA_BUFFERING */

#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoron();
#endif

  if (nocache) {
    Ffprintf(stderr, "XFlush()\n");
    XFlush(m_display);
  }
}

int hitdet_fillrect(x1, y1, x2, y2)
int x1, y1, x2, y2;
{
  return ((x1 >= m_clip_x1 || x2 >= m_clip_x1) &&
	  (x1 <= m_clip_x2 || x2 <= m_clip_x2) &&
	  (y1 >= m_clip_y1 || y2 >= m_clip_y1) &&
	  (y1 <= m_clip_y2 || y2 <= m_clip_y2));
}

void m_fillrect(x1, y1, x2, y2)
int x1, y1, x2, y2;
{
  int x, y;

  Mfprintf(stderr, "m_fillrect(%d, %d, %d, %d)\n", x1, y1, x2, y2);

  LTRNSFRM(x1, y1);
  LTRNSFRM(x2, y2);

  if (currentmode == m_hitdet) {
    m_hitcount += hitdet_fillrect(x1, y1, x2, y2);
    return;
  }

  x = MIN(x1,x2);
  y = MIN(y1,y2);

#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoroff();
#endif
  
#ifdef EXTRA_BUFFERING
  if (x1 == x2)
    if (y1 == y2)
      buffer_point(currentcolor, x, y);
    else
      buffer_line(currentcolor, x1, y1, x1, y2);
  else
    if (y1 == y2)
      buffer_line(currentcolor, x1, y1, x2, y1);
    else
      buffer_fillrect(currentcolor, x, y, x1+x2-x-x+1, y1+y2-y-y+1);
#else
  Xfprintf(stderr, "XFillRectangle()\n");
  if (x1 == x2)
    if (y1 == y2)
      XDrawPoint(m_display, m_window, gc[currentcolor], x, y);
    else
      XDrawLine(m_display, m_window, gc[currentcolor], x1, y1, x1, y2);
  else
    if (y1 == y2)
      XDrawLine(m_display, m_window, gc[currentcolor], x1, y1, x2, y1);
    else
      XFillRectangle(m_display, m_window, gc[currentcolor],
		     x, y, x1+x2-x-x+1, y1+y2-y-y+1);
#endif /* EXTRA_BUFFERING */

#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoron();
#endif

  if (nocache) {
    Ffprintf(stderr, "XFlush()\n");
    XFlush(m_display);
  }
}



void m_grid(x1, y1, x2, y2, dx, dy, ax, ay)
int x1, y1, x2, y2, dx, dy, ax, ay;
{
  struct grid *swapgrid;
  int x, y, wid, hei, i, j;
  GC gc2;

  Mfprintf(stderr, "m_grid(%d, %d, %d, %d, %d, %d, %d, %d)\n",
                           x1, y1, x2, y2, dx, dy, ax, ay   );

  LTRNSFRM(x1, y1);
  LTRNSFRM(x2, y2);
  DTRNSFRM(dx, dy);
  TRNSFRM(ax, ay);

#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoroff();
#endif

  dx = abs(dx);
  dy = abs(dy);

  if (dx < 60 && dy < 60) {
    x = MIN(x1, x2);
    y = MIN(y1, y2);
    wid = x1+x2-x-x+1;
    hei = y1+y2-y-y+1;
    if (dx == 0 || dy == 0 || (dx == 1 && dy == 1)) {
      x = MIN(x1,x2);
      y = MIN(y1,y2);
      Xfprintf(stderr, "XFillRectangle()\n");
      XFillRectangle(m_display, m_window, gc[currentcolor], x, y, wid, hei);
      if (nocache) {
	Ffprintf(stderr, "XFlush()\n");
	XFlush(m_display);
      }
      return;
    }
    if (dx == 1) {
      if (y > ay)
	i = (y - ay) % dy + y;
      else
	i = y - (ay - y) % dy;
      for (; i < y+hei; i += dy) {
	Xfprintf(stderr, "XDrawLine()\n");
	XDrawLine(m_display, m_window, gc[currentcolor], x1, i, x2, i);
      }
      if (nocache) {
	Ffprintf(stderr, "XFlush()\n");
	XFlush(m_display);
      }
      return;
    }
    if (dy == 1) {
      if (x > ax)
	i = (x - ax) % dx + x;
      else
	i = x - (ax - x) % dx;
      for (; i < x+wid; i += dx)
	Xfprintf(stderr, "XDrawLine()\n");
	XDrawLine(m_display, m_window, gc[currentcolor], i, y1, i, y2);
      if (nocache) {
	Ffprintf(stderr, "XFlush()\n");
	XFlush(m_display);
      }
      return;
    }

    if (dx != newgrid->dx || dy != newgrid->dy || currentcolor != newgrid->color) {
      swapgrid = oldgrid;
      oldgrid = newgrid;
      newgrid = swapgrid;
    }
    if (dx != newgrid->dx || dy != newgrid->dy || currentcolor != newgrid->color) {
      if(newgrid->p1) {
	Xfprintf(stderr, "XFreePixmap(m_display, newgrid->p1)\n");
	XFreePixmap(m_display, newgrid->p1);
	Xfprintf(stderr, "XFreePixmap(m_display, newgrid->p2)\n");
	XFreePixmap(m_display, newgrid->p2);
      }
      Xfprintf(stderr, "newgrid->p1 = XCreatePixmap()\n");
      newgrid->p1 = XCreatePixmap(m_display, m_window, 60/dx*dx, 60/dy*dy, RealWinDepth);
      Xfprintf(stderr, "newgrid->p2 = XCreatePixmap()\n");
      newgrid->p2 = XCreatePixmap(m_display, m_window, 60/dx*dx, 60/dy*dy, RealWinDepth);
      newgrid->color = currentcolor;
      
      Xfprintf(stderr, "gc2 = XCreateGC()\n");
      gc2 = XCreateGC(m_display, newgrid->p1, 0, NULL);
      
      Xfprintf(stderr, "XSetForeground(m_display, gc2, 0)\n");
      XSetForeground(m_display, gc2, 0);
      Xfprintf(stderr, "XFillRectangle()\n");
      XFillRectangle(m_display, newgrid->p1, gc2, 0, 0, 60/dx*dx, 60/dy*dy);
      Xfprintf(stderr, "XSetForeground(m_display, gc2, -1)\n");
      XSetForeground(m_display, gc2, -1);
      Xfprintf(stderr, "XFillRectangle()\n");
      XFillRectangle(m_display, newgrid->p2, gc2, 0, 0, 60/dx*dx, 60/dy*dy);
      Xfprintf(stderr, "XSetForeground(m_display, gc2)\n");
      XSetForeground(m_display, gc2, m_colors[0][currentcolor].pixel);
      for (i = 0; i < 60/dx; i++)
	for (j = 0; j < 60/dy; j++) {
	  Xfprintf(stderr, "XDrawPoint()\n");
	  XDrawPoint(m_display, newgrid->p1, gc2, i*dx, j*dy);
	  Xfprintf(stderr, "XDrawPoint()\n");
	  XDrawPoint(m_display, newgrid->p2, gc2, i*dx, j*dy);
	}
      XFreeGC(m_display, gc2);

      Xfprintf(stderr, "XSetTile(m_display, newgrid->gc1, newgrid->p1)\n");
      XSetTile(m_display, newgrid->gc1, newgrid->p1);
      Xfprintf(stderr, "XSetTile(m_display, newgrid->gc2, newgrid->p2)\n");
      XSetTile(m_display, newgrid->gc2, newgrid->p2);
    }

    if ((newgrid->ax != ax) || (newgrid->ay != ay)) {
      Xfprintf(stderr, "XSetTSOrigin(m_display, newgrid->gc1)\n");
      XSetTSOrigin(m_display, newgrid->gc1,
		   newgrid->ax = ax, newgrid->ay = ay);
      Xfprintf(stderr, "XSetTSOrigin(m_display, newgrid->gc2)\n");
      XSetTSOrigin(m_display, newgrid->gc2,
		   newgrid->ax = ax, newgrid->ay = ay);
    }
    newgrid->dx = dx;
    newgrid->dy = dy;

    switch (currentmode) {
    case m_normal:
      Xfprintf(stderr, "XFillRectangle()\n");
      XFillRectangle(m_display, m_window, newgrid->gc1, x, y, wid, hei);
      Xfprintf(stderr, "XFillRectangle()\n");
      XFillRectangle(m_display, m_window, newgrid->gc2, x, y, wid, hei);
      break;
    case m_xor:
    case m_over:
    case m_erase:
      Xfprintf(stderr, "XFillRectangle()\n");
      XFillRectangle(m_display, m_window, newgrid->gc1, x, y, wid, hei);
      break;
    case m_mask:
      Xfprintf(stderr, "XFillRectangle()\n");
      XFillRectangle(m_display, m_window, newgrid->gc2, x, y, wid, hei);
      break;
    case m_trans:
      break;
    }
  } else {
    if (x1>ax)
      ax = x1+dx-(x1-ax)%dx;
    else
      ax = x1+(ax-x1)%dx;
    if (y1>ay)
      ay = y1+dy-(y1-ay)%dy;
    else
      ay = y1+(ay-y1)%dy;

    for (j = y1; j <= y2; j += dy)
      for (i = x1; i <= x2; i += dx) {
	Xfprintf(stderr, "XDrawPoint()\n");
	XDrawPoint(m_display, m_window, gc[currentcolor], i, j);
      }
  }

#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoron();
#endif

  if (nocache) {
    Ffprintf(stderr, "XFlush()\n");
    XFlush(m_display);
  }
}

int hitdet_point(x, y)
int x, y;
{
  return (x >= m_clip_x1 && x <= m_clip_x2 &&
	  y >= m_clip_y1 && y <= m_clip_y2);
}

void m_drawpoint(x, y)
int x, y;
{
  TRNSFRM(x, y);

  Mfprintf(stderr, "m_drawpoint()\n");

  if (currentmode == m_hitdet) {
    m_hitcount += hitdet_point(x, y);
    return;
  }

#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoroff();
#endif

#ifdef EXTRA_BUFFERING
  buffer_point(currentcolor, x, y);
#else
  Xfprintf(stderr, "XDrawPoint()\n");
  XDrawPoint(m_display, m_window, gc[currentcolor], x, y);
#endif /* EXTRA_BUFFERING */

#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoron();
#endif

  if (nocache) {
    Ffprintf(stderr, "XFlush()\n");
    XFlush(m_display);
  }
}

int hitdet_ellipse(x, y, rx, ry, filled)
int x, y, rx, ry, filled;
{
  int clp_x1 = m_clip_x1, clp_x2 = m_clip_x2;
  int clp_y1 = m_clip_y1, clp_y2 = m_clip_y2;
  int rsq, xx, yy;

  if (rx == 0 || ry == 0)
    return hitdet_drawrect(x - rx, y - ry, x + rx, y + ry);
  if (ry > rx) {
    clp_x1 = clp_x1 * ry / rx;
    clp_x2 = clp_x2 * ry / rx;
    x = x * ry / rx;
    rx = ry;
  } else if (rx > ry) {
    clp_y1 = clp_y1 * rx / ry;
    clp_y2 = clp_y2 * rx / ry;
    y = y * rx / ry;
  }
  clp_x1 -= x;
  clp_x2 -= x;
  clp_y1 -= y;
  clp_y2 -= y;
  rsq = rx * rx;
  if (clp_x1 > 0) {
    if (clp_y1 > 0) {
      if (clp_x1*clp_x1 + clp_y1*clp_y1 > rsq)
	return 0;
      return (filled || clp_x2*clp_x2 + clp_y2*clp_y2 >= rsq);
    } else if (clp_y2 >= 0) {
      if (clp_x1 > rx)
	return 0;
      yy = (-clp_y1 > clp_y2) ? clp_y1 : clp_y2;
      return (filled || clp_x2*clp_x2 + yy*yy >= rsq);
    } else {
      if (clp_x1*clp_x1 + clp_y2*clp_y2 > rsq)
	return 0;
      return (filled || clp_x2*clp_x2 + clp_y1*clp_y1 >= rsq);
    }
  } else if (clp_x2 >= 0) {
    if (clp_y1 > 0) {
      if (clp_y1 > rx)
	return 0;
      xx = (-clp_x1 > clp_x2) ? clp_x1 : clp_x2;
      return (filled || clp_y2*clp_y2 + xx*xx >= rsq);
    } else if (clp_y2 >= 0) {
      if (filled)
	return 1;
      xx = (-clp_x1 > clp_x2) ? clp_x1 : clp_x2;
      yy = (-clp_y1 > clp_y2) ? clp_y1 : clp_y2;
      return (xx*xx + yy*yy >= rsq);
    } else {
      if (-clp_y2 > rx)
	return 0;
      xx = (-clp_x1 > clp_x2) ? clp_x1 : clp_x2;
      return (filled || clp_y1*clp_y1 + xx*xx >= rsq);
    }
  } else {
    if (clp_y1 > 0) {
      if (clp_x2*clp_x2 + clp_y1*clp_y1 > rsq)
	return 0;
      return (filled || clp_x1*clp_x1 + clp_y2*clp_y2 >= rsq);
    } else if (clp_y2 >= 0) {
      if (-clp_x2 > rx)
	return 0;
      yy = (-clp_y1 > clp_y2) ? clp_y1 : clp_y2;
      return (filled || clp_x1*clp_x1 + yy*yy >= rsq);
    } else {
      if (clp_x2*clp_x2 + clp_y2*clp_y2 > rsq)
	return 0;
      return (filled || clp_x1*clp_x1 + clp_y1*clp_y1 >= rsq);
    }
  }
}

void m_circle(x, y, r)
int x, y, r;
{
  Mfprintf(stderr, "m_circle(%d, %d, %d)\n", x, y, r);

  TRNSFRM(x, y);
  r = abs(r);

  if (currentmode == m_hitdet) {
    m_hitcount += hitdet_ellipse(x, y, r, r, 0);
    return;
  }

#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoroff();
#endif

  Xfprintf(stderr, "XDrawArc()\n");
  XDrawArc(m_display, m_window, gc[currentcolor], x-r, y-r, r*2, r*2, 0, 360*64);

#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoron();
#endif

  if (nocache) {
    Ffprintf(stderr, "XFlush()\n");
    XFlush(m_display);
  }
}

void m_ellipse(x, y, rx, ry, c)
int x, y, rx, ry, c;
{
  Mfprintf(stderr, "m_ellipse(%d, %d, %d, %d, %d)\n", x, y, rx, ry, c);

  TRNSFRM(x, y);
  DTRNSFRM(rx, ry);
  rx = abs(rx);
  ry = abs(ry);

  if (currentmode == m_hitdet) {
    m_hitcount += hitdet_ellipse(x, y, rx, ry, (currentcolor != m_trans));
    return;
  }

#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoroff();
#endif

  if (currentcolor != m_trans) {
    Xfprintf(stderr, "XFillArc()\n");
    XFillArc(m_display, m_window, gc[currentcolor], x-rx, y-ry, rx*2, ry*2, 0, 360*64);
  }

  if (c != m_trans) {
    Xfprintf(stderr, "XDrawArc()\n");
    XDrawArc(m_display, m_window, gc[c], x-rx, y-ry, rx*2, ry*2, 0, 360*64);
  }

#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoron();
#endif
  if (nocache) {
    Ffprintf(stderr, "XFlush()\n");
    XFlush(m_display);
  }
}

void m_ellipse2(x, y, rx, ry, c1, c2)
int x, y, rx, ry, c1, c2;
{
  Mfprintf(stderr, "m_ellipse2(%d, %d, %d, %d, %d, %d)\n", x, y, rx, ry, c1, c2);

  TRNSFRM(x, y);
  DTRNSFRM(rx, ry);
  rx = abs(rx);
  ry = abs(ry);

  if (currentmode == m_hitdet) {
    m_hitcount += hitdet_ellipse(x, y, rx, ry, (c2 != m_trans));
    return;
  }

#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoroff();
#endif

  if (c2 != m_trans) {
    Xfprintf(stderr, "XFillArc()\n");
    XFillArc(m_display, m_window, gc[c2], x-rx, y-ry, rx*2, ry*2, 0, 360*64);
  }

  if (c1 != m_trans) {
    Xfprintf(stderr, "XDrawArc()\n");
    XDrawArc(m_display, m_window, gc[c1], x-rx, y-ry, rx*2, ry*2, 0, 360*64);
  }

#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoron();
#endif
  if (nocache) {
    Ffprintf(stderr, "XFlush()\n");
    XFlush(m_display);
  }
}

void m_drawarc(x, y, rx, ry, theta1, theta2, rotate, chord)
long x, y, rx, ry;
double theta1, theta2, rotate;
long chord;
{
  double a, c1, s1, c2, s2, c3, s3, rc, rs, d, th1, th2, start, stop,
  rcc1, rcs1, rsc1, rss1, rcc2, rcs2, rsc2, rss2, temp;
  long q, quad1, quad2, chx, chy;
  boolean rotflag;
  
  Mfprintf(stderr, "m_drawarc(%ld, %ld, %ld, %ld, %f, %f, %f, %ld)\n",
                               x,   y,  rx, ry, theta1, theta2, rotate, chord);

#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoroff();
#endif
  
  if (rx < 0 && ry < 0) {
    temp = theta1;
    theta1 = theta2;
    theta2 = temp;
  }
  
  if (!(theta1 < theta2 && rx && ry))
    return;
  if (rx < 0)
    rx = -rx;
  if (ry < 0)
    ry = -ry;

  if ((rotflag = (rotate != 0))) {
    if (rx == ry) {
      rotflag = 0;
      theta1 += rotate;
      theta2 += rotate;
    } else {
      rs = sin(rotate*dr);
      rc = cos(rotate*dr);
    }
  }
  if (theta2 - theta1 >= 360.0) {
    theta1 = 0.0;
    theta2 = 360.0;
    s1 = 0.0;
    c1 = 1.0;
    if (chord != m_extarc) {
      if (rotflag)
	m_move2(x + (long)floor(rx*rc + 0.5), y + (long)floor(rx*rs + 0.5));
      else
	m_move2(x + rx, y);
    }
    chord = 0;
  } else {
    s1 = sin(theta1*dr);
    c1 = cos(theta1*dr);
    if (rotflag) {
      chx = x + (long)floor(rx * c1 * rc - ry * s1 * rs + 0.5);
      chy = y + (long)floor(ry * s1 * rc + rx * c1 * rs + 0.5);
    } else {
      chx = x + (long)floor(rx * c1 + 0.5);
      chy = y + (long)floor(ry * s1 + 0.5);
    }
    switch (chord) {
    case m_pie:
      m_move2(x, y);
      m_draw(chx, chy);
      break;
    case m_extarc:
      break;
    default:
      m_move(chx, chy);
    }
  }
  th1 = theta1-floor(theta1/90.0)*90.0;
  quad1 = (int)(theta1/90.0) & 3;
  th2 = theta2-floor(theta2/90.0)*90.0;
  quad2 = (int)(theta2/90.0) & 3;
  start = th1;
  if (quad2 < quad1 || (quad2 == quad1 && th2 <= th1))
    quad2 += 4;
  if (quad2 == quad1 + 1 && th2 <= th1) {
    quad2 = quad1;
    th2 += 90.0;
  }
  if (quad1 < quad2) {
    stop = 90.0;
    d = (90.0 - start) / 2;
  }
  for (q = quad1; q <= quad2; q++) {
    if (q == quad2) {
      stop = th2;
      s2 = sin(theta2*dr);
      c2 = cos(theta2*dr);
      d = (stop - start) / 2;
    } else {
      s2 = sin((stop+q*90)*dr);
      c2 = cos((stop+q*90)*dr);
    }
    if (d > 1e-5) {
      if (d == 45.0)
	a = 0.5522847498;
      else {
	s3 = sin(d*dr);
	c3 = cos(d*dr);
	a = 4 * (1 - c3) / (3 * s3);
      }
      if (rotflag) {
	rcc1 = rc * c1;
	rcs1 = rc * s1;
	rsc1 = rs * c1;
	rss1 = rs * s1;
	rcc2 = rc * c2;
	rcs2 = rc * s2;
	rsc2 = rs * c2;
	rss2 = rs * s2;
	m_cbezier(x + (long)floor(rx * rcc1 - ry * rss1 + 0.5),
		  y + (long)floor(ry * rcs1 + rx * rsc1 + 0.5),
		  x + (long)floor(rx * (rcc1 - a * rcs1) -
				  ry * (rss1 + a * rsc1) + 0.5),
		  y + (long)floor(ry * (rcs1 + a * rcc1) +
				  rx * (rsc1 - a * rss1) + 0.5),
		  x + (long)floor(rx * (rcc2 + a * rcs2) +
				  ry * (a * rsc2 - rss2) + 0.5),
		  y + (long)floor(ry * (rcs2 - a * rcc2) +
				  rx * (rsc2 + a * rss2) + 0.5),
		  x + (long)floor(rx * rcc2 - ry * rss2 + 0.5),
		  y + (long)floor(ry * rcs2 + rx * rsc2 + 0.5), 1000L);
      } else
	m_cbezier(x + (long)floor(rx * c1 + 0.5),
		  y + (long)floor(ry * s1 + 0.5),
		  x + (long)floor(rx * (c1 - a * s1) + 0.5),
		  y + (long)floor(ry * (s1 + a * c1) + 0.5),
		  x + (long)floor(rx * (c2 + a * s2) + 0.5),
		  y + (long)floor(ry * (s2 - a * c2) + 0.5),
		  x + (long)floor(rx * c2 + 0.5),
		  y + (long)floor(ry * s2 + 0.5), 1000L);
    }
    if (q < quad2) {
      start = 0.0;
      d = 45.0;
      s1 = s2;
      c1 = c2;
    }
  }
  switch (chord) {
  case m_pie:
    m_draw(x, y);
    break;
  case m_chord:
    m_draw(chx, chy);
    break;
  }

#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoron();
#endif
}

typedef long pts[506];

void m_fillarc(x, y, rx, ry, theta1, theta2, rotate, chord)
long x, y, rx, ry;
double theta1, theta2, rotate;
long chord;
{
  pts xp, yp;
  double ss, cc, rs, rc, rsx, rsy, rcx, rcy, th, temp;
  long i, n;
  boolean rotflag, fullflag;

  Mfprintf(stderr, "m_fillarc(%ld, %ld, %ld, %ld, %f, %f, %f, %ld)\n",
                               x,   y,  rx, ry, theta1, theta2, rotate, chord);

#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoroff();
#endif
  
  if (rx < 0 && ry < 0) {
    temp = theta1;
    theta1 = theta2;
    theta2 = temp;
  }
  if (!(theta1 < theta2 && rx && ry))
    return;
  if (rx < 0)
    rx = -rx;
  if (ry < 0)
    ry = -ry;
  rotflag = (rotate != 0);
  fullflag = (theta2 - theta1 >= 360.0);
  if (fullflag && !rotflag) {
    m_ellipse(x, y, rx, ry, m_trans);
    return;
  }
  if (fullflag) {
    theta1 = 0.0;
    theta2 = 360.0;
  }
  if (rotflag) {
    if (rx == ry) {
      rotflag = 0;
      theta1 += rotate;
      theta2 += rotate;
    } else {
      rs = sin(rotate*dr);
      rc = cos(rotate*dr);
      rcx = rc * rx;
      rcy = rc * ry;
      rsx = rs * rx;
      rsy = rs * ry;
    }
  }
  n = (long)floor((theta2 - theta1) * (rx + ry) / 2000 + 0.5) + 3;
  if (n > 500)
    n = 500;
  th = (theta2 - theta1) / n;
  if (fullflag)
    --n;
  for (i = 0; i <= n; i++) {
    if (i == n) {
      ss = sin(theta2*dr);
      cc = cos(theta2*dr);
    } else {
      ss = sin((theta1+th*i)*dr);
      cc = cos((theta1+th*i)*dr);
    }
    if (rotflag) {
      xp[i] = x + (long)floor(rcx * cc - rsy * ss + 0.5);
      yp[i] = y + (long)floor(rcy * ss + rsx * cc + 0.5);
    } else {
      xp[i] = x + (long)floor(rx * cc + 0.5);
      yp[i] = y + (long)floor(ry * ss + 0.5);
    }
  }
  if (chord == m_pie && !fullflag) {
    ++n;
    xp[n] = x;
    yp[n] = y;
  }
  m_fillpoly(n + 1, (int *)xp, (int *)yp);

#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoron();
#endif
}

void m_roundrect(x1, y1, x2, y2, rx, ry, c)
int x1, y1, x2, y2, rx, ry, c;
{
  int x, y, wid, hei;

  Mfprintf(stderr, "m_roundrect(%d, %d, %d, %d, %d, %d, %d)\n",
                                x1, y1, x2, y2, rx, ry, c    );

  LTRNSFRM(x1, y1);
  LTRNSFRM(x2, y2);

  if (currentmode == m_hitdet) {    /* cheat a bit... */
    if (currentcolor == m_trans)
      m_hitcount += hitdet_drawrect(x1, y1, x2, y2);
    else
      m_hitcount += hitdet_fillrect(x1, y1, x2, y2);
    return;
  }

#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoroff();
#endif

  x = MIN(x1,x2);
  y = MIN(y1,y2);
  wid = x1+x2-x-x;
  hei = y1+y2-y-y;
  
  if (currentcolor != m_trans) {
    Xfprintf(stderr, "XFillRectangle()\n");
    XFillRectangle(m_display, m_window, gc[currentcolor], x, y, wid+1, hei+1);
  }

  if (c != m_trans) {
    Xfprintf(stderr, "XDrawRectangle()\n");
    XDrawRectangle(m_display, m_window, gc[c], x, y, wid, hei);
  }

#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoron();
#endif

  if (nocache) {
    Ffprintf(stderr, "XFlush()\n");
    XFlush(m_display);
  }
}

void m_roundrect2(x1, y1, x2, y2, rx, ry, c1, c2)
int x1, y1, x2, y2, rx, ry, c1, c2;
{
  int x, y, wid, hei;

  Mfprintf(stderr, "m_roundrect2(%d, %d, %d, %d, %d, %d, %d, %d)\n",
	                         x1, y1, x2, y2, rx, ry, c1, c2   );

  LTRNSFRM(x1, y1);
  LTRNSFRM(x2, y2);

  if (currentmode == m_hitdet) {    /* cheat a bit... */
    if (c2 == m_trans)
      m_hitcount += hitdet_drawrect(x1, y1, x2, y2);
    else
      m_hitcount += hitdet_fillrect(x1, y1, x2, y2);
    return;
  }

#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoroff();
#endif

  x = MIN(x1,x2);
  y = MIN(y1,y2);
  wid = x1+x2-x-x;
  hei = y1+y2-y-y;
  
  if (c2 != m_trans) {
    Xfprintf(stderr, "XFillRectangle()\n");
    XFillRectangle(m_display, m_window, gc[c2], x, y, wid+1, hei+1);
  }

  if (c1 != m_trans) {
    Xfprintf(stderr, "XDrawRectangle()\n");
    XDrawRectangle(m_display, m_window, gc[c1], x, y, wid, hei);
  }

#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoron();
#endif

  if (nocache) {
    Ffprintf(stderr, "XFlush()\n");
    XFlush(m_display);
  }
}

static XPoint bezbuf[2048];
static int bezbufp;
static int bezthresh;

int hitdet_bezier(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
{
  int minx, maxx, miny, maxy;

  minx = (x1 < x2) ? x1 : x2;
  if (x3 < minx) minx = x3;
  if (x4 < minx) minx = x4;
  maxx = (x1 > x2) ? x1 : x2;
  if (x3 > maxx) maxx = x3;
  if (x4 > maxx) maxx = x4;
  miny = (y1 < y2) ? y1 : y2;
  if (y3 < miny) miny = y3;
  if (y4 < miny) miny = y4;
  maxy = (y1 > y2) ? y1 : y2;
  if (y3 > maxy) maxy = y3;
  if (y4 > maxy) maxy = y4;
  if (maxx < m_clip_x1 || minx > m_clip_x2 ||
      maxy < m_clip_y1 || miny > m_clip_y2)
    return 0;
  if (minx >= m_clip_x1 && maxx <= m_clip_x2 && miny >= m_clip_y1 && maxy <= m_clip_y2)
    return 1;
  if (x1 >= m_clip_x1 && x1 <= m_clip_x2 && y1 >= m_clip_y1 && y1 <= m_clip_y2)
    return 1;
  if (x4 >= m_clip_x1 && x4 <= m_clip_x2 && y4 >= m_clip_y1 && y4 <= m_clip_y2)
    return 1;
  if (maxx - minx < 2 || maxy - miny < 2)
    return hitdet_fillrect(minx, miny, maxx, maxy);
  return (hitdet_bezier(x1, y1, (x1+x2)>>1, (y1+y2)>>1,
			(x1+(x2<<1)+x3)>>2, (y1+(y2<<1)+y3)>>2,
			(x1+3*x2+3*x3+x4)>>3, (y1+3*y2+3*y3+y4)>>3) ||
	  hitdet_bezier((x4+3*x3+3*x2+x1)>>3, (y4+3*y3+3*y2+y1)>>3,
			(x4+(x3<<1)+x2)>>2, (y4+(y3<<1)+y2)>>2,
			(x4+x3)>>1, (y4+y3)>>1, x4, y4));
}

void dobezier(x1, y1, x2, y2, x3, y3, x4, y4, fx, fy)
int x1, y1, x2, y2, x3, y3, x4, y4, fx, fy;
{
  if ((abs((y4-y3)*(x3-x1)-(y3-y1)*(x4-x3)) < bezthresh) &&
      (abs((y4-y2)*(x2-x1)-(y2-y1)*(x4-x2)) < bezthresh)) {
    bezbuf[bezbufp].x = x4>>4;
    bezbuf[bezbufp++].y = y4>>4;
    if (x4==fx && y4==fy) {
      Xfprintf(stderr, "XDrawLines()\n");
      XDrawLines(m_display, m_window, gc[currentcolor], bezbuf, bezbufp, CoordModeOrigin);
    }
  } else {
    dobezier(x1, y1, (x1+x2)>>1, (y1+y2)>>1,
	     (x1+(x2<<1)+x3)>>2, (y1+(y2<<1)+y3)>>2,
	     (x1+3*x2+3*x3+x4)>>3, (y1+3*y2+3*y3+y4)>>3, fx, fy);
    dobezier((x4+3*x3+3*x2+x1)>>3, (y4+3*y3+3*y2+y1)>>3,
	     (x4+(x3<<1)+x2)>>2, (y4+(y3<<1)+y2)>>2,
	     (x4+x3)>>1, (y4+y3)>>1, x4, y4, fx, fy);
  }
}

void m_bezier(x1, y1, x2, y2, x3, y3, x4, y4)
int x1, y1, x2, y2, x3, y3, x4, y4;
{
  Mfprintf(stderr, "m_bezier(%d, %d, %d, %d, %d, %d, %d, %d)\n",
                             x1, y1, x2, y2, x3, y3, x4, y4   );

  TRNSFRM(x1, y1);
  TRNSFRM(x2, y2);
  TRNSFRM(x3, y3);
  TRNSFRM(x4, y4);

  if (currentmode == m_hitdet) {
    m_hitcount += hitdet_bezier(x1, y1, x2, y2, x3, y3, x4, y4);
    return;
  }

#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoroff();
#endif

  bezbuf[0].x = x1;
  bezbuf[0].y = y1;
  bezbufp = 1;
  bezthresh = 2500;
  dobezier((x1<<4)+8, (y1<<4)+8, (x2<<4)+8, (y2<<4)+8, (x3<<4)+8, (y3<<4)+8,
	   (x4<<4)+8, (y4<<4)+8, (x4<<4)+8, (y4<<4)+8);
 
#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoron();
#endif
  if (nocache) {
    Ffprintf(stderr, "XFlush()\n");
    XFlush(m_display);
  }
}

void m_bezier2(x1, y1, x2, y2, x3, y3, x4, y4, thresh)
int x1, y1, x2, y2, x3, y3, x4, y4, thresh;
{
  Mfprintf(stderr, "m_bezier2(%d, %d, %d, %d, %d, %d, %d, %d, %d)\n",
                              x1, y1, x2, y2, x3, y3, x4, y4, thresh);

  TRNSFRM(x1, y1);
  TRNSFRM(x2, y2);
  TRNSFRM(x3, y3);
  TRNSFRM(x4, y4);

  if (currentmode == m_hitdet) {
    m_hitcount += hitdet_bezier(x1, y1, x2, y2, x3, y3, x4, y4);
    return;
  }

#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoroff();
#endif

  bezbuf[0].x = x1;
  bezbuf[0].y = y1;
  bezbufp = 1;
  bezthresh = thresh;
  dobezier((x1<<4)+8, (y1<<4)+8, (x2<<4)+8, (y2<<4)+8, (x3<<4)+8, (y3<<4)+8,
	   (x4<<4)+8, (y4<<4)+8, (x4<<4)+8, (y4<<4)+8);

#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoron();
#endif

  if (nocache) {
    Ffprintf(stderr, "XFlush()\n");
    XFlush(m_display);
  }
}

void m_cbezier(x1, y1, x2, y2, x3, y3, x4, y4, thresh)
int x1, y1, x2, y2, x3, y3, x4, y4, thresh;
{
  Mfprintf(stderr, "m_cbezier(%d, %d, %d, %d, %d, %d, %d, %d, %d)\n",
                              x1, y1, x2, y2, x3, y3, x4, y4, thresh);

  TRNSFRM(x1, y1);
  TRNSFRM(x2, y2);
  TRNSFRM(x3, y3);
  TRNSFRM(x4, y4);

  if (currentmode == m_hitdet) {
    m_hitcount += (hitdet_line(curx, cury, x1, y1) ||
		   hitdet_bezier(x1, y1, x2, y2, x3, y3, x4, y4));
    return;
  }

#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoroff();
#endif

  bezbuf[0].x = curx;
  bezbuf[0].y = cury;
  bezbuf[1].x = x1;
  bezbuf[1].y = y1;

  bezbufp = 2;
  bezthresh = thresh;

  dobezier((x1<<4)+8, (y1<<4)+8, (x2<<4)+8, (y2<<4)+8, (x3<<4)+8, (y3<<4)+8,
	   (x4<<4)+8, (y4<<4)+8, (x4<<4)+8, (y4<<4)+8);
 
  curx = x4;
  cury = y4;

#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoron();
#endif

  if (nocache) {
    Ffprintf(stderr, "XFlush()\n");
    XFlush(m_display);
  }
}

/*
  Polycurve stuff added by Adam Greenblatt, 3 March 1990.  Sigh.
*/

#define SCALE           256

/* Local variables for polycurve: */
struct LOC_polycurve {
    int meef;
} ;

static void polycurve_doit(x0, y0, x1, y1, x2, y2,
                           x3, y3, x4, y4, x5, y5,
                           LINK_polycurve)
long x0, y0, x1, y1, x2, y2, x3, y3, x4, y4, x5, y5;
struct LOC_polycurve *LINK_polycurve;
{
    if (((x4 < 0) ? -x4 : x4) + ((x5 < 0) ? -x5 : x5) + ((y4 < 0) ? -y4 : y4) + ((y5
        < 0) ? -y5 : y5) < SCALE) {
        m_cbezier(x0 / SCALE, y0 / SCALE, (x0 + x1 / 3) / SCALE, (y0 + y1 / 3) /
                  SCALE, (x0 + (x1 * 2 + x2) / 3) / SCALE, (y0 + (y1 * 2 + y2) /
            3) / SCALE, (x0 + x1 + x2 + x3 + x4 + x5) / SCALE, (y0 + y1 + y2 + y3
            + y4 + y5) / SCALE, 2500L);
        return;
    }
    polycurve_doit(x0, y0, x1 / 2, y1 / 2, x2 / 4, y2 / 4,
                   x3 / 8, y3 / 8, x4 / 16, y4 / 16,
                   x5 / 32, y5 / 32, LINK_polycurve);
    x1 *= 16;
    y1 *= 16;
    x2 *= 8;
    y2 *= 8;
    x3 *= 4;
    y3 *= 4;
    x4 *= 2;
    y4 *= 2;
    polycurve_doit(x0 + (x1 + x2 + x3 + x4 + x5) / 32,
		   y0 + (y1 + y2 + y3 + y4 + y5) / 32,
		   (x1 + x2 * 2 + x3 * 3 + x4 * 4 + x5 * 5) / 32,
		   (y1 + y2 * 2 + y3 * 3 + y4 * 4 + y5 * 5) / 32,
		   (x2 + x3 * 3 + x4 * 6 + x5 * 10) / 32,
		   (y2 + y3 * 3 + y4 * 6 + y5 * 10) / 32,
		   (x3 + x4 * 4 + x5 * 10) / 32, 
		   (y3 + y4 * 4 + y5 * 10) / 32,
		   (x4 + x5 * 5) / 32,
		   (y4 + y5 * 5) / 32,
		   x5 / 32, y5 / 32,
		   LINK_polycurve);
}

void m_polycurve(x0, y0, x1, y1, x2, y2, x3, y3, x4, y4, x5, y5)
double x0, y0, x1, y1, x2, y2, x3, y3, x4, y4, x5, y5;
{
    struct LOC_polycurve V_polycurve;

    m_move((long)floor(SCALE * x0 + 0.5) / SCALE,
	   (long)floor(SCALE * y0 + 0.5) / SCALE);
    polycurve_doit((long)floor(SCALE * x0 + 0.5),
		   (long)floor(SCALE * y0 + 0.5),
		   (long)floor(SCALE * x1 + 0.5),
		   (long)floor(SCALE * y1 + 0.5),
		   (long)floor(SCALE * x2 + 0.5),
		   (long)floor(SCALE * y2 + 0.5),
		   (long)floor(SCALE * x3 + 0.5), 
		   (long)floor(SCALE * y3 + 0.5),
		   (long)floor(SCALE * x4 + 0.5), 
		   (long)floor(SCALE * y4 + 0.5),
		   (long)floor(SCALE * x5 + 0.5), 
		   (long)floor(SCALE * y5 + 0.5),
		   &V_polycurve);
}

int hitdet_drawpoly(n, points)
int n;
XPoint *points;
{
  int i;

  if (hitdet_line(points[0].x, points[0].y, points[n-1].x, points[n-1].y))
    return 1;
  for (i = 1; i < n; i++) {
    if (hitdet_line(points[i].x, points[i].y, points[i-1].x, points[i-1].y)) {
      return 1;
    }
  }
  return 0;
}

void m_drawpoly(n, x, y)
int n, x[], y[];
{
  XPoint *pointlist;
  int i, newx, newy;

  Mfprintf(stderr, "m_drawpoly(%d, x, y)\n", n);

  pointlist = (XPoint *) calloc(n+1, sizeof(XPoint));

  for (i = 0; i < n; i++) {
    newx = x[i];
    newy = y[i];
    TRNSFRM(newx, newy);
    pointlist[i].x = (short) newx;
    pointlist[i].y = (short) newy;
  }

  newx = x[0];
  newy = y[0];
  TRNSFRM(newx, newy);
  pointlist[n].x = (short) newx;
  pointlist[n].y = (short) newy;

  if (currentmode == m_hitdet) {
    m_hitcount += hitdet_drawpoly(n, pointlist);
    free(pointlist);
    return;
  }

#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoroff();
#endif

  Xfprintf(stderr, "XDrawLines()\n");
  XDrawLines(m_display, m_window, gc[currentcolor], pointlist, n+1, CoordModeOrigin);

  free(pointlist);

#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoron();
#endif

  if (nocache) {
    Ffprintf(stderr, "XFlush()\n");
    XFlush(m_display);
  }
}



#if 1

/* If no edges intersect, then rectangle must be entirely inside or
   entirely outside polygon.  Thus, we can simply check if any one point
   in the rectangle is in the polygon. */

int hitdet_fillpoly(n, points)
int n;
XPoint *points;
{
  int i, y1, y2, count;
  XPoint *pt1, *pt2;

  if (hitdet_drawpoly(n, points))
    return 1;
  count = 0;
  pt1 = points;
  pt2 = &points[n-1];
  for (i = 0; i < n; i++) {
    y1 = MIN(pt1->y, pt2->y);
    y2 = MAX(pt1->y, pt2->y);
    if (y1 <= m_clip_y1 && y2 > m_clip_y1) {
      if ((pt2->x - pt1->x) * (m_clip_y1 - pt1->y) / (pt2->y - pt1->y) >
	  (m_clip_x1 - pt1->x))
	count++;
    }
    pt2 = pt1;
    pt1++;
  }
  return (count & 1);
}

#else

#define SWAPINTS(x,y)  { int _t = (x); (x) = (y); (y) = _t; }

struct hitdet_edge {
  int x1, y1, x2, y2, dir, curx;
};

int hitdet_fillpoly_comp(e1, e2)
struct hitdet_edge *e1, *e2;
{
  return e1->y1 - e2->y1;
}

int hitdet_fillpoly(n, x, y)
int n, x[], y[];
{
  int i, j, count, cury1, cury2, first, last;
  struct hitdet_edge *edges, *e, tedge;

  printf("\n\n\n\nhitdet_fillpoly called...\n");     /***/

  /* Create edges array */
  edges = (struct hitdet_edge *) calloc(n, sizeof(struct hitdet_edge));
  edges[0].x2 = x[0];
  edges[0].y2 = y[0];
  TRNSFRM(edges[0].x2, edges[0].y2);
  printf("edges[0] = (%d,%d)\n", edges[0].x2, edges[0].y2);   /***/
  for (i = 1, e = &edges[1]; i < n; i++, e++) {
    e->x1 = e[-1].x2;
    e->y1 = e[-1].y2;
    e->x2 = x[i];
    e->y2 = y[i];
    TRNSFRM(e->x2, e->y2);
    printf("edges[%d] = (%d,%d)\n", i, edges[i].x2, edges[i].y2);   /***/
  }
  edges[0].x1 = edges[n-1].x2;
  edges[0].y1 = edges[n-1].y2;

  /* Sort each edge so y1 <= y2 */
  for (i = 0, e = edges; i < n; i++, e++) {
    if (e->y1 > e->y2) {
      SWAPINTS(e->x1, e->x2);
      SWAPINTS(e->y1, e->y2);
      e->dir = -1;
    } else
      e->dir = 1;
    printf("edges[%d] = (%d,%d) - (%d,%d)\n",                     /***/
	   i, edges[i].x1, edges[i].y1, edges[i].x2, edges[i].y2);   /***/
  }

  /* Sort edges in order of increasing y1 */
  qsort(edges, n, sizeof(struct hitdet_edge), hitdet_fillpoly_comp);
  for (i = 0; i < n; i++)    /***/
    printf("edges[%d] = (%d,%d) - (%d,%d)\n",                     /***/
	   i, edges[i].x1, edges[i].y1, edges[i].x2, edges[i].y2);   /***/

  /* Initialize "active" list */
  first = 0;
  last = 0;
  cury1 = edges[0].y1;
  if (cury1 < m_clip_y1)
    cury1 = m_clip_y1;
  while (cury1 <= m_clip_y2) {

    /* Collect things into active list */
    while (last < n && edges[last].y1 <= cury1)
      last++;

    /* Find next y position of interest */
    if (last < n) {
      cury2 = edges[last].y1;
      if (cury2 > m_clip_y2)
	cury2 = m_clip_y2 + 1;
    } else
      cury2 = m_clip_y2 + 1;
    printf("cury1 = %d, cury2 = %d, first=%d, last=%d\n",    /***/
	   cury1, cury2, first, last);		    /***/

    /* Compute x-coordinate of each edge at this y position */
    for (i = first, e = &edges[first]; i < last; i++, e++) {
      e->curx = e->x1 + (cury2 - e->y1) * (e->x2 - e->x1) / (e->y2 - e->y1);
    }

    /* Sort active list on x, dropping edges if necessary */
    /* (Insertion sort because list is likely almost-sorted already) */
    for (i = first+1; i < last; i++) {
      e = &edges[i];
      j = i;
      while (j > first &&
	     (e->y2 <= cury1 || e->curx < e[-1].curx)) {
	tedge = e[0];
	e[0] = e[-1];
	e[-1] = tedge;
	j--, e--;
      }
    }
    while (first < last && edges[first].y2 <= cury1)
      first++;
    printf("first = %d, last = %d\n", first, last);    /***/
    for (i = first; i < last; i++)		       /***/
      printf("edges[%d] = (%d,%d) - (%d,%d) at %d\n",                /***/
	     i, edges[i].x1, edges[i].y1, edges[i].x2, edges[i].y2,     /***/
	     edges[i].curx);   /***/

    /* Check if any trapezoids intersect clipping rectangle */
    for (i = first; i < last; i = j) {
      j = i;
      count = 0;
      do {
	count += edges[j].dir;
	j++;
      } while (count != 0 && j < last);
      if (edges[i].curx <= m_clip_x2 && edges[j-1].curx >= m_clip_x1) {
	free(edges);
	return 1;     /* Found a visible trapezoid */
      }
    }

    /* Advance to next y position */
    cury1 = cury2;
  }

  /* No trapezoids intersect */
  free(edges);
  return 0;
}

#endif


void m_fillpoly(n, x, y)
int n, x[], y[];
{
  XPoint *pointlist;
  int i, j, newx, newy;

  Mfprintf(stderr, "m_fillpoly(%d, x, y)\n", n);

  pointlist = (XPoint *) calloc(n, sizeof(XPoint));

  for (i = 0; i < n; i++) {
    newx = x[i];
    newy = y[i];
    TRNSFRM(newx, newy);
    pointlist[i].x = (short) newx;
    pointlist[i].y = (short) newy;
  }

  if (currentmode == m_hitdet) {
    m_hitcount += hitdet_fillpoly(n, pointlist);
    free(pointlist);
    return;
  }

#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoroff();
#endif

  Xfprintf(stderr, "XFillPolygon()\n");
  XFillPolygon(m_display, m_window, gc[currentcolor], pointlist, n, Complex, CoordModeOrigin);

  free(pointlist);

#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoron();
#endif
  if (nocache) {
    Ffprintf(stderr, "XFlush()\n");
    XFlush(m_display);
  }
}

void m_displaytext(str)
char *str;
{
  int dir, desc, len;
  XCharStruct size;

  Mfprintf(stderr, "m_displaytext(%s)\n", str);

  len = strlen(str);

  if (currentmode == m_hitdet) {
    XTextExtents(currentfont, str, len, &dir, &fontasc, &desc, &size);
    m_hitcount += hitdet_fillrect(curx, cury,
				  curx + size.width, cury + fontasc + desc);
    curx += size.width;
    return;
  }

#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoroff();
#endif

  Xfprintf(stderr, "XTextExtents()\n");
  XTextExtents(currentfont, str, len, &dir, &fontasc, &desc, &size);

  Xfprintf(stderr, "XDrawStrings()\n");
  XDrawString(m_display, m_window, gc[currentcolor], curx, cury+fontasc-1, str, len);

  if (nocache) {
    Ffprintf(stderr, "XFlush()\n");
    XFlush(m_display);
  }
  curx += size.width;

#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoron();
#endif
}

void m_drawstr(x, y, f, str)
int x, y;
char *f, *str;
{
  int dir, desc, len;
  XCharStruct size;

  Mfprintf(stderr, "m_drawstr(%d, %d, f, %s)\n", x, y, str);

  TRNSFRM(x, y);

  len = strlen(str);

  if (currentmode == m_hitdet) {
    XTextExtents(currentfont, str, len, &dir, &fontasc, &desc, &size);
    m_hitcount += hitdet_fillrect(x, y,
				  x + size.width, y + fontasc + desc);
    return;
  }

#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoroff();
#endif

  Xfprintf(stderr, "XDrawString()\n");
  XDrawString(m_display, m_window, gc[currentcolor], x, y+fontasc-1, str, len);

#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoron();
#endif

  if (nocache) {
    Ffprintf(stderr, "XFlush()\n");
    XFlush(m_display);
  }
}

void m_centerstr(x, y, f, str)
int x, y;
char *f, *str;
{
  int dir, desc, len;
  XCharStruct size;

  Mfprintf(stderr, "m_centerstr(%d, %d, f, %s)\n", x, y, str);

  TRNSFRM(x, y);

  len = strlen(str);

  if (currentmode == m_hitdet) {
    XTextExtents(currentfont, str, len, &dir, &fontasc, &desc, &size);
    m_hitcount += hitdet_fillrect(x - size.width/2, y,
				  x + size.width/2, y + fontasc + desc);
    return;
  }

#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoroff();
#endif

  Xfprintf(stderr, "XTextExtents()\n");
  XTextExtents(currentfont, str, len, &dir, &fontasc, &desc, &size);
  Xfprintf(stderr, "XDrawString()\n");
  XDrawString(m_display, m_window, gc[currentcolor], x-size.width/2, y+fontasc-1, str, len);

#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoron();
#endif

  if (nocache) {
    Ffprintf(stderr, "XFlush()\n");
    XFlush(m_display);
  }
}

void m_rightstr(x, y, f, str)
int x, y;
char *f, *str;
{
  int dir, desc, len;
  XCharStruct size;

  Mfprintf(stderr, "m_rightstr(%d, %d, f, %s)\n", x, y, str);

  TRNSFRM(x, y);

  len = strlen(str);

  if (currentmode == m_hitdet) {
    XTextExtents(currentfont, str, len, &dir, &fontasc, &desc, &size);
    m_hitcount += hitdet_fillrect(x - size.width, y,
				  x, y + fontasc + desc);
    return;
  }

#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoroff();
#endif

  Xfprintf(stderr, "XTextExtents()\n");
  XTextExtents(currentfont, str, len, &dir, &fontasc, &desc, &size);
  Xfprintf(stderr, "XDrawString()\n");
  XDrawString(m_display, m_window, gc[currentcolor], x-size.width, y+fontasc-1, str, len);

#ifdef SAVECURSOR
  if (cursor_is_on)
    turncursoron();
#endif

  if (nocache) {
    Ffprintf(stderr, "XFlush()\n");
    XFlush(m_display);
  }
}

long m_strwidth(f, str)      /* daveg, 10/6/89 */
char *f, *str;
{
  Mfprintf(stderr, "m_strwidth(f, %s)\n", str);

  Xfprintf(stderr, "XTextWidth()\n");
  return XTextWidth(currentfont, str, strlen(str));
}

/*
---m_getcpicture(x1, y1, x2, y2, p)
int x1, y1, x2, y2;
m_picturevar *p;
{
  
*/

/* Added these two functions for auto raise and lower support for X11.
 * The functions existed but were just #define'ed as nothing.
 * WES - 7/17/91
 */

/* This function raises the graphics window above the alpha window */

/* Save current window geometry for next session - public so can be called before exit */
void m_save_window_geometry()
{
  char *home;
  char geo_path[512];
  FILE *geo_file;
  
  home = getenv("HOME");
  if (home == NULL || last_window_width == 0 || last_window_height == 0)
    return;
    
  /* Validate reasonable size (not too small, not bigger than 4K screen) */
  if (last_window_width < 400 || last_window_height < 300 || 
      last_window_width > 3840 || last_window_height > 2160)
    return;
    
  /* Save to ~/.chipmunk_geometry */
  snprintf(geo_path, sizeof(geo_path), "%s/.chipmunk_geometry", home);
  geo_file = fopen(geo_path, "w");
  if (geo_file != NULL) {
    fprintf(geo_file, "%dx%d+0+0\n", last_window_width, last_window_height);
    fclose(geo_file);
  }
}

/* Update tracked window size - called from ConfigureNotify handlers */
static void update_window_size(unsigned int width, unsigned int height)
{
  last_window_width = width;
  last_window_height = height;
}

void m_graphics_on()
{
  XWindowChanges  changes;
  static int geometry_save_registered = 0;

  /* Register geometry save on first call */
  if (!geometry_save_registered) {
    atexit(m_save_window_geometry);
    geometry_save_registered = 1;
  }

  if (m_autoraise)
    {
      changes.sibling     = nc_window;
      changes.stack_mode  = Above;

      XConfigureWindow(m_display, m_window, CWStackMode, &changes);
    }
  return;
}


/* This function raises the alpha window above the graphics window */

void m_alpha_on()
{
  XWindowChanges  changes;

  if (m_autoraise)
    {
      changes.sibling     = m_window;
      changes.stack_mode  = Above;
      
      XConfigureWindow(m_display, nc_window, CWStackMode, &changes);
    }
  return;
}


void m_readpen(pen)
m_tablet_info *pen;
{
  XEvent event;
  int newx, newy;
  int gotevent, found = 0, giveup = 0;
  static int ignore_next_left_click = 0;  /* Counter: ignore left-clicks until this reaches 0 */

  Pfprintf(stderr, "m_readpen(pen)\n");

#ifdef EXTRA_BUFFERING
  flush_buffers();
#endif /* EXTRA_BUFFERING */
  
  /* Check for FocusIn events to detect window activation */
  while (XCheckMaskEvent(m_display, FocusChangeMask, &event)) {
    if (event.type == FocusIn && event.xfocus.window == m_window) {
      /* Set to ignore the next left-click */
      ignore_next_left_click = 1;
    } else if (event.type == FocusOut && event.xfocus.window == m_window) {
      /* Reset on focus out */
      ignore_next_left_click = 0;
    }
  }
  
  while (1) {
    Xfprintf(stderr, "XCheckMaskEvent()\n");
    gotevent = XCheckMaskEvent(m_display, ButtonPressMask | PointerMotionMask |
			                  ButtonReleaseMask, &event);
    if (gotevent) {
      m_events_received++;
      found = 1;
      mouse.inalpha = (nc_initialized && event.xany.window == nc_window);
      if (event.type != MotionNotify)
	break;
    } else
      break;
  }

  if (found) {
    if (event.type == ButtonPress) {
      /* Ignore first left-click after window focus to prevent accidental wire-drawing */
      if (ignore_next_left_click && event.xbutton.button == Button1) {
        ignore_next_left_click = 0;  /* Clear flag after ignoring */
        /* Act as if no event occurred */
        found = 0;
      } else {
        /* Any other button press clears the ignore flag */
        if (event.xbutton.button != Button1) {
          ignore_next_left_click = 0;
        }
        pen->dn = (event.xbutton.button == Button1);
        pen->depressed = (event.xbutton.state & Button1Mask) || pen->dn;
        pen->up = 0;
        pen->near_ = ! (event.xbutton.state & Button3Mask) &&
    	            ! (event.xbutton.button == Button3);
      }
    } else if (event.type == ButtonRelease) {
      pen->dn = 0;
      pen->up = (event.xbutton.button == Button1);
      pen->depressed = (event.xbutton.state & Button1Mask) && (! pen->up);
      pen->near_ = ! (event.xbutton.state & Button3Mask) ||
    	          (event.xbutton.button == Button3);
    } else {
      pen->dn = 0;
      pen->up = 0;
      pen->depressed = ((event.xmotion.state & Button1Mask) != 0);
      pen->near_ = ! (event.xmotion.state & Button3Mask);
    }

    if (event.xany.window == m_window) {
      newx = (int) event.xbutton.x;
      newy = (int) event.xbutton.y;
      pen->ax = mouse.ax;
      pen->ay = mouse.ay;
      UNTRNSFRM(newx, newy);
      pen->x = (long) newx;
      pen->y = (long) newy;
    } else if (event.xany.window == nc_window) {
      pen->x = mouse.x;
      pen->y = mouse.y;
      pen->ax = event.xbutton.x / nc_fontwidth;
      pen->ay = event.xbutton.y / nc_fontheight;
    } else {
      pen->x = mouse.x;
      pen->y = mouse.y;
      pen->ax = mouse.ax;
      pen->ay = mouse.ay;
    }    
  } else {
    pen->dn = 0;
    pen->up = 0;
    pen->depressed = mouse.depressed;
    pen->near_ = mouse.near_;
    pen->x = mouse.x;
    pen->y = mouse.y;
    pen->ax = mouse.ax;
    pen->ay = mouse.ay;
  }

  pen->moving = pen->x != mouse.x || pen->y != mouse.y ||
                pen->depressed != mouse.depressed || pen->near_ != mouse.near_;
  pen->inalpha = mouse.inalpha;

  mouse.x = pen->x;
  mouse.y = pen->y;
  mouse.ax = pen->ax;
  mouse.ay = pen->ay;
  mouse.depressed = pen->depressed;
  mouse.near_ = pen->near_;
}

void m_trackpen(pen)
m_tablet_info *pen;
{
  Pfprintf(stderr, "m_trackpen(pen)\n");

  m_readpen(pen);

/*  fprintf(stderr, "m_cursor(%d, %d)   from m_trackpen\n", pen->x, pen->y);  */
  m_cursor(pen->x, pen->y); 
}

void m_trackpen2(pen)
m_tablet_info *pen;
{
  XEvent event;
  int found = 0, giveup = 0;

  Window root = 0, parent, child, *childlist;
  int rootx, rooty, x, y, mx, my, nx, ny, dx, dy;
  unsigned int w, h, bw, d, nchild, mask;
  int newx, newy;

  Pfprintf(stderr, "m_trackpen2(pen)\n");

#ifdef EXTRA_BUFFERING
  flush_buffers();
#endif /* EXTRA_BUFFERING */
  while (1) {
    Xfprintf(stderr, "XCheckMaskEvent()  (m_trackpen2())\n");
    found = XCheckMaskEvent(m_display, ButtonPressMask | PointerMotionMask,
			    &event);
    if (found) {
      if (event.type == ButtonPress) 
	break;
    } else
      break;
  }

  if (found) {
    pen->dn = (event.xbutton.button == Button1);
    pen->depressed = (event.xbutton.state & Button1Mask) || pen->dn;
    pen->up = 0;
    pen->near_ = ! (event.xbutton.state & Button3Mask) &&
                ! (event.xbutton.button == Button3);
    
    if (nc_initialized) {
      if (event.xany.window == m_window) {
	parent = nc_window;
	nx = ny = 0;
	while (parent != root) {
	  Xfprintf(stderr, "XGetGeometry()\n");
	  XGetGeometry(m_display, parent, &root, &dx, &dy, &w, &h, &bw, &d);
	  nx += dx+bw;
	  ny += dy+bw;
	  Xfprintf(stderr, "XQueryTree()\n");
	  XQueryTree(m_display, parent, &root, &parent, &childlist, &nchild);
	}
	pen->x = event.xbutton.x;
	pen->y = event.xbutton.y;
	pen->ax = (event.xbutton.x_root - nx) / nc_fontwidth;
	pen->ay = (event.xbutton.y_root - ny) / nc_fontheight;
      } else {
	parent = m_window;
	mx = my = 0;
	while (parent != root) {
	  Xfprintf(stderr, "XGetGeometry()\n");
	  XGetGeometry(m_display, parent, &root, &dx, &dy, &w, &h, &bw, &d);
	  mx += dx+bw;
	  my += dy+bw;
	  Xfprintf(stderr, "XQueryTree()\n");
	  XQueryTree(m_display, parent, &root, &parent, &childlist, &nchild);
	}
	pen->x = event.xbutton.x_root - mx;
	pen->y = event.xbutton.y_root - my;
	pen->ax = event.xbutton.x / nc_fontwidth;
	pen->ay = event.xbutton.y / nc_fontheight;
      }
    }
    newx = pen->x;
    newy = pen->y;
    UNTRNSFRM(newx, newy);
    pen->x = newx;
    pen->y = newy;
  } else {
    Xfprintf(stderr, "XQueryPointer()\n");
    XQueryPointer(m_display, m_window, &root, &child, &rootx, &rooty,
		  &x, &y, &mask);
    if (nc_initialized) {
      parent = m_window;
      mx = my = 0;
      while (parent != root) {
	Xfprintf(stderr, "XGetGeometry()\n");
	XGetGeometry(m_display, parent, &root, &dx, &dy, &w, &h, &bw, &d);
	mx += dx+bw;
	my += dy+bw;
	Xfprintf(stderr, "XQueryTree()\n");
	XQueryTree(m_display, parent, &root, &parent, &childlist, &nchild);
      }
      parent = nc_window;
      nx = ny = 0;
      while (parent != root) {
	Xfprintf(stderr, "XGetGeometry()\n");
	XGetGeometry(m_display, parent, &root, &dx, &dy, &w, &h, &bw, &d);
	nx += dx+bw;
	ny += dy+bw;
	Xfprintf(stderr, "XQueryTree()\n");
	XQueryTree(m_display, parent, &root, &parent, &childlist, &nchild);
      }
      pen->ax = (x + mx - nx) / nc_fontwidth;
      pen->ay = (y + my - ny) / nc_fontheight;
    }
    if ((x >= 0) && (x <= m_across) && (y >= 0) && (y <= m_down)) {
      pen->depressed = ((mask & Button1Mask) != 0);
      pen->near_ = ! (mask & Button3Mask);
      pen->up = 0;
      pen->dn = 0;
    } else {
      if (mouse.depressed)
	pen->depressed = ((mask & Button1Mask) != 0);
      else
	pen->depressed = 0;
      pen->near_ = mouse.near_ || ! (mask & Button3Mask);
      pen->dn = 0;
      pen->up = (mouse.depressed && ! pen->depressed);
    }
    UNTRNSFRM(x, y);
    pen->x = x;
    pen->y = y;
  }

  pen->moving = pen->x != mouse.x || pen->y != mouse.y ||
                pen->depressed != mouse.depressed || pen->near_ != mouse.near_;

  mouse.x = pen->x;
  mouse.y = pen->y;
  mouse.depressed = pen->depressed;
  mouse.near_ = pen->near_;

/*  fprintf(stderr, "m_cursor(%d, %d)   from m_trackpen2\n", pen->x, pen->y);  */
  m_cursor(pen->x, pen->y);
}


void m_waitpen(pen)
m_tablet_info *pen;
{
  XEvent event;
  int x, y;
  x = cursx;
  y = cursy;

  UNTRNSFRM(x, y);

  if (!cursor_is_on)
    m_cursor(x, y);

  /* wait whilst event queue is empty. */
  while (1) {
    XMaskEvent(m_display, ButtonPressMask | ButtonReleaseMask
                      | PointerMotionMask | KeyPressMask
                      | ExposureMask | StructureNotifyMask
                      | LeaveWindowMask, &event);
    if (event.type != LeaveNotify)
      break;
    m_nocursor();
  }

  XPutBackEvent(m_display, &event);
  m_trackpen(pen);
}


boolean m_pollkbd()
{
  XEvent event;
  char buf[10];
  KeySym sym;

  Kfprintf(stderr, "m_pollkbd()\n");
  
#ifdef EXTRA_BUFFERING
  flush_buffers();
#endif /* EXTRA_BUFFERING */
  for (;;) {
    Xfprintf(stderr, "XCheckMaskEvent()\n");
    if (! XCheckMaskEvent(m_display, KeyPressMask |
                                     ExposureMask |
                                     StructureNotifyMask, &event))
      return(0); /* sleep(1) could be added here for load-average problem */
                 /* but may be too drastic (MDG)                          */
    switch (event.type) {
    case KeyPress: 
      /* Check for arrow keys via KeySym first - XLookupString may return 0 for them */
      sym = XLookupKeysym((XKeyEvent *)&event, 0);
      if (sym == XK_Up || sym == XK_KP_Up || sym == XK_Down || sym == XK_KP_Down ||
	  sym == XK_Left || sym == XK_KP_Left || sym == XK_Right || sym == XK_KP_Right) {
	XPutBackEvent(m_display, &event);
	return(1);
      }
      if (XLookupString((XKeyEvent *)&event, buf, 10, &sym, NULL)) {
	Xfprintf(stderr, "XPutBackEvent()  (m_pollkbd() Key event)\n");
	XPutBackEvent(m_display, &event);
	return(1);
      } else {
	/* Even if XLookupString fails, if it's a keypress, we should return true */
	/* This handles keys that don't have string representations (like arrow keys) */
	XPutBackEvent(m_display, &event);
	return(1);
      }
      break;
    case Expose:
      if ((event.xexpose.window == m_window) && (event.xexpose.count == 0)) {
	Xfprintf(stderr, "XPutBackEvent()  (m_pollkbd() Expose event)\n");
	XPutBackEvent(m_display, &event);
	return(1);
      } else if ((event.type == Expose) && nc_initialized &&
		 (event.xexpose.window == nc_window) &&
		 (event.xexpose.count == 0)) {
	if (nc_initialized)
	  nc_refreshScreen();
      }
      break;
    case ConfigureNotify:
      if ((event.xconfigure.window == m_window) &&
	  ((event.xconfigure.width != m_across+1) ||
	   (event.xconfigure.height != m_down+1))) {
	update_window_size(event.xconfigure.width, event.xconfigure.height);
	Xfprintf(stderr, "XPutBackEvent()  (m_pollkbd() resize event)\n");
	XPutBackEvent(m_display, &event);
	return(1);
      }
      break;
    case DestroyNotify:
      fprintf(stderr, "Mylib panic:  DestroyNotify event detected\n");
      exit(1);
      break;
    case CirculateNotify:
    case GravityNotify:
    case MapNotify:
    case ReparentNotify:
    case UnmapNotify:
    case ClientMessage:
    case MappingNotify:
    case SelectionClear:
    case SelectionNotify:
    case SelectionRequest:
    case GraphicsExpose:
    case NoExpose:
      break;
    default:
      fprintf(stderr, "Unknown event (type = %d) detected\n", event.type);
      break;
    }
  }
}

uchar m_inkey()
{
  XEvent event;
  char buf[10];
  KeySym sym;
  static int debug_esc = -1;  /* -1: uninit, 0: off, 1: on */
  static int use_file = -1;
  static char dbgpath[256];
  auto void dbglog(const char *msg) {
    if (debug_esc <= 0)
      return;
    if (use_file == -1) {
      const char *p = getenv("CHIPMUNK_DEBUG_ESC_FILE");
      if (p && *p) {
        strncpy(dbgpath, p, sizeof(dbgpath) - 1);
        dbgpath[sizeof(dbgpath) - 1] = '\0';
        use_file = 1;
      } else {
        strcpy(dbgpath, "/tmp/chipmunk-esc.log");
        use_file = 1;
      }
    }
    FILE *f = fopen(dbgpath, "a");
    if (f) {
      fputs(msg, f);
      fputc('\n', f);
      fclose(f);
    } else {
      fprintf(stderr, "%s\n", msg);
    }
  }

  Kfprintf(stderr, "m_inkey()\n");

#ifdef EXTRA_BUFFERING
  flush_buffers();
#endif /* EXTRA_BUFFERING */
  for (;;) {
    Xfprintf(stderr, "XMaskEvent()  (m_inkey())\n");
    nc_cursor_on();
    XMaskEvent(m_display, KeyPressMask |
	                  ExposureMask |
                          StructureNotifyMask, &event);
    nc_cursor_off();
    if (event.type == KeyPress) {
      /* First try XLookupString - XRebindKeysym should make arrow keys return scroll codes */
      if (XLookupString((XKeyEvent *)&event, buf, 10, &sym, NULL)) {
	/* Check if this is an arrow key character (from XRebindKeysym) */
	if (buf[0] == '\037' || buf[0] == '\n' || buf[0] == '\b' || buf[0] == '\034') {
	  return((uchar) buf[0]);
	}
	/* Regular character */
	if ((unsigned char)buf[0] == 27) {
	  if (debug_esc == -1) {
	    const char *env = getenv("CHIPMUNK_DEBUG_ESC");
	    debug_esc = (env && *env && (*env == '1' || *env == 'y' || *env == 'Y')) ? 1 : 0;
	  }
	  dbglog("[chipmunk] ESC detected at X layer (m_inkey) -> ^C");
	  return((uchar) '\003');
	}
	return((uchar) buf[0]);
      }
      /* If XLookupString returns 0, try XLookupKeysym as fallback for arrow keys and ESC */
      /* This handles cases where XRebindKeysym didn't work or the key isn't rebound */
      sym = XLookupKeysym((XKeyEvent *)&event, 0);
      /* Check for Escape key */
      if (sym == XK_Escape) {
	if (debug_esc == -1) {
	  const char *env = getenv("CHIPMUNK_DEBUG_ESC");
	  debug_esc = (env && *env && (*env == '1' || *env == 'y' || *env == 'Y')) ? 1 : 0;
	}
	dbglog("[chipmunk] ESC keysym detected (m_inkey fallback) -> ^C");
	return((uchar) '\003');
      }
      if (sym == XK_Up || sym == XK_KP_Up) {
	return((uchar) '\037');  /* Up arrow -> scroll up */
      } else if (sym == XK_Down || sym == XK_KP_Down) {
	return((uchar) '\n');    /* Down arrow -> scroll down */
      } else if (sym == XK_Left || sym == XK_KP_Left) {
	return((uchar) '\b');    /* Left arrow -> scroll left */
      } else if (sym == XK_Right || sym == XK_KP_Right) {
	return((uchar) '\034');  /* Right arrow -> scroll right */
      } else if ((event.xkey.state & ControlMask) &&
                 (sym == XK_c || sym == XK_C)) {
	if (debug_esc == -1) {
	  const char *env = getenv("CHIPMUNK_DEBUG_ESC");
	  debug_esc = (env && *env && (*env == '1' || *env == 'y' || *env == 'Y')) ? 1 : 0;
	}
	dbglog("[chipmunk] Ctrl-C detected (m_inkey) -> ^C");
	return((uchar) '\003');
      }
      /* If neither XLookupString nor XLookupKeysym worked, continue loop */
    } else if ((event.type == Expose) && (event.xexpose.window == m_window) &&
	       (event.xexpose.count == 0)) {
#ifdef SHOW_EXPOSE_EVENTS
      fprintf(stderr, "m_inkey(): Expose event detected\n");
#endif/*  SHOW_EXPOSE_EVENTS */
      return((uchar) 250);
    } else if ((event.type == Expose) && nc_initialized &&
	       (event.xexpose.window == nc_window) &&
	       (event.xexpose.count == 0)) {
      if (nc_initialized)
	nc_refreshScreen();
    } else if ((event.type == ConfigureNotify) &&
	       (event.xconfigure.window == m_window) &&
	       ((event.xconfigure.width != m_across+1) ||
		(event.xconfigure.height != m_down+1))) {
      update_window_size(event.xconfigure.width, event.xconfigure.height);
#ifdef SHOW_CONFIGURE_EVENTS
      fprintf(stderr, "m_inkey(): ConfigureNotify event detected\n");
#endif /*SHOW_CONFIGURE_EVENTS*/
      m_across = event.xconfigure.width - 1;
      m_down = event.xconfigure.height - 1;
      return((uchar) 251);
    }
  }
}

uchar m_inkeyn()
{
  XEvent event;
  char buf[10];
  KeySym sym;
  static int debug_esc = -1;  /* -1: uninit, 0: off, 1: on */
  static int use_file = -1;
  static char dbgpath[256];
  auto void dbglog(const char *msg) {
    if (debug_esc <= 0)
      return;
    if (use_file == -1) {
      const char *p = getenv("CHIPMUNK_DEBUG_ESC_FILE");
      if (p && *p) {
        strncpy(dbgpath, p, sizeof(dbgpath) - 1);
        dbgpath[sizeof(dbgpath) - 1] = '\0';
        use_file = 1;
      } else {
        strcpy(dbgpath, "/tmp/chipmunk-esc.log");
        use_file = 1;
      }
    }
    FILE *f = fopen(dbgpath, "a");
    if (f) {
      fputs(msg, f);
      fputc('\n', f);
      fclose(f);
    } else {
      fprintf(stderr, "%s\n", msg);
    }
  }

  Kfprintf(stderr, "m_inkeyn()\n");

#ifdef EXTRA_BUFFERING
  flush_buffers();
#endif /* EXTRA_BUFFERING */
  for (;;) {
    Xfprintf(stderr, "XCheckMaskEvent()  (m_inkeyn())\n");
    if (! XCheckMaskEvent(m_display, KeyPressMask |
	                             ExposureMask |
                                     StructureNotifyMask, &event))
      return(0);
    else {
      if (event.type == KeyPress) {
	/* First try XLookupString - XRebindKeysym should make arrow keys return scroll codes */
	if (XLookupString((XKeyEvent *)&event, buf, 10, &sym, NULL)) {
	  /* Check if this is an arrow key character (from XRebindKeysym) */
	  if (buf[0] == '\037' || buf[0] == '\n' || buf[0] == '\b' || buf[0] == '\034') {
	    XPutBackEvent(m_display, &event);
	    return((uchar) buf[0]);
	  }
	  /* Regular character */
	  Xfprintf(stderr, "XPutBackEvent()  (m_inkeyn() Key event)\n");
	  XPutBackEvent(m_display, &event);
	  if ((unsigned char)buf[0] == 27) {
	    if (debug_esc == -1) {
	      const char *env = getenv("CHIPMUNK_DEBUG_ESC");
	      debug_esc = (env && *env && (*env == '1' || *env == 'y' || *env == 'Y')) ? 1 : 0;
	    }
	    dbglog("[chipmunk] ESC detected at X layer (m_inkeyn) -> ^C");
	    return((uchar) '\003');
	  }
	  return(buf[0]);
	}
	/* If XLookupString returns 0, try XLookupKeysym as fallback for arrow keys and ESC */
	sym = XLookupKeysym((XKeyEvent *)&event, 0);
	/* Check for Escape key */
	if (sym == XK_Escape) {
	  if (debug_esc == -1) {
	    const char *env = getenv("CHIPMUNK_DEBUG_ESC");
	    debug_esc = (env && *env && (*env == '1' || *env == 'y' || *env == 'Y')) ? 1 : 0;
	  }
	  dbglog("[chipmunk] ESC keysym detected (m_inkeyn fallback) -> ^C");
	  XPutBackEvent(m_display, &event);
	  return((uchar) '\003');
	}
	if (sym == XK_Up || sym == XK_KP_Up) {
	  XPutBackEvent(m_display, &event);
	  return((uchar) '\037');  /* Up arrow -> scroll up */
	} else if (sym == XK_Down || sym == XK_KP_Down) {
	  XPutBackEvent(m_display, &event);
	  return((uchar) '\n');    /* Down arrow -> scroll down */
	} else if (sym == XK_Left || sym == XK_KP_Left) {
	  XPutBackEvent(m_display, &event);
	  return((uchar) '\b');    /* Left arrow -> scroll left */
	} else if (sym == XK_Right || sym == XK_KP_Right) {
	  XPutBackEvent(m_display, &event);
	  return((uchar) '\034');  /* Right arrow -> scroll right */
	} else if ((event.xkey.state & ControlMask) &&
                   (sym == XK_c || sym == XK_C)) {
	  XPutBackEvent(m_display, &event);
	  if (debug_esc == -1) {
	    const char *env = getenv("CHIPMUNK_DEBUG_ESC");
	    debug_esc = (env && *env && (*env == '1' || *env == 'y' || *env == 'Y')) ? 1 : 0;
	  }
	  dbglog("[chipmunk] Ctrl-C detected (m_inkeyn) -> ^C");
	  return((uchar) '\003');
	}
	/* If neither worked, put event back and return 0 (no key available) */
	XPutBackEvent(m_display, &event);
	return(0);
      } else if ((event.type == Expose) && 
		 (event.xexpose.window == m_window) &&
		 (event.xexpose.count == 0)) {
	Xfprintf(stderr, "XPutBackEvent()  (m_inkeyn() expose event)\n");
	XPutBackEvent(m_display, &event);
	return((uchar) 250);
      } else if ((event.type == ConfigureNotify) &&
		 (event.xconfigure.window == m_window) &&
		 ((event.xconfigure.width != m_across+1) ||
		  (event.xconfigure.height != m_down+1))) {
	update_window_size(event.xconfigure.width, event.xconfigure.height);
	m_across = event.xconfigure.width - 1;
	m_down = event.xconfigure.height - 1;
	Xfprintf(stderr, "XPutBackEvent()  (m_inkeyn() resize event)\n");
	XPutBackEvent(m_display, &event);
	return((uchar) 251);
      }
    }
  }
}

uchar m_testkey()
{
  XEvent event;
  char buf[10];
  KeySym sym;
  static int debug_esc = -1;  /* -1: uninit, 0: off, 1: on */
  static int use_file = -1;
  static char dbgpath[256];
  auto void dbglog(const char *msg) {
    if (debug_esc <= 0)
      return;
    if (use_file == -1) {
      const char *p = getenv("CHIPMUNK_DEBUG_ESC_FILE");
      if (p && *p) {
        strncpy(dbgpath, p, sizeof(dbgpath) - 1);
        dbgpath[sizeof(dbgpath) - 1] = '\0';
        use_file = 1;
      } else {
        strcpy(dbgpath, "/tmp/chipmunk-esc.log");
        use_file = 1;
      }
    }
    FILE *f = fopen(dbgpath, "a");
    if (f) {
      fputs(msg, f);
      fputc('\n', f);
      fclose(f);
    } else {
      fprintf(stderr, "%s\n", msg);
    }
  }

  Kfprintf(stderr, "m_testkey()\n");

#ifdef EXTRA_BUFFERING
  flush_buffers();
#endif /* EXTRA_BUFFERING */
  for (;;) {
    Xfprintf(stderr, "XCheckMaskEvent()  (m_testkey())\n");
    if (! XCheckMaskEvent(m_display, KeyPressMask |
                                     ExposureMask |
                                     StructureNotifyMask, &event))
      return(0);
    if (event.type == KeyPress) {
      /* First try XLookupString - XRebindKeysym should make arrow keys return scroll codes */
      if (XLookupString((XKeyEvent *)&event, buf, 10, &sym, NULL)) {
	/* Check if this is an arrow key character (from XRebindKeysym) */
	if (buf[0] == '\037' || buf[0] == '\n' || buf[0] == '\b' || buf[0] == '\034') {
	  XPutBackEvent(m_display, &event);
	  return((uchar) buf[0]);
	}
	/* Regular character */
	Xfprintf(stderr, "XPutBackEvent()  (m_testkey() key event)\n");
	XPutBackEvent(m_display, &event);
	if ((unsigned char)buf[0] == 27) {
	  if (debug_esc == -1) {
	    const char *env = getenv("CHIPMUNK_DEBUG_ESC");
	    debug_esc = (env && *env && (*env == '1' || *env == 'y' || *env == 'Y')) ? 1 : 0;
	  }
	  dbglog("[chipmunk] ESC detected at X layer (m_testkey) -> ^C");
	  return((uchar) '\003');
	}
	return(buf[0]);
      }
      /* If XLookupString returns 0, try XLookupKeysym as fallback for arrow keys and ESC */
      sym = XLookupKeysym((XKeyEvent *)&event, 0);
      /* Check for Escape key */
      if (sym == XK_Escape) {
	if (debug_esc == -1) {
	  const char *env = getenv("CHIPMUNK_DEBUG_ESC");
	  debug_esc = (env && *env && (*env == '1' || *env == 'y' || *env == 'Y')) ? 1 : 0;
	}
	dbglog("[chipmunk] ESC keysym detected (m_testkey fallback) -> ^C");
	XPutBackEvent(m_display, &event);
	return((uchar) '\003');
      }
      if (sym == XK_Up || sym == XK_KP_Up) {
	XPutBackEvent(m_display, &event);
	return((uchar) '\037');  /* Up arrow -> scroll up */
      } else if (sym == XK_Down || sym == XK_KP_Down) {
	XPutBackEvent(m_display, &event);
	return((uchar) '\n');    /* Down arrow -> scroll down */
      } else if (sym == XK_Left || sym == XK_KP_Left) {
	XPutBackEvent(m_display, &event);
	return((uchar) '\b');    /* Left arrow -> scroll left */
      } else if (sym == XK_Right || sym == XK_KP_Right) {
	XPutBackEvent(m_display, &event);
	return((uchar) '\034');  /* Right arrow -> scroll right */
      } else if ((event.xkey.state & ControlMask) &&
                 (sym == XK_c || sym == XK_C)) {
	XPutBackEvent(m_display, &event);
	if (debug_esc == -1) {
	  const char *env = getenv("CHIPMUNK_DEBUG_ESC");
	  debug_esc = (env && *env && (*env == '1' || *env == 'y' || *env == 'Y')) ? 1 : 0;
	}
	dbglog("[chipmunk] Ctrl-C detected (m_testkey) -> ^C");
	return((uchar) '\003');
      }
    } else if ((event.type == Expose) &&
	       (event.xexpose.window == m_window) &&
	       (event.xexpose.count == 0)) {
      Xfprintf(stderr, "XPutBackEvent()  (m_testkey() expose event)\n");
      XPutBackEvent(m_display, &event);
      return((uchar) 250);
    } else if ((event.type == ConfigureNotify) &&
	       (event.xconfigure.window == m_window) &&
	       ((event.xconfigure.width != m_across+1) ||
		(event.xconfigure.height != m_down+1))) {
      update_window_size(event.xconfigure.width, event.xconfigure.height);
#ifdef SHOW_CONFIGURE_EVENTS
      fprintf(stderr, "m_testkey(): ConfigureNotify event detected\n");
#endif/* SHOW_CONFIGURE_EVENTS*/
      m_across = event.xconfigure.width - 1;
      m_down = event.xconfigure.height - 1;
      Xfprintf(stderr, "XPutBackEvent()  (m_testkey() resize event)\n");
      XPutBackEvent(m_display, &event);
      return((uchar) 251);
    }
  }
}


#define buttonw         40
#define buttonh         30
#define buttony         30
#define yesbuttonx      10
#define nobuttonx       60
#define popupw          110
#define popuph          70


boolean m_yes_or_no(prompt)
Char *prompt;
{
  Window root, popup, yesbutton, nobutton;
  char buf[10];
  KeySym sym;
  XEvent event;
  XSizeHints hints;
  GC tempgc;
  int done = 0, return_value = 0;
  int inyes = 0, inno = 0, yesactive = 0, noactive = 0;

  Mfprintf(stderr, "m_yes_or_no(%s)\n", prompt);

#ifdef EXTRA_BUFFERING
  flush_buffers();
#endif /* EXTRA_BUFFERING */

  root = DefaultRootWindow(m_display);

  Xfprintf(stderr, "XCreateSimpleWindow()\n");
  popup = XCreateSimpleWindow(m_display, root, 0, 0, popupw, popuph, 2,
			      m_colors[0][m_red].pixel,
			      m_colors[0][m_black].pixel);
  Xfprintf(stderr, "XCreateSimpleWindow()\n");
  yesbutton = XCreateSimpleWindow(m_display, popup, yesbuttonx, buttony,
				  buttonw, buttonh, 1,
				  m_colors[0][m_white].pixel,
				  m_colors[0][m_black].pixel);
  Xfprintf(stderr, "XCreateSimpleWindow()\n");
  nobutton = XCreateSimpleWindow(m_display, popup, nobuttonx, buttony,
				 buttonw, buttonh, 1,
				 m_colors[0][m_white].pixel,
				 m_colors[0][m_black].pixel);

  Xfprintf(stderr, "tempgc = XCreateGC()\n");
  tempgc = XCreateGC(m_display, popup, 0, NULL);
  Xfprintf(stderr, "XSetForeground(m_display, tempgc)\n");
  XSetForeground(m_display, tempgc, m_colors[0][m_white].pixel);

  hints.flags = PMinSize | PMaxSize;
  hints.min_width = hints.max_width = popupw;
  hints.min_height = hints.max_height = popuph;
  Xfprintf(stderr, "XSetStandardProperties(m_display, popup)\n");
  XSetStandardProperties(m_display, popup, "YesNo", "YesNo", None, NULL, 0, &hints);

  Xfprintf(stderr, "XSelectInput(m_display, popup)\n");
  XSelectInput(m_display, popup, KeyPressMask);
  Xfprintf(stderr, "XSelectInput(m_display, yesbutton)\n");
  XSelectInput(m_display, yesbutton, KeyPressMask | ButtonPressMask |
                                     ButtonReleaseMask |
	                             EnterWindowMask | LeaveWindowMask);
  Xfprintf(stderr, "XSelectInput(m_display, nobutton)\n");
  XSelectInput(m_display, nobutton, KeyPressMask | ButtonPressMask |
                                    ButtonReleaseMask |
                                    EnterWindowMask | LeaveWindowMask);

  Xfprintf(stderr, "XMapWindow(m_display, yesbutton)\n");
  XMapWindow(m_display, yesbutton);
  Xfprintf(stderr, "XMapWindow(m_display, nobutton)\n");
  XMapWindow(m_display, nobutton);
  Xfprintf(stderr, "XMapWindow(m_display, popup)\n");
  XMapWindow(m_display, popup);

  do {
    Xfprintf(stderr, "XMaskEvent()\n");
    XMaskEvent(m_display, KeyPressMask | ButtonPressMask | ButtonReleaseMask |
	                  EnterWindowMask | LeaveWindowMask, &event);
    switch (event.type) {
    case KeyPress:
/*      fprintf(stderr, "KeyPress event received");
      if (event.xany.window == yesbutton)
	fprintf(stderr, " in yes button window\n");
      else if (event.xany.window == nobutton)
	fprintf(stderr, " in no button window\n");
      else if (event.xany.window == popup)
        fprintf(stderr, " in popup window\n");
      else
	fprintf(stderr, " in window %d\n", event.xany.window);   */
      if (XLookupString((XKeyEvent *)&event, buf, 10, &sym, NULL))
	{
	  if ((buf[0] == 'y') || (buf[0] == 'Y')) 
	    {
	      done = 1;
	      return_value = 1;
	    } 
	  else 
	    if ((buf[0] == 'n') || (buf[0] == 'N')) 
	      {
		done = 1;
		return_value = 0;
	      }
	}
      break;
    case ButtonPress:
/*      fprintf(stderr, "ButtonPress event received");
      if (event.xany.window == yesbutton)
	fprintf(stderr, " in yes button window\n");
      else if (event.xany.window == nobutton)
	fprintf(stderr, " in no button window\n");
      else if (event.xany.window == popup)
        fprintf(stderr, " in popup window\n");
      else
	fprintf(stderr, " in window %d\n", event.xany.window);   */
      if (event.xany.window == yesbutton) {
	Xfprintf(stderr, "XFillRectangle()\n");
	XFillRectangle(m_display, yesbutton, tempgc, 0, 0, buttonw, buttonh);
	yesactive = 1;
      } else if (event.xany.window == nobutton) {
	Xfprintf(stderr, "XFillRectangle()\n");
	XFillRectangle(m_display, nobutton, tempgc, 0, 0, buttonw, buttonh);
	noactive = 1;
      }
      break;
    case ButtonRelease:
/*      fprintf(stderr, "ButtonRelease event received");
      if (event.xany.window == yesbutton)
	fprintf(stderr, " in yes button window\n");
      else if (event.xany.window == nobutton)
	fprintf(stderr, " in no button window\n");
      else if (event.xany.window == popup)
        fprintf(stderr, " in popup window\n");
      else
	fprintf(stderr, " in window %d\n", event.xany.window);   */
      if (inyes && yesactive) {
	done = 1;
	return_value = 1;
      } else if (inno && noactive) {
	done = 1;
	return_value = 0;
      }
      break;
    case EnterNotify:
/*      fprintf(stderr, "EnterNotify event received");
      if (event.xany.window == yesbutton)
	fprintf(stderr, " in yes button window\n");
      else if (event.xany.window == nobutton)
	fprintf(stderr, " in no button window\n");
      else if (event.xany.window == popup)
        fprintf(stderr, " in popup window\n");
      else
	fprintf(stderr, " in window %d\n", event.xany.window);   */
      if (event.xany.window == yesbutton) {
	inyes = 1;
	if (yesactive && (event.xcrossing.state &
			  (Button1Mask | Button2Mask | Button3Mask))) {
	  Xfprintf(stderr, "XFillRectangle()\n");
	  XFillRectangle(m_display, yesbutton, tempgc, 0, 0, buttonw, buttonh);
	} else
	  yesactive = 0;
      } else if (event.xany.window == nobutton) {
	inno = 1;
	if (noactive && (event.xcrossing.state &
			 (Button1Mask | Button2Mask | Button3Mask))) {
	  Xfprintf(stderr, "XFillRectangle()\n");
	  XFillRectangle(m_display, nobutton, tempgc, 0, 0, buttonw, buttonh);
	} else
	  noactive = 0;
      }
      break;
    case LeaveNotify:
/*      fprintf(stderr, "LeaveNotify event received");
      if (event.xany.window == yesbutton)
	fprintf(stderr, " in yes button window\n");
      else if (event.xany.window == nobutton)
	fprintf(stderr, " in no button window\n");
      else if (event.xany.window == popup)
        fprintf(stderr, " in popup window\n");
      else
	fprintf(stderr, " in window %d\n", event.xany.window);  */
      if (event.xany.window == yesbutton) {
	inyes = 0;
	if (yesactive) {
	  Xfprintf(stderr, "XClearWindow(m_display, yesbutton)\n");
	  XClearWindow(m_display, yesbutton);
	}
      } else if (event.xany.window == nobutton) {
	inno = 0;
	if (noactive) {
	  Xfprintf(stderr, "XClearWindow(m_display, nobutton)\n");
	  XClearWindow(m_display, nobutton);
	}
      }
      break;
/*    default:
      fprintf(stderr, "Other event received");
      if (event.xany.window == yesbutton)
	fprintf(stderr, " in yes button window\n");
      else if (event.xany.window == nobutton)
	fprintf(stderr, " in no button window\n");
      else if (event.xany.window == popup)
        fprintf(stderr, " in popup window\n");
      else
	fprintf(stderr, " in window %d\n", event.xany.window);  */
    }
  } while (! done);
  Xfprintf(stderr, "XDestroyWindow(m_display, popup)\n");
  XDestroyWindow(m_display, popup);
  return(return_value);
}




/* The following added by daveg, 10/6/89 */


void m_makechar(cp, a, b, c, d, e, f)
Anyptr *cp;
long a, b, c, d, e, f;
{
}


void m_changechar(cp, a, b, c, d, e)
Anyptr *cp;
long a, b, c, d, e;
{
}


void m_drawchar(cp)
Anyptr *cp;
{
}
