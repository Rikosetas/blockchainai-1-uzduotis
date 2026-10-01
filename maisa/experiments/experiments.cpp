#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include "../maisa/myhash.h"

namespace fs = std::filesystem;

const int ALPHABET_LOW = 33;
const int ALPHABET_HIGH = 126;
const int ALPHABET_SIZE = 94;

struct SM
{
    uint64_t s;

    explicit SM( uint64_t seed )
    {
        s = seed;
    }

    uint64_t next( )
    {
        s += 0x9E3779B97F4A7C15ULL;

        uint64_t z = s;
        z = ( z ^ ( z >> 30 ) ) * 0xBF58476D1CE4E5B9ULL;
        z = ( z ^ ( z >> 27 ) ) * 0x94D049BB133111EBULL;
        z = z ^ ( z >> 31 );

        return z;
    }

    uint32_t below( uint32_t m )
    {
        return ( uint32_t ) ( next( ) % m );
    }
};

std::string random_string( SM& rng, int len )
{
    std::string s( len, ' ' );

    for ( int i = 0; i < len; ++i )
    {
        s[ i ] = ( char ) ( ALPHABET_LOW + rng.below( ALPHABET_SIZE ) );
    }

    return s;
}

int popcount8( uint8_t x )
{
    int count = 0;

    while ( x )
    {
        count += x & 1;
        x >>= 1;
    }

    return count;
}

int hexval( char c )
{
    if ( c >= '0' && c <= '9' )
        return c - '0';
    else
        return c - 'a' + 10;
}

void hex_to_bytes( const std::string& h, uint8_t out[ 32 ] )
{
    for ( int i = 0; i < 32; ++i )
    {
        int hi = hexval( h[ 2 * i ] );
        int lo = hexval( h[ 2 * i + 1 ] );
        out[ i ] = ( uint8_t ) ( ( hi << 4 ) | lo );
    }
}

int bit_diff( const std::string& a, const std::string& b )
{
    uint8_t x[ 32 ];
    uint8_t y[ 32 ];

    hex_to_bytes( a, x );
    hex_to_bytes( b, y );

    int diff = 0;

    for ( int i = 0; i < 32; ++i )
    {
        diff += popcount8( x[ i ] ^ y[ i ] );
    }

    return diff;
}

int hex_diff( const std::string& a, const std::string& b )
{
    int diff = 0;

    for ( int i = 0; i < 64; ++i )
    {
        if ( a[ i ] != b[ i ] )
            diff += 1;
    }

    return diff;
}

std::string HV( const std::string& s, int version )
{
    return myhash::hash( s, version );
}

std::string bytes_to_hex( const std::string& bytes )
{
    static const char* dig = "0123456789abcdef";

    std::string out;

    for ( size_t i = 0; i < bytes.size( ); ++i )
    {
        unsigned char c = ( unsigned char ) bytes[ i ];
        out.push_back( dig[ c >> 4 ] );
        out.push_back( dig[ c & 15 ] );
    }

    return out;
}


