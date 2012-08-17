/***************************************************************************
                          fssConfig.cpp  -  description
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


#include "fssConfig.h"
#include <iostream>
#include <fstream>

#include <string>
#include <regex.h>

fssConfig::fssConfig() { }

fssConfig::~fssConfig() { }

bool fssConfig::dirExists( std::string dir ) {
    bool ret = false;
    std::vector<fssShare*>::iterator it;
    if( dir.find(" ") != std::string::npos )
        dir = "\""+dir+"\"";

    for( it=shareList.begin(); it != shareList.end(); it++ ) {
        if( (*it)->getPath() == dir ) {
            ret = true;
            break;
        }
    }
    return ret;
}

std::string fssConfig::getType() {
    return type;
}

int fssConfig::removeDir( std::string delDir ) {
    int ret = 0;
    if( delDir.find(' ') != std::string::npos )
        delDir = "\""+delDir+"\"";

    std::vector<fssShare*>::iterator it;
    for( it=shareList.begin(); it != shareList.end(); it++ ) {
        if( delDir == (*it)->getPath() ) {
            delete( *it );
            shareList.erase(it);
            ret = 1;
            break;
        }
    }
    return ret;
}

std::vector<std::string> fssConfig::getPathList() {
    std::vector<std::string> list;

    std::vector<fssShare*>::iterator it;
    for( it=shareList.begin(); it != shareList.end(); it++ ) {
        list.push_back( (*it)->getPath() );
    }
    return list;
}

void fssConfig::write() {
    std::ofstream file( configFilename.c_str() );
    //std::ofstream file( (configFilename+".new").c_str() );

    if ( file ) {
        std::vector<fssShare*>::iterator it;
        for( it=shareList.begin(); it != shareList.end(); it++ ) {
            file << (*it)->print();
        }
        file.close();
    }
    return;
}

std::string fssConfig::getOptions( std::string dir ) {
    std::string ret("");

    if( !dir.empty() ) {
        std::vector<fssShare*>::iterator it;
        for( it=shareList.begin(); it != shareList.end(); it++ ) {
            if( dir == (*it)->getPath() ) {
                ret = (*it)->getReadOnly() ? "readonly":"readwrite";
                break;
            }
        }
    }
    return ret;
}
