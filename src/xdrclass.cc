#include "xdrclass.hh"

#ifdef IBM_XLC
  typedef char xdrbuff_t;
  typedef char xdraddr_t;
#else
  typedef void xdrbuff_t;
  typedef void xdraddr_t;
#endif

static int read_xdr_file(void *file, xdrbuff_t *buffer, xdrsize_t size)
{
  int result;

  result = (int)fread(buffer, 1, (size_t)size, (FILE *)file);

  if(size > 0 && result == 0)
    return -1;

 return result;
}

static int write_xdr_file(void *file, xdrbuff_t *buffer, xdrsize_t size)
{
  return (fwrite(buffer, (size_t)size, 1, (FILE *)file) == 1 ? (int)size : 0);
}

typedef struct {
  FILE *fp;
  char *filename;
  bool_t gezipt;
} XDR_info;

XDR *XDRopen(const char *filename,const xdr_op op)
{
  XDR *x=NULL;
  XDR_info *info;
  int gezipt=0;
  FILE *fp;
  if (op==XDR_DECODE)
    fp=fopen(filename,"r");
  else if (op==XDR_ENCODE)
    fp=fopen(filename,"w");
  else 
    abort();
  if (!fp && op==XDR_DECODE)
  {
    /* Schau nach geziptem File */
    /* ... */
    if (!fp)
      return NULL;
  } 
  assert(fp);
  x=(XDR*) malloc(sizeof(XDR));
  info=(XDR_info*) malloc(sizeof(XDR_info));
  assert(x && info);
  xdrrec_create(x, 65536, 65536, (xdraddr_t *)fp,
                read_xdr_file, write_xdr_file);
  x->x_op = op;
  info->fp = fp;
  info->gezipt = gezipt;
  x->x_public = (caddr_t)info; 
  if (op==XDR_DECODE)
    xdrrec_skiprecord(x);
  return x;
}
XDR *XDRclose(XDR *x)
{
  XDR_info *info=(XDR_info*)x->x_public;
  if (x->x_op==XDR_ENCODE)
    xdrrec_endofrecord(x, TRUE); 
  xdr_destroy(x);
  fclose(info->fp);
  if (info->gezipt) {abort();}
  free(info);
  free(x);
  return NULL;
}
