/***************************************************************************
                          fssShare.h  -  description
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


#ifndef __fssShare_h_included__
#define __fssShare_h_included__


#include<string>

class fssShare {
public:
    std::string getPath();
    std::string getRawData();
    std::string getType();
    bool        getReadOnly();

    void setPath(std::string);
    void setRawData(std::string);
    void setType(std::string);
    void setReadOnly(bool);

    virtual std::string print() = 0;

    fssShare();
    virtual ~fssShare();


protected:
    std::string path;
    std::string rawData;
    std::string type;
    bool        readonly;
};

#endif
