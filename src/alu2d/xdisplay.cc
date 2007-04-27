
static volatile char RCSId_xdisplay_cc [] = "$Id$";

// ***************************************************
// #begin(header)
// #filename:
//   xdisplay.cc
// #description:
//   Implementierungen zu Xdisplay
// #methods:
//   Xdisplay::Xdisplay()
//   Xdisplay::~Xdisplay()
//   Xdisplay::linedraw() 
// #end(header)
// ***************************************************

#include <stdio.h>	//  sprintf()

#include "grid.h"

// needed for new Silicon-CC
#include <string.h>

// #include "xdisplay.h"

// ***************************************************
// #begin(method)
// #method:
//   Xdisplay::Xdisplay(int argc, char **argv,
//                      char *text, char *windowname,
//                      double pscale, double pxoffset, double pyoffset)
// #parameters:
//   \ int | argc | Anzahl der Argumente der variablen
//                  Argumentenliste
//   \ char | **argv | variable Argumentenliste
//   \ char | *text | Textausgabe im Zeichenfenster
//   \ char | *windowname | Name des Fensters
//   \ double | pscale | Skalierungsfaktor f"ur Gitterdarstellung
//   \ double | pxoffset | Verschiebung des Gitters im Zeichenfenster
//                         in x--Richtung
//   \ double | pyoffset | Verschiebung des Gitters im Zeichenfenster
//                         in y--Richtung
// #description:
//   Der Konstruktor erzeugt ein ausgabebereites
//   X11--Fenster und definiert Farben f"ur die
//   Ausgabe der Randelemente. Die Argumente
//   ''argc'' und ''argv'' sind obligatorisch und m"ussen
//   von ''main'' durchgereicht werden. F"ur die "ubrigen
//   Argumente sind Default--Werte definiert, die die
//   optimale Einstellung f"ur das Rechtecksgebiet
//   [0,1]x[0,1] realisieren. Im einzelnen sind das
//   die Werte:
//   \ text := ''''
//   \ windowname := ''Xdisplay: output''
//   \ pscale := 500.0
//   \ pxoffset := 125.0
//   \ pyoffset := 600.0
// #end(method)
// ***************************************************

Xdisplay::Xdisplay(int argc, char **argv, char *text, char *windowname,
                   double pscale, double pxoffset, double pyoffset)
{
  int i;

  disp.scale   = pscale;
  disp.xoffset = pxoffset;
  disp.yoffset = pyoffset;

  disp.display = XOpenDisplay("");
  disp.screen  = DefaultScreen(disp.display);

  disp.background = WhitePixel(disp.display, disp.screen);
  disp.foreground = BlackPixel(disp.display, disp.screen);

  disp.sizehints.x      = 600;
  disp.sizehints.y      = 700;
  disp.sizehints.width  = 900;
  disp.sizehints.height = 650;
  disp.sizehints.flags  = PPosition | PSize;

  disp.window = XCreateSimpleWindow(disp.display,
                                    DefaultRootWindow(disp.display),
                                    disp.sizehints.x,
                                    disp.sizehints.y,
                                    disp.sizehints.width,
                                    disp.sizehints.height,
                                    0,
                                    disp.foreground,
                                    disp.background);

  XSetStandardProperties(disp.display, disp.window,
                         windowname, windowname, None,
                         argv, argc, &(disp.sizehints));

  disp.gc = XCreateGC(disp.display, disp.window, 0,0);
  XSetBackground(disp.display, disp.gc, disp.background);
  XSetForeground(disp.display, disp.gc, disp.foreground);

  XSelectInput(disp.display,disp.window,KeyPressMask|ExposureMask);

  XMapRaised(disp.display, disp.window);

  XNextEvent(disp.display, &disp.event);

  XDrawImageString(disp.event.xexpose.display, disp.event.xexpose.window,
                           disp.gc, 30, 30, text, strlen(text));
  XDrawLine(disp.event.xexpose.display, disp.event.xexpose.window,
            disp.gc, 0, 50, 900, 50);

  disp.colormap = DefaultColormap(disp.event.xexpose.display,
                                  disp.screen);

  sprintf(bnd_color[0], "black");
  sprintf(bnd_color[1], "green");
  sprintf(bnd_color[2], "dark green");
  sprintf(bnd_color[3], "forest green");
  sprintf(bnd_color[4], "lime green");
  sprintf(bnd_color[5], "spring green");
  sprintf(bnd_color[6], "orange");
  sprintf(bnd_color[7], "gold");
  sprintf(bnd_color[8], "dark turquoise");
  sprintf(bnd_color[9], "medium turquoise");
  sprintf(bnd_color[10],"turquoise");
  sprintf(bnd_color[11],"salmon");
  sprintf(bnd_color[12],"red");
  sprintf(bnd_color[13],"indian red");
  sprintf(bnd_color[14],"medium violet red");
  sprintf(bnd_color[15],"orange red");
  sprintf(bnd_color[16],"violet red");
  sprintf(bnd_color[17],"blue");
  sprintf(bnd_color[18],"cadet blue");
  sprintf(bnd_color[19],"dark slate blue");
  sprintf(bnd_color[20],"light blue");
  sprintf(bnd_color[21],"sky blue");
  sprintf(bnd_color[22],"white");
  sprintf(bnd_color[23],"yellow");

  for (i=0;i<nrof_bndcols;i++)
  {
    XColor rgb_db;

    bcol_text[i][0] = '\0';
    XAllocNamedColor(disp.event.xexpose.display,disp.colormap,bnd_color[i],
                     &(bcol[i]),&rgb_db);
  } 
}


