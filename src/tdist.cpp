//  GBM by Greg Ridgeway  Copyright (C) 2003

#include <vector>

#include "tdist.h"




GBMRESULT CTDist::ComputeWorkingResponse
(
    double *adY,
    double *adMisc,
    double *adOffset,
    double *adF,
    double *adZ,
    double *adWeight,
    bool *afInBag,
    unsigned long nTrain,
	int cIdxOff
)
{
    unsigned long i = 0;
	double dU = 0.0;

    if(adOffset == NULL)
    {
        for(i=0; i<nTrain; i++)
        {
		    dU = adY[i] - adF[i];
			adZ[i] = (2 * dU) / (mdNu + (dU * dU));
        }
    }
    else
    {
        for(i=0; i<nTrain; i++)
        {
 		    dU = adY[i] - adOffset[i] - adF[i];
			adZ[i] = (2 * dU) / (mdNu + (dU * dU));
        }
    }

    return GBM_OK;
}


GBMRESULT CTDist::InitF
(
    double *adY,
    double *adMisc,
    double *adOffset,
    double *adWeight,
    double &dInitF,
    unsigned long cLength
)
{

	// Local variables
	int ii;

	// Get objects to pass into the LocM function
	int iN = int(cLength);
	std::vector<double> adArr(cLength);

	for (ii = 0; ii < iN; ii++)
	{
		double dOffset = (adOffset==NULL) ? 0.0 : adOffset[ii];
		adArr[ii] = adY[ii] - dOffset;
	}

	dInitF = mpLocM.LocationM(iN, &adArr[0], adWeight, 0.5);


    return GBM_OK;
}

double CTDist::Deviance
(
    double *adY,
    double *adMisc,
    double *adOffset,
    double *adWeight,
    double *adF,
    unsigned long cLength,
	int cIdxOff
)
{
    unsigned long i=0;
    double dL = 0.0;
    double dW = 0.0;
	double dU = 0.0;

    if(adOffset == NULL)
    {
        for(i=cIdxOff; i<cLength+cIdxOff; i++)
        {
			dU = adY[i] - adF[i];
			dL += adWeight[i] * log(mdNu + (dU * dU));
            dW += adWeight[i];
        }
    }
    else
    {
        for(i=cIdxOff; i<cLength+cIdxOff; i++)
        {
			dU = adY[i] - adOffset[i] - adF[i];
		    dL += adWeight[i] * log(mdNu + (dU * dU));
            dW += adWeight[i];
        }
    }

    return dL/dW;
}


GBMRESULT CTDist::FitBestConstant
(
    double *adY,
    double *adMisc,
    double *adOffset,
    double *adW,
    double *adF,
    double *adZ,
    const std::vector<unsigned long>& aiNodeAssign,
    unsigned long nTrain,
    VEC_P_NODETERMINAL vecpTermNodes,
    unsigned long cTermNodes,
    unsigned long cMinObsInNode,
    bool *afInBag,
    double *adFadj,
	int cIdxOff
)
{
   	// Local variables
    GBMRESULT hr = GBM_OK;
    unsigned long iNode = 0;
    unsigned long iObs = 0;


    std::vector<double> adArr, adWeight;
	// Call LocM for the array of values on each node
    for(iNode=0; iNode<cTermNodes; iNode++)
    {
      if(vecpTermNodes[iNode]->cN >= cMinObsInNode)
        {
	  adArr.clear();
	  adWeight.clear();

	  for (iObs = 0; iObs < nTrain; iObs++)
	    {
	      if(afInBag[iObs] && (aiNodeAssign[iObs] == iNode))
                {
		  const double dOffset = (adOffset==NULL) ? 0.0 : adOffset[iObs];
		  adArr.push_back(adY[iObs] - dOffset - adF[iObs]);
		  adWeight.push_back(adW[iObs]);
                }
	    }

	  vecpTermNodes[iNode]->dPrediction = mpLocM.LocationM(adArr.size(), &adArr[0],
							       &adWeight[0], 0.5);

        }
    }

    return hr;
}

double CTDist::BagImprovement
(
    double *adY,
    double *adMisc,
    double *adOffset,
    double *adWeight,
    double *adF,
    double *adFadj,
    bool *afInBag,
    double dStepSize,
    unsigned long nTrain
)
{
    double dReturnValue = 0.0;
    unsigned long i = 0;
    double dU = 0.0;
    double dW = 0.0;

    for(i=0; i<nTrain; i++)
    {
        if(!afInBag[i])
        {
            const double dF = adF[i] + ((adOffset==NULL) ? 0.0 : adOffset[i]);
	    const double dU = (adY[i] - dF);
	    const double dV = (adY[i] - dF - dStepSize * adFadj[i]) ;

            dReturnValue += adWeight[i] * (log(mdNu + (dU * dU)) - log(mdNu + (dV * dV)));
            dW += adWeight[i];
        }
    }

    return dReturnValue/dW;
}




