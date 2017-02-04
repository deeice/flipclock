/*  flip SDL clock
******************************************************************
  fc3.c
  Nokia N810
  
  gcc -g -Wall `sdl-config --cflags` `sdl-config --libs` -L /usr/include/SDL -lSDL_image fc3.c -o fc
  Ciro Ippolito 2009
  ******************************************************************
*/

/*2DO
one png for the entire number sets
...
...
********************************************************************
*/

#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#include <sys/select.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include "SDL_image.h"
#if defined(_MSC_VER)
#include "SDL.h"
#else
#include "SDL/SDL.h"
#endif

#ifdef ZIPIT_Z2
#include "SFont.h"
extern int SFont_DefaultCharWidth;

SFont_Font* Font=NULL;
SDL_Surface *font=NULL;
int fontheight = 30;

SDL_Surface* fade;
SDL_Surface* fadeSurface;

// NOTE: musl does NOT support %l for space padded hours.
// So the code to toggle between space and 0 pad (+'H'-'k') is busted.
// The Usage description says the unsupported %l is part of the default.
// And the default is indeed broken.  It displays all blanks
// because musl TERMINATES the output string when it hits the %l.
// 
// The solution is to split any format string with %l right before the %l
// The replace the l with an I and do a strftime for each half string.
// If the result string for the 2nd half starts with 0, replace it with space.
// The concatenate the two result halves.
// Maybe consier more string splits if multiple %l tokens are possible.

// Make a wrapper
size_t STRFTIME(char *s, size_t max, char *format, const struct tm *tm)
{
  int n;
  char c, *F, *f=format, *S=s; 
  //printf("STRFtime(%s) = <%s>\n",format,s);
  *s = 0;
  while (F = strstr(f, "%l"))
  {
    F++;
    *F = 'I';
    c = F[1];
    F[1] = 0;
    if (n = strftime(s,max,f,tm)) {
      //printf("STRFTIME(%s) = <%s>\n",f,s);
      if (s[n-2] == '0')
        s[n-2] = ' ';
      s += n;
      max -= n;
    }
    *F = 'l';
    f = F+1;
    *f = c;
  }
  strftime(s,max,f,tm);
  //printf("StrfTIME(%s) = <%s>\n",f,s);

  return (size_t)strlen(S);
}
char bannerfmt[30] = "%b %d, %Y  -  %l:%M %p";
char *bannertext[4] = {NULL,NULL,bannerfmt,NULL};
char bannertype[4] = {'t','t','D','t'};
int bannerfade[4] = {1,1,1,1};
int bannerfile[4] = {-1,-1,-1,-1};
int bannerfpos[4] = {0,0,0,0};
char *bannerfstr[4] = {NULL,NULL,NULL,NULL};
#endif
char *timestr = "%b %d, %Y; %l:%M:%S";
int time24hr = 0;
int time0pad = 0;

int update_display = 1;

SDL_Surface *screen;
SDL_Surface *secs;
SDL_Surface *fondo;
SDL_Surface *zero;
SDL_Surface *uno;
SDL_Surface *due;
SDL_Surface *tre;
SDL_Surface *quattro;
SDL_Surface *cinque;
SDL_Surface *sei;
SDL_Surface *sette;
SDL_Surface *otto;
SDL_Surface *nove;
SDL_Surface *numbers[10];
SDL_Surface *elements[10];
void initBMP()
{
    elements[0] = IMG_Load("fondo.png");
	elements[1] = IMG_Load("barra.png");
    elements[2] = IMG_Load("secs.png");
    numbers[0]  = IMG_Load("0.png");
    numbers[1]  = IMG_Load("1.png");
    numbers[2]  = IMG_Load("2.png");
    numbers[3]  = IMG_Load("3.png");
    numbers[4]  = IMG_Load("4.png");
    numbers[5]  = IMG_Load("5.png");
    numbers[6]  = IMG_Load("6.png");
    numbers[7]  = IMG_Load("7.png");
    numbers[8]  = IMG_Load("8.png");
    numbers[9]  = IMG_Load("9.png");
}

void LoadBMP(char *file, SDL_Surface *surface)
{
 surface= SDL_LoadBMP(file);
   if ( surface == NULL ) {
        fprintf(stderr, "Couldn't load %s: %s\n", file, SDL_GetError());
        return;
    }
  return;
}

