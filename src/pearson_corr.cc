#include "pearson_corr.h"


double PearsonCorr(const double* x, const double* y, int n){
	double xMean = 0, yMean = 0, xSd = 0, ySd = 0, rho = 0;
	int i;
	for(i = 0; i < n; i++){
		xMean += x[i];
		yMean += y[i];
		xSd += x[i] * x[i];
		ySd += y[i] * y[i];
		rho += x[i] * y[i];
	}
	xMean /= n;
	xSd = xSd - n * xMean * xMean;
	xSd = xSd <= 0? 1 : sqrt(xSd);
	yMean /= n;
	ySd = ySd - n * yMean * yMean;
	ySd = ySd <= 0? 1 : sqrt(ySd);
	rho = (rho - n * xMean * yMean) / xSd / ySd;
	return rho;
}



void corR(const double* x, const double* y, int* n, double *cOut){
	double xMean = 0, yMean = 0, xSd = 0, ySd = 0, rho = 0;
	int i;
	for(i = 0; i < *n; i++){
		xMean += x[i];
		yMean += y[i];
		xSd += x[i] * x[i];
		ySd += y[i] * y[i];
		rho += x[i] * y[i];
	}
	xMean /= *n;
	xSd = sqrt(xSd - *n * xMean * xMean);
	yMean /= *n;
	ySd = sqrt(ySd - *n * yMean * yMean);
	rho = (rho - *n * xMean * yMean) / xSd / ySd;
	*cOut = rho;
}

void GetAllCorWz(
    const double *data, 
    const double *vec , 
    const int m, 
    const int n, 
    double *rs){
  double vMean = 0, yMean, vSd = 0, ySd;
  int i, j;
  double y;

  for(j = 0; j < n; j++){
    vMean += vec[j];
    vSd += vec[j] * vec[j];
  }
  vMean /= n;
  vSd = sqrt(vSd - n * vMean * vMean);
  if (fabs(vSd) < 1E-10) {
    for(int i = 0; i < m; i++) rs[i] = 0;
    return;
  }

  for(i = 0; i < m; i++){
    yMean = 0;
    ySd = 0;
    rs[i] = 0;
    for(j = 0; j < n; j++){
      y = data[i + j * m];
      yMean += y;
      ySd += y * y;
      rs[i] += vec[j] * y;
    }
    yMean /= n;
    ySd = sqrt(ySd - n * yMean * yMean);
    if (fabs(ySd) < 1E-10){
      rs[i] = 0;
    } else {
      rs[i] = (rs[i] - n * vMean * yMean) / vSd / ySd;
    }
  }
}

void GetAllCorWzC(const double *data, const double *vec , int *m, int *n, double *rs){
	double vMean = 0, yMean, vSd = 0, ySd;
	int i, j;
	double y;

	for(j = 0; j < *n; j++){
		vMean += vec[j];
		vSd += vec[j] * vec[j];
	}
	vMean /= *n;
	vSd = sqrt(vSd - *n * vMean * vMean);

	for(i = 0; i < *m; i++){
		yMean = 0;
		ySd = 0;
		rs[i] = 0;
		for(j = 0; j < *n; j++){
			y = data[i + j * (*m)];
			yMean += y;
			ySd += y * y;
			rs[i] += vec[j] * y;
		}
		yMean /= *n;
		ySd = sqrt(ySd - *n * yMean * yMean);
		rs[i] = (rs[i] - *n * vMean * yMean) / vSd / ySd;
	}
}

void AllPairwiseCor(
    const double *arr, 
    const int m, 
    const int n, 
    double *pcorr){
  
  for(int i = 0; i < m; i++){
    double x_mean = 0.0;
    double x_sd = 0.0;
    for(int k = 0; k < n; k++){
      x_mean += arr[i + m*k];
      x_sd += arr[i + m*k] * arr[i + m*k];
    }
    x_mean /= n;
    x_sd = sqrt(x_sd - n * x_mean * x_mean);
    
    for(int j = i+1; j < m; j++){
      R_xlen_t idx = (R_xlen_t) TriangularIndex(i, j, m);
      //Rprintf("idx: %d\n", idx);
      pcorr[idx] = 0.0;
      //Rprintf("pass 1\n");

      double y_mean = 0.0;
      double y_sd = 0.0;
      for(int k = 0; k < n; k++){
        y_mean += arr[j + m*k];
        y_sd += arr[j + m*k] * arr[j + m*k];
        pcorr[idx] += arr[i + m*k] * arr[j + m*k];
        //Rprintf("pass 2\n");
      }
      y_mean /= n;
      y_sd = sqrt(y_sd - n * y_mean * y_mean);
      pcorr[idx] = (pcorr[idx] - n * x_mean * y_mean) / x_sd / y_sd;
      //Rprintf("pass 3\n");
    }
  }
}