void exp1_inputs( )
{
    fs::create_directories( "data/inputs" );

    struct Case
    {
        std::string name;
        std::string bytes;
        std::string note;
    };

    std::vector<Case> cases;

    cases.push_back( { "empty", "", "0 baitu, tuscia" } );
    cases.push_back( { "one_a", "a", "1 baitas" } );
    cases.push_back( { "one_b", "b", "1 baitas" } );

    SM rng( 12345 );
    std::string r = random_string( rng, 2000 );
    cases.push_back( { "rand2000", r, "2000 atsitiktiniu ASCII" } );

    {
        std::string x = r;
        x[ 0 ] = ( x[ 0 ] == 'A' ) ? 'B' : 'A';
        cases.push_back( { "rand2000_first", x, "pakeistas 1-as baitas" } );
    }
    {
        std::string x = r;
        x[ 1000 ] = ( x[ 1000 ] == 'A' ) ? 'B' : 'A';
        cases.push_back( { "rand2000_mid", x, "pakeistas vidurio baitas" } );
    }
    {
        std::string x = r;
        x[ 1999 ] = ( x[ 1999 ] == 'A' ) ? 'B' : 'A';
        cases.push_back( { "rand2000_last", x, "pakeistas paskutinis baitas" } );
    }

    cases.push_back( { "repeat32a", std::string( 32, 'a' ), "pasikartojantys simboliai" } );
    cases.push_back( { "perm_abcdef", "abcdef", "perstatymas A" } );
    cases.push_back( { "perm_fedcba", "fedcba", "perstatymas B" } );
    cases.push_back( { "lead_space", " hello", "tarpas pradzioje" } );
    cases.push_back( { "trail_space", "hello ", "tarpas pabaigoje" } );
    cases.push_back( { "no_newline", "hello", "be naujos eilutes" } );
    cases.push_back( { "with_newline", "hello\n", "su nauja eilute" } );
    cases.push_back( { "utf8", std::string( "\x41\xC4\x8D\x69\xC5\xAB\xE2\x82\xAC" ), "UTF-8: 5 simboliai, 9 baitai" } );

    std::ofstream csv( "results/exp1_inputs.csv" );
    csv << "name,bytes,note,v1,v2\n";

    for ( size_t i = 0; i < cases.size( ); ++i )
    {
        Case& c = cases[ i ];

        std::ofstream f( "data/inputs/" + c.name + ".bin", std::ios::binary );
        f.write( c.bytes.data( ), ( std::streamsize ) c.bytes.size( ) );
        f.close( );

        csv << c.name << "," << c.bytes.size( ) << ",\"" << c.note << "\","
            << HV( c.bytes, 1 ) << "," << HV( c.bytes, 2 ) << "\n";
    }

    std::cout << "[1] Ivestys: " << cases.size( ) << " atveju\n";
}


void exp2_format( )
{
    SM rng( 777 );

    int checked = 0;
    int ok = 0;
    int leading_zero = 0;

    std::ofstream csv( "results/exp2_format.csv" );
    csv << "sample,len_ok,hex_ok,leading_zero,file_eq_mem\n";

    for ( int i = 0; i < 20; ++i )
    {
        int len = 10 + rng.below( 50 );
        std::string in = random_string( rng, len );
        std::string h = HV( in, 2 );

        bool len_ok = ( h.size( ) == 64 );

        bool hex_ok = true;
        for ( size_t k = 0; k < h.size( ); ++k )
        {
            char c = h[ k ];
            bool is_hex = ( c >= '0' && c <= '9' ) || ( c >= 'a' && c <= 'f' );
            if ( !is_hex )
                hex_ok = false;
        }

        bool leading = ( h[ 0 ] == '0' );

        {
            std::ofstream f( "results/_tmp.bin", std::ios::binary );
            f.write( in.data( ), ( std::streamsize ) in.size( ) );
        }

        std::ifstream f( "results/_tmp.bin", std::ios::binary );
        std::string fromfile( ( std::istreambuf_iterator<char>( f ) ), std::istreambuf_iterator<char>( ) );
        bool file_eq = ( HV( fromfile, 2 ) == h );

        checked += 1;

        if ( len_ok && hex_ok && file_eq )
            ok += 1;

        if ( leading )
            leading_zero += 1;

        csv << i << "," << len_ok << "," << hex_ok << "," << leading << "," << file_eq << "\n";
    }

    fs::remove( "results/_tmp.bin" );

    std::cout << "[2] Formatas: " << ok << "/" << checked << " praejo\n";
}


