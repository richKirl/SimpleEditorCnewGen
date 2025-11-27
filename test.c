#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xft/Xft.h>
#include <X11/Xlocale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <locale.h>
#include <stdbool.h>

size_t cLine = 0;
size_t minLine=0,maxLine=0;
size_t cPos = 0;
size_t minPos=0,maxPos=0;

typedef struct {
  char* data;
  size_t len;
  size_t cap;
} String;

typedef struct {
  String** string;  // массив указателей на строки
  size_t line;      // текущий индекс/количество строк
  size_t cap;       // емкость массива строк
} Bufer;

// Инициализация строки
void stringInit(String *s) {
  s->cap = 40; // начальная емкость
  s->data = malloc(s->cap * sizeof(char));
  if (!s->data) {
    perror("malloc");
    exit(1);
  }
  s->len = 0; // пустая строка
  s->data[0] = '\0'; // терминатор
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
  s->len += c_len;
  return c_len;
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
  b->string = malloc(b->cap * sizeof(String*));
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


typedef struct {
  
}cCursor;


int main(int argc,char **argv) {
  setlocale(LC_ALL, "");

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
  Window win = XCreateSimpleWindow(dpy, RootWindow(dpy, screen),
				   10, 10, 800, 600, 1,
				   WhitePixel(dpy, screen),
				   BlackPixel(dpy, screen));

  Atom wm_delete_window = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(dpy, win, &wm_delete_window, 1);
  XSelectInput(dpy, win, ExposureMask | KeyPressMask | StructureNotifyMask);
  XMapWindow(dpy, win);
  // Создаем графический контекст
  GC gc = XCreateGC(dpy, win, 0, NULL);
  XftFont *font = XftFontOpenName(dpy, screen, "Ubuntu Mono");
  if (!font) {
    fprintf(stderr, "Не удалось загрузить шрифт\n");
    XCloseDisplay(dpy);
    return 1;
  }

  XftColor color;
  XftColorAllocName(dpy, DefaultVisual(dpy, screen), DefaultColormap(dpy, screen), "white", &color);

  XIM im = XOpenIM(dpy, NULL, NULL, NULL);
  if (!im) {
    fprintf(stderr, "Не удалось открыть XIM\n");
    XftFontClose(dpy, font);
    XCloseDisplay(dpy);
    return 1;
  }

  XIC xic = XCreateIC(
		      im,
		      XNInputStyle, XIMPreeditNothing | XIMStatusNothing,
		      XNClientWindow, win,
		      NULL
		      );
  if (!xic) {
    fprintf(stderr, "Не удалось создать XIC\n");
    XftFontClose(dpy, font);
    XCloseDisplay(dpy);
    return 1;
  }
  Bufer buf;
  BuferInit(&buf, 10);
  BufAddString(&buf, 0);
  // Инициализация начального текста
  //stringAdds(buf.string[0], 0, "Hello");
  //stringAdds(buf.string[0], 5, " World");
  //cPos = 11; // позиция после "Hello World"
  int running = 1;
  // Устанавливаем цвет красного
  XSetForeground(dpy, gc, 0xff0000);



  /////////////////////////////////////////////////////////////////////////////
    FILE *sFile=0;
    sFile=fopen("/home/topa/SimpleEditorC/main.c","rb");

    // Check if the file was opened successfully
    if (sFile == NULL) {
      perror("Error opening file");
      return 1; // Indicate an error
    }

    printf("Contents of %s:\n", "test.c");
    int character;
    // Read and print characters until the end of the file (EOF)
    while ((character = fgetc(sFile)) != EOF) {
      //putchar(character);
      if(character=='\n'){
	cLine++;
	maxLine++;
	cPos=0;
	BufAddString(&buf, cLine);
      }
      else {
	size_t inserted_len = stringAdds(buf.string[cLine], (int)cPos, (char*)&character);
	cPos += inserted_len; // обновляем позицию
      }
    }


    fclose(sFile);
    ////////////////////////////////////////////////////////////////////////////
  
    cLine=0;
    cPos=0;

    while (running) {

      XEvent event;
      XNextEvent(dpy, &event);
      switch (event.type) {
      case ClientMessage:
	if ((Atom)event.xclient.data.l[0] == wm_delete_window) {
	  running = 0;
	}
	break;
      case Expose:
	{
	  XClearWindow(dpy, win);
	  XftDraw *draw = XftDrawCreate(dpy, win,
					DefaultVisual(dpy, screen),
					DefaultColormap(dpy, screen));
	  if (draw) {
	    size_t i = 0;
	    for (; i < 40; ++i) {
	      XftDrawStringUtf8(draw, &color, font, 10, (i + 1) * 20, (const FcChar8*)buf.string[i]->data, buf.string[i]->len);
	    }
	    XftDrawDestroy(draw);
	    int cursor_x = 10 + cPos * 8;
	    int cursor_y = (cLine + 1) * 20 - 15;
	    XSetForeground(dpy, gc, 0x00AA00); // черный цвет
	    XFillRectangle(dpy, win, gc, cursor_x, cursor_y, 8, 16);
	  }
	  break;
	}
      case KeyPress:
	{
	  char buff[128];
	  KeySym keysym;
	  int len = Xutf8LookupString(xic, &event.xkey, buff, sizeof(buff), &keysym, NULL);
	  if(keysym == XK_Left){
	    if(cPos){
	      cPos--;
	    }
	    else if(cPos==0&&cLine!=0){	    
	      cLine--;
	      cPos=buf.string[cLine]->len;
	    }
	    //printf("LEFT\n");
	    XClearArea(dpy, win,10,(cLine)*20+3,800,20,False);
	    XftDraw *draw = XftDrawCreate(dpy, win,
					  DefaultVisual(dpy, screen),
					  DefaultColormap(dpy, screen));
	    if(draw){
	      XftDrawStringUtf8(draw, &color, font, 10, (cLine+1) * 20, (const FcChar8*)buf.string[cLine]->data, buf.string[cLine]->len);
	    }
	    XftDrawDestroy(draw);
	    int cursor_x = 10 + cPos * 8;
	    int cursor_y = (cLine + 1) * 20 - 15;
	    XSetForeground(dpy, gc, 0x00AA00); // черный цвет
	    XFillRectangle(dpy, win, gc, cursor_x, cursor_y, 8, 16);
	  }
	  else if(keysym == XK_Right){
	    bool newLine=false;
	    if (cLine < buf.line) {
	      if (cPos < buf.string[cLine]->len) {
		cPos++;
	      } else if (cLine < buf.line - 1) {
		cLine++;
		cPos = 0;
		newLine=true;
	      }
	      // иначе, курсор уже в конце текста, ничего не делаем
	    }

	    //printf("LEFT\n");
	    //printf("%lu %lu %lu\n",cPos,cLine,buf.line);
	    if(newLine){
	      XClearArea(dpy, win,10+buf.string[cLine-1]->len*8,(cLine-1)*20+3,800,20,False);
	    }
	    XClearArea(dpy, win,10,(cLine)*20+3,800,20,False);
	    XftDraw *draw = XftDrawCreate(dpy, win,
					  DefaultVisual(dpy, screen),
					  DefaultColormap(dpy, screen));
	    if(draw){
	      XftDrawStringUtf8(draw, &color, font, 10, (cLine+1) * 20, (const FcChar8*)buf.string[cLine]->data, buf.string[cLine]->len);
	    }
	    XftDrawDestroy(draw);
	    int cursor_x = 10 + cPos * 8;
	    int cursor_y = (cLine + 1) * 20 - 15;
	    XSetForeground(dpy, gc, 0x00AA00); // черный цвет
	    XFillRectangle(dpy, win, gc, cursor_x, cursor_y, 8, 16);
	    //printf("Right\n");
	  }
	  else if(keysym == XK_Up){
	    //printf("Up\n");
	  }
	  else if(keysym == XK_Down){
	    //printf("Down\n");
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
	      int cursor_x = 10 + cPos * 8;
	      int cursor_y = (cLine + 1) * 20 - 15;
	      XSetForeground(dpy, gc, 0x00AA00); // черный цвет
	      XFillRectangle(dpy, win, gc, cursor_x, cursor_y, 8, 16);
	      XftDraw *draw = XftDrawCreate(dpy, win,
					    DefaultVisual(dpy, screen),
					    DefaultColormap(dpy, screen));
	      if (draw) {
		size_t i = 0;
		for (; i < cLine; ++i) {
		  XftDrawStringUtf8(draw, &color, font, 10, (i + 1) * 20, (const FcChar8*)buf.string[i]->data, buf.string[i]->len);
		}
		XftDrawDestroy(draw);
	      }
	    }
	    else if (keysym == XK_q && (event.xkey.state & ControlMask)) {
	      // Ctrl+Q — выход
	      running = 0;
	    }

	    else {
	      //printf("%zu\n",maxLine-cLine);
	      // Ввод текста
	      size_t inserted_len = stringAdds(buf.string[cLine], (int)cPos, buff);
	      cPos += inserted_len; // обновляем позицию
	      size_t tempN = cLine+maxLine;
	      size_t tempM = maxLine-cLine;
	      if(tempM==0){
		XClearArea(dpy, win,10,(cLine)*20+3,800,600,False);
		int cursor_x = 10 + cPos * 8;
		int cursor_y = (cLine + 1) * 20 - 15;
		XSetForeground(dpy, gc, 0x00AA00); // черный цвет
		XFillRectangle(dpy, win, gc, cursor_x, cursor_y, 8, 16);
		XftDraw *draw = XftDrawCreate(dpy, win,
					      DefaultVisual(dpy, screen),
					      DefaultColormap(dpy, screen));
		if (draw) {
		  XftDrawStringUtf8(draw, &color, font, 10, (cLine+1) * 20, (const FcChar8*)buf.string[cLine]->data, buf.string[cLine]->len);
		  XftDrawDestroy(draw);

		}
	      }
	      else if(tempM==1){
		size_t i=cLine;
		for(;i<maxLine+1;i++){
		  XClearArea(dpy, win,10,(i)*20+3,800,600,False);
		  int cursor_x = 10 + cPos * 8;
		  int cursor_y = (cLine + 1) * 20 - 15;
		  XSetForeground(dpy, gc, 0x00AA00); // черный цвет
		  XFillRectangle(dpy, win, gc, cursor_x, cursor_y, 8, 16);
		}
		XftDraw *draw = XftDrawCreate(dpy, win,
					      DefaultVisual(dpy, screen),
					      DefaultColormap(dpy, screen));
	    
		if (draw) {
		  size_t i=cLine;
		  for(;i<maxLine+1;i++){
		    XftDrawStringUtf8(draw, &color, font, 10, (i+1) * 20, (const FcChar8*)buf.string[i]->data, buf.string[i]->len);
		  }
		  XftDrawDestroy(draw);

		}
	      }
	      else if(tempM>1){
		size_t i=cLine;
		for(;i<maxLine+1;i++){
		  XClearArea(dpy, win,10,(i)*20+3,800,600,False);
		}
		int cursor_x = 10 + cPos * 8;
		int cursor_y = (cLine + 1) * 20 - 15;
		XSetForeground(dpy, gc, 0x00AA00); // черный цвет
		XFillRectangle(dpy, win, gc, cursor_x, cursor_y, 8, 16);
		XftDraw *draw = XftDrawCreate(dpy, win,
					      DefaultVisual(dpy, screen),
					      DefaultColormap(dpy, screen));
	    
		if (draw) {
		  size_t i=cLine;
		  for(;i<maxLine+1;i++){
		    XftDrawStringUtf8(draw, &color, font, 10, (i+1) * 20, (const FcChar8*)buf.string[i]->data, buf.string[i]->len);
		  }
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
