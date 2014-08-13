package com.shemanigans.mime;

import android.os.Bundle;
import android.app.Activity;
import android.view.Menu;

public class AccelerometerDataParser extends Activity {

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_accelerometer_data_parser);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.accelerometer_data_parser, menu);
		return true;
	}

}
