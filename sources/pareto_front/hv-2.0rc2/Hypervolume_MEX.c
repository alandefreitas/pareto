/**************************************************************************
 Hypervolume computation in MATLAB

 MEX Interface created by Guillaume Jacquenot
 guillaume.jacquenot@gmail.com

 ----------------
 Introduction
 ----------------

 This function computes the hypervolume value for a set of points in
 d-dimension.
 All objectives are considered to be minimized.
 Points don't have to non-dominated points.
 This function takes two arguments, the second is optional
    The first one is the matrix of points
        It contains n rows for the different points
        and m columns for the dimension of the points.
    The second argument is a m-dimension vector containing the
    coordinate of the reference point.

 If not provided, the evaluated Nadir point taken as the maximum value
 on all objectives is considered as the reference point.

 The function is called like this:

    HV = Hypervolume_MEX(Objectives);
    HV = Hypervolume_MEX(Objectives,Reference_point);

 ----------------
 Description
 ----------------

 This program implements a recursive, dimension-sweep algorithm for
 computing the hypervolume indicator of the quality of a set of n
 non-dominated points in d dimensions. It also incorporates a recent
 result for the three-dimensional special case. The proposed algorithm
 achieves O(n^{d-2} log n) time and linear space complexity in the
 worst-case, but experimental results show that the pruning techniques
 used may reduce the time complexity even further.

 Relevant literature:

 [1] Carlos M. Fonseca, Luís Paquete, and Manuel López-Ibáñez. An
     improved dimension-sweep algorithm for the hypervolume
     indicator. In IEEE Congress on Evolutionary Computation, pages
     1157-1163, Vancouver, Canada, July 2006.

 [2] Nicola Beume, Carlos M. Fonseca, Manuel Lopez-Ibanez, Luis
     Paquete, and J. Vahrenhold. On the complexity of computing the
     hypervolume indicator. IEEE Transactions on Evolutionary
     Computation, 13(5):1075–1082, 2009.

 ------------
 License
 ------------

 Copyright (C) 2009 - 2011
 MEX Interface created by Guillaume Jacquenot
 guillaume.jacquenot@gmail.com

 Copyright (C) 2006
 Carlos M. Fonseca, Manuel Lopez-Ibanez and Luis Paquete.

 This program is free software (software libre); you can redistribute
 it and/or modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2 of the
 License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 General Public License for more details.

 ------------
 History
 ------------

 This binding is based on the version 1.3 of the original code
 available from:

             http://iridia.ulb.ac.be/~manuel/hypervolume

 ----------------
 Compilation
 ----------------

 To compile the MEX function, you can enter the
    mex -DVARIANT=4 Hypervolume_MEX.c hv.c

 If you want to specify a mex file, you can run
    eval([...
    'mex -DVARIANT=4 -f ' matlabroot '\bin\win32\mexopts\lccopts.bat '...
    'Hypervolume_MEX.c hv.c '])

 ------------
 Usage
 ------------

 %2D Example;
    n = 200;
    theta = linspace(-3*pi/2,-pi,n)';
    FP    = [1+cos(theta) 1-sin(theta)];
    Res  = Hypervolume_MEX(FP,[1 1])
    disp('This result tends to pi/4=0.7854 when n goes to infinity');

 %3D Example;
    n     = 50;
    [X,Y] = meshgrid([0:1/n:1]);
    X=X(:);Y=Y(:);
    R = X.^2+Y.^2;
    Ind = R<=1;
    XX=X(Ind);
    YY=Y(Ind);
    ZZ=sqrt(1-R(Ind));
    Res  = Hypervolume_MEX([XX,YY,ZZ],[1 1 1])
    disp('This result tends to 1-pi/6=0.4764 when n goes to infinity');
    scatter3(XX,YY,ZZ);
*************************************************************************/

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

# include "hv.h"
#include "mex.h"

#if !defined(TRUE) || !defined(FALSE)
#define TRUE 1
#define FALSE 0
#endif


#define MALLOC mxMalloc

/*#define DEBUG*/

void version(void)
{
    printf(
"\nCopyright (C) 2009"
"\nMEX Interface created by Guillaume Jacquenot"
"\n(guillaume.jacquenot@gmail.com)"
"\n"
"\nbased on previous hypervolume software"
"\n<http://iridia.ulb.ac.be/~manuel/hypervolume>"
"\nCopyright (C) 2006"
"\nCarlos M. Fonseca, Manuel Lopez-Ibanez and Luis Paquete\n"
"\n"
"This is free software, and you are welcome to redistribute it under certain\n"
"conditions.  See the GNU General Public License for details. There is NO   \n"
"warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
"\n"        );
}



void description(void)
{
    printf(
"\nThis program implements a recursive, dimension-sweep algorithm for "
"\ncomputing the hypervolume indicator of the quality of a set of n "
"\nnon-dominated points in d dimensions. It also incorporates a "
"\nrecent result for the three-dimensional special case. The proposed "
"\nalgorithm achieves O(n^{d-2} log n) time and linear space complexity "
"\nin the worst-case, but experimental results show that the pruning "
"\ntechniques used may reduce the time complexity even further. "
"\n "
"\nRelevant literature: "
"\n "
"\n[1] Carlos M. Fonseca, Luís Paquete, and Manuel López-Ibáñez. An "
"\n	improved dimension-sweep algorithm for the hypervolume "
"\n	indicator. In IEEE Congress on Evolutionary Computation, pages "
"\n	1157-1163, Vancouver, Canada, July 2006. "
"\n[2] Nicola Beume, Carlos M. Fonseca, Manuel Lopez-Ibanez, Luis "
"\n    Paquete, and J. Vahrenhold. On the complexity of computing the "
"\n    hypervolume indicator. IEEE Transactions on Evolutionary "
"\n    Computation, 13(5):1075–1082, 2009. "
"\n"
"\n"
"\n\n\n ");
}


