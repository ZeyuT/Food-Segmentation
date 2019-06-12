#include "stdafx.h"
#include "resource.h"
#include "globals.h"

*indices = NULL;
grow_flag = 0;
RegionColor = RGB(0, 0, 255);	/* Default color is blue */
ROWS = 480;		/* actual height and width of resized images*/
COLS = 600;
PRE_ROWS = 0;	/*height and width of the previous images*/
PRE_COLS = 0;
Intensity_difference = 10;
Distance_to_centroid = 200; /* Predicates */
contour_flag = 0;
contour_finish_flag = 0;
balloon_flag = 0;
PointNum = -1;
left_click_flag = 0;
radius = 10;
grab_idx = -1;
iterations = 50;
ThreadRunning_AC = 0;
shift_flag = 0;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
				LPTSTR lpCmdLine, int nCmdShow)

{
MSG			msg;
HWND		hWnd;
WNDCLASS	wc;

wc.style=CS_HREDRAW | CS_VREDRAW;
wc.lpfnWndProc=(WNDPROC)WndProc;
wc.cbClsExtra=0;
wc.cbWndExtra=0;
wc.hInstance=hInstance;
wc.hIcon=LoadIcon(hInstance,"ID_PLUS_ICON");
wc.hCursor=LoadCursor(NULL,IDC_ARROW);
wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
wc.lpszMenuName="ID_MAIN_MENU";
wc.lpszClassName="PLUS";

if (!RegisterClass(&wc))
  return(FALSE);

hWnd=CreateWindow("PLUS","plus program",
		WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,
		CW_USEDEFAULT,0,720,600,NULL,NULL,hInstance,NULL);
if (!hWnd)
  return(FALSE);

ShowScrollBar(hWnd,SB_BOTH,FALSE);
ShowWindow(hWnd,nCmdShow);
UpdateWindow(hWnd);
MainWnd=hWnd;

ShowPixelCoords=0;

strcpy(filename,"");
OriginalImage=NULL;
rows=cols=0;

InvalidateRect(hWnd,NULL,TRUE);
UpdateWindow(hWnd);

while (GetMessage(&msg,NULL,0,0))
  {
  TranslateMessage(&msg);
  DispatchMessage(&msg);
  }
return(msg.wParam);
}


BOOL CALLBACK AboutDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	BOOL bSuccess;
	char str1[10];
	char str2[10];

	switch (Message)
	{
	case WM_INITDIALOG:
		sprintf(str1, "%d", Intensity_difference);
		sprintf(str2, "%d", Distance_to_centroid);
		prev_Intensity_difference = Intensity_difference;
		prev_Distance_to_centroid = Distance_to_centroid;
		SetDlgItemText(hwnd, IDC_EDIT1, str1);
		SetDlgItemText(hwnd, IDC_EDIT2, str2);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hwnd, IDOK);
			break;
		case IDCANCEL:
			EndDialog(hwnd, IDCANCEL);
			Intensity_difference = prev_Intensity_difference;
			Distance_to_centroid = prev_Distance_to_centroid;
			break;
		case IDC_EDIT1:
			Intensity_difference = GetDlgItemInt(hwnd, IDC_EDIT1, &bSuccess, FALSE);
		case IDC_EDIT2:
			Distance_to_centroid = GetDlgItemInt(hwnd, IDC_EDIT2, &bSuccess, FALSE);

		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

LRESULT CALLBACK WndProc (HWND hWnd, UINT uMsg,
		WPARAM wParam, LPARAM lParam)