void exp3_determinism( )
{
    std::string A = "Lietuva";
    std::string B = "Vilnius";

    std::string hA = HV( A, 2 );

    bool repeat_ok = true;
    for ( int i = 0; i < 1000; ++i )
    {
        if ( HV( A, 2 ) != hA )
            repeat_ok = false;
    }

    std::string s1 = HV( A, 2 );
    std::string s2 = HV( B, 2 );
    std::string s3 = HV( A, 2 );
    bool seq_ok = ( s1 == s3 ) && ( s1 != s2 );

    std::ofstream csv( "results/exp3_determinism.csv" );
    csv << "check,result\n";
    csv << "repeat_1000x_same," << ( repeat_ok ? "PASS" : "FAIL" ) << "\n";
    csv << "sequence_A_B_A," << ( seq_ok ? "PASS" : "FAIL" ) << "\n";
    csv << "hash_A," << hA << "\n";
    csv << "hash_B," << s2 << "\n";

    std::cout << "[3] Determinizmas: kartojimas=" << ( repeat_ok ? "PASS" : "FAIL" )
              << ", seka A,B,A=" << ( seq_ok ? "PASS" : "FAIL" ) << "\n";
}


struct TimeResult
{
    double mean_ns;
    double min_ns;
    double max_ns;
};

TimeResult measure( const std::string& buf, int which, volatile uint64_t& sink )
{
    const uint8_t* data = ( const uint8_t* ) buf.data( );
    size_t n = buf.size( );

    for ( int w = 0; w < 3; ++w )
    {
        std::string h;
        if ( which == 0 )
            h = myhash::hash_v1( data, n );
        else
            h = myhash::hash_v2( data, n );

        sink += ( uint8_t ) h[ 0 ];
    }

    size_t iters = 2000000 / ( n + 1 );
    if ( iters < 1 )
        iters = 1;
    if ( iters > 200000 )
        iters = 200000;

    double best = 1e18;
    double worst = 0;
    double sum = 0;

    int batches = 7;

    for ( int b = 0; b < batches; ++b )
    {
        auto t0 = std::chrono::steady_clock::now( );

        for ( size_t it = 0; it < iters; ++it )
        {
            std::string h;
            if ( which == 0 )
                h = myhash::hash_v1( data, n );
            else
                h = myhash::hash_v2( data, n );

            sink += ( uint8_t ) h[ 0 ];
        }

        auto t1 = std::chrono::steady_clock::now( );

        double per = std::chrono::duration<double, std::nano>( t1 - t0 ).count( ) / ( double ) iters;

        if ( per < best )
            best = per;
        if ( per > worst )
            worst = per;
        sum += per;
    }

    TimeResult result;
    result.mean_ns = sum / batches;
    result.min_ns = best;
    result.max_ns = worst;

    return result;
}

void exp4_speed( )
{
    std::ifstream in( "data/dataset.txt", std::ios::binary );
    if ( !in )
    {
        std::cout << "[4] KLAIDA: nera data/dataset.txt\n";
        return;
    }

    std::vector<std::string> lines;
    std::string line;
    while ( std::getline( in, line ) )
    {
        lines.push_back( line + "\n" );
    }

    size_t nlines = lines.size( );

    std::vector<size_t> sizes;
    for ( size_t k = 1; k < nlines; k *= 2 )
    {
        sizes.push_back( k );
    }
    sizes.push_back( nlines );

    std::ofstream csv( "results/exp4_speed.csv" );
    csv << "lines,bytes,v1_mean_ns,v1_min_ns,v1_max_ns,v2_mean_ns,v2_min_ns,v2_max_ns\n";
    csv << std::fixed << std::setprecision( 2 );

    volatile uint64_t sink = 0;

    for ( size_t idx = 0; idx < sizes.size( ); ++idx )
    {
        size_t count = sizes[ idx ];

        std::string buf;
        for ( size_t i = 0; i < count; ++i )
            buf += lines[ i ];

        TimeResult v1 = measure( buf, 0, sink );
        TimeResult v2 = measure( buf, 1, sink );

        csv << count << "," << buf.size( ) << ","
            << v1.mean_ns << "," << v1.min_ns << "," << v1.max_ns << ","
            << v2.mean_ns << "," << v2.min_ns << "," << v2.max_ns << "\n";
    }

    std::cout << "[4] Sparta: " << sizes.size( ) << " dydziu (sink=" << sink << ")\n";
}


