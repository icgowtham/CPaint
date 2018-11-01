#include <stdio.h>
#include <graphics.h>
#include <dos.h>
#include <math.h>
#include <io.h>
#include <stdlib.h>
#define  MAX 1024

typedef enum boolean BOOL;

enum boolean { FALSE, TRUE };

union REGS i, o;
struct SREGS s;
int X, Y;
int x, y, _button;
int _thickness;

int CUR_COL  = 0;
int CLICKED  = 0;
int MAX_COL  = 15;
int CANV_COL = WHITE;
int CUR_PAT  = 0;
int CLIP_X1, CLIP_Y1, CLIP_X2, CLIP_Y2;
int cpyheight, cpywidth;
char* file;

struct button
{
	int left;
	int top;
	int right;
	int bottom;
};

struct button canvas, prev, prev1, new, open1, save, cut, copy, paste;
struct button polyg, lin, trans, rot, scale, clip, help, about, arc1;
struct button ellipse1, spiral, box, select, eraser, fill, spray, pen;
struct button brush, rect, text, clear, _fill;

enum choice
{
	CLOSE, NEW, OPEN, SAVE, CUT, COPY, PASTE, TRANS, ROTATE, SCALE,
	CLIP, HELP, ABOUT, SELECT, ERASER, FILL, SPRAY, PEN, BRUSH,
	LINE, CURVE, CANV_FILL, RECT, POLY, ELLIPSE, TEXT, CLEAR, SPIRAL
};
enum choice SELC;

int hand[32] = 		{
				0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
				0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
				0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
				0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,

				0x6000, 0x9000, 0x9000, 0x9000,
				0x9000, 0x9600, 0x9960, 0x9996,
				0x9999, 0x9999, 0x9999, 0xFFFF,
				0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
			};

int pencil[32] = 	{
				0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
				0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
				0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
				0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,

				0x8000, 0x6000, 0x7000, 0x2400,
				0x1E00, 0x1F00, 0x0FC0, 0x07E0,
				0x03F0, 0x03F8, 0x01FC, 0x00E4,
				0x0064, 0x0038, 0x0000, 0x0000,
			};

int pointer[32] = 	{
				0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
				0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
				0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
				0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,

				0x8000, 0xC000, 0xE000, 0xF000,
				0xF800, 0xFC00, 0xFE00, 0xFF80,
				0xFFC0, 0xEC00, 0xC600, 0x8600,
				0x0300, 0x0300, 0x0180, 0x0180,
			};

int hand1[32] = 	{
				0xE1FF, 0xE1FF, 0xE1FF, 0xE1FF,
				0xE1FF, 0x0000, 0x0000, 0x0000,
				0x0000, 0x0000, 0x0000, 0x0000,
				0x0000, 0x0000, 0x0000, 0x0000,

				0x1E00, 0x1200, 0x1200, 0x1200,
				0x13FF, 0x1249, 0x1249, 0xF249,
				0x9001, 0x9001, 0x9001, 0x8001,
				0x8001, 0x8001, 0xFFFF, 0x0000,
			};

int text1[32] = 	{
				0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
				0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
				0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
				0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,

				0xFFFF, 0x0180, 0x0180, 0x0180,
				0x0180, 0x0180, 0x0180, 0x0180,
				0x0180, 0x0180, 0x0180, 0x0180,
				0x0180, 0x0180, 0x0180, 0xFFFF,
			};

char pattern1[8] = { 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x55, 0x55 };
char pattern2[8] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };

void init ()
{
	int gd = DETECT, gm,errorcode;
	initgraph (&gd, &gm, "");
	errorcode = graphresult ();
	if (errorcode != grOk)
	{
		printf ("%s", grapherrormsg (errorcode));
		getch ();
		exit (0);
	}
}

void deInit ()
{
	clearviewport ();
	closegraph ();
	restorecrtmode ();
	if (access ("copyBuf", 00) == 0)
		unlink ("copyBuf");
	if (access ("transBuf", 00) == 0)
		unlink ("transBuf");
	if (access ("temp", 00) == 0)
		unlink ("temp");
	if (access ("tmp", 00) == 0)
		unlink ("tmp");
	exit (0);
}

int initmouse ()
{
	i.x.ax = 0;
	int86 (0x33, &i, &o);
	return (o.x.ax);
}

void showmouseptr ()
{
	i.x.ax = 1;
	int86 (0x33, &i, &o);
}

void hidemouseptr ()
{
	i.x.ax = 2;
	int86 (0x33, &i, &o);
}

void restrictmouseptr ( int x1, int y1, int x2, int y2 )
{
	i.x.ax = 7;
	i.x.cx = x1;
	i.x.dx = x2;
	int86 (0x33, &i, &o);

	i.x.ax = 8;
	i.x.cx = y1;
	i.x.dx = y2;
	int86 (0x33, &i, &o);
}

void getmousepos ( int* button, int* x, int* y )
{
	i.x.ax = 3;
	int86(0x33, &i, &o);
	*button = o.x.bx;
	*x = o.x.cx;
	*y = o.x.dx;
}

void setmousepos ( int x, int y )
{
	i.x.ax = 4;
	i.x.cx = x;
	i.x.dx = y;
	int86 (0x33, &i, &o);
}

void changecursor (int* shape)
{
	i.x.ax = 9;
	i.x.bx = 0;
	i.x.cx = 0;
	i.x.dx = (unsigned) shape;
	segread (&s);
	s.es = s.ds;
	int86x (0x33, &i, &o, &s);
}

void button ( struct button selc, int fg, int bg )
{
	int x1 = selc.left;
	int y1 = selc.top;
	int x2 = selc.right;
	int y2 = selc.bottom;
	hidemouseptr ();
	setcolor (DARKGRAY);
	rectangle ( x1 - 3, y1 - 3, x2 + 6, y2 + 3);
	setcolor (fg);
	setlinestyle (SOLID_LINE, 0, 1);
	line (x1 - 3, y1 - 3, x1 - 3, y1 + 29);
	line (x1 + 30, y1 - 3, x1 - 3, y1 - 3);
	setcolor (bg);
	setlinestyle (SOLID_LINE, 0, 2);
	line (x1 - 3, y1 + 29, x1 + 30, y1 + 29);
	line (x1 + 30, y1 + 29, x1 + 30, y1 - 3);
	showmouseptr ();
}

void button_small ( int x1, int y1, int x2, int y2, int fg, int bg )
{
	hidemouseptr ();
	setcolor (DARKGRAY);
	rectangle ( x1 - 1, y1 - 1, x2 + 1, y2 + 1);
	setcolor (fg);
	setlinestyle(SOLID_LINE, 0, 1);
	line (x1 - 1, y1 - 1, x1 - 1, y1 +13);
	line (x1 + 13, y1 - 1, x1 - 1, y1 - 1);
	setcolor (bg);
	setlinestyle(SOLID_LINE, 0, 2);
	line (x1 - 1, y1 + 13, x1 + 13, y1 + 13);
	line (x1 + 13, y1 + 13, x1 + 13, y1 - 1);
	showmouseptr ();
}

void button_click ( struct button selc, BOOL sp )
{
	if (prev.left != 0)
		button (prev, WHITE, BLACK);

	prev.left   = selc.left;
	prev.top    = selc.top;
	prev.right  = selc.right;
	prev.bottom = selc.bottom;

	if (sp == TRUE)
		button (selc, BLACK, WHITE);
	else
	{
		button (selc, BLACK, WHITE);
		delay (100);
		button (selc, WHITE, BLACK);
	}
}

void cmd_button ( int x, int y, int width, int height,
		  int fg, int bg, char* text )
{
	setfillstyle (SOLID_FILL, LIGHTGRAY);
	bar (x, y, x + width, y + height);
	setcolor (fg);
	line (x - 1, y, x - 1, y + height);
	line (x - 1, y - 1, x + width, y - 1);
	setcolor (bg);
	line (x - 1, y + height, x + width, y + height);
	line (x + width, y + height, x + width, y - 1);
	setlinestyle (SOLID_LINE, 0, NORM_WIDTH);
	settextstyle (DEFAULT_FONT, HORIZ_DIR, 1);

	outtextxy (x + (width / 4), y + (height / 3), text);
}

int cmd_button_click ( int x, int y, int width, int height, char* text )
{
	cmd_button (x, y, width, height, BLACK, WHITE, text);
	delay (100);
	cmd_button (x, y, width, height, WHITE, BLACK, text);

	return 0;
}

int input (char* title, char* prompt)
{
	int size;
	int value;
	char* area = NULL;
	setviewport (1, 1, X, Y, 1);

	size = imagesize (174, 138, 484, 268);
	area = (char *) malloc (size);
	getimage (174, 138, 484, 268, area);

	setfillstyle (SOLID_FILL, BLACK);
	bar (184, 148, 484, 268);
	setfillstyle (SOLID_FILL, LIGHTGRAY);
	bar (174, 138, 474, 258);
	setfillstyle (SOLID_FILL, LIGHTBLUE);
	bar (174, 138, 474, 154);
	cmd_button (457, 140, 15, 13, WHITE, BLACK, "X");

	setcolor (WHITE);
	outtextxy (180, 143, title);
	setcolor (BLUE);
	outtextxy (230, 185, prompt);
	setfillstyle (SOLID_FILL, BLACK);
	bar (230, 205, 260, 225);
	gotoxy (30, 14);
	scanf ("%d", &value);

	setcolor (WHITE);
	getch ();
	putimage (174, 138, area, COPY_PUT);
	free (area);
	showmouseptr ();
	return (value);
}

char* get_file (char* title, char* prompt)
{
	int size;
	char* name;
	char* area = NULL;
	setviewport (1, 1, X, Y, 1);

	size = imagesize (174, 138, 484, 268);
	area = (char *) malloc (size);
	getimage (174, 138, 484, 268, area);

	setfillstyle (SOLID_FILL, BLACK);
	bar (184, 148, 484, 268);
	setfillstyle (SOLID_FILL, LIGHTGRAY);
	bar (174, 138, 474, 258);
	setfillstyle (SOLID_FILL, LIGHTBLUE);
	bar (174, 138, 474, 154);
	cmd_button (457, 140, 15, 13, WHITE, BLACK, "X");

	setcolor (WHITE);
	outtextxy (180, 143, title);
	setcolor (BLUE);
	outtextxy (230, 185, prompt);
	setfillstyle (SOLID_FILL, BLACK);
	bar (230, 205, 260, 225);
	gotoxy (30, 14);
	scanf ("%s", name);

	setcolor (WHITE);
	getch ();
	putimage (174, 138, area, COPY_PUT);
	free (area);
	showmouseptr ();
	return (name);
}

