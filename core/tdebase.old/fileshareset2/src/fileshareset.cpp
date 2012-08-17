/***************************************************************************
                          fileshareset.cpp  -  description
                             -------------------
    copyright            : (C) 2003 SuSE AG 
    email                : Uwe.Gansert@suse.de

    based on a perl script by MandrakeSoft
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>
#include <getopt.h>

#include "fssConfig.h"
#include "fssConfigNfs.h"
#include "fssShareNfs.h"
#include "fssConfigSmb.h"
#include "fssShareSmb.h"

#define AUTHFILE  "/etc/security/fileshare.conf"
#define AUTHGROUP "fileshare"

uid_t myUID;
char* myLogin;

void printHelp () {
    std::cout << "usage: fileshareset --add <dir>" << std::endl;
    std::cout << "\tfileshareset --remove <dir>" << std::endl;
}

bool myDir( std::string dir ) {
    struct stat dirStat;
    bool   ret=false;

    if( ! stat( dir.c_str(), &dirStat ) && (dirStat.st_uid == myUID) )
        ret = true;
    return ret;
}

// this function decides if the "--add <directory>"
// can be exportet or not.
bool pathOkay( std::string checkPath ) {
    bool okay = true;
    struct stat dirStat;
    //std::string::size_type ST;

    // CHECK: we need the complete path, so it must begin with "/"
    if( checkPath[0] != '/' )
        okay = false;

    // CHECK: nasty characters in Path
    if( checkPath.find("..") != std::string::npos )
        okay = false;
    if( checkPath.find("\"") != std::string::npos )
        okay = false;
    std::string::iterator it;
    for( it=checkPath.begin(); it < checkPath.end(); it++ ) {
        if( ! isprint(*it) ) // 0..31 + 127
            okay = false;
    }

    // CHECK: try to stat PATH
    if( stat( checkPath.c_str(), &dirStat ) )
        okay = false;
    else {
        // CHECK: is PATH a directory?
        if( ! S_ISDIR(dirStat.st_mode) )
            okay = false;
        // CHECK: if we aren't root, does PATH belongs to us?
        if( (myUID != 0) && !myDir(checkPath) )
            okay = false;
    }

    return okay;
}

// read /etc/security/fileshare
// if "RESTRICT=yes" the user has to be in
// group "fileshare"
// if "RESTRICT=no" everone can export his
// own directorys to the LAN
// a missing file is handled as "RESTRICT=yes"
//
bool authCheck( char* uname ) {
    std::ifstream fileStream( AUTHFILE );
    std::string::iterator it;
    bool ret = false;

    // !!! no locking !!! problem?
    if ( fileStream ) {
        //QRegExp rx_authParam( "^RESTRICT\\s*=\\s*([^\\s]+)" );
        std::string line;
        while ( getline( fileStream, line ) ) {
            if( line.substr( 0, 8 ) == "RESTRICT" ) {
                it = (line.begin()+8);
                // 0x09 == TAB
                while( *it == ' ' || *it == 0x09 || *it == '=' )
                    it++;
                std::string param(it,line.end());
                if( param == "no" || param == "NO" )
                    ret = true;
            }
        }
        fileStream.close();
    }

    // restricted access.
    // CHECK: is user in group AUTHGROUP?
    if( ret == false ) {
        struct  group *grp;
        char    *grpMember;
        int     i=0;

        setgrent();
        grp = getgrnam( AUTHGROUP );
        if( grp ) {
            grpMember = grp->gr_mem[i++];
            while( grpMember ) {
                if( !strcmp(grpMember, uname) )
                    ret = true;
                grpMember = grp->gr_mem[i++];
            }
        }
        endgrent();
    }
    return ret;
}

// search string "search" in string vector "*v"
/*bool contains( std::vector<std::string> *v, std::string search ) {
    std::vector<std::string>::iterator it;
    bool ret = false;
    std::string searchStr(search);

    if( searchStr.find(' ') != std::string::npos )
        searchStr = "\""+searchStr+"\"";
    for( it=v->begin(); it != v->end(); it++ ) {
        if( (*it) == searchStr ) {
            ret = true;
            break;
        }
    }
    return ret;
}*/

