/* Code for solving block-tridiagonal matrix problems, using the Thomas
   algorithm.  The only subroutine in here that you'll -need- to call is
   SolveThomas, although things like Add3x3 or AddVec might be useful,
   too. */

/* LHS array is sized as [*][3][3][3].  The last two indices identify
   the element within a block; the third index is the row and the fourth
   is the column of the Jacobian matrix.  The second index tells which
   block it is: 0 is below the main diagonal, 1 is on the main diagonal,
   2 is above the main diagonal.  The first index tells which block row
   you're looking at (the i or j index from the discretization). */

/* RHS array is [*][3].  The second index tells which element of the
   solution vector, and the first is the block row. */

/* Before linking this with your own code, you'll want to remove the
   main program included here as a test. */

#include <math.h>
#include <stdio.h>

#define MAXSIZE (200+2)

static void SpewMatrix(vector<vector<double>> &Source)
{
  printf("%10.6f %10.6f %10.6f\n", Source[0][0], Source[1][0], Source[2][0]);
  printf("%10.6f %10.6f %10.6f\n", Source[0][1], Source[1][1], Source[2][1]);
  printf("%10.6f %10.6f %10.6f\n", Source[0][2], Source[1][2], Source[2][2]);
}

static void SpewVector(vector<double> &Source)
{
  printf("%10.6f %10.6f %10.6f\n", Source[0], Source[1], Source[2]);
}

static inline void CopyVec(const vector<double> &Source, vector<double> &Target)
{
  Target[0] = Source[0];
  Target[1] = Source[1];
  Target[2] = Source[2];
}

static inline void Copy3x3(vector<vector<double>> &Source, vector<vector<double>> &Target)
{
  Target[0][0] = Source[0][0];
  Target[0][1] = Source[0][1];
  Target[0][2] = Source[0][2];

  Target[1][0] = Source[1][0];
  Target[1][1] = Source[1][1];
  Target[1][2] = Source[1][2];

  Target[2][0] = Source[2][0];
  Target[2][1] = Source[2][1];
  Target[2][2] = Source[2][2];
}

static inline void Mult3x3(vector<vector<double>> &A,
			   vector<vector<double>> &B,
			   vector<vector<double>> &C)
{
  C[0][0] = A[0][0]*B[0][0] + A[0][1]*B[1][0] + A[0][2]*B[2][0];
  C[0][1] = A[0][0]*B[0][1] + A[0][1]*B[1][1] + A[0][2]*B[2][1]; 
  C[0][2] = A[0][0]*B[0][2] + A[0][1]*B[1][2] + A[0][2]*B[2][2]; 

  C[1][0] = A[1][0]*B[0][0] + A[1][1]*B[1][0] + A[1][2]*B[2][0]; 
  C[1][1] = A[1][0]*B[0][1] + A[1][1]*B[1][1] + A[1][2]*B[2][1]; 
  C[1][2] = A[1][0]*B[0][2] + A[1][1]*B[1][2] + A[1][2]*B[2][2]; 

  C[2][0] = A[2][0]*B[0][0] + A[2][1]*B[1][0] + A[2][2]*B[2][0]; 
  C[2][1] = A[2][0]*B[0][1] + A[2][1]*B[1][1] + A[2][2]*B[2][1]; 
  C[2][2] = A[2][0]*B[0][2] + A[2][1]*B[1][2] + A[2][2]*B[2][2]; 
}

static inline void MultVec(vector<vector<double>> &A,
			   const vector<double> &Vec,
			   vector<double> &Result)
{
  Result[0] = A[0][0]*Vec[0] + A[0][1]*Vec[1] + A[0][2]*Vec[2]; 
  Result[1] = A[1][0]*Vec[0] + A[1][1]*Vec[1] + A[1][2]*Vec[2]; 
  Result[2] = A[2][0]*Vec[0] + A[2][1]*Vec[1] + A[2][2]*Vec[2]; 
}

static inline void Add3x3(vector<vector<double>> &A,
			  vector<vector<double>> &B,
			  const double Factor,
			  vector<vector<double>> &C)
{
  C[0][0] = A[0][0] + Factor * B[0][0];
  C[0][1] = A[0][1] + Factor * B[0][1];
  C[0][2] = A[0][2] + Factor * B[0][2];

  C[1][0] = A[1][0] + Factor * B[1][0];
  C[1][1] = A[1][1] + Factor * B[1][1];
  C[1][2] = A[1][2] + Factor * B[1][2];

  C[2][0] = A[2][0] + Factor * B[2][0];
  C[2][1] = A[2][1] + Factor * B[2][1];
  C[2][2] = A[2][2] + Factor * B[2][2];
}