void dialog (char* title, char* message, char* tip)
{
	int size;
	char* area = NULL;
	setviewport (1, 1, X, Y, 1);

	size = imagesize (174, 138, 484, 268);
	area = (char *) malloc (size);
	getimage (174, 138, 484, 268, area);

	setfillstyle (SOLID_FILL, BLACK);
	bar (184, 148, 484, 268);
	setfillstyle (SOLID_FILL, LIGHTGRAY);
	bar (174, 138, 474, 258);
	setfillstyle (SOLID_FILL, LIGHTBLUE);
	bar (174, 138, 474, 154);
	cmd_button (457, 140, 15, 13, WHITE, BLACK, "X");

	setcolor (WHITE);
	outtextxy (180, 143, title);
	setcolor (BLUE);
	outtextxy (230, 200, message);
	setcolor (BLACK);
	outtextxy (200, 240, tip);

	setcolor (WHITE);
	getch ();
	putimage (174, 138, area, COPY_PUT);
	free (area);
	showmouseptr ();
}

void draw_tools ()
{
	int x, y, r, i;

	new.left   = 20;
	new.top    = 30;
	new.right  = 45;
	new.bottom = 55;
	button (new, WHITE, BLACK);			/* new */
	setcolor (BLACK);
	line (22, 32, 35, 32);
	line (22, 32, 22, 53);
	line (22, 53, 42, 53);
	line (42, 53, 42, 37);
	line (35, 32, 42, 37);
	line (35, 32, 37, 38);
	line (37, 38, 42, 37);
	setfillstyle (SOLID_FILL, WHITE);
	floodfill (24, 36, BLACK);
	setcolor (WHITE);

	open1.left   = 55;
	open1.top    = 30;
	open1.right  = 80;
	open1.bottom = 55;
	button (open1, WHITE, BLACK);			/* open  */
	setcolor (BLACK);
	line (58, 33, 64, 33);
	line (64, 33, 64, 36);
	line (64, 36, 75, 36);
	line (75, 36, 75, 39);
	line (58, 33, 58, 52);
	line (58, 52, 65, 40);
	line (65, 40, 78, 40);
	line (58, 52, 71, 52);
	line (78, 40, 71, 52);
	setfillstyle (SOLID_FILL, YELLOW);
	floodfill (63, 42, BLACK);
	setfillstyle (SOLID_FILL, DARKGRAY);
	floodfill (69, 50, BLACK);
	setcolor(WHITE);

	save.left   = 90;
	save.top    = 30;
	save.right  = 115;
	save.bottom = 55;
	button (save, WHITE, BLACK);			/* save    */
	setcolor (BLACK);
	line (93, 33, 112, 33);
	line (93, 33, 93, 48);
	line (93, 48, 97, 52);
	line (97, 52, 112, 52);
	line (112, 52, 112, 33);
	rectangle (96, 33, 109, 39);
	rectangle (98, 52, 109, 45);
	setfillstyle (SOLID_FILL, BLACK);
	floodfill (99, 48, BLACK);
	for ( y = 46; y < 52; y++)
		putpixel (105, y, WHITE);
	for ( y = 46; y < 52; y++)
		putpixel (106, y, WHITE);
	setfillstyle (SOLID_FILL, BROWN);
	floodfill (94, 43, BLACK);
	setcolor (WHITE);

	cut.left   = 165;
	cut.top	   = 30;
	cut.right  = 190;
	cut.bottom = 55;
	button (cut, WHITE, BLACK);  			/* cut      */
	circle (175, 50, 2);
	circle (185, 48, 2);
	line (177, 48, 177, 44);
	line (177, 44, 183, 40);
	line (183, 40, 183, 33);
	line (183, 46, 183, 44);
	line (183, 44, 177, 40);
	line (177, 40, 177, 33);

	copy.left   = 200;
	copy.top    = 30;
	copy.right  = 225;
	copy.bottom = 55;
	button (copy, WHITE, BLACK);  			/* copy      */
	line (202, 31, 212, 31);
	line (202, 31, 202, 45);
	line (202, 45, 215, 45);
	line (215, 45, 215, 33);
	line (215, 33, 212, 31);
	line (212, 31, 212, 35);
	line (213, 35, 215, 33);
	setfillstyle (SOLID_FILL, WHITE);
	floodfill (203, 32, BLACK);
	line (210, 38, 220, 38);
	line (210, 38, 210, 51);
	line (210, 51, 223, 51);
	line (223, 51, 223, 41);
	line (223, 41, 220, 38);
	line (220, 38, 220, 42);
	line (220, 42, 223, 41);
	setfillstyle (SOLID_FILL, WHITE);
	floodfill (219, 40, BLACK);
	for (y = 39; y < 45; y++)
		putpixel ( 215, y, WHITE);
	for (x = 211; x < 216; x++)
		putpixel ( x, 45, WHITE);

	paste.left   = 235;
	paste.top    = 30;
	paste.right  = 260;
	paste.bottom = 55;
	button (paste, WHITE, BLACK); 			/* paste     */
	line (248, 40, 259, 40);
	line (248, 40, 248, 55);
	line (248, 55, 261, 55);
	line (261, 55, 261, 43);
	line (261, 43, 259, 40);
	line (259, 40, 259, 46);
	line (259, 46, 261, 43);
	setfillstyle (SOLID_FILL, WHITE);
	floodfill (249, 41, BLACK);
	line (238, 33, 257, 33);
	line (238, 33, 238, 52);
	line (238, 52, 248, 52);
	line (257, 33, 257, 40);
	line (242, 35, 253, 35);
	ellipse (247, 35, 0, 180, 7, 4);
	setfillstyle (SOLID_FILL, BROWN);
	floodfill (239, 34, BLACK);

	trans.left   = 290;
	trans.top    = 30;
	trans.right  = 315;
	trans.bottom = 55;
	button (trans, WHITE, BLACK); 			/* translate  */
	setlinestyle (USERBIT_LINE, 0xAAAA, 1);
	rectangle (292, 32, 310, 45);
	rectangle (300, 40, 318, 53);
	setlinestyle (SOLID_LINE, 0, 1);

	rot.left   = 325;
	rot.top    = 30;
	rot.right  = 350;
	rot.bottom = 55;
	button (rot, WHITE, BLACK); 			/* rotate     */
	arc (340, 43, 300, 150, 10);
	line (331, 38, 334, 32);
	line (331, 38, 334, 40);

	scale.left   = 360;
	scale.top    = 30;
	scale.right  = 385;
	scale.bottom = 55;
	button (scale, WHITE, BLACK); 			/* scale */
	rectangle (362, 32, 385, 53);
	line (370, 40, 375, 40);
	line (370, 45, 375, 45);
	line (370, 40, 370, 37);
	line (370, 37, 362, 42);
	line (370, 45, 370, 48);
	line (370, 48, 362, 42);
	line (375, 40, 375, 37);
	line (375, 45, 375, 48);
	line (375, 37, 385, 42);
	line (375, 48, 385, 42);

	clip.left   = 395;
	clip.top    = 30;
	clip.right  = 420;
	clip.bottom = 55;
	button (clip, WHITE, BLACK); 			/* clip  */
	setlinestyle (USERBIT_LINE, 0xBBBB, 1);
	rectangle (398, 33, 415, 50);
	circle (418, 36, 2);
	circle (418, 33, 2);
	setlinestyle(SOLID_LINE, 0, 2);
	line (417, 35, 405, 32);
	line (417, 34, 405, 37);
	setlinestyle(SOLID_LINE, 0, 1);

	help.left   = 450;
	help.top    = 30;
	help.right  = 475;
	help.bottom = 55;
	button (help, WHITE, BLACK); 			/* help  */
	settextstyle (TRIPLEX_FONT, HORIZ_DIR, 1);
	setcolor (BLUE);
	outtextxy (459, 33, "?");
	setcolor (WHITE);

	about.left   = 485;
	about.top    = 30;
	about.right  = 510;
	about.bottom = 55;
	button (about, WHITE, BLACK); 			/* about  */
	settextstyle (GOTHIC_FONT, HORIZ_DIR, 1);
	setcolor (LIGHTBLUE);
	outtextxy (478, 30, " m ");
	setcolor (RED);
	outtextxy (492, 30, " D ");
	setcolor (WHITE);

	select.left   = 16;
	select.top    = 80;
	select.right  = 41;
	select.bottom = 105;
	button (select, WHITE, BLACK);  		/* select */
	setlinestyle (DOTTED_LINE, 0, 2);
	rectangle (18, 82, 41, 105);
	setlinestyle (SOLID_LINE, 0, 1);

	eraser.left   = 52;
	eraser.top    = 80;
	eraser.right  = 77;
	eraser.bottom = 105;
	button (eraser, WHITE, BLACK);			/* eraser */
	rectangle (56, 95, 66, 100);
	setfillstyle (SOLID_FILL, WHITE);
	floodfill (57, 96, BLACK);
	line (56, 95, 66, 85);
	line (66, 95, 76, 85);
	line (66, 85, 76, 85);
	setfillstyle (SOLID_FILL, YELLOW);
	floodfill (67, 89, BLACK);
	line (76, 85, 76, 90);
	line (76, 90, 66, 100);
	setfillstyle (SOLID_FILL, YELLOW);
	floodfill (74, 89, BLACK);

	fill.left   = 16;
	fill.top    = 115;
	fill.right  = 41;
	fill.bottom = 140;
	button (fill, WHITE, BLACK);			/* fill */
	line (30, 118, 40, 128);
	line (40, 128, 30, 138);
	line (30, 138, 20, 128);
	line (20, 128, 30, 118);
	line (20, 128, 40, 128);
	setfillstyle (SOLID_FILL, BLUE);
	floodfill (30, 132, BLACK);
	for (x = 21; x >= 19; x--)
	{
		for (y = 128; y <= 132; y++)
			putpixel (x, y, BLUE);
	}

	spray.left   = 52;
	spray.top    = 115;
	spray.right  = 77;
	spray.bottom = 140;
	button (spray, WHITE, BLACK);			/* spray */
	line (56, 123, 56, 140);
	line (65, 123, 65, 140);
	line (56, 140, 65, 140);
	line (56, 123, 60, 116);
	line (61, 116, 65, 123);
	setfillstyle (SOLID_FILL, LIGHTBLUE);
	floodfill (57, 124, BLACK);
	setlinestyle (DOTTED_LINE, 0, 1);
	line (60, 116, 75, 116);
	line (60, 116, 75, 118);
	line (60, 116, 75, 120);
	line (60, 116, 75, 122);
	setlinestyle (SOLID_LINE, 0, 1);

	pen.left   = 16;
	pen.top    = 150;
	pen.right  = 41;
	pen.bottom = 175;
	button (pen, WHITE, BLACK);			/* freehand */
	setcolor (BLUE);
	line (41, 154, 23, 172);
	line (38, 152, 20, 170);
	line (41, 154, 38, 152);
	line (20, 170, 23, 172);
	setfillstyle (SOLID_FILL, GREEN);
	floodfill (36, 157, BLUE);
	setcolor (BLACK);
	line (23, 172, 18, 175);
	line (18, 175, 20, 170);

	brush.left   = 52;
	brush.top    = 150;
	brush.right  = 77;
	brush.bottom = 175;
	button (brush, WHITE, BLACK);			/* brush */
	line (62, 152, 62, 165);
	line (65, 152, 65, 165);
	line (62, 165, 65, 165);
	ellipse (63, 152, 0, 180, 1, 1);
	arc (62, 169, 90, 270, 5);
	arc (65, 169, 270, 90, 5);
	setfillstyle (SOLID_FILL, YELLOW);
	bar (57, 169, 70, 175);

	setfillstyle (SOLID_FILL, WHITE);
	bar (60, 170, 62, 175);
	lin.left   = 16;
	lin.top    = 185;
	lin.right  = 41;
	lin.bottom = 210;

	button (lin, WHITE, BLACK);			/* line */
	setlinestyle (SOLID_LINE, 0, 2);
	line (41, 187, 18, 210);

	arc1.left   = 52;
	arc1.top    = 185;
	arc1.right  = 77;
	arc1.bottom = 210;
	button (arc1, WHITE, BLACK);			/* arc */
	arc (65, 192, 300, 140, 7);
	arc (75, 203, 140, 300, 8);

	rect.left   = 16;
	rect.top    = 220;
	rect.right  = 41;
	rect.bottom = 245;
	button (rect, WHITE, BLACK);			/* rectangle */
	setlinestyle (SOLID_LINE, 0, 2);
	rectangle (19, 223, 41, 240);

	polyg.left   = 52;
	polyg.top    = 220;
	polyg.right  = 77;
	polyg.bottom = 245;
	button (polyg, WHITE, BLACK);			/* polygon */
	line (59, 222, 68, 222);
	line (59, 222, 54, 242);
	line (68, 222, 64, 235);
	line (64, 235, 77, 235);
	line (54, 242, 77, 242);
	line (77, 242, 77, 235);

	ellipse1.left   = 16;
	ellipse1.top    = 255;
	ellipse1.right  = 41;
	ellipse1.bottom = 280;
	button (ellipse1, WHITE, BLACK);		/* ellipse */
	ellipse (28, 267, 0, 360, 11, 7);

	text.left   = 52;
	text.top    = 255;
	text.right  = 77;
	text.bottom = 280;
	button (text, WHITE, BLACK);			/* text */
	settextstyle (TRIPLEX_FONT, HORIZ_DIR, 2);
	outtextxy(60, 256, "T");

	clear.left   = 16;
	clear.top    = 290;
	clear.right  = 41;
	clear.bottom = 315;
	button (clear, WHITE, BLACK);               	/* clear */

	spiral.left   = 52;
	spiral.top    = 290;
	spiral.right  = 77;
	spiral.bottom = 315;
	button (spiral, WHITE, BLACK);			/* spiral */
	r = 2;
	for (i = 0; i < 3; i++)
	{
		arc (66, 304, 270, 90, r);
		r += 2;

		arc (66, 302, 90, 270, r);
		r += 2;
	}
}

