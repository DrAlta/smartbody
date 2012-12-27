package com.android.sbmjni;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.util.Log;
import android.view.WindowManager;

import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.TextView;
import android.view.LayoutInflater;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.Menu;
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
        SbmJNILib.executeSbm("time pause"); 
        //mView.onPause();
    }

    @Override protected void onResume() {
    	/*
    	try {
    		Thread.sleep(1000*15);
    		
    	}
    	catch (Exception e)
    	{
    	
    	}
    	*/
        super.onResume();
        SbmJNILib.executeSbm("time resume"); 
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
        	onResume();
            break;
        case R.id.PauseSim:
        	onPause();
            break;
        case R.id.ShowLog:   
        	showDialog(0);
        	break;
        case R.id.Connect:
        	SbmJNILib.openConnection();
            break;
        case R.id.Disconnect:
        	SbmJNILib.closeConnection();
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
    	String logs = SbmJNILib.getLog();
    	text.setText(logs);
  	  	//text.setText("fjajfiosajgihagosdghaogiha");    	
    }    
    public AlertDialog createLogDialog(){    	
    	  AlertDialog.Builder ad = new AlertDialog.Builder(this);
    	  ad.setIcon(R.drawable.icon);
    	  ad.setTitle("Smartbody Log");   	    
    	  ad.setView(LayoutInflater.from(this).inflate(R.layout.custom_dialog,null));    	  
    	  ad.setPositiveButton("OK", new android.content.DialogInterface.OnClickListener() {
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
        	//SbmJNILib.executeSbm(sbmCmd); 
        	SbmJNILib.executePython(sbmCmd);  
        	sbmTextEdit.getText().clear();
        }
    };
}