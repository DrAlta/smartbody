/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.sbjniapp;

import android.view.MotionEvent;

// Wrapper for native library

public class SBJNIAppLib {

     static {
    	 //System.loadLibrary("python2.6");
    	 //System.loadLibrary("sbm");
         System.loadLibrary("sbjniapp");
     }

    /**
     * @param width the current view width
     * @param height the current view height
     */
     
     public static native void init(int width, int height);
     public static native void step();
     public static native void openConnection();
     public static native void closeConnection();
     public static native void restart();
     public static native void executeSB(String sbmCmd);
     //public static native void executePython(String pyCmd);
     public static native String getLog();
     public static native boolean handleInputEvent(int action, float x, float y, MotionEvent event);
     public static native void reloadTexture();
     
     //public static native void closeConnection();
}
