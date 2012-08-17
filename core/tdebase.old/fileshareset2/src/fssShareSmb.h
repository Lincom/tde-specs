/***************************************************************************
                          fssShareSmb.h  -  description
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


#ifndef __fssShareSmb_h_included__
#define __fssShareSmb_h_included__


#include "fssShare.h"
#include <string>

class fssShareSmb : public fssShare
{
public:
    fssShareSmb(std::string);
    ~fssShareSmb();

    std::string getSmbLabel();
    void    setSmbLabel(std::string);
    std::string print();
protected:
    std::string smbLabel;
    std::string defaultSettings;
};

#endif
