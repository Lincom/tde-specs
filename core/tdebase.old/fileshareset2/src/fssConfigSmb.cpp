/***************************************************************************
                          fssConfigSmb.cpp  -  description
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


#include "fssConfigSmb.h"
#include "fssShareSmb.h"
#include <iostream>
#include <fstream>
#include <cctype>
#include <ctype.h>
#include <regex.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>
#include <algorithm>

fssConfigSmb::fssConfigSmb() : fssConfig()
{
   configFilename = "/etc/samba/smb.conf";
   read();
   type = "SMB";
}

fssConfigSmb::~fssConfigSmb() {}

void fssConfigSmb::read() {
    std::ifstream file( configFilename.c_str() );

    if ( file ) {
        std::string old;
        //QRegExp rx_sectionStart("^\\s*\\[([^\\]]+)\\]");
        //QRegExp rx_path("path\\s*=\\s*([^\\s][^\\n]+)");

        regex_t rx_sectionStart;
        regex_t rx_path;
        regex_t rx_writeable;
        regmatch_t match_SectionStart[2];
        regmatch_t match_path[2];
        regmatch_t match_writeable[2];

        if( regcomp(&rx_sectionStart, "^[[:space:]]*\\[(.+)\\]",
                    REG_EXTENDED) ||
            regcomp(&rx_path, "path[[:space:]]*=[[:space:]]*([^[:space:]].*)",
                    REG_EXTENDED|REG_NEWLINE) ||
            regcomp(&rx_writeable, "writeable[[:space:]]*=[[:space:]]*([^[:space:]].*)",
                    REG_EXTENDED|REG_NEWLINE) )
        {
            std::cerr << "failed to compile regex" << std::endl;
            exit(255);
        }

        std::string line;
        while ( ! file.eof() ) {
            getline( file, line );

            if( ! regexec(&rx_sectionStart, line.c_str(), 1, match_SectionStart, 0) ||
                file.eof() ) 
            {
                std::string label("");
                std::string path("");
                std::string writeable("no");

                if( !regexec(&rx_sectionStart, old.c_str(), 2, match_SectionStart, 0) ) {
                    size_t size = match_SectionStart[1].rm_eo-match_SectionStart[1].rm_so;
                    if( size > 0 && size < old.length() ) {
                        label = old.substr( match_SectionStart[1].rm_so, size );
                    }
                }

                if( !regexec(&rx_path, old.c_str(), 2, match_path, 0)) {
                    size_t size = match_path[1].rm_eo-match_path[1].rm_so;
                    if ( size > 0 && size < old.length() ) {
                        path = old.substr( match_path[1].rm_so, size );
                    }
                }

                if( !regexec(&rx_writeable, old.c_str(), 2, match_writeable, 0)) {
                    size_t size = match_writeable[1].rm_eo-match_writeable[1].rm_so;
                    if( size > 0 && size < old.length() ) {
                        writeable = old.substr( match_writeable[1].rm_so, size );
                    }
                }

                fssShareSmb* share = new fssShareSmb( path );
                if( writeable != "no" )
                    share->setReadOnly( false );
                else
                    share->setReadOnly( true );
                share->setRawData( old );
                share->setSmbLabel( label );
                shareList.push_back( share );
                old = "";
           }
           old += (line+"\n");
        }
        regfree( &rx_sectionStart );
        regfree( &rx_path );
    }
}

int fssConfigSmb::addDir(std::string newDir, bool ro=true) {
    int ret = 1;
    fssShareSmb* newShare = new fssShareSmb( newDir );
    newShare->setReadOnly( ro );

    if( ! dirExists( newDir ) ) {
        std::string cleanLabel = tidyLabel( newDir );
        if( ! cleanLabel.empty() ) {
            newShare->setSmbLabel( cleanLabel );
            shareList.push_back( newShare );
        } else {
            ret = 0;
        }
    } else {
        ret = 0;
    }

    return ret;
}

// this is more or less a copy of the perl-code from the
// original fileshareset, except that it is C++ now (of course)
std::string fssConfigSmb::tidyLabel( std::string oldLabel ) {
    std::string newLabel = oldLabel;
    std::string::size_type index;

    // to uppercase
    std::transform( newLabel.begin(), newLabel.end(),
                    newLabel.begin(), toupper );

    // only keep legal characters
    //newLabel = newLabel.replace( QRegExp("[^A-Z0-9#\\-_!/]"), "_" );
    std::string::iterator it;
    for( it=newLabel.begin(); it != newLabel.end(); it++ ) {
        if( (*it < 'A' || *it > 'Z') &&
            (*it < '0' || *it > '9') &&
            *it != '#' && *it != '-' && *it != '_' && *it != '!' && *it != '/')
        {
            *it = '_';
        }
    }

    // remove the leading and trailing '/' and '_'
    while( newLabel[0] == '/' || newLabel[0] == '_')
        newLabel.erase( newLabel.begin() );
    while( newLabel[newLabel.length()-1] == '/' ) 
        newLabel.erase( newLabel.length()-1 );

    //if( newLabel.substr( 0, 5 ) == "home/" )
    //    newLabel.erase( 0, 5 );

    // just some underscore trimming:
    //
    // replacing RegExp: s|_*/_*|/|
    //                   s|_+|_|
    // newLabel = newLabel.replace( QRegExp("_*/_*"), "/");
    // newLabel = newLabel.replace( QRegExp("_+"), "_");
    for( it=newLabel.begin(); it != newLabel.end(); it++ ) {
        if( (*it) == '_' ) {
            while( *(it+1) == '_' )
                newLabel.erase(it);
            if( (*it == '_') && (*(it+1) == '/') )
                newLabel.erase(it); 
        }
        if( (*it) == '/' ) {
            it++;
            while( *it == '_' )
                newLabel.erase(it);
        }
    }

    // if size is too small (!), make it bigger
    while( newLabel.length() < MIN_SHARENAME_LENGTH )
        newLabel += "_";

    // if size is too big, shorten it
    // The NT4.0 Explorer does not handle NetBIOS share names
    // longer than 12 characters
    while ( newLabel.length() > MAX_SHARENAME_LENGTH ) {
        std::string substr;

        //remove leading dir s|[^/]*/(.*)|$1|
        //
        index = newLabel.find('/');
        if( index != std::string::npos ) {
            substr = newLabel.substr( index+1 );
            if( substr.length() > 6 && ! smbLabelExists(substr)) {
                newLabel = substr;
                continue;
            }
        }

        // replacing Code:
        // QRegExp rx_nonLetters("(.*)[0-9#\\-_!/]");
        // ...
        // newLabel = rx_nonLetters.cap(1);
        //
        regex_t regex;
        regmatch_t matches[2];
        if( regcomp(&regex, "(.*)[0-9#_!-]", REG_EXTENDED) ) {
            // the thing that should not be
            std::cerr << "failed to compile regex" << std::endl;
            exit(255);
        }
        if( ! regexec(&regex, newLabel.c_str(), 2, matches, 0) ) {
            size_t size = matches[1].rm_eo-matches[1].rm_so;
            if( size > 0 && size < newLabel.length() ) {
                newLabel = newLabel.substr( matches[1].rm_so, size );
            }
            regfree(&regex);
            continue;
        }
        regfree(&regex);

        /* KICKED
        //
        // inspired by "Christian Brolin" "Long names are doom"
        // on comp.lang.functional
        QRegExp rx_allButFirstVowel("(.+)[AEIOU]");
        if( rx_allButFirstVowel.search( newLabel ) != -1 ) {
            list = rx_allButFirstVowel.capturedTexts();
            newLabel = list[1];
            continue;
        }

        QRegExp rx_adjacentDuplicates("(.*)(.)\2");
        if( rx_adjacentDuplicates.search( newLabel ) != -1 ) {
            list = rx_allButFirstVowel.capturedTexts();
            newLabel = list[1]+list[2];
            continue;
        }
        */
        // nothing else matches. So we have to cut it down
        newLabel.erase( 0, 1 );
    }
    // remove "/"s still there
    // newLabel = newLabel.replace( QRegExp("/"), "_" );
    for( it=newLabel.begin(); it != newLabel.end(); it++ ) {
        if( *it == '/' )
            *it = '_';
    }

    if( smbLabelExists( newLabel ) ) {
        // trouble. No uniqe label possible
        newLabel = "";
    }
    return newLabel;
}

