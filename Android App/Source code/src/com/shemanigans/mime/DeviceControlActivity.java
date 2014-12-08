package com.shemanigans.mime;

import java.io.File;
import java.io.FileOutputStream;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.HashMap;
import java.util.List;

import android.app.Notification;
import android.app.PendingIntent;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.graphics.Color;
import android.graphics.Paint;
import android.os.Bundle;
import android.os.Environment;
import android.os.IBinder;
import android.support.v4.app.DialogFragment;
import android.support.v7.app.ActionBarActivity;
import android.support.v7.widget.SwitchCompat;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.ExpandableListView;
import android.widget.ProgressBar;
import android.widget.RelativeLayout;
import android.widget.SimpleExpandableListAdapter;
import android.widget.TableLayout;
import android.widget.TextView;
import android.widget.Toast;

import com.androidplot.xy.BoundaryMode;
import com.androidplot.xy.LineAndPointFormatter;
import com.androidplot.xy.SimpleXYSeries;
import com.androidplot.xy.XYPlot;

/**
 * For a given BLE device, this Activity provides the user interface to connect, display data,
 * and display GATT services and characteristics supported by the device.  The Activity
 * communicates with {@code BluetoothLeService}, which in turn interacts with the
 * Bluetooth LE API.
 */
public class DeviceControlActivity 
extends ActionBarActivity 
implements 
NameTextFileFragment.NameTextFileListener,
SampleRateFragment.SampleRateListener,
FrequencySweepFragment.FrequencySweepListener{

	private final static String TAG = DeviceControlActivity.class.getSimpleName();

	public static final String EXTRAS_DEVICE_NAME = "DEVICE_NAME";
	public static final String EXTRAS_DEVICE_ADDRESS = "DEVICE_ADDRESS";

	// Final strings used to identify arguments passed from one class to another
	public static final String EXTRA_DEVICE_NAME_BINDER = "DEVICE_NAME_BINDER";
	public static final String EXTRA_DEVICE_ADDRESS_BINDER = "DEVICE_ADDRESS_BINDER";
	public static final String EXTRA_SAMPLE_RATE_BINDER = "SAMPLE_RATE_BINDER";
	public static final String EXTRA_START_FREQ_BINDER = "START_FREQ_BINDER";
	public static final String EXTRA_STEP_SIZE_BINDER = "STEP_SIZE_BINDER";
	public static final String EXTRA_NUM_OF_INCREMENTS_BINDER = "NUM_OF_INCREMENTS_BINDER";

	private String mDeviceName;
	private String mDeviceAddress;
	private ExpandableListView mGattServicesList;
	private BluetoothLeService mBluetoothLeService;
	private ArrayList<ArrayList<BluetoothGattCharacteristic>> mGattCharacteristics =
			new ArrayList<ArrayList<BluetoothGattCharacteristic>>();
	private boolean mConnected = false;
	private BluetoothGattCharacteristic mNotifyCharacteristic;
	private BluetoothGattCharacteristic mSampleRateCharacteristic;
	private BluetoothGattCharacteristic mACFrequencyCharacteristic;

	public ArrayList<String> textFile = new ArrayList<String>();	
	private String textFileName = "BIdata";	
	private Calendar c = Calendar.getInstance();
	private boolean checkNamedTextFile = false;
	private double[] imp = {1, 2, 3, 4};
	private int sampleRate = 50;
	private byte startFreq = 0;
	private byte stepSize = 0;
	private byte numOfIncrements = 0;

	// Declare UI references	
	private TextView mConnectionState;
	private TextView mDataField;
	private TextView mDeviceAddressTextView;
	private TextView sampleRateTextView;
	private TextView startFreqTextView;
	private TextView stepSizeTextView;
	private TextView numOfIncrementsTextView;
	private TextView summaryTableHeader;
	private Button startButton;
	private View startButtonBar;
	private SwitchCompat enableNotifications;
	private ProgressBar connectionStateBar;
	private RelativeLayout textFileButtons;
	private TableLayout summaryTable;

	private Intent serviceIntent; 
	private Notification notification;
	private Intent activityIntent;
	private PendingIntent activityPendingIntent;

	// Graph variables
	private static final int HISTORY_SIZE = 360;
	private XYPlot bioimpedancePlot = null;
	private SimpleXYSeries accelXseries = null;
	private SimpleXYSeries accelYseries = null;
	private SimpleXYSeries accelZseries = null;
	private SimpleXYSeries bioimpedanceSeries = null;

	// Fragment to set name
	NameTextFileFragment nameTextFileDialog;
	SampleRateFragment sampleRateDialog;
	FrequencySweepFragment frequencySweepDialog;

	private final String LIST_NAME = "NAME";
	private final String LIST_UUID = "UUID";
	private final int ONGOING_NOTIFICATION_ID = 1;

	// Service life-cycle management.
	private final ServiceConnection mServiceConnection = new ServiceConnection() {

		@Override
		public void onServiceConnected(ComponentName componentName, IBinder service) {
			mBluetoothLeService = ((BluetoothLeService.LocalBinder) service).getService();
			Log.i(TAG, "mBluetoothLeService first initialized.");

			if (!mBluetoothLeService.initialize()) {
				Log.e(TAG, "Unable to initialize Bluetooth");
				finish();
			}
			// Automatically connects to the device upon successful start-up initialization.
			mBluetoothLeService.connect(mDeviceAddress);
			mBluetoothLeService.clientConnected();
		}

		@Override
		public void onServiceDisconnected(ComponentName componentName) {
			mBluetoothLeService.clientDisconnected();
			mBluetoothLeService = null;
		}
	};

	// Handles various events fired by the Service.
	// ACTION_GATT_CONNECTED: connected to a GATT server.
	// ACTION_GATT_DISCONNECTED: disconnected from a GATT server.
	// ACTION_GATT_SERVICES_DISCOVERED: discovered GATT services.
	// ACTION_DATA_AVAILABLE_SAMPLE_RATE: read sample rate of GATT client.
	// ACTION_DATA_AVAILABLE_FREQUENCY_PARAMS: read frequency parameters of GATT client.
	// ACTION_DATA_AVAILABLE_BIOIMPEDANCE: Received data from BI characteristic.
	// ACTION_DATA_AVAILABLE: received data from the device.  This can be a result of read
	// or notification operations from other characteristics.

	private final BroadcastReceiver mGattUpdateReceiver = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent intent) {
			final String action = intent.getAction();
			if (BluetoothLeService.ACTION_GATT_CONNECTED.equals(action)) {
				mConnected = true;
				updateConnectionState(R.string.connected);
				enableNotifications.setClickable(true);
				connectionStateBar.setVisibility(View.GONE);
				summaryTable.setVisibility(View.VISIBLE);
				summaryTableHeader.setVisibility(View.VISIBLE);
				startButton.setVisibility(View.VISIBLE);
				startButtonBar.setVisibility(View.VISIBLE);
				invalidateOptionsMenu();
			} 
			else if (BluetoothLeService.ACTION_GATT_CONNECTING.equals(action)) {
				mConnected = false;
				connectionStateBar.getIndeterminateDrawable().setColorFilter(
						getResources().getColor(R.color.appbasetheme_color),
						android.graphics.PorterDuff.Mode.SRC_IN);
				connectionStateBar.setVisibility(View.VISIBLE);
				updateConnectionState(R.string.connecting);
				invalidateOptionsMenu();
			}
			else if (BluetoothLeService.ACTION_GATT_DISCONNECTED.equals(action)) {
				mConnected = false;
				updateConnectionState(R.string.disconnected);
				enableNotifications.setChecked(false);
				enableNotifications.setClickable(false);
				startButton.setVisibility(View.GONE);
				startButtonBar.setVisibility(View.GONE);
				summaryTable.setVisibility(View.GONE);
				summaryTableHeader.setVisibility(View.GONE);
				mBluetoothLeService.stopForeground(true);
				mBluetoothLeService.close();
				invalidateOptionsMenu();
				clearUI();
				startButton.setTextColor(Color.WHITE);
				startButton.setBackgroundColor(getResources().getColor(R.color.appbasetheme_color));
			} 
			else if (BluetoothLeService.ACTION_GATT_SERVICES_DISCOVERED.equals(action)) {
				// Show all the supported services and characteristics on the user interface.
				displayGattServices(mBluetoothLeService.getSupportedGattServices());
			} 
			else if (BluetoothLeService.ACTION_DATA_AVAILABLE.equals(action)) {
				displayData(intent.getStringExtra(BluetoothLeService.EXTRA_DATA));
			}
			else if (BluetoothLeService.ACTION_DATA_AVAILABLE_SAMPLE_RATE.equals(action)) {
				sampleRate = intent.getByteExtra(BluetoothLeService.EXTRA_DATA_SAMPLE_RATE, Byte.valueOf("0"));
				setSampleRateTextView(sampleRate);
				Log.i(TAG, "Default sample rate: " + sampleRate);
			}
			else if (BluetoothLeService.ACTION_DATA_AVAILABLE_FREQUENCY_PARAMS.equals(action)) {
				byte[] freqParams = intent.getByteArrayExtra(BluetoothLeService.EXTRA_DATA_FREQUENCY_PARAMS);
				startFreq = freqParams[0];
				stepSize = freqParams[1];
				numOfIncrements = freqParams[2];
				setAcFreqTextViewParams(startFreq, stepSize, numOfIncrements);
				Log.i(TAG, "Default frequency start frequency: " + startFreq);
			}
			else if (BluetoothLeService.ACTION_DATA_AVAILABLE_BIOIMPEDANCE.equals(action)) {
				displayData("\n"
						+ fixedLengthString("X", 6)
						+ fixedLengthString("Y", 6)
						+ fixedLengthString("Z", 6)
						+ fixedLengthString("Ω", 9)
						+ fixedLengthString("θ", 6)
						+ fixedLengthString("KHz", 4)
						+ "\n"												
						+ intent.getStringExtra(BluetoothLeService.EXTRA_DATA_BIOIMPEDANCE_STRING));
				textFile.add(intent.getStringExtra(BluetoothLeService.EXTRA_DATA_BIOIMPEDANCE_STRING));
				imp = intent.getDoubleArrayExtra(BluetoothLeService.EXTRA_DATA_BIOIMPEDANCE_DOUBLE);

				// update instantaneous data:

				// get rid the oldest sample in history:
				if (accelXseries.size() > HISTORY_SIZE) {		        	
					accelXseries.removeFirst();
					accelYseries.removeFirst();
					accelZseries.removeFirst();
					bioimpedanceSeries.removeFirst();
				}

				// add the latest history sample:
				accelXseries.addLast(null, imp[0]);
				accelYseries.addLast(null, imp[1]);
				accelZseries.addLast(null, imp[2]);
				bioimpedanceSeries.addLast(null, imp[3]);

				// redraw the Plots:
				bioimpedancePlot.redraw();
			}
		}
	};

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		Intent gattServiceIntent = new Intent(this, BluetoothLeService.class);
		bindService(gattServiceIntent, mServiceConnection, BIND_AUTO_CREATE);
		setContentView(R.layout.gatt_services_characteristics);

		final Intent intent = getIntent();
		mDeviceName = intent.getStringExtra(EXTRAS_DEVICE_NAME);
		mDeviceAddress = intent.getStringExtra(EXTRAS_DEVICE_ADDRESS);

		// Set up UI references.
		initializeViewComponents();
		mDeviceAddressTextView.setText(mDeviceAddress);
		mGattServicesList.setOnChildClickListener(servicesListClickListner);
		mDeviceName = mDeviceName.substring(0, mDeviceName.length() - 9);
		bioimpedancePlot = (XYPlot) findViewById(R.id.bioimpedancePlot);
		getSupportActionBar().setTitle(mDeviceName);
		getSupportActionBar().setDisplayHomeAsUpEnabled(true);

		// Set up BI plots
		setupPlot();
	}

	@Override
	protected void onResume() {
		super.onResume();
		registerReceiver(mGattUpdateReceiver, makeGattUpdateIntentFilter());
		if (mBluetoothLeService != null) {
		}
	}

	@Override
	protected void onPause() {
		super.onPause();
		unregisterReceiver(mGattUpdateReceiver);
	}

	@Override
	protected void onStop() {
		super.onStop();
		finish();
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
		unbindService(mServiceConnection);
		/*if(mBluetoothLeService.getNumberOfBoundClients() == 2) {
			mBluetoothLeService.setDeviceName(mDeviceName);
			mBluetoothLeService.setDeviceAdress(mDeviceAddress);
			Log.i(TAG, "Names have been saved.");
		}*/
		Log.i(TAG, "Activity destroyed.");
	}

	@Override
	public void onSaveInstanceState(Bundle savedInstanceState) {
		// mBluetoothLeService.setDeviceName(mDeviceName);
		// mBluetoothLeService.setDeviceAdress(mDeviceAddress);
		Log.i(TAG, "Attempted state and name save.");
		super.onSaveInstanceState(savedInstanceState);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		getMenuInflater().inflate(R.menu.gatt_services, menu);
		if (mConnected) {    	
			menu.findItem(R.id.menu_refresh).setVisible(false);
			menu.findItem(R.id.menu_connect).setVisible(false);
			menu.findItem(R.id.menu_disconnect).setVisible(true);
		} else {
			menu.findItem(R.id.menu_refresh).setVisible(false);
			menu.findItem(R.id.menu_connect).setVisible(true);
			menu.findItem(R.id.menu_disconnect).setVisible(false);
		}
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch(item.getItemId()) {
		case R.id.menu_connect:
			mBluetoothLeService.connect(mDeviceAddress);
			return true;
		case R.id.menu_disconnect:
			mBluetoothLeService.stopForeground(true);
			mBluetoothLeService.disconnect();
			//mBluetoothLeService.close();
			return true;
		case android.R.id.home:
			onBackPressed();
			return true;
		case R.id.name_text_file:
			setTextFileName();
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

	public void clearTextFile(View view) {		
		textFile = new ArrayList<String>();
		Toast.makeText(getBaseContext(),
				"Database sucessfully purged",
				Toast.LENGTH_SHORT).show();
	}

	// If a given GATT characteristic is selected, check for supported features.  This sample
	// demonstrates 'Read' and 'Notify' features.  See
	// http://d.android.com/reference/android/bluetooth/BluetoothGatt.html for the complete
	// list of supported characteristic features.
	
	// This ExpandableListView is always hidden. Code is kept for posterity

	private final ExpandableListView.OnChildClickListener servicesListClickListner =
			new ExpandableListView.OnChildClickListener() {
		@Override
		public boolean onChildClick(ExpandableListView parent, View v, int groupPosition,
				int childPosition, long id) {

			// Hide export to text button

			RelativeLayout buttons = (RelativeLayout) findViewById(R.id.buttons);
			buttons.setVisibility(View.GONE);

			com.androidplot.xy.XYPlot data = (com.androidplot.xy.XYPlot) findViewById(R.id.bioimpedancePlot);
			data.setVisibility(View.GONE);

			if (mGattCharacteristics != null) {
				final BluetoothGattCharacteristic characteristic =
						mGattCharacteristics.get(groupPosition).get(childPosition);
				final int charaProp = characteristic.getProperties();
				if ((charaProp & BluetoothGattCharacteristic.PROPERTY_READ) > 0) {

					// If there is an active notification on a characteristic, clear
					// it first so it doesn't update the data field on the user interface.

					//also uncheck notifications switch that may have been previously checked.
					enableNotifications.setChecked(false);
					enableNotifications.setVisibility(View.GONE); 

					if (mNotifyCharacteristic != null) {
						mBluetoothLeService.removeCharacteristicNotification(mNotifyCharacteristic, false);
						mNotifyCharacteristic = null;
					}
					mBluetoothLeService.readCharacteristic(characteristic);
				}
				if ((charaProp & BluetoothGattCharacteristic.PROPERTY_NOTIFY) > 0) {
					enableNotifications.setVisibility(View.VISIBLE);
					mNotifyCharacteristic = characteristic;
				}
				if ((charaProp & BluetoothGattCharacteristic.PROPERTY_WRITE) > 0) {
				}
				return true;
			}
			return false;
		}
	};

	public void enableNotifications(View view) {	
		enableNotifications.setVisibility(View.VISIBLE);

		// Turn off notifications.
		if(enableNotifications.isChecked() == false) {		
			Intent intent = new Intent(this, ServiceBinder.class);
			stopService(intent);
			textFileButtons.setVisibility(View.VISIBLE);
			mBluetoothLeService.removeCharacteristicNotification(mNotifyCharacteristic, false);
			mBluetoothLeService.stopForeground(true);
		}

		// Turn on notifications.
		else {
			startSampling();
		}
	}

	public void startSampling(View view) {
		enableNotifications.setVisibility(View.VISIBLE);
		enableNotifications.setChecked(true);
		startButton.setVisibility(View.GONE);
		startButtonBar.setVisibility(View.GONE);
		startSampling();
	}

	public void exportToText(View view) {
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

			File bioImpDataDir = new File(Environment.getExternalStorageDirectory() + "/Biohm/");	

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
					+ fixedLengthString("Θ", 8)
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

	private void clearUI() {
		mGattServicesList.setAdapter((SimpleExpandableListAdapter) null);
		mDataField.setText(R.string.no_data);
	}

	private void updateConnectionState(final int resourceId) {
		runOnUiThread(new Runnable() {
			@Override
			public void run() {
				mConnectionState.setText(resourceId);
			}
		});
	}

	private void displayData(String data) {
		if (data != null) {
			mDataField.setText(data);
		}
	}

	private boolean findCharacteristic(String characteristicUUID, String referenceUUID) {
		byte[]characteristic;
		byte[] reference;
		boolean check = false;
		characteristic = characteristicUUID.getBytes();
		reference = referenceUUID.getBytes();
		for(int i = 0; i< characteristic.length; i++) {
			if(characteristic[i] == reference[i]) {
				check = true;
			}
			else {
				check = false;
				i = characteristic.length;
			}
		}
		return check;
	}


	// Iterates through and identifies the supported GATT Services/Characteristics.

	private void displayGattServices(List<BluetoothGattService> gattServices) {
		if (gattServices == null) return;
		String uuid = null;
		String unknownServiceString = getResources().getString(R.string.unknown_service);
		String unknownCharaString = getResources().getString(R.string.unknown_characteristic);
		ArrayList<HashMap<String, String>> gattServiceData = new ArrayList<HashMap<String, String>>();

		ArrayList<ArrayList<HashMap<String, String>>> gattCharacteristicData
		= new ArrayList<ArrayList<HashMap<String, String>>>();

		mGattCharacteristics = new ArrayList<ArrayList<BluetoothGattCharacteristic>>();

		// Loops through available GATT Services.
		for (BluetoothGattService gattService : gattServices) {
			HashMap<String, String> currentServiceData = new HashMap<String, String>();
			uuid = gattService.getUuid().toString();

			currentServiceData.put(LIST_NAME, SampleGattAttributes.lookup(uuid, unknownServiceString));
			currentServiceData.put(LIST_UUID, uuid);
			gattServiceData.add(currentServiceData);

			ArrayList<HashMap<String, String>> gattCharacteristicGroupData =
					new ArrayList<HashMap<String, String>>();

			List<BluetoothGattCharacteristic> gattCharacteristics =
					gattService.getCharacteristics();

			ArrayList<BluetoothGattCharacteristic> charas =
					new ArrayList<BluetoothGattCharacteristic>();

			// Loops through available Characteristics.
			for (BluetoothGattCharacteristic gattCharacteristic : gattCharacteristics) {	
				if((gattCharacteristic.getProperties() & BluetoothGattCharacteristic.PROPERTY_NOTIFY) > 0) {
					if(findCharacteristic(gattCharacteristic.getUuid().toString(), 
							SampleGattAttributes.BIOIMPEDANCE_DATA)) {
						mNotifyCharacteristic = gattCharacteristic;	
					}
				}
				if(
						(gattCharacteristic.getProperties() & BluetoothGattCharacteristic.PROPERTY_WRITE) > 0
						&&
						(gattCharacteristic.getProperties() & BluetoothGattCharacteristic.PROPERTY_READ) > 0) {

					if(findCharacteristic(gattCharacteristic.getUuid().toString(), 
							SampleGattAttributes.SAMPLE_RATE)) {
						mSampleRateCharacteristic = gattCharacteristic;
						mBluetoothLeService.readCharacteristic(mSampleRateCharacteristic);
					}
					else if(findCharacteristic(gattCharacteristic.getUuid().toString(), 
							SampleGattAttributes.AC_FREQ)) {
						mACFrequencyCharacteristic = gattCharacteristic;
						mBluetoothLeService.readCharacteristic(mACFrequencyCharacteristic);
					}
				}
				charas.add(gattCharacteristic);
				HashMap<String, String> currentCharaData = new HashMap<String, String>();
				uuid = gattCharacteristic.getUuid().toString();
				currentCharaData.put(
						LIST_NAME, SampleGattAttributes.lookup(uuid, unknownCharaString));
				currentCharaData.put(LIST_UUID, uuid);
				gattCharacteristicGroupData.add(currentCharaData);
			}
			mGattCharacteristics.add(charas);
			gattCharacteristicData.add(gattCharacteristicGroupData);
		}

		SimpleExpandableListAdapter gattServiceAdapter = new SimpleExpandableListAdapter(
				this,
				gattServiceData,
				android.R.layout.simple_expandable_list_item_2,
				new String[] {LIST_NAME, LIST_UUID},
				new int[] { android.R.id.text1, android.R.id.text2 },
				gattCharacteristicData,
				android.R.layout.simple_expandable_list_item_2,
				new String[] {LIST_NAME, LIST_UUID},
				new int[] { android.R.id.text1, android.R.id.text2 }
				);
		mGattServicesList.setAdapter(gattServiceAdapter);
	}

	public static String fixedLengthString(String string, int length) {
		return String.format("%-"+length+ "s", string);
	}

	public void setTextFileName() {
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

	private void setSampleRateTextView(int sampleRate) {
		sampleRateTextView.setText(String.valueOf(sampleRate) + " Hz");
	}

	public void setSampleRate() {
		// Create an instance of the dialog fragment and show it
		sampleRateDialog = new SampleRateFragment();
		Bundle args = new Bundle();
		args.putInt(DeviceControlActivity.EXTRA_SAMPLE_RATE_BINDER, sampleRate);
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
			setSampleRateTextView(sampleRate);
			mBluetoothLeService.writeCharacteristic(mSampleRateCharacteristic, sampleRate);
			updatePendingIntent(activityPendingIntent);		
		}
		catch (Exception e) {
			Toast.makeText(this, R.string.set_fail, Toast.LENGTH_SHORT).show();
		}
	}

	@Override
	public void onDialogNegativeClickSampleRate(DialogFragment dialog) {
		// User touched the dialog's negative button
	}

	private void setAcFreqTextViewParams(byte startFreq, byte stepSize, byte numOfIncrements) {
		startFreqTextView.setText(String.valueOf(startFreq) + " KHz");
		if(stepSize == 0) {
			stepSizeTextView.setText(R.string.not_applicable);
		}
		else {
			stepSizeTextView.setText(String.valueOf(stepSize) + " KHz");
		}
		if(numOfIncrements == 0) {
			numOfIncrementsTextView.setText(R.string.not_applicable);
		}
		else {
			numOfIncrementsTextView.setText(String.valueOf(numOfIncrements));
		}
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
			setAcFreqTextViewParams(startFreq, stepSize, numOfIncrements);
			Toast.makeText(this, R.string.freq_sweep_enabled, Toast.LENGTH_SHORT).show();
			mBluetoothLeService.writeCharacteristicArray(mACFrequencyCharacteristic, freqValuesByte);
			updatePendingIntent(activityPendingIntent);		
		}
		catch (Exception e) {
			if(e instanceof IllegalArgumentException){
				Toast.makeText(this, R.string.check_value, Toast.LENGTH_SHORT).show();
				Log.i(TAG, getStackTrace(e));
			}
			else {
				Toast.makeText(this, R.string.set_fail, Toast.LENGTH_SHORT).show();
				Log.i(TAG, getStackTrace(e));
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
			setAcFreqTextViewParams(startFreq, stepSize, numOfIncrements);
			mBluetoothLeService.writeCharacteristicArray(mACFrequencyCharacteristic, freqValuesByte);
			updatePendingIntent(activityPendingIntent);		
		}
		catch (Exception e) {
			if(e instanceof IllegalArgumentException) {
				Toast.makeText(this, R.string.no_zero, Toast.LENGTH_SHORT).show();
				Log.i(TAG, getStackTrace(e));
			}
			else {
				Toast.makeText(this, R.string.set_fail, Toast.LENGTH_SHORT).show();
				Log.i(TAG, getStackTrace(e));
			}
		}

	}

	/* Checks if external storage is available for read and write */
	public boolean isExternalStorageWritable() {
		String state = Environment.getExternalStorageState();
		if (Environment.MEDIA_MOUNTED.equals(state)) {
			return true;
		}
		return false;
	}

	private void initializeViewComponents() {	
		startButton = (Button) findViewById(R.id.begin);
		startButtonBar = (View) findViewById(R.id.begin_bar);
		enableNotifications  = (SwitchCompat) findViewById(R.id.enable_notifications);
		connectionStateBar = (ProgressBar) findViewById(R.id.connection_state_bar);
		bioimpedancePlot = (com.androidplot.xy.XYPlot) findViewById(R.id.bioimpedancePlot);
		textFileButtons = (RelativeLayout) findViewById(R.id.buttons);
		mGattServicesList = (ExpandableListView) findViewById(R.id.gatt_services_list);
		mConnectionState = (TextView) findViewById(R.id.connection_state);
		mDataField = (TextView) findViewById(R.id.data_value);
		mDeviceAddressTextView = (TextView) findViewById(R.id.device_address);
		sampleRateTextView = (TextView) findViewById(R.id.sample_rate);
		startFreqTextView = (TextView) findViewById(R.id.start_freq);
		stepSizeTextView = (TextView) findViewById(R.id.step_size);
		numOfIncrementsTextView = (TextView) findViewById(R.id.num_of_increments);
		summaryTable = (TableLayout) findViewById(R.id.summary_table);
		summaryTableHeader = (TextView) findViewById(R.id.parameter_summary);
	}

	private void updatePendingIntent(PendingIntent activityPendingIntent) {			
		activityIntent = new Intent(this, LongTerm.class);

		activityIntent.removeExtra(EXTRA_SAMPLE_RATE_BINDER);
		activityIntent.removeExtra(EXTRA_START_FREQ_BINDER);
		activityIntent.removeExtra(EXTRA_STEP_SIZE_BINDER);
		activityIntent.removeExtra(EXTRA_NUM_OF_INCREMENTS_BINDER);

		activityIntent.putExtra(EXTRA_SAMPLE_RATE_BINDER, sampleRate);
		activityIntent.putExtra(EXTRA_START_FREQ_BINDER, startFreq);
		activityIntent.putExtra(EXTRA_STEP_SIZE_BINDER, stepSize);
		activityIntent.putExtra(EXTRA_NUM_OF_INCREMENTS_BINDER, numOfIncrements);

		activityPendingIntent = PendingIntent.getActivity(this, 0, activityIntent, PendingIntent.FLAG_UPDATE_CURRENT);
	}

	private void startSampling() {
		serviceIntent = new Intent(this, ServiceBinder.class);
		serviceIntent.putExtra(EXTRA_DEVICE_ADDRESS_BINDER, mDeviceAddress);
		serviceIntent.putExtra(EXTRA_DEVICE_NAME_BINDER, mDeviceName);
		startService(serviceIntent);

		notification = new Notification(R.drawable.ic_notification, getText(R.string.sampling_data),System.currentTimeMillis());
		activityIntent = new Intent(this, LongTerm.class);

		activityIntent.putExtra(EXTRA_DEVICE_ADDRESS_BINDER, mDeviceAddress);
		activityIntent.putExtra(EXTRA_DEVICE_NAME_BINDER, mDeviceName);
		activityIntent.putExtra(EXTRA_SAMPLE_RATE_BINDER, sampleRate);
		activityIntent.putExtra(EXTRA_START_FREQ_BINDER, startFreq);
		activityIntent.putExtra(EXTRA_STEP_SIZE_BINDER, stepSize);
		activityIntent.putExtra(EXTRA_NUM_OF_INCREMENTS_BINDER, numOfIncrements);

		activityPendingIntent = PendingIntent.getActivity(this, 0, activityIntent, PendingIntent.FLAG_CANCEL_CURRENT);
		notification.setLatestEventInfo(this, getText(R.string.sampling_data), getText(R.string.connected), activityPendingIntent);
		mBluetoothLeService.startForeground(ONGOING_NOTIFICATION_ID, notification);

		bioimpedancePlot.setVisibility(View.VISIBLE);
		textFileButtons.setVisibility(View.VISIBLE);

		mBluetoothLeService.setCharacteristicNotification(mNotifyCharacteristic, true);	
	}

	private void setupPlot() {
		Paint bgPaint = new Paint();
		bgPaint.setColor(Color.parseColor("#d8d8d8"));
		bgPaint.setStyle(Paint.Style.FILL);

		accelXseries = new SimpleXYSeries("X");
		accelXseries.useImplicitXVals();
		accelYseries = new SimpleXYSeries("Y");
		accelYseries.useImplicitXVals();
		accelZseries = new SimpleXYSeries("Z");
		accelZseries.useImplicitXVals();
		bioimpedanceSeries = new SimpleXYSeries("Ω");
		bioimpedanceSeries.useImplicitXVals();

		bioimpedancePlot.setRangeBoundaries(-100, 600, BoundaryMode.FIXED);
		bioimpedancePlot.setDomainBoundaries(0, 360, BoundaryMode.FIXED);

		// Format general area
		bioimpedancePlot.setBackgroundColor(Color.WHITE);
		bioimpedancePlot.getBackgroundPaint().set(bgPaint);
		bioimpedancePlot.getBackgroundPaint().setColor(Color.parseColor("#d8d8d8"));
		bioimpedancePlot.getGraphWidget().getBackgroundPaint().setColor(Color.parseColor("#d8d8d8"));
		bioimpedancePlot.getGraphWidget().setGridBackgroundPaint(null);
		bioimpedancePlot.setBorderStyle(XYPlot.BorderStyle.SQUARE, null, null);
		bioimpedancePlot.getGraphWidget().setPadding(12, 12, 12, 12);
		bioimpedancePlot.getTitleWidget().setText("");

		bioimpedancePlot.setBorderPaint(bgPaint);
		//bioimpedancePlot.getGraphWidget().getBorderPaint().setColor(Color.TRANSPARENT);

		// Format domain
		bioimpedancePlot.getDomainLabelWidget().getLabelPaint().setColor(Color.parseColor("#006bb2"));
		bioimpedancePlot.getDomainLabelWidget().getLabelPaint().setTextSize(20);
		bioimpedancePlot.getGraphWidget().getDomainLabelPaint().setColor(Color.BLACK);
		bioimpedancePlot.getGraphWidget().getDomainLabelPaint().setTextSize(20);
		bioimpedancePlot.getGraphWidget().getDomainOriginLabelPaint().setColor(Color.BLACK);
		bioimpedancePlot.getGraphWidget().getDomainOriginLinePaint().setColor(Color.BLACK);
		bioimpedancePlot.getGraphWidget().getDomainGridLinePaint().setColor(Color.TRANSPARENT);
		bioimpedancePlot.setDomainStepValue(5);
		bioimpedancePlot.setDomainLabel("Sample Index");
		bioimpedancePlot.getDomainLabelWidget().pack();


		// Format range
		bioimpedancePlot.getRangeLabelWidget().getLabelPaint().setColor(Color.parseColor("#006bb2"));
		bioimpedancePlot.getRangeLabelWidget().getLabelPaint().setTextSize(20);
		bioimpedancePlot.getGraphWidget().getRangeLabelPaint().setColor(Color.BLACK);
		bioimpedancePlot.getGraphWidget().getRangeLabelPaint().setTextSize(20);
		bioimpedancePlot.getGraphWidget().getRangeOriginLabelPaint().setColor(Color.BLACK);
		bioimpedancePlot.getGraphWidget().getRangeOriginLinePaint().setColor(Color.BLACK);
		bioimpedancePlot.getGraphWidget().getRangeGridLinePaint().setColor(Color.TRANSPARENT);
		bioimpedancePlot.getGraphWidget().getRangeSubGridLinePaint().setColor(Color.TRANSPARENT);
		bioimpedancePlot.setRangeLabel("Data");
		bioimpedancePlot.setTicksPerRangeLabel(3);
		bioimpedancePlot.getRangeLabelWidget().pack();

		// Format legend

		bioimpedancePlot.getLegendWidget().getTextPaint().setColor(Color.parseColor("#006bb2"));
		bioimpedancePlot.getLegendWidget().getTextPaint().setTextSize(20);
		bioimpedancePlot.getLegendWidget().setPaddingBottom(10);

		// Add series
		bioimpedancePlot.addSeries(accelXseries, new LineAndPointFormatter(Color.parseColor("#008b8b"), null, null, null));
		bioimpedancePlot.addSeries(accelYseries, new LineAndPointFormatter(Color.parseColor("#8b008b"), null, null, null));
		bioimpedancePlot.addSeries(accelZseries, new LineAndPointFormatter(Color.parseColor("#8b8b00"), null, null, null));
		bioimpedancePlot.addSeries(bioimpedanceSeries, new LineAndPointFormatter(Color.parseColor("#006bb2"), null, null, null));

		bioimpedancePlot.getBackgroundPaint().set(bgPaint);

		// Debugging reference 

		//bioimpedancePlot.getDomainLabelWidget().getBackgroundPaint().setColor(Color.RED);
		//bioimpedancePlot.getDomainLabelWidget().getBorderPaint().setColor(Color.BLUE);
		//bioimpedancePlot.getLegendWidget().setBackgroundPaint(bgPaint);
		//bioimpedancePlot.getRangeLabelWidget().setBackgroundPaint(bgPaint);
		//bioimpedancePlot.getDomainLabelWidget().setBackgroundPaint(bgPaint);
		//bioimpedancePlot.getTitleWidget().setBackgroundPaint(bgPaint);
	}

	private static IntentFilter makeGattUpdateIntentFilter() {
		final IntentFilter intentFilter = new IntentFilter();
		intentFilter.addAction(BluetoothLeService.ACTION_GATT_CONNECTED);
		intentFilter.addAction(BluetoothLeService.ACTION_GATT_CONNECTING);
		intentFilter.addAction(BluetoothLeService.ACTION_GATT_DISCONNECTED);
		intentFilter.addAction(BluetoothLeService.ACTION_GATT_SERVICES_DISCOVERED);
		intentFilter.addAction(BluetoothLeService.ACTION_DATA_AVAILABLE);
		intentFilter.addAction(BluetoothLeService.ACTION_DATA_AVAILABLE_SAMPLE_RATE);
		intentFilter.addAction(BluetoothLeService.ACTION_DATA_AVAILABLE_FREQUENCY_PARAMS);
		intentFilter.addAction(BluetoothLeService.ACTION_DATA_AVAILABLE_BIOIMPEDANCE);
		return intentFilter;
	}

	private static String getStackTrace(final Throwable throwable) {
		final StringWriter sw = new StringWriter();
		final PrintWriter pw = new PrintWriter(sw, true);
		throwable.printStackTrace(pw);
		return sw.getBuffer().toString();
	}

}