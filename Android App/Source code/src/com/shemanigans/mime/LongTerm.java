package com.shemanigans.mime;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;

import android.support.v4.app.DialogFragment;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Environment;
import android.os.IBinder;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentTransaction;
import android.support.v4.widget.DrawerLayout;
import android.support.v7.app.ActionBar;
import android.support.v7.app.ActionBarActivity;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.Toast;

public class LongTerm 
extends 
ActionBarActivity
implements 
NavigationDrawerFragment.NavigationDrawerCallbacks, 
NameTextFileFragment.NameTextFileListener,
BioimpFragment.OnButtonClickedListener,
SampleRateFragment.SampleRateListener,
FrequencySweepFragment.FrequencySweepListener{

	private NavigationDrawerFragment mNavigationDrawerFragment; // Fragment managing the behaviors, interactions and presentation of the navigation drawer.
	private NameTextFileFragment nameTextFileDialog;
	private SampleRateFragment sampleRateDialog;
	private FrequencySweepFragment frequencySweepDialog;

	/**
	 * Used to store the last screen title. For use in {@link #restoreActionBar()}.
	 */
	private CharSequence mTitle;
	
	private final static String TAG = LongTerm.class.getSimpleName();
	
	private static final String LIVE_DATA_TAG = "LIVE_DATA_TAG";
	private static final String PAST_HOUR_TAG = "PAST_HOUR_TAG";
	private static final String PAST_DAY_TAG = "PAST_DAY_TAG";
	private static final String EXPORT_DATA_TAG = "EXPORT_DATA_TAG";
	private static final String SCAN_TAG = "SCAN_TAG";
	
	private String mDeviceName;
	private String mDeviceAddress;
	private String textFileName = "BIdata";

	public ArrayList<String> textFile = new ArrayList<String>();
	private Calendar c = Calendar.getInstance();
	
	private BioimpFragment bioimpFrag;

	private int sampleRate = 70;
	private byte startFreq = 0;
	private byte stepSize = 0;
	private byte numOfIncrements = 0;
	
	private boolean checkNamedTextFile = false;
	private boolean mConnected = true;
	
	public double[] values = {1, 2, 3, 4};
	
	private BluetoothLeService mBluetoothLeService;
	private ServiceBinder mServiceBinder;

	// Code to manage Service life-cycle for BluetoothLeService.
	private final ServiceConnection mServiceConnectionBLE = new ServiceConnection() {

		@Override
		public void onServiceConnected(ComponentName componentName, IBinder service) {
			mBluetoothLeService = ((BluetoothLeService.LocalBinder) service).getService();
			Log.i(TAG, "Bound to BLE Service.");
		}

		@Override
		public void onServiceDisconnected(ComponentName componentName) {
			mBluetoothLeService.clientDisconnected();
		}
	};

	// Code to manage Service life-cycle for ServiceBinder.
	private final ServiceConnection mServiceConnectionBioImp = new ServiceConnection() {

		@Override
		public void onServiceConnected(ComponentName componentName, IBinder service) {
			mServiceBinder = ((ServiceBinder.LocalBinder) service).getService();
			Log.i(TAG, "Bound to ServiceBinder");
		}

		@Override
		public void onServiceDisconnected(ComponentName componentName) {
		}
	};

	private final BroadcastReceiver mGattUpdateReceiver = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent intent) {
			final String action = intent.getAction();

			if (BluetoothLeService.ACTION_GATT_DISCONNECTED.equals(action)) {
				mConnected = false;
				mTitle = getString(R.string.disconnected);
				invalidateOptionsMenu();
			} 
			else if (BluetoothLeService.ACTION_DATA_AVAILABLE_BIOIMPEDANCE.equals(action)) {
				values = intent.getDoubleArrayExtra(BluetoothLeService.EXTRA_DATA_BIOIMPEDANCE_DOUBLE);
				textFile.add(intent.getStringExtra(BluetoothLeService.EXTRA_DATA_BIOIMPEDANCE_STRING));

				// Find fragment hosted in the activity with the specified tag and update the plot
				BioimpFragment bioimpFragment = (BioimpFragment)
						getSupportFragmentManager().findFragmentByTag(LIVE_DATA_TAG);
				if (bioimpFragment != null) {
					bioimpFragment.updatePlot(values);
				}
			}
		}
	};

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_long_term);
		Intent intent = getIntent();

		// Get client information from pendingIntent in DeviceControlActivity
		mDeviceAddress = intent.getStringExtra(DeviceControlActivity.EXTRA_DEVICE_ADDRESS_BINDER);
		mDeviceName = intent.getStringExtra(DeviceControlActivity.EXTRA_DEVICE_NAME_BINDER);
		sampleRate = intent.getIntExtra(DeviceControlActivity.EXTRA_SAMPLE_RATE_BINDER, 10);
		startFreq = intent.getByteExtra(DeviceControlActivity.EXTRA_START_FREQ_BINDER, (byte) 10);
		stepSize = intent.getByteExtra(DeviceControlActivity.EXTRA_STEP_SIZE_BINDER, (byte) 10);
		numOfIncrements = intent.getByteExtra(DeviceControlActivity.EXTRA_NUM_OF_INCREMENTS_BINDER, (byte) 10);

		Intent BLE_Intent = new Intent(this, BluetoothLeService.class);
		Intent ServiceBinderIntent = new Intent(this, ServiceBinder.class);
		bindService(BLE_Intent, mServiceConnectionBLE, BIND_AUTO_CREATE);
		bindService(ServiceBinderIntent, mServiceConnectionBioImp, BIND_AUTO_CREATE);
		registerReceiver(mGattUpdateReceiver, makeGattUpdateIntentFilter());

		mNavigationDrawerFragment = (NavigationDrawerFragment)
				getSupportFragmentManager().findFragmentById(R.id.navigation_drawer);
		mTitle = getTitle();

		// Set up the navigation drawer.
		mNavigationDrawerFragment.setUp(
				R.id.navigation_drawer,
				(DrawerLayout) findViewById(R.id.drawer_layout));
	}

	@Override
	protected void onStop() {
		super.onStop();
		finish();
	}

	@Override
	public void onDestroy() {
		super.onDestroy();
		unregisterReceiver(mGattUpdateReceiver);
		unbindService(mServiceConnectionBLE);
		unbindService(mServiceConnectionBioImp);
	}

	@Override
	public void onNavigationDrawerItemSelected(int position) {
		// update the main content by updating the fragment BioimpFragment
		FragmentManager fragmentManager = getSupportFragmentManager();
		FragmentTransaction fragmentTransaction = fragmentManager.beginTransaction();
		switch (position) {
		// Depending on the item selected in the list, add unique tags / identifiers.
		case 0:
			fragmentTransaction
			.replace(R.id.container, BioimpFragment.newInstance(position + 1), LIVE_DATA_TAG);			
			break;
		case 1:
			fragmentTransaction
			.replace(R.id.container, BioimpFragment.newInstance(position + 1), PAST_HOUR_TAG);			
			break;
		case 2:
			fragmentTransaction
			.replace(R.id.container, BioimpFragment.newInstance(position + 1), PAST_DAY_TAG);			
			break;
		case 3:
			fragmentTransaction
			.replace(R.id.container, BioimpFragment.newInstance(position + 1), EXPORT_DATA_TAG);			
			break;
		case 4:
			fragmentTransaction
			.replace(R.id.container, BioimpFragment.newInstance(position + 1), SCAN_TAG);			
			break;
		}
		fragmentTransaction.commit();
	}

	public void onSectionAttached(int number) {
		switch (number) {
		// When a fragment is attached to the hosting activity, pass the connected device's name.
		case 1:
			bioimpFrag = (BioimpFragment)
			getSupportFragmentManager().findFragmentByTag(LIVE_DATA_TAG);
			bioimpFrag.setDeviceName(mDeviceName);
			bioimpFrag.setDeviceAddress(mDeviceAddress);
			bioimpFrag.updateSampleRate(sampleRate);
			bioimpFrag.UpdateFrequencyParams(startFreq, stepSize, numOfIncrements);

			if(mConnected == true) {
				mTitle = getString(R.string.live_data);
			}
			else {
				mTitle = getString(R.string.disconnected);
			}
			break;
		case 2:
			if(mConnected == true) {
				mTitle = getString(R.string.past_hour);
			}
			else {
				mTitle = getString(R.string.disconnected);
			}
			break;
		case 3:
			if(mConnected == true) {
				mTitle = getString(R.string.past_12);
			}
			else {
				mTitle = getString(R.string.disconnected);
			}
			break;
		case 4:
			if(mConnected == true) {
				mTitle = getString(R.string.data_export);
			}
			else {
				mTitle = getString(R.string.disconnected);
			}
			break;
		case 5:
			if(mConnected == true) {
				mTitle = getString(R.string.back_to_scan);
			}
			else {
				mTitle = getString(R.string.disconnected);
			}
			mBluetoothLeService.stopForeground(true);
			mBluetoothLeService.disconnect();
			Intent intent = new Intent(this, ServiceBinder.class);
			stopService(intent);
			Intent scan = new Intent(this, Scan.class);
			startActivity(scan);
			finish();
			break;
		}
	}

	public void restoreActionBar() {
		ActionBar actionBar = getSupportActionBar();
		//actionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_STANDARD);
		actionBar.setDisplayShowTitleEnabled(true);
		actionBar.setTitle(mTitle);
	}


	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		if (!mNavigationDrawerFragment.isDrawerOpen()) {
			// Only show items in the action bar relevant to this screen
			// if the drawer is not showing. Otherwise, let the drawer
			// decide what to show in the action bar.
			getMenuInflater().inflate(R.menu.long_term, menu);
			restoreActionBar();
			if (mConnected) {    	
				menu.findItem(R.id.menu_disconnect).setVisible(true);
			} else {
				menu.findItem(R.id.menu_disconnect).setVisible(false);
			}
			return true;
		}
		return super.onCreateOptionsMenu(menu);
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch(item.getItemId()) {

		case R.id.menu_disconnect:
			mBluetoothLeService.stopForeground(true);
			mBluetoothLeService.disconnect();
			Intent intent = new Intent(this, ServiceBinder.class);
			stopService(intent);
			Log.i(TAG, "Attempted disconnect.");
			mBluetoothLeService.close();
			return true;
		case R.id.name_text_file:
			showTextFileDialog();
			Log.i(TAG, "Attempted text file naming.");
			return true;
		case R.id.set_sample_rate:
			setSampleRate();
			return true;
		case R.id.set_frequency_sweep:
			setFrequencySweep();
			return true;
		}
		return super.onOptionsItemSelected(item);
	}

	public void clearTextFile() {		
		textFile = new ArrayList<String>();
		Toast.makeText(getBaseContext(),
				"Database sucessfully purged",
				Toast.LENGTH_SHORT).show();
	}

	public void readText() {
		File DataDir = new File(Environment.getExternalStorageDirectory() + "/Biohm/");

		//Get the text file
		File duodecimalMinute = new File(DataDir, "duodecimalMinute.txt");

		//Read text from file
		StringBuilder text = new StringBuilder();

		try {
			BufferedReader br = new BufferedReader(new FileReader(duodecimalMinute));
			String line;

			while ((line = br.readLine()) != null) {
				text.append(line);
				text.append('\n');
			}
			br.close();
		}
		catch (IOException e) {
			//TODO need to add proper error handling here
		}

	}

	public void exportToText() {
		// write on SD card file data in the text box
		try {
			SimpleDateFormat sdf = new SimpleDateFormat("yyyyMMdd_HHmmss");
			String strDate = sdf.format(c.getTime());

			// check if User has changed filename
			if (checkNamedTextFile == false) {
				textFileName = "AccelData" + strDate;
			}
			else {
				textFileName = nameTextFileDialog.getName();
			}
			// set checkNamedTextFile back to false to revert back to default naming scheme.
			checkNamedTextFile = false;

			File bioImpDataDir = new File(Environment.getExternalStorageDirectory() + "/Mime/");	

			bioImpDataDir.mkdirs();			

			File bioImpData = new File(bioImpDataDir, textFileName + ".txt");			
			bioImpData.createNewFile();
			FileOutputStream fOut = new FileOutputStream(bioImpData);
			OutputStreamWriter myOutWriter = new OutputStreamWriter(fOut);

			myOutWriter.append(
					fixedLengthString("X", 6) 
					+ fixedLengthString("Y", 6)
					+ fixedLengthString("Z", 6)
					+ fixedLengthString("Ω", 9)
					+ fixedLengthString("θ", 8)
					+ fixedLengthString("KHz", 4)
					+ "\n");
			for (int i = 0; i < textFile.size(); i++) {
				myOutWriter.append(textFile.get(i));
				myOutWriter.append("\n");
			}

			myOutWriter.close();
			fOut.close();
			Toast.makeText(getBaseContext(),
					"Done writing to SD Card " + textFileName + ".txt",
					Toast.LENGTH_SHORT).show();

			textFile = new ArrayList<String>();
		} 
		catch (Exception e) {
			Toast.makeText(getBaseContext(), e.getMessage(),
					Toast.LENGTH_SHORT).show();
		}
	}

	public static String fixedLengthString(String string, int length) {
		return String.format("%-"+length+ "s", string);
	}

	public void showTextFileDialog() {
		// Create an instance of the dialog fragment and show it
		nameTextFileDialog = new NameTextFileFragment();
		nameTextFileDialog.show(getSupportFragmentManager(), "NameTextFileFragment");
	}

	// The dialog fragment receives a reference to this Activity through the
	// Fragment.onAttach() callback, which it uses to call the following methods
	// defined by the NoticeDialogFragment.NoticeDialogListener interface
	@Override
	public void onDialogPositiveClickNameTextFile(DialogFragment dialog) {
		// User touched the dialog's positive button
		//checkNamedTextFile
		checkNamedTextFile = true;
	}

	@Override
	public void onDialogNegativeClickNameTextFile(DialogFragment dialog) {
		// User touched the dialog's negative button
		checkNamedTextFile = false;
	}

	public void setSampleRate() {
		// Create an instance of the dialog fragment and show it
		sampleRateDialog = new SampleRateFragment();
		Bundle args = new Bundle();
		args.putInt(DeviceControlActivity.EXTRA_SAMPLE_RATE_BINDER, sampleRate);
		Log.i(TAG, "Sample rate passed to dialog: " + String.valueOf(sampleRate));
		sampleRateDialog.setArguments(args);
		sampleRateDialog.show(getSupportFragmentManager(), "SampleRateFragment");
	}


	@Override
	public void onDialogPositiveClickSampleRate(DialogFragment dialog) {
		// User touched the dialog's positive button
		// Set Value
		try {
			sampleRate = Integer.parseInt(sampleRateDialog.getValue());
			Toast.makeText(this, "New frequency: " + sampleRateDialog.getValue(), Toast.LENGTH_SHORT).show();
			mServiceBinder.writeSampleRateCharacteristic(sampleRate);
			if(bioimpFrag.getTag() == LIVE_DATA_TAG) {
				bioimpFrag.setSampleRateTextView(sampleRate);
			}
		}
		catch (Exception e) {
			Toast.makeText(this, R.string.set_fail, Toast.LENGTH_SHORT).show();
		}
	}

	@Override
	public void onDialogNegativeClickSampleRate(DialogFragment dialog) {
		// User touched the dialog's negative button
	}

	public void setFrequencySweep() {
		// Create an instance of the dialog fragment and show it
		frequencySweepDialog = new FrequencySweepFragment();
		Bundle args = new Bundle();
		args.putString(DeviceControlActivity.EXTRA_START_FREQ_BINDER, Byte.toString(startFreq));
		args.putString(DeviceControlActivity.EXTRA_STEP_SIZE_BINDER, Byte.toString(stepSize));
		args.putString(DeviceControlActivity.EXTRA_NUM_OF_INCREMENTS_BINDER, Byte.toString(numOfIncrements));
		frequencySweepDialog.setArguments(args);
		frequencySweepDialog.show(getSupportFragmentManager(), "FrequencySweepFragment");
	}

	@Override
	public void onDialogPositiveClickFrequencySweep(DialogFragment dialog) {
		// User touched the dialog's positive button
		// Set Value
		String[] freqValuesString;
		freqValuesString = frequencySweepDialog.getValue();
		try {
			startFreq = (byte) (Integer.parseInt(freqValuesString[0]));
			stepSize = (byte) (Integer.parseInt(freqValuesString[1]));
			numOfIncrements = (byte) (Integer.parseInt(freqValuesString[2]));
			if((startFreq + (stepSize * numOfIncrements) > 120) || stepSize > 125 || numOfIncrements < 0) {
				throw new IllegalArgumentException();
			}
			if(startFreq == 0 || stepSize == 0 || numOfIncrements == 0) {
				throw new IllegalArgumentException();
			}
			byte[] freqValuesByte = {startFreq, stepSize, numOfIncrements}; 
			Toast.makeText(this, R.string.freq_sweep_enabled, Toast.LENGTH_SHORT).show();
			mServiceBinder.writeFrequencySweepCharacteristic(freqValuesByte);
			if(bioimpFrag.getTag() == LIVE_DATA_TAG) {
				bioimpFrag.setAcFreqTextViewParams(startFreq, stepSize, numOfIncrements);
			}
		}
		catch (Exception e) {
			if(e instanceof IllegalArgumentException) {
				Toast.makeText(this, R.string.check_value, Toast.LENGTH_SHORT).show();
			}
			else {
				Toast.makeText(this, R.string.set_fail, Toast.LENGTH_SHORT).show();
			}
		}
	}

	@Override
	public void onDialogNeutralClickFrequencySweep(DialogFragment dialog) {
		Toast.makeText(this, R.string.cancelled, Toast.LENGTH_SHORT).show();
	}

	@Override
	public void onDialogNegativeClickFrequencySweep(DialogFragment dialog) {
		// User touched the dialog's negative button
		String[] freqValuesString;
		freqValuesString = frequencySweepDialog.getValue();
		try {
			startFreq = (byte) (Integer.parseInt(freqValuesString[0]));
			stepSize = (byte) (Integer.parseInt(freqValuesString[1]));
			numOfIncrements = (byte) (Integer.parseInt(freqValuesString[2]));
			if(startFreq == 0) {
				throw new IllegalArgumentException();
			}
			byte[] freqValuesByte = {startFreq, stepSize, numOfIncrements}; 
			Toast.makeText(this, R.string.freq_sweep_disabled, Toast.LENGTH_SHORT).show();
			mServiceBinder.writeFrequencySweepCharacteristic(freqValuesByte);
			if(bioimpFrag.getTag() == LIVE_DATA_TAG) {
				bioimpFrag.setAcFreqTextViewParams(startFreq, stepSize, numOfIncrements);
			}
		}
		catch (Exception e) {
			if(e instanceof IllegalArgumentException) {
				Toast.makeText(this, R.string.no_zero, Toast.LENGTH_SHORT).show();
			}
			else {
				Toast.makeText(this, R.string.set_fail, Toast.LENGTH_SHORT).show();
			}
		}
	}


	private static IntentFilter makeGattUpdateIntentFilter() {
		final IntentFilter intentFilter = new IntentFilter();
		intentFilter.addAction(BluetoothLeService.ACTION_GATT_CONNECTED);
		intentFilter.addAction(BluetoothLeService.ACTION_GATT_DISCONNECTED);
		intentFilter.addAction(BluetoothLeService.ACTION_GATT_SERVICES_DISCOVERED);
		intentFilter.addAction(BluetoothLeService.ACTION_DATA_AVAILABLE);
		intentFilter.addAction(BluetoothLeService.ACTION_DATA_AVAILABLE_BIOIMPEDANCE);
		return intentFilter;
	}

}
