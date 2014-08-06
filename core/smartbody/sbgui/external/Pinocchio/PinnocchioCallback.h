/*  This file is part of the Pinocchio automatic rigging library.
    Copyright (C) 2007 Ilya Baran (ibaran@mit.edu)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef PINNOCCHIOCALLBACK_H_INCLUDED
#define PINNOCCHIOCALLBACK_H_INCLUDED
#include <math.h>
#include <cstddef>
#include "Pinocchio.h"
#include "mesh.h"
#include "attachment.h"

class PinnocchioCallBack
{
public:
	PinnocchioCallBack() {}
	~PinnocchioCallBack() {}
	virtual void callbackFunc() = 0;
	virtual void skeletonCompleteCallBack(std::vector<Vector3>& embedding) = 0;
};

class PINOCCHIO_API PinnocchioCallBackManager
{
public:
	PinnocchioCallBackManager();
	~PinnocchioCallBackManager();
	void setCallBack(PinnocchioCallBack* pinoCallBack);
	PinnocchioCallBack* getCallBack();
	void runCallBack();
	static PinnocchioCallBackManager* _singleton;
public:
	static PinnocchioCallBackManager& singleton();

	static PinnocchioCallBackManager* singletonPtr();

	static void destroy_singleton();	
protected:
	PinnocchioCallBack* callBack;
};
#endif //UTILS_H_INCLUDED
