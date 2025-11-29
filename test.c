#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xft/Xft.h>
#include <X11/Xlocale.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <locale.h>
#include <stdbool.h>
//🙂
size_t cLine = 0;
size_t minLine=0,maxLine=0;
size_t cPos = 0;
size_t minPos=0,maxPos=0;
size_t topLine = 0;


typedef struct {
  char* data;
  size_t len;
  size_t rLen;
  size_t cap;
} String;



typedef struct {
  String** string;  // массив указателей на строки
  size_t line;      // текущий индекс/количество строк
  size_t cap;       // емкость массива строк
  size_t sz;
  size_t rsz;
} Bufer;

// Инициализация строки
void stringInit(String *s) {
  s->cap = 40; // начальная емкость
  s->data = malloc(s->cap * sizeof(char));
  if (!s->data) {
    perror("malloc");
    exit(1);
  }
  s->rLen=0;
  s->len = 0; // пустая строка
  s->data[0] = '\0'; // терминатор
}

size_t utf8_strlen(const char *s) {
  size_t count = 0;
  while (*s) {
    unsigned char c = (unsigned char)*s;
    if (c <= 0x7F) {
      s += 1;
    } else if ((c & 0xE0) == 0xC0) {
      s += 2;
    } else if ((c & 0xF0) == 0xE0) {
      s += 3;
    } else if ((c & 0xF8) == 0xF0) {
      s += 4;
    } else {
      // некорректный байт, можно обработать ошибку
      s += 1;
    }
    count++;
  }
  return count;
}

// Вставка строки c на позицию p, возвращает длину вставленного текста
size_t stringAdds(String *s, int p, const char* c) {
  if (p < 0 || p > (int)s->len) {
    printf("T");
    return 0;
  }

  size_t c_len = strlen(c);
  // Расширение буфера
  if (s->len + c_len + 1 > s->cap) {
    while (s->len + c_len + 1 > s->cap) {
      s->cap += 40;
    }
    char *newS = realloc(s->data, s->cap * sizeof(char));
    if (!newS) {
      perror("realloc");
      exit(1);
    }
    s->data = newS;
    //s->data[s->len + c_len]='\0';
  }

  memmove(s->data + p + c_len, s->data + p, s->len - p + 1);
  memcpy(s->data + p, c, c_len);
  s->rLen++;
  s->len += c_len;
  return c_len;
}

size_t stringAdds1(String *s, int p, const char* c) {
  if (p < 0 || p > (int)s->len) {
    printf("T");
    return 0;
  }

  size_t c_byte_len = strlen(c);
  size_t c_char_len = utf8_strlen(c);

  // Расширение буфера
  if (s->len + c_byte_len + 1 > s->cap) {
    while (s->len + c_byte_len + 1 > s->cap) {
      s->cap += 40;
    }
    char *newS = realloc(s->data, s->cap * sizeof(char));
    if (!newS) {
      perror("realloc");
      exit(1);
    }
    s->data = newS;
  }

  memmove(s->data + p + c_byte_len, s->data + p, s->len - p + 1);
  memcpy(s->data + p, c, c_byte_len);
  s->len += c_byte_len;

  return c_char_len; // возвращаем длину в символах
}

// Печать строки
void stringPrint(const String *s) {
  printf("%s\n", s->data);
}



// Освобождение строки
void stringFree(String *s) {
  free(s->data);
  s->data = NULL;
  s->len = 0;
  s->cap = 0;
}



// Инициализация буфера
void BuferInit(Bufer *b, size_t initial_capacity) {
  b->cap = initial_capacity;
  b->line = 0;
  b->string = malloc(b->cap * sizeof(String *));
  b->sz=0;
  if (!b->string) {
    perror("malloc");
    exit(1);
  }
  size_t i = 0;
  for (; i < b->cap; ++i) {
    b->string[i] = NULL;
  }
}