void exp5_collisions( )
{
    const int PAIRS = 100000;

    std::vector<int> lengths;
    lengths.push_back( 10 );
    lengths.push_back( 100 );
    lengths.push_back( 500 );
    lengths.push_back( 1000 );

    std::ofstream csv( "results/exp5_collisions.csv" );
    csv << "version,length,pairs,pairwise_collisions,distinct_inputs,collision_groups\n";

    std::ofstream examples( "results/exp5_collision_examples.txt" );

    for ( size_t li = 0; li < lengths.size( ); ++li )
    {
        int L = lengths[ li ];

        SM rng( 1000 + L );

        std::vector<std::string> inputs;
        inputs.reserve( 2 * PAIRS );

        for ( int p = 0; p < PAIRS; ++p )
        {
            std::string a = random_string( rng, L );
            std::string b = random_string( rng, L );

            if ( b == a )
            {
                if ( b[ 0 ] == ALPHABET_LOW )
                    b[ 0 ] = ( char ) ( ALPHABET_LOW + 1 );
                else
                    b[ 0 ] = ( char ) ALPHABET_LOW;
            }

            inputs.push_back( a );
            inputs.push_back( b );
        }

        for ( int v = 1; v <= 2; ++v )
        {
            std::vector<std::string> H( inputs.size( ) );
            for ( size_t i = 0; i < inputs.size( ); ++i )
                H[ i ] = HV( inputs[ i ], v );

            int pairwise = 0;
            for ( int p = 0; p < PAIRS; ++p )
            {
                if ( H[ 2 * p ] == H[ 2 * p + 1 ] )
                {
                    pairwise += 1;
                    examples << "PAIR v" << v << " L" << L << ": "
                             << inputs[ 2 * p ] << " | " << inputs[ 2 * p + 1 ] << "\n";
                }
            }

            std::vector<int> idx( H.size( ) );
            for ( size_t i = 0; i < idx.size( ); ++i )
                idx[ i ] = ( int ) i;

            std::sort( idx.begin( ), idx.end( ), [ &H ]( int a, int b )
            {
                return H[ a ] < H[ b ];
            } );

            int groups = 0;
            size_t i = 0;
            while ( i < idx.size( ) )
            {
                size_t j = i;
                while ( j < idx.size( ) && H[ idx[ j ] ] == H[ idx[ i ] ] )
                    j += 1;

                if ( j - i > 1 )
                {
                    bool different = false;
                    for ( size_t a = i; a < j && !different; ++a )
                    {
                        for ( size_t b = a + 1; b < j; ++b )
                        {
                            if ( inputs[ idx[ a ] ] != inputs[ idx[ b ] ] )
                            {
                                different = true;
                                break;
                            }
                        }
                    }

                    if ( different )
                    {
                        groups += 1;
                        examples << "SET v" << v << " L" << L << ": " << inputs[ idx[ i ] ] << " ...\n";
                    }
                }

                i = j;
            }

            std::vector<std::string> uniq( inputs );
            std::sort( uniq.begin( ), uniq.end( ) );
            int distinct = ( int ) ( std::unique( uniq.begin( ), uniq.end( ) ) - uniq.begin( ) );

            csv << "v" << v << "," << L << "," << PAIRS << "," << pairwise << ","
                << distinct << "," << groups << "\n";
        }

        std::cout << "[5] Kolizijos L=" << L << " baigta\n";
    }

    examples << "\n-- Strukturuoti bandymai --\n";

    std::vector<std::string> structured;
    structured.push_back( "abcdefghij" );
    structured.push_back( "jihgfedcba" );
    structured.push_back( "aaaaabbbbb" );
    structured.push_back( "bbbbbaaaaa" );
    structured.push_back( "0123456789" );
    structured.push_back( "9876543210" );
    structured.push_back( "ababababab" );
    structured.push_back( "bababababa" );

    for ( int v = 1; v <= 2; ++v )
    {
        std::vector<std::string> H;
        for ( size_t i = 0; i < structured.size( ); ++i )
            H.push_back( HV( structured[ i ], v ) );

        int collisions = 0;
        for ( size_t a = 0; a < structured.size( ); ++a )
        {
            for ( size_t b = a + 1; b < structured.size( ); ++b )
            {
                if ( H[ a ] == H[ b ] && structured[ a ] != structured[ b ] )
                {
                    collisions += 1;
                    examples << "STRUCT v" << v << ": " << structured[ a ] << " = " << structured[ b ] << "\n";
                }
            }
        }

        examples << "structured collisions v" << v << ": " << collisions << "\n";
    }

    std::cout << "[5] Kolizijos baigtos\n";
}