void colour_bar ()
{
	setfillstyle (SOLID_FILL, CUR_COL);
	box.left   = 65;
	box.top    = 445;
	box.right  = 90;
	box.bottom = 470;
	bar (65, 445, 90, 470);
	button (box, BLACK, WHITE);

	setfillstyle (SOLID_FILL, 0);
	bar (100, 445, 112, 457);
	button_small (100, 445, 112, 457, BLACK, WHITE);
	setfillstyle (SOLID_FILL, 1);
	bar (115, 445, 127, 457);
	button_small (115, 445, 127, 457, BLACK, WHITE);
	setfillstyle (SOLID_FILL, 2);
	bar (130, 445, 142, 457);
	button_small (130, 445, 142, 457, BLACK, WHITE);
	setfillstyle (SOLID_FILL, 3);
	bar (145, 445, 158, 457);
	button_small (145, 445, 157, 457, BLACK, WHITE);
	setfillstyle (SOLID_FILL, 4);
	bar (160, 445, 172, 457);
	button_small (160, 445, 172, 457, BLACK, WHITE);
	setfillstyle (SOLID_FILL, 5);
	bar (175, 445, 187, 457);
	button_small (175, 445, 187, 457, BLACK, WHITE);
	setfillstyle (SOLID_FILL, 6);
	bar (190, 445, 202, 457);
	button_small (190, 445, 202, 457, BLACK, WHITE);
	setfillstyle (SOLID_FILL, 7);
	bar (205, 445, 217, 457);
	button_small (205, 445, 217, 457, BLACK, WHITE);

	setfillstyle (SOLID_FILL, 8);
	bar (100, 460, 112, 472);
	button_small (100, 460, 112, 472, BLACK, WHITE);
	setfillstyle (SOLID_FILL, 9);
	bar (115, 460, 127, 472);
	button_small (115, 460, 127, 472, BLACK, WHITE);
	setfillstyle (SOLID_FILL, 10);
	bar (130, 460, 142, 472);
	button_small (130, 460, 142, 472, BLACK, WHITE);
	setfillstyle (SOLID_FILL, 11);
	bar (145, 460, 157, 472);
	button_small (145, 460, 157, 472, BLACK, WHITE);
	setfillstyle (SOLID_FILL, 12);
	bar (160, 460, 172, 472);
	button_small (160, 460, 172, 472, BLACK, WHITE);
	setfillstyle (SOLID_FILL, 13);
	bar (175, 460, 187, 472);
	button_small (175, 460, 187, 472, BLACK, WHITE);
	setfillstyle (SOLID_FILL, 14);
	bar (190, 460, 202, 472);
	button_small (190, 460, 202, 472, BLACK, WHITE);
	setfillstyle (SOLID_FILL, 15);
	bar (205, 460, 217, 472);
	button_small (205, 460, 217, 472, BLACK, WHITE);
}

void pattern_bar ()
{
	setfillstyle (CUR_PAT, WHITE);
	_fill.left   = 300;
	_fill.top    = 447;
	_fill.right  = 325;
	_fill.bottom = 472;
	bar (300, 447, 325, 472);
	button (_fill, BLACK, WHITE);

	setfillstyle (EMPTY_FILL, WHITE);
	bar (340, 445, 352, 457);
	button_small (340, 445, 352, 457, BLACK, WHITE);
	setfillstyle (SOLID_FILL, WHITE);
	bar (355, 445, 367, 457);
	button_small (355, 445, 367, 457, BLACK, WHITE);
	setfillstyle (LINE_FILL, WHITE);
	bar (370, 445, 382, 457);
	button_small (370, 445, 382, 457, BLACK, WHITE);
	setfillstyle (LTSLASH_FILL, WHITE);
	bar (385, 445, 397, 457);
	button_small (385, 445, 397, 457, BLACK, WHITE);
	setfillstyle (SLASH_FILL, WHITE);
	bar (400, 445, 412, 457);
	button_small (400, 445, 412, 457, BLACK, WHITE);
	setfillstyle (BKSLASH_FILL, WHITE);
	bar (415, 445, 427, 457);
	button_small (415, 445, 427, 457, BLACK, WHITE);
	setfillstyle (LTBKSLASH_FILL, WHITE);
	bar (430, 445, 442, 457);
	button_small (430, 445, 442, 457, BLACK, WHITE);

	setfillstyle (HATCH_FILL, WHITE);
	bar (340, 460, 352, 472);
	button_small (340, 460, 352, 472, BLACK, WHITE);
	setfillstyle (XHATCH_FILL, WHITE);
	bar (355, 460, 367, 472);
	button_small (355, 460, 367, 472, BLACK, WHITE);
	setfillstyle (INTERLEAVE_FILL, WHITE);
	bar (370, 460, 382, 472);
	button_small (370, 460, 382, 472, BLACK, WHITE);
	setfillstyle (WIDE_DOT_FILL, WHITE);
	bar (385, 460, 397, 472);
	button_small (385, 460, 397, 472, BLACK, WHITE);
	setfillstyle (CLOSE_DOT_FILL, WHITE);
	bar (400, 460, 412, 472);
	button_small (400, 460, 412, 472, BLACK, WHITE);
	setfillpattern (pattern1, WHITE);
	bar (415, 460, 427, 472);
	button_small (415, 460, 427, 472, BLACK, WHITE);
	setfillpattern (pattern2, WHITE);
	bar (430, 460, 442, 472);
	button_small (430, 460, 442, 472, BLACK, WHITE);
}

void screen ()
{
	X = getmaxx ();
	Y = getmaxy ();
	setviewport (0, 0, X, Y, 1);
	setlinestyle (SOLID_LINE,0,2);			/* main border */
	setfillstyle (SOLID_FILL, BLUE);
	bar (0, 0, X, 24);
	rectangle (0, 0, X, 24);    		 	/* task bar */

	cmd_button (X - 30, 4, 20, 16, WHITE, BLACK, "X");

	setfillstyle (SOLID_FILL, LIGHTGRAY);
	bar (0, 60, 90, Y - 40);
	rectangle (0, 60, 90, Y - 40); 			/* draw_toolbar */

	setfillstyle (SOLID_FILL, WHITE);
	bar (90, 63, X, Y - 40);
	rectangle (90, 63, X, Y - 40);          	/* work area*/
	canvas.left   = 90;
	canvas.top    = 63;
	canvas.right  = X;
	canvas.bottom = Y - 40;

	setcolor (BLACK);
	line (90, 63, 90, Y - 40);
	line (90, 63, X, 63);

	setfillstyle (SOLID_FILL, LIGHTGRAY);
	bar (0, 24, X, 63);
	rectangle (0, 24, X, 63);                	/* toolbar */

	setfillstyle (SOLID_FILL, LIGHTGRAY);
	bar (0, Y - 40, X, Y);
	rectangle (0, Y - 40, X, Y);			/* colour bar */

	settextstyle (4, HORIZ_DIR, 1);
	setcolor (LIGHTCYAN);
	outtextxy (5, 0, " miniDraw ");

	draw_tools ();
	colour_bar ();
	pattern_bar ();
}

