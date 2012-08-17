/***************************************************************************
                          fssConfigNfs.h  -  description
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


#ifndef __fssConfigNfs_h_included__
#define __fssConfigNfs_h_included__


#include "fssConfig.h"
#include "fssShare.h"
#include "fssShareNfs.h"
#include <vector>

class fssConfigNfs : public fssConfig
{
public:
    fssConfigNfs();
    ~fssConfigNfs();

    void    read();
    int     addDir(std::string,bool);
    int     restartService();
protected:
};

#endif