void exp6_avalanche( )
{
    std::vector<int> lengths;
    lengths.push_back( 10 );
    lengths.push_back( 100 );
    lengths.push_back( 500 );
    lengths.push_back( 1000 );

    const int PER = 25000;

    std::ofstream csv( "results/exp6_avalanche.csv" );
    csv << std::fixed << std::setprecision( 4 );
    csv << "version,length,bit_min,bit_max,bit_mean,hex_min,hex_max,hex_mean\n";

    std::vector<long long> hist_v1( 257, 0 );
    std::vector<long long> hist_v2( 257, 0 );

    double bit_percent = 100.0 / 256.0;
    double hex_percent = 100.0 / 64.0;

    for ( int v = 1; v <= 2; ++v )
    {
        int overall_bit_min = 256;
        int overall_bit_max = 0;
        double overall_bit_sum = 0;

        int overall_hex_min = 64;
        int overall_hex_max = 0;
        double overall_hex_sum = 0;

        long long overall_count = 0;

        for ( size_t li = 0; li < lengths.size( ); ++li )
        {
            int L = lengths[ li ];

            SM rng( 5000 + L + v * 100000 );

            int bit_min = 256;
            int bit_max = 0;
            double bit_sum = 0;

            int hex_min = 64;
            int hex_max = 0;
            double hex_sum = 0;

            for ( int p = 0; p < PER; ++p )
            {
                std::string base = random_string( rng, L );
                std::string var = base;

                int pos = ( int ) rng.below( L );

                char nc;
                do
                {
                    nc = ( char ) ( ALPHABET_LOW + rng.below( ALPHABET_SIZE ) );
                }
                while ( nc == var[ pos ] );

                var[ pos ] = nc;

                std::string h1 = HV( base, v );
                std::string h2 = HV( var, v );

                int bd = bit_diff( h1, h2 );
                int hd = hex_diff( h1, h2 );

                if ( bd < bit_min ) bit_min = bd;
                if ( bd > bit_max ) bit_max = bd;
                bit_sum += bd;

                if ( hd < hex_min ) hex_min = hd;
                if ( hd > hex_max ) hex_max = hd;
                hex_sum += hd;

                if ( v == 1 )
                    hist_v1[ bd ] += 1;
                else
                    hist_v2[ bd ] += 1;

                if ( bd < overall_bit_min ) overall_bit_min = bd;
                if ( bd > overall_bit_max ) overall_bit_max = bd;
                overall_bit_sum += bd;

                if ( hd < overall_hex_min ) overall_hex_min = hd;
                if ( hd > overall_hex_max ) overall_hex_max = hd;
                overall_hex_sum += hd;

                overall_count += 1;
            }

            csv << "v" << v << "," << L << ","
                << bit_min * bit_percent << "," << bit_max * bit_percent << "," << ( bit_sum / PER ) * bit_percent << ","
                << hex_min * hex_percent << "," << hex_max * hex_percent << "," << ( hex_sum / PER ) * hex_percent << "\n";
        }

        csv << "v" << v << ",all,"
            << overall_bit_min * bit_percent << "," << overall_bit_max * bit_percent << "," << ( overall_bit_sum / overall_count ) * bit_percent << ","
            << overall_hex_min * hex_percent << "," << overall_hex_max * hex_percent << "," << ( overall_hex_sum / overall_count ) * hex_percent << "\n";

        std::cout << "[6] Lavina v" << v << ": vid. bitu skirtumas = "
                  << std::fixed << std::setprecision( 2 ) << ( overall_bit_sum / overall_count ) * bit_percent << "%\n";
    }

    std::ofstream hist_file( "results/exp6_bithist.csv" );
    hist_file << "bit_diff,v1_count,v2_count\n";
    for ( int i = 0; i <= 256; ++i )
        hist_file << i << "," << hist_v1[ i ] << "," << hist_v2[ i ] << "\n";

    std::cout << "[6] Lavina baigta\n";
}


