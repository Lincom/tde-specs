/***************************************************************************
                          fssConfigNfs.cpp  -  description
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


#include "fssConfigNfs.h"
#include "fssShareNfs.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <regex.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include <algorithm>

fssConfigNfs::fssConfigNfs() : fssConfig()
{
    configFilename = "/etc/exports";
    //configFilename = "./exports";
    read();
    type = "NFS";
}

fssConfigNfs::~fssConfigNfs() {}

void fssConfigNfs::read() {
    std::ifstream file( configFilename.c_str() );

    if ( file ) {
        std::string prevLine;
        std::string rawData;
        //QRegExp rx( "(\"[^\"]*\"|\\S+)\\s+(.*)" );
        regex_t rx;
        regmatch_t match[3];
        std::string::size_type index;

        if( regcomp(&rx, "^(\"[^\"]+\"|[^[:space:]]*)[[:space:]]+(.*)", REG_EXTENDED ) ) {
            std::cerr << "failed to compile regex" << std::endl;
            exit(255);
        }

        while ( !file.eof() ) {
            std::string line;
            getline( file, line ); // read without "\n"
            rawData += (line+"\n");

            // remove comments
            index = line.find_first_of( "#" );
            if( index != std::string::npos )
                line.replace( index, line.length()-1, "" );

            // strip WS
            while( (line.length() > 0) && (line[0] == ' ') )
                line.erase( line.begin() );
            while( line[line.length()-1] == ' ' )
                line.erase( line.end()-1 );

            // skip empty lines
            if( line == "" )
                continue;

            // multiline?
            if( line[line.length()-1] == '\\' ) {
                line.erase( line.end() ); // cut off "\"
                prevLine = prevLine + line;
                continue;
            }

            // all multilines are collected
            if( prevLine != "" )
                line = prevLine+line;
            prevLine = "";  //clear prevLine buffer

            // do the regex : /("[^"]*"|\S+)\s+(.*)/
            std::string dir, options;
            if( !regexec(&rx, line.c_str(), 3, match, 0) ) {
                size_t size = match[1].rm_eo-match[1].rm_so;
                if( size >= 0 && size < line.length() ) {
                    dir = line.substr( match[1].rm_so, size );
                }
                size = match[2].rm_eo-match[2].rm_so;
                if( size >= 0 && size < line.length() ) {
                    options = line.substr( match[2].rm_so, size );
                }
            }

            fssShareNfs* share = new fssShareNfs(dir);
            share->setNfsOptions( options );
            share->setRawData( rawData );
            // !!! FIX ME with REGEX !!!
            if( rawData.find("rw") != std::string::npos )
                share->setReadOnly(false);
            else
                share->setReadOnly(true);
            shareList.push_back( share );
            rawData = "";
        }
        file.close();
    }
}

int fssConfigNfs::addDir(std::string newDir, bool ro=true) {
    int ret = 1;
    fssShareNfs* newShare = new fssShareNfs( newDir );
    newShare->setReadOnly( ro );

    if( ! dirExists( newDir ) ) {
        shareList.push_back( newShare );
    } else {
        ret = 0;
        delete(newShare);
    }

    return ret;
}

int fssConfigNfs::restartService() {
    struct stat fStat;
//    const char  *file  = "/etc/init.d/nfsserver";
//    char* const argv[] = { "nfsserver", "reload", NULL };
    const char  *file  = "/usr/sbin/exportfs";
    char* const argv[] = { "exportfs", "-r", NULL };
    char* const envp[] = { "PATH=/sbin:/usr/sbin:/bin:/usr/bin", NULL };
    int ret = 0;
    pid_t pid;

    if( stat(file, &fStat) || ! (S_IXUSR & fStat.st_mode) )
        return -1;

    switch ( pid = fork() ) {
        case -1:
            // could not fork
            perror("nfs fork");
            return -1;
        case 0:
            // in child
            execve( file, argv, envp );
            perror(file); exit(1);
    }

    int status;
    while (waitpid(pid, &status, 0) < 0) {
        if (errno != EINTR) {
            perror("nfs waitpid");
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


