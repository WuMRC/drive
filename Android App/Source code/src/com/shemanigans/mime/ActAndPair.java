package com.shemanigans.mime;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothManager;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.support.v4.app.NavUtils;
import android.support.v7.app.ActionBarActivity;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.ProgressBar;
import android.widget.TextView;

public class ActAndPair extends ActionBarActivity {

	// Declare final integers
	private final int REQUEST_ENABLE_BT = 2;
	private final int BLUETOOTH_ALREADY_ON = 10;

	// Declare UI elements
	private TextView infoText;
	private ProgressBar bluetoothAttempt;
	private TextView pairText;
	private Button pairButton;
	private View divider;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_act_and_pair);
		getSupportActionBar().setTitle("Scan Request");	

		initializeViewComponents();
		infoText.setText(R.string.bt_attempt);

		// Show the Up button in the action bar.
		getSupportActionBar().setDisplayHomeAsUpEnabled(true);

		final BluetoothManager bluetoothManager =
				(BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);

		BluetoothAdapter mBluetoothAdapter = bluetoothManager.getAdapter();

		Intent enableBtIntent = new Intent();

		// Check if BT is on, if not request to turn it on
		if (mBluetoothAdapter == null || !mBluetoothAdapter.isEnabled()) {
			enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
			startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
		}
		
		// if BT is already on, prompt user to go ahead and scan
		else if (mBluetoothAdapter.isEnabled() == true) {
			infoText.setText(R.string.bt_already_on);	
			onActivityResult(REQUEST_ENABLE_BT, BLUETOOTH_ALREADY_ON, enableBtIntent);
		}
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
	
	// Launch scan activity
	public void bluetoothScan(View view) {
		Intent intent = new Intent(this, Scan.class);
		startActivity(intent);
	}

	private void initializeViewComponents() {
		infoText = (TextView) findViewById(R.id.bluetooth_prompt);
		bluetoothAttempt = (ProgressBar) findViewById(R.id.bluetooth_prompt_progressbar);
		pairText = (TextView) findViewById(R.id.pairing_prompt);
		pairButton = (Button) findViewById(R.id.button_pair);
		divider = (View ) findViewById(R.id.divider);
	}

	// Processes BT request result
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {

		if (requestCode == REQUEST_ENABLE_BT && resultCode == RESULT_OK) {
			infoText.setText(R.string.bt_turned_on);
			pairButton.setVisibility(View.VISIBLE); 
			pairText.setVisibility(View.VISIBLE); 
			bluetoothAttempt.setVisibility(View.GONE); 
		}

		else if (requestCode == REQUEST_ENABLE_BT && resultCode == BLUETOOTH_ALREADY_ON) {
			infoText.setText(R.string.bt_already_on);
			divider.setVisibility(View.VISIBLE);
			pairButton.setVisibility(View.VISIBLE);
			pairText.setVisibility(View.VISIBLE); 
			bluetoothAttempt.setVisibility(View.GONE); 
		}

		else {
			infoText.setText(R.string.bt_denied);
			bluetoothAttempt.setVisibility(View.GONE);
		}
	}
}