int main( int argc, char* argv[] ) {
    std::string  calledAs( argv[0] );
    std::string  dirParameter;
    //uid_t   uid;
    //char*   login;
    int c;
    int digit_optind = 0;
    struct  passwd *pwd;
    struct  stat   fileStat;
    //std::vector<std::string> pathList;
    std::vector<fssConfig*>  services;
    std::vector<fssConfig*>::iterator it;
    std::map<std::string,std::string> pathMap;

    // nfs deaktivated because of a security hole
    //
    //if( !stat("/etc/exports", &fileStat) ) {
    //    fssConfigNfs* nfs = new fssConfigNfs();
    //    services.push_back( nfs );
    //}
    if( !stat("/etc/samba/smb.conf", &fileStat) ) {
        fssConfigSmb* smb = new fssConfigSmb();
        services.push_back( smb );
    }

    // create the pathList
    // a unique list of all exported path
    for( it = services.begin(); it != services.end(); it++ ) {
        std::vector<std::string> dummy = (*it)->getPathList();
        std::vector<std::string>::iterator dit;
        for( dit = dummy.begin(); dit != dummy.end(); dit++ ) {
            //if( ! contains( &pathList, *dit ) )
            //    pathList.push_back( *dit );
            pathMap[*dit] = (*it)->getOptions(*dit);
        }
    }

    myUID = getuid();

    // read login name
    setpwent();
    pwd = getpwuid( myUID );
    if( ! pwd->pw_name || strlen(pwd->pw_name) > 32 )
        // unusual long login name or no (?) login name at all
        exit(255);
    myLogin = (char*)(calloc( strlen(pwd->pw_name)+1, sizeof(char)));
    if( !myLogin )
        // hmmm, are we short of memory?
        exit(255);
    strcpy( myLogin, pwd->pw_name );
    endpwent();

    // CHECK: is user authorized?
    if( !authCheck( myLogin ) ) {
        std::cerr << "You are not authorised to use fileshare'ing" << std::endl;
        std::cerr << "To grant you the rights:" << std::endl;
        std::cerr << "- put \"RESTRICT=no\" in " << AUTHFILE << std::endl;
        std::cerr << "- or get membership of group: " << AUTHGROUP << std::endl;
        exit(1); // not authorized
    }

    // called as : filesharelist
    //
    //if( calledAs.endsWith("filesharelist") ) {
    std::string substr("");
    if( calledAs.length() >= 13 )
        substr = calledAs.substr(calledAs.length()-13);
    if( substr == "filesharelist" ) {
        //std::vector<std::string>::iterator sit;
        std::map<std::string,std::string>::iterator sit;
        //for( sit=pathList.begin(); sit != pathList.end(); sit++ ) {
        for( sit=pathMap.begin(); sit != pathMap.end(); sit++ ) {
            // just our own directories will be listed
            if( myDir((*sit).first ) )
            //  std::cout << *sit << std::endl;
                std::cout << (*sit).second << " " << (*sit).first << std::endl;
        }
        exit(0);
    }

    std::string action;
    std::string host;
    bool readonly = true;
    while (1) {
        //int this_option_optind = optind ? optind : 1;
        int option_index = 0;
        static struct option long_options[] = {
            {"add",    required_argument, 0, 'a'},
            {"host",   required_argument, 0, 'h'},
            {"remove", required_argument, 0, 'r'},
            {"rw",     no_argument,       0, 'w'},
            {0, 0, 0, 0}
        };
        c = getopt_long_only (argc, argv, "",
                              long_options, &option_index);
        if (c == -1)
            break;
        switch (c) {
        case 'a':
            action = "add";
            dirParameter = optarg;
        break;
        case 'r':
            action = "remove";
            dirParameter = optarg;
        break;
        case 'h':
            host = optarg;
        break;
        case 'w':
            readonly = false;
        break;
        default:
            printHelp();
            exit(255);
        }
    }

    if( action == "remove" && !host.empty() ) {
        std::cerr << "don't mix --host and --remove" << std::endl
                  << "you have to remove the complete path and add it again"
                  << std::endl;
        exit(255);
    }

    // called as fileshareset
    // strip trailing slashes
    while( dirParameter[dirParameter.length()-1] == '/' )
        dirParameter.erase( dirParameter.length()-1 );

    if( action == "add" ) {
        if( ! pathOkay( dirParameter ) ) {
            std::cerr << "Err: illegal path " << dirParameter << std::endl;
            exit(4);
        } else {
            std::string dummyOptions;
            for( it = services.begin(); it != services.end(); it++ ) {
                //if( ! contains( &pathList, dirParameter ) ) {
                if( pathMap.find(dirParameter) == pathMap.end() ) {
                    if( ! (*it)->addDir( dirParameter, readonly ) )
                         std::cerr << "Warn: add failed for service " << (*it)->getType()
                                   << std::endl;
                    else
                        dummyOptions = (*it)->getOptions( dirParameter );
                } else {
                    std::cerr << "Err: already exported " << dirParameter << std::endl;
                    exit(3);
                }
            }
            //pathList.push_back( dirParameter );
            pathMap[dirParameter] = dummyOptions;
        }
    } else if( action == "remove" ) {
        if( ! pathOkay( dirParameter ) ) {
            std::cerr << "Err: illegal path " << dirParameter << std::endl;
            exit(4);
        } //else if( ! contains( &pathList, dirParameter) ) {
          else if( pathMap.find(dirParameter) == pathMap.end() ) {
            std::cerr << "Err: not exported " << dirParameter << std::endl;
            exit(5);
        } else {
            for( it = services.begin(); it != services.end(); it++ ) {
                if( (*it)->dirExists( dirParameter ) )
                    (*it)->removeDir( dirParameter );
                else
                    std::cerr << "Warn: remove failed for service " << (*it)->getType()
                              << std::endl;
            }
            // remove path from unique pathList
            //std::vector<std::string>::iterator slit;
            //for( slit=pathList.begin(); slit < pathList.end(); slit++ ) {
            //    if( *slit == dirParameter ) {
            //        pathList.erase(slit);
            //        break;
            //    }
            //}
            pathMap.erase( pathMap.find( dirParameter ) );
        }
    } else {
        printHelp();
        exit(2);
    }

    // write all configfiles and restart services
    for( it = services.begin(); it != services.end(); it++ ) {
        (*it)->write();
        if( (*it)->restartService() == -1 )
            std::cerr << "Warn: restarting " << (*it)->getType() << " failed"
                      << std::endl;
    }

    // always print my shares
    //std::vector<std::string>::iterator slit;
    //for( slit=pathList.begin(); slit < pathList.end(); slit++ ) {
    //    if( myDir( *slit ) )
    //        std::cout << *slit << std::endl;
    //}
    std::map<std::string,std::string>::iterator smit;
    for( smit=pathMap.begin(); smit != pathMap.end(); smit++ ) {
        // just our own directories will be listed
        if( myDir( (*smit).first ) )
            std::cout << (*smit).second << " " << (*smit).first << std::endl;
    }
    return 0;
}


