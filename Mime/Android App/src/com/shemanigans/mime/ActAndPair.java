package com.shemanigans.mime;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothManager;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.support.v4.app.NavUtils;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.ProgressBar;
import android.widget.TextView;

public class ActAndPair extends Activity {

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_act_and_pair);
		
		getActionBar().setTitle("Scan Request");

		// Show the Up button in the action bar.
		setupActionBar();
		

		// Get the message from the intent
		TextView textView = (TextView) findViewById(R.id.bluetooth_prompt);
		textView.setTextSize(20);
		textView.setText("Attempting to turn on Bluetooth...");

		/*
		// Create the text view
		TextView textView = new TextView(this);
		textView.setTextSize(40);
		textView.setText(message);

		setContentView(textView);

		 */

		// Show the Up button in the action bar.
		setupActionBar();

		final BluetoothManager bluetoothManager =
				(BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
		BluetoothAdapter mBluetoothAdapter = bluetoothManager.getAdapter();

		Intent enableBtIntent = new Intent();
		int REQUEST_ENABLE_BT = 2;
		int BLUETOOTH_ALREADY_ON = 10;


		if (mBluetoothAdapter == null || !mBluetoothAdapter.isEnabled()) {
			enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
			startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
			setResult (RESULT_OK, enableBtIntent);		
		}

		else if (mBluetoothAdapter.isEnabled() == true) {
			textView.setText("Bluetooth is already on.");	
			onActivityResult(REQUEST_ENABLE_BT, BLUETOOTH_ALREADY_ON, enableBtIntent);
		}
	}

	/**
	 * Set up the {@link android.app.ActionBar}.
	 */

	protected void onActivityResult (int requestCode, int resultCode, Intent data) {

		if (requestCode == 2 && resultCode == RESULT_OK) {
			TextView textView = (TextView) findViewById(R.id.bluetooth_prompt);
			textView.setTextSize(20);
			textView.setText("Bluetooth was sucessfully turned on.");
			ProgressBar bluetoothAttempt = (ProgressBar) findViewById(R.id.bluetooth_prompt_progressbar);

			TextView pairText = (TextView) findViewById(R.id.pairing_prompt);
			Button pairButton = (Button) findViewById(R.id.button_pair);
			pairButton.setVisibility(0); // Correct to VISIBLE sometime
			pairText.setVisibility(0); // Correct to VISIBLE sometime
			bluetoothAttempt.setVisibility(8); // Correct to GONE sometime
		}

		else if (requestCode == 2 && resultCode == 10) {
			TextView textView = (TextView) findViewById(R.id.bluetooth_prompt);
			textView.setTextSize(20);
			textView.setText("Bluetooth is already on.");
			ProgressBar bluetoothAttempt = (ProgressBar) findViewById(R.id.bluetooth_prompt_progressbar);

			TextView pairText = (TextView) findViewById(R.id.pairing_prompt);
			Button pairButton = (Button) findViewById(R.id.button_pair);
			pairButton.setVisibility(0); // Correct to VISIBLE sometime
			pairText.setVisibility(0); // Correct to VISIBLE sometime
			bluetoothAttempt.setVisibility(8); // Correct to GONE sometime
		}

		else {
			TextView textView = (TextView) findViewById(R.id.bluetooth_prompt);
			ProgressBar bluetoothAttempt = (ProgressBar) findViewById(R.id.bluetooth_prompt_progressbar);

			textView.setTextSize(20);
			textView.setText("It's either you denied the bluetooth request, or there is a problem with your bluetooth adapter. Please try again later.");
			bluetoothAttempt.setVisibility(8); // Correct to GONE sometime

		}

	}

	/**
	 * Set up the {@link android.app.ActionBar}.
	 */

	private void setupActionBar() {

		getActionBar().setDisplayHomeAsUpEnabled(true);

	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.act_and_pair, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch (item.getItemId()) {
		case android.R.id.home:
			// This ID represents the Home or Up button. In the case of this
			// activity, the Up button is shown. Use NavUtils to allow users
			// to navigate up one level in the application structure. For
			// more details, see the Navigation pattern on Android Design:
			//
			// http://developer.android.com/design/patterns/navigation.html#up-vs-back
			//
			NavUtils.navigateUpFromSameTask(this);
			return true;
		}
		return super.onOptionsItemSelected(item);
	}

	public void bluetoothScan(View view) {
		// Do something in response to button
		Intent intent = new Intent(this, Scan.class);
		startActivity(intent);
	}


}
