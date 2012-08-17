/***************************************************************************
                          fssConfigSmb.h  -  description
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


#ifndef __fssConfigSmb_h_included__
#define __fssConfigSmb_h_included__


#include "fssConfig.h"

#define MIN_SHARENAME_LENGTH 3
#define MAX_SHARENAME_LENGTH 12

class fssConfigSmb : public fssConfig
{
public:
    fssConfigSmb();
    ~fssConfigSmb();

    void    read();
    int     addDir();
    int     addDir(std::string,bool);
    int     restartService();
    bool    smbLabelExists(std::string);
protected:
    std::string tidyLabel( std::string );
};

#endif
