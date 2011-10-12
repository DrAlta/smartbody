//
//  speech_text.h
//  smartbody-lib
//
//  Created by Yuyu Xu on 10/7/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//
#ifndef SPEECH_TEXT_H
#define SPEECH_TEXT_H

#ifdef __APPLE__
#include "TargetConditionals.h"
#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
#ifndef SBM_IPHONE
#define SBM_IPHONE
#endif
#endif
#endif
#include <iostream>
#ifdef SBM_IPHONE
#include "pocketsphinx.h"
#endif

class SpeechTextLocal
{
public:
    SpeechTextLocal();
    ~SpeechTextLocal();

    // initialization
    void initialize(const char* hmmPath, const char* lmFilePath, const char* dicFilePath);
    
    // decode from specific audio file
    std::string decodeAudioFile(std::string filePath);
    
private:
    ps_decoder_t* ps;
	cmd_ln_t* config;
};


#endif