// Добавление новой строки
void BufAddString(Bufer *buf, int l) {
  if (l >= buf->cap) {
    size_t new_cap = buf->cap + 10;
    String** new_array = realloc(buf->string, new_cap * sizeof(String*));
    if (!new_array) {
      perror("realloc");
      exit(1);
    }
    buf->string = new_array;
    size_t i = buf->cap;
    for (; i < new_cap; ++i) {
      buf->string[i] = NULL;
    }
    buf->cap = new_cap;
  }
  if (buf->string[l] == NULL) {
    buf->string[l] = malloc(sizeof(String));
    stringInit(buf->string[l]);
    buf->line++;
    buf->sz+=buf->line*buf->string[l]->len;
  }
}

// Печать всех строк
void BuferPrint(Bufer *buf) {
  size_t i = 0;
  for (; i < buf->line; ++i) {
    stringPrint(buf->string[i]);
  }
}

// Освобождение буфера
void BuferFree(Bufer *b) {
  size_t i = 0;
  for (; i < b->line; ++i) {
    if (b->string[i]) {
      stringFree(b->string[i]);
      free(b->string[i]);
    }
  }
  free(b->string);
  b->string = NULL;
  b->line = 0;
  b->cap = 0;
}

void drawTextB(size_t start,size_t maxVisibleLines,XftDraw *draw,XftColor color,XftFont *font,Bufer *buf){
  size_t i = start;
  for (; i < maxVisibleLines; ++i) {
    size_t bufLineIndex = topLine + i;
    XftDrawStringUtf8(draw,&color,font,10,(i + 1) * 20,(const FcChar8*)buf->string[bufLineIndex]->data,buf->string[bufLineIndex]->len);
  }
}

typedef struct {

}cCursor;

void drawcCursor(Display *dpy,Window win,GC gc){
  int cursor_x = 10 + cPos * 8;
  int cursor_y = (cLine + 1) * 20 - 15;
  XSetForeground(dpy, gc, 0x00AA00); // черный цвет
  XFillRectangle(dpy,win,gc,cursor_x,cursor_y,8,16);
}



typedef struct {

}cPanel;

//topPanel
void drawTPanel(Display *dpy,GC gc,Window win,int pf_x,int pf_y,int xW,int h,unsigned int col){
  XSetForeground(dpy, gc, col); // черный цвет0x00AA00
  XFillRectangle(dpy, win, gc, pf_x, pf_y, xW, h);
}

//BottomPanel
void drawBPanel(Display *dpy,GC gc,Window win,int pf_x,int pf_y,int xW,int h,unsigned int col,XftDraw *draw,XftColor color,XftFont *font,Bufer *buf){
  char bufPanel[512];
  int p_x = 0;
  int p_y = pf_y-h;
  XSetForeground(dpy, gc, col); // черный цвет
  XFillRectangle(dpy, win, gc, p_x, p_y, xW, h);//16
  size_t lx=sprintf(bufPanel,"NameFile: test.c Totality Bytes: %zu Char Nums: %zu",buf->sz,buf->rsz);
  XftDrawStringUtf8(draw,&color,font,p_x+5,p_y+12,(const FcChar8 *)bufPanel,strlen(bufPanel));
}

//получение размера окна
void getSizeWindow(Display *dpy,Window win,int *x,int *y){
  XWindowAttributes attr;
  XGetWindowAttributes(dpy, win, &attr);
  *x=attr.width;
  *y=attr.height;
}

size_t utf8_char_size(const unsigned char first_byte) {
  if (first_byte <= 0x7F) {
    return 1;
  } else if ((first_byte & 0xE0) == 0xC0) {
    return 2;
  } else if ((first_byte & 0xF0) == 0xE0) {
    return 3;
  } else if ((first_byte & 0xF8) == 0xF0) {
    return 4;
  } else {
    // некорректный байт или продолжительный байт без начала символа
    return 1; // или обработка ошибки
  }
}

