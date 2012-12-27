/*
-------------------------------------------------------------------------------
    This file is part of OgreKit.
    http://gamekit.googlecode.com/

    Copyright (c) 2006-2010 zcube(JiSeop Moon)

    Contributor(s): harkon.kr
-------------------------------------------------------------------------------
  This software is provided 'as-is', without any express or implied
  warranty. In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
-------------------------------------------------------------------------------
*/

package com.android.sbogreapp;

import com.nvidia.devtech.NvGLES2Activity;
import com.nvidia.devtech.NvGLESActivity;
import com.nvidia.devtech.NvActivity;


import android.app.Activity;
import android.content.pm.ActivityInfo;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Window;
import android.view.WindowManager;

import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.widget.Button;
import android.widget.TextView;
import android.widget.EditText;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;

import android.view.LayoutInflater;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.Menu;
import android.view.View;
import android.view.View.OnClickListener;

public class Main
	//extends NvActivity {
	//extends NvGLESActivity {
	extends NvGLES2Activity {
  /** Called when the activity is first created. */
  @Override
  public void onCreate(Bundle savedInstanceState) {	   
	  //requestWindowFeature(Window.FEATURE_NO_TITLE);  
	  //getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,   
	  //		  WindowManager.LayoutParams.FLAG_FULLSCREEN);  
      super.onCreate(savedInstanceState);
  			setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
	
  			//setContentView(R.layout.main);
  			
  	  Button button = (Button)findViewById(R.id.SbmButton);        
        button.setOnClickListener(sbmListenser);
        
      CheckBox check = (CheckBox)findViewById(R.id.UseDeformable);
         check.setOnCheckedChangeListener(sbmCheckListener);
      
    }

    @Override protected void onPause() {
        super.onPause();
        //executeSbm("time pause"); 
        //mView.onPause();
    }

    @Override protected void onResume() {
        super.onResume();
        //executeSbm("time resume"); 
        //mView.onResume();
    }
    
      
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
    	MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.menu, menu);
        return super.onCreateOptionsMenu(menu);
    }
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle all of the possible menu actions.
        switch (item.getItemId()) {
        case R.id.StartSim:    
        	//onResume();
        	executeSbm("time resume"); 
            break;
        case R.id.PauseSim:
        	//onPause();
        	executeSbm("time pause");         	
            break;
        case R.id.ShowLog:   
        	showDialog(0);
        	break;
        case R.id.Connect:
        	//SbmJNILib.openConnection();
        	openConnection();//"172.16.33.21","61616");
            break;
        case R.id.Disconnect:
        	 closeConnection();
        	//SbmJNILib.closeConnection();
            break;
        }
        return super.onOptionsItemSelected(item);
        
    }
    @Override
    protected Dialog onCreateDialog(int id) {
        Dialog dialog;        
        return createLogDialog();
    }
    
    @Override
    protected void onPrepareDialog(int id, Dialog dialog) {     
    	AlertDialog alertDialog = (AlertDialog)dialog;
    	TextView text = (TextView)dialog.findViewById(R.id.TextView02); 
    	String logs = getLogStr();
    	text.setText(logs);
  	  	//text.setText("fjajfiosajgihagosdghaogiha");    	
    }    
    public AlertDialog createLogDialog(){    	
    	  AlertDialog.Builder ad = new AlertDialog.Builder(this);
    	  ad.setIcon(R.drawable.icon);
    	  ad.setTitle("Smartbody Log");   	    
    	  ad.setView(LayoutInflater.from(this).inflate(R.layout.custom_dialog,null));    	  
    	  ad.setPositiveButton("OK", new android.content.DialogInterface.OnClickListener() {
    		//@Override
			public void onClick(DialogInterface dialog, int which) {
				// TODO Auto-generated method stub
				
			}  }  );	    	  
    	  
    	  AlertDialog dialog = ad.create();    	  
    	  return dialog;    	  
     }
    
    private OnClickListener sbmListenser = new OnClickListener()
    {
        public void onClick(View v)
        {
        	//openOptionsMenu();
        	EditText sbmTextEdit = (EditText)findViewById(R.id.SbmText);
        	String sbmCmd = sbmTextEdit.getText().toString();
        	/*
        	CheckBox sbmPyCheck = (CheckBox)findViewById(R.id.usePython);        	
        	if (sbmPyCheck.isChecked())
        	{
        		SbmJNILib.executePython(sbmCmd);        		
        	}
        	else
        	*/
        	executeSbm(sbmCmd); 
        	sbmTextEdit.getText().clear();
        }
    };
    
    private OnCheckedChangeListener sbmCheckListener = new OnCheckedChangeListener()
    {
    	public void onCheckedChanged(CompoundButton buttonView, boolean isChecked)
    	{
    		if (isChecked)
    		{
    			setDeformable(1);    			
    		}
    		else
    		{
    			setDeformable(0);
    		}
    	}
    };    

  @Override
	public void onWindowFocusChanged(boolean hasFocus) {
		int screenHeight = this.getWindowManager().getDefaultDisplay().getHeight();
		int viewHeight = surfaceView.getHeight();
		
		// Use the difference as the cursor offset
		setOffsets(0, viewHeight - screenHeight);		
		super.onWindowFocusChanged(hasFocus);
	}

  // Native

	//@Override
	public native boolean render(int drawWidth, int drawHeight, boolean forceRedraw);

	@Override
	public native void cleanup();

	@Override
	public native boolean init(String initArg, int width, int height);

	@Override
	public native boolean inputEvent(int action, float x, float y, MotionEvent event);

	@Override
	public native boolean keyEvent(int action, int unicodeChar, int keyCode, KeyEvent event);
	
	public native void setOffsets(int x, int y);
	
	public native boolean executeSbm(String sbmCmd);
	
	public native void openConnection();
	
	public native void closeConnection();	
	public native void setDeformable(int isDeformable);
	public native String getLogStr();
	
	static
	{	
		//System.loadLibrary("python2.6");
		System.loadLibrary("sbogreapp");
	}
}

