// Simple live reloading of variables, featuring type inference. Written in C++11
// - rlyeh, zlib/libpng license (2014)

#pragma once

#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <ctime>
#include <sys/stat.h>

#if defined(_WIN32)
#   include <sys/utime.h>
#else
#   include <sys/time.h>
#   include <utime.h>
#endif

#ifndef LIVE_TOKEN_IDENTIFIER
#define LIVE_TOKEN_IDENTIFIER "$live"
#endif

namespace {

    class livemonitor {

        static bool timestamp( const char * const pathfile, bool modify ) {
            auto date = [&]() -> time_t {
                struct stat fileInfo;
                if( stat( pathfile, &fileInfo ) < 0 )
                    return std::time(0);
                return fileInfo.st_mtime;
            };

            time_t curtime = date();

            static std::map< const char * const, time_t > cache;
            if( cache.find( pathfile ) == cache.end() ) {
                cache[ pathfile ] = curtime;
                return true;
            }

            auto &modtime = cache[ pathfile ];
            if( std::difftime( modtime, curtime ) != 0 ) {
                if( modify ) modtime = curtime;
                return true;
            }

            return false;
        };

        static bool has_changed( const char * const pathfile ) {
            return timestamp( pathfile, false );
        }

        static void touch( const char * const pathfile ) {
            timestamp( pathfile, true );
        }

        static std::vector< std::string > &get( const char *const pathfile ) {
            static std::map< const char *const, std::vector< std::string > > cache;
            return cache[ pathfile ];
        }

        static void parse( const char *const pathfile ) {
            auto split = []( const std::string &str, const std::string &delimiters ) {
                std::string map( 256, '\0' );
                for( const unsigned char &ch : delimiters )
                    map[ ch ] = '\1';
                std::vector< std::string > tokens(1);
                for( const unsigned char &ch : str ) {
                    /**/ if( !map.at(ch)          ) tokens.back().push_back( ch );
                    else if( tokens.back().size() ) tokens.push_back( std::string() );
                }
                while( tokens.size() && !tokens.back().size() ) tokens.pop_back();
                return tokens;
            };
            auto trim = [&]( std::string &str, const char *chars ) {
                // left
                auto begin = str.find_first_not_of(chars);
                if( std::string::npos != begin ) {
                    str = str.substr( begin );
                }
                // right
                auto end = str.find_last_not_of(chars);
                if( std::string::npos != end ) {
                    str = str.substr( 0, end+1 );
                }
            };

            auto &cache = get( pathfile );
            cache.clear();

            std::stringstream data;
            std::ifstream ifs( pathfile );
            data << ifs.rdbuf();

            auto words = split( data.str(), "()\r\n" );
            for( auto it = words.begin(), end = words.end(); it != end; ++it ) {
                auto tokens = split( *it, "<>!=;+- ");
                for( auto &token : tokens ) {
                    if( LIVE_TOKEN_IDENTIFIER == token ) {
                        cache.push_back( *(++it) );
                        trim( cache.back(), " \t" );
                        trim( cache.back(), "\"" );
                    }
                }
            }
       }

        template< typename T >
        static void cast( const std::string &value, T &t ) {
            T t_;
            std::stringstream ss;
            ss << std::setprecision( std::numeric_limits<T>::digits10 + 2 );
            ss << value;
            t = ss >> t_ ? t_ : t;
        }
        static void cast( const std::string &value, std::string &t ) {
            std::stringstream ss( value );
            std::getline( ss, t );
        }

    public:

        template< typename T >
        static T &check( const char *const pathfile, int counter, const T &t ) {
            using pair = std::pair< const char *const, int >;
            static std::map< pair, T > cache;
            static std::map< pair, unsigned > hits;
            auto key = pair( pathfile, counter );
            T &item = cache[ key ];
            if( has_changed( pathfile ) ) {
                parse( pathfile );
                auto &values = get( pathfile );
                if( counter < values.size() ) {
                    auto &value = values.at( counter );
                    if( !value.empty() ) {
                        cast( value, item );
                    }
                }
                if( ++hits[ key ] >= values.size() ) {
                    hits[ key ] = 0;
                    touch( pathfile );
                }
            }
            return item;
        }

        template< typename T, size_t N >
        static const char *check( const char *const pathfile, int counter, const char (&t)[N] ) {
            return check<std::string>( pathfile, counter, std::string(t) ).c_str();
        }
    };
}

#undef $live

#if defined(NDEBUG) || defined(_NDEBUG) || defined(RELEASE) || defined(MASTER) || defined(GOLD)
#define $live(...) (__VA_ARGS__)
#else
#define $live(...) livemonitor::check<decltype(__VA_ARGS__)>( __FILE__,__COUNTER__,__VA_ARGS__ )
#endif