void close_button ()
{
	cmd_button_click (X - 30, 4, 20, 16, "X");
	delay (100);
	deInit ();
}

void check_line_style ()
{
	cmd_button (16, 325, 60, 10, WHITE, BLACK, "");
	line (18, 330, 74, 330);
	cmd_button (16, 340, 60, 10, WHITE, BLACK, "");
	setlinestyle (SOLID_LINE, 0, THICK_WIDTH);
	line (18, 345, 74, 345);
	setlinestyle (SOLID_LINE, 0, NORM_WIDTH);
}

void check ()
{
	setviewport (0, 0, X, Y, 1);

	if (x >= 609  && x <= 629 && y >= 4 && y <= 20)
		close_button ();

	else
	if (x >= 16 && x <= 76 && y >= 340 && y <= 350)
	{
		cmd_button_click (16, 340, 60, 10, "");
		_thickness = 3;
		setlinestyle (SOLID_LINE, 0, _thickness);
		line (18, 345, 74, 345);
	}

	else
	if (x >= 16 && x <= 76 && y >= 325 && y <= 335)
	{
		cmd_button_click (16, 325, 60, 10, "");
		_thickness = 1;
		setlinestyle (SOLID_LINE, 0, _thickness);
		line (18, 330, 74, 330);
	}

	else
	if (x >= 20 && x <= 45 && y >= 30 && y <= 55)
	{
		button_click (new, FALSE);
		CLICKED = 1;
		SELC = NEW;
	}

	else
	if (x >= 55 && x <= 80 && y >= 30 && y <= 55)
	{
		button_click (open1, FALSE);
		CLICKED = 1;
		SELC = OPEN;
	}

	else
	if (x >= 90 && x <= 115 && y >= 30 && y <= 55)
	{
		button_click (save, FALSE);
		CLICKED = 1;
		SELC = SAVE;
	}

	else
	if (x >= 165 && x <= 190 && y >= 30 && y <= 55)
	{
		button_click (cut, FALSE);
		CLICKED = 1;
		SELC = CUT;
	}

	else
	if (x >= 200 && x <= 225 && y >= 30 && y <= 55)
	{
		button_click (copy, FALSE);
		CLICKED = 1;
		SELC = COPY;
	}

	else
	if (x >= 235 && x <= 260 && y >= 30 && y <= 55)
	{
		button_click (paste, FALSE);
		CLICKED = 1;
		SELC = PASTE;
	}

	else
	if (x >= 290 && x <= 315 && y >= 30 && y <= 55)
	{
		button_click (trans, FALSE);
		CLICKED = 1;
		SELC = TRANS;
	}

	else
	if (x >= 325 && x <= 350 && y >= 30 && y <= 55)
	{
		button_click (rot, FALSE);
		CLICKED = 1;
		SELC = ROTATE;
	}

	else
	if (x >= 360 && x <= 385 && y >= 30 && y <= 55)
	{
		button_click (scale, FALSE);
		CLICKED = 1;
		SELC = SCALE;
	}

	else
	if (x >= 395 && x <= 420 && y >= 30 && y <= 55)
	{
		button_click (clip, FALSE);
		CLICKED = 1;
		SELC = CLIP;
	}

	else
	if (x >= 450 && x <= 475 && y >= 30 && y <= 55)
	{
		button_click (help, FALSE);
		CLICKED = 1;
		SELC = HELP;
	}

	else
	if (x >= 485 && x <= 510 && y >= 30 && y <= 55)
	{
		button_click (about, FALSE);
		CLICKED = 1;
		SELC = ABOUT;
	}

	else
	if (x >= 16 && x <= 41 && y >= 80 && y <= 105 )
	{
		button_click (select, TRUE);
		CLICKED = 1;
		SELC = SELECT;
	}

	else
	if (x >= 52 && x <= 77 && y >= 80 && y <= 105 )
	{
		button_click (eraser, TRUE);
		CLICKED = 1;
		SELC = ERASER;
	}

	else
	if (x >= 16 && x <= 41 && y >= 115 && y <= 140 )
	{
		button_click (fill, TRUE);
		CLICKED = 1;
		SELC = FILL;
	}

	else
	if (x >= 52 && x <= 77 && y >= 115 && y <= 140 )
	{
		button_click (spray, TRUE);
		CLICKED = 1;
		SELC = SPRAY;
	}

	else
	if (x >= 16 && x <= 41 && y >= 150 && y <= 175 )
	{
		button_click (pen, TRUE);
		CLICKED = 1;
		SELC = PEN;
		check_line_style ();
	}

	else
	if (x >= 52 && x <= 77 && y >= 150 && y <= 175 )
	{
		button_click (brush, TRUE);
		CLICKED = 1;
		SELC = BRUSH;
	}

	else
	if (x >= 16 && x <= 41 && y >= 185 && y <= 210 )
	{
		button_click (lin, TRUE);
		CLICKED = 1;
		SELC = LINE;
		check_line_style ();
	}

	else
	if (x >= 52 && x <= 77 && y >= 185 && y <= 210 )
	{
		button_click (arc1, FALSE);
		CLICKED = 1;
		SELC = CURVE;
		check_line_style ();
	}

	else
	if (x >= 16 && x <= 41 && y >= 220 && y <= 245 )
	{
		button_click (rect,TRUE);
		CLICKED = 1;
		SELC = RECT;
		check_line_style ();
	}

	else
	if (x >= 52 && x <= 77 && y >= 220 && y <= 245 )
	{
		button_click (polyg, TRUE);
		CLICKED = 1;
		SELC = POLY;
		check_line_style ();
	}

	else
	if (x >= 16 && x <= 41 && y >= 255 && y <= 280 )
	{
		button_click (ellipse1, TRUE);
		CLICKED = 1;
		SELC = ELLIPSE;
		check_line_style ();
	}

	else
	if (x >= 52 && x <= 77 && y >= 255 && y <= 280 )
	{
		button_click (text, TRUE);
		CLICKED = 1;
		SELC = TEXT;
	}

	else
	if (x >= 16 && x <= 41 && y >= 290 && y <= 315 )
	{
		button_click (clear, FALSE);
		CLICKED = 1;
		SELC = CANV_FILL;
	}

	else
	if (x >= 52 && x <= 77 && y >= 290 && y <= 315 )
	{
		button_click (spiral, TRUE);
		CLICKED = 1;
		SELC = SPIRAL;
	}
	setviewport (canvas.left, canvas.top, canvas.right, canvas.bottom, 1);
}

void check_col ()
{
	setviewport (1, 1, X, Y, 1);

	if (x >= 100 && x <= 112 && y >= 445 && y <= 457)
	{
		CUR_COL = 0;
		setfillstyle (1, 0);
		bar (box.left, box.top, box.right, box.bottom);
	}
	else
	if (x >= 115 && x <= 127 && y >= 445 && y <= 457)
	{
		CUR_COL = 1;
		setfillstyle (1, 1);
		bar (box.left, box.top, box.right, box.bottom);
	}
	else
	if (x >= 130 && x <= 142 && y >= 445 && y <= 457)
	{
		CUR_COL = 2;
		setfillstyle (1, 2);
		bar (box.left, box.top, box.right, box.bottom);
	}
	else
	if (x >= 145 && x <= 157 && y >= 445 && y <= 457)
	{
		CUR_COL = 3;
		setfillstyle (1, 3);
		bar (box.left, box.top, box.right, box.bottom);
	}
	else
	if (x >= 160 && x <= 172 && y >= 445 && y <= 457)
	{
		CUR_COL = 4;
		setfillstyle (1, 4);
		bar (box.left, box.top, box.right, box.bottom);
	}
	else
	if (x >= 175 && x <= 187 && y >= 445 && y <= 457)
	{
		CUR_COL = 5;
		setfillstyle (1, 5);
		bar (box.left, box.top, box.right, box.bottom);
	}
	else
	if (x >= 190 && x <= 202 && y >= 445 && y <= 457)
	{
		CUR_COL = 6;
		setfillstyle (1, 6);
		bar (box.left, box.top, box.right, box.bottom);
	}
	else
	if (x >= 205 && x <= 217 && y >= 445 && y <= 457)
	{
		CUR_COL = 7;
		setfillstyle (1, 7);
		bar (box.left, box.top, box.right, box.bottom);
	}
	else
	if (x >= 100 && x <= 112 && y >= 460 && y <= 472)
	{
		CUR_COL = 8;
		setfillstyle (1, 8);
		bar (box.left, box.top, box.right, box.bottom);
	}
	else
	if (x >= 115 && x <= 127 && y >= 460 && y <= 472)
	{
		CUR_COL = 9;
		setfillstyle (1, 9);
		bar (box.left, box.top, box.right, box.bottom);
	}
	else
	if (x >= 130 && x <= 142 && y >= 460 && y <= 472)
	{
		CUR_COL = 10;
		setfillstyle (1, 10);
		bar (box.left, box.top, box.right, box.bottom);
	}
	else
	if (x >= 145 && x <= 157 && y >= 460 && y <= 472)
	{
		CUR_COL = 11;
		setfillstyle (1, 11);
		bar (box.left, box.top, box.right, box.bottom);
	}
	else
	if (x >= 160 && x <= 172 && y >= 460 && y <= 472)
	{
		CUR_COL = 12;
		setfillstyle (1, 12);
		bar (box.left, box.top, box.right, box.bottom);
	}
	else
	if (x >= 175 && x <= 187 && y >= 460 && y <= 472)
	{
		CUR_COL = 13;
		setfillstyle (1, 13);
		bar (box.left, box.top, box.right, box.bottom);
	}
	else
	if (x >= 190 && x <= 202 && y >= 460 && y <= 472)
	{
		CUR_COL = 14;
		setfillstyle (1, 14);
		bar (box.left, box.top, box.right, box.bottom);
	}
	else
	if (x >= 205 && x <= 217 && y >= 460 && y <= 472)
	{
		CUR_COL = 15;
		setfillstyle (1, 15);
		bar (box.left, box.top, box.right, box.bottom);
	}
	setviewport (canvas.left, canvas.top, canvas.right, canvas.bottom, 1);
}