void BMPShow(SDL_Surface *screen, SDL_Surface *pic,int x, int y)
{
  SDL_Rect dest;
  dest.x = x;
  dest.y = y;
  dest.w = pic->w;
  dest.h = pic->h;
  SDL_BlitSurface(pic, NULL, screen, &dest);
    // Update the changed portion of the screen 
    //SDL_UpdateRects(screen, 1, &dest);
  
  return;
}
void BMPShowArray(SDL_Surface *screen, SDL_Surface *pic,int x, int y)
{
  SDL_Rect dest;
  dest.x = x;
  dest.y = y;
  dest.w = pic->w;
  dest.h = pic->h;
  SDL_BlitSurface(pic, NULL, screen, &dest);
  //SDL_UpdateRects(screen, 1, &dest);
  return;	
}
void ShowBMP(char *file, SDL_Surface *screen, int x, int y)
{
    SDL_Surface *image;
    SDL_Rect dest;
    image = SDL_LoadBMP(file);
    if ( image == NULL ) {
        fprintf(stderr, "Couldn't load %s: %s\n", file, SDL_GetError());
        return;
    }
    dest.x = x;
    dest.y = y;
    dest.w = image->w;
    dest.h = image->h;
    SDL_BlitSurface(image, NULL, screen, &dest);
    SDL_BlitSurface(image, NULL, screen, &dest);
    //SDL_UpdateRects(screen, 1, &dest);
}

Uint32 TimerFunc( Uint32 interval, void *p )
{
    //printf("ciao modno\n");
    return 1000;
}

int checknumbers(char number,int x, int y){
    switch (number){
        case '0':
	  //if (x > 10)
            BMPShow (screen,numbers[0] , x   ,y);
            break;
        case '1':
            BMPShow (screen,numbers[1] , x   ,y);
            break;
        case '2':
            BMPShow (screen,numbers[2] , x   ,y);
            break;
        case '3':
            BMPShow (screen,numbers[3] , x   ,y);
            break;
        case '4':
            BMPShow (screen,numbers[4] , x   ,y);
            break;
        case '5':
            BMPShow (screen,numbers[5] , x   ,y);
            break;
        case '6':
            BMPShow (screen,numbers[6] , x   ,y);
            break;
        case '7':
            BMPShow (screen,numbers[7] , x   ,y);
            break;
        case '8':
            BMPShow (screen,numbers[8] , x   ,y);
            break;
        case '9':
            BMPShow (screen,numbers[9] , x   ,y);
            break;
    }    
return 1;
    
}

Uint32 UpdateClock (Uint32 interval, void *p)
{
    update_display = 1;
    return 1000;
}

int is_ready(int fd) {
    fd_set fdset;
    struct timeval timeout;
    int ret;
    FD_ZERO(&fdset);
    FD_SET(fd, &fdset);
    timeout.tv_sec = 0;
    timeout.tv_usec = 1;
    return select(fd+1, &fdset, NULL, NULL, &timeout) == 1 ? 1 : 0;
}