{
HMENU				hMenu;
OPENFILENAME		ofn;
FILE				*fpt;
HDC					hDC;
char				header[320],text[320];
int					BYTES,xPos,yPos,idx, i, j;
unsigned char		R, G, B;

switch (uMsg)
  {
  case WM_COMMAND:
    switch (LOWORD(wParam))
      {
	  case ID_SHOWPIXELCOORDS:
		ShowPixelCoords=(ShowPixelCoords+1)%2;
		PlayMode = 0;
		StepMode = 0;
		grow_flag = 0;
		PaintImage(OriginalImage, ROWS, COLS);
		break;

	  case ID_COLOR_GREEN:
		Green = (Green + 1) % 2;
		Blue = 0;
		Red = 0;
		RegionColor = RGB(0, 255, 0);
		break;

	  case ID_COLOR_RED:
		Red = (Red + 1) % 2;
		Green = 0;
		Blue = 0;
		RegionColor = RGB(255, 0, 0);
		break;

	  case ID_COLOR_BLUE:
		Blue = (Blue + 1) % 2;
		Green = 0;
		Red = 0;
		RegionColor = RGB(0, 0, 255);
		break;

	  case ID_REGIONGROW_PREDICATE:
		DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ABOUT), hWnd, AboutDlgProc);
		break;

	  case ID_REGIONGROW_PLAYMODE:
		PlayMode = (PlayMode + 1) % 2;
		StepMode = 0;
		ShowPixelCoords = 0;
		if (grow_flag == 0)
		    PaintImage(OriginalImage, ROWS, COLS);
		break;

	  case ID_REGIONGROW_STEPMODE:
		StepMode = (StepMode + 1) % 2;
		PlayMode = 0;
		ShowPixelCoords = 0;
		if (grow_flag == 0)
			PaintImage(OriginalImage, ROWS, COLS);
		break;

	  case ID_CLEAR_CLEAR:
		StepMode = 0;
		PlayMode = 0;
		ShowPixelCoords = 0;
		grow_flag = 0;
		RegionGrow_ThreadRunning = 0;
		ThreadRunning = 0;
		indices = NULL;
		PaintImage(OriginalImage, ROWS, COLS);
		break;

	  case ID_CONTOUR_START:
		  contour_flag = 1;
		  contour_finish_flag = 0;
		  Xcoordinates = (int*)calloc(5000, 1);
		  Ycoordinates = (int*)calloc(5000, 1);
		  PointNum = 0;
		  break;

	  case ID_CONTOUR_RESET:
		  ThreadRunning_AC = 0;
		  contour_flag = 1;
		  contour_finish_flag = 0;
		  memset(Xcoordinates,0, 5000);
		  memset(Ycoordinates,0, 5000);
		  PointNum = 0;
		  grab_idx = -1;
		  PaintImage(OriginalImage, ROWS, COLS);
		  break;

	  case ID_CONTOUR_END:
		  ThreadRunning_AC = 0;
		  contour_flag = 0;
		  contour_finish_flag = 0;
		  free(Xcoordinates);
		  free(Ycoordinates);
		  break;

	  case ID_FILE_LOAD:
		if (OriginalImage != NULL)
		  {
		  free(OriginalImage);
		  OriginalImage=NULL;
		  }
		memset(&(ofn),0,sizeof(ofn));
		ofn.lStructSize=sizeof(ofn);
		ofn.lpstrFile=filename;
		filename[0]=0;
		ofn.nMaxFile=MAX_FILENAME_CHARS;
		ofn.Flags=OFN_EXPLORER | OFN_HIDEREADONLY;
		ofn.lpstrFilter = "PNM files\0*.pnm\0All files\0*.*\0\0";
		if (!( GetOpenFileName(&ofn))  ||  filename[0] == '\0')
		  break;		/* user cancelled load */
		if ((fpt=fopen(filename,"rb")) == NULL)
		  {
		  MessageBox(NULL,"Unable to open file",filename,MB_OK | MB_APPLMODAL);
		  break;
		  }
		fscanf(fpt,"%s %d %d %d",header,&cols,&rows,&BYTES);
		if (strcmp(header, "P5") == 0 && BYTES == 255) {
			OriginalImage = (unsigned char *)calloc(rows*cols, 1);
			header[0] = fgetc(fpt);	/* whitespace character after header */
			fread(OriginalImage, 1, rows*cols, fpt);
			fclose(fpt);
			COLS = cols;
			ROWS = rows;
			SetWindowText(hWnd, filename);
		}
		else if (strcmp(header,"P6") == 0  &&  BYTES == 255){
			ColorImage = (unsigned char *)calloc(rows*cols * 3, 1);
			GrayImage = (unsigned char *)calloc(rows*cols, 1);
			//currentPixel = (struct pixel *)calloc(rows*cols, 3);
			header[0] = fgetc(fpt);	/* whitespace character after header */
			fread(ColorImage, 1, rows*cols * 3, fpt);
			//fread(currentPixel, sizeof(struct pixel), rows*cols, fpt);

			fclose(fpt);
			//ROWS = rows;
			//COLS = cols;
			//OriginalImage = (unsigned char *)calloc(ROWS *COLS, 1);

			//for (i = 0; i < rows*cols; i++){
			//		R = currentPixel[i].R;
			//		G = currentPixel[i].G;
			//		B = currentPixel[i].B;
			//		OriginalImage[i] = (R+G+B)/3;
			//	}
			for (i = 0; i < rows*cols; i++) {
				R = ColorImage[3*i];
				G = ColorImage[3*i+1];
				B = ColorImage[3*i+2];
				GrayImage[i] = (R + G + B)/3;
			}

			if (cols > DES_COLS) {
				COLS = DES_COLS;
				ROWS = (int)(COLS*rows / cols);
			}
			else if (rows > DES_ROWS) {
				ROWS = DES_ROWS;
				COLS = (int)(ROWS*cols / rows);
			}
			else {
				COLS = cols;
				ROWS = rows;
			}
			OriginalImage = (unsigned char *)calloc(ROWS *COLS, 1);
			bilinear_interpolation(GrayImage, rows, cols, OriginalImage, ROWS, COLS);
			SetWindowText(hWnd, filename);
		}
		else
		{
			MessageBox(NULL, "Not a PNM (P6) image or a PPM (P5) image", filename, MB_OK | MB_APPLMODAL);
			fclose(fpt);
			break;
		}
		/* Clear the window*/
		if (PRE_ROWS * PRE_COLS > 0) {
			BlankImage = (unsigned char *)calloc(PRE_ROWS *PRE_COLS, sizeof(unsigned char));
			memset(BlankImage, 255, PRE_ROWS *PRE_COLS);
			PaintImage(BlankImage, PRE_ROWS, PRE_COLS);
		}
		Xcoordinates = (int *)calloc(5000, sizeof(int));
		Ycoordinates = (int *)calloc(5000, sizeof(int));
		PaintImage(OriginalImage, ROWS, COLS);
		PRE_COLS = COLS;
		PRE_ROWS = ROWS;
		if (PRE_ROWS % 4 != 0)
			PRE_ROWS = (PRE_ROWS / 4 + 1) * 4;
		if (PRE_COLS % 4 != 0)
			PRE_COLS = (PRE_COLS / 4 + 1) * 4;
		break;

      case ID_FILE_QUIT:
        DestroyWindow(hWnd);
        break;
      }
    break;

  case WM_SIZE:		  /* could be used to detect when window size changes */
	PaintImage(OriginalImage, ROWS, COLS);
    return(DefWindowProc(hWnd,uMsg,wParam,lParam));
	break;

  case WM_PAINT:
	PaintImage(OriginalImage, ROWS, COLS);
    return(DefWindowProc(hWnd,uMsg,wParam,lParam));
	break;

  case WM_LBUTTONDOWN:
	xPos = c = LOWORD(lParam);
	yPos = r = HIWORD(lParam);
	if (contour_flag == 1) 
	  left_click_flag = 1;
	if (xPos >= 0 && xPos < COLS  &&  yPos >= 0 && yPos < ROWS)
	{
	  sprintf(text, "%d,%d=>%d     ", xPos, yPos, OriginalImage[yPos*COLS + xPos]);
	  hDC = GetDC(MainWnd);
	  TextOut(hDC, 0, 0, text, strlen(text));		/* draw text on the window */
	  ReleaseDC(MainWnd, hDC);
	  if (grow_flag == 0 && (PlayMode == 1 || StepMode == 1)) {
		RegionGrow_ThreadRunning = 1;
		indices = (int *)calloc(ROWS*COLS, sizeof(int));
		_beginthread(RegionGrow, 0, MainWnd);
		grow_flag = 1;
	  }
	  grab_idx = -1;
	  if (contour_flag == 1 && contour_finish_flag == 1)
		for (i = 0; i < PointNum; i++)
		  if (abs(xPos - Xcoordinates[i]) <= 5 && abs(yPos - Ycoordinates[i]) <= 5) {
		    grab_idx = i;
			break;
		  }
	  //char text[100];
	  //sprintf(text, "%d", grab_idx);
	  //MessageBox(NULL, text, "Paint", MB_OK | MB_APPLMODAL);
	}
	return(DefWindowProc(hWnd, uMsg, wParam, lParam));
	break;

  case WM_LBUTTONUP:
	  xPos = LOWORD(lParam);
	  yPos = HIWORD(lParam);
	  left_click_flag = 0;
	  /* Downsampled the contour to every fifth point.*/
	  hDC = GetDC(MainWnd);
	  idx = 0;
	  if (contour_flag == 1 && contour_finish_flag == 0) {
		balloon_flag = 0;
	    while (5 * idx < PointNum) {
		  Xcoordinates[idx] = Xcoordinates[5 * idx];
		  Ycoordinates[idx] = Ycoordinates[5 * idx];
		  SetPixel(hDC, Xcoordinates[idx], Ycoordinates[idx], RGB(0, 0, 255));	/* color the new contour points blue */
		  idx += 1;
		}
		/* Erase redundant points*/
		for (j = idx; j < PointNum; j++)
			Xcoordinates[j] = Ycoordinates[j] = 0;
		PointNum = idx;
	  }
	  if (contour_flag == 1 && contour_finish_flag == 1 && grab_idx >= 0) {
		Xcoordinates[grab_idx] = xPos;
		Ycoordinates[grab_idx] = yPos;
		contour_finish_flag = 0;
	  }
	  ReleaseDC(MainWnd, hDC);
	  if (contour_flag == 1 && PointNum != 0) {
		  ThreadRunning_AC = 1;
		  _beginthread(ActiveContour, 0, MainWnd);
		  contour_finish_flag = 1;
	  }
	  else
		  PaintImage(OriginalImage, ROWS, COLS);
	  return(DefWindowProc(hWnd, uMsg, wParam, lParam));
	  break;

  case WM_RBUTTONDOWN:
	RegionGrow_ThreadRunning = 0;
	grow_flag = 0;

	xPos = c = LOWORD(lParam);
	yPos = r = HIWORD(lParam);
	if (xPos >= 0 && xPos < COLS  &&  yPos >= 0 && yPos < ROWS)
	{
		sprintf(text, "%d,%d=>%d     ", xPos, yPos, OriginalImage[yPos*COLS + xPos]);
		hDC = GetDC(MainWnd);
		TextOut(hDC, 0, 0, text, strlen(text));		/* draw text on the window */
		PointNum = 0;
		if (contour_flag == 1 && contour_finish_flag == 0) {
			balloon_flag = 1;
			for (i = -radius; i <= radius; i++) {
				Ycoordinates[PointNum] = yPos + i;
				Xcoordinates[PointNum] = xPos + (int)sqrt(radius*radius - i * i);
				PaintDot(hDC, Xcoordinates[PointNum], Ycoordinates[PointNum], RGB(0, 255, 0));	/* color the cursor position green */
				if (abs(i) != radius) {
					Ycoordinates[4 * radius - PointNum] = yPos + i;
					Xcoordinates[4 * radius - PointNum] = xPos - (int)sqrt(radius*radius - i * i);
					PaintDot(hDC, Xcoordinates[4 * radius - (PointNum-1)], Ycoordinates[4 * radius - (PointNum-1)], RGB(0, 255, 0));	/* color the cursor position green */
				}
				PointNum += 1;
			}
			PointNum = 4 * radius;
		}
		ReleaseDC(MainWnd, hDC);
	}
    return(DefWindowProc(hWnd,uMsg,wParam,lParam));
	break;	  balloon_flag = 0;


  case WM_RBUTTONUP:
	  /* Downsampled the contour to every third point.*/
	  hDC = GetDC(MainWnd);
	  if (contour_flag==1 && contour_finish_flag == 0) {
		balloon_flag = 1;
		idx = 0;
		while (3 * idx < PointNum) {
		  Xcoordinates[idx] = Xcoordinates[3 * idx];
		  Ycoordinates[idx] = Ycoordinates[3 * idx];
		  idx += 1;
		}
		/* Erase redundant points*/
		for (j = idx; j < PointNum; j++)
			Xcoordinates[j] = Ycoordinates[j] = 0;
		PointNum = idx;
		//for (j = 0; j < PointNum;j++)
		//	fprintf(fpt, "%d %d \r\n", Xcoordinates[j], Ycoordinates[j]);
		//fclose(fpt);
	  }
	  ReleaseDC(MainWnd, hDC);
	  if (contour_flag == 1 && PointNum != 0 && contour_finish_flag == 0) {
		  ThreadRunning_AC = 1;
		  _beginthread(ActiveContour, 0, MainWnd);
		  contour_finish_flag = 1;
	  }
	  else
		PaintImage(OriginalImage, ROWS, COLS);
	  return(DefWindowProc(hWnd, uMsg, wParam, lParam));
	  break;

  case WM_MOUSEMOVE:
	  xPos=LOWORD(lParam);
	  yPos=HIWORD(lParam);
	  if (xPos >= 0 && xPos < COLS && yPos >= 0 && yPos < ROWS)
	  {	
	    hDC = GetDC(MainWnd);
		if (ShowPixelCoords == 1 && OriginalImage!=NULL) {
		  sprintf(text, "%d,%d=>%d     ", xPos, yPos, OriginalImage[yPos*COLS + xPos]);
		  TextOut(hDC, 0, 0, text, strlen(text));		/* draw text on the window */
		  SetPixel(hDC, xPos, yPos, RGB(255, 0, 0));	/* color the cursor position red */
		}

		if (left_click_flag == 1 && contour_finish_flag == 0) {
		  Xcoordinates[PointNum] = xPos;
		  Ycoordinates[PointNum] = yPos;
		  PointNum += 1;
		  PaintDot(hDC, xPos, yPos, RGB(0, 0, 255));	/* color the cursor position blue */
		}

	    ReleaseDC(MainWnd, hDC);
	  }
    return(DefWindowProc(hWnd,uMsg,wParam,lParam));
	break;

  case WM_KEYDOWN:
	if (wParam == 'a' || wParam == 'A')
	  PostMessage(MainWnd, WM_COMMAND, ID_CONTOUR_START, 0);	  /* send message to self */
	if (wParam == 'r' || wParam == 'R')
		PostMessage(MainWnd, WM_COMMAND, ID_CONTOUR_RESET, 0);	  /* send message to self */
	if (wParam == 'e' || wParam == 'E')
		PostMessage(MainWnd, WM_COMMAND, ID_CONTOUR_END, 0);	  /* send message to self */
	if (wParam == 's'  ||  wParam == 'S')
	  PostMessage(MainWnd,WM_COMMAND,ID_SHOWPIXELCOORDS,0);	  /* send message to self */
	if (wParam == 'z' || wParam == 'Z')
		PostMessage(MainWnd, WM_COMMAND, ID_REGIONGROW_PLAYMODE, 0);	  /* send message to self */
	if (wParam == 'x' || wParam == 'X')
		PostMessage(MainWnd, WM_COMMAND, ID_REGIONGROW_STEPMODE, 0);	  /* send message to self */
	if (wParam == 'c' || wParam == 'C')
		PostMessage(MainWnd, WM_COMMAND, ID_CLEAR_CLEAR, 0);	  /* send message to self */
	if (wParam == 'j' || wParam == 'J')
		step_flag = 1;
	if (wParam == MOD_SHIFT)
		shift_flag = 1;
	if ((TCHAR)wParam == '1')
	  {
	  TimerRow=TimerCol=0;
	  SetTimer(MainWnd,TIMER_SECOND,10,NULL);	/* start up 10 ms timer */
	  }
	if ((TCHAR)wParam == '2')
	  {
	  KillTimer(MainWnd,TIMER_SECOND);			/* halt timer, stopping generation of WM_TIME events */
	  PaintImage(OriginalImage, ROWS, COLS);								/* redraw original image, erasing animation */
	  }
	if ((TCHAR)wParam == '3')
	  {
	  ThreadRunning=1;
	  _beginthread(AnimationThread,0,MainWnd);	/* start up a child thread to do other work while this thread continues GUI */
	  }
 	if ((TCHAR)wParam == '4')
	  {
	  ThreadRunning=0;							/* this is used to stop the child thread (see its code below) */
	  }
	return(DefWindowProc(hWnd,uMsg,wParam,lParam));
	break;
  case WM_KEYUP:
	 if (wParam == MOD_SHIFT)
		shift_flag = 0;
  case WM_TIMER:	  /* this event gets triggered every time the timer goes off */
	hDC=GetDC(MainWnd);
	SetPixel(hDC,TimerCol,TimerRow,RGB(0,0,255));	/* color the animation pixel blue */
	ReleaseDC(MainWnd,hDC);
	TimerRow++;
	TimerCol+=2;
	break;
  case WM_HSCROLL:	  /* this event could be used to change what part of the image to draw */
	PaintImage(OriginalImage, ROWS, COLS);	  /* direct PaintImage calls eliminate flicker; the alternative is InvalidateRect(hWnd,NULL,TRUE); UpdateWindow(hWnd); */
    return(DefWindowProc(hWnd,uMsg,wParam,lParam));
	break;
  case WM_VSCROLL:	  /* this event could be used to change what part of the image to draw */
	PaintImage(OriginalImage, ROWS, COLS);
    return(DefWindowProc(hWnd,uMsg,wParam,lParam));
	break;
  case WM_DESTROY:
    PostQuitMessage(0);
    break;
  default:
    return(DefWindowProc(hWnd,uMsg,wParam,lParam));
    break;
  }