void openFile(Bufer *buf,const char* filename){
  /////////////////////////////////////////////////////////////////////////////
  FILE *sFile=0;
  sFile=fopen(filename,"rb");

  // Check if the file was opened successfully
  if (sFile == NULL) {
    perror("Error opening file");
    return ; // Indicate an error
  }
  // Перемещаемся в конец файла
  fseek(sFile, 0, SEEK_END);

  // Получаем текущую позицию (размер файла в байтах)
  long file_size = ftell(sFile);

  // Возвращаемся в начало файла
  fseek(sFile, 0, SEEK_SET);

  // Теперь у вас есть размер файла в переменной file_size
  printf("Размер файла: %ld байт\n", file_size);
  printf("Contents of %s:\n", filename);
  int character;
  // Read and print characters until the end of the file (EOF)
  while ((character = fgetc(sFile)) != EOF) {
    if (character == '\n') {
      buf->sz+=sizeof(char)*buf->string[cLine]->len;//пока так
      //buf->sz=buf->line*cLine;
      buf->rsz+=buf->string[cLine]->rLen;
      cLine++;
      maxLine++;
      cPos=0;
      BufAddString(buf, cLine);
    }
    else if(character == '\t'){
      char tempTab[2] ="  ";//подобрано из-за емакса пока временно
      size_t inserted_len = stringAdds(buf->string[cLine],(int)cPos,(char*)&tempTab);     
      cPos += inserted_len;
      continue;
    }
    else {
      unsigned char first_byte = (unsigned char)character;
      size_t symbol_size = utf8_char_size(first_byte);
      //printf("symbol_size: %d\n",symbol_size);
      char symbol_buf[5] = {0}; // максимум 4 байта + нуль-терминатор
      symbol_buf[0] = first_byte;
      if ( symbol_size > 1) {
	symbol_buf[1] = fgetc(sFile);
      }
      size_t inserted_len = stringAdds(buf->string[cLine],(int)cPos,(char*)&symbol_buf);
      cPos += inserted_len; // обновляем позицию
    }
  }
  //buf.sz+=sizeof(char)*strlen(buf.string[cLine]->data);
  printf("%zu\n",buf->sz);

  fclose(sFile);
  ////////////////////////////////////////////////////////////////////////////
}

void stdCHECKER(){
#ifdef __STDC__
  printf("__STDC__ is defined (ANSI C compliant).\n");
#else
  printf("__STDC__ is not defined (not strictly ANSI C compliant).\n");
#endif

#ifdef __STDC_VERSION__
  printf("__STDC_VERSION__: %ld\n", __STDC_VERSION__);
#else
  printf("__STDC_VERSION__ is not defined.\n");
#endif
}

