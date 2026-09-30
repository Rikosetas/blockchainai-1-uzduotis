#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "myhash.h"

static void usage( )
{
    std::cout <<
        "Naudojimas:\n"
        "  maisa -f <failas>     maisuoja failo turini (tikslus baitai)\n"
        "  maisa -t <tekstas>    maisuoja teksta kaip UTF-8 (be naujos eilutes)\n"
        "  maisa -i              rankinis ivedimas (viena eilute, be Enter naujos eilutes)\n";
}

int main( int argc, char** argv )
{
    std::string mode, arg;

    for ( int i = 1; i < argc; ++i )
    {
        std::string a = argv [ i ];

        if ( a == "-i" )
            mode = "i";
        else if ( ( a == "-f" || a == "-t" ) && i + 1 < argc )
        {
            mode = a.substr( 1 );
            arg = argv [ ++i ];
        }
        else
        {
            usage( );
            return 2;
        }
    }

    if ( mode.empty( ) )
    {
        usage( );
        return 2;
    }

    std::vector<uint8_t> bytes;
    std::string mode_name;

    if ( mode == "f" )
    {
        std::ifstream in( arg, std::ios::binary );
        if ( !in )
        {
            std::cout << "KLAIDA: nepavyko atidaryti failo: " << arg << "\n";
            return 1;
        }

        bytes.assign( std::istreambuf_iterator<char>( in ), std::istreambuf_iterator<char>( ) );
        mode_name = "failas(" + arg + ")";
    }
    else if ( mode == "t" )
    {
        bytes.assign( arg.begin( ), arg.end( ) );
        mode_name = "tekstas(-t)";
    }
    else
    {
        std::string line;
        std::cout << "Iveskite teksta ir spauskite Enter (naujos eilutes simbolis NEITRAUKIAMAS):\n";

        std::getline( std::cin, line );
        bytes.assign( line.begin( ), line.end( ) );

        mode_name = "rankinis(-i)";
    }

    std::cout << myhash::hash_v1( bytes.data( ), bytes.size( ) ) << "\n";
    std::cout << "rezimas=" << mode_name << " versija=v0.1 baitai=" << bytes.size( ) << "\n";

    return 0;
}
