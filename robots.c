
#include <stdio.h>
#include <math.h>
#include <windows.h>
#include <sys/timeb.h>
#include <time.h>


		/*
		** Robot functions
		*/



		/* This robot moves randomly */

void RandomRobot(int FoodClosestDistance,	/* input - closest food in pixels */
			int FoodClosestAngle,		/* input - angle in degrees towards closest food */
			int RobotClosestDistance,	/* input - closest other robot, in pixels */
			int RobotClosestAngle,		/* input - angle in degrees towards closest robot */
			int SharkClosestDistance,	/* input - closest shark in pixels */
			int SharkClosestAngle,		/* input - angle in degrees towards closest shark */
			int CurrentRobotEnergy,		/* input - this robot's current energy (50 - 255) */
			int *RobotMoveAngle,		/* output - angle in degrees to move */
			int *RobotExpendEnergy)		/* output - energy to expend in motion (cannot exceed Current-50) */

{
	int Sk_Fd_RelativeAngle;
	int RBT_RBT_RelativeAngle;
	int mid;
	mid = (100 + SharkClosestAngle) % 360;
	(Sk_Fd_RelativeAngle) = abs(SharkClosestAngle - FoodClosestDistance) % 360;
	(RBT_RBT_RelativeAngle) = abs(*RobotMoveAngle - RobotClosestAngle) % 360;
	if (SharkClosestDistance < 150)
	{
		(*RobotExpendEnergy) = 30;
		if (FoodClosestDistance < 25)
			if (Sk_Fd_RelativeAngle > 90)
				(*RobotMoveAngle) = FoodClosestAngle;
			else
				(*RobotMoveAngle) = (180 + SharkClosestAngle) % 360;
		else
			(*RobotMoveAngle) = (180 + SharkClosestAngle) % 360;
	}
	else
	{
		if (FoodClosestDistance > 150)
		{
			(*RobotExpendEnergy) = 0.7*(30 - SharkClosestDistance / 10);
			(*RobotMoveAngle) = (180 + SharkClosestAngle) % 360;
		}
		else
		{
			if (Sk_Fd_RelativeAngle > 45)
			{
				(*RobotExpendEnergy) = 1.5*(30 - SharkClosestDistance / 10);
				(*RobotMoveAngle) = FoodClosestAngle;
			}
			else if (Sk_Fd_RelativeAngle > 30)
			{
				(*RobotExpendEnergy) = 1.4*(30 - SharkClosestDistance / 20);
				(*RobotMoveAngle) = FoodClosestAngle;
			}
			else
			{
				(*RobotExpendEnergy) = 30 - SharkClosestDistance / 10;
				(*RobotMoveAngle) = (180 + SharkClosestAngle) % 360;
			}
		}
	}
	if ((*RobotExpendEnergy) < 3)
	{
		(*RobotExpendEnergy) = 3;
	}
	if ((CurrentRobotEnergy) > 25)
	{
		if (SharkClosestDistance < 20)
		{
			(*RobotExpendEnergy) = CurrentRobotEnergy;
			if ((RobotClosestAngle > mid))
			{
				(*RobotMoveAngle) = RobotClosestAngle;
			}
			else
			{
				(*RobotMoveAngle) = mid;
			}

		}
	}
	if ((RobotClosestDistance) < 40 && (RBT_RBT_RelativeAngle) < 90)
	{
		(*RobotMoveAngle) = (*RobotMoveAngle + 60) % 360;
	}
}



		/* This robot moves towards food, ignoring sharks */

void TripleSix(int FoodClosestDistance,	/* input - closest food in pixels */
	int FoodClosestAngle,		/* input - angle in degrees towards closest food */
	int RobotClosestDistance,	/* input - closest other robot, in pixels */
	int RobotClosestAngle,		/* input - angle in degrees towards closest robot */
	int SharkClosestDistance,	/* input - closest shark in pixels */
	int SharkClosestAngle,		/* input - angle in degrees towards closest shark */
	int CurrentRobotEnergy,		/* input - this robot's current energy (50 - 255) */
	int *RobotMoveAngle,		/* output - angle in degrees to move */
	int *RobotExpendEnergy)		/* output - energy to expend in motion (cannot exceed Current-50) */