void RedrawClock(void)
{
    char s[30];
    size_t i;
    struct tm tim;
    time_t now;
    now = time(NULL);
    tim = *(localtime(&now));

#ifdef ZIPIT_Z2
    char timeformat[32];

    // Redraw the background if we allow spaces in strftime hours %l.
    BMPShow (screen, elements[0] ,0   ,0);        //Fondo

    strcpy(timeformat, timestr);
    if (time24hr)
      timeformat[12] = timeformat[12]-1;
    if (time0pad)
      timeformat[12] = timeformat[12] + 'H' - 'k';
    i = STRFTIME(s,30,timeformat,&tim);
    //printf("strftime(%s) = <%s>\n",timeformat,s);
    checknumbers(s[14],  0,8);                             //crappier time routine
    checknumbers(s[15],77,8);                             //crappier time routine 2
    checknumbers(s[17],166,8);                             //crappier time routine  3
    checknumbers(s[18],243,8);                             //crappier time routine   4
    BMPShow (screen,elements[1] ,0   ,0);                   //Redraw the top minute bar
    BMPShow (screen,elements[2] , (tim.tm_sec*52/10)+4 ,0);     //Seconds arrow
    if (Font)
    {
      int n;
      SDL_Rect dest;
      dest.x = 0;
      dest.y = 150 + 2 + 22-fontheight;
      dest.w = 320; //SFont_TextWidth(Font, s);
      dest.h = 22;
      for (n=0; n<4; n++)
      {
	//SDL_FillRect(screen, &dest, 0 );
	//printf("Clock text test(h = %d) %s\n", dest.h, s);
	// SFont_Write(screen, Font, 0, dest.y, "Clock text test (y=150)");
	if (bannertext[n] && strlen(bannertext[n]))
	{
	  //SFont_Write(screen, Font, 0, dest.y, bannertext); 
	  if (bannertype[n] == 'f')
	  {
	    int fd = bannerfile[n];
	    if (fd < 0)
	      fd = open(bannertext[n], O_RDONLY);
	    bannerfile[n] = fd;
	    if (fd >= 0)
	    {
	      char buffer[128];
	      char *p;
	      int j = bannerfpos[n];
	      buffer[0] = 0;
	      if (is_ready(fd)) {
		j += read(fd, buffer, sizeof(buffer)-2-j);
		buffer[j] = 0;
		if (j == sizeof(buffer)-2)
		  strcat(buffer, "\n");
		if (p = strpbrk(buffer, "\r\n")){
		  *p = 0;
		  close(fd);
		  bannerfile[n] =-1;
		  bannerfpos[n] = 0;
		  if (bannerfstr[n]) free(bannerfstr[n]);
		  bannerfstr[n] = strdup(buffer);
		}
	      }
	    }
	    SDL_FillRect(screen, &dest, 0 );
	    if (bannerfstr[n])
	      SFont_WriteCenter(screen, Font, dest.y, bannerfstr[n]);
	  }
	  else
	  if (bannertype[n] == 'D')
	  {
	    i = STRFTIME(s,30,bannertext[n],&tim);
	    //printf("Clock text test(%s) = $s \n",bannertext[n],s);
	    SDL_FillRect(screen, &dest, 0 );
	    SFont_WriteCenter(screen,Font,dest.y,s);
	  }
	  else
	    SFont_WriteCenter(screen, Font, dest.y, bannertext[n]);
	  for (i=0; i<bannerfade[n]; i++)
	    SDL_BlitSurface(fade, &dest, screen, &dest); // Fade it.
	}
	dest.y += dest.h;
      }

      //SDL_FillRect(screen, &dest, 0 );
      //SDL_FillRect(screen, &dest, SDL_MapRGBA(screen->format,0,0,0,127));
      
      // Update the screen (Should just calculate the rect)
      //SDL_UpdateRect(screen, 0, 0, 0, 0);
    }
#else
    i = strftime(s,30,"%b %d, %Y; %H:%M:%S\n",&tim);
    checknumbers(s[14],  2,20);                             //crappier time routine
    checknumbers(s[15],193,20);                             //crappier time routine 2
    checknumbers(s[17],415,20);                             //crappier time routine  3
    checknumbers(s[18],605,20);                             //crappier time routine   4
    BMPShow (screen,elements[1] ,0   ,0);                   //Redraw the top minute bar
    BMPShow (screen,elements[2] , tim.tm_sec*13+10 ,0);     //Seconds arrow
#endif
  }
/* ----------------------------------------------------------------------------
MAIN ()
 ----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    /* loop along the possible list of arguments */
    while((--argc>0) && (**++argv=='-'))
    {
      char c;
      int n;
      //printf("processing(%d, %s)\n",argc,*argv); 
      while(c=*++*argv) // Eat a bunch of one char commands after -.
      {
	//printf("cmd %c (%d, %s)\n",c, argc,*argv); 
        switch(c)
        {
	case 't':
	  time24hr ^= 1;
          break;
	case 'p':
	case '0':
	  time0pad ^= 1;
          break;
	case 'h':
	case '?':
	  printf("Usage:  fc -t -p -[1|2|3|4][D|f][b|d] [message]\n"
	    "\n"
	    "Display a flipclock with 4 lines of text messages.\n"
	    "\n"
	    "  t = 24hr clock.\n"
	    "  p = Pad clock hour with a zero.\n"
	    "  f = Poll file for message updates.\n"
	    "  D = Use message as strftime date format string.\n"
	    "      Line 3 defaults to a date using: \n"
	    "      \"%%b %%d, %%Y  -  %%l:%%M %%p\".\n"
	    "  b = brighter (bold) text\n"
	    "  d = darker text.\n"
	    "\n"
	    "eg:"
	    "  fc -tp -1d \"Nice weather.\" -2D \"%%F - %%R %%p\" -3 \"\" -4 \"Wake at 7 AM\"\n"
	     );
	  exit(0);
	case '1': /* -nc bannertext */
	case '2': 
	case '3': 
	case '4': 
	  n = c-'1';
	  while(c=*++*argv) // Eat a bunch of one char commands after -.
	  {
	    if (c == 'f') bannertype[n] = c; // fifo filename
	    if (c == 'D') bannertype[n] = c;
	    if (c == 'w') bannerfade[n] = 0;
	    if (c == 'b') bannerfade[n] = 0;
	    if (c == 'd') bannerfade[n] = 2;
	  }
	  --*argv; // Put back one char for the loop to eat.
	  --argc;
	  //printf("argc,v = (%d, %d)\n",argc,argv+1); 
	  if ((argc > 0) && *++argv){
	    bannertext[n] = strdup(*argv);
	    //printf("bannertext[%d].%d =  %s\n",n,bannerfade[n],bannertext[n]); 
	    *argv += strlen(*argv)-1;
	  }
	  //else printf("No text\n");
	  //printf("Done with(%d, %s)\n",argc,*argv); 
          break;

	case 'W': 
	  sscanf(*argv, "W%d", &SFont_DefaultCharWidth);
	  *argv += strlen(*argv)-1;
	  printf("SFont_DefaultCharWidth = %d\n", SFont_DefaultCharWidth);
	  printf("Done with(%d, %s)\n",argc,*argv); 
          break;

        default:
          // ignore it
          break;
        }
      }
    }

  //    if ( SDL_Init(SDL_INIT_EVERYTHING) < 0 ) 
    if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) 
    {
        fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
        exit(1);
    }
    SDL_Init(SDL_INIT_TIMER);
    // Register SDL_Quit to be called at exit; makes sure things are
    atexit(SDL_Quit); 								// cleaned up when we quit.