SEXP AllPairwiseCorCC(
    SEXP arr
    ){
  SEXP dim = getAttrib(arr, R_DimSymbol);
  int m = INTEGER(dim)[0];
  int n = INTEGER(dim)[1];
  double *parr = REAL(arr);
  R_xlen_t npairs = (long) m * (m-1) / 2;
  SEXP out = PROTECT(allocVector(REALSXP, npairs));
  double *pout = REAL(out);
  AllPairwiseCor(parr, m, n, pout);
  UNPROTECT(1);
  return out;
}

/*
SEXP PairwiseCor(
  SEXP data_r, 
  SEXP idx_start_r,
  SEXP all_tasks_r,
  SEXP m_r, 
  SEXP n_r,
  SEXP buffer_exp_r
){
  SEXP out_r;
  double* data;
  double* out;
  long idx_start;
  long all_tasks;
  int m;
  int n;
  int buffer_exp;
  R_len_t ol;

  const int kOutRowNum = 3;
  long buffer_size;
  long i;
  int j;
  double x, y, x_mean, y_mean, x_sd, y_sd, rho;
  int r1, r2;
  
  //Rprintf("GO\n");
  
  PROTECT(data_r = AS_NUMERIC(data_r));
  PROTECT(idx_start_r = AS_INTEGER(idx_start_r));
  PROTECT(all_tasks_r = AS_INTEGER(all_tasks_r));
  PROTECT(m_r = AS_INTEGER(m_r));
  PROTECT(n_r = AS_INTEGER(n_r));
  PROTECT(buffer_exp_r = AS_INTEGER(buffer_exp_r));

  //Rprintf("Finish PROTECT\n");

  data = NUMERIC_POINTER(data_r);
  idx_start = INTEGER_POINTER(idx_start_r)[0];
  all_tasks = INTEGER_POINTER(all_tasks_r)[0];
  m = INTEGER_POINTER(m_r)[0];
  n = INTEGER_POINTER(n_r)[0];
  buffer_exp = INTEGER_POINTER(buffer_exp_r)[0];

  //Rprintf("Finish POINTER\n");
  
  buffer_size = 1 << buffer_exp;
  if(all_tasks - idx_start < buffer_size){
    buffer_size = all_tasks - idx_start;
  }
  ol = (R_len_t) (3 * buffer_size);
  PROTECT(out_r = NEW_NUMERIC(ol));
  out = NUMERIC_POINTER(out_r);
  
  i = 0;
  while (i < buffer_size) {
    r1 = RowIndex(idx_start, m);
    r2 = ColIndex(idx_start, m, r1);
    x_mean = 0;
    y_mean = 0;
    x_sd = 0;
    y_sd = 0;
    rho = 0;
    for(j = 0; j < n; j++) {
      x = data[r1 + j*m];
      y = data[r2 + j*m];
      x_mean += x;
      y_mean += y;
      x_sd += x*x;
      y_sd += y*y;
      rho += x*y;
    }
    x_mean /= n;
    y_mean /= n;
    x_sd = sqrt(x_sd - n * x_mean * x_mean);
    y_sd = sqrt(y_sd - n * y_mean * y_mean);
    rho = x_sd == 0 || y_sd == 0 ? 
      0 : (rho - n * x_mean * y_mean) / x_sd / y_sd;
    //Rprintf("%d\t%d\t%f\n", r1, r2, rho);
    out[kOutRowNum * i] = r1;
    out[kOutRowNum * i + 1] = r2;
    out[kOutRowNum * i + 2] = rho;
    ++idx_start;
    ++i;

  }

  UNPROTECT(7);
  return out_r;
}
*/