int main(int argc,char **argv) {

  setlocale(LC_ALL, "");

  stdCHECKER();

  Display *dpy = XOpenDisplay(NULL);

  if (!dpy) {
    fprintf(stderr, "Не удалось открыть дисплей\n");
    return 1;
  }

  if (!XSupportsLocale()) {
    fprintf(stderr, "Локаль не поддерживается\n");
    XCloseDisplay(dpy);
    return 1;
  }

  int screen = DefaultScreen(dpy);

  Window win = XCreateSimpleWindow(dpy, RootWindow(dpy, screen),10, 10, 800, 600, 1,WhitePixel(dpy, screen),BlackPixel(dpy, screen));

  Atom wm_delete_window = XInternAtom(dpy, "WM_DELETE_WINDOW", False);

  XSetWMProtocols(dpy, win, &wm_delete_window, 1);

  XSelectInput(dpy, win, ExposureMask | KeyPressMask | StructureNotifyMask);

  XMapWindow(dpy, win);
  // Создаем графический контекст

  GC gc = XCreateGC(dpy, win, 0, NULL);

  //Ubuntu Mono:pixelsize=14Noto Color Emoji
  XftFont *font = XftFontOpenName(dpy,screen,"Ubuntu Mono");

  if (!font) {
    fprintf(stderr, "Не удалось загрузить шрифт\n");
    XCloseDisplay(dpy);
    return 1;
  }

  XftColor color;
  XftColorAllocName(dpy,DefaultVisual(dpy, screen),DefaultColormap(dpy, screen),"white",&color);

  XIM im = XOpenIM(dpy, NULL, NULL, NULL);

  if (!im) {
    fprintf(stderr, "Не удалось открыть XIM\n");
    XftFontClose(dpy, font);
    XCloseDisplay(dpy);
    return 1;
  }

  XIC xic = XCreateIC(im,XNInputStyle, XIMPreeditNothing | XIMStatusNothing,XNClientWindow, win,NULL);

  if (!xic) {
    fprintf(stderr, "Не удалось создать XIC\n");
    XftFontClose(dpy, font);
    XCloseDisplay(dpy);
    return 1;
  }

  Bufer buf;

  BuferInit(&buf, 10);

  BufAddString(&buf, 0);

  int running = 1;
  // Устанавливаем цвет красного
  XSetForeground(dpy, gc, 0xff0000);

  openFile(&buf,"test.c");

  char bufPanel[512];
  cLine=0;
  cPos=0;
  int xW,xY;
  size_t maxVisibleLines = xY / 20;
  while (running) {

    XEvent event;
    XNextEvent(dpy, &event);
    switch (event.type) {
    case ClientMessage:
      if ((Atom)event.xclient.data.l[0] == wm_delete_window) {
	running = 0;
      }
      break;
    case ConfigureNotify:
      // Handle the ConfigureNotify event
      XConfigureEvent* configureEvent = &event.xconfigure;
      xW=configureEvent->width;
      xY=configureEvent->height;
      maxVisibleLines = xY / 20;
      break;
    case Expose:
      {
	XClearWindow(dpy, win);
	XftDraw *draw = XftDrawCreate(dpy,win,DefaultVisual(dpy, screen),DefaultColormap(dpy, screen));
	if (draw) {
	  //size_t  = xY / 20;
          if (topLine + maxVisibleLines > buf.line) {
	    maxVisibleLines = buf.line - topLine;
          }
	  drawTextB(0,maxVisibleLines,draw,color,font,&buf);
	  drawcCursor(dpy,win,gc);
	  //getSize
	  drawTPanel(dpy,gc,win,0,0,xW,20,0x00AA00);//topPanel
	  //panel
	  drawBPanel(dpy,gc,win,0,xY,xW,16,0x00AA00,draw,color,font,&buf);
	  XftDrawDestroy(draw);
	  //endpanel
	}
	break;
      }
    case KeyPress:
      {
	char buff[128];
	KeySym keysym;
	int len = Xutf8LookupString(xic,&event.xkey,buff,sizeof(buff),&keysym, NULL);
	if(keysym == XK_Left){
	  if(cPos){
	    cPos--;
	  }
	  else if(cPos==0&&cLine!=0){
	    cLine--;
	    cPos=buf.string[cLine]->len;
	  }
	  //printf("LEFT\n");
	  XClearArea(dpy, win,10,(cLine)*20+3,xW,20,False);
	  XftDraw *draw = XftDrawCreate(dpy,win,DefaultVisual(dpy, screen),DefaultColormap(dpy, screen));
	  if(draw){
	    XftDrawStringUtf8(draw,&color,font,10,(cLine+1) * 20,(const FcChar8*)buf.string[cLine]->data,buf.string[cLine]->len);
	  }
	  XftDrawDestroy(draw);
	  drawcCursor(dpy,win,gc);
	}
	else if(keysym == XK_Right){
	  bool newLine=false;
	  if (cLine < buf.line) {
	    if (cPos < buf.string[cLine]->rLen) {
	      cPos++;
	    } else if (cLine < buf.line - 1) {
	      cLine++;
	      cPos = 0;
	      newLine=true;
	    }
	    // иначе, курсор уже в конце текста, ничего не делаем
	  }
	  if(newLine){
	    XClearArea(dpy,win,10+buf.string[cLine-1]->len*8,(cLine-1)*20+3,xW,20,False);
	  }
	  XClearArea(dpy,win,10,(cLine)*20+3,xW,20,False);
	  XftDraw *draw = XftDrawCreate(dpy,win,DefaultVisual(dpy, screen),DefaultColormap(dpy, screen));
	  if(draw){
	    XftDrawStringUtf8(draw,&color,font,10,(cLine+1) * 20,(const FcChar8*)buf.string[cLine]->data,buf.string[cLine]->len);
	  }
	  XftDrawDestroy(draw);
	  drawcCursor(dpy,win,gc);
	  //printf("Right\n");
	}
	if (keysym == XK_Up) {
	  cLine--;
	  cPos=0;
	  drawcCursor(dpy,win,gc);
	  /* 	      int cursor_x = 10 + cPos * 8; */
	  /* 	      int cursor_y = (cLine + 1) * 20 - 15; */
	  /* 	      XSetForeground(dpy, gc, 0x00AA00); // черный цвет */
	  /* 	      XFillRectangle(dpy, win, gc, cursor_x, cursor_y, 8, 16); */
	  /* 	  if (cLine > 0) { */
	  /* 	    cLine--; */
	  /* 	    cPos=0; */
	  /* 	    XClearWindow(dpy, win); */
	  /* 	    XftDraw *draw = XftDrawCreate(dpy,win,DefaultVisual(dpy, screen),DefaultColormap(dpy, screen)); */
	  /* 	    if (draw) { */
	  /* 	      size_t linesOnScreen = xY / 20; */
	  /* 	      if (cLine + linesOnScreen > buf.line) { */
	  /* 		linesOnScreen = cLine - topLine; */
	  /* 	      } */
	  /* 	      size_t i = 0; */
	  /* 	      for (; i < linesOnScreen; ++i) { */
	  /* 		size_t bufLineIndex = cLine + i; */
	  /* 		XftDrawStringUtf8(draw,&color,font,10,(i + 1) * 20,(const FcChar8*)buf.string[bufLineIndex]->data,buf.string[bufLineIndex]->len); */
	  /* 	      } */
	  /* 	      XftDrawDestroy(draw); */
	  /* 	      int cursor_x = 10 + cPos * 8; */
	  /* 	      int cursor_y = (cLine + 1) * 20 - 15; */
	  /* 	      XSetForeground(dpy, gc, 0x00AA00); // черный цвет */
	  /* 	      XFillRectangle(dpy, win, gc, cursor_x, cursor_y, 8, 16); */
	  /* 	      //getSize */
	  /* 	      printf("%d %d\n",xW,xY); */
	  /* 	      // filePanel */
	  /* 	      int pf_x = 0; */
	  /* 	      int pf_y = 0; */
	  /* 	      XSetForeground(dpy, gc, 0x00AA00); // черный цвет */
	  /* 	      XFillRectangle(dpy, win, gc, pf_x, pf_y, xW, 20); */
	  /* 	      //panel */
	  /* 	      int p_x = 0; */
	  /* 	      int p_y = xY-16; */
	  /* 	      XSetForeground(dpy, gc, 0x00AA00); // черный цвет */
	  /* 	      XFillRectangle(dpy, win, gc, p_x, p_y, xW, 16); */
	  /* 	      XftDraw *draw = XftDrawCreate(dpy,win,DefaultVisual(dpy, screen),DefaultColormap(dpy, screen)); */
	  /* 	      size_t lx=sprintf(bufPanel,"NameFile: test.c Chars: %zu",buf.sz); */
	  /* 	      XftDrawStringUtf8(draw,&color,font,p_x+5,p_y+12,(const FcChar8 *)bufPanel,strlen(bufPanel)); */
	  /* 	      //endpanel */
	  /* 	    } */
	  /* 	  } */
	     } else if (keysym == XK_Down) {
	  if (topLine+maxVisibleLines-2 < buf.line) {
	    //printf("NewPage1str++\n");
	    topLine++;
	    //cLine=topLine;
	    XClearWindow(dpy, win);
	    XftDraw *draw = XftDrawCreate(dpy,win,DefaultVisual(dpy, screen),DefaultColormap(dpy, screen));
	    if (draw) {
	      //size_t linesOnScreen = xY / 20;
	      if (topLine + maxVisibleLines > buf.line) {
	    	maxVisibleLines = buf.line - topLine;
	      }
	      drawTextB(0,maxVisibleLines,draw,color,font,&buf);
	      drawcCursor(dpy,win,gc);
	      // filePanel
	      drawTPanel(dpy,gc,win,0,0,xW,20,0x00AA00);//topPanel
	      //panel
	      drawBPanel(dpy,gc,win,0,xY,xW,16,0x00AA00,draw,color,font,&buf);//bottomPanel
	      XftDrawDestroy(draw);
	      //endpanel
	    }
	  }
	  else {
	    cLine=topLine+maxVisibleLines-2;
	    //cLine++;
	    cPos=0;
	    drawcCursor(dpy,win,gc);
	    drawTPanel(dpy,gc,win,0,0,xW,20,0x00AA00);//topPanel
	    //panel
	    XftDraw *draw = XftDrawCreate(dpy,win,DefaultVisual(dpy, screen),DefaultColormap(dpy, screen));
	    drawBPanel(dpy,gc,win,0,xY,xW,16,0x00AA00,draw,color,font,&buf);//bottomPanel
	    XftDrawDestroy(draw);
	  }
	}
	if (len > 0) {
	  if (keysym == XK_Return) {
	    // Создаем новую строку
	    cPos = 0;
	    cLine++;
	    maxLine++;
	    BufAddString(&buf, cLine);
	    // Обновляем отображение
	    XClearWindow(dpy, win);
	    drawcCursor(dpy,win,gc);
	    XftDraw *draw = XftDrawCreate(dpy,win,DefaultVisual(dpy, screen),DefaultColormap(dpy, screen));
	    if (draw) {
	      drawTextB(0,cLine,draw,color,font,&buf);
	      XftDrawDestroy(draw);
	    }
	  }
	  else if (keysym == XK_q && (event.xkey.state & ControlMask)) {
	    // Ctrl+Q — выход
	    running = 0;
	  }
	  else {
	    // Ввод текста
	    size_t inserted_len = stringAdds(buf.string[cLine],(int)cPos,buff);
	    cPos += inserted_len; // обновляем позицию
	    size_t tempN = cLine+maxLine;
	    size_t tempM = maxLine-cLine;
	    if(tempM==0){
	      XClearArea(dpy,win,10,(cLine)*20+3,xW,xY, False);
	      drawcCursor(dpy,win,gc);
	      XftDraw *draw = XftDrawCreate(dpy,win,DefaultVisual(dpy, screen),DefaultColormap(dpy, screen));
	      if (draw) {
		XftDrawStringUtf8(draw,&color,font,10,(cLine+1) * 20,(const FcChar8*)buf.string[cLine]->data,buf.string[cLine]->len);
		XftDrawDestroy(draw);
	      }
	    }
	    else if(tempM==1){
	      size_t i=cLine;
	      for(;i<maxLine+1;i++){
		XClearArea(dpy,win,10,(i)*20+3,xW,xY,False);
		drawcCursor(dpy,win,gc);
	      }
	      XftDraw *draw = XftDrawCreate(dpy,win,DefaultVisual(dpy, screen),DefaultColormap(dpy, screen));
	      if (draw) {
		drawTextB(cLine,maxLine+1,draw,color,font,&buf);
		XftDrawDestroy(draw);
	      }
	    }
	    else if(tempM>1){
	      size_t i=cLine;
	      for(;i<maxLine+1;i++){
		XClearArea(dpy,win,10,(i)*20+3,xW,xY,False);
	      }
	      drawcCursor(dpy,win,gc);
	      XftDraw *draw = XftDrawCreate(dpy,win,DefaultVisual(dpy, screen),DefaultColormap(dpy, screen));
	      if (draw) {
		drawTextB(cLine,maxLine+1,draw,color,font,&buf);
		XftDrawDestroy(draw);
	      }
	    }
	  }
	}
	break;
      }
    default:
      break;
    }
  }
  // Освобождение ресурсов
  XFreeGC(dpy, gc);
  XDestroyIC(xic);
  XftFontClose(dpy, font);
  XDestroyWindow(dpy, win);
  XCloseDisplay(dpy);
  BuferFree(&buf);
  return 0;
}