bool fssConfigSmb::smbLabelExists( std::string label ) {
    bool ret = false;

    std::vector<fssShare*>::iterator it;
    for( it=shareList.begin(); it != shareList.end(); it++ ) {
        if( ((fssShareSmb*)(*it))->getSmbLabel() == label ) {
            ret = true;
            break;
        }
    }
    return ret;
}

int fssConfigSmb::restartService() {
    struct stat fStat;
    const char  *file  = "/etc/init.d/smb";
    char* const argv[] = { "smb", "reload", NULL };
    char* const envp[] = { "PATH=/sbin:/usr/sbin:/bin:/usr/bin", NULL };
    int ret = 0;
    pid_t pid;

    if( stat(file, &fStat) || !(S_IXUSR & fStat.st_mode) )
        return -1;

    switch ( pid = fork() ) {
        case -1:
            // could not fork
            perror("smb fork");
            return -1;
        case 0:
            // in child
            execve( file, argv, envp );
            perror(file); exit(1);
    }

    int status;
    while (waitpid(pid, &status, 0) < 0) {
        if (errno != EINTR) {
            perror("smb waitpid");
            return -1;
        }
    }
    ret = 0;
    if (WIFEXITED(status)) {
        ret = WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        std::cerr << file << " crashed, signal " << WTERMSIG(status) << std::endl;
    } else {
        std::cerr << file << ": status " << status << std::endl;
    }
    return ret;
}