void check_pattern ()
{
	setviewport (1, 1, X, Y, 1);

	if (x >= 340 && x <= 352 && y >= 445 && y <= 457)
	{
		CUR_PAT = 0;
		setfillstyle (EMPTY_FILL, WHITE);
		bar (_fill.left, _fill.top, _fill.right, _fill.bottom);
	}
	else
	if (x >= 355 && x <= 372 && y >= 445 && y <= 457)
	{
		CUR_PAT = 1;
		setfillstyle (SOLID_FILL, WHITE);
		bar (_fill.left, _fill.top, _fill.right, _fill.bottom);
	}
	else
	if (x >= 370 && x <= 382 && y >= 445 && y <= 457)
	{
		CUR_PAT = 2;
		setfillstyle (LINE_FILL, WHITE);
		bar (_fill.left, _fill.top, _fill.right, _fill.bottom);
	}
	else
	if (x >= 385 && x <= 397 && y >= 445 && y <= 457)
	{
		CUR_PAT = 3;
		setfillstyle (LTSLASH_FILL, WHITE);
		bar (_fill.left, _fill.top, _fill.right, _fill.bottom);
	}
	else
	if (x >= 400 && x <= 412 && y >= 445 && y <= 457)
	{
		CUR_PAT = 4;
		setfillstyle (SLASH_FILL, WHITE);
		bar (_fill.left, _fill.top, _fill.right, _fill.bottom);
	}
	else
	if (x >= 415 && x <= 427 && y >= 445 && y <= 457)
	{
		CUR_PAT = 5;
		setfillstyle (BKSLASH_FILL, WHITE);
		bar (_fill.left, _fill.top, _fill.right, _fill.bottom);
	}
	else
	if (x >= 430 && x <= 442 && y >= 445 && y <= 457)
	{
		CUR_PAT = 6;
		setfillstyle (LTBKSLASH_FILL, WHITE);
		bar (_fill.left, _fill.top, _fill.right, _fill.bottom);
	}
	else
	if (x >= 340 && x <= 352 && y >= 460 && y <= 472)
	{
		CUR_PAT = 7;
		setfillstyle (HATCH_FILL, WHITE);
		bar (_fill.left, _fill.top, _fill.right, _fill.bottom);
	}
	else
	if (x >= 355 && x <= 367 && y >= 460 && y <= 472)
	{
		CUR_PAT = 8;
		setfillstyle (XHATCH_FILL, WHITE);
		bar (_fill.left, _fill.top, _fill.right, _fill.bottom);
	}
	else
	if (x >= 370 && x <= 382 && y >= 460 && y <= 472)
	{
		CUR_PAT = 9;
		setfillstyle (INTERLEAVE_FILL, WHITE);
		bar (_fill.left, _fill.top, _fill.right, _fill.bottom);
	}
	else
	if (x >= 385 && x <= 397 && y >= 460 && y <= 472)
	{
		CUR_PAT = 10;
		setfillstyle (WIDE_DOT_FILL, WHITE);
		bar (_fill.left, _fill.top, _fill.right, _fill.bottom);
	}
	else
	if (x >= 400 && x <= 412 && y >= 460 && y <= 472)
	{
		CUR_PAT = 11;
		setfillstyle (CLOSE_DOT_FILL, WHITE);
		bar (_fill.left, _fill.top, _fill.right, _fill.bottom);
	}
	else
	if (x >= 415 && x <= 427 && y >= 460 && y <= 472)
	{
		CUR_PAT = 12;
		setfillpattern (pattern1, WHITE);
		bar (_fill.left, _fill.top, _fill.right, _fill.bottom);
	}
	else
	if (x >= 430 && x <= 442 && y >= 460 && y <= 472)
	{
		CUR_PAT = 13;
		setfillpattern (pattern2, WHITE);
		bar (_fill.left, _fill.top, _fill.right, _fill.bottom);
	}
	setviewport (canvas.left, canvas.top, canvas.right, canvas.bottom, 1);
}

int leftbuttonpressed ()
{
	getmousepos (&_button, &x, &y);
	if ((_button & 1) == 1)
		return 1;
	return 0;
}

int rightbutton ()
{
	getmousepos (&_button, &x, &y);
	if ((_button & 2) == 2)
		return 1;
	return 0;
}

void Pencil ()
{
	int x1, y1, x2, y2;
	changecursor (pencil);
	while (leftbuttonpressed () != 1)
		;

	while (1)
	{
		x1 = x - canvas.left;
		y1 = y - canvas.top;

		while (leftbuttonpressed ())
		{
			x2 = x - canvas.left;
			y2 = y - canvas.top;

			if (x1 != x2 || y1 != y2)
			{
				hidemouseptr ();
				setcolor (CUR_COL);
				line (x1, y1, x2, y2);
				showmouseptr ();
				x1 = x2;
				y1 = y2;
			}
		}
		while (leftbuttonpressed () != 1)
			;
		check ();
		if (CLICKED && PEN != 1)
		{
			CLICKED = 0;
			break;
		}
		check_col ();
	}
	changecursor (pointer);
}

void Line ()
{
	int x1, y1;
	int x2, y2;
	changecursor (pencil);
	while (leftbuttonpressed () != 1)
		;

	while (1)
	{
		setcolor (MAX_COL - CUR_COL);
		setwritemode (XOR_PUT);

		x1 = x - canvas.left;
		y1 = y - canvas.top;
		x2 = x1;
		x2 = y1;
		hidemouseptr ();

		while (leftbuttonpressed ())
		{
			x2 = x - canvas.left;
			y2 = y - canvas.top;

			line (x1, y1, x2, y2);
			delay (150);
			line (x1, y1, x2, y2);
		}
		setcolor (CUR_COL);
		setwritemode (COPY_PUT);
		line (x1, y1, x2, y2);
		showmouseptr ();
		while (leftbuttonpressed () != 1)
			;
		check ();

		if (CLICKED == 1 && SELC != LINE)
		{
			CLICKED = 0;
			break;
		}
		check_col ();
	}
	changecursor (pointer);
}

void Rect ()
{
	int x1, y1;
	int x2, y2;

	while (leftbuttonpressed () != 1)
		;

	while (1)
	{
		setcolor (MAX_COL - CUR_COL);
		setwritemode (XOR_PUT);
		x1 = x - canvas.left;
		y1 = y - canvas.top;
		x2 = x1;
		y2 = y1;
		hidemouseptr ();

		while (leftbuttonpressed ())
		{
			x2 = x - canvas.left;
			y2 = y - canvas.top;

			rectangle (x1, y1, x2, y2);
			delay (150);
			rectangle (x1, y1, x2, y2);
		}

		while (rightbutton ())
		{
			x2 = x - canvas.left;
			y2 = y - canvas.top;

			setcolor (MAX_COL - CUR_COL);
			if (CUR_PAT == 12)
				setfillpattern (pattern1, CUR_COL);
			else
			if (CUR_PAT == 13)
				setfillpattern (pattern2, CUR_COL);
			else
				setfillstyle (CUR_PAT, CUR_COL);
			bar (x1, y1, x2, y2);
			delay (150);
			bar (x1, y1, x2, y2);
		}

		setcolor (CUR_COL);
		setwritemode (COPY_PUT);
		if (rightbutton ())
			bar (x1, y1, x2, y2);
		else
			rectangle (x1, y1, x2, y2);

		showmouseptr ();
		while (leftbuttonpressed () != 1)
			;
		check ();

		if(CLICKED == 1 && SELC != RECT)
		{
			CLICKED = 0;
			break;
		}
		check_col ();
		check_pattern ();
	}
}

void plotEllipsePoints (int xcen, int ycen, int x, int y)
{
	if (xcen + x >= 0 && xcen + x <= canvas.right-canvas.left && ycen + y >=0  && ycen + y <= canvas.bottom-canvas.top)
		line(xcen + x, ycen + y , xcen + x, ycen + y);
	if (xcen - x >= 0 && xcen - x <= canvas.right-canvas.left && ycen + y >=0  && ycen + y <= canvas.bottom-canvas.top)
		line(xcen - x, ycen + y, xcen - x, ycen + y);
	if (xcen + x >= 0 && xcen + x <= canvas.right-canvas.left && ycen - y >=0  && ycen - y <=canvas.bottom-canvas.top)
		line(xcen + x, ycen - y, xcen + x, ycen - y);
	if (xcen - x >= 0 && xcen - x <= canvas.right-canvas.left && ycen - y >=0  && ycen - y <= canvas.bottom-canvas.top)
		line(xcen - x, ycen - y, xcen - x, ycen - y);
}

void plotEllipse (int xcen, int ycen, float radx, float rady)
{
	float p;
	float px, py;
	float x, y;
	float rx, ry;
	float tworx, twory;

	ry = rady * rady;
	rx = radx * radx;
	twory = 2 * ry;
	tworx = 2 * rx;
	x = 0;
	y = rady;
	plotEllipsePoints (xcen, ycen, x, y);
	p = (ry - rx * rady + (0.25 * rx));
	px = 0;
	py = tworx * y;

	while (px < py)
	{
		x++;
		px = px + twory;
		if (p >= 0)
		{
			y--;
			py = py - tworx;
		}
		if (p < 0)
			p = p + ry + px;
		else
			p = p + ry + px - py;
		plotEllipsePoints (xcen, ycen, x, y);
	}

	p = (ry * (x + 0.5) * (x + 0.5) + rx * (y - 1) * (y - 1) - rx * ry);

	while (y > 0)
	{
		y--;
		py = py - tworx;
		if (p <= 0)
		{
			x++;
			px = px + twory;
		}
		if (p > 0)
			p = p + rx - py;
		else
			p = p + rx - py + px;
		plotEllipsePoints (xcen, ycen, x, y);
	}
}

