/***************************************************************************
                          fssConfig.h  -  description
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


#ifndef __fssConfig_h_included__
#define __fssConfig_h_included__

#include <vector>
#include <string>
#include "fssShare.h"
#include <regex.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

class fssConfig {
public:
    fssConfig();
    virtual ~fssConfig();

    virtual void    read() = 0;
    virtual void    write();
    virtual int     removeDir(std::string);
    virtual int     addDir(std::string, bool) = 0;
    virtual int     restartService() = 0;

    bool dirExists(std::string);
    std::string getType();
    std::vector<std::string> getPathList();
    std::string getOptions(std::string);
protected:
    //void startingServiceFailed(QProcess*);
    std::vector<fssShare*> shareList;
    std::string type;
    std::string configFilename;

//    std::string match2string( regmatch_t[], int=0 );
};

#endif
