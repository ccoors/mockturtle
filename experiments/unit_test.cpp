#include <mockturtle/io/verilog_reader.hpp>
#include <mockturtle/networks/mig.hpp>
#include <mockturtle/algorithms/aqfp/buffer_insertion.hpp>

#include <lorina/lorina.hpp>
#include <fmt/format.h>
#include <string>

int main()
{
  using namespace mockturtle;

  //std::string file = "benchmarks_aqfp/count.v";
  std::string file = "tmp.v";

  mig_network mig;
  if ( lorina::read_verilog( file, verilog_reader( mig ) ) != lorina::return_code::success )
  {
    //std::cout << "[mockturtle] input file invalid\n";
    return 1;
  }
  //std::cout << "[mockturtle] testing on a network with " << mig.num_gates() << " gates\n";
  
  buffer_insertion_params ps;
  ps.optimization_effort = buffer_insertion_params::optimal;
  ps.assume.splitter_capacity = 3u;
  ps.assume.branch_pis = true;
  ps.assume.balance_pis = true;
  ps.assume.balance_pos = false;
  
  buffer_insertion aqfp( mig, ps );
  aqfp.schedule();
  if ( aqfp.optimize() )
    return 0;
  else
    return 2;
}
