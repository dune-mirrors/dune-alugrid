#ifndef __HEADER__XDISPLAY
#define __HEADER__XDISPLAY

// ***************************************************
// #begin(header)
// #filename:
//   xdisplay.h
// #description:
//   Basisklasse f"ur einfache Grafikausgabe 
// #classes:
//   class Xdisplay
// #copyright:
//   IAM Freiburg
// #end(header)
// ***************************************************

#ifdef USE_ALUGRID_XDISPLAY 
#define USE_ALUGRID_XDISPLAY 1
#endif

#if USE_ALUGRID_XDISPLAY

#include <X11/Xlib.h>
#include <X11/Xutil.h>

class Vertex;

// ***************************************************
// #begin(class)
// #description:
//   Fenster und einfache Zeichenroutinen.
// #definition:
class Xdisplay
{
  public:
    typedef struct 
    {
      Display    *display;
      Window     window;
      GC         gc;
      XEvent     event;
      KeySym     keysym;
      XSizeHints sizehints;
      Colormap   colormap;
      int        screen;
      unsigned long foreground, background;
      double scale,xoffset,yoffset;
    } display_t;
    enum {nrof_bndcols=24};
    char bnd_color[nrof_bndcols][30];
    XColor bcol[nrof_bndcols];
    char bcol_text[nrof_bndcols][30];
  private:
    display_t disp;
  public:
    Xdisplay(int argc, char **argv,
             char *text = "\0", char *windowname = "Xdisplay: output",
             double pscale = 500.0,
             double pxoffset = 125.0, double pyoffset = 600.0);
    ~Xdisplay();
    void linedraw(Vertex *, Vertex *) const;
    void linedraw(Vertex *, Vertex *, XColor) const;
    void linedraw(Vertex *, Vertex *, char [30]) const;
};
#endif
// #end(class)
// ***************************************************
#endif
