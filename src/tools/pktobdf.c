/*
**  pktobdf
**
**  adapted from "pktopx in C by Tomas Rokicki" by AJCD 1/8/90
**  modified by Richard Verhoeven to add XLFD support and
**  fixed some bugs (maybe also introduced some).
**    
**  compile with: cc -o pktobdf pktobdf.c
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#define MAXPKCHAR 256
#define MAX_COMMENT 256

#define round(a) ((int)(a+0.5))

typedef int integer ;
typedef unsigned char quarterword ;
typedef char boolean ;
typedef quarterword eightbits ;
typedef FILE *bytefile ;

bytefile pkfile ;
bytefile bdffile ;
char pkname[2048] ;
char *fnname ;
integer pkloc ;
/* integer i, j ;*/
char *filename ;
integer dynf ;
eightbits inputbyte ;
eightbits bitweight ;
integer repeatcount ;
integer flagbyte ;
char *comments[MAX_COMMENT];
integer ascent_dist[MAXPKCHAR], descent_dist[MAXPKCHAR];
integer nr_comments=0;
integer skip_nr_specials=1;
struct pkchar {
   integer tfmwidth;
   integer hesc, vesc;
   integer width, height;
   integer hoffset, voffset;
   char * bitmap;
} pkchar[MAXPKCHAR];

integer bitarray[] = {128, 64, 32, 16, 8, 4, 2, 1};
#define bit(c) (bitarray[(c) & 7])

char *hexarray = "0123456789abcdef";
#define hex(c) (hexarray[(c) & 15])

