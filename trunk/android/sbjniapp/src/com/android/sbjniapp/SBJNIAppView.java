/*
 * Copyright (C) 2009 The Android Open Source Project
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
/*
 * Copyright (C) 2008 The Android Open Source Project
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

import android.content.Context;
import android.graphics.PixelFormat;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

class SBJNIAppView extends GLSurfaceView {
    private static String TAG = "GL2JNIView";
    private static boolean sbRestart = false;
    private static boolean sbReloadTexture = false;

    public SBJNIAppView(Context context) {
        super(context);
        init(false, 0, 0);
    }
    
    public SBJNIAppView(Context context, AttributeSet attrs)
    {
        super(context, attrs);
        init(false, 0, 0);
    }
   
    public SBJNIAppView(Context context, boolean translucent, int depth, int stencil) {
        super(context);
        init(translucent, depth, stencil);
    }
    
    public void restartSB()
    {
    	sbRestart = true;    	
    }
    @Override
    public void onResume() {
    	super.onResume();
    	//SBJNIAppLib.reloadTexture();
    	sbReloadTexture = true;
    	    
    }

    private void init(boolean translucent, int depth, int stencil) {

        /* By default, GLSurfaceView() creates a RGB_565 opaque surface.
         * If we want a translucent one, we should change the surface's
         * format here, using PixelFormat.TRANSLUCENT for GL Surfaces
         * is interpreted as any 32-bit surface with alpha by SurfaceFlinger.
         */
        if (translucent) {
            this.getHolder().setFormat(PixelFormat.TRANSLUCENT);
        }       
        setEGLContextClientVersion(2);
        setRenderer(new Renderer());
        setPreserveEGLContextOnPause(true);
        
        this.setOnTouchListener(new OnTouchListener() {

            @Override
            public boolean onTouch(View v, MotionEvent event) {
                return SBJNIAppLib.handleInputEvent(event.getAction(), event.getX(), event.getY());
            }
        });
    }   

    private static class Renderer implements GLSurfaceView.Renderer {
        public void onDrawFrame(GL10 gl) {
        	if (sbRestart)
        	{
        		SBJNIAppLib.restart();
        		sbRestart = false;
        	}
        	if (sbReloadTexture)
        	{
        		SBJNIAppLib.reloadTexture();
        		sbReloadTexture = false;
        	}
            SBJNIAppLib.step();
        }

        public void onSurfaceChanged(GL10 gl, int width, int height) {
        	SBJNIAppLib.init(width, height);        	
        }

        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            // Do nothing.
        }
    }
}
