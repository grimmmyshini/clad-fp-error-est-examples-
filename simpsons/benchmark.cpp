#include <iomanip>  // necessary for setprecision
#include <iostream> // necessary for 'cout'

#include "benchmark/benchmark.h"

int ITERATIONS;

#include "simpsons.hpp"
#include "simpsons-adapt.hpp"
#include "clad/Differentiator/Differentiator.h"
#include "../PrintModel/ErrorFunc.h"

#include "Derivative.hpp"

#include "adapt.h"


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

static void ErrorEstimateSimp(benchmark::State &state)
{
    ITERATIONS = state.range(0);
    for (auto _ : state)
    {
        double a = 0, b = 1, d_a = 0, d_b = 0, final_error = 0;

        double result_dbl = simpsons(a, b);
        benchmark::DoNotOptimize(result_dbl);
    }
}

static void ErrorEstimateSimpAdapt(benchmark::State &state)
{
    ITERATIONS = state.range(0);
    cout_suppressor suppress;

    for (auto _ : state)
    {
        AD_begin();

        AD_real a = 0, b = 1;

        AD_INDEPENDENT(a, "a");
        AD_INDEPENDENT(b, "b");

        AD_real result = adapt::simpsons(a, b);

        AD_DEPENDENT(result, "result", 0.0);
        AD_report();
        AD_end();
    }
}

static void ErrorEstimateSimpClad(benchmark::State &state)
{
    ITERATIONS = state.range(0);
    // clad::estimate_error(simpsons<double>);
    cout_suppressor suppress;
    
    for (auto _ : state)
    {
        double a = 0, b = 1, d_a = 0, d_b = 0, final_error = 0;

        double result_dbl = simpsons(a, b);
        benchmark::DoNotOptimize(result_dbl);

        clad::resetErrors();
        clad::simpsons_grad(a, b, &d_a, &d_b, final_error);
        clad::printErrorReport();
    }
}

BENCHMARK(ErrorEstimateSimp)->Unit(benchmark::kMillisecond)->RangeMultiplier(10)->Range(10000, 100000000);
BENCHMARK(ErrorEstimateSimpClad)->Unit(benchmark::kMillisecond)->RangeMultiplier(10)->Range(10000, 100000000);
BENCHMARK(ErrorEstimateSimpAdapt)->Unit(benchmark::kMillisecond)->RangeMultiplier(10)->Range(10000, 10000000);

BENCHMARK_MAIN();