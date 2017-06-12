
package edu.usc.ict.sbmobile;
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
import android.view.MotionEvent;
import android.view.View;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.os.Environment;


public class SBMobileSurfaceView extends GLSurfaceView {
    private static String TAG = "SBM";
    public static boolean sbReloadTexture = false;
    final  String TARGET_BASE_PATH = Environment.getExternalStorageDirectory().getPath();
    
    public SBMobileSurfaceView(Context context) {
        super(context);
        init(false, 0, 0, context);
    }
    
    public SBMobileSurfaceView(Context context, AttributeSet attrs)
    {
        super(context, attrs);
        init(false, 0, 0, context);
    }
   
    public SBMobileSurfaceView(Context context, boolean translucent, int depth, int stencil) {
        super(context);
        init(translucent, depth, stencil, context);
    }
    
    @Override
     public void onResume() {
        super.onResume();
        sbReloadTexture = true;
        //Log.d("SBM","VHMobileSurfaceView::onResume");
    }

    @Override
    public void onPause() {
        super.onPause();
        //Log.d("SBM", "VHMobileSurfaceView::onPause");

    }

    private void init(boolean translucent, int depth, int stencil, Context context) {

        /* By default, GLSurfaceView() creates a RGB_565 opaque surface.
         * If we want a translucent one, we should change the surface's
         * format here, using PixelFormat.TRANSLUCENT for GL Surfaces
         * is interpreted as any 32-bit surface with alpha by SurfaceFlinger.
         */
        if (translucent) {
            this.getHolder().setFormat(PixelFormat.TRANSLUCENT);
        }       
        setEGLContextClientVersion(3);
        setRenderer(new Renderer());
        setPreserveEGLContextOnPause(true);
    }   

    private static class Renderer implements GLSurfaceView.Renderer {
        public void onDrawFrame(GL10 gl) {
        	if (sbReloadTexture)
        	{
                SBMobileLib.reloadTexture();
        		sbReloadTexture = false;
        	}
            //Log.e("myapp", "onDrawFrame");
            SBMobileLib.render();
        }

        public void onSurfaceChanged(GL10 gl, int width, int height) {
            //Log.e("myapp", "onSurfaceChanged");
            SBMobileLib.surfaceChanged(width, height);
        }

        public void onSurfaceCreated(GL10 gl, EGLConfig config) {

        }
    }

    
}
