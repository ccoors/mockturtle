#include <lorina/aiger.hpp>
#include <lorina/verilog.hpp>
#include <mockturtle/algorithms/cleanup.hpp>
#include <mockturtle/algorithms/mig_resub.hpp>
#include <mockturtle/io/aiger_reader.hpp>
#include <mockturtle/io/write_verilog.hpp>
#include <mockturtle/networks/mig.hpp>
#include <mockturtle/views/depth_view.hpp>
#include <mockturtle/views/fanout_view.hpp>

#include <fmt/format.h>
#include <string>

class LiMHDLWriter : public lorina::verilog_writer
{
public:
  explicit LiMHDLWriter( std::ostream& Os ) : verilog_writer( Os ) {}

  void on_assign_maj3( std::string const& out, std::vector<std::pair<bool, std::string>> const& ins ) const override
  {
    assert( ins.size() == 3u );
    const auto a = fmt::format( "{}{}", ins.at( 0 ).first ? "~" : "", ins.at( 0 ).second );
    const auto b = fmt::format( "{}{}", ins.at( 1 ).first ? "" : "~", ins.at( 1 ).second );
    const auto c = fmt::format( "{}{}", ins.at( 2 ).first ? "~" : "", ins.at( 2 ).second );
    _os << fmt::format( "  assign {} = RM3({}, {}, {}) ;\n",
                        out, a, b, c );
  }

  // Hacky way to insert LiMHDLbegin and LiMHDLend
  void on_wire( std::vector<std::string> const& names ) const override
  {
    _os << fmt::format( "  wire {} ;\nLiMHDLbegin\n", fmt::join( names, " , " ) );
  }

  void on_module_end() const override
  {
    _os << "LiMHDLend\nendmodule" << std::endl;
  }
};

int main( int argc, char* argv[] )
{
  using namespace mockturtle;

  if ( argc != 3 )
  {
    fmt::print( "Usage: {} in_file.aig out_file.v\n", argv[0] );
    return EXIT_FAILURE;
  }

  mig_network mig;
  if ( lorina::read_aiger( argv[1], aiger_reader( mig ) ) != lorina::return_code::success )
  {
    fmt::print( "Failed loading {}\n", argv[1] );
    return EXIT_FAILURE;
  }

  resubstitution_params ps;
  resubstitution_stats st;
  ps.progress = true;

  depth_view depth_mig{ mig };
  fanout_view fanout_mig{ depth_mig };

  uint32_t const size_before = fanout_mig.num_gates();
  const auto depth_before = depth_mig.depth();
  mig_resubstitution( fanout_mig, ps, &st );
  mig = cleanup_dangling( mig );

  depth_view depth_resubsituted_mig{ mig };

  fmt::print( "\nSize: {} -> {}\nDepth: {} -> {}\nTook {} seconds\n",
              size_before, mig.num_gates(),
              depth_before, depth_resubsituted_mig.depth(),
              to_seconds( st.time_total ) );

  write_verilog<mig_network, LiMHDLWriter>( mig, argv[2] );

  return EXIT_SUCCESS;
}
