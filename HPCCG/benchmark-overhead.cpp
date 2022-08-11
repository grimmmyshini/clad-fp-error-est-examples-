#include <iostream>
using std::cerr;
using std::cout;
using std::endl;
#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <cassert>
#include <string>
#include <cmath>
#include <iomanip>

#include "benchmark/benchmark.h"

#include "clad/Differentiator/Differentiator.h"
#include "../PrintModel/ErrorFunc.h"

#include "generate_matrix.hpp"
#include "read_HPC_row.hpp"
#include "HPC_Sparse_Matrix.hpp"
#include "dump_matlab_matrix.hpp"
#include "ddot.hpp"

#include "HPCCG-clad.hpp"

#undef DEBUG

#define nx 20
#define ny 30
#define nz 160

struct cout_suppressor
{
  cout_suppressor()
      : buffer(), old(std::cout.rdbuf(buffer.rdbuf()))
  {
  }

  ~cout_suppressor()
  {
    std::cout.rdbuf(old);
  }

private:
  std::stringstream buffer;
  std::streambuf *old;
};

static void HPCCG(benchmark::State &state)
{
  double *x, *b, *xexact;
  double norm, d;
  int ierr = 0;
  int i, j;
  int ione = 1;
  double times[7];
  double t6 = 0.0;
  
  generate_matrix(nx, ny, nz, clad::A, &x, &b, &xexact);

  bool dump_matrix = false;
  if (dump_matrix)
    dump_matlab_matrix(clad::A);

  int nrow = clad::A.local_nrow;
  int ncol = clad::A.local_ncol;

  double *r = new double[nrow]();
  double *p = new double[ncol]();
  double *Ap = new double[nrow]();

  double residual;

  for (auto _ : state)
  {
    residual = clad::HPCCG_residual(b, x, xexact, r, p, Ap);
  }

  delete[] p;
  delete[] Ap;
  delete[] r;

  delete[] x;
  delete[] xexact;
  delete[] b;
}

static void HPCCGWithClad(benchmark::State &state)
{
  double *x, *b, *xexact;
  double norm, d;
  int ierr = 0;
  int i, j;
  int ione = 1;
  double times[7];
  double t6 = 0.0;
  
  generate_matrix(nx, ny, nz, clad::A, &x, &b, &xexact);

  bool dump_matrix = false;
  if (dump_matrix)
    dump_matlab_matrix(clad::A);

  int nrow = clad::A.local_nrow;
  int ncol = clad::A.local_ncol;

  double *x_diff = new double[nrow]();
  clad::array_ref<double> d_x(x_diff, nrow);
  
  double *b_diff = new double[nrow]();
  clad::array_ref<double> d_b(b_diff, nrow);

  double *xexact_diff = new double[nrow]();
  clad::array_ref<double> d_xexact(xexact_diff, nrow);

  double *r = new double[nrow]();
  double *r_diff = new double[nrow]();
  clad::array_ref<double> d_r(r_diff, nrow);

  double *p = new double[ncol]();
  double *p_diff = new double[ncol]();
  clad::array_ref<double> d_p(p_diff, ncol);

  double *Ap = new double[nrow]();
  double *Ap_diff = new double[nrow]();
  clad::array_ref<double> d_Ap(Ap_diff, nrow);

  double _final_error = 0, residual;

  cout_suppressor suppressor;
  for (auto _ : state)
  {
    clad::resetErrors();

    residual = clad::HPCCG_residual(b, x, xexact, r, p, Ap);

    HPCCG_residual_grad(b, x, xexact, r, p, Ap, d_b, d_x, d_xexact, d_r, d_p, d_Ap, _final_error);

    cout << "\nFinal error in HPCCG =" << _final_error << endl;

    clad::printErrorReport();
  }

  delete[] b_diff;
  delete[] x_diff;
  delete[] xexact_diff;
  delete[] p;
  delete[] p_diff;
  delete[] Ap;
  delete[] Ap_diff;
  delete[] r;
  delete[] r_diff;

  delete[] x;
  delete[] xexact;
  delete[] b;
}

BENCHMARK(HPCCG)->Unit(benchmark::kSecond);
BENCHMARK(HPCCGWithClad)->Unit(benchmark::kSecond);

// Define our main
BENCHMARK_MAIN();