void Ellipse ()
{
	int x1, y1;
	int x2, y2;
	int color;
	int xmid, ymid;
	float radx, rady;
	while (leftbuttonpressed () != 1)
		;

	while (1)
	{
		setcolor (MAX_COL - CUR_COL);
		setwritemode (XOR_PUT);

		x1 = x - canvas.left;
		y1 = y - canvas.top;
		x2 = x1;
		x2 = y1;

		hidemouseptr ();

		while (leftbuttonpressed ())
		{
			x2 = x - canvas.left;
			y2 = y - canvas.top;

			xmid = (x1 + x2) / 2;
			ymid = (y1 + y2) / 2;
			radx = abs((x2 - x1) / 2);
			rady = abs((y2 - y1) / 2);

			if (radx < 1)
				radx = 1;
			if (rady < 1)
				rady = 1;

			plotEllipse (xmid, ymid, radx, rady);
			delay (150);
			plotEllipse (xmid, ymid, radx, rady);
		}

		setcolor (CUR_COL);
		setwritemode (COPY_PUT);
		if (CUR_PAT == 12)
			setfillpattern (pattern1, CUR_COL);
		else
		if (CUR_PAT == 13)
			setfillpattern (pattern2, CUR_COL);
		else
			setfillstyle (CUR_PAT, CUR_COL);
		if (rightbutton ())
			fillellipse (xmid, ymid, radx, rady);
		else
			ellipse (xmid, ymid, 0, 360, radx, rady);

		showmouseptr ();
		while (leftbuttonpressed () != 1)
			;
		check ();
		if (CLICKED == 1 && SELC != ELLIPSE)
		{
			CLICKED = 0;
			break;
		}
		check_col ();
		check_pattern ();
	}
}

void Polygon ()
{
	int x1, y1;
	int x2, y2;
	int x3, y3;
	int x4, y4;

	changecursor (pencil);
	while (leftbuttonpressed () != 1)
		;
	x1 = x - canvas.left;
	y1 = y - canvas.top;
	x2 = x1;
	x3 = x1;
	y2 = y1;
	y3 = y1;

	while (1)
	{
		setcolor (CUR_COL);

		if (leftbuttonpressed () && y <= canvas.bottom)
		{
			x3 = x - canvas.left;
			y3 = y - canvas.top;
			hidemouseptr ();

			if (x1 != x2 && y1 != y2)
				line (x2, y2, x3, y3);
			showmouseptr ();

			x2 = x3;
			y2 = y3;
		}

		while (leftbuttonpressed () != 1)
			;
		check ();
		if (CLICKED == 1 && SELC != POLY)
		{
			CLICKED = 0;
			break;
		}
		check_col ();
	}
	changecursor (pointer);
}

void plotspiral (int x, int y, int rad)
{
	int r = 5;
	int tr = r;
	const int INC = 5;
	int ty = y;
	setcolor (CUR_COL);

	do
	{
		ellipse (x, ty, 270, 90, tr, tr);
		tr += INC;
		ty -= INC;
		ellipse (x, ty, 90, 270, tr, tr);
		tr += INC;
		ty += INC;
	}while (tr < rad && x + tr + r < canvas.right - canvas.left \
	&& x - tr > 0 	&& y + tr < canvas.bottom - canvas.left && \
	y - tr - 2 * r > 0);
}


void Spiral ()
{
	int x1, y1;
	int x2;
	setcolor (CUR_COL);
	while (leftbuttonpressed () != 1)

	while (1)
	{
		x1 = x - canvas.left;
		y1 = y - canvas.top;

		hidemouseptr ();

		while (leftbuttonpressed ())
		{
			x2 = x - canvas.left;
			plotspiral (x1, y1, abs (x1 - x2));
		}
		showmouseptr ();

		while (leftbuttonpressed () != 1)
			;
		check ();
		if (CLICKED == 1 && SELC != SPIRAL)
		{
			CLICKED = 0;
			break;
		}
		check_col ();
	}
}

void Select ()
{
	int x1, y1;
	int x2, y2;
	while (leftbuttonpressed () != 1)
		;

	while (1)
	{
		setwritemode (XOR_PUT);
		setlinestyle (DOTTED_LINE, 0, 1);
		x1 = x - canvas.left;
		y1 = y - canvas.top;

		while (leftbuttonpressed ())
		{
			x2 = x - canvas.left;
			y2 = y - canvas.top;
			if (x != x2 || y != y2)
			{
				hidemouseptr ();
				rectangle (x1, y1, x2, y2);
				delay(100);
				rectangle (x1, y1, x2, y2);
				showmouseptr ();
			}
		}
		hidemouseptr ();
		rectangle (x1, y1, x2, y2);
		setwritemode (COPY_PUT);
		setlinestyle (SOLID_LINE, 0, 1);
		showmouseptr ();

		CLIP_X1 = x1;
		CLIP_X2 = x2;
		CLIP_Y1 = y1;
		CLIP_Y2 = y2;

		while (leftbuttonpressed () != 1)
			;
		hidemouseptr ();
		rectangle (x1, y1, x2, y2);
		showmouseptr ();
		setwritemode (COPY_PUT);
		setlinestyle (SOLID_LINE, 0, 1);

		check ();
		if (CLICKED == 1 && SELC != SELECT)
		{
			CLICKED = 0;
			break;
		}
	}
}

void Eraser ()
{
	int x1, y1;

	while (leftbuttonpressed() != 1)
		;

	while (1)
	{
		x1 = x - canvas.left;
		y1 = y - canvas.top;

		hidemouseptr ();

		while (leftbuttonpressed ())
		{
			x1 = x - canvas.left;
			y1 = y - canvas.top;
			if ( (x1 <= canvas.right - canvas.left - 22 && \
			y1 <= canvas.bottom - canvas.top - 22) && (x1 >= 2 \
			&& y1 >= 2) )
			{
				setcolor (BLACK);
				rectangle (x1, y1, x1 + 20, y1 + 20);
				setfillstyle (1, WHITE);
				delay (50);
				bar (x1, y1, x1 + 20, y1 + 20);
			}
		}
		showmouseptr ();
		while (leftbuttonpressed () != 1)
			;
		check ();
		if (CLICKED == 1 && SELC != ERASER)
		{
			CLICKED = 0;
			break;
		}
		check_col ();
	}
}

void Spray ()
{
	int xc, yc, x1, y1;
	int xrad = 6, yrad = 6;
	long double val = 0;

	randomize ();

	while (leftbuttonpressed () != 1)
		;

	while (1)
	{
		while (leftbuttonpressed ())
		{
			xc = x - canvas.left;
			yc = y - canvas.top;
			hidemouseptr ();
			setcolor (CUR_COL);
			x1 = random(xrad);
			y1 = random(yrad);
			val = x1 * x1 * yrad * yrad + y1 * y1 * xrad * xrad -\
			      xrad * xrad * yrad * yrad;

			if(val <= 0)
			{
				if(x1 == 0 || y1 == 0)
				{
					putpixel ((xc + random (xrad + 1)), yc, CUR_COL);
					putpixel ((xc - random (xrad + 1)), yc, CUR_COL);
					putpixel (xc, (yc + random (yrad + 1)), CUR_COL);
					putpixel (xc, (yc - random (yrad + 1)), CUR_COL);
				}
				else
				{
					putpixel ((xc + x1), (yc + y1), CUR_COL);
					delay (2);
					putpixel ((xc + x1), (yc - y1), CUR_COL);
					delay (2);
					putpixel ((xc - x1), (yc + y1), CUR_COL);
					delay (2);
					putpixel ((xc - x1), (yc - y1), CUR_COL);
					delay (2);
				}
			}
			showmouseptr ();
		}
		showmouseptr ();
		while (leftbuttonpressed () != 1)
			;
		check ();
		if (CLICKED == 1 && SELC != SPRAY)
		{
			CLICKED = 0;
			break;
		}
		check_col ();
	 }
}

void Brush ()
{
	int x1, y1;
	while (leftbuttonpressed () != 1)
		;

	while (1)
	{
		while (leftbuttonpressed ())
		{
			x1 = x - canvas.left;
			y1 = y - canvas.top;
			setfillstyle (1, CUR_COL);

			if( ((x1 + 6) <= canvas.right - canvas.left) && \
			((y1 + 6) <= canvas.bottom - canvas.top)  )
			{
				hidemouseptr ();
				bar (x1, y1, x1 + 6, y1 + 6);
			}
			showmouseptr ();
		}
		showmouseptr ();
		while (leftbuttonpressed () != 1)
			;
		check ();
		if (CLICKED == 1 && SELC != BRUSH)
		{
			CLICKED = 0;
			break;
		}
		check_col ();
	}
}

void Text ()
{
	char str[MAX];
	char ch;
	int strtx, x1, y1;
	int pos;
	changecursor (text1);

	do
	{
		getmousepos (&_button, &x, &y);
	} while(_button & 1);

	do
	{
		getmousepos (&_button, &x, &y);
		if(_button & 1)
		{
				if (x >= canvas.left && x <= canvas.right \
				   && y >= canvas.top && y <= canvas.bottom)
				break;
		}
	} while (1);
	setviewport (1, 1, X, Y, 1);
	x1 = strtx = x;
	y1 = y;
	pos = 0;
	str[pos] = '\0';

	hidemouseptr ();
	setcolor (CUR_COL);
	settextstyle (TRIPLEX_FONT, HORIZ_DIR, 1);

	while (pos < MAX - 1)
	{
		ch = getch();

		if (ch == 27)
			break;

		else if (ch == '\r')
		{
			x1 = strtx;
			y1 += textheight (str);
			pos = 0;
			str[pos] = '\0';
			continue;
		}

		str[pos] = ch;
		pos++;
		str[pos] = '\0';
		outtextxy (x1, y1, str);
	}

	settextstyle (DEFAULT_FONT, HORIZ_DIR, 1);
	showmouseptr ();
	changecursor (pointer);
	setviewport (canvas.left, canvas.top, canvas.right, canvas.bottom, 1);
}

void Clip ()
{
	int x1, y1, x2, y2;
	setviewport (1, 1, X, Y, 1);
	if (CLIP_X1 == CLIP_X2)
	{
		dialog ("Error", "Select region for clipping", "");
		return;
	}
	x1 = (CLIP_X1 > CLIP_X2) ? CLIP_X2 : CLIP_X1;
	x2 = (CLIP_X1 > CLIP_X2) ? CLIP_X1 : CLIP_X2;
	y1 = (CLIP_Y1 > CLIP_Y2) ? CLIP_Y2 : CLIP_Y1;
	y2 = (CLIP_Y1 > CLIP_Y2) ? CLIP_Y1 : CLIP_Y2;

	x1 += canvas.left;
	x2 += canvas.left;
	y1 += canvas.top;
	y2 += canvas.top;

	setfillstyle (SOLID_FILL, WHITE);
	hidemouseptr ();

	bar (canvas.left, canvas.top, x1 - 1, canvas.bottom);
	bar (canvas.left, canvas.top, canvas.right, y1 - 1);
	bar (canvas.right, canvas.top, x2 + 1, canvas.bottom);
	bar (canvas.left, y2 + 1, canvas.right, canvas.bottom);
	showmouseptr ();
}

