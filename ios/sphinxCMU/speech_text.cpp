//
//  speech_text.cpp
//  smartbody-lib
//
//  Created by Yuyu Xu on 10/7/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "speech_text.h"
#include "vhcl.h"

#ifdef SBM_IPHONE
SpeechTextLocal::SpeechTextLocal()
{
    config = NULL;
    ps = NULL;
}

SpeechTextLocal::~SpeechTextLocal()
{
    if (ps)
        ps_free(ps);
}

void SpeechTextLocal::initialize(const char* hmmPath, const char* lmFilePath, const char* dicFilePath)
{
    config = cmd_ln_init(NULL, ps_args(), TRUE,
						 "-hmm", hmmPath,
						 "-lm", lmFilePath,
						 "-dict", dicFilePath,
						 NULL);
    
    
    if (config == NULL)
    {
        LOG("SpeechTextLocal::initialize ERR: fail to initialize configuration");
        return;
    }
    
    ps = ps_init(config);
    if (ps == NULL)
    {
        LOG("SpeechTextLocal::initialize ERR: fail to initialize speech decoder.");
        return;
    }
}

std::string SpeechTextLocal::decodeAudioFile(std::string filePath)
{
    FILE *fh;
	char const *hyp, *uttid;
	int16 buf[512];
	int rv;
	int32 score;

	fh = fopen(filePath.c_str(), "rb");
	if (fh == NULL) 
    {
		LOG("SpeechTextLocal::decodeAudioFile ERR: unable to open file %s", filePath.c_str());
		return "";
	}
    
    rv = ps_decode_raw(ps, fh, "audiofile", -1);
    
	hyp = ps_get_hyp(ps, &score, &uttid);
    
    LOG("Speech to text (audiofile): %s", hyp);
    return hyp;
}

#else
SpeechTextLocal::SpeechTextLocal()
{
}

SpeechTextLocal::~SpeechTextLocal()
{
}

// initialization
void SpeechTextLocal::initialize(const char* hmmPath, const char* lmFilePath, const char* dicFilePath)
{
}

// decode from specific audio file
std::string SpeechTextLocal::decodeAudioFile(std::string filePath)
{
    return "";
}


#endif