void help(void)
{
        mexPrintf("\nHypervolume computation");
        mexPrintf("\nThis function computes the hypervolume value for a set of points in d-dimension.");
        mexPrintf("\nAll objectives are considered to be minimized.");
        mexPrintf("\nPoints don't have to non-dominated points.");
        mexPrintf("\nThis function takes two arguments, the second is optional");
        mexPrintf("\n\tThe first one is the matrix of points");
        mexPrintf("\n\t\tIt contains n rows for the different points");
        mexPrintf("\n\t\tand m columns for the dimension of the points.");
        mexPrintf("\n\tThe second argument is a m-dimension vector containing ");
        mexPrintf("\n\tthe coordinate of the reference point");
        mexPrintf("\n\tIf not provided, the evaluated Nadir point taken as the maximum");
        mexPrintf("\n\tvalue on  all objectives is considered as the reference point.");
        mexPrintf("\nThe function is called like this:");
        mexPrintf("\n\tHV = Hypervolume_MEX(Objectives);");
        mexPrintf("\n\tHV = Hypervolume_MEX(Objectives,Reference_point);");
        mexPrintf("\n\n");
        description();
        version();
        mexPrintf("%%2D Example;\n");
        mexPrintf("\tn = 200;\n");
        mexPrintf("\ttheta = linspace(-3*pi/2,-pi,n)';\n");
        mexPrintf("\tFP    = [1+cos(theta) 1-sin(theta)];\n");
        mexPrintf("\tRes  = Hypervolume_MEX(FP,[1 1])\n");
        mexPrintf("\tdisp('This result tends to pi/4=0.7854 when n goes to infinity');\n\n");
        mexPrintf("%%3D Example;\n");
        mexPrintf("\tn     = 50;\n");
        mexPrintf("\t[X,Y] = meshgrid([0:1/n:1]);\n");
        mexPrintf("\tX=X(:);Y=Y(:);\n");
        mexPrintf("\tR = X.^2+Y.^2;\n");
        mexPrintf("\tInd = R<=1;\n");
        mexPrintf("\tXX=X(Ind);\n");
        mexPrintf("\tYY=Y(Ind);\n");
        mexPrintf("\tZZ=sqrt(1-R(Ind));\n");
        mexPrintf("\tRes  = Hypervolume_MEX([XX,YY,ZZ],[1 1 1])\n");
        mexPrintf("\tdisp('This result tends to 1-pi/6=0.4764 when n goes to infinity');\n");
        mexPrintf("\tscatter3(XX,YY,ZZ);\n");
}

double * data_maximum(double *data, int nobj, int rows)
{
    double *vector;
    int n, r, k;

    vector = MALLOC(nobj*sizeof(double));

    for (k = 0; k < nobj; k++)
        vector[k] = data[k];

    for (r = 1 ; r < rows; r++) {
        for (n = 0; n < nobj; n++, k++) {
            if (vector[n] < data[k] )
                vector[n] = data[k];
        }
    }
    return vector;
}


void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    int nobj;               /* Number of objectives */
    int popsize;            /* Number of solutions  */

    double *MatLab_Obj;     /* Pointer to the objective matrix */

    double *reference;      /* Reference vector*/
    double *data;           /* Reshaped vector of MatLab_Obj */
    double *volume = 0;     /* Hypervolume computation */

    int i,j,k;
    if ((nrhs==0)||mxIsChar(prhs[0]))
    {
        help();
        return;
    }
    if (!mxIsNumeric(prhs[0]))
        mexErrMsgTxt("Data points must be a numeric matrix.");
    nobj    = mxGetN(prhs[0]);
    popsize = mxGetM(prhs[0]);
    #ifdef DEBUG
        mexPrintf("nobj=%d\n",nobj);
        mexPrintf("popsize=%d\n",popsize);
    #endif
    MatLab_Obj   = mxGetPr(prhs[0]);

    /* Transpose the objective matrice*/
    data = MALLOC(nobj*popsize*sizeof(double));
    k = 0;
    for (i=0;i<popsize;i++)
        for (j=0;j<nobj;j++)
            data[k++] = MatLab_Obj[i+j*popsize];
    /*
    #ifdef DEBUG
        k = 0;
        for (k=0;k<(nobj*popsize);k++)
        mexPrintf("O[%d]=%f\n",k,data[k]);
    #endif
    */
    if (nrhs == 1)
    {
        reference = data_maximum(data, nobj, popsize);
    }
    else
    {
        if (!mxIsNumeric(prhs[1]))
            mexErrMsgTxt("The reference point must be a numeric vector.");
        if (nobj!=(mxGetM(prhs[1])*mxGetN(prhs[1])))
            mexErrMsgTxt("The reference vector should have the same dimension any point.");
        reference = mxGetPr(prhs[1]);
    }
    plhs[0] = mxCreateDoubleMatrix(1, 1, mxREAL);
     volume = mxGetPr(plhs[0]);
    *volume = fpli_hv(data, nobj, popsize, reference);
    #ifdef DEBUG
        mexPrintf("HyperVolume = %f\n",*volume);
    #endif
    mxFree(data);
}

