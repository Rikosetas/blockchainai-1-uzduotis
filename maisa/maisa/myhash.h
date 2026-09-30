#pragma once
#include <cstdint>
#include <cstddef>
#include <string>

namespace myhash {

    inline void init_state( uint64_t h [ 4 ] )
    {
        h [ 0 ] = 0xA5A5A5A5A5A5A5A5ULL;
        h [ 1 ] = 0x0123456789ABCDEFULL;
        h [ 2 ] = 0xFEDCBA9876543210ULL;
        h [ 3 ] = 0x5A5A5A5A5A5A5A5AULL;
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

    inline std::string hash_v1( const std::string& s )
    {
        return hash_v1( reinterpret_cast< const uint8_t* >( s.data( ) ), s.size( ) );
    }

}