// ***************************************************
// #begin(method)
// #method:
//   Xdisplay::~Xdisplay()
// #description:
//   Der Destruktor zeigt gegebenenfalls eine
//   Legende zur Farbwahl der Randelemente an
//   und wartet vor dem Schlie"sen des X11--Fensters
//   auf eine Tastatureingabe.
// #end(method)
// ***************************************************

Xdisplay::~Xdisplay()
{
  int i,done = 0,bnd_used = 0;
  char text[100];

  for (i=0;i<nrof_bndcols;i++)
    if (strlen(bcol_text[i]) > 0) bnd_used++;

  if (0 != bnd_used)
  {
    sprintf(text,"Bndel::typ");
    XDrawImageString(disp.event.xexpose.display, disp.event.xexpose.window,
                             disp.gc, 700, 100, text, strlen(text));
    bnd_used = 0;
    for (i=0;i<nrof_bndcols;i++)
      if (strlen(bcol_text[i]))
      {
        bnd_used++;
        XSetForeground(disp.event.xexpose.display,disp.gc,(bcol[i]).pixel);
        XFillArc(disp.event.xexpose.display,disp.event.xexpose.window,
                         disp.gc, 680, 110+bnd_used*25,
                         10,10,0,23040);
        XSetForeground(disp.event.xexpose.display,disp.gc,disp.foreground);
        XDrawImageString(disp.event.xexpose.display, disp.event.xexpose.window,
                         disp.gc, 700, 120+bnd_used*25,
                         bcol_text[i], strlen(bcol_text[i]));
      }
  }

  sprintf(text,"press <q> to quit ...");
  XDrawImageString(disp.event.xexpose.display, disp.event.xexpose.window,
                           disp.gc, 630, 630, text, strlen(text));  

  while (!done) {
    XNextEvent(disp.display, &disp.event);
    switch (disp.event.type) {
      case MappingNotify:
        XRefreshKeyboardMapping((XMappingEvent *)&disp.event);
        break;
    case KeyPress:done=1;break;
        i = XLookupString((XKeyEvent *)&disp.event, text, 10, &disp.keysym, 0);
        if (i==1 && text[0] == 'q') done = 1;
        break;
     }
  }
  XFreeGC(disp.display, disp.gc);
  XDestroyWindow(disp.display, disp.window);
  XCloseDisplay(disp.display);
}


// ***************************************************
// #begin(method)
// #method:
//   void Xdisplay::linedraw(Vertex *v0, Vertex *v1) const
// #parameters:
//   \ Vertex | *v0 | Zeiger auf Punkt 0
//   \ Vertex | *v1 | Zeiger auf Punkt 1
// #description:
//   Punkt 0 und Punkt 1 werden durch ein Geradensegment
//   in der aktuellen Zeichenfarbe verbunden.
// #end(method)
// ***************************************************

void Xdisplay::linedraw(Vertex *v0, Vertex *v1) const
{
  XDrawLine(disp.event.xexpose.display,
            disp.event.xexpose.window,
            disp.gc,
           (int)((v0->coord())[0]*disp.scale+disp.xoffset),
           (int)(-(v0->coord())[1]*disp.scale+disp.yoffset), 
           (int)((v1->coord())[0]*disp.scale+disp.xoffset),
           (int)(-(v1->coord())[1]*disp.scale+disp.yoffset));
}



// ***************************************************
// #begin(method)
// #method:
//   void Xdisplay::linedraw(Vertex *v0, Vertex *v1, XColor col) const
// #parameters:
//   \ Vertex | *v0 | Zeiger auf Punkt 0
//   \ Vertex | *v1 | Zeiger auf Punkt 1 
//   \ XColor | col | Zeichenfarbe
// #description:
//   Punkt 0 und Punkt 1 werden durch ein Geradensegment
//   in der Farbe col verbunden.
// #end(method)
// ***************************************************

void Xdisplay::linedraw(Vertex *v0, Vertex *v1, XColor col) const
{
  XSetForeground(disp.event.xexpose.display,disp.gc,col.pixel);
  linedraw(v0,v1);
  XSetForeground(disp.event.xexpose.display,disp.gc,disp.foreground);
}


// ***************************************************
// #begin(method)
// #method:
//   void Xdisplay::linedraw(Vertex *v0, Vertex *v1, char col[30]) const
// #parameters:
//   \ Vertex | *v0 | Zeiger auf Punkt 0
//   \ Vertex | *v1 | Zeiger auf Punkt 1 
//   \ char | col[30] | Name der gew"unschten Zeichenfarbe  
// #description:
//   Punkt 0 und Punkt 1 werden durch ein Geradensegment
//   in der gew"unschten Farbe verbunden, falls in der
//   X11--Farbdatenbank ein entsprechender Eintrag
//   gefunden wird; andernfalls wird nach Ausgabe einer
//   Fehlermeldung das Segment in der aktuellen Farbe
//   gezeichnet.
// #end(method)
// ***************************************************

void Xdisplay::linedraw(Vertex *v0, Vertex *v1, char col[30]) const
{
  XColor colcell, rgb_db;

  if (XAllocNamedColor(disp.event.xexpose.display,disp.colormap,col,
                       &colcell,&rgb_db))
    linedraw(v0,v1,colcell);
  else
  {
    fprintf(stderr,"Xdisplay::linedraw : unable to find color \"%s\"\n",col);
    fprintf(stderr,"                     in color database, using default color!\n");
    linedraw(v0,v1);
  }
}
