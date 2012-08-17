/***************************************************************************
                          fssShare.cpp  -  description
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


#include "fssShare.h"

fssShare::fssShare() {}
fssShare::~fssShare() {}

std::string fssShare::getPath() {
    return path;
}

std::string fssShare::getRawData() {
    return rawData;
}

std::string fssShare::getType() {
    return type;
}

bool fssShare::getReadOnly() {
    return readonly;
}

void fssShare::setPath(std::string np) {
    path = np;
    return;
}

void fssShare::setRawData(std::string nd) {
    rawData = nd;
    return;
}

void fssShare::setType(std::string nt) {
    type = nt;
    return;
}

void fssShare::setReadOnly(bool ro) {
    readonly = ro;
    return;
}

