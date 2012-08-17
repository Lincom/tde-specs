/***************************************************************************
                          fssShareSmb.cpp  -  description
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


#include "fssShareSmb.h"
extern char* myLogin;

fssShareSmb::fssShareSmb(std::string dir) {
    path = dir;
    defaultSettings = "public = yes\nguest ok = yes\n";
    defaultSettings += "wide links = no\n";
}

fssShareSmb::~fssShareSmb() {}


std::string fssShareSmb::getSmbLabel() {
    return smbLabel;
}

void fssShareSmb::setSmbLabel(std::string nl) {
    smbLabel = nl;
    return;
}

std::string fssShareSmb::print() {
    std::string ret;
    std::string quotePath = path;

    if( quotePath.find(' ') != std::string::npos )
        quotePath = "\""+path+"\"";

    if( ! rawData.empty() ) {
        ret = rawData;
    } else {
        ret = "["+smbLabel+"]\n";
        ret += "path = "+quotePath+"\n";
        ret += "comment = "+path+"\n";
        if( readonly ) {
            ret += "writeable = no\n";
        } else {
            ret += "writeable = yes\n";
            ret += "create mask = 0640\n";

            std::string myLoginStr(myLogin);
            ret += "force user = "+myLoginStr+"\n";
        }
        ret += defaultSettings;
    }

    return ret;
}
