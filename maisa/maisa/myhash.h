#pragma once
#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

namespace myhash {

    inline uint64_t rotl( uint64_t x, int r ) 
    {
        return ( x << r ) | ( x >> ( 64 - r ) );
    }

    inline std::string to_hex( const uint64_t h [ 4 ] ) 
    {
        static const char* dig = "0123456789abcdef";

        std::string out;
        out.reserve( 64 );
        
        for ( int k = 0; k < 4; ++k )
            for ( int shift = 60; shift >= 0; shift -= 4 )
                out.push_back( dig [ ( h [ k ] >> shift ) & 0xF ] );
        
        return out;
    }

    inline void init_state( uint64_t h [ 4 ] ) 
    {
        h [ 0 ] = 0xA5A5A5A5A5A5A5A5ULL;
        h [ 1 ] = 0x0123456789ABCDEFULL;
        h [ 2 ] = 0xFEDCBA9876543210ULL;
        h [ 3 ] = 0x5A5A5A5A5A5A5A5AULL;
    }

    inline std::string hash_v1( const uint8_t* data, size_t n ) 
    {
        uint64_t h [ 4 ];
        init_state( h );

        const uint64_t P = 0x00000100000001B3ULL;
        
        for ( size_t i = 0; i < n; ++i ) 
        {
            int lane = ( int ) ( i & 3 );
            h [ lane ] = ( h [ lane ] ^ ( uint64_t ) data [ i ] ) * P;
        }
        
        h [ 3 ] ^= ( uint64_t ) n;
        return to_hex( h );
    }

    inline uint64_t fmix64( uint64_t x ) 
    {
        x ^= x >> 33;
        x *= 0xFF51AFD7ED558CCDULL;
        x ^= x >> 33;
        x *= 0xC4CEB9FE1A85EC53ULL;
        x ^= x >> 33;
        return x;
    }

    inline std::string hash_v2( const uint8_t* data, size_t n ) 
    {
        uint64_t h [ 4 ];
        init_state( h );
    
        const uint64_t P = 0x9E3779B97F4A7C15ULL;
        
        for ( size_t i = 0; i < n; ++i ) 
        {
            int lane = ( int ) ( i & 3 );
            h [ lane ] ^= ( uint64_t ) data [ i ] + 1;
            h [ lane ] *= P;
            h [ lane ] = rotl( h [ lane ], 27 );
            h [ ( lane + 1 ) & 3 ] ^= h [ lane ];
        }

        for ( int k = 0; k < 4; ++k )
            h [ k ] ^= ( uint64_t ) n * ( 2 * ( uint64_t ) k + 1 );
        
        for ( int r = 0; r < 3; ++r ) 
        {
            h [ 0 ] += rotl( h [ 1 ], 13 );
            h [ 1 ] ^= rotl( h [ 2 ], 29 );
            h [ 2 ] += rotl( h [ 3 ], 41 );
            h [ 3 ] ^= rotl( h [ 0 ], 17 );
        }
        
        for ( int k = 0; k < 4; ++k )
            h [ k ] = fmix64( h [ k ] );
        
        return to_hex( h );
    }

    inline std::string hash_v1( const std::string& s ) 
    {
        return hash_v1( reinterpret_cast< const uint8_t* >( s.data( ) ), s.size( ) );
    }

    inline std::string hash_v2( const std::string& s ) 
    {
        return hash_v2( reinterpret_cast< const uint8_t* >( s.data( ) ), s.size( ) );
    }

    inline std::string hash( const uint8_t* data, size_t n, int version ) 
    {
        return version == 1 ? hash_v1( data, n ) : hash_v2( data, n );
    }

    inline std::string hash( const std::string& s, int version )
    {
        return hash( reinterpret_cast< const uint8_t* >( s.data( ) ), s.size( ), version );
    }

}