#ifdef ZIPIT_Z2
    //screen = SDL_SetVideoMode(320, 240, 16, SDL_SWSURFACE);             // Window PC
    screen = SDL_SetVideoMode(320, 240, 16, SDL_HWSURFACE|SDL_DOUBLEBUF); // Window PC
#else
    //screen = SDL_SetVideoMode(800, 480, 16, SDL_SWSURFACE|SDL_FULLSCREEN);  // Full screen Tablet
    screen = SDL_SetVideoMode(800, 480, 16, SDL_SWSURFACE);                   // Window PC
#endif
    if ( screen == NULL )     							                      // If fail, return error
    {
        fprintf(stderr, "Unable to set 640x480 video: %s\n", SDL_GetError());
        exit(1);
    }

#ifdef ZIPIT_Z2

    // WARNING!!  Gotta fix sfont for utf8 bits or this crashes on gmenu2x font.
    //            Maybe just stop at first utf8 (190?)

    //Font = SFont_InitFont(IMG_Load("font.png"));
    font = IMG_Load("font.png");
    Font = SFont_InitFont(font);
    if(!Font) {
      fprintf(stderr, "An error occured while loading the font.\n");
	exit(1);
    }
    fontheight = SFont_TextHeight(Font);
    //printf("sfont = %d, %d\n", font, Font);

    //SDL_SetAlpha(Font->Surface, SDL_SRCALPHA | SDL_RLEACCEL, 128);

    // Create a surface with alpha to use for fade effect.
    fadeSurface = SDL_CreateRGBSurface( SDL_SWSURFACE | SDL_SRCALPHA, 320, 240, 32, 0xff000000,0x00ff0000,0x0000ff00,0x000000ff);

    // Convert it to the format of the screen
    fade = SDL_DisplayFormat( fadeSurface );
    SDL_SetAlpha(fade, SDL_SRCALPHA | SDL_RLEACCEL, 128);

    // Free the created surface
    SDL_FreeSurface( fadeSurface );
#endif

    initBMP();

    SDL_AddTimer (1000,UpdateClock,NULL); ///////////////////////////////////////////// ////
    typedef enum { FALSE, TRUE } boolean;
    boolean clockticking = TRUE;
    while (clockticking)					        // Main loop: loop forever
    {
        SDL_Event event;			                // Poll for events

	if (update_display)
	{
	    RedrawClock();
	    SDL_Flip(screen);
	    update_display = 0;
	}

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_KEYDOWN:
#ifdef ZIPIT_Z2
            if (event.key.keysym.sym == SDLK_ESCAPE) // If ESC is pressed, return (and quit)
                return 0;
	    else if (event.key.keysym.sym == SDLK_t) 
	      time24hr ^= 1;
	    else if ((event.key.keysym.sym == SDLK_p) || (event.key.keysym.sym == SDLK_0))
	      time0pad ^= 1;
#else
                if (event.key.keysym.sym == SDLK_a)	{
                printf("premuto A");
            }
            else if (event.key.keysym.sym == SDLK_s){
                printf("premuto S");
            }
            else if (event.key.keysym.sym == SDLK_d){
                printf("premuto D");
            }
            else if (event.key.keysym.sym == SDLK_f){
                printf("premuto F");
            }
#endif
            else if (event.key.keysym.sym == SDLK_g)
                BMPShow (screen, elements[0]  ,0    ,0);
		break;
            case SDL_KEYUP:
            if (event.key.keysym.sym == SDLK_q)		// If Q is pressed, return (and zen quit)
                return 0;
                break;     
            case SDL_QUIT:
                return(0);
            }
        }
	SDL_Delay(10);
    }
    return 0;
}


