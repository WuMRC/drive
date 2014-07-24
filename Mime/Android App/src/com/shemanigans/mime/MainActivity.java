package com.shemanigans.mime;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.Menu;
import android.view.View;


public class MainActivity extends Activity {
	public final static String EXTRA_MESSAGE = "com.shemanigans.mime.MESSAGE";

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
	}


	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	/** Called when the user clicks the Send button */
	
	/*
	public void sendMessage(View view) {
		// Do something in response to button
		Intent intent = new Intent(this, DisplayMessageActivity.class);
		EditText editText = (EditText) findViewById(R.id.edit_message);
		String message = editText.getText().toString();
		intent.putExtra(EXTRA_MESSAGE, message);
		startActivity(intent);
	}
	*/

	/** Called when the user clicks the Yes button to turn on Bluetooth*/
	public void buttonYes(View view) {
		// Do something in response to button
		Intent intent = new Intent(this, ActAndPair.class);
		String message = "Requesting to turn on Bluetooth...";
		intent.putExtra(EXTRA_MESSAGE, message);
		startActivity(intent);
	}
	
	public void buttonNo(View view) {
		// Do something in response to button
		Intent intent = new Intent(this, ButtonNo.class);
		startActivity(intent);
	}




}
