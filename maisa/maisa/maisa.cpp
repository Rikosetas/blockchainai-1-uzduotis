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
        "  maisa -i              rankinis ivedimas (viena eilute, be Enter naujos eilutes)\n"
        "  parinktys: --v1 | --v2  (numatyta --v2)\n";
}

int main( int argc, char** argv )
{
    int version = 2;
    std::string mode, arg;

    for ( int i = 1; i < argc; ++i )
    {
        std::string a = argv [ i ];

        if ( a == "--v1" ) 
            version = 1;
        else if ( a == "--v2" ) 
            version = 2;
        else if ( a == "-i" )
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
        if ( in.bad( ) )
        {
            std::cout << "KLAIDA: nepavyko perskaityti failo: " << arg << "\n";
            return 1;
        }

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

    std::string digest = myhash::hash( bytes.data( ), bytes.size( ), version );

    std::cout << digest << "\n";
    std::cout << "rezimas=" << mode_name
        << " versija=v0." << version
        << " baitai=" << bytes.size( ) << "\n";

    return 0;
}