void Cut ()
{
	if (! Copy ()) {
		return;
	}
	setfillstyle (SOLID_FILL, WHITE);
	setviewport (canvas.left, canvas.top, canvas.right, canvas.bottom, 1);
	bar (CLIP_X1, CLIP_Y1, CLIP_X2, CLIP_Y2);
	setviewport (canvas.left, canvas.top, canvas.right, canvas.bottom, 1);
}

int Copy ()
{
	FILE *fp = fopen ("copyBuf", "wb");
	int x1, y1, x2, y2, i, j;
	setviewport (canvas.left, canvas.top, canvas.right, canvas.bottom, 1);
	x1 = (CLIP_X1 > CLIP_X2) ? CLIP_X2 : CLIP_X1;
	x2 = (CLIP_X1 > CLIP_X2) ? CLIP_X1 : CLIP_X2;
	y1 = (CLIP_Y1 > CLIP_Y2) ? CLIP_Y2 : CLIP_Y1;
	y2 = (CLIP_Y1 > CLIP_Y2) ? CLIP_Y1 : CLIP_Y2;

	cpyheight = y2 - y1;
	cpywidth  = x2 - x1;

	hidemouseptr ();
	for (j = y1; j <= y2; j++)
		for (i = x1; i <= x2; i++)
			fputc (getpixel (i, j), fp);
	showmouseptr ();
	fclose (fp);
	return 1;
}

void Paste ()
{
	FILE *fp = fopen ("copyBuf", "rb");
	int i, j;
	if (access ("copyBuf", 00) != 0)
	{
		dialog ("Error", "Nothing to paste.", "TIP: Select region.");
		return;
	}
	setviewport (canvas.left, canvas.top, canvas.right, canvas.bottom, 1);

	do
		getmousepos (&_button, &x, &y);
	while ( _button == 0);
	x -= canvas.left;
	y -= canvas.top;
	hidemouseptr ();
	for (j = y; j <= y + cpyheight; j++)
		for (i = x; i <= x + cpywidth; i ++)
			putpixel (i, j, fgetc (fp));
	showmouseptr ();
	fclose (fp);
}

void Clear ()
{
	hidemouseptr ();
	clearviewport ();
	setfillstyle (1, WHITE);
	bar (1, 1, canvas.right - canvas.left, canvas.bottom - canvas.top);
	showmouseptr ();
}

void Canvs_fill ()
{
	int i = 1,j = 1, col;
	delay (10);

	hidemouseptr ();
	while (1)
	{
		while (j <= canvas.bottom - canvas.top)
		{
			i = 1;
			while (i <= canvas.right - canvas.left)
			{
				col = getpixel (i, j);
				if (col == CANV_COL)
					putpixel (i, j, CUR_COL);

				i++;
			}
			j++;
		}
		CANV_COL = CUR_COL;

		showmouseptr ();
		while (leftbuttonpressed () != 1)
			;
		check ();
		if (CLICKED == 1 && SELC != CANV_FILL)
		{
			CLICKED = 0;
			break;
		}
		check_col ();
		check_pattern ();
	}
}

void Fill (int x1, int y1, int col)
{
	int right, left, i;
	right = x1;
	left = x1 - 1;
	hidemouseptr ();
	while (getpixel (right, y1) == col)
	{
		putpixel (right, y1, CUR_COL);
		right++;
	}
	while (getpixel (left, y1) == col)
	{
		putpixel (left, y1, CUR_COL);
		left--;
	}
	for (i = left + 1; i < right - 1; i++)
	{
		Fill (i, y1 - 1, col);
		Fill (i, y1 + 1, col);
	}
	showmouseptr ();
}

void call_fill ()
{
	int x1, y1, col;

	while (leftbuttonpressed () != 1)
		;
	while (1)
	{
		hidemouseptr ();
		x1 = x - canvas.left;
		y1 = y - canvas.top;

		Fill (x1, y1, WHITE);
		showmouseptr ();

		while (leftbuttonpressed () != 1)
			;
		check ();
		if (CLICKED == 1 && SELC != FILL)
		{
			CLICKED = 0;
			break;
		}
		check_col ();
		check_pattern ();
	}
}

void Save (char* file)
{
	int i, j;
	FILE *fp = fopen (file, "wb");
	setviewport (1, 1, X, Y, 1);
	hidemouseptr ();

	for (j = canvas.top + 1; j < canvas.bottom; j++)
		for (i = canvas.left + 1; i < canvas.right; i++)
			fputc (getpixel (i, j), fp);
	showmouseptr ();
	fclose (fp);
}

void Open (char* file)
{
	int i, j;
	FILE *fp;
	setviewport (1, 1, X, Y, 1);
	if (access (file, 04) != 0)
	{
		dialog ("Error", "Cannot read the file!", "TIP: Make sure the file exists.");
		return;
	}
	else
	{
		fp = fopen (file, "rb");
		for (j = canvas.top + 1; j < canvas.bottom; j++)
			for (i = canvas.left + 1; i < canvas.right; i++)
				putpixel (i, j, fgetc (fp));
	}
}

void Curve ()
{
	int x1, x2, x3, x4;
	int y1, y2, y3, y4;
	int i, j;
	FILE *fp;
	float xt, yt, t;

	do
		getmousepos (&_button, &x1, &y1);
	while (_button == 0);
	x1 -= canvas.left;
	y1 -= canvas.top;
	x = x2 = x1;
	y = y2 = y1;

	setwritemode (XOR_PUT);
	setlinestyle (SOLID_LINE, 0, _thickness);

	do
	{
		getmousepos (&_button, &x, &y);
		x -= canvas.left;
		y -= canvas.top;
		if (x != x2 || y != y2)
		{
			hidemouseptr ();
			line (x1, y1, x2, y2);
			line (x1, y1, x, y);
			showmouseptr ();
			x2 = x;
			y2 = y;
		}
	} while (_button == 1);
	x4 = x2;
	y4 = y2;
	while (_button == 1)
		getmousepos (&_button, &x, &y);
	do
	{
		getmousepos (&_button, &x, &y);
	} while (_button != 1);
	x3 = x2 = x -= canvas.left;
	y3 = y2 = y -= canvas.top;

	hidemouseptr ();
	line (x1, y1, x4, y4);

	for(t = 0; t < 1; t += 0.0005)
	{
		xt = (1 - t) * (1 - t) * (1 - t) * x1 + 3 * t * (1 - t)*
		     (1 - t) * x2 + 3 * t * t * (1 - t) * x3 + t * t * t* x4;
		yt = (1 - t) * (1 - t) * (1 - t) * y1 + 3 * t * (1 - t) *
		     (1 - t) * y2 + 3 * t * t * (1 - t) * y3 + t * t * t * y4;
		putpixel (xt, yt, CUR_COL);
		if (_thickness == 3)
		{
			putpixel (xt + 1, yt + 1, CUR_COL);
			putpixel (xt - 1, yt - 1, CUR_COL);
			putpixel (xt - 1, yt + 1, CUR_COL);
			putpixel (xt + 1, yt - 1, CUR_COL);
		}
	}
	showmouseptr ();

	while (_button == 1)
		getmousepos (&_button, &x, &y);
	do
	{
		getmousepos (&_button, &x, &y);
	} while (_button != 1);
	x3 = x - canvas.left;
	y3 = y - canvas.top;
	hidemouseptr ();

	Open ("temp");
	fclose (fp);

	setcolor (CUR_COL);
	setwritemode (COPY_PUT);
	for(t = 0; t < 1; t += 0.0005)
	{
		xt = (1 - t) * (1 - t) * (1 - t) * x1 + 3 * t * (1 - t) *
		     (1 - t) * x2 + 3 * t * t * (1 - t) * x3 + t * t * t * x4;
		yt = (1 - t) * (1 - t) * (1 - t) * y1 + 3 *t * (1 - t) *
		     (1 - t) * y2 + 3 * t * t * (1 - t) * y3 + t * t * t * y4;
		putpixel (xt, yt, CUR_COL);
		if (_thickness == 3)
		{
			putpixel (xt + 1, yt + 1, CUR_COL);
			putpixel (xt - 1, yt - 1, CUR_COL);
			putpixel (xt - 1, yt + 1, CUR_COL);
			putpixel (xt + 1, yt - 1, CUR_COL);
		}
	}
	while (_button == 1)
		getmousepos (&_button, &x, &y);
	showmouseptr ();
}

void Rotate ()
{
	FILE *fp = fopen ("transBuf", "wb");
	int xt1, yt1, xt2, yt2, x1, y1, x2, y2, i, j;
	float an;
	float cosa, sina;
	float newx, newy, x, y;
	int pix;
	int angle;
	if (CLIP_X1 == CLIP_X2)
	{
		dialog ("Error", "Select region first", "");
		return;
	}
	angle = input ("Input", "Enter the rotation angle");
	setviewport (canvas.left, canvas.top, canvas.right, canvas.bottom, 1);
	xt1 = (CLIP_X1 > CLIP_X2) ? CLIP_X2 : CLIP_X1;
	xt2 = (CLIP_X1 > CLIP_X2) ? CLIP_X1 : CLIP_X2;
	yt1 = (CLIP_Y1 > CLIP_Y2) ? CLIP_Y2 : CLIP_Y1;
	yt2 = (CLIP_Y1 > CLIP_Y2) ? CLIP_Y1 : CLIP_Y2;

	hidemouseptr ();
	for (j = yt1; j <= yt2; j++)
		for (i = xt1; i <= xt2; i++)
			fputc (getpixel (i, j), fp);
	showmouseptr ();
	fclose (fp);

	setviewport (canvas.left, canvas.top, canvas.right, canvas.bottom, 1);
	fp = fopen ("transBuf", "rb");
	hidemouseptr ();
	setfillstyle (SOLID_FILL, WHITE);
	bar (xt1, yt1, xt2, yt2);

	an = (float) angle * M_PI / 180;
	cosa = cos (an);
	sina = sin (an);

	for (j = yt1; j <= yt2; j++)
		for (i = xt1; i <= xt2; i ++)
		{
			x = i - xt1;
			y = abs (j - yt2);
			newx = (float) x * cosa - y * sina;
			newy = (float) x * sina + y * cosa;
			x = newx + xt1;
			y = yt2 - newy;
			pix = fgetc (fp);
			putpixel (x, y, pix);
		}
		showmouseptr ();
		fclose (fp);
}