char *texenc[256] = { /* TeX typewriter encoding vector */
    "Gamma", "Delta", "Theta", "Lamdba", "Xi", "Pi", "Sigma", "Upsilon",
    "Phi", "Psi", "Omega", "arrowup", "arrowdown", "quotesingle", "exclamdown", "questiondown",
    "dotlessi", "dotlessj", "grave", "acute", "caron", "breve", "macron", "ring",
   "cedilla", "germandbls", "ae", "oe", "oslash", "AE", "OE", "Oslash",
   "tilde", "exclam", "quotedblright", "numbersign", "dollar", "percent", "ampersand", "quoteright",
   "parenleft", "parenright", "asterisk", "plus", "comma", "hyphen", "period", "slash",
   "0", "1", "2", "3", "4", "5", "6", "7",
   "8", "9", "colon", "semicolon", "less", "equal", "greater", "question",
   "at", "A", "B", "C", "D", "E", "F", "G",
   "H", "I", "J", "K", "L", "M", "N", "O",
   "P", "Q", "R", "S", "T", "U", "V", "W",
   "X", "Y", "Z", "bracketleft", "leftdblquote", "bracketright", "circumflex", "dotaccent",
   "leftquote", "a", "b", "c", "d", "e", "f", "g",
   "h", "i", "j", "k", "l", "m", "n", "o",
   "p", "q", "r", "s", "t", "u", "v", "w",
   "x", "y", "z", "endash", "emdash", "quotedbl", "hungarumlaut", "dieresis",
   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

static void add_suffix(name, suffix)
     char *name, *suffix ;
{
  int haveext = 0;
  if (name) {
    while (*name) {
      if (*name == '/') haveext = 0 ;
      else if (*name == '.') haveext = 1 ;
      name++ ;
    }
    if (!haveext) {
      *name++ = '.';
      strcpy(name,suffix) ;
    }
  }
}

static void jumpout()
{
  exit(1) ;
}

static void initialize()
{
  integer i ;
  
  fprintf(stderr, "This is Pktobdf, C Version 2.3\n") ;
  for (i = 0; i < MAXPKCHAR; i++)
     pkchar[i].bitmap = NULL;
  filename = NULL ;
}

static eightbits pkbyte()
{
  pkloc++ ;
  return(getc(pkfile)) ;
}

static integer get16()
{
  integer a = pkbyte() ;
  return((a<<8) + pkbyte()) ;
}

static integer get32()
{
  integer a = get16() ;
  if (a > 32767) a -= 65536 ;
  return((a<<16) + get16()) ;
}

static integer getnyb()
{
  eightbits temp ;
  if (bitweight == 0) {
    inputbyte = pkbyte() ;
    bitweight = 16 ;
  }
  temp = inputbyte / bitweight ;
  inputbyte -= temp * bitweight ;
  bitweight >>= 4 ;
  return(temp) ;
}

static boolean getbit()
{
  boolean temp ;
  bitweight >>= 1 ;
  if (bitweight == 0) {
    inputbyte = pkbyte() ;
    bitweight = 128 ;
  }
  temp = (inputbyte >= bitweight) ;
  if (temp) inputbyte -= bitweight ;
  return(temp) ;
}

static integer pkpackednum()
{
  integer i, j ;
  i = getnyb() ;
  if (i == 0) {
    do {
      j = getnyb() ;
      i++ ;
    } while (!(j != 0)) ;
    while (i > 0) {
      j = (j<<4) + getnyb() ;
      i-- ;
    }
    return(j - 15 +((13 - dynf)<<4) + dynf) ;
  } else if (i <= dynf) return(i) ;
  else if (i < 14) return(((i - dynf - 1)<<4) + getnyb() + dynf + 1) ;
  else {
    if (i == 14) repeatcount = pkpackednum() ;
    else repeatcount = 1 ;
    return(pkpackednum()) ;
  }
}

static void skipspecials()
{
  integer i, j, k ;
  do {
    flagbyte = pkbyte() ;
    if (flagbyte >= 240)
      switch(flagbyte) {
      case 240:
      case 241:
      case 242:
      case 243:
	i = 0 ;
	for (j = 240 ; j <= flagbyte ; j ++) i = (i<<8) + pkbyte() ;
	if (nr_comments==MAX_COMMENT) {
	    fprintf(stderr, "Skipping special (%i characters).\n", i);
	    for (j=0 ; j<i ; j++) k = pkbyte() ;
	} else {
	    if (!(comments[nr_comments] = malloc(sizeof(char)*(i+1)))) {
		fprintf(stderr, "Out of memory.\n"
			"Skipping special (%i characters).\n", i);
		for (j=0 ; j<i ; j++) k = pkbyte() ;
	    } else {
		for (j=0 ; j<i ; j++) comments[nr_comments][j] = pkbyte() ;
		comments[nr_comments++][j] = '\0';
	    }
	}
	break ;
      case 244:
	i = get32() ;
	if (!skip_nr_specials) {
	    if (nr_comments==MAX_COMMENT) {
		fprintf(stderr, "Skipping numeric special (%i).\n", i);
	    } else {
		if (!(comments[nr_comments] =
		      (char*) malloc(sizeof(char)*20))) {
		    fprintf(stderr, "Out of memory.\n"
			    "Skipping numeric special (%i).\n", i);
		} else {
		    sprintf(comments[nr_comments],"Nr: %i", i);
		    nr_comments++;
		}
	    }
	}
	break ;
      case 245:
	break ;
      case 246:
	break ;
      case 247:
      case 248:
      case 249:
      case 250:
      case 251:
      case 252:
      case 253:
      case 254:
      case 255:
	fprintf(stderr, " Unexpected flag byte %d!\n", flagbyte) ;
	jumpout() ;
      }
  } while (!((flagbyte < 240) || (flagbyte == 245))) ;
}

static void usage()
{
  fprintf(stderr,
	  " Usage: pktobdf [options] pkfile bdffile\n\n"
	  " where the following options are allowed:\n"
	  "\t-foundry <name>      (metafont, adobe)\n"
	  "\t-family <name>       (times, lucida)\n"
	  "\t-weight <name>       (medium, bold)\n"
	  "\t-slant <name>        (r, i, o)\n"
	  "\t-widthname <name>    (normal, semicondensed)\n"
	  "\t-style <name>        (sans)\n"
	  "\t-pixelsize <num>     (10,12,14)\n"
	  "\t-pointsize <num>     (100,120,140)\n"
	  "\t-resolution <num>    (75, 100)\n"
	  "\t-spacing <name>      (p, m, c)\n"
	  "\t-avgwidth <num>      (70,80,90)\n"
	  "\t-registry <name>     (metafont, adobe)\n"
	  "\t-encoding <name>     (fontspecific, 1, 8)\n\n"
	  "\t-default <num>       (default character)\n"
	  "\t-fullname <name>     (font full name)\n"
	  "\t-avgcapital <num>    (average capital width)\n"
	  "\t-avglowercase <num>  (average lowercase width)\n"
	  "\t-copyright <name>    (copyright notice)\n\n");
  jumpout() ;
}

char *foundry="metafont";
char *familyname=NULL;
char *weightname="medium";
char *slant="r";
char *setwidthname="normal";
char *addstylename="";
integer pixelsize=0;
integer defaultchar=-2;
integer pointsize=0;
integer resolution=75;
integer percent=95;
integer checkhesc=0;
char *spacing="P";
integer averagewidth=0;
char *charsetregistry="MetaFont";
char *charsetencoding="FONTSPECIFIC";
integer avgcapitalwidth=0;
integer avglowercasewidth=0;
char *copyright="MetaFont (public domain)";
char *fullname=NULL;


static void dialog(gargc, gargv)
     int gargc ;
     char **gargv ;
{
  char *s;
  integer i=1;
  integer pkfilename=0;

  pkloc = 0;
  pkfile = stdin;
  bdffile = stdout;
  fnname = strcpy(pkname, "stdin");
  while (i<gargc) {
      if (gargv[i][0]=='-' && gargv[i][1] && i+1<gargc) {
	  if (!strcmp(gargv[i]+1,"weight"))
	      weightname=gargv[++i];
	  else if (!strcmp(gargv[i]+1,"widthname"))
	      setwidthname=gargv[++i];
	  else if (!strcmp(gargv[i]+1,"foundry"))
	      foundry=gargv[++i];
	  else if (!strcmp(gargv[i]+1,"family"))
	      familyname=gargv[++i];
	  else if (!strcmp(gargv[i]+1,"fullname"))
	      fullname=gargv[++i];
	  else if (!strcmp(gargv[i]+1,"slant"))
	      slant=gargv[++i];
	  else if (!strcmp(gargv[i]+1, "style"))
	      addstylename=gargv[++i];
	  else if (!strcmp(gargv[i]+1,"spacing"))
	      spacing=gargv[++i];
	  else if (!strcmp(gargv[i]+1,"default"))
	      sscanf(gargv[++i],"%i", &defaultchar);
	  else if (!strcmp(gargv[i]+1,"percent"))
	      sscanf(gargv[++i],"%i", &percent);
	  else if (!strcmp(gargv[i]+1,"pixelsize"))
	      sscanf(gargv[++i],"%i", &pixelsize);
	  else if (!strcmp(gargv[i]+1, "pointsize"))
	      sscanf(gargv[++i],"%i", &pointsize);
	  else if (!strcmp(gargv[i]+1, "resolution"))
	      sscanf(gargv[++i], "%i", &resolution);
	  else if (!strcmp(gargv[i]+1,"registy"))
	      charsetregistry=gargv[++i];
	  else if (!strcmp(gargv[i]+1,"avgwidth"))
	      sscanf(gargv[++i], "%i", &averagewidth);
	  else if (!strcmp(gargv[i]+1,"avgcapital"))
	      sscanf(gargv[++i], "%i", &avgcapitalwidth);
	  else if (!strcmp(gargv[i]+1,"avglowercase"))
	      sscanf(gargv[++i], "%i", &avglowercasewidth);
	  else if (!strcmp(gargv[i]+1,"encoding"))
	      charsetencoding=gargv[++i];
	  else if (!strcmp(gargv[i]+1,"copyright"))
	      copyright=gargv[++i];
	  else if (!strcmp(gargv[i]+1,"specials"))
	      skip_nr_specials=0;
	  else if (!strcmp(gargv[i]+1,"check"))
	      checkhesc=1;
	  else usage();
      } else if (gargv[i][0]=='-')
	  if (gargv[i][1]) {
	      fprintf(stderr, " Option %s needs an argument.\n", gargv[i]);
	      usage();
	  } else
	      pkfilename++;
      else {
	  if (pkfilename >2) {
	      fprintf(stderr, " Too many filenames specified.\n");
	      usage();
	  } else {
	      if (!pkfilename) {
		  strcpy(pkname, gargv[i]) ;
		  add_suffix(pkname, "pk") ;
		  if ((pkfile = fopen(pkname, "r")) == NULL) {
		      fprintf(stderr, " Can't open pk file %s!\n", pkname) ;
		      exit(1);
		  }
		  fnname = pkname;
		  for (s = pkname; *s; s++) {
		      if (*s == '/') fnname = s+1;
		      else if (*s == '.') *s = '\0';
		  }
		  if (!familyname) familyname=fnname;
		  if (!fullname) fullname=fnname;
	      } else {
		  if ((bdffile = fopen(gargv[i], "w")) == NULL) {
		      fprintf(stderr, " Can't open bdf file %s!\n", gargv[2]) ;
		      exit(1);
		  }
	      }
	      pkfilename++;
	  }
      }
      i++;
  }
}

static void add_dist(dist,size,nr)
     integer *dist;
     integer size;
     integer nr;
{
    integer i;
    i=size;
    while (i && dist[i-1]>nr) {
	dist[i]=dist[i-1];
	i--;
    }
    dist[i]=nr;
}

int main(argc, argv)
     int argc ;
     char *argv[] ;
{
  integer endofpacket ;
  integer designsize ;
  integer checksum ;
  integer hppp, vppp ;
  integer cheight, cwidth, bwidth, hoffset, voffset;
  char *bitmap, *row=0;
  integer rowwidth=0;
  integer packetlength ;
  integer rowsleft ;
  boolean turnon ;
  integer hbit ;
  integer count ;
  integer rp ;
  integer i, j;
  integer car ;
  integer charsinfile, bbx, bby, bbw, bbh, ascent, descent, defaultch ;
  float hdpi, vdpi, ptsize;
  integer avgwidth ;
  integer avgcwidth ;
  integer avglwidth ;
  

  initialize() ;
  charsinfile = avgwidth = bbw = bbh = ascent = descent = defaultch =
      avgcwidth = avglwidth = 0;
  bbx = bby = 1000000; /* a very large number */
  dialog(argc, argv) ;
  if (pkbyte() != 247) {
    fprintf(stderr, " Bad pk file (pre command missing)!\n") ;
    jumpout() ;
  }
  if (pkbyte() != 89) {
    fprintf(stderr, " Wrong version of packed file!\n") ;
    jumpout() ;
  }
  j = pkbyte();
  comments[nr_comments] = (char*) malloc(sizeof(char)*(j+1));
  comments[nr_comments][j] = '\0' ;
  for (i=0 ; i<j ; i ++)
     comments[nr_comments][i] = pkbyte();
  nr_comments++;
  ptsize = (float)(designsize = get32())/(1<<20) ;
  checksum = get32() ;
  hdpi = (hppp = get32())*72.27/(1<<16)+0.5 ;
  vdpi = (vppp = get32())*72.27/(1<<16)+0.5 ;
  if (hppp != vppp) fprintf(stderr, " Warning: aspect ratio not 1:1!\n") ;
  skipspecials() ;
  while (flagbyte != 245) {
    dynf = (flagbyte>>4) ;
    flagbyte &= 15 ;
    turnon = (flagbyte >= 8) ;
    if (turnon) flagbyte &= 7 ;
    if (flagbyte == 7) { /* long format character descriptor */
      packetlength = get32() ;
      car = get32() ;
      endofpacket = packetlength + pkloc ;
      if ((car >= MAXPKCHAR) || (car < 0)) goto lab9997 ;
      pkchar[car].tfmwidth = get32() ;
      pkchar[car].hesc = get32() ;
      pkchar[car].vesc = get32() ;
      pkchar[car].width = cwidth = get32() ;
      pkchar[car].height = cheight = get32() ;
      if ((cwidth < 0) || (cheight < 0) || (cwidth > 65535) || (cheight > 65535)) goto lab9997 ;
      pkchar[car].hoffset = hoffset = get32() ;
      pkchar[car].voffset = voffset = get32() ;
    } else if (flagbyte > 3) { /* extended format character descriptor */
      packetlength =((flagbyte - 4)<<16) + get16() ;
      car = pkbyte() ;
      endofpacket = packetlength + pkloc ;
      if (car >= MAXPKCHAR) goto lab9997 ;
      pkchar[car].tfmwidth = pkbyte()<<16 ; /* tfmwidth */
      pkchar[car].tfmwidth |= get16() ;
      pkchar[car].hesc = get16() << 16 ;
      pkchar[car].vesc = 0;
      pkchar[car].width = cwidth = get16() ;
      pkchar[car].height = cheight = get16() ;
      if ((pkchar[car].hoffset = hoffset = get16()) >= 32768)
	 pkchar[car].hoffset = (hoffset -= 65536);
      if ((pkchar[car].voffset = voffset = get16()) >= 32768)
	 pkchar[car].voffset = (voffset -= 65536);
    } else { /* short format character descriptor */
      packetlength = (flagbyte<<8) + pkbyte() ;
      car = pkbyte() ;
      endofpacket = packetlength + pkloc ;
      if (car >= MAXPKCHAR) goto lab9997 ;
      pkchar[car].tfmwidth = pkbyte() << 16 ; /* tfmwidth */
      pkchar[car].tfmwidth |= get16() ;
      pkchar[car].hesc = pkbyte() << 16;
      pkchar[car].vesc = 0;
      pkchar[car].width = cwidth = pkbyte() ;
      pkchar[car].height = cheight = pkbyte() ;
      if ((pkchar[car].hoffset = hoffset = pkbyte()) >= 128)
	 pkchar[car].hoffset = (hoffset -= 256);
      if ((pkchar[car].voffset = voffset = pkbyte()) >= 128)
	 pkchar[car].voffset = (voffset -= 256);
    }
    bwidth = (cwidth+7) >> 3;
    if ((pkchar[car].bitmap = bitmap = calloc(cheight, bwidth)) == NULL) {
       fprintf(stderr, " Out of memory allocating bitmap!\n") ;
       jumpout() ;
    }
    if (rowwidth<bwidth) {
	if (row) free(row);
	if (!(row = calloc(1,bwidth))) {
	    fprintf(stderr, " Out of memory allocating row!\n") ;
	    jumpout() ;
	}
    } else
	for (i=0; i<bwidth; i++) row[i]=0;
    bitweight = 0 ;
    if (dynf == 14) {
      for (i = 0 ; i < cheight ; i ++) {
	 for (j = 0 ; j < cwidth ; j ++)
	    if (getbit())
	       bitmap[i*bwidth+(j>>3)] |= bit(j);
      }
    } else {
      rowsleft = cheight ;
      hbit = cwidth ;
      repeatcount = rp =0 ;
      while (rowsleft > 0) {
	count = pkpackednum() ;
	while (count > 0) {
	  if (count < hbit) {
	    hbit -= count ;
	    while (count--) {
	      if (turnon)
		row[rp >> 3] |= bit(rp);
	      else
		row[rp >> 3] &= ~bit(rp);
	      rp++;
	    }
	  } else {
	    count -= hbit ;
	    while (hbit--) {
	      if (turnon)
		row[rp >> 3] |= bit(rp);
	      else
		row[rp >> 3] &= ~bit(rp);
	      rp++;
	    }
	    for (i = 0; i <= repeatcount; i++) {
	      int roff = bwidth*(cheight+i-rowsleft);
	      for (j = 0; j < bwidth; j++)
		bitmap[roff+j] = row[j] ;
	    }
	    rowsleft -= repeatcount + 1;
	    repeatcount = rp = 0 ;
	    hbit = cwidth ;
	  }
	}
	turnon = ! turnon ;
      }
      if ((rowsleft != 0) || (hbit != cwidth)) {
	fprintf(stderr, " Bad pk file (more bits than required)!\n") ;
	jumpout() ;
      }
    }
    if (endofpacket != pkloc) {
      fprintf(stderr, " Bad pk file (bad packet length)!\n") ;
      jumpout() ;
    }
    /* set font bounding box */
    if (-hoffset < bbx) bbx = -hoffset;
    if (voffset+1-cheight < bby) bby = voffset+1-cheight;
    if (cwidth > bbw) bbw = cwidth;
    if (voffset+1 > bbh) bbh = voffset+1;
    /* set default character */
    if (!defaultch) defaultch = car;
    /* set ascent and descent */
    i = cheight-1-voffset;
    if (i > descent) descent = i;
    add_dist(descent_dist, charsinfile, i);
    if (voffset+1 >ascent) ascent = voffset+1;
    add_dist(ascent_dist, charsinfile, voffset+1);
    /* add to average */
    avgwidth += cwidth * 10;
    if (car>='A' && car<='Z') avgcwidth += cwidth*10;
    if (car>='a' && car<='z') avglwidth += cwidth*10;
    charsinfile++;
    goto lab9998 ;
  lab9997: while (pkloc != endofpacket) i = pkbyte() ;
    if (car < 0 || car >= MAXPKCHAR)
      fprintf(stderr, " Character %d out of range!\n", car) ;
  lab9998: skipspecials() ;
  }
  while (! feof(pkfile)) i = pkbyte() ;
  pkloc-- ;
  fprintf(stderr, "%d bytes read from packed file.\n", pkloc) ;
  fclose(pkfile);
  fprintf(bdffile, "STARTFONT 2.1\n");
  for (i=0; i<nr_comments; i++)
      fprintf(bdffile, "COMMENT %s\n", comments[i]);
  fprintf(bdffile, "COMMENT Checksum %x\n", checksum);
  fprintf(bdffile, "COMMENT \n");
  fprintf(bdffile, "COMMENT This file is made by pktobdf using the above"
	           " specified pk file.\n");
  fprintf(bdffile, "COMMENT \n");
/* example XLFD
   -bitstream-courier-medium-r-normal--10-100-75-75-m-70--1
   -fndry-fmly-wgt-slant-sWid-adstyl-pxlsz-ptSz-resx-resy-spc-avgWid-rgstry-enc
*/
  i = (round(hdpi*ptsize+resolution/2)/resolution)*10;
  if (pixelsize<=0) pixelsize=(i*resolution*10+3613)/7227;
  if (pointsize<=0) pointsize=i;
  if (averagewidth<=0) averagewidth=(integer)(avgwidth/charsinfile);
  if (avgcapitalwidth<=0) avgcapitalwidth=(integer)(avgcwidth/charsinfile);
  if (avglowercasewidth<=0) avglowercasewidth=(integer)(avglwidth/charsinfile);

  fprintf(bdffile, "FONT -%s-%s-%s-%s-%s-%s-%d-%d-%d-%d-%s-%d-%s-%s\n",
	  foundry, familyname, weightname, slant, setwidthname, addstylename,
	  pixelsize, pointsize, resolution, resolution, spacing, averagewidth,
	  charsetregistry, charsetencoding);
  fprintf(bdffile, "SIZE %d %d %d\n", pixelsize, resolution, resolution);
  fprintf(bdffile, "FONTBOUNDINGBOX %d %d %d %d\n", bbw-bbx, bbh-bby, bbx, bby);
  if (defaultchar<-1)
      fprintf(bdffile, "STARTPROPERTIES 20\n");
  else {
      fprintf(bdffile, "STARTPROPERTIES 21\n");
      if (defaultchar==-1)
	  fprintf(bdffile, "DEFAULT_CHAR %d\n",defaultch);
      else
	  fprintf(bdffile, "DEFAULT_CHAR %d\n",defaultchar);
  }
  if (percent<0) percent=100;
  else if (percent>100) percent=100;
  j = (charsinfile*percent+50)/100 -1;
  if (j<0) j=0;
  if (ascent_dist[j]<descent_dist[j]) {
      fprintf(bdffile,"FONT_ASCENT %d\n", (ascent_dist[j]+descent_dist[j])/2);
      fprintf(bdffile,"FONT_DESCENT %d\n",(ascent_dist[j]+descent_dist[j])/2);
  } else {
      fprintf(bdffile,"FONT_ASCENT %d\n", ascent_dist[j]);
      fprintf(bdffile,"FONT_DESCENT %d\n",descent_dist[j]);
  }
  fprintf(bdffile, "FOUNDRY \"%s\"\n", foundry);
  fprintf(bdffile, "FAMILY_NAME \"%s\"\n", familyname);
  fprintf(bdffile, "WEIGHT_NAME \"%s\"\n", weightname);
  fprintf(bdffile, "SLANT \"%s\"\n", slant);
  fprintf(bdffile, "SETWIDTH_NAME \"%s\"\n", setwidthname);
  fprintf(bdffile, "ADD_STYLE_NAME \"%s\"\n", addstylename);
  fprintf(bdffile, "PIXEL_SIZE %d\n", pixelsize);
  fprintf(bdffile, "POINT_SIZE %d\n", pointsize);
  fprintf(bdffile, "RESOLUTION_X %d\n", resolution);
  fprintf(bdffile, "RESOLUTION_Y %d\n", resolution);
  fprintf(bdffile, "SPACING \"%s\"\n", spacing);
  fprintf(bdffile, "AVERAGE_WIDTH %d\n", averagewidth);
  fprintf(bdffile, "CHARSET_REGISTRY \"%s\"\n", charsetregistry);
  fprintf(bdffile, "CHARSET_ENCODING \"%s\"\n", charsetencoding);
  fprintf(bdffile, "AVG_CAPITAL_WIDTH %d\n", avgcapitalwidth);
  fprintf(bdffile, "AVG_LOWERCASE_WIDTH %d\n", avglowercasewidth);
  fprintf(bdffile, "COPYRIGHT \"%s\"\n", copyright);
  fprintf(bdffile, "FULLNAME \"%s\"\n", fullname);
  fprintf(bdffile, "ENDPROPERTIES\n");
  fprintf(bdffile, "CHARS %d\n", charsinfile);
  for (car = 0; car < MAXPKCHAR; car++)
    if (pkchar[car].bitmap) {
       integer dwidth;
       if (texenc[car])
	   fprintf(bdffile, "STARTCHAR %s\n", texenc[car]);
       else
	   fprintf(bdffile, "STARTCHAR C%.4x\n", car);
       fprintf(bdffile, "ENCODING %d\n", car);
       dwidth=pkchar[car].hesc>>16;
       if (checkhesc) {
	   integer tfmtoesc;
	   /*
	   ** hesc is not always valid: msbm gives problems. To check whether
	   ** to use hesc or tfmwidth depends on the difference between
	   ** tfmwidth*hdpi*ptsize*7/8000 and hesc
	   ** ( the formula is just a guess by trail and error)
	   */
	   tfmtoesc = (pkchar[car].tfmwidth*hdpi*ptsize*7+262144000)/524288000;
	   if (dwidth-1 > tfmtoesc || tfmtoesc > dwidth+2) dwidth=tfmtoesc;
       }
       fprintf(bdffile, "SWIDTH %d %d\n",
	       (integer)(dwidth*72000/(hdpi*ptsize)),
	       0);
       fprintf(bdffile, "DWIDTH %d %d\n", dwidth, 0);
       fprintf(bdffile, "BBX %d %d %d %d\n", pkchar[car].width,
	       pkchar[car].height, -pkchar[car].hoffset,
	       pkchar[car].voffset+1-pkchar[car].height);
       fprintf(bdffile, "BITMAP\n");
       bwidth = (pkchar[car].width+7) >> 3;
       for (i = 0; i < pkchar[car].height; i++) {
	  int roff = bwidth*i;
	  for (j = 0; j < bwidth; j++) {
	     fputc(hex(pkchar[car].bitmap[roff+j] >> 4), bdffile);
	     fputc(hex(pkchar[car].bitmap[roff+j]), bdffile);
	  }
	  fputc('\n', bdffile);
       }
       fprintf(bdffile, "ENDCHAR\n");
    }
  fprintf(bdffile, "ENDFONT\n");
  fclose(bdffile);
  exit(0);
}