static inline void AddVec(const vector<double> &A,
			  const vector<double> &B,
			  const double Factor,
			  vector<double> &C)
{
  C[0] = A[0] + Factor * B[0];
  C[1] = A[1] + Factor * B[1];
  C[2] = A[2] + Factor * B[2];
}

static inline void Invert3x3(vector<vector<double>> &Block,
			     vector<vector<double>> &Inverse)
{
  double DetInv = 1. / (+ Block[0][0]*Block[1][1]*Block[2][2]
			+ Block[0][1]*Block[1][2]*Block[2][0]
			+ Block[0][2]*Block[1][0]*Block[2][1]
			- Block[0][2]*Block[1][1]*Block[2][0]
			- Block[0][1]*Block[1][0]*Block[2][2]
			- Block[0][0]*Block[1][2]*Block[2][1]);

  /* Expand by minors to compute the inverse */
  Inverse[0][0] = + DetInv * (Block[1][1]*Block[2][2] -
			      Block[2][1]*Block[1][2]); 
  Inverse[1][0] = - DetInv * (Block[1][0]*Block[2][2] -
			      Block[2][0]*Block[1][2]); 
  Inverse[2][0] = + DetInv * (Block[1][0]*Block[2][1] -
			      Block[2][0]*Block[1][1]); 
  Inverse[0][1] = - DetInv * (Block[0][1]*Block[2][2] -
			      Block[2][1]*Block[0][2]); 
  Inverse[1][1] = + DetInv * (Block[0][0]*Block[2][2] -
			      Block[2][0]*Block[0][2]); 
  Inverse[2][1] = - DetInv * (Block[0][0]*Block[2][1] -
			      Block[2][0]*Block[0][1]); 
  Inverse[0][2] = + DetInv * (Block[0][1]*Block[1][2] -
			      Block[1][1]*Block[0][2]); 
  Inverse[1][2] = - DetInv * (Block[0][0]*Block[1][2] -
			      Block[1][0]*Block[0][2]); 
  Inverse[2][2] = + DetInv * (Block[0][0]*Block[1][1] -
			      Block[1][0]*Block[0][1]); 
}

void SolveBlockTri(vector<vector<vector<vector<double>>>> &LHS,
		   vector<vector<double>> &RHS,
		   int iNRows)
{
  int j;
  vector<vector<double>> Inv(3, vector<double>(3));

  for (j = 0; j < iNRows-1; j++) {
    /* Compute the inverse of the main block diagonal. */
    Invert3x3(LHS[j][1], Inv);
    /* Scale the right-most block diagonal by the inverse. */
    {
      vector<vector<double>> Temp(3, vector<double>(3));
      Mult3x3(Inv, LHS[j][2], Temp);
      Copy3x3(Temp, LHS[j][2]);
    }

    /* Scale the right-hand side by the inverse. */
    {
      vector<double> Temp(3);
      MultVec(Inv, RHS[j], Temp);
      CopyVec(Temp, RHS[j]);
    }      


    /* Left-multiply the jth row by the sub-diagonal on the j+1st row
       and subtract from the j+1st row.  This involves the
       super-diagonal term and the RHS of the jth row. */
    {
      /* First the LHS manipulation */
#define A LHS[j+1][0]
#define B LHS[j+1][1]
#define C LHS[j  ][2]
      vector<vector<double>> Temp(3, vector<double>(3));
      vector<vector<double>> Temp2(3, vector<double>(3));
      vector<double> TVec(3);
      vector<double> TVec2(3);
      Mult3x3(A, C, Temp);
      Add3x3(B, Temp, -1., Temp2);
      Copy3x3(Temp2, B);

      /* Now the RHS manipulation */
      MultVec(A, RHS[j], TVec);
      AddVec(RHS[j+1], TVec, -1., TVec2);
      CopyVec(TVec2, RHS[j+1]);
#undef A
#undef B
#undef C
    }
  } /* Done with forward elimination loop */
  /* Compute the inverse of the last main block diagonal. */
  j = iNRows-1;
  Invert3x3(LHS[j][1], Inv);

  /* Scale the right-hand side by the inverse. */
  {
    vector<double> Temp(3);
    MultVec(Inv, RHS[j], Temp);
    CopyVec(Temp, RHS[j]);
  }      
  
  /* Now do the back-substitution. */
  for (j = iNRows-2; j >= 0; j--) {
    /* Matrix-vector multiply and subtract. */
#define C LHS[j][2]
    RHS[j][0] -= (C[0][0]*RHS[j+1][0] +
		  C[0][1]*RHS[j+1][1] +
		  C[0][2]*RHS[j+1][2]);
    RHS[j][1] -= (C[1][0]*RHS[j+1][0] +
		  C[1][1]*RHS[j+1][1] +
		  C[1][2]*RHS[j+1][2]);
    RHS[j][2] -= (C[2][0]*RHS[j+1][0] +
		  C[2][1]*RHS[j+1][1] +
		  C[2][2]*RHS[j+1][2]);
#undef C
  }
}