hMenu=GetMenu(MainWnd);
if (ShowPixelCoords == 1)
  CheckMenuItem(hMenu,ID_SHOWPIXELCOORDS,MF_CHECKED);
else
  CheckMenuItem(hMenu,ID_SHOWPIXELCOORDS,MF_UNCHECKED);

if (PlayMode == 1)
CheckMenuItem(hMenu, ID_REGIONGROW_PLAYMODE, MF_CHECKED);
else
CheckMenuItem(hMenu, ID_REGIONGROW_PLAYMODE, MF_UNCHECKED);

if (StepMode == 1)
CheckMenuItem(hMenu, ID_REGIONGROW_STEPMODE, MF_CHECKED);
else 
CheckMenuItem(hMenu, ID_REGIONGROW_STEPMODE, MF_UNCHECKED);

if (Green == 1)
CheckMenuItem(hMenu, ID_COLOR_GREEN, MF_CHECKED);
else
CheckMenuItem(hMenu, ID_COLOR_GREEN, MF_UNCHECKED);

if (Red == 1)
CheckMenuItem(hMenu, ID_COLOR_RED, MF_CHECKED);
else
CheckMenuItem(hMenu, ID_COLOR_RED, MF_UNCHECKED);

if (Blue == 1)
CheckMenuItem(hMenu, ID_COLOR_BLUE, MF_CHECKED);
else
CheckMenuItem(hMenu, ID_COLOR_BLUE, MF_UNCHECKED);