void exp7_guessing( )
{
    std::string target = "4242";

    std::ofstream csv( "results/exp7_guessing.csv" );
    csv << "version,scenario,attempts_to_first_match,total_candidates,total_time_ms,matches\n";
    csv << std::fixed << std::setprecision( 3 );

    std::string salt;
    {
        SM rng( 2026 );
        for ( int i = 0; i < 8; ++i )
            salt.push_back( ( char ) rng.below( 256 ) );
    }

    std::string secret_r;
    {
        SM rng( 999 );
        for ( int i = 0; i < 8; ++i )
            secret_r.push_back( ( char ) rng.below( 256 ) );
    }

    std::ofstream params( "results/exp7_params.txt" );
    params << "target=" << target << "\n";
    params << "salt(hex)=" << bytes_to_hex( salt ) << "\n";
    params << "secret_r(hex)=" << bytes_to_hex( secret_r ) << "\n";

    for ( int v = 1; v <= 2; ++v )
    {
        {
            std::string target_hash = HV( target, v );

            auto t0 = std::chrono::steady_clock::now( );

            int matches = 0;
            int first = -1;

            for ( int c = 0; c < 10000; ++c )
            {
                char buf[ 5 ];
                snprintf( buf, 5, "%04d", c );

                if ( HV( std::string( buf ), v ) == target_hash )
                {
                    matches += 1;
                    if ( first < 0 )
                        first = c + 1;
                }
            }

            auto t1 = std::chrono::steady_clock::now( );
            double ms = std::chrono::duration<double, std::milli>( t1 - t0 ).count( );

            csv << "v" << v << ",no_salt," << first << ",10000," << ms << "," << matches << "\n";
        }

        {
            std::string target_hash = HV( target + salt, v );

            auto t0 = std::chrono::steady_clock::now( );

            int matches = 0;
            int first = -1;

            for ( int c = 0; c < 10000; ++c )
            {
                char buf[ 5 ];
                snprintf( buf, 5, "%04d", c );

                if ( HV( std::string( buf ) + salt, v ) == target_hash )
                {
                    matches += 1;
                    if ( first < 0 )
                        first = c + 1;
                }
            }

            auto t1 = std::chrono::steady_clock::now( );
            double ms = std::chrono::duration<double, std::milli>( t1 - t0 ).count( );

            csv << "v" << v << ",public_salt," << first << ",10000," << ms << "," << matches << "\n";
        }

        {
            std::string commit = HV( target + secret_r, v );
            bool verify = ( HV( target + secret_r, v ) == commit );

            csv << "v" << v << ",secret_r_reveal_verify," << ( verify ? 1 : 0 ) << ",NA,0," << ( verify ? 1 : 0 ) << "\n";
        }
    }

    std::cout << "[7] Spejimas baigtas\n";
}


int main( )
{
    fs::create_directories( "results" );

    std::cout << "=== Maisos eksperimentai ===\n";

    exp1_inputs( );
    exp2_format( );
    exp3_determinism( );
    exp4_speed( );
    exp5_collisions( );
    exp6_avalanche( );
    exp7_guessing( );

    std::cout << "=== Baigta. Rezultatai: results/ ===\n";

    return 0;
}
