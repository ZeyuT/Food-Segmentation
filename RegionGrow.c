#include "stdafx.h"
#include "globals.h"

void RegionGrow(HWND AnimationWindowHandle) {
	int		count;
	HDC		hDC;
	int	r2, c2;
	int	queue[MAX_QUEUE], qh, qt;
	int	average, total;	/* average and total intensity in growing region */
	int centroid_r, centroid_c; /* centroid location in growing region */
	int total_r, total_c;
	int paint_over_label = 0;
	int new_label = 255;


	if (OriginalImage == NULL)
		return;		/* no image to draw */

	count = 0;
	if (c >= 0 && c < COLS  &&  r >= 0 && r < ROWS)
	{
		/* segmentation image = labels; calloc initializes all labels to 0 */
		labels = (unsigned char *)calloc(ROWS*COLS, sizeof(unsigned char));
		/* used to quickly erase small grown regions */
		indices = (int *)calloc(ROWS*COLS, sizeof(int));
		hDC = GetDC(MainWnd);

		if (labels[r*COLS + c] != paint_over_label)
			return;
		labels[r*COLS + c] = new_label;
		average = total = (int)OriginalImage[r*COLS + c];
		centroid_r = total_r = r;
		centroid_c = total_c = c;

		if (indices != NULL)
			indices[0] = r * COLS + c;
		queue[0] = r * COLS + c;
		qh = 1;	/* queue head */
		qt = 0;	/* queue tail */
		count = 1;
		while (qt != qh && RegionGrow_ThreadRunning == 1)
		{
			if (count % 50 == 0)	/* recalculate average after each 50 pixels join */
			{
				average = total / count;
			}
			for (r2 = -1; r2 <= 1; r2++)
				for (c2 = -1; c2 <= 1; c2++)
				{
					centroid_r = total_r / count;
					centroid_c = total_c / count;
					if (r2 == 0 && c2 == 0)
						continue;
					if ((queue[qt] / COLS + r2) < 0 || (queue[qt] / COLS + r2) >= ROWS ||
						(queue[qt] % COLS + c2) < 0 || (queue[qt] % COLS + c2) >= COLS)
						continue;
					if (labels[(queue[qt] / COLS + r2)*COLS + queue[qt] % COLS + c2] != paint_over_label)
						continue;
					/* test criteria to join region */
					/* Test the difference between average intensity to the pixel value*/
					if (abs((int)(OriginalImage[(queue[qt] / COLS + r2)*COLS + queue[qt] % COLS + c2])
						- average) > Intensity_difference)
						continue;
					/* Test the distance from the centroid of the region to the pixel*/
					else if (sqrt(SQR(centroid_r - (queue[qt] / COLS + r2)) + SQR(centroid_c - (queue[qt] % COLS + c2))) > Distance_to_centroid)
						continue;

					labels[(queue[qt] / COLS + r2)*COLS + queue[qt] % COLS + c2] = new_label;
					if (indices != NULL)
						indices[count] = (queue[qt] / COLS + r2)*COLS + queue[qt] % COLS + c2;
					total += OriginalImage[(queue[qt] / COLS + r2)*COLS + queue[qt] % COLS + c2];
					total_r += queue[qt] / COLS + r2;
					total_c += queue[qt] % COLS + c2;
					count++;
					queue[qh] = (queue[qt] / COLS + r2)*COLS + queue[qt] % COLS + c2;
					qh = (qh + 1) % MAX_QUEUE;
					if (qh == qt)
					{
						MessageBox(NULL, "Max queue size exceeded", "Growing Region", MB_OK | MB_APPLMODAL);
						exit(0);
					}

					/* Color labeled pixels*/
					if (RegionGrow_ThreadRunning == 1) {
						while (PlayMode == 0 && StepMode == 0)
							Sleep(1);

						if (PlayMode == 1) {
							///* query timer */
							//t = GetTickCount();
							//while ((t - prev_t) < 1.0) 	/* join pixels each 1 ms */
							//	t = GetTickCount();

							SetPixel(hDC, queue[qt] % COLS + c2, queue[qt] / COLS + r2, RegionColor);
							Sleep(1);
							//prev_t = GetTickCount();
						}
						if (StepMode == 1) {
							/* Wait for next step trigger*/
							while (StepMode == 1 && step_flag == 0)
								Sleep(1);
							SetPixel(hDC, queue[qt] % COLS + c2, queue[qt] / COLS + r2, RegionColor);
							step_flag = 0;
						}
					}
				}
			qt = (qt + 1) % MAX_QUEUE;
		}

		ReleaseDC(MainWnd, hDC);
	}
}