//------------------------------------------------------------------------------
//  GBM alteration by Daniel Edwards
//  File:       locationm.cpp
//
//  Purpose:    Class to provide methods to calculate the location M-estimates
//              of a variety of functions
//
//  History:    31/03/2008 created
//
//------------------------------------------------------------------------------


#include "locationm.h"
#include <algorithm>

using namespace std;


/////////////////////////////////////////////////
// weightedQuantile
//
// Function to return the weighted quantile of
// a vector of a given length
//
// Parameters: iN     - Length of vector
//             adV    - Vector of doubles
//             adW    - Array of weights
//             dAlpha - Quantile to calculate (0.5 for median)
//
// Returns :   Weighted quantile
/////////////////////////////////////////////////
double CLocationM::weightedQuantile(int iN, double *adV, double *adW, double dAlpha)
{

	// Local variables
	int ii, iMedIdx;
	vector<double> vecW;
	vector< pair<int, double> > vecV;
	double dCumSum, dWSum, dMed;

	// Check the vector size
	if (iN == 0)
	{
		return 0.0;
	}
	else if(iN == 1)
	{
		return adV[0];
	}

	// Create vectors containing the values and weights
	vecV.resize(iN);
	for (ii = 0; ii < iN; ii++)
	{
		vecV[ii] = make_pair(ii, adV[ii]);
	}

	// Sort the vector
	std::stable_sort(vecV.begin(), vecV.end(), comp());

	// Sort the weights correspondingly and calculate their sum
	vecW.resize(iN);
	dWSum = 0.0;
	for (ii = 0; ii < iN; ii++)
	{
		vecW[ii] = adW[vecV[ii].first];
		dWSum += adW[ii];
	}

	// Get the first index where the cumulative weight is >=0.5
	iMedIdx = -1;
	dCumSum = 0.0;
	while (dCumSum < dAlpha * dWSum)
	{
	    iMedIdx ++;
		dCumSum += vecW[iMedIdx];
	}

	// Get the index of the next non-zero weight
	int iNextNonZero = iN;
	for (ii = (iN - 1); ii > iMedIdx; ii--)
	{
		if (vecW[ii] > 0)
		{
			iNextNonZero = ii;
		}
	}

	// Use this index unless the cumulative sum is exactly alpha
	if (iNextNonZero == iN || dCumSum > dAlpha * dWSum)
	{
		dMed = vecV[iMedIdx].second;
	}
	else
	{
		dMed = dAlpha * (vecV[iMedIdx].second + vecV[iNextNonZero].second);
	}

	return dMed;
}

/////////////////////////////////////////////////
// PsiFun
//
// Function to calculate the psi of the supplied
// value, given the type of function to use and
// the supplied parameters
//
// Parameters: dX - Value
//
// Returns :   Psi(X)
/////////////////////////////////////////////////
double CLocationM::PsiFun(double dX)
{
	// Local variables
	double dPsiVal = 0.0;

	// Switch on the type of function
	if(msType == "tdist")
	{
		dPsiVal = dX / (madParams[0] + (dX * dX));
	}
	else
	{
		// TODO: Handle the error
	  Rprintf("Error: Function type %s not found\n", msType.c_str());
	}

	return dPsiVal;
}

/////////////////////////////////////////////////
// LocationM
//
// Function to calculate location M estimate for
// the supplied weighted data, with the psi-function
// type and parameters specified in this class
//
// Parameters: iN  - Number of data points
//             adX - Data vector
//             adW - Weight vector
//             dAlpha - Quantile to calculate (0.5 for median)
//
// Returns :   Location M-Estimate of (X, W)
/////////////////////////////////////////////////
double CLocationM::LocationM(int iN, double *adX, double *adW, double dAlpha)
{

	// Local variables
	int ii;

	// Get the initial estimate of location
	double dBeta0 = weightedQuantile(iN, adX, adW, dAlpha);

	// Get the initial estimate of scale
	std::vector<double> adDiff(iN);
	for (ii = 0; ii < iN; ii++)
	{
		adDiff[ii] = fabs(adX[ii] - dBeta0);
	}

	double dScale0 = 1.4826 * weightedQuantile(iN, &adDiff[0], adW, dAlpha);
	dScale0 = fmax(dScale0, mdEps);

	// Loop over until the error is low enough
	double dErr = 1.0;
	int iCount = 0;

	while (iCount < 50)
	{
		double dSumWX = 0.0;
		double dSumW = 0.0;
		for (ii = 0; ii < iN; ii++)
		{
			double dT = fabs(adX[ii] - dBeta0) / dScale0;
			dT = fmax(dT, mdEps);
			double dWt = adW[ii] * PsiFun(dT) / dT;

			dSumWX += dWt * adX[ii];
			dSumW += dWt;
		}

		double dBeta = dBeta0;
		if (dSumW > 0){
			dBeta = dSumWX / dSumW;
		}

		dErr = fabs(dBeta - dBeta0);
		if (dErr > mdEps)
		{
			dErr /= fabs(dBeta0);
		}
		dBeta0 = dBeta;

		if (dErr < mdEps)
		{
			iCount = 100;
		}
		else
		{
			iCount++;
		}

	}


	return dBeta0;
}