void InitLHS(vector<vector<vector<vector<double>>>> &LHS, const int NRows)
{
  int i;
  for (i = 0; i < NRows; i++) {
    LHS[i][0][0][0] = 1.-2.;
    LHS[i][0][0][1] = 2.;
    LHS[i][0][0][2] = 3.;
    LHS[i][0][1][0] = 4.;
    LHS[i][0][1][1] = 5.-2.;
    LHS[i][0][1][2] = 6.;
    LHS[i][0][2][0] = 7.;
    LHS[i][0][2][1] = 8.;
    LHS[i][0][2][2] = 0.-2.;

    LHS[i][1][0][0] = 1.;
    LHS[i][1][0][1] = 2.;
    LHS[i][1][0][2] = 3.;
    LHS[i][1][1][0] = 4.;
    LHS[i][1][1][1] = 5.;
    LHS[i][1][1][2] = 6.;
    LHS[i][1][2][0] = 7.;
    LHS[i][1][2][1] = 8.;
    LHS[i][1][2][2] = 0.;

    LHS[i][2][0][0] = 1.-3.;
    LHS[i][2][0][1] = 2.;
    LHS[i][2][0][2] = 3.;
    LHS[i][2][1][0] = 4.;
    LHS[i][2][1][1] = 5.-3.;
    LHS[i][2][1][2] = 6.;
    LHS[i][2][2][0] = 7.;
    LHS[i][2][2][1] = 8.;
    LHS[i][2][2][2] = 0.-3.;

  }

  LHS[0][0][0][0] = 
    LHS[0][0][0][1] = 
    LHS[0][0][0][2] = 
    LHS[0][0][1][0] = 
    LHS[0][0][1][1] = 
    LHS[0][0][1][2] = 
    LHS[0][0][2][0] = 
    LHS[0][0][2][1] = 
    LHS[0][0][2][2] = 0.;

  LHS[NRows-1][2][0][0] = 
    LHS[NRows-1][2][0][1] = 
    LHS[NRows-1][2][0][2] = 
    LHS[NRows-1][2][1][0] = 
    LHS[NRows-1][2][1][1] = 
    LHS[NRows-1][2][1][2] = 
    LHS[NRows-1][2][2][0] = 
    LHS[NRows-1][2][2][1] = 
    LHS[NRows-1][2][2][2] = 0.;
}

// int main(void)
// {
//   double LHS[100][3][3][3], RHS[100][3], Check[100][3];
//   double TVec1[3], TVec2[3];
//   int NRows = 20, i;

//   InitLHS(LHS, NRows);
  
//   for (i = 0; i < NRows; i++) {
//     RHS[i][0] = 2*i+1;
//     RHS[i][1] = 2*i+2;
//     RHS[i][2] = 2*i+3;
//   }

//   SolveBlockTri(LHS, RHS, NRows);

//   printf("\nHere's the solution\n\n");
//   for (i = 0; i < NRows; i++) {
//     printf("%d %15.10f %15.10f %15.10f\n",
// 	   i, RHS[i][0], RHS[i][1], RHS[i][2]);
//   }

//   printf("\nChecking result and printing error in Ax - b = 0\n\n");
  
//   InitLHS(LHS, NRows);

//   MultVec(LHS[0][1], RHS[0], TVec1);
//   MultVec(LHS[0][2], RHS[1], TVec2);
//   AddVec(TVec1, TVec2, 1, Check[0]);

//   for (i = 1; i < NRows-1; i++) {
//     MultVec(LHS[i][0], RHS[i-1], Check[i]);
//     MultVec(LHS[i][1], RHS[i], TVec1);
//     AddVec(TVec1, Check[i], 1, TVec2);
//     MultVec(LHS[i][2], RHS[i+1], TVec1);
//     AddVec(TVec1, TVec2, 1, Check[i]);
//   }

//   i = NRows - 1;
//   MultVec(LHS[i][0], RHS[i-1], TVec2);
//   MultVec(LHS[i][1], RHS[i], TVec1);
//   AddVec(TVec1, TVec2, 1, Check[i]);

//   for (i = 0; i < NRows; i++) {
//     printf("%d %15.10g %15.10g %15.10g\n",
// 	   i, Check[i][0] - 2*i - 1, Check[i][1] - 2*i - 2,
// 	   Check[i][2] - 2*i - 3);
//   }
  
// }