void Translate ()
{
	FILE *fp = fopen ("transBuf", "wb");
	int sx, sy;
	int xt1, yt1, xt2, yt2, x, y, x1, y1, x2, y2, i, j, height, width;
	int incrx, incry;
	if (CLIP_X1 == CLIP_X2)
	{
		dialog ("Error", "Select region first", "");
		return;
	}
	sx = input ("Input", "Enter the value for x");
	sy = input ("Input", "Enter the value for y");

	incrx = sx;
	incry = sy;

	setviewport (canvas.left, canvas.top, canvas.right, canvas.bottom, 1);
	xt1 = (CLIP_X1 > CLIP_X2) ? CLIP_X2 : CLIP_X1;
	xt2 = (CLIP_X1 > CLIP_X2) ? CLIP_X1 : CLIP_X2;
	yt1 = (CLIP_Y1 > CLIP_Y2) ? CLIP_Y2 : CLIP_Y1;
	yt2 = (CLIP_Y1 > CLIP_Y2) ? CLIP_Y1 : CLIP_Y2;

	height = yt2 - yt1;
	width = xt2 - xt1;

	hidemouseptr ();
	for (j = yt1; j <= yt2; j++)
		for (i = xt1; i <= xt2; i++)
			fputc (getpixel (i, j), fp);
	showmouseptr ();
	fclose (fp);

	setviewport (canvas.left, canvas.top, canvas.right, canvas.bottom, 1);
	fp = fopen ("transBuf", "rb");
	hidemouseptr ();
	setfillstyle (SOLID_FILL, WHITE);
	bar (xt1, yt1, xt2, yt2);
	x = xt1 + incrx;
	y = yt1 - incry;
	for (j = y; j <= y + height; j++)
		for (i = x; i <= x + width; i ++)
			putpixel (i, j, fgetc (fp));
	showmouseptr ();
	fclose (fp);
}

void Scale ()
{
	int sx, sy;
	int k, l, pix, x, ym;
	float newx, newy;
	FILE *fp = fopen ("transBuf", "wb");
	int xt1, yt1, xt2, yt2, x1, y1, x2, y2, i, j;
	if (CLIP_X1 == CLIP_X2)
	{
		dialog ("Error", "Select region first", "");
		return;
	}
	sx = input ("Input", "Enter the x scaling factor");
	sy = input ("Input", "Enter the y scaling factor");

	setviewport (canvas.left, canvas.top, canvas.right, canvas.bottom, 1);
	xt1 = (CLIP_X1 > CLIP_X2) ? CLIP_X2 : CLIP_X1;
	xt2 = (CLIP_X1 > CLIP_X2) ? CLIP_X1 : CLIP_X2;
	yt1 = (CLIP_Y1 > CLIP_Y2) ? CLIP_Y2 : CLIP_Y1;
	yt2 = (CLIP_Y1 > CLIP_Y2) ? CLIP_Y1 : CLIP_Y2;

	hidemouseptr ();
	for (j = yt1; j <= yt2; j++)
		for (i = xt1; i <= xt2; i++)
			fputc (getpixel (i, j), fp);
	showmouseptr ();
	fclose (fp);

	setviewport (canvas.left, canvas.top, canvas.right, canvas.bottom, 1);
	fp = fopen ("transBuf", "rb");
	hidemouseptr ();
	setfillstyle (SOLID_FILL, WHITE);
	bar (xt1, yt1, xt2, yt2);
	for (j = yt1; j <= yt2; j ++)
		for (i = xt1; i <= xt2; i++)
		{
			pix = fgetc (fp);
			x = i - xt1;
			y = abs (j - yt2);
			newx = (float) x * sx;
			newx += xt1;
			for (k = 1; k <= sx; k++)
			{
				newy = (float) y * sy;
				newy = yt2 - newy;
				for (l = 1; l <= sy; l ++)
				{
					putpixel (floor (newx), ceil (newy), pix);
					putpixel (floor (newx), floor (newy), pix);
					putpixel (ceil (newx), floor (newy), pix);
					putpixel (ceil (newx), ceil (newy), pix);
					newy ++;
				}
				newx ++;
			}
		}
	showmouseptr ();
}

void Help ()
{
	char* tmp;
	Save ("tmp");
	setviewport (canvas.left, canvas.top, canvas.right, canvas.bottom, 1);
	Clear ();
	setcolor (BLUE);
	settextstyle (GOTHIC_FONT, HORIZ_DIR, 1);
	outtextxy (200, 10, "miniDraw ");
	settextstyle (TRIPLEX_FONT, HORIZ_DIR, 1);
	outtextxy (270, 10, " Help");
	outtextxy (200, textheight("A"),"컴컴컴컴 컴�");
	setcolor (LIGHTBLUE);
	settextstyle (DEFAULT_FONT, HORIZ_DIR, 0);
	outtextxy (10, 45, " The drawing tools are to the left of the screen.");
	outtextxy (10, 60, " The file, transformation and clipping tools are at the top of ");
	outtextxy (10, 70, "  the screen.");
	outtextxy (10, 85, " The colour and pattern selection can be done from the bottom of ");
	outtextxy (10, 95, "  the screen.");

	outtextxy (10, 120, " To perform drawing operations such as drawing rectangles, lines, ");
	outtextxy (10, 135, "  circles, floodfill, spray, etc., select the appropriate button ");
	outtextxy (10, 150, "  from the drawing tool box (left of screen). Move to the drawing ");
	outtextxy (10, 165, "  area and click and / or drag depending on the operation selected.");
	outtextxy (10, 185, " For image transforms such as scaling, rotation & translate select ");
	outtextxy (10, 200, "  the region and click on the appropriate button on the toolbar.");
	outtextxy (10, 220, " For paste, after cut or copy, click on the paste button. Move the ");
	outtextxy (10, 235, "  mouse pointer to the top left corner of the destination and click.");
	outtextxy (10, 255, " For drawing a filled rectangle and / or a filled ellipse, while ");
	outtextxy (10, 270, "  releasing the mouse button to complete the drawing, click the ");
	outtextxy (10, 285, "  right-mouse button. The result of this operation is a filled ");
	outtextxy (10, 300, "  rectangle or an ellipse with the current fill-pattern.");
	outtextxy (10, 360, "  Press any key to quit help");
	getch ();
	Clear ();
	Open ("tmp");
	showmouseptr ();
}

void About ()
{
	int size;
	char* area = NULL;
	hidemouseptr ();
	size = imagesize (100, 50, 410, 210);
	area = (char *) malloc (size);
	getimage (100, 50, 410, 210, area);
	setfillstyle (SOLID_FILL, BLACK);
	bar (110, 60, 410, 210);
	setfillstyle (SOLID_FILL, LIGHTGRAY);
	bar (100, 50, 400, 200);
	setcolor (WHITE);
	rectangle (101, 51, 399, 199);

	settextstyle(GOTHIC_FONT, HORIZ_DIR, 1);
	setcolor(LIGHTBLUE);
	outtextxy(202, 60, " m ");
	setcolor (BLUE);
	settextstyle (DEFAULT_FONT, HORIZ_DIR, 1);
	outtextxy(217, 69, " ini ");
	settextstyle(GOTHIC_FONT, HORIZ_DIR, 1);
	setcolor (LIGHTRED);
	outtextxy (241, 60, " D ");
	setcolor (RED);
	settextstyle (DEFAULT_FONT, HORIZ_DIR, 1);
	outtextxy (254, 69, " raw ");
	setcolor (BLUE);
	outtextxy (210, 100, " Coded By: ");
	outtextxy (180, 120, " Deepak Vernekar ");
	outtextxy (152, 140, " Ishwarachandra Gowtham ");
	outtextxy (158, 160, " VI Semester CSE, DSCE ");
	getch ();
	putimage (100, 50, area, COPY_PUT);
	free (area);
	showmouseptr ();
}

void call ()
{
	switch (SELC)
	{
		case PEN:
				Pencil ();
				break;
		case LINE:
				Line ();
				break;
		case RECT:
				Rect ();
				break;
		case ELLIPSE:
				Ellipse ();
				break;
		case POLY:
				Polygon ();
				break;
		case SPIRAL:
				Spiral ();
				break;
		case SELECT:
				Select ();
				break;
		case ERASER:
				Eraser ();
				break;
		case SPRAY:
				Spray ();
				break;
		case BRUSH:
				Brush ();
				break;
		case FILL:
				call_fill ();
				break;
		case CURVE:
				Save ("temp");
				Curve ();
				break;
		case TEXT:
				Text ();
				break;
		case CLIP:
				Clip ();
				break;
		case CUT:
				Cut ();
				break;
		case COPY:
				Copy ();
				break;
		case PASTE:
				Paste ();
				break;
		case NEW:
				Clear ();
				break;
		case CANV_FILL:
				Canvs_fill ();
				break;
		case ROTATE:
				Rotate ();
				break;
		case TRANS:
				Translate ();
				break;
		case SCALE:
				Scale ();
				break;
		case SAVE:
				file = get_file ("Input", "Enter the file to be saved");
				Save (file);
				break;
		case OPEN:
				file = get_file ("Input", "Enter the file to be opened");
				Open (file);
				break;
		case HELP:
				Help ();
				break;
		case ABOUT:
				About ();
				break;
	}
}

void main (void)
{
	init ();
	if (initmouse () == 0)
	{
		printf ("\nMouse driver not loaded");
		deInit ();
	}
	prev.left = 0;
	setviewport (0, 0, X, Y, 1);
	screen ();
	showmouseptr ();
	restrictmouseptr (0, 0, X - 2, Y - 2);
	setmousepos (X / 2, Y / 2);
	setviewport (canvas.left, canvas.top, canvas.right, canvas.bottom, 1);

	while (1)
	{
		getmousepos (&_button, &x, &y);
		if ((_button & 1) == 1)
		{
			check_col ();
			check_pattern ();
			check ();
		}
		if (CLICKED)
		{
			CLICKED = 0;
			call ();
		}
	}
}