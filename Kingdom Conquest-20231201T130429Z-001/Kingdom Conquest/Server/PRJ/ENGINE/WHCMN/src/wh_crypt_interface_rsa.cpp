// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : wh_crypt_interface_ras.cpp
// Creator      : Wei Hua (魏华)
// Comment      : 基于RSA的加密接口实现
// CreationDate : 2006-05-25
// Change LOG   : 

#include "../inc/wh_crypt_interface.h"
#include "../inc/whstring.h"
#include "../inc/whdes.h"
#include "../inc/whtime.h"
#include <cryptlib/randpool.h>
#include <cryptlib/dh.h>
#include <cryptlib/files.h>
#include <cryptlib/hex.h>
#include <cryptlib/base64.h>
#include <sstream>
USING_NAMESPACE(CryptoPP)

namespace n_whcmn
{

class	ICryptFactory_RSAAndDES	: public	ICryptFactory
{
};

ICryptFactory *	ICryptFactory_RSAAndDES_Create()
{
	// return	new	ICryptFactory_RSAAndDES;
	return	NULL;
}

}		// EOF namespace n_whcmn
