package com.shemanigans.mime;

import java.io.File;
import java.io.FileOutputStream;
import java.io.OutputStreamWriter;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.HashMap;
import java.util.List;

import android.support.v4.app.DialogFragment;
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
import android.support.v7.app.ActionBarActivity;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.ExpandableListView;
import android.widget.RelativeLayout;
import android.widget.SimpleExpandableListAdapter;
import android.widget.Switch;
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

	//	implements NameTextFileFragment.NameTextFileListener

	private final static String TAG = DeviceControlActivity.class.getSimpleName();

	public static final String EXTRAS_DEVICE_NAME = "DEVICE_NAME";
	public static final String EXTRAS_DEVICE_ADDRESS = "DEVICE_ADDRESS";

	public static final String EXTRA_DEVICE_NAME_BINDER = "DEVICE_NAME_BINDER";
	public static final String EXTRA_DEVICE_ADDRESS_BINDER = "DEVICE_ADDRESS_BINDER";
	public static final String EXTRA_SAMPLE_RATE_BINDER = "SAMPLE_RATE_BINDER";
	public static final String EXTRA_START_FREQ_BINDER = "START_FREQ_BINDER";
	public static final String EXTRA_STEP_SIZE_BINDER = "STEP_SIZE_BINDER";
	public static final String EXTRA_NUM_OF_INCREMENTS_BINDER = "NUM_OF_INCREMENTS_BINDER";
	//public final static String EXTRA_MESSAGE = "EXPORT_TO_TEXT";

	private TextView mConnectionState;
	private TextView mDataField;
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
	private String textFileName = "AccelData";	
	private Calendar c = Calendar.getInstance();
	private boolean checkNamedTextFile = false;
	private double[] imp = {1, 2, 3, 4};
	private int sampleRate = 50;
	private byte startFreq = 0;
	private byte stepSize = 0;
	private byte numOfIncrements = 0;

	private Intent serviceIntent; 
	private Intent activityIntent;
	private PendingIntent activityPendingIntent;
	private Notification notification;

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

	// Code to manage Service lifecycle.
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
	// ACTION_DATA_AVAILABLE: received data from the device.  This can be a result of read
	//                        or notification operations.
	private final BroadcastReceiver mGattUpdateReceiver = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent intent) {
			final String action = intent.getAction();
			if (BluetoothLeService.ACTION_GATT_CONNECTED.equals(action)) {
				mConnected = true;
				updateConnectionState(R.string.connected);
				Switch enableNotifications = (Switch) findViewById(R.id.enable_notifications);
				enableNotifications.setClickable(true);
				invalidateOptionsMenu();
			} 
			else if (BluetoothLeService.ACTION_GATT_CONNECTING.equals(action)) {
				mConnected = false;
				//updateConnectionState(R.string.connecting);
				invalidateOptionsMenu();
			}
			else if (BluetoothLeService.ACTION_GATT_DISCONNECTED.equals(action)) {
				mConnected = false;
				updateConnectionState(R.string.disconnected);
				Switch enableNotifications = (Switch) findViewById(R.id.enable_notifications);
				enableNotifications.setChecked(false);
				enableNotifications.setClickable(false);
				mBluetoothLeService.stopForeground(true);
				invalidateOptionsMenu();
				clearUI();
			} 
			else if (BluetoothLeService.ACTION_GATT_SERVICES_DISCOVERED.equals(action)) {
				// Show all the supported services and characteristics on the user interface.
				displayGattServices(mBluetoothLeService.getSupportedGattServices());
			} 
			else if (BluetoothLeService.ACTION_DATA_AVAILABLE.equals(action)) {
				displayData(intent.getStringExtra(BluetoothLeService.EXTRA_DATA));
			}
			else if (BluetoothLeService.ACTION_DATA_AVAILABLE_BIOIMPEDANCE.equals(action)) {
				displayData("\n"
						+ fixedLengthString("X", 6)
						+ fixedLengthString("Y", 6)
						+ fixedLengthString("Z", 6)
						+ fixedLengthString("Ω", 7)
						+ fixedLengthString("θ", 6)
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
		Log.i(TAG, "Activity started afresh.");	

		Intent gattServiceIntent = new Intent(this, BluetoothLeService.class);
		bindService(gattServiceIntent, mServiceConnection, BIND_AUTO_CREATE);
		setContentView(R.layout.gatt_services_characteristics);

		final Intent intent = getIntent();
		mDeviceName = intent.getStringExtra(EXTRAS_DEVICE_NAME);
		mDeviceAddress = intent.getStringExtra(EXTRAS_DEVICE_ADDRESS);
		Log.i(TAG, "Names written for the first time.");

		// Sets up UI references.
		((TextView) findViewById(R.id.device_address)).setText(mDeviceAddress);
		mGattServicesList = (ExpandableListView) findViewById(R.id.gatt_services_list);
		mGattServicesList.setOnChildClickListener(servicesListClickListner);
		mConnectionState = (TextView) findViewById(R.id.connection_state);
		mDataField = (TextView) findViewById(R.id.data_value);
		mDeviceName = mDeviceName.substring(0, mDeviceName.length() - 9);

		getSupportActionBar().setTitle(mDeviceName);
		getSupportActionBar().setDisplayHomeAsUpEnabled(true);

		// Set up bioimpedance plots

		Paint bgPaint = new Paint();
		bgPaint.setColor(Color.parseColor("#d8d8d8"));
		bgPaint.setStyle(Paint.Style.FILL);

		bioimpedancePlot = (XYPlot) findViewById(R.id.bioimpedancePlot);

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

	@Override
	protected void onResume() {
		super.onResume();
		registerReceiver(mGattUpdateReceiver, makeGattUpdateIntentFilter());
		if (mBluetoothLeService != null) {
			//final boolean result = mBluetoothLeService.connect(mDeviceAddress);
			//Log.d(TAG, "Connect request result=" + result);
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
		if(mBluetoothLeService.getNumberOfBoundClients() == 2) {
			mBluetoothLeService.setDeviceName(mDeviceName);
			mBluetoothLeService.setDeviceAdress(mDeviceAddress);
			Log.i(TAG, "Names have been saved.");
		}
		Log.i(TAG, "Activity destroyed.");
		//mBluetoothLeService = null;
	}

	@Override
	public void onSaveInstanceState(Bundle savedInstanceState) {
		mBluetoothLeService.setDeviceName(mDeviceName);
		mBluetoothLeService.setDeviceAdress(mDeviceAddress);
		Log.i(TAG, "Attempted state and name save.");
		// Save the user's current game state
		// Always call the superclass so it can save the view hierarchy state
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

					Switch enableNotifications = (Switch) findViewById(R.id.enable_notifications);
					enableNotifications.setChecked(false);
					enableNotifications.setVisibility(View.GONE); //

					if (mNotifyCharacteristic != null) {
						mBluetoothLeService.removeCharacteristicNotification(mNotifyCharacteristic, false);
						mNotifyCharacteristic = null;
					}
					mBluetoothLeService.readCharacteristic(characteristic);
				}
				if ((charaProp & BluetoothGattCharacteristic.PROPERTY_NOTIFY) > 0) {

					Switch enableNotifications = (Switch) findViewById(R.id.enable_notifications);
					enableNotifications.setVisibility(View.VISIBLE);

					mNotifyCharacteristic = characteristic;
				}
				if ((charaProp & BluetoothGattCharacteristic.PROPERTY_WRITE) > 0) {
					//Log.i(TAG, characteristic.getUuid().toString());
				}
				return true;
			}
			return false;
		}
	};

	public void enableNotifications(View view) {	

		Switch enableNotifications = (Switch) findViewById(R.id.enable_notifications);
		enableNotifications.setVisibility(View.VISIBLE);

		// Turn off notifications.
		if(enableNotifications.isChecked() == false) {		
			Intent intent = new Intent(this, ServiceBinder.class);
			stopService(intent);

			RelativeLayout buttons = (RelativeLayout) findViewById(R.id.buttons);
			buttons.setVisibility(View.VISIBLE);

			mBluetoothLeService.removeCharacteristicNotification(mNotifyCharacteristic, false);
			//mNotifyCharacteristic = null;
			mBluetoothLeService.stopForeground(true);
		}

		// Turn on notifications.
		else {
			//ic_action_name.png
			updateNotifications(serviceIntent, activityIntent, activityPendingIntent, notification);		
		}
	}

	public void startSampling(View view) {
		Switch enableNotifications = (Switch) findViewById(R.id.enable_notifications);
		enableNotifications.setVisibility(View.VISIBLE);
		enableNotifications.setChecked(true);

		Button start = (Button) findViewById(R.id.begin);
		start.setVisibility(View.GONE);
		View start_bar = (View) findViewById(R.id.begin_bar);
		start_bar.setVisibility(View.GONE);

		updateNotifications(serviceIntent, activityIntent, activityPendingIntent, notification);	
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

			File accelDataDir = new File(Environment.getExternalStorageDirectory() + "/Biohm/");	

			accelDataDir.mkdirs();			

			File accelData = new File(accelDataDir, textFileName + ".txt");			

			accelData.createNewFile();
			FileOutputStream fOut = new FileOutputStream(accelData);
			OutputStreamWriter myOutWriter = new OutputStreamWriter(fOut);

			myOutWriter.append(
					fixedLengthString("X", 6) 
					+ fixedLengthString("Y", 6)
					+ fixedLengthString("Z", 6)
					+ fixedLengthString("Ω", 7)
					+ fixedLengthString("Θ", 6)
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


	// Demonstrates how to iterate through the supported GATT Services/Characteristics.
	// In this sample, we populate the data structure that is bound to the ExpandableListView
	// on the UI.

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

			currentServiceData.put(
					LIST_NAME, SampleGattAttributes.lookup(uuid, unknownServiceString));

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

				if ((gattCharacteristic.getProperties() & BluetoothGattCharacteristic.PROPERTY_NOTIFY) > 0) {
					mNotifyCharacteristic = gattCharacteristic;			
				}
				if ((gattCharacteristic.getProperties() & BluetoothGattCharacteristic.PROPERTY_WRITE) > 0) {

					if(findCharacteristic(gattCharacteristic.getUuid().toString(), 
							SampleGattAttributes.SAMPLE_RATE)) {
						mSampleRateCharacteristic = gattCharacteristic;
					}
					if(findCharacteristic(gattCharacteristic.getUuid().toString(), 
							SampleGattAttributes.AC_FREQ)) {
						mACFrequencyCharacteristic = gattCharacteristic;
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

	public void setSampleRate() {
		// Create an instance of the dialog fragment and show it
		Switch enableNotifications = (Switch) findViewById(R.id.enable_notifications);
		if(!enableNotifications.isChecked()) {
			Toast.makeText(this, "Notifications are not enabled.", Toast.LENGTH_SHORT).show();
		}	
		else {
			sampleRateDialog = new SampleRateFragment();
			Bundle args = new Bundle();
			args.putInt(DeviceControlActivity.EXTRA_SAMPLE_RATE_BINDER, sampleRate);
			sampleRateDialog.setArguments(args);
			sampleRateDialog.show(getSupportFragmentManager(), "SampleRateFragment");
		}
	}

	@Override
	public void onDialogPositiveClickSampleRate(DialogFragment dialog) {
		// User touched the dialog's positive button
		// Set Value
		try {
			sampleRate = Integer.parseInt(sampleRateDialog.getValue());
			Log.i(TAG, "This value was passed back: " + String.valueOf(sampleRate));
			Toast.makeText(this, "New frequency: " + sampleRateDialog.getValue(), Toast.LENGTH_SHORT).show();
			mBluetoothLeService.writeCharacteristic(mSampleRateCharacteristic, sampleRate);
			updateNotifications(serviceIntent, activityIntent, activityPendingIntent, notification);		
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
		Switch enableNotifications = (Switch) findViewById(R.id.enable_notifications);
		if(!enableNotifications.isChecked()) {
			Toast.makeText(this, "Notifications are not enabled.", Toast.LENGTH_SHORT).show();
		}	
		else {
			frequencySweepDialog = new FrequencySweepFragment();
			Bundle args = new Bundle();
			args.putString(DeviceControlActivity.EXTRA_START_FREQ_BINDER, Byte.toString(startFreq));
			args.putString(DeviceControlActivity.EXTRA_STEP_SIZE_BINDER, Byte.toString(stepSize));
			args.putString(DeviceControlActivity.EXTRA_NUM_OF_INCREMENTS_BINDER, Byte.toString(numOfIncrements));
			frequencySweepDialog.setArguments(args);
			frequencySweepDialog.show(getSupportFragmentManager(), "FrequencySweepFragment");
		}
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
			if(startFreq + (stepSize * numOfIncrements) > 100) {
				throw new IllegalArgumentException();
			}
			byte[] freqValuesByte = {startFreq, stepSize, numOfIncrements}; 
			Toast.makeText(this, R.string.freq_sweep_enabled, Toast.LENGTH_SHORT).show();
			mBluetoothLeService.writeCharacteristicArray(mACFrequencyCharacteristic, freqValuesByte);
			updateNotifications(serviceIntent, activityIntent, activityPendingIntent, notification);		
		}
		catch (Exception e) {
			if(e instanceof IllegalArgumentException){
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
			mBluetoothLeService.writeCharacteristicArray(mACFrequencyCharacteristic, freqValuesByte);
			updateNotifications(serviceIntent, activityIntent, activityPendingIntent, notification);		
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

	/* Checks if external storage is available for read and write */
	public boolean isExternalStorageWritable() {
		String state = Environment.getExternalStorageState();
		if (Environment.MEDIA_MOUNTED.equals(state)) {
			return true;
		}
		return false;
	}

	public void updateNotifications(
			Intent serviceIntent, 
			Intent activityIntent,
			PendingIntent activityPendingIntent,
			Notification notification) {

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

		com.androidplot.xy.XYPlot data = (com.androidplot.xy.XYPlot) findViewById(R.id.bioimpedancePlot);
		data.setVisibility(View.VISIBLE);

		RelativeLayout buttons = (RelativeLayout) findViewById(R.id.buttons);
		buttons.setVisibility(View.VISIBLE);

		mBluetoothLeService.setCharacteristicNotification(mNotifyCharacteristic, true);	

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