if (contour_flag == 1)
CheckMenuItem(hMenu, ID_CONTOUR_START, MF_CHECKED);
else
CheckMenuItem(hMenu, ID_CONTOUR_START, MF_UNCHECKED);


return(0L);
}




void PaintImage(unsigned char* Image, int ROWS, int COLS)
{
PAINTSTRUCT			Painter;
HDC					hDC;
BITMAPINFOHEADER	bm_info_header;
BITMAPINFO			*bm_info;
int					i, j, r, c, DISPLAY_ROWS, DISPLAY_COLS;
unsigned char		*DisplayImage;
COLORREF			cur_color;
if (Image == NULL)
  return;		/* no image to draw */

		/* Windows pads to 4-byte boundaries.  We have to round the size up to 4 in each dimension, filling with black. */
DISPLAY_ROWS=ROWS;
DISPLAY_COLS=COLS;
if (DISPLAY_ROWS % 4 != 0)
  DISPLAY_ROWS=(DISPLAY_ROWS/4+1)*4;
if (DISPLAY_COLS % 4 != 0)
  DISPLAY_COLS=(DISPLAY_COLS/4+1)*4;
DisplayImage=(unsigned char *)calloc(DISPLAY_ROWS*DISPLAY_COLS,1);
for (r=0; r<ROWS; r++)
	for (c = 0; c < COLS; c++)
		DisplayImage[r*DISPLAY_COLS + c] = Image[r*COLS + c];

BeginPaint(MainWnd,&Painter);
hDC=GetDC(MainWnd);
bm_info_header.biSize=sizeof(BITMAPINFOHEADER); 
bm_info_header.biWidth=DISPLAY_COLS;
bm_info_header.biHeight=-DISPLAY_ROWS; 
bm_info_header.biPlanes=1;
bm_info_header.biBitCount=8;    
bm_info_header.biCompression=BI_RGB; 
bm_info_header.biSizeImage=0; 
bm_info_header.biXPelsPerMeter=0; 
bm_info_header.biYPelsPerMeter=0;
bm_info_header.biClrUsed=256;
bm_info_header.biClrImportant=256;
bm_info=(BITMAPINFO *)calloc(1,sizeof(BITMAPINFO) + 256*sizeof(RGBQUAD));
bm_info->bmiHeader=bm_info_header;
for (i=0; i<256; i++)
  {
  bm_info->bmiColors[i].rgbBlue=bm_info->bmiColors[i].rgbGreen=bm_info->bmiColors[i].rgbRed=i;
  bm_info->bmiColors[i].rgbReserved=0;
  } 

SetDIBitsToDevice(hDC,0,0,DISPLAY_COLS,DISPLAY_ROWS,0,0,
			  0, /* first scan line */
			  DISPLAY_ROWS, /* number of scan lines */
			  DisplayImage,bm_info,DIB_RGB_COLORS);

if (indices != NULL) {
	j = 0;
	while (indices[j] != 0) {
		SetPixel(hDC, indices[j] % COLS, indices[j] / COLS, RegionColor);
		j++;
	}
}
if (PointNum > 0) {

	if (balloon_flag == 1)
	  cur_color = RGB(0,255,0);
	else
	  cur_color = RGB(0, 0, 255);
	for (j = 0; j < PointNum; j++)
		PaintDot(hDC, Xcoordinates[j], Ycoordinates[j], cur_color);
}
ReleaseDC(MainWnd,hDC);
EndPaint(MainWnd,&Painter);
free(DisplayImage);
free(bm_info);
}




void AnimationThread(HWND AnimationWindowHandle)

{
HDC		hDC;
char	text[300];

ThreadRow=ThreadCol=0;
while (ThreadRunning == 1)
  {
  hDC=GetDC(MainWnd);
  SetPixel(hDC,ThreadCol,ThreadRow,RGB(0,255,0));	/* color the animation pixel green */
  sprintf(text,"%d,%d     ",ThreadRow,ThreadCol);
  TextOut(hDC,300,0,text,strlen(text));		/* draw text on the window */
  ReleaseDC(MainWnd,hDC);
  ThreadRow+=3;
  ThreadCol++;
  Sleep(100);		/* pause 100 ms */
  }
}

void PaintDot(HDC hDC, int x, int y, COLORREF color) {
	for (int i = -1; i <= 1; i++)
		for (int j = -1; j <= 1; j++)
			SetPixel(hDC, max(0,min(COLS-1,x + i)), max(0,min(ROWS-1,y + j)), color);	/* color the animation pixel blue */
}

