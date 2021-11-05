#include <iostream>
#include <fstream>
#include <algorithm>
#include <array>

enum class Result : int
{
  PASS = 0,
  FAIL = 1,
  INCONCLUSIVE = 2,
};

Result test( std::string const& str )
{
  std::ofstream os( "tmp_model.txt", std::ofstream::out );
  os << str;
  os.close();

  std::array<char, 128> buffer;
  std::string res;
  std::unique_ptr<FILE, decltype( &pclose )> pipe( popen( "z3 tmp_model.txt", "r" ), pclose );
  if ( !pipe )
  {
    throw std::runtime_error( "popen() failed" );
  }
  while ( fgets( buffer.data(), buffer.size(), pipe.get() ) != nullptr )
  {
    res += buffer.data();
  }
  if ( res.substr( 0, 3 ) == "sat" )
  {
    std::cout << "[i] return value is " << res << " (passing)\n";
    return Result::PASS;
  }
  else if ( res.substr( 0, 5 ) == "unsat" )
  {
    std::cout << "[i] return value is " << res << " (failing)\n";
    return Result::FAIL;
  }
  return Result::INCONCLUSIVE;
}

/*
Result test( std::string const& str )
{
  for ( auto i = 0; i < str.length(); ++i )
  {
    if ( str[i] == 'e' )
    {
      return Result::FAIL;
    }
  }
  if ( str[0] == 't' && str[str.size() - 1] == 't' )
  {
    return Result::PASS;
  }
  else
  {
    return Result::INCONCLUSIVE;
  }
}
*/

class DD
{
public:
  explicit DD( std::string const& input )
    : input_( input )
  {}

  std::string min()
  {
    while ( input_.length() >= 2 )
    {      
      const std::size_t subset_length = input_.length() / n;

      int start{0};
      bool some_complement_is_passing{false};
      while ( start < input_.length() )
      {
        std::string complement = input_.substr( 0, start );
        if ( start + subset_length < input_.size() )
        {
          complement += input_.substr( start + subset_length, input_.length() - start - subset_length );
        }

        if ( test( complement ) == Result::FAIL )
        {
          input_ = complement;
          n = std::max( n - 1, 2ul );
          some_complement_is_passing = true;
          break;
        }

        start += subset_length;
      }

      if ( !some_complement_is_passing )
      {
        if ( n == input_.length() )
        {
          break;
        }
        n = std::min( n << 1, input_.length() );
      }
    }

    return input_;
  }

  std::string max()
  {
    while ( input_.length() > 2 )
    {
      const int k = input_.length() / n; /* subset length */

      int start{0};
      bool some_complement_is_passing{false};
      while ( start < input_.length() )
      {
        std::string complement =
          input_.substr( 0, start ) +
          input_.substr( start + k, input_.length() - start - k );

        if ( test( complement ) == Result::PASS )
        {
          input_ = complement;
          n = std::max<int>( n - 1, 2 );
          some_complement_is_passing = true;
        }

        start += k;
      }

      if ( !some_complement_is_passing )
      {
        if ( n == input_.length() )
        {
          break;
        }
        n = std::min<std::size_t>( n << 1, input_.length() );
      }
    }
    return input_;
  }

private:
  std::string input_;
  std::size_t n{2}; // granularity
}; // ddmax

int main()
{
  std::string buffer;

  /* read file into buffer */
  std::ifstream ifs( "model.txt" );
  std::string line;
  while ( std::getline( ifs, line ) )
  {
    buffer += line;
    buffer += '\n';
  }
  ifs.close();

  /* find a minimum failing input */
  std::string ddmin = DD( buffer ).min();
  std::cout << "ddmin:\n" << ddmin << std::endl; std::cout.flush();
  std::ofstream os1( "ddmin.txt", std::ofstream::out );
  os1 << ddmin;
  os1.close();

  /* find a maximum passing input */ 
  /*
  std::string ddmax = DD( buffer ).max();
  std::cout << "ddmax:\n" << ddmax << std::endl;
  std::ofstream os2( "ddmax.v", std::ofstream::out );
  os2 << ddmax;
  os2.close();
  */

  return 0;
}

// http://users.ece.utexas.edu/~miryung/teaching/EE382V-Spring2009/PDF-Lecture%20Slides/Lecture18-DeltaDebugging.pdf
// https://publications.cispa.saarland/3062/1/icse2020-poster-paper42-camera-ready.pdf
