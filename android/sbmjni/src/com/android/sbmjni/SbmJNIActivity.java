package com.android.sbmjni;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.WindowManager;

import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.view.View;
import android.view.View.OnClickListener;



import java.io.File;


public class SbmJNIActivity extends Activity {
    @Override protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);        
        setContentView(R.layout.main);
        
        Button button = (Button)findViewById(R.id.SbmButton);        
        button.setOnClickListener(sbmListenser);
    }

    @Override protected void onPause() {
        super.onPause();
        //mView.onPause();
    }

    @Override protected void onResume() {
        super.onResume();
        //mView.onResume();
    }
    
    private OnClickListener sbmListenser = new OnClickListener()
    {
        public void onClick(View v)
        {
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
        	SbmJNILib.executeSbm(sbmCmd); 
        	sbmTextEdit.getText().clear();
        }
    };
}