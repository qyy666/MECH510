//==================================================
// Header file containing error functions
//==================================================
#pragma once
#ifndef HEADER_FILE
#define HEADER_FILE
#endif // !HEADER_FILE

#include "constant.h"

vector<vector<double>> error(vector<vector<double>> num, vector<vector<double>> exact); 	// Standard error
double maxChange(vector<vector<double>> prev, vector<vector<double>> curr);	// Maxximum change between the previous and current timestep
double L2Norm(vector<vector<double>> error);					// L2 Norm of the error
double L1Norm(vector<vector<double>> error);					// L1 Norm of the error
double Linf(vector<vector<double>> error);					// L infinite Norm of the error

vector<vector<double>> error(vector<vector<double>> num, vector<vector<double>> exact)
{
	vector<vector<double>> err(jmax, vector<double>(imax));
	for (int j = 1; j < jmax - 1; j++)
	{
		for (int i = 1; i < imax - 1; i++)
		{
			err[j][i] = exact[j][i] - num[j][i];
		}
	}
	return err;
}

// Calculates the maximum change of all cells between iterations given the change matrix
double maxChange(vector<vector<double>> prev, vector<vector<double>> curr) 
{			

	double maxChange = 0;
	vector<vector<double>> delta(jmax, vector<double>(imax));
	for (int j = 1; j < jmax - 1; j++)
	{
		for (int i = 1; i < imax - 1; i++)
		{
			delta[j][i] = prev[j][i] - curr[j][i];
			if (abs(delta[j][i]) > maxChange) {
				maxChange = delta[j][i];
			}
		}
	}
	return maxChange;
}

double L2Norm(vector<vector<double>> error) 
{			
	double sum = 0;
	for (int j = 1; j < jmax - 1; j++)
	{
		for (int i = 1; i < imax - 1; i++)
		{
			sum += pow(error[j][i], 2);
		}
	}
	return sqrt(sum / ((jmax - 2)*(imax - 2)));
}

double L1Norm(vector<vector<double>> error)
{			
	double sum = 0;
	for (int j = 1; j < jmax - 1; j++)
	{
		for (int i = 1; i < imax - 1; i++)
		{
			sum += abs(error[j][i]);
		}
	}
	return sum / ((jmax - 2)*(imax - 2));
}

double Linf(vector<vector<double>> error)
{			
	double Linf = 0;
	for (int j = 1; j < jmax - 1; j++)
	{
		for (int i = 1; i < imax - 1; i++)
		{
			if (abs(error[j][i]) > Linf)
			{
				Linf = abs(error[j][i]);
			}
		}
	}
	return Linf;
}