{
	static int Pre_SharkClosestDistance[2] = { 0,0 };
	static int Pre_SharkClosestAngle[2] = { 0,0 };
	int	DangerLevel = 3;
	int SharkVel[2], aver_V;
	int MaxV = 30;
	int DangerDistance = 200;

	if (Pre_SharkClosestDistance[0] * Pre_SharkClosestDistance[1] != 0) {
		/* If previous closest shark is the same as the current closest one */
		if (abs(SharkClosestAngle - Pre_SharkClosestAngle[0]) < 30 && abs(Pre_SharkClosestAngle[0] - Pre_SharkClosestAngle[1]) < 30) {
			SharkVel[0] = SharkClosestDistance - Pre_SharkClosestDistance[0];
			SharkVel[1] = Pre_SharkClosestDistance[0] - Pre_SharkClosestDistance[1];
			aver_V = (SharkVel[0] + SharkVel[1]) / 2;
			/* If the shark moves towards me */
			if (aver_V < 0) {
				if (SharkClosestDistance > DangerDistance) {
					if (FoodClosestDistance * 10 < (CurrentRobotEnergy - 50) - 10)
						DangerLevel = 0;
					else
						DangerLevel = 1;
				}
				else if (abs(FoodClosestAngle - ((180 + SharkClosestAngle) % 360)) < 60)
					DangerLevel = 2;
				else
					DangerLevel = 4;
			}
			else if (CurrentRobotEnergy < 150 && FoodClosestDistance < 200)
				DangerLevel = 0;
			else
				DangerLevel = 1;
		}

		else
			DangerLevel = 4;
	}
	else if (SharkClosestDistance < DangerDistance)
		DangerLevel = 4;
	else
		DangerLevel = 1;

	if (RobotClosestDistance < 10 && DangerLevel != 4)
		DangerLevel = 3;

	switch (DangerLevel)
	{
	case 0:	/* this robot moves towards food when the shark is far away */

		(*RobotMoveAngle) = FoodClosestAngle;
		(*RobotExpendEnergy) = max(3, MaxV - FoodClosestDistance / 5);
		break;
	case 1:	/* this robot slowly moves to the left bottom when nothing to do */
		(*RobotMoveAngle) = 315;
		(*RobotExpendEnergy) = 10;
	case 2: /* this robot moves towards food when the food and the shark are not the same direction */
		(*RobotMoveAngle) = FoodClosestAngle;
		if (SharkClosestDistance < DangerDistance)
			(*RobotExpendEnergy) = MaxV;
		else
			(*RobotExpendEnergy) = MaxV - SharkClosestDistance / 10;
		(*RobotExpendEnergy) = max(3, (*RobotExpendEnergy));
		break;
	case 3: /* this robot moves away from the nearest robot when it is close */
		(*RobotMoveAngle) = (180 + RobotClosestAngle) % 360;
		(*RobotExpendEnergy) = 10;
	case 4:	/* this robot moves away from sharks when the shark is close */
		(*RobotMoveAngle) = (180 + SharkClosestAngle) % 360;
		if (SharkClosestDistance < DangerDistance)
			(*RobotExpendEnergy) = MaxV;
		else
			(*RobotExpendEnergy) = MaxV - SharkClosestDistance / 10;
		(*RobotExpendEnergy) = max(3, (*RobotExpendEnergy));
		break;
	}
	Pre_SharkClosestDistance[1] = Pre_SharkClosestDistance[0];
	Pre_SharkClosestDistance[0] = SharkClosestDistance;
	Pre_SharkClosestAngle[1] = Pre_SharkClosestAngle[0];
	Pre_SharkClosestAngle[0] = SharkClosestAngle;
}


		/* This robot moves away from sharks, ignoring food */

void ScaredRobot(int FoodClosestDistance,	/* input - closest food in pixels */
			int FoodClosestAngle,		/* input - angle in degrees towards closest food */
			int RobotClosestDistance,	/* input - closest other robot, in pixels */
			int RobotClosestAngle,		/* input - angle in degrees towards closest robot */
			int SharkClosestDistance,	/* input - closest shark in pixels */
			int SharkClosestAngle,		/* input - angle in degrees towards closest shark */
			int CurrentRobotEnergy,		/* input - this robot's current energy (50 - 255) */
			int *RobotMoveAngle,		/* output - angle in degrees to move */
			int *RobotExpendEnergy)		/* output - energy to expend in motion (cannot exceed Current-50) */

{
(*RobotMoveAngle)=(180+SharkClosestAngle)%360;
if (SharkClosestDistance < 30)
  (*RobotExpendEnergy)=30;
else
  (*RobotExpendEnergy)=30-SharkClosestDistance/10;
if ((*RobotExpendEnergy) < 3)
  (*RobotExpendEnergy)=3;
}




