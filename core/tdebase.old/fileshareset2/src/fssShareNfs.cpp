/***************************************************************************
                          fssShareNfs.cpp  -  description
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


#include "fssShareNfs.h"
#include <iostream>
#include <sstream>

extern uid_t myUID;

fssShareNfs::fssShareNfs(std::string dir) {
    path = dir;
    nfsOptions = std::string( DEFAULT_NFS_OPTIONS );
}

fssShareNfs::~fssShareNfs() {}


std::string fssShareNfs::getNfsOptions() {
    return nfsOptions;
}

void fssShareNfs::setNfsOptions(std::string no) {
    nfsOptions = no;
    return;
}

std::string fssShareNfs::print() {
    std::string ret;
    std::string quotePath = path;

    if( quotePath.find(' ') != std::string::npos )
        quotePath = "\""+path+"\"";
    if( ! rawData.empty() )
        ret = rawData;
    else {
        if( readonly ) {
            ret = quotePath+" *(ro,";
        } else {
            std::ostringstream myUIDStr;
            myUIDStr << myUID;
            ret = quotePath+" *(rw,anonuid="+myUIDStr.str()+",";
        }
        ret += nfsOptions+")\n";
    }

    return ret